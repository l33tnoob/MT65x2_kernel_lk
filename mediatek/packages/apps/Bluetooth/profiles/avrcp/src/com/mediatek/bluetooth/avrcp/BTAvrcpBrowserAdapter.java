/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth.avrcp;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.Toast;

/**
 * @brief Avrcp-Browser Adpater for the native Android SD Card media
 */
public class BTAvrcpBrowserAdapter extends Thread {
    public static final String TAG = "BWS_AVRCP";

    private static final int DELAY_TIME = 0; // 6 seconds

    private BluetoothAvrcpService mAvrcpServ;

    private BTAvrcpBrowserSearch mSearchAdapter;

    private Handler mHandler;

    private static final int ACTION_NONE = 0x10;

    private static final int ACTION_CHANGE_PATH_UP = 0x11;

    private static final int ACTION_CHANGE_PATH_DOWN = 0x12;

    private static final int ACTION_GET_ATTR = 0x13;

    private static final int ACTION_SEARCH = 0x14;

    private static final int ACTION_GET_FILELIST = 0x15;

    private static final long ROOT_FOLDER_ID = 0x100;

    private static final long ARTIST_FOLDER_ID = 0x101;

    private static final long ALBUM_FOLDER_ID = 0x102;

    private static final long TEST_FOLDER_ID = 0x103;

    private static final long NO_EXIST_FOLDER = 0x0F0F;

    private boolean mInsideFakeFolder = true;

    private long mParentID = ROOT_FOLDER_ID; /* only record local ID */

    private long mCurFakeFolderID = ROOT_FOLDER_ID; /* */

    private long mCurFolderID = ROOT_FOLDER_ID; /* */

    private short mUidCounter = 0;

    private static final short UNAWARE_UIDCOUNTER = 0;

    private int mCurPathItemNum = 2; /* default root folder has two folder */

    private byte mDepth = 1;

    private String mSearchText;

    private String[] mCurPaths;

    private long[] mSearchResultList; /* save uid of serached song */

    private static final byte OK = BTAvrcpProfile.OK;

    private static final byte FAIL = BTAvrcpProfile.FAIL;

    private static final short UTF8_CHARSET = BTAvrcpProfile.UTF8_CHARSET; /* 0x06a */

    //AVRCP status definition
    public static final byte STATUS_OK = BTAvrcpProfile.STATUS_OK;

    //AVRCP status definition
    public static final byte STATUS_FAIL = BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM;

    private boolean mIsSearch = false;

    /* Used for get Item Attribute in now playing scope */
    private long mNowId = 0;

    private String mNowTtitle = null;

    private String mNowAlbum = null;

    private String mNowArtist = null;

    /* Used for get Item Attribute in Search scope */
    private long mSearchId = 0;

    private String mSearchTtitle = null;

    private String mSearchAlbum = null;

    private String mSearchArtist = null;

    BTAvrcpBrowseAlbum mAlbumCategory;

    BTAvrcpBrowseAlbum mArtistCategory;

    BTAvrcpBrowseAlbum mEmptyCategory;

    BTAvrcpBrowseAlbum mAllCategory;

    BTAvrcpBrowseAlbum[] mCategory;

    BTAvrcpBrowseAlbum mCurCategory = null;

    /* */
    private int mGetFileAttrCount = 0;

    private int[] mGetFileAttr = null;

    private volatile Looper mServiceLooper = null;

    private boolean mDebug = false;

    BTAvrcpBrowserAdapter(BluetoothAvrcpService service) {
        mAvrcpServ = service;
        mIsSearch = false;
        mSearchAdapter = new BTAvrcpBrowserSearch(service);
        setName("BTAvrcpBrowserAdapterThread");

        mAlbumCategory = new BTAvrcpBrowseAlbum(0x1001, mAvrcpServ, BTAvrcpBrowseAlbum.ALBUM_TYPE);
        mArtistCategory = new BTAvrcpBrowseAlbum(0x1002, mAvrcpServ, BTAvrcpBrowseAlbum.ARTIST_TYPE);
        mEmptyCategory = new BTAvrcpBrowseAlbum(0x1003, mAvrcpServ, BTAvrcpBrowseAlbum.EMPTY_TYPE);
        mAllCategory = new BTAvrcpBrowseAlbum(0x1004, mAvrcpServ, BTAvrcpBrowseAlbum.ALL_TYPE);
        if (checkPTSMode()) {
            mCategory = new BTAvrcpBrowseAlbum[4];
            mCategory[0] = mAllCategory; // for PTS Test
            mCategory[1] = mArtistCategory;
            mCategory[2] = mAlbumCategory;
            mCategory[3] = mEmptyCategory; // for PTS Test
        } else {
            mCategory = new BTAvrcpBrowseAlbum[2];
            mCategory[0] = mArtistCategory;
            mCategory[1] = mAlbumCategory;
        }
        mCurCategory = null;

        if (BluetoothAvrcpService.isSupportBrowse()) {
            this.start(); // start to run a thread
            Log.v(TAG, "[BT][AVRCP] AvrcpBrowse start looper");
        } else {
            Log.v(TAG, "[BT][AVRCP] No AvrcpBrowse debug looper");
        }
    }

    public void deinit() {
        if (null != mServiceLooper) {
            mServiceLooper.quit();
            mServiceLooper = null;
        }

        if (null != mHandler) {
            Log.v(TAG, "[BT][AVRCP] mHandler is existed. call join");
            this.interrupt();

            try {
                this.join(100);
            } catch (InterruptedException ex) {
                Log.v(TAG, "[BT][AVRCP] join fail");
            }
        }
    }

    public void onConnect() {
        Log.v(TAG, "[BT][AVRCP] onConnect");
        if (mCurCategory != null) {
            mCurCategory.resetSubFolder();
            mCurCategory = null;
        }
        if (!BluetoothAvrcpService.sSupportBrowse) {
            Log.v(TAG, "[BT][AVRCP] No Support Avrcp Browse feature");
        }

        // init some local variable
        mIsSearch = false;
    }

    public void onDisconnect() {
        Log.v(TAG, "[BT][AVRCP] onDisconnect");
    }

    public void onSelect() {
        Log.v(TAG, "[BT][AVRCP] OnSelect ");
    }

    public void onUnselect() {
        Log.v(TAG, "[BT][AVRCP] OnUnselect ");
    }

    private boolean sendMyselfMsg(int what, int arg1, int arg2) {
        if (null == mHandler) {
            return false;
        }
        Message msg = mHandler.obtainMessage();
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        return mHandler.sendMessageDelayed(msg, DELAY_TIME);
    }

    private boolean sendMyselfMsg(int what, String data) {
        if (null != mHandler) {
            Message msg = mHandler.obtainMessage();
            msg.what = what;
            msg.obj = data;
            return mHandler.sendMessageDelayed(msg, DELAY_TIME);
        } else {
            return false;
        }
    }

    public boolean changePath(short uidCounter, byte dir, long uid) {
        boolean bRet = false;
        if (uidCounter != UNAWARE_UIDCOUNTER) {
            Log.i(TAG, "[BT][AVRCP] changePath fail to  uid_counter:" + uidCounter);
            return false;
        }
        if (uidCounter == UNAWARE_UIDCOUNTER && uid == 0 && dir != 0) {
            Log.i(TAG, "[BT][AVRCP] changePath fail  uid_counter:" + uidCounter + " dir:" + dir
                    + " uid:" + uid);
            return false;
        }

        if (dir == 1) {
            sendMyselfMsg(ACTION_CHANGE_PATH_DOWN, Long.toString(uid));
            return true;
        } else if (dir == 0) {
            sendMyselfMsg(ACTION_CHANGE_PATH_UP, Long.toString(uid));
            return true;
        }
        return false;
    }

    public boolean getFileSystemitemsList(int start, int end, byte count, int attrIds[]) {
        //
        mGetFileAttrCount = count;
        mGetFileAttr = attrIds;

        sendMyselfMsg(ACTION_GET_FILELIST, start, end);
        return true;
    }

    public int getCurPathItems() {
        if (null == mCurCategory) {
            mCurPathItemNum = mCategory.length;
        } else {
            mCurPathItemNum = mCurCategory.getCurPathItems();
        }
        Log.v(TAG, "[BT][AVRCP] getCurPathItems mCurPathItemNum:" + mCurPathItemNum);
        return mCurPathItemNum;
    }

    public short getUidCounter() {
        return mUidCounter;
    }

    public byte getCurPathDepth() {
        if (null == mCurCategory) {
            mDepth = (byte) 1;
        } else {
            mDepth = (byte) (1 + mCurCategory.getCurPathDepth());
        }
        Log.v(TAG, "[BT][AVRCP] getCurPathDepth mDepth:" + mDepth);
        return mDepth;
    }

    public String[] getCurPaths() {
        if (null != mCurCategory) {
            return mCurCategory.getCurPaths();
        }
        if (mCurPaths == null) {
            mCurPaths = new String[1];
            mCurPaths[0] = "root";
        }
        return mCurPaths;
    }

    private boolean checkPTSMode() {
        return true;
    }

    public boolean search(String searchText) {
        Log.i(TAG, "[BT][AVRCP] search");

        if (searchText == null || "".equals(searchText)) {
            // empty search - found none.
            return false;
        }
        if (mIsSearch) {
            // mAvrcpServ.searchRspNative(
            // BTAvrcpProfile.AVRCP_ERRCODE_SEARCH_IN_PROGRESS,
            // BTAvrcpProfile.AVRCP_ERRCODE_SEARCH_IN_PROGRESS,
            // UNAWARE_UIDCOUNTER, 0);
            Log.i(TAG, "[BT][AVRCP] search is in progress");
            return false;
        }
        if (BluetoothAvrcpService.sSupportBrowse) {
            // pending
            if (this.isAlive()) {
                Log.v(TAG, "[BT][AVRCP] check isAlive");
            } else {
                // delay some times
                yield();
                try {
                    sleep(10000); // sleep one sec
                } catch (InterruptedException ex) {
                    Log.v(TAG, "[BT][AVRCP] sleep fail");
                }

                Log.i(TAG, "[BT][AVRCP] delay search 10000");
            }

            if (this.isAlive()) {
                mIsSearch = true;
                mSearchText = searchText;
                sendMyselfMsg(ACTION_SEARCH, 0, 0);
                return true;
            } else {
                Log.i(TAG, "[BT][AVRCP] Thread not start yets!");
                return false;
            }
        } else {
            Log.i(TAG, "[BT][AVRCP] search fail because bSupportBrowse is false");
        }
        return false;
    }

    public boolean isSearch() {
        return mIsSearch;
    }

    public boolean isItemExist(byte scope, long identifier, short uidCounter) {
        if (uidCounter != UNAWARE_UIDCOUNTER) {
            return false;
        }

        return true;
    }

    String getItemAttribute(byte scope, long identifier, short uidCounter, int attrId) {
        String sRet = null;

        if (uidCounter != UNAWARE_UIDCOUNTER) {
            Log.e(TAG, "[BT][AVRCP] getItemAttribute wrong uid_counter" + uidCounter);
            return null;
        }

        switch (scope) {
            case BTAvrcpProfile.AVRCP_SCOPE_PLAYER_LIST: // 0x00
                return "Player";
            case BTAvrcpProfile.AVRCP_SCOPE_FILE_SYSTEM: // 0x01
                if (null != mCurCategory) {
                    return mCurCategory.getItemAttribute(scope, identifier, uidCounter, attrId);
                }
                return null; // it is folder
                // break;
            case BTAvrcpProfile.AVRCP_SCOPE_SEARCH: // 0x02
                // item id - get Curor by id (TC_TG_MCN_SRC_BV_06_C & TC_TG_MCN_SRC_BV_04_I)
                if (identifier != mSearchId) {
                    if (true != this.updateSearchSongId(identifier)) {
                    }
                }

                /* update success */
                if (identifier == mSearchId) {
                    switch (attrId) {
                        case 0x01:
                            sRet = mSearchTtitle;
                            break;
                        case 0x02: // artist
                            sRet = mSearchArtist;
                            break;
                        case 0x03: // album
                            sRet = mSearchAlbum;
                            break;
                        default:
                            break;
                    }
                    Log.i(TAG, "[BT][AVRCP] getItemAttribute ret:" + sRet);
                } else {
                    Log.e(TAG, "[BT][AVRCP] fail to updateSearchSongId ! id:" + identifier);
                }
                break;

            default:
            case BTAvrcpProfile.AVRCP_SCOPE_NOW_PLAYING: // 0x03
                // item id - get Curor by id
                if (identifier != mNowId) {
                    this.updateNowSongId(identifier);
                }

                /* update success */
                if (identifier == mNowId) {
                    switch (attrId) {
                        case 0x01:
                            sRet = mNowTtitle;
                            break;
                        case 0x02: // artist
                            sRet = mNowArtist;
                            break;
                        case 0x03: // album
                            sRet = mNowAlbum;
                            break;
                        default:
                            break;
                    }
                } else {
                    Log.e(TAG, "[BT][AVRCP] fail to updateNowSongId ! id:" + identifier);
                }
                break;
        }

        // return String.format("[BT][AVRCP] scope:%d id:%d attr:%d", scope,
        // identifier, attr_id);
        return sRet;
    }

    String getSearchedItemAttribute(int index, int attrId) {
        switch (attrId) {
            case 0x01:
                return mSearchAdapter.getSearchedTitleString(index);
            case 0x02:
                return mSearchAdapter.getSearchedArtistString(index);
            case 0x03:
                return mSearchAdapter.getSearchedAlbumString(index);
            default:
                return null;
        }
//        return null;
    }

    public boolean checkSongIdExisted(long newId) {
        boolean bRet = false;
        Cursor cursor = getCursorById(mAvrcpServ, newId);

        if (newId == 0) { // now playing item
            bRet = true;
        } else if (cursor == null) {
            bRet = false;
        } else if (cursor.getCount() > 0) {
            bRet = true;
        }
        if (null != cursor) {
            cursor.close();
            cursor = null;
        }
        return bRet;
    }

    private boolean updateSearchSongId(long newId) {
        Cursor cursor = getCursorById(mAvrcpServ, newId);
        Log.v(TAG, "[BT][AVRCP][BWS] updateSearchSongId " + newId);

        if (cursor == null) {
            Log.w(TAG, "[BT][AVRCP] updateSearchSongId got null");
            return false;
        }

        try {
            if (cursor != null) {
                if (cursor.getCount() > 0) {
                    cursor.moveToNext();

                    mSearchTtitle = null;
                    mSearchArtist = null;
                    mSearchAlbum = null;

                    mSearchTtitle = cursor.getString(0);
                    mSearchArtist = cursor.getString(1);
                    mSearchAlbum = cursor.getString(2);
                    mSearchId = newId;
                    Log.v(TAG, "[BT][AVRCP] updateSearchSongId '" + mSearchTtitle + "' '"
                            + mSearchArtist + "' '" + mSearchAlbum + "'");
                    return true;
                } else {
                    Log.e(TAG, "[BT][AVRCP] query and get empty result !");
                }
            }
            return false;
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
    }

    private boolean updateNowSongId(long newId) {
        Cursor cursor = getCursorById(mAvrcpServ, newId);
        Log.v(TAG, "[BT][AVRCP][BWS] updateNowSongId " + newId);

        if (cursor == null) {
            Log.w(TAG, "[BT][AVRCP] updateNowSongId got null");
            return false;
        }

        try {
            if (cursor != null) {
                if (cursor.getCount() > 0) {
                    cursor.moveToNext();

                    mNowTtitle = null;
                    mNowArtist = null;
                    mNowAlbum = null;

                    mNowTtitle = cursor.getString(0); // index basis 0
                    mNowArtist = cursor.getString(1);
                    mNowAlbum = cursor.getString(2);
                    mNowId = newId;

                    Log.v(TAG, "[BT][AVRCP] updateNowSongId '" + mNowTtitle + "' '" + mNowArtist
                            + "' '" + mNowAlbum + "'");
                    return true;
                } else {
                    Log.e(TAG, "[BT][AVRCP] query and get empty result !");
                }
            }
            return false;
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }

    }

    public long[] getSearchedList() {
        mSearchResultList = mSearchAdapter.getSearchedList();
        return mSearchResultList;
    }

    /**
     * @brief MusicAdapter as a Looper
     */
    public void run() {
        Log.v(TAG, "[BT][AVRCP] browse run");
        Looper.prepare();
        Log.v(TAG, "[BT][AVRCP] browse run prepare ");
        mServiceLooper = Looper.myLooper();
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                // process incoming messages here
                passToHandleMessage(msg);
            }
        };

        Looper.loop();
        mHandler = null;
    }

    public boolean registerNotification(byte eventId, int interval) {
        /*
         * // Disable this support. This is a database no-aware music browser
         * if( eventId == BTAvrcpProfile.EVENT_UIDS_CHANGED){
         * mAvrcpServ.notificationUIDSChangedNative(OK, (byte)1, (short)0); //
         * player_id return true; }
         */
        return false;
    }

    public void passToHandleMessage(Message msg) {
        String sMsg;
        sMsg = String.format(
                "[BT][AVRCP] AVRCPBrowser Receive a msg.what:%d msg.arg1:%d msg.arg2:%d ",
                msg.what, msg.arg1, msg.arg2);
        Log.i(TAG, String.format("[BT][AVRCP] AVRCPBWS passToHandleMessage what:%d", msg.what));
        if (BluetoothAvrcpService.sDebugMsg) {
            Toast.makeText(mAvrcpServ, sMsg, Toast.LENGTH_LONG).show();
        }
        long uid;
        switch (msg.what) {
            case ACTION_NONE:
                break;
            case ACTION_CHANGE_PATH_UP:
//                long uid;
                uid = Long.valueOf((String) msg.obj);
                Log.i(TAG, "[BT][AVRCP] compose up id:" + uid);
                handleChangePath(UNAWARE_UIDCOUNTER, (byte) 0, uid);
                break;

            case ACTION_CHANGE_PATH_DOWN:
//                long uid;
                uid = Long.valueOf((String) msg.obj);
                Log.i(TAG, "[BT][AVRCP] compose down id:" + uid);
                handleChangePath(UNAWARE_UIDCOUNTER, (byte) 1, uid);
                break;
            // case ACTION_GET_ATTR:
            // break;
            case ACTION_SEARCH:
                handleSearch(mSearchText);
                break;

            case ACTION_GET_FILELIST:
                handleGetFileList(msg.arg1, msg.arg2, mGetFileAttrCount, mGetFileAttr);
                break;

            default:
                Toast.makeText(mAvrcpServ, "[BT][AVRCP][BWS]no Handle msg !", Toast.LENGTH_LONG)
                        .show();
                break;
        }
    }

    private void handleGetFileList(int start, int end, int attrCount, int[] attrIds) {
        int total;
        long[] idlist;
        int length = 0;
        int i = 0;
        int j = 0;
        byte attrIndex;
        short u2AttrLength;
        short u2ItemLength;
        String sAttrValue;
        String sItemValue;

        // use service to response the file's list
        Log.v(TAG, "[BT][AVRCP][BWS] handleGetFileList start:" + start + " end:" + end);
        mAvrcpServ.getFileSystemItemStartRspNative();
        if (null != mCurCategory) {
            // return the adpater's current folder list
            idlist = mCurCategory.getCurrentList();
            if (idlist != null) {
                length = idlist.length;
            }
            if (null == idlist || start >= length) {
                // out of bound. PTS TC_TG_MCN_CB_BI_03_C test start with the
                // length.
                Log.v(TAG, String.format(
                        "[BT][AVRCP] handleGetFileList  out-of-bound start:%d list.length:%d",
                        start, length));
                mAvrcpServ.getFileSystemItemEndRspNative((byte) FAIL,
                        BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, mUidCounter);
                return;
            }
            if (idlist.length == 0) {
                mAvrcpServ.getFileSystemItemEndRspNative((byte) FAIL,
                        BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, mUidCounter);
                return;
            }

            Log.v(TAG, String.format(
                    "[BT][AVRCP] handleGetFileList start:%d end:%d list.length:%d", start, end,
                    length));
            if (end >= length) {
                end = length - 1;
            }

            //
            switch (mCurCategory.getType()) {
                case BTAvrcpProfile.ITEM_TYPE_FOLDER:
                    // return folder items
                    total = end - start + 1;
                    for (i = start; i <= end; i++) {
                        // configure the attribute
                        // getFileSystemItemFileAttrRspNative(byte error, byte
                        // item, byte attrIndex, int attrId, short charset,
                        // short len, String AttrValue);
                        attrIndex = 0;
                        sItemValue = mCurCategory.getNameByIndex(i);
                        if (null != sItemValue) {
                            u2ItemLength = (short) sItemValue.length();
                        } else {
                            u2ItemLength = 0;
                        }
                        // getFileSystemItemFolderRspNative(byte error, byte
                        // item, byte total, long uid_counter, byte foldertype,
                        // byte playable, short charset, short len, String
                        // foldername );
                        mAvrcpServ.getFileSystemItemFolderRspNative(OK, (byte) (i - start),
                                (byte) total, idlist[i], mCurCategory.getFolderType(), (byte) 0,
                                UTF8_CHARSET, u2ItemLength, sItemValue);
                    }

                    mAvrcpServ.getFileSystemItemEndRspNative(OK, OK, mUidCounter);
                    break;
                case BTAvrcpProfile.ITEM_TYPE_ELEMENT:
                    // return element items
                    total = end - start + 1;
                    for (i = start; i <= end; i++) {
                        // configure the attribute
                        attrIndex = 0;
                        for (j = 0; j < attrCount; j++) {
                            sAttrValue = mCurCategory.getAttributeByIndex(i, attrIds[j]);

                            if (sAttrValue != null) {
                                u2AttrLength = (short) sAttrValue.length();
                                // getFileSystemItemFileAttrRspNative(byte
                                // error, byte item, byte attrIndex, int attrId,
                                // short charset, short len, String AttrValue);
                                mAvrcpServ.getFileSystemItemFileAttrRspNative(OK,
                                        (byte) (i - start), attrIndex, attrIds[j], UTF8_CHARSET,
                                        u2AttrLength, sAttrValue);
                                attrIndex++;
                            }
                        }

                        // configure file's information
                        sItemValue = mCurCategory.getNameByIndex(i);
                        if (null != sItemValue) {
                            u2ItemLength = (short) sItemValue.length();
                        } else {
                            u2ItemLength = 0;
                        }

                        // getFileSystemItemFileRspNative(byte error, byte item,
                        // byte total, long uid, byte mediatype, short charset,
                        // short len, String filename);
                        mAvrcpServ.getFileSystemItemFileRspNative(OK, (byte) (i - start),
                                (byte) total, idlist[i], (byte) 0x00, UTF8_CHARSET, u2ItemLength,
                                sItemValue);

                    }
                    mAvrcpServ.getFileSystemItemEndRspNative(OK, OK, mUidCounter);
                    break;
                default:
                    Log.e(TAG, "[BT][AVRCP]  mCurCategory.getType Wrong " + mCurCategory.getType());
                    mAvrcpServ.getFileSystemItemEndRspNative((byte) 0x01, STATUS_FAIL, mUidCounter);
                    break;
            }

        } else {
            // reutrn the category folder
            if (start > mCategory.length) {
                // out-of-bound
                Log.e(TAG, String.format(
                        "[BT][AVRCP] handleGetFileList out-of-bound start:%d mCategory.length:%d",
                        start, mCategory.length));
                mAvrcpServ.getFileSystemItemEndRspNative((byte) FAIL,
                        BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, mUidCounter);
                return;
            }
            if (end >= mCategory.length) {
                end = mCategory.length - 1; // end will be included !!!
            }
            total = end - start + 1;
            for (i = start; i <= end; i++) {
                if (i >= mCategory.length) {
                    Log.e(TAG, String.format("[BT][AVRCP][ERR] Out-of-Array mCategory.length:%d",
                            i, mCategory.length));
                    mAvrcpServ.getFileSystemItemEndRspNative((byte) FAIL,
                            BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, mUidCounter);
                    return;
                } else {
                    sItemValue = mCategory[i].getCategoryName();
                    u2ItemLength = (short) sItemValue.length();
                    // getFileSystemItemFolderRspNative(byte error, byte item,
                    // byte total, long uid_counter, byte foldertype, byte
                    // playable, short charset, short len, String foldername );
                    mAvrcpServ.getFileSystemItemFolderRspNative(OK, (byte) (i - start),
                            (byte) total, mCategory[i].getPresentId(),
                            BTAvrcpProfile.ITEM_TYPE_FOLDER, (byte) 0, UTF8_CHARSET, u2ItemLength,
                            sItemValue);
                }
            }

            mAvrcpServ.getFileSystemItemEndRspNative(OK, OK, mUidCounter);
        }
    }

    private void handleSearch(String sText) {
        int numOfItems = 3;
        byte status = STATUS_OK;

        Log.i(TAG, "[BT][AVRCP] handleSearch " + sText);
        // AVRCP_ERRCODE_SEARCH_NOT_SUPPORTED
        // AVRCP_ERRCODE_SEARCH_IN_PROGRESS
        // AVRCP_ERRCODE_PLAYER_NOT_ADDRESSED

        if (!mIsSearch) {
            Log.i(TAG, "[BT][AVRCP] isSearch false. No return result ");
            return;
        }

        if (0 == sText.length()) {
            Log.e(TAG, "[BT][AVRCP][WRN] handleSearch request empty string !");
            numOfItems = 0;
            mAvrcpServ.searchRspNative(BTAvrcpProfile.OK, status, UNAWARE_UIDCOUNTER, numOfItems);
            mIsSearch = false;
            return;
        }

        if (null != mSearchAdapter) {
            numOfItems = mSearchAdapter.search(sText);
        }

        mIsSearch = false;
        // num_of_items = 3; /* assume 3 items found */
        mAvrcpServ.searchRspNative(BTAvrcpProfile.OK, status, UNAWARE_UIDCOUNTER, numOfItems);
    }

    private void handleChangePath(short uidCounter, byte dir, long uid) {
        boolean bRet = false;
        byte status = 0;
        int i = 0;

        // success or fail - both invoke server's
        Log.i(TAG, "[BT][AVRCP] handleChangePath uid_counter:" + uidCounter + " dir:" + dir
                + " uid:" + uid);

        if (dir == BTAvrcpProfile.DIR_UP) {
            if (null != mCurCategory) {
                if (mCurCategory.isCategoryRoot()) {
                    mCurCategory = null; // up to category level
                    mCurPathItemNum = mCategory.length;
                    Log.v(TAG, "[BT][AVRCP][BWS] handleChangePath goUp to catagory");
                    bRet = true;
                } else {
                    mCurCategory.goUp();
                    if (null != mCurCategory) { // for klockwise warnning
                        mCurPathItemNum = mCurCategory.getCurPathItems();
                    } else {
                        mCurPathItemNum = 0;
                    }
                    Log.v(TAG, "[BT][AVRCP][BWS] handleChangePath goUp ok num-of-items:"
                            + mCurPathItemNum);
                    bRet = true;
                }
            } else {
                // mCurCategory is null. cannot up
                // donothing
                bRet = false;
                status = BTAvrcpProfile.AVRCP_ERRCODE_INVALID_DIRECTION;
            }
        } else if (dir == BTAvrcpProfile.DIR_DOWN) {
            if (null != mCurCategory) {
                if (mCurCategory.goDown(uid)) {
                    // success to change path
                    mCurPathItemNum = mCurCategory.getCurPathItems();
                    Log.v(TAG, "[BT][AVRCP][BWS] handleChangePath goDown ok:" + uid
                            + " num-of-items:" + mCurPathItemNum);
                    bRet = true;
                } else {
                    // fail to chang path
                    status = BTAvrcpProfile.AVRCP_ERRCODE_NOT_A_DIRECTORY;
                    Log.w(TAG, "[BT][AVRCP][BWS] handleChangePath mCurCategory.goDown fail uid"
                            + uid);
                    bRet = false;
                }
            } else {
                // down deeper - select a category by its id (mCurCategory is
                // NULL)
                for (i = 0; i < mCategory.length; i++) {
                    if (uid == mCategory[i].getPresentId()) {
                        mCurCategory = mCategory[i];
                        mCurCategory.resetSubFolder();
                        mCurCategory.buildSubFolder();

                        if (null != mCurCategory) {
                            mCurPathItemNum = mCurCategory.getCurPathItems();
                        } else {
                            mCurPathItemNum = 0; // treat it as 0 items
                        }
                        bRet = true;
                        Log.v(TAG, "[BT][AVRCP][BWS] handleChangePath down to category ok:" + uid
                                + " num:" + mCurPathItemNum);
                        break;
                    }
                }

                if (!bRet) {
                    Log.e(TAG, "[BT][AVRCP][BWS] handleChangePath down to " + uid
                            + " fail! cannot found ");
                    bRet = false;
                    status = BTAvrcpProfile.AVRCP_ERRCODE_NOT_EXIST;
                }
            }
        } else {
            bRet = false;
            status = BTAvrcpProfile.AVRCP_ERRCODE_NOT_EXIST;
        }

        Log.i(TAG, "[BT][AVRCP] changePath final bRet:" + Boolean.toString(bRet));
        if (bRet) {
            mAvrcpServ.changePathRspNative((byte) OK, status, mCurPathItemNum);
        } else {
            // Only allow to send fail response with
            // AVRCP_ERRCODE_INVALID_DIRECTION
            // AVRCP_ERRCODE_NOT_A_DIRECTORY
            // AVRCP_ERRCODE_NOT_EXIST
            switch (status) {
                case BTAvrcpProfile.AVRCP_ERRCODE_INVALID_DIRECTION:
                case BTAvrcpProfile.AVRCP_ERRCODE_NOT_A_DIRECTORY:
                case BTAvrcpProfile.AVRCP_ERRCODE_NOT_EXIST:
                    mAvrcpServ.changePathRspNative((byte) status, status, mCurPathItemNum);
                    break;
                default:
                    mAvrcpServ.changePathRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_NOT_EXIST,
                            status, mCurPathItemNum);
                    break;
            }
        }
    }

    private static Cursor getCursorById(Context context, long id) {
        final String[] ccols = new String[] {
                MediaStore.Audio.Media.TITLE, MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.ALBUM,
        };

        String where = MediaStore.Audio.Media._ID + "=" + id + " AND "
                + MediaStore.Audio.Media.IS_MUSIC + "=1";
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, ccols, where,
                null, MediaStore.Audio.Media.TRACK);

        return cursor;
    }

    private static Cursor query(Context context, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder, int limit) {
        try {
            ContentResolver resolver = context.getContentResolver();
            if (resolver == null) {
                return null;
            }
            if (limit > 0) {
                uri = uri.buildUpon().appendQueryParameter("limit", "" + limit).build();
            }
            return resolver.query(uri, projection, selection, selectionArgs, sortOrder);
        } catch (UnsupportedOperationException ex) {
            return null;
        }

    }

    private static Cursor query(Context context, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }
}
