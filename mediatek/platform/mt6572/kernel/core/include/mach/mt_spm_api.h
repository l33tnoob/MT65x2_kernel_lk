#ifndef _MT_SPM_API_
#define _MT_SPM_API_

#include <mach/mt_spm.h>
#include <mach/mt_spm_mtcmos.h>

/*
 * for suspend
 */
//extern int spm_set_sleep_wakesrc(u32 wakesrc, bool enable, bool replace);
extern wake_reason_t spm_go_to_sleep(void);
//extern void spm_output_sleep_option(void);

/*
 * for deep idle
 */
extern void spm_dpidle_before_wfi(void);        /* can be redefined */
extern void spm_dpidle_after_wfi(void);         /* can be redefined */
extern wake_reason_t spm_go_to_dpidle(bool cpu_pdn, u8 pwrlevel);

/*
*for MCDI
*/
extern void spm_check_core_status_before(u32 target_core);
extern void spm_check_core_status_after(u32 target_core);
extern void spm_hot_plug_in_before(u32 target_core);
extern void spm_hot_plug_out_after(u32 target_core);
extern void spm_mcdi_wfi(void);
extern void spm_disable_sodi(void);
extern void spm_enable_sodi(void);
extern void spm_disable_sodi_user(void);
extern void spm_enable_sodi_user(void);
extern bool spm_is_sodi_user_en(void);
//#define SPM_MCDI_FUNC

#endif