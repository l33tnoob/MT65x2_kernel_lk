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

import java.util.ArrayList;

import com.mediatek.apst.target.data.proxy.bookmark.BookmarkProxy;
import com.mediatek.apst.util.entity.bookmark.BookmarkData;
import com.mediatek.apst.util.entity.bookmark.BookmarkFolder;

import android.content.Context;
import android.test.AndroidTestCase;

public class BookmarkProxyTest extends AndroidTestCase {
    private BookmarkProxy mBookmarkProxy;
    private Context mContext;
    private ArrayList<BookmarkData> mBookmarkDataList;
    private ArrayList<BookmarkFolder> mBookmarkFolderList;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mBookmarkDataList = new ArrayList<BookmarkData>();
        mBookmarkDataList
                .add(new BookmarkData(100, "google", "google.com", 10,
                        20120602, 20120602, 20120602, 20120602, "googleHK", 1,
                        null, 1));
        mBookmarkDataList.add(new BookmarkData(102, "baidu", "www.baidu.com",
                11, 201254, 20114541, 20120101, 20102121, "baidu.com", 1,
                new byte[] { 1, 2, 3 }, 2));
        mBookmarkDataList.add(new BookmarkData(103, "jinshan",
                "www.jinshan.com", 11, 201254, 20114541, 20120101, 20102121,
                "jinshan.com", 0, null, 3));
        mBookmarkDataList
                .add(new BookmarkData(104, "xiaomi", "mui.com", 10, 20120602,
                        20120602, 20120602, 20120602, "xiaomiHK", 1, new byte[] { 1, 2, 3 }, 1));
        mBookmarkFolderList = new ArrayList<BookmarkFolder>();
        mBookmarkFolderList.add(new BookmarkFolder(0, 0, 0, "folder", 20120602,
                10));
        mBookmarkFolderList.add(new BookmarkFolder(1, 2, 1, "folder2",
                20120802, 20));
        mBookmarkFolderList.add(new BookmarkFolder(2, 0, 2, "folder3",
                20120802, 20));
        mBookmarkFolderList.add(new BookmarkFolder(3, 2, 3, "folder4",
                20120802, 20));

    }

    @Override
    protected void tearDown() throws Exception {
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        mBookmarkDataList.clear();
        mBookmarkFolderList.clear();
        mBookmarkProxy.deleteAll();
        mContext = null;
        super.tearDown();
    }

    /**
     * Test singleton, if sInstance is null.
     */
    public void test01_getInstance01() {
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy);
    }

    /**
     * Test singleton, if sInstance is not null.
     */
    public void test02_getInstance02() {
        BookmarkProxy mBookmarkProxy2 = null;
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy);
        mBookmarkProxy2 = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy2);
        assertSame(mBookmarkProxy, mBookmarkProxy2);
    }

    /**
     * Test Whether delete all the book marks.
     */
    public void test03_deleteAll() {
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy);
        mBookmarkProxy.deleteAll();
        ArrayList<BookmarkData> bookmarkDataList = new ArrayList<BookmarkData>();
        ArrayList<BookmarkFolder> bookmarkFolderList = new ArrayList<BookmarkFolder>();
        mBookmarkProxy
                .asynGetAllBookmarks(bookmarkDataList, bookmarkFolderList);
        assertEquals(0, bookmarkDataList.size());
    }

    /**
     * Test insert bookmark.
     */
    public void test04_insertBookmark() {
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy);
        mBookmarkProxy.deleteAll();
        mBookmarkProxy.insertBookmark(mBookmarkDataList, mBookmarkFolderList);
        ArrayList<BookmarkData> bookmarkDataList = new ArrayList<BookmarkData>();
        ArrayList<BookmarkFolder> bookmarkFolderList = new ArrayList<BookmarkFolder>();
        mBookmarkProxy
                .asynGetAllBookmarks(bookmarkDataList, bookmarkFolderList);
        BookmarkData bookmarkData = bookmarkDataList.get(0);
        assertNotNull(bookmarkData);
    }

    /**
     * Test whether get all the bookmarks.
     */
    public void test05_asynGetAllBookmarks() {
        mBookmarkProxy = BookmarkProxy.getInstance(mContext);
        assertNotNull(mBookmarkProxy);
        mBookmarkProxy.deleteAll();
        mBookmarkProxy.insertBookmark(mBookmarkDataList, mBookmarkFolderList);

        ArrayList<BookmarkData> bookmarkDataList = new ArrayList<BookmarkData>();
        ArrayList<BookmarkFolder> bookmarkFolderList = new ArrayList<BookmarkFolder>();
        mBookmarkProxy
                .asynGetAllBookmarks(bookmarkDataList, bookmarkFolderList);

        BookmarkData bookmarkData = bookmarkDataList.get(0);
        assertNotNull(bookmarkData);
        // the id generated by database automatically.
        assertNotNull(bookmarkData.getId());
        assertEquals("google", bookmarkData.getTitle());
        assertEquals("google.com", bookmarkData.getUrl());
        // 
        assertNotNull(bookmarkData.getDate());
        assertNotNull(bookmarkData.getCreated());
        assertNotNull(bookmarkData.getModified());
        assertNotNull(bookmarkData.getAccess());
        // description ignore.
        // assertEquals("googleHK", bookmarkData.getDescription());
        assertEquals(1, bookmarkData.getBookmark());
        assertEquals(null, bookmarkData.getFavIcon());
        // the id generated by database automatically.
        // assertEquals(1, bookmarkData.getFolderId());
    }
}
