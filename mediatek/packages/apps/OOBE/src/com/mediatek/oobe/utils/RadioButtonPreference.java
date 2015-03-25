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

package com.mediatek.oobe.utils;

import android.content.Context;
import android.preference.Preference;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RadioButton;

import com.mediatek.oobe.R;

public class RadioButtonPreference extends Preference {
    private static final String TAG = "OOBE";
    private String mTitle;
    private String mSummary;
    private RadioButton mRadioButton;
    private boolean mIsChecked;
    private String mNote;

    /**
     * RadioButtonPreference constructor
     * @param context Context
     */
    public RadioButtonPreference(Context context) {
        this(context, "title", "summary");
    }

    /**
     * RadioButtonPreference constructor
     * @param context Context
     * @param attrs AttributeSet
     */
    public RadioButtonPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_radiobutton);
    }

    /**
     * RadioButtonPreference constructor
     * @param context Context
     * @param title String
     * @param summary String
     */
    public RadioButtonPreference(Context context, String title, String summary) {
        this(context, title, summary, false);
    }

    /**
     * RadioButtonPreference
     * @param context Context
     * @param title String
     * @param summary String
     * @param isChecked boolean
     */
    public RadioButtonPreference(Context context, String title, String summary, boolean isChecked) {
        super(context);
        mTitle = title;
        mSummary = summary;
        mIsChecked = isChecked;
        setLayoutResource(R.layout.preference_radiobutton);

        if (!TextUtils.isEmpty(mTitle)) {
            setTitle(mTitle);
        }
        if (mSummary != null) {
            setSummary(mSummary);
        }
    }

    @Override
    protected void onBindView(View view) {
        // Xlog.i(TAG, "onBindView() is called, title="+mTitle+", summary="+mSummary);
        super.onBindView(view);
        mRadioButton = (RadioButton) view.findViewById(R.id.radiobutton);
        mRadioButton.setChecked(mIsChecked);
    }

    /**
     * set radio button Checked
     * @param newCheckStatus boolean
     */
    public void setChecked(boolean newCheckStatus) {
        mIsChecked = newCheckStatus;
        notifyChanged();
    }

    /**
     * set note string
     * @param note String
     */
    public void setNote(String note) {
        this.mNote = note;
    }

    /**
     * get note string
     * @return a string
     */
    public String getNote() {
        return mNote;
    }
}
