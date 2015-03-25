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
import android.provider.MediaStore;
import android.util.Log;

public class BTAvrcpBrowseAlbum {
    public static final String TAG = "AVRCP_ALBUM";

    private Context mContext;

    private long mId = 0x1;

    private long[] mIdList = null; /* album's list */

    private String[] mNameList = null; /* name's list */

    private long mSelectId = 0; /* record the selected sub folder */

    private long[] mIdSubList = null; /* selected album's id's sublist */

    private String[] mSubTitleLis = null;

    private String[] mSubArtistLis = null;

    private String[] mSubAlbumLis = null;

    private String[] mPathArray;

    private String[] mSubPathArray;

    public static final byte MIX_TYPE = 0x00;

    public static final byte TITLE_TYPE = 0x01;

    public static final byte ALBUM_TYPE = 0x02;

    public static final byte ARTIST_TYPE = 0x03;

    public static final byte GENRES_TYPE = 0x04;

    public static final byte PLAYLIST_TYPE = 0x05;

    public static final byte YEAR_TYPE = 0x06;

    public static final byte ALL_TYPE = 0x0E;

    public static final byte EMPTY_TYPE = 0x0F;

    private byte mType = 0x02;

    BTAvrcpBrowseAlbum(long id, Context context, byte type) {
        mId = id;
        mContext = context;
        resetSubFolder();
        mType = type;

        mPathArray = new String[2];
        mSubPathArray = new String[3];
        mPathArray[0] = "root";
        mSubPathArray[0] = "root";

        switch (type) {
            case ARTIST_TYPE:
                mPathArray[1] = "Artist";
                mSubPathArray[1] = "Artist";
                break;
            case ALBUM_TYPE:
                mPathArray[1] = "Album";
                mSubPathArray[1] = "Album";
                break;
            case EMPTY_TYPE:
                mPathArray[1] = "Empty";
                mSubPathArray[1] = "Empty";
                break;
            case MIX_TYPE:
            case ALL_TYPE:
                mPathArray[1] = "All";
                mSubPathArray[1] = "All";
                break;
            default:
                mPathArray[1] = "" + type;
                mSubPathArray[1] = "" + type;
                break;
        }

        if (0 == mId) {
            Log.e(TAG, "[BT][AVRCP] Should not use 0 as Id ");
        }

    }

    public long getPresentId() {
        return mId;
    }

    /*
     * @brief build the first sub folder's list
     */
    public void buildSubFolder() {
        if (mType == ALL_TYPE) {
            mSelectId = 1;
            mIdList = updateSubSongs(mContext, 0);
        } else {
            mIdList = updateSongs(mContext);
        }
    }

    private void destroySubFolder() {
        mIdList = null;
    }

    public String[] getCurPaths() {
        if (0 == mSelectId) {
            return mPathArray;
        }
        mSubPathArray[2] = "" + mSelectId;
        return mSubPathArray;
    }

    public long[] getCurrentList() {
        if (0 == mSelectId) {
            // return album list
            return mIdList;
        }

        // return selected album's id list
        mSubPathArray[2] = "" + mSelectId;
        return mIdSubList;
    }

    public int getCurPathItems() {
        if (0 == mSelectId) {
            // return album list
            if (null == mIdList) {
                return 0;
            }
            return mIdList.length;
        }
        if (null == mIdSubList) {
            return 0;
        }
        return mIdSubList.length;
    }

    /* clean up current folder */
    public void resetSubFolder() {
        mSelectId = 0;
        mIdSubList = null;
        mSubTitleLis = null;
        mSubArtistLis = null;
        mSubAlbumLis = null;

        if (mType == ALL_TYPE) {
            mSelectId = 1; // always has the selected id (enter the subfolder)
        }
    }

    /**
     * @brief get the current depth
     */
    public int getCurPathDepth() {
        if (mType == ALL_TYPE) {
            return 1; // all-type is flat
        }
        if (0 == mSelectId) {
            return 1;
        }
        return 2;
    }

    public boolean isCategoryRoot() {
        if (mType == ALL_TYPE) {
            return true; // all-type only has one level
        }
        if (0 == mSelectId) {
            return true;
        }
        return false;
    }

    boolean goDown(long id) {

        if (0 != mSelectId) {
            Log.v(TAG, "[BT][AVRCP] goDown reject because has mSelectId:" + mSelectId);
            return false;
        }
        for (int i = 0; i < mIdList.length; i++) {
            if (mIdList[i] == id) {
                mSelectId = id;
                updateSubSongs(mContext, id);
                return true;
            }
        }
        Log.v(TAG, "[BT][AVRCP] goDown fail because not found id:" + id);
        return false;
    }

    public String getCategoryName() {
        if (mPathArray != null) {
            return mPathArray[1];
        }
        return "<unknow>";
    }

    public byte getType() {
        if (mType == ALL_TYPE) {
            // all are element
            return BTAvrcpProfile.ITEM_TYPE_ELEMENT;
        }
        if (0 == mSelectId) {
            return BTAvrcpProfile.ITEM_TYPE_FOLDER;
        }
        return BTAvrcpProfile.ITEM_TYPE_ELEMENT;
    }

    public byte getFolderType() {
        return mType;
    }

    public String getNameByIndex(int index) {
        if (0 == mSelectId) {
            // return current list's by index
            if (index < mIdList.length) {
                return mNameList[index];
            }
        } else {
            switch (mType) {
                case ALL_TYPE:
                    if (index < mIdSubList.length) {
                        return mSubTitleLis[index];
                    }
                    break;
                case ARTIST_TYPE:
                    if (index < mIdSubList.length) {
                        return mSubArtistLis[index];
                    }
                    break;
                case ALBUM_TYPE:
                default:
                    if (index < mIdSubList.length) {
                        return mSubAlbumLis[index];
                    }
                    break;
            }
        }
        return null;
    }

    /*
     * @breif get the current item's attribute by its index and attr-id
     */
    public String getAttributeByIndex(int index, int attrId) {
        if (0 == mSelectId) {
            // return current list's by index
            if (index < mIdList.length) {
                return mNameList[index];
            }
        } else {
            if (index < mIdSubList.length) {
                switch (attrId) {
                    case 0x01: // title
                        return mSubTitleLis[index];
                    case 0x02:
                        return mSubArtistLis[index];
                    case 0x03:
                        return mSubAlbumLis[index];
                    default:
                        return null;
                }

            }
        }
        return null;
    }

    boolean goUp() {
        if (0 == mSelectId) {
            return false;
        }
        resetSubFolder();
        return true;
    }

    private long[] updateSongs(Context context) {
        boolean printDetail = true;
        String[] ccols;
        Cursor c;

        switch (mType) {
            case EMPTY_TYPE:
                mIdList = new long[0];
                mNameList = new String[0];
                return mIdList;
                // break;
            case ALL_TYPE:
                ccols = new String[] {
                        MediaStore.Audio.Artists._ID, MediaStore.Audio.Artists.ARTIST
                };
                c = query(context, MediaStore.Audio.Artists.EXTERNAL_CONTENT_URI, ccols, null, null, null);
                break;
            case ARTIST_TYPE:
                ccols = new String[] {
                        MediaStore.Audio.Artists._ID, MediaStore.Audio.Artists.ARTIST
                };
                c = query(context, MediaStore.Audio.Artists.EXTERNAL_CONTENT_URI, ccols, null, null, null);

                break;
            case ALBUM_TYPE:
            default:
                ccols = new String[] {
                        MediaStore.Audio.Albums._ID, MediaStore.Audio.Albums.ALBUM
                };
                c = query(context, MediaStore.Audio.Albums.EXTERNAL_CONTENT_URI, ccols, null, null, null);

                break;
        }

        Cursor cursor;
        try {
            if (c == null || c.getCount() == 0) {
                return null;
            }
            int len = c.getCount();
            if (len > 0) {
                mIdList = new long[len];
                mNameList = new String[len];

                for (int i = 0; i < len; i++) {

                    c.moveToNext();

                    mIdList[i] = c.getLong(0);
                    mNameList[i] = c.getString(1);

                }

            }
            return mIdList;

        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    public String getItemAttribute(byte scope, long identifier, short uidCounter, int attrId) {
        int i = 0;
        int index;
        if (attrId >= 0x01 && attrId <= 0x03) {
            if (0 != mSelectId) {
                for (i = 0; i < mIdSubList.length; i++) {
                    if (mIdSubList[i] == identifier) {
                        // found
                        index = i;
                        switch (attrId) {
                            case 0x01:
                                return mSubTitleLis[index];
                            case 0x02:
                                return mSubArtistLis[index];
                            case 0x03:
                                return mSubAlbumLis[index];
                            default:
                                continue;
                        }
                    }
                }
            } else {
                for (i = 0; i < mIdList.length; i++) {
                    if (mIdList[i] == identifier) {
                        index = i;
                        return mNameList[index];
                    }
                }
            }
        }
        return null;
    }

    /*
     * @brief Update the subfolder's list by current selected id if the type is ALL_TYPE, the current selected id is always
     * 1.
     */
    private long[] updateSubSongs(Context context, long selectedId) {
        boolean printDetail = true;
        final String[] ccols = new String[] {
                MediaStore.Audio.Media._ID, MediaStore.Audio.Media.TITLE, MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.ALBUM,
        };
        String where;
        switch (mType) {
            case EMPTY_TYPE:
                mIdSubList = new long[0];
                return mIdSubList;
                // break;
            case ALL_TYPE:
                where = MediaStore.Audio.Media.IS_MUSIC + "=1";
                break;
            case ARTIST_TYPE:
                where = MediaStore.Audio.Media.ARTIST_ID + "=" + selectedId + " AND " + MediaStore.Audio.Media.IS_MUSIC
                        + "=1";
                break;
            case ALBUM_TYPE:
            default:
                where = MediaStore.Audio.Media.ALBUM_ID + "=" + selectedId + " AND " + MediaStore.Audio.Media.IS_MUSIC
                        + "=1";
                break;

        }

        Cursor c = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, ccols, where, null,
                MediaStore.Audio.Media.TRACK);
        Cursor cursor;
        try {
            if (c == null || c.getCount() == 0) {
                return null;
            }
            int len = c.getCount();
            if (len > 0) {
                mIdSubList = new long[len];
                mSubTitleLis = new String[len];
                mSubArtistLis = new String[len];
                mSubAlbumLis = new String[len];

                for (int i = 0; i < len; i++) {

                    c.moveToNext();

                    mIdSubList[i] = c.getLong(0);
                    mSubTitleLis[i] = c.getString(1);
                    mSubArtistLis[i] = c.getString(2);
                    mSubAlbumLis[i] = c.getString(3);
                    if (printDetail) {
                        cursor = c;
                        Log.v(TAG, String.format("[AVRCP][UTIL] id:%d title:%s artist:%s album:%s", cursor.getLong(0),
                                cursor.getString(1), cursor.getString(2), cursor.getString(3)));
                    }
                }

            }
            return mIdSubList;

        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    public static Cursor query(Context context, Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder, int limit) {
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

    public static Cursor query(Context context, Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }
}
