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

package com.mediatek.ygps;

import android.os.SystemProperties;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

/**
 * GPS MNL flag setting
 * 
 * @author mtk54046
 * @version 1.0
 */
public class GpsMnlSetting {

    private static final String TAG = "YGPS/Mnl_Setting";
    public static final String PROP_VALUE_0 = "0";
    public static final String PROP_VALUE_1 = "1";
    public static final String PROP_VALUE_2 = "2";

    public static final String KEY_DEBUG_DBG2SOCKET = "debug.dbg2socket";
    public static final String KEY_DEBUG_NMEA2SOCKET = "debug.nmea2socket";
    public static final String KEY_DEBUG_DBG2FILE = "debug.dbg2file";
    public static final String KEY_DEBUG_DEBUG_NMEA = "debug.debug_nmea";
    public static final String KEY_BEE_ENABLED = "BEE_enabled";
    public static final String KEY_TEST_MODE = "test.mode";
    public static final String KEY_SUPLLOG_ENABLED = "SUPPLOG_enabled";

    private static final String MNL_PROP_NAME = "persist.radio.mnl.prop";
    private static final String GPS_CHIP_PROP = "gps.gps.version";

    private static final String DEFAULT_MNL_PROP = "0001100";
    private static ArrayList<String> sKeyList = null;
    private static final String GPS_CLOCK_PROP = "gps.clock.type";
    /**
     * Get gps chip version
     * 
     * @param defaultValue
     *            Default value
     * @return GPS chip version
     */
    public static String getChipVersion(String defaultValue) {
        String chipVersion = SystemProperties.get(GPS_CHIP_PROP);
        if (null == chipVersion || chipVersion.isEmpty()) {
            chipVersion = defaultValue;
        }
        return chipVersion;
    }

    /**
     * Get MNL system property
     * 
     * @param key
     *            The key of the property
     * @param defaultValue
     *            The default value of the property
     * @return The value of the property
     */
    public static String getMnlProp(String key, String defaultValue) {
        String result = defaultValue;
        String prop = SystemProperties.get(MNL_PROP_NAME);
        if (null == sKeyList) {
            initList();
        }
        int index = sKeyList.indexOf(key);
        Xlog.v(TAG, "getMnlProp: " + prop);
        if (null == prop || prop.isEmpty()
                || -1 == index || index >= prop.length()) {
            result = defaultValue;
        } else {
            char c = prop.charAt(index);
            result = String.valueOf(c);
        }
        Xlog.v(TAG, "getMnlProp result: " + result);
        return result;
    }

    /**
     * Set MNL system property
     * 
     * @param key
     *            The key of the property
     * @param value
     *            The value of the property
     */
    public static void setMnlProp(String key, String value) {
        Xlog.v(TAG, "setMnlProp: " + key + " " + value);
        String prop = SystemProperties.get(MNL_PROP_NAME);
        if (null == sKeyList) {
            initList();
        }
        int index = sKeyList.indexOf(key);
        if (index != -1) {
            if (null == prop || prop.isEmpty()) {
                prop = DEFAULT_MNL_PROP;
            }
            if (prop.length() > index) {
                char[] charArray = prop.toCharArray();
                charArray[index] = value.charAt(0);
                String newProp = String.valueOf(charArray);
                SystemProperties.set(MNL_PROP_NAME, newProp);
                Xlog.v(TAG, "setMnlProp newProp: " + newProp);
            }
        }
    }

    public static String getClockProp(String defaultValue){
        String clockType = SystemProperties.get(GPS_CLOCK_PROP);
        if (null == clockType || clockType.isEmpty()) {
            clockType = defaultValue;
        }
        return clockType;
    }
    
    private static void initList() {
        sKeyList = new ArrayList<String>();
        sKeyList.add(KEY_DEBUG_DBG2SOCKET);
        sKeyList.add(KEY_DEBUG_NMEA2SOCKET);
        sKeyList.add(KEY_DEBUG_DBG2FILE);
        sKeyList.add(KEY_DEBUG_DEBUG_NMEA);
        sKeyList.add(KEY_BEE_ENABLED);
        sKeyList.add(KEY_TEST_MODE);
        sKeyList.add(KEY_SUPLLOG_ENABLED);
    }
}
