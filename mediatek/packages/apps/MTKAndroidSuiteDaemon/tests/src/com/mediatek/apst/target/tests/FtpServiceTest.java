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

package com.mediatek.apst.target.tests;

import com.mediatek.apst.target.ftp.FtpService;

import java.lang.reflect.Field;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.test.ServiceTestCase;

public class FtpServiceTest extends ServiceTestCase<FtpService> {

    private static final String ACTION_ENCODING = "com.mediatek.apst.target.action.PC_ENCODING";
    private static final String KEY_ENCODING = "encoding";
    private Context mContext;
    private ServiceConnection mConn = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            assertNull(service);
            assertNotNull(name);
        }

        public void onServiceDisconnected(ComponentName name) {
            assertNotNull(name);
        }

    };

    public FtpServiceTest() {
        super(FtpService.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getSystemContext();
    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        super.tearDown();
    }

    public void test01_Start() {
        try {
            Intent startIntent = new Intent();
            startIntent.setClass(mContext, FtpService.class);
            startService(startIntent);
            FtpService ftpService = getService();
            assertNotNull(ftpService);
            Class<FtpService> clazz = FtpService.class;
            Field field = clazz.getDeclaredField("mFtpServer");
            assertNotNull(field);
            ftpService.stopService(startIntent);
            Thread.sleep(1000);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    public void test02_Bind() {
        try {
            Intent intent = new Intent();
            intent.setClass(mContext, FtpService.class);
            mContext.bindService(intent, mConn, Context.BIND_AUTO_CREATE);
            Thread.sleep(1000);
            mContext.unbindService(mConn);
            Thread.sleep(1000);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

    public void test03_EncodeBroadcast() {
        final String[] ENCODE_ARRAYS = { "MS950", "WINDOWS-125",
                "8859", "2022", "932", "EUC-KR", "EUC-JP", "936" };
        try {
            Intent intent = new Intent();
            intent.setClass(mContext, FtpService.class);
            startService(intent);
            FtpService ftpService = getService();
            assertNotNull(ftpService);
            Class<FtpService> clazz = FtpService.class;
            Field field = clazz.getDeclaredField("sFtpEncoding");
            String encode = null;
            Intent encodeBroadcast = new Intent(ACTION_ENCODING);
            for (int i = 0; i < ENCODE_ARRAYS.length; i++) {
                encodeBroadcast.removeExtra(KEY_ENCODING);
                encodeBroadcast.putExtra(KEY_ENCODING, ENCODE_ARRAYS[i]);
                mContext.sendBroadcast(encodeBroadcast);
                Thread.sleep(1000);
                encode = (String) field.get(clazz);
                assertNotNull(encode);
            }
            ftpService.stopService(intent);
        } catch (Exception e) {
            fail(e.getMessage());
        }
    }

}
