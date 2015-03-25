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
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class WiFiMcr extends WiFiTestActivity implements OnClickListener {

    private static final String TAG = "EM/WiFi_MCR";
    private static final int RADIX = 16;
    private static final int ADDRESS_ALIGN = 4;

    private EditText mEtAddr = null;
    private EditText mEtValue = null;
    private Button mBtnRead = null;
    private Button mBtnWrite = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wifi_mcr);
        mEtAddr = (EditText) findViewById(R.id.WiFi_MCR_Addr_Content);
        mEtValue = (EditText) findViewById(R.id.WiFi_MCR_Value_Content);
        mBtnRead = (Button) findViewById(R.id.WiFi_MCR_ReadBtn);
        mBtnWrite = (Button) findViewById(R.id.WiFi_MCR_WriteBtn);
        mBtnRead.setOnClickListener(this);
        mBtnWrite.setOnClickListener(this);
    }

    @Override
    public void onClick(View arg0) {
        if (!EMWifi.sIsInitialed) {
            showDialog(DIALOG_WIFI_ERROR);
            return;
        }
        if (arg0.getId() == mBtnRead.getId()) {
            Xlog.v(TAG, "read clicked");
            long u4Addr = -1;
            long[] u4Value = new long[1];
            try {
                u4Addr = Long.parseLong(mEtAddr.getText().toString(), RADIX);
                if (!isAddrAvalible(u4Addr)) {
                    throw(new NumberFormatException());
                }
            } catch (NumberFormatException e) {
                Toast.makeText(WiFiMcr.this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            EMWifi.readMCR32(u4Addr, u4Value);
            String result = String.format("%1$08x", u4Value[0]);
            mEtValue.setText(result);
        } else if (arg0.getId() == mBtnWrite.getId()) {
            Xlog.v(TAG, "write clicked");
            long u4Addr = -1;
            long u4Value = -1;
            try {
                u4Addr = Long.parseLong(mEtAddr.getText().toString(), RADIX);
                if (!isAddrAvalible(u4Addr)) {
                    throw(new NumberFormatException());
                }
                u4Value = Long.parseLong(mEtValue.getText().toString(), RADIX);
            } catch (NumberFormatException e) {
                Toast.makeText(WiFiMcr.this, "invalid input value",
                        Toast.LENGTH_SHORT).show();
                return;
            }
            EMWifi.writeMCR32(u4Addr, u4Value);
        }
    }

    private boolean isAddrAvalible(long u4Addr) {
        return (0 == u4Addr % ADDRESS_ALIGN);
    }
}
