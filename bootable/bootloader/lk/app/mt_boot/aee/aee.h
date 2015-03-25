#if !defined (__AEE_H__)
#define __AEE_H__

#include <stdint.h>
#include <dev/mrdump.h>

typedef enum {
  AEE_MODE_MTK_ENG = 1,
  AEE_MODE_MTK_USER,
  AEE_MODE_CUSTOMER_ENG,
  AEE_MODE_CUSTOMER_USER 
} AEE_MODE;

typedef enum {
  AEE_REBOOT_MODE_NORMAL = 0,
  AEE_REBOOT_MODE_KERNEL_PANIC,
  AEE_REBOOT_MODE_NESTED_EXCEPTION,
  AEE_REBOOT_MODE_WDT,
  AEE_REBOOT_MODE_EXCEPTION_KDUMP,
} AEE_REBOOT_MODE;

#define AEE_IPANIC_MAGIC 0xaee0dead
#define AEE_IPANIC_PHDR_VERSION   0x04

struct ipanic_header {
	/* The magic/version field cannot be moved or resize */
	uint32_t magic;
	uint32_t version;

	uint32_t oops_header_offset;
	uint32_t oops_header_length;

	uint32_t oops_detail_offset;
	uint32_t oops_detail_length;

	uint32_t console_offset;
	uint32_t console_length;

	uint32_t android_main_offset;
	uint32_t android_main_length;
	
	uint32_t android_event_offset;
	uint32_t android_event_length;
	
	uint32_t android_radio_offset;
	uint32_t android_radio_length;
	
	uint32_t android_system_offset;
	uint32_t android_system_length;

	uint32_t userspace_info_offset;
	uint32_t userspace_info_length;
};

#define IPANIC_OOPS_HEADER_PROCESS_NAME_LENGTH 256
#define IPANIC_OOPS_HEADER_BACKTRACE_LENGTH 3840

struct ipanic_oops_header 
{
	char process_path[IPANIC_OOPS_HEADER_PROCESS_NAME_LENGTH];
	char backtrace[IPANIC_OOPS_HEADER_BACKTRACE_LENGTH];
};

extern uint32_t g_aee_mode;

struct mrdump_control_block *aee_mrdump_get_params(void);

void voprintf_verbose(const char *msg, ...);
void voprintf_debug(const char *msg, ...);
void voprintf_info(const char *msg, ...);
void voprintf_warning(const char *msg, ...);
void voprintf_error(const char *msg, ...);

void mrdump_status_none(const char *fmt, ...);
void mrdump_status_ok(const char *fmt, ...);
void mrdump_status_error(const char *fmt, ...);

struct aee_timer {
    unsigned int acc_ms;
    
    unsigned int start_ms;
};

void aee_timer_init(struct aee_timer *t);
void aee_timer_start(struct aee_timer *t);
void aee_timer_stop(struct aee_timer *t);

/* FIXME: move to platform/mtk_wdt.h */
extern void mtk_wdt_restart(void);
extern ulong get_timer_masked (void);
extern uint32_t memory_size(void);

#endif
