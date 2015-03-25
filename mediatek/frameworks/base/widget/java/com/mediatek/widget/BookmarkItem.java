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

package com.mediatek.widget;

import android.graphics.Bitmap;

/**
 * New added class for new common control BookmarkView.
 *
 * Bookmark item is the only recognized data structure for BookmarkAdapter.
 *
 * @hide
 */
public class BookmarkItem {
    // The content image of bookmark item could be either a resource id or a bitmap.
    int mResId = -1;
    Bitmap mContent;

    String mTitle;
    String mInfo;
    
    public BookmarkItem(int resId, String title, String info) {
        this(resId, null, title, info);
    }
    
    public BookmarkItem(Bitmap content, String title, String info) {
        this(-1, content, title, info);
    }

    public BookmarkItem(final BookmarkItem item) {
        this(item.mResId, item.mContent, item.mTitle, item.mInfo);
    }
    
    public BookmarkItem(int resId, Bitmap content, String title, String info) {
        mResId = resId;
        mContent = content;
        mTitle = title;
        mInfo = info;
    }
    
    /**
     * Set bookmark item content bitmap.
     * 
     * @param bmp
     *
     * @internal
     */
    public void setContentBitmap(final Bitmap bmp) {
        mContent = bmp;
    }
    
    /**
     * Set bookmark item content bitmap resource.
     * 
     * @param resourceId
     */
    public void setBitmapResource(final int resourceId) {
        mResId = resourceId;
    }
    
    /**
     * Set bookmark item title string.
     * 
     * @param titleString
     *
     * @internal
     */
    public void setTitleString(String titleString) {
        mTitle = titleString;
    }
    
    /**
     * Set bookmark item detail information string.
     * 
     * @param infoString
     */
    public void setInfoString(String infoString) {
        mInfo = infoString;
    }
    
    /**
     * Get bookmark item content bitmap.
     * 
     * @return
     *
     * @internal
     */
    public Bitmap getContentBitmap() {
        return mContent;
    }
    
    /**
     * Get bookmark item title string.
     * 
     * @return
     */
    public String getTitleString() {
        return mTitle;
    }
    
    /**
     * Get bookmark item detail information string.
     * 
     * @return
     */
    public String getInfoString() {
        return mInfo;
    }
}
