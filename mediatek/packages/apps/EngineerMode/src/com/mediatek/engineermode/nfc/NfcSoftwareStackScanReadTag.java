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
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcSoftwareStackScanReadTag extends Activity
        implements
        OnClickListener {

    public static final String TAG = "EM/nfc";
    public static final int TAG_TYPE_NONE = 0;
    public static final int TAG_TYPE_NDEF = 1;
    public static final int TAG_TYPE_MC1K = 2;
    public static final int TAG_TYPE_MC4K = 3;
    public static final int NUM_3 = 3;
    private TextView mTextLang;
    private TextView mTextRecFlags;
    private TextView mTextRecId;
    private TextView mTextRecTnf;
    private TextView mTextPayloadLen;
    private EditText mEditPayloadHex;
    private EditText mEditPayloadASCII;
    private EditText mEditPageAddress;
    private EditText mEditPageI;
    private EditText mEditPageII;
    private EditText mEditPageIII;
    private EditText mEditPageIV;
    private EditText mEditSector;
    private EditText mEditBlock;
    private EditText mEditSectorInfo;

    private Button mBtn1KOK;
    private Button mBtn4KOK;
    private int mCurrentTag = TAG_TYPE_NONE;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        Elog.i(TAG, "NfcSoftwareStackScanReadTag onCreate");
        setContentView(R.layout.nfc_software_stack_scan_read_ndef);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
            WindowManager.LayoutParams.WRAP_CONTENT);
        initUI();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mCurrentTag == TAG_TYPE_NDEF) {
            readNDEF();
        }
    }

    /**
     * on click the button ok
     * 
     * @param arg0
     *            : clicked which view
     */
    public void onClick(View arg0) {
        Elog.e(TAG, "NfcSoftwareStackScanReadNDEF onClick");
        if (arg0 == mBtn1KOK) {
            String s = mEditPageAddress.getText().toString();
            if (s == null || s.equals("")) {
                Toast.makeText(getApplicationContext(), R.string.addr_need,
                    Toast.LENGTH_LONG).show();
                return;
            }
            readMC1K();
        }
        if (arg0 == mBtn4KOK) {
            String s = mEditBlock.getText().toString();
            String ss = mEditSector.getText().toString();
            if (s == null || s.equals("") || ss == null || ss.equals("")) {
                Toast.makeText(getApplicationContext(),
                    R.string.block_sector_need, Toast.LENGTH_LONG).show();
                return;
            }
            readMC4K();
        }

    }

    private void initUI() {
        mTextLang =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Read_TEXT_Lang);
        mTextRecFlags =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Read_RecFlags);
        mTextRecId =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Read_RecId);
        mTextRecTnf =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Read_RecTnf);
        mTextPayloadLen =
            (TextView) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PayloadLen);
        mEditPayloadHex =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PayloadHex);
        mEditPayloadASCII =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PayloadASCII);
        mEditPageAddress =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PageAddr);
        mEditPageI =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PageI);
        mEditPageII =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PageII);
        mEditPageIII =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PageIII);
        mEditPageIV =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_PageIV);
        mEditSector =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_SectorNo);
        mEditBlock =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_BlockNo);
        mEditSectorInfo =
            (EditText) findViewById(R.id.NFC_SoftwareStack_Scan_Read_SectorInfo);
        mBtn1KOK =
            (Button) findViewById(R.id.NFC_SoftwareStack_Scan_Read_Btn1KOK);
        mBtn4KOK =
            (Button) findViewById(R.id.NFC_SoftwareStack_Scan_Read_Btn4KOK);

        mTagTypeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_Read_URI), 0));
        mTagTypeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_Read_TEXT), 1));
        mTagTypeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_Read_SmartPos),
            2));
        mTagTypeArray.add(new ModeMap(
            (CheckBox) findViewById(R.id.NFC_SoftwareStack_Scan_Read_Other),
            NUM_3));

        OnClickListenerSpecial specListener = new OnClickListenerSpecial();
        for (ModeMap m : mTagTypeArray) {
            m.mChkBox.setOnClickListener(specListener);
        }

        mBtn1KOK.setOnClickListener(this);

        selectView();
    }

    private void selectView() {
        View viewTag = (View) findViewById(R.id.View_Tag);
        View viewPage = (View) findViewById(R.id.View_Page);
        View viewSector = (View) findViewById(R.id.View_Sector);

        NfcNativeCallClass.nfc_dis_notif_response resp;
        resp =
            (NfcNativeCallClass.nfc_dis_notif_response) NfcRespMap.getInst()
                .take(NfcRespMap.KEY_SS_SCAN_COMPLETE);
        if (resp == null) {
            Elog.e(TAG,
                "selectView(): Take NfcRespMap.KEY_SS_SCAN_COMPLETE is null");
            // assert
            NfcSoftwareStackScanReadTag.this.finish();
            return;
        }
        int cardType =
            ((NfcNativeCallClass.nfc_tag_det_response) resp.target).card_type;
        switch (cardType) {
        case NfcSoftwareStackScan.CARDTYPE_MC1K_VAL:
            viewTag.setVisibility(View.GONE);
            viewSector.setVisibility(View.GONE);
            mCurrentTag = TAG_TYPE_MC1K;
            break;
        case NfcSoftwareStackScan.CARDTYPE_MC4K_VAL:
            viewTag.setVisibility(View.GONE);
            viewPage.setVisibility(View.GONE);
            mCurrentTag = TAG_TYPE_MC4K;
            break;
        case NfcSoftwareStackScan.CARDTYPE_NDEF_VAL:
            viewPage.setVisibility(View.GONE);
            viewSector.setVisibility(View.GONE);
            mCurrentTag = TAG_TYPE_NDEF;
            break;
        default:
            Elog.e(TAG, "selectView() garbage card_type:" + cardType);
            NfcSoftwareStackScanReadTag.this.finish();
            break;
        }
    }

    private int readMC1K() {
        NfcNativeCallClass.nfc_tag_read_request req =
            new NfcNativeCallClass.nfc_tag_read_request();
        req.read_type = NfcSoftwareStackScan.CARDTYPE_MC1K_VAL;
        req.address = Integer.valueOf(mEditPageAddress.getText().toString());

        NfcNativeCallClass.nfc_tag_read_response resp;
        resp = NfcNativeCallClass.readTag(req);
        if (resp == null || resp.status != 0) {
            return NfcCommonDef.ERROR_ERROR;
        }
        int address =
            ((NfcNativeCallClass.nfc_tag_read_Mifare1K) (resp.target)).address;
        mEditPageAddress.setText(String.valueOf(address));

        short[] data =
            ((NfcNativeCallClass.nfc_tag_read_Mifare1K) (resp.target)).data;
        mEditPageI.setText(NfcUtils.printArray(data));

        return NfcCommonDef.ERROR_OK;
    }

    private int readMC4K() {
        NfcNativeCallClass.nfc_tag_read_request req =
            new NfcNativeCallClass.nfc_tag_read_request();
        req.read_type = NfcSoftwareStackScan.CARDTYPE_MC4K_VAL;
        req.block = Integer.valueOf(mEditBlock.getText().toString());
        req.sector = Integer.valueOf(mEditSector.getText().toString());

        NfcNativeCallClass.nfc_tag_read_response resp;
        resp = NfcNativeCallClass.readTag(req);
        if (resp == null || resp.status != 0) {
            return NfcCommonDef.ERROR_ERROR;
        }

        int block =
            ((NfcNativeCallClass.nfc_tag_read_Mifare4K) (resp.target)).block;
        mEditBlock.setText(String.valueOf(block));
        int sector =
            ((NfcNativeCallClass.nfc_tag_read_Mifare4K) (resp.target)).sector;
        mEditSector.setText(String.valueOf(sector));

        short[] data =
            ((NfcNativeCallClass.nfc_tag_read_Mifare4K) (resp.target)).data;
        mEditSectorInfo.setText(NfcUtils.printArray(data));

        return NfcCommonDef.ERROR_OK;

    }

    private int readNDEF() {
        NfcNativeCallClass.nfc_tag_read_request req =
            new NfcNativeCallClass.nfc_tag_read_request();
        req.read_type = NfcSoftwareStackScan.CARDTYPE_NDEF_VAL;

        NfcNativeCallClass.nfc_tag_read_response resp;
        resp = NfcNativeCallClass.readTag(req);
        if (resp == null || resp.status != 0) {
            return NfcCommonDef.ERROR_ERROR;
        }

        int ndefType =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).ndef_type;
        int recordFlage =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).recordFlage;
        int recordId =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).recordId;
        int recordInfo =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).recordInfo;
        int length =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).length;

        setCurrentMode(ndefType);
        mTextRecFlags.setText(String.valueOf(recordFlage));
        mTextRecId.setText(String.valueOf(recordId));
        mTextRecTnf.setText(String.valueOf(recordInfo));
        mTextPayloadLen.setText(String.valueOf(length));

        short[] dataHex =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).dataHex;
        char[] dataAscii =
            ((NfcNativeCallClass.nfc_tag_read_ndef) (resp.target)).dataAscii;
        mEditPayloadASCII.setText(new String(dataAscii));
        mEditPayloadHex.setText(NfcUtils.printArray(dataHex));
        return NfcCommonDef.ERROR_OK;
    }

    private void setCurrentMode(int tagTypeVal) {
        for (ModeMap m : mTagTypeArray) {
            if ((tagTypeVal & (1 << m.mBit)) != 0) {
                if (m.mChkBox.isEnabled()) {
                    m.mChkBox.setChecked(true);
                }
            } else {
                m.mChkBox.setChecked(false);
            }
        }

    }

    private ArrayList<ModeMap> mTagTypeArray = new ArrayList<ModeMap>();

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
