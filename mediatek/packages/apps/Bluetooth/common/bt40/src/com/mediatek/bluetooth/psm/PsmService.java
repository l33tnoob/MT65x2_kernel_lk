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

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;

import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.Handler.Callback;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ilm.MessageListener;
import com.mediatek.bluetooth.ilm.MessageService;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.ClassUtils;

/**
 * @author Jerry Hsu
 *
 * Profile State Machine Service:
 *
 * 1. single instance of service for all profiles.
 * 2. register / unregister profile.
 * 3. dispatch message to registered profile.
 *
 *
 * Objective: handle message (object) according to current state and provide sync / async options.
 *
 *     1. State: INIT / SIGNALLING (CONNECTING, DISCONNECTING) / CONNECTED / BUSY
 *     4. handle thread wait and notify under sync mode.
 *     5. callback listener when result is available.
 *
 */
public abstract class PsmService extends MessageService implements MessageListener, Callback {

    /**
     * keep service binder instance for registered profile: <Action, PsmServiceBinder>
     */
    private HashMap<String, Psm> psmActionMap;

    /**
     * keep the registered profile object(s) - used to poll profiles to handle the incoming message(s)
     */
    private ArrayList<Psm> psmList;

    /**
     * handler thread with priority: Process.THREAD_PRIORITY_BACKGROUND
     */
    private HandlerThread handlerThread;

    /**
     * Handler to process message from BT stack (let all profiles can run under thread with prepared looper)
     */
    private Handler handler;

    /*************************************************************************************************
     * Public API
     *************************************************************************************************/

    @Override
    public void onCreate(){

        BtLog.i( "PsmService.onCreate()[+]" );

        // initialize InterlayerService
        super.onCreate();

        // create objects
        this.psmActionMap = new HashMap<String, Psm>( this.getProfileCount() );
        this.psmList = new ArrayList<Psm>( this.getProfileCount() );
        this.registerProfileServices();

        // init thread and handler
        this.handlerThread = new HandlerThread( "PsmServiceHandler", Process.THREAD_PRIORITY_BACKGROUND );
        this.handlerThread.start();
        this.handler = new Handler( this.getHandlerLooper(), this );

        // start listening message from internal layer
        this.registerMessageListener( this );

        // create psm objects
        for( Psm psm : this.psmList ){
            psm.onServiceCreate();
        }
    }

    @Override
    public void onDestroy(){

        BtLog.i( "PsmService.onDestroy()[+]" );

        // destroy psm objects
        for( Psm psm : this.psmList ){
            psm.onServiceDestroy();
        }

        // stop listening message from internal layer
        this.unregisterMessageListener( this );

        // destroy thread and handler
        this.handlerThread.quit();
        try {
            this.handlerThread.join(1000);
        }
        catch( InterruptedException e ){

            BtLog.w( "PsmServiceHandler thread is not stopped: " + e.getMessage() );
        }
        this.handlerThread = null;

        // release objects
        this.psmList.clear();    // set null maybe let handleMessage() get a NPE
        this.psmActionMap = null;

        // destroy InterlayerService
        super.onDestroy();
    }

    @Override
    public IBinder onBind( Intent intent ){

        BtLog.i( "PsmService.onBind()[+]" );

        // check action
        String action = intent.getAction();
        if( action == null ){

            BtLog.e( "onBind() error: invalid action(null)" );
            return null;
        }

        // get psm object and use it to create service binder
        Psm psm = this.psmActionMap.get( action );
        if( psm != null && psm.onServiceBind() ){

            return psm.getServiceBinder().getBinder();
        }
        else {
            String msg = (psm == null) ? "psm is null" : "psm.onServiceBind() failed";
            BtLog.e( "onBind() error: cann't find service for action:[" + action + "], reason[" + msg + "]" );
            return null;
        }
    }

    /**
     * let subclass to register all profile psm objects
     */
    protected abstract int getProfileCount();
    protected abstract void registerProfileServices();

    /**
     * register profile state-machine
     *
     * @param profileFsm
     */
    protected synchronized void registerProfile( String action, String className ){

        BtLog.i( "PsmService.registerProfile()[+]" );

        if( Options.LL_DEBUG ){

            BtLog.d( "PsmService.registerProfile(): action[" + action + "], profile[" + className + "]" );
        }

        // instantiate psm object
        Class<?>[] paramType = { PsmService.class };
        Object[] paramValue = { this };
        Psm psm = (Psm)ClassUtils.newObject( className, paramType, paramValue );
        if( psm == null ){

            BtLog.e( "PsmService.registerProfile() error: className[" + className + "]" );
            return;
        }

        // doesn't need to check parameters: both action and profileFsm are checked by caller
        this.psmActionMap.put( action, psm );
        this.psmList.add( psm );
    }

    /**
     * dispatch incoming message to profile
     */
    public void onMessageReceived( int messageId, ByteBuffer content ){

        // deliver message to handler (use another HandlerThread to process message)
        this.handler.sendMessage( this.handler.obtainMessage( messageId, content ) );
    }

    /**
     * implements android.os.Handler.Callback interface
     */
    public boolean handleMessage( Message msg ){

        if( Options.LL_DEBUG ){

            BtLog.d( "handleMessage()[+]: msgId[" + msg + "]" );
        }

        // convert parameters
        int msgId = msg.what;
        ByteBuffer content = (ByteBuffer)msg.obj;

        // find proper ProfileFsm to handle this message
        boolean isMessageHandled = false;
        PsmMessage message = new PsmMessage( msgId, content );
        try {
            for( Psm psm : this.psmList ){
                if( psm.isValidMessage( msgId ) ){
                    // handle message by ProfileFsm ( e.g. send out ilm message, handle incoming indication )
                    psm.handleMessage( message );
                    isMessageHandled = true;
                }
            }
        }
        catch( Exception ex ){

            BtLog.e( "Fail to loop psm-list for message handling: ", ex );
        }

        // report error if no ProfileFsm can handle this message
        if( !isMessageHandled ){
            BtLog.w( "cann't find Profile to handle received message:" + message.toPrintString() );
        }

        // skip handler's implementation if any
        return true;
    }

    /**
     * get the Looper object in PsmService for Handler processing
     *
     * @return
     */
    public Looper getHandlerLooper(){

        if( this.handlerThread == null ){

            BtLog.e( "fail to get looper from PsmService (handlerThread is null)" );
            return null;
        }
        else {
            return this.handlerThread.getLooper();
        }
    }
}
