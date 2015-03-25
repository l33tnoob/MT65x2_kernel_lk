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
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.TextView;

import java.util.ArrayList;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/** Just show settings rsp result, just can read. */
public class NfcSettingsResult extends Activity {

    public static final String TAG = "EM/nfc";
    private TextView mTextFWVersion = null;
    private TextView mTextHWVersion = null;
    private TextView mTextSWVersion = null;

    private ArrayList<ModeMap> mReaderModeArray = new ArrayList<ModeMap>();
    private ArrayList<ModeMap> mCardModeArray = new ArrayList<ModeMap>();
    private static final int NUMER_3 = 3;
    private static final int NUMER_4 = 4;
    private static final int NUMER_5 = 5;
    private static final int NUMER_6 = 6;
    private static final int NUMER_7 = 7;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.nfc_settings_result);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.WRAP_CONTENT);
        Elog.d(TAG, "NfcSettingsResult onCreate");
        initUI();
    }

    @Override
    protected void onResume() {
        super.onResume();
        setDisplayUI();
    }

    private void setDisplayUI() {
        NfcNativeCallClass.nfc_setting_response resp;
        resp =
            (NfcNativeCallClass.nfc_setting_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SETTINGS);
        if (resp == null) {
            Elog.e(TAG, "Take NfcRespMap.KEY_SETTINGS is null");
            // assert
            return;
        }

        mTextFWVersion.setText(String.format("0x%X", resp.fw_ver));
        mTextHWVersion.setText(String.format("0x%X", resp.hw_ver));
        mTextSWVersion.setText(String.format("0x%X", resp.sw_ver));

        setCurrentMode(resp.reader_mode, resp.card_mode);
    }

    private void initUI() {
        mTextFWVersion =
            (TextView) findViewById(R.id.NFC_Settings_Result_Version_FW);
        mTextHWVersion =
            (TextView) findViewById(R.id.NFC_Settings_Result_Version_HW);
        mTextSWVersion =
            (TextView) findViewById(R.id.NFC_Settings_Result_Version_SW);

        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_MifareUL),
                0));
        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_MifareStd),
                1));
        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_ISO14443_4A),
                2));
        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_ISO14443_4B),
                NUMER_3));
        mReaderModeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_Jewel),
            NUMER_4));
        mReaderModeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_NFC),
            NUMER_5));
        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_Felica),
                NUMER_6));
        mReaderModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_RM_ISO15693),
                NUMER_7));
        // =========================
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_MifareUL),
                0));
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_MifareStd),
                1));
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_ISO14443_4A),
                2));
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_ISO14443_4B),
                NUMER_3));
        mCardModeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_Jewel),
            NUMER_4));
        mCardModeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_NFC),
            NUMER_5));
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_Felica),
                NUMER_6));
        mCardModeArray
            .add(new ModeMap(
                (CheckBox) findViewById(R.id.NFC_Settings_Result_Support_CM_ISO15693),
                NUMER_7));

        OnClickListenerSpecial specListener = new OnClickListenerSpecial();
        for (ModeMap m : mReaderModeArray) {
            m.mChkBox.setOnClickListener(specListener);
        }
        for (ModeMap m : mCardModeArray) {
            m.mChkBox.setOnClickListener(specListener);
        }

    }

    private void setCurrentMode(int readerModeVal, int cardModeVal) {
        for (ModeMap m : mReaderModeArray) {
            if ((readerModeVal & (1 << m.mBit)) == 0) {
                m.mChkBox.setChecked(false);
            } else {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            }
        }
        for (ModeMap m : mCardModeArray) {
            if ((cardModeVal & (1 << m.mBit)) == 0) {
                m.mChkBox.setChecked(false);
            } else {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            }
        }
    }

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

}
