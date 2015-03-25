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

package com.mediatek.engineermode.cpustress;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class ApMcu extends CpuStressCommon implements OnClickListener, OnCheckedChangeListener {

    private static final String TAG = "EM/CpuStress_ApMcu";
    public static final int INDEX_NEON = 0;
    public static final int INDEX_CA9 = 1;
    public static final int INDEX_DHRY = 2;
    public static final int INDEX_MEMCPY = 3;
    public static final int INDEX_FDCT = 4;
    public static final int INDEX_IMDCT = 5;

    public static final int MASK_NEON_0 = 0;
    public static final int MASK_CA9_0 = 8;
    public static final int MASK_DHRY_0 = 16;
    public static final int MASK_MEMCPY_0 = 24;
    public static final int MASK_FDCT_0 = 32;
    public static final int MASK_IMDCT_0 = 40;

    private static final double PERCENT = 100.0;
    private static final int TEST_ITEM = 6;
    private static final String MAXPOWER_TYPE_9 = "CA9";
    private static final String MAXPOWER_TYPE_7 = "CA7";
    private EditText mEtLoopCount = null;
    private CheckBox[] mCbArray = new CheckBox[TEST_ITEM];
    private TextView[] mTvArray = new TextView[TEST_ITEM * CpuStressTestService.CORE_NUMBER_8];
    private Button mBtnStart = null;
    private TextView mTvResult = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_cpustress_apmcu);
        mEtLoopCount = (EditText) findViewById(R.id.apmcu_loopcount);
        mCbArray[INDEX_NEON]   = (CheckBox) findViewById(R.id.apmcu_neon_test);
        mCbArray[INDEX_CA9]    = (CheckBox) findViewById(R.id.apmcu_ca9_test);
        mCbArray[INDEX_DHRY]   = (CheckBox) findViewById(R.id.apmcu_dhrystone_test);
        mCbArray[INDEX_MEMCPY] = (CheckBox) findViewById(R.id.apmcu_memcpy_test);
        mCbArray[INDEX_FDCT]   = (CheckBox) findViewById(R.id.apmcu_fdct_test);
        mCbArray[INDEX_IMDCT]  = (CheckBox) findViewById(R.id.apmcu_imdct_test);

        if (!(CpuStressTest.INDEX_OCTA == CpuStressTestService.sIndexMode ||
                (CpuStressTest.INDEX_TEST == CpuStressTestService.sIndexMode &&
                CpuStressTestService.CORE_NUMBER_8 == CpuStressTestService.sCoreNumber))) {
            findViewById(R.id.apmcu_neon_result_octa).setVisibility(View.GONE);
            findViewById(R.id.apmcu_ca9_result_octa).setVisibility(View.GONE);
            findViewById(R.id.apmcu_dhrystone_result_octa).setVisibility(View.GONE);
            findViewById(R.id.apmcu_memcpy_result_octa).setVisibility(View.GONE);
            findViewById(R.id.apmcu_fdct_result_octa).setVisibility(View.GONE);
            findViewById(R.id.apmcu_imdct_result_octa).setVisibility(View.GONE);
        }

        String[] textViewIds = {"apmcu_neon_result", "apmcu_ca9_result", "apmcu_dhrystone_result", 
                "apmcu_memcpy_result", "apmcu_fdct_result", "apmcu_imdct_result"};
        for (int group = 0; group < TEST_ITEM; group++) {
            for (int column = 0; column < CpuStressTestService.CORE_NUMBER_8; column++) {
                // Id is like "R.id.apmcu_neon_result_1", see hqa_cpustree_apmcu.xml
                int id = getResources().getIdentifier(textViewIds[group] + "_" + column, "id", getPackageName());
                if (findViewById(id) == null) {
                    throw new RuntimeException("Check the text view id!");
                }
                mTvArray[group * CpuStressTestService.CORE_NUMBER_8 + column] = (TextView) findViewById(id);
            }
        }
        mBtnStart = (Button) findViewById(R.id.apmcu_btn);
        mTvResult = (TextView) findViewById(R.id.apmcu_result);
        for (int i = 0; i < TEST_ITEM; i++) {
            mCbArray[i].setOnCheckedChangeListener(this);
        }
        mBtnStart.setOnClickListener(this);
        if (ChipSupport.MTK_6589_SUPPORT > ChipSupport.getChip()) {
            // Hide DHRY test, Memcpy test, FDCT test and IMDCT test
            for (int i = MASK_DHRY_0; i < mTvArray.length; i++) {
                mTvArray[i].setVisibility(View.GONE);
            }
            for (int i = INDEX_DHRY; i < mCbArray.length; i++) {
                mCbArray[i].setVisibility(View.GONE);
            }
        } else {
            // Show VFP test and CA7 test
            mCbArray[INDEX_CA9].setText(mCbArray[1].getText().toString().replaceAll(
                    MAXPOWER_TYPE_9, MAXPOWER_TYPE_7));
            mCbArray[INDEX_NEON].setText(getString(R.string.hqa_cpustress_apmcu_vfp));
        }
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                Xlog.v(TAG, "mHandler receive message: " + msg.what);
                if (INDEX_UPDATE_RADIOBTN == msg.what || INDEX_UPDATE_RADIOGROUP == msg.what) {
                    updateTestResult();
                }
            }
        };
    }

    /**
     * Update apmcu test status
     */
    private void updateTestResult() {
        Xlog.v(TAG, "Enter updateTestResult");
        try {
            Bundle data = mBoundService.getData();
            if (null != data) {
                // Get test results from test service. See also updateTestUi(), updateTestCount()
                int timesNeon = data.getInt(CpuStressTestService.RESULT_NEON);
                int timesCa9 = data.getInt(CpuStressTestService.RESULT_CA9);
                int timesDhry = data.getInt(CpuStressTestService.RESULT_DHRY);
                int timesMemcpy = data.getInt(CpuStressTestService.RESULT_MEMCPY);
                int timesFdct = data.getInt(CpuStressTestService.RESULT_FDCT);
                int timesImdct = data.getInt(CpuStressTestService.RESULT_IMDCT);
                updateTestUi(data.getBoolean(CpuStressTestService.VALUE_RUN),
                        data.getLong(CpuStressTestService.VALUE_LOOPCOUNT),
                        data.getInt(CpuStressTestService.VALUE_MASK),
                        data.getLong(CpuStressTestService.VALUE_RESULT),
                        timesNeon, timesCa9, timesDhry, timesMemcpy, timesFdct, timesImdct);
                updateTestCount(timesNeon, data.getInt(CpuStressTestService.RESULT_PASS_NEON),
                        timesCa9, data.getInt(CpuStressTestService.RESULT_PASS_CA9),
                        timesDhry, data.getInt(CpuStressTestService.RESULT_PASS_DHRY),
                        timesMemcpy, data.getInt(CpuStressTestService.RESULT_PASS_MEMCPY),
                        timesFdct, data.getInt(CpuStressTestService.RESULT_PASS_FDCT),
                        timesImdct, data.getInt(CpuStressTestService.RESULT_PASS_IMDCT));
            }
        } catch (NullPointerException e) {
            Xlog.w(TAG, "updateTestUI NullPointerException: " + e.getMessage());
        }
    }

    /**
     * Update component status by test result
     * 
     * @param bRun
     *            Apmcu test running or not
     * @param iCount
     *            Test loop count
     * @param mask
     *            Test items mask
     * @param result
     *            Test result
     * @param timesN
     *            NEON test total count
     * @param timesC
     *            CA9 test total count
     * @param timesD
     *            DHRY test total count
     * @param timesM
     *            MEMCPY test total count
     *@param timesF
     *            FDCT test total count
     * @param timesI
     *            IMDCT test total count
     */
    private void updateTestUi(boolean bRun, long iCount, int mask, long result,
            int timesN, int timesC, int timesD, int timesM, int timesF,
            int timesI) {
        Xlog.v(TAG, "updateTestUI: " + bRun + " " + iCount + " " + mask + " 0x"
                + Long.toHexString(result));
        mEtLoopCount.setEnabled(!bRun);
        mEtLoopCount.setText(String.valueOf(iCount));
        mEtLoopCount.setSelection(mEtLoopCount.getText().length());
        mBtnStart.setText(bRun ? R.string.hqa_cpustress_apmcu_stop
                : R.string.hqa_cpustress_apmcu_start);
        updateCbStatus(mask);
        if (timesN > 0 || timesC > 0 || timesD > 0 || timesM > 0 || timesF > 0
                || timesI > 0) {
            showTestResult(CpuStressTestService.sIndexMode, result);
        }
        // Clear text views of unchecked items
        for (int i = 0; i < TEST_ITEM; i++) {
            if (!bRun || !mCbArray[i].isChecked()) {
                for (int j = 0; j < CpuStressTestService.CORE_NUMBER_8; j++) {
                    mTvArray[i * CpuStressTestService.CORE_NUMBER_8 + j]
                            .setText(R.string.hqa_cpustress_result);
                }
            }
        }
        if (!mBoundService.mWantStopApmcu) {
            removeDialog(DIALOG_WAIT);
        }
    }

    private void showTestResult(int indexMode, long result) {
        int index = 0;
        switch (indexMode) {
        case CpuStressTest.INDEX_SINGLE:
            index = 0;
            break;
        case CpuStressTest.INDEX_DUAL:
            index = 1;
            break;
        case CpuStressTest.INDEX_TRIPLE:
            index = 2;
            break;
        case CpuStressTest.INDEX_QUAD:
            index = 3;
            break;
        case CpuStressTest.INDEX_OCTA:
            index = 7;
            break;
        case CpuStressTest.INDEX_TEST:
            if (CpuStressTestService.CORE_NUMBER_8 == CpuStressTestService.sCoreNumber) {
                index = 7;
            } else if (CpuStressTestService.CORE_NUMBER_4 == CpuStressTestService.sCoreNumber) {
                index = 3;
            } else if (CpuStressTestService.CORE_NUMBER_2 == CpuStressTestService.sCoreNumber) {
                index = 1;
            } else {
                index = 0;
            }
            break;
        default:
            break;
        }

        for (int i = 0; i <= index; i++) {
            for (int j = 0; j < TEST_ITEM; j++) {
                mTvArray[j * CpuStressTestService.CORE_NUMBER_8 + i]
                        .setText(0 == (result & 1 << (j * 8 + i)) ? R.string.hqa_cpustress_result_fail
                                : R.string.hqa_cpustress_result_pass);
            }
        }
    }

    private void updateCbStatus(int mask) {
        for (int i = 0; i < TEST_ITEM; i++) {
            mCbArray[i].setEnabled(false);  // Don't callback onCheckedChanged()
            mCbArray[i].setChecked(0 != (mask & (1 << i)));
            mCbArray[i].setEnabled(true);
        }
    }

    /**
     * Update test result by percent
     * 
     * @param neonTest
     *            NEON test total count
     * @param neonTestPass
     *            NEON test pass count
     * @param maxPowerTest
     *            MaxPower test total count
     * @param maxPowerTestPass
     *            MaxPower test pass count
     * @param dhryTest
     *            Dhry test total count
     * @param dhryTestPass
     *            Dhry test pass count
     * @param memcpyTest
     *            MEMCPY test total count
     * @param memcpyTestPass
     *            MEMCPY test pass count
     * @param fdctTest
     *            FDCT test total count
     * @param fdctTestPass
     *            FDCT test pass count
     * @param imdctTest
     *            IMDCT test total count
     * @param imdctTestPass
     *            IMDCT test pass count
     */
    private void updateTestCount(int neonTest, int neonTestPass,
            int maxPowerTest, int maxPowerTestPass, int dhryTest,
            int dhryTestPass, int memcpyTest, int memcpyTestPass, int fdctTest,
            int fdctTestPass, int imdctTest, int imdctTestPass) {
        StringBuffer sb = new StringBuffer();
        if (0 != neonTest) {
            if (CpuStressTestService.CORE_NUMBER_4 <= CpuStressTestService.sCoreNumber) {
                sb.append(String.format(
                        getString(R.string.hqa_cpustress_apmcu_result_vfp),
                        neonTestPass, neonTest, neonTestPass * PERCENT
                                / neonTest));
            } else {
                sb.append(String.format(
                        getString(R.string.hqa_cpustress_apmcu_result_neon),
                        neonTestPass, neonTest, neonTestPass * PERCENT
                                / neonTest));
            }
            sb.append('\t');
        }
        if (0 != maxPowerTest) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_apmcu_result_ca9),
                    maxPowerTestPass, maxPowerTest, maxPowerTestPass * PERCENT
                            / maxPowerTest));
            sb.append('\t');
        }
        if (0 != dhryTest) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_apmcu_result_dhry),
                    dhryTestPass, dhryTest, dhryTestPass * PERCENT / dhryTest));
            sb.append('\t');
        }
        if (0 != memcpyTest) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_apmcu_result_memcpy),
                    memcpyTestPass, memcpyTest, memcpyTestPass * PERCENT
                            / memcpyTest));
            sb.append('\t');
        }
        if (0 != fdctTest) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_apmcu_result_fdct),
                    fdctTestPass, fdctTest, fdctTestPass * PERCENT / fdctTest));
            sb.append('\t');
        }
        if (0 != imdctTest) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_apmcu_result_imdct),
                    imdctTestPass, imdctTest, imdctTestPass * PERCENT
                            / imdctTest));
            sb.append('\t');
        }
        Xlog.v(TAG, "test result: " + sb.toString());
        mTvResult.setText(sb.toString());
    }

    @Override
    public void onClick(View arg0) {
        if (mBtnStart.getId() == arg0.getId()) {
            Xlog.v(TAG, mBtnStart.getText() + " is clicked");
            if (getResources().getString(R.string.hqa_cpustress_apmcu_start)
                    .equals(mBtnStart.getText())) {
                // Start test
                long count = 0;
                try {
                    count = Long.valueOf(mEtLoopCount.getText().toString());
                } catch (NumberFormatException nfe) {
                    Xlog.d(TAG, "Loopcount string: " + mEtLoopCount.getText()
                            + nfe.getMessage());
                    Toast.makeText(this,
                            R.string.hqa_cpustress_toast_loopcount_error,
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                Bundle data = new Bundle();
                data.putLong(CpuStressTestService.VALUE_LOOPCOUNT, count);
                int cbMask = 0;
                for (int i = 0; i < TEST_ITEM; i++) {
                    cbMask |= mCbArray[i].isChecked() ? 1 << i : 0;
                }
                data.putInt(CpuStressTestService.VALUE_MASK, cbMask);
                mBoundService.startTest(data);
                updateStartUi();
            } else {
                // Stop test
                showDialog(DIALOG_WAIT);
                mBoundService.stopTest();
            }
        } else {
            Xlog.v(TAG, "Unknown event");
        }
    }

    /**
     * Update test result on UI
     */
    private void updateStartUi() {
        mEtLoopCount.setEnabled(false);
        mBtnStart.setText(R.string.hqa_cpustress_apmcu_stop);
        for (int i = 0; i < mTvArray.length; i++) {
            mTvArray[i].setText(R.string.hqa_cpustress_result);
        }
        mTvResult.setText(R.string.hqa_cpustress_result);
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.isEnabled()) {
            int cbMask = 0;
            for (int i = 0; i < TEST_ITEM; i++) {
                cbMask |= mCbArray[i].isChecked() ? 1 << i : 0;
            }
            Xlog.v(TAG, "onCheckChanged, mask: 0x"
                    + Integer.toHexString(cbMask));
            Bundle data = new Bundle();
            data.putInt(CpuStressTestService.VALUE_MASK, cbMask);
            mBoundService.updateData(data);
        }
    }
}
