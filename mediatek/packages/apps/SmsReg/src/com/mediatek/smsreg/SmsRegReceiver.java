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

package com.mediatek.smsreg;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings.System;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.xlog.Xlog;

public class SmsRegReceiver extends BroadcastReceiver {
    private static final String TAG = "SmsReg/Receiver";
    private ConfigInfoGenerator mXmlG = XMLGenerator
            .getInstance(SmsRegConst.getConfigPath());;

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        String intentAction = null;
        intentAction = intent.getAction();
        Xlog.w(TAG, "The intent is " + intentAction);
        try {
            IBinder binder = ServiceManager.getService("DmAgent");
            DmAgent agent = DmAgent.Stub.asInterface(binder);
            byte[] switchValue = agent.getSmsRegSwitchValue();
            if (switchValue != null && (new String(switchValue)).equals("1")) {
                Log.d(TAG, "There is a pending SmsReg flag.");
                agent.writeImsi("0".getBytes());
                agent.setSmsRegSwitchValue("0".getBytes());
                Log.d(TAG, "IMSI cleared.");
            } else {
                Log.d(TAG, "There is no pending SmsReg flag.");
            }
        } catch (RemoteException e) {
            throw new Error(e);
        }
        if (intentAction.equals(SmsRegConst.ACTION_BOOTCOMPLETED)) {
            // if sms register is enabled in engineer mode
            
            if (!enableAutoBoot(context)) {
                Log.d(TAG, "Disable Auto boot, Stop SmsReg.");
                return ;
            }
            if (enableSmsReg()) {
                // if the phone is customized, start smsRegService
                if (mXmlG == null) {
                    mXmlG = XMLGenerator.getInstance(SmsRegConst.getConfigPath());
                }
                if (mXmlG != null) {
                    Boolean isCustomized = mXmlG.getCustomizedStatus();
                    if (isCustomized) {
                        Intent bootCompletedIntent = new Intent();
                        bootCompletedIntent.setAction("BOOTCOMPLETED");
                        bootCompletedIntent.setClass(context,
                                SmsRegService.class);
                        context.startService(bootCompletedIntent);
                    } else {
                        Xlog.w(TAG, "The phone is not a customized phone ");
                    }
                }
            } else {
                Xlog.w(TAG, "Sms register is disabled by engineer mode !");
            }
        }
        if (intentAction.equals(SmsRegConst.RETRY_SEND_SMSREG)) {
            Intent retrySendSmsregIntent = new Intent();
            retrySendSmsregIntent.setAction("RETRY_SEND_SMSREG");
            retrySendSmsregIntent.setClass(context,
                    SmsRegService.class);
            context.startService(retrySendSmsregIntent);
        }
        if (intentAction.equals(SmsRegConst.ACTION_PREPARE_CONFIRM_DIALOG)) {
            Intent preConfirmIntent = new Intent();
            preConfirmIntent.setAction(SmsRegConst.ACTION_PREPARE_CONFIRM_DIALOG);
            preConfirmIntent.setClass(context,
                    SmsRegService.class);
            context.startService(preConfirmIntent);
        }
        if (intentAction.equals(SmsRegConst.ACTION_CONFIRM_DIALOG_END)) {

            intent.setClass(context,SmsRegService.class);
            context.startService(intent);
        }
    }

    public boolean enableSmsReg() {
        InfoPersistentor mInfoPersistentor = new InfoPersistentor();
        return mInfoPersistentor.getSavedCTA() == 1;
    }
    
    public boolean enableAutoBoot(Context context) {
        //DEF_DM_BOOT_START_ENABLE_VALUE = 1;
        int value = System.getInt(context.getContentResolver(), SmsRegConst.DM_BOOT_START_ENABLE_KEY, SmsRegConst.DEF_DM_BOOT_START_ENABLE_VALUE);
        Xlog.w(TAG, "MDM AutoBoot switch is  " + value);
        if (value == 1) {
            return true;
        } else {
            return false;
        }
    }
}
