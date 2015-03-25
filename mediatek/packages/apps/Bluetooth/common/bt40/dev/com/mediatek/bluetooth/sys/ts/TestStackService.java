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

package com.mediatek.bluetooth.sys.ts;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Random;

import android.content.Intent;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.ilm.Message;
import com.mediatek.bluetooth.ilm.MessageListener;
import com.mediatek.bluetooth.ilm.MessageService;
import com.mediatek.bluetooth.ilm.ilm_native;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.util.BtLog;

public class TestStackService extends MessageService {

    public static final String ACTION_START_STACK_SERVICE = "com.mediatek.bluetooth.sys.ts.action.START_STACK_SERVICE";

    @Override
    public void onCreate(){

        BtLog.i( "TestStackService.onCreate()[+]" );

        // init attributes
        this.isListening = false;
        this.listenerList = new ArrayList<MessageListener>(2);

        // create server & client sockets and then start to listen incoming messages
        this.serverSocketFd = this.openSocket( true,  this.serverSocketFd, Options.ILM_SOCKET_NAME_INT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );
        if( this.serverSocketFd > -1 ){

            this.startListen();
        }
        else {
            BtLog.e( "TestStackService.onCreate() error: can't create server sockets." );
        }
    }

    @Override
    public int onStartCommand( Intent intent, int flags, int startId ) {

        if( ACTION_START_STACK_SERVICE.equals( intent.getAction() ) ){

            this.registerMessageListener( new TsMessageListener(this) );
        }
        return super.onStartCommand( intent, flags, startId );
    }

    @Override
    public void send( Message message ){

        this.clientSocketFd = this.openSocket( false, this.clientSocketFd, Options.ILM_SOCKET_NAME_EXT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );
        super.send( message );
    }
}

class TsMessageListener implements MessageListener {

    private MessageService service;
    private Random rnd = new Random( System.currentTimeMillis() );

    private MockPrxm prxm;
    private MockPrxr prxr;

    public TsMessageListener( MessageService service ){

        this.service = service;

        this.prxm = new MockPrxm( this.service );
        this.prxr = new MockPrxr( this.service );
    }

    public void onMessageReceived( int messageId, ByteBuffer content ) {

        PsmMessage message = new PsmMessage( messageId, content );

        BtLog.i( "[BLE][TS] recv message: " + message.toPrintString() );

        // random delay
        try {
            Thread.sleep( rnd.nextInt(300) );
        }
        catch( InterruptedException e ){

            e.printStackTrace();
        }

        // handle to mock
        this.prxm.onMessageReceived( message );
        this.prxr.onMessageReceived( message );
    }
}
