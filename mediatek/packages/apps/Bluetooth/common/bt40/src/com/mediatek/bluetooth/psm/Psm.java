/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.psm;

import android.app.Service;
import android.os.Looper;

import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.IdentityList;

/**
 * @author Jerry Hsu
 *
 * Profile State Machine (for each profile)
 *
 * Communication:
 *
 *     1. Binder - Up:     : messageListener ?
 *     2. Binder - Down:    : Psm.sendMessage() -> Psm.handleMessage()
 *     3. PsmService - Up    : Psm.handleMessage()
 *     4. PsmService - Down    : PsmService.send()
 *
 */
public abstract class Psm {

    protected int profile;        // profile id
    protected int minMessageId;    // min message id for profile
    protected int maxMessageId;    // max message id for profile
    protected int defaultConnCount, maxConnCount;

    protected PsmService psmService;            // reference to PsmService
    protected PsmServiceBinder psmServiceBinder;        // reference to PsmServiceBinder
    protected IdentityList<PsmConnection> psmConnList;    // profile connection list

    /**
     * Constructor
     *
     * @param profile
     * @param psmService
     * @param minMessageId
     * @param maxMessageId
     * @param defaultConnCount
     * @param maxConnCount
     */
    protected Psm( int profile, PsmService psmService, int minMessageId, int maxMessageId, int defaultConnCount, int maxConnCount ){

        // define profile attribute
        this.profile = profile;
        this.psmService = psmService;
        this.minMessageId = minMessageId;
        this.maxMessageId = maxMessageId;
        this.defaultConnCount = defaultConnCount;
        this.maxConnCount = maxConnCount;
    }

/******************************************************************************************
 * Lifecycle callback functions - BEG
 ******************************************************************************************/

    /**
     * Service onCreate() - will be called when Bluetooth Power On
     */
    protected void onServiceCreate(){

        // enable profile service here (e.g. FTP Server / Prxr) and accept incoming request
    }

    /**
     * Service onBind() - can used to allocate resource here
     *
     * @return
     */
    protected synchronized boolean onServiceBind(){

        // allocate resource when service binding
        if( this.psmConnList == null ){

            this.psmConnList = new IdentityList<PsmConnection>( this.defaultConnCount, this.maxConnCount );
        }

        // create service binder
        if( this.psmServiceBinder == null ){

            // let subclass create service binder object
            this.psmServiceBinder = this.createServiceBinder();

            // call service binder onBind() callback
            if( this.psmServiceBinder != null ){

                this.psmServiceBinder.onServiceBind();
            }
        }
        if( this.psmServiceBinder == null ){

            BtLog.w( "onServiceBind() failed: psmServiceBinder is null [" + this.getClass().getName() + "]" );
        }

        return ( this.psmConnList != null && this.psmServiceBinder != null );
    }

    /**
     * Ask subclass to implement ServiceBinder initialization => called on Service bind
     *
     * @return
     */
    protected abstract PsmServiceBinder createServiceBinder();

    /**
     * Service onDestroy()
     */
    protected void onServiceDestroy(){

        if( this.psmServiceBinder != null ){

            this.psmServiceBinder.onServiceDestroy();
            this.psmServiceBinder = null;
        }

        // release resources -> don't release psmConnList or ASYNC service unregister will fail
        //this.psmService = null;
        //this.psmConnList = null;
    }

/******************************************************************************************
 * Lifecycle callback functions - END
 ******************************************************************************************/

    /**
     * check the given message can be handled by this ProfileFsm
     *
     * @param messageId
     * @return
     */
    public boolean isValidMessage( int messageId ){

        return (this.minMessageId <= messageId && messageId <= this.maxMessageId);
    }

    /**
     * register profile context for new connection and get the connection id
     *
     * @param profileConnection
     * @return connection id for this profile context
     */
    public int registerConnection( PsmConnection profileConnection ){

        // acquire connId (0, 1, 2...) always new (according to connKey, e.g. bd-addr, is implemented by profile)
        int connId = this.psmConnList.registerElement( profileConnection );
        profileConnection.setConnId(connId);
        return connId;
    }

    /**
     * unregister profile context for specific connection id
     *
     * @param connId
     */
    public boolean unregisterConnection( int connId ){

        return this.psmConnList.unregisterElement( connId );
    }

    /**
     * entry point of Profile State Machine, it will handle message from:
     *
     *   1. Upper Layer (binder): usually use PsmService.send() to sned out message to Lower Layer.
     *   2. Lower Layer (native): usually use Psm.recvMessage() to deliver message to Upper Layer
     *
     * @param message
     * @return
     */
    public abstract int handleMessage( PsmMessage message );

    /**
     * send message to native-layer(stack)
     *
     * @param connId
     * @param message
     * @return
     */
    protected void sendMessage( PsmMessage message ){

        this.psmService.send( message );
    }

    /**
     * deliver message to mmi-layer(service binder)
     *
     * @param message
     */
    protected void recvMessage( PsmMessage message ){

        if( this.psmServiceBinder != null ){

            this.psmServiceBinder.onMessageReceived( message );
        }
    }


    /**
     * get profile context for specific connection id ( maybe null )
     *
     * @param connId
     * @return
     */
    public PsmConnection getConnection( int connId ){

        return this.psmConnList.get( connId );
    }

    /**
     * get service binder instance
     */
    public PsmServiceBinder getServiceBinder(){

        return this.psmServiceBinder;
    }

    /**
     * get service instance
     * @return
     */
    public Service getService(){

        return this.psmService;
    }

    /**
     * get looper instance
     *
     * @return
     */
    public Looper getHandlerLooper(){

        return this.psmService.getHandlerLooper();
    }
}