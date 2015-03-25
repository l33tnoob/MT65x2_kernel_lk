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

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;
import java.util.ArrayList;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.util.BtLog;

/**
 * Objective: communicate with bt stack via ilm message (sned / recv) in async mode.
 *
 *     1. connect to bt stack server socket.
 *     2. send ilm using socket.
 *     3. create server socket for bt stack.
 *     4. recv ilm from socket.
 *
 * Issue
 *
 *     1. message size   => add function: size()
 *     2. ilm message id => add function: getId()
 *
 *  Architecture
 *
 *      1. JNI for send and recv function
 *      2. no thread management in this layer
 */
public class MessageService extends Service implements Runnable {

    /**
     * ilm message used to stop listening thread
     */
    public static final int MSG_ID_SYS_SHUTDOWN_SERVICE_REQ = -1;    // shutdown service request
    private static Message SHUTDOWN_SERVICE_REQUEST = new Message( Message.ILM );
    static {
        SHUTDOWN_SERVICE_REQUEST.setInt( Message.ILM_I_MSG_ID, MSG_ID_SYS_SHUTDOWN_SERVICE_REQ );
    }

    /**
     * client socket fd - connect to internal layer
     */
    protected int clientSocketFd = -1;

    /**
     * server socket fd - listen message from internal layer
     */
    protected int serverSocketFd = -1;

    /**
     * listening control flag
     */
    protected boolean isListening;

    /**
     * keep registered message listener(s)
     */
    protected ArrayList<MessageListener> listenerList;


    /*************************************************************************************************
     * Public API
     *************************************************************************************************/

    @Override
    public void onCreate(){

        BtLog.i( "MessageService.onCreate()[+]" );

        // init attributes
        this.isListening = false;
        this.listenerList = new ArrayList<MessageListener>(2);

        // create server & client sockets and then start to listen incoming messages
        this.serverSocketFd = this.openSocket( true,  this.serverSocketFd, Options.ILM_SOCKET_NAME_EXT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );
        this.clientSocketFd = this.openSocket( false, this.clientSocketFd, Options.ILM_SOCKET_NAME_INT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_FILESYSTEM );    // ilm_native.EXT_IL_SOCKET_NAMESPACE_FILESYSTEM

        if( this.serverSocketFd > -1 && this.clientSocketFd > -1 ){

            this.startListen();
        }
        else {
            BtLog.e( "MessageService.onCreate() error: can't create server & client sockets." );
        }
    }

    @Override
    public void onDestroy(){

        BtLog.i( "MessageService.onDestroy()[+]" );

        // close client socket
        this.closeSocket( this.clientSocketFd );

        // close server socket via shutdown message
        this.stopListen();
    }

    @Override
    public IBinder onBind( Intent intent ){

        BtLog.e( "MessageService.onBind(): unimplemented function." );
        return null;
    }

    /**
     * used to send message to internal adaptation layer
     *
     * @param message
     */
    public void send( Message message ){

        if( Options.LL_DEBUG ){

            BtLog.d( "send()[+]: msgId[" + message.getId() + "]" );
        }

        // TODO [L2][message reuse] cache ilm message (it's too big)
        // init ilm_struct
        Message ilms = new Message( Message.ILM );
        ilms.setInt( Message.ILM_I_MSG_ID, message.getId() );
        ilms.setInt( Message.ILM_I_SRC_MOD_ID, ilm_native.MOD_MMI );    // MOD_MMI - for catcher only
        ilms.setInt( Message.ILM_I_DEST_MOD_ID, ilm_native.MOD_BT );    // MOD_BT - for catcher only

        // copy message data block
        ByteBuffer data = message.getBuffer();
        //ilms.setByteArray( SysMsg.ILM_BA_ilm_data, SysMsg.ILM_BL_ilm_data, data.array() );
        ByteBuffer bb = ilms.getBuffer();
        bb.position( Message.ILM_BA_ILM_DATA );
        bb.put( data );
        data.flip();        // reset buffer

        // send message
        int size = ilms.size() - ilm_native.MAX_ILM_BUFFER_SIZE + message.size();
        ilm_native.send_message( this.clientSocketFd, ilms, size );
    }

    /**
     * register message listener for messages from internal adaptation layer
     *
     * @param messageListener
     */
    public synchronized void registerMessageListener( MessageListener messageListener ){

        this.listenerList.add( messageListener );
    }

    /**
     * unregister message listener
     *
     * @param messageListener
     */
    public synchronized void unregisterMessageListener( MessageListener messageListener ){

        this.listenerList.remove( messageListener );
    }

    /*************************************************************************************************
     * Non-Public API
     *************************************************************************************************/

    /**
     * open 'client' socket for specified parameters and return the socket fd
     *
     * @param socketFd
     * @param socketName
     * @param socketNamespace
     * @return
     */
    protected int openSocket( boolean isServer, int socketFd, String socketName, int socketNamespace ){

        // close socket first if it's opened
        if( socketFd >= 0 ){

            this.closeSocket( socketFd );
        }

        // create socket: client/server
        if( isServer ){

            socketFd = ilm_native.create_server_socket( socketName, socketNamespace );
        }
        else {
            socketFd = ilm_native.create_client_socket( socketName, socketNamespace );
        }

        // check result
        if( socketFd < 0 ){

            BtLog.e( "MessageService.openSocket() error: isServer[" + isServer + "], fd[" + socketFd + "], name[" + socketName + "], namespace[" + socketNamespace + "]" );
        }

        // return result: socketFd
        return socketFd;
    }

    protected void closeSocket( int socketFd ){

        BtLog.d( "MessageService.closeSocket()[+]:" + socketFd );

        if( socketFd >= 0 ){

            ilm_native.close_socket( socketFd );
        }
    }

    /**
     * will block Thread until stopListen is called or connection is terminated
     */
    protected void startListen(){

        BtLog.d( "MessageService.startListen()[+]" );

        // TODO [L3] export Thread object or use caller thread (=> can be config by caller)
        // loop until end event
        new Thread( this, "MessageServiceThread" ).start();
    }

    protected synchronized void stopListen(){

        if( this.isListening ){

            this.isListening = false;

            int fd = ilm_native.create_client_socket( Options.ILM_SOCKET_NAME_EXT_ADP, ilm_native.EXT_IL_SOCKET_NAMESPACE_ABSTRACT );
            ilm_native.send_message(fd, SHUTDOWN_SERVICE_REQUEST, (SHUTDOWN_SERVICE_REQUEST.size() - ilm_native.MAX_ILM_BUFFER_SIZE));
            ilm_native.close_socket(fd);
        }
    }

    public void run(){

        int res = -1;
        this.isListening = true;
        while( this.isListening ){

            if( Options.LL_DEBUG ){

                BtLog.d( "MessageService.run() - listening message..." );
            }

            try {
                // TODO [L2][message reuse] reuse ilm_struct object
                Message ilm = new Message( Message.ILM );
                res = ilm_native.recv_message( serverSocketFd, ilm );
                if( res == 1 ){

                    // debug message
                    if( Options.LL_DEBUG ){

                        BtLog.d( "MessageService.run(): recv message[" + ilm.toPrintString() + "]" );
                    }

                    // message id
                    int msgId = (int)ilm.getInt( Message.ILM_I_MSG_ID );

                    // check if STOP request is received
                    if( msgId == MSG_ID_SYS_SHUTDOWN_SERVICE_REQ )    break;

                    // get message size according to short 1
                    ShortBuffer sb = ilm.getBuffer( Message.ILM_BA_ILM_DATA, 4 ).asShortBuffer();
                    int size = sb.get(1);

                    // dispatch message to registered listeners
                    for( MessageListener listener: listenerList ){

                        listener.onMessageReceived( msgId, ilm.getBuffer( Message.ILM_BA_ILM_DATA, size ) );
                    }
                }
                else {
                    BtLog.w( "MessageService.run(): revc_message() failed" );
                }
            }
            catch( Exception ex ){

                BtLog.e( "MessageService.run() error: ", ex );
            }
        }

        // release resource after socket is idle
        this.closeSocket( this.serverSocketFd );
        this.listenerList = null;
    }
}
