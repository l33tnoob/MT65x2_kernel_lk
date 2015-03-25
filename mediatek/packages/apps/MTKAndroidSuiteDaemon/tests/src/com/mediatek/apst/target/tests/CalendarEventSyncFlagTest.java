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
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.sync.CalendarEventSyncFlag;

import java.nio.ByteBuffer;

public class CalendarEventSyncFlagTest extends AndroidTestCase {
    private CalendarEventSyncFlag mCalendarEventSyncFlag;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCalendarEventSyncFlag = new CalendarEventSyncFlag();

    }

    @Override
    protected void tearDown() throws Exception {

        mCalendarEventSyncFlag = null;
        super.tearDown();
    }

    public void test01_getTimeFrom() {
        mCalendarEventSyncFlag.setTimeFrom(123);
        assertEquals(123, mCalendarEventSyncFlag.getTimeFrom());
    }

    public void test02_getTimeZone() {
        mCalendarEventSyncFlag.setTimeZone("Beijing");
        assertEquals("Beijing", mCalendarEventSyncFlag.getTimeZone());
    }

    public void test03_getTitle() {
        mCalendarEventSyncFlag.setTitle("Meeting");
        assertEquals("Meeting", mCalendarEventSyncFlag.getTitle());
    }

    public void test04_getModifyTime() {
        mCalendarEventSyncFlag.setModifyTime(123456);
        assertEquals(123456, mCalendarEventSyncFlag.getModifyTime());
    }

    public void test05_getCalendarId() {
        mCalendarEventSyncFlag.setCalendarId(11);
        assertEquals(11, mCalendarEventSyncFlag.getCalendarId());
    }

    public void test06_writeRawWithVersion() {
        ByteBuffer buffer = ByteBuffer.allocate(200);
        int versionCode = Config.VERSION_CODE;
        mCalendarEventSyncFlag.setCalendarId(11);
        mCalendarEventSyncFlag.setModifyTime(123);
        mCalendarEventSyncFlag.setTimeFrom(10);
        mCalendarEventSyncFlag.setTimeZone("Beijing");
        mCalendarEventSyncFlag.setTitle("Meeting");
        mCalendarEventSyncFlag.writeRawWithVersion(buffer, versionCode);
        buffer.position(8);
        assertEquals(123, buffer.getLong());
        assertEquals(11, buffer.getLong());
        assertEquals("Meeting", RawTransUtil.getString(buffer));
        assertEquals("Beijing", RawTransUtil.getString(buffer));
        assertEquals(10, buffer.getLong());
    }

    public void test07_readRawWithVersion() {
        ByteBuffer buffer = ByteBuffer.allocate(200);
        int versionCode = Config.VERSION_CODE;
        mCalendarEventSyncFlag.setCalendarId(11);
        mCalendarEventSyncFlag.setModifyTime(123);
        mCalendarEventSyncFlag.setTimeFrom(10);
        mCalendarEventSyncFlag.setTimeZone("Beijing");
        mCalendarEventSyncFlag.setTitle("Meeting");
        mCalendarEventSyncFlag.writeRawWithVersion(buffer, versionCode);
        buffer.position(0);
        mCalendarEventSyncFlag.readRawWithVersion(buffer, versionCode);
        assertEquals(123, mCalendarEventSyncFlag.getModifyTime());
        assertEquals(11, mCalendarEventSyncFlag.getCalendarId());
        assertEquals("Meeting", mCalendarEventSyncFlag.getTitle());
        assertEquals("Beijing", mCalendarEventSyncFlag.getTimeZone());
        assertEquals(10, mCalendarEventSyncFlag.getTimeFrom());
    }

    public void test08_getId() {
        mCalendarEventSyncFlag.setId(101);
        assertEquals(101, mCalendarEventSyncFlag.getId());
    }

    public void test09_clone() {
        try {
            assertTrue(mCalendarEventSyncFlag.clone() instanceof DatabaseRecordEntity);
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test10_writeRaw() {
        mCalendarEventSyncFlag.setId(101);
        ByteBuffer buffer = ByteBuffer.allocate(100);
        mCalendarEventSyncFlag.writeRaw(buffer);
        buffer.position(0);
        assertEquals(101, buffer.getLong());
    }

    public void test11_readRaw() {
        mCalendarEventSyncFlag.setId(101);
        ByteBuffer buffer = ByteBuffer.allocate(100);
        mCalendarEventSyncFlag.writeRaw(buffer);
        buffer.position(0);
        mCalendarEventSyncFlag.readRaw(buffer);
        assertEquals(101, mCalendarEventSyncFlag.getId());
    }
}
