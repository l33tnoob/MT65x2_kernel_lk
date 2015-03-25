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

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.contacts.ext.CallListExtension;

public class CallListExtensionForRCS extends CallListExtension {
	private static final String TAG = "CallListExtensionForRCS";
	private CallLogExtention mCallLogPlugin;
	// private Action [] RCSActions;
	public static final String RCS_DISPLAY_NAME = "rcs_display_name";
	public static final String RCS_PHONE_NUMBER = "rcs_phone_number";
	public static final String COMMD_FOR_RCS = "ExtenstionForRCS";
	// private Activity mActivity;
	// private ImageView mRCSIcon;
	private int mRCSIconViewWidth;
	private int mRCSIconViewHeight;
	private boolean mRCSIconViewWidthAndHeightAreReady = false;
	private Drawable mExtenstionIcon;

	public CallListExtensionForRCS(Context mContext) {
		mCallLogPlugin = new CallLogExtention(mContext);
	}

	public int layoutExtentionIcon(int leftBound, int topBound, int bottomBound, int rightBound,
			int mGapBetweenImageAndText, ImageView mExtentionIcon, String commd) {
		if (this.isVisible(mExtentionIcon) && mExtentionIcon != null && COMMD_FOR_RCS.equals(commd)) {
			int photoTop1 = topBound + (bottomBound - topBound - mRCSIconViewHeight) / 2;
			mExtentionIcon.layout(rightBound - (mRCSIconViewWidth), photoTop1, rightBound,
					photoTop1 + mRCSIconViewHeight);
			rightBound -= (mRCSIconViewWidth + mGapBetweenImageAndText);
		}
		return rightBound;

	}

	protected boolean isVisible(View view) {
		return view != null && view.getVisibility() == View.VISIBLE;
	}

	public void measureExtention(ImageView mRCSIcon, String commd) {
		if (isVisible(mRCSIcon) && COMMD_FOR_RCS.equals(commd)) {
			if (!mRCSIconViewWidthAndHeightAreReady) {
				if (mCallLogPlugin != null) {
					Drawable a = mCallLogPlugin.getAppIcon();
					if (a != null) {
						mRCSIconViewWidth = a.getIntrinsicWidth();
						mRCSIconViewHeight = a.getIntrinsicHeight();
					} else {
						mRCSIconViewWidth = 0;
						mRCSIconViewHeight = 0;
					}
				} else {
					mRCSIconViewWidth = 0;
					mRCSIconViewHeight = 0;
				}
				Log.i(TAG, "measureExtention mRCSIconViewWidth : " + mRCSIconViewWidth
						+ " | mRCSIconViewHeight : " + mRCSIconViewHeight);
				mRCSIconViewWidthAndHeightAreReady = true;
			}
		}
	}

	public boolean setExtentionIcon(String number, String commd) {
		if (mCallLogPlugin != null && number != null) {
			Drawable a = mCallLogPlugin.getContactPresence(number);
			boolean isEnabled = mCallLogPlugin.isEnabled();
			Log.i(TAG, "[setExtentionIcon] isEnabled : " + isEnabled + " | commd : " + commd);
			if ((a != null) && isEnabled && COMMD_FOR_RCS.equals(commd)) {
				return true;
			} else {
				Log.i(TAG, "[setExtentionIcon] a : " + a + " |isEnabled : " + isEnabled);
				return false;
			}
		} else {
			Log.e(TAG, "[setExtentionIcon] mCallLogPlugin : " + mCallLogPlugin);
			return false;
		}
	}

	public void setExtentionImageView(ImageView view, String commd) {
		Log.i(TAG, "[setExtentionImageView] commd : " + commd);
		if (null != mCallLogPlugin && COMMD_FOR_RCS.equals(commd)) {
			if (mExtenstionIcon == null) {
				mExtenstionIcon = mCallLogPlugin.getAppIcon();
			}
			view.setImageDrawable(mExtenstionIcon);
		} else {
			Log.e(TAG, "mCallLogPlugin is null");
		}

	}

	public boolean checkPluginSupport(String commd) {
		Log.i(TAG, "[checkPluginSupport] commd : " + commd);
		if (COMMD_FOR_RCS.equals(commd)) {
			return true;
		} else {
			return false;
		}
	}
}
