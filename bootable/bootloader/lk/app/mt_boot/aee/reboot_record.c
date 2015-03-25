#include <string.h>
#include "aee.h"
#include "kdump.h"
#include <dev/mrdump.h>

static struct mrdump_control_block *mrdump_cb = NULL;

static void mrdump_query_bootinfo(void)
{
    if (mrdump_cb == NULL) {
        struct mrdump_control_block *bufp = (struct mrdump_control_block *)MRDUMP_CB_ADDR;
        voprintf_debug("Search boot record at %p\n", bufp);
        if (memcmp(bufp->sig, "MRDUMP01", 8) == 0) {
            voprintf_debug("Boot record found at %p\n", bufp);
            mrdump_cb = bufp;
            bufp->sig[0] = 'X';
        }
	else {
            voprintf_debug("No boot record found %x-%x\n", bufp->sig[0], bufp->sig[1]);
        }
    }
}

struct mrdump_control_block *aee_mrdump_get_params(void)
{
    mrdump_query_bootinfo();
    return mrdump_cb;
}

void mrdump_run(const struct mrdump_regset *per_cpu_regset, const struct mrdump_regpair *regpairs)
{
#if 0
    mrdump_query_bootinfo();
    reboot_mode = AEE_REBOOT_MODE_WDT;

    memset(&hw_reboot_crash_record, 0, sizeof(struct kdump_crash_record));
    strcpy(hw_reboot_crash_record.msg, "HW_REBOOT");

    int i;
    for (i = 0; i < NR_CPUS; i++) {
        hw_reboot_crash_record.cpu_regs[i][0] = (unsigned long)per_cpu_regset[i].r0;
        hw_reboot_crash_record.cpu_regs[i][1] = (unsigned long)per_cpu_regset[i].r1;
        hw_reboot_crash_record.cpu_regs[i][2] = (unsigned long)per_cpu_regset[i].r2;
        hw_reboot_crash_record.cpu_regs[i][3] = (unsigned long)per_cpu_regset[i].r3;

        hw_reboot_crash_record.cpu_regs[i][4] = (unsigned long)per_cpu_regset[i].r4;
        hw_reboot_crash_record.cpu_regs[i][5] = (unsigned long)per_cpu_regset[i].r5;
        hw_reboot_crash_record.cpu_regs[i][6] = (unsigned long)per_cpu_regset[i].r6;
        hw_reboot_crash_record.cpu_regs[i][7] = (unsigned long)per_cpu_regset[i].r7;

        hw_reboot_crash_record.cpu_regs[i][8] = (unsigned long)per_cpu_regset[i].r8;
        hw_reboot_crash_record.cpu_regs[i][9] = (unsigned long)per_cpu_regset[i].r9;
        hw_reboot_crash_record.cpu_regs[i][10] = (unsigned long)per_cpu_regset[i].r10;
        hw_reboot_crash_record.cpu_regs[i][11] = (unsigned long)per_cpu_regset[i].fp;
        
        hw_reboot_crash_record.cpu_regs[i][12] = (unsigned long)per_cpu_regset[i].r12;
        hw_reboot_crash_record.cpu_regs[i][13] = (unsigned long)per_cpu_regset[i].sp;
        hw_reboot_crash_record.cpu_regs[i][14] = (unsigned long)per_cpu_regset[i].lr;
        hw_reboot_crash_record.cpu_regs[i][15] = (unsigned long)per_cpu_regset[i].pc;

        hw_reboot_crash_record.cpu_regs[i][16] = (unsigned long)per_cpu_regset[i].cpsr;
    }

    aee_kdump_detection();
#endif
}

