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

package com.mediatek.engineermode.baseband;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;

public class Baseband extends Activity implements OnClickListener {

    private static final String TAG = "EM-Baseband";
    private static final int RADIX_LENGTH_SIXTEEN = 16;
    private static final int RADIX_LENGTH_TEN = 10;
    private static final int MAX_VALUE = 1024;
    private static final int PARA_NUM = 4;
    private static final int READ = 0;
    private static final int WRITE = 1;
    private Button mBtnRead;
    private Button mBtnWrite;
    private EditText mEditAddr;
    private EditText mEditLen;
    private EditText mEditVal;

    private TextView mInfo;
    private AFMFunctionCallEx mFmFunctionCallEx;

    @Override
    public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.baseband);

    mBtnRead = (Button) findViewById(R.id.Baseband_Read);
    mBtnWrite = (Button) findViewById(R.id.Baseband_Write);
    mEditAddr = (EditText) findViewById(R.id.Baseband_Addr);
    mEditLen = (EditText) findViewById(R.id.Baseband_Len);
    mEditVal = (EditText) findViewById(R.id.Baseband_Val);

    mInfo = (TextView) findViewById(R.id.Baseband_Info);

    mBtnRead.setOnClickListener(this);
    mBtnWrite.setOnClickListener(this);

    }

    /**
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */

    public void onClick(View arg0) {

        String addrString = mEditAddr.getText().toString();
        String lenString = mEditLen.getText().toString();
        String valString = mEditVal.getText().toString();

    if (arg0.getId() == mBtnRead.getId()) {

        boolean result0 = checkValue(addrString, lenString, valString);
        if (!result0) {
            Toast.makeText(this, R.string.number_format_check,
                    Toast.LENGTH_LONG).show();
            return;
        }
        boolean result = functionCall(READ, addrString, lenString, valString);
        if (!result) {
            Toast.makeText(this, R.string.pipe_check, Toast.LENGTH_LONG).show();
            return;
        }
        updateUI();
    } else if (arg0.getId() == mBtnWrite.getId()) {
        boolean result0 = checkValue(addrString, lenString, valString);
        if (!result0) {
            Toast.makeText(this, R.string.length_check, Toast.LENGTH_LONG)
                    .show();
            return;
        }
        boolean result = functionCall(WRITE, addrString, lenString, valString);

        if (!result) {
            Toast.makeText(this, R.string.pipe_check, Toast.LENGTH_LONG).show();

            return;
        }
        updateUI();
    }

    }

    /**
     * @param flag
     *            the integer of READ or WRITE
     * @param address
     *            the String of the start address
     * @param len
     *            the String of the address length
     * @param value
     *            the String of the value to address
     * @return the boolean of true or false
     */
    public boolean functionCall(int flag, String address, String len,
            String value) {
    mFmFunctionCallEx = new AFMFunctionCallEx();
    boolean result = mFmFunctionCallEx
            .startCallFunctionStringReturn(AFMFunctionCallEx.FUNCTION_EM_BASEBAND);
    mFmFunctionCallEx.writeParamNo(PARA_NUM);
    if (flag == READ) {
        mFmFunctionCallEx.writeParamString("r");
    } else {
        mFmFunctionCallEx.writeParamString("w");
    }
    mFmFunctionCallEx.writeParamString(address);
    mFmFunctionCallEx.writeParamString(len);
    if (flag == READ) {
        mFmFunctionCallEx.writeParamString("0");
    } else {
        mFmFunctionCallEx.writeParamString(value);
    }
    return result;
    }

    /**
     * @param addrString
     *            the String of address
     * @param lenString
     *            the String of length
     * @param valString
     *            the String of Value
     * @return the true or false if the value is null
     */
    public boolean checkValue(String addrString, String lenString, String valString) {
        if (null == addrString || null == lenString) {
            return false;
        }

        long lenValue = 0;
        try {
            Long.parseLong(addrString, RADIX_LENGTH_SIXTEEN);
            lenValue = Long.parseLong(lenString, RADIX_LENGTH_TEN);
            Long.parseLong(valString, RADIX_LENGTH_SIXTEEN);
        } catch (NumberFormatException e) {
            return false;
        }
        if (lenValue <= 0 || lenValue > MAX_VALUE) {
            return false;
        }
        return true;
    }

    /**
     * The Function is to make the result to show
     */
    private void updateUI() {
    FunctionReturn r;
    do {
        r = mFmFunctionCallEx.getNextResult();
        if ("".equals(r.mReturnString)) {
            break;
        }
        mInfo.setText(r.mReturnString);

    } while (r.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
    if (r.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {

        mInfo.setText(R.string.information);
    }
    }
}
