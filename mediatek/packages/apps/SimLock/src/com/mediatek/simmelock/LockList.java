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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.IccCard;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

public class LockList extends PreferenceActivity {
    public static final String LOCKCATEGORY = "LockCategory";
    private static final int NPLOCKTYPE = 0;
    private static final int NSPLOCKTYPE = 1;
    private static final int SPLOCKTYPE = 2;
    private static final int CPLOCKTYPE = 3;
    private static final int SIMPLOCKTYPE = 4;
    private static final int QUERY_ICC_SML_COMPLETE = 120;

    int simNumber = 0;
    boolean isSimInsert = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.addPreferencesFromResource(R.xml.locklist);

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            Bundle bundleReceive = this.getIntent().getExtras();
            simNumber = bundleReceive.getInt("Setting SIM Number");
            isSimInsert = TelephonyManagerEx.getDefault().hasIccCard(simNumber);
        } else {
            isSimInsert = TelephonyManager.getDefault().hasIccCard();
        }

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();

            boolean isRadioOn  = false;
            if (geminiPhone != null) {
                isRadioOn = geminiPhone.isRadioOnGemini(simNumber);
            }
            Log.i("SIMMELOCK", "isSimInsert == " + isSimInsert + " isRadioOn == " + isRadioOn);
            if ((!isSimInsert) || (!isRadioOn)) {
                getPreferenceScreen().setEnabled(false);
                return;
            } else {
                getPreferenceScreen().setEnabled(true);
            }
        } else {
            final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
            try {
                if (!isSimInsert || !iTel.isRadioOn()) {
                    Log.w("SIMMELOCK", "isSimInsert == " + isSimInsert);
                    getPreferenceScreen().setEnabled(false);
                    return;
                } else {
                    getPreferenceScreen().setEnabled(true);
                }
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);

        for (int category = 0; category < 5; category++) {
            Message callback = Message.obtain(mHandler, QUERY_ICC_SML_COMPLETE);
            queryIccNetworkLock(category, callback);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferencescreen, Preference preference) {
        Bundle bundle = new Bundle();
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            // if it single SIM, no need to pass the card number
        } else {
            // Gemini SIM: put the SIM number to the bundle
            bundle.putInt("SIMNo", simNumber);
        }

        if (this.getPreferenceScreen().findPreference("nplock") == preference) {
            Intent intent = new Intent(LockList.this, ActionList.class);
            bundle.putInt(LOCKCATEGORY, NPLOCKTYPE);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("nsplock") == preference) {
            Intent intent = new Intent(LockList.this, ActionList.class);
            bundle.putInt(LOCKCATEGORY, NSPLOCKTYPE);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("splock") == preference) {
            Intent intent = new Intent(LockList.this, ActionList.class);
            bundle.putInt(LOCKCATEGORY, SPLOCKTYPE);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("cplock") == preference) {
            Intent intent = new Intent(LockList.this, ActionList.class);
            bundle.putInt(LOCKCATEGORY, CPLOCKTYPE);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("simplock") == preference) {
            Intent intent = new Intent(LockList.this, ActionList.class);
            bundle.putInt(LOCKCATEGORY, SIMPLOCKTYPE);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        }
        return false;
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case QUERY_ICC_SML_COMPLETE:
                Log.i("SIMMELOCK", "QUERY_ICC_SML_COMPLETE");
                AsyncResult ar1 = (AsyncResult) msg.obj;
                int[] LockState = (int[]) ar1.result;
                if (LockState == null) {
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().setEnabled(false);
                } else if (LockState[2] == 0) {
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(0).setEnabled(false);
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(1).setEnabled(false);
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(2).setEnabled(false);
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(3).setEnabled(false);
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(4).setEnabled(false);
                } else if (LockState[1] == 4) {
                    // be disabled!
                    ((PreferenceActivity) (LockList.this)).getPreferenceScreen().getPreference(LockState[0]).setEnabled(
                            false);
                }
                break;
            default:
                break;
            }
        }
    };

    private void queryIccNetworkLock(int lockCategory, Message callback) {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            // if it single SIM, no need to pass the card number
            Phone phone = PhoneFactory.getDefaultPhone();
            if (phone != null) {
                IccCard iccCard = phone.getIccCard();
                if (iccCard != null) {
                    iccCard.QueryIccNetworkLock(lockCategory, 4, null, null, null, null, callback);
                }
            }
        } else {
            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            if (geminiPhone != null) {
                IccCard iccCard = geminiPhone.getPhonebyId(simNumber).getIccCard();
                if (iccCard != null) {
                    iccCard.QueryIccNetworkLock(lockCategory, 4, null, null, null, null, callback);
                }
            }
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            }
        }
    };
}
