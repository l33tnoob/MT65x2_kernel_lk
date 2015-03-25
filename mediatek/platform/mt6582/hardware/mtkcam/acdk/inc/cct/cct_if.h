#ifndef _CCTIF_H_
#define _CCTIF_H_

#include <mtkcam/acdk/cct_feature.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
*    @CCT_IF.CPP
*    CCT_IF provides user to do camera calibration or tuning by means of
*    mixing following commands. Which involoved with
*    3A, ISP, sensor, NVRAM, Calibration, ACDK releated.
*
*    CCT FW working model is based on ACDK framework, so it is required make
*    sure ACDKBase object is available before CCTIF Open.
*
*    CCT FW presents the following APIs:
*
*    CCTIF_Open
*    CCTIF_Close
*    CCTIF_Init
*    CCTIF_DeInit
*    CCTIF_IOControl
*/

MBOOL CCTIF_Open();


MBOOL CCTIF_Close();


MBOOL CCTIF_Init(MINT32 dev = 1); //default sensor device = Main Cam


MBOOL CCTIF_DeInit();


MBOOL CCTIF_IOControl(MUINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo);

MBOOL CCTIF_FeatureCtrl(MUINT32 a_u4Ioctl, MUINT8 *puParaIn, MUINT32 u4ParaInLen, MUINT8 *puParaOut, MUINT32 u4ParaOutLen, MUINT32 *pu4RealParaOutLen);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
