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

package com.mediatek.apst.target.data.proxy.bookmark;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.bookmark.BookmarkData;
import com.mediatek.apst.util.entity.bookmark.BookmarkFolder;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * Class Name: BookmarkProxy
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.bookmark
 * <p>
 * Created on: 2011-5-18
 * <p>
 * <p>
 * Description:
 * <p>
 * Facade of the sub system of bookmark content related operations.
 * 
 * @author mtk54043 Yu.Chen
 * @version V1.0
 */

public class BookmarkProxy extends ContextBasedProxy {
    /** Singleton instance. */
    private static BookmarkProxy sInstance = null;

    private static final Uri BOOKMARKS_URI = Uri
            .parse("content://com.android.browser/bookmarks");

    /**
     * @param context
     *            The Context.
     */
    public BookmarkProxy(final Context context) {
        super(context);
        setProxyName("BookmarkProxy");
    }

    /**
     * @param context
     *            The current Context.
     * @return A instance of the BookmarkProxy.
     */
    public static synchronized BookmarkProxy getInstance(final Context context) {
        if (null == sInstance) {
            sInstance = new BookmarkProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    /**
     * delete all the book marks.
     */
    public void deleteAll() {
        getContentResolver().delete(BOOKMARKS_URI, "_id<>1", null);
    }

    /**
     * Get all the bookmarks.
     * @param mBookmarkDataList
     *            The list of the BookmarkData.
     * @param mBookmarkFolderList
     *            The list of the BookmarkFolder.
     */
    public void asynGetAllBookmarks(final ArrayList<BookmarkData> mBookmarkDataList,
            final ArrayList<BookmarkFolder> mBookmarkFolderList) {
        final HashMap<Integer, BookmarkFolder> map = new HashMap<Integer, BookmarkFolder>();
        Cursor c = null;
        c = getContentResolver().query(BOOKMARKS_URI, null, "deleted=0", null,
                null);
        final int iId = c.getColumnIndex("_id");
        final int iTitle = c.getColumnIndex("title");
        final int iUrl = c.getColumnIndex("url");
        final int iFolder = c.getColumnIndex("folder");
        final int iParent = c.getColumnIndex("parent");
        // int iVersion = c.getColumnIndex("version");
        final int iCreate = c.getColumnIndex("created");
        final int iModified = c.getColumnIndex("modified");
        // int iDirty = c.getColumnIndex("dirty");
        try {
            while (c.moveToNext()) {
                final int folder = c.getInt(iFolder);
                if (0 == folder) {
                    BookmarkData tempData = new BookmarkData();
                    tempData.setId(c.getInt(iId));
                    tempData.setTitle(c.getString(iTitle));
                    tempData.setUrl(c.getString(iUrl));
                    final int i = c.getInt(iParent);
                    if (i == 1) {
                        tempData.setFolderId(0);
                    } else {
                        tempData.setFolderId(i);
                    }
                    tempData.setBookmark(1);
                    tempData.setFavicon(null);
                    tempData.setCreated(c.getLong(iCreate));
                    tempData.setModified(c.getLong(iModified));
                    mBookmarkDataList.add(tempData);
                    tempData = null;
                } else {
                    BookmarkFolder tempFolder = new BookmarkFolder();
                    tempFolder.setId(c.getInt(iId));
                    tempFolder.setName(c.getString(iTitle));
                    final int i = c.getInt(iParent);
                    if (i < 1) {
                        continue;
                    } else if (i == 1) {
                        tempFolder.setParentId(0);
                    } else {
                        tempFolder.setParentId(i);
                    }
                    tempFolder.setDate(c.getLong(iCreate));
                    mBookmarkFolderList.add(tempFolder);
                    map.put(tempFolder.getId(), tempFolder);
                    tempFolder = null;
                }
            }
            Debugger.logD("Folder size: " + mBookmarkFolderList.size()
                    + " Data size: " + mBookmarkDataList.size());
            final ArrayList<BookmarkFolder> removeFolders = new ArrayList<BookmarkFolder>();
            int id = 0;
            BookmarkFolder folder;
            for (final BookmarkFolder temp : mBookmarkFolderList) {
                id = temp.getParentId();
                while (0 != id) {
                    folder = map.get(id);
                    if (null == folder) {
                        removeFolders.add(temp);
                        map.remove(temp.getId());
                        break;
                    }
                    id = folder.getParentId();
                }
            }
            mBookmarkFolderList.removeAll(removeFolders);
            Debugger.logD("removeFolders size: " + removeFolders.size()
                    + " Folder size: " + mBookmarkFolderList.size());
            for (int i = 0; i < mBookmarkFolderList.size(); i++) {
                int level = 1;
                final BookmarkFolder tempFolder = mBookmarkFolderList.get(i);
                BookmarkFolder temp = mBookmarkFolderList.get(i);
                while (temp.getParentId() != 0) {
                    level++;
                    temp = map.get(temp.getParentId());
                    if (null == temp) {
                        break;
                    }
                }
                tempFolder.setFolderLevel(level);
            }
            final ArrayList<BookmarkData> removeDatas = new ArrayList<BookmarkData>();
            for (final BookmarkData temp : mBookmarkDataList) {
                if (0 == temp.getFolderId()) {
                    continue;
                }
                if (null == map.get(temp.getFolderId())) {
                    removeDatas.add(temp);
                }
            }
            mBookmarkDataList.removeAll(removeDatas);
            Debugger.logD("removeDatas size: " + removeDatas.size()
                    + " Data size: " + mBookmarkDataList.size());
        } finally {
            if (null != c) {
                c.close();
            }
        }
    }

    /**
     * @param mBookmarkDataList
     *            The list of the BookmarkData.
     * @param mBookmarkFolderList
     *            The list of the BookmarkFolder.
     */
    public void insertBookmark(final ArrayList<BookmarkData> mBookmarkDataList,
            final ArrayList<BookmarkFolder> mBookmarkFolderList) {
        Debugger.logI(new Object[] {}, "InsertBookmark: BookmarkData size: "
                + mBookmarkDataList.size() + " BookmarkFolder size: "
                + mBookmarkFolderList.size());
        final HashMap<Integer, BookmarkFolder> map = new HashMap<Integer, BookmarkFolder>();
        int maxLevel = 0;
        for (final BookmarkFolder folder : mBookmarkFolderList) {
            map.put(folder.getId(), folder);
            if (maxLevel < folder.getFolderLevel()) {
                maxLevel = folder.getFolderLevel();
            }
        }
        for (int i = 1; i <= maxLevel; i++) {
            for (final BookmarkFolder folder : mBookmarkFolderList) {
                if (i == folder.getFolderLevel()) {
                    ContentValues values = new ContentValues();
                    values.put("title", folder.getName());
                    values.put("folder", 1);
                    int folderId = -1;
                    if (1 == i) {
                        folderId = 1;
                    } else if (null == map.get(folder.getParentId())) {
                        Debugger.logW(new Object[] {},
                                "Insert folder: map return null, key is "
                                        + folder.getParentId());
                        folderId = folder.getParentId();
                    } else {
                        folderId = map.get(folder.getParentId()).getId();
                    }
                    values.put("parent", folderId);
                    // values.put("deleted", 0);
                    // values.put("created", System.currentTimeMillis());
                    // values.put("dirty", 0);
                    final Uri uri = getContentResolver()
                            .insert(BOOKMARKS_URI, values);
                    int iNewId;
                    try {
                        iNewId = Integer.parseInt(uri.getLastPathSegment());
                    } catch (final NumberFormatException e) {
                        e.printStackTrace();
                        iNewId = 1;
                    }
                    folder.setId(iNewId);
                    values = null;
                }
            }
        }
        for (final BookmarkData data : mBookmarkDataList) {
            ContentValues values = new ContentValues();
            values.put("title", data.getTitle());
            values.put("url", data.getUrl());
            values.put("folder", 0);
            int folderId = -1;
            if (0 == data.getFolderId()) {
                folderId = 1;
            } else if (null == map.get(data.getFolderId())) {
                Debugger.logW(new Object[] {},
                        "Insert data: map return null, key is "
                                + data.getFolderId());
                folderId = data.getFolderId();
            } else {
                folderId = map.get(data.getFolderId()).getId();
            }
            values.put("parent", folderId);
            // values.put("deleted", 0);
            // values.put("created", System.currentTimeMillis());
            // values.put("dirty", 0);
            getContentResolver().insert(BOOKMARKS_URI, values);
            values = null;
        }
    }
}
