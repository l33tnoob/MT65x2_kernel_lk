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
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.capability.CapabilityApi;
import com.orangelabs.rcs.service.api.client.messaging.IChatSession;
import com.orangelabs.rcs.service.api.client.messaging.IFileTransferSession;
import com.orangelabs.rcs.service.api.client.messaging.MessagingApi;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

/**
 * This class is used to notify related contact my capability has changed.
 **/
public final class ExchangeMyCapability {
    private static final String TAG = "ExchangeMyCapability";
    public static final int FILE_TRANSFER_CAPABILITY_CHANGE = 1;
    public static final int STORAGE_STATUS_CHANGE = 2;
    public static final int IMAGE_STATUS_CHANGE = 3;
    public static final int VIDEO_STATUS_CHANGE = 4;
    private HashSet<String> mContactList = null;
    private CapabilityApi mCapabilityApi = null;
    private RichCallApi mRichCallApi = null;
    private static ExchangeMyCapability sInstance = null;

    /**
     * Create and return the ExchangeMyCapability instance by special context.
     * 
     * @param context Current context instance.
     * @return ExchangeMyCapability instance.
     */
    public static synchronized ExchangeMyCapability getInstance(Context context) {
        Logger.d(TAG, "getInstance() of ExchangeMyCapability entry, sInstance is " + sInstance);
        if (sInstance == null) {
            sInstance = new ExchangeMyCapability(context);
        }
        return sInstance;
    }

    private ExchangeMyCapability(Context context) {
        Logger.d(TAG, "ExchangeMyCapability entry");
        mCapabilityApi = new CapabilityApi(context);
        mRichCallApi = new RichCallApi(context);
        mCapabilityApi.connectApi();
        mRichCallApi.connectApi();
        mContactList = new HashSet<String>();
    }

    private void addImContactList(List<IBinder> sessions) {
        int size = sessions.size();
        Logger.d(TAG, "addImContactList() entry! size is " + size);
        for (int i = 0; i < size; i++) { 
            Logger.d(TAG, "session class is : " + sessions.get(i).toString());
            IChatSession chatSession = (IChatSession) IChatSession.Stub
                    .asInterface(sessions.get(i));
            try {
                mContactList.addAll(chatSession.getParticipants());
            } catch (RemoteException e) {
                Logger.d(TAG, "addImContactList() remote exception");
                e.printStackTrace();
            }
        }
        Logger.d(TAG, "addImContactList() exit, size is " + size);
    }

    private void addFileTransferContactList(List<IBinder> sessions) {
        Logger.d(TAG, "addFileTransferContactList() entry! sesssions is "
                + (sessions == null ? "null" : "not null"));
        if (sessions != null) {
            int size = sessions.size();
            for (int i = 0; i < size; i++) {
                IFileTransferSession fileTransferSession = (IFileTransferSession) IFileTransferSession.Stub
                        .asInterface(sessions.get(i));
                try {
                    mContactList.add(fileTransferSession.getRemoteContact());
                } catch (RemoteException e) {
                    Logger.d(TAG, "addFileTransferContactList() remote exception");
                    e.printStackTrace();
                }
            }
        }
        Logger.d(TAG, "addFileTransferContactList() exit!");
    }

    private void getContactListOngoingImSessions() {
        Logger.d(TAG, "getContactListOngoingImSessions() entry");
        List<IBinder> chatSessions;
        MessagingApi messagingApi = ApiManager.getInstance().getMessagingApi();
        try {
            Logger.d(TAG, "The messagingApi = " + messagingApi);
            if (messagingApi != null) {
                chatSessions = messagingApi.getChatSessions();
                addImContactList(chatSessions);
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "getContactListOngoingImSessions() exit");
    }

    private void getContactListOngoingFileTransferSessions() {
        List<IBinder> fileTransferSession;
        MessagingApi messagingApi = ApiManager.getInstance().getMessagingApi();
        try {
            Logger.d(TAG, "The messagingApi = " + messagingApi);
            if (messagingApi != null) {
                fileTransferSession = messagingApi.getFileTransferSessions();
                addFileTransferContactList(fileTransferSession);
            } 
        } catch (ClientApiException e) {
            e.printStackTrace();
        }
    }

    private void getContactListOngoingRichCall() {
        String remotePhoneNumber = null;
        try {
            remotePhoneNumber = mRichCallApi.getRemotePhoneNumber();
        } catch (ClientApiException e) {
            e.printStackTrace();
        }
        if (remotePhoneNumber != null) {
            String phoneUri = PhoneUtils.formatNumberToSipUri(remotePhoneNumber);
            mContactList.add(phoneUri);
        }
    }

    private void notifyCapabilityChanged() {
        Logger.d(TAG, "notifyCapabilityChanged entry");
        Iterator<String> contactIterator = mContactList.iterator();
        Logger.d(TAG, "The contactIterator is " + contactIterator);
        while (contactIterator != null && contactIterator.hasNext()) {
            try {
                String contact = contactIterator.next();
                Logger.d(TAG, "The contact is " + contact);
                if (contact != null) {
                    mCapabilityApi.requestCapabilities(contact);
                }
            } catch (ClientApiException e) {
                e.printStackTrace();
            }
        }
        Logger.d(TAG, "notifyCapabilityChanged exit");
    }

    private void doFileTransferCapabilityChanged(boolean capability) {
        Logger.d(TAG, "doFileTransferCapabilityChanged() entry");
        getContactListOngoingFileTransferSessions();
        getContactListOngoingImSessions();
        if (capability) {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER,
                    RcsSettingsData.TRUE);
        } else {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER,
                    RcsSettingsData.FALSE);
        }
        Logger.d(TAG, "doFileTransferCapabilityChanged() exit");
    }

    private void doImageShareCapabilityChanged(boolean capability) {
        getContactListOngoingRichCall();
        if (capability) {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_IMAGE_SHARING,
                    RcsSettingsData.TRUE);
        } else {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_IMAGE_SHARING,
                    RcsSettingsData.FALSE);
        }
    }

    private void doVideoShareCapabilityChanged(boolean capability) {
        getContactListOngoingRichCall();
        if (capability) {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING,
                    RcsSettingsData.TRUE);
        } else {
            RcsSettings.getInstance().writeParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING,
                    RcsSettingsData.FALSE);
        }
    }

    private void notifyStorageChanged(boolean capability) {
        getContactListOngoingImSessions();
        getContactListOngoingFileTransferSessions();
        doFileTransferCapabilityChanged(capability);
        doImageShareCapabilityChanged(capability);
        doVideoShareCapabilityChanged(capability);
    }

    private void doCapabilityChanged(int capabilityType, boolean capability) {
        Logger.d(TAG, "doCapabilityChanged entry, capabilityType is " + capabilityType);
        switch (capabilityType) {
            case FILE_TRANSFER_CAPABILITY_CHANGE:
                doFileTransferCapabilityChanged(capability);
                break;
            case STORAGE_STATUS_CHANGE:
                notifyStorageChanged(capability);
                break;
            case IMAGE_STATUS_CHANGE:
                doImageShareCapabilityChanged(capability);
                break;
            case VIDEO_STATUS_CHANGE:
                doVideoShareCapabilityChanged(capability);
                break;
            default:
                break;
        }
        Logger.d(TAG, "doCapabilityChanged exit");
    }

    /**
     * Notify related contact my capability
     * 
     * @param int capability type
     * @param boolean capability
     * @return void
     * @throws ClientApiException
     */
    public void notifyCapabilityChanged(int capabilityType, boolean capability) {
        Logger.d(TAG, "notifyCapabilityChanged entry");
        mContactList.clear();
        doCapabilityChanged(capabilityType, capability);
        notifyCapabilityChanged();
        Logger.d(TAG, "notifyCapabilityChanged exit");
    }
}
