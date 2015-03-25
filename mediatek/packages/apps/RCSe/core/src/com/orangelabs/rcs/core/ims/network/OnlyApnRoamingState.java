/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.network;


import android.content.Context;
import android.content.Intent;

/**
 * HTTPS provisioning - Input of MSISDN
 *
 * @author Orange
 */
public final class OnlyApnRoamingState {

    /**
     * HttpsProvionningMSISDNInput instance
     */
    private static volatile OnlyApnRoamingState instance = null;

    /**
     * MSISDN
     */
    private Boolean roaming_state;

    /**
     * Constructor
     */
    private OnlyApnRoamingState() {
        super();
    }

    /**
     * Get the MSISDN
     *
     * @return MSISDN
     */
    protected Boolean getResponse() {
        return roaming_state;
    }

    /**
     * Returns the Instance of HttpsProvionningMSISDNDialog
     *
     * @return Instance of HttpsProvionningMSISDNDialog
     */
    public final static OnlyApnRoamingState getInstance() {
        if (OnlyApnRoamingState.instance == null) {
            synchronized (OnlyApnRoamingState.class) {
                if (OnlyApnRoamingState.instance == null) {
                	OnlyApnRoamingState.instance = new OnlyApnRoamingState();
                }
            }
        }
        return OnlyApnRoamingState.instance;
    }

    /**
     * Display the MSISDN popup
     *
     * @param context
     * @return 
     */
    protected Boolean displayPopupAndWaitResponse(Context context) {
        Intent intent = new Intent(context, OnlyApnRoamingAlertDialog.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
        try {
            synchronized (OnlyApnRoamingState.instance) {
                super.wait();
            }
        } catch (InterruptedException e) {
            // nothing to do
        }

        return roaming_state;
    }

    /**
     * Callback of the MSISDN
     *
     * @param value
     */
    protected void responseReceived(Boolean value) {
        synchronized (OnlyApnRoamingState.instance) {
            roaming_state = value;
            super.notify();
        }
    }
}
