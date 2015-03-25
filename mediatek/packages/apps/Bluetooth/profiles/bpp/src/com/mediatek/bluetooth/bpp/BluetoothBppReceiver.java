/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;

import android.os.Environment;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.ComponentName;


import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;


import android.widget.Toast;

import android.app.ActivityManager;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;



import java.util.List;


public class BluetoothBppReceiver extends BroadcastReceiver {
    private static final String TAG = "BluetoothBppReceiver";

    public static final String ACTION_PASS_OBJECT = "com.mediatek.bluetooth.bppReceiver.action.PASS_OBJECT";
    private static String mFilePath = null;
    private static String mMimeType = null;
    private static String mFileSize = null;
 
   private static ActivityManager mAm;


    @Override
    public void onReceive(Context context, Intent intent) {

        Xlog.v(TAG, "BPP broadcast receiver receives intent");

        String action = intent.getAction();

        if (BluetoothDevicePicker.ACTION_DEVICE_SELECTED.equals(action)) {

            if( false == Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED) ){
                return;
            }

            mAm = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
            List<ActivityManager.RunningServiceInfo> services = mAm.getRunningServices(100);
            final int NS = services != null ? services.size() : 0;

            for (int i=0; i<NS; i++) {
                ActivityManager.RunningServiceInfo si = services.get(i);
                if ( si.service.getClassName().equals("com.mediatek.bluetooth.bpp.BluetoothBppManager") ) {
                    Toast.makeText(context, R.string.bt_bpp_reentry_error, Toast.LENGTH_LONG).show();
                    return;
                }
            }



            BluetoothDevice remoteDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice);


            Intent in = new Intent(context, BluetoothBppManager.class);
            in.putExtra("action", BluetoothBppManager.ACTION_GET_PRINTER_ATTR);

            in.putExtra(BluetoothBppManager.EXTRA_FILE_PATH, mFilePath);
            in.putExtra(BluetoothBppManager.EXTRA_MIME_TYPE, mMimeType);
            in.putExtra(BluetoothBppManager.EXTRA_FILE_SIZE, mFileSize);

            in.putExtra(BluetoothDevice.EXTRA_DEVICE, remoteDevice);
            context.startService(in);

        } else if (ACTION_PASS_OBJECT.equals(action) ) {
            Xlog.v(TAG, "ACTION_PASS_OBJECT");

            mFilePath = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_PATH);
            mMimeType = intent.getStringExtra(BluetoothBppManager.EXTRA_MIME_TYPE);
            mFileSize = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_SIZE);


        }
           
    }

}

