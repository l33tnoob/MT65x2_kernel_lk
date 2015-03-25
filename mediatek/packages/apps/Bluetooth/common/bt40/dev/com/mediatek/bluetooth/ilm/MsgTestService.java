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

package com.mediatek.bluetooth.ilm;

import java.nio.ShortBuffer;
import java.util.List;

import android.os.SystemClock;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.util.BtLog;

public class MsgTestService extends MessageService {

    public static final String ACTION_START_SERVICE = "com.mediatek.bluetooth.sys.msg.action.START_SERVICE";

    private MsgProvider msgProvider;

    public MsgTestService( MsgProvider msgProvider ){

        this.msgProvider = msgProvider;
    }

    private void connect(){

        // create server & client sockets and then start to listen incoming messages
        this.serverSocketFd = this.openSocket( true,  this.serverSocketFd, Options.ILM_SOCKET_NAME_EXT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );
        this.clientSocketFd = this.openSocket( false, this.clientSocketFd, Options.ILM_SOCKET_NAME_INT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );    // sys_native.EXT_IL_SOCKET_NAMESPACE_FILESYSTEM

        if( this.serverSocketFd > -1 && this.clientSocketFd > -1 ){

            //this.startListen();
        }
        else {
            BtLog.e( "InterlayerService.onCreate() error: can't create server & client sockets." );
        }
    }

    private void disconnect(){

        // close client and server sockets
        this.closeSocket( this.clientSocketFd );
        this.closeSocket( this.serverSocketFd );
    }

    public void startTest(){

        // prepare native connection
        this.connect();

        // execute test
        int loopCount = msgProvider.getLoopCount();
        int failCount = 0;
        long t1 = SystemClock.elapsedRealtime();
        for( int i=0; i<loopCount; i++ ){

            List<Message> msgList = msgProvider.getMessageList();
            for( Message req : msgList ){

                // send to native
                this.send( req );
                Message cnf = this.recv();
                if( !this.msgProvider.compareMessage( req, cnf ) ){

                    failCount++;
                }
            }
        }
        long t2 = SystemClock.elapsedRealtime();
        long diff = t2-t1;
        BtLog.i( "[*] Test Result: Fail[" + failCount + "], Total[" + diff + "], Count[" + loopCount + "], Avg[" + (diff/loopCount) + "]" );

        // release native connection
        this.disconnect();
    }

    protected Message recv(){

        try {
            Message ilm = new Message( Message.ILM );
            int res = ilm_native.recv_message( serverSocketFd, ilm );
            if( res == 1 ){

                int msgId = (int)ilm.getInt( Message.ILM_I_MSG_ID );
                // get message size according to short 1
                ShortBuffer sb = ilm.getBuffer( Message.ILM_BA_ILM_DATA, 4 ).asShortBuffer();
                int size = sb.get(1);
                return msgProvider.createMessage( msgId, ilm.getBuffer( Message.ILM_BA_ILM_DATA, size ) );
            }
            else {
                BtLog.w( "recv(): revc_message() failed" );
            }
        }
        catch( Exception ex ){

            BtLog.e( "recv() error: ", ex );
        }
        return null;
    }

//    @Override
//    protected void startListen(){
//
//        // loop until end event
//        new Thread(){
//
//            public void run(){
//
//                int res = -1;
//                isListening = true;
//
//                while( isListening ){
//
//                    BtLog.d( "InterlayerService.startListen() - listening message..." );
//
//                    try {
//                        ilm_struct ilm = ilm_struct.create();
//                        res = sys_native.recv_message( serverSocketFd, ilm );
//                        if( res == 1 ){
//
//                            int msgId = (int)ilm.msg_id();
//                            // check if STOP request is received
//                            if( msgId == InterlayerMessage.MSG_ID_SYS_SHUTDOWN_SERVICE_REQ ){
//
//                                break;
//                            }
//
//                            // get message size according to short 1
//                            ShortBuffer sb = ilm.ilm_data(4).asShortBuffer();
//                            int size = sb.get(1);
//
//                            // dispatch message to registered listeners
//                            for( InterlayerMessageListener listener: listenerList ){
//
//                                listener.onMessageReceived( msgProvider.createMessage( msgId, ilm.ilm_data(size) ) );
//                            }
//                        }
//                        else {
//                            BtLog.w( "InterlayerService.startListen(): revc_message() failed" );
//                        }
//                    }
//                    catch( Exception ex ){
//
//                        BtLog.e( "InterlayerService.startListen() error: ", ex );
//                    }
//                }
//            }
//        }.start();
//    }

}
