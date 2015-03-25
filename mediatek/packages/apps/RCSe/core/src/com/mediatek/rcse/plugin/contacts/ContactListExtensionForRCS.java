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

import java.util.List;

import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.contacts.ext.ContactListExtension;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.service.UnreadMessageManager;

public class ContactListExtensionForRCS extends ContactListExtension{
	private static final String TAG = "ContactListExtensionForRCS";
	private ContactExtention mContactPlugin;
	public static final String COMMD_FOR_RCS = "ExtenstionForRCS";
	private Drawable mExtenstionIcon;

	public ContactListExtensionForRCS (Context mContext) {
		mContactPlugin = new ContactExtention(mContext);
	}

	public void setExtentionImageView(ImageView view, String commd) {

		Log.i(TAG, "[setExtentionImageView] commd : " + commd);
		if (null != mContactPlugin && COMMD_FOR_RCS.equals(commd)) {
			if (mExtenstionIcon == null) {
				mExtenstionIcon = mContactPlugin.getAppIcon();
			}
			Log.i(TAG, "[setExtentionImageView] icon : " + mExtenstionIcon);
			view.setImageDrawable(mExtenstionIcon);
		} else {
			Log.e(TAG, "setExtentionImageView mContactPlugin is null");
		}
	}
	
	public void setExtentionTextView(TextView view, long contactId, String commd) {

		Log.i(TAG, "[setExtentionTextView] commd : " + commd + "contactId" +contactId);
		if (null != mContactPlugin && COMMD_FOR_RCS.equals(commd)) {
			
			List<String> numbers = mContactPlugin.getNumbersByContactId(contactId);
			if(numbers != null && numbers.size() >0 )
			{
			int unreadCount = mContactPlugin.getUnreadMessageCount(numbers.get(0));
			Log.i(TAG, "[setExtentionTextView] unreadcount : " + unreadCount +" contact : "+ numbers.get(0));
			if(view != null && unreadCount !=0)
			{
				view.setTextColor(Color.BLACK);
				view.setText(String.valueOf(unreadCount));
			}
			}
			else
			{
			    Log.e(TAG, "setExtentionTextView numbers is null/empty " + numbers);
			}
			//Log.i(TAG, "[setExtentionTextView] icon : " + mExtenstionText);
			
		} else {
			Log.e(TAG, "setExtentionTextView mContactPlugin is null");
		}
	}
}
