#include <asm/page.h>
#include <linux/module.h> 

// return the size of memory from start pfn to max pfn,
// _NOTE_
// the memory area may be discontinuous
extern unsigned long max_pfn;
unsigned int get_memory_size (void)
{
    return (unsigned int)(max_pfn << PAGE_SHIFT);
}
EXPORT_SYMBOL(get_memory_size);

// return the actual physical DRAM size
// wrapper function of mtk_get_max_DRAM_size
extern unsigned int mtk_get_max_DRAM_size(void);
unsigned int get_max_DRAM_size(void)
{
        return mtk_get_max_DRAM_size();
}
EXPORT_SYMBOL(get_max_DRAM_size);
