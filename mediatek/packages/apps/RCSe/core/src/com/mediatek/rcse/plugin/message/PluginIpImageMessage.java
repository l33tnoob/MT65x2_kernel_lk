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
import android.provider.MediaStore.Images.Thumbnails;

import com.mediatek.mms.ipmessage.message.IpImageMessage;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.binder.FileStructForBinder;

import com.orangelabs.rcs.platform.AndroidFactory;

/**
 * Provide text message
 */
public class PluginIpImageMessage extends IpImageMessage {
    private static final String TAG = "PluginIpImageMessage";
    private long mProgress = 0;
    private int mStatus = 0;
    private int mRcsStatus = 0;
    private String mTransferTag;
    private String mFileName;
    private String thumbnail = null;

    public PluginIpImageMessage(FileStructForBinder fileStruct, String remote) {
        Logger.d(TAG, "PluginIpImageMessage(), fileStruct = " + fileStruct + " remote = " + remote);
        setSimId((int) PluginUtils.DUMMY_SIM_ID);
        setPath(fileStruct.filePath);
        setSize((int) (fileStruct.fileSize / PluginUtils.SIZE_K));
        mFileName = fileStruct.fileName;
        setType(IpMessageConsts.IpMessageType.PICTURE);
        setFrom(remote);
        setTo(remote);
        thumbnail = fileStruct.thumbnail;
        mTransferTag = fileStruct.fileTransferTag;
        analysisAttribute();
    }

    private void analysisAttribute() {
        long imageId = -1;
        String whereClause = MediaStore.Images.Media.DISPLAY_NAME + "='" + mFileName
                + "'";
        ContentResolver cr = AndroidFactory.getApplicationContext().getContentResolver();
        Cursor cursor = null;
        try {
            cursor = cr.query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, new String[] {
                    MediaStore.Images.Media._ID, MediaStore.Images.Media.WIDTH,
                    MediaStore.Images.Media.HEIGHT
            }, whereClause, null, null);
            if (cursor != null && cursor.getCount() != 0) {
                cursor.moveToFirst();
                imageId = cursor.getLong(cursor.getColumnIndex(MediaStore.Images.Media._ID));
                int width = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.Media.WIDTH));
                int hight = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.Media.HEIGHT));
                setWidthInPixel(width);
                setHeightInPixel(hight);
            } else {
                Logger.w(TAG, "analysisAttribute(), cursor is null!");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        if (imageId != -1) {
            whereClause = Thumbnails.IMAGE_ID + "='" + imageId + "'";
            try {
                cursor = cr.query(Thumbnails.EXTERNAL_CONTENT_URI, new String[] {
                    Thumbnails.DATA
                }, whereClause, null, null);
                if (cursor != null && cursor.getCount() != 0) {
                    cursor.moveToFirst();
                    String thumbnailPath = cursor.getString(cursor.getColumnIndex(Thumbnails.DATA));
                    if(thumbnailPath == null || thumbnailPath.equals(""))
                    	thumbnailPath = thumbnail;
                    setThumbPath(thumbnailPath);
                } else {
                    Logger.w(TAG, "analysisAttribute(), cursor is null!");
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
        } else {
        	if(thumbnail != null)
        	setThumbPath(thumbnail);
            Logger.w(TAG, "analysisAttribute(), have no this image!");
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
        super.setStatus(status);
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
