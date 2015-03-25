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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.provider.MediaStore;

// This class help to generate thumbnail
public class ThumbnailBuilder {
    public static final int TYPE_NEED_LOAD = 0;
    public static final int TYPE_LOADED_NO_PREVIEW = 1;
    public static final int TYPE_LOADED_HAS_PREVIEW = 2;
    
    private static final String TAG = "ThumbnailBuilder";
    private final Bitmap mDefaultThumbnail;
    private final Bitmap mDefaultOverlay3D;
    private Bitmap mDefaultThumbnail3D = null;
    private static final int TASK_GROUP_ID = 1999;//just a number
    private final Context mContext;

    public ThumbnailBuilder(Context context) {
        mContext = context;
        // For small image, decodeResource directly will cost time less than only get option for size.
        mDefaultThumbnail = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_video_default);
        mDefaultOverlay3D = BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_three_dimen);;
    }
    
    public Bitmap getmDefaultThumbnail() {
        return mDefaultThumbnail;
    };
    
    // Generate the thumbnail with 3D
    public Bitmap getDefaultThumbnailWith3D(boolean isSupport3D) {
        Bitmap thumbnail = mDefaultThumbnail;
        if (MtkUtils.isSupport3d() && isSupport3D) {
            if (mDefaultThumbnail3D == null) {
                mDefaultThumbnail3D = mDefaultThumbnail.copy(Bitmap.Config.ARGB_8888, true);
                mDefaultThumbnail3D = overlay3DImpl(mDefaultThumbnail3D);
            }
            thumbnail = mDefaultThumbnail3D;
        } 
        return thumbnail;
    }
    
    public Bitmap overlay3DImpl(final Bitmap bitmap) {
        final Canvas overlayCanvas = new Canvas(bitmap);
        final int overlayWidth = mDefaultOverlay3D.getWidth();
        final int overlayHeight = mDefaultOverlay3D.getHeight();
        final int left = 0;//bitmap.getWidth() - overlayWidth;
        final int top = bitmap.getHeight() - overlayHeight;
        final Rect newBounds = new Rect(left, top, left + overlayWidth, top + overlayHeight);
        overlayCanvas.drawBitmap(mDefaultOverlay3D, null, newBounds, null);
        return bitmap;
    }
    
    public void recycle(ThumbnailEntity thumbnailEntity) {
        if (thumbnailEntity != null) {
            Bitmap thumbnail = thumbnailEntity.getThumbnail();
            if (thumbnail != null && thumbnail != mDefaultThumbnail && thumbnail != mDefaultThumbnail3D) {
                thumbnail.recycle();
            }
        } else {
            MtkLog.w(TAG, "recycle() thumbnailEntity is null");
        }
    }
    
    public Bitmap getThumbnailFromDb(final long id) {
        MtkLog.v(TAG, "getThumbnailFromDb() id: " + id);
        Bitmap bitmap = MediaStore.Video.Thumbnails.getThumbnail(mContext.getContentResolver(),
                id, TASK_GROUP_ID, MediaStore.Video.Thumbnails.MICRO_KIND, null);
        MtkLog.v(TAG, "getThumbnailFromDb() return " + bitmap);
        return bitmap;
    }
    
    public void cancelThumbnailFromDb() {
        MtkLog.v(TAG, "cancelThumbnailFromDb()");
        MediaStore.Video.Thumbnails.cancelThumbnailRequest(mContext.getContentResolver(), -1, TASK_GROUP_ID);
    }
}
