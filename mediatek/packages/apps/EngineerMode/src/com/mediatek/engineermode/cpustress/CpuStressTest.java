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

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.FeatureHelpPage;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;

public class CpuStressTest extends CpuStressCommon implements OnItemClickListener,
        RadioGroup.OnCheckedChangeListener, CheckBox.OnCheckedChangeListener {

    private static final String TAG = "EM/CpuStress";
    public static final int INDEX_DEFAULT = 0;
    public static final int INDEX_TEST = 1;
    public static final int INDEX_SINGLE = 2;
    public static final int INDEX_DUAL = 3;
    public static final int INDEX_TRIPLE = 4;
    public static final int INDEX_QUAD = 5;
    public static final int INDEX_OCTA = 6;
    private static final int RADIO_BUTTON_COUNT = 7;
    public static final int TEST_BACKUP = 20;
    public static final int TEST_RESTORE = 40;
    private static final int ITEM_COUNT = 3;
    private static final String TYPE_LOAD_ENG = "eng";
    private static final String ERROR = "ERROR";

    private static final String[] HQA_CPU_STRESS_TEST_ITEMS = new String[ITEM_COUNT];
    private ArrayList<String> mListCpuTestItem = null;
    private RadioButton mRbDefault = null;
    private RadioButton mRbTest = null;
    private RadioButton mRbSingle = null;
    private RadioButton mRbDual = null;
    private RadioButton mRbTriple = null;
    private RadioButton mRbQuad = null;
    private RadioButton mRbOcta = null;

    private RadioButton[] mRdoBtn = new RadioButton[RADIO_BUTTON_COUNT];
    private CheckBox mCbThermal = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_cpustress);
        if (!Build.TYPE.equals("eng")) {
            Toast.makeText(this, R.string.hqa_cpustress_toast_load_notsupport,
                    Toast.LENGTH_LONG).show();
            Xlog.d(TAG, "Not eng load, finish");
            finish();
            return;
        }
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                Xlog.v(TAG, "mHandler receive message: " + msg.what);
                CpuStressTestService.sIsThermalSupport = new File(
                        CpuStressTestService.THERMAL_ETC_FILE).exists();
                switch (msg.what) {
                case INDEX_UPDATE_RADIOBTN:
                    switch (CpuStressTestService.sCoreNumber) {
                    case CpuStressTestService.CORE_NUMBER_8:
                        updateCbThermal();
                        break;
                    case CpuStressTestService.CORE_NUMBER_4:
                        updateCbThermal();
                        mRbOcta.setVisibility(View.GONE);
                        break;
                    case CpuStressTestService.CORE_NUMBER_2:
                        updateCbThermal();
                        mRbTriple.setVisibility(View.GONE);
                        mRbQuad.setVisibility(View.GONE);
                        mRbOcta.setVisibility(View.GONE);
                        break;
                    default:
                        mRbDual.setVisibility(View.GONE);
                        mRbSingle.setVisibility(View.GONE);
                        mRbTriple.setVisibility(View.GONE);
                        mRbQuad.setVisibility(View.GONE);
                        mRbOcta.setVisibility(View.GONE);
                        mCbThermal.setVisibility(View.GONE);
                        break;
                    }
                    checkRdoBtn(CpuStressTestService.sIndexMode);
                    updateRadioGroup(!mBoundService.isTestRun());
                    break;
                case INDEX_UPDATE_RADIOGROUP:
                    updateRadioGroup(!mBoundService.isTestRun());
                    break;
                default:
                    super.handleMessage(msg);
                    break;
                }
            }
        };
        HQA_CPU_STRESS_TEST_ITEMS[0] = getString(R.string.hqa_cpustress_apmcu_name);
        HQA_CPU_STRESS_TEST_ITEMS[1] = getString(R.string.hqa_cpustress_swvideo_name);
        HQA_CPU_STRESS_TEST_ITEMS[2] = getString(R.string.hqa_cpustress_clockswitch_name);
        ListView testItemList = (ListView) findViewById(R.id.listview_hqa_cpu_main);
        RadioGroup  rgRadioGroup = (RadioGroup)findViewById(R.id.hqa_cpu_main_radiogroup);
        mRbDefault = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_default);
        mRbTest = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_test);
        mRbSingle = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_single);
        mRbDual = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_dual);
        mRbTriple = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_triple);
        mRbQuad = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_quad);
        mRbOcta = (RadioButton) findViewById(R.id.hqa_cpu_main_raidobutton_octa);
        mCbThermal = (CheckBox)findViewById(R.id.hqa_cpu_main_checkbox);
        mRdoBtn[0] = mRbDefault;
        mRdoBtn[1] = mRbTest;
        mRdoBtn[2] = mRbSingle;
        mRdoBtn[3] = mRbDual;
        mRdoBtn[4] = mRbTriple;
        mRdoBtn[5] = mRbQuad;
        mRdoBtn[6] = mRbOcta;
        mListCpuTestItem = new ArrayList<String>(Arrays.asList(HQA_CPU_STRESS_TEST_ITEMS));
        mListCpuTestItem.add(getString(R.string.help));
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mListCpuTestItem);
        testItemList.setAdapter(adapter);
        testItemList.setOnItemClickListener(this);
        rgRadioGroup.setOnCheckedChangeListener(this);
        mCbThermal.setOnCheckedChangeListener(this);
        updateRadioGroup(false);
        startService(new Intent(this, CpuStressTestService.class));
        Xlog.i(TAG, "start cpu test service");
    }

    private void updateCbThermal() {
        mCbThermal.setEnabled(false);
        mCbThermal.setChecked(CpuStressTestService.sIsThermalDisabled);
        mCbThermal.setEnabled(CpuStressTestService.sIsThermalSupport);
    }

    private void checkRdoBtn(int indexRdoBtn) {
        mRdoBtn[indexRdoBtn].setEnabled(false);
        mRdoBtn[indexRdoBtn].setChecked(true);
        mRdoBtn[indexRdoBtn].setEnabled(true);
    }

    /**
     * Update radio group and check box status
     * 
     * @param testRunning
     *            Test running or not
     */
    protected void updateRadioGroup(boolean testRunning) {
        for (int i = 0; i < RADIO_BUTTON_COUNT; i++) {
            mRdoBtn[i].setEnabled(testRunning);
        }
        mCbThermal.setEnabled(testRunning);
        if (testRunning && (!CpuStressTestService.sIsThermalSupport)) {
            mCbThermal.setEnabled(false);
        }
        removeDialog(DIALOG_WAIT);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = null;
        Xlog.i(TAG, "User select: " + mListCpuTestItem.get(arg2));
        if (mListCpuTestItem.get(arg2).equals(getString(R.string.help))) {
            intent = new Intent(this, FeatureHelpPage.class);
            intent.putExtra(FeatureHelpPage.HELP_TITLE_KEY, R.string.help);
            if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
                intent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY,
                        R.string.hqa_cpustress_help_msg_new);
            } else {
                intent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY,
                        R.string.hqa_cpustress_help_msg);
            }
            startActivity(intent);
            return;
        }
        if (0 == CpuStressTestService.sIndexMode) {
            Toast.makeText(this, R.string.hqa_cpustress_toast_mode,
                    Toast.LENGTH_SHORT).show();
            Xlog.d(TAG, "Not select mode");
        } else {
            if (mListCpuTestItem.get(arg2).equals(
                    getString(R.string.hqa_cpustress_apmcu_name))) {
                intent = new Intent(this, ApMcu.class);
            } else if (mListCpuTestItem.get(arg2).equals(
                    getString(R.string.hqa_cpustress_swvideo_name))) {
                intent = new Intent(this, SwVideoCodec.class);
            } else if (mListCpuTestItem.get(arg2).equals(
                    getString(R.string.hqa_cpustress_clockswitch_name))) {
                intent = new Intent(this, ClockSwitch.class);
                if (2 > CpuStressTestService.sIndexMode
                        && CpuStressTestService.CORE_NUMBER_2 <= CpuStressTestService.sCoreNumber) {
                    Toast.makeText(this, R.string.hqa_cpustress_toast_not_force,
                            Toast.LENGTH_SHORT).show();
                    return;
                }
            }
            if (null == intent) {
                Toast.makeText(this, R.string.hqa_cpustress_toast_item_error,
                        Toast.LENGTH_LONG).show();
                Xlog.d(TAG, "Select error");
            } else {
                this.startActivity(intent);
            }
        }
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        Xlog.v(TAG, "Enter onCheckedChanged: " + checkedId);
        int index = 0;
        for (; index < RADIO_BUTTON_COUNT; index++) {
            if (checkedId == mRdoBtn[index].getId()) {
                break;
            }
        }
        if (index < RADIO_BUTTON_COUNT) {
            mBoundService.setIndexMode(index);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.isEnabled()) {
            if (buttonView.getId() == mCbThermal.getId()) {
                Xlog.v(TAG, "check box checked: " + isChecked);
                doThermalDisable(isChecked);
            } else {
                Xlog.v(TAG, "Unknown checkbox");
            }
        }
    }

    /**
     * Set thermal disable/enable
     * 
     * @param disableThermal
     *            True to disable thermal, false to enable thermal
     */
    private void doThermalDisable(boolean disableThermal) {
        Xlog.v(TAG, "Enter doThermalDisable: " + disableThermal);
        CpuStressTestService.sIsThermalDisabled = disableThermal;
        StringBuilder build = new StringBuilder();
        AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
        boolean result = functionCall.startCallFunctionStringReturn(
                AFMFunctionCallEx.FUNCTION_EM_CPU_STRESS_TEST_THERMAL);
        functionCall.writeParamNo(1);
        functionCall.writeParamInt(disableThermal ? 0 : 1);
        if (result) {
            FunctionReturn r;
            do {
                r = functionCall.getNextResult();
                if (r.mReturnString.isEmpty()) {
                    break;
                }
                build.append(r.mReturnString);
            } while (r.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
            if (r.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {
                Xlog.d(TAG, "AFMFunctionCallEx: RESULT_IO_ERR");
                build.replace(0, build.length(), ERROR);
            }
        } else {
            Xlog.d(TAG, "AFMFunctionCallEx return false");
            build.append(ERROR);
        }
        Xlog.v(TAG, "doThermalDisable response: " + build.toString());
    }
}

