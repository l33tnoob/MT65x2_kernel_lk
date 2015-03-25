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
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcSoftwareStackSecure extends Activity implements OnClickListener {

    public static final String TAG = "EM/nfc";
    public static final int SE_MODE_NOTSUPPORT = 0;
    public static final int SE_MODE_SE1 = 1;
    public static final int SE_MODE_SE2 = 2;
    public static final int SE_MODE_STATUS_OFF = 0;
    public static final int SE_MODE_STATUS_VIRTUAL = 1;
    public static final int SE_MODE_STATUS_WIRED = 2;
    private static final int DIALOG_EXCEPTION = 0;

    private TextView mEleInfo;
    private CheckBox mDummyTag;
    private Button mBtnSet;
    private RadioGroup mGpSE1;
    private RadioGroup mGpSE2;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.nfc_software_stack_secure);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.WRAP_CONTENT);
        Elog.d(TAG, "NfcSoftwareStackSecure onCreate");
        initUI();
        // Because driver is not ready. disable for temp.
        mBtnSet.setEnabled(false);

    }

    @Override
    protected void onResume() {
        super.onResume();
        readSecureEleInfo();
    }

    private void readSecureEleInfo() {
        NfcNativeCallClass.nfc_reg_notif_response resp;
        resp =
            (NfcNativeCallClass.nfc_reg_notif_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SS_REGISTER_NOTIF);
        if (resp == null) {
            Elog.e(TAG, "Take NfcRespMap.KEY_SS_REGISTER_NOTIF is null");
            // assert
            this.finish();
            return;
        }

        Elog.d(TAG, "se " + resp.se + " se_status " + resp.se_status);
        if (resp.se == SE_MODE_NOTSUPPORT) {
            // not support, normally not happen.
            Toast.makeText(getApplicationContext(),
                R.string.secure_elem_nosupt, Toast.LENGTH_LONG).show();
            this.finish();
        } else if (resp.se == SE_MODE_SE1) {
            // mGpSE1.setEnabled(true); //it does not work ?!!
            setRadioGpEnable(mGpSE1);
            setSEIdx(mGpSE1, resp.se_status);
        } else if (resp.se == SE_MODE_SE2) {
            // mGpSE2.setEnabled(true);
            // mGpSE1.setEnabled(false);
            setRadioGpEnable(mGpSE2);
            setSEIdx(mGpSE2, resp.se_status);
        }

        if (resp.length == 0) {
            mEleInfo.setText("No Info.");
        } else {
            if (resp.data != null) {
                mEleInfo.setText(new String(resp.data));
            } else {
                mEleInfo.setText("Broken Info.");
            }
        }

        Elog.d(TAG, "mask " + resp.se_type);
        setCurrentSEEventMask(resp.se_type);

        return;
    }

    private void setRadioGpEnable(RadioGroup gp) {
        RadioButton gp11 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_OFF);
        RadioButton gp12 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Virtual);
        RadioButton gp13 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Wired);
        RadioButton gp21 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_OFF);
        RadioButton gp22 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Virtual);
        RadioButton gp23 =
            (RadioButton) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Wired);

        if (gp.getId() == mGpSE1.getId()) {
            gp11.setEnabled(true);
            gp12.setEnabled(true);
            gp13.setEnabled(true);
            gp21.setEnabled(false);
            gp22.setEnabled(false);
            gp23.setEnabled(false);
        } else if (gp.getId() == mGpSE2.getId()) {
            gp21.setEnabled(true);
            gp22.setEnabled(true);
            gp23.setEnabled(true);
            gp11.setEnabled(false);
            gp12.setEnabled(false);
            gp13.setEnabled(false);
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mBtnSet) {
            int seIdx = SE_MODE_STATUS_OFF;
            if (mGpSE1.isEnabled()) {
                seIdx = getSEIdx(mGpSE1.getCheckedRadioButtonId());
            } else if (mGpSE2.isEnabled()) {
                seIdx = getSEIdx(mGpSE2.getCheckedRadioButtonId());
            } else {
                return;
            }
            int setSeResult = setSEOption(seIdx);
            if (NfcCommonDef.ERROR_OK == setSeResult) {
                Toast.makeText(getApplicationContext(), android.R.string.ok,
                    Toast.LENGTH_LONG).show();

            } else if (NfcCommonDef.ERROR_ERROR == setSeResult) {
                Toast.makeText(getApplicationContext(), "Response is null.",
                    Toast.LENGTH_LONG).show();
            } else {
                // Set Error. return status != 0.
                Toast.makeText(getApplicationContext(),
                    "Response status is: " + setSeResult, Toast.LENGTH_LONG)
                    .show();
            }
        }
    }

    private int setSEOption(int seIdx) {
        NfcNativeCallClass.nfc_se_set_request req =
            new NfcNativeCallClass.nfc_se_set_request();
        req.set_SEtype = seIdx;

        NfcNativeCallClass.nfc_se_set_response resp;
        resp = NfcNativeCallClass.setSEOption(req);
        if (resp == null) {
            return NfcCommonDef.ERROR_ERROR;
        } else if (resp.status == 0) {
            return NfcCommonDef.ERROR_OK;
        } else {
            return resp.status;
        }
    }

    private int getSEIdx(int radioId) {
        int idx = SE_MODE_STATUS_OFF;
        switch (radioId) {
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_OFF:
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_OFF:
            idx = SE_MODE_STATUS_OFF;
            break;
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Virtual:
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Virtual:
            idx = SE_MODE_STATUS_VIRTUAL;
            break;
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Wired:
        case R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Wired:
            idx = SE_MODE_STATUS_WIRED;
            break;
        default:
            break;
        }
        return idx;
    }

    private void setSEIdx(RadioGroup gp, int status) {
        if (gp.getId() == mGpSE1.getId()) {
            switch (status) {
            case SE_MODE_STATUS_OFF:
                gp.check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_OFF);
                break;
            case SE_MODE_STATUS_VIRTUAL:
                gp
                    .check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Virtual);
                break;
            case SE_MODE_STATUS_WIRED:
                gp.check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1_Wired);
                break;
            default:
                gp.clearCheck();
                break;
            }
        } else {
            switch (status) {
            case SE_MODE_STATUS_OFF:
                gp.check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_OFF);
                break;
            case SE_MODE_STATUS_VIRTUAL:
                gp
                    .check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Virtual);
                break;
            case SE_MODE_STATUS_WIRED:
                gp.check(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2_Wired);
                break;
            default:
                gp.clearCheck();
                break;
            }
        }
    }

    private void initUI() {
        mEleInfo =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_Info);
        mDummyTag =
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_Dect);
        mBtnSet =
            (Button) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Set);
        mGpSE1 =
            (RadioGroup) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele1);
        mGpSE2 =
            (RadioGroup) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_SetMode_Ele2);

        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_StartTrans),
                NfcNativeCallClass.BM_START_OF_TRANSACTION));
        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_EndTrans),
                NfcNativeCallClass.BM_END_OF_TRANSACTION));
        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_Trans),
                NfcNativeCallClass.BM_TRANSACTION));
        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_RF_ON),
                NfcNativeCallClass.BM_RF_FIELD_ON));

        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_RF_OFF),
                NfcNativeCallClass.BM_RF_FIELD_OFF));

        mDisplayChkBoxArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_SoftwareStack_Secure_Ele_EventDect_Conn),
                NfcNativeCallClass.BM_CONNECTIVITY));

        // =========================
        mBtnSet.setOnClickListener(this);

        OnClickListenerSpecial specListener = new OnClickListenerSpecial();
        for (ModeMap m : mDisplayChkBoxArray) {
            m.mChkBox.setOnClickListener(specListener);
        }
        mDummyTag.setOnClickListener(specListener);

        mDummyTag.setChecked(true); // stupid check it.

    }

    private ArrayList<ModeMap> mDisplayChkBoxArray = new ArrayList<ModeMap>();

    private class OnClickListenerSpecial implements OnClickListener {
        // this function's purpose is making checkBox only display the status,
        // and can not be set.
        public void onClick(View arg0) {
            if (arg0 instanceof CheckBox) {
                CheckBox chk = (CheckBox) arg0;
                if (chk.isChecked()) {
                    chk.setChecked(false);
                } else {
                    chk.setChecked(true);
                }
            }
        }
    }

    private void setCurrentSEEventMask(int eventVal) {
        for (ModeMap m : mDisplayChkBoxArray) {
            if ((eventVal & (1 << m.mBit)) != 0) {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            } else {
                m.mChkBox.setChecked(false);
            }
        }

    }

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
        default:
            break;
        }
        return dialog;
    }
}
