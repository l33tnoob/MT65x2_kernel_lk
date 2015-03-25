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

package com.android.simmelock;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult; //import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;//To find the SIM card Ready State
import android.view.KeyEvent;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.IccCard;
import com.mediatek.common.featureoption.FeatureOption;

public class PermanUnlockSetting extends SimLockBaseActivity {
    static int DIALOG_PERMANUNLOCK = 2;
    private static final int PERREMOVELOCK_ICC_SML_COMPLETE = 120;
    private static final int PERREMOVELOCK_ICC_SML_SHOW_DIALOG = 666;
    private static final int PERREMOVELOCK_ICC_SML_CANCEL = 777;

    private Handler mHandlerFinish = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case PERREMOVELOCK_ICC_SML_SHOW_DIALOG:
                showDialog(DIALOG_PERMANUNLOCK);
                break;
            case PERREMOVELOCK_ICC_SML_CANCEL:
                PermanUnlockSetting.this.finish();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.permanunlocksetting);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mHandlerFinish.sendEmptyMessage(PERREMOVELOCK_ICC_SML_SHOW_DIALOG);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.strAttention)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setMessage(R.string.srtConfirmPermanUnlock)
                .setOnKeyListener(this)
                .setPositiveButton(R.string.strConfirm, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        Message callback = Message.obtain(mHandler, PERREMOVELOCK_ICC_SML_COMPLETE);
                        // PermanUnlock a lock
                        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
                            Phone phone = PhoneFactory.getDefaultPhone();
                            phone.getIccCard().setIccNetworkLockEnabled(lockCategory, 4, null, null, null, null, callback);
                        } else {
                            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
                            geminiPhone.getPhonebyId(simNumber).getIccCard().setIccNetworkLockEnabled(
                                    lockCategory, 4, null, null, null, null, callback);
                        }
                        finish();
                    }
                }).setNegativeButton(R.string.strCancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        mHandlerFinish.sendEmptyMessage(PERREMOVELOCK_ICC_SML_CANCEL);
                    }
                });
        return builder.create();
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case PERREMOVELOCK_ICC_SML_COMPLETE:
                if (ar.exception != null) {
                    // Toast.makeText(PermanUnlockSetting.this,
                    // "PermanUnlock lock fail", Toast.LENGTH_LONG).show();
                } else {
                    // Toast.makeText(PermanUnlockSetting.this,
                    // "PermanUnlock lock succeed!", Toast.LENGTH_LONG).show();
                    PermanUnlockSetting.this.finish();
                }
                break;
            default:
                break;
            }
        }
    };
}
