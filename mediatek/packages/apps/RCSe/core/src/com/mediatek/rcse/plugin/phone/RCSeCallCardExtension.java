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

package com.mediatek.rcse.plugin.phone;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.incallui.ext.CallCardExtension;
import com.android.services.telephony.common.Call;

import com.orangelabs.rcs.R;


public class RCSeCallCardExtension extends CallCardExtension implements View.OnClickListener {

    private static final String LOG_TAG = "RCSeCallCardExtension";
    private static final boolean DBG = true;
	private static final int ID_LARGE_AREA_SHARING = 1234562;
        private static final int ID_CENTER_AREA_SHARING = 1234563;

    private RCSePhonePlugin mRCSePhonePlugin;
    private Context mPluginContext;
    private View mCallCard;
    private TextView mCallStateLabel;
    private ImageView mPhoto;
    private ViewGroup mPrimaryCallBanner;
    private ViewGroup mCenterArea;
    private ViewGroup mWholeArea;
    private View mPrimaryCallInfo;
    private TextView mPhoneNumberGeoDescription;

    protected float mDensity;
    private boolean mIsCenterAreaFullScreen;

    public RCSeCallCardExtension(Context pluginContext, RCSePhonePlugin rcsePhonePlugin) {
        mRCSePhonePlugin = rcsePhonePlugin;
        mPluginContext = pluginContext;
    }

    protected Resources getHostResources() {
        return mRCSePhonePlugin.getInCallScreenActivity().getResources();
    }

    protected String getHostPackageName() {
        return mRCSePhonePlugin.getInCallScreenActivity().getPackageName();
    }
 
    public void onViewCreated(Context context, View callCard) {    	
        if (DBG) {
            log("onViewCreated()");
        }
        mCallCard = callCard;

        Resources resource = getHostResources();
        String packageName = getHostPackageName();

       /* mCenterArea =
                (ViewGroup) callCard.findViewById(resource.getIdentifier("centerAreaForSharing",
                        "id", packageName));*/
        /*mWholeArea =
                (ViewGroup) callCard.findViewById(resource.getIdentifier("largeAreaForSharing",
                        "id", packageName));*/
        if (DBG) {
            log("onViewCreated() PackageName = " + packageName + "resource =" + resource + "callCrad =" + callCard);
        }
        RelativeLayout mWholeArea = new RelativeLayout(context);
        mWholeArea.setId(ID_LARGE_AREA_SHARING);
        RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.FILL_PARENT,
                RelativeLayout.LayoutParams.FILL_PARENT);
        mWholeArea.setLayoutParams(params);
        ((ViewGroup) callCard).addView(mWholeArea, 1, params);
        mWholeArea.setVisibility(View.GONE);
        mWholeArea.setOnClickListener(this);

        RelativeLayout mCenterArea = new RelativeLayout(context);
        mCenterArea.setId(ID_CENTER_AREA_SHARING);
        RelativeLayout.LayoutParams centerParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT);
        centerParams.addRule(RelativeLayout.CENTER_IN_PARENT);
        mCenterArea.setLayoutParams(centerParams);
        ((ViewGroup) callCard).addView(mCenterArea, 2, centerParams);
        mCenterArea.setVisibility(View.GONE);

        mCallStateLabel =
                (TextView) callCard.findViewById(resource.getIdentifier("callStateLabel", "id",
                        packageName));
        mPhoto =
                (ImageView) callCard.findViewById(resource
                        .getIdentifier("photo", "id", packageName));
        mPrimaryCallBanner =
                (ViewGroup) callCard.findViewById(resource.getIdentifier("call_banner_1", "id",
                        packageName));
        mPrimaryCallInfo =
                callCard.findViewById(resource
                        .getIdentifier("primary_call_info", "id", packageName));
        mPhoneNumberGeoDescription =
                (TextView) callCard.findViewById(resource.getIdentifier(
                        "location", "id", packageName));

        mDensity = resource.getDisplayMetrics().density;
        
        if (DBG) {
            log("onViewCreated() mcallstatelabel = " + mCallStateLabel+ "photo= " + mPhoto + "primaryCallinfo =" + mPrimaryCallInfo + " geoDesc =" + mPhoneNumberGeoDescription);
        }
    }

    /*public boolean updateCallInfoLayout(PhoneConstants.State state) {
        if (DBG) {
            log("updateCallInfoLayout(), state = " + state);
        }
        if (shouldResetLayoutMargin()) {
            ViewGroup.MarginLayoutParams callInfoLp =
                    (ViewGroup.MarginLayoutParams) mCallCard.getLayoutParams();
            callInfoLp.bottomMargin = 0; // Equivalent to setting
            // android:layout_marginBottom in XML
            if (DBG) {
                log("  ==> callInfoLp.bottomMargin: 0");
            }
            mCallCard.setLayoutParams(callInfoLp);
            return true;
        } else {
            return false;
        }
    }*/

    /**
     * Updates the state of all UI elements on the CallCard, based on the
     * current state of the phone.
     */
    public void onStateChange(Call call) {
    	CallManager cm = CallManager.getInstance();
        RCSeUtils.setmFgCall(call);
        if (RCSeUtils.canShare(cm)) {
            if (DBG) {
                log("updateState(), can share");
            }
            // have capability to share
            Drawable drawable =
                    mPluginContext.getResources().getDrawable(R.drawable.ic_rcse_indicaton);
            if (null != drawable) {
                drawable.setBounds(0, 0, drawable.getIntrinsicWidth(), drawable
                        .getIntrinsicHeight());
            } else {
                if (DBG) {
                    log("rcse indication icon drawable is null");
                }
            }
            if (null != mPhoneNumberGeoDescription
                    && View.VISIBLE == mPhoneNumberGeoDescription.getVisibility()) {
                mPhoneNumberGeoDescription.setCompoundDrawables(drawable, null, null, null);
                mPhoneNumberGeoDescription.setCompoundDrawablePadding((int) (mDensity * 5));
            } else {
                if (null != mCallStateLabel) {
                    mCallStateLabel.setCompoundDrawables(drawable, null, null, null);
                    mCallStateLabel.setCompoundDrawablePadding((int) (mDensity * 5));
                }
            }
            if (RCSeInCallUIExtension.isTransferingFile()) {
                if (DBG) {
                    log("updateState(), is transfering file");
                }
                // share file
                if (null != mPhoto) {
                    mPhoto.setVisibility(View.INVISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.VISIBLE);
                }
                if (mIsCenterAreaFullScreen) {
                    fullDisplayCenterArea(false);
                }
            } else if (RCSeInCallUIExtension.isDisplayingFile()) {
                if (DBG) {
                    log("updateState(), is displaying file");
                }
                if (null != mPhoto) {
                    mPhoto.setVisibility(View.INVISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.VISIBLE);
                }
            } else {
                if (DBG) {
                    log("updateState(), not sharing file");
                }
                // not share file
                if (null != mPhoto) {
                    mPhoto.setVisibility(View.VISIBLE);
                }
                if (null != mCenterArea) {
                    mCenterArea.setVisibility(View.GONE);
                }
                if (mIsCenterAreaFullScreen) {
                    fullDisplayCenterArea(false);
                }
            }

            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("updateState(), is sharing video");
                }
                // share video
                if (null != mWholeArea) {
                    mWholeArea.setVisibility(View.VISIBLE);
                }
                if (null != mPrimaryCallInfo) {
                    mPrimaryCallInfo.setVisibility(View.GONE);
                }
            } else {
                if (DBG) {
                    log("updateState(), not sharing video");
                }
                if (null != mWholeArea) {
                    mWholeArea.setVisibility(View.GONE);
                }
                if (null != mPrimaryCallInfo) {
                    mPrimaryCallInfo.setVisibility(View.VISIBLE);
                }
                hideStatusBar(false);
            }

            if (RCSeInCallUIExtension.isSharingVideo()
                    || (RCSeInCallUIExtension.isDisplayingFile() && mIsCenterAreaFullScreen)) {
                hideStatusBar(true);
            } else {
                hideStatusBar(false);
            }
        } else {
            if (DBG) {
                log("updateState(), can not share");
            }
            // Clear out any icons
            if (null != mPhoneNumberGeoDescription
                    && View.VISIBLE == mPhoneNumberGeoDescription.getVisibility()) {
                mPhoneNumberGeoDescription.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
            } else {
                if (null != mCallStateLabel) {
                    mCallStateLabel.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
                }
            }
            if (null != mCenterArea) {
                mCenterArea.setVisibility(View.GONE);
            }
            if (null != mWholeArea) {
                mWholeArea.setVisibility(View.GONE);
            }
            hideStatusBar(false);
            if (mIsCenterAreaFullScreen) {
                fullDisplayCenterArea(false);
            }
        }
        RCSeInCallUIExtension.getInstance().onPhoneStateChanged(cm);
    }

    // View.OnClickListener implementation
    public void onClick(View view) {
        int id = view.getId();
        if (DBG) {
            log("onClick(View " + view + ", id " + id + ")...");
        }

        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        View inCallTouchUi =
                mRCSePhonePlugin.getInCallScreenActivity().findViewById(
                        resource.getIdentifier("inCallTouchUi", "id", packageName));

        if (id == resource.getIdentifier("largeAreaForSharing", "id", packageName)) {
            int visibility = inCallTouchUi.getVisibility();
            if (DBG) {
                log("large area for sharing is clicked, visibility is " + visibility);
            }
            inCallTouchUi.setVisibility(visibility == View.VISIBLE ? View.INVISIBLE : View.VISIBLE);
            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("is sharing video, so need to set stored video ui");
                }
               
            } else {
                if (DBG) {
                    log("is sharing image, so no need to set stored video ui");
                }
            }
        } else if (id == resource.getIdentifier("centerAreaForSharing", "id", packageName)) {
            if (DBG) {
                log("center area for sharing is clicked");
            }
            if (RCSeInCallUIExtension.isDisplayingFile()) {
                hideStatusBar(!mIsCenterAreaFullScreen);
                inCallTouchUi
                        .setVisibility(mIsCenterAreaFullScreen ? View.INVISIBLE : View.VISIBLE);
                fullDisplayCenterArea(!mIsCenterAreaFullScreen);
            }
        }
    }

    private void hideStatusBar(final boolean isHide) {
        if (DBG) {
            log("hideStatusBar(), isHide = " + isHide);
        }
        WindowManager.LayoutParams attrs =
                mRCSePhonePlugin.getInCallScreenActivity().getWindow().getAttributes();
        if (isHide) {
            attrs.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        } else {
            attrs.flags &= (~WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        mRCSePhonePlugin.getInCallScreenActivity().getWindow().setAttributes(attrs);
    }

    private void fullDisplayCenterArea(final boolean isFullDisplay) {
        if (DBG) {
            log("fullDisplayCenterArea(), isFullDisplay = " + isFullDisplay);
        }
        if (isFullDisplay) {
            mIsCenterAreaFullScreen = true;
            //mPrimaryCallBanner.setVisibility(View.GONE);
            mCallStateLabel.setVisibility(View.GONE);
        } else {
            mIsCenterAreaFullScreen = false;
            //mPrimaryCallBanner.setVisibility(View.VISIBLE);
            mCallStateLabel.setVisibility(View.VISIBLE);
        }
    }

    private boolean shouldResetLayoutMargin() {
        if (DBG) {
            log("shouldResetLayoutMargin()");
        }
        if (!RCSeUtils.canShare(mRCSePhonePlugin.getCallManager())) {
            if (DBG) {
                log("Can not share, so no need reset layout margin");
            }
            return false;
        }
        if (RCSeInCallUIExtension.isSharingVideo()) {
            if (DBG) {
                log("is sharing video, so need reset layout margin");
            }
            return true;
        }
        if (RCSeInCallUIExtension.isDisplayingFile() && mIsCenterAreaFullScreen) {
            if (DBG) {
                log("is displaying file and full screen, so need reset layout margin");
            }
            return true;
        }
        return false;
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }

}
