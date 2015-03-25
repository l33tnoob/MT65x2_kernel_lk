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
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcSoftwareStackScan extends Activity implements OnClickListener {

    public static final String TAG = "EM/nfc";
    private static final int TAGTYPE_NORMAL = 1; // according to protocol
    private static final int TAGTYPE_NDEF = 2; // according to protocol

    private static final int FUNCTION_SUPPORT_RW = 1;
    private static final int FUNCTION_SUPPORT_RAW = 2;
    private static final int FUNCTION_SUPPORT_NONE = 3;

    // because read / write will access these constants.
    public static final int CARDTYPE_TAG_TYPE_DEFAULT_VAL =
        NfcNativeCallClass.TAG_TYPE_DEFAULT;
    public static final int CARDTYPE_MC1K_VAL =
        NfcNativeCallClass.TAG_TYPE_MIFARE_UL;
    public static final int CARDTYPE_MC4K_VAL =
        NfcNativeCallClass.TAG_TYPE_MIFARE_STD;
    public static final int CARDTYPE_NDEF_VAL =
        NfcNativeCallClass.TAG_TYPE_NDEF;
    // only support raw data.
    public static final int CARDTYPE_TAG_TYPE_ISO1443_4A_VAL =
        NfcNativeCallClass.TAG_TYPE_ISO1443_4A;
    public static final int CARDTYPE_TAG_TYPE_ISO1443_4B_VAL =
        NfcNativeCallClass.TAG_TYPE_ISO1443_4B;
    public static final int CARDTYPE_TAG_TYPE_JEWWL_VAL =
        NfcNativeCallClass.TAG_TYPE_JEWWL;
    public static final int CARDTYPE_TAG_TYPE_FELICA_VAL =
        NfcNativeCallClass.TAG_TYPE_FELICA;
    // not support currently.
    public static final int CARDTYPE_TAG_TYPE_NFC_VAL =
        NfcNativeCallClass.TAG_TYPE_NFC;
    public static final int CARDTYPE_TAG_TYPE_ISO15693_VAL =
        NfcNativeCallClass.TAG_TYPE_ISO15693;

    private static final String CARDTYPE_MC1K = "MIFARE_UL";
    private static final String CARDTYPE_MC4K = "MIFARE_STD";
    private static final String CARDTYPE_NDEF = "NDEF";
    private static final String CARDTYPE_TAG_TYPE_ISO1443_4A = "ISO14443_4A";
    private static final String CARDTYPE_TAG_TYPE_ISO1443_4B = "ISO14443_4B";
    private static final String CARDTYPE_TAG_TYPE_JEWWL = "JEWEL";
    private static final String CARDTYPE_TAG_TYPE_FELICA = "FELICA";
    private static final String CARDTYPE_TAG_TYPE_NFC = "NFC";
    private static final String CARDTYPE_TAG_TYPE_ISO15693 = "ISO15693";

    private TextView mTextCardType;
    private TextView mTextUid;
    private TextView mTextSak;
    private TextView mTextAtqA;
    private TextView mTextAppData;
    private TextView mTextMaxDataRate;
    private Button mBtnRead;
    private Button mBtnWrite;
    private Button mBtnRawCmd;
    private Button mBtnDisconnect;
    private Button mBtnFormatNDEF;
    private CheckBox mChkNormalTag;
    private CheckBox mChkNDEFTag;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.nfc_software_stack_scan);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.WRAP_CONTENT);
        Elog.e(TAG, "NfcSoftwareStackScan onCreate");
        initUI();
    }

    @Override
    protected void onResume() {
        super.onResume();
        readTagInfo(); // depends on rsp init the UI, such as card type act.
    }

    @Override
    protected void onDestroy() {
        disconnectCard();
        super.onDestroy();
    }

    private void readTagInfo() {
        NfcNativeCallClass.nfc_tag_det_response resp;
        resp =
            (NfcNativeCallClass.nfc_tag_det_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SS_TAG_DECT);
        if (resp == null) {
            Elog.e(TAG, "Take NfcRespMap.KEY_SS_TAG_DECT is null");
            // assert
            this.finish();
            return;
        }
        resp.printMember();

        if (resp.tag_type == TAGTYPE_NORMAL) {
            mChkNormalTag.setChecked(true);
            mChkNDEFTag.setChecked(false);
        } else if (resp.tag_type == TAGTYPE_NDEF) {
            mChkNormalTag.setChecked(false);
            mChkNDEFTag.setChecked(true);
        } else {
            Elog.e(TAG, "Garbarge: tag_type" + resp.tag_type);
        }
        switch (resp.card_type) {
        case CARDTYPE_MC1K_VAL:
            mTextCardType.setText(CARDTYPE_MC1K);
            setBtnSupportState(FUNCTION_SUPPORT_RW);
            break;
        case CARDTYPE_MC4K_VAL:
            mTextCardType.setText(CARDTYPE_MC4K);
            setBtnSupportState(FUNCTION_SUPPORT_RW);
            break;
        case CARDTYPE_NDEF_VAL:
            mTextCardType.setText(CARDTYPE_NDEF);
            setBtnSupportState(FUNCTION_SUPPORT_RW);
            mBtnFormatNDEF.setEnabled(true);
            break;
        case CARDTYPE_TAG_TYPE_ISO1443_4A_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_ISO1443_4A);
            setBtnSupportState(FUNCTION_SUPPORT_RAW);
            break;
        case CARDTYPE_TAG_TYPE_ISO1443_4B_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_ISO1443_4B);
            setBtnSupportState(FUNCTION_SUPPORT_RAW);
            break;
        case CARDTYPE_TAG_TYPE_JEWWL_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_JEWWL);
            setBtnSupportState(FUNCTION_SUPPORT_RAW);
            break;
        case CARDTYPE_TAG_TYPE_FELICA_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_FELICA);
            setBtnSupportState(FUNCTION_SUPPORT_RAW);
            break;
        case CARDTYPE_TAG_TYPE_NFC_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_NFC);
            setBtnSupportState(FUNCTION_SUPPORT_NONE);
            break;
        case CARDTYPE_TAG_TYPE_ISO15693_VAL:
            mTextCardType.setText(CARDTYPE_TAG_TYPE_ISO15693);
            setBtnSupportState(FUNCTION_SUPPORT_NONE);
            break;
        default:
            mTextCardType.setText("Garbarge: card_type" + resp.card_type);
            setBtnSupportState(FUNCTION_SUPPORT_NONE);
        }

        mTextUid.setText(NfcUtils.printArray(resp.uid));
        mTextSak.setText(String.valueOf(resp.sak));
        mTextAtqA.setText(String.valueOf(resp.atag));
        mTextAppData.setText(String.valueOf(resp.appdata));
        mTextMaxDataRate.setText(String.valueOf(resp.maxdatarate));
    }

    private void setBtnSupportState(int functionSupport) {

        // because driver is not ready. Disable all button.
        if (true) {
            functionSupport = FUNCTION_SUPPORT_NONE;
        }
        switch (functionSupport) {
        case FUNCTION_SUPPORT_RW:
            mBtnRead.setEnabled(true);
            mBtnWrite.setEnabled(true);
            mBtnRawCmd.setEnabled(false);
            mBtnDisconnect.setEnabled(true);
            mBtnFormatNDEF.setEnabled(false);
            break;
        case FUNCTION_SUPPORT_RAW:
            mBtnRead.setEnabled(false);
            mBtnWrite.setEnabled(false);
            mBtnRawCmd.setEnabled(true);
            mBtnDisconnect.setEnabled(true);
            mBtnFormatNDEF.setEnabled(false);
            break;
        case FUNCTION_SUPPORT_NONE:
            mBtnRead.setEnabled(false);
            mBtnWrite.setEnabled(false);
            mBtnRawCmd.setEnabled(false);
            mBtnDisconnect.setEnabled(false);
            mBtnFormatNDEF.setEnabled(false);
            break;
        default:
            break;

        }
    }

    private void initUI() {
        mTextCardType =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_CardType);
        mTextUid = (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Uid);
        mTextSak = (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Sak);
        mTextAtqA = (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_AtqA);
        mTextAppData =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_AppData);
        mTextMaxDataRate =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_MaxDataRate);

        mBtnRead = (Button) findViewById(R.id.NFC_SoftwareStack_Scan_BtnRead);
        mBtnWrite = (Button) findViewById(R.id.NFC_SoftwareStack_Scan_BtnWrite);
        mBtnRawCmd =
            (Button) findViewById(R.id.NFC_SoftwareStack_Scan_BtnRawCommand);
        mBtnDisconnect =
            (Button) findViewById(R.id.NFC_SoftwareStack_Scan_BtnDisconnected);
        mBtnFormatNDEF =
            (Button) findViewById(R.id.NFC_SoftwareStack_Scan_BtnFormat);

        mChkNormalTag =
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_TagDect);
        mChkNDEFTag = (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_NDEF);

        mBtnRead.setOnClickListener(this);
        mBtnWrite.setOnClickListener(this);
        mBtnRawCmd.setOnClickListener(this);
        mBtnDisconnect.setOnClickListener(this);
        mBtnFormatNDEF.setOnClickListener(this);

        OnClickListenerSpecial specListener = new OnClickListenerSpecial();
        mChkNormalTag.setOnClickListener(specListener);
        mChkNDEFTag.setOnClickListener(specListener);
    }
    
    /**
     * on click the button set
     * 
     * @param arg0
     *            : clicked which view
     */
    public void onClick(View arg0) {
        Elog.e(TAG, "NfcSoftwareStackScan onClick");
        Intent intent = null;
        if (arg0 == mBtnRead) {
            intent = new Intent();
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackScanReadTag");
        } else if (arg0 == mBtnWrite) {
            intent = new Intent();
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackScanWriteTag"); // no
                                                                               // support
        } else if (arg0 == mBtnRawCmd) {
            intent = new Intent();
            intent.setClassName(this,
                "com.mediatek.engineermode.nfc.NfcSoftwareStackRawCommand"); // no
                                                                             // support
        } else if (arg0.getId() == mBtnDisconnect.getId()) {
            disconnectCard(); // no support
        } else if (arg0.getId() == mBtnFormatNDEF.getId()) {
            formatNdef(); // no support
        } else {
            Elog.e(TAG, "ghost button?");
        }
        if (intent != null) {
            this.startActivity(intent);
        }
    }

    private void disconnectCard() {
        return;
    }

    private void formatNdef() {
        return;
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
