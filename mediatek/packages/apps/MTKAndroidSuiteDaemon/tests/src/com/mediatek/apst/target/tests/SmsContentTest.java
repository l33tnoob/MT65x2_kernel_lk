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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.test.AndroidTestCase;

import com.mediatek.android.content.MeasuredContentValues;
import com.mediatek.apst.target.data.provider.message.SmsContent;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.message.Message;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class SmsContentTest extends AndroidTestCase {
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

    /**
     * The cursor is null.
     */
    public void test01_cursorToSms() {
        Cursor smsCursor = null;
        Sms sms = SmsContent.cursorToSms(smsCursor);
        assertNull(sms);
    }

    /**
     * The cursor is not null.
     */
    // Remove for KK
   /* public void test02_cursorToSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        MessageProxy.getInstance(mContext).importSms(MessageUtils.SMS_RAW, threadIdsToReturn);
        Cursor smsCursor = mContext.getContentResolver().query(SmsContent.CONTENT_URI, null, null, null, null);
        assertTrue(smsCursor.moveToNext());
        Sms sms = SmsContent.cursorToSms(smsCursor);
        if (smsCursor.getPosition() == -1 || smsCursor.getPosition() == smsCursor.getCount()) {
            assertNull(sms);
        } else {
            assertNotNull(sms);
        }
        smsCursor.close();
    }*/
 // Remove for KK
  /* public void test03_cursorToRaw() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        MessageProxy.getInstance(mContext).importSms(MessageUtils.SMS_RAW, threadIdsToReturn);
        Cursor smsCursor = mContext.getContentResolver().query(SmsContent.CONTENT_URI, null, null, null, null);
        assertTrue(smsCursor.moveToNext());
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        int result = SmsContent.cursorToRaw(null, buffer);
        assertTrue(result == IRawBufferWritable.RESULT_FAIL);
        SmsContent.cursorToRaw(smsCursor, null);
        assertTrue(result == IRawBufferWritable.RESULT_FAIL);
        SmsContent.cursorToRaw(smsCursor, buffer);
        assertTrue(result >= IRawBufferWritable.RESULT_FAIL);
        smsCursor.close();

    }*/

    public void test04_createContentValues() {
        ContentValues result = null;
        result = SmsContent.createContentValues(getAsms(), true, true);
        assertNotNull(result);
        result = SmsContent.createContentValues(null, true, true);
        assertNull(result);
    }

    public void test05_createMeasuredContentValues() {
        MeasuredContentValues result = null;
        result = SmsContent.createMeasuredContentValues(getAsms(), true, true);
        assertNotNull(result);
        result = SmsContent.createMeasuredContentValues(null, true, true);
        assertNull(result);
    }

    private Sms getAsms() {
        Sms sms = new Sms();
        sms.setBody("a short message");
        sms.setBox(Message.BOX_INBOX);
        sms.setLocked(false);
        sms.setRead(false);
        sms.setDate_sent(0);
        sms.setDate(1266752658);
        sms.setSimId(1);
        TargetAddress target = new TargetAddress();
        target.setAddress("15965232514");
        target.setName("Lisi");
        sms.setTarget(target);
        return sms;
    }
}
