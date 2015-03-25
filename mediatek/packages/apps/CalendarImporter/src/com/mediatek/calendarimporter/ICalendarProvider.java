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
 */
package com.mediatek.calendarimporter;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.provider.CalendarContract.Events;
import android.provider.OpenableColumns;

import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.vcalendar.VCalComposer;

import java.io.FileNotFoundException;
import java.io.IOException;

public class ICalendarProvider extends ContentProvider {
    private static final String TAG = "ICalendarProvider";

    private static final String VCS = ".vcs";
    private VCalComposer mComposer;

    private static final String DEFAULT_FILE_NAME = "vCalendar";

    private static final String FILENAME_REG_EXP = "[/\\\\:*?\"<>|$()~]";
    private static final int VCS_FILENAME_MAX_LENGTH = 100;//the max length should less than 255;

    @Override
    public String getType(Uri uri) {
        LogUtils.d(TAG, "getType() " + uri.toString());
        return "text/x-vcalendar";
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder) {
        LogUtils.i(TAG, "query()--->> uri=" + uri.toString());
        /*
         * if the other applications(e.g. email,gmail, Bluetooth) only want to
         * get their column fields just return what they want.
         */
        String[] curProjection = projection;
        if (curProjection == null) {
            curProjection = new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE };
            LogUtils.v(TAG, "query projection is null, create one.");
        }
        MatrixCursor cursor = new MatrixCursor(curProjection);

        createComposer(uri);
        AssetFileDescriptor descriptor = null;
        String fileName = null;
        try {
            descriptor = mComposer.getAccountsMemoryFile();
            fileName = mComposer.getMemoryFileName();
        } catch (Exception e) {
            LogUtils.e(TAG, "ICalendarProvider, query, JE happened.");
            e.printStackTrace();
            return cursor;
        }

        String title = DEFAULT_FILE_NAME; // default value
        if (fileName != null) {
            fileName = fileName.replaceAll(FILENAME_REG_EXP, "");
            int length = fileName.getBytes().length;
            if (length > VCS_FILENAME_MAX_LENGTH) {
                byte[] bytes = fileName.getBytes();
                fileName = new String(bytes, 0, VCS_FILENAME_MAX_LENGTH);
                // discard the last character in order to avoid messy code.
                fileName = fileName.substring(0, fileName.length() - 1);
                LogUtils.d(TAG, "fileName is too long, format it, fileName=" + fileName);
            }

            if (fileName.length() > 0) {
                title = fileName;
            }
        }
        LogUtils.i(TAG, "query, title = " + title);
        
        int len = curProjection.length;
        Object[] values = new Object[len];

        for (int i = 0; i < len; i++) {
            if (curProjection[i].equals(OpenableColumns.DISPLAY_NAME)) {
                values[i] = title + VCS;
            } else if (curProjection[i].equals(OpenableColumns.SIZE)) {
                values[i] = descriptor.getLength();
            } else {
                values[i] = null;
                LogUtils.e(TAG, "can not support column:" + curProjection[i]);
                continue;
            }
        }
        cursor.addRow(values);
        try {
            descriptor.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        LogUtils.i(TAG, "query(): return the Cursor.count = " + cursor.getCount());
        return cursor;
    }

    @Override
    public AssetFileDescriptor openAssetFile(Uri uri, String mode) throws FileNotFoundException {
        LogUtils.i(TAG, "openAssetFile()--->> uri=" + uri.toString());
        createComposer(uri);
        /** M: The getAccountsMemoryFile method would throw IllegalArgumentException
         *  if the file can not be found. So, we convert it to FileNotFoundException. @{ */
        AssetFileDescriptor descriptor;
        try {
            descriptor = mComposer.getAccountsMemoryFile();
        } catch (IllegalArgumentException ex) {
            LogUtils.e(TAG, "openAssetFile, getAccountsMemoryFile trrow IllegalArgumentException.");
            descriptor = null;
        }

        if (descriptor == null) {
            LogUtils.e(TAG, "openAssetFile,trrow FileNotFoundException.");
            throw new FileNotFoundException();
        }

        String fileName = mComposer.getMemoryFileName();
        LogUtils.i(TAG, "openAssetFile(): return the fileName=" + fileName + "fileLength = " + descriptor.getLength());
        /** @} */

        return descriptor;
    }

    private void createComposer(Uri eventsUri) {
        long eventId = -1;
        eventId = ContentUris.parseId(eventsUri);
        if (eventId < 0) {
            LogUtils.e(TAG, "Constructor,The given eventId is inlegal or empty, eventId :" + eventId);
            throw new IllegalArgumentException(eventsUri.toString());
        }
        String selection = "_id=" + String.valueOf(eventId) + " AND " + Events.DELETED + "!=1";
        LogUtils.i(TAG, "Constructor: the going query selection = \"" + selection + "\"");
        mComposer = new VCalComposer(getContext(), selection, null);
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        return null;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        return 0;
    }
}

