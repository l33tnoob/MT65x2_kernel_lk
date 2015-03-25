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

package com.mediatek.FMRadio;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.BaseColumns;

/**
 * This class provider interface to operator databases, use by activity and service
 *
 */
public class FMRadioStation {
    public static final String TAG = "FmRx/Station"; // log tag
    // authority use composite content provider uri
    public static final String AUTHORITY = "com.mediatek.FMRadio.FMRadioContentProvider";
    // use to composite content provider uri
    public static final String STATION = "station";
    // default current station name
    public static final String CURRENT_STATION_NAME = "FmDfltSttnNm";
    
    // Default RDS settings
    private static final boolean DEFAULT_PSRT_ENABLED = false; // PS RT whether open
    private static final boolean DEFAULT_AF_ENABLED = false; // AF whether open
    private static final boolean DEFAULT_TA_ENABLED = false; // TA whether open
    
    // Station types.
    public static final int STATION_TYPE_CURRENT = 1; // station type current station
    public static final int STATION_TYPE_FAVORITE = 2; // station type favorite station
    public static final int STATION_TYPE_SEARCHED = 3; // station type searched station
    // just use to save rds, not really station type
    public static final int STATION_TYPE_RDS_SETTING = 4; 

    // RDS setting items
    public static final int RDS_SETTING_FREQ_PSRT = 1; // save PSRT set
    public static final int RDS_SETTING_FREQ_AF = 2; // save AF set
    public static final int RDS_SETTING_FREQ_TA = 3; // save TA set

    // RDS setting values for every item
    public static final String RDS_SETTING_VALUE_ENABLED = "ENABLED"; 
    public static final String RDS_SETTING_VALUE_DISABLED = "DISABLED";
    
    // The max count of favorite stations.
    public static final int MAX_FAVORITE_STATION_COUNT = 5;
    
    // stationList table in database columns
    static final String COLUMNS[] = new String[] {
        Station._ID,
        Station.COLUMN_STATION_NAME,
        Station.COLUMN_STATION_FREQ,
        // Use this type to identify different stations.
        Station.COLUMN_STATION_TYPE
    };

    /**
     * class provider the columns of StationList table
     *
     */
    public static final class Station implements BaseColumns {
        public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY + "/" + STATION);
        // Extra columns of the table: COLUMN_STATION_NAME COLUMN_STATION_FREQ COLUMN_STATION_TYPE
        public static final String COLUMN_STATION_NAME = "COLUMN_STATION_NAME";
        public static final String COLUMN_STATION_FREQ = "COLUMN_STATION_FREQ";
        public static final String COLUMN_STATION_TYPE = "COLUMN_STATION_TYPE";
    }
    
    /**
     * Init FM database for current station and RDS setting
     * 
     * @param context The context
     */
    public static void initFMDatabase(Context context) {
        // Init current station, if no current station in database, set to be FIXED_STATION_FREQ
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_FREQ},
                Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(STATION_TYPE_CURRENT)},
                null);
        
        if (null != cur) {
            try {
                if (!cur.moveToFirst()) {
                    // Can't find current station, insert FIXED_STATION_FREQ to be
                    final int size = 3; 
                    ContentValues values = new ContentValues(size);
                    values.put(Station.COLUMN_STATION_NAME, CURRENT_STATION_NAME);
                    values.put(Station.COLUMN_STATION_FREQ, FMRadioUtils.DEFAULT_STATION);
                    values.put(Station.COLUMN_STATION_TYPE, STATION_TYPE_CURRENT);
                    context.getContentResolver().insert(Station.CONTENT_URI, values);
                }
            } finally {
                cur.close();
            }
        }
        
        // Init PSRT, AF and TA
        int[] types = new int[] {RDS_SETTING_FREQ_PSRT, RDS_SETTING_FREQ_AF, RDS_SETTING_FREQ_TA};
        boolean[] enables = new boolean[] {DEFAULT_PSRT_ENABLED, DEFAULT_AF_ENABLED, DEFAULT_TA_ENABLED};
        for (int i = 0; i < types.length; i++) {
            cur = context.getContentResolver().query(
                    Station.CONTENT_URI,
                    new String[] {Station.COLUMN_STATION_NAME},
                    Station.COLUMN_STATION_FREQ + "=" + String.valueOf(types[i]),
                    null,
                    null);
            if (null != cur) {
                try {
                    if (!cur.moveToFirst()) {
                        final int size = 3;
                        ContentValues values = new ContentValues(size);
                        values.put(Station.COLUMN_STATION_NAME,
                                enables[i] ? RDS_SETTING_VALUE_ENABLED : RDS_SETTING_VALUE_DISABLED);
                        values.put(Station.COLUMN_STATION_FREQ, types[i]);
                        values.put(Station.COLUMN_STATION_TYPE, STATION_TYPE_RDS_SETTING);
                        context.getContentResolver().insert(Station.CONTENT_URI, values);
                    }
                } finally {
                    cur.close();
                }
            }
        }
        LogUtils.d(TAG, "FMRadioStation.initFMDatabase");
    }
    
    /**
     *  insert station information to database
     * @param context The context
     * @param stationName The station name
     * @param stationFreq The station frequency
     * @param stationType The station type
     */
    public static void insertStationToDB(Context context, String stationName, int stationFreq, int stationType) {
        LogUtils.d(TAG, "FMRadioStation.insertStationToDB start");
        final int size = 3;
        ContentValues values = new ContentValues(size);
        values.put(Station.COLUMN_STATION_NAME, stationName);
        values.put(Station.COLUMN_STATION_FREQ, stationFreq);
        values.put(Station.COLUMN_STATION_TYPE, stationType);
        context.getContentResolver().insert(Station.CONTENT_URI, values);
        LogUtils.d(TAG, "FMRadioStation.insertStationToDB end");
    }
    
    /**
     * update station name, station frequency according original station frequency, station type
     * @param context The context
     * @param stationName new station's name
     * @param oldStationFreq old station frequency
     * @param newStationFreq new station frequency
     * @param stationType old station type
     */
    public static void updateStationToDB(Context context, String stationName, int oldStationFreq,
            int newStationFreq, int stationType) {
        final int size = 3;
        ContentValues values = new ContentValues(size);
        values.put(Station.COLUMN_STATION_NAME, stationName);
        values.put(Station.COLUMN_STATION_FREQ, newStationFreq);
        values.put(Station.COLUMN_STATION_TYPE, stationType);
        context.getContentResolver().update(
                Station.CONTENT_URI,
                values,
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(oldStationFreq), String.valueOf(stationType)});
        LogUtils.d(TAG, "FMRadioStation.updateStationToDB: name = " + stationName + ", new freq = " + newStationFreq);
    }
    
    /**
     * update station name and station type according station frequency
     * @param context The context
     * @param newStationName station new name
     * @param newStationType station new type
     * @param stationFreq original station frequency
     */
    public static void updateStationToDB(Context context, String newStationName, int newStationType, int stationFreq) {
        final int size = 3;
        ContentValues values = new ContentValues(size);
        values.put(Station.COLUMN_STATION_NAME, newStationName);
        values.put(Station.COLUMN_STATION_FREQ, stationFreq);
        values.put(Station.COLUMN_STATION_TYPE, newStationType);
        context.getContentResolver().update(
                Station.CONTENT_URI,
                values,
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "<>" + STATION_TYPE_CURRENT,
                new String[] {String.valueOf(stationFreq)});
        LogUtils.d(TAG, "FMRadioStation.updateStationToDB: new name = " + newStationName
                + ", new freq type = " + newStationType);
    } 
    
    /**
     * delete station according station frequency and station type
     * @param context The context
     * @param stationFreq station frequency
     * @param stationType station type
     */
    public static void deleteStationInDB(Context context, int stationFreq, int stationType) {
       context.getContentResolver().delete(
                Station.CONTENT_URI,
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(stationFreq), String.valueOf(stationType)});
       LogUtils.d(TAG, "FMRadioStation.deleteStationInDB: freq = " + stationFreq + ", type = " + stationType);
    }
    
    /**
     * judge a station whether exist according station frequency and station type
     * @param context The context
     * @param stationFreq station frequency
     * @param stationType station type
     * @return true or false indicate whether station is exist
     */
    public static boolean isStationExist(Context context, int stationFreq, int stationType) {
        LogUtils.d(TAG, ">>> isStationExist: stationFreq=" + stationFreq + ",stationType=" + stationType);
        boolean isExist = false;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(stationFreq), String.valueOf(stationType)},
                null);
        
        if (null != cur) {
            try {
                if (cur.moveToFirst()) {
                    // This station is exist
                    isExist = true;
                }
            } finally {
                cur.close();
            }
        }
        
        LogUtils.d(TAG, "<<< isStationExist: " + isExist);
        return isExist;
    }
    
    /**
     * judge a station whether exist according station frequency
     * @param context The context
     * @param stationFreq station frequency
     * @param stationType station type
     * @return true or false indicate whether station is exist
     */
    public static boolean isStationExistInChList(Context context, int stationFreq) {
        LogUtils.d(TAG, ">>> isStationExist: stationFreq=" + stationFreq);
        boolean isExist = false;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "<>1",
                new String[] {String.valueOf(stationFreq)},
                null);
        
        if (null != cur) {
            try {
                if (cur.moveToFirst()) {
                    // This station is exist
                    isExist = true;
                }
            } finally {
                cur.close();
            }
        }
        
        LogUtils.d(TAG, "<<< isStationExist: " + isExist);
        return isExist;
    }
    
    /**
     * get current station from database
     * @param context The context
     * @return the station which station type is current
     */
    public static int getCurrentStation(Context context) {
        int currentStation = FMRadioUtils.DEFAULT_STATION;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_FREQ},
                Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(STATION_TYPE_CURRENT)},
                null);
        if (null != cur) {
            try {
                if (cur.moveToFirst()) {
                    currentStation = cur.getInt(0);
                    if (!FMRadioUtils.isValidStation(currentStation)) {
                        // If current station is invalid, use default and update the database
                        currentStation = FMRadioUtils.DEFAULT_STATION;
                        setCurrentStation(context, currentStation);
                        LogUtils.w(TAG, "FMRadioStation.getCurrentStation: current station is invalid, use default!");
                    }
                }
            } finally {
                cur.close();
            }
        }
        LogUtils.d(TAG, "FMRadioStation.getCurrentStation: " + currentStation);
        return currentStation;
    }
    
    /**
     * set current station 
     * @param context The context 
     * @param station station frequency
     */
    public static void setCurrentStation(Context context, int station) {
        LogUtils.d(TAG, "FMRadioStation.setCurrentStation start");
        // Update current station to database.
        final int size = 3;
        ContentValues values = new ContentValues(size);
        values.put(Station.COLUMN_STATION_NAME, CURRENT_STATION_NAME);
        values.put(Station.COLUMN_STATION_FREQ, station);
        values.put(Station.COLUMN_STATION_TYPE, STATION_TYPE_CURRENT);
        context.getContentResolver().update(
                Station.CONTENT_URI,
                values,
                Station.COLUMN_STATION_NAME + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {CURRENT_STATION_NAME, String.valueOf(STATION_TYPE_CURRENT)});
        LogUtils.d(TAG, "FMRadioStation.setCurrentStation end");
    }
    
    /**
     * clean all stations which station type is searched
     * @param context The context
     */
    public static void cleanSearchedStations(Context context) {
        LogUtils.d(TAG, "FMRadioStation.cleanSearchedStations start");
        context.getContentResolver().delete(
                Station.CONTENT_URI,
                Station.COLUMN_STATION_TYPE + "=" + String.valueOf(STATION_TYPE_SEARCHED),
                null);
        LogUtils.d(TAG, "FMRadioStation.cleanSearchedStations end");
    }
    
    /**
     * get station name according station frequency and station type
     * @param context The context
     * @param stationFreq station frequency
     * @param stationType station type
     * @return station name
     */
    public static String getStationName(Context context, int stationFreq, int stationType) {
        LogUtils.d(TAG, "FMRadioStation.getStationName: type = "
                + stationType + ", freq = " + stationFreq);
        // If can't find this station id database, return default station name
        String stationName = context.getString(R.string.default_station_name);
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(stationFreq), String.valueOf(stationType)},
                null);
        if (null != cur) {
            try {
                if (cur.moveToFirst()) {
                    stationName = cur.getString(0);
                }
            } finally {
                cur.close();
            }
        }
        
        LogUtils.d(TAG, "FMRadioStation.getStationName: stationName = " + stationName);
        return stationName;
    }
    
    /**
     * judge whether station is a favorite station
     * @param context The context
     * @param iStation station frequency
     * @return true or false indicate whether station type is favorite
     */
    public static boolean isFavoriteStation(Context context, int iStation) {
        return isStationExist(context, iStation, STATION_TYPE_FAVORITE);
    }
    
    /**
     * get station count according station type
     * @param context The context
     * @param stationType station type
     * @return numbers of station according station type
     */
    public static int getStationCount(Context context, int stationType) {
        LogUtils.d(TAG, "FMRadioStation.getStationCount Type: " + stationType);
        int stationNus = 0;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                COLUMNS,
                Station.COLUMN_STATION_TYPE + "=?",
                new String[]{String.valueOf(stationType)},
                null);
        if (null != cur) {
            try {
                stationNus = cur.getCount();
            } finally {
                cur.close();
            }
        }
        
        LogUtils.d(TAG, "FMRadioStation.getStationCount: " + stationNus);
        return stationNus;
    }

    /**
     * Set the RDS setting include PSRT, AF and TA into database
     * 
     * @param context The context to get resolver
     * @param type The type which RDS setting to be get. One of
     *  {@link #RDS_SETTING_FREQ_PSRT}, {@link #RDS_SETTING_FREQ_AF},or {@link #RDS_SETTING_FREQ_TA}.
     * @param enable Whether the RDS setting enable
     */
    /*public static void setRDSEnabled(Context context, int type, boolean enable) {

        // Update RDS settings whether enabled to database
        final int size = 3;
        ContentValues values = new ContentValues(size);
        values.put(Station.COLUMN_STATION_NAME, enable ? RDS_SETTING_VALUE_ENABLED
                : RDS_SETTING_VALUE_DISABLED);
        values.put(Station.COLUMN_STATION_FREQ, type);
        values.put(Station.COLUMN_STATION_TYPE, STATION_TYPE_RDS_SETTING);
        context.getContentResolver().update(Station.CONTENT_URI, values,
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(type), String.valueOf(STATION_TYPE_RDS_SETTING) });
        LogUtils.d(TAG, "FMRadioStation.setRDSEnabled: enable = " + enable);
    }*/

    /**
     * Get the given type RDS setting
     * 
     * @param context The context to get resolver
     * @param type The type which RDS setting to be get. One of
     *  {@link #RDS_SETTING_FREQ_PSRT}, {@link #RDS_SETTING_FREQ_AF},or {@link #RDS_SETTING_FREQ_TA}.
     * @return If the given type's RDS setting is enabled, return true, otherwise return false 
     */
    /*public static boolean getRDSEnabled(Context context, int type) {
        boolean enable = false;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                Station.COLUMN_STATION_FREQ + "=" + String.valueOf(type),
                null,
                null);
        if (null != cur) {
            try {
                if (cur.moveToFirst()) {
                    enable = (RDS_SETTING_VALUE_ENABLED.equals(cur.getString(0)));
                }
            } finally {
                cur.close();
            }
        }
        LogUtils.d(TAG, "FMRadioStation.getRDSEnable: " + type + ":" + enable);
        return enable;
    }*/
    
    /**
     * clear all station of FMRadio database 
     * @param context application context
     */
    public static void cleanAllStations(Context context) {
        Uri uri = Station.CONTENT_URI;
        Cursor cur = context.getContentResolver().query(uri, COLUMNS, null, null, null);
        if (null != cur) {
            try {
                cur.moveToFirst();
                while (!cur.isAfterLast()) {
                    // Have find one station.
                    uri = ContentUris.appendId(Station.CONTENT_URI.buildUpon(),
                            cur.getInt(cur.getColumnIndex(Station._ID))).build();
                    context.getContentResolver().delete(uri, null, null);
                    cur.moveToNext();
                }
            } finally {
                cur.close();
            }
        }
    }
}
