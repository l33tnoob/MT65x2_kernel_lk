/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.plugin.message;

import android.content.ContentResolver;
import android.database.Cursor;
import android.provider.MediaStore;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpVoiceMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.binder.FileStructForBinder;

import com.orangelabs.rcs.platform.AndroidFactory;

/**
 * Provide voice message
 */
public class PluginIpVoiceMessage extends IpVoiceMessage {
    private static final String TAG = "PluginIpVoiceMessage";
    private long mProgress = 0;
    private int mStatus = 0;
    private int mRcsStatus = 0;
    private String mTransferTag;
    private String mFileName;
    private static final int SIZE_SECOND = 1000;

    public PluginIpVoiceMessage(FileStructForBinder fileStruct, String remote) {
        Logger.d(TAG, "PluginIpImageMessage(), fileStruct = " + fileStruct + " remote = " + remote);
        setSimId((int) PluginUtils.DUMMY_SIM_ID);
        setSize((int) fileStruct.fileSize / PluginUtils.SIZE_K);
        setPath(fileStruct.filePath);
        mFileName = fileStruct.fileName;
        setType(IpMessageConsts.IpMessageType.VOICE);
        setFrom(remote);
        setTo(remote);
        mTransferTag = fileStruct.fileTransferTag;
        analysisAttribute();
    }

    /**
     * Analysis attribute
     */
    public void analysisAttribute() {
        Logger.d(TAG, "analysisAttribute(), mFileName = " + mFileName);
        String whereClause = MediaStore.Audio.AudioColumns.DISPLAY_NAME + "='" + mFileName + "'";
        ContentResolver cr = AndroidFactory.getApplicationContext().getContentResolver();
        Cursor cursor = null;
        int duration = 0;
        try {
            cursor = cr.query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, new String[] {
                    MediaStore.Audio.AudioColumns.DURATION}, whereClause, null, null);
            if (cursor != null && cursor.getCount() != 0) {
                cursor.moveToFirst();
                duration = cursor.getInt(cursor.getColumnIndex(MediaStore.Audio.AudioColumns.DURATION));
            } else {
                cursor = cr.query(MediaStore.Audio.Media.INTERNAL_CONTENT_URI, new String[] {
                    MediaStore.Audio.AudioColumns.DURATION
                }, whereClause, null, null);
                if (cursor != null && cursor.getCount() != 0) {
                    cursor.moveToFirst();
                    duration = cursor.getInt(cursor
                            .getColumnIndex(MediaStore.Audio.Media.DURATION));
                } else {
                    Logger.w(TAG, "analysisAttribute(), cursor is null!");
                }
            }
            Logger.d(TAG, "analysisAttribute(), duration = " + duration);
            setDuration(duration / SIZE_SECOND);
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
    }

    /**
     * Set file progress
     * 
     * @param progress
     */
    public void setProgress(long progress) {
        Logger.d(TAG, "setProgress(), progress = " + progress);
        mProgress = progress;
    }

    /**
     * Get file progress
     * 
     * @return progress
     */
    public int getProgress() {
        Logger.d(TAG, "getProgress()");
        return (int) mProgress;
    }

    /**
     * Set file status
     * 
     * @param status
     */
    public void setStatus(int status) {
        Logger.d(TAG, "SetStatus()");
        mStatus = status;
    }

    /**
     * Get file status
     * 
     * @return status
     */
    public int getStatus() {
        Logger.d(TAG, "getStatus()");
        return mStatus;
    }

    /**
     * Set rcse status
     * 
     * @param status
     */
    public void setRcsStatus(int status) {
        Logger.d(TAG, "setRcsStatus()");
        mRcsStatus = status;
    }

    /**
     * Get rcse status
     * 
     * @return status
     */
    public int getRcsStatus() {
        Logger.d(TAG, "getRcsStatus()");
        return mRcsStatus;
    }

    /**
     * Set file transfer tag
     * 
     * @param tag
     */
    public void setTag(String tag) {
        Logger.d(TAG, "setTag()");
        mTransferTag = tag;
    }

    /**
     * Get file transfer tag
     * 
     * @return tag
     */
    public String getTag() {
        Logger.d(TAG, "getTag()");
        return mTransferTag;
    }

    /**
     * Get file transfer name
     * 
     * @return name
     */
    public String getName() {
        Logger.d(TAG, "getName()");
        return mFileName;
    }
}



