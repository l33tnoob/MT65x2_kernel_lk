#include <malloc.h>
#include <mt_partition.h>
#include <printf.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <video.h>
#include <dev/mrdump.h>
#include <platform/mtk_key.h>
#include <platform/mtk_wdt.h>
#include <platform/mt_gpt.h>
#include <target/cust_key.h>

#include "aee.h"
#include "kdump.h"

#define MRDUMP_DELAY_TIME 10

struct mrdump_cblock_result *cblock_result = NULL;

static void voprintf(char type, const char *msg, va_list ap)
{
    char msgbuf[128], *p;

    p = msgbuf;
    if (msg[0] == '\r') {
        *p++ = msg[0];
        msg++;
    }

    *p++ = type;
    *p++ = ':';
    vsnprintf(p, sizeof(msgbuf) - (p - msgbuf), msg, ap);
    switch (type) {
    case 'I':
    case 'W':
    case 'E':
        video_printf("%s", msgbuf);
        break;
    }

    dprintf(CRITICAL,"%s", msgbuf);
    
    /* Write log buffer */
    p = msgbuf;
    while ((*p != 0) && (cblock_result->log_size < sizeof(cblock_result->log_buf))) {
	cblock_result->log_buf[cblock_result->log_size] = *p++;
	cblock_result->log_size++;
    }
}

void voprintf_verbose(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('V', msg, ap);
    va_end(ap);
}

void voprintf_debug(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('D', msg, ap);
    va_end(ap);
}

void voprintf_info(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('I', msg, ap);
    va_end(ap);
}

void voprintf_warning(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('W', msg, ap);
    va_end(ap);
}

void voprintf_error(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    voprintf('E', msg, ap);
    va_end(ap);
}

static void mrdump_status(const char *status, const char *fmt, va_list ap)
{
    if (cblock_result != NULL) {
        char *dest = strcpy(cblock_result->status, status);
        dest += strlen(dest);
        *dest++ = '\n';
    
        vsnprintf(dest, sizeof(cblock_result->status) - (dest - cblock_result->status), fmt, ap);
    }
}

void mrdump_status_ok(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("OK", fmt, ap);
    va_end(ap);
}

void mrdump_status_none(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("NONE", fmt, ap);
    va_end(ap);
}

void mrdump_status_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    mrdump_status("FAILED", fmt, ap);
    va_end(ap);
}

uint32_t g_aee_mode = AEE_MODE_MTK_ENG;

const const char *mode2string(uint8_t mode)
{
  switch (mode) {
  case AEE_REBOOT_MODE_NORMAL:
    return "Normal reboot";

  case AEE_REBOOT_MODE_KERNEL_PANIC:
    return "Kernel panic";

  case AEE_REBOOT_MODE_NESTED_EXCEPTION:
    return "Nested CPU exception";

  case AEE_REBOOT_MODE_WDT:
    return "Hardware watch dog triggered";

  case AEE_REBOOT_MODE_EXCEPTION_KDUMP:
    return "Kernel exception dump";

  default:
    return "Unknown reboot mode";
  }
}

static void kdump_ui(struct mrdump_control_block *mrdump_cblock)
{
    video_clean_screen();

    mrdump_status_error("Unknown error\n");
    voprintf_info("Kdump triggerd by '%s'\n", mode2string(mrdump_cblock->crash_record.reboot_mode));

    struct aee_timer elapse_time;
    aee_timer_init(&elapse_time);

    uint32_t total_dump_size = memory_size();
    
    aee_timer_start(&elapse_time);
    switch (mrdump_cblock->machdesc.output_device) {
    case MRDUMP_DEV_NULL:
        kdump_null_output(mrdump_cblock, total_dump_size);
        break;
#if 0
    case MRDUMP_DEV_SDCARD:
        kdump_sdcard_output(mrdump_cblock, total_dump_size);
        break;
#endif
    case MRDUMP_DEV_EMMC:
        kdump_emmc_output(mrdump_cblock, total_dump_size);
        break;

    default:
        voprintf_error("Unknown device id %d\n", mrdump_cblock->machdesc.output_device);
    }

    aee_timer_stop(&elapse_time);
    
    voprintf_info("Reset count down %d ...\n", MRDUMP_DELAY_TIME);
    mtk_wdt_restart();

    int timeout = MRDUMP_DELAY_TIME;
    while(timeout-- >= 0) {
        mdelay(1000);
        mtk_wdt_restart();
	voprintf_info("\rsec %d", timeout);
    }
    voprintf_info("Prepare to boot...");
    mtk_arch_reset(1);
}

int aee_kdump_detection(void)
{
#ifdef USER_BUILD
    return 0;
#else
    struct mrdump_control_block *mrdump_cblock = aee_mrdump_get_params();
    if (mrdump_cblock == NULL) {
        return 0;
    }

    memset(&mrdump_cblock->result, 0, sizeof(struct mrdump_cblock_result));
    cblock_result = &mrdump_cblock->result;

    uint8_t reboot_mode = mrdump_cblock->crash_record.reboot_mode;
    voprintf_debug("sram record with mode %d\n", reboot_mode);
    switch (reboot_mode) {
    case AEE_REBOOT_MODE_NORMAL:  {
	mrdump_status_none("Normal boot\n");
        return 0;
    }
    case AEE_REBOOT_MODE_KERNEL_PANIC:
    case AEE_REBOOT_MODE_NESTED_EXCEPTION:
      kdump_ui(mrdump_cblock);
      break;

    case AEE_REBOOT_MODE_WDT:
      kdump_ui(mrdump_cblock);
      break;

    case AEE_REBOOT_MODE_EXCEPTION_KDUMP:
      kdump_ui(mrdump_cblock);
      break;
    }
    return 0;
#endif
}


void aee_timer_init(struct aee_timer *t)
{
    memset(t, 0, sizeof(struct aee_timer));
}

void aee_timer_start(struct aee_timer *t)
{
    t->start_ms = get_timer_masked();
}

void aee_timer_stop(struct aee_timer *t)
{
    t->acc_ms += (get_timer_masked() - t->start_ms);
    t->start_ms = 0;
}

