/*
 *  linux/arch/arm/mm/context.c
 *
 *  Copyright (C) 2002-2003 Deep Blue Solutions Ltd, all rights reserved.
 *  Copyright (C) 2012 ARM Limited
 *
 *  Author: Will Deacon <will.deacon@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/percpu.h>

#include <asm/mmu_context.h>
/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
#include <asm/smp_plat.h>
#include <asm/tlbflush.h>

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
/*
 * On ARMv6, we have the following structure in the Context ID:
 *
 * 31                         7          0
 * +-------------------------+-----------+
 * |      process ID         |   ASID    |
 * +-------------------------+-----------+
 * |              context ID             |
 * +-------------------------------------+
 *
 * The ASID is used to tag entries in the CPU caches and TLBs.
 * The context ID is used by debuggers and trace logic, and
 * should be unique within all running processes.
 */
#define ASID_FIRST_VERSION	(1ULL << ASID_BITS)

static DEFINE_RAW_SPINLOCK(cpu_asid_lock);
/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
static u64 cpu_last_asid = ASID_FIRST_VERSION;

static DEFINE_PER_CPU(u64, active_asids);
static DEFINE_PER_CPU(u64, reserved_asids);
static cpumask_t tlb_flush_pending;

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
#ifdef CONFIG_ARM_LPAE
static void cpu_set_reserved_ttbr0(void)
{
	unsigned long ttbl = __pa(swapper_pg_dir);
	unsigned long ttbh = 0;

	/*
	 * Set TTBR0 to swapper_pg_dir which contains only global entries. The
	 * ASID is set to 0.
	 */
	asm volatile(
	"	mcrr	p15, 0, %0, %1, c2		@ set TTBR0\n"
	:
	: "r" (ttbl), "r" (ttbh));
	isb();
}
#else
static void cpu_set_reserved_ttbr0(void)
{
	u32 ttb;
	/* Copy TTBR1 into TTBR0 */
	asm volatile(
	"	mrc	p15, 0, %0, c2, c0, 1		@ read TTBR1\n"
	"	mcr	p15, 0, %0, c2, c0, 0		@ set TTBR0\n"
	: "=r" (ttb));
	isb();
}
#endif

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
static void flush_context(unsigned int cpu)
{
	int i;

	/* Update the list of reserved ASIDs. */
	per_cpu(active_asids, cpu) = 0;
	for_each_possible_cpu(i)
		per_cpu(reserved_asids, i) = per_cpu(active_asids, i);

	/* Queue a TLB invalidate and flush the I-cache if necessary. */
	if (!tlb_ops_need_broadcast())
		cpumask_set_cpu(cpu, &tlb_flush_pending);
	else
		cpumask_setall(&tlb_flush_pending);

	if (icache_is_vivt_asid_tagged())
		__flush_icache_all();
}

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
static int is_reserved_asid(u64 asid, u64 mask)
{
	int cpu;
	for_each_possible_cpu(cpu)
		if ((per_cpu(reserved_asids, cpu) & mask) == (asid & mask))
			return 1;
	return 0;
}

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
static void new_context(struct mm_struct *mm, unsigned int cpu)
{
	u64 asid = mm->context.id;

	if (asid != 0 && is_reserved_asid(asid, ULLONG_MAX)) {
		/*
		 * Our current ASID was active during a rollover, we can
		 * continue to use it and this was just a false alarm.
		 */
		asid = (cpu_last_asid & ASID_MASK) | (asid & ~ASID_MASK);
	} else {
		/*
		 * Allocate a free ASID. If we can't find one, take a
		 * note of the currently active ASIDs and mark the TLBs
		 * as requiring flushes.
		 */
		do {
			asid = ++cpu_last_asid;
			if ((asid & ~ASID_MASK) == 0)
				flush_context(cpu);
		} while (is_reserved_asid(asid, ~ASID_MASK));
		cpumask_clear(mm_cpumask(mm));
	}

	mm->context.id = asid;
}

/*
 * apply kernel patch: b5466f8728527a05a493cc4abe9e6f034a1bbaab
 */
void check_and_switch_context(struct mm_struct *mm, struct task_struct *tsk)
{
	unsigned long flags;
	unsigned int cpu = smp_processor_id();

	if (unlikely(mm->context.kvm_seq != init_mm.context.kvm_seq))
		__check_kvm_seq(mm);

	/*
	 * Required during context switch to avoid speculative page table
	 * walking with the wrong TTBR.
	 */
	cpu_set_reserved_ttbr0();

	raw_spin_lock_irqsave(&cpu_asid_lock, flags);
	/* Check that our ASID belongs to the current generation. */
	if ((mm->context.id ^ cpu_last_asid) >> ASID_BITS)
		new_context(mm, cpu);

	*this_cpu_ptr(&active_asids) = mm->context.id;
	cpumask_set_cpu(cpu, mm_cpumask(mm));

	if (cpumask_test_and_clear_cpu(cpu, &tlb_flush_pending))
		local_flush_tlb_all();
	raw_spin_unlock_irqrestore(&cpu_asid_lock, flags);

	cpu_switch_mm(mm->pgd, mm);
}
