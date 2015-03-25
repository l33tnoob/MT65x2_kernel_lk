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

package com.mediatek.FMTransmitter;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.BaseColumns;

import com.mediatek.common.featureoption.FeatureOption;

public class FMTransmitterStation {
    public static final String TAG = "FMTx/FMTransmitterStation";
    
    public static final String AUTHORITY  = "com.mediatek.FMTransmitter.FMTransmitterContentProvider";
    public static final String STATION = "TxStation";
    public static final int FIXED_STATION_FREQ = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 10000 : 1000; 
    public static final int HIGHEST_STATION = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 10800 : 1080;
    public static final int LOWEST_STATION = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 8750 : 875;
    public static final String CURRENT_STATION_NAME = "FmTxDfltSttnNm";
    
    // Station types.
    public static final int STATION_TYPE_CURRENT = 1;
    public static final int STATION_TYPE_FAVORITE = 2;
    public static final int STATION_TYPE_SEARCHED = 3;
    public static final int STATION_TYPE_RDS_SETTING = 4;

    // RDS setting items
    public static final int RDS_SETTING_FREQ_PSRT = 1;
    public static final int RDS_SETTING_FREQ_AF = 2;
    public static final int RDS_SETTING_FREQ_TA = 3;

    // RDS setting values for every item
    public static final String RDS_SETTING_VALUE_ENABLED = "ENABLED";
    public static final String RDS_SETTING_VALUE_DISABLED = "DISABLED";
    
    // The max count of favorite stations.
    public static final int MAX_FAVORITE_STATION_COUNT = 5;

    static final String COLUMNS[] = new String[] {
        Station._ID,
        Station.COLUMN_STATION_NAME,
        Station.COLUMN_STATION_FREQ,
        // Use this type to identify different stations.
        Station.COLUMN_STATION_TYPE
    };

    // BaseColumn._ID = "_id"
    public static final class Station implements BaseColumns {
        public static final Uri CONTENT_URI  = Uri.parse("content://" + AUTHORITY + "/" + STATION);
        // Extra columns of the table: COLUMN_STATION_NAME COLUMN_STATION_FREQ COLUMN_STATION_TYPE
        public static final String COLUMN_STATION_NAME = "COLUMN_STATION_NAME";
        public static final String COLUMN_STATION_FREQ = "COLUMN_STATION_FREQ";
        public static final String COLUMN_STATION_TYPE = "COLUMN_STATION_TYPE";
    }
    
    public static void insertStationToDB(Context context, String stationName, int stationFreq, int stationType) {
        FMTxLogUtils.d(TAG, "insertStationToDB Start");
        ContentValues values = new ContentValues(3);
        values.put(Station.COLUMN_STATION_NAME, stationName);
        values.put(Station.COLUMN_STATION_FREQ, stationFreq);
        values.put(Station.COLUMN_STATION_TYPE, stationType);
        context.getContentResolver().insert(Station.CONTENT_URI, values);
        FMTxLogUtils.d(TAG, "insertStationToDB End");
    }

    public static void updateStationToDB(Context context, String stationName, int stationFreq, int stationType) {
            ContentValues values = new ContentValues(3);
            values.put(Station.COLUMN_STATION_NAME, stationName);
            values.put(Station.COLUMN_STATION_FREQ, stationFreq);
            values.put(Station.COLUMN_STATION_TYPE, stationType);
            context.getContentResolver().update(
                    Station.CONTENT_URI,
                    values,
                    Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                    new String[] {String.valueOf(stationFreq), String.valueOf(stationType)});
            FMTxLogUtils.d(TAG, "updateStationToDB: Name= " + stationName + ", FreqType = " + stationType);
    }
    
    public static void deleteStationInDB(Context context, int stationFreq, int stationType) {
        context.getContentResolver().delete(
                Station.CONTENT_URI,
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(stationFreq), String.valueOf(stationType)});
        FMTxLogUtils.d(TAG,
                "deleteStationInDB: Freq = " + stationFreq + ", FreqType = " + stationType);
    }
    
    public static boolean isStationExist(Context context, int stationFreq, int stationType) {
        boolean isExist = false;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                Station.COLUMN_STATION_FREQ + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                new String[] {String.valueOf(stationFreq), String.valueOf(stationType)},
                null);
        
        if (null != cur) {
                if (cur.getCount() > 0) {
                    isExist = true;
                }
                cur.close();
                cur = null;
            }
        FMTxLogUtils.d(TAG, "isStationExist: " + isExist);
        return isExist;
    }
    
    public static int getCurrentStation(Context context) {
        int currentStation = FIXED_STATION_FREQ;
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
                    if (currentStation < LOWEST_STATION || currentStation > HIGHEST_STATION) {
                        //Update the database
                        currentStation = FIXED_STATION_FREQ;
                        setCurrentStation(context, currentStation);
                        FMTxLogUtils.d(TAG, 
                                "FMTransmitterStation.getCurrentStation: current station is invalid, use default!");
                    }
                }
            } finally {
                cur.close();
                cur = null;
            }
        }
        FMTxLogUtils.d(TAG, "getCurrentStation: " + currentStation);
        return currentStation;

    }
    
    public static void setCurrentStation(Context context, int iStation) {
        ContentValues values = new ContentValues(3);
        values.put(Station.COLUMN_STATION_NAME, CURRENT_STATION_NAME);
        values.put(Station.COLUMN_STATION_FREQ, iStation);
        values.put(Station.COLUMN_STATION_TYPE, STATION_TYPE_CURRENT);
        if (getStationCount(context,STATION_TYPE_CURRENT) == 1) {
            context.getContentResolver().update(
                    Station.CONTENT_URI,
                    values,
                    Station.COLUMN_STATION_NAME + "=? AND " + Station.COLUMN_STATION_TYPE + "=?",
                    new String[] {CURRENT_STATION_NAME, String.valueOf(STATION_TYPE_CURRENT)});
        } else {
            context.getContentResolver().insert(
                    Station.CONTENT_URI,
                    values
                    );
        }
        FMTxLogUtils.d(TAG, "setCurrentStation: " + iStation);
    }
    
    public static void cleanDB(Context context) {
        FMTxLogUtils.d(TAG, "cleanDB Start");
        context.getContentResolver().delete(Station.CONTENT_URI, null, null);
        FMTxLogUtils.d(TAG, "cleanDB End");
    }
    
    public static void cleanSearchedStations(Context context) {
        FMTxLogUtils.d(TAG, "cleanSearchedStations Start");
        context.getContentResolver().delete(
                Station.CONTENT_URI,
                Station.COLUMN_STATION_TYPE + "=" + String.valueOf(STATION_TYPE_SEARCHED),
                null);
        FMTxLogUtils.d(TAG, "cleanSearchedStations End");
    }
    
    public static boolean isDBEmpty(Context context) {
        boolean isEmpty = true;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                new String[] {Station.COLUMN_STATION_NAME},
                null,
                null,
                null);
        if (null != cur) {
            if (cur.getCount() > 0) {
              isEmpty = false;
            }
        cur.close();
        cur = null;
    }
        FMTxLogUtils.d(TAG, "isDBEmpty: " + isEmpty);
        return isEmpty;
    }
    

    public static int getStationCount(Context context, int stationType) {
        int stationCount = 0;
        Cursor cur = context.getContentResolver().query(
                Station.CONTENT_URI,
                COLUMNS,
                Station.COLUMN_STATION_TYPE + "=?",
                new String[]{String.valueOf(stationType)},
                null);
        if (null != cur) {
            stationCount = cur.getCount();
            cur.close();
            cur = null;
        }
        FMTxLogUtils.d(TAG, "getStationCount: " + stationCount);
        return stationCount;
        
    }

}
