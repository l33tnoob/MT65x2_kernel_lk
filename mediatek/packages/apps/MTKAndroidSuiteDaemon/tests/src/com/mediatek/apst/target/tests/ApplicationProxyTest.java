/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.apst.target.tests;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.app.ApplicationProxy;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.app.ApplicationInfo;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class ApplicationProxyTest extends AndroidTestCase {
    private ApplicationProxy mAppProxy;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        super.tearDown();
    }

    public void test01_getInstance() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        assertNotNull(mAppProxy);
    }

    public void test02_getEntity() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        assertNotNull(mAppProxy);
        int AppCount = mAppProxy.getApplicationsCount();
        assertTrue(AppCount > 0);
    }

    /**
     * consumer is not null, buffer is not null.
     */
    public void test03_fastGetAllApplications() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            // @Override
            public void consume(byte[] block, int blockNo, int totalNo) {}

        };
        ByteBuffer buffer = Global.getByteBuffer();
        mAppProxy.fastGetAllApplications(consumer, buffer, 20,  20);
    }

    /**
     * consume is null.
     */
    public void test04_fastGetAllApplications() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        IRawBlockConsumer consumer = null;
        ByteBuffer buffer = Global.getByteBuffer();
        mAppProxy.fastGetAllApplications(consumer, buffer,  20,  20);
    }

    /**
     * buffer is null.
     */
    public void test05_fastGetAllApplications() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {}
        };
        ByteBuffer buffer = null;
        mAppProxy.fastGetAllApplications(consumer, buffer,  20,  20);
    }

    /**
     * The buffer is not null, the consumer is not null.
     */
    public void test06_fastGetAllApps2Backup() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {}
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mAppProxy.fastGetAllApps2Backup(consumer, buffer,  20,  20);
        // consumer is null.
        consumer = null;
        mAppProxy.fastGetAllApps2Backup(consumer, buffer,  20,  20);
        // buffer is null.
        consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {}
        };
        buffer = null;
        mAppProxy.fastGetAllApps2Backup(consumer, buffer,  20,  20);
    }

    public void test07_getApplicationsCount() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        int appCount = mAppProxy.getApplicationsCount();
        assertTrue(appCount > 0);
    }

    public void test08_getApplicationsForUid() {
        mAppProxy = ApplicationProxy.getInstance(mContext);
        int contactsAppUid = 0;
        int messageAppUid = 0;
        try {
            contactsAppUid = mContext.getPackageManager().getPackageInfo(
                    "com.android.contacts", 0).applicationInfo.uid;
            messageAppUid = mContext.getPackageManager().getPackageInfo(
                    "com.android.mms", 0).applicationInfo.uid;
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        ArrayList<ApplicationInfo> appInfoList = mAppProxy.getApplicationsForUid(contactsAppUid);
        ArrayList<ApplicationInfo> appInfoList2 = mAppProxy.getApplicationsForUid(messageAppUid);
        assertNotNull(appInfoList);
        assertNotNull(appInfoList2);
    }
}
