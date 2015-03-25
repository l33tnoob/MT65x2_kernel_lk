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

package com.mediatek.rcse.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.rcse.service.ICapabilities;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.util.concurrent.CopyOnWriteArrayList;

//This class provides an access to query and observe the capabilities.
public class CapabilityApi extends ICapabilityRemoteListener.Stub implements RemoteRcseApi {
    public static final String TAG = "CapabilityApi";
    /**
     * If the name of an input contact is "Me", the returned capabilities will
     * be my capabilities.
     */
    public static final String ME = "Me";
    private Context mContext = null;
    private ICapabilities mICapabilities = null;
    private CopyOnWriteArrayList<ICapabilityListener> mListeners =
            new CopyOnWriteArrayList<ICapabilityListener>();

    /**
     * Constructor of CapabilityApi. It's recommended that each application
     * reserves no more than one CapabilityApi instance.
     * 
     * @param context This Context should not be null, otherwise the constructor
     *            will threw a RuntimeException.
     */
    public CapabilityApi(Context context) {
        if (null == context) {
            Logger.e(TAG, "Constructor: context is null");
            throw new RuntimeException("Illigal input context, it's null!!");
        }
        mContext = context;
    }

    /**
     * Query my capabilities.
     * 
     * @return My capabilities.
     */
    public Capabilities getMyCapabilities() {
        Logger.v(TAG, "getMyCapabilities() entry");
        if (null != mICapabilities) {
            try {
                Capabilities myCapabilities = mICapabilities.getMyCapabilities();
                Logger.i(TAG, "getMyCapabilities() exit, myCapabilities is " + myCapabilities);
                return myCapabilities;
            } catch (RemoteException e) {
                e.printStackTrace();
                return null;
            }
        } else {
            throw new RuntimeException("Connect this Api before calling this method!");
        }
    }

    public void setMyCapabilities(Capabilities myCapabilities) {
        // TODO
    }

    /**
     * Query the capabilities of a specific contact.
     * 
     * @param contact The contact whose capabilities you want to query. Setting
     *            this parameter to be "Me" indicates that the target contact is
     *            the phone owner.
     * @return The capabilities of a specific contact you're querying
     */
    public Capabilities getContactCapabilities(String contact) {
        Logger.v(TAG, "getContactCapabilities() entry, the contact is " + contact);
        if (!ME.equals(contact)) {
            if (null != mICapabilities) {
                try {
                    Capabilities contactCapabilities =
                            mICapabilities.getContactCapabilities(contact);
                    Logger.i(TAG, "getContactCapabilities() exit, contactCapabilities is "
                            + contactCapabilities);
                    return contactCapabilities;
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return null;
                }
            } else {
                throw new RuntimeException("Connect this Api before calling this method!");
            }
        } else {
            return getMyCapabilities();
        }

    }

	

	 public Capabilities getContactCurentCapabilities(String contact) {
        Logger.v(TAG, "getContactCurentCapabilities() entry, the contact is " + contact);
        if (!ME.equals(contact)) {
            if (null != mICapabilities) {
                try {
                    Capabilities contactCapabilities =
                            mICapabilities.getContactCurentCapabilities(contact);
                    Logger.i(TAG, "getContactCurentCapabilities() exit, contactCapabilities is "
                            + contactCapabilities);
                    return contactCapabilities;
                } catch (RemoteException e) {
                    e.printStackTrace();
                    return null;
                }
            } else {
                throw new RuntimeException("Connect this Api before calling this method!");
            }
        } else {
            return getMyCapabilities();
        }

    }

	/**
     * Query the capabilities of a specific contact.
     * 
     * @param contact The contact whose capabilities you want to query. Setting
     *            this parameter to be "Me" indicates that the target contact is
     *            the phone owner.
     * @return The capabilities of a specific contact you're querying
     */
    public void refreshContactCapabilities(String contact) {
        Logger.v(TAG, "refreshContactCapabilities() entry, the contact is " + contact);
        if (!ME.equals(contact)) {
            if (null != mICapabilities) {
                try {
                    mICapabilities.refreshContactCapabilities(contact);
                    Logger.i(TAG, "refreshContactCapabilities() exit, contactCapabilities is ");
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            } else {
                throw new RuntimeException("Connect this Api before calling this method!");
            }
        }
    }

    /**
     * Listeners used to observe the changes of capabilities.
     */
    public interface ICapabilityListener {
        /**
         * Call when the capabilities have changed.
         * 
         * @param contact The contact whose capabilities have changed.
         * @param capabilities The current capabilities of the contact.
         */
        void onCapabilityChanged(String contact, Capabilities capabilities);
    }

    /**
     * Register a CapabilityListener.
     * 
     * @param listener The listener you want to register.
     */
    public void registerCapabilityListener(ICapabilityListener listener) {
        Logger.v(TAG, "registerCapabilityListener() entry");
        Logger.w(TAG, "registerCapabilityListener() entry");
        mListeners.add(listener);
        Logger.v(TAG, "registerCapabilityListener() exit");
    }

    /**
     * Unregister the CapabilityListener that once registered.
     * 
     * @param listener The listener you want to unregister.
     * @return true If the input listener was registered and now has been
     *         removed. false If the input listener wasn't registered and no
     *         need to remove.
     */
    public boolean unregisterCapabilityListener(ICapabilityListener listener) {
        Logger.v(TAG, "unregisterCapabilityListener() entry");
        Logger.w(TAG, "unregisterCapabilityListener() entry");
        boolean result = mListeners.remove(listener);
        Logger.v(TAG, "unregisterCapabilityListener() exit, the result is " + result);
        return result;
    }

    public void onCapabilityChanged(String contact, Capabilities capabilities)
            throws RemoteException {
        Logger.v(TAG, "onCapabilityChanged() entry, contact = " + contact);
        for (ICapabilityListener listener : mListeners) {
            listener.onCapabilityChanged(contact, capabilities);
        }
    }

    protected ServiceConnection mCapabilityConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Logger.v(TAG, "onServiceConnected() entry");
            mICapabilities = ICapabilities.Stub.asInterface(service);
            if (null != mICapabilities) {
                try {
                    mICapabilities.addCapabilityListener(CapabilityApi.this);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            } else {
                Logger.e(TAG, "onServiceConnected() mCoreStatus is null");
            }
            handleConnected();
            Logger.v(TAG, "onServiceConnected() exit");
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            Logger.v(TAG, "onServiceDisconnected() entry");
            mICapabilities = null;
            handleDisconnected();
            Logger.v(TAG, "onServiceDisconnected() exit");
        }
    };

    @Override
    public void connect() {
        Logger.v(TAG, "connect() entry");
        Logger.w(TAG, "connect() entry");
        boolean result =
                mContext.bindService(new Intent(ICapabilities.class.getName()),
                        mCapabilityConnection, Context.BIND_AUTO_CREATE);
        Logger.v(TAG, "connect() exit, the result is " + result);
        Logger.w(TAG, "connect() exit, the result is " + result);
    }

    /*
     * (non-Javadoc)
     * @see com.mediatek.rcse.api.RemoteRcseApi#disconnect()
     */
    @Override
    public void disconnect() {
        Logger.v(TAG, "disconnect() entry");
        mContext.unbindService(mCapabilityConnection);
        Logger.v(TAG, "disconnect() exit");
    }

    /*
     * (non-Javadoc)
     * @see com.mediatek.rcse.api.RemoteRcseApi#handleConnected()
     */
    @Override
    public void handleConnected() {
        // TODO Auto-generated method stub

    }

    /*
     * (non-Javadoc)
     * @see com.mediatek.rcse.api.RemoteRcseApi#handleDisconnected()
     */
    @Override
    public void handleDisconnected() {
        // TODO Auto-generated method stub

    }
}
