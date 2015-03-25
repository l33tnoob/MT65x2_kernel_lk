#include <linux/mm.h>
#include <linux/aee.h>
#include <linux/elf.h>
#include <linux/kdebug.h>
#include <linux/mmc/sd_misc.h>
#include <linux/mrdump.h>

extern int msdc_init_panic(int dev);
extern int emmc_ipanic_write(void *buf, int off, int len);

/* 
 * ramdump mini 
 * implement ramdump mini prototype here 
 */

struct mrdump_mini_reg_desc {
	unsigned long reg;	/* register value */
	unsigned long kstart;	/* start kernel addr of memory dump */
	unsigned long kend;	/* end kernel addr of memory dump */
	int valid;		/* 1: valid regiser, 0: invalid regiser */
	loff_t offset;		/* offset in buffer */
};

/* it should always be smaller than MRDUMP_MINI_HEADER_SIZE */
struct mrdump_mini_header {
	struct mrdump_mini_reg_desc reg_desc[ELF_NGREG];
};
static char mrdump_mini_buf[MRDUMP_MINI_BUF_SIZE];

#ifdef MTK_EMMC_SUPPORT
static void __mrdump_mini_core(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, va_list ap)
{
	int i;
	unsigned long reg, start, end, size;
	loff_t offset = 0;
	struct mrdump_mini_header *hdr = (struct mrdump_mini_header *)mrdump_mini_buf;
	char *buf = mrdump_mini_buf + MRDUMP_MINI_HEADER_SIZE;

	if (!msdc_init_panic(DUMP_INTO_BOOT_CARD_IPANIC)) {
		/* ipanic init fail */
		return;
	}

#ifdef DUMMY
	/* 
	 * test code - write dummy data to ipanic partition 
	 */ 
	memset(mrdump_mini_buf, 0x3e, MRDUMP_MINI_BUF_SIZE);

	if (emmc_ipanic_write(mrdump_mini_buf, IPANIC_MRDUMP_OFFSET, MRDUMP_MINI_BUF_SIZE)) {
		printk(KERN_ALERT"card_dump_func_write failed (%d)\n", i);
	} else {
		printk(KERN_ALERT"card_dump_func_write ok, (%d)\n", i);
	}
#else
	memset(mrdump_mini_buf, 0x0, MRDUMP_MINI_BUF_SIZE);

	if (sizeof(struct mrdump_mini_header) > MRDUMP_MINI_HEADER_SIZE) {
		/* mrdump_mini_header is too large, write 0x0 headers to ipanic */
		printk(KERN_ALERT"mrdump_mini_header is too large(%d)\n", 
				sizeof(struct mrdump_mini_header));
		offset += MRDUMP_MINI_HEADER_SIZE;
		goto ipanic_write;
	}

	for(i = 0; i < ELF_NGREG; i++) {
		reg = regs->uregs[i];
		hdr->reg_desc[i].reg = reg;
		if (virt_addr_valid(reg)) {
			/* 
			 * ASSUMPION: memory is always in normal zone.
			 * 1) dump at most 32KB around valid kaddr
			 */
			/* align start address to PAGE_SIZE for gdb */
			start = round_down((reg - SZ_16K), PAGE_SIZE);
			end = start + SZ_32K;
			start = clamp(start, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory);
			end = clamp(end, (unsigned long)PAGE_OFFSET, (unsigned long)high_memory) - 1;
			hdr->reg_desc[i].kstart = start;
			hdr->reg_desc[i].kend = end;
			hdr->reg_desc[i].offset = offset;
			hdr->reg_desc[i].valid = 1;
			size = end - start + 1;

			memcpy(buf + offset, (void*)start, size);
			offset += size;
		} else {
			hdr->reg_desc[i].kstart = 0;
			hdr->reg_desc[i].kend = 0;
			hdr->reg_desc[i].offset = 0;
			hdr->reg_desc[i].valid = 0;
		}
	}

ipanic_write:
	if (emmc_ipanic_write(mrdump_mini_buf, IPANIC_MRDUMP_OFFSET, ALIGN(offset, SZ_512))) {
		printk(KERN_ALERT"card_dump_func_write failed (%d), size: 0x%llx\n", 
				i, (unsigned long long)offset);
	} else {
		printk(KERN_ALERT"card_dump_func_write ok (%d), size: 0x%llx\n", 
				i, (unsigned long long)offset);
	}
#endif
}
#else //MTK_EMMC_SUPPORT
static void __mrdump_mini_core(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, va_list ap)
{
}
#endif //MTK_EMMC_SUPPORT

static void __mrdump_mini_create_oops_dump(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	__mrdump_mini_core(reboot_mode, regs, msg, ap);
	va_end(ap);
}

static int mrdump_mini_create_oops_dump(struct notifier_block *self, unsigned long cmd, void *ptr)
{
	struct die_args *dargs = (struct die_args *)ptr;
	smp_send_stop();
	__mrdump_mini_create_oops_dump(AEE_REBOOT_MODE_KERNEL_PANIC, dargs->regs, "kernel Oops");
	return NOTIFY_DONE;
}

static struct notifier_block mrdump_mini_oops_blk = {
	.notifier_call	= mrdump_mini_create_oops_dump,
};

static int __init mrdump_mini_init(void)
{
	register_die_notifier(&mrdump_mini_oops_blk);
	return 0;
}

late_initcall(mrdump_mini_init);
