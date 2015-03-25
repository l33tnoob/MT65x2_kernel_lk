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

package com.mediatek.filemanager;

import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.os.Bundle;
import android.os.Message;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewParent;
import android.widget.Button;
import android.widget.RelativeLayout;

import android.widget.TextView;

import com.mediatek.filemanager.service.ProgressInfo;

public class ProgressDialogFragment extends DialogFragment {
    public static final String TAG = "ProgressDialogFragment";
    private static final String STYLE = "style";
    private static final String TITLE = "title";
    private static final String CANCEL = "cancel";
    private static final String TOTAL = "total";
    private static final String PROGRESS = "progress";
    private static final String CURRENTNUM = "currentNumber";
    private static final String TOTALNUM = "totalNumber";
    private static final String MESSAGE = "message";
    private int mSavedCurrentNum = 0;
    private int mSavedTotalNum = 0;
    private TextView mProgressNum = null;
    private View.OnClickListener mCancelListener = null;
    private int mViewDirection = 0;

    /**
     * This method gets a instance of ProgressDialogFragment
     * 
     * @param style
     *            resource ID of style of DialogFragment
     * @param title
     *            resource ID of title shown on DialogFragment
     * @param message
     *            resource ID of message shown on DialogFragment
     * @param cancel
     *            resource ID of content on cancel button
     * @return a progressDialogFragment
     */
    public static ProgressDialogFragment newInstance(int style, int title, int message, int cancel) {
        ProgressDialogFragment f = new ProgressDialogFragment();
        Bundle args = new Bundle();
        args.putInt(STYLE, style);
        args.putInt(TITLE, title);
        args.putInt(CANCEL, cancel);
        args.putInt(MESSAGE, message);
        f.setArguments(args);
        return f;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putAll(getArguments());
        ProgressDialog dialog = (ProgressDialog) getDialog();
        if (dialog != null) {
            outState.putInt(TOTAL, dialog.getMax());
            outState.putInt(PROGRESS, dialog.getProgress());
            outState.putInt(CURRENTNUM, mSavedCurrentNum);
            outState.putInt(TOTALNUM, mSavedTotalNum);
        }
        super.onSaveInstanceState(outState);
    }

    @Override
    public void onResume() {
        super.onResume();
        if (getDialog() != null) {
            Button mButtonNegative = (Button) getDialog().findViewById(
                    com.android.internal.R.id.button3);
            mButtonNegative.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mCancelListener != null) {
                        mCancelListener.onClick(v);
                    }
                    ProgressDialog dialog = (ProgressDialog) getDialog();
                    dialog.setMessage(getString(R.string.wait));
                    v.setVisibility(View.INVISIBLE);
                }
            });
        }
    }

    /**
     * This method sets cancel listener to cancel button
     * 
     * @param listener
     *            clickListener, which will do proper things when touch cancel
     *            button
     */
    public void setCancelListener(View.OnClickListener listener) {
        mCancelListener = listener;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        this.setCancelable(false);
        ProgressDialog dialog = new ProgressDialog(getActivity());
        dialog.setCancelable(false);
        dialog.setIndeterminate(false);
        Bundle args = null;
        if (savedInstanceState == null) {
            args = getArguments();
        } else {
            args = savedInstanceState;
        }
        if (args != null) {
            int style = args.getInt(STYLE, ProgressDialog.STYLE_HORIZONTAL);
            dialog.setProgressStyle(style);
            int title = args.getInt(TITLE, AlertDialogFragment.INVIND_RES_ID);
            if (title != AlertDialogFragment.INVIND_RES_ID) {
                dialog.setTitle(title);
            }
            int cancel = args.getInt(CANCEL, AlertDialogFragment.INVIND_RES_ID);
            if (cancel != AlertDialogFragment.INVIND_RES_ID) {
                dialog.setButton(ProgressDialog.BUTTON_NEUTRAL, getString(cancel), (Message) null);
            }

            int message = args.getInt(MESSAGE, AlertDialogFragment.INVIND_RES_ID);
            if (message != AlertDialogFragment.INVIND_RES_ID) {
                dialog.setMessage(getString(message));
            }
            int total = args.getInt(TOTAL, -1);
            if (total != -1) {
                dialog.setMax(total);
            }
            int progress = args.getInt(PROGRESS, -1);
            if (progress != -1) {
                dialog.setProgress(progress);
            }
            if (mProgressNum != null) {
                int currentNum = args.getInt(CURRENTNUM, -1);
                int totalNum = args.getInt(TOTALNUM, -1);
                if (currentNum != -1 && totalNum != -1) {
                    mProgressNum.setText(currentNum + "/" + totalNum);
                }
            }
        }

        dialog.setOnKeyListener(new OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_SEARCH) {
                    return true;
                }
                return false;
            }
        });
        return dialog;
    }

    @Override
    public void onStart() {
        super.onStart();
        /*
         * in order to customize the Dialog to show file Number, add a new
         * TextView to replace the default TextView of the progressdialog note:
         * cannot put this code section to onCreateDialog, because the Dialog
         * has not being created yet when onCreateDialog is called..
         */
        if (mProgressNum == null) {
            ProgressDialog mDialog = (ProgressDialog) getDialog();
            mProgressNum = new TextView(mDialog.getContext());
            TextView view = (TextView) mDialog
                    .findViewById(com.android.internal.R.id.progress_number);
            LayoutParams params = view.getLayoutParams();
            mProgressNum.setLayoutParams(params);
            view.setVisibility(View.GONE);
            mProgressNum.setTextColor(view.getCurrentTextColor());
            mProgressNum.setSingleLine();

            ViewParent vParent = view.getParent();
            if (vParent instanceof RelativeLayout) {
                RelativeLayout parentLayout = (RelativeLayout) vParent;
                parentLayout.addView(mProgressNum);
            }
        }
    }

    /**
     * This method sets progress of progressDialog according to information of
     * received ProgressInfo.
     * 
     * @param progeressInfo
     *            information which need to be updated on progressDialog
     */
    public void setProgress(ProgressInfo progeressInfo) {
        ProgressDialog progressDialog = (ProgressDialog) getDialog();
        if (progressDialog != null && progeressInfo != null) {
            TextView messageView = (TextView) progressDialog
                    .findViewById(com.android.internal.R.id.message);
            if (messageView != null) {
                messageView.setSingleLine();
                messageView.setEllipsize(TextUtils.TruncateAt.MIDDLE);
            }
            progressDialog.setProgress(progeressInfo.getProgeress());
            String message = progeressInfo.getUpdateInfo();
            if (!TextUtils.isEmpty(message)) {
                progressDialog.setMessage(message);
            }
            progressDialog.setMax((int) progeressInfo.getTotal());
            mSavedCurrentNum = progeressInfo.getCurrentNumber();
            mSavedTotalNum = (int) progeressInfo.getTotalNumber();
            if (mProgressNum != null) {
                if (mViewDirection == ViewGroup.LAYOUT_DIRECTION_LTR) {
                    mProgressNum.setText(mSavedCurrentNum + "/" + mSavedTotalNum);
                } else if (mViewDirection == ViewGroup.LAYOUT_DIRECTION_RTL) {
                    mProgressNum.setText(mSavedCurrentNum + "\\" + mSavedTotalNum);
                }
            }
        }
    }

    public void setViewDirection(int direction) {
        mViewDirection = direction;
    }
}
