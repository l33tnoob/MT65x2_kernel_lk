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

package com.mediatek.engineermode.touchscreen;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;


import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class TouchScreenSettings extends Activity implements OnClickListener {

    // private ToggleButton mBtnSDcard;
    // private ToggleButton mBtnUart;
    private Button mBtnSet;
    private EditText mEdit;
//    private Spinner mModeSpinner;
//    private ArrayAdapter<String> mModeAdatper;
    private int mModeIndex;
    private ArrayList<SpinnerData> mCategory;

    private static String[] sFirstCommand = { "/system/bin/sh", "-c",
    // "echo 1 > /sys/module/tpd_debug/parameters/tpd_em_log" };
            "echo 2 > /sys/module/tpd_setting/parameters/tpd_mode" };

    private static final String PARA_PATH = "/sys/module/tpd_debug/parameters";
    private static final String PARA_PATH2 = "/sys/module/tpd_setting/parameters";
    private static final String PARA_TAG = "tpd_em_";
//    private int mErrorCode = 0;
//    private static final int ERR_OK = 0;
//    private static final int ERR_ERR = 1;

    private static final String TAG = "EM/TouchScreen/set";

    private boolean mSdcardExist = false;
    
    private static volatile boolean sRun = false;
    private static String sCurrentFileName = null;
    private static final int EVENT_UPDATE = 1;
    
    public Handler mUpdateHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == EVENT_UPDATE) {
                Toast.makeText(TouchScreenSettings.this,
                        "Finish file logging.", Toast.LENGTH_SHORT).show();
            }           
//            switch (msg.what) {
//            case EVENT_UPDATE:
//                Toast.makeText(TouchScreenSettings.this,
//                        "Finish file logging.", Toast.LENGTH_SHORT).show();
//                break;
//            default:
//                break;
//            }
        }
    };
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
//        Xlog.v(TAG, "-->onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.touch_settings);

        // mBtnSDcard = (ToggleButton)
        // findViewById(R.id.TouchScreen_Settings_sdcard);
        // mBtnUart = (ToggleButton)
        // findViewById(R.id.TouchScreen_Settings_uart);

        mBtnSet = (Button) findViewById(R.id.TouchScreen_Settings_TextSet);
        mEdit = (EditText) findViewById(R.id.TouchScreen_Settings_Value);
        Spinner modeSpinner = (Spinner) findViewById(R.id.TouchScreen_Settings_Spinner);

        ArrayAdapter<String> modeAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);
        modeAdatper
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mCategory = getCategory();
        if (null == mCategory) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Warning");
            builder.setMessage("No setting file exist.");
            builder.setCancelable(false);
            builder.setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            TouchScreenSettings.this.finish();
                        }
                    });
            builder.create().show();
            return;
        }

        for (int i = 0; i < mCategory.size(); i++) {
            modeAdatper.add(mCategory.get(i).mName);
        }
        modeSpinner.setAdapter(modeAdatper);
        modeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {
                mModeIndex = arg2;
                mEdit.setText(getFileValue(mCategory.get(mModeIndex).mFullPath));
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                mModeIndex = 0;
            }
        });

        mBtnSet.setOnClickListener(this);
        // mBtnSDcard.setOnClickListener(this);
        // mBtnUart.setOnClickListener(this);

    }

    private String getFileValue(String path) {
        String[] cmd = { "/system/bin/sh", "-c", "cat " + path }; // file must
        // exist or
        // wait()
        // return2
        Xlog.v(TAG, "-->GetFileValue:" + path);
        int ret;
        try {
            ret = TouchScreenShellExe.execCommand(cmd);

            if (0 == ret) {
                return TouchScreenShellExe.getOutput();
            } else {
                return "N/A";
            }
        } catch (IOException e) {
            Xlog.v(TAG, "-->GetFileValue:" + e.getMessage());
            return "N/A";
        }
    }

    @Override
    public void onResume() {
//        Xlog.v(TAG, "-->onResume");
        super.onResume();
        // final SharedPreferences preferences =
        // this.getSharedPreferences("touch_screen_settings",
        // android.content.Context.MODE_PRIVATE);
        
        mSdcardExist = false;
        if (!Environment.getExternalStorageState().equals(
                Environment.MEDIA_REMOVED)) {
            mSdcardExist = true;
        }
    }

    private ArrayList<SpinnerData> getCategory() {
        File dir = new File(PARA_PATH);
        File[] files = dir.listFiles();
        if (files == null) {
            return null;
        }
        ArrayList<SpinnerData> result = new ArrayList<SpinnerData>();
        for (File f : files) {
            if (f.getName().indexOf(PARA_TAG) == 0) {
                SpinnerData data = new SpinnerData();
                data.mName = f.getName();
                data.mFullPath = f.getAbsolutePath();
                result.add(data);
            }

        }
        dir = new File(PARA_PATH2);
        files = dir.listFiles();
        if (files == null) {
            return result;
        }
        for (File f : files) {
            if (f.getName().indexOf(PARA_TAG) == 0) {
                SpinnerData data = new SpinnerData();
                data.mName = f.getName();
                data.mFullPath = f.getAbsolutePath();
                result.add(data);
            }
        }

//        if (result.size() == 0) {
        if (result.isEmpty()) {
            return null;
        }
        return result;
    }

    /*
     * private int getLastError() { return err_no; }
     */
//    private void setLastError(int err) {
//        mErrorCode = err;
//    }

    
    /**
     * workThread :write file para
     * 
     */
    class WorkThread extends Thread {

        public void run() {
            while (sRun) {
                // Log.i("MTHR", "LOOP mRun = "+mRun);
                if (mSdcardExist) {
                    // String shell = "cat /proc/tpd_em_log >> " +
                    // currentFileName;
                    String shell = "cat /sys/module/tpd_debug/parameters/tpd_em_log  >> "
                            + sCurrentFileName;
                    Xlog.v(TAG, "run file shell--" + shell);
                    String[] cmd2 = { "/system/bin/sh", "-c", shell };
                    int ret = 0;
                    try {
                        ret = TouchScreenShellExe.execCommand(cmd2);
                        if (0 != ret) {
                            Xlog.i(TAG, "cat >> failed!! ");
                            // return;
                        }
                    } catch (IOException e) {
                        Xlog.w(TAG, "cat >> failed!!  io exception");
                    }
                }

                try {
                    sleep(10);
                    // Log.i("MTHR", "After sleep");
                } catch (InterruptedException e) {
                    Xlog.w(TAG, "sleep(10) >> exception!!!");

                }
            }
            // Toast.makeText(TouchScreen_Settings.this, "Finish file logging.",
            // Toast.LENGTH_LONG).show();
            Message msg = new Message();
            msg.what = EVENT_UPDATE;

            mUpdateHandler.sendMessage(msg);

            Xlog.i(TAG, "Copy /proc/tpd_em_log success");
        }
    }

    public void onClick(View arg0) {
        if (arg0.getId() == mBtnSet.getId()) {
            String editString = mEdit.getText().toString();
            if (null == editString || editString.length() == 0) {
                Toast.makeText(this, "Please input the value.",
                        Toast.LENGTH_LONG).show();
//                setLastError(ERR_OK);
                return;
            }

            // boolean resultOK = false;
            try {
                if (mCategory.get(mModeIndex).mName.equals("tpd_em_log_to_fs")) { // if
                    // no
                    // sdcard
                    // exists.
                    if (!mSdcardExist) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(
                                this);
                        builder.setTitle("Error");
                        builder.setMessage("No SD card exists.");
                        builder.setPositiveButton("OK", null);
                        builder.create().show();
                        return;
                    }
                    final SharedPreferences preferences = this
                            .getSharedPreferences("touch_screen_settings",
                                    android.content.Context.MODE_PRIVATE);
                    // open file log
                    if ("0".equals(editString)) {
//                        if (editString.equals("0")) {
                        // close file log
                        sRun = false;
                        Xlog.i(TAG, "close file log mRun = " + sRun);
                        setCategory("0");
                        SharedPreferences.Editor editor = preferences.edit();
                        editor.putString("filename", "N");
                        editor.commit();
                        
                    } else {
                        String curVal = getFileValue(mCategory.get(mModeIndex).mFullPath);
                     // already open file log.
                        if (!"0".equals(curVal) && !"N/A".equals(curVal)) {
//                            if (!curVal.equals("0") && !curVal.equals("N/A")) {
                            Toast.makeText(this, "File Log Already Opened.",
                                    Toast.LENGTH_SHORT).show();
                            return;
                        }

                        runFirstCommand();

                        String[] cmd = {
                                "/system/bin/sh",
                                "-c",
                                "echo " + editString + " > " + PARA_PATH
                                        + "/tpd_em_log" }; // file

                        int ret = TouchScreenShellExe.execCommand(cmd);
                        if (0 != ret) {
                            Toast.makeText(this,
                                   "Set tpd_em_log failed. open file log failed.",
                                            Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_ERR);
                            return;
                        }

                        String[] cmdx = {
                                "/system/bin/sh",
                                "-c",
                                "echo " + editString + " > "
                                        + mCategory.get(mModeIndex).mFullPath }; // file

                        ret = TouchScreenShellExe.execCommand(cmdx);
                        if (0 == ret) {
                            Toast.makeText(this, "open file log success.",
                                    Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_OK);
                        } else {
                            Toast.makeText(this, "open file log failed.",
                                    Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_ERR);
                            return;
                        }

                        File sdcard = Environment.getExternalStorageDirectory();
                        File touchLog = new File(sdcard.getParent() + "/"
                                + sdcard.getName() + "/TouchLog/");

                        if (!touchLog.isDirectory()) {
                            touchLog.mkdirs();
                            Xlog.i(TAG, "mkdir " + touchLog.getPath()
                                    + " success");
                        }
                        SimpleDateFormat df = new SimpleDateFormat(
                                "yyyy-MM-dd_HH-mm-ss");
                        sCurrentFileName = touchLog.getPath() + "/L"
                                + df.format(new Date().getTime());
                        String shell = "echo START > " + sCurrentFileName;
                        Xlog.i(TAG, "file shell " + shell);
                        String[] cmd2 = { "/system/bin/sh", "-c", shell };
                        ret = TouchScreenShellExe.execCommand(cmd2);
                        if (0 != ret) {
                            // Log.i("MTH",
                            // "Create file failed.(echo ###> failed!! )");
                            Toast.makeText(this,
                                    "Error: Create file in sdcard failed!!",
                                    Toast.LENGTH_SHORT).show();
                            return;
                        }

                        sRun = true;
                        new WorkThread().start();

                        Xlog.v(TAG, "thread start mRun = " + sRun);
                        Toast.makeText(this, "Start log file to sdcard.",
                                Toast.LENGTH_SHORT).show();
                        Xlog.v(TAG, "Start log file to sdcard.");
                        SharedPreferences.Editor editor = preferences.edit();
                        editor.putString("filename", sCurrentFileName);
                        editor.commit();
                    

                    }
                } else if (mCategory.get(mModeIndex).mName.equals("tpd_em_log")) {
                    // close uart log
                    if ("0".equals(editString)) {
//                        if (editString.equals("0")) {
                        sRun = false;
                        Xlog.i(TAG, "uart close mRun = " + sRun);
                        final SharedPreferences preferences = this
                                .getSharedPreferences("touch_screen_settings",
                                        android.content.Context.MODE_PRIVATE);
                        SharedPreferences.Editor editor = preferences.edit();
                        editor.putString("filename", "N");
                        editor.commit();

                        runFirstCommand();

                        String[] cmd = {
                                "/system/bin/sh",
                                "-c",
                                "echo 0 > " + PARA_PATH
                                        + "/tpd_em_log_to_fs" };
                        int ret = TouchScreenShellExe.execCommand(cmd);
                        if (0 != ret) {
                            Toast.makeText(this,
                                            "Set tpd_em_log_to_fs failed. close file log failed.",
                                            Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_ERR);
                            return;
                        }

                        String[] cmdx = {
                                "/system/bin/sh",
                                "-c",
                                "echo " + editString + " > "
                                        + mCategory.get(mModeIndex).mFullPath }; // file
                        // must
                        // exist
                        // or
                        // wait()
                        // return2
                        ret = TouchScreenShellExe.execCommand(cmdx);
                        if (0 == ret) {
                            Toast.makeText(this, "Close uart log success.",
                                    Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_OK);
                        } else {
                            Toast.makeText(this, "Close uart log failed.",
                                    Toast.LENGTH_SHORT).show();
//                            setLastError(ERR_ERR);
                            return;
                        }
                    } else {
                        setCategory(editString);
                    }
                } else {
                    setCategory(editString);
                }

            } catch (IOException e) {
                Xlog.i(TAG, e.toString());
                Toast.makeText(this,
                                "Set ." + mCategory.get(mModeIndex).mName
                                 + " exception.", Toast.LENGTH_LONG)
                        .show();
//                setLastError(ERR_ERR);
            }
        }

    }

    private void setCategory(String editString) throws IOException {
        runFirstCommand();

        String[] cmd = {
                "/system/bin/sh",
                "-c",
                "echo " + editString + " > "
                        + mCategory.get(mModeIndex).mFullPath };
        int ret = TouchScreenShellExe.execCommand(cmd);
        if (0 == ret) {
            Toast.makeText(this,
                    "Set ." + mCategory.get(mModeIndex).mName + " success.",
                    Toast.LENGTH_SHORT).show();
//            setLastError(ERR_OK);
        } else {
            Toast.makeText(this,
                    "Set ." + mCategory.get(mModeIndex).mName + " failed.",
                    Toast.LENGTH_SHORT).show();
//            setLastError(ERR_ERR);
        }
    }

    // private String firstCommand =
    // "adb shell echo 2 > /sys/module/tpd_setting/parameters/tpd_mode";

    /**
     * Run init command 
     * 
     */
    public void runFirstCommand() {
        try {

            int ret = TouchScreenShellExe.execCommand(sFirstCommand);

            Xlog.v(TAG, "write tpd_mode result:"
                    + TouchScreenShellExe.getOutput());
            if (0 == ret) {
                Toast.makeText(this, "write tpd_mode 2 success.",
                        Toast.LENGTH_SHORT).show();
//                setLastError(ERR_OK);
            } else {
                Toast.makeText(this, "write tpd_mode 2 failed.",
                        Toast.LENGTH_SHORT).show();
//                setLastError(ERR_ERR);
//                return;
            }
        } catch (IOException e) {
            Xlog.i(TAG, e.toString());
            Toast.makeText(this, "write tpd_mode 2  exception.",
                    Toast.LENGTH_SHORT).show();
//            setLastError(ERR_ERR);
        }
    }
    
    /**
     * class for setting name and para path 
     * @author mtk54040
     *
     */
    private class SpinnerData {
        public String mName;
        public String mFullPath;
    }
}
