/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

import android.app.Service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.activities.ConfigMessageActicity;
import com.mediatek.rcse.api.ICapabilityRemoteListener;
import com.mediatek.rcse.api.IRegistrationStatusRemoteListener;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi.IRegistrationStatusListener;
import com.mediatek.rcse.service.ApiManager.RcseComponentController;
import com.mediatek.rcse.service.binder.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.WindowBinder;

import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.service.StartService;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ClientApiIntents;
import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApi;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApiIntents;
import com.orangelabs.rcs.service.api.client.contacts.ContactsApiIntents;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApiIntents;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApiIntents;
import com.orangelabs.rcs.service.api.client.terms.TermsApiIntents;

import java.util.List;

//This Service will provide the remote API to the applications.
public class ApiService extends Service {
    public static final String TAG = "ApiService";

    private ApiReceiver mReceiver = null;
    private RegistrationStatusStub mRegistrationStatusStub = null;
    private CapabilitiesStub mCapabilitiesStub = null;
    private WindowBinder mWindowBinder = null;
    // Low memory broadcast action
    private static final String MEMORY_LOW_ACTION = "ACTION_DEVICE_STORAGE_LOW";
    // Memory okay broadcast action
    private static final String MEMORY_OK_ACTION = "ACTION_DEVICE_STORAGE_OK";
    public static final String CORE_CONFIGURATION_STATUS = "status";

    @Override
    public void onCreate() {
        Logger.v(TAG, "ApiService onCreate() entry");
        mReceiver = new ApiReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(CapabilityApiIntents.CONTACT_CAPABILITIES);
        intentFilter.addAction(ClientApiIntents.SERVICE_REGISTRATION);
        intentFilter.addAction(MessagingApiIntents.CHAT_INVITATION);
        intentFilter.addAction(MessagingApiIntents.CHAT_SESSION_REPLACED);
        intentFilter.addAction(MessagingApiIntents.FILE_TRANSFER_INVITATION);
        intentFilter.addAction(ContactsApiIntents.CONTACT_BLOCK_REQUEST);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_OK);
        intentFilter.addAction(Intent.ACTION_DEVICE_STORAGE_LOW);
        intentFilter.addAction(ClientApiIntents.SERVICE_STATUS);
        intentFilter.addAction(StartService.CONFIGURATION_STATUS);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_ACK);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_USER_NOTIFICATION);
        intentFilter.addAction(TermsApiIntents.TERMS_SIP_REQUEST);
        
        this.registerReceiver(mReceiver, intentFilter);
        registerSdCardReceiver();
        mRegistrationStatusStub = new RegistrationStatusStub();
        mCapabilitiesStub = new CapabilitiesStub(this);
        mWindowBinder = new WindowBinder();

        // Instantiate the contacts manager
        ContactsManager.createInstance(getApplicationContext());
        // Keep a initial IM blocked contacts list to local copy
        ContactsManager.getInstance().loadImBlockedContactsToLocal();

        Logger.v(TAG, "ApiService onCreate() exit");
    }
    
    private void registerSdCardReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addDataScheme("file");
        this.registerReceiver(new SdcardReceiver(), intentFilter);
    }

    @Override
    public void onDestroy() {
        Logger.v(TAG, "ApiService onDestroy() entry");
        super.onDestroy();
        if (null != mReceiver) {
            this.unregisterReceiver(mReceiver);
        } else {
            Logger.e(TAG, "onDestroy() mReceiver is null");
        }

        Logger.v(TAG, "ApiService onDestroy() exit");
    }

    @Override
    public IBinder onBind(Intent intent) {
        String action = intent.getAction();
        Logger.i(TAG, "onBind() entry, the action is " + action);
        if (IRegistrationStatus.class.getName().equals(action)) {
            return mRegistrationStatusStub;
        } else if (ICapabilities.class.getName().equals(action)) {
            return mCapabilitiesStub;
        } else if (IRemoteWindowBinder.class.getName().equals(action)) {
            return mWindowBinder;
        }
        Logger.v(TAG, "onBind() exit");
        return null;
    }

    // This receiver will handle sdcard mount and unmount broadcast
    private static class SdcardReceiver extends BroadcastReceiver {
        private static final String TAG = "SdcardReceiver";
        @Override
        public void onReceive(final Context context, final Intent intent) {
            Logger.d(TAG, "onReceive() SdcardReceiver entry");
            new AsyncTask<Void, Void, Void>() {
                 @Override
                 protected Void doInBackground(Void... params) {
                     String action = intent.getAction();
                        Logger.d(TAG, "doInBackground() SdcardReceiver action is " + action);
                        if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                            Logger.d(TAG, "doInBackground() SdcardReceiver() sdcard mounted");
                            boolean ftCapability = FileTransferCapabilityManager
                                    .isFileTransferCapabilitySupported();
                            FileTransferCapabilityManager.setFileTransferCapability(context,
                                    ftCapability);
                        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                            Logger.d(TAG, "doInBackground() SdcardReceiver() sdcard unmounted");
                            FileTransferCapabilityManager.setFileTransferCapability(context, false);
                        }
                     return null;
                 }
                }.execute();
            Logger.d(TAG, "onReceive() SdcardReceiver exit");
        }
    }

    // This receiver will handle some RCS-e related broadcasts.
    private class ApiReceiver extends BroadcastReceiver {
        public static final String TAG = "ApiReceiver";
        public static final String KEY_STATUS = "status";
        public static final String KEY_CONTACT = "contact";
        public static final String KEY_CAPABILITIES = "capabilities";

        @Override
        public void onReceive(final Context context, final Intent intent) {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    asyncOnReceive(context, intent);
                    return null;
                }
            }.execute();
        }

        private void asyncOnReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Logger.v(TAG, "asyncOnReceive() entry, the action is " + action);
            if (CapabilityApiIntents.CONTACT_CAPABILITIES.equals(action)) {
                handleCapabilitiesNotification(context, intent);
            } else if (ContactsApiIntents.CONTACT_BLOCK_REQUEST.equals(action)) {
                handleBlockRequest(context, intent);
            } else if (ClientApiIntents.SERVICE_REGISTRATION.equals(action)) {
                handleRegistrationStatus(context, intent);
            } else if (MessagingApiIntents.CHAT_INVITATION.equalsIgnoreCase(action)
                    || MessagingApiIntents.FILE_TRANSFER_INVITATION.equalsIgnoreCase(action)
                    || RichCallApiIntents.IMAGE_SHARING_INVITATION.equalsIgnoreCase(action)
                    || RichCallApiIntents.VIDEO_SHARING_INVITATION.equalsIgnoreCase(action)) {
                RcsNotification.handleInvitation(context, intent);
            } else if (MessagingApiIntents.CHAT_SESSION_REPLACED.equalsIgnoreCase(action)) {
                RcsNotification.handleInvitation(context, intent);
            } else if (action.equals(MEMORY_LOW_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability =
                        ExchangeMyCapability.getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG, "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE, false);
            } else if (action.equals(MEMORY_OK_ACTION)) {
                final ExchangeMyCapability exchangeMyCapability =
                        ExchangeMyCapability.getInstance(ApiService.this);
                if (exchangeMyCapability == null) {
                    Logger.e(TAG, "Current ExchangeMyCapability instance is null");
                    return;
                }
                exchangeMyCapability.notifyCapabilityChanged(
                        ExchangeMyCapability.STORAGE_STATUS_CHANGE, true);
            } else if (StartService.CONFIGURATION_STATUS.equals(action)) {
                boolean status = intent.getBooleanExtra(CORE_CONFIGURATION_STATUS, true);
                handleConfigurationStatus(status);
            }
            else if (TermsApiIntents.TERMS_SIP_ACK.equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_REQUEST.equalsIgnoreCase(action)
                    || TermsApiIntents.TERMS_SIP_USER_NOTIFICATION.equalsIgnoreCase(action)) {
            	Logger.e(TAG, "asyncOnReceive() unknown action! The action is " + action);
            	Intent mIntent = new Intent(getApplicationContext(), ConfigMessageActicity.class);
                mIntent.setAction(action);
                startActivity(mIntent);
            }
            else {
                Logger.e(TAG, "asyncOnReceive() unknown action! The action is " + action);
            }
            Logger.v(TAG, "asyncOnReceive() exit");
        }

        private void handleCapabilitiesNotification(Context context, Intent intent) {
            Logger.v(TAG, "handleCapabilitiesNotification() entry");
            Bundle data = intent.getExtras();
            if (null != data) {
                String contact = data.getString(KEY_CONTACT);
                Capabilities capabilities = data.getParcelable(KEY_CAPABILITIES);
                Logger.i(TAG, "handleCapabilitiesNotification() Contact is " + contact
                        + " Capabilities is " + capabilities);
                mCapabilitiesStub.notifyCapabilities(contact, capabilities);
            } else {
                Logger.e(TAG, "handleCapabilitiesNotification() the data is null!");
            }
            Logger.v(TAG, "handleCapabilitiesNotification() exit");
        }

        private void handleBlockRequest(Context context, Intent intent) {
            Logger.v(TAG, "handleBlockRequest() entry");
            Bundle data = intent.getExtras();
            if (null != data) {
                String number = data.getString("number");
                ContactsManager instance = ContactsManager.getInstance();
                if(instance != null)
                {
                	instance.setImBlockedForContact(number, true);
                	
                }
            } else {
                Logger.e(TAG, "handleBlockRequest() the data is null!");
            }
            Logger.v(TAG, "handleBlockRequest() exit");
        }

        private void handleRegistrationStatus(Context context, Intent intent) {
            Logger.d(TAG, "handleRegistrationStatus() entry");
            Bundle data = intent.getExtras();
            boolean status = false;
            if (data != null) {
                status = data.getBoolean(KEY_STATUS);
            } else {
                Logger.w(TAG, "handleRegistrationStatus() data is null");
            }
            if (!status) {
                if (RcsNotification.getInstance() != null) {
                    RcsNotification.getInstance().sIsStoreAndForwardMessageNotified = false;
                } else {
                    Logger.d(TAG,
                            "handleRegistrationStatus, RcsNotification.getInstance() is null!");
                }
                Logger.d(TAG, "handleRegistrationStatus, status is false, " +
                        "set sIsStoreAndForwardMessageNotified to false!");
            } else {
                Logger.d(TAG, "handleRegistrationStatus, status is true!");
                boolean ftCapability = FileTransferCapabilityManager
                        .isFileTransferCapabilitySupported();
                FileTransferCapabilityManager.setFileTransferCapability(context, ftCapability);
            }
            Logger.i(TAG, "handleRegistrationStatus() the status is " + status);
            
            mRegistrationStatusStub.notifyRegistrationStatus(status);
            mCapabilitiesStub.onStatusChanged(status);
        }

        private void handleConfigurationStatus(boolean status) {
            Logger.d(TAG, "handleConfigurationStatus() entry the status is " + status);
            RcseComponentController rcseComponentController = ApiManager.getInstance().getRcseComponentController();
            Logger.d(TAG,"handleConfigurationStatus() : rcseComponentController " + rcseComponentController);
            if (rcseComponentController != null) {
                if (Logger.IS_DEBUG) {
                    Logger.d(TAG, "handleConfigurationStatus() it is debug version");
                } else {
                    rcseComponentController.onConfigurationStatusChanged(status);
                }
            } else {
                Logger.e(TAG, "handleConfigurationStatus()) " +
                        "ApiManager.getInstance().getRcseComponentController() is null");
            }
        }
    }
}

class RegistrationStatusStub extends IRegistrationStatus.Stub {
    public static final String TAG = "RegistrationStatusStub";
    private boolean mIsRcseRegistered = false;

    /**
     * List of listeners
     */
    private RemoteCallbackList<IRegistrationStatusRemoteListener> mListeners =
            new RemoteCallbackList<IRegistrationStatusRemoteListener>();
    /**
     * Lock used for synchronization
     */
    private Object mLock = new Object();

    @Override
    public void addRegistrationStatusListener(IRegistrationStatusRemoteListener listener)
            throws RemoteException {
        boolean result = mListeners.register(listener);
        Logger.i(TAG, "addRegistrationStatusListener() The result is " + result);
    }

    public void notifyRegistrationStatus(boolean status) {
        Logger.v(TAG, "notifyRegistrationStatus() entry: The status is " + status);
        // update the registration status
        mIsRcseRegistered = status;
        synchronized (mLock) {
            // Notify status listeners
            final int n = mListeners.beginBroadcast();
            for (int i = 0; i < n; i++) {
                try {
                    mListeners.getBroadcastItem(i).onStatusChanged(status);
                } catch (RemoteException e) {
                    Logger.w(TAG,
                            "notifyRegistrationStatus() Failed to notify target listener, the index is "
                                    + i);
                }
            }
            mListeners.finishBroadcast();
        }
        Logger.v(TAG, "notifyRegistrationStatus() exit");
    }

    @Override
    public boolean isRegistered() throws RemoteException {
        Logger.d(TAG, "isRegistered(), call ApiService:isRegistered()! mIsRcseRegistered = " + mIsRcseRegistered);
        return mIsRcseRegistered;
    }
}

class CapabilitiesStub extends ICapabilities.Stub implements IRegistrationStatusListener {
    public static final String TAG = "CapabilitiesStub";
    private Context mContext = null;
    private CapabilityApi mApi = null;

    CapabilitiesStub(Context context) {
        mContext = context;
        CapabilityApi capabilityApi = new CapabilityApi(mContext);
        capabilityApi.addApiEventListener(new CapabilityApiListener(capabilityApi));
        capabilityApi.connectApi();
    }

    /**
     * List of listeners
     */
    private RemoteCallbackList<ICapabilityRemoteListener> mListeners =
            new RemoteCallbackList<ICapabilityRemoteListener>();

    /**
     * Lock used for synchronization
     */
    private Object mLock = new Object();

    @Override
    public void addCapabilityListener(ICapabilityRemoteListener listener) throws RemoteException {
        boolean result = mListeners.register(listener);
        Logger.i(TAG, "addCapabilityListener() The result is " + result);
    }

    public void notifyCapabilities(String contact, Capabilities capabilities) {
        Logger.v(TAG, "notifyCapabilities() entry: The contact is " + contact
                + " the capabilities are " + capabilities);
        synchronized (mLock) {
            // Notify status listeners
            final int n = mListeners.beginBroadcast();
            for (int i = 0; i < n; i++) {
                try {
                    mListeners.getBroadcastItem(i).onCapabilityChanged(contact, capabilities);
                } catch (RemoteException e) {
                    Logger.w(TAG,
                            "notifyCapabilities() Failed to notify target listener, the index is "
                                    + i);
                }
            }
            mListeners.finishBroadcast();
        }
        Logger.v(TAG, "notifyCapabilities() exit");
    }

    @Override
    public Capabilities getContactCurentCapabilities(String contact) throws RemoteException{
        Logger.v(TAG, "getContactCapabilities() entry, the contact is "
                + contact + ", mApi = " + mApi);
        Capabilities contactCapabilities = null;
            if (mApi != null) {
                contactCapabilities = mApi.getContactCapabilities(contact);
            }
			else
                return null;
        Logger.d(TAG, "getContactCapabilities() exit, the contact is " + contact
                + " the capabilities are " + contactCapabilities);
        return contactCapabilities;
    }

    @Override
    public Capabilities getContactCapabilities(String contact) throws RemoteException {
        Logger.v(TAG, "getContactCapabilities() entry, the contact is "
                + contact + ", mApi = " + mApi);
        Capabilities contactCapabilities = null;
        try {
            if (mApi != null) {
                contactCapabilities = mApi.requestCapabilities(contact);
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
            return null;
        }
        Logger.d(TAG, "getContactCapabilities() exit, the contact is " + contact
                + " the capabilities are " + contactCapabilities);
        return contactCapabilities;
    }

    @Override
    public Capabilities getMyCapabilities() throws RemoteException {
        Logger.v(TAG, "getMyCapabilities() entry : mApi = " + mApi);
        Capabilities myCapabilities = null;
        if (mApi != null) {
            myCapabilities = mApi.getMyCapabilities();
            Logger.v(TAG, "getMyCapabilities(), my capabilities are " + myCapabilities);
        }
        Logger.v(TAG, "getMyCapabilities() exit");
        return myCapabilities;
    }

    @Override
    public void refreshContactCapabilities(String contact) throws RemoteException {
        Logger.v(TAG, "refreshContactCapabilities() entry, the contact is "
                + contact + ", mApi = " + mApi);
        try {
            if (mApi != null) {
                 mApi.refreshContactCapabilities(contact);
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "refreshContactCapabilities() exit ");
	}

    @Override
    public List<String> getRcsContacts() {
        Logger.d(TAG, "getRcsContacts() : mApi = " + mApi);
        List<String> list = null;
        if (mApi != null) {
            list = mApi.getRcsContacts();
        }
        return list;
    }

    @Override
    public void onStatusChanged(boolean status) {
        Logger.d(TAG, "onStatusChanged() : status = " + status + ", mApi = "
                + mApi);
        if (status) {
            if (null == mApi) {
                CapabilityApi capabilityApi = new CapabilityApi(mContext);
                capabilityApi.addApiEventListener(new CapabilityApiListener(capabilityApi));
                capabilityApi.connectApi();
            }
        }
    }

    private class CapabilityApiListener implements ClientApiListener {
        public static final String TAG = "CapabilityApiListener";

        private CapabilityApi mCapabilityApi = null;

        public CapabilityApiListener(CapabilityApi capabilityApi) {
            mCapabilityApi = capabilityApi;
        }

        @Override
        public void handleApiConnected() {
            Logger.v(TAG, "handleConnected() entry");
            mApi = mCapabilityApi;
        }

        @Override
        public void handleApiDisabled() {
            Logger.v(TAG, "handleApiDisabled() entry");
            handleApiDisconnected();
        }

        @Override
        public void handleApiDisconnected() {
            Logger.v(TAG, "handleApiDisconnected() entry: mApi = " + mApi
                    + ", mCapabilityApi = " + mCapabilityApi);
            if (mApi == mCapabilityApi) {
                Logger.i(TAG, "handleDisconnected() mApi disconnected");
                mApi = null;
            }
        }
    }
}
