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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cat.CatLog;

/**
 * Boot completed receiver. used to reset the app install state every time the
 * device boots.
 */
public class BootCompletedReceiver extends BroadcastReceiver {
    private static final int GEMINI_SIM_2 = 1;

    private static final String LOGTAG = "Stk2-BCR ";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        StkAppInstaller appInstaller = StkAppInstaller.getInstance();
        StkAppService appService = StkAppService.getInstance();

        // make sure the app icon is removed every time the device boots.
        if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            Bundle args = new Bundle();
            args.putInt(StkAppService.OPCODE, StkAppService.OP_BOOT_COMPLETED);
            context.startService(new Intent(context, StkAppService.class)
                    .putExtras(args));
        } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
            CatLog.d(LOGTAG, "get ACTION_SIM_STATE_CHANGED");

            int SIMID = intent.getIntExtra(Phone.GEMINI_SIM_ID_KEY, -1);
            String SIMStatus = intent.getStringExtra(IccCard.INTENT_KEY_ICC_STATE);
            CatLog.d(LOGTAG, "[ACTION_SIM_STATE_CHANGED][simId] : " + SIMID);
            CatLog.d(LOGTAG, "[ACTION_SIM_STATE_CHANGED][SimStatus] : " + SIMStatus);
            if (SIMID != GEMINI_SIM_2) {

            } else {
                // deal with SIM1
                CatLog.d(LOGTAG, "[ACTION_SIM_STATE_CHANGED][GEMINI_SIM_1]");
                Bundle bundle = new Bundle();
                bundle.putString("affinity", "com.android.stk");
                final Intent it = new Intent();
                it.putExtras(bundle);

                CatLog.d(LOGTAG, "isSetupMenuCalled[" + StkAppService.isSetupMenuCalled() + "]");

                boolean bUnInstall = true;
                if ((StkAppService.isSetupMenuCalled())
                        && (((IccCard.INTENT_VALUE_ICC_READY).equals(SIMStatus))
                                || ((IccCard.INTENT_VALUE_ICC_IMSI).equals(SIMStatus)) || ((IccCard.INTENT_VALUE_ICC_LOADED)
                                .equals(SIMStatus)))) {
                    bUnInstall = false;
                }
                CatLog.d(LOGTAG, "[ACTION_SIM_STATE_CHANGED][bUnInstall] : " + bUnInstall);
                int miSTKInstalled = appInstaller.getIsInstalled();
                if (bUnInstall && (miSTKInstalled == 1)) {
                    CatLog.d(LOGTAG, "ADD_RECENET_IGNORE");
                    it.setAction("android.intent.action.ADD_RECENET_IGNORE");
                    context.sendBroadcast(it);

                    CatLog.d(LOGTAG, "get ACTION_SIM_STATE_CHANGED - unInstall");
                    appInstaller.unInstall(context);
                    if (appService != null) {
                        appService.setUserAccessState(false);
                    }
                } else if ((!bUnInstall) && (miSTKInstalled == 0)) {
                    CatLog.d(LOGTAG, "REMOVE_RECENET_IGNORE");
                    it.setAction("android.intent.action.REMOVE_RECENET_IGNORE");
                    context.sendBroadcast(it);

                    CatLog.d(LOGTAG, "get ACTION_SIM_STATE_CHANGED - install");
                    appInstaller.install(context);
                } else if((miSTKInstalled == -1) && 
                    ((IccCard.INTENT_VALUE_ICC_ABSENT).equals(SIMStatus) 
                        || (IccCard.INTENT_VALUE_ICC_NOT_READY).equals(SIMStatus))) {
                    CatLog.d(LOGTAG, " miSTKInstalled == -1");
                    it.setAction("android.intent.action.ADD_RECENET_IGNORE");
                    context.sendBroadcast(it);

                    CatLog.d(LOGTAG, "get ACTION_SIM_STATE_CHANGED - unInstall: -1");
                    appInstaller.unInstall(context);
                    if (appService != null) {
                        appService.setUserAccessState(false);
                    }        	
                }
            }
            CatLog.d(LOGTAG, "get ACTION_SIM_STATE_CHANGED  finish");
        }
    }
}
