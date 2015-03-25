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

package com.mediatek.apst.target.ftp;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;

public class FtpService extends Service {

    private static final String CHARSET_GBK = "GBK";
    private static final String CHARSET_BIG5 = "Big5-HKSCS";
    private static final String CHARSET_SHIFT_JIS = "Shift-JIS";
    private static final String CHARSET_EUC_KR = "EUC-KR";
    private static final String CHARSET_EUC_JP = "EUC-JP";
    private static final String PC_CHARSET_950 = "950";
    private static final String PC_CHARSET_932 = "932";
    private static final String PC_CHARSET_936 = "936";
    private static final String PC_CHARSET_CONTAIN_125 = "WINDOWS-125";
    private static final String PC_CHARSET_CONTAIN_8859 = "8859";
    private static final String PC_CHARSET_CONTAIN_2022 = "2022";
    private static final String ACTION_ENCODING = "com.mediatek.apst.target.action.PC_ENCODING";
    private static final String KEY_ENCODING = "encoding";
    public static String sFtpEncoding = CHARSET_GBK;
    private FtpServer mFtpServer;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (null != intent) {
                String charset = intent.getStringExtra(KEY_ENCODING);
                if (null != charset) {
                    String upper = charset.toUpperCase();
                    if (upper.contains(PC_CHARSET_950)) {
                        sFtpEncoding = CHARSET_BIG5;
                    } else if (upper.contains(PC_CHARSET_CONTAIN_125)) {
                        sFtpEncoding = charset;
                    } else if (upper.contains(PC_CHARSET_CONTAIN_8859)) {
                        sFtpEncoding = upper;
                    } else if (upper.contains(PC_CHARSET_932)) {
                        sFtpEncoding = CHARSET_SHIFT_JIS;
                    } else if (upper.contains(PC_CHARSET_936)) {
                        sFtpEncoding = CHARSET_GBK;
                    } else if (upper.contains(CHARSET_EUC_KR)) {
                        sFtpEncoding = CHARSET_EUC_KR;
                    } else if (upper.contains(CHARSET_EUC_JP)) {
                        sFtpEncoding = CHARSET_EUC_JP;
                    } else if (upper.contains(PC_CHARSET_CONTAIN_2022)) {
                        sFtpEncoding = charset;
                    }
                }
            }
        }
        
    };

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mFtpServer = new FtpServer();
        mFtpServer.start();
        registerReceiver(mReceiver, new IntentFilter(ACTION_ENCODING));
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        mFtpServer.destroy();
        super.onDestroy();
    }

}
