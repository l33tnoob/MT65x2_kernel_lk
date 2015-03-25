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

#define LOG_TAG "MtkCam/ParamsManager"
//
#include <mtkcam/Log.h>
//
#include <camera/MtkCameraParameters.h>
using namespace android;
//
#include <FeatureDef.h>
using namespace NSCameraFeature;
//


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/*******************************************************************************
 *
 ******************************************************************************/
#define FTABLE_DEFINITION(_func_name_)                                          \
    extern "C"                                                                  \
    bool                                                                        \
    queryCustCamfeature_##_func_name_(FeatureKeyedMap& rFMap, uint32_t u4SensorType, int facing)    \
    {                                                                           \
        MY_LOGD("");                                                            \
        /*rOut.clear();*/

#define END_FTABLE_DEFINITION()                                                 \
        return  true;                                                           \
    }

#define FTABLE_SCENE_INDEP()                                                    \
    {

#define END_FTABLE_SCENE_INDEP()                                                \
    }

#define FTABLE_SCENE_DEP()                                                      \
    {

#define END_FTABLE_SCENE_DEP()                                                  \
    }

/*******************************************************************************
 *
 ******************************************************************************/
#define KEY_AS_(_key_)                  _key_
//
#define ITEM_AS_DEFAULT_(_item_)        _item_
//
#define ITEM_AS_VALUES_(_items_...)     _items_
#define ITEM_AS_SUPPORTED_(_item_)      _item_
#define ITEM_AS_USER_LIST_(_items_...)  _items_
//
#define SCENE_AS_DEFAULT_SCENE(_macro_item_default_, _macro_item_list_)                         \
    char const* szDefaultScene_ItemDefault[] = { _macro_item_default_ };                        \
    char const* szDefaultScene_ItemList   [] = { _macro_item_list_ };                           \
    enum { eDefaultScene_ItemListSize = sizeof(szDefaultScene_ItemList)/sizeof(szDefaultScene_ItemList[0]) }; \
    SceneKeyedMap sceneKeyedMap(szFType, FeatureInfo(                                           \
        szDefaultScene_ItemDefault[0],                                                          \
        szDefaultScene_ItemList, eDefaultScene_ItemListSize                                     \
    ));
//
#define SCENE_AS_(_scene_key_, _macro_item_default_, _macro_item_list_)                         \
        {                                                                                       \
        char const* szScene = _scene_key_;                                                      \
        char const* szDefault[] = { _macro_item_default_ };                                     \
        char const* szList   [] = { _macro_item_list_ };                                        \
        enum { eListSize = sizeof(szList)/sizeof(szList[0]) };                                  \
        sceneKeyedMap.add(FSCENE_T(szScene), FeatureInfo(szDefault[0], szList, eListSize));     \
        }
//
#define FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(_macro_feature_key_, _macro_scene_default_, _macro_scene_...)   \
    {                                                                                           \
    char const* szFType = "default-values";                                                     \
    char const* szFKey = _macro_feature_key_;                                                   \
    _macro_scene_default_                                                                       \
    _macro_scene_                                                                               \
    rFMap.add(FKEY_T(szFKey), sceneKeyedMap);                                                   \
    MY_LOGD_IF(0, "%s=%s", szFKey, sceneKeyedMap.getDefault().getDefaultItem().string());       \
    }
//
#define FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(_macro_feature_key_, _macro_scene_default_, _macro_scene_...)   \
    {                                                                                           \
    char const* szFType = "default-supported";                                                  \
    char const* szFKey = _macro_feature_key_;                                                   \
    _macro_scene_default_                                                                       \
    _macro_scene_                                                                               \
    rFMap.add(FKEY_T(szFKey), sceneKeyedMap);                                                   \
    MY_LOGD_IF(0, "%s=%s", szFKey, sceneKeyedMap.getDefault().getDefaultItem().string());       \
    }
//
#define FTABLE_CONFIG_AS_TYPE_OF_USER(_macro_feature_key_, _macro_scene_default_, _macro_scene_...) \
    {                                                                                           \
    char const* szFType = "user";                                                               \
    char const* szFKey = _macro_feature_key_;                                                   \
    _macro_scene_default_                                                                       \
    _macro_scene_                                                                               \
    rFMap.add(FKEY_T(szFKey), sceneKeyedMap);                                                   \
    MY_LOGD_IF(0, "%s=%s", szFKey, sceneKeyedMap.getDefault().getDefaultItem().string());       \
    }
//
/******************************************************************************
 *
 ******************************************************************************/
#include "custgen.config.ftbl.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCameraFeature   {
namespace NSCustom          {
void
showCustInfo()
{
#if defined(MY_CUST_VERSION)
    MY_LOGD("MY_CUST_VERSION=\"%s\"", MY_CUST_VERSION);
#endif
#if defined(MY_CUST_FTABLE_FILE_LIST)
    MY_LOGD("MY_CUST_FTABLE_FILE_LIST=\"%s\"", MY_CUST_FTABLE_FILE_LIST);
#endif
}
};  //  namespace NSCustom
};  //  namespace NSCameraFeature

