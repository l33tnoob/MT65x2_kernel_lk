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
 */
package com.mediatek.calendarimporter;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.calendarimporter.service.VCalService.MyBinder;
import com.mediatek.calendarimporter.utils.LogUtils;

public class BindServiceHelper {
    private static final String TAG = "BindServiceHelper";
    private final Context mContext;
    private ServiceConnectedOperation mConnectedOperation;
    protected VCalService mService = null;

    private final ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
            mService.disconnected(mContext.getClass().getName());
            LogUtils.d(TAG, "onServiceDisconnected");
            if (mConnectedOperation != null) {
                mConnectedOperation.serviceUnConnected();
            }
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            LogUtils.d(TAG, "onServiceConnected");
            mService = ((MyBinder) service).getService();
            if (mConnectedOperation != null) {
                mConnectedOperation.serviceConnected(mService);
            }
        }
    };

    /**
     * Constructor function
     * 
     * @param context
     *            the context going to bind the service
     */
    public BindServiceHelper(Context context) {
        mContext = context;
        if (context instanceof ServiceConnectedOperation) {
            mConnectedOperation = (ServiceConnectedOperation) context;
        }
    }

    /**
     * Constructor function
     * 
     * @param context
     *            the context going to bind the service
     * @param listener
     *            operation listener
     */
    public BindServiceHelper(Context context, ServiceConnectedOperation listener) {
        mContext = context;
        mConnectedOperation = listener;
    }

    /**
     * bind the service
     */
    public void onBindService() {
        mContext.bindService(new Intent(mContext.getApplicationContext(), VCalService.class), mServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    /**
     * unBind the service.
     */
    public void unBindService() {
        LogUtils.d(TAG, "unBindService");
        if (mService != null) {
            mContext.unbindService(mServiceConnection);
        }
    }

    /**
     * the interface define the operation when service bind or unBind.
     */
    interface ServiceConnectedOperation {
        void serviceConnected(VCalService service);

        void serviceUnConnected();
    }
}
