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

package com.mediatek.datatransfer.modules;

import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;

import com.mediatek.datatransfer.utils.BackupZip;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.vcalendar.VCalParser;
import com.mediatek.vcalendar.VCalStatusChangeOperator;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class CalendarRestoreComposer extends Composer implements VCalStatusChangeOperator {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/CalendarRestoreComposer";
    private static final String COLUMN_ID = "_id";
    private static final Uri calanderEventURI = CalendarContract.Events.CONTENT_URI;
    private int mIndex;

    VCalParser mCalParser;
    private int mCount;
    private Object mLock = new Object();
    //private boolean mReceiveStarted;
    private boolean mReceiveFinished;

    /**
     * Creates a new <code>CalendarRestoreComposer</code> instance.
     *
     * @param context a <code>Context</code> value
     */
    public CalendarRestoreComposer(Context context) {
        super(context);
    }


    /**
     * Describe <code>getModuleType</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getModuleType() {
        return ModuleType.TYPE_CALENDAR;
    }

    /**
     * Describe <code>getCount</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getCount() {
        MyLogger.logD(CLASS_TAG, "getCount():" + mCount);
        return mCount;
    }

    /**
     * Describe <code>init</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean init() {
        boolean result = false;
        String fileName = mParentFolderPath + File.separator + ModulePath.FOLDER_CALENDAR
                + File.separator + ModulePath.NAME_CALENDAR;
        File file = new File(fileName);
        if (file.exists() && file.isFile()) {
            mCount = getCalendarEventNum(fileName);
            if (fileName.contains("#")) {
                return false;
            }
            mCalParser = new VCalParser(Uri.parse("file://" + fileName), mContext, this);
            mIndex = 0;
            mReceiveFinished = false;
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result);
        return result;
    }
    
    private int getCalendarEventNum(String fileName) {
        int calEventNum = 0;
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(fileName));
            String str = null;
            while ((str = reader.readLine()) != null) {
                if (str.contains("END:VEVENT")) {
                    ++calEventNum;
                }
            }
        } catch (IOException e) {
            MyLogger.logE(CLASS_TAG, "getCalendarEventNum read file failed");
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    MyLogger.logE(CLASS_TAG, "getCalendarEventNum close reader failed");
                }
            }
        }

        return (calEventNum == 0) ? -1 : calEventNum;
    }
    
    /**
     * Describe <code>isAfterLast</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean isAfterLast() {
        boolean result = false;
        if (mCount > -1) {
            result = (mIndex >= mCount) ? true : false;
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    /**
     * Describe <code>composeOneEntity</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public final boolean composeOneEntity() {
        return implementComposeOneEntity();
    }


    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean implementComposeOneEntity() {
        MyLogger.logD(CLASS_TAG, "implementComposeOneEntity():" + mIndex++);
        return true;
    }

    /**
     * Describe <code>deleteAllCalendarEvents</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    private boolean deleteAllCalendarEvents() {
        boolean result = false;
        int count = 0;
        Cursor cur = mContext.getContentResolver().query(calanderEventURI, null, null, null, null);
        if (cur != null && cur.moveToFirst()) {
            int[] cId = new int[cur.getCount()];
            int i = 0;
            while (!cur.isAfterLast()) {
                cId[i++] = cur.getInt(cur.getColumnIndex(COLUMN_ID));
                cur.moveToNext();
            }

            for (int j = 0; j < cId.length; ++j) {
                Uri uri = ContentUris.withAppendedId(calanderEventURI, cId[j]);
                count += mContext.getContentResolver().delete(uri, null, null);
            }

            cur.close();
            result = true;
        }

        MyLogger.logD(CLASS_TAG, "deleteAllCalendarEvents()result:" + result + ", " + count
                + " events deleted!");
        return result;
    }

    
    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public void onStart() {
        //super.onStart();
        deleteAllCalendarEvents();
        if(mCalParser != null) {
            mCalParser.startParse();
        } else {
            super.onStart();
        }

        MyLogger.logD(CLASS_TAG, "onStart()");
    }

    
    /**
     * Describe <code>onEnd</code> method here.
     *
     */
    public void onEnd() {
        if(mCalParser != null) {
            if(!mReceiveFinished) {
                synchronized (mLock) {
                    try {
                        mLock.wait();
                    } catch(InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }

            mCalParser.close();
        }

        super.onEnd();
        MyLogger.logD(CLASS_TAG, "onEnd()");
    }

    /**
     * Will be called when compose/parse started.
     */
    public void vCalOperationStarted(int totalCnt) {
        mCount = totalCnt;
        super.onStart();
        if(totalCnt == 0) {
            synchronized (mLock) {
                mReceiveFinished = true;
                mLock.notify();
            }
        }

        MyLogger.logD(CLASS_TAG, "vCalOperationStarted():" + totalCnt);
    }

    /**
     * Will be called when the compose/parse operation finished
     * 
     * @param successCnt
     *            the successful handled count
     * @param totalCnt
     *            total count
     */
    public void vCalOperationFinished(int successCnt, int totalCnt, Object obj) {

        synchronized (mLock) {
            mReceiveFinished = true;
            mLock.notify();
        }

        MyLogger.logD(CLASS_TAG, "vCalOperationFinished():successCnt:" + successCnt + ",totalCnt:" + totalCnt);
    }

    /**
     * Will be called when the process status update
     * 
     * @param currentCnt
     *            current handled count
     * @param totalCnt
     *            total count
     */
    public void vCalProcessStatusUpdate(int currentCnt, int totalCnt) {
        //mIndex = currentCnt - 1;
        mCount = totalCnt;
        synchronized (mLock) {
            increaseComposed(true);
            mLock.notify();
        }
        MyLogger.logD(CLASS_TAG, "vCalProcessStatusUpdate():currentCnt:" + currentCnt + ",totalCnt:" + totalCnt);
    }

    /**
     * Will be called when the cancel request has been finished.
     * 
     * @param finishedCnt
     *            the count has been finished before the cancel operation
     * @param totalCnt
     *            total count
     */
    public void vCalOperationCanceled(int finishedCnt, int totalCnt) {
    }

    /**
     * Will be called when exception occurred.
     * 
     * @param finishedCnt
     *            the count has been finished before the exception occurred.
     * @param totalCnt
     *            total count
     * @param type
     *            the exception type.
     */
    public void vCalOperationExceptionOccured(int finishedCnt, int totalCnt, int type) {
    }
}

