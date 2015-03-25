/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.videoplayer;

import android.graphics.Bitmap;

// The thumbnail wrap class, save info, one ThumbnailEntiry for one thumbnail
public class ThumbnailEntity {
    private static final String TAG = "ThumbnailEntity";
    private long mDateModified;
    private int mType;
    private Bitmap mThumbnail;
    private boolean mIsSupport3D;

    public ThumbnailEntity(final Bitmap thumbnail, final int type, final long dateModified, final boolean isSupport3D) {
        mType = type;
        mThumbnail = thumbnail;
        mDateModified = dateModified;
        mIsSupport3D = isSupport3D;
    }

    public long getDateModified() {
        return mDateModified;
    }
    
    public int getType() {
        return mType;
    }
    
    public Bitmap getThumbnail() {
        return mThumbnail;
    }
    
    public boolean isSupport3D() {
        return mIsSupport3D;
    }
    
    public void setDateModified(long dateModified) {
        mDateModified = dateModified;
    }
    
    public void setType(int type) {
        mType = type;
    }
    
    public void setThumbnail(Bitmap bitmap) {
        mThumbnail = bitmap;
    }
    
    public void setSupport3D(boolean support) {
        mIsSupport3D = support;
    }

    @Override
    public String toString() {
        return new StringBuilder()
        .append("ThumbnailEntity(mType=")
        .append(mType)
        .append(", mThumbnail=")
        .append(mThumbnail)
        .append(", mDateModified=")
        .append(mDateModified)
        .append(", mIsSupport3D")
        .append(mIsSupport3D)
        .append(")")
        .toString();
    }
}