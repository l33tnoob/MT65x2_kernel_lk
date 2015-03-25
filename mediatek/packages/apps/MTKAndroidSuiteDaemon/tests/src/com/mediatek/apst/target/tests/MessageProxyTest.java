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

import android.content.Context;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.service.SmsSender;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.message.Message;
import com.mediatek.apst.util.entity.message.Mms;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.LinkedList;

public class MessageProxyTest extends AndroidTestCase {
    private Context mContext;
    private MessageProxy mMessageProxy;
    private ByteBuffer mBuffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
    private IRawBlockConsumer mConsumer = new IRawBlockConsumer() {

        public void consume(byte[] block, int blockNo, int totalNo) {

        }

    };

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mMessageProxy = MessageProxy.getInstance(mContext);
        mBuffer.clear();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_getinstance() {
        assertNotNull(mMessageProxy);
    }

    public void test02_asyncGetAllSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        Sms sms = MessageUtils.getAsms();
        mBuffer.putInt(1);
        sms.writeRawWithVersion(mBuffer, Config.VERSION_CODE);
        mBuffer.position(0);
        byte[] raw = mBuffer.array();
        long[] ids = mMessageProxy.importSms(raw, threadIdsToReturn);
        assertTrue(ids.length > 0);
        mMessageProxy.importSms(MessageUtils.SMS_RAW, threadIdsToReturn);
        mMessageProxy.importSms(raw, threadIdsToReturn);
        mMessageProxy.asyncGetAllSms(mConsumer, mBuffer);
        assertTrue(mBuffer.position() > 0);
    }

    public void test03_asyncGetAllMms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.asyncGetAllMms(mConsumer, mBuffer);
        assertTrue(mBuffer.position() > 0);
    }

    public void test04_getOneMmsResource() {
        long maxMmsId = mMessageProxy.getMaxMmsId();
        mMessageProxy.getOneMmsResource(mConsumer, mBuffer, maxMmsId);
    }

    public void test05_getMmsData() {
        final LinkedList<Long> list = new LinkedList<Long>();
        long maxMmsId = mMessageProxy.getMaxMmsId();
        list.add(maxMmsId);
        mMessageProxy.getMmsData(mConsumer, mBuffer, false, list);
        mMessageProxy.getMmsData(mConsumer, mBuffer, true, list);
    }

    public void test06_asyncGetPhoneList() {
        mMessageProxy.asyncGetPhoneList(mConsumer, mBuffer);
    }

    /**
     * Test "long[] importSms(final byte[] raw, final ArrayList<Long> threadIdsToReturn) ". raw is null.
     */
    public void test07_importSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        int smsCount = mMessageProxy.getSmsCount();
        assertTrue(smsCount > 0);
        mBuffer.putInt(smsCount);
        mMessageProxy.asyncGetAllSms(mConsumer, mBuffer);
        long[] ids = mMessageProxy.importSms(mBuffer.array(), threadIdsToReturn);
        // assertTrue(ids.length > 0);
    }

    /**
     * Test "long[] importSms(final byte[] raw, final ArrayList<Long> threadIdsToReturn) ".
     */
    public void test08_importSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        Sms sms = MessageUtils.getAsms();
        mBuffer.putInt(1);
        sms.writeRawWithVersion(mBuffer, Config.VERSION_CODE);
        mBuffer.position(0);
        byte[] raw = mBuffer.array();
        long[] ids = mMessageProxy.importSms(raw, threadIdsToReturn);
        assertTrue(ids.length > 0);
    }

    public void test09_importMms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        int mmsCount = mMessageProxy.getMmsCount();
        assertTrue(mmsCount > 0);
        mBuffer.putInt(mmsCount);
        mMessageProxy.asyncGetAllMms(mConsumer, mBuffer);
        long[] ids = mMessageProxy.importMms(mBuffer.array(), threadIdsToReturn);
        mMessageProxy.importMms(mBuffer.array(), threadIdsToReturn);

        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);

        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        // assertTrue(ids.length > 0);
    }

    public void test10_importMms() {
        Mms mms = MessageUtils.getAmms();
        mBuffer.putInt(1);
        mms.writeAllWithVersion(mBuffer, Config.VERSION_CODE);
        mBuffer.position(0);
        byte[] raw = mBuffer.array();
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        long[] ids = mMessageProxy.importMms(raw, threadIdsToReturn);
        assertTrue(ids.length > 0);
    }

    public void test11_getMaxSmsId() {
        assertNotNull(mMessageProxy.getMaxSmsId());
    }

    public void test12_getMaxMmsId() {
        assertNotNull(mMessageProxy.getMaxMmsId());
    }

    public void test13_getMaxMmsId() {
        assertNotNull(mMessageProxy.getMaxMmsId(Mms.BOX_INBOX));
        assertNotNull(mMessageProxy.getMaxMmsId(Mms.BOX_DRAFT));
    }

    public void test13_getMaxMmsPartId() {
        assertNotNull(mMessageProxy.getMaxMmsPartId());
    }

    public void test14_findSms() {
        long afterTimeof = 0l;
        String address = MessageUtils.getAsms().getTarget().getAddress();
        String smsBody = MessageUtils.getAsms().getBody();
        int box = MessageUtils.getAsms().getBox();
        Sms findSms = mMessageProxy.findSms(afterTimeof, address, smsBody, box);
        assertNotNull(findSms);
    }

    public void test15_findSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        Sms sms = MessageUtils.getAsms();
        mBuffer.putInt(1);
        sms.writeRawWithVersion(mBuffer, Config.VERSION_CODE);
        mBuffer.position(0);
        byte[] raw = mBuffer.array();
        mMessageProxy.importSms(raw, threadIdsToReturn);
        String address = MessageUtils.getAsms().getTarget().getAddress();
        String smsBody = MessageUtils.getAsms().getBody();
        SmsSender smsSender = SmsSender.getInstance();
        mMessageProxy.resendSms(mMessageProxy.getMaxSmsId(), 0l, null, null, smsSender, 1);
        mMessageProxy.resendSms(mMessageProxy.getMaxSmsId(), 0l, "", null, smsSender, 1);
        mMessageProxy.resendSms(mMessageProxy.getMaxSmsId(), 0l, smsBody, null, smsSender, 1);
        mMessageProxy.resendSms(mMessageProxy.getMaxSmsId(), 0l, smsBody, address, smsSender, 1);
    }

    public void test16_saveSmsDraft() {
        mMessageProxy.saveSmsDraft(MessageUtils.getAsms().getBody(), new String[] { "1", "2" });
    }

    public void test17_sendSms() {
        mMessageProxy.sendSms(MessageUtils.getAsms().getBody(), new String[] { "1", "2" }, SmsSender.getInstance(), 1);
    }

    public void test15_lookupContact() {
        String number = "15865232541";
        TargetAddress target = mMessageProxy.lookupContact(number);
        assertNotNull(target);
    }

    /**
     * Test "int moveSmsToBox(final long ids[], final boolean checkDates, final long[] dates, final int box)".
     */
    public void test16_moveSmsToBox() {
        long[] ids = { 1, 2, 3 };
        long[] dates = { 1, 2, 3 };
        int updateCount = mMessageProxy.moveSmsToBox(ids, true, dates, Sms.BOX_DRAFT);
        assertTrue(updateCount > 0);
        int updateCount2 = mMessageProxy.moveSmsToBox(ids, false, dates, Sms.BOX_DRAFT);
        assertTrue(updateCount2 >= 0);
    }

    /**
     * Test "int moveMmsToBox(final long ids[], final boolean checkDates, final long[] dates, final int box)".
     */
    public void test17_moveMmsToBox() {
        long[] ids = { 1, 2, 3 };
        long[] dates = { 1, 2, 3 };
        int updateCount = mMessageProxy.moveMmsToBox(ids, true, dates, Mms.BOX_DRAFT);
        assertTrue(updateCount >= 0);
        int updateCount2 = mMessageProxy.moveMmsToBox(ids, false, dates, Mms.BOX_DRAFT);
        assertTrue(updateCount2 >= 0);
    }

    /**
     * Test "int lockSms(final long id, final boolean state)".
     */
    public void test18_lockSms() {
        long id = 1l;
        boolean state = true;
        int updateCount = mMessageProxy.lockSms(id, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int lockSms(final long[] ids, final boolean state)".
     */
    public void test19_lockSms() {
        long[] ids = { 1, 2, 3 };
        assertTrue(ids.length > 0);
        boolean state = true;
        int updateCount = mMessageProxy.lockSms(ids, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int lockMms(final long id, final boolean state)".
     */
    public void test20_lockMms() {
        long id = 1l;
        boolean state = true;
        int updateCount = mMessageProxy.lockMms(id, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int lockMms(final long[] ids, final boolean state)".
     */
    public void test21_lockMms() {
        long[] ids = { 1, 2, 3 };
        assertTrue(ids.length > 0);
        boolean state = true;
        int updateCount = mMessageProxy.lockMms(ids, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int markSmsAsRead(final long id, final boolean state)".
     */
    public void test22_markSmsAsRead() {
        long id = 1l;
        boolean state = true;
        int updateCount = mMessageProxy.markSmsAsRead(id, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int markSmsAsRead(final long[] ids, final boolean state)".
     */
    public void test23_markSmsAsRead() {
        long[] ids = { 1, 2, 3 };
        assertTrue(ids.length > 0);
        boolean state = true;
        int updateCount = mMessageProxy.markSmsAsRead(ids, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int markMmsAsRead(final long id, final boolean state)".
     */
    public void test24_markMmsAsRead() {
        long id = 1l;
        boolean state = true;
        int updateCount = mMessageProxy.markMmsAsRead(id, state);
        assertTrue(updateCount >= 0);
    }

    /**
     * Test "int markMmsAsRead(final long[] ids, final boolean state)".
     */
    public void test25_markMmsAsRead() {
        long[] ids = { 1, 2, 3 };
        assertTrue(ids.length > 0);
        boolean state = true;
        int updateCount = mMessageProxy.markMmsAsRead(ids, state);
        assertTrue(updateCount >= 0);
    }

    public void test26_getSms() {
        long id = mMessageProxy.getMaxSmsId();
        assertTrue(id >= 0);
        Sms sms = mMessageProxy.getSms(id);
        assertNotNull(sms);
    }

    public void test27_getMms() {
        long id = mMessageProxy.getMaxMmsId();
        assertTrue(id >= 0);
        Mms mms = mMessageProxy.getMms(id);
        assertNotNull(mms);
    }

    public void test28_getMmsCount() {
        int mmsCount = mMessageProxy.getMmsCount();
        assertTrue(mmsCount >= 0);
    }

    public void test29_getSmsCount() {
        int smsCount = mMessageProxy.getSmsCount();
        assertTrue(smsCount >= 0);
    }

    public void test30_insertMms() {
        mMessageProxy.insertMms(getMmsFromDB());
        mMessageProxy.insertMms(getMmsFromDB());
    }

    /**
     * Test "long insertSms(final String address)".
     */
    public void test31_insertSms() {
        String address = "15865232541";
        long threadId = mMessageProxy.insertSms(address);
        // assertTrue(threadId >= 0);
    }

    /**
     * Test "long insertSms(final Sms sms, final boolean regenThreadId)".
     */
    public void test32_insertSms() {
        Sms sms = MessageUtils.getAsms();
        boolean regenThreadId = false;
        long insertId = mMessageProxy.insertSms(sms, regenThreadId);
        assertTrue(insertId >= 0);
    }

    public void test33_updateSms() {
        Sms newSms = mMessageProxy.getSms(mMessageProxy.getMaxSmsId());
        long updateCount = 0l;
        updateCount = mMessageProxy.updateSms(mMessageProxy.getMaxSmsId() - 1, newSms, false, true);
        mMessageProxy.updateSms(mMessageProxy.getMaxSmsId() - 1, newSms, true, true);
        assertTrue(updateCount >= 0);
    }

    public void test34_getOrCreateThreadId() {
        long threadId = mMessageProxy.getOrCreateThreadId("1");
        assertTrue(threadId >= -1);
    }

    public void test35_getOrCreateThreadId2() {
        String[] recipientList = { "1", "2" };
        long threadId = mMessageProxy.getOrCreateThreadId2(recipientList);
        assertTrue(threadId >= -1);
    }

    public void test36_getMessagesCount() {
        int count = mMessageProxy.getMessagesCount();
        assertTrue(count >= 0);
    }

    public void test37_getSmsAddress() {
        String smsAddress = mMessageProxy.getSmsAddress(mMessageProxy.getMaxSmsId());
        assertNotNull(smsAddress);
    }

    public void test38_getMmsAddress() {
        Mms mms = mMessageProxy.getMms(mMessageProxy.getMaxMmsId());
        mMessageProxy.getMmsAddress(mms);
    }

    // ************************* Test delete methods **********************

    /**
     * Test "int deleteSms(final long id, final boolean checkDate, final long date)".
     */
    public void test38_deleteSms() {
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        Sms sms = MessageUtils.getAsms();
        mBuffer.putInt(1);
        sms.writeRawWithVersion(mBuffer, Config.VERSION_CODE);
        mBuffer.position(0);
        byte[] raw = mBuffer.array();
        long[] ids = mMessageProxy.importSms(raw, threadIdsToReturn);
        assertTrue(ids.length > 0);
        boolean checkDate = true;
        long date = 2012;
        int deleteSmsCount;
        deleteSmsCount = mMessageProxy.deleteSms(mMessageProxy.getMaxSmsId(), checkDate, date);
        assertTrue(deleteSmsCount >= 0);
        deleteSmsCount = mMessageProxy.deleteSms(ids[0], false, date);
        assertTrue(deleteSmsCount >= 0);
    }

    /**
     * Test "int deleteSms(final long[] ids, final boolean checkDates, final long[] dates)".
     */
    public void test39_deleteSms() {
        long[] ids = { 1, 2, 3 };
        boolean checkDates = true;
        long[] dates = { 2012 };
        mMessageProxy.deleteSms(ids, checkDates, dates);
        int deleteSms = mMessageProxy.deleteSms(ids, false, dates);
        assertTrue(deleteSms >= 0);
    }

    /**
     * Test "int deleteMms(final long id, final boolean checkDate, final long date)".
     */
    public void test40_deleteMms() {

        long date = 0;
        int deleteMms;
        deleteMms = mMessageProxy.deleteMms(mMessageProxy.getMaxMmsId(), true, date);
        assertTrue(deleteMms >= 0);
        deleteMms = mMessageProxy.deleteMms(mMessageProxy.getMaxMmsId(), false, date);
        assertTrue(deleteMms >= 0);
    }

    /**
     * Test "int deleteMms(final long[] ids, final boolean checkDates, final long[] dates)".
     */
    public void test41_deleteMms() {
        long[] ids = { 1, 2, 3 };
        long[] dates = { 2012 };
        mMessageProxy.deleteMms(ids, true, dates);
        int deleteMms = mMessageProxy.deleteMms(ids, false, dates);
        assertTrue(deleteMms >= 0);
    }

    public void test42_clearMessageBox() {
        int box = Message.BOX_DRAFT;
        boolean keepLockedMessage = true;
        int deleteCount = mMessageProxy.clearMessageBox(box, keepLockedMessage);
        assertTrue(deleteCount >= 0);
    }

    public void test43_deleteAllMessages() {
        boolean keepLockedMessage = true;
        int deleteMessages = mMessageProxy.deleteAllMessages(keepLockedMessage);
        assertTrue(deleteMessages >= 0);
        assertEquals(0, mMessageProxy.getMessagesCount());
    }

    // ************************* Test delete done **********************

    /**
     * A MMS with attachments in phone is better before test.
     * 
     * @return
     */
    private Mms getMmsFromDB() {
        Mms mms = new Mms();
        long maxMmsId = mMessageProxy.getMaxMmsId();
        if (maxMmsId > 0) {
            mms = mMessageProxy.getMms(maxMmsId);
            return mms;
        } else {
            mms = MessageUtils.getAmms();
            mMessageProxy.insertMms(mms);
            getMmsFromDB();
            return mms;
        }
    }
}
