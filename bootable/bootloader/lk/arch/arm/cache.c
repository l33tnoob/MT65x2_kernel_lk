/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <kernel/thread.h> 
#include <platform/mt_typedefs.h>
#include <arch/arm/cache.h>
#include <platform/mt_reg_base.h>

volatile struct pl310_regs *PL310_L2CC = (struct pl310_regs *)0xC000E000;
void dsb(void);
static unsigned int way_mask;	

static void l2_cache_sync(void)
{
	enter_critical_section();
	PL310_L2CC->CacheSync = 0;
	exit_critical_section();
}

static void l2_inv_all(void)
{
	/* invalidate all ways */
	enter_critical_section();
	PL310_L2CC->InvalidateByWay = way_mask;
	while (PL310_L2CC->InvalidateByWay & way_mask);
	PL310_L2CC->CacheSync = 0;
	exit_critical_section();
}


void l2_clean_range(unsigned long start, unsigned long len)
{
	unsigned long end = start + len; 
	
	CACHE_ALLIGN(start);
	
	enter_critical_section();
	
	while (start < end) {
			PL310_L2CC->CleanLineByPA = start;
			start += CACHE_LINE_SIZE;
	}
	
	PL310_L2CC->CacheSync = 0;
	exit_critical_section();
}

void l2_flush_range(unsigned long start, unsigned long len)
{
	unsigned long end = start + len; 
	CACHE_ALLIGN(start);
	
	enter_critical_section();
	
	PL310_L2CC->DebugControl = 0x3;
	while (start < end) {		
#if MACH_TYPE == 6575
		PL310_L2CC->CleanLineByPA = start;
		PL310_L2CC->InvalidateLineByPA = start;
#else
		PL310_L2CC->FlushLineByPA = start;
#endif
		start += CACHE_LINE_SIZE;
	}
	PL310_L2CC->DebugControl = 0x0;
	
	PL310_L2CC->CacheSync = 0;
	exit_critical_section();
}

void l2_disable(void)
{

	// flush all 
	PL310_L2CC->DebugControl = 0x3;
	PL310_L2CC->FlushByWay = way_mask;
	while (PL310_L2CC->FlushByWay & way_mask);	
	PL310_L2CC->CacheSync = 0;
	PL310_L2CC->DebugControl = 0x0;
	
	//disable l2
	PL310_L2CC->Control = 0x0; 
	dsb();

}

unsigned int get_num_ways(void)
{
	unsigned int aux_val;
	unsigned int num_ways;
	
	aux_val = PL310_L2CC->AuxControl;

	if (aux_val & (1 << 16))
		num_ways = 16;
	else
		num_ways = 8;
	return num_ways;
}

#define Auxiliary_VALUE  0x70000000
void arch_enable_l2cache(void)
{
	PL310_L2CC->PowerControl = DYNAMIC_CLOCK_GATING_ENABLE;
	PL310_L2CC->PrefetchControl = (PL310_L2CC->PrefetchControl) | 0x40000000 ;
	PL310_L2CC->AuxControl = (PL310_L2CC->AuxControl & 0X8FFFFFFF) | Auxiliary_VALUE;
	
	way_mask = (1 << get_num_ways()) - 1;

	l2_inv_all();

	PL310_L2CC->Control = 0x1;
}

