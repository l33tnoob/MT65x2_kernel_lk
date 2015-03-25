#if !defined(__MRDUMP_H__)
#define __MRDUMP_H__

#include <stdarg.h>
#include <linux/elf.h>
#include <linux/elfcore.h>
#include <linux/aee.h>

#define MRDUMP_CPU_MAX 16

struct kdump_alog {
	unsigned char *buf;
	int size;
	size_t *woff;
	size_t *head;
};

#define MRDUMP_DEV_NULL 0
#define MRDUMP_DEV_SDCARD 1
#define MRDUMP_DEV_EMMC 2

struct mrdump_crash_record {
	int reboot_mode;

	char msg[128];
	char backtrace[512];
	
	uint32_t fault_cpu;
	elf_gregset_t cpu_regs[MRDUMP_CPU_MAX];
};

struct mrdump_machdesc {
	uint32_t crc;
  
	uint32_t output_device;

	uint32_t nr_cpus;

	void *page_offset;
	void *high_memory;

        void *vmalloc_start;
        void *vmalloc_end;

        void *modules_start;
        void *modules_end;

	void *phys_offset;
	void *master_page_table;

	char *log_buf;
	int log_buf_len;
	unsigned int *log_end;

	struct kdump_alog android_main_log;
	struct kdump_alog android_system_log;
	struct kdump_alog android_radio_log;
};

struct mrdump_cblock_result {
	char status[128];

	size_t log_size;
	char log_buf[2048];
};

struct mrdump_control_block {
	char sig[8];

	struct mrdump_machdesc machdesc;
	struct mrdump_crash_record crash_record;
	struct mrdump_cblock_result result;
};

struct mrdump_platform {
	void (*hw_enable)(bool enabled);
	void (*reboot)(void);
};

#if defined(CONFIG_MTK_AEE_MRDUMP)

void mrdump_reserve_memory(void);

void mrdump_platform_init(struct mrdump_control_block *cblock, const struct mrdump_platform *plafrom);

void __mrdump_create_oops_dump(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, ...);
#else

static inline void mrdump_reserve_memory(void) {}

static inline void mrdump_platform_init(struct mrdump_control_block *cblock, void (*hw_enable)(bool enabled)) {}

static inline void __mrdump_create_oops_dump(AEE_REBOOT_MODE reboot_mode, struct pt_regs *regs, const char *msg, ...) {}
#endif

//#define DUMMY

#ifdef DUMMY
#define MRDUMP_MINI_BUF_SIZE (8192)
#else
#define MRDUMP_MINI_HEADER_SIZE (SZ_1K)
#define MRDUMP_MINI_DATA_SIZE (SZ_1M)
#define MRDUMP_MINI_BUF_SIZE (MRDUMP_MINI_HEADER_SIZE + MRDUMP_MINI_DATA_SIZE)
#endif 
#define IPANIC_MRDUMP_OFFSET (0x400000)

#endif
