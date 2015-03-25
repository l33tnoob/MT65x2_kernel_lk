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
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class SwVideoCodec extends CpuStressCommon implements OnClickListener {

    private static final String TAG = "EM/CpuStress_SwVideoCodec";
    private static final double PERCENT = 100.0;
    private Button mBtnStart = null;
    private EditText mEtLoopCount = null;
    private EditText mEtIteration = null;
    private TextView[] mTvArray = new TextView[CpuStressTestService.CORE_NUMBER_8];
    private TextView mTvResult = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_cpustress_swvideo);
        mBtnStart = (Button) findViewById(R.id.swvideo_btn);
        mEtLoopCount = (EditText) findViewById(R.id.swvideo_loopcount);
        mEtIteration = (EditText) findViewById(R.id.swvideo_iteration);
        mTvArray[0] = (TextView) findViewById(R.id.swvideo_iteration_result);
        mTvArray[1] = (TextView) findViewById(R.id.swvideo_iteration_result_1);
        mTvArray[2] = (TextView) findViewById(R.id.swvideo_iteration_result_2);
        mTvArray[3] = (TextView) findViewById(R.id.swvideo_iteration_result_3);
        mTvArray[4] = (TextView) findViewById(R.id.swvideo_iteration_result_4);
        mTvArray[5] = (TextView) findViewById(R.id.swvideo_iteration_result_5);
        mTvArray[6] = (TextView) findViewById(R.id.swvideo_iteration_result_6);
        mTvArray[7] = (TextView) findViewById(R.id.swvideo_iteration_result_7);
        mTvResult = (TextView) findViewById(R.id.swvideo_result);
        mBtnStart.setOnClickListener(this);
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
     * Update video codec test status
     */
    private void updateTestResult() {
        Xlog.v(TAG, "Enter updateTestResult");
        Bundle data = mBoundService.getData();
        if (null != data) {
            int times = data.getInt(CpuStressTestService.RESULT_VIDEOCODEC);
            updateTestUi(data.getBoolean(CpuStressTestService.VALUE_RUN),
                    data.getLong(CpuStressTestService.VALUE_LOOPCOUNT),
                    data.getInt(CpuStressTestService.VALUE_ITERATION),
                    data.getInt(CpuStressTestService.VALUE_RESULT), times);
            updateTestCount(times, data.getInt(CpuStressTestService.RESULT_PASS_VIDEOCODEC));
        }
    }

    /**
     * Update component status by test result
     * 
     * @param bRun
     *            Video codec test running or not
     * @param iCount
     *            Test loop count
     * @param iItera
     *            Test iterator
     * @param result
     *            Test result
     * @param times
     *            Test times
     */
    private void updateTestUi(boolean bRun, long iCount, int iItera,
            int result, int times) {
        Xlog.v(TAG, "updateTestUI: " + bRun + " " + iCount + " 0x"
                + Integer.toHexString(result));
        mEtLoopCount.setEnabled(!bRun);
        mEtLoopCount.setText(iCount + "");
        mEtIteration.setEnabled(!bRun);
        mEtIteration.setText(iItera + "");
        mEtLoopCount.setSelection(mEtLoopCount.getText().length());
        if (times > 0) {
            showTestResult(CpuStressTestService.sIndexMode, result);
        }
        mBtnStart.setText(bRun ? R.string.hqa_cpustress_swvideo_stop
                : R.string.hqa_cpustress_swvideo_start);
        if (!bRun) {
            for (int i = 0; i < mTvArray.length; i++) {
                mTvArray[i].setText(R.string.hqa_cpustress_result);
            }
        }
        if (!mBoundService.mWantStopSwCodec) {
            removeDialog(DIALOG_WAIT);
        }
    }

    private void showTestResult(int indexMode, int result) {
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
            mTvArray[i].setText(0 == (result & 1 << i) ? R.string.hqa_cpustress_result_fail
                    : R.string.hqa_cpustress_result_pass);
        }
    }

    /**
     * Update test result by percent
     * 
     * @param total
     *            Test pass count
     * @param pass
     *            Test pass count
     */
    private void updateTestCount(int total, int pass) {
        Xlog.v(TAG, "Enter updateTestResult: " + total + " " + pass);
        StringBuffer sb = new StringBuffer();
        if (0 != total) {
            sb.append(String.format(
                    getString(R.string.hqa_cpustress_codec_result), pass,
                    total, pass * PERCENT / total));
        }
        mTvResult.setText(sb.toString());
    }

    @Override
    public void onClick(View arg0) {
        if (mBtnStart.getId() == arg0.getId()) {
            Xlog.v(TAG, mBtnStart.getText() + " is clicked");
            if (getResources().getString(R.string.hqa_cpustress_swvideo_start)
                    .equals(mBtnStart.getText())) {
                long count = 0;
                int iteration = 0;
                try {
                    count = Long.valueOf(mEtLoopCount.getText().toString());
                    iteration = Integer.valueOf(mEtIteration.getText()
                            .toString());
                } catch (NumberFormatException nfe) {
                    Xlog.d(TAG, nfe.getMessage());
                    Toast.makeText(this,
                            R.string.hqa_cpustress_toast_loopcount_error,
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                Bundle data = new Bundle();
                data.putLong(CpuStressTestService.VALUE_LOOPCOUNT, count);
                data.putInt(CpuStressTestService.VALUE_ITERATION, iteration);
                mBoundService.startTest(data);
                updateStartUI();
            } else {
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
    private void updateStartUI() {
        for (int i = 0; i < mTvArray.length; i++) {
            mTvArray[i].setText(R.string.hqa_cpustress_result);
        }
        mEtLoopCount.setEnabled(false);
        mEtIteration.setEnabled(false);
        mBtnStart.setText(R.string.hqa_cpustress_swvideo_stop);
        mTvResult.setText(R.string.hqa_cpustress_result);
    }
}
