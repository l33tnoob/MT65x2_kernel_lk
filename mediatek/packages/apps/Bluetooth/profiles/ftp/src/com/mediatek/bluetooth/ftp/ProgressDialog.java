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


package com.mediatek.bluetooth.ftp;

import com.mediatek.bluetooth.R;

import android.app.AlertDialog;
import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.StyleSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

public class ProgressDialog extends AlertDialog {

	private static final String TAG = "ProgressDialog";

	public static final int MODE_BASE					= 0,
							MODE_SIMPLE					= MODE_BASE + 1,
							MODE_SINGLE_TRANSFERRED		= MODE_BASE + 2,
							MODE_SINGLE_PERCENTAGE		= MODE_BASE + 3,
							MODE_MULTIPLE				= MODE_BASE + 4;

	private int mMode = MODE_SIMPLE;

	private ProgressBar mProgress;

	private TextView mMessageView;

	private TextView mProgressText;

	private Handler mHandler;

	private int mMax;

	private long mProgressValue;

	private boolean mIndeterminate;

	private String mMessage = null;

	public ProgressDialog(Context context) {
		super(context);
	}

	public ProgressDialog(Context context, int theme) {
		super(context, theme);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		mHandler = new Handler() {
			public void handleMessage(Message msg) {
				if (mProgress != null) {
					mProgress.setProgress((int) mProgressValue);
				}

				if (mProgressText != null) {
					if (mMode == MODE_SINGLE_PERCENTAGE) {
						mProgressText.setText("" + mProgressValue + "%");
					} else if (mMode == MODE_SINGLE_TRANSFERRED) {
						mProgressText.setText(Utils.getReadableSize(mProgressValue));
					} else {
						mProgressText.setText("" + mProgressValue + "/" + mMax);
					}
				}

				if (mMessageView != null) {
					mMessageView.setText(mMessage);
				}
			}
		};

		LayoutInflater inflater = LayoutInflater.from(getContext());
		if (mMode == MODE_SIMPLE) {
			View view = inflater.inflate(R.layout.bt_ftp_progress_dialog_simple, null);
			mProgress = (ProgressBar) view.findViewById(R.id.progress);
			mMessageView = (TextView) view.findViewById(R.id.message);
			mProgressText = null;
			setView(view);

		} else {
			View view = inflater.inflate(R.layout.bt_ftp_progress_dialog, null);
			mProgress = (ProgressBar) view.findViewById(R.id.progress);
			mMessageView = (TextView) view.findViewById(R.id.message);
			mProgressText = (TextView) view.findViewById(R.id.progress_text);
			setView(view);
		}

		setIndeterminate(mIndeterminate);
		updateUI();

		super.onCreate(savedInstanceState);
	}

	public void setProgress(long value) {
		mProgressValue = value;
		updateUI();
	}

	public void setMax(int max) {
		mMax = max;
		updateUI();
	}

	public void setMessage(String message) {
		mMessage = message;
		updateUI();
	}

	public void setIndeterminate(boolean indeterminate) {
		if (mProgress != null) {
			mProgress.setIndeterminate(indeterminate);
		}
		mIndeterminate = indeterminate;
	}

	public void setProgressMode(int mode) {
		mMode = mode;
	}

	private void updateUI() {
		if (mHandler != null) {
			mHandler.sendEmptyMessage(0);
		}
	}

}
