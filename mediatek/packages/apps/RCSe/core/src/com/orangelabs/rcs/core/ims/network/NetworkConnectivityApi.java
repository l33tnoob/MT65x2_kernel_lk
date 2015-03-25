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

package com.orangelabs.rcs.core.ims.network;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.service.api.client.ClientApi;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.CoreServiceNotAvailableException;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Network API service
 */
public class NetworkConnectivityApi extends ClientApi {
    /**
     * The logger
     */
    private Logger mLogger = Logger.getLogger(this.getClass().getName());

    /**
     * Core service API
     */
    private INetworkConnectivityApi mCoreApi = null;

    /**
     * Constructor
     * 
     * @param ctx Application context
     */
    public NetworkConnectivityApi(Context ctx) {
        super(ctx);
    }

    /**
     * Connect API
     */
    public void connectApi() {
        super.connectApi();

        ctx.bindService(new Intent(INetworkConnectivityApi.class.getName()), mApiConnection, 0);
    }

    /**
     * Disconnect API
     */
    public void disconnectApi() {
        super.disconnectApi();

        ctx.unbindService(mApiConnection);
    }

    /**
     * Core service API connection
     */
    private ServiceConnection mApiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mCoreApi = INetworkConnectivityApi.Stub.asInterface(service);

            // Notify event listener
            notifyEventApiConnected();
        }

        public void onServiceDisconnected(ComponentName className) {
            // Notify event listener
            notifyEventApiDisconnected();

            mCoreApi = null;
        }
    };

    /**
     * Add network connectivity listener.
     * 
     * @param listener Network connectivity Listener
     * @throws ClientApiException
     */
    public void addNetworkConnectivityListener(INetworkConnectivity listener)
            throws ClientApiException {
        if (mCoreApi != null) {
            try {
                mCoreApi.addNetworkConnectivityListener(listener);
            } catch (Exception e) {
                throw new ClientApiException(e.getMessage());
            }
        } else {
            throw new CoreServiceNotAvailableException();
        }
    }

    /**
     * Remove network connectivity listener
     * 
     * @param listener Listener
     * @throws ClientApiException
     */
    public void removeNetworkConnectivityListener(INetworkConnectivity listener)
            throws ClientApiException {
        if (mCoreApi != null) {
            try {
                mCoreApi.removeNetworkConnectivityListener(listener);
            } catch (Exception e) {
                throw new ClientApiException(e.getMessage());
            }
        } else {
            throw new CoreServiceNotAvailableException();
        }
    }
}
