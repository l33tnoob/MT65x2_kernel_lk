#ifndef _MT_SLEEP_
#define _MT_SLEEP_

#include <linux/kernel.h>
#include <mach/mt_spm_api.h>



extern wake_reason_t slp_get_wake_reason(void); //used in battery resume
extern bool slp_will_infra_pdn(void);
extern void slp_module_init(void);

#endif
