/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/******************************************************
*created by mingliangzhong(mtk80309)@2010-11-04
*modify history:
*
******************************************************/

#ifndef _VT_COMPLE_OPTIONS_H_
#define _VT_COMPLE_OPTIONS_H_

/***************************************
*ut with vttests program.
****************************************/
//#define _TEST_ON_VTTESTS_APP


/***************************************
*log debug infos. plse disable this marco when qc started.
****************************************/
//#define VTMAL_LOG_VERSION


/***************************************
*dump some raw datas for debug enalyze.
****************************************/
//#define VTMAL_DUMP_FILE

//#define VTMAL_DUMP_RECEIVE_BITSTREAM
//#define  VTMAL_USE_YUV_FILE_FROM_CC


/***************************************
*enable this macro if platform version >=2.3
****************************************/
#define  VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE


/***************************************
*plesase do not enable this macro if  mtk surfaceflinger is not ready
****************************************/
#define DEF_ISURFACE_EXTEN_API 


/***************************************
*plesase do not enable this macro if  mtk ISurface variavle api is not ready
****************************************/
#define DEF_ISURFACE_VARIABLE_POST 

/***************************************
*use unify log filter for VT project
****************************************/
#define VT_ENABLE_LOG_FILTER

/***************************************
*use to calculate the framrate of sensor
****************************************/
#define VT_CALCULATE_SENSOR_FR

//********************************************************************************************************
//********************************************************************************************************
//********************************************************************************************************
//define log macro!!!!!!!!!!!!!!!!
#define VTMAL_USE_XLOG

#ifdef VT_ENABLE_LOG_FILTER
//#include "../../VT/utils/mtk_vt_log.h"
#include <cutils/xlog.h>
#define _V(...) XLOGV(__VA_ARGS__);
#define _D(...) XLOGD(__VA_ARGS__);
#define _I(...) XLOGI(__VA_ARGS__);
#define _W(...) XLOGW(__VA_ARGS__);
#define _E(...) XLOGE(__VA_ARGS__);
#endif //VT_ENABLE_LOG_FILTER
#ifdef VTMAL_USE_XLOG
#include <cutils/xlog.h>
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOGV
#define LOGE XLOGE
#define LOGW XLOGW
#define LOGI XLOGI
#define LOGD XLOGD
#define LOGV XLOGV
#elif defined(ALOGI)
#define LOGE ALOGE
#define LOGW ALOGW
#define LOGI ALOGI
#define LOGD ALOGD
#define LOGV ALOGV
#endif //VT_ENABLE_LOG_FILTER


#ifdef VT_ENABLE_LOG_FILTER
	#ifdef VTMAL_LOG_VERSION
	#undef VTMAL_LOG_VERSION
	#endif
	#define VTMAL_LOGERR  _E("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
#else
	#define VTMAL_LOGERR  LOGE("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
#endif //VT_ENABLE_LOG_FILTER

#ifdef  VTMAL_LOG_VERSION
	#define VTMAL_LOGDEBUG  LOGD("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
	#define VTMAL_LOGINFO  LOGD("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
	#define VTMAL_PRINTINFO

#else
	#ifdef VT_ENABLE_LOG_FILTER
		#ifndef  LOG_TAG_IND
		#define LOG_TAG_IND VT_LOG_MAL_TAG
		#endif //LOG_TAG_IND

		#ifdef LOGE
		#undef LOGE
		#endif //LOGE

		#ifdef LOGD
		#undef LOGD
		#endif //LOGD

		#ifdef LOGI
		#undef LOGI
		#endif //LOGD

		#define LOGE _E
		#define LOGD _D
		#define LOGI _I
		

		#define VTMAL_LOGDEBUG _D("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
		#define VTMAL_LOGINFO   _I("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
		#define VTMAL_PRINTINFO _V("[VTMAL]@%s,@%d ",__FUNCTION__,__LINE__);
	#else
		#define VTMAL_LOGDEBUG
		#define VTMAL_LOGINFO  
		#define VTMAL_PRINTINFO 
	#endif //VT_ENABLE_LOG_FILTER
#endif //VTMAL_LOG_VERSION

#ifdef _TEST_ON_VTTESTS_APP
	#ifdef VTMAL_PRINTINFO
	#undef VTMAL_PRINTINFO
	#endif //VTMAL_PRINTINFO
	#define VTMAL_PRINTINFO  printf("[VTMAL]@%s,@%s,@%d \n",__FILE__,__FUNCTION__,__LINE__);
#endif //_TEST_ON_VTTESTS_APP

/*****************************************************
 *this complie macro should be enable !!!!!!!!!!!!!!!!
 *to implement zoom feature mingliang marked @2011-03-16
*****************************************************/
#define VT_DISABLE_RECORDING_MODE

/*****************************************************
 * this complie macro is use to create another process
 * thread to handle async-callback, avoiding following
 * issue due to CPU loading:
 * 1. binder allocating async memory failure.
 * 2. image tearing
 * @2013-05-29
*****************************************************/
#ifdef VT_DISABLE_RECORDING_MODE
    #define VT_CAMERASOURCE_PROCESSTHREAD
#endif
/*****************************************************
 *this complie macro is add by mtk80691 on 2011-10-31
 *for VT PeerVideo and Mixed Audio Record feature need timestamp
 *we can choose the timestamp provider:VTStack or VTMAL 
*****************************************************/
#define VTMS_ENABLE_CHOOSE_TM_PROVIDER


/*****************************************************
 * plse enable this marco on new platform
 *
*****************************************************/
#define DISABLE_PMEMORY_MODE
//********************************************************************************************************
//********************************************************************************************************
//********************************************************************************************************

#ifdef  VTMAL_SUPPORT_ANDROID_2_3_VER_AND_ABOVE
#ifdef VT_CAMERA_NO_ROTATE_PREVIEW
//#define VT_ROTATE_PREVIEW_SW
#endif
#endif
#endif //_VT_COMPLE_OPTIONS_H_
