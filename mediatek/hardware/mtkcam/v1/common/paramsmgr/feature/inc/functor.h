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

#ifndef _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FUNCTOR_H_
#define _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FUNCTOR_H_

namespace NSCameraFeature
{


/*******************************************************************************
*
*   Functor of Handle One Feature
*
*   @param _Impl_T Type of functor implementation.
*   An implementation type must define the functor with prototype:
*       char const* getFKey() const
*       bool operator()(FeatureInfo*const pFInfo_Tgt) const
*
*******************************************************************************/
template <class _Impl_T>
class Functor_Handle_One_Feature
{
public:
    bool operator()(FeatureKeyedMap& rFeatureMap) const
    {
        ssize_t kidx = rFeatureMap.indexOfKey(String8(m_Impl.getFKey()));
        if  ( kidx < 0 ) {
            MY_LOGW("not found KEY: %s", m_Impl.getFKey());
            return  false;
        }
        //
        SceneKeyedMap& rSceneKeyedMap = rFeatureMap.editValueAt(kidx);
        MY_LOGD_IF(1, "<%s> has %d scenes", m_Impl.getFKey(), rSceneKeyedMap.size());
        //  [2.1]   default scene.
        if  ( ! m_Impl(rSceneKeyedMap.getDefault()) )
        {
            MY_LOGE("<%s> fails for default scene", m_Impl.getFKey());
            return  false;
        }
        for (size_t scene = 0; scene < rSceneKeyedMap.size(); scene++)
        {
            FeatureInfo& rFInfo = rSceneKeyedMap.editValueAt(scene);
            if  ( ! m_Impl(rFInfo) )
            {
                MY_LOGE("<%s> fails for %s scene", m_Impl.getFKey(), rSceneKeyedMap.keyAt(scene).string());
                return  false;
            }
        }
        return  true;
    }

    Functor_Handle_One_Feature()
        : m_Impl()
    {}

    Functor_Handle_One_Feature(_Impl_T _impl)
        : m_Impl(_impl)
    {}

private:
    _Impl_T     m_Impl;
};


/*******************************************************************************
 *
 ******************************************************************************/
};  //  namespace NSCameraFeature
#endif // _MTK_CAMERA_COMMON_PARAMSMGR_FEATURE_INC_FUNCTOR_H_

