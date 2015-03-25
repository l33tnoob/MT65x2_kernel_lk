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

package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import com.android.services.telephony.common.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;
import com.mediatek.rcse.api.Logger;

import android.os.Handler;
import android.os.Looper;
import android.os.HandlerThread;

import com.mediatek.incallui.ext.IInCallScreen;
import com.mediatek.incallui.ext.InCallUIExtension;
import com.mediatek.phone.ext.CallNotifierExtension ;
import com.orangelabs.rcs.platform.AndroidFactory;

public class RCSeInCallUIExtension extends InCallUIExtension {

    private static final String LOG_TAG = "RCSeInCallUIExtension";
    private static final boolean DBG = true;

    private Context mPluginContext;
    private RCSePhonePlugin mRCSePhonePlugin;
    private IInCallScreen mInCallScreenHost;
    private Activity mActivity;
    private CallManager mCM;
    private static final int ID_LARGE_AREA_SHARING = 1234562;
    private static final int ID_CENTER_AREA_SHARING = 1234563;

    private ShareFileCallScreenHost mShareFileHost;
    private ShareVideoCallScreenHost mShareVideoHost;

    private ICallScreenPlugIn mShareFilePlugIn;
    public ICallScreenPlugIn getmShareFilePlugIn() {
		return mShareFilePlugIn;
	}

	public void setmShareFilePlugIn(ICallScreenPlugIn mShareFilePlugIn) {
		this.mShareFilePlugIn = mShareFilePlugIn;
	}

	public ICallScreenPlugIn getVideoSharePlugIn() {
		if (DBG) {
            log("getVideoSharePlugIn(), mShareVideoPlugIn is " + mShareVideoPlugIn);
        }
		return mShareVideoPlugIn;
	}

	public static ViewGroup mWholeArea = null;
        public static ViewGroup mCenterArea = null;
    private ICallScreenPlugIn mShareVideoPlugIn;
    private static RCSeInCallUIExtension sInstance = null;

    public static void initialize(Context context, RCSePhonePlugin rcsePhonePlugin) {
        AndroidFactory.setApplicationContext(context);
        sInstance = new RCSeInCallUIExtension(context, rcsePhonePlugin);
    }

    public static RCSeInCallUIExtension getInstance() {
        return sInstance;
    }

    public static void resetDisplayArea() {
        if (DBG) {
            log("resetDisplayArea");
        }
       
        if(mWholeArea!=null)
        	mWholeArea.setVisibility(View.GONE);
        if(mCenterArea!=null)
        	mCenterArea.setVisibility(View.GONE); 
        if (DBG) {
            log("ShareFileScreenHost::resetDisplayArea() exit" + mWholeArea + "center area" + mCenterArea);
        }
        
    }

    protected RCSeInCallUIExtension(Context context, RCSePhonePlugin rcsePhonePlugin) {
        mPluginContext = context;
        mRCSePhonePlugin = rcsePhonePlugin;
    }

    protected Resources getHostResources() {
        return mActivity.getResources();
    }

    protected String getHostPackageName() {
        return mActivity.getPackageName();
    }

    public void onCreate(Bundle icicle, Activity activity, IInCallScreen inCallScreenHost
            ) {
    	CallManager cm = CallManager.getInstance();
    	if (DBG) {
            log("onCreate(), inCallScreen is " + activity);
        }
        mActivity = activity;
        mRCSePhonePlugin.setInCallScreenActivity(activity);
        mInCallScreenHost = inCallScreenHost;
        mCM = cm;
        mRCSePhonePlugin.setCallManager(cm);
        mShareFilePlugIn = new ImageSharingPlugin(mPluginContext);
        mShareVideoPlugIn = new VideoSharingPlugin(mPluginContext);
        mShareFileHost = new ShareFileCallScreenHost();
        mShareVideoHost = new ShareVideoCallScreenHost();
        if (DBG) {
            log("onCreate(), plugins are image " + mShareFilePlugIn + "video" + mShareVideoPlugIn);
        }
        // set host to plug-in
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.setCallScreenHost(mShareFileHost);
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.setCallScreenHost(mShareVideoHost);
        }
    }

    public void onDestroy(Activity activity) {
        if (DBG) {
            log("onDestroy(), inCallScreen is " + activity);
        }
        if (mActivity == activity) {
            mActivity = null;
        }
        if(mShareFilePlugIn != null){
            if (mShareFileHost == mShareFilePlugIn.getCallScreenHost()) {
                mShareFilePlugIn.setCallScreenHost(null);
            }
            mShareFilePlugIn.clearSavedDialogs();
        }
        if(mShareVideoPlugIn != null){
            if (mShareVideoHost == mShareVideoPlugIn.getCallScreenHost()) {
                mShareVideoPlugIn.setCallScreenHost(null);
            }
            mShareVideoPlugIn.clearSavedDialogs();
        }
    }

    public void onPause(Activity activity) {
        if (DBG) {
            log("onPause()");
        }
        if(mShareFilePlugIn != null){
            mShareFilePlugIn.saveAlertDialogs();
        }
        if(mShareVideoPlugIn != null){
            mShareVideoPlugIn.saveAlertDialogs();
        }
        
    }
    
    public void onResume(Activity activity) {
        if (DBG) {
            log("onResume()");
        }
        if(mShareFilePlugIn != null){
            mShareFilePlugIn.showAlertDialogs();
        }
        if(mShareVideoPlugIn != null){
            mShareVideoPlugIn.showAlertDialogs();
        }
        
    }
    
    // The function called by onPrepareOptionsMenu() to set visibility of menu
    // items
    public void setupMenuItems(Menu menu, Call call) {
    	RCSeUtils.setmFgCall(call);
        int state = call.getState();
        boolean canHold = call.can(Call.Capabilities.HOLD);
        boolean canAddCall = call.can(Call.Capabilities.ADD_CALL);
        if (DBG) {
            log("setupMenuItems()");
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        final MenuItem addMenu =
                menu.findItem(resource.getIdentifier("menu_add_call", "id", packageName));
        final MenuItem holdMenu =
                menu.findItem(resource.getIdentifier("menu_hold_voice", "id", packageName));
       

        if (RCSeUtils.canShare(mCM)) {
            if (DBG) {
                log("setupMenuItems(), can share");
            }
            if (isSharingVideo()) {
                if (DBG) {
                    log("setupMenuItems(), is sharing video");
                }
                // share video
                if (state == Call.State.IDLE) {
                    int size = menu.size();
                    for (int i = 0; i < size; ++i) {
                        menu.getItem(i).setVisible(false);
                    }                    
                    holdMenu.setVisible(canHold);
                    if (canHold) {
                        String title = null;
                        if (state == Call.State.ONHOLD) {
                            title = resource.getString(resource.getIdentifier("incall_toast_unhold",
                                    "string", packageName));
                        } else {
                            title = "Hold";//resource.getString(resource.getIdentifier("incall_toast_hold",
                                    //"string", packageName));
                        }
                        holdMenu.setTitle(title);
                    }
                    if (!ViewConfiguration.get(mActivity).hasPermanentMenuKey()) {
                        if (canAddCall) {
                            if (addMenu != null) {
                                addMenu.setVisible(true);
                            }
                        }
                    }
                }
            } else {
                if (DBG) {
                    log("setupMenuItems(), not share video");
                }
                holdMenu.setVisible(canHold);
                String title = null;
                    if (state == Call.State.ONHOLD) {
                    title = resource.getString(resource.getIdentifier("incall_toast_unhold",
                            "string", packageName));
                    } else {
                    title = "Hold";//resource.getString(resource.getIdentifier("incall_toast_hold",
                            //"string", packageName));
                }
                holdMenu.setTitle(title);
            }
        }
    }

    // The function called to handle option menu item clicking event
    public boolean handleMenuItemClick(MenuItem menuItem) {
        Resources resource = getHostResources();
        String packageName = getHostPackageName();

        if (menuItem.getItemId() == resource.getIdentifier("menu_hold_voice", "id", packageName)) {
            if (DBG) {
                log("hold voice menu item is clicked");
            }
            if ((isTransferingFile())) {
                mShareFilePlugIn.stop();
            } else if (isSharingVideo()) {
                mShareVideoPlugIn.stop();
            }
        }
        return false;
    }

    // The function called when phone state is changed
    public boolean onPhoneStateChanged(CallManager cm) {
        if (DBG) {
            log("onPhoneStateChanged(), cm = ");
        }
        if (RCSeUtils.canShareFromCallState(cm)) {
            String number = RCSeUtils.getRCSePhoneNumber(cm);
            if (null != number) {
                if (null != mShareFilePlugIn) {
                    mShareFilePlugIn.registerForCapabilityChange(number);
                }
                if (null != mShareVideoPlugIn) {
                    mShareVideoPlugIn.registerForCapabilityChange(number);
                }
            }
        }
        if (RCSeUtils.shouldStop(cm)) {
            if ((isTransferingFile())) {
                mShareFilePlugIn.stop();
            } else if (isSharingVideo()) {
                mShareVideoPlugIn.stop();
            }
        }
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.onPhoneStateChange(cm);
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.onPhoneStateChange(cm);
        }
        return false;
    }

    public boolean dismissDialogs() {
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.dismissDialog();
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.dismissDialog();
        }
        return false;
    }

    public static ICallScreenPlugIn getShareFilePlugIn() {
        return getInstance().mShareFilePlugIn;
    }

    public static ICallScreenPlugIn getShareVideoPlugIn() {
    	if (DBG) {
            log("getShareVideoPlugIn(), mShareVideoPlugIn is " + getInstance().mShareVideoPlugIn);
        }
        return getInstance().mShareVideoPlugIn;
    }

    public static boolean isCapabilityToShare(String number) {
        if (DBG) {
            log("isCapabilityToShare(), number = " + number);
        }
        ICallScreenPlugIn filePlugin = getShareFilePlugIn();
        ICallScreenPlugIn videoPlugin = getShareVideoPlugIn();
        if (null == filePlugin && null == videoPlugin) {
            if (DBG) {
                log("both plug-in are null, no capability");
            }
            return false;
        } else if (null != filePlugin && filePlugin.getCapability(number)) {
            if (DBG) {
                log("Share file plugIn has capability");
            }
            return true;
        } else if (null != videoPlugin && videoPlugin.getCapability(number)) {
            if (DBG) {
                log("Share video plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("Neither plug-ins have capability");
            }
            return false;
        }
    }

    public static boolean isCapabilityToShareImage(String number) {
        if (DBG) {
            log("sharing isCapabilityToShareImage(), number = " + number);
        }
        ICallScreenPlugIn filePlugin = getShareFilePlugIn();
        if (null == filePlugin) {
            if (DBG) {
                log("sharing file plug-in is null, no file share capability");
            }
            return false;
        } else if (filePlugin.isImageShareSupported(number)) {
            if (DBG) {
                log("sharing Share file plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("sharing file plug-in has no capability");
            }
            return false;
        }
    }

    public static boolean isCapabilityToShareVideo(String number) {
        if (DBG) {
            log("isCapabilityToShareVideo(), number = " + number);
        }
        ICallScreenPlugIn videoPlugin = getShareVideoPlugIn();
        if (null == videoPlugin) {
            if (DBG) {
                log("Video sharing plug-in is null, no Video Sharing capability");
            }
            return false;
        } else if (videoPlugin.isVideoShareSupported(number)) {
            if (DBG) {
                log("Share video plugIn has capability");
            }
            return true;
        } else {
            if (DBG) {
                log("Video plug-in has no capability");
            }
            return false;
        }
    }
    
    public static boolean isTransferingFile() {
        ICallScreenPlugIn plugin = getShareFilePlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_FILE_STATE_TRANSFERING == plugin.getState();
    }

    public static boolean isDisplayingFile() {
        ICallScreenPlugIn plugin = getShareFilePlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_FILE_STATE_DISPLAYING == plugin.getState();
    }

    public static boolean isSharingVideo() {
        ICallScreenPlugIn plugin = getShareVideoPlugIn();
        if (null == plugin) {
            return false;
        }
        return Constants.SHARE_VIDEO_STATE_SHARING == plugin.getState();
    }

    public boolean onDisconnect(String number) {
        if (DBG) {
            log("onDisconnect(), number =" + number);
        }
        dismissDialogs();
        if (null != mShareFilePlugIn) {
            mShareFilePlugIn.stop();
            mShareFilePlugIn.unregisterForCapabilityChange(number);
            mShareFilePlugIn.clearSavedDialogs();
        }
        if (null != mShareVideoPlugIn) {
            mShareVideoPlugIn.stop();
            mShareVideoPlugIn.unregisterForCapabilityChange(number);
            mShareVideoPlugIn.clearSavedDialogs();
        }
        return false;
    }

    public boolean updateScreen(CallManager callManager, boolean isForegroundActivity) {
        if (RCSeUtils.canShare(callManager)) {
            if (isSharingVideo() || isDisplayingFile()) {
                return false;
            }
        }
        Resources resource = getHostResources();
        String packageName = getHostPackageName();
        View inCallTouchUi =
                (View) mActivity.findViewById(resource.getIdentifier("bottomButtons", "id",
                        packageName));
        if (null != inCallTouchUi) {
            inCallTouchUi.setVisibility(View.VISIBLE);
        }
        return false;
    }

    /*
     * public boolean handleOnscreenButtonClick(int id) { switch (id) { case
     * R.id.endSharingVideo: if (DBG)
     * log("end sharing video button is clicked"); if (null !=
     * mShareVideoPlugIn) { mShareVideoPlugIn.stop(); } return true; case
     * R.id.shareFileButton: if (DBG) log("share file button is clicked"); if
     * (null != mShareFilePlugIn) { String phoneNumber =
     * RCSeUtils.getRCSePhoneNumber(mCM); if (null != phoneNumber) {
     * mShareFilePlugIn.start(phoneNumber); } } return true; case
     * R.id.shareVideoButton: if (DBG) log("share video button is clicked"); if
     * (null != mShareVideoPlugIn) { String phoneNumber =
     * RCSeUtils.getRCSePhoneNumber(mCM); if (null != phoneNumber) {
     * mShareVideoPlugIn.start(phoneNumber); } } return true; } return false; }
     */

    public class ShareFileCallScreenHost implements ICallScreenHost {

        public ShareFileCallScreenHost() {
        }

        public ViewGroup requestAreaForDisplay() {
            if (DBG) {
                log("ShareFileCallScreenHost::requestAreaForDisplay()");
            }
            mCenterArea =
            		(ViewGroup) mActivity.findViewById(ID_CENTER_AREA_SHARING);
            mCenterArea.setVisibility(View.VISIBLE);
            if (DBG) {
                log("ShareFileScreenHost::requestAreaForDisplay() exit" + mCenterArea);
            }
            return mCenterArea;
        }

        public void onStateChange(final int state) {
            if (DBG) {
                log("ShareFileCallScreenHost::onStateChange(), state = " + state);
            }
            if (null != mInCallScreenHost) {
                mInCallScreenHost.requestUpdateScreen();
            }
        }

        public void onCapabilityChange(String number, boolean isSupport) {
        	 final String contact = number;
        	 Logger.w(LOG_TAG, "onCapabilitiesChanged  contact0: " + contact);
            if (DBG) {
                log("options ShareFileCallScreenHost::onCapabilityChange(), number = " + number
                        + ", isSupport = " + isSupport);
            }
            if (null != mInCallScreenHost) {
            	Handler refresh = new Handler(Looper.getMainLooper());
            	refresh.post(new Runnable() {
            	    public void run()
            	    {
            	        Logger.w(LOG_TAG, "onCapabilitiesChanged  contact: " + contact);
                mInCallScreenHost.requestUpdateScreen();
            }
            	});
               // mInCallScreenHost.requestUpdateScreen();
            }
        }

        public Activity getCallScreenActivity() {
            return mActivity;
        }
    }

    public class ShareVideoCallScreenHost implements ICallScreenHost {

        public ShareVideoCallScreenHost() {
        }

        public ViewGroup requestAreaForDisplay() {
            if (DBG) {
                log("ShareVideoCallScreenHost::requestAreaForDisplay()");
            }
            Resources resource = getHostResources();
            String packageName = getHostPackageName();
            mWholeArea =
            		(ViewGroup) mActivity.findViewById(ID_LARGE_AREA_SHARING);
            mWholeArea.setVisibility(View.VISIBLE);
            if (DBG) {
                log("ShareVideoCallScreenHost::requestAreaForDisplay() exit" + mWholeArea);
            }
            return mWholeArea;
        }

        public void onStateChange(final int state) {
            if (DBG) {
                log("ShareVideoCallScreenHost::onStateChange(), state = " + state);
            }
            if (null != mInCallScreenHost) {

                mInCallScreenHost.requestUpdateScreen();
            }
        }

        public void onCapabilityChange(String number, boolean isSupport) {
        	final String contact = number;
       	 	Logger.w(LOG_TAG, "onCapabilitiesChanged  contact0: " + contact);
            if (DBG) {
                log("options ShareVideoCallScreenHost::onCapabilityChange(), number = " + number
                        + ", isSupport = " + isSupport);
            }
            if (null != mInCallScreenHost) {
            	Handler refresh = new Handler(Looper.getMainLooper());
            	refresh.post(new Runnable() {
            	    public void run()
            	    {
                   	 	Logger.w(LOG_TAG, "onCapabilitiesChanged  contact: " + contact);
                mInCallScreenHost.requestUpdateScreen();
            }
            	});
               // mInCallScreenHost.requestUpdateScreen();
            }
        }

        public Activity getCallScreenActivity() {
            return mActivity;
        }
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
