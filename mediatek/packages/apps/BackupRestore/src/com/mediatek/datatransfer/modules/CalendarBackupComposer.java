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
import android.provider.CalendarContract;

import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.SDCardUtils;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.vcalendar.VCalComposer;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Describe class <code>CalendarBackupComposer</code> here.
 *
 * @author 
 * @version 1.0
 */
public class CalendarBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/CalendarBackupComposer";
    private static final Uri calanderEventURI = CalendarContract.Events.CONTENT_URI;
    private Cursor mCursor;

    private VCalComposer mVCalComposer;
    BufferedWriter mOut;

    /**
     * Creates a new <code>CalendarBackupComposer</code> instance.
     *
     * @param context a <code>Context</code> value
     */
    public CalendarBackupComposer(Context context) {
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
        int count = 0;

        if (mCursor != null&& !mCursor.isClosed()) {
            count = mCursor.getCount();
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    /**
     * Describe <code>init</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean init() {
        boolean result = true;

        mCursor = mContext.getContentResolver().query(calanderEventURI, null, null, null, null);

        if (mCursor != null) {
            mCursor.moveToFirst();
            mVCalComposer = new VCalComposer(mContext);
        } else {
            result = false;
        }

        MyLogger.logD(CLASS_TAG, "init(),result:" + result + ", count:"
                + (mCursor != null ? mCursor.getCount() : 0));
        return result;
    }

    /**
     * Describe <code>isAfterLast</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean isAfterLast() {
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
    public boolean implementComposeOneEntity() {
        boolean result = false;

        if (mCursor != null && !mCursor.isAfterLast()) {
            int id = mCursor.getInt(mCursor.getColumnIndex("_id"));
            MyLogger.logD(CLASS_TAG, "implementComposeOneEntity id:" + id);
            mCursor.moveToNext();

            if(mVCalComposer != null && mOut != null) {
                try {
                    String vcal = mVCalComposer.buildVEventString(id);
                    if(vcal != null) {
                        mOut.write(vcal);
                        result = true;
                    }
                } catch(Exception e) {
                    MyLogger.logD(CLASS_TAG, "VCAL: implementComposeOneEntity() write file failed");
                }
            }
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public final void onStart() {
        super.onStart();
        if(mVCalComposer != null) {
            if(getCount() > 0) {
                File path = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_CALENDAR);
                if (!path.exists()) {
                    path.mkdirs();
                }

                File file = new File(path.getAbsolutePath() + File.separator + ModulePath.NAME_CALENDAR);
                if (!file.exists()) {
                    try {
                        file.createNewFile();
                    } catch (Exception e) {
                        MyLogger.logE(CLASS_TAG, "onStart():create file failed");
                    }
                }

                try {
                    FileWriter fstream = new FileWriter(file);
                    mOut = new BufferedWriter(fstream);
                    String vcal = mVCalComposer.getVCalHead();
                    if(vcal != null) {
                        mOut.write(vcal);
                    }
                } catch(Exception e) {
                    MyLogger.logD(CLASS_TAG, "VCAL: onStart() write file failed");
                }
            }
        }
    }

    /**
     * Describe <code>onEnd</code> method here.
     *
     */
    public void onEnd() {
        super.onEnd();
        if (mCursor != null) {
            mCursor.close();
            mCursor = null;
        }

        MyLogger.logD(CLASS_TAG, "onEnd");
        if(mVCalComposer != null) {
            if(mOut != null) {
                try {
                    String vcal = mVCalComposer.getVCalEnd();
                    if(vcal != null) {
                        mOut.write(vcal);
                    }
                } catch(Exception e) {
                    MyLogger.logD(CLASS_TAG, "VCAL: onEnd() write file failed");
                } finally {
                    try {
                        mOut.close();
                    } catch(IOException e) {
                    }
                }
            }
        }
    }
}
