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

package com.mediatek.engineermode.nfc;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioGroup;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcSettings extends Activity implements OnClickListener {

    private static final String TAG = "EM/nfc";
    private static final String ENTRY_NFC_ONOFF = "nfconoff";
    private static final String ENTRY_DBG_ONOFF = "dbgonoff";

    private static final int DIALOG_EXCEPTION = 0;
    private static final int DIALOG_RSP_ERROR = 1;
    private static final int DIALOG_PROCESS = 2;
    private static final int SELECTION_NFC_OFF = 0;
    private static final int SELECTION_NFC_ON = 1;
    private static final int SELECTION_DBG_OFF = 0;
    private static final int SELECTION_DBG_ON = 1;
    private static final int SELECTION_PROT_SW = 0;
    private static final int SELECTION_PROT_RD = 1;
    private static final int SELECTION_PROT_OFF = 2;

    private static final int EVENT_OP_SEARCH_START = 101;
    private static final int EVENT_OP_SEARCH_FIN = 103;
    private static final int EVENT_OP_ERR = 104;
    private static final int EVENT_OP_EXCEPTION = 105;
    private static final int EVENT_OP_TIMEOUT = 106;
    private static final int EVENT_OP_OK = 107;

    private Button mBtnSet = null;
    private RadioGroup mGpNfcOnOff = null;
    private RadioGroup mGpDebug = null;
    private RadioGroup mGpProtocol = null;

    private int mSetRspStatus = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.nfc_settings);
        initUI();
    }

    /**
     * on click the button set
     * 
     * @param arg0
     *            : clicked which view
     */
    public void onClick(View arg0) {
        if (arg0 == mBtnSet) {
            Elog.d(TAG, "NfcSettings onClick");
            startSetting();
        }
    }

    private void initUI() {
        mBtnSet = (Button) findViewById(R.id.NFC_Settings_Btn_Set);
        mGpNfcOnOff = (RadioGroup) findViewById(R.id.NFC_Settings_NFC);
        mGpDebug = (RadioGroup) findViewById(R.id.NFC_Settings_NFCDBG);
        mGpProtocol = (RadioGroup) findViewById(R.id.NFC_Settings_Protocol);
        mBtnSet.setOnClickListener(this);

        final SharedPreferences preferences =
            this.getSharedPreferences(NfcCommonDef.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        if (preferences.getBoolean(ENTRY_NFC_ONOFF, false)) {
            mGpNfcOnOff.check(R.id.NFC_Settings_NFC_ON);
        } else {
            mGpNfcOnOff.check(R.id.NFC_Settings_NFC_OFF);
        }

        if (preferences.getBoolean(ENTRY_DBG_ONOFF, false)) {
            mGpDebug.check(R.id.NFC_Settings_NFCDBG_ON);
        } else {
            mGpDebug.check(R.id.NFC_Settings_NFCDBG_OFF);
        }

        //if (preferences.getBoolean(NfcEntry.ENTRY_SOFTWARESTACK, false)) {
        //    mGpProtocol.check(R.id.NFC_Settings_Protocol_SW);
        //} else 
        if (preferences.getBoolean(NfcEntry.ENTRY_RAWDATA, false)) {
            mGpProtocol.check(R.id.NFC_Settings_Protocol_RD);
        } else {
            mGpProtocol.check(R.id.NFC_Settings_Protocol_OFF);
        }
    }

    private int getSelection(int checkedId) {
        int selection = 0;
        switch (checkedId) {
        case R.id.NFC_Settings_NFC_ON:
            selection = SELECTION_NFC_ON;
            break;
        case R.id.NFC_Settings_NFC_OFF:
            selection = SELECTION_NFC_OFF;
            break;
        case R.id.NFC_Settings_NFCDBG_ON:
            selection = SELECTION_DBG_ON;
            break;
        case R.id.NFC_Settings_NFCDBG_OFF:
            selection = SELECTION_DBG_OFF;
            break;
        case R.id.NFC_Settings_Protocol_RD:
            selection = SELECTION_PROT_RD;
            break;
        //case R.id.NFC_Settings_Protocol_SW:
        //    selection = SELECTION_PROT_SW;
        //    break;
        case R.id.NFC_Settings_Protocol_OFF:
            selection = SELECTION_PROT_OFF;
            break;
        default:
            break;
        }
        return selection;
    }

    private int sendCommand() {

        NfcNativeCallClass.nfc_setting_request req =
            new NfcNativeCallClass.nfc_setting_request();

        req.nfc_enable = getSelection(mGpNfcOnOff.getCheckedRadioButtonId());
        req.debug_enable = getSelection(mGpDebug.getCheckedRadioButtonId());
        req.sw_protocol = getSelection(mGpProtocol.getCheckedRadioButtonId());
        req.get_capabilities = 0;
        Elog.d(TAG, String.format(
            "REQ: nfc_enable, debug_enable, sw_protocol, %d,%d,%d",
            req.nfc_enable, req.debug_enable, req.sw_protocol));
        NfcNativeCallClass.nfc_setting_response resp;
        resp = NfcNativeCallClass.getSettings(req);
        if (resp == null) {
            Elog.e(TAG, "NfcNativeCallClass.getSettings(req) (resp == null)");
            return NfcCommonDef.ERROR_ERROR;
        }
        if (resp.status != 0) {
            // if (resp.status == NfcCommonDef.RESULT_STATUS_TIMEOUT) {
            // Elog.e(TAG, "NfcNativeCallClass.getSettings(req) time out");
            // return NfcCommonDef.ERROR_TIMEOUT;
            // }
            Elog.e(TAG,
                "NfcNativeCallClass.getSettings(req) (resp.status != 0) = "
                    + resp.status);
            mSetRspStatus = resp.status;
            return NfcCommonDef.ERROR_ERROR;
        }
        Elog.d(TAG, String.format(
            "RES: nfc_enable, debug_enable, sw_protocol, %d,%d,%d",
            resp.nfc_enable, resp.debug_enable, resp.sw_protocol));

        NfcRespMap.getInst().put(NfcRespMap.KEY_SETTINGS, resp);
        return NfcCommonDef.ERROR_OK;
    }

    private void checkRadiobox(int nfcEnable, int debugEnable, int swProtocol) {
        final SharedPreferences preferences =
            this.getSharedPreferences(NfcCommonDef.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        Editor editor = preferences.edit();
        if (nfcEnable == SELECTION_NFC_ON) {
            mGpNfcOnOff.check(R.id.NFC_Settings_NFC_ON);
            editor.putBoolean(ENTRY_NFC_ONOFF, true);
            //if (swProtocol == SELECTION_PROT_SW) {
            //    mGpProtocol.check(R.id.NFC_Settings_Protocol_SW);
            //    editor.putBoolean(NfcEntry.ENTRY_SOFTWARESTACK, true);
            //    editor.putBoolean(NfcEntry.ENTRY_RAWDATA, false);
            //} else 
            if (swProtocol == SELECTION_PROT_RD) {
                mGpProtocol.check(R.id.NFC_Settings_Protocol_RD);
                editor.putBoolean(NfcEntry.ENTRY_SOFTWARESTACK, false);
                editor.putBoolean(NfcEntry.ENTRY_RAWDATA, true);
            } else {
                mGpProtocol.check(R.id.NFC_Settings_Protocol_OFF);
                editor.putBoolean(NfcEntry.ENTRY_SOFTWARESTACK, false);
                editor.putBoolean(NfcEntry.ENTRY_RAWDATA, false);
            }
        } else {
            mGpNfcOnOff.check(R.id.NFC_Settings_NFC_OFF);
            mGpProtocol.check(R.id.NFC_Settings_Protocol_OFF);
            editor.putBoolean(ENTRY_NFC_ONOFF, false);
            editor.putBoolean(NfcEntry.ENTRY_SOFTWARESTACK, false);
            editor.putBoolean(NfcEntry.ENTRY_RAWDATA, false);
        }

        if (debugEnable == SELECTION_DBG_ON) {
            mGpDebug.check(R.id.NFC_Settings_NFCDBG_ON);
            editor.putBoolean(ENTRY_DBG_ONOFF, true);
        } else {
            mGpDebug.check(R.id.NFC_Settings_NFCDBG_OFF);
            editor.putBoolean(ENTRY_DBG_ONOFF, false);
        }
        editor.commit();
    }

    private void handleResp() {
        NfcNativeCallClass.nfc_setting_response resp;
        resp =
            (NfcNativeCallClass.nfc_setting_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SETTINGS);
        if (resp == null) {
            Elog.e(TAG, "processResp Take NfcRespMap.KEY_SETTINGS is null");
            return;
        }
        checkRadiobox(resp.nfc_enable, resp.debug_enable, resp.sw_protocol);
    }

    private void startSetting() {
        new Thread() {
            @Override
            public void run() {
                mHandler.sendEmptyMessage(EVENT_OP_SEARCH_START);
                int errCode = sendCommand();
                Elog.d(TAG, "sendScanCommand errCode = " + errCode);
                if (NfcCommonDef.ERROR_OK != errCode) {
                    // if (NfcCommonDef.ERROR_TIMEOUT == errCode) {
                    // mHandler.sendEmptyMessage(EVENT_OP_TIMEOUT);
                    // } else {
                    mHandler.sendEmptyMessage(EVENT_OP_ERR);
                    // }
                } else {
                    mHandler.sendEmptyMessage(EVENT_OP_OK);
                }
                mHandler.sendEmptyMessage(EVENT_OP_SEARCH_FIN);
            }
        }.start();
    }

    Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_OP_SEARCH_START:
                showDialog(DIALOG_PROCESS);
                break;
            case EVENT_OP_SEARCH_FIN:
                dismissDialog(DIALOG_PROCESS);
                break;
            case EVENT_OP_ERR:
                showDialog(DIALOG_RSP_ERROR); // show user setting resp status
                break;
            // case EVENT_OP_TIMEOUT:
            // checkRadiobox(SELECTION_NFC_OFF, SELECTION_DBG_OFF,
            // SELECTION_PROT_OFF);
            // showDialog(DIALOG_TIME_OUT);
            // break;
            case EVENT_OP_EXCEPTION:
                showDialog(DIALOG_EXCEPTION);
                break;
            case EVENT_OP_OK:
                handleResp();
                final Intent intent = new Intent();
                intent.setClassName(NfcSettings.this,
                    "com.mediatek.engineermode.nfc.NfcSettingsResult");
                NfcSettings.this.startActivity(intent);
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder;
        switch (id) {
        case DIALOG_EXCEPTION:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.dialog_error_title).setMessage(
                R.string.jni_error_msg).setPositiveButton(android.R.string.ok,
                null);
            dialog = builder.create();
            break;
        case DIALOG_RSP_ERROR:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.dialog_error_title).setMessage(
                "setting rsp status is :" + mSetRspStatus).setPositiveButton(
                android.R.string.ok, null);
            dialog = builder.create();
            break;
        case DIALOG_PROCESS:
            ProgressDialog progressDialog =
                new ProgressDialog(NfcSettings.this);
            if (null != progressDialog) {
                progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                progressDialog.setMessage("In Progress...");
                progressDialog.setTitle("Setting");
                progressDialog.setCancelable(false);
            } else {
                Elog.e(TAG, "new progressDialog failed");
            }
            return progressDialog;
        default:
            break;
        }
        return dialog;
    }
}
