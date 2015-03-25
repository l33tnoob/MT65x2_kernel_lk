#ifndef SEC_H_
#define SEC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *  INCLUDING
 ******************************************************************************/
#include "../minzip/Zip.h"

/******************************************************************************
 *  DEFINITION
 ******************************************************************************/
#define SEC_OK                                  0x0000
#define SEC_SBOOT_NOT_ENABLED                   0x9007
#define SEC_SUSBDL_NOT_ENABLED                  0x9009


/******************************************************************************
 *  EXPORT FUNCTION
 ******************************************************************************/
extern bool sec_init(bool bDebug);
extern int sec_mark_status(bool bDebug);
extern int sec_update(bool bDebug);
extern int sec_verify_img_info (ZipArchive *zip,bool bDebug);
extern int sec_boot_init (bool, bool);
extern int siu_invalid (void);
extern int siu_verify_cust_name (unsigned char* cust_name, unsigned int name_len);
extern int siu_verify_img_ver (unsigned char* img_name, unsigned int img_ver);
extern int sec_verify_img_info_fota(const char *fname,bool bDebug);
extern int siu_mark_status (void);
extern int siu_recovery_update (void);

#ifdef __cplusplus
}
#endif

#endif  // SEC_H_
