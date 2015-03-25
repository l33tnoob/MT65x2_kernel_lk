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

package com.mediatek.filemanager.utils;

import android.os.SystemProperties;
import com.mediatek.common.featureoption.FeatureOption;


/**
 * The class is defined for easily use options.
 */
public final class OptionsUtils {
    private static final String TAG = "OptionsManager";

    /**
     * Get the option value of FeatureOption.MTK_DRM_APP.
     * 
     * @return is MtkDrmApp, or not.
     */
    public static boolean isMtkDrmApp() {
      //  LogUtils.d(TAG, "FeatureOption.MTK_DRM_APP: " + FeatureOption.MTK_DRM_APP);
        return FeatureOption.MTK_DRM_APP;
    }
/*
    public static boolean isMtkThemeSupported() {
        //LogUtils.d(TAG, "FeatureOption.MTK_THEMEMANAGER_APP: " + FeatureOption.MTK_THEMEMANAGER_APP);
        return FeatureOption.MTK_THEMEMANAGER_APP;
    }
*/

    public static boolean isMtkBeamSurpported() {
        //LogUtils.d(TAG, "FeatureOption.MTK_BEAM_PLUS_SUPPORT: " + FeatureOption.MTK_BEAM_PLUS_SUPPORT);
        return FeatureOption.MTK_BEAM_PLUS_SUPPORT;
    }

    public static boolean isMtkSDSwapSurpported() {
        return FeatureOption.MTK_2SDCARD_SWAP;
    }

    public static boolean isMtkHotKnotSupported() {
        return FeatureOption.MTK_HOTKNOT_SUPPORT;
    }
    public static boolean isOP01Surported() {
        return "OP01".equals(SystemProperties.get("ro.operator.optr"));
    }
}
