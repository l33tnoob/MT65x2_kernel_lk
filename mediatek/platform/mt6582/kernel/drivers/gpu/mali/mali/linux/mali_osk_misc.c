/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2008-2013 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * @file mali_osk_misc.c
 * Implementation of the OS abstraction layer for the kernel device driver
 */
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/sched.h>
#include <linux/module.h>
#include "mali_osk.h"
#include "mt_reg_base.h"

extern void smi_dumpDebugMsg(void);
extern int m4u_dump_debug_registers(void);;

void _mali_osk_dbgmsg( const char *fmt, ... )
{
	va_list args;
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
}

u32 _mali_osk_snprintf( char *buf, u32 size, const char *fmt, ... )
{
	int res;
	va_list args;
	va_start(args, fmt);

	res = vscnprintf(buf, (size_t)size, fmt, args);

	va_end(args);
	return res;
}

#define CLK_CFG_0           (INFRA_BASE + 0x0040)
#define VENCPLL_CON0        (DDRPHY_BASE+0x800)
#define MMPLL_CON0          (APMIXEDSYS_BASE + 0x0230)

void _mali_osk_abort(void)
{
    int index;

	/* make a simple fault by dereferencing a NULL pointer */
	dump_stack();

    for (index = 0; index < 5; index++)
    {
        MALI_DEBUG_PRINT(2, ("=== [MALI] PLL Dump %d ===\n", index));       
        MALI_DEBUG_PRINT(2, ("CLK_CFG_0: 0x%08x\n", *((volatile unsigned int*)CLK_CFG_0)));
        MALI_DEBUG_PRINT(2, ("VENCPLL_CON0: 0x%08x\n", *((volatile unsigned int*)VENCPLL_CON0)));
        MALI_DEBUG_PRINT(2, ("MMPLL_CON0: 0x%08x\n", *((volatile unsigned int*)MMPLL_CON0)));

        MALI_DEBUG_PRINT(2, ("=== [MALI] SMI Dump %d ===\n", index));
        smi_dumpDebugMsg();

        MALI_DEBUG_PRINT(2, ("=== [MALI] M4U Dump %d ===\n", index));
        m4u_dump_debug_registers();
    }

	*(int *)0 = 0;
}

void _mali_osk_break(void)
{
	_mali_osk_abort();
}

u32 _mali_osk_get_pid(void)
{
	/* Thread group ID is the process ID on Linux */
	return (u32)current->tgid;
}

u32 _mali_osk_get_tid(void)
{
	/* pid is actually identifying the thread on Linux */
	u32 tid = current->pid;

	/* If the pid is 0 the core was idle.  Instead of returning 0 we return a special number
	 * identifying which core we are on. */
	if (0 == tid) {
		tid = -(1 + raw_smp_processor_id());
	}

	return tid;
}
