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

import android.test.AndroidTestCase;

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.util.entity.sync.ContactsSyncFlag;

import java.nio.ByteBuffer;

public class ContactsSyncFlagTest extends AndroidTestCase {
    private ContactsSyncFlag mContactsSyncFlag;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContactsSyncFlag = new ContactsSyncFlag();

    }

    @Override
    protected void tearDown() throws Exception {

        mContactsSyncFlag = null;
        super.tearDown();
    }

    public void test01_getVersion() {
        mContactsSyncFlag.setVersion(1);
        assertEquals(1, mContactsSyncFlag.getVersion());
    }

    public void test02_getDisplayName() {
        mContactsSyncFlag.setDisplayName("display name");
        assertEquals("display name", mContactsSyncFlag.getDisplayName());
    }

    public void test03_getModifyTime() {
        mContactsSyncFlag.setModifyTime(12345);
        assertEquals(12345, mContactsSyncFlag.getModifyTime());
    }

    public void test04_readRawWithVersion() {
        ByteBuffer buffer = ByteBuffer.allocate(100);
        int versionCode = Config.VERSION_CODE;
        ContactsSyncFlag contactsSyncFlag = new ContactsSyncFlag();
        contactsSyncFlag.setDisplayName("display name");
        contactsSyncFlag.setModifyTime(3455);
        contactsSyncFlag.setVersion(2);
        contactsSyncFlag.writeRawWithVersion(buffer, versionCode);
        buffer.position(0);
        mContactsSyncFlag.readRawWithVersion(buffer, versionCode);
        assertEquals("display name", mContactsSyncFlag.getDisplayName());
        assertEquals(3455, mContactsSyncFlag.getModifyTime());
        assertEquals(2, mContactsSyncFlag.getVersion());

    }

}
