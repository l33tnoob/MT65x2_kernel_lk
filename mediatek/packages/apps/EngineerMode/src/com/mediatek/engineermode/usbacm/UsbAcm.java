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

package com.mediatek.engineermode.usbacm;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import android.provider.Settings;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

public class UsbAcm extends Activity implements OnClickListener {
    private static final String TAG = "UsbAcm";

    private static final int BUTTON_NUM = 4;
    private Button[] mBtnList = new Button[BUTTON_NUM];
    private Button mBtnClose = null;
    private Toast mToast;

    private static final boolean DEBUG = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_acm);

        mBtnList[0] = (Button) findViewById(R.id.button_acm_open_gs0);
        mBtnList[1] = (Button) findViewById(R.id.button_acm_open_gs1);
        mBtnList[2] = (Button) findViewById(R.id.button_acm_open_gs2);
        mBtnList[3] = (Button) findViewById(R.id.button_acm_open_gs3);
        mBtnClose = (Button) findViewById(R.id.button_acm_close);

        for (int i = 0; i < mBtnList.length; i++) {
            mBtnList[i].setOnClickListener(this);
        }
        mBtnClose.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        Xlog.v(TAG, "-->onResume");
        super.onResume();
        String tmp = SystemProperties.get("persist.radio.port_index");
        Xlog.v(TAG, "getprop persist.radio.port_index=" + tmp);
        if (!"".equals(tmp)) {
            enableAllButton(false);
        }
    }

    public void onClick(View arg0) {
        for (int i = 0; i < mBtnList.length; i++) {
            if (arg0.getId() == mBtnList[i].getId()) {
                if (openPort(i + 1)) {
                    showToast(R.string.usbacm_open_succeed);
                    enableAllButton(false);
                } else {
                    showToast(R.string.usbacm_open_failed);
                }
            }
        }

        if (arg0.getId() == mBtnClose.getId()) {
            if (closePort()) {
                showToast(R.string.usbacm_close_succeed);
                enableAllButton(true);
            } else {
                showToast(R.string.usbacm_close_failed);
            }
        }
    }

    private boolean openPort(int index) {
        Xlog.v(TAG, "Port Index = " + index);
        SystemProperties.set("persist.radio.port_index", String.valueOf(index));
        if (DEBUG) {
            String tmp = SystemProperties.get("persist.radio.port_index");
            Xlog.v(TAG, "getprop persist.radio.port_index=" + tmp);
        }

        boolean ret =  Settings.Global.putInt(getContentResolver(), Settings.Global.ACM_ENABLED, index);
        if (DEBUG) {
            String tmp = SystemProperties.get("sys.usb.config");
            Xlog.v(TAG, "getprop sys.usb.config=" + tmp);
        }

        return ret;
    }

    private boolean closePort() {
        SystemProperties.set("persist.radio.port_index", "");
        if (DEBUG) {
            String tmp = SystemProperties.get("persist.radio.port_index");
            Xlog.v(TAG, "getprop persist.radio.port_index=" + tmp);
        }

        boolean ret =  Settings.Global.putInt(getContentResolver(), Settings.Global.ACM_ENABLED, 0);
        if (DEBUG) {
            String tmp = SystemProperties.get("sys.usb.config");
            Xlog.v(TAG, "getprop sys.usb.config=" + tmp);
        }

        return ret;
    }

    private void enableAllButton(boolean enable) {
        for (Button btn : mBtnList) {
            btn.setEnabled(enable);
        }
    }

    private void showToast(int msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}
