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

import java.util.List;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.webkit.MimeTypeMap;
import com.mediatek.rcse.service.Utils;
import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.orangelabs.rcs.R;

/**
 * Provide attach message
 */
public class PluginIpAttachMessage extends IpAttachMessage {
    private static final String TAG = "PluginIpAttachMessage";
    private long mProgress = 0;
    private int mStatus = 0;
    private int mRcsStatus = 0;
    private String mTransferTag;
    private String mFileName;
    private String mFilePath;

    public PluginIpAttachMessage(FileStructForBinder fileStruct, String remote) {
        Logger
                .d(TAG, "PluginIpAttachMessage(), fileStruct = " + fileStruct + " remote = "
                        + remote);
        setSimId((int) PluginUtils.DUMMY_SIM_ID);
        setSize((int) fileStruct.fileSize / PluginUtils.SIZE_K);
        setPath(fileStruct.filePath);
        mFilePath = fileStruct.filePath;
        setType(IpMessageConsts.IpMessageType.UNKNOWN_FILE);
        setFrom(remote);
        setTo(remote);
        mFileName = fileStruct.fileName;
        mTransferTag = fileStruct.fileTransferTag;
    }

    public Drawable getFileTypeIcon()
    {
    	Context context = ApiManager.getInstance().getContext();
    	String mimeType = MediaFile.getMimeTypeForFile(mFileName);
    	if (mimeType == null) {
    	mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
    	Utils.getFileExtension(mFileName));
    	}

    	Intent intent = new Intent(Intent.ACTION_VIEW);
    	intent.setDataAndType(Utils.getFileNameUri(mFileName), mimeType);
    	PackageManager packageManager = context.getPackageManager();
    	List<ResolveInfo> list = packageManager.queryIntentActivities(intent,
    	PackageManager.MATCH_DEFAULT_ONLY);
    	int size = list.size();
    	Drawable drawable = context.getResources()
    	.getDrawable(R.drawable.rcs_ic_ft_default_preview);
    	if (size > 0) {
    	drawable = list.get(0).activityInfo.loadIcon(packageManager);
    	}
     return drawable;
    }
    /**
     * get name
     * @return file name
     */
    public String getName() {
        Logger.d(TAG, "getName()");
        return mFileName;
    }

    /**
     * Set file progress
     * @param progress
     */
    public void setProgress(long progress) {
        Logger.d(TAG, "setProgress(), progress = " + progress);
        mProgress = progress;
    }

    /**
     * Get file progress
     * @return file name
     */
    public int getProgress() {
        Logger.d(TAG, "getProgress()");
        return (int) mProgress;
    }

    /**
     * Set file status
     * @param status
     */
    public void setStatus(int status) {
        Logger.d(TAG, "SetStatus()");
        mStatus = status;
    }

    /**
     * Get file status
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
     * @param tag
     */
    public void setTag(String tag) {
        Logger.d(TAG, "setTag()");
        mTransferTag = tag;
    }

    /**
     * Get file transfer tag
     * @return status
     */
    public String getTag() {
        Logger.d(TAG, "getTag()");
        return mTransferTag;
    }
}
