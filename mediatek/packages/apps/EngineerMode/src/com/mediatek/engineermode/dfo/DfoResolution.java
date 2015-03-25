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

package com.mediatek.engineermode.dfo;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.WallpaperManager;

import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.mediatek.engineermode.dfo.DfoNative.DfoReadCount;
import com.mediatek.engineermode.dfo.DfoNative.DfoReadReq;
import com.mediatek.engineermode.dfo.DfoNative.DfoReadCnf;
import com.mediatek.engineermode.dfo.DfoNative.DfoWriteReq;
import com.mediatek.engineermode.dfo.DfoNative.DfoDefaultSize;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class DfoResolution extends Activity implements OnClickListener {
    private static final String TAG = "EM/DFO";
    private static final int MSG_QUERY_DONE = 0;
    private static final int MSG_SET_DONE = 1;
    private static final int MSG_SET_FAILED = 2;
    private static final int DIALOG_REBOOT = 0;
    private static final int DIALOG_QUERY = 1;
    private static final int DIALOG_SET = 2;
    private static final String LCM_HEIGHT = "LCM_FAKE_HEIGHT";
    private static final String LCM_WIDTH = "LCM_FAKE_WIDTH";
    private static final int RESOLUTION_COUNT = 5;

    private final int mStandardHeight[] = new int[] {1920, 1280, 960, 854, 800};
    private final int mStandardWidth[] = new int[] {1080, 720, 540, 480, 480};

    private RadioGroup mRadioGroup;
    private RadioButton mRadioButtons[] = new RadioButton[RESOLUTION_COUNT];
    private Button mButton = null;
    private ProgressDialog mDialogQuery = null;
    private ProgressDialog mDialogSet = null;
    private Toast mToast = null;

    private int mDefaultHeight = 0;
    private int mDefaultWidth = 0;
    private int mCurrentHeight = 0;
    private int mCurrentWidth = 0;
    private int mHeight = 0;
    private int mWidth = 0;
    private WallpaperManager mWallpaperManager = null;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            mButton.setEnabled(true);
            switch (msg.what) {
            case MSG_QUERY_DONE:
                if (mDialogQuery != null) {
                    mDialogQuery.dismiss();
                }
                // Got current screen resolution
                for (int i = 0; i < RESOLUTION_COUNT; i++) {
                    if (mCurrentHeight == mStandardHeight[i] && mCurrentWidth == mStandardWidth[i]) {
                        mRadioButtons[i].setChecked(true);
                        return;
                    }
                }
                showToast(getString(R.string.dfo_resolution_invalid) + LCM_HEIGHT + "=" +
                        mCurrentHeight + ", " + LCM_WIDTH + "=" + mCurrentWidth);
                break;
            case MSG_SET_DONE:
                PowerManager pm = (PowerManager) DfoResolution.this
                        .getSystemService(Context.POWER_SERVICE);
                pm.reboot("");
                break;
            case MSG_SET_FAILED:
                if (mDialogSet != null) {
                    mDialogSet.dismiss();
                }
                showToast(R.string.dfo_resolution_fail);
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.dfo_resolution);

        mWallpaperManager = (WallpaperManager) getSystemService(WALLPAPER_SERVICE);

        mRadioGroup = (RadioGroup) findViewById(R.id.dfo_resolution_radio_group);
        mRadioButtons[0] = (RadioButton) findViewById(R.id.dfo_resolution_fhd);
        mRadioButtons[1] = (RadioButton) findViewById(R.id.dfo_resolution_hd);
        mRadioButtons[2] = (RadioButton) findViewById(R.id.dfo_resolution_qhd);
        mRadioButtons[3] = (RadioButton) findViewById(R.id.dfo_resolution_fwvga);
        mRadioButtons[4] = (RadioButton) findViewById(R.id.dfo_resolution_wvga);
        mButton = (Button) findViewById(R.id.dfo_resolution_set);
        mButton.setOnClickListener(this);

        // Query default screen resolution
        DfoDefaultSize size = new DfoDefaultSize();
        DfoNative.getDefaultSize(size);
        mDefaultHeight = size.height;
        mDefaultWidth = size.width;

        // Cannot switch to larger than default resolution
        for (int i = 0; i < RESOLUTION_COUNT; i++) {
            if (mDefaultHeight < mStandardHeight[i] && mDefaultWidth < mStandardWidth[i]) {
                mRadioButtons[i].setEnabled(false);
            }
        }
        if (!mRadioButtons[RESOLUTION_COUNT - 1].isEnabled()) {
            mButton.setEnabled(false);
            return;
        }

        // Query current screen resolution
        showDialog(DIALOG_QUERY);
        new Thread(new Runnable() {
            public void run() {
                DfoReadCount readCount = new DfoReadCount();
                DfoReadReq req = new DfoReadReq();
                DfoReadCnf cnf = new DfoReadCnf();

                mCurrentHeight = -1;
                mCurrentWidth = -1;
                if (DfoNative.init() == DfoNative.RET_SUCCESS
                        && DfoNative.readCount(readCount) == DfoNative.RET_SUCCESS) {
                    for (int i = 0; i < readCount.count; i++) {
                        req.index = i;
                        if (DfoNative.read(req, cnf) != DfoNative.RET_SUCCESS) {
                            continue;
                        }
                        if (cnf.name != null && cnf.name.equals(LCM_HEIGHT)) {
                            mCurrentHeight = (cnf.value == 0) ? mDefaultHeight : cnf.value;
                        } else if (cnf.name != null && cnf.name.equals(LCM_WIDTH)) {
                            mCurrentWidth = (cnf.value == 0) ? mDefaultWidth : cnf.value;
                        }
                    }
                    DfoNative.deinit();
                }
                mHandler.sendEmptyMessageDelayed(MSG_QUERY_DONE, 500);
            }
        }).start();
    }

    @Override
    public void onClick(View v) {
        if (mRadioGroup.getCheckedRadioButtonId() < 0) {
            return;
        }
        for (int i = 0; i < RESOLUTION_COUNT; i++) {
            if (mRadioButtons[i].isChecked()) {
                if (mCurrentHeight == mStandardHeight[i] && mCurrentWidth == mStandardWidth[i]) {
                    showToast(R.string.dfo_resolution_already_set);
                    return;
                } else {
                    mHeight = mStandardHeight[i];
                    mWidth = mStandardWidth[i];
                }
            }
        }

        mButton.setEnabled(false);
        showDialog(DIALOG_REBOOT);
    }

    private void setResolution() {
        new Thread(new Runnable() {
            public void run() {
                DfoWriteReq req = new DfoWriteReq();

                DfoNative.init();
                req.name = LCM_HEIGHT;
                req.value = mHeight;
                req.partition = 1;
                req.save = 1;
                int retHeight = DfoNative.write(req);
                req.name = LCM_WIDTH;
                req.value = mWidth;
                int retWidth = DfoNative.write(req);
                int ret = DfoNative.propertySet(mHeight, mWidth);
                DfoNative.deinit();

                if (retHeight == DfoNative.RET_SUCCESS && retWidth == DfoNative.RET_SUCCESS
                        && ret == DfoNative.RET_SUCCESS) {
                    mHandler.sendEmptyMessageDelayed(MSG_SET_DONE, 500);
                } else {
                    mHandler.sendEmptyMessageDelayed(MSG_SET_FAILED, 500);
                }
            }
        }).start();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_REBOOT:
            return new AlertDialog.Builder(this)
                    .setTitle(R.string.dfo_resolution)
                    .setMessage(R.string.dfo_resolution_reboot_hint)
                    .setPositiveButton(R.string.dfo_resolution_reboot, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            showDialog(DIALOG_SET);
                            setResolution();
                        }
                    })
                    .setNegativeButton(R.string.dfo_resolution_cancel, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mButton.setEnabled(true);
                            for (int i = 0; i < RESOLUTION_COUNT; i++) {
                                if (mCurrentHeight == mStandardHeight[i]
                                        && mCurrentWidth == mStandardWidth[i]) {
                                    mRadioButtons[i].setChecked(true);
                                    return;
                                }
                            }
                        }
                    })
                    .create();
        case DIALOG_QUERY:
            mDialogQuery = new ProgressDialog(this);
            mDialogQuery.setMessage(getString(R.string.dfo_resolution_query));
            mDialogQuery.setCancelable(false);
            return mDialogQuery;
        case DIALOG_SET:
            mDialogSet = new ProgressDialog(this);
            mDialogSet.setMessage(getString(R.string.dfo_resolution_change));
            mDialogSet.setCancelable(false);
            return mDialogSet;
        default:
            return super.onCreateDialog(id);
        }
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private void showToast(int id) {
        showToast(getString(id));
    }
}

