#include "mmu.h"
#include "dram_buffer.h"
#define MB (1024*1024)
#define DRAM_BASE 0x80000000
#define L2SRAM_BASE 0x00200000
#define INTERNALSRAM_BASE 0x00100000
#define tt g_page_table->l1_page_table

 void arm_mmu_map_section(ulong paddr, ulong vaddr, u32 flags)
{
	int index;

	/* Get the index into the translation table */
	index = vaddr / MB;

	/* Set the entry value:
	 * (2<<0): Section entry
	 * (0<<5): Domain = 0
	 *  flags: TEX, CB and AP bit settings provided by the caller.
	 */
	tt[index] = (paddr & ~(MB-1)) | (0<<5) | (2<<0) | flags;

	arm_invalidate_tlb();
}

void arm_mmu_init(void)
{
	int i;

	/* set some mmu specific control bits:
	 * access flag disabled, TEX remap disabled, mmu disabled
	 */
	arm_write_cr1(arm_read_cr1() & ~((1<<29)|(1<<28)|(1<<0)));

	/* set up an identity-mapped translation table with
	 * strongly ordered memory type and read/write access.
	 */
	for (i=0; i < 4096; i++) {
		arm_mmu_map_section(i * MB,
				    i * MB,
				    MMU_MEMORY_TYPE_STRONGLY_ORDERED |
				    MMU_MEMORY_AP_READ_WRITE);
	}
	
	/*Map Share SRAM into Normal*/
	arm_mmu_map_section(L2SRAM_BASE ,
		L2SRAM_BASE ,
		MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE|
		MMU_MEMORY_AP_READ_WRITE|
		EXECUTE);
	
	/*Map Internal SRAM into Normal*/
	
	arm_mmu_map_section(INTERNALSRAM_BASE ,
		INTERNALSRAM_BASE ,
		MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE |
		MMU_MEMORY_AP_READ_WRITE |
		EXECUTE);
	
	
	for( i = DRAM_BASE>>20 ; i< 4096; i++){
		arm_mmu_map_section(i * MB,
			i * MB,
			MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_ALLOCATE|
			MMU_MEMORY_AP_READ_WRITE);	
	}

	/* set up the translation table base */
	arm_write_ttbr(tt);

	/* set up the domain access register */
	//arm_write_dacr(0x00000001);

	/* turn on the mmu */
	arm_write_cr1(arm_read_cr1() | 0x1);
}

void arch_disable_mmu(void)
{
	arm_write_cr1(arm_read_cr1() & ~(1<<0));
}

void L1_DCACHE_ENABLE()
{

	apmcu_dsb();

	arm_cp15_dcache_invalidate();

	apmcu_enable_dcache();

	apmcu_isb();

}

void L1_DCACHE_DISABLE()
{

	/*1.DCACHE disable*/
	apmcu_disable_dcache();
	/*2.DCACHE clean_invalidate*/
	apmcu_dcache_clean_invalidate();
	/*3. DSB*/
	apmcu_dsb();
	/*4. ISB*/
	apmcu_isb();

}

void Cache_Enable(void)
{

        //-------------------------------------------------
        // L1 cache 
        //-------------------------------------------------
        L1_DCACHE_ENABLE();
        //-------------------------------------------------
        // MMU 
        //-------------------------------------------------
         arm_mmu_init();
}

 void Cache_Disable(void)
{
        //-------------------------------------------------
        // MMU 
        //-------------------------------------------------
         arch_disable_mmu();	

        //-------------------------------------------------
        // L1 cache 
        //-------------------------------------------------
        L1_DCACHE_DISABLE();
    	 
}