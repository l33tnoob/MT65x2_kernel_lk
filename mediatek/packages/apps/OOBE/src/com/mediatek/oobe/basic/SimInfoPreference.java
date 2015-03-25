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

package com.mediatek.oobe.basic;

import android.content.Context;
import android.preference.Preference;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

public class SimInfoPreference extends Preference {

    private int mStatus;

    private String mSimNum;


    private boolean mChecked = true;
    private static final int DISPLAY_NONE = 0;
    private static final int DISPLAY_FIRST_FOUR = 1;
    private static final int DISPLAY_LAST_FOUR = 2;
    private static final String TAG = "SimInfoPreference";

    private Context mContext;
    private int mProgressCurrNum = 0;
    private int mProgressTotalCount = 0;
    private String mTextImportingProgress = "";
    // flag for contact import progress
    private boolean mIsImporting = false;
    private boolean mFinishImporting = false;
    private SimInfoRecord mSimInfo;

    /**
     * SimInfoPreference
     * @param context Context
     * @param simInfo sim info of the sim card
     * @param status status of the sim card
     */
    public SimInfoPreference(Context context, SimInfoRecord simInfo, int status) {
        super(context, null);

        mSimNum = simInfo.mNumber;
        mStatus = status;
        mContext = context;
        mSimInfo = simInfo;

        setLayoutResource(R.layout.preference_contact_sim_info);

        if (mSimInfo.mDisplayName != null) {
            setTitle(mSimInfo.mDisplayName);
        }
        if ((mSimNum != null) && (mSimNum.length() != 0)) {
            setSummary(mSimNum);
        }
    }

    @Override
    public View getView(View convertView, ViewGroup parent) {
        View view = super.getView(convertView, parent);

        TextView textTitle = (TextView) view.findViewById(android.R.id.title);

        if ((textTitle != null)) {
            textTitle.setText(mSimInfo.mDisplayName);
        }

        TextView textNum = (TextView) view.findViewById(android.R.id.summary);
        if (textNum != null) {
            if ((mSimNum != null) && (mSimNum.length() != 0)) {
                textNum.setText(mSimNum);

            } else {

                textNum.setVisibility(View.GONE);
            }
        }

        ImageView imageStatus = (ImageView) view.findViewById(R.id.simStatus);

        if (imageStatus != null) {
            int res = Utils.getStatusResource(mStatus);

            if (res == -1) {
                imageStatus.setVisibility(View.GONE);
            } else {
                imageStatus.setImageResource(res);
            }
        }

        RelativeLayout viewSim = (RelativeLayout) view.findViewById(R.id.simIcon);

        if (viewSim != null) {

            int res = Utils.getSimColorResource(mSimInfo.mColor);

            if (res < 0) {
                viewSim.setBackgroundDrawable(null);
            } else {
                viewSim.setBackgroundResource(res);
            }

        }

        // checkbox
        CheckBox ckRadioOn = (CheckBox) view.findViewById(R.id.Check_Enable);
        if (ckRadioOn != null) {
            ckRadioOn.setChecked(mChecked);
            ckRadioOn.setVisibility((mFinishImporting || mIsImporting) ? View.GONE : View.VISIBLE);
            Xlog.i(TAG, "ckRadioOn.setVisibility " + !mIsImporting);
        }

        final int formatNum = 4;
        TextView textNumForShort = (TextView) view.findViewById(R.id.simNum);
        if ((textNum != null) && (mSimNum != null)) {

            switch (mSimInfo.mDispalyNumberFormat) {
            case DISPLAY_NONE: 
                textNumForShort.setVisibility(View.GONE);
                break;
            case DISPLAY_FIRST_FOUR: 
                if (mSimNum.length() >= formatNum) {
                    textNumForShort.setText(mSimNum.substring(0, formatNum));
                } else {
                    textNumForShort.setText(mSimNum);
                }
                break;
            case DISPLAY_LAST_FOUR: 
                if (mSimNum.length() >= formatNum) {
                    textNumForShort.setText(mSimNum.substring(mSimNum.length() - formatNum));
                } else {
                    textNumForShort.setText(mSimNum);
                }
                break;
            default:
                break;

            }
        }


        ImageView importingProgressFlag = (ImageView) view.findViewById(R.id.importing_completed);
        importingProgressFlag.setVisibility(mFinishImporting ? View.VISIBLE : View.GONE);

        TextView summaryView = (TextView) view.findViewById(android.R.id.summary);
        String summaryStr = (summaryView == null ? "" : summaryView.getText().toString());
        if (summaryView != null) {
                summaryView.setVisibility(((mIsImporting && !mFinishImporting) || TextUtils.isEmpty(summaryStr)) ? View.GONE
                        : View.VISIBLE);
        }

        ProgressBar importingProgressBar = (ProgressBar) view.findViewById(R.id.progress_importing);
        TextView importingProgressTextView = (TextView) view.findViewById(R.id.textView_importing);
        if (mIsImporting) {
            importingProgressBar.setVisibility(View.VISIBLE);
            importingProgressBar.setMax(mProgressTotalCount);
            importingProgressBar.setProgress(mProgressCurrNum);
            Xlog.i(TAG, "importingProgressBar.setVisibility");
            importingProgressTextView.setVisibility(View.VISIBLE);
            String text = String.format(mContext.getResources().getString(R.string.oobe_note_progress_copy_contacts),
                mProgressCurrNum, mProgressTotalCount);
            importingProgressTextView.setText(text);
        } else {
            importingProgressBar.setVisibility(View.GONE);
            Xlog.i(TAG, "importingProgressBar.setVisibility : GONE");
            importingProgressTextView.setVisibility(View.GONE);
        }

        return view;
    }

    public void setCheck(boolean bCheck) {
        mChecked = bCheck;
        notifyChanged();
    }

    public boolean isChecked() {
        return mChecked;

    }

    public void setStatus(int status) {
        mStatus = status;
        notifyChanged();
    }

    /**
     * get slot index
     * 
     * @return the slot index
     */
    public int getSlotIndex() {
        return mSimInfo.mSimSlotId;
    }

    public long getSimId() {
        return mSimInfo.mSimInfoId;
    }

    /**
     * get the status of importing status
     * 
     * @return boolean true or false
     */
    public boolean isImporting() {
        return mIsImporting;
    }

    /**
     * set importing status
     * 
     * @param isImporting
     *            true/false
     */
    public void setImporting(boolean isImporting) {
        this.mIsImporting = isImporting;
        notifyChanged();
    }

    /**
     * is finish importing status
     * 
     * @return true or false
     */
    public boolean isFinishImporting() {
        return mFinishImporting;
    }

    /**
     * set finish importing
     * 
     * @param finish
     *            the status of importing
     */
    public void setFinishImporting(boolean finish) {
        this.mFinishImporting = finish;
        notifyChanged();
    }

    /**
     * init progress bar
     * 
     * @param totalCount
     *            int
     */
    public void initProgressBar(int totalCount) {
        if (mProgressTotalCount == 0) {
            mTextImportingProgress = mContext.getResources().getString(R.string.oobe_note_copy_contacts_waiting);
            mProgressTotalCount = totalCount;
            notifyChanged();
        }
    }

    /**
     * increment progress
     * 
     * @param newProgress
     *            int
     */
    public void updateProgressBar(int newProgress) {
        mIsImporting = true;
        mFinishImporting = false;
        mProgressCurrNum = newProgress;
        mTextImportingProgress = String.format(mContext.getResources().getString(R.string.oobe_note_progress_copy_contacts),
                mProgressCurrNum, mProgressTotalCount);
        notifyChanged();
    }

    /**
     * finish progress bar
     */
    public void finishProgressBar() {
        mFinishImporting = true;
        mIsImporting = false;
        mProgressTotalCount = 0;
        notifyChanged();
    }

    /**
     * deal with cancel button click
     */
    public void dealWithCancel() {
        mFinishImporting = false;
        mIsImporting = false;
        mProgressTotalCount = 0;
        notifyChanged();
    }
}
