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
import android.database.Cursor;
import android.net.Uri;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.FastCursorParser;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.data.proxy.bookmark.BookmarkProxy;
import com.mediatek.apst.util.command.contacts.AsyncGetAllGroupsRsp;
import com.mediatek.apst.util.entity.bookmark.BookmarkData;
import com.mediatek.apst.util.entity.bookmark.BookmarkFolder;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class FastCursorParserTest extends AndroidTestCase {
    private FastCursorParser mFastCursorParser;
    private Context mContext;
    Cursor mBookMarkCursor;
    Cursor mBookMarkCursor2 = null;
    ArrayList<BookmarkData> mBookmarkDataList;
    ArrayList<BookmarkFolder> mBookmarkFolderList;
    IRawBlockConsumer mConsumer;
    ByteBuffer mBuffer;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mBookMarkCursor = mContext.getContentResolver().query(
                Uri.parse("content://com.android.browser/bookmarks"), null,
                "deleted=0", null, null);
        mConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
                AsyncGetAllGroupsRsp rsp = new AsyncGetAllGroupsRsp(1);
                rsp.setRaw(block);
                rsp.setProgress(blockNo);
                rsp.setTotal(totalNo);
                // Send response command or append it to batch
            }

        };
        mBuffer = ByteBuffer.allocate(100);
        mFastCursorParser = new FastCursorParser(mBookMarkCursor, mConsumer,
                mBuffer);

        mBookmarkDataList = new ArrayList<BookmarkData>();
        mBookmarkDataList
                .add(new BookmarkData(100, "google", null, 10, 20120602,
                        20120602, 20120602, 20120602, "googleHK", 1, null, 1));
        mBookmarkFolderList = new ArrayList<BookmarkFolder>();
        mBookmarkFolderList.add(new BookmarkFolder(0, 0, 0, "folder", 20120602,
                10));
        BookmarkProxy.getInstance(mContext).insertBookmark(mBookmarkDataList,
                mBookmarkFolderList);

    }

    @Override
    protected void tearDown() throws Exception {

        BookmarkProxy.getInstance(mContext).deleteAll();
        mBookMarkCursor.close();
        mContext = null;
        mFastCursorParser = null;
        super.tearDown();
    }

    public void test01_isBlockReady() {
        assertFalse(mFastCursorParser.isBlockReady());
    }

    public void test02_onParseCursorToRaw() {
        assertEquals(IRawBufferWritable.RESULT_FAIL, mFastCursorParser
                .onParseCursorToRaw(mBookMarkCursor, mBuffer));
    }

    /* abstract class AsyncCursorParser *** */

    public void test03_getCursor() {
        Cursor cursor = mFastCursorParser.getCursor();
        assertSame(mBookMarkCursor, cursor);
    }

    public void test04_getCount() {
        int count = mFastCursorParser.getCount();
        assertEquals(mBookMarkCursor.getCount(), count);
    }

    public void test05_getPosition() {
        assertEquals(0, mFastCursorParser.getPosition());
        mFastCursorParser.parse();
        assertTrue(mFastCursorParser.getPosition() > 0);
    }

    /**
     * Cursor is null.
     */
    public void test06_getCursorPosition() {
        FastCursorParser fastCursorParser = new FastCursorParser(
                mBookMarkCursor2, mConsumer, mBuffer);
        int cursorPosition = fastCursorParser.getCursorPosition();
        assertEquals(-1, cursorPosition);
    }

    /**
     * Cursor is not null.
     */
    public void test07_getCursorPosition() {
        int cursorPisition = mFastCursorParser.getCursorPosition();
        assertEquals(mBookMarkCursor.getPosition(), cursorPisition);
    }

    public void test08_moveToNext() {
        FastCursorParser fastCursorParser = new FastCursorParser(
                mBookMarkCursor2, mConsumer, mBuffer);
        assertFalse(fastCursorParser.moveToNext());
    }

    public void test09_moveToNext() {
        int cursorPosition = mFastCursorParser.getCursorPosition();
        int count = mFastCursorParser.getCount();
        if (cursorPosition >= count - 1) {
            assertFalse(mFastCursorParser.moveToNext());
        } else if (cursorPosition >= 0 && cursorPosition < count - 1) {
            assertTrue(mFastCursorParser.moveToNext());
        }

    }

    public void test10_moveToPrevious() {
        FastCursorParser fastCursorParser = new FastCursorParser(
                mBookMarkCursor2, mConsumer, mBuffer);
        assertFalse(fastCursorParser.moveToPrevious());
    }

    public void test11_moveToPrevious() {
        int cursorPosition = mFastCursorParser.getCursorPosition();
        if (cursorPosition <= 0) {
            assertFalse(mFastCursorParser.moveToPrevious());
        } else {
            assertTrue(mFastCursorParser.moveToPrevious());
        }

    }

    public void test12_resetCursor() {
        mFastCursorParser.resetCursor(mBookMarkCursor2);
        assertNull(mFastCursorParser.getCursor());
    }

    public void test13_resetCursor() {
        FastCursorParser fastCursorParser = new FastCursorParser(
                mBookMarkCursor2, mConsumer, mBuffer);
        fastCursorParser.resetCursor(mBookMarkCursor);
        assertNotNull(fastCursorParser.getCursor());
    }

    public void test14_parse() {
        mFastCursorParser.parse();
        assertEquals(mFastCursorParser.getCursorPosition(), mFastCursorParser
                .getCount());
    }
    
    public void test15_onBlockReady() {
        FastCursorParserChild cursorParser = new FastCursorParserChild(mBookMarkCursor, mConsumer);
        cursorParser.BlockReadyForEx();
    }
    
    public void test16_FastCursorParser() {
        FastCursorParser parser = new FastCursorParser(mBookMarkCursor,mConsumer,null);
        assertNotNull(parser);
    }
    
    class FastCursorParserChild extends FastCursorParser {

        public FastCursorParserChild(Cursor cursor, IRawBlockConsumer consumer) {
            super(cursor, consumer);
        }
        
        public void BlockReadyForEx() {
            onBlockReadyForEx();
        }
    }
}
