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

#include "Local.h"
#include "Feature.h"
#include <camera/MtkCameraParameters.h>
using namespace android;
//
#include <sys/sysconf.h>
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[Feature::%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[Feature::%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#include "functor.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace
{


//  AF mode must be supported at least one item.
struct FUNC_AF_Mode
{
    char const* getFKey() const { return  CameraParameters::KEY_FOCUS_MODE; }

    bool        operator()(FeatureInfo& rFInfo) const
                {
                    if  ( 0 == rFInfo.mSupportedList.size() )
                    {   //  If empty, set just one item as the default.
                        String8 const s8Item(CameraParameters::FOCUS_MODE_INFINITY);
                        MY_LOGD("<correct> %s=%s", getFKey(), s8Item.string());
                        rFInfo.mDefault = s8Item;
                        rFInfo.mSupportedList.clear();
                        rFInfo.mSupportedList.push_back(s8Item);
                    }
                    return  true;
                }
};


//  Not support shot mode due to low memory
struct FUNC_Not_Support_Shot_Mode_due_to_Low_Memory
{
    char const* getFKey() const { return  MtkCameraParameters::KEY_CAPTURE_MODE; }

    bool        operator()(FeatureInfo& rFInfo) const
                {
                    rFInfo.removeOneItem(FITEM_T(MtkCameraParameters::CAPTURE_MODE_HDR_SHOT));
                    return  true;
                }
};

//  Not support scene mode due to low memory
struct FUNC_Not_Support_Scene_Mode_due_to_Low_Memory
{
    char const* getFKey() const { return  MtkCameraParameters::KEY_SCENE_MODE; }

    bool        operator()(FeatureInfo& rFInfo) const
                {
                    rFInfo.removeOneItem(FITEM_T(MtkCameraParameters::SCENE_MODE_HDR));
                    return  true;
                }
};


#ifdef MTK_UMTS_TDD128_MODE
struct FUNC_Support_Normal_Scene_Mode_For_TDD_Projects
{
    char const* getFKey() const { return  MtkCameraParameters::KEY_SCENE_MODE; }

    bool        operator()(FeatureInfo& rFInfo) const
                {
                    String8 const s8Item(MtkCameraParameters::SCENE_MODE_NORMAL);
                    if  ( 0 == rFInfo.mSupportedList.size() )
                    {
                        MY_LOGD("Insert NORMAL scene mode for TDD projects\n");
                        rFInfo.mSupportedList.push_back(s8Item);
                        return true;
                    }

                    size_t i;
                    for (i = 0; i < rFInfo.mSupportedList.size(); i++)
                    {
                        if  ( s8Item == rFInfo.mSupportedList[i] )
                            break;
                    }
                    if (i == rFInfo.mSupportedList.size())
                    {   /* Not found */
                        MY_LOGD("Insert NORMAL scene mode for TDD projects\n");
                        rFInfo.mSupportedList.push_back(s8Item);
                    }
                    return  true;
                }
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
};  //  namespace


/******************************************************************************
 *
 ******************************************************************************/
bool
Feature::
initFeatures_NoWarningCorrection()
{
    //  Correct without warning
    bool ret = false;

    int const   memory_total_in_byte = ::sysconf(_SC_PHYS_PAGES) * getpagesize();
    bool const  fgIsLowMem = (memory_total_in_byte <= 256 * 1024 *1024);    //  low memory if < 256MB (2Gb)

    MY_LOGI("(fgIsLowMem, memory_total_in_byte, getpagesize)=(%d, %d, %d)",fgIsLowMem,memory_total_in_byte, getpagesize());

    if(fgIsLowMem)
    {
#if 1
        //  Force to disable some shot modes.
        if(! Functor_Handle_One_Feature<FUNC_Not_Support_Shot_Mode_due_to_Low_Memory>()(mFeatureMap) )
        {
            MY_LOGE("Functor_Handle_One_Feature<FUNC_Not_Support_Shot_Mode_due_to_Low_Memory> fail");
            goto lbExit;
        }

        //  Force to disable some scene modes.
        if(! Functor_Handle_One_Feature<FUNC_Not_Support_Scene_Mode_due_to_Low_Memory>()(mFeatureMap) )
        {
            MY_LOGE("Functor_Handle_One_Feature<FUNC_Not_Support_Scene_Mode_due_to_Low_Memory> fail");
            goto lbExit;
        }
#endif
    }


#if 1
    //  AF mode must be supported at least one item.
    if  ( ! Functor_Handle_One_Feature<FUNC_AF_Mode>()(mFeatureMap) )
    {
        MY_LOGE("Functor_Handle_One_Feature<FUNC_AF_Mode> fail");
        goto lbExit;
    }
#endif

#ifdef MTK_UMTS_TDD128_MODE
    //  Add NORMAL scene mode for TDD projects.
    if  ( ! Functor_Handle_One_Feature<FUNC_Support_Normal_Scene_Mode_For_TDD_Projects>()(mFeatureMap) )
    {
        MY_LOGE("Functor_Handle_One_Feature<FUNC_Support_Normal_Scene_Mode_For_TDD_Projects> fail");
        goto lbExit;
    }
#endif

    ret = true;
lbExit:
    return  ret;
}

