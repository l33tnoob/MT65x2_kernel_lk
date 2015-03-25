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

package com.android.stk2;

import android.content.ComponentName;
import android.content.Context;
import android.content.pm.PackageManager;

import com.android.internal.telephony.cat.CatLog;

/**
 * Application installer for SIM Toolkit.
 */
class StkAppInstaller {
    Context mContext;
    private static StkAppInstaller mInstance = new StkAppInstaller();

    private StkAppInstaller() {
    }

    public static StkAppInstaller getInstance() {
        return mInstance;
    }

    public static final int STK_NOT_INSTALLED = 0;
    public static final int STK_INSTALLED = 1;

    private static int miSTKInstalled = -1; // 0 -not_ready, 1-ready

    void install(Context context) {
        mContext = context;
        new Thread(installThread).start();
    }

    void unInstall(Context context) {
        mContext = context;
        new Thread(uninstallThread).start();
    }

    private static void setAppState(Context context, boolean install) {
        CatLog.d("StkAppInstaller", "Stk2 - [setAppState]+");
        if (context == null) {
            return;
        }
        PackageManager pm = context.getPackageManager();
        if (pm == null) {
            return;
        }
        // check that STK app package is known to the PackageManager
        ComponentName cName = new ComponentName("com.android.stk2",
                "com.android.stk2.StkLauncherActivity");
        ComponentName cNameMenu = new ComponentName("com.android.stk2",
                "com.android.stk2.StkMenuActivity");
        int state = install ? PackageManager.COMPONENT_ENABLED_STATE_ENABLED
                : PackageManager.COMPONENT_ENABLED_STATE_DISABLED;

        CatLog.d("StkAppInstaller", "Stk2 - setAppState - curState[" + miSTKInstalled
                + "] to state[" + install + "]");

        if (((PackageManager.COMPONENT_ENABLED_STATE_ENABLED == state) && (STK_INSTALLED == miSTKInstalled))
                ||
                ((PackageManager.COMPONENT_ENABLED_STATE_DISABLED == state) && (STK_NOT_INSTALLED == miSTKInstalled))) {
            CatLog.d("StkAppInstaller", "Stk2 - Need not change app state!!");
        } else {
            CatLog.d("StkAppInstaller", "Stk2 - StkAppInstaller - Change app state[" + install
                    + "]");

            miSTKInstalled = install ? STK_INSTALLED : STK_NOT_INSTALLED;

            try {
                pm.setComponentEnabledSetting(cName, state, PackageManager.DONT_KILL_APP);
                // pm.setComponentEnabledSetting(cNameMenu, state,
                // PackageManager.DONT_KILL_APP);
            } catch (Exception e) {
                CatLog.d("StkAppInstaller", "Could not change STK2 app state");
            }
        }
        CatLog.d("StkAppInstaller", "Stk2 - [setAppState]-");
    }

    private class InstallThread implements Runnable {
        @Override
        public void run() {
            setAppState(mContext, true);
        }
    }

    private class UnInstallThread implements Runnable {
        @Override
        public void run() {
            setAppState(mContext, false);
        }
    }

    private InstallThread installThread = new InstallThread();
    private UnInstallThread uninstallThread = new UnInstallThread();

    public static int getIsInstalled() {
        return miSTKInstalled;
    }

}
