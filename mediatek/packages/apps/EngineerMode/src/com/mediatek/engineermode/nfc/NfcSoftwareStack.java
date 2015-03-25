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
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import java.util.ArrayList;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcSoftwareStack extends Activity implements OnClickListener {

    public static final String TAG = "EM/nfc";

    private static final int EVENT_OP_SEARCH_START = 101;
    private static final int EVENT_OP_SEARCH_FIN = 103;
    private static final int EVENT_OP_ERR = 104;
    private static final int EVENT_OP_EXCEPTION = 105;
    private static final int EVENT_OP_TIMEOUT = 106;
    private static final int EVENT_UNDER_CONSTRUCT = 107;
    private static final int DIALOG_EXCEPTION = 0;
    private static final int DIALOG_NOT_READY = 1;
    private static final int DIALOG_PROCESS = 2;
    private static final int DIALOG_SCAN_ERROR = 3;
    private Button mBtnSet = null;
    private Button mBtnScan = null;
    private CheckBox mChkRegisterNotificationAll = null;
    private EditText mEditDuration = null;

    private ArrayList<ModeMap> mRegisterNotificationArray =
        new ArrayList<ModeMap>();
    private ArrayList<ModeMap> mDiscoveryNotificationArray =
        new ArrayList<ModeMap>();
    private int mScanErrorCode = NfcCommonDef.ERROR_OK;
    private static final int NUMER_3 = 3;
    private static final int NUMER_4 = 4;
    private static final int NUMER_5 = 5;
    private static final int NUMER_6 = 6;
    private static final int NUMER_7 = 7;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.nfc_software_stack);
        Elog.e(TAG, "NfcSettingsResult onCreate");
        initUI();
    }

    @Override
    protected void onDestroy() {
        // dismissDialog(DIALOG_PROCESS);
        super.onDestroy();
    }

    /**
     * on click the button set
     * 
     * @param arg0
     *            : clicked which view
     */
    public void onClick(View arg0) {
        Elog.e(TAG, "NfcSoftwareStack onClick");
        if (arg0 == mBtnSet) {
            if (!checkRegisterNotiParam()) {
                Toast.makeText(getApplicationContext(),
                    R.string.regi_notif_nonsele, Toast.LENGTH_LONG).show();
                return;
            }
            final int checkReslut =
                checkSecureElementSupport(getValFromRegNotiBox());
            if (NfcCommonDef.ERROR_OK == checkReslut) {
                handleSecureEleResp();
            } else if (NfcCommonDef.ERROR_ERROR == checkReslut) {
                Toast.makeText(getApplicationContext(), "Response is null!",
                    Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(getApplicationContext(),
                    "Response status is: " + checkReslut, Toast.LENGTH_LONG)
                    .show();
            }
        } else if (arg0 == mBtnScan) {
            if (!checkScanParam()) {
                Toast.makeText(getApplicationContext(),
                    R.string.disc_notif_nonsele, Toast.LENGTH_LONG).show();
                return;
            }
            startScan();
        } else if (arg0 == mChkRegisterNotificationAll) {
            boolean shouldSelect = mChkRegisterNotificationAll.isChecked();
            for (ModeMap m : mRegisterNotificationArray) {
                m.mChkBox.setChecked(shouldSelect);
            }
            Elog.d(TAG, "mChkRegisterNotificationAll " + shouldSelect);
        } else {
            Elog.e(TAG, "Ghost view " + arg0.getClass().toString());
        }
        return;
    }

    private boolean checkRegisterNotiParam() {
        boolean result = true;
        if (getValFromRegNotiBox() == 0) {
            result = false;
        }
        return result;
    }

    private int checkSecureElementSupport(int secureEleMask) {
        NfcNativeCallClass.nfc_reg_notif_request req =
            new NfcNativeCallClass.nfc_reg_notif_request();
        req.reg_type = secureEleMask;
        NfcNativeCallClass.nfc_reg_notif_response resp;
        resp = NfcNativeCallClass.getRegisterNotif(req);
        if (resp == null) {
            return NfcCommonDef.ERROR_ERROR;
        } else if (resp.status != 0) {
            return resp.status;
        } else {
            NfcRespMap.getInst().put(NfcRespMap.KEY_SS_REGISTER_NOTIF, resp);
            return NfcCommonDef.ERROR_OK;
        }
    }

    private void handleSecureEleResp() {
        NfcNativeCallClass.nfc_reg_notif_response resp;
        resp =
            (NfcNativeCallClass.nfc_reg_notif_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SS_REGISTER_NOTIF);
        if (resp == null) {
            Elog.e(TAG, "Take NfcRespMap.KEY_SS_REGISTER_NOTIF is null");
            Toast.makeText(getApplicationContext(), "Response is null.",
                Toast.LENGTH_LONG).show();
            return;
        }
        if (resp.se == 0) {
            // not support
            Toast.makeText(getApplicationContext(),
                R.string.secure_elem_nosupt, Toast.LENGTH_LONG).show();
        } else {
            final Intent intent = new Intent();
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackSecure");
            this.startActivity(intent);
        }
    }

    private boolean checkScanParam() {
        String strDuration = mEditDuration.getText().toString();
        if (strDuration == null || strDuration.length() == 0) {
            strDuration = "500"; // 500ms for default.
            mEditDuration.setText(strDuration);
        }
        boolean result = true;
        if (getValFromDisNotiBox() == 0) {
            result = false;
        }
        return result;
    }

    private void startScan() {
        new Thread() {
            @Override
            public void run() {

                mHandler.sendEmptyMessage(EVENT_OP_SEARCH_START);
                int errCode = sendScanCommand();
                Elog.d(TAG, "sendScanCommand errCode = " + errCode);
                if (NfcCommonDef.ERROR_OK == errCode) {
                    onScanOK();
                } else {
                    // if (NfcCommonDef.ERROR_TIMEOUT == errCode) {
                    // mHandler.sendEmptyMessage(EVENT_OP_TIMEOUT);
                    // } else {
                    mScanErrorCode = errCode;
                    mHandler.sendEmptyMessage(EVENT_OP_ERR);
                    // }
                }
                mHandler.sendEmptyMessage(EVENT_OP_SEARCH_FIN);
            }

        }.start();
    }

    private int sendScanCommand() {
        NfcNativeCallClass.nfc_dis_notif_request req =
            new NfcNativeCallClass.nfc_dis_notif_request();

        req.dis_type = getValFromDisNotiBox();
        // this is for sure NOT throw Exception. because of checkScanParam()
        req.duration = Integer.valueOf(mEditDuration.getText().toString());

        Elog.d(TAG, String.format("REQ: dis_type, duration,%d,%d",
            req.dis_type, req.duration));
        NfcNativeCallClass.nfc_dis_notif_response resp;
        resp = NfcNativeCallClass.getDiscoveryNotif(req);

        if (resp == null) {
            Elog.e(TAG, "NfcNativeCallClass.getSettings(req) (resp == null)");
            return NfcCommonDef.ERROR_ERROR;
        }
        if (resp.status != 0) {
            Elog.e(TAG,
                "NfcNativeCallClass.getSettings(req) (resp.status != 0) = "
                    + resp.status);
            return resp.status;
        }
        Elog.i(TAG, "NfcNativeCallClass.getDiscoveryNotif(req) status = "
            + resp.status);
        NfcRespMap.getInst().put(NfcRespMap.KEY_SS_SCAN_COMPLETE, resp);
        return NfcCommonDef.ERROR_OK;
    }

    // start other activity depends on rsp target
    private void onScanOK() {
        NfcNativeCallClass.nfc_dis_notif_response resp;
        resp =
            (NfcNativeCallClass.nfc_dis_notif_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SS_SCAN_COMPLETE);
        if (resp == null) {
            Elog.e(TAG, "Take NfcRespMap.KEY_SS_SCAN_COMPLETE is null");
            // assert
            return;
        }

        Intent intent = new Intent();
        if (resp.target instanceof NfcNativeCallClass.nfc_none_det_response) {
            Toast.makeText(getApplicationContext(), R.string.no_target_detect,
                Toast.LENGTH_LONG).show();
        } else if (resp.target instanceof NfcNativeCallClass.nfc_tag_det_response) {
            // FEATURE DRIVER IS NOT READY. SO COMMENT IT.
            NfcRespMap.getInst().put(NfcRespMap.KEY_SS_TAG_DECT, resp.target);
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackScan");
            this.startActivity(intent);
            // mHandler.sendEmptyMessage(EVENT_UNDER_CONSTRUCT);
        } else if (resp.target instanceof NfcNativeCallClass.nfc_p2p_det_response) {
            // FEATURE DRIVER IS NOT READY. SO COMMENT IT.
            NfcRespMap.getInst().put(NfcRespMap.KEY_SS_P2P_TARGET_DECT,
                resp.target);
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackP2PInitDect"); // no
            // support
            this.startActivity(intent);
            // mHandler.sendEmptyMessage(EVENT_UNDER_CONSTRUCT);
        } else {
            Elog.e(TAG, "unknown target type "
                + resp.target.getClass().toString());
        }
        return;
    }

    private void initUI() {
        mBtnSet =
            (Button) findViewById(R.id.NFC_SoftwareStack_RegisterNotification_BtnSet);
        mBtnScan =
            (Button) findViewById(R.id.NFC_SoftwareStack_DiscoveryNotification_BtnSet);
        mChkRegisterNotificationAll =
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_ALL);
        mEditDuration =
            (EditText) findViewById(R.id.NFC_SoftwareStack_DN_Duration);

        mBtnSet.setOnClickListener(this);
        mBtnScan.setOnClickListener(this);
        mChkRegisterNotificationAll.setOnClickListener(this);

        mRegisterNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_MifareUL),
            0));
        mRegisterNotificationArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_MifareStd),
                1));
        mRegisterNotificationArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_ISO14443_4A),
                2));
        mRegisterNotificationArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_ISO14443_4B),
                NUMER_3));
        mRegisterNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_Jewel),
            NUMER_4));
        mRegisterNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_NFC),
            NUMER_5));
        mRegisterNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_Felica),
            NUMER_6));
        mRegisterNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_ISO15693),
            NUMER_7));
        // =========================
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_ISO14443A), 0));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_ISO14443B), 1));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_felica212), 2));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_felica424),
            NUMER_3));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_ISO15693),
            NUMER_4));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_NFCActive),
            NUMER_5));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_DCE), NUMER_6));
        mDiscoveryNotificationArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_DN_DisableP2P),
            NUMER_7));

    }

    private int getValFromRegNotiBox() {
        int val = 0;
        for (ModeMap m : mRegisterNotificationArray) {
            if (m.mChkBox.isChecked()) {
                val |= (1 << m.mBit);
            }
        }
        return val;
    }

    private int getValFromDisNotiBox() {
        int val = 0;
        for (ModeMap m : mDiscoveryNotificationArray) {
            if (m.mChkBox.isChecked()) {
                val |= (1 << m.mBit);
            }
        }
        return val;
    }

    Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AlertDialog.Builder builder = null;
            switch (msg.what) {
            case EVENT_OP_SEARCH_START:
                showDialog(DIALOG_PROCESS);
                break;
            case EVENT_OP_SEARCH_FIN:
                dismissDialog(DIALOG_PROCESS);
                break;
            case EVENT_OP_ERR:
                // Set Error. return status != 0.
                showDialog(DIALOG_SCAN_ERROR);
                break;
            // case EVENT_OP_TIMEOUT:
            // showDialog(DIALOG_TIME_OUT);
            // break;
            case EVENT_OP_EXCEPTION:
                showDialog(DIALOG_EXCEPTION);
                break;
            case EVENT_UNDER_CONSTRUCT:
                showDialog(DIALOG_NOT_READY);
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
        case DIALOG_NOT_READY:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.not_ready_title).setMessage(
                R.string.not_ready_msg).setPositiveButton(android.R.string.ok,
                null);
            dialog = builder.create();
            break;
        case DIALOG_SCAN_ERROR:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.dialog_error_title).setMessage(
                "Scan response status is: " + mScanErrorCode)
                .setPositiveButton(android.R.string.ok, null);
            dialog = builder.create();
            break;
        case DIALOG_PROCESS:
            ProgressDialog progressDialog = new ProgressDialog(this);
            if (null != progressDialog) {
                progressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                progressDialog.setMessage(getResources().getString(
                    R.string.scan_msg));
                progressDialog.setTitle(getResources().getString(
                    R.string.scan_title));
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
