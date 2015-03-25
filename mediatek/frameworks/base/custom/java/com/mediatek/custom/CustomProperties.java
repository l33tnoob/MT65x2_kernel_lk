/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.custom;

// import android.util.Log;
import java.io.StringWriter;
import java.io.IOException;
import dalvik.system.PathClassLoader;
import com.mediatek.common.featureoption.FeatureOption;

public class CustomProperties {
    public static final int PROP_MODULE_MAX = 32;
    public static final int PROP_NAME_MAX = 64;

    public static final String MODULE_BROWSER = "browser";
    public static final String MODULE_MMS     = "mms";
    public static final String MODULE_HTTP_STREAMING = "http_streaming";
    public static final String MODULE_RTSP_STREAMING = "rtsp_streaming";
    public static final String MODULE_CMMB    = "cmmb";
    public static final String MODULE_WLAN    = "wlan";
    public static final String MODULE_FMTRANSMITTER = "fmtransmitter";
    public static final String MODULE_BLUETOOTH = "bluetooth";
    public static final String MODULE_DM = "dm";
    public static final String MODULE_SYSTEM = "system";

    public static final String USER_AGENT = "UserAgent";
    public static final String UAPROF_URL = "UAProfileURL";
    public static final String RDS_VALUE  = "RDSValue";
    public static final String MANUFACTURER = "Manufacturer";
    public static final String MODEL = "Model";
    public static final String SSID = "SSID";
    public static final String HOST_NAME = "HostName";

    static ClassLoader mLoader;

    static {
    	  if (!FeatureOption.MTK_BSP_PACKAGE) {
            System.loadLibrary("custom_jni");
            mLoader = new PathClassLoader("/system/framework/CustomPropInterface.jar", ClassLoader.getSystemClassLoader());
        }
    }

    public static String getString(String name) {
        return getString(null, name, null);
    }

    public static String getString(String module, String name) {
        return getString(module, name, null);
    }

    public static String getString(String module, String name, String defaultValue) {
        if (FeatureOption.MTK_BSP_PACKAGE) {
          return null;
        }
        else {
            if ((module != null) && (module.length() > PROP_MODULE_MAX)) {
                throw new IllegalArgumentException("module.length >" + PROP_MODULE_MAX);
            }
            if ((name == null) || (name.length() > PROP_NAME_MAX)) {
                throw new IllegalArgumentException("name.length > " + PROP_NAME_MAX);
            }

            return native_get_string(module, name, defaultValue);
        }
    }

    private static Class loadInterface() {
        if (FeatureOption.MTK_BSP_PACKAGE) {
          return null;
        }
        else {
            Class clazz;        

            try {
                clazz = mLoader.loadClass("com.mediatek.custom.CustomPropInterface");
            } catch (ClassNotFoundException e) {
                clazz = null;
                e.printStackTrace();
            }

            System.out.println("[CustomProp]loadInterface->clazz:" + clazz);

            return clazz;
        }
    }

    private static native String native_get_string(String module, String name, String defaultValue);
}
