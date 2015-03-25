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
import android.widget.EditText;

public class BTAvrcpBrowserSearch {
    private EditText mAddrET;
    private EditText mToAddrET;
    public static int sNum = 0;
    public static final String TAG = "AVRCP_UTIL";

    private long[]  mSearchIDList;
    private String[] mSearchTitle;
    private String[] mSearchArtist;
    private String[] mSearchAlbum;


    private Context mContext;

    BTAvrcpBrowserSearch(Context context) {
        mContext = context;
    }

    public int search(String sText) {
        Log.v(TAG, "Search data " + sText);
        mSearchIDList = getSearchSongs(this.mContext, sText);
        for (int j = 0; mSearchTitle != null && j < mSearchTitle.length; j++) {
            if (null != mSearchIDList && j < mSearchIDList.length) {
                Log.v(TAG, "Search Result " + j + " id:" + mSearchIDList[j] + " title:" + mSearchTitle[j]);
            } else {
                Log.v(TAG, "Search Result mSearchIDList[j] is not exist!");
            }
        }
        if (null == mSearchIDList) {
            return 0;
        }
        return mSearchIDList.length;
    }

    public long[] getSearchedList() {
        if (null == mSearchIDList) {
            Log.v(TAG, "[BT][AVRCP] getSearchedList mSearchIDList is null");
            return EMPTY_LIST;
        }
        return mSearchIDList;
    }
    public String getSearchedTitleString(int index) {

        return mSearchTitle[index];
    }
    public String getSearchedArtistString(int index) {
        return mSearchArtist[index];
    }
    public String getSearchedAlbumString(int index) {
        return mSearchAlbum[index];
    }
    /* for test */
    private void listAllMusic() {
        long list[];
        long id;
        int half;

        list = getAllSongs(this.mContext);
        for (int i = 0; null != list && i < list.length ; i++) {
            id = list[i];
            Log.v(TAG, "find id:" + id);
        }

        if (list != null) {
            half = list.length;
        } else {
            half = 0;
        }
        half = half / 2;
        getSongData(this.mContext, half);

        Log.v(TAG, "Search empty data ");
        mSearchIDList = getSearchSongs(this.mContext, "fdsafsfasfasd");
        for (int j = 0; mSearchTitle != null && j < mSearchTitle.length; j++) {
            Log.v(TAG, "Search Result " + j + " " + mSearchTitle[j]);
        }

        Log.v(TAG, "Search es data ");
        mSearchIDList = getSearchSongs(this.mContext, "es");
        for (int j = 0; mSearchTitle != null && j < mSearchTitle.length; j++) {
            Log.v(TAG, "Search Result " + j + " " + mSearchTitle[j]);
        }
    }

    /***** ******/
    private static final long[] EMPTY_LIST = new long[0];

    public static long [] getSongListForCursor(Cursor cursor) {
        if (cursor == null) {
            return EMPTY_LIST;
        }
        int len = cursor.getCount();
        long [] list = new long[len];
        cursor.moveToFirst();
        int colidx = -1;
        try {
            colidx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Playlists.Members.AUDIO_ID);
        } catch (IllegalArgumentException ex) {
            colidx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
        }
        for (int i = 0; i < len; i++) {
            list[i] = cursor.getLong(colidx);
            cursor.moveToNext();
        }
        return list;
    }

    public static long [] getSongListForArtist(Context context, long id) {
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID };
        String where = MediaStore.Audio.Media.ARTIST_ID + "=" + id + " AND " +
        MediaStore.Audio.Media.IS_MUSIC + "=1";
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, null,
                MediaStore.Audio.Media.ALBUM_KEY + ","  + MediaStore.Audio.Media.TRACK);

        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return EMPTY_LIST;
    }

    public static long [] getSongListForAlbum(Context context, long id) {
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID };
        String where = MediaStore.Audio.Media.ALBUM_ID + "=" + id + " AND " +
                MediaStore.Audio.Media.IS_MUSIC + "=1";
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, null, MediaStore.Audio.Media.TRACK);

        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return EMPTY_LIST;
    }

    public static long [] getSongListForPlaylist(Context context, long plid) {
        final String[] ccols = new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID };
        Cursor cursor = query(context, MediaStore.Audio.Playlists.Members.getContentUri("external", plid),
                ccols, null, null, MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);

        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return EMPTY_LIST;
    }


    public static String getSongData(Context context, long id) {
        final String[] ccols = new String[] {
                MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.TITLE  };
        String album = "";
        int albumIndex;
        int titleIndex;
        int artistIndex;

        String where = MediaStore.Audio.Media._ID + "=" + id + " AND " +
                MediaStore.Audio.Media.IS_MUSIC + "=1";
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, null, MediaStore.Audio.Media.TRACK);

        if (cursor != null) {
            try {
                albumIndex = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM);
                titleIndex = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST);
                artistIndex = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.TITLE);
            } catch (IllegalArgumentException ex) {
                albumIndex = 0;
                titleIndex = 0;
                artistIndex = 0;
            }

            if( titleIndex != artistIndex ){ // avoid not getting the index
            if( cursor.getCount() > 0 ){
                cursor.moveToNext();
                album = cursor.getString(albumIndex);
            } else {
                Log.e(TAG, "[UTIL] query and get empty result !");
            }

            for (int j = 0; j < cursor.getCount(); j++) {
                Log.v(TAG, String.format("[UTIL] found: album:'%s' artist:'%s' title:'%s' ", cursor.getString(albumIndex),
                        cursor.getString(artistIndex) , cursor.getString(albumIndex)));
                cursor.moveToNext();
            }
            }
            cursor.close();
            cursor = null;
        }
        return album;
    }

    public static long [] getAllSongs(Context context) {
        boolean printDetail = true;
        final String[] ccols = new String[] {
                MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.TITLE  };
        int albumIndex = 1;
        int titleIndex = 2;
        int artistIndex = 3;
        Cursor c = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, MediaStore.Audio.Media.IS_MUSIC + "=1",
                null, null);
        Cursor cursor;
        try {
            if (c == null || c.getCount() == 0) {
                return null;
            }
            int len = c.getCount();
            long [] list = new long[len];
            for (int i = 0; i < len; i++) {



                c.moveToNext();
                list[i] = c.getLong(0);
                if (printDetail) {
                    cursor = c;
                    Log.v(TAG, String.format("[UTIL] id:%d album:'%s' artist:'%s' title:'%s' " ,
                            cursor.getLong(0), cursor.getString(titleIndex),
                            cursor.getString(artistIndex) , cursor.getString(albumIndex)));
                }
            }

            return list;
        } finally {
            if (c != null) {
                c.close();
            }
        }

    }

    public long [] getSearchSongs(Context context, String sSearch) {
        boolean printDetail = true;
        int num = 0;

        final String[] ccols = new String[] {
                MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.TITLE  };
        int albumIndex = 1;
        int titleIndex = 3;
        int artistIndex = 2;
        Cursor c = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, MediaStore.Audio.Media.IS_MUSIC + "=1",
                null, null);
        Cursor cursor;
        String scompare;

        if (null == sSearch || sSearch.length() == 0) {
            Log.e(TAG, "[UTIL] getSearchSongs wrong parameter");
            if( null != c ){
                c.close();
                c = null;
            }  
            return EMPTY_LIST;
        }
        Log.v(TAG, "[AVRCP] search getSearchSongs");
        try {
            if (c == null || c.getCount() == 0) {
                return null;
            }
            int len = c.getCount();

            for (int i = 0; i < len; i++) {
                c.moveToNext();

                scompare = c.getString(titleIndex);
                if (scompare.indexOf(sSearch) != -1) {
                    // add
                    num ++;
                    if (printDetail) {
                        cursor = c;
                        Log.v(TAG, String.format("[UTIL] id:%d album:'%s' artist:'%s' title:'%s' ", 
                                cursor.getLong(0), cursor.getString(titleIndex), 
                                cursor.getString(artistIndex) , cursor.getString(albumIndex)));
                    }
                }
            }

            long [] list = new long[num];
            mSearchTitle = new String[num];
            mSearchArtist = new String[num];
            mSearchAlbum = new String[num];
            if (num > 0) {
                c.moveToFirst();

                num = 0;
                for (int i = 0; i < len ; i++) {
                    scompare = c.getString(titleIndex);

                    if (scompare.indexOf(sSearch) != -1) {
                        list[num] = c.getLong(0);
                        mSearchTitle[num] = c.getString(titleIndex);
                        mSearchArtist[num] = c.getString(artistIndex);
                        mSearchAlbum[num] = c.getString(albumIndex);

                        num ++;
                    }

                    c.moveToNext();
                }
            } else {
                Log.w(TAG, "[BT][AVRCP][WRN] found 0 result !");
            }
            Log.w(TAG, "[BT][AVRCP] found result list.length:" + list.length);
            return list;
        } finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
    }

    public static Cursor query(Context context, Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder, int limit) {
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
    public static Cursor query(Context context, Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }
}
