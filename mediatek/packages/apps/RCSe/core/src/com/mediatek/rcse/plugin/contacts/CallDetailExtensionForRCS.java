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

package com.mediatek.rcse.plugin.contacts;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.contacts.ext.CallDetailExtension;

import com.mediatek.rcse.plugin.contacts.CallLogExtention.Action;

public class CallDetailExtensionForRCS extends CallDetailExtension {

	private static final String TAG = "CallDetailExtensionForRCS";
	private CallLogExtention mCallLogPlugin;
	private Action[] mRcsActions;
	public static final String RCS_DISPLAY_NAME = "rcs_display_name";
	public static final String RCS_PHONE_NUMBER = "rcs_phone_number";
	private Activity mActivity;

	// private ImageView mRCSIcon;
	// private int mRCSIconViewWidth;
	// private int mRCSIconViewHeight;
	// private boolean mRCSIconViewWidthAndHeightAreReady = false;
	public static final String COMMD_FOR_RCS = "ExtenstionForRCS";

	public CallDetailExtensionForRCS(Context mContext) {
		mCallLogPlugin = new CallLogExtention(mContext);
	}

	public void setViewVisibleByActivity(Activity activity, String displayName, String number,
			int container, int separator, int viewId, int actionViewId, int textViewId,
			int imageViewId, int divider, String commd) {

		if (!COMMD_FOR_RCS.equals(commd)) {
			return;
		}
		String chat = null;
		Drawable rcsIcon = null;
		Drawable rcsActionIcon = null;
		mActivity = activity;
		// if it has im and file transfer function the values is true.
		boolean hasIM = false;
		boolean hasFT = false;
		boolean isEnable = false;
		String[] contactInfo = {
				number, displayName
		};
		Log.i(TAG, "[setViewVisibleByActivity] number = " + number
				+ " | displayName : " + displayName + ",mCallLogPlugin = "
				+ mCallLogPlugin);

		if (mCallLogPlugin != null) {
			chat = mCallLogPlugin.getChatString();
			rcsIcon = mCallLogPlugin.getContactPresence(number);
			isEnable = mCallLogPlugin.isEnabled();
			mRcsActions = mCallLogPlugin.getContactActions(number);
			if (mRcsActions[0] != null && mRcsActions[1] != null) {
				if (null != mRcsActions[0].intentAction) {
					hasIM = true;
				}
				if (null != mRcsActions[1].intentAction) {
					hasFT = true;
				}
			}
			Log.i(TAG, "[setViewVisibleByActivity] rcsIcon : " + (rcsIcon != null)
					+ " | isEnable : " + isEnable + " | hasIM , hasFT : " + hasIM + " , " + hasFT);
		}
		if (mRcsActions[1] != null) {
			rcsActionIcon = mRcsActions[1].icon;
		}
		// add consider the number is support rcs
		boolean result = ((rcsIcon != null) && isEnable);
		// boolean result = true;
		View rcsContainer = activity.findViewById(container);
		View separator03 = activity.findViewById(separator);
		View convertView3 = activity.findViewById(viewId);
		View rcsAction = convertView3.findViewById(actionViewId);

		String rcsTextVaule = chat + " " + number;
		Log.i(TAG, "[setViewVisibleByActivity] chat = " + chat + " | rcsTextVaule : "
				+ rcsTextVaule);
		if (!hasIM) {
			rcsTextVaule = number;
		}
		rcsAction.setTag(contactInfo);
		TextView rcsText = (TextView) convertView3.findViewById(textViewId);
		rcsText.setText(rcsTextVaule);
		ImageView icon = (ImageView) convertView3.findViewById(imageViewId);
		icon.setOnClickListener(mRcsTransforActionListener);
		icon.setTag(contactInfo);
		View dividerView = convertView3.findViewById(divider);
		rcsAction.setOnClickListener(mRcsTextActionListener);
		icon.setImageDrawable(rcsActionIcon);

		rcsContainer.setVisibility(result ? View.VISIBLE : View.GONE);
		icon.setVisibility(result ? View.VISIBLE : View.GONE);
		dividerView.setVisibility(result ? View.VISIBLE : View.GONE);
		separator03.setVisibility(result ? View.VISIBLE : View.GONE);
		rcsAction.setVisibility(result ? View.VISIBLE : View.GONE);
		if (hasIM && !hasFT) {
			icon.setVisibility(View.GONE);
			dividerView.setVisibility(View.GONE);
		} else if (!hasIM && hasFT) {
			rcsAction.setClickable(false);
		} else if (!hasIM && !hasFT) {
			rcsContainer.setVisibility(View.GONE);
			icon.setVisibility(View.GONE);
			dividerView.setVisibility(View.GONE);
			separator03.setVisibility(View.GONE);
			rcsAction.setVisibility(View.GONE);
		}
	}

	public boolean isEnabled(String number) {
		if (mCallLogPlugin != null && number != null) {
			Drawable a = mCallLogPlugin.getContactPresence(number);
			boolean isEnabled = mCallLogPlugin.isEnabled();
			Log.i(TAG, "[isEnabled] a is not null and edbaled :" + (null != a) + " , " + isEnabled);
			return ((null != a) && isEnabled);
		} else {
			Log.e(TAG, "[isEnabled] mCallLogPlugin or number is null " + mCallLogPlugin + " , "
					+ number);
			return false;
		}
	}

	protected boolean isVisible(View view) {
		return view != null && view.getVisibility() == View.VISIBLE;
	}

	private final View.OnClickListener mRcsTransforActionListener = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			String[] contactInfo = (String[]) view.getTag();
			Intent intent = new Intent();
			String number = contactInfo[0];
			String name = contactInfo[1];
			if (mRcsActions[1] != null) {
				intent = mRcsActions[1].intentAction;
				Log.i(TAG, "[mRcsTransforActionListener] intent : " + intent);
			}

			Log.i(TAG, "[mRcsTransforActionListener] name : " + name
					+ " | number : " + number + ",mRcsActions[1] = "
					+ mRcsActions[1]);
			if (TextUtils.isEmpty(name)) {
				name = number;
			}
			intent.putExtra(RCS_DISPLAY_NAME, name);
			intent.putExtra(RCS_PHONE_NUMBER, number);

			mActivity.startActivity(intent);
		}
	};

	private final View.OnClickListener mRcsTextActionListener = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			String[] contactInfo = (String[]) view.getTag();
			Intent intent = new Intent();
			String number = contactInfo[0];
			String name = contactInfo[1];
			if (mRcsActions[0] != null) {
				intent = mRcsActions[0].intentAction;
				Log.i(TAG, "[mRcsTextActionListener] intent : " + intent);
			}
			Log.i(TAG, "[mRcsTextActionListener] name : " + name
					+ " | number : " + number + ", mRcsActions[0] = "
					+ mRcsActions[0]);
			if (TextUtils.isEmpty(name)) {
				name = number;
			}
			intent.putExtra(RCS_DISPLAY_NAME, name);
			intent.putExtra(RCS_PHONE_NUMBER, number);

			mActivity.startActivity(intent);
		}
	};

	/**
	 * set RCS view for CallDetailHistoryAdapter
	 */
	public void setViewVisible(View view, String number, String commd2, int headerRcsContainer,
			int res2, int res3, int res4, int res5, int res6, int res7) {
		if (!COMMD_FOR_RCS.equals(commd2)) {
			return;
		}
		boolean result = isEnabled(number);
		View rcsContainer = view.findViewById(headerRcsContainer);
		rcsContainer.setVisibility(result ? View.VISIBLE : View.GONE);
		View separator03 = view.findViewById(headerRcsContainer);
		separator03.setVisibility(result ? View.VISIBLE : View.GONE);
	}
}
