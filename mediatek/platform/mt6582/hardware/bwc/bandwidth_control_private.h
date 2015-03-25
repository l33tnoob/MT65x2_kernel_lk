#ifndef __BANDWIDTH_CONTROL_PRIVATE_H__
#define __BANDWIDTH_CONTROL_PRIVATE_H__

/*=============================================================================
    Compile Flags
  =============================================================================*/
#define FLAG_SUPPORT_PROPERTY
//#define FLAG_SUPPORT_MODEM_SCALE
//#define FLAG_SUPPORT_SMI_SETTING




/*=============================================================================
    Header Files
  =============================================================================*/
#include    <stdio.h>   //printf()
#include    <unistd.h>  //gettid()
#include    <utils/Log.h>




/*=============================================================================
    MACRO
  =============================================================================*/
#define BWC_INFO(fmt, arg...)       { ALOGI("[BWC INFO](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define BWC_WARNING(fmt, arg...)    { ALOGW("[BWC W](%lu): "fmt,(unsigned long)gettid(), ##arg);   }
#define BWC_ERROR(fmt, arg...)      { ALOGE("[BWC E](%lu): %s(): %s@%d: "fmt,(unsigned long)gettid(),__FUNCTION__, __FILE__,__LINE__, ##arg);/*MdpDrv_DumpCallStack(NULL);*/  }



#endif  /*__BANDWIDTH_CONTROL_PRIVATE_H__*/

