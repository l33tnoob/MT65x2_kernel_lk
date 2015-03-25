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

package com.mediatek.rcse.service.binder;

import android.os.Parcel;
import android.os.Parcelable;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;

import java.util.Date;

/**
 * This class is for IPC FileStruct
 */
public class FileStructForBinder implements Parcelable {

    private static final String TAG = "FileStructForBinder";

    public String filePath = null;

    public String thumbnail = null;

    public String fileName = null;

    public long fileSize = -1;

    public String fileTransferTag = null;

    public Date date;

    public String toString() {
        return TAG + "file path is " + filePath + " file name is " + fileName + " size is " + fileSize
                + " FileTransferTag is " + fileTransferTag + " date is " + date;
    }

    public FileStructForBinder(Parcel source) {
        Logger.d(TAG, "FileStructForBinder() entry! source = " + source);
        filePath = source.readString();
        fileName = source.readString();
        fileSize = source.readLong();
        fileTransferTag = source.readString();
        thumbnail = source.readString();
        date = new Date(source.readLong());
        Logger.d(TAG, "readfromparcel(), mFilePath = " + filePath + " mName = " + fileName + " mSize = " + fileSize
                + " mFileTransferTag = " + fileTransferTag + " mDate = " + date + "thumbnial =" + thumbnail);
              
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        Logger.d(TAG, "writeToParcel(), mFilePath = " + filePath + " mName = " + fileName + " mSize = " + fileSize
                + " mFileTransferTag = " + fileTransferTag + " mDate = " + date + "thumbnial =" + thumbnail);
        dest.writeString(filePath);
        dest.writeString(fileName);
        dest.writeLong(fileSize);
        dest.writeString(fileTransferTag);
        dest.writeString(thumbnail);       
        dest.writeLong(date.getTime());
    }

    public FileStructForBinder(FileStruct fileStruct) {
        Logger.d(TAG, "FileStructForBinder() entry! fileStruct = " + fileStruct);
        filePath = fileStruct.mFilePath;
        fileName = fileStruct.mName;
        fileSize = fileStruct.mSize;
        TagTranslater.saveTag(fileStruct.mFileTransferTag);
        fileTransferTag = fileStruct.mFileTransferTag.toString();
        date = fileStruct.mDate;
        thumbnail = fileStruct.mThumbnail;
        if(thumbnail == null)
        	thumbnail = "";
        if(filePath == null)
        	filePath = "";
    }

    public FileStructForBinder(String path, String name, long size, Object tag, Date fileDate) {
        filePath = path;
        fileName = name;
        fileSize = size;
        TagTranslater.saveTag(tag);
        fileTransferTag = tag.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }
    
    /**
     * Parcelable creator
     */
    public static final Parcelable.Creator<FileStructForBinder> CREATOR = new Parcelable.Creator<FileStructForBinder>() {
        public FileStructForBinder createFromParcel(Parcel source) {
            return new FileStructForBinder(source);
        }

        public FileStructForBinder[] newArray(int size) {
            return new FileStructForBinder[size];
        }
    };

}
