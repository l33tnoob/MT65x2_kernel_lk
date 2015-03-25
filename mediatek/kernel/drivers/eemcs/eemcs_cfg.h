#ifndef __EEMCS_CFG_H__
#define __EEMCS_CFG_H__


#define _ECCMNI_SEQ_SUPPORT_   /* ADD sequence number in cccih->reserved */

#define __ECCMNI_SUPPORT__
#define __EEMCS_EXPT_SUPPORT__ /* exception mode support */
//#define _EEMCS_EXCEPTION_UT    //Enable exception mode UT

//#define _ECCMNI_LB_UT_       /* configure EMCS_NET  as UL loopback mode */
//#define _EEMCS_CDEV_LB_UT_     /* configure EMCS_CHAR as UL loopback mode */

//#define _EEMCS_CCCI_LB_UT      /* configure EMCS_CCCI as UL loopback mode*/

//Temp disable for testing
#define __EEMCS_XBOOT_SUPPORT__    // Enable/Disable xBoot flow
//#define _EEMCS_TRACE_SUPPORT    // Enable/Disable xBoot flow tracing
//#define _EEMCS_BOOT_UT          // Enable/Disable xBoot UT

//#define _EEMCS_FS_UT            // Enable FS UT
//#define _EEMCS_RPC_UT            // Enable RPC UT


//******other feature configure******//
#define  ENABLE_AEE_MD_EE				//disable for bring up
#define  ENABLE_MD_WDT_PROCESS			//disable for bring up for md not enable wdt at bring up

#ifdef _EEMCS_CCCI_LB_UT
#ifndef _EEMCS_BOOT_UT
#define _EEMCS_BOOT_UT
#endif
#endif

#endif //__EEMCS_CFG_H__

