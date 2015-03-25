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
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.IccCard;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.telephony.TelephonyManagerEx;

public class SimLockBaseActivity extends Activity implements DialogInterface.OnKeyListener {
    private static final String TAG = "SIMMELOCK";
    int simNumber = 0;
    int lockCategory = -1;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Bundle bundle = this.getIntent().getExtras();
        simNumber = bundle.getInt("SIMNo");
        if (bundle != null) {
            lockCategory = bundle.getInt(ActionList.LOCKCATEGORY, -1);
        }
        if (lockCategory == -1) {
            finish();
            return;
        }

        // set the title
        String lockName = getLockName(lockCategory);
        this.setTitle(lockName);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public boolean onKey(DialogInterface arg0, int arg1, KeyEvent arg2) {
        if (arg2.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            arg0.dismiss();
            finish();
            return true;
        }
        return false;
    }

    protected String getLockName(final int locktype) {
        switch (locktype) {
        case 0:
            return getString(R.string.strLockNameNetwork);
        case 1:
            return getString(R.string.strLockNameNetworkSub);
        case 2:
            return getString(R.string.strLockNameService);
        case 3:
            return getString(R.string.strLockNameCorporate);
        case 4:
            return getString(R.string.strLockNameSIM);
        default:
            return getString(R.string.simmelock_name);
        }
    }

    protected boolean isSimReady() {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
            if (TelephonyManager.SIM_STATE_ABSENT == telephonyManager.getSimState()) {
                return false;
            }
        } else {
            TelephonyManagerEx telephonyManagerEx = TelephonyManagerEx.getDefault();
            if (TelephonyManager.SIM_STATE_ABSENT == telephonyManagerEx.getSimState(simNumber)) {
                return false;
            }
        }
        return true;
    }

    protected void setIccNetworkLockEnabled(int category, int op, String passwd, String mcc, String gid1, String gid2, Message callback) {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            Phone phone = PhoneFactory.getDefaultPhone();
            if (phone != null) {
                phone.getIccCard().setIccNetworkLockEnabled(category, op, passwd, mcc, gid1, gid2, callback);
            }
        } else {
            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            if (geminiPhone != null) {
                geminiPhone.getPhonebyId(simNumber).getIccCard().setIccNetworkLockEnabled(category, op, passwd, mcc, gid1, gid2, callback);
            }
        }
    }

    protected void log(String str) {
    	Log.i(TAG, "[SIM" + (simNumber + 1) + "] " + str);
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            }
        }
    };

    protected View.OnLongClickListener mOnLongClickListener = new View.OnLongClickListener() {
        @Override
        public boolean onLongClick(View v) {
            return true;
        }
    };
}
