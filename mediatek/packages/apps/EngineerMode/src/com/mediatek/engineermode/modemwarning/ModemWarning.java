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

package com.mediatek.engineermode.modemwarning;

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class ModemWarning extends PreferenceActivity {
    private static final String TAG = "EM/ModemWarning";
    private static final String PREF_KEY = "modem_warning_key";
    private static final String PROPERTY = "persist.radio.modem.warning";
    private static final String ENABLE = "1";
    private static final String DISABLE = "0";

    private static final int DIALOG_FAILED = 1;
    private static final int MSG_MODEM_QUERY = 0;
    private static final int MSG_MODEM_SET = 1;
    private static final int MODEM_WARNING_FLAG = 0x100;
    private static final String QUERY_CMD[] = { "AT+EINFO?", "+EINFO" };
    private static final String SET_CMD = "AT+EINFO=";
    private static final String FORE_CMD = "+EINFO:";

    private CheckBoxPreference mStatusPref;

    private Handler mResponseHander = new Handler() {
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem " + msg.what);
            AsyncResult ar;
            switch (msg.what) {
            case MSG_MODEM_QUERY:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    String str = parseData((String[]) ar.result);
                    if (null != str && str.length() > 0) {
                        try {
                            int value = Integer.parseInt(str);
                            if (((String) ar.userObj).equals(ENABLE)) {
                                value = value | MODEM_WARNING_FLAG;
                            } else {
                                value = value &~ MODEM_WARNING_FLAG;
                            }
                            String[] cmd = new String[] {SET_CMD + value, ""};
                            sendAtCommand(cmd, mResponseHander.obtainMessage(MSG_MODEM_SET));
                            return;
                        } catch (NumberFormatException e) {
                            Elog.e(TAG, "Invalid number format: " + str);
                        }
                    }
                } else {
                    Elog.e(TAG, "MODEM_QUERY: exception" + ar.exception);
                }
                showDialog(DIALOG_FAILED);
                break;
            case MSG_MODEM_SET:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Elog.e(TAG, "MODEM_SET: exception" + ar.exception);
                    showDialog(DIALOG_FAILED);
                }
                break;
            default:
                break;
            }
        }
    };

    private String parseData(String[] data) {
        String ret = null;
        Elog.i(TAG, "parseData() " + data);
        if (data != null && data[0] != null && data[0].startsWith(FORE_CMD)) {
            Elog.i(TAG, "parseData() " + data[0]);
            ret = data[0].substring(FORE_CMD.length());
            ret = ret.trim();
        }
        Elog.d(TAG, "parseData() return " + ret);
        return ret;
    }

    private void sendAtCommand(String[] cmd, Message msg) {
        Elog.i(TAG, "send AT Command:" + cmd[0]);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            geminiPhone.invokeOemRilRequestStringsGemini(cmd, msg, PhoneConstants.GEMINI_SIM_1);
        } else {
            Phone phone = PhoneFactory.getDefaultPhone();
            phone.invokeOemRilRequestStrings(cmd, msg);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.modem_warning);
        mStatusPref = (CheckBoxPreference) getPreferenceScreen().findPreference(PREF_KEY);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String status = SystemProperties.get(PROPERTY, DISABLE);
        Elog.d(TAG, "Get " + PROPERTY + " = " + status);
        mStatusPref.setChecked(status.equals(ENABLE));
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Elog.d(TAG, "Enter onPreferenceTreeClick()");
        if (mStatusPref.equals(preference)) {
            String status = mStatusPref.isChecked() ? ENABLE : DISABLE;
            Elog.d(TAG, "Set " + PROPERTY + " = " + status);
            SystemProperties.set(PROPERTY, status);
            sendAtCommand(QUERY_CMD, mResponseHander.obtainMessage(MSG_MODEM_QUERY, status));
            return true;
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_FAILED:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Error");
            builder.setMessage("Set failed.");
            builder.setPositiveButton("OK", null);
            return builder.create();
        default:
            return super.onCreateDialog(id);
        }
    }
}

