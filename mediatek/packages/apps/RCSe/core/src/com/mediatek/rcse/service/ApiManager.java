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

package com.mediatek.rcse.service;

import android.content.Context;
import android.os.AsyncTask;
import android.os.RemoteException;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.ModelImpl;

import com.orangelabs.rcs.core.ims.network.NetworkConnectivityApi;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.service.api.client.messaging.IMessageDeliveryListener;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.terms.TermsApi;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * This class manages the APIs, providing a convenient way for API invocations.
 */
public class ApiManager {
    public static final  String TAG = "ApiManager";

    private static ApiManager sInstance = null;

    private RegistrationApi mRegistrationApi = null;
    private CapabilityApi mCapabilitiesApi = null;
    private TermsApi mTermsApi = null;
    private MessagingApi mMessagingApi = null;
    private EventsLogApi mEventsLogApi = null;
    private NetworkConnectivityApi mNetworkConnectivityApi = null;
    private Context mContext = null;
    private static long sMaxFileSize = 0;
    private static long sWarningFileSize = 0;
    private RcseComponentController mRcseComponentController = null;

    /**
     * This method should only be called from ApiService, for APIs
     * initialization.
     * 
     * @param context The Context of this application.
     * @return true If initialize successfully, otherwise false.
     */
    public static synchronized boolean initialize(Context context) {
        Logger.v(TAG, "initialize() entry");
        if (null != sInstance) {
            Logger
                    .w(TAG,
                            "initialize() sInstance has existed, is it really the first time you call this method?");
            return true;
        } else {
            if (null != context) {
                RcsSettings rcsSetting = RcsSettings.getInstance();
                if (rcsSetting == null) {
                    RcsSettings.createInstance(context);
                }
                ApiManager apiManager = new ApiManager(context);
                sInstance = apiManager;
                AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... arg0) {
                        sMaxFileSize = RcsSettings.getInstance().getMaxFileTransferSize() * 1024;
                        sWarningFileSize =
                                RcsSettings.getInstance().getWarningMaxFileTransferSize() * 1024;
                        return null;
                    }
                };
                task.execute();
                return true;
            } else {
                Logger.e(TAG, "initialize() the context is null");
                return false;
            }
        }
    }

    /**
     * Get the context
     * 
     * @return Context
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Get the RcseComponentController
     * 
     * @return RcseComponentController
     */
    public RcseComponentController getRcseComponentController() {
        return mRcseComponentController;
    }

    /**
     * Get the max size for file transfer
     * 
     * @return max size
     */
    public long getMaxSizeforFileThransfer() {
        return sMaxFileSize;
    }

    /**
     * Get the warning size for file transfer
     * 
     * @return warning size
     */
    public long getWarningSizeforFileThransfer() {
        return sWarningFileSize;
    }

    /**
     * Get the instance of ApiManager
     * 
     * @return The instance of ApiManager, or null if the instance has not been
     *         initialized.
     */
    public static ApiManager getInstance() {
        Logger.v(TAG, "getInstance() : sInstance = " + sInstance);
        return sInstance;
    }

    /**
     * Get the connected RegistrationApi
     * 
     * @return The instance of RegistrationApi, or null if the instance has not
     *         connected.
     */
    public RegistrationApi getRegistrationApi() {
        Logger.v(TAG, "getRegistrationApi() : mRegistrationApi = " + mRegistrationApi);
        return mRegistrationApi;
    }

    /**
     * Get the connected CapabilityApi
     * 
     * @return The instance of CapabilityApi, or null if the instance has not
     *         connected.
     */
    public CapabilityApi getCapabilityApi() {
        Logger.v(TAG, "getCapabilityApi() : mCapabilitiesApi = " + mCapabilitiesApi);
        return mCapabilitiesApi;
    }

    /**
     * Get the connected TermsApi
     * 
     * @return The instance of TermsApi, or null if the instance has not
     *         connected.
     */
    public TermsApi getTermsApi() {
        Logger.v(TAG, "getCapabilityApi() : mTermsApi = " + mTermsApi);
        return mTermsApi;
    }
    
    /**
     * Get the connected MessagingApi
     * 
     * @return The instance of MessagingApi, or null if the instance has not
     *         connected.
     */
    public MessagingApi getMessagingApi() {
        Logger.v(TAG, "getMessagingApi() : mMessagingApi = " + mMessagingApi);
        return mMessagingApi;
    }

    /**
     * Get the connected NetworkConnectivityApi
     * 
     * @return The instance of getNetworkConnectivityApi, or null if the
     *         instance has not connected.
     */
    public NetworkConnectivityApi getNetworkConnectivityApi() {
        Logger.v(TAG,
                "getNetworkConnectivityApi() : mNetworkConnectivityApi = "
                        + mNetworkConnectivityApi);
        return mNetworkConnectivityApi;
    }

    /**
     * Get the connected EventsLogApi
     * 
     * @return The instance of EventsLogApi, or null if the instance has not
     *         connected.
     */
    public EventsLogApi getEventsLogApi() {
        Logger.v(TAG, "getEventsLogApi() : mEventsLogApi = " + mEventsLogApi);
        return mEventsLogApi;
    }

    private ApiManager(Context context) {
        Logger.d(TAG, "ApiManager() entry");
        mContext = context;
        new ManagedRegistrationApi(context).connect();
        new ManagedCapabilityApi(context).connect();
        TermsApi termsApi = new TermsApi(context);
        termsApi.addApiEventListener(new TermsApiListener(
                termsApi));
        termsApi.connectApi();
        MessagingApi messagingApi = new MessagingApi(context);
        messagingApi.addApiEventListener(new MessagingApiListener(messagingApi));
        messagingApi.connectApi();
        NetworkConnectivityApi networkConnectivityApi = new NetworkConnectivityApi(context);
        networkConnectivityApi.addApiEventListener(new NetworkConnectivityApiListener(
                networkConnectivityApi));
        networkConnectivityApi.connectApi();
        mEventsLogApi = new EventsLogApi(context);
        mRcseComponentController = new RcseComponentController();
    }

    private class ManagedRegistrationApi extends RegistrationApi {
        public static final String TAG = "ManagedRegistrationApi";

        public ManagedRegistrationApi(Context context) {
            super(context);
        }

        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry, this.isRegistered() = "
                    + this.isRegistered());
            mRegistrationApi = this;

            mRegistrationApi
                    .addRegistrationStatusListener(new RegistrationStatusListener());
        }

        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            if (mRegistrationApi == this) {
                Logger.i(TAG,
                        "handleDisconnected() mRegistrationApi disconnected");
                mRegistrationApi = null;
            }
        }

        private class RegistrationStatusListener implements
                IRegistrationStatusListener {
            @Override
            public void onStatusChanged(boolean status) {
                Logger.d(TAG, "onStatusChanged() entry, status is " + status);
                if (status) {
                    Logger.d(TAG, "onStatusChanged() : mMessagingApi = "
                            + mMessagingApi);
                    if (null == mMessagingApi) {
                        Logger.d(TAG,
                                "onStatusChanged() : try to initialize MessagingApi");
                        initializeMessagingApi();
                    }
                    if (null == mNetworkConnectivityApi) {
                        Logger.d(TAG,
                                "onStatusChanged():try to initialize NetworkConnectivityApi");
                        initializeNetworkConnectivityApi();
                    }
                    AsyncTask.execute(new Runnable() {
                        @Override
                        public void run() {
                            sMaxFileSize = RcsSettings.getInstance()
                                    .getMaxFileTransferSize() * 1024;
                            sWarningFileSize = RcsSettings.getInstance()
                                    .getWarningMaxFileTransferSize() * 1024;
                        }
                    });
                }
            }

            private boolean initializeNetworkConnectivityApi() {
                Logger.d(TAG, "initializeNetworkConnectivityApi() entry");
                NetworkConnectivityApi networkConnectivityApi = new NetworkConnectivityApi(
                        mContext);
                networkConnectivityApi
                        .addApiEventListener(new NetworkConnectivityApiListener(
                                networkConnectivityApi));
                networkConnectivityApi.connectApi();
                return true;
            }

            private boolean initializeMessagingApi() {
                Logger.d(TAG, "initializeMessagingApi() entry");
                MessagingApi messagingApi = new MessagingApi(mContext);
                messagingApi.addApiEventListener(new MessagingApiListener(
                        messagingApi));
                messagingApi.connectApi();
                return true;
            }
        }
    }

    private class ManagedCapabilityApi extends CapabilityApi {
        public static final String TAG = "ManagedCapabilityApi";

        public ManagedCapabilityApi(Context context) {
            super(context);
        }

        @Override
        public void handleConnected() {
            Logger.v(TAG, "handleConnected() entry");
            mCapabilitiesApi = this;
        }

        @Override
        public void handleDisconnected() {
            Logger.v(TAG, "handleDisconnected() entry");
            if (mCapabilitiesApi == this) {
                Logger.i(TAG, "handleDisconnected() mCapabilitiesApi disconnected");
                mCapabilitiesApi = null;
            }
        }
    }

    /**
     * Listener to observe MessagingApi's connected/disconnected status
     */
    private class MessagingApiListener extends IMessageDeliveryListener.Stub implements
            ClientApiListener {
        public static final String TAG = "MessagingApiListener";

        private MessagingApi mMessagingApi = null;

        public MessagingApiListener(MessagingApi messagingApi) {
            mMessagingApi = messagingApi;
        }


        @Override
        public void handleApiConnected() {
            Logger.v(TAG, "handleConnected() entry");
            ApiManager.this.mMessagingApi = MessagingApiListener.this.mMessagingApi;
            try {
                MessagingApiListener.this.mMessagingApi.addMessageDeliveryListener(this);
                Logger.d(TAG, "handleApiConnected() add message delivery listener success");
            } catch (ClientApiException e) {
                Logger.w(TAG, "handleApiConnected() add message delivery listener fail");
                e.printStackTrace();
            }
        }

        @Override
        public void handleApiDisabled() {
            Logger.v(TAG, "handleApiDisabled() entry");
            handleApiDisconnected();
        }

        @Override
        public void handleApiDisconnected() {
            Logger.v(TAG, "handleApiDisconnected() entry");
            if (ApiManager.this.mMessagingApi == MessagingApiListener.this.mMessagingApi) {
                Logger.i(TAG, "handleDisconnected() mMessagingApi disconnected");
                ApiManager.this.mMessagingApi = null;
            }
        }

        @Override
        public void handleMessageDeliveryStatus(final String contact, final String msgId,
                final String status, final long timeStamp) throws RemoteException {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    IChatManager chatManager = ModelImpl.getInstance();
                    chatManager.handleMessageDeliveryStatus(PhoneUtils
                            .extractNumberFromUri(contact), msgId, status, timeStamp);
                }
            });

        }
        
        public void handleFileDeliveryStatus( String ftSessionId,  String status){
        }


		@Override
		public void handleFileDeliveryStatus(String ftSessionId, String status,
				String contact) throws RemoteException {
			// TODO Auto-generated method stub
			
		}
    }

    private class NetworkConnectivityApiListener implements ClientApiListener {
        public static final String TAG = "NetworkConnectivityApiListener";

        private NetworkConnectivityApi mNetworkApi = null;

        public NetworkConnectivityApiListener(NetworkConnectivityApi networkConnectivityApi) {
            mNetworkApi = networkConnectivityApi;
        }

        @Override
        public void handleApiConnected() {
            Logger.v(TAG, "handleConnected() entry");
            ApiManager.this.mNetworkConnectivityApi = mNetworkApi;
        }

        @Override
        public void handleApiDisabled() {
            Logger.v(TAG, "handleApiDisabled() entry");
            handleApiDisconnected();
        }

        @Override
        public void handleApiDisconnected() {
            Logger.v(TAG, "handleApiDisconnected() entry");
            if (ApiManager.this.mNetworkConnectivityApi == mNetworkApi) {
                Logger.i(TAG, "handleDisconnected() mNetworkConnectivityApi disconnected");
                ApiManager.this.mNetworkConnectivityApi = null;
            }
        }
    }

    private class TermsApiListener implements ClientApiListener {
        public static final String TAG = "NetworkConnectivityApiListener";

        private TermsApi mTermApi = null;

        public TermsApiListener(TermsApi termsApi) {
            mTermApi = termsApi;
        }

        @Override
        public void handleApiConnected() {
            Logger.v(TAG, "handleConnected() entry");
            ApiManager.this.mTermsApi = mTermApi;
        }

        @Override
        public void handleApiDisabled() {
            Logger.v(TAG, "handleApiDisabled() entry");
            handleApiDisconnected();
        }

        @Override
        public void handleApiDisconnected() {
            Logger.v(TAG, "handleApiDisconnected() entry");
            if (ApiManager.this.mTermsApi == mTermsApi) {
                Logger.i(TAG, "handleDisconnected() mTermsApi disconnected");
                ApiManager.this.mTermsApi = null;
            }
        }
    }
    
    /**
     * control the RCS component according to the configuration and active
     * status.
     */
    public class RcseComponentController {
        private boolean mConfigurationStatus = true;
        private boolean mServiceActiveStatus = true;

        /**
         * Set configuration status and control the rcse component.
         * 
         * @param status the configuration status.
         */
        public void onConfigurationStatusChanged(boolean status) {
            Logger.d(TAG, "onConfigurationStatusChanged() entry status is " + status);
            mConfigurationStatus = status;
            controlRcseComponent();

        }

        /**
         * Set ServiceActive status and control the rcse component.
         * 
         * @param status the status of ServiceActive .
         */
        public void onServiceActiveStatusChanged(boolean status) {
            Logger.d(TAG, "onServiceActiveStatusChanged() entry status is " + status);
            mServiceActiveStatus = status;
            controlRcseComponent();
        }

        /**
         * control the RCS component according to the configuration and active
         * status.
         */
        private void controlRcseComponent() {
            Logger.d(TAG, "controlRcseComponent() entry  the mConfigurationstatus is "
                    + mConfigurationStatus + " ServiceactiveStatus is " + mServiceActiveStatus);
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG, "controlRcseComponent() entry is integration mode ");
                CoreApplication.setIntegrationModeComponent(mContext);
            } else {
                if (mConfigurationStatus && mServiceActiveStatus) {
                    CoreApplication.setComponentStatus(mContext, true);
                } else {
                    CoreApplication.setComponentStatus(mContext, false);
                    RcsNotification.getInstance().cancelNotification();
                }
            }
        }
    }

}
