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
import android.app.AlertDialog;
import android.graphics.drawable.Drawable;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.view.View;
import android.view.View.MeasureSpec;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.contacts.ext.ContactDetailExtension;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl.UnreadMessageListener;
import com.mediatek.rcse.plugin.contacts.ContactExtention.Action;
import com.mediatek.rcse.plugin.contacts.ContactExtention.OnPresenceChangedListener;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.UnreadMessageManager;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.contacts.ContactsApiIntents;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

public class ContactDetailExtensionForRCS extends ContactDetailExtension {
	private static final String TAG = "ContactExtentionForRCS";
	private ContactExtention mContactPlugin;
	public static final String RCS_DISPLAY_NAME = "rcs_display_name";
	public static final String RCS_PHONE_NUMBER = "rcs_phone_number";
    public static final String RCS_IS_CONVERGED = "rcs_is_converged";
	private static final int RCS_ENABLED_CONTACT = 1;
	private int mRCSIconViewWidth;
	private int mRCSIconViewHeight;
	private int mRCSTextViewWidth;
	private int mRCSTextViewHeight;
	private boolean mRCSIconViewWidthAndHeightAreReady = false;
	private boolean mRCSTextViewWidthAndHeightAreReady = false;
	private String mNumber;
	private String mName;
    private boolean mIsRCSeDisabled = false;
	private Activity mActivity;
	private int mIMValue;
	private int mFTValue;
	private Context mContext;
	private int mVtcallAction;
	private int mSecondaryAction;
	public static final String RCS_CONTACT_PRESENCE_CHANGED = "android.intent.action.RCS_CONTACT_PRESENCE_CHANGED";
	public static final String COMMD_FOR_RCS = "ExtenstionForRCS";
	private PluginApiManager mInstance = null;

	public ContactDetailExtensionForRCS(Context context) {
		mContext = context;
		mInstance = PluginApiManager.getInstance();
		mContactPlugin = new ContactExtention(context);
		RcsSettings.createInstance(mContext);
	}

	public void onContactDetailOpen(Uri contactLookupUri, String commd) {
		if (!COMMD_FOR_RCS.equals(commd)) {
			Log.i(TAG, "[onContactDetailOpen]not RCSe commd " + commd);
			return;
		}
		if (mContactPlugin != null) {
			mContactPlugin.onContactDetailOpen(contactLookupUri);
		} else {
			Log.e(TAG, "[onContactDetailOpen] mContactPlutin is null");
		}
		Log.i(TAG, "[onContactDetailOpen] contactLookupUri : " + contactLookupUri);
	}

	public void setViewVisible(View view, Activity activity, String mimetype, String data,
			String displayName, String commd, int vtcall_action_view_container,
			int vertical_divider_vtcall, int vtcall_action_button,
			int secondary_action_view_container, int secondary_action_button, int vertical_divider, int plugin_action_view_container, int plugin_action_button, long contactId, Context context,
			int messaging_action_view_container, int messaging_action_button, int vertical_divider_messaging, int vertical_divider_secondary_action, int number_text_view) {
		// TODO Auto-generated method stub
		View detailView = view;
		Log.i(TAG, "[setViewVisible] commd : " + commd);
		mVtcallAction = vtcall_action_button;
		mSecondaryAction = secondary_action_button;
		if (mContactPlugin != null && mContactPlugin.isEnabled() && COMMD_FOR_RCS.equals(commd)) {
			if (mimetype != null && mimetype.equals(mContactPlugin.getMimeType())
					&& detailView != null) {
				View vtcallActionViewContainer = detailView
						.findViewById(vtcall_action_view_container);
				View vewVtCallDivider = detailView.findViewById(vertical_divider_vtcall);
				ImageView btnVtCallAction = (ImageView) detailView
						.findViewById(vtcall_action_button);
				View secondaryActionViewContainer = detailView
						.findViewById(secondary_action_view_container);
				ImageView secondaryActionButton = (ImageView) detailView
						.findViewById(secondary_action_button);
				View pluginActionViewContainer = detailView
				        .findViewById(plugin_action_view_container);
		        ImageView pluginActionButton = (ImageView) detailView
				        .findViewById(plugin_action_button);
		        View messagingActionViewContainer = detailView
		                .findViewById(messaging_action_view_container);
                ImageView messagingActionButton = (ImageView) detailView
		                .findViewById(messaging_action_button);
                TextView numberTextView = (TextView) detailView
                        .findViewById(number_text_view);
                numberTextView.setOnClickListener(null);
                vewVtCallDivider.setOnClickListener(null);
                View messagingDivider = detailView.findViewById(vertical_divider_messaging);
                View BlockingDivider = detailView.findViewById(vertical_divider_secondary_action);
		        List<String> numbers = mContactPlugin.getNumbersByContactId(contactId);
				View secondaryActionDivider = detailView.findViewById(vertical_divider);
				Action[] mRCSACtions = mContactPlugin.getContactActions();
				messagingDivider.setOnClickListener(null);
				BlockingDivider.setOnClickListener(null);
				secondaryActionDivider.setOnClickListener(null);
				Drawable a = null;
				Drawable b = null;
				Drawable c = null;
				Intent intentMessaging = null;
				Intent intentFT = null;
				Intent intentXms = null;
				mName = displayName;
				mNumber = data;
				mActivity = activity;
				if (vewVtCallDivider != null && secondaryActionDivider != null
						&& btnVtCallAction != null && secondaryActionButton != null && pluginActionButton != null
						&& secondaryActionViewContainer != null
						&& vtcallActionViewContainer != null && pluginActionViewContainer != null) {
					// mRCSACtions cannot be null, since
					// mContactPlugin.getContactActions() will not return null
                	//Plugincontainer is for block contact
                	if (Logger.getIsIntegrationMode()) 
                	{
                		
                		int mode = RcsSettings.getInstance().getMessagingUx();
                		String isRCSeDisabled = RcsSettings.getInstance().getProvisioningVersion();
                		if(isRCSeDisabled.equals("-1"))
                		{
                			mIsRCSeDisabled = true;
                		}
                		//0 means converged, 1 means integrated
                		boolean converged = false;
                		a = mRCSACtions[0].icon;
                        b = mRCSACtions[1].icon;
                        c = mContactPlugin.getXMSDrawable();
                        intentMessaging = mRCSACtions[0].intentAction;
                        intentFT = mRCSACtions[1].intentAction;
                        intentXms = mRCSACtions[0].intentAction;
                        //intent.putExtra(RCS_IS_CONVERGED, mode);
                		//also need to add check if converged/fully integrated mode
                		if(mode == 1)
                		{
	                		//in fully integrated we need to show  1 messaging(unified) icon only
	                        
                            Log.i(TAG, "setViewVisible 1 integrated mode");
                            vewVtCallDivider.setVisibility(View.GONE);
                            secondaryActionDivider.setVisibility(View.GONE);
    						btnVtCallAction.setVisibility(View.GONE);
    						secondaryActionButton.setVisibility(View.VISIBLE);
    						secondaryActionViewContainer.setVisibility(View.VISIBLE);
    						//secondaryActionButton.setTag(intentMessaging);
    						secondaryActionButton.setImageDrawable(a);
    						secondaryActionViewContainer.setClickable(false);
                            
                            pluginActionViewContainer.setVisibility(View.VISIBLE);
    						pluginActionButton.setVisibility(View.VISIBLE);
    						pluginActionButton.setImageDrawable(mContactPlugin.getBlockingDrawable());
    						pluginActionButton.setTag(vtcall_action_button, mNumber);
    						pluginActionButton.setTag(secondary_action_button, context);
    						pluginActionButton.setOnClickListener(mBlockingButtonOnClickListner);
                            
                		}
                		else
                		{
                			//in converged mode, need to show different icons for Joyn, messaging, FT
                			
                			if (mIMValue == 1 && mFTValue == 1) {
                			Log.i(TAG, "setViewVisible 1");
                            vewVtCallDivider.setVisibility(View.GONE);
                            secondaryActionDivider.setVisibility(View.VISIBLE);
                            btnVtCallAction.setVisibility(View.VISIBLE);
                            secondaryActionButton.setVisibility(View.VISIBLE);
                            secondaryActionViewContainer.setVisibility(View.VISIBLE);
                            vtcallActionViewContainer.setVisibility(View.VISIBLE);
                            btnVtCallAction.setImageDrawable(a);
        						btnVtCallAction.setTag(intentMessaging);
        						btnVtCallAction.setOnClickListener(mIMButtononClickListner);
                            secondaryActionButton.setImageDrawable(b);
                            vtcallActionViewContainer.setClickable(false);
        						secondaryActionButton.setTag(intentFT);
                            secondaryActionButton.setOnClickListener(msetScondBuottononClickListner);
        						
        						//set XMS icon to be visible
        						messagingActionViewContainer.setVisibility(View.VISIBLE);
        						messagingActionViewContainer.setClickable(false);
        						messagingActionButton.setVisibility(View.VISIBLE);
        						messagingActionButton.setImageDrawable(c);
        						messagingActionButton.setTag(intentXms);
        						messagingActionButton.setOnClickListener(mMessagingButtononClickListner);
        						messagingDivider.setVisibility(View.VISIBLE);
        						

        					}

        					if (mIMValue == 1 && mFTValue != 1) {
        						Log.i(TAG, "setViewVisible 2");
        						vewVtCallDivider.setVisibility(View.GONE);
        						secondaryActionDivider.setVisibility(View.GONE);
        						btnVtCallAction.setVisibility(View.GONE);
        						secondaryActionButton.setVisibility(View.VISIBLE);
        						secondaryActionViewContainer.setVisibility(View.VISIBLE);
        						//secondaryActionButton.setTag(intentMessaging);
        						secondaryActionButton.setImageDrawable(a);
        						secondaryActionButton.setTag(intentMessaging);
        						secondaryActionButton.setOnClickListener(mIMButtononClickListner);
        						secondaryActionViewContainer.setClickable(false);
        						
        						//set XMS icon to be visible
        						messagingActionViewContainer.setVisibility(View.VISIBLE);
        						messagingActionViewContainer.setClickable(false);
        						messagingActionButton.setVisibility(View.VISIBLE);
        						messagingActionButton.setImageDrawable(c);
        						messagingActionButton.setTag(intentXms);
        						messagingActionButton.setOnClickListener(mMessagingButtononClickListner);
        						messagingDivider.setVisibility(View.VISIBLE);
        					}

        					if (mIMValue != 1 && mFTValue == 1) {
        						Log.i(TAG, "setViewVisible 3");
        						vewVtCallDivider.setVisibility(View.GONE);
        						secondaryActionDivider.setVisibility(View.GONE);
        						btnVtCallAction.setVisibility(View.GONE);
        						secondaryActionButton.setVisibility(View.VISIBLE);
        						secondaryActionViewContainer.setVisibility(View.VISIBLE);
        						//secondaryActionButton.setTag(intentFT);
        						secondaryActionButton.setImageDrawable(b);
        						secondaryActionViewContainer.setClickable(false);
        					}
                            pluginActionViewContainer.setVisibility(View.VISIBLE);
    						pluginActionButton.setVisibility(View.VISIBLE);
    						pluginActionButton.setImageDrawable(mContactPlugin.getBlockingDrawable());
    						pluginActionButton.setTag(vtcall_action_button, mNumber);
    						pluginActionButton.setTag(secondary_action_button, context);
    						pluginActionButton.setOnClickListener(mBlockingButtonOnClickListner);
                		}
                		messagingDivider.setVisibility(View.VISIBLE);

                	}
                	else
                	{
							a = mRCSACtions[0].icon;
					b = mRCSACtions[1].icon;
					intentFT = mRCSACtions[1].intentAction;

					if (mIMValue == 1 && mFTValue == 1) {
						Log.i(TAG, "setViewVisible 1");
						vewVtCallDivider.setVisibility(View.GONE);
						secondaryActionDivider.setVisibility(View.VISIBLE);
						btnVtCallAction.setVisibility(View.VISIBLE);
						secondaryActionButton.setVisibility(View.VISIBLE);
						secondaryActionViewContainer.setVisibility(View.VISIBLE);
						vtcallActionViewContainer.setVisibility(View.VISIBLE);
						btnVtCallAction.setImageDrawable(a);
						secondaryActionButton.setImageDrawable(b);
						vtcallActionViewContainer.setClickable(false);
						secondaryActionButton.setTag(intentFT);
						secondaryActionButton.setOnClickListener(msetScondBuottononClickListner);
						/*pluginActionViewContainer.setVisibility(View.VISIBLE);
						pluginActionButton.setVisibility(View.VISIBLE);
						pluginActionButton.setImageDrawable(b);
						pluginActionButton.setTag(intent);
						pluginActionButton.setOnClickListener(msetScondBuottononClickListner);*/
						
						
					}

					if (mIMValue == 1 && mFTValue != 1) {
						Log.i(TAG, "setViewVisible 2");
						vewVtCallDivider.setVisibility(View.GONE);
						secondaryActionDivider.setVisibility(View.GONE);
						btnVtCallAction.setVisibility(View.GONE);
						secondaryActionButton.setVisibility(View.VISIBLE);
						secondaryActionViewContainer.setVisibility(View.VISIBLE);

						secondaryActionButton.setImageDrawable(a);
						secondaryActionViewContainer.setClickable(false);
					}

					if (mIMValue != 1 && mFTValue == 1) {
						Log.i(TAG, "setViewVisible 3");
						vewVtCallDivider.setVisibility(View.GONE);
						secondaryActionDivider.setVisibility(View.GONE);
						btnVtCallAction.setVisibility(View.GONE);
						secondaryActionButton.setVisibility(View.VISIBLE);
						secondaryActionViewContainer.setVisibility(View.VISIBLE);

						secondaryActionButton.setImageDrawable(b);
						secondaryActionViewContainer.setClickable(false);
					}
                	}
				} else {
					Log.e(TAG, "[setViewVisible] vewVtCallDivider : " + vewVtCallDivider
							+ " | secondaryActionDivider : " + secondaryActionDivider
							+ " | btnVtCallAction : " + btnVtCallAction
							+ " | secondaryActionButton : " + secondaryActionButton
							+ " | secondaryActionViewContainer : " + secondaryActionViewContainer
							+ " | vtcallActionViewContainer : " + vtcallActionViewContainer);
				}

			} else {
				Log.e(TAG, "[setViewVisible] detailView or mimetype is not equals mimetype : "
						+ mimetype + " | detailView : " + detailView);
			}
		} else {
			Log.e(TAG, "[setViewVisible] mContactPlugin is null or not enabled | mContactPlugin : "
					+ mContactPlugin);
		}
	}

	public void setViewVisibleWithCharSequence(View resultView, Activity activity, String mimeType,
			String data2, CharSequence number, String commd, int vertical_divider_vtcall,
			int vtcall_action_button, int vertical_divider, int secondary_action_button, int res5,
			int res6) {

		mActivity = activity;
		mNumber = number.toString();
		String RCSMimType = null;
		Drawable a = null;
		Drawable b = null;
		Action[] mRCSAction = null;
		if (mContactPlugin != null && mContactPlugin.isEnabled() && COMMD_FOR_RCS.equals(commd)) {
			RCSMimType = mContactPlugin.getMimeType();
			mRCSAction = mContactPlugin.getContactActions();
			if (mRCSAction[0] != null && mRCSAction[1] != null) {
				a = mRCSAction[0].icon;
				b = mRCSAction[1].icon;
			} else {
				Log.e(TAG, "setViewVisibleWithCharSequence action is null");
			}

			if (mimeType != null && RCSMimType != null && !mimeType.equals(RCSMimType)) {
				return;
			}
			final View vewFirstDivider = resultView.findViewById(vertical_divider_vtcall);
			final ImageView btnFirstAction = (ImageView) resultView
					.findViewById(vtcall_action_button);

			final View vewSecondDivider = resultView.findViewById(vertical_divider);
			final ImageView btnSecondButton = (ImageView) resultView
					.findViewById(secondary_action_button);
			if (vewFirstDivider != null && vewSecondDivider != null) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 1");
				vewFirstDivider.setVisibility(View.GONE);
				vewSecondDivider.setVisibility(View.GONE);
			}
			if (btnFirstAction != null && btnSecondButton != null) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 2");
				btnFirstAction.setVisibility(View.GONE);
				btnSecondButton.setVisibility(View.GONE);
			}

			if (btnFirstAction != null && mIMValue == 1 && mFTValue == 1) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 3");

				vewFirstDivider.setVisibility(View.VISIBLE);
			}

			if (btnFirstAction != null && btnSecondButton != null && mIMValue == 1 && mFTValue == 1) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 4");
				btnFirstAction.setImageDrawable(a);
				btnFirstAction.setVisibility(View.VISIBLE);
				btnFirstAction.setClickable(false);
				btnSecondButton.setTag(mRCSAction[1].intentAction);

				btnSecondButton.setImageDrawable(b);
				btnSecondButton.setVisibility(View.VISIBLE);
				btnSecondButton.setOnClickListener(msetScondBuottononClickListner);
			}

			if (btnFirstAction != null && btnSecondButton != null && mIMValue == 1 && mFTValue != 1) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 5");
				btnFirstAction.setImageDrawable(a);
				btnFirstAction.setVisibility(View.VISIBLE);
				btnFirstAction.setClickable(false);

			}

			if (btnFirstAction != null && btnSecondButton != null && mIMValue != 1 && mFTValue == 1) {
				Log.i(TAG, "[setViewVisibleWithCharSequence] 5");
				btnFirstAction.setImageDrawable(b);
				btnFirstAction.setVisibility(View.VISIBLE);
				btnFirstAction.setClickable(false);

			}
		}
		Log.i(TAG, "[setViewVisibleWithCharSequence] mimeType : " + mimeType + " | RCSMimType : "
				+ RCSMimType + " | mRCSAction : " + Arrays.toString(mRCSAction));

	}

	public String getExtentionMimeType(String commd) {
		String mimeType = null;
		if (mContactPlugin != null && COMMD_FOR_RCS.equals(commd)) {
			mimeType = mContactPlugin.getMimeType();
			Log.i(TAG, "getExtentionMimeType mimeType : " + mimeType);
			return mimeType;
		} else {
			Log.e(TAG, "getExtentionMimeType mContactPlugin is null ");
			return mimeType;
		}

	}

	public int layoutExtentionIcon(int leftBound, int topBound, int bottomBound, int rightBound,
			int mGapBetweenImageAndText, ImageView mExtentionIcon, String commd) {
		if (this.isVisible(mExtentionIcon) && COMMD_FOR_RCS.equals(commd)) {
			int photoTop1 = topBound + (bottomBound - topBound - mRCSIconViewHeight) / 2;
			mExtentionIcon.layout(rightBound - (mRCSIconViewWidth), photoTop1, rightBound,
					photoTop1 + mRCSIconViewHeight);
			rightBound -= (mRCSIconViewWidth + mGapBetweenImageAndText);
		}
		return rightBound;
	}

	public int layoutExtentionText(int leftBound, int topBound, int bottomBound, int rightBound,
			int mGapBetweenImageAndText, TextView mExtentionText, String commd) {
		if (this.isVisible(mExtentionText) && COMMD_FOR_RCS.equals(commd)) {
			int photoTop1 = topBound + (bottomBound - topBound - (mRCSTextViewHeight/2) ) / 2;
			mExtentionText.layout(rightBound - (mRCSTextViewWidth), photoTop1, rightBound,
					photoTop1 + mRCSTextViewHeight);
			rightBound -= (mRCSTextViewWidth + mGapBetweenImageAndText);
		}
		return rightBound;
	}

	public void measureExtentionIcon(ImageView mRCSIcon, String commd) {

		if (isVisible(mRCSIcon) && COMMD_FOR_RCS.equals(commd)) {
			if (!mRCSIconViewWidthAndHeightAreReady) {
				if (mContactPlugin != null) {
					Drawable a = mContactPlugin.getAppIcon();
					if (a != null) {
						mRCSIconViewWidth = a.getIntrinsicWidth();
						mRCSIconViewHeight = a.getIntrinsicHeight();
						mRCSIcon.measure(mRCSIconViewWidth, mRCSIconViewHeight);
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

	public void measureExtentionText(TextView mRCSText, String commd) {

		if (isVisible(mRCSText) && COMMD_FOR_RCS.equals(commd)) {
			if (!mRCSTextViewWidthAndHeightAreReady) {
				if (mContactPlugin != null) {
					//Drawable a = mContactPlugin.getAppIcon();
					if (mRCSText != null) {
						//mRCSTextViewWidth = 90;//mRCSText.getMeasuredWidth();
						//mRCSTextViewHeight = 90;//mRCSText.getMeasuredHeight();
						/// need measure here
						mRCSText.measure(MeasureSpec.UNSPECIFIED, MeasureSpec.UNSPECIFIED);
						mRCSTextViewWidth = mRCSText.getMeasuredWidth();
						mRCSTextViewHeight = mRCSText.getMeasuredHeight();
					} else {
						mRCSTextViewWidth = 0;
						mRCSTextViewHeight = 0;
					}
				} else {
					mRCSTextViewWidth = 0;
					mRCSTextViewHeight = 0;
				}
				Log.i(TAG, "measureExtention mRCSTextViewWidth : " + mRCSTextViewWidth
						+ " | mRCSTextViewHeight : " + mRCSTextViewHeight);
				//mRCSTextViewWidthAndHeightAreReady = true;
			}
		}
		
	}

	public Intent getExtentionIntent(int im, int ft, String commd) {
		Intent intent = null;
		mIMValue = im;
		mFTValue = ft;
		if (mContactPlugin != null && COMMD_FOR_RCS.equals(commd)) {
			Action[] actions = mContactPlugin.getContactActions();
			if (mIMValue == 1) {
				intent = actions[0].intentAction;
			} else if (mFTValue == 1) {
				intent = actions[1].intentAction;
			}
		} else {
			Log.e(TAG, "[getExtentionIntent] mContactPlugin is null");
		}
		Log.i(TAG, "[getExtentionIntent] intent : " + intent + " | im : " + im + " | ft : " + ft
				+ " | commd : " + commd);
		return intent;
	}

	public boolean getExtentionKind(String mimeType, boolean needSetName, String name, String commd) {
		if (mContactPlugin != null && mContactPlugin.isEnabled() && COMMD_FOR_RCS.equals(commd)) {
			String newMimeType = mContactPlugin.getMimeType();
			Log.i(TAG, "[getExtentionKind] newMimeType : " + newMimeType);
			if (newMimeType != null && newMimeType.equals(mimeType)) {
				if (needSetName) {
					mName = name;
				}
				return true;
			} else {
				Log.i(TAG, "[getExtentionKind] retrun kind ");
				return false;
			}
		} else {
			return false;
		}
	}

	private OnClickListener msetScondBuottononClickListner = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			Intent intent = (Intent) view.getTag();
			Log.e(TAG, "msetScondBuottononClickListner.onClick():intent = "
					+ intent);
			if (intent != null) {
				Log.i(TAG, "[msetScondBuottononClickListner] name : " + mName
						+ " | number : " + mNumber);
				intent.putExtra(RCS_DISPLAY_NAME, mName);
				intent.putExtra(RCS_PHONE_NUMBER, mNumber);
				intent.putExtra("is_rcse_disabled", mIsRCSeDisabled);
				mActivity.startActivity(intent);
			}

		}
	};
	
	private OnClickListener mMessagingButtononClickListner = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			Intent intent = (Intent) view.getTag();
			Log.e(TAG, "mMessagingButtononClickListner.onClick():intent = "
					+ intent);
			if (intent != null) {
				Log.i(TAG, "[mMessagingButtononClickListner] name : " + mName
						+ " | number : " + mNumber);
				intent.putExtra(RCS_DISPLAY_NAME, mName);
				intent.putExtra(RCS_PHONE_NUMBER, mNumber);
				intent.putExtra("isjoyn", false);
				intent.putExtra("is_rcse_disabled", mIsRCSeDisabled);
				mActivity.startActivity(intent);
			}

		}
	};
	
	private OnClickListener mIMButtononClickListner = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			Intent intent = (Intent) view.getTag();
			Log.e(TAG, "mIMButtononClickListner.onClick():intent = "
					+ intent);
			if (intent != null) {
				Log.i(TAG, "[mIMButtononClickListner] name : " + mName
						+ " | number : " + mNumber);
				intent.putExtra(RCS_DISPLAY_NAME, mName);
				//for Joyn messaging ,need 9+++ extra to differentiate
				intent.putExtra(RCS_PHONE_NUMBER, mNumber);
				intent.putExtra("isjoyn", true);
				intent.putExtra("is_rcse_disabled", mIsRCSeDisabled);
				mActivity.startActivity(intent);
			}

		}
	};

	private OnClickListener mBlockingButtonOnClickListner = new View.OnClickListener() {
		@Override
		public void onClick(View view) {
			//code to block contact
			final String number = (String) view.getTag(mVtcallAction);
			final Context context = (Context) view.getTag(mSecondaryAction);
			String blockTitle = mContactPlugin.getBlockingTitle();
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
					context);
	 
				// set title
				alertDialogBuilder.setTitle("Block contact" + number);
	 
				// set dialog message
				alertDialogBuilder
					.setMessage(blockTitle+ " " + number)
					.setCancelable(false)
					.setPositiveButton("Yes",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,int id) {
							// if this button is clicked, block the contact
							/*ContactsManager.createInstance(context);
							ContactsManager.getInstance().setImBlockedForContact(number, true);
							Toast.makeText(context, number + " is blocked", Toast.LENGTH_LONG).show();*/
							
							//send intent to ApiService
							Intent intent = new Intent(ContactsApiIntents.CONTACT_BLOCK_REQUEST);
							intent.putExtra("number", number);
							context.sendBroadcast(intent);
						}
					  })
					.setNegativeButton("No",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,int id) {
							// if this button is clicked, just close
							// the dialog box and do nothing
							dialog.cancel();
						}
					});
	 
					// create alert dialog
					AlertDialog alertDialog = alertDialogBuilder.create();
	 
					// show it
					alertDialog.show();
			
		}
	};

	protected boolean isVisible(View view) {
		return view != null && view.getVisibility() == View.VISIBLE;
	}

	public boolean checkPluginSupport(String commd) {
		if (mContactPlugin != null && COMMD_FOR_RCS.equals(commd)) {
			boolean result = mContactPlugin.isEnabled();
			Log.i(TAG, "[isEnabled] result : " + result);
			return result;
		} else {
			Log.e(TAG, "isEnabled]mContactPlugin is null");
			return false;
		}
	}

	public String getExtensionTitles(String data, String mimeType, String kind,
			HashMap<String, String> mPhoneAndSubtitle, String commd) {
		Log.i(TAG, "[getExtensionTitles] data : " + data + " | mimeType : " + mimeType
				+ " | kind : " + kind + " | mPhoneAndSubtitle : " + mPhoneAndSubtitle
				+ " | commd : " + commd);
		if (!COMMD_FOR_RCS.equals(commd)) {
			return kind;
		}

		if (null != data && null != mPhoneAndSubtitle) {
			if (mContactPlugin != null && mimeType != null
					&& mimeType.equals(mContactPlugin.getMimeType())) {
				String subTitle = null;
				subTitle = mPhoneAndSubtitle.get(data);
				Log.i(TAG, "[getExtensionTitles] subTitle : " + subTitle + "| data : " + data);
				return subTitle;
			} else {
				Log.e(TAG, "getExtensionTitles return null");
				return null;
			}
		} else {
			if (mContactPlugin != null && mimeType != null
					&& mimeType.equals(mContactPlugin.getMimeType())) {
				String title = mContactPlugin.getAppTitle();
				Log.i(TAG, "[getExtensionTitles] title : " + title);
				return title;
			} else {
				Log.e(TAG, "getExtensionTitles return null");
				return kind;
			}
		}

	}

	public boolean canSetExtensionIcon(long contactId, String commd) {
		Drawable icon = null;
		Log.i(TAG, "[canSetExtensionIcon] commd : " + commd);
		if (mContactPlugin != null && mContactPlugin.isEnabled() && COMMD_FOR_RCS.equals(commd)) {
			Log.i(TAG, "[canSetExtensionIcon] contactId : " + contactId);
			mContactPlugin.addOnPresenceChangedListener(new OnPresenceChangedListener() {
				public void onPresenceChanged(long contactId, int presence) {
					// TODO Auto-generated method stub
					Intent intent = new Intent(RCS_CONTACT_PRESENCE_CHANGED);
					mContext.sendBroadcast(intent);
					Log.i(TAG, "[canSetExtensionIcon] contactId : " + contactId + " | presence : "
							+ presence);
				}
			}, contactId);
			icon = mContactPlugin.getContactPresence(contactId);
			if (null != icon) {
				return true;
			} else {
				return false;
			}
		} else {
			Log.e(TAG, "setExtentionIcon mContactPlugin : " + mContactPlugin);
			return false;
		}
	}

	public void setExtensionImageView(ImageView view, long contactId, String commd) {
		Drawable icon = null;
		if (null != mContactPlugin) {
			icon = mContactPlugin.getContactPresence(contactId);
		} else {
			Log.e(TAG, "mCallLogPlugin is null");
		}
		//UnreadMessageManager.getInstance().getUnreadMessageByContact();
		Log.i(TAG, "[setExtentionImageView] commd : " + commd);
		if (null != view && COMMD_FOR_RCS.equals(commd)) {
			view.setImageDrawable(icon);
		}
	}

	public void setExtensionTextView(TextView view, long contactId, String commd) {
		//Drawable icon = null;
		/*if (null != mContactPlugin) {
			icon = mContactPlugin.getContactPresence(contactId);
		} else {
			Log.e(TAG, "mCallLogPlugin is null");
		}*/
		Log.i(TAG, "[setExtentionTextView] commd : " + commd);
		if (null != view && COMMD_FOR_RCS.equals(commd)) {
			//view.setText("1");
			//get the unread counter and set the text
		}
	}

	/**
	 * get the rcs-e icon on the Detail Actvitiy's action bar
	 * 
	 * @return if there isn't show rcs-e icon,return null.
	 */
	@Override
	public Drawable getRCSIcon(long id) {
		Log.i(TAG, "[updateRCSIconWithActionBar]");
		if (null != mContactPlugin) {
			if (mInstance.getContactPresence(id) == RCS_ENABLED_CONTACT) {
				Log.e(TAG, "getRCSIcon()-is rcs-e contact");
				return mContactPlugin.getRCSIcon();
			} else {
				Log.e(TAG, "getRCSIcon()-is not rcs-e contact");
				return null;
			}
		} else {
			Log.e(TAG, "getRCSIcon()-mCallLogPlugin is null");
			return null;
		}
	}
}
