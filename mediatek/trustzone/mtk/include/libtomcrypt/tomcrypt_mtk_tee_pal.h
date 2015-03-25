#ifndef TOMCRYPT_MTK_TEE_PAL_H_
#define TOMCRYPT_MTK_TEE_PAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tz_private/sys_ipc.h"
#define LTC_NO_PROTOTYPES

/* [mtk00719] for MTK In-House TEE  */
#ifndef MTK_TEE_SUPPORT
#define MTK_TEE_SUPPORT
#endif

/* random number generator of mtk tee platform */
extern unsigned long rng_mtk_tee(unsigned char *buf, unsigned long len, 
                                 void (*callback)(void));

#ifdef __cplusplus
   }
#endif

#endif /* TOMCRYPT_MTK_TEE_PAL_H_ */

