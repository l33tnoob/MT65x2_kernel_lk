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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Toast;

import com.android.internal.telephony.CallManager;
import com.android.services.telephony.common.Call;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.CapabilitiesChangeListener;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ClientApiIntents;
import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * This class defined to abstract sharing class
 */
public class SharingPlugin implements ICallScreenPlugIn, CapabilitiesChangeListener {

    private static final String TAG = "SharingPlugin";

    /* package */static final String SERVICE_STATUS = "com.orangelabs.rcs.SERVICE_STATUS";
    /* package */static final String SERVICE_REGISTRATION =
            "com.orangelabs.rcs.SERVICE_REGISTRATION";
    private static final String CORE_SERVICE_STATUS = "status";
    private static final int INVALID_VALUE = -1;

    Context mContext;
    ICallScreenHost mCallScreenHost = null;
    Handler mMainHandler = null;
    WakeLock mWakeLock = null;
    final AtomicInteger mWakeLockCount = new AtomicInteger(0);
    ViewGroup mDisplayArea = null;
    int mRichCallStatus = RichCallStatus.DISCONNECTED;
    RichCallApi mRichCallApi = null;
    ISharingPlugin mInterface = null;

    // CountDownLatch used to synchronize initialize
    CountDownLatch mCountDownLatch = new CountDownLatch(1);

    String mNumber = null;
    boolean mIsOnHold = false;

    private RichCallApiListener mRichCallApiListener = null;

    private RcseCoreServiceStatusReceiver mRcseCoreServiceStatusReceiver = null;
    private boolean mIsRegistered = false;

    static final class RichCallStatus {
        static final int DISCONNECTED = 0;
        static final int CONNECTING = 1;
        static final int CONNECTED = 2;
    }

    /**
     * Constructor
     * 
     * @param ctx Application context
     */
    public SharingPlugin(final Context context) {
    	Logger.d(TAG, "SharingPlugin() entry!");
        mContext = context;
        PluginApiManager.initialize(mContext);
        mRichCallApi = new RichCallApi(mContext);
        connectRichCallApi();
        mMainHandler = new Handler(mContext.getMainLooper());
        PowerManager pm = (PowerManager) mContext.getSystemService(ContextWrapper.POWER_SERVICE);
        mWakeLock =
                pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK
                        | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);

        mRcseCoreServiceStatusReceiver = new RcseCoreServiceStatusReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(SERVICE_STATUS);
        intentFilter.addAction(SERVICE_REGISTRATION);
        mContext.registerReceiver(mRcseCoreServiceStatusReceiver, intentFilter);
        RcsSettings.createInstance(mContext);
    }

    void connectRichCallApi() {
        Logger.d(TAG, "connectRichCallApi() entry!");
        if (mRichCallStatus == RichCallStatus.DISCONNECTED) {
            mRichCallStatus = RichCallStatus.CONNECTING;
            PluginApiManager.getInstance().addCapabilitiesChangeListener(SharingPlugin.this);
            mRichCallApiListener = new RichCallApiListener();
            mRichCallApi.addApiEventListener(mRichCallApiListener);
            mRichCallApi.connectApi();
            Logger.d(TAG, "mRichCallStatus is CONNECTING, wait to notify when CONNECTED");
        } else {
            Logger.d(TAG, "connectRichCallApi(), RichCallApi has been connected!");
        }
    }

    public int getCurrentState() {
    	return Constants.SHARE_FILE_STATE_IDLE;
    }

    /**
     * Image sharing invitation receiver.
     */
    private class RcseCoreServiceStatusReceiver extends BroadcastReceiver {
        private static final String TAG = "SharingPlugIn RcseCoreServiceStatusReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive(), context = " + context + ", intent = " + intent);
            if (intent != null) {
                String action = intent.getAction();
                Logger.v(TAG, "action = " + action);
                if (SERVICE_STATUS.equals(action)) {
                    int status = intent.getIntExtra(CORE_SERVICE_STATUS, INVALID_VALUE);
                    Logger.v(TAG, "status = " + status);
                    handleRcsCoreServiceStatusChanged(status);
                } else if (SERVICE_REGISTRATION.equals(action)) {
                    handleRegistrationStatusChanged(intent);
                } else {
                    Logger.w(TAG, "Unknown action");
                }
            } else {
                Logger.w(TAG, "intent is null");
            }
        }
    }

    private void handleRegistrationStatusChanged(Intent intent) {
        Logger.d(TAG, "handleRegistrationStatusChanged() entry");
        boolean status = intent.getBooleanExtra(CORE_SERVICE_STATUS, false);
        Logger.v(TAG, "handleRegistrationStatusChanged() status = " + status);
        mIsRegistered = status;        
        if (status) {
            if (mRichCallStatus != RichCallStatus.CONNECTED) {
                Logger.v(TAG, "handleRegistrationStatusChanged() connect rich call api");
                mRichCallApi.connectApi();
            } else {
                Logger.d(TAG,
                        "handleRegistrationStatusChanged() mRichCallStatus = RichCallStatus.CONNECTED");
            }
        }
    }

    private void handleRcsCoreServiceStatusChanged(int status){
        Logger.d(TAG, "handleRcsCoreServiceStatusChanged( ) : " + status);
        if (status == ClientApiIntents.SERVICE_STATUS_STARTED) {
            if (PluginApiManager.getInstance().getManagedApiStatus()) {
                PluginApiManager.getInstance().reConnectManagedApi();
            }
        } else if (status == ClientApiIntents.SERVICE_STATUS_STOPPING) {
            PluginApiManager.getInstance().setManagedApiStatus(true);
        }
    }

    private class RichCallApiListener implements ClientApiListener {
        private static final String TAG = "RichCallApiListener";

        @Override
        public void handleApiConnected() {
            Logger.d(TAG, "handleApiConnected entry");
            mRichCallStatus = RichCallStatus.CONNECTED;
            if (mCallScreenHost != null) {
                mCallScreenHost.onCapabilityChange(mNumber, getCapability(mNumber));
            } else {
                Logger.d(TAG, "handleApiConnected mCallScreenHost is null");
            }
            mInterface.onApiConnected();
            Logger.d(TAG, "handleApiConnected exit");
        }

        @Override
        public void handleApiDisabled() {
            Logger.d(TAG, "handleApiDisabled entry");
            mRichCallStatus = RichCallStatus.DISCONNECTED;
            Logger.d(TAG, "handleApiDisabled exit");

        }

        @Override
        public void handleApiDisconnected() {
            Logger.d(TAG, "handleApiDisconnected entry");
            mRichCallStatus = RichCallStatus.DISCONNECTED;
            Logger.d(TAG, "handleApiDisconnected exit");
        }

    }

    @Override
    public void start(String number) {
        Logger.d(TAG, "start entry, with number: " + number);
        if (number == null) {
            Logger.e(TAG, "start number is null");
            return;
        }
        int networkType = Utils.getNetworkType(mContext);
        if (networkType == Utils.NETWORK_TYPE_GPRS || networkType == Utils.NETWORK_TYPE_EDGE) {
            Logger.v(TAG, "At such case, not allowed to send a video share, networkType is "
                    + networkType);
            return;
        }
        mWakeLock.acquire();
        mWakeLockCount.addAndGet(1);
        if (mCallScreenHost != null) {
            mDisplayArea = mCallScreenHost.requestAreaForDisplay();
        } else {
            Logger.d(TAG, "start mCallScreenHost is null");
        }
        mNumber = number;
        Logger.d(TAG, "start exit");
    }

    @Override
    public void stop() {
    }

    @Override
    public void setCallScreenHost(ICallScreenHost host) {
        Logger.v(TAG, "setCallScreenHost(), host = " + host);
        mCallScreenHost = host;
    }

    @Override
    public boolean getCapability(String number) {
        Logger.v(TAG, "sharing getCapability(), number: " + number);
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.v(TAG, "sharing getCapability() mRichCallStatus is NOT CONNECTED, return false");
            mRichCallApi.connectApi();
            return false;
        }
        if (!mIsRegistered) {
            Logger.v(TAG, "sharing getCapability() off line, return false");
            return false;
        } else {
            Logger.d(TAG, "sharing getCapability() the registration is true ");
        }
        final String vodafoneAccount = getVodafoneAccount(number);
        boolean isSharingSupported = PluginApiManager.getInstance().isRcseContact(vodafoneAccount);
        Logger.v(TAG, "sharing getCapability() isSharingSupported: " + isSharingSupported);

        return isSharingSupported;
    }

    @Override
    public boolean isImageShareSupported(String number) {
        Logger.w(TAG, "isImageShareSupported(), number: " + number);
        mNumber = number;
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.w(TAG, "sharing isImageShareSupported() mRichCallStatus is NOT CONNECTED, return false");
            mRichCallApi.connectApi();
            return false;
        }		
        if (!mIsRegistered) {
            Logger.v(TAG, "sharing isImageShareSupported() off line, return false");
            return false;
        }
        Logger.w(TAG, "isImageShareSupported() the registration is true");
        final String vodafoneAccount = getVodafoneAccount(number);
        boolean isImageShareSupported = PluginApiManager.getInstance().isImageShareSupported(
                vodafoneAccount);
        Logger.w(TAG, "sharing isImageShareSupported() isImageShareSupported: " + isImageShareSupported);
        return isImageShareSupported;
    }

    protected void imageShareNotSupported() {
        Logger.v(TAG, "imageShareNotSupported entry");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.image_sharing_not_available);
                showToast(message);
            }
        });
        Logger.v(TAG, "imageShareNotSupported exit");
    }

    protected void videoShareNotSupported() {
        Logger.v(TAG, "videoShareNotSupported entry");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.video_sharing_not_available);
                showToast(message);
            }
        });
        Logger.v(TAG, "videoShareNotSupported exit");
    }

    protected void showToast(String message) {
        Logger.v(TAG, "showToast() entry, message = " + message);
        Toast toast = Toast.makeText(mContext, message, Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
        Logger.v(TAG, "showToast() exit");
    }

    @Override
    public boolean isVideoShareSupported(String number) {
        Logger.v(TAG, "isVideoShareSupported(), number: " + number);
        mNumber = number;
        if (mRichCallStatus != RichCallStatus.CONNECTED) {
            Logger.v(TAG, "isVideoShareSupported() mRichCallStatus is NOT CONNECTED, return false");
            mRichCallApi.connectApi();
            return false;
        }
				
        if (!mIsRegistered) {
            Logger.v(TAG, "isVideoShareSupported() off line, return false");
            return false;
        }
        Logger.d(TAG, "isVideoShareSupported() the registration is true");
        final String vodafoneAccount = getVodafoneAccount(number);
        boolean isVideoShareSupported = PluginApiManager.getInstance().isVideoShareSupported(
                vodafoneAccount);
        Logger.v(TAG, "isVideoShareSupported() isVideoShareSupported: " + isVideoShareSupported);
        return isVideoShareSupported;
    }

    @Override
    public ICallScreenHost getCallScreenHost() {
        Logger.v(TAG, "getCallScreenHost(), mCallScreenHost = " + mCallScreenHost);
        return mCallScreenHost;
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation contactInformation) {
        Logger.d(TAG, "onCapabilitiesChanged() contactInformation: " + contactInformation
                + " mCallScreenHost: " + mCallScreenHost + " contact: " + contact + " mNumber: "
                + mNumber);
        if (contactInformation == null || mCallScreenHost == null) {
            return;
        }
        boolean supported = contactInformation.isRcsContact == 1;
        String number = getVodafoneAccount(mNumber);
        if (number != null && number.equals(contact)) {
			Logger.w(TAG, "onCapabilitiesChanged  contact: " + contact);
            mCallScreenHost.onCapabilityChange(contact, supported);
            if (!supported) {
                mInterface.onFinishSharing();
            }
        }
    }

    @Override
    public void onApiConnectedStatusChanged(boolean isConnected) {
        Logger.d(TAG, "onApiConnectedStatusChanged isConnected: " + isConnected);
    }

    @Override
    public void registerForCapabilityChange(final String number) {
        Logger.d(TAG, "registerForCapabilityChange number: " + number);
        mNumber = number;
    }

    @Override
    public void unregisterForCapabilityChange(final String number) {
        Logger.d(TAG, "unregisterForCapabilityChange number: " + number);
    }

    @Override
    public int getState() {
        return 0;
    }

	@Override
    public int getStatus() {
		 return 0 ;  //not to be used
	}
	
    public void saveAlertDialogs()
    {
        Logger.v(TAG, "saveAlertDialogs() entry");
        Logger.v(TAG, "saveAlertDialogs() exit");
    }
    
    public void showAlertDialogs()
    {
        Logger.v(TAG, "showAlertDialogs() entry");
        Logger.v(TAG, "showAlertDialogs() exit");
    }
    
    public void clearSavedDialogs(){
        Logger.v(TAG, "clearSavedDialogs() entry");
        Logger.v(TAG, "clearSavedDialogs() exit");
    }
    
    public void onPhoneStateChange(CallManager cm) {
    	boolean hasActiveBgCall = false;
    	if(RCSeUtils.getmFgCall()!=null && (RCSeUtils.getmFgCall().getState()== Call.State.ONHOLD))
    		hasActiveBgCall = true;
        //boolean hasActiveBgCall = cm.hasActiveBgCall();
        Logger.d(TAG, "onPhoneStateChange(), hasActiveBgCall = " + hasActiveBgCall
                + " mIsOnHold = " + mIsOnHold);
        if (hasActiveBgCall ^ mIsOnHold) {
            // exchange capability with remote people
            CapabilityApi api = PluginApiManager.getInstance().getCapabilityApi();
            if (api != null) {
                Capabilities capabilities = api.getMyCapabilities();
                if (capabilities != null) {
                    boolean isHaveImageShareCapability = capabilities.isImageSharingSupported() && !hasActiveBgCall;
                    capabilities.setImageSharingSupport(isHaveImageShareCapability);
                    capabilities.setVideoSharingSupport(isHaveImageShareCapability);
                    api.setMyCapabilities(capabilities);
                    api.getContactCapabilities(getVodafoneAccount(mNumber));
                } else {
                    Logger.e(TAG, "onPhoneStateChange(), capabilities is null!");
                }
            } else {
                Logger.d(TAG, "getCapability, capability api is null");
            }
        } else {
            Logger.d(TAG, "Hold status not changed, no need to exchange capability!");
        }
        mIsOnHold = hasActiveBgCall;
    }

    protected String getVodafoneAccount(String normalNumber) {
        Logger.d(TAG, "getVodafoneAccount() entry, normalNumber: " + normalNumber);
        String vodafoneAccount = normalNumber;
        if (LauncherUtils.getDebugMode(mContext)) {
            if (normalNumber != null) {
                try {
                    String number = mRichCallApi.getVfAccountViaNumber(normalNumber);
                    if (number != null) {
                        vodafoneAccount = number;
                    }
                } catch (ClientApiException e) {
                    e.printStackTrace();
                    Logger.e(TAG, "getVodafoneAccount() ClientApiException, return null");
                }
            }
        }
        // format to + country code xxx
        vodafoneAccount = PhoneUtils.formatNumberToInternational(vodafoneAccount);
        Logger.d(TAG, "getVodafoneAccount() exit, vodafoneAccount: " + vodafoneAccount);
        return vodafoneAccount;
    }

    @Override
    public boolean dismissDialog() {
        return false;
    }

    interface ISharingPlugin {
        void onApiConnected();

        void onFinishSharing();

    }

}
