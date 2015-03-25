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

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.widget.ImageView;

import com.mediatek.rcse.api.Logger;

public class AsyncImageView extends ImageView {

    public static final String TAG = "AsyncAvatarView";
    private boolean mIsGrey = false;
    private AvatarViewWorker mAvatarViewWorker = new AvatarViewWorker(mContext, this);

    public AsyncImageView(Context context) {
        super(context);
        PhotoLoaderManager.initialize(context);
    }

    public AsyncImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        PhotoLoaderManager.initialize(context);
    }

    public AsyncImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        PhotoLoaderManager.initialize(context);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        mAvatarViewWorker.onAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        mAvatarViewWorker.onDetachedFromWindow();
        super.onDetachedFromWindow();
    }

    public void setAsyncContact(String number, boolean isGrey) {
        mIsGrey = isGrey;
        setAsyncContact(number);
    }

    /**
     * Use the contact number to get photo uri
     * 
     * @param number The number or this contact
     */
    public void setAsyncContact(String number) {
        Logger.d(TAG, "setAsyncContact enter! number is " + number);
        if (!TextUtils.isEmpty(number)) {
            mAvatarViewWorker.setAsyncContact(number);
        } else {
            Logger.e(TAG, "setAsyncContact, number is null!");
        }
    }

    @Override
    public void setImageBitmap(Bitmap bm) {
        if (mIsGrey) {
            BitmapDrawable drawable = new BitmapDrawable(getResources(), bm);
            drawable.mutate();
            ColorMatrix cm = new ColorMatrix();
            cm.setSaturation(0);
            ColorMatrixColorFilter cf = new ColorMatrixColorFilter(cm);
            drawable.setColorFilter(cf);
            super.setImageDrawable(drawable);
        } else {
            super.setImageBitmap(bm);
        }
    }

    @Override
    public void setImageResource(int resId) {
        if (mIsGrey) {
            Drawable drawable = mContext.getResources().getDrawable(resId);
            drawable.mutate();
            ColorMatrix cm = new ColorMatrix();
            cm.setSaturation(0);
            ColorMatrixColorFilter cf = new ColorMatrixColorFilter(cm);
            drawable.setColorFilter(cf);
            super.setImageDrawable(drawable);
        } else {
            super.setImageResource(resId);
        }
    }

}
