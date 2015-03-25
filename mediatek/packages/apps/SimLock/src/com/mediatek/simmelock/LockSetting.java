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
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.IccCard;
import com.mediatek.common.featureoption.FeatureOption;

public class LockSetting extends SimLockBaseActivity {
    private static final int LOCK_ICC_SML_COMPLETE = 120;
    final int DIALOG_LOCKFAIL = 3;
    final int DIALOG_PASSWORDLENGTHINCORRECT = 1;
    final int DIALOG_LOCKSUCCEED = 2;
    final int DIALOG_PASSWORDWRONG = 4;

    EditText et = null;
    EditText re_et = null;
    private String lockPassword = null;// the true password string which need to be compared with the input string
    private boolean clickFlag = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.locksetting);

        // set the regulation of EditText
        et = (EditText) findViewById(R.id.idEditInputPassword);
        et.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        et.setOnLongClickListener(mOnLongClickListener);
        SMLCommonProcess.limitEditTextPassword(et, 8);
        if (lockCategory == 0 || lockCategory == 1 || lockCategory == 2 || lockCategory == 3 || lockCategory == 4) {
            TextView t = (TextView) findViewById(R.id.idInputPasswordAgain);
            t.setVisibility(View.VISIBLE);
            re_et = (EditText) findViewById(R.id.idEditInputPasswordAgain);
            re_et.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
            SMLCommonProcess.limitEditTextPassword(re_et, 8);
            re_et.setVisibility(View.VISIBLE);
            re_et.setOnLongClickListener(mOnLongClickListener);
        }

        Button btnConfirm = (Button) findViewById(R.id.idButtonConfirm);
        // Yu for ICS
        btnConfirm.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                // whether some lock category is disabled?
                // make sure the password's length is correct
                // make sure the length of inputed password is 4 to 8
                Log.i("SIMMELOCK", "clickFlag: " + clickFlag);
                if (clickFlag) {
                    return;
                } else {
                    clickFlag = true;
                }
                if ((et.getText().length() < 4) || ((et.getText().length() > 8))) {
                    showDialog(DIALOG_PASSWORDLENGTHINCORRECT);
                    return;
                }
                if (lockCategory == 0 || lockCategory == 1 || lockCategory == 2 || lockCategory == 3 || lockCategory == 4) {
                    if ((et.getText().toString().equals(re_et.getText().toString())) == false) {
                        showDialog(DIALOG_PASSWORDWRONG);
                        return;
                    }
                }

                Message callback = Message.obtain(mHandler, LOCK_ICC_SML_COMPLETE);
                if (!FeatureOption.MTK_GEMINI_SUPPORT) {
                    Phone phone = PhoneFactory.getDefaultPhone();
                    phone.getIccCard().setIccNetworkLockEnabled(lockCategory, 1, et.getText().toString(), null, null, null,
                            callback);
                } else {
                    Log.i("SIMMELOCK", "[LockSetting]simNumber is" + simNumber);
                    GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
                    geminiPhone.getPhonebyId(simNumber).getIccCard().setIccNetworkLockEnabled(lockCategory, 1,
                            et.getText().toString(), null, null, null, callback);
                }
            }
        });

        Button btnCancel = (Button) findViewById(R.id.idButtonCancel);
        btnCancel.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                LockSetting.this.finish();
            }
        });
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_LOCKFAIL || id == DIALOG_PASSWORDLENGTHINCORRECT || id == DIALOG_LOCKSUCCEED
                || id == DIALOG_PASSWORDWRONG) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setCancelable(false)
                    .setTitle(R.string.strAttention)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setOnKeyListener(this);

            switch (id) {
            case DIALOG_LOCKFAIL: // Fail
                builder.setMessage(R.string.strLockFail).setNegativeButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                et.setText("");
                                if (re_et != null){
                                    re_et.setText("");
                                }                                      
                                clickFlag = false;
                                dialog.cancel();
                            }
                        });
                return builder.create();
            case DIALOG_PASSWORDLENGTHINCORRECT:// Length is incorrect
                builder.setMessage(R.string.strPasswordLengthIncorrect).setNegativeButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                et.setText("");
                                if (re_et != null) {
                                    re_et.setText("");
                                }
                                clickFlag = false;
                                dialog.cancel();
                            }
                        });
                return builder.create();
            case DIALOG_LOCKSUCCEED:// Succeed
                builder.setMessage(R.string.strLockSucceed).setPositiveButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                et.setText("");
                                if (re_et != null) {
                                    re_et.setText("");
                                }                                       
                                dialog.cancel();
                                clickFlag = false;
                                LockSetting.this.finish();
                            }
                        });
                return builder.create();
            case DIALOG_PASSWORDWRONG:// Wrong password
                builder.setMessage(R.string.str_simme_passwords_dont_match).setNegativeButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                et.setText("");
                                if (re_et != null) {
                                    re_et.setText("");
                                }                                           
                                dialog.cancel();
                                et.requestFocus();
                                clickFlag = false;
                            }
                        });
                return builder.create();
            }
        }
        return super.onCreateDialog(id);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case LOCK_ICC_SML_COMPLETE:
                if (ar.exception != null) {
                    showDialog(DIALOG_LOCKFAIL);
                } else {
                    showDialog(DIALOG_LOCKSUCCEED);
                }
                break;
            default:
                break;
            }
        }
    };
}
