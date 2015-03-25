/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//MTK_OP01_PROTECT_START
package com.mediatek.videoplayer;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.format.Formatter;
import android.view.View;
import android.widget.TextView;

import com.mediatek.videoplayer.MovieListActivity.ViewHolder;

public class DetailDialog extends AlertDialog implements DialogInterface.OnClickListener {
    private static final String TAG = "DetailDialog";
    private static final int BTN_OK = DialogInterface.BUTTON_POSITIVE;
    private final Context mContext;
    private View mView;
    private TextView mTitleView;
    private TextView mTimeView;
    private TextView mPathView;
    private TextView mDurationView;
    private TextView mFileSizeView;
    private final ViewHolder mHolder;

    public DetailDialog(final Context context, final ViewHolder holder) {
        super(context);
        mContext = context;
        mHolder = holder;
        MtkLog.v(TAG, "DetailDialog() holder=" + holder);
    }

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        setTitle(R.string.media_detail);
        mView = getLayoutInflater().inflate(R.layout.detail_dialog, null);
        if (mView != null) {
            setView(mView);
            mTitleView = (TextView) mView.findViewById(R.id.title);
            mTimeView = (TextView) mView.findViewById(R.id.time);
            mDurationView = (TextView) mView.findViewById(R.id.duration);
            mPathView = (TextView) mView.findViewById(R.id.path);
            mFileSizeView = (TextView) mView.findViewById(R.id.filesize);

            mTitleView.setText(mContext.getString(R.string.detail_title, mHolder.mTitle));
            mPathView.setText(mContext.getString(R.string.detail_path, mHolder.mData));
            mDurationView.setText(mContext.getString(R.string.detail_duration,
                    MtkUtils.stringForTime(mHolder.mDuration)));
            mTimeView.setText(mContext.getString(R.string.detail_date_taken,
                    MtkUtils.localTime(mHolder.mDateTaken)));
            mFileSizeView.setText(mContext.getString(R.string.detail_filesize,
                    Formatter.formatFileSize(mContext, mHolder.mFileSize)));
            setButton(BTN_OK, mContext.getString(android.R.string.ok), this);
        } else {
            MtkLog.e(TAG, "onCreate() mView is null");
        }
        super.onCreate(savedInstanceState);
    }
    
    public void onClick(final DialogInterface dialogInterface, final int button) {
        
    }
    
}