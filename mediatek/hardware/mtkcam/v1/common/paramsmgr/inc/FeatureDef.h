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

#ifndef _MTK_CAMERA_COMMON_PARAMSMGR_INC_FEATUREDEF_H_
#define _MTK_CAMERA_COMMON_PARAMSMGR_INC_FEATUREDEF_H_
//
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>


/*******************************************************************************
 *
 ******************************************************************************/
namespace NSCameraFeature
{


namespace NSSensorType
{
    enum
    {
        eSensorType_RAW =       0,      //  RAW Sensor
        eSensorType_YUV,                //  YUV Sensor
    };
}


/*******************************************************************************
 *
 ******************************************************************************/
typedef android::String8            FKEY_T;
typedef android::String8            FSCENE_T;
typedef android::String8            FITEM_T;
typedef android::Vector<FITEM_T>    FITEMLIST_T;


/*******************************************************************************
 *  Feature Information.
 ******************************************************************************/
struct FeatureInfo
{
public:     ////            Data Members.
    FITEM_T                 mDefault;           //  "auto"
    FITEMLIST_T             mSupportedList;     //  "auto", "normal", "portrait"

public:     ////            Operations.
                            FeatureInfo()
                                : mDefault()
                                , mSupportedList()
                            {}

                            FeatureInfo(
                                char const* szDefault, 
                                char const* szSupportedList[], 
                                size_t cbSupportedSize
                            )
                                : mDefault(FITEM_T(szDefault))
                            {
                                for (size_t i = 0; i < cbSupportedSize; i++) {
                                    mSupportedList.push_back(FITEM_T(szSupportedList[i]));
                                }
                            }

public:     ////            Operations.
                            //
    FITEM_T const&          getDefaultItem() const                  { return mDefault; }
    void                    setDefaultItem(FITEM_T const& item)     { mDefault = item; }
                            //
    size_t                  getSupportedSize() const                { return mSupportedList.size(); }
    FITEM_T const&          getSupportedItem(size_t index) const    { return mSupportedList[index]; }
                            //
    FITEM_T                 getSupportedList() const
                            {
                                FITEM_T list = getSupportedItem(0);
                                for (uint32_t j = 1; j < getSupportedSize(); j++)
                                {
                                    list += ",";
                                    list += getSupportedItem(j);
                                }
                                return  list;
                            }
                            //
    bool                    removeOneItem(FITEM_T const& rFItem)
                            {
                                for (size_t i = 0; i < mSupportedList.size(); i++)
                                {
                                    if  ( rFItem == mSupportedList[i] )
                                    {
                                        mSupportedList.removeAt(i);
                                        if  ( rFItem == mDefault )
                                        {
                                            mDefault = (mSupportedList.size() > 0)
                                                        ? mSupportedList[0] : FITEM_T::empty();
                                        }
                                        return  true;
                                    }
                                }
                                return  false;
                            }
};


/*******************************************************************************
 *  Map between Scene key and Feature Information.
 ******************************************************************************/
struct SceneKeyedMap : public android::KeyedVector<FSCENE_T, FeatureInfo>
{
public:     ////            Data Members.
    FeatureInfo             mDefault;
    android::String8        ms8FType;   //  "default-values", "default-supported", "user-defined"

public:     ////            Operations.
                            SceneKeyedMap(
                                char const* szFType = "default-values", 
                                FeatureInfo const& rFInfo = FeatureInfo()
                            )
                                :
                            	#if (PLATFORM_VERSION_MAJOR > 2)
							 	KeyedVector(),
                                #endif 
								mDefault(rFInfo)
                                , ms8FType(android::String8(szFType))
                            {
                            }

public:     ////            Operations.
    android::String8 const& getType() const     { return ms8FType; }
    FeatureInfo&            getDefault()        { return mDefault; }
    FeatureInfo const&      getDefault() const  { return mDefault; }
    FeatureInfo const&      valueFor(FSCENE_T const& key) const
                            {
                                ssize_t i = indexOfKey(key);
                                return i >= 0 ? valueAt(i) : mDefault;
                            }
};


/*******************************************************************************
 *  Map between Feature key and Scene-keyed Feature Information.
 ******************************************************************************/
struct FeatureKeyedMap : public android::KeyedVector<FKEY_T, SceneKeyedMap>
{
public:     ////            Operations.
    FITEM_T const&          getCurrentSceneMode() const
                            {
                                return  valueFor(String8(CameraParameters::KEY_SCENE_MODE)).getDefault().getDefaultItem();
                            }
    bool                    setCurrentSceneMode(FITEM_T const& scene)
                            {
                                editValueFor(String8(CameraParameters::KEY_SCENE_MODE)).getDefault().setDefaultItem(scene);
                                return  true;
                            }
};


/*******************************************************************************
 *
 ******************************************************************************/
};  //  namespace NSCameraFeature
#endif // _MTK_CAMERA_COMMON_PARAMSMGR_INC_FEATUREDEF_H_

