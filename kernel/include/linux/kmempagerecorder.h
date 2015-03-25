#ifndef KMEMPAGERECORDER
#include <linux/types.h>
#define OBJECT_TABLE_SIZE      1543
#define BACKTRACE_SIZE 10
#define BACKTRACE_LEVEL 10
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

typedef enum
{
   NODE_PAGE_RECORD,
   NODE_PAGE_MAX
}PAGE_RECORD_TYPE;

typedef enum
{
   HASH_PAGE_NODE_KERNEL_BACKTRACE,
   HASH_PAGE_NODE_KERNEL_SYMBOL,
   HASH_PAGE_NODE_KERNEL_PAGE_ALLOC_BACKTRACE,
   HASH_PAGE_NODE_MAX
}PAGE_HASH_NODE_TYPE;

// for BT
typedef struct PageObjectEntry
{
    size_t slot;
    struct PageObjectEntry* prev;
    struct PageObjectEntry* next;
    size_t numEntries;
    size_t reference;
    unsigned int size;
    void *object[0];
}PageObjectEntry, *PPageObjectEntry;

typedef struct 
{
    size_t count;
    PageObjectEntry* slots[OBJECT_TABLE_SIZE];
}PageObjectTable,*PPageObjectTable;

typedef struct page_BT
{
    unsigned int numEntries;
    void * backtrace[0];
}page_BT, *page_PBT;

// for page
typedef struct PageHashEntry
{
        void* page;
        unsigned int size;
        struct PageHashEntry *prev;
        struct PageHashEntry *next;
	PPageObjectEntry allocate_map_entry;
        PPageObjectEntry bt_entry;
        page_PBT free_bt;
        unsigned int flag;
}PageHashEntry, *PPageHashEntry;

typedef struct {
        size_t count;
        size_t table_size;
        PageHashEntry *page_hash_table[OBJECT_TABLE_SIZE];
} PageHashTable;

struct page_mapping
{
    char *name;
    unsigned int address;
    unsigned int size;
};

typedef struct page_record_param
{
    unsigned int *page;
    unsigned int address_type;
    unsigned int address;
    unsigned int length;
    unsigned int backtrace[BACKTRACE_SIZE];
    unsigned int backtrace_num;
    unsigned int kernel_symbol[BACKTRACE_SIZE];
    struct page_mapping mapping_record[BACKTRACE_SIZE];
    unsigned int size;
}page_record_t;

//rank object
struct page_object_rank_entry
{
        PageObjectEntry *entry;
        struct page_object_rank_entry *prev;
        struct page_object_rank_entry *next;
};

void page_debug_show_backtrace(void);
int record_page_record(void *page, unsigned int order);
int remove_page_record(void *page, unsigned int order);
void disable_page_alloc_tracer(void);
#endif
#ifndef KMEMPAGERECORDER
#define KMEMPAGERECORDER
#endif

