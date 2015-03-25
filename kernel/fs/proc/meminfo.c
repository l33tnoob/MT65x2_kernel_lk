#include <linux/fs.h>
#include <linux/hugetlb.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include "internal.h"
/// M: LCA @{
#include <linux/export.h>
/// @}

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

/// M: LCA @{
int extra_log_meminfo_lite(void)
{
#ifdef CONFIG_ZRAM
	struct sysinfo i;
	unsigned long committed;
	struct vmalloc_info vmi;
	long cached;
	unsigned long pages[NR_LRU_LISTS];
	int lru;

/*
 * display in kilobytes.
 */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	si_meminfo(&i);
	si_swapinfo(&i);
	committed = percpu_counter_read_positive(&vm_committed_as);
	cached = global_page_state(NR_FILE_PAGES) -
			total_swapcache_pages - i.bufferram;
	if (cached < 0)
		cached = 0;

	get_vmalloc_info(&vmi);

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_page_state(NR_LRU_BASE + lru);

	/*
	 * Tagged format, for easy grepping and expansion.
	 */
	printk(KERN_INFO "[MTK_MSR]meminfo"\
		"(%lu)"			// (MemFree)
		"(%lu"			// (Buffers, Cache, SwapCached)
		",%lu"
		",%lu)"
		"(%lu"			// (Active, Inactive, Unevictable, MLocked)
		",%lu"
		",%lu"
		",%lu"
		",%lu"
		",%lu"
		",%lu"
		",%lu)"
		"(%lu"			// (Swap total, Swap Free)
		",%lu)"
		"(%lu"			// (Anon, Mapped)
		",%lu)"
		"(%lu"			// (Slab KStack, PageTable)
		",%lu"
		",%lu)\n"
		,
		K(i.freeram),
		K(i.bufferram),
		K(cached),
		K(total_swapcache_pages),
		K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_ACTIVE_ANON]),
		K(pages[LRU_INACTIVE_ANON]),
		K(pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_UNEVICTABLE]),
		K(global_page_state(NR_MLOCK)),
		K(i.totalswap),
		K(i.freeswap),
		K(global_page_state(NR_ANON_PAGES)),
		K(global_page_state(NR_FILE_MAPPED)),
		K(global_page_state(NR_SLAB_RECLAIMABLE) + global_page_state(NR_SLAB_UNRECLAIMABLE)),
		global_page_state(NR_KERNEL_STACK) * THREAD_SIZE >> 10,
		K(global_page_state(NR_PAGETABLE))
		);
#undef K
#endif	/* CONFIG_ZRAM */
	return 0;
}
EXPORT_SYMBOL(extra_log_meminfo_lite);
/// @}

static int meminfo_proc_show(struct seq_file *m, void *v)
{
	struct sysinfo i;
	unsigned long committed;
	unsigned long allowed;
	struct vmalloc_info vmi;
	long cached;
	unsigned long pages[NR_LRU_LISTS];
	int lru;

/*
 * display in kilobytes.
 */
#define K(x) ((x) << (PAGE_SHIFT - 10))
	si_meminfo(&i);
	si_swapinfo(&i);
	committed = percpu_counter_read_positive(&vm_committed_as);
	allowed = ((totalram_pages - hugetlb_total_pages())
		* sysctl_overcommit_ratio / 100) + total_swap_pages;

	cached = global_page_state(NR_FILE_PAGES) -
			total_swapcache_pages - i.bufferram;
	if (cached < 0)
		cached = 0;

	get_vmalloc_info(&vmi);

	for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
		pages[lru] = global_page_state(NR_LRU_BASE + lru);

	/*
	 * Tagged format, for easy grepping and expansion.
	 */
	seq_printf(m,
		"MemTotal:       %8lu kB\n"
		"MemFree:        %8lu kB\n"
		"Buffers:        %8lu kB\n"
		"Cached:         %8lu kB\n"
		"SwapCached:     %8lu kB\n"
		"Active:         %8lu kB\n"
		"Inactive:       %8lu kB\n"
		"Active(anon):   %8lu kB\n"
		"Inactive(anon): %8lu kB\n"
		"Active(file):   %8lu kB\n"
		"Inactive(file): %8lu kB\n"
		"Unevictable:    %8lu kB\n"
		"Mlocked:        %8lu kB\n"
#ifdef CONFIG_HIGHMEM
		"HighTotal:      %8lu kB\n"
		"HighFree:       %8lu kB\n"
		"LowTotal:       %8lu kB\n"
		"LowFree:        %8lu kB\n"
#endif
#ifndef CONFIG_MMU
		"MmapCopy:       %8lu kB\n"
#endif
		"SwapTotal:      %8lu kB\n"
		"SwapFree:       %8lu kB\n"
		"Dirty:          %8lu kB\n"
		"Writeback:      %8lu kB\n"
		"AnonPages:      %8lu kB\n"
		"Mapped:         %8lu kB\n"
		"Shmem:          %8lu kB\n"
		"Slab:           %8lu kB\n"
		"SReclaimable:   %8lu kB\n"
		"SUnreclaim:     %8lu kB\n"
		"KernelStack:    %8lu kB\n"
		"PageTables:     %8lu kB\n"
#ifdef CONFIG_QUICKLIST
		"Quicklists:     %8lu kB\n"
#endif
		"NFS_Unstable:   %8lu kB\n"
		"Bounce:         %8lu kB\n"
		"WritebackTmp:   %8lu kB\n"
		"CommitLimit:    %8lu kB\n"
		"Committed_AS:   %8lu kB\n"
		"VmallocTotal:   %8lu kB\n"
		"VmallocUsed:    %8lu kB\n"
		"VmallocChunk:   %8lu kB\n"
#ifdef CONFIG_MEMORY_FAILURE
		"HardwareCorrupted: %5lu kB\n"
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		"AnonHugePages:  %8lu kB\n"
#endif
		,
		K(i.totalram),
		K(i.freeram),
		K(i.bufferram),
		K(cached),
		K(total_swapcache_pages),
		K(pages[LRU_ACTIVE_ANON]   + pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_ACTIVE_ANON]),
		K(pages[LRU_INACTIVE_ANON]),
		K(pages[LRU_ACTIVE_FILE]),
		K(pages[LRU_INACTIVE_FILE]),
		K(pages[LRU_UNEVICTABLE]),
		K(global_page_state(NR_MLOCK)),
#ifdef CONFIG_HIGHMEM
		K(i.totalhigh),
		K(i.freehigh),
		K(i.totalram-i.totalhigh),
		K(i.freeram-i.freehigh),
#endif
#ifndef CONFIG_MMU
		K((unsigned long) atomic_long_read(&mmap_pages_allocated)),
#endif
		K(i.totalswap),
		K(i.freeswap),
		K(global_page_state(NR_FILE_DIRTY)),
		K(global_page_state(NR_WRITEBACK)),
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		K(global_page_state(NR_ANON_PAGES)
		  + global_page_state(NR_ANON_TRANSPARENT_HUGEPAGES) *
		  HPAGE_PMD_NR),
#else
		K(global_page_state(NR_ANON_PAGES)),
#endif
		K(global_page_state(NR_FILE_MAPPED)),
		K(global_page_state(NR_SHMEM)),
		K(global_page_state(NR_SLAB_RECLAIMABLE) +
				global_page_state(NR_SLAB_UNRECLAIMABLE)),
		K(global_page_state(NR_SLAB_RECLAIMABLE)),
		K(global_page_state(NR_SLAB_UNRECLAIMABLE)),
		global_page_state(NR_KERNEL_STACK) * THREAD_SIZE / 1024,
		K(global_page_state(NR_PAGETABLE)),
#ifdef CONFIG_QUICKLIST
		K(quicklist_total_size()),
#endif
		K(global_page_state(NR_UNSTABLE_NFS)),
		K(global_page_state(NR_BOUNCE)),
		K(global_page_state(NR_WRITEBACK_TEMP)),
		K(allowed),
		K(committed),
		(unsigned long)VMALLOC_TOTAL >> 10,
		vmi.used >> 10,
		vmi.largest_chunk >> 10
#ifdef CONFIG_MEMORY_FAILURE
		,atomic_long_read(&mce_bad_pages) << (PAGE_SHIFT - 10)
#endif
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
		,K(global_page_state(NR_ANON_TRANSPARENT_HUGEPAGES) *
		   HPAGE_PMD_NR)
#endif
		);

	hugetlb_report_meminfo(m);

	arch_report_meminfo(m);

	return 0;
#undef K
}

static int meminfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_proc_show, NULL);
}

static const struct file_operations meminfo_proc_fops = {
	.open		= meminfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
////////////////////////////////////////////////////
//ifdef CONFIG_MT_ENG_BUILD true == under Eng.Build
//Otherwise, it should be under User-Build. (So, we would disable Kernel Memory Status Report Mechanism.)
//
//////////////////
#ifdef CONFIG_MT_ENG_BUILD
#include <asm/kmap_types.h>
#include <asm/uaccess.h>

//Kmem Log Buffer Depth
#define Log_Buffer_Depth 120 //would keep Log_Buffer_Depth lines in Buffer
#define Log_Max_Length 1024 //Per-Log would occupied Log_Max_Length bytes
static char gKMemLogBuffer[Log_Buffer_Depth][Log_Max_Length];
static int gKMemLogBufferIndex=0;
static long gKMemLogBufferCounter=0;
//Kmem Status Basic Parameters
static unsigned long  kmem_status_timer_interval=1;
static unsigned long kmem_status_timer_interval_HZ=1;
//for interval
static unsigned long kmem_status_page_reclaim_counter=0;
static unsigned long kmem_status_lmk_counter=0;
static unsigned long kmem_status_oom_counter=0;
static unsigned long kmem_status_filemap_fault_counter=0;
static unsigned long kmem_status_writeback_counter=0;
//for total counter (from kernel boot-up to now.)
static unsigned long kmem_status_page_reclaim_total=0;
static unsigned long kmem_status_lmk_total=0;
static unsigned long kmem_status_oom_total=0;
static unsigned long kmem_status_filemap_fault_total=0;
static unsigned long kmem_status_writeback_total=0;
//
struct timer_list kmem_status_timer;
static int gkmem_status_timer=0;
static void kmem_status_callback(unsigned long);
struct kmem_status_data {
    int count;
};
static struct kmem_status_data gkmem_status_data;
///////////
#define MAX_USER_INPUT 256
static char kmem_user_input[MAX_USER_INPUT]={0x00};
#define MAX_STATUS_MSG_SIZE 4096
static char kmem_status_msg[MAX_STATUS_MSG_SIZE]={0x00};
/////
#define MAX_READ_OUT_MSG_SIZE (MAX_STATUS_MSG_SIZE + Log_Max_Length*Log_Buffer_Depth)
static char READ_OUT_BUFFER[MAX_READ_OUT_MSG_SIZE];
static int gREAD_OUT_BUFFER_LEN=0;
//
static int kmem_user_input_len=0;
static int kmem_status_msg_len=0;
//embedded in __alloc_pages_direct_reclaim of mm/page_alloc.c
void add_kmem_status_page_reclaim_counter(void)
{kmem_status_page_reclaim_counter++;}
//embedded in lowmem_shrink of drivers/staging/android/lowmemorykiller.c
void add_kmem_status_lmk_counter(void)
{kmem_status_lmk_counter++;}
//embedded in out_of_memory of mm/oom_kill.c
void add_kmem_status_oom_counter(void)
{kmem_status_oom_counter++;}
//embedded in filemap_fault of mm/filemap.c
void add_kmem_status_filemap_fault_counter(void)
{kmem_status_filemap_fault_counter++;}
//embedded in balance_dirty_pages of mm/page-writeback.c
void add_kmem_status_writeback_counter(void)
{kmem_status_writeback_counter++;}
//
static void kmem_status_prepare_msg(void)
{
	//add to total
	kmem_status_page_reclaim_total+=kmem_status_page_reclaim_counter;
	kmem_status_lmk_total+=kmem_status_lmk_counter;
	kmem_status_oom_total+=kmem_status_oom_counter;
	kmem_status_filemap_fault_total+=kmem_status_filemap_fault_counter;
	kmem_status_writeback_total+=kmem_status_writeback_counter;
	//prepare the user space print-out log.
	snprintf(kmem_status_msg,MAX_STATUS_MSG_SIZE,
	"Kernel Memory Status Report (/proc/kmem_interval)\nTimer Interval:%ld ms (%ld HZ)\nPage Reclaim Counter:%ld(%ld)\nLMK Counter:%ld(%ld)\nOOM Counter:%ld(%ld)\nFileMap Fault Counter:%ld(%ld)\nWriteBack Counter:%ld(%ld)\n==============================\n",kmem_status_timer_interval,kmem_status_timer_interval_HZ,kmem_status_page_reclaim_counter,kmem_status_page_reclaim_total,kmem_status_lmk_counter,kmem_status_lmk_total,kmem_status_oom_counter,kmem_status_oom_total,kmem_status_filemap_fault_counter,kmem_status_filemap_fault_total,kmem_status_writeback_counter,kmem_status_writeback_total);
	kmem_status_msg_len=strlen(kmem_status_msg);
	//printk in console log. by default.
	//Chnage prink to internal buffer by Julius' request.
	snprintf(gKMemLogBuffer[gKMemLogBufferIndex],Log_Max_Length-2,"(%ld) : Jiffies:%lu,PRC:%ld(%ld),LMKC:%ld(%ld),OOMC:%ld(%ld),FMFC:%ld(%ld),WBC:%ld(%ld)\n",gKMemLogBufferCounter,jiffies,kmem_status_page_reclaim_counter,kmem_status_page_reclaim_total,kmem_status_lmk_counter,kmem_status_lmk_total,kmem_status_oom_counter,kmem_status_oom_total,kmem_status_filemap_fault_counter,kmem_status_filemap_fault_total,kmem_status_writeback_counter,kmem_status_writeback_total);
	gKMemLogBufferIndex++;
	if(gKMemLogBufferIndex>=Log_Buffer_Depth)
		gKMemLogBufferIndex=0;
	//
	gKMemLogBufferCounter++;
	if(gKMemLogBufferCounter > 0x7fff0000)
		gKMemLogBufferCounter=0;
	//reset interval to zero.
	kmem_status_page_reclaim_counter=0;
	kmem_status_lmk_counter=0;
	kmem_status_oom_counter=0;
	kmem_status_filemap_fault_counter=0;
	kmem_status_writeback_counter=0;
}
static void prepare_read_out_buffer(void)
{
	int i,index;
	memset(READ_OUT_BUFFER,0x00,MAX_READ_OUT_MSG_SIZE);
	gREAD_OUT_BUFFER_LEN=0;
	strcat(READ_OUT_BUFFER,kmem_status_msg);
	index=gKMemLogBufferIndex;
	if(index>=(Log_Buffer_Depth-1))
		index=0;
	else
		index++;

	for(i=0;i<(Log_Buffer_Depth-1);i++)
	{
		if(gKMemLogBuffer[index][0]==0)
		{
			//break;
		}
		else
		{
			strcat(READ_OUT_BUFFER,gKMemLogBuffer[index]);
		}
		if(index>=(Log_Buffer_Depth-1))
			index=0;
		else
			index++;
	}
	////
	//
	gREAD_OUT_BUFFER_LEN=strlen(READ_OUT_BUFFER);
}
//
static void kmem_status_callback(unsigned long data)
{
    //printk("kmem_status_callback  %s(): %d\n", __FUNCTION__, dp->count);
    kmem_status_prepare_msg();
    //
    if(kmem_status_timer_interval_HZ>0)
    	mod_timer(&kmem_status_timer, jiffies + kmem_status_timer_interval_HZ);//kmem_status_timer_interval * HZ);
}
static void kmem_status_init_timer(void)
{
    if(gkmem_status_timer!=0)
    {
        del_timer(&kmem_status_timer);
	//return;
    }
    if(kmem_status_timer_interval_HZ<=0)
	return;
    gkmem_status_timer=1;
    init_timer(&kmem_status_timer);
    //
    kmem_status_timer.expires = jiffies + kmem_status_timer_interval_HZ;//kmem_status_timer_interval * HZ;
    kmem_status_timer.function = &kmem_status_callback;
    kmem_status_timer.data = (unsigned long) &gkmem_status_data;
    add_timer(&kmem_status_timer);
}
static void kmem_status_fini_timer(void)
{
    if(gkmem_status_timer==0)
	return;
    gkmem_status_timer=0;
    del_timer(&kmem_status_timer); 
}
static int kmem_status_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m,
                "kmem status\n");
	return 0;
}

static int kmem_status_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file,kmem_status_proc_show, NULL);
}
//
static ssize_t kmem_status_proc_read(struct file *filp, char __user *buf,
                                size_t count, loff_t *ppos)
{
	int vSize;
	int offset=*ppos + filp->f_pos;

	if(offset==0)
		prepare_read_out_buffer();
	//printk("kmem_status_proc_read:filp->f_pos:%ld_%xh offset:%ld",filp->f_pos,filp->f_pos,offset);
	if(offset >=gREAD_OUT_BUFFER_LEN)//kmem_user_input_len)
	{
		printk("kmem_status_proc_read offset:%d_gREAD_OUT_BUFFER_LEN:%d",offset,gREAD_OUT_BUFFER_LEN);
 		return 0;
	}
	//
	if(count > gREAD_OUT_BUFFER_LEN-offset) 
                vSize=gREAD_OUT_BUFFER_LEN-offset;
        else
                vSize=count;
	if(copy_to_user(buf,READ_OUT_BUFFER+offset,vSize))
	{
		printk("kmem_status_proc_read error#1\n");
		return 0;
	}
	//
	*ppos+=vSize;// offset;
	return vSize;
}
static void prepare_init_msg(void)
{
	snprintf(kmem_status_msg,MAX_STATUS_MSG_SIZE,"Kernel memory status report daemon is disable now. \nPlease echo interval (in ms) to /proc/kmem_interval to enable it.\n eq. echo \"1000\" > /proc/kmem_interval \n\n");
        kmem_status_msg_len=strlen(kmem_status_msg);
}
static ssize_t kmem_status_proc_write(struct file *filp, const char __user *buf,
                                size_t count, loff_t *ppos)
{
	int interval;
	int vSize;
	int offset=*ppos;
		
	if(count > MAX_USER_INPUT-1)
		vSize=MAX_USER_INPUT-1;
	else
		vSize=count;
	memset(kmem_user_input,0,MAX_USER_INPUT);
	memcpy(kmem_user_input,buf,vSize);
	kmem_user_input_len=vSize;
	offset=vSize;
	*ppos=offset;
	///
	interval =simple_strtol(kmem_user_input, NULL, 0);//atoi(kmem_user_input);
	if(interval > 0)
	{
		//printk("kmem interval:%ld init timer",interval);
		kmem_status_init_timer();
	}
	else
	{
		interval=0;
		//printk("kmem interval:%ld fini timer",interval);
		kmem_status_fini_timer();
		//
		prepare_init_msg();
	}
	kmem_status_timer_interval=interval;
	if(interval>0)
	{
		kmem_status_timer_interval_HZ = interval/10;
		if(kmem_status_timer_interval_HZ==0)
			kmem_status_timer_interval_HZ=1;
	}
	else
	{
		kmem_status_timer_interval_HZ=0;
	}
	return vSize; 
}
static const struct file_operations kmem_status_proc_fops = {
        .open           = kmem_status_proc_open, //single_open, //kmem_status_proc_open,
	.write          = kmem_status_proc_write,
        .read           = kmem_status_proc_read,
        .llseek         = seq_lseek, //kmem_status_proc_write,
        .release        = single_release,
};
static int __init proc_kmem_status(void)
{
	prepare_init_msg();
	memset(gKMemLogBuffer,0x00,Log_Buffer_Depth*Log_Max_Length);
	gKMemLogBufferIndex=0;
	gKMemLogBufferCounter=0;
	memset(READ_OUT_BUFFER,0x00,MAX_READ_OUT_MSG_SIZE);
	gREAD_OUT_BUFFER_LEN=0;

	proc_create("kmem_interval", 0, NULL, &kmem_status_proc_fops);
	return 0;
}
#endif
//////////////////////////////////////////////////////
static int __init proc_meminfo_init(void)
{
	proc_create("meminfo", 0, NULL, &meminfo_proc_fops);
#ifdef CONFIG_MT_ENG_BUILD
	proc_kmem_status();
#endif
	return 0;
}
module_init(proc_meminfo_init);
