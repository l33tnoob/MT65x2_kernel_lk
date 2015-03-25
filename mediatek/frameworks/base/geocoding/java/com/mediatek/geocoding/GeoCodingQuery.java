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

package com.mediatek.geocoding;

import android.database.Cursor;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import java.sql.SQLException;

import java.io.File;
import android.telephony.Rlog;

import android.location.CountryDetector;
import java.util.Locale;

/**
 * Singleton design pattern
 */
public class GeoCodingQuery {
    private static final String LOG_TAG = "GeoCodingQuery";
    private static final String DBFilePath = "/system/etc/geocoding.db";
    private DBHelper mDbHelper = null;
    private SQLiteDatabase mDatabase;
    private boolean mIsDBReady = false;
    private Context mContext = null;
    
    public GeoCodingQuery(Context context) {
        mContext = context;
        openDatabase(context);
    }

    private boolean canQuery() {
        return mIsDBReady;
    }

    public String queryByNumber(String number) {
        String returnValue = "";
        int numberValidLength = 11;
        int numberTailLength = 4;
        String countryIso = "";

        if (mDbHelper == null) {
            Rlog.d(LOG_TAG, "Database is not opened !");
            return returnValue;
        }

        Rlog.d(LOG_TAG, "number = " + number);
        /* Ignore space character and get the previous 7 number */
        String queryNumber = number.replaceAll(" ", "");
        int numberTotalLength = queryNumber.length();
        if (numberTotalLength < numberValidLength) {
            Rlog.d(LOG_TAG, "The length of dial number is less than 11 !");
            return returnValue;
        }

        if (queryNumber.startsWith("+") || queryNumber.startsWith("00")) {
            // international number
            if (!queryNumber.startsWith("+86") && !queryNumber.startsWith("0086")) {
                Rlog.d(LOG_TAG, "The dial number is a international number and didn't start with +86!");
                return returnValue;
            }
        } else {
            // non-international number
            CountryDetector detector = (CountryDetector)mContext.getSystemService(Context.COUNTRY_DETECTOR);
            if ((detector != null) && (detector.detectCountry() != null))
                countryIso = detector.detectCountry().getCountryIso();
            else
                countryIso = mContext.getResources().getConfiguration().locale.getCountry();           
            if (!countryIso.equalsIgnoreCase("cn")) {
                Rlog.d(LOG_TAG, "The dial number is not at CN!");
                return returnValue;
            }
        }

        int startIndex = numberTotalLength - numberValidLength;
        int endIndex = numberTotalLength - numberTailLength;
        queryNumber = queryNumber.substring(startIndex, endIndex);
        Rlog.d(LOG_TAG, "Query number = " + queryNumber);
        int queryNumberLength = 7;

        for (int i = 0; i < queryNumberLength; i++)
        {
            if ((queryNumber.charAt(i) < '0') || (queryNumber.charAt(i) > '9')) {
               return returnValue;
            }
        }

        //Poor query performance, it takes about 300 mseconds
        //String sqlCmd = KEY_BEGIN + " <= '" + queryNumber + "' and " + KEY_END + " >= '" + queryNumber + "'";
        //Cursor cursor = mDatabase.query(TABLE_NAME, new String[] { KEY_CITY },   
        //                                   sqlCmd, null, null, null, null, null); 

        //Join query, only takes about 10 mseconds
        String sqlCmd = "Select city_name from NumberCity, city where _id = CityID and NumberHead = " + queryNumber;
        Cursor cursor = mDatabase.rawQuery(sqlCmd, null);

        if ((cursor != null) && (cursor.getCount() > 0)) {     
            cursor.moveToFirst();     
            returnValue = cursor.getString(0);
        }

        if (cursor != null) {
            cursor.close();
        }
        
        return returnValue;
    }

    private void openDatabase(Context context) {
        try {
            Rlog.d(LOG_TAG, "Open GeoCoding database.");
            if (new File(DBFilePath).exists()) {
                mDbHelper = new DBHelper(context);
                mDatabase = mDbHelper.openDatabase();
                mIsDBReady = true;
            }
            else {
                closeDatabase();
            }
        }catch (Exception e) {
            Rlog.d(LOG_TAG, "Failed to open GeoCoding database!");
            closeDatabase();
        }
    }

    private void closeDatabase() {
        try {
            if (mDbHelper != null) {
                mDbHelper.close();
            }
        } catch (Exception e) {
        }
        mDbHelper = null;
        mIsDBReady = false;
    }
    

    public class DBHelper extends SQLiteOpenHelper {
       private static final String DATABASE_NAME = DBFilePath;
       private static final int DATABASE_VERSION = 4;
       private SQLiteDatabase mDatabase;

       public DBHelper(Context context) {
         super(context, DATABASE_NAME, null, DATABASE_VERSION);
       }

       @Override
       public void onCreate(SQLiteDatabase db) {
       }

       @Override
       public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
       }
       
       public SQLiteDatabase openDatabase() throws SQLException {
          mDatabase = SQLiteDatabase.openDatabase(DATABASE_NAME, null, SQLiteDatabase.OPEN_READONLY);
          return mDatabase;
       }
    }/* End of DBHelper class */
}/* End of GeoCodingQuery class */
