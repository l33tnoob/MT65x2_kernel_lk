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

package com.mediatek.engineermode.wifi;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.nio.charset.Charset;

public class WiFiEeprom extends WiFiTestActivity implements OnClickListener {

    private static final String TAG = "EM/WiFi_EEPROM";

    private EditText mEtWordAddr = null;
    private EditText mEtWorkValue = null;
    private Button mBtnWordRead = null;
    private Button mBtnWordWrite = null;
    private EditText mEtStringAddr = null;
    private EditText mEtStringLength = null;
    private EditText mEtStringValue = null;
    private Button mBtnStringRead = null;
    private Button mBtnStringWrite = null;
    private EditText mEtShowWindow = null;

    private static final int RADIX_16 = 16;
    private static final int DEFAULT_EEPROM_SIZE = 512;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_eeprom);
        mEtWordAddr = (EditText) findViewById(R.id.WiFi_addr_Content);
        mEtWorkValue = (EditText) findViewById(R.id.WiFi_value_Content);
        mBtnWordRead = (Button) findViewById(R.id.WiFi_Read_Word);
        mBtnWordWrite = (Button) findViewById(R.id.WiFi_Write_Word);
        mEtStringAddr = (EditText) findViewById(R.id.WiFi_addr_Content_String);
        mEtStringLength = (EditText) findViewById(R.id.WiFi_length_Content_String);
        mEtStringValue = (EditText) findViewById(R.id.WiFi_value_Content_String);
        mBtnStringRead = (Button) findViewById(R.id.WiFi_Read_String);
        mBtnStringWrite = (Button) findViewById(R.id.WiFi_Write_String);
        mEtShowWindow = (EditText) findViewById(R.id.WiFi_ShowWindow);
        mBtnWordRead.setOnClickListener(this);
        mBtnWordWrite.setOnClickListener(this);
        mBtnStringRead.setOnClickListener(this);
        mBtnStringWrite.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (EMWifi.setEEPRomSize(DEFAULT_EEPROM_SIZE) != 0) {
            Xlog.d(TAG, "initial setEEPRomSize to 512 failed");
        }
    }

    @Override
    public void onClick(View arg0) {
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        long u4Addr = 0;
        long u4Value = 0;
        long u4Length = 0;
        CharSequence inputVal;
        String text;
        if (arg0.getId() == mBtnWordRead.getId()) {
            long[] u4Val = new long[1];
            try {
                u4Addr = Long.parseLong(mEtWordAddr.getText().toString(),
                        RADIX_16);
            } catch (NumberFormatException e) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            EMWifi.readEEPRom16(u4Addr, u4Val);
            mEtWorkValue.setText(Long.toHexString(u4Val[0]));
        } else if (arg0.getId() == mBtnWordWrite.getId()) {
            try {
                u4Addr = Long.parseLong(mEtWordAddr.getText().toString(),
                        RADIX_16);
                u4Value = Long.parseLong(mEtWorkValue.getText().toString(),
                        RADIX_16);
            } catch (NumberFormatException e) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            EMWifi.writeEEPRom16(u4Addr, u4Value);
            EMWifi.setEEPromCKSUpdated();
        } else if (arg0.getId() == mBtnStringRead.getId()) {
            byte[] acSzTmp = new byte[DEFAULT_EEPROM_SIZE];
            try {
                u4Addr = Long.parseLong(mEtStringAddr.getText().toString(),
                        RADIX_16);
                u4Length = Long.parseLong(mEtStringLength.getText().toString());
            } catch (NumberFormatException e) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            if (u4Length == 0) {
                return;
            }
            EMWifi.eepromReadByteStr(u4Addr, u4Length, acSzTmp);
            text = new String(acSzTmp, 0, (int) u4Length * 2, Charset.defaultCharset());
            mEtStringValue.setText(text);
        } else if (arg0.getId() == mBtnStringWrite.getId()) {
            String szTmp;
            inputVal = mEtStringAddr.getText();
            if (TextUtils.isEmpty(inputVal)) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            try {
                u4Addr = Long.parseLong(inputVal.toString(), RADIX_16);
                // u4Length = Long.parseLong(mEtStringLength.getText().toString());
            } catch (NumberFormatException e) {
                Toast.makeText(this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            szTmp = mEtStringValue.getText().toString();
            // szTmp = inputVal.toString();
            int len = szTmp.length();
            if ((len == 0) || (len % 2 == 1)) {
                mEtShowWindow.append("Byte string length error:" + len
                        + "bytes\n");
                return;
            }
            EMWifi.eepromWriteByteStr(u4Addr, (len / 2), szTmp);
            EMWifi.setEEPromCKSUpdated();
        } else {
            Xlog.v(TAG, "unknown button");
        }
    }
}
