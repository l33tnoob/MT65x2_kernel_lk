/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.apst.target.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;

import com.mediatek.apst.target.ftp.FtpService;
import com.mediatek.apst.target.service.MainService;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.SharedPrefs;

/**
 * Class Name: DaemonReceiver
 * <p>
 * Package: com.mediatek.apst.target.receiver
 * <p>
 * Created on: 2010-6-30
 * <p>
 * <p>
 * Description:
 * <p>
 * This receiver is activated even the main service is not started, thus, it
 * should be used to do task when main service is not running. Typically it's
 * used to start the main service, or react to some system events without
 * starting main service.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public class DaemonReceiver extends BroadcastReceiver {
    public static final String ACTION_START_MAIN_SERVICE = "com.mediatek.apst.target.action.START_MAIN_SERVICE";

    // public static final String
    // SERVICE_NAME="com.mediatek.apst.target.service.MainService";

    @Override
    public void onReceive(Context context, Intent intent) {
        String strAction = intent.getAction();

        if (null == strAction) {
            Debugger.logW(new Object[] { context, intent },
                    "intent.getAction() returns null.");
        } else if (ACTION_START_MAIN_SERVICE.equals(strAction)) {

            Intent newIntent = new Intent(context, MainService.class);
            context.startService(newIntent);
            Intent intentFtp = new Intent(context, FtpService.class);
            context.startService(intentFtp);
            Debugger.logI(new Object[] { context, intent },
                    "Starting APST Daemon, version " + Config.VERSION_STRING);

            // Intent newIntent = new Intent(context, MainService.class);
            // long startTime = System.currentTimeMillis();
            // if (isServiceRunning(context, SERVICE_NAME)) {
            // Debugger.logE(new Object[] { context },
            // "MainService is running");
            // context.stopService(newIntent);
            // } else {
            // context.startService(newIntent);
            // Debugger.logI(new Object[] { context },
            // "MainService isnot running");
            // Debugger.logI(new Object[] { context, intent },
            // "Starting APST Daemon, version "
            // + Config.VERSION_STRING);
            // }
            // long endTime = System.currentTimeMillis();
            // Debugger.logE(new Object[] { context }, "Speed time :"
            // + (endTime - startTime));

        } else if (Intent.ACTION_PACKAGE_DATA_CLEARED.equals(strAction)) {
            int uid = intent.getIntExtra(Intent.EXTRA_UID, -1);
            try {
                int contactsAppUid = context.getPackageManager()
                        .getPackageInfo("com.android.contacts", 0).applicationInfo.uid;
                if (contactsAppUid == uid) {
                    Debugger.logW(new Object[] { context, intent },
                            "Contacts package data cleared!");
                    // Reset sync history. We need slow sync next time we do
                    // outlook sync
                    SharedPrefs.open(context).edit().putBoolean(
                            SharedPrefs.SYNC_NEED_REINIT, true).commit();
                }
            } catch (NameNotFoundException e) {
                Debugger.logE(new Object[] { context, intent }, null, e);
            } catch (NullPointerException e) {
                Debugger.logE(new Object[] { context, intent }, null, e);
            }
        }
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================

    // /**
    // * Judge service is running ?
    // *
    // * @param context
    // * @param className
    // *
    // * @return isRunning
    // */
    // public static boolean isServiceRunning(Context mContext, String
    // serviceName) {
    // boolean isRunning = false;
    // ActivityManager activityManager = (ActivityManager) mContext
    // .getSystemService(Context.ACTIVITY_SERVICE);
    // List<ActivityManager.RunningServiceInfo> serviceList = activityManager
    // .getRunningServices(30);
    // if (serviceList.size() <= 0) {
    // return false;
    // }
    // for (int i = 0; i < serviceList.size(); i++) {
    // Debugger.logE("serviceList.get(i).service.getClassName()::"
    // + serviceList.get(i).service.getClassName());
    // if (serviceList.get(i).service.getClassName().equals(serviceName) ==
    // true) {
    // isRunning = true;
    // break;
    // }
    // }
    // return isRunning;
    // }
}
