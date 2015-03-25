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

import android.database.Cursor;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.provider.message.MmsContent;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.message.FastMmsResourceCursorParser;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.util.Global;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class FastMmsResourceCursorParserTest extends AndroidTestCase {

    private MessageProxy mMessageProxy;

    private FastMmsResourceCursorParser mCursorParser;

    private ByteBuffer mBuffer = ByteBuffer
            .allocate(Global.DEFAULT_BUFFER_SIZE);

    IRawBlockConsumer mConsumer = new IRawBlockConsumer() {

        public void consume(byte[] block, int blockNo, int totalNo) {

        }
    };

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMessageProxy = MessageProxy.getInstance(getContext());
        mBuffer.clear();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_OnParseCursorToRaw() {
        int result;
        ArrayList<Long> threadIdsToReturn = new ArrayList<Long>();
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);

        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.MMS_RAW, threadIdsToReturn);
        mMessageProxy.importMms(MessageUtils.getMmsRaw(), threadIdsToReturn);

        Cursor cursor = mMessageProxy.getContentResolver().query(
                MmsContent.CONTENT_URI_PART,
                new String[] { MmsContent.COLUMN_PART_ID,
                        MmsContent.COLUMN_PART_MID, MmsContent.COLUMN_PART_SEQ,
                        MmsContent.COLUMN_PART_CONTENTTYPE,
                        MmsContent.COLUMN_PART_NAME,
                        MmsContent.COLUMN_PART_CHARSET,
                        MmsContent.COLUMN_PART_CID, MmsContent.COLUMN_PART_CL,
                        MmsContent.COLUMN_PART_DATAPATH,
                        MmsContent.COLUMN_PART_TEXT },
                null, null,
                MmsContent.COLUMN_PART_ID);
        assertTrue(cursor.moveToNext());
        mCursorParser = new FastMmsResourceCursorParser(cursor, mConsumer,
                mBuffer, mMessageProxy, 1);
        result = mCursorParser.onParseCursorToRaw(cursor, mBuffer);
        assertTrue(result >= 0);
        new FastMmsResourceCursorParser(cursor, mConsumer);
        new FastMmsResourceCursorParser(cursor, mConsumer, mBuffer,
                mMessageProxy);

        result = mCursorParser.onParseCursorToRaw(null, mBuffer);
        assertTrue(result == 0);
        result = mCursorParser.onParseCursorToRaw(cursor, null);
        assertTrue(result == 0);
        result = mCursorParser.onParseCursorToRaw(cursor, mBuffer);
        assertTrue(result >= 0);
        mBuffer.clear();
        Cursor cursor2 = mMessageProxy.getContentResolver().query(
                MmsContent.CONTENT_URI_PART,
                new String[] { MmsContent.COLUMN_PART_ID,
                        MmsContent.COLUMN_PART_MID, MmsContent.COLUMN_PART_SEQ,
                        MmsContent.COLUMN_PART_CONTENTTYPE,
                        MmsContent.COLUMN_PART_NAME,
                        MmsContent.COLUMN_PART_CHARSET,
                        MmsContent.COLUMN_PART_CID, MmsContent.COLUMN_PART_CL,
                        MmsContent.COLUMN_PART_DATAPATH,
                        MmsContent.COLUMN_PART_TEXT },
                null, null,
                MmsContent.COLUMN_PART_ID);
        mCursorParser = new FastMmsResourceCursorParser(cursor, mConsumer,
                mBuffer, mMessageProxy, mMessageProxy.getMaxMmsId());
        while (cursor2.moveToNext()) {
            result = mCursorParser.onParseCursorToRaw(cursor2, mBuffer);
            assertTrue(result >= 0);
        }
        cursor.close();
        cursor2.close();
    }
}
