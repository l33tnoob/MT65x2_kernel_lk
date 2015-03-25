#include <linux/init.h>
#include <linux/kmempagerecorder.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <asm/fixmap.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#include <linux/irqflags.h>
#include <linux/spinlock.h>

#include <linux/device.h>
#include <linux/file.h>
#include <linux/freezer.h>
#include <linux/fs.h>
#include <linux/anon_inodes.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/memblock.h>
#include <linux/miscdevice.h>
#include <linux/export.h>
#include <linux/rbtree.h>
#include <linux/rtmutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#define BACKTRACE_LEVEL 10
#define DEBUG_DEFAULT_FLAGS 1
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL


extern void * high_memory;
PageHashTable gPageHashTable;
PageObjectTable gKernelPageSymbolTable;
PageObjectTable gKernelPageBtTable;
static struct kmem_cache *page_cachep = NULL;
static unsigned int page_cache_created = false;

static unsigned int Object_rank_max = 10;
static unsigned int queried_address = 0;
static unsigned int debug_log = 0;
static struct dentry *debug_root;
static unsigned int page_record_total = 0;
static unsigned int page_record_max = 0;
static unsigned int page_record_count = 0;
static unsigned int bt_record_total = 0;
static unsigned int bt_record_max = 0;

//init hash table mutex
unsigned int page_record_lock_init = 0;
unsigned int bt_record_lock_init = 0;
unsigned int symbol_record_lock_init = 0;
spinlock_t page_record_lock;
spinlock_t bt_record_lock;
spinlock_t symbol_record_lock;
int page_recorder_debug = DEBUG_DEFAULT_FLAGS;
unsigned int page_recorder_memory_usage = 0;
unsigned int page_recorder_limit = 524288;
static char page_recorder_debug_function;

static int page_recorder_debug_show(struct seq_file *s, void *unused);
static inline unsigned int hash_32(unsigned int val, unsigned int bits);
static inline PageHashEntry* find_page_entry(void* page, int slot);
static char page_recorder_debug_function;

void disable_page_alloc_tracer(void)
{
	page_recorder_debug = 0;
}

static int page_recorder_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, page_recorder_debug_show, inode->i_private);
}

static const struct file_operations debug_page_recorder_fops = {
	.open = page_recorder_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int query_page_backtrace(struct seq_file *s,unsigned int *page)
{
	char symbol[KSYM_SYMBOL_LEN];
	unsigned long flags;
	unsigned int *backtrace ;
	unsigned int i;
	unsigned int hash = hash_32((unsigned int)page, 16);
	unsigned int slot = hash % OBJECT_TABLE_SIZE;

	PageObjectEntry *bt_entry = NULL;
	PageHashEntry* entry = NULL;
	seq_printf(s, "%s %x\n","query Page address:",(unsigned int)page);

	//search page record in hash table
	spin_lock_irqsave(&page_record_lock,flags);
	entry = find_page_entry(page, slot);
	if(entry != NULL)
	{
		bt_entry = entry->bt_entry;
		backtrace = (unsigned int *)bt_entry->object;
		seq_printf(s, "%x allocate %d %s\n",(unsigned int)entry->page,entry->size*4096,"bytes and backtrace is ");
		for(i = 0;i < bt_entry->numEntries;i++)
		{
			sprint_symbol(symbol,*(backtrace+i));
			seq_printf(s, "  KERNEL[%d] 0x%x :: symbol %s\n", i,backtrace[i],symbol);
		}
	}
	else
	{
		seq_printf(s, "can't get page(0x%x) backtrace information\n",(unsigned int)page);
	}
	spin_unlock_irqrestore(&page_record_lock,flags);
	return 0;
}

static struct page *fixmap_virt_to_page(const void *fixmap_addr)
{
	unsigned long addr = (unsigned long) fixmap_addr;
	struct page *page = NULL;
	pgd_t *pgd = pgd_offset_k(addr);
 
	if (!pgd_none(*pgd)) 
	{
		pud_t *pud = pud_offset(pgd, addr);
		if (!pud_none(*pud)) 
		{
			pmd_t *pmd = pmd_offset(pud, addr);
			if (!pmd_none(*pmd)) 
			{
				pte_t *ptep, pte;
				ptep = pte_offset_map(pmd, addr);
				pte = *ptep;
				if (pte_present(pte))
					page = pte_page(pte);
				pte_unmap(ptep);
			}
		}
	}
	return page;
}

static int query_page_bt_open(struct seq_file *s, void *data)
{
	unsigned int *page_address = NULL;
	seq_printf(s, "queried_page is  : %x\n", queried_address);

	if(is_vmalloc_or_module_addr((const void *)queried_address)) // vmalloc
	{
		seq_printf(s, "[vmalloc or module]queried_page is  : %x\n", queried_address);
		page_address = (unsigned int *)vmalloc_to_page((unsigned int *)(queried_address&0xfffff000));
	}
        else if((queried_address >= 0xC0000000) && (queried_address <= (unsigned int)high_memory ))//lowmem 
        {
		seq_printf(s, "[lowmem]queried_page is  : %x\n", queried_address);
		page_address = (unsigned int *)virt_to_page((void*)(queried_address&0xfffff000 ));
        }
	else if((queried_address >= FIXADDR_START) && (queried_address <= FIXADDR_TOP)) //fixmap
	{
		seq_printf(s, "[fixmap]queried_page is  : %x\n", queried_address);
		page_address = (unsigned int *)fixmap_virt_to_page((const void *)(queried_address&0xfffff000));	
	}
	else if((queried_address >= PKMAP_BASE) && (queried_address)<= PKMAP_ADDR(LAST_PKMAP))//pkmap
	{
		seq_printf(s, "[pkmap]queried_page is  : %x\n", queried_address);
		page_address = (unsigned int *)fixmap_virt_to_page((const void *)(queried_address&0xfffff000));
	}
	else
	{
		seq_printf(s, "[ERROR!!]queried_page is  : %x can't find address in memory map\n", queried_address);
	}
	query_page_backtrace(s,(unsigned int*)page_address);
	return 0;
}

static int query_page_single_open(struct inode *inode, struct file *file)
{
	return single_open(file, query_page_bt_open, inode->i_private);
}

static const struct file_operations query_page_ios_fops = {
	.open = query_page_single_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static unsigned int get_kernel_backtrace(unsigned long *backtrace,unsigned int debug)
{
	unsigned long stack_entries[BACKTRACE_LEVEL];
	unsigned int i = 0;
	char tmp[KSYM_SYMBOL_LEN];
	struct stack_trace trace = {
		.nr_entries = 0,
		.entries = &stack_entries[0],
		.max_entries = BACKTRACE_LEVEL,
		.skip = 3
	};
	save_stack_trace(&trace);
	if(trace.nr_entries > 0)
	{
		if(debug)
		{
			for(i= 0 ; i < trace.nr_entries; i++)
			{
				sprint_symbol(tmp,trace.entries[i]);
				//printk("[%d] 0x%x %s\n",i,(unsigned int)trace.entries[i],tmp);
			}
		}
		else
		{
			memcpy(backtrace,(unsigned long *)trace.entries,sizeof(unsigned int)*trace.nr_entries);
		}
	}
	else
	{
		printk("[ERROR]can't get backtrace [get_kernel_backtrace] backtrace num: [%d]\n",trace.nr_entries);
	}
	return trace.nr_entries;
}

static inline unsigned int hash_32(unsigned int val, unsigned int bits)
{
	/* On some cpus multiply is faster, on others gcc will do shifts */
	unsigned int hash = val * GOLDEN_RATIO_PRIME_32;

	/* High bits are more random, so use them. */
	return hash >> (32 - bits);
}

static uint32_t get_hash(void* object, size_t numEntries)
{
	unsigned int *backtrace = NULL;
	unsigned int hash = 0;
	size_t i;

	backtrace = (unsigned int *)object;
	if (backtrace == NULL) 
	{
		return 0;
	}
	for (i = 0 ; i < numEntries ; i++) 
	{
		hash = (hash * 33) + (*(backtrace+i) >> 2);
	}
	return hash;
}

PageObjectEntry* find_entry(PageObjectTable* table, unsigned int slot,void* object,unsigned int  numEntries,unsigned int size)
{
	PageObjectEntry* entry = table->slots[slot];
	while (entry != NULL) 
	{
		if ( entry->numEntries == numEntries &&
		!memcmp(object, entry->object, numEntries * sizeof(unsigned int))) 
		{
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}

static void *allocate_record(unsigned int type)
{
	switch(type)
	{
		case NODE_PAGE_RECORD:
		{
			if(!page_cache_created)
			{
				page_cachep = kmem_cache_create("page_record",sizeof(PageHashEntry),0,SLAB_NO_DEBUG,NULL);
				page_cache_created = true;
			}
			//if system ram < 2G, page_record_total should less than 524288
			if((page_cachep != NULL) && (page_record_total < page_recorder_limit ))
			{
				void *tmp = NULL;
				tmp  = (void *)kmem_cache_alloc(page_cachep,GFP_KERNEL);
				if(tmp == 0)
				{
					return NULL;
				}
				return tmp;
			}
			return NULL;
			break;
		}
	}
	return NULL;
}
//get record from hash table or create new node from slab allocator
static void *get_record(unsigned int type, page_record_t *param)
{
	page_record_t *tmp = param;
	PageObjectEntry *entry = NULL;
	unsigned int hash;
	unsigned int slot;
	unsigned long flags;
	
	if(tmp != NULL)
	{
		switch(type)
		{
			case HASH_PAGE_NODE_KERNEL_PAGE_ALLOC_BACKTRACE:
			{
				hash = get_hash(param->backtrace,param->backtrace_num);
				slot = hash % OBJECT_TABLE_SIZE;
				spin_lock_irqsave(&bt_record_lock,flags);	
				entry = find_entry(&gKernelPageBtTable,slot,(void *)param->backtrace,param->backtrace_num,param->size);
				if(entry != NULL)
				{
					entry->reference++;
					entry->size = entry->size + (1 << param->size);
					spin_unlock_irqrestore(&bt_record_lock,flags);
				}
				else
				{
					spin_unlock_irqrestore(&bt_record_lock,flags);

					//total bt reocrd size should less than 5MB
					if(bt_record_total < 50412)
					{
						entry = kmalloc(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)),GFP_KERNEL);
						if(entry == NULL)
						{
							printk( "[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
							return NULL;
						}
					}
					else
					{
						return NULL;
					}

					//kmalloc can't get right memory space when booting
					if((unsigned int)entry < 0xC0000000)
					{
						//printk("[BAKCTRACEINFO][allocate bt mem] entry (0x%x) drop address \n",(unsigned int)entry);
						return NULL;
					}
					entry->reference = 1;
					entry->prev = NULL;
					entry->slot = slot;
					entry->numEntries = param->backtrace_num;
					entry->size = 1 << param->size;
					memcpy(entry->object,param->backtrace,entry->numEntries * sizeof(unsigned int));
					spin_lock_irqsave(&bt_record_lock,flags);
					entry->next = gKernelPageBtTable.slots[slot];
					gKernelPageBtTable.slots[slot] = entry;
					if(entry->next != NULL)
					{
						entry->next->prev = entry;
					}
					gKernelPageBtTable.count++;
					bt_record_total++;
					if(bt_record_total > bt_record_max)
					{
						bt_record_max = bt_record_total;
					}
					spin_unlock_irqrestore(&bt_record_lock,flags);
				}
				return entry;
			}
			case HASH_PAGE_NODE_KERNEL_SYMBOL:
			{
				hash = get_hash(param->kernel_symbol,param->backtrace_num);
				slot = hash % OBJECT_TABLE_SIZE;
				spin_lock_irqsave(&symbol_record_lock,flags);
				entry = find_entry(&gKernelPageSymbolTable,slot,(void *)param->kernel_symbol,param->backtrace_num,param->size);
				if(entry != NULL)
				{
					entry->reference++;
					spin_unlock_irqrestore(&symbol_record_lock,flags);
					return NULL;
				}
				else
				{
					spin_unlock_irqrestore(&symbol_record_lock,flags);
					entry = kmalloc(sizeof(PageObjectEntry)+(param->backtrace_num * sizeof(unsigned int)),GFP_KERNEL);
					if(entry == NULL)
					{
						printk( "[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
						return NULL;
					}
					entry->reference = 1;
					entry->prev = NULL;
					entry->slot = slot;
					entry->numEntries = param->backtrace_num;
					memcpy(entry->object,param->kernel_symbol,entry->numEntries * sizeof(unsigned int));
					spin_lock_irqsave(&symbol_record_lock,flags);
					entry->next = gKernelPageSymbolTable.slots[slot];
					gKernelPageSymbolTable.slots[slot] = entry;
					if(entry->next != NULL)
					{
						entry->next->prev = entry;
					}
					gKernelPageSymbolTable.count++;
					spin_unlock_irqrestore(&symbol_record_lock,flags);
				}
			}
		}
	}
	return NULL;
}

static inline PageHashEntry* find_page_entry(void* page, int slot)
{
	PageHashEntry* entry = gPageHashTable.page_hash_table[slot];
	while (entry != NULL) 
	{
		if (entry->page == page) 
		{
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}

PageHashEntry* record_page_info(PageObjectEntry* bt_entry,PageObjectEntry* map_entry, void* page, unsigned int order, unsigned int flag)
{
	// calculate the hash value
	unsigned int hash = hash_32((unsigned int)page, 16);
	unsigned int  slot = hash % OBJECT_TABLE_SIZE;
	unsigned long flags;

	PPageHashEntry entry = (PPageHashEntry)allocate_record(NODE_PAGE_RECORD);
	if (!entry) 
	{
		//printk("        [get_record][KERNEL_PAGE_ALLOC_BACKTRACE]can't get enough memory to create page entry\n");
		return NULL;
	}

	// initialize page entry
	entry->page = page;
	entry->size = 1 << order;
	entry->allocate_map_entry = map_entry;
	entry->bt_entry = bt_entry;
	entry->flag = (2 | flag);
	entry->prev = NULL;
	spin_lock_irqsave(&page_record_lock,flags);

	// insert the entry to the head of slot list
	if(gPageHashTable.page_hash_table[slot] == NULL) 
	{
		entry->next = NULL;
	} 
	else 
	{
		(gPageHashTable.page_hash_table[slot])->prev = entry;
		entry->next = gPageHashTable.page_hash_table[slot];
	}
	gPageHashTable.page_hash_table[slot] = entry;
	gPageHashTable.count++;
	page_record_total++;
	if(page_record_total > page_record_max)
	{
		page_record_max = page_record_total;
	}
	page_record_count++;
	if(page_record_count >= 1000)
        {
                page_recorder_memory_usage = page_record_total*sizeof(PageHashEntry) + bt_record_total*(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)));
                //printk("[TOTAL PAGE RECORD !!!] page record size is %d max page record size is %d\n",page_record_total*sizeof(PageHashEntry),page_record_max*sizeof(PageHashEntry));
                //printk("[TOTAL BACKTRACE RECORD !!!] bt record size is %d max bt record size is %d\n",bt_record_total*(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int))),bt_record_max*(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int))));
                page_record_count = 0;
        }

	spin_unlock_irqrestore(&page_record_lock,flags);
	return entry;
}

int remove_page_info(void *page,unsigned int order )
{
	unsigned int hash = hash_32((unsigned int)page, 16);
	unsigned int slot = hash % OBJECT_TABLE_SIZE;
	PageObjectEntry *bt_entry = NULL;
	PageHashEntry* entry = NULL;
	unsigned long flags;

	//search page record in hash table
	if(page_record_lock_init == 0)
	{
		page_record_lock_init =1;
		spin_lock_init(&page_record_lock);
	}
	if(bt_record_lock_init == 0)
	{
		bt_record_lock_init =1;
		spin_lock_init(&bt_record_lock);
	}

	spin_lock_irqsave(&page_record_lock,flags);	
	entry = find_page_entry(page, slot);
	if (entry == NULL) 
	{
		spin_unlock_irqrestore(&page_record_lock,flags);
		//printk("[remove_page_info]can't find page info 0x%x\n",page);
		if(debug_log)
		{
			get_kernel_backtrace(NULL,1);
		}
		return 1; 
	} 
	else 
	{
		//remove page record from hash table
		if (entry->prev == NULL) 
		{ //head
			gPageHashTable.page_hash_table[slot] = entry->next;
			if(gPageHashTable.page_hash_table[slot] != NULL) // not only one entry in the slot
				gPageHashTable.page_hash_table[slot]->prev = NULL;
		} 
		else if(entry->next == NULL) 
		{ // tail
			entry->prev->next = NULL;
		} 
		else 
		{ // middle
			entry->next->prev = entry->prev;
			entry->prev->next = entry->next;
		}

		gPageHashTable.count--;
		page_record_total--;
		spin_unlock_irqrestore(&page_record_lock,flags);

		// clean page entry
		entry->next = NULL;
		entry->prev = NULL;
		bt_entry = entry->bt_entry;
		kmem_cache_free(page_cachep,entry);

		// create alloc bt entry for historical allocation
		if(bt_entry == NULL) 
		{
			return -1;
		}
		else
		{
			spin_lock_irqsave(&bt_record_lock,flags);
			if(bt_entry->reference > 1)
			{
				(bt_entry->reference)--;
				bt_entry->size = bt_entry->size - (1 << order);
				spin_unlock_irqrestore(&bt_record_lock,flags);
				//printk("[remove_page_info] bt_entry->size %d\n",bt_entry->size);
			}
			else if(bt_entry->reference == 1)
			{
				unsigned int hash_bt;
				unsigned int slot_bt;
				hash_bt = get_hash(bt_entry->object,bt_entry->numEntries);
				slot_bt = hash_bt % OBJECT_TABLE_SIZE;

				if (bt_entry->prev == NULL) 
				{ //head
					gKernelPageBtTable.slots[slot_bt] = bt_entry->next;
					if(gKernelPageBtTable.slots[slot_bt] != NULL) // not only one entry in the slot
						gKernelPageBtTable.slots[slot_bt]->prev = NULL;
 				}
				else if(bt_entry->next == NULL) 
				{ // tail
					bt_entry->prev->next = NULL;
				} 
				else 
				{ // middle
					bt_entry->next->prev = bt_entry->prev;
					bt_entry->prev->next = bt_entry->next;
				}
				spin_unlock_irqrestore(&bt_record_lock,flags);
				bt_record_total--;
				kfree(bt_entry);
			}
			else
			{
				spin_unlock_irqrestore(&bt_record_lock,flags);
				printk("ERROR !!!!free page info\n");
			}
		}
	}
	return 0;
}
int record_page_record(void *page, unsigned int order)
{
	void *entry,*map_entry = NULL;
	page_record_t record_param;
	if(!page_recorder_debug)
	{
		return 0;
	}
	if(page_record_lock_init == 0)
	{
		page_record_lock_init =1;
		spin_lock_init(&page_record_lock);
	}
	if(bt_record_lock_init == 0)
	{
		bt_record_lock_init =1;
		spin_lock_init(&bt_record_lock);
	}
	if(debug_log & 1)
	{
		//get_kernel_backtrace(NULL,1);
	}
	record_param.page = page;
	record_param.size = order;
	record_param.backtrace_num = (unsigned int)get_kernel_backtrace((unsigned long *)record_param.backtrace,(unsigned int)0);

	entry = get_record(HASH_PAGE_NODE_KERNEL_PAGE_ALLOC_BACKTRACE,&record_param);
	if (entry == NULL)
	{
		//printk("        [get_record][KERNEL_PAGE_ALLOC_BACKTRACE]can't get enough memory to create backtrace object\n");
		return 0;
	}
	record_page_info((PageObjectEntry*)entry,(PageObjectEntry*)map_entry, record_param.page, record_param.size, 0);
	return 1;
}
EXPORT_SYMBOL(record_page_record);

int remove_page_record(void *page, unsigned int order)
{
	page_record_t record_param;
	record_param.page = page;
	record_param.size = order;

        if(!page_recorder_debug)
        {
                return 0;
        }

	//record_param.backtrace_num = get_kernel_backtrace((unsigned long *)record_param.backtrace,(unsigned int)0);
	//get_kernel_symbol((unsigned long *)record_param.backtrace,record_param.backtrace_num,&(record_param.kernel_symbol[0]));
	if(debug_log & 2)
	{
		//get_kernel_backtrace(NULL,1);
	}

	remove_page_info(record_param.page,record_param.size);
	return 1;
}
EXPORT_SYMBOL(remove_page_record);

static int page_recorder_debug_show(struct seq_file *s, void *unused)
{
	unsigned int index = 0;
	unsigned int *backtrace ;
	unsigned int rank_index = 0;
	char symbol[KSYM_SYMBOL_LEN];
	unsigned int i = 0;
	struct page_object_rank_entry *rank_head = NULL;
	struct page_object_rank_entry *rank_tail = NULL;
	unsigned int Object_rank_count = 0;
	PageObjectEntry *tmp = NULL;
	unsigned long flags;

	seq_printf(s, "page_recorder_debug: [%d]\n", page_recorder_debug);
	seq_printf(s, "page_recorder_limit: [%d]\n", page_recorder_limit);
	seq_printf(s, "TOP %d page allocation \n", Object_rank_max);
	for(index = 0 ;index < OBJECT_TABLE_SIZE;index++)
	{
		tmp = NULL;
		spin_lock_irqsave(&bt_record_lock,flags);
		tmp = gKernelPageBtTable.slots[index];
		while(tmp != NULL)
		{
			struct page_object_rank_entry *rank_tmp = rank_head;
			struct page_object_rank_entry *rank_tmp_prev = rank_head;
			for(rank_index = 0;rank_index < Object_rank_max;rank_index++)
			{
				struct page_object_rank_entry *new_rank_entry = NULL;
				PageObjectEntry *entry = NULL;
				if((rank_tmp!= NULL)&&(rank_tmp->entry->size <= tmp->size))
				{
					//insert current record into list
					PageObjectEntry *entry = NULL;
					new_rank_entry = (struct page_object_rank_entry *)kmalloc(sizeof(struct page_object_rank_entry),GFP_ATOMIC);	
					if(new_rank_entry == NULL)
					{
						spin_unlock_irqrestore(&bt_record_lock,flags);
						printk( "[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
						return NULL;
					}
					entry = kmalloc(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)),GFP_ATOMIC);
					if(entry == NULL)
					{
						spin_unlock_irqrestore(&bt_record_lock,flags);
						printk("[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
						return NULL;	
					}
					memcpy(entry,tmp, sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)));
					new_rank_entry->entry = entry;
					new_rank_entry->prev = rank_tmp->prev;
					if(rank_tmp->prev != NULL)
					{
						rank_tmp->prev->next = new_rank_entry;		
					}
					rank_tmp->prev = new_rank_entry;
					new_rank_entry->next = rank_tmp;
					if(new_rank_entry->prev == NULL)
					{
						rank_head = new_rank_entry;	
					}	
					if(Object_rank_count < (Object_rank_max))
					{
						Object_rank_count++;	
					}
					else
					{
						//free last rank_entry
						if(rank_tail != NULL)
						{
							struct page_object_rank_entry *new_tail = NULL;
							new_tail = rank_tail->prev;
							rank_tail->prev->next = NULL;
							kfree(rank_tail->entry);
							kfree(rank_tail);
							rank_tail = new_tail;	
						}
						else
						{
							printk("ERROR!!! rank_tail is NULL\n");
						}
					}
					break;
				}
				else if((rank_tmp == NULL)&& (Object_rank_count < Object_rank_max))
				{
					//if rank entry is less than object_entry_max, create new rank entry and insert it in rank list
					new_rank_entry = (struct page_object_rank_entry *)kmalloc(sizeof(struct page_object_rank_entry),GFP_ATOMIC);
					if(new_rank_entry == NULL)
					{
						spin_unlock_irqrestore(&bt_record_lock,flags);
						printk( "[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
						return NULL;
					}
					entry = kmalloc(sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)),GFP_ATOMIC);
					if(entry == NULL)
					{
						spin_unlock_irqrestore(&bt_record_lock,flags);
						printk("[PAGE_RECORDER]Error!!! can't get memory from kmalloc\n");
						return NULL;
					}
					memcpy(entry,tmp, sizeof(PageObjectEntry)+(20 * sizeof(unsigned int)));
					new_rank_entry->entry = entry;
					new_rank_entry->next = NULL;
					new_rank_entry->prev = rank_tmp_prev;
					if(rank_tmp_prev != NULL)
					{
						rank_tmp_prev->next = new_rank_entry;
					}
					if(new_rank_entry->prev == NULL)
					{
						rank_head = new_rank_entry;
					}
					rank_tail = new_rank_entry;
					Object_rank_count++;
					break;		
				}
				rank_tmp_prev = rank_tmp;
				rank_tmp = rank_tmp->next; 	
			}
			tmp = tmp->next;
		}
		spin_unlock_irqrestore(&bt_record_lock,flags);
	}
	
	//print top object_rank_max record	
	{
		struct page_object_rank_entry *rank_tmp = rank_head;
		struct page_object_rank_entry *tmp_record = NULL;
		rank_index = 0;
		while(rank_tmp != NULL)
		{	
			backtrace = (unsigned int *)rank_tmp->entry->object;
			seq_printf(s, "[%d]%s %d %s\n",rank_index,"Backtrace pages ",rank_tmp->entry->size*4096,"bytes");
			for(i = 0;i < rank_tmp->entry->numEntries;i++)
			{
				sprint_symbol(symbol,*(backtrace+i));
				seq_printf(s, "  KERNEL[%d] 0x%x :: symbol %s\n", i,backtrace[i],symbol);
			}
			rank_index++;
			tmp_record = rank_tmp;
			rank_tmp = rank_tmp->next;
			kfree(tmp_record->entry);
			kfree(tmp_record);
		}
	}
	return 0;
}
static int __init setup_page_recorder_debug(char *str)
{
        page_recorder_debug = DEBUG_DEFAULT_FLAGS;
        if (*str++ != '=' || !*str)
                /*
                 * No options specified. Switch on full debugging.
                 */
                goto out;

        if (*str == ',')
                /*
                 * No options but restriction on page recorder. This means full
                 * debugging for page recorder matching a pattern.
                 */
                goto check_page_recorder;

        page_recorder_debug = 0;
        if (*str == '-')
                /*
                 * Switch off all debugging measures.
                 */
                goto out;

check_page_recorder:
        if (*str == ',')
                page_recorder_debug_function = *(str + 1);
out:
        return 1;
}
__setup("page_recorder_debug", setup_page_recorder_debug);

static int __init page_recorder_init(void)
{
	/* Create page allocate */
	debug_root = debugfs_create_dir("page_recorder", NULL);
	debugfs_create_file("Usage_rank", 0444,debug_root, NULL,&debug_page_recorder_fops);
	debugfs_create_u32("Rank_number", 0644,debug_root, &Object_rank_max);
	debugfs_create_file("query_page", 0644,debug_root, NULL, &query_page_ios_fops);
	debugfs_create_u32("page_virtual_address", 0644,debug_root, &queried_address);
	debugfs_create_u32("debug_log", 0644,debug_root, &debug_log);
	debugfs_create_u32("page_recorder_debug", 0644,debug_root, &page_recorder_debug);
	debugfs_create_u32("page_recorder_memory_usage", 0644,debug_root, &page_recorder_memory_usage);
	debugfs_create_u32("page_recorder_limit", 0644,debug_root, &page_recorder_limit);
	return 0;
}
late_initcall(page_recorder_init);
