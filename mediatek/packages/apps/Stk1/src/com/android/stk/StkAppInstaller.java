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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.stk;

import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;


import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageManager;

/**
 * Application installer for SIM Toolkit.
 *
 */
 class StkAppInstaller {
    Context mContext;
    private static StkAppInstaller mInstance = new StkAppInstaller();
    private StkAppInstaller() {
    }
    public static StkAppInstaller getInstance(){
        if (mInstance != null)
        {
            mInstance.initThread();
        }
        return mInstance;
    }
    private void initThread()
    {
        int i = 0;
        for (i = 0; i < StkAppService.STK_GEMINI_SIM_NUM; i++)
        {
            if (installThread[i] == null)
            {
                CatLog.d("StkAppInstaller", "Init thread");
                installThread[i] = new InstallThread();
                miSTKInstalled[i] = -1;
            }
            if (uninstallThread[i] == null) uninstallThread[i] = new UnInstallThread();
        }
    }
    
    public static final int STK_NOT_INSTALLED = 1;
    public static final int STK_INSTALLED = 2;

    //private static int miSTKInstalled = -1;  // 1 -not_ready, 2-ready
    private static int[] miSTKInstalled = new int[StkAppService.STK_GEMINI_SIM_NUM];  // 1 -not_ready, 2-ready
    /* TODO: Gemini+ */
    private static final String STK1_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivity";
    private static final String STK2_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityII";
    private static final String STK3_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityIII";
    private static final String STK4_LAUNCHER_ACTIVITY = "com.android.stk.StkLauncherActivityIV";

     void install(Context context, int sim_id) {
        if (installThread[sim_id] != null)
        {
            mContext = context;
            installThread[sim_id].setSim(sim_id);
            new Thread(installThread[sim_id]).start();
        }
    }

    void unInstall(Context context, int sim_id) {
        if (uninstallThread[sim_id] != null)
        {
            mContext = context;
            uninstallThread[sim_id].setSim(sim_id);
            new Thread(uninstallThread[sim_id]).start();
        }
    }


    private static void setAppState(Context context, boolean install, int sim_id) {
        CatLog.d("StkAppInstaller", "[setAppState]+");
        if (context == null) {
            return;
        }
        PackageManager pm = context.getPackageManager();
        if (pm == null) {
            return;
        }
        // check that STK app package is known to the PackageManager
        /* TODO: Gemini+ begin */
        String class_name = STK1_LAUNCHER_ACTIVITY;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                class_name = STK1_LAUNCHER_ACTIVITY;
                break;
            case PhoneConstants.GEMINI_SIM_2:
                class_name = STK2_LAUNCHER_ACTIVITY;
                break;
            case PhoneConstants.GEMINI_SIM_3:
                class_name = STK3_LAUNCHER_ACTIVITY;
                break;
            case PhoneConstants.GEMINI_SIM_4:
                class_name = STK4_LAUNCHER_ACTIVITY;
                break;
            default:
                CatLog.d("StkAppInstaller", "setAppState, ready to return because sim id " + sim_id +" is wrong.");
                return;
        }
        /* TODO: Gemini+ end */
        CatLog.d("StkAppInstaller", "setAppState, target class name: " + class_name);
        ComponentName cName = new ComponentName("com.android.stk", class_name);
        ComponentName cNameMenu = new ComponentName("com.android.stk",
        "com.android.stk.StkMenuActivity");
        int state = install ? PackageManager.COMPONENT_ENABLED_STATE_ENABLED
                : PackageManager.COMPONENT_ENABLED_STATE_DISABLED;

        CatLog.d("StkAppInstaller", "Stk1 - setAppState - curState[" + miSTKInstalled[sim_id] + "] to state[" + install + "]" );

        if (((PackageManager.COMPONENT_ENABLED_STATE_ENABLED == state) && (STK_INSTALLED == miSTKInstalled[sim_id])) || 
            ((PackageManager.COMPONENT_ENABLED_STATE_DISABLED == state) && (STK_NOT_INSTALLED == miSTKInstalled[sim_id])))
        {
            CatLog.d("StkAppInstaller", "Stk " + sim_id + " - Need not change app state!!");
        } else {
            CatLog.d("StkAppInstaller", "Stk " + sim_id + "- StkAppInstaller - Change app state[" + install + "]");

            miSTKInstalled[sim_id] = install ? STK_INSTALLED : STK_NOT_INSTALLED;

            try {
                pm.setComponentEnabledSetting(cName, state, PackageManager.DONT_KILL_APP);
                // pm.setComponentEnabledSetting(cNameMenu, state, PackageManager.DONT_KILL_APP);
            } catch (Exception e) {
                CatLog.d("StkAppInstaller", "Could not change STK1 app state");
            }
        }
        CatLog.d("StkAppInstaller", "[setAppState]-");
    }
    private class InstallThread implements Runnable{
        private int mSimId = -1;
        @Override
        public void run(){
            CatLog.d("StkAppInstaller", "InstallThread, run , sim id: " + mSimId);
            if (mSimId >= 0 && mSimId < StkAppService.STK_GEMINI_SIM_NUM)
                setAppState(mContext, true, mSimId);
        }

        public void setSim(int sim_id)
        {
            CatLog.d("StkAppInstaller", "InstallThread, sim id: " + sim_id);
            mSimId = sim_id;
        }
    }
    private class UnInstallThread implements Runnable{
        private int mSimId = -1;
        @Override
        public void run(){
            CatLog.d("StkAppInstaller", "UninstallThread, run , sim id: " + mSimId);
            if (mSimId >= 0 && mSimId < StkAppService.STK_GEMINI_SIM_NUM)
                setAppState(mContext, false, mSimId);
        }

        public void setSim(int sim_id)
        {
            CatLog.d("StkAppInstaller", "UninstallThread, sim id: " + sim_id);
            mSimId = sim_id;
        }
    }
    private InstallThread[] installThread = new InstallThread[StkAppService.STK_GEMINI_SIM_NUM];
    private UnInstallThread[] uninstallThread = new UnInstallThread[StkAppService.STK_GEMINI_SIM_NUM];

    public static int getIsInstalled(int sim_id) {
        CatLog.d("StkAppInstaller", "getIsInstalled, sim id: " + sim_id + ", install status: " + miSTKInstalled[sim_id]);
        return miSTKInstalled[sim_id];
    }
}
