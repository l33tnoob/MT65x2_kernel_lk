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

package com.mediatek.engineermode.power;

import android.app.TabActivity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.InputFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;

import java.io.File;
import java.io.IOException;

public class PMU6575 extends TabActivity implements OnClickListener {

    private TextView mInfo = null;
    private Button mBtnGetRegister = null;
    private Button mBtnSetRegister = null;
    private EditText mEditAddr = null;
    private EditText mEditVal = null;
    private Spinner mBankSpinner;
    private Button mBtnGetReg6333 = null;
    private Button mBtnSetReg6333 = null;
    private EditText mEditAddr6333 = null;
    private EditText mEditVal6333 = null;

    private ArrayAdapter<String> mBankAdatper;
    private String mBankType[] = { "Bank0", "Bank1" };
    private int mBankIndex = 0;

    private static final int TAB_REG = 1;
    private static final int TAB_INFO = 2;
    private int mWhichTab = TAB_INFO;
    private static final int EVENT_UPDATE = 1;
    private static final int MAX_LENGTH_89 = 4;

    private static final int RADIX_HEX = 16;
    private static final int UPDATE_INTERVAL = 1500;
    private static final int WAIT_INTERVAL = 500;
    private static final float MAGIC_TEN = 10.0f;
    
    private static final String FS_MT6333_ACCESS = "/sys/devices/platform/mt6333-user/mt6333_access";
    private static final String CMD_SET_MT6333_ACCESS = 
            "echo %1$s %2$s > " + FS_MT6333_ACCESS;
    private static final String CMD_GET_MT6333_ACCESS = 
            "echo %1$s > " + FS_MT6333_ACCESS;
    
    private void setLayout() {
        mInfo = (TextView) findViewById(R.id.pmu_info_text);
        mBtnGetRegister = (Button) findViewById(R.id.pmu_btn_get);
        mBtnSetRegister = (Button) findViewById(R.id.pmu_btn_set);
        mEditAddr = (EditText) findViewById(R.id.pmu_edit_addr);
        mEditVal = (EditText) findViewById(R.id.pmu_edit_val);
        mBankSpinner = (Spinner) findViewById(R.id.pmu_bank_spinner);
        
        mBtnGetReg6333 = (Button) findViewById(R.id.pmu_btn_get_mt6333);
        mBtnSetReg6333 = (Button) findViewById(R.id.pmu_btn_set_mt6333);
        mBtnGetReg6333.setOnClickListener(this);
        mBtnSetReg6333.setOnClickListener(this);
        mEditAddr6333 = (EditText) findViewById(R.id.pmu_edit_addr_mt6333);
        mEditVal6333 = (EditText) findViewById(R.id.pmu_edit_val_mt6333);

        if (mInfo == null || mBtnGetRegister == null || mBtnSetRegister == null || mEditAddr == null || mEditVal == null
                || mBankSpinner == null) {
            Elog.e("PMU", "clocwork worked...");
            // not return and let exception happened.
        }

        mBtnGetRegister.setOnClickListener(this);
        mBtnSetRegister.setOnClickListener(this);

        mBankAdatper = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        mBankAdatper.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < mBankType.length; i++) {
            mBankAdatper.add(mBankType[i]);
        }
        mBankSpinner.setAdapter(mBankAdatper);
        mBankSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mBankIndex = arg2;
            }

            public void onNothingSelected(AdapterView<?> arg0) {

            }
        });
        if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
            mBankSpinner.setVisibility(View.GONE);
            TextView updateView = (TextView) findViewById(R.id.address6575_text);
            updateView.setText(R.string.address6589_text);
            updateView = (TextView) findViewById(R.id.value6575_text);
            updateView.setText(R.string.value6589_text);
            mEditAddr
                    .setFilters(new InputFilter[] { new InputFilter.LengthFilter(
                            MAX_LENGTH_89) });
            mEditVal
                    .setFilters(new InputFilter[] { new InputFilter.LengthFilter(
                            MAX_LENGTH_89) });
        }
        
        if (!new File(FS_MT6333_ACCESS).exists()) {
            ((LinearLayout)findViewById(R.id.mt6333_layout_controler)).setVisibility(View.GONE);
        } 
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TabHost tabHost = getTabHost();

        LayoutInflater.from(this).inflate(R.layout.power_pmu6575_tabs, tabHost.getTabContentView(), true);

        // tab1
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.pmu_info_text)).setIndicator(
                this.getString(R.string.pmu_info_text)).setContent(R.id.LinerLayout_pmu_info_text));

        // tab2
        tabHost.addTab(tabHost.newTabSpec(this.getString(R.string.pmu_reg)).setIndicator(this.getString(R.string.pmu_reg))
                .setContent(R.id.LinerLayout_pmu_reg));

        setLayout();
        tabHost.setOnTabChangedListener(new OnTabChangeListener() {
            public void onTabChanged(String tabId) {
                String pmuInfoText = PMU6575.this.getString(R.string.pmu_info_text);
                String pmuReg = PMU6575.this.getString(R.string.pmu_reg);
                if (tabId.equals(pmuInfoText)) {
                    onTabInfo();
                } else if (tabId.equals(pmuReg)) {
                    onTabReg();
                }
            }
        });
        // init
    }

    private void onTabReg() {
        mWhichTab = TAB_REG;
    }

    private void onTabInfo() {
        mWhichTab = TAB_INFO;
    }
    
    private void handleGetClick(String addr, String[] cmds, EditText toSetTxt) {
        getInfo(cmds[0]);
        String out = getInfo(cmds[1]);
        try {
            String text = Integer.toHexString(Integer.parseInt(out));
            Elog.i("EM-PMU", "addr:" + addr + "out :" + out);
            Elog.i("EM-PMU", "addr:" + addr + "text :" + text);
            toSetTxt.setText(text);
        } catch (NumberFormatException e) {
            e.printStackTrace();
            Toast.makeText(this, "Please check return value :" + out, Toast.LENGTH_LONG).show();
        }
    }
    
    private void handleSetClick(String addr, String val, String[] cmds) {

        String out = getInfo(cmds[0]);
        Elog.i("EM-PMU", "addr:" + addr + "val:" + val + "out :" + out);
        if (null != out && out.length() != 0) {
            Toast.makeText(this, out, Toast.LENGTH_LONG).show();
        }
    }

    @Override
    public void onClick(View arg0) {
        String regFile = null;
        if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
            regFile = "/sys/devices/platform/mt-pmic/pmic_access";
        } else {
            if (mBankIndex == 0) {
                regFile = "/sys/devices/platform/mt-pmic/pmic_access_bank0";
            } else if (mBankIndex == 1) {
                regFile = "/sys/devices/platform/mt-pmic/pmic_access_bank1";
            } else {
                Toast.makeText(this, "Internal error. bankX too large.",
                        Toast.LENGTH_LONG).show();
                return;
            }
        }
        if (arg0.getId() == mBtnGetRegister.getId()) {
            String addr = mEditAddr.getText().toString();
            if (checkAddr(addr)) {
                String cmd = "echo " + addr + " > " + regFile;
                getInfo(cmd);
                cmd = "cat " + regFile;
                String out = getInfo(cmd);
                try {
                    String text = Integer.toHexString(Integer.parseInt(out));
                    Elog.i("EM-PMU", "addr:" + addr + "out :" + out);
                    Elog.i("EM-PMU", "addr:" + addr + "text :" + text);
                    mEditVal.setText(text);
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                    Toast.makeText(this, "Please check return value :" + out, Toast.LENGTH_LONG).show();
                }
            } else {
                Toast.makeText(this, "Please check address.", Toast.LENGTH_LONG).show();
            }
        } else if (arg0.getId() == mBtnSetRegister.getId()) {
            String addr = mEditAddr.getText().toString();
            String val = mEditVal.getText().toString();
            if (checkAddr(addr.trim()) && checkVal(val.trim())) {
                String cmd = "echo " + addr + " " + val + " > " + regFile;
                String out = getInfo(cmd);
                Elog.i("EM-PMU", "addr:" + addr + "val:" + val + "out :" + out);
                if (null != out && out.length() != 0) {
                    Toast.makeText(this, out, Toast.LENGTH_LONG).show();
                }
            } else {
                Toast.makeText(this, "Please check address or value.", Toast.LENGTH_LONG).show();
            }
        } else if (arg0.getId() == mBtnGetReg6333.getId()) {
            String addr = mEditAddr6333.getText().toString();
            if (checkAddr(addr)) {
                String[] cmds = new String[2];
                cmds[0] = String.format(CMD_GET_MT6333_ACCESS, addr);
                cmds[1] = "cat " + FS_MT6333_ACCESS;
                handleGetClick(addr, cmds, mEditVal6333);
            } else {
                Toast.makeText(this, "Please check address.", Toast.LENGTH_LONG).show();
            }
        } else if (arg0.getId() == mBtnSetReg6333.getId()) {
            String addr = mEditAddr6333.getText().toString();
            String val = mEditVal6333.getText().toString();
            if (checkAddr(addr.trim()) && checkVal(val.trim())) {
                String[] cmds = new String[]{String.format(CMD_SET_MT6333_ACCESS, addr, val)};
                handleSetClick(addr, val, cmds);
            } else {
                Toast.makeText(this, "Please check address or value.", Toast.LENGTH_LONG).show();
            }

        }
    }

    private boolean checkAddr(String s) {
        if (s == null || s.length() < 1) {
            return false;
        }
        String temp = s;
        try {
            Integer.parseInt(temp, RADIX_HEX);
        } catch (NumberFormatException e) {
            return false;
        }
        return true;
    }

    private boolean checkVal(String s) {
        if (s == null || s.length() < 1) {
            if (s != null) {
                Elog.w("EM-PMU", "s.length() is wrong!" + s.length());
            }
            return false;
        }
        if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
            if (s.length() > MAX_LENGTH_89) {
                Elog.w("EM-PMU", "s.length() is too long: " + s.length());
                return false;
            }
        } else {
            if (s.length() > 2) {
                Elog.w("EM-PMU", "s.length() is too long: " + s.length());
                return false;
            }
        }
        String temp = s;
        try {
            Integer.parseInt(temp, RADIX_HEX);
        } catch (NumberFormatException e) {
            return false;
        }
        return true;
    }

    private String getInfo(String cmd) {
        String result = null;
        try {
            String[] cmdx = { "/system/bin/sh", "-c", cmd }; // file must
            // exist// or
            // wait()
            // return2
            int ret = ShellExe.execCommand(cmdx);
            if (0 == ret) {
                result = ShellExe.getOutput();
            } else {
                // result = "ERROR";
                result = ShellExe.getOutput();
            }

        } catch (IOException e) {
            Elog.i("EM-PMU", e.toString());
            result = "ERR.JE";
        }
        return result;
    }

    // private int mUpdateInterval = 1500; // 1.5 sec
    public Handler mUpdateHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_UPDATE:
                Bundle b = msg.getData();
                mInfo.setText(b.getString("INFO"));
                break;
            default:
                break;
            }
        }
    };

    private String mPromptSw = "0/1=off/on";
    private String mPromptUnit = "mV";
    private String[][] mFiles = { { "BUCK_VAPROC_STATUS", mPromptSw }, { "BUCK_VCORE_STATUS", mPromptSw },
            { "BUCK_VIO18_STATUS", mPromptSw }, { "BUCK_VPA_STATUS", mPromptSw }, { "BUCK_VRF18_STATUS", mPromptSw },
            { "SEP", "" }, { "LDO_VA1_STATUS", mPromptSw }, { "LDO_VA2_STATUS", mPromptSw },
            { "LDO_VCAM_AF_STATUS", mPromptSw }, { "LDO_VCAM_IO_STATUS", mPromptSw }, { "LDO_VCAMA_STATUS", mPromptSw },
            { "LDO_VCAMD_STATUS", mPromptSw }, { "LDO_VGP_STATUS", mPromptSw }, { "LDO_VGP2_STATUS", mPromptSw },
            { "LDO_VIBR_STATUS", mPromptSw }, { "LDO_VIO28_STATUS", mPromptSw }, { "LDO_VM12_1_STATUS", mPromptSw },
            { "LDO_VM12_2_STATUS", mPromptSw }, { "LDO_VM12_INT_STATUS", mPromptSw }, { "LDO_VMC_STATUS", mPromptSw },
            { "LDO_VMCH_STATUS", mPromptSw }, { "LDO_VRF_STATUS", mPromptSw }, { "LDO_VRTC_STATUS", mPromptSw },
            { "LDO_VSIM_STATUS", mPromptSw }, { "LDO_VSIM2_STATUS", mPromptSw }, { "LDO_VTCXO_STATUS", mPromptSw },
            { "LDO_VUSB_STATUS", mPromptSw }, { "SEP", "" }, { "BUCK_VAPROC_VOLTAGE", mPromptUnit },
            { "BUCK_VCORE_VOLTAGE", mPromptUnit }, { "BUCK_VIO18_VOLTAGE", mPromptUnit },
            { "BUCK_VPA_VOLTAGE", mPromptUnit }, { "BUCK_VRF18_VOLTAGE", mPromptUnit }, { "SEP", "" },
            { "LDO_VA1_VOLTAGE", mPromptUnit }, { "LDO_VA2_VOLTAGE", mPromptUnit }, { "LDO_VCAM_AF_VOLTAGE", mPromptUnit },
            { "LDO_VCAM_IO_VOLTAGE", mPromptUnit }, { "LDO_VCAMA_VOLTAGE", mPromptUnit },
            { "LDO_VCAMD_VOLTAGE", mPromptUnit }, { "LDO_VGP_VOLTAGE", mPromptUnit }, { "LDO_VGP2_VOLTAGE", mPromptUnit },
            { "LDO_VIBR_VOLTAGE", mPromptUnit }, { "LDO_VIO28_VOLTAGE", mPromptUnit },
            { "LDO_VM12_1_VOLTAGE", mPromptUnit }, { "LDO_VM12_2_VOLTAGE", mPromptUnit },
            { "LDO_VM12_INT_VOLTAGE", mPromptUnit }, { "LDO_VMC_VOLTAGE", mPromptUnit },
            { "LDO_VMCH_VOLTAGE", mPromptUnit }, { "LDO_VRF_VOLTAGE", mPromptUnit }, { "LDO_VRTC_VOLTAGE", mPromptUnit },
            { "LDO_VSIM_VOLTAGE", mPromptUnit }, { "LDO_VSIM2_VOLTAGE", mPromptUnit }, { "LDO_VTCXO_VOLTAGE", mPromptUnit },
            { "LDO_VUSB_VOLTAGE", mPromptUnit } };
    private String[][] mFilesFor89 = {
            { "BUCK_VPROC_STATUS", mPromptSw },
            { "BUCK_VSRAM_STATUS", mPromptSw },
            { "BUCK_VCORE_STATUS", mPromptSw },
            { "BUCK_VM_STATUS", mPromptSw },
            { "BUCK_VIO18_STATUS", mPromptSw },
            { "BUCK_VPA_STATUS", mPromptSw },
            { "BUCK_VRF18_STATUS", mPromptSw },
            { "BUCK_VRF18_2_STATUS", mPromptSw },
            { "SEP", "" },
            { "LDO_VIO28_STATUS", mPromptSw },
            { "LDO_VUSB_STATUS", mPromptSw },
            { "LDO_VMC1_STATUS", mPromptSw },
            { "LDO_VMCH1_STATUS", mPromptSw },
            { "LDO_VEMC_3V3_STATUS", mPromptSw },
            { "LDO_VEMC_1V8_STATUS", mPromptSw },
            { "LDO_VGP1_STATUS", mPromptSw },
            { "LDO_VGP2_STATUS", mPromptSw },
            { "LDO_VGP3_STATUS", mPromptSw },
            { "LDO_VGP4_STATUS", mPromptSw },
            { "LDO_VGP5_STATUS", mPromptSw },
            { "LDO_VGP6_STATUS", mPromptSw },
            { "LDO_VSIM1_STATUS", mPromptSw },
            { "LDO_VSIM2_STATUS", mPromptSw },
            { "LDO_VIBR_STATUS", mPromptSw },
            { "LDO_VRTC_STATUS", mPromptSw },
            { "LDO_VAST_STATUS", mPromptSw },
            { "LDO_VRF28_STATUS ", mPromptSw },
            { "LDO_VRF28_2_STATUS ", mPromptSw },
            { "LDO_VTCXO_STATUS", mPromptSw },
            { "LDO_VTCXO_2_STATUS", mPromptSw },
            { "LDO_VA_STATUS", mPromptSw },
            { "LDO_VA28_STATUS", mPromptSw },
            { "LDO_VCAMA_STATUS", mPromptSw },
            { "SEP", "" },
            { "BUCK_VPROC_VOLTAGE", mPromptUnit },
            { "BUCK_VSRAM_VOLTAGE", mPromptUnit },
            { "BUCK_VCORE_VOLTAGE", mPromptUnit },
            { "BUCK_VM_VOLTAGE", mPromptUnit },
            { "BUCK_VIO18_VOLTAGE", mPromptUnit },
            { "BUCK_VPA_VOLTAGE", mPromptUnit },
            { "BUCK_VRF18_VOLTAGE", mPromptUnit },
            { "BUCK_VRF18_2_VOLTAGE", mPromptUnit },
            { "SEP", "" },
            { "LDO_VIO28_VOLTAGE", mPromptUnit },
            { "LDO_VUSB_VOLTAGE", mPromptUnit },
            { "LDO_VMC1_VOLTAGE", mPromptUnit },
            { "LDO_VMCH1_VOLTAGE", mPromptUnit },
            { "LDO_VEMC_3V3_VOLTAGE", mPromptUnit },
            { "LDO_VEMC_1V8_VOLTAGE", mPromptUnit },
            { "LDO_VGP1_VOLTAGE", mPromptUnit },
            { "LDO_VGP2_VOLTAGE", mPromptUnit },
            { "LDO_VGP3_VOLTAGE", mPromptUnit },
            { "LDO_VGP4_VOLTAGE", mPromptUnit },
            { "LDO_VGP5_VOLTAGE", mPromptUnit },
            { "LDO_VGP6_VOLTAGE", mPromptUnit },
            { "LDO_VSIM1_VOLTAGE", mPromptUnit },
            { "LDO_VSIM2_VOLTAGE", mPromptUnit },
            { "LDO_VIBR_VOLTAGE", mPromptUnit },
            { "LDO_VRTC_VOLTAGE", mPromptUnit },
            { "LDO_VAST_VOLTAGE", mPromptUnit },
            { "LDO_VRF28_VOLTAGE", mPromptUnit },
            { "LDO_VRF28_2_VOLTAGE", mPromptUnit },
            { "LDO_VTCXO_VOLTAGE", mPromptUnit },
            { "LDO_VTCXO_2_VOLTAGE", mPromptUnit },
            { "LDO_VA_VOLTAGE", mPromptUnit },
            { "LDO_VA28_VOLTAGE", mPromptUnit },
            { "LDO_VCAMA_VOLTAGE", mPromptUnit }
    };
    
    private String[][] mFilesFor7282 = {
            {"BUCK_VPROC_STATUS", mPromptSw},
            {"BUCK_VSYS_STATUS", mPromptSw},
            {"BUCK_VPA_STATUS", mPromptSw},
            {"SEP", ""},
            {"LDO_VTCXO_STATUS", mPromptSw},
            {"LDO_VA_STATUS", mPromptSw},
            {"LDO_VCAMA_STATUS", mPromptSw},
            {"LDO_VCN28_STATUS", mPromptSw},
            {"LDO_VCN33_STATUS", mPromptSw},
            {"LDO_VIO28_STATUS", mPromptSw},
            {"LDO_VUSB_STATUS", mPromptSw},
            {"LDO_VMC_STATUS", mPromptSw},
            {"LDO_VMCH_STATUS", mPromptSw},
            {"LDO_VEMC_3V3_STATUS", mPromptSw},
            {"LDO_VGP1_STATUS", mPromptSw},
            {"LDO_VGP2_STATUS", mPromptSw},
            {"LDO_VGP3_STATUS", mPromptSw},
            {"LDO_VCN_1V8_STATUS", mPromptSw},
            {"LDO_VSIM1_STATUS", mPromptSw},
            {"LDO_VSIM2_STATUS", mPromptSw},
            {"LDO_VRTC_STATUS", mPromptSw},
            {"LDO_VCAM_AF_STATUS", mPromptSw},
            {"LDO_VIBR_STATUS", mPromptSw},
            {"LDO_VM_STATUS", mPromptSw},
            {"LDO_VRF18_STATUS", mPromptSw},
            {"LDO_VIO18_STATUS", mPromptSw},
            {"LDO_VCAMD_STATUS", mPromptSw},
            {"LDO_VCAM_IO_STATUS", mPromptSw},
            {"SEP", ""},
            {"BUCK_VPROC_VOLTAGE", mPromptUnit},
            {"BUCK_VSYS_VOLTAGE", mPromptUnit},
            {"BUCK_VPA_VOLTAGE", mPromptUnit},
            {"SEP", ""},
            {"LDO_VMC_VOLTAGE", mPromptUnit},
            {"LDO_VMCH_VOLTAGE", mPromptUnit},
            {"LDO_VEMC_3V3_VOLTAGE", mPromptUnit},
            {"LDO_VGP1_VOLTAGE", mPromptUnit},
            {"LDO_VGP2_VOLTAGE", mPromptUnit},
            {"LDO_VGP3_VOLTAGE", mPromptUnit},
            {"LDO_VSIM1_VOLTAGE", mPromptUnit},
            {"LDO_VSIM2_VOLTAGE", mPromptUnit},
            {"LDO_VCAM_AF_VOLTAGE", mPromptUnit},
            {"LDO_VIBR_VOLTAGE", mPromptUnit},
            {"LDO_VM_VOLTAGE", mPromptUnit},
            {"LDO_VCAMD_VOLTAGE", mPromptUnit},
            {"LDO_VCAMA_VOLTAGE", mPromptUnit},
            {"LDO_VCN33_VOLTAGE", mPromptUnit}
    };

    private boolean mRun = false;

    class RunThread extends Thread {

        public void run() {
            while (mRun) {
                StringBuilder text = new StringBuilder("");
                String cmd = "";
                String[][] fileArray = null;
                if (ChipSupport.MTK_6589_SUPPORT <= ChipSupport.getChip()) {
                    fileArray = mFilesFor89;
                    if (ChipSupport.isCurrentChipHigher(ChipSupport.MTK_6589_SUPPORT, false)) {
                        fileArray = mFilesFor7282;
                    }
                } else {
                    fileArray = mFiles;
                }
                for (int i = 0; i < fileArray.length; i++) {
                    if (fileArray[i][0].equalsIgnoreCase("SEP")) {
                        text.append("- - - - - - - - - -\n");
                        continue;
                    }
                    cmd = "cat /sys/devices/platform/mt-pmic/" + fileArray[i][0];

                    if (fileArray[i][1].equalsIgnoreCase("mA")) {
                        double f = 0.0f;
                        try {
                            f = Float.valueOf(getInfo(cmd)) / MAGIC_TEN;
                        } catch (NumberFormatException e) {
                            Elog.e("EM-PMU", "read file error " + fileArray[i][0]);
                        }
                        text.append(String.format("%1$-28s:[%2$-6s]%3$s\n", fileArray[i][0], f, fileArray[i][1]));
                    } else {
                        text.append(String.format("%1$-28s:[%2$-6s]%3$s\n", fileArray[i][0], getInfo(cmd), fileArray[i][1]));
                    }
                }

                Bundle b = new Bundle();
                b.putString("INFO", text.toString());

                Message msg = new Message();
                msg.what = EVENT_UPDATE;
                msg.setData(b);
                mUpdateHandler.sendMessage(msg);
                try {
                    sleep(UPDATE_INTERVAL);
                    while(mWhichTab == TAB_REG && mRun) {
                        sleep(WAIT_INTERVAL);
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        mRun = false;
    }

    @Override
    protected void onResume() {
        super.onResume();
        mRun = true;
        new RunThread().start();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }
}
