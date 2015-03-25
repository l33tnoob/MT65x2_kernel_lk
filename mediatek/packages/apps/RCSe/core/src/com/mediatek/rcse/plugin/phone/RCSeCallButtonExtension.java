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

import java.util.HashMap;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PorterDuff.Mode;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.CompoundButton;
import android.widget.ImageButton;
import android.widget.ToggleButton;

import com.android.services.telephony.common.Call;

import com.android.internal.telephony.CallManager;

import com.mediatek.incallui.ext.CallButtonExtension;

import com.orangelabs.rcs.R;

public class RCSeCallButtonExtension extends CallButtonExtension implements
        View.OnClickListener {

    private static final String LOG_TAG = "RCSeCallButtonExtension";
    private static final boolean DBG = true;

    private RCSePhonePlugin mRCSePhonePlugin;
    // private ViewGroup mEndSharingVideoButtonWrapper;
    private ImageButton mEndSharingVideoButton;
    private ImageButton mShareFileButton;
    private ImageButton mShareVideoButton;
    private ViewGroup mInCallControlArea;
    private ToggleButton mDialpadButton;
    private ImageButton mHoldButton;
    private ImageButton mMuteButton;
    private ImageButton mAudioButton;
    private View mShareFileShareVideoSpacer;
    private View mLeftDialpadSpacer;
    private Context mPluginContext;
    private static final int ID_FT_BUTTON = 15432345;
    private static final int ID_VIDEO_BUTTON = 18432120;

    public RCSeCallButtonExtension(Context context,RCSePhonePlugin rcsePhonePlugin) {
        mPluginContext = context;
        mRCSePhonePlugin = rcsePhonePlugin;
    }

    protected Resources getHostResources() {
        return mRCSePhonePlugin.getInCallScreenActivity().getResources();
    }

    protected String getHostPackageName() {
        return mRCSePhonePlugin.getInCallScreenActivity().getPackageName();
    }

    public void onViewCreated(Context context, View inCallTouchUi) {
        if (DBG) {
            log("onFinishInflate()...");
        }

        // !!!! Todo: add end sharing button, share file, share video button
        // dynamically
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        mInCallControlArea =
                (ViewGroup) inCallTouchUi.findViewById(resource.getIdentifier("callButtonContainer",
                        "id", packageName));
       
        /*mEndSharingVideoButton =
                (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier("endSharingVideo",
                        "id", packageName));
        mEndSharingVideoButton.setOnClickListener(this);*/
        // mEndSharingVideoButtonWrapper = (ViewGroup)
        // inCallTouchUi.findViewById(resource.getIdentifier("endSharingVideoWrapper",
        // "id", packageName));
        //mShareFileButton = (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier(
               // "shareFileButton", "id", packageName));
        mShareFileButton = new ImageButton(context);
        mShareFileButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.FILL_PARENT));
        Drawable shareFileDrawable = mPluginContext.getResources().getDrawable(
                R.drawable.btn_share_file);
        mShareFileButton.setEnabled(true);
        mShareFileButton.setClickable(true);
        if (shareFileDrawable != null) {
            if (DBG) {
                log("onFinishInflate()2-image share drawable is not null");
            }
        mShareFileButton.setBackgroundDrawable(shareFileDrawable);
        } else {
            if (DBG) {
                log("onFinishInflate()-image share drawable is null");
            }
        }       
        mInCallControlArea.addView(mShareFileButton,8);
        mShareFileButton.setOnClickListener(this);
        mShareFileButton.setVisibility(View.GONE);
        mShareFileButton.setId(ID_FT_BUTTON);
       // mShareVideoButton = (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier(
              //  "shareVideoButton", "id", packageName));
        mShareVideoButton = new ImageButton(context);
        mShareVideoButton.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.FILL_PARENT));
        mShareVideoButton.setEnabled(true);
        mShareVideoButton.setClickable(true);
        mShareVideoButton.setVisibility(View.GONE);
        mShareVideoButton.setId(ID_VIDEO_BUTTON);
        Drawable shareVideoDrawable = mPluginContext.getResources().getDrawable(
                R.drawable.btn_share_video);
        if (shareVideoDrawable != null) {
            if (DBG) {
                log("onFinishInflate()-video2 share drawable is not null");
            }
        mShareVideoButton.setBackgroundDrawable(shareVideoDrawable);
        } else {
            if (DBG) {
                log("onFinishInflate()-video share drawable is null");
            }
        }
        
        View verticalSpacerFile = new View(context);
        verticalSpacerFile.setLayoutParams(new LayoutParams(35 , LayoutParams.FILL_PARENT));
        
        mInCallControlArea.addView(verticalSpacerFile,9);
        
        mInCallControlArea.addView(mShareVideoButton,10);
        
        View verticalSpacerVideo = new View(context);
        verticalSpacerVideo.setLayoutParams(new LayoutParams(2, LayoutParams.FILL_PARENT));

        
        mInCallControlArea.addView(verticalSpacerVideo,11);
        mShareVideoButton.setOnClickListener(this);
        mDialpadButton =
                (ToggleButton) inCallTouchUi.findViewById(resource.getIdentifier("dialpadButton",
                        "id", packageName));
        mHoldButton =
                (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier("holdButton",
                        "id", packageName));
        mMuteButton =
                (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier("muteButton",
                        "id", packageName));
        mAudioButton =
                (ImageButton) inCallTouchUi.findViewById(resource.getIdentifier("audioButton",
                        "id", packageName));
        mShareFileShareVideoSpacer =
                inCallTouchUi.findViewById(resource.getIdentifier("shareFileShareVideoSpacer",
                        "id", packageName));
        mLeftDialpadSpacer =
                inCallTouchUi.findViewById(resource.getIdentifier("leftDialpadSpacer", "id",
                        packageName));
        if (DBG) {
            log("onFinishInflate() values buttons are -file2 " + mShareFileButton + "InCallControlarea " + mInCallControlArea + "mvideobutton" + mShareVideoButton + "dialpad" + mDialpadButton +"mHoldButton" + mHoldButton
            		+ "mMuteButton" + mMuteButton + "mAudioButton" + mAudioButton + "mLeftDialpadSpacer" + mLeftDialpadSpacer);
        }
    }

    private void setCompoundButtonBackgroundTransparency(ImageButton button, int transparency) {/*
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        LayerDrawable layers = (LayerDrawable) button.getBackground();
        if (null != layers) {
            Drawable drawable =
                    layers.findDrawableByLayerId(resource.getIdentifier("compoundBackgroundItem",
                            "id", packageName));
            if (drawable != null) {
                drawable.setAlpha(transparency);
            } else {
                if (DBG) {
                    log("setCompoundButtonBackgroundTransparency(), drawable is null!");
                }
            }
        }
    */}

    private void setImageButtonGray(ImageButton button, boolean  isImageShare, CallManager cm) {
    	Drawable originalIcon;
    	Drawable grayScaleIcon;
    	if(isImageShare){
    		originalIcon = mPluginContext.getResources().getDrawable(R.drawable.richcall_image_share);
    		if(originalIcon != null){
    			grayScaleIcon = RCSeUtils.canShareImage(cm) ? originalIcon : convertDrawableToGrayScale(originalIcon);
    			button.setImageDrawable(grayScaleIcon);
    		}
    	}else{
    		originalIcon = mPluginContext.getResources().getDrawable(R.drawable.richcall_video_share);
    		if(originalIcon != null)
    		{
    			grayScaleIcon = RCSeUtils.canShareVideo(cm) ? originalIcon : convertDrawableToGrayScale(originalIcon);
    			button.setImageDrawable(grayScaleIcon);
    		}
    	}
    }
    
	private static Drawable convertDrawableToGrayScale(Drawable drawable) {
	    if (drawable == null) {
	        return null;
	    }
	    Drawable res = drawable.mutate();
	    res.setColorFilter(Color.GRAY, Mode.SRC_IN);
	    return res;
	}
	
    private void updateBottomButtons(CallManager cm) {
        if (RCSeUtils.canShare(cm)) {
            if (RCSeInCallUIExtension.isSharingVideo()) {
                if (DBG) {
                    log("updateBottomButtons(), is sharing video");
                }
                if (null != mEndSharingVideoButton) {
                    mEndSharingVideoButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareFileButton) {
                    mShareFileButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareVideoButton) {
                    mShareVideoButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareFileShareVideoSpacer) {
                    mShareFileShareVideoSpacer.setVisibility(View.VISIBLE);
                }
                if (null != mHoldButton) {
                    mHoldButton.setVisibility(View.GONE);
                }
                if (null != mInCallControlArea) {
                    Drawable drawable = mInCallControlArea.getBackground();
                    if (drawable != null) {
                        drawable.setAlpha(200);
                    }
                }
                setCompoundButtonBackgroundTransparency(mMuteButton, 150);
                setCompoundButtonBackgroundTransparency(mAudioButton, 150);
            } else {
                if (DBG) {
                    log("updateBottomButtons(), not sharing video");
                }
                if (null != mEndSharingVideoButton) {
                    mEndSharingVideoButton.setVisibility(View.GONE);
                }
                if (null != mShareFileButton) {
                	if(!RCSeUtils.canShareImage(cm)){
						if (DBG) {
                    		log("sharing updateBottomButtons() can not share image");
                		}
                		mShareFileButton.setEnabled(false);
                        mShareFileButton.setClickable(false);
                	}
                    else
                    {
                        if (DBG) {
                    		log("sharing updateBottomButtons() can share image");
                		}
                        mShareFileButton.setEnabled(true);
                        mShareFileButton.setClickable(true);	
                    }
                    mShareFileButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareVideoButton) {
                	if(!RCSeUtils.canShareVideo(cm)){
                		mShareVideoButton.setEnabled(false);
                        mShareFileButton.setClickable(false);
                	}
                    else
                    {
                        mShareVideoButton.setEnabled(true);
                        mShareFileButton.setClickable(true);
                    }
                    mShareVideoButton.setVisibility(View.VISIBLE);
                }
                if (null != mShareFileShareVideoSpacer) {
                    mShareFileShareVideoSpacer.setVisibility(View.VISIBLE);
                }
                if (null != mHoldButton) {
                    mHoldButton.setVisibility(View.GONE);
                }
                if (null != mInCallControlArea) {
                    Drawable drawable = mInCallControlArea.getBackground();
                    if (drawable != null) {
                        //drawable.setAlpha(255);
                    }
                }
                if (null != mShareFileButton) {
                	setImageButtonGray(mShareFileButton, true, cm);
                }
                if (null != mShareVideoButton) {
                	setImageButtonGray(mShareVideoButton, false, cm);
                }
                setCompoundButtonBackgroundTransparency(mMuteButton, 255);
                setCompoundButtonBackgroundTransparency(mAudioButton, 255);
            }
            if (null != mDialpadButton) {
                mDialpadButton.setVisibility(View.GONE);
            }
            if (null != mLeftDialpadSpacer) {
                mLeftDialpadSpacer.setVisibility(View.GONE);
            }
        } else {
            if (null != mShareFileButton) {
				 if (DBG) {
                    log("sharing updateBottomButtons() visibility gone");
                }
                mShareFileButton.setVisibility(View.GONE);
            }
            if (null != mShareVideoButton) {
                mShareVideoButton.setVisibility(View.GONE);
            }
            if (null != mDialpadButton) {
                mDialpadButton.setVisibility(View.VISIBLE);
            }
            if (null != mLeftDialpadSpacer) {
                mLeftDialpadSpacer.setVisibility(View.VISIBLE);
            }
            if (null != mEndSharingVideoButton) {
                mEndSharingVideoButton.setVisibility(View.GONE);
            }
            if (null != mHoldButton) {
                mHoldButton.setVisibility(View.VISIBLE);
            }
            if (null != mInCallControlArea) {
                Drawable drawable = mInCallControlArea.getBackground();
                if (drawable != null) {
                    drawable.setAlpha(255);
                }
            }
            setCompoundButtonBackgroundTransparency(mMuteButton, 255);
            setCompoundButtonBackgroundTransparency(mAudioButton, 255);
        }
    }

    public void onStateChange(Call  call, HashMap<Integer,Call> mCallMap) {
        if (DBG) {
            log("updateState()");
        }
        RCSeUtils.setmFgCall(call);
        CallManager cm = CallManager.getInstance();
        updateBottomButtons(cm);
    }

    public void onClick(View view) {
        int id = view.getId();
        if (DBG) {
            log("onClick(View " + view + ", id " + id + ")...");
        }

        Resources resource = getHostResources();
        String packageName = getHostPackageName();

        if (id == resource.getIdentifier("endSharingVideo", "id", packageName)) {
            if (DBG) {
                log("end sharing video button is clicked");
            }
            if (null != RCSeInCallUIExtension.getShareVideoPlugIn()) {
                RCSeInCallUIExtension.getShareVideoPlugIn().stop();
            }
        } else if (id == ID_FT_BUTTON) {
            if (DBG) {
                log("share file button is clicked");
            }
            if (null != RCSeInCallUIExtension.getShareFilePlugIn()) {
                String phoneNumber =
                        RCSeUtils.getRCSePhoneNumber(mRCSePhonePlugin.getCallManager());
                if (null != phoneNumber) {
                    RCSeInCallUIExtension.getShareFilePlugIn().start(phoneNumber);
                }
            }
        } else if (id == ID_VIDEO_BUTTON) {
            if (DBG) {
                log("share video button is clicked");
            }
            if (null != RCSeInCallUIExtension.getInstance().getVideoSharePlugIn()) {
                String phoneNumber =
                        RCSeUtils.getRCSePhoneNumber(mRCSePhonePlugin.getCallManager());
                log("phone number is" + phoneNumber);
                if (null != phoneNumber) {
                    RCSeInCallUIExtension.getShareVideoPlugIn().start(phoneNumber);
                }
            }
            else
            {
            	log("share video plugin is null");
            }
        }
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
