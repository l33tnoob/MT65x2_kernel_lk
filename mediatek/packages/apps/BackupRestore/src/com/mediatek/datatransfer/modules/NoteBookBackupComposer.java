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

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Describe class <code>NoteBookBackupComposer</code> here.
 * 
 * @author
 * @version 1.0
 */
public class NoteBookBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/NoteBookBackupComposer";
    private static final String COLUMN_NAME_TITLE = "title";
    private static final String COLUMN_NAME_NOTE = "note";
    private static final String COLUMN_NAME_CREATED = "created";
    private static final String COLUMN_NAME_MODIFIED = "modified";
    private static final String COLUMN_NAME_GROUP = "notegroup";

    private Cursor mCursor;
    private NoteBookXmlComposer mXmlComposer;

    public NoteBookBackupComposer(Context context) {
        super(context);
    }

    /**
     * Describe <code>init</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean init() {
        boolean result = false;
        Uri uri = Uri.parse(Constants.URI_NOTEBOOK);
        mCursor = mContext.getContentResolver().query(uri, null, null, null, null);
        if (mCursor != null) {
            mCursor.moveToFirst();
            result = true;
        }

        MyLogger.logD(CLASS_TAG,
                "init():" + result + ",count::" + (mCursor != null ? mCursor.getCount() : 0));
        return result;
    }

    /**
     * Describe <code>getModuleType</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getModuleType() {
        return ModuleType.TYPE_NOTEBOOK;
    }

    /**
     * Describe <code>getCount</code> method here.
     * 
     * @return an <code>int</code> value
     */
    public final int getCount() {
        int count = 0;
        if (mCursor != null&& !mCursor.isClosed()) {
            count = mCursor.getCount();
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    /**
     * Describe <code>isAfterLast</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean isAfterLast() {
        boolean result = true;
        if (mCursor != null) {
            result = mCursor.isAfterLast();
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     * 
     * @return a <code>boolean</code> value
     */
    public final boolean implementComposeOneEntity() {
        boolean result = false;
        if (mCursor != null && !mCursor.isAfterLast()) {
            if (mXmlComposer != null) {
                try {
                    String title = mCursor.getString(mCursor
                            .getColumnIndexOrThrow(COLUMN_NAME_TITLE));
                    String note = mCursor
                            .getString(mCursor.getColumnIndexOrThrow(COLUMN_NAME_NOTE));
                    String created = mCursor.getString(mCursor
                            .getColumnIndexOrThrow(COLUMN_NAME_CREATED));
                    String modified = mCursor.getString(mCursor
                            .getColumnIndexOrThrow(COLUMN_NAME_MODIFIED));
                    String notegroup = mCursor.getString(mCursor
                            .getColumnIndexOrThrow(COLUMN_NAME_GROUP));
                    NoteBookXmlInfo record = new NoteBookXmlInfo(title, note, created, modified,
                            notegroup);
                    mXmlComposer.addOneMmsRecord(record);
                    result = true;
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                }
            }

            mCursor.moveToNext();
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     * 
     */
    public void onStart() {
        super.onStart();
        if(getCount() > 0) {
            File path = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_NOTEBOOK);
            if (path.exists()) {
                File[] files = path.listFiles();
                for(File file : files) {
                    if (file.isFile()) {
                        file.delete();
                    }
                }
            } else {
                path.mkdirs();
            }

            if ((mXmlComposer = new NoteBookXmlComposer()) != null) {
                mXmlComposer.startCompose();
            }
        }
    }

    /**
     * Describe <code>onEnd</code> method here.
     * 
     */
    public void onEnd() {
        super.onEnd();
        if (mXmlComposer != null) {
            mXmlComposer.endCompose();
            String tmpXmlInfo = mXmlComposer.getXmlInfo();
            if (getComposed() > 0 && tmpXmlInfo != null) {
                try {
                    writeToFile(tmpXmlInfo);
                } catch (IOException e) {
                    if (super.mReporter != null) {
                        super.mReporter.onErr(e);
                    }
                    e.printStackTrace();
                }
            }
        }

        if (mCursor != null) {
            mCursor.close();
            mCursor = null;
        }
    }

    private void writeToFile(String inBuf) throws IOException {
        try {
            FileOutputStream outStream = new FileOutputStream(mParentFolderPath + File.separator
                    + ModulePath.FOLDER_NOTEBOOK + File.separator + ModulePath.NOTEBOOK_XML);
            byte[] buf = inBuf.getBytes();
            outStream.write(buf, 0, buf.length);
            outStream.flush();
            outStream.close();
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
