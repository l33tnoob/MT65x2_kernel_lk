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


package com.mediatek.app.mtv;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.mediatek.atv.AtvChannelManager;
import com.mediatek.atv.AtvChannelManager.AtvChannelEntry;
import com.mediatek.mtvbase.ChannelManager;
import com.mediatek.mtvbase.ChannelManager.ChannelTableEmpty;

import java.util.Arrays;

/**
 *ChannelProvider supply operations of database. We can save the channel list by
 *county in database.
 */
public class ChannelProvider implements ChannelManager.ChannelHolder {

    private static final String TAG = "ATV/ChannelProvider";

    private static final String DATABASE_NAME = "channels.db";
    private static final int DATABASE_VERSION = 2;
    private String mCurrTable;    
    private static final int MTV_MODE = MtvEngine.MTV_ATV;
    private String[] mAreaCodes;
    private DatabaseHelper mOpenHelper;   
    private SQLiteDatabase mDb;
    private static ChannelProvider sChannelProvider = new ChannelProvider();

    /**
     * Get the ChannelProvider instance.
     * @param context The context ChannelProvider run in.
     * @param codes Support country code.
     * @return The ChannelProvider instance.
     */
    public static ChannelProvider instance(Context context,String[] codes) {
        if (sChannelProvider.mOpenHelper == null) {
            sChannelProvider.mOpenHelper = sChannelProvider.new DatabaseHelper(context.getApplicationContext());          
        }
        //activity may be restarted so update codes every time.
        if (codes != null) {
            sChannelProvider.mAreaCodes = codes;
        }
        sChannelProvider.mDb = sChannelProvider.mOpenHelper.getWritableDatabase();
        return sChannelProvider;
    }    

    private ChannelProvider() {
        //just to declare a private constructor.
    }    

    public static final String CHANNEL_ENTRY = "chEntry";
    public static final String CHANNEL_NUM = "chNum";
    public static final String CHANNEL_NAME = "chName";
    //public static final String IMAGE_PATH = "imgPath";    
    
    /**
     * This class helps open, create, and upgrade the database file.
     */
    private class DatabaseHelper extends SQLiteOpenHelper {

        DatabaseHelper(Context context) {
            super(context, DATABASE_NAME, null, DATABASE_VERSION);
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            for (int i = 0; i < mAreaCodes.length; i++) {
                if (!mAreaCodes[i].equals("-1")) {
                    db.execSQL("CREATE TABLE " + "Channel" + MTV_MODE + "_" + mAreaCodes[i] + " ("
                            //use channel number as row ID to get benefits on such as sorting,inserting...
                            + CHANNEL_NUM + " INTEGER PRIMARY KEY,"
                            + CHANNEL_ENTRY + " INTEGER,"                    
                            + CHANNEL_NAME + " TEXT"                    
                            //+ IMAGE_PATH + " TEXT"
                            + ");");
                }
            }
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            XLogUtils.w(TAG, "Upgrading database from version " + oldVersion + " to "
                    + newVersion + ", which will destroy all old data");
            db.execSQL("DROP TABLE IF EXISTS " + mCurrTable);
            onCreate(db);
        }
    }

    //marked for not use
    /*public void clear() {    
        XLogUtils.d(TAG, "clear");
        SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        db.execSQL("DROP TABLE IF EXISTS "+mCurrTable);
        mOpenHelper.onCreate(db);
    }*/
    
    /**
     * Insert a channel info in database.
     * @param initialValues Contains the channel info.
     * @return The row id of the newly inserted row, or -1 if an error occurred.
     */
    public long insert(ContentValues initialValues) {
        //SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        return mDb.insert(mCurrTable, null, initialValues);
    }

    /**
     * Delete the channel info in current table.
     * @param chNum Tje channel number we want to delete.
     * @return The number of row affected.
     */
    public int delete(long chNum) {
        XLogUtils.d(TAG, "delete chNum = " + chNum);
        //SQLiteDatabase db = mOpenHelper.getWritableDatabase();        
        return mDb.delete(mCurrTable, CHANNEL_NUM + "=" + chNum, null);
    }

    /**
     * Update the channel's info.
     * @param chNum The channel we want to update.
     * @param values Content new channel value.
     * @return the number of rows affected.
     */
    public int update(long chNum, ContentValues values) {
        XLogUtils.d(TAG, "update chNum = " + chNum);
        
        //SQLiteDatabase db = mOpenHelper.getWritableDatabase();
        return mDb.update(mCurrTable, values, CHANNEL_NUM + "=" + chNum, null);
    }

    /**
     * Set the current location.
     * @param mode The current mode.
     * @param loc The current location.
     */
    public synchronized void setTableLocation(int mode,String loc) {
        mCurrTable = "Channel" + mode + "_" + loc;
        XLogUtils.d(TAG, "setTableLocation mCurrTable = " + mCurrTable);
    }

    /**
     * Set current location.
     * @param currTable The current location table.
     */
    public synchronized void setTableLocation(String currTable) {
        mCurrTable = currTable;
        XLogUtils.d(TAG, "setTableLocation mCurrTable = " + mCurrTable);
    }

    /**
     * Get the current location table.
     * @return Current location table.
     */
    public synchronized String getTableLocation() {
        return mCurrTable;        
    }

    /**
     * Query the current location table cursor.
     * @param columns A list of which columns to return.
     * @param all Query all or not.
     * @return Query result first entry.
     */
    public Cursor getCursor(String[] columns,boolean all) {
        //SQLiteDatabase db = mOpenHelper.getReadableDatabase();        
        Cursor c = mDb.query(mCurrTable, columns, all ? null : CHANNEL_ENTRY + ">0", null, null,null, CHANNEL_NUM + " ASC");
        if (c == null) {
            throw new IllegalArgumentException("Unknown fields " + Arrays.toString(columns));
        } else {
            return c;
        }
    }
    
    
    /**
     * Handy function for inserting channel info.
     * @param chNum The channel number needed to be insert.
     * @param The channel's info.
     * @param The channel's name.
     */
    public long insertChannelEntry(int chNum,long entry,String name) {
        XLogUtils.d(TAG, "insertChannelEntry() chNum = " + chNum + " entry = " + entry);
    
        ContentValues initialValues = new ContentValues();
        
        initialValues.put(CHANNEL_ENTRY, entry);
        initialValues.put(CHANNEL_NUM, chNum);
        //default channel name is Ch xx,while xx is the channel number.
        initialValues.put(CHANNEL_NAME,name);
        //initialValues.put(IMAGE_PATH, "");
        
        return insert(initialValues);
    }
    
    /**
     *Handy function for updating channel info.
     * @param chNum The channel's number.
     * @param entry The channel's info.
     * @return The row number affected.
     */
    public int updateChannelEntry(int chNum,long entry) {
        XLogUtils.d(TAG, "updateChannelEntry chNum = " + chNum + " entry = " + entry);
        ContentValues initialValues = new ContentValues();
        
        initialValues.put(CHANNEL_ENTRY, entry);
        return update(chNum,initialValues);
    }    
    
    
    /**
     * Handy function for updating channel name.
     * @param chNum The channel's number.
     */
    public int updateChannelName(int chNum,String name) {
        XLogUtils.d(TAG, "updateChannelName chNum = " + chNum + " name = " + name);
        ContentValues initialValues = new ContentValues();
        
        initialValues.put(CHANNEL_NAME, name);
        return update(chNum,initialValues);
    }     
    
    /**
     * Get channel table from database.
     * @param mode Current mode.
     * @param manager ChannelManager instance.
     * @return ATV channel info table.
     * @throws ChannelTableEmpty
     */
    public synchronized ChannelManager.ChannelEntry[] getChannelTable(
              int mode,ChannelManager manager) throws ChannelTableEmpty {
        if (mode == MtvEngine.MTV_ATV) {
            Cursor cursor = getCursor(new String[]{CHANNEL_NUM,CHANNEL_ENTRY},false);
            if (cursor.getCount() <= 0) {
                cursor.close();
                throw new ChannelTableEmpty();
            }
            
            AtvChannelEntry[] table = new AtvChannelEntry[cursor.getCount()];
            
            int chIndex = cursor.getColumnIndex(ChannelProvider.CHANNEL_NUM);
            int entryIndex = cursor.getColumnIndex(ChannelProvider.CHANNEL_ENTRY);
            int i = 0;
            
            cursor.moveToFirst();
            do {
                table[i] = ((AtvChannelManager)manager).new AtvChannelEntry();
                table[i].ch = cursor.getInt(chIndex);
                table[i].packedEntry = cursor.getLong(entryIndex);
                i++;
            } while (cursor.moveToNext());
            cursor.close();
            return table;
        }
        
        return new AtvChannelEntry[0];
    }
    
}
