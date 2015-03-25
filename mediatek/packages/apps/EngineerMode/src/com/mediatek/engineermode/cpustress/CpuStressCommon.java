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

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Handler;
import android.os.IBinder;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.cpustress.CpuStressTestService.ICpuStressTestComplete;
import com.mediatek.xlog.Xlog;

public class CpuStressCommon extends Activity implements ServiceConnection, ICpuStressTestComplete {

    private static final String TAG = "EM/CpuStressCommon";
    static final int INDEX_UPDATE_RADIOBTN = 1;
    static final int INDEX_UPDATE_RADIOGROUP = 2;
    static final int DIALOG_WAIT = 1001;
    CpuStressTestService mBoundService = null;
    Handler mHandler = null;

    @Override
    protected void onStart() {
        super.onStart();
        showDialog(DIALOG_WAIT);
        bindService(new Intent(this, CpuStressTestService.class), this, BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        // mBoundService.testObject = null;
        unbindService(this);
        super.onStop();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        ProgressDialog dialog = null;
        if (DIALOG_WAIT == id) {
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.hqa_cpustress_dialog_waiting_title);
            dialog.setMessage(getString(R.string.hqa_cpustress_dialog_waiting_message));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
        } else {
            dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.hqa_cpustress_dialog_error_title);
            dialog.setMessage(getString(R.string.hqa_cpustress_dialog_error_message));
        }
        return dialog;
    }

    public void onServiceConnected(ComponentName name, IBinder service) {
        Xlog.v(TAG, "Enter onServiceConnected");
        mBoundService = ((CpuStressTestService.StressTestBinder) service).getService();
        mBoundService.mTestClass = this;
        mHandler.sendEmptyMessage(INDEX_UPDATE_RADIOBTN);
    }

    public void onServiceDisconnected(ComponentName name) {
        Xlog.v(TAG, "Enter onServiceDisconnected");
        // mBoundService.testObject = null;
        mBoundService = null;
    }

    public void onUpdateTestResult() {
        mHandler.sendEmptyMessage(INDEX_UPDATE_RADIOGROUP);
    }
}
