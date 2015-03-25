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

package com.mediatek.oobe.utils;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.oobe.ext.DefaultOobeMiscExt;
import com.mediatek.oobe.ext.IOobeMiscExt;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.Plugin.ObjectCreationException;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.Comparator;

/**
 * Contains utility functions for getting framework resource
 */
public class Utils {
    public static final String TAG = "OOBE";
    public static final String OOBE_BASIC_STEP_TOTAL = "oobe_step_total";
    public static final String OOBE_BASIC_STEP_INDEX = "oobe_step_index";
    public static final String OOBE_HAS_RUN_KEY = "oobe_has_run";

    public static final int RESULT_CODE_NEXT = 20;
    public static final int RESULT_CODE_BACK = 21;
    public static final int RESULT_CODE_FINISH = 22;

    private static final int COLORNUM = 7;

    public static int getStatusResource(int state) {

        switch (state) {
        case PhoneConstants.SIM_INDICATOR_RADIOOFF:
            return com.mediatek.internal.R.drawable.sim_radio_off;
        case PhoneConstants.SIM_INDICATOR_LOCKED:
            return com.mediatek.internal.R.drawable.sim_locked;
        case PhoneConstants.SIM_INDICATOR_INVALID:
            return com.mediatek.internal.R.drawable.sim_invalid;
        case PhoneConstants.SIM_INDICATOR_SEARCHING:
            return com.mediatek.internal.R.drawable.sim_searching;
        case PhoneConstants.SIM_INDICATOR_ROAMING:
            return com.mediatek.internal.R.drawable.sim_roaming;
        case PhoneConstants.SIM_INDICATOR_CONNECTED:
            return com.mediatek.internal.R.drawable.sim_connected;
        case PhoneConstants.SIM_INDICATOR_ROAMINGCONNECTED:
            return com.mediatek.internal.R.drawable.sim_roaming_connected;
        default:
            return -1;
        }
    }

    public static int getSimColorResource(int color) {

        if ((color >= 0) && (color <= COLORNUM)) {
            return SimInfoManager.SimBackgroundDarkRes[color];
        } else {
            return -1;
        }

    }

    /**
     * if is Wifi Only
     * @param context Context
     * @return boolean
     */
    public static boolean isWifiOnly(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        return (!cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE));
    }

   /**
     * if voice support 
     * @param context Context
     * @return boolean
     */
    public static boolean isVoiceSupport(Context context) {
        TelephonyManager telephony = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        return (telephony.isVoiceCapable());
    }

    public static class SimInfoComparable implements Comparator<SimInfoRecord> {
        @Override
        public int compare(SimInfoRecord sim1, SimInfoRecord sim2) {
            return sim1.mSimSlotId - sim2.mSimSlotId;
        }
    }

    public static void startLauncher(Context context) {
        Intent i = new Intent(Intent.ACTION_MAIN);
        i.addCategory(Intent.CATEGORY_HOME);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(i);
    }

    /**
     * M: create oobe misc plugin object
     * @param context Context
     * @return IOobeMiscExt
     */
    public static IOobeMiscExt getOobeMiscPlugin(Context context) {
        IOobeMiscExt ext;
        try {
            ext = (IOobeMiscExt)PluginManager.createPluginObject(context,
                IOobeMiscExt.class.getName());
            Xlog.d(TAG , "IOobeMiscExt plugin object created");
        } catch (Plugin.ObjectCreationException e) {
            ext = new DefaultOobeMiscExt();    
            Xlog.d(TAG , "DefaultOobeMiscExt plugin object created, e = " + e);
        }
        return ext;
    }
}
