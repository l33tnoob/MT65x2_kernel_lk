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

/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.engineermode.ratmode;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyExt;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.bandselect.BandSelect;
import com.mediatek.xlog.Xlog;

public class RadioInfo extends Activity {
    private static final String TAG = "EM/RATMode_RadioInfo";
    private static final int EVENT_QUERY_PREFERRED_TYPE_DONE = 1000;
    private static final int EVENT_SET_PREFERRED_TYPE_DONE = 1001;
    private static final int MODEM_MASK_WCDMA = 0x04;
    private static final int MODEM_MASK_TDSCDMA = 0x08;
    private static final int GSM_WCDMA_AUTO_INDEX = 0;
    private static final int WCDMA_PREFERRED_INDEX = 1;
    private static final int LTE_ONLY_INDEX = 2;
    private static final int WCDMA_ONLY_INDEX = 3;
    private static final int GSM_ONLY_INDEX = 4;
    private static final int LTE_GSM_WCDMA_INDEX = 5;
    private static final int LTE_WCDMA_INDEX = 6;
    private static final int LTE_GSM_INDEX = 7;
    private static final int NOT_SPECIFIED_INDEX = 8;
    private static final int GSM_WCDMA_AUTO_VALUE = 3;
    private static final int WCDMA_PREFERRED_VALUE = 0;
    private static final int LTE_ONLY_VALUE = 11;
    private static final int WCDMA_ONLY_VALUE = 2;
    private static final int GSM_ONLY_VALUE = 1;
    private static final int LTE_GSM_WCDMA_VALUE = 9;
    private static final int LTE_WCDMA_VALUE = 12;
    private static final int LTE_GSM_VALUE = 34;
    private static final int NOT_SPECIFIED_VALUE = 8;

    private Spinner mPreferredNetworkType = null;
    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    private IntentFilter mFilter = null;
    private BroadcastReceiver mReceiver = null;
    private int mStatusEfRat = IccRecords.EF_RAT_FOR_OTHER_CASE;
    private final int mSimId = -1;
    private int mModeType = -1;
    private boolean mIsTddType = false;
    private ITelephonyExt mTelephonyExt = null;
    private int mCurrentSelected = -1;

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case EVENT_QUERY_PREFERRED_TYPE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int pos = NOT_SPECIFIED_INDEX;
                    int type = ((int[]) ar.result)[0];
                    Xlog.d(TAG, "Get Preferred Type " + type);
                    switch (type) {
                    case WCDMA_PREFERRED_VALUE:
                        pos = WCDMA_PREFERRED_INDEX;
                        break;
                    case GSM_ONLY_VALUE:
                        pos = GSM_ONLY_INDEX;
                        break;
                    case WCDMA_ONLY_VALUE:
                        pos = WCDMA_ONLY_INDEX;
                        break;
                    case GSM_WCDMA_AUTO_VALUE:
                        pos = GSM_WCDMA_AUTO_INDEX;
                        break;
                    case LTE_ONLY_VALUE:
                        pos = LTE_ONLY_INDEX;
                        break;
                    case LTE_GSM_WCDMA_VALUE:
                        pos = LTE_GSM_WCDMA_INDEX;
                        break;
                    case LTE_WCDMA_VALUE:
                        pos = LTE_WCDMA_INDEX;
                        break;
                    case LTE_GSM_VALUE:
                        pos = LTE_GSM_INDEX;
                        break;
                    default:
                        return;
                    }
                    mCurrentSelected = pos;
                    mPreferredNetworkType.setSelection(pos, true);
                } else {
                    Toast.makeText(RadioInfo.this,
                            R.string.rat_query_preferred_type, Toast.LENGTH_SHORT).show();
                }
                break;
            case EVENT_SET_PREFERRED_TYPE_DONE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_1).getPreferredNetworkType(
                                obtainMessage(EVENT_QUERY_PREFERRED_TYPE_DONE));
                    } else {
                        mPhone.getPreferredNetworkType(
                                obtainMessage(EVENT_QUERY_PREFERRED_TYPE_DONE));
                    }
                }
                break;
            default:
                break;
            }
        }
    };
    
    AdapterView.OnItemSelectedListener mPreferredNetworkHandler = new AdapterView.OnItemSelectedListener() {
        public void onItemSelected(AdapterView parent, View v, int pos, long id) {
            Xlog.d(TAG, "current " + mCurrentSelected + ", pos: " + pos);
            if (mCurrentSelected == pos) {
                return;
            }
            mCurrentSelected = pos;

            Message msg = mHandler.obtainMessage(EVENT_SET_PREFERRED_TYPE_DONE);

            switch (pos) {
            case GSM_WCDMA_AUTO_INDEX:
                if (mModeType == -1) {
                    mPreferredNetworkType.setSelection(GSM_ONLY_INDEX, true);
                    Toast.makeText(RadioInfo.this, R.string.rat_3g_status, Toast.LENGTH_LONG).show();
                } else {
                    Xlog.d(TAG, "GSM/WCDMA(auto) " + 3);
                    writePreferred(GSM_WCDMA_AUTO_VALUE);
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        mGeminiPhone.setPreferredNetworkTypeGemini(GSM_WCDMA_AUTO_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                    } else {
                        mPhone.setPreferredNetworkType(GSM_WCDMA_AUTO_VALUE, msg);
                    }
                }
                break;
            case WCDMA_PREFERRED_INDEX:
                Xlog.d(TAG, "WCDMA Preferred " + 0);
                if (FeatureOption.MTK_RAT_WCDMA_PREFERRED) {
                    if (mModeType == -1) {
                            mPreferredNetworkType.setSelection(GSM_ONLY_INDEX,
                                    true);
                            Toast.makeText(RadioInfo.this,
                                    R.string.rat_3g_status, Toast.LENGTH_LONG)
                                    .show();
                        } else {
                            writePreferred(WCDMA_PREFERRED_VALUE);
                            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                                mGeminiPhone.setPreferredNetworkTypeGemini(
                                        WCDMA_PREFERRED_VALUE, msg,
                                        PhoneConstants.GEMINI_SIM_1);
                            } else {
                                mPhone.setPreferredNetworkType(
                                        WCDMA_PREFERRED_VALUE, msg);
                            }
                        }
                } else {
                    if (mIsTddType) {
                        Toast.makeText(RadioInfo.this, R.string.rat_tdcdmapreferred_support, Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(RadioInfo.this, R.string.rat_wcdmapreferred_support, Toast.LENGTH_LONG).show();
                    }
                }
                break;
            case WCDMA_ONLY_INDEX:
                if (mModeType == -1) {
                    mPreferredNetworkType.setSelection(GSM_ONLY_INDEX, true);
                    Toast.makeText(RadioInfo.this, R.string.rat_3g_status, Toast.LENGTH_LONG).show();
                } else {
                    Xlog.d(TAG, "WCDMA only " + 2);
                    writePreferred(WCDMA_ONLY_VALUE);
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        mGeminiPhone.setPreferredNetworkTypeGemini(WCDMA_ONLY_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                    } else {
                        mPhone.setPreferredNetworkType(WCDMA_ONLY_VALUE, msg);
                    }
                }
                break;
            case GSM_ONLY_INDEX:
                Xlog.d(TAG, "GSM only " + 1);
                writePreferred(GSM_ONLY_VALUE);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(GSM_ONLY_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.setPreferredNetworkType(GSM_ONLY_VALUE, msg);
                }
                break;
            case LTE_ONLY_INDEX:
                Xlog.d(TAG, "LTE only " + 11);
                writePreferred(LTE_ONLY_VALUE);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(LTE_ONLY_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.setPreferredNetworkType(LTE_ONLY_VALUE, msg);
                }
                break;
            case LTE_GSM_WCDMA_INDEX:
                Xlog.d(TAG, "LTE/GSM/WCDMA " + 9);
                writePreferred(LTE_GSM_WCDMA_VALUE);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(LTE_GSM_WCDMA_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.setPreferredNetworkType(LTE_GSM_WCDMA_VALUE, msg);
                }
                break;
            case LTE_WCDMA_INDEX:
                Xlog.d(TAG, "LTE/WCDMA " + 12);
                writePreferred(LTE_WCDMA_VALUE);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(LTE_WCDMA_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.setPreferredNetworkType(LTE_WCDMA_VALUE, msg);
                }
                break;
            case LTE_GSM_INDEX:
                Xlog.d(TAG, "LTE/GSM " + 34);
                writePreferred(LTE_GSM_VALUE);
                if (FeatureOption.MTK_GEMINI_SUPPORT) {
                    mGeminiPhone.setPreferredNetworkTypeGemini(LTE_GSM_VALUE, msg, PhoneConstants.GEMINI_SIM_1);
                } else {
                    mPhone.setPreferredNetworkType(LTE_GSM_VALUE, msg);
                }
                break;
            case NOT_SPECIFIED_INDEX:
                writePreferred(NOT_SPECIFIED_VALUE);
                break;
            default:
                break;
            }
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
            Elog.i(TAG, "onNothingSelected");
        }
        
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.radio_info);
        mIsTddType = (ModemCategory.getModemType() == ModemCategory.MODEM_TD);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
        } else {
            mPhone = PhoneFactory.getDefaultPhone();
        }
        mPreferredNetworkType = (Spinner) findViewById(R.id.preferredNetworkType);
        if (mIsTddType) {            
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item,
                    getResources().getStringArray(R.array.mTddPreferredNetworkLabels));
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mPreferredNetworkType.setAdapter(adapter);
        } else {
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item,
                     getResources().getStringArray(R.array.mPreferredNetworkLabels));
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mPreferredNetworkType.setAdapter(adapter);
        }
        mPreferredNetworkType
                .setOnItemSelectedListener(mPreferredNetworkHandler);
        if (getPreferred() == NOT_SPECIFIED_VALUE) {
            mPreferredNetworkType.setSelection(NOT_SPECIFIED_VALUE, false);
        } else if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mGeminiPhone.getPhonebyId(PhoneConstants.GEMINI_SIM_1).getPreferredNetworkType(
                    mHandler.obtainMessage(EVENT_QUERY_PREFERRED_TYPE_DONE));
        } else {
            mPhone.getPreferredNetworkType(
                    mHandler.obtainMessage(EVENT_QUERY_PREFERRED_TYPE_DONE));
        }

        mTelephonyExt = MediatekClassFactory.createInstance(ITelephonyExt.class);

        if (mTelephonyExt.isRatMenuControlledBySIM()) {
            mFilter = new IntentFilter(
                    TelephonyIntents.ACTION_EF_RAT_CONTENT_NOTIFY);
            mReceiver = new BroadcastReceiver() {

                @Override
                public void onReceive(Context context, Intent intent) {
                    Xlog.v(TAG, "onReceive, receive event");
                    mStatusEfRat = intent.getIntExtra(
                            TelephonyIntents.INTENT_KEY_EF_RAT_STATUS,
                            IccRecords.EF_RAT_FOR_OTHER_CASE);
                    int sim = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, 0);
                    if (mSimId == sim) {
                        updateUi();
                        Xlog.v(TAG, "statusEfRat: " + mStatusEfRat + " simId: "
                                + mSimId);
                    } else {
                        Xlog.d(TAG, "sim id not equal, simId: " + mSimId
                                + " receive sim: " + sim);
                    }
                }
            };
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (mTelephonyExt.isRatMenuControlledBySIM()) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                mModeType = mGeminiPhone.get3GCapabilitySIM();
                if (-1 == mModeType) {
                    Xlog.v(TAG, "3G off!");
                } else {
                    mStatusEfRat = mGeminiPhone.getPhonebyId(mModeType).getEfRatBalancing();
                    Xlog.v(TAG, "Gemini support, statusEfRat: " + mStatusEfRat + " modetype: " + mModeType);
                }
            } else {
                mModeType = mPhone.get3GCapabilitySIM();
                if (-1 == mModeType) {
                    Xlog.v(TAG, "3G off!");
                } else {
                    mStatusEfRat = mPhone.getEfRatBalancing();
                    Xlog.v(TAG, "Gemini not support, statusEfRat: " + mStatusEfRat);
                }
            }
            registerReceiver(mReceiver, mFilter);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        Xlog.v(TAG, "Enter onResume");
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            mModeType = mGeminiPhone.get3GCapabilitySIM();
        } else {
            mModeType = mPhone.get3GCapabilitySIM();
        }
        if (mTelephonyExt.isRatMenuControlledBySIM()) {
            updateUi();
        }
    }

    @Override
    public void onPause() {        
        super.onPause();
    }

    @Override
    public void onStop() {       
        if (mTelephonyExt.isRatMenuControlledBySIM()) {
            unregisterReceiver(mReceiver);
        }
        super.onStop();
    }

    /**
     * when sim card change to update ui
     */
    protected void updateUi() {
        boolean status = IccRecords.EF_RAT_NOT_EXIST_IN_USIM == mStatusEfRat;
        Xlog.v(TAG, "update UI: " + status);
        mPreferredNetworkType.setVisibility(status ? View.VISIBLE
                : View.INVISIBLE);
        Toast.makeText(this,
                IccRecords.EF_RAT_NOT_EXIST_IN_USIM == mStatusEfRat ? R.string.rat_not_exit_usim
                        : R.string.rat_for_other_case,
                        Toast.LENGTH_SHORT).show();
        if (status) {
            if (-1 == mModeType) {
                Toast.makeText(this, R.string.rat_3g_status
                        , Toast.LENGTH_SHORT).show();
                mPreferredNetworkType.setEnabled(false);
            }
        }
    }

    /**
     * @param type
     *            the integer value of the network type
     */
    private void writePreferred(int type) {
        SharedPreferences sh = this.getSharedPreferences("RATMode",
                MODE_WORLD_READABLE);
        SharedPreferences.Editor editor = sh.edit();
        if (type == NOT_SPECIFIED_VALUE) {
            editor.clear();
        } else {
            editor.putInt("ModeType", type);
        }
        editor.commit();
    }

    private int getPreferred() {
        SharedPreferences sh = this.getSharedPreferences("RATMode",
                MODE_WORLD_READABLE);
        return sh.getInt("ModeType", NOT_SPECIFIED_VALUE);
    }
}
