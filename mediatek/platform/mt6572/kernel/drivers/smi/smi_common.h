#ifndef __SMI_COMMON_H__
#define __SMI_COMMON_H__

#include <linux/xlog.h>
#include <linux/aee.h>
#include <mach/mt_smi.h>


#define SMIMSG(string, args...)	xlog_printk(ANDROID_LOG_INFO, SMI_LOG_TAG, "[pid=%d]"string,current->tgid,##args)
#define SMIDBG(string, args...)  xlog_printk(ANDROID_LOG_DEBUG, SMI_LOG_TAG, "[pid=%d]"string,current->tgid,##args)
#define SMIERR(string, args...) do{\
	xlog_printk(ANDROID_LOG_ERROR,  SMI_LOG_TAG, "error: "string, ##args); \
	aee_kernel_warning(SMI_LOG_TAG, "error: "string, ##args);  \
}while(0)

#define smi_aee_print(string, args...) do{\
    char smi_name[100];\
    snprintf(smi_name,100, "["SMI_LOG_TAG"]"string, ##args); \
  aee_kernel_warning(smi_name, "["SMI_LOG_TAG"]error:"string,##args);  \
}while(0)


#define MAU_ENTRY_NR    3
#define SMI_LARB_NR     1


extern unsigned int gLarbBaseAddr[SMI_LARB_NR];
extern char *smi_port_name[16];


int larb_clock_on(int larb_id, const char *mod_name);
int larb_clock_off(int larb_id, const char *mod_name);

int mau_init(void);

int smi_bwc_config( MTK_SMI_BWC_CONFIG* p_conf );

#endif

