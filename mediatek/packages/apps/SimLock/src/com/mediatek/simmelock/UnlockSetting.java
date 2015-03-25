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
import android.content.DialogInterface.OnDismissListener;
import android.content.res.Configuration;
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

import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.common.featureoption.FeatureOption;

public class UnlockSetting extends SimLockBaseActivity {
    static final int DIALOG_PASSWORDLENGTHINCORRECT = 1;
    static final int DIALOG_UNLOCKSUCCEED = 2;
    static final int DIALOG_UNLOCKFAILED = 3;
    static final int DIALOG_QUERYFAIL = 4;
    private static final int UNLOCK_ICC_SML_COMPLETE = 120;
    private static final int UNLOCK_ICC_SML_QUERYLEFTTIMES = 110;

    EditText etPwd = null;
    TextView etPwdLeftChances = null;
    private int mPwdLeftChances = 5;
    private boolean clickFlag = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.unlocksetting);

        Bundle bundle = this.getIntent().getExtras();
        String localName = bundle.getString("LOCALNAME");
        Configuration conf = getResources().getConfiguration();
        String locale = conf.locale.getDisplayName(conf.locale);
        Log.i("SIMMELOCK", "localName: " + localName + "    || getLocalClassName: " + locale);
        if (localName != null && !localName.equals(locale)) {
            finish();
            return;
        }

        // initial left password input chances
        etPwdLeftChances = (TextView) findViewById(R.id.idunlockEditInputChancesleft);

        // set the regulation of EditText
        etPwd = (EditText) findViewById(R.id.idunlockEditInputPassword);
        etPwd.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        SMLCommonProcess.limitEditTextPassword(etPwd, 8);
        etPwd.setOnLongClickListener(mOnLongClickListener);

        Button btnConfirm = (Button) findViewById(R.id.idunlockButtonConfirm);
        // Yu for ICS
        btnConfirm.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                Log.i("SIMMELOCK", "clickFlag: " + clickFlag);
                if (clickFlag) {
                    return;
                } else {
                    clickFlag = true;
                }

                if ((etPwd.getText().length() < 4) || ((etPwd.getText().length() > 8))) {
                    showDialog(DIALOG_PASSWORDLENGTHINCORRECT);
                } else {
                    // get the left chances to unlock(less than 5)
                    Message callback = Message.obtain(mHandler, UNLOCK_ICC_SML_COMPLETE);
                    if (!FeatureOption.MTK_GEMINI_SUPPORT) {
                        Phone phone = PhoneFactory.getDefaultPhone();
                        phone.getIccCard().setIccNetworkLockEnabled(lockCategory, 0, etPwd.getText().toString(), null, null,
                                null, callback);
                    } else {
                        Log.i("SIMMELOCK", "[btnconfirm]simNumber: " + simNumber);
                        GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
                        geminiPhone.getPhonebyId(simNumber).getIccCard().setIccNetworkLockEnabled(lockCategory, 0,
                                etPwd.getText().toString(), null, null, null, callback);
                    }
                }
            }
        });

        Button btnCancel = (Button) findViewById(R.id.idunlockButtonCancel);
        btnCancel.setOnClickListener(new Button.OnClickListener() {
            public void onClick(View view) {
                UnlockSetting.this.finish();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        queryLeftTimes();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_UNLOCKFAILED || id == DIALOG_PASSWORDLENGTHINCORRECT || id == DIALOG_UNLOCKSUCCEED
                || id == DIALOG_QUERYFAIL) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.strAttention)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setOnKeyListener(this);

            switch (id) {
            case DIALOG_UNLOCKFAILED: // Fail
                Log.i("SIMMELOCK", "show DIALOG_UNLOCKFAILED");
                builder.setMessage(R.string.strUnlockFail).setPositiveButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                Log.i("SIMMELOCK", "query mPwdLeftChances: " + mPwdLeftChances);
                                etPwd.setText("");
                                dialog.cancel();
                                clickFlag = false;
                            }
                        });
                return builder.create();
            case DIALOG_PASSWORDLENGTHINCORRECT:// Length is incorrect
                Log.i("SIMMELOCK", "show DIALOG_PASSWORDLENGTHINCORRECT");
                builder.setMessage(R.string.strPasswordLengthIncorrect).setPositiveButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                etPwd.setText("");
                                dialog.cancel();
                                clickFlag = false;
                            }
                        });
                return builder.create();
            case DIALOG_UNLOCKSUCCEED:// Succeed
                Log.i("SIMMELOCK", "show DIALOG_UNLOCKSUCCEED");
                builder.setMessage(R.string.strUnlockSucceed).setPositiveButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                if (id == AlertDialog.BUTTON_POSITIVE) {
                                    Log.i("SIMMELOCK", "Success dialog UnlockSetting dismissed.");
                                    clickFlag = false;
                                    try {
                                        if (null != dialog) {
                                            dialog.cancel();
                                        }
                                    } catch (IllegalArgumentException e) {
                                        Log.e("SIMMELOCK", "Catch IllegalArgumentException");
                                    }
                                    finish();
                                    Log.i("SIMMELOCK", "Success dialog dismissed.");
                                }
                            }
                        });
                return builder.create();
            case DIALOG_QUERYFAIL:// Query fail
                Log.i("SIMMELOCK", "show DIALOG_QUERYFAIL");
                builder.setMessage(R.string.strQueryFailed).setPositiveButton(R.string.strYes,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                etPwd.setText("");
                                dialog.cancel();
                                clickFlag = false;
                            }
                        });
                return builder.create();
            }
        }
        Log.i("SIMMELOCK", "show null");
        return super.onCreateDialog(id);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case UNLOCK_ICC_SML_COMPLETE: {
                if (clickFlag == true) {
                    Log.i("SIMMELOCK", "set ar: " + ar.exception);
                    if (ar.exception != null) {  // fail to unlock
                        queryLeftTimes();
                        showDialog(DIALOG_UNLOCKFAILED);
                    } else {
                        showDialog(DIALOG_UNLOCKSUCCEED);
                    }
                    Log.i("SIMMELOCK", "handler UNLOCK_ICC_SML_COMPLETE mPwdLeftChances: " + mPwdLeftChances);
                }
                break;
            }

            case UNLOCK_ICC_SML_QUERYLEFTTIMES: {
                Log.i("SIMMELOCK", "handler query");
                Log.i("SIMMELOCK", "query ar: " + ar.exception);
                if (ar.exception != null) {
                    showDialog(DIALOG_QUERYFAIL);// Query fail!
                } else {
                    AsyncResult ar1 = (AsyncResult) msg.obj;
                    int[] LockState = (int[]) ar1.result;
                    if (LockState[2] > 0) {
                        // still have chances to unlock
                        mPwdLeftChances = LockState[2];
                        etPwdLeftChances.setText(String.valueOf(mPwdLeftChances));
                        Log.i("SIMMELOCK", "query mPwdLeftChances: " + mPwdLeftChances);
                    } else {
                        UnlockSetting.this.finish();
                    }
                }
                break;
            }

            }
        }
    };

    private void queryLeftTimes() {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            Message callback_query = Message.obtain(mHandler, UNLOCK_ICC_SML_QUERYLEFTTIMES);
            Phone phone = PhoneFactory.getDefaultPhone();
            phone.getIccCard().QueryIccNetworkLock(lockCategory, 0, null, null, null, null, callback_query);
        } else {
            Message callback_query = Message.obtain(mHandler, UNLOCK_ICC_SML_QUERYLEFTTIMES);
            GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.getPhonebyId(simNumber).getIccCard().QueryIccNetworkLock(lockCategory, 0, null,
                    null, null, null, callback_query);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        Log.i("SIMMELOCK", "[UnlckSetting]onConfigurationChanged ");
        super.onConfigurationChanged(newConfig);
    }
}
