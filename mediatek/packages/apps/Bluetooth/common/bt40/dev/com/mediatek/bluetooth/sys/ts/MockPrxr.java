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

import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import com.mediatek.bluetooth.ResponseCode;
import com.mediatek.bluetooth.ilm.MessageService;
import com.mediatek.bluetooth.prx.PrxMsg;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.util.BtLog;

public class MockPrxr {

    private MessageService service;

    private Random rnd = new Random( System.currentTimeMillis() );

    private int state = 0;    // 0: new / 1:registered / 2:connected

    private Byte[] CONN_RESP_CODE = { ResponseCode.SUCCESS, ResponseCode.SUCCESS, ResponseCode.SUCCESS };
    private Byte[] LEVEL = { 0, 0, 1, 1, 2, 2 };

    private Object nextRnd( Object[] src ){

        return src[rnd.nextInt( src.length )];
    }

    public MockPrxr( MessageService service ){

        this.service = service;
    }

//    private void sleep( long period ){
//
//        try {
//            Thread.sleep(period);
//
//        } catch (InterruptedException e) {
//
//            e.printStackTrace();
//        }
//    }

    public void onMessageReceived( PsmMessage message ) {


        int mid = message.getId();
        switch( mid ){

            case PrxMsg.MSG_ID_BT_PRXR_REGISTER_REQ:
                PsmMessage cnf1 = new PsmMessage( PrxMsg.PRXR_REGISTER_CNF, message.getIndex() );
                cnf1.setByte( PrxMsg.PRXR_REGISTER_CNF_B_RSPCODE, (Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf1);
                BtLog.d( "Register Rsp:" + cnf1.getByte( PrxMsg.PRXR_REGISTER_CNF_B_RSPCODE ) );
                if( cnf1.getByte( PrxMsg.PRXR_REGISTER_CNF_B_RSPCODE ) == ResponseCode.SUCCESS ){

                    // registered
                    this.state = 1;
                    this.startConnectedEvent( message.getIndex() );
                }
                break;

            case PrxMsg.MSG_ID_BT_PRXR_DEREGISTER_REQ:
                // disconnect
                PsmMessage ind1 = new PsmMessage( PrxMsg.PRXR_DISCONNECT_IND, message.getIndex() );
                this.service.send(ind1);
                //this.sleep( 1000 );
                this.state = 1;

                // unregister
                PsmMessage cnf2 = new PsmMessage( PrxMsg.PRXR_DEREGISTER_CNF, message.getIndex() );
                cnf2.setByte( PrxMsg.PRXR_DEREGISTER_CNF_B_RSPCODE, (Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf2);
                this.state = 0;
                this.stopConnectedEvent( message.getIndex() );
                BtLog.d( "Unregister Rsp:" + cnf2.getByte( PrxMsg.PRXR_DEREGISTER_CNF_B_RSPCODE ) );
                break;

            case PrxMsg.MSG_ID_BT_PRXR_DISCONNECT_REQ:
                PsmMessage ind2 = new PsmMessage( PrxMsg.PRXR_DISCONNECT_IND, message.getIndex() );
                this.service.send(ind2);
                this.state = 1;
                BtLog.d( "Disconnect Ind: index=" + ind2.getIndex() );
                break;

            case PrxMsg.MSG_ID_BT_PRXR_AUTHORIZE_RSP:
                byte rsp = message.getByte( PrxMsg.PRXR_AUTHORIZE_RSP_B_RSPCODE );
                if( rsp == ResponseCode.SUCCESS ){

                    this.state = 2;
                }
                BtLog.d( "Connected" );
                break;
            default:
                BtLog.w( "[PRXR] unsupported message: " + message.toPrintString() );
        }
    }


    private Map<Byte, ConnectedEventTask> taskMap = new HashMap<Byte, ConnectedEventTask>();

    private void startConnectedEvent( byte connId ){

        ConnectedEventTask task = this.taskMap.get(connId);
        if( task == null ){

            task = new ConnectedEventTask(connId);
            taskMap.put( connId, task );
            task.start();
        }
    }

    private void stopConnectedEvent( byte connId ){

        ConnectedEventTask task = this.taskMap.remove(connId);
        if( task != null ){

            task.stop();
        }
    }

    class ConnectedEventTask extends TimerTask {

        private Timer timer = new Timer();
        private byte connId;

        public ConnectedEventTask( byte connId ){

            this.connId = connId;
        }

        public void stop(){

            this.timer.cancel();
        }

        public void start(){

            timer.schedule( this, rnd.nextInt( 5000 ), 2000 + rnd.nextInt( 5000 ) );
        }

        public void run(){

            // registered
            if( state == 1 ){

                PsmMessage con = new PsmMessage( PrxMsg.PRXR_CONNECT_IND, this.connId );
                con.setByteArray( PrxMsg.PRXR_CONNECT_IND_BA_ADDR, PrxMsg.PRXR_CONNECT_IND_BL_ADDR, new byte[]{ 1,1,1,1,1,1 } );
                service.send(con);
                BtLog.d( "Connect Ind: index=" + con.getIndex() );
            }
            else if( state == 2 ){

                boolean isPathLoss = rnd.nextBoolean();
                if( isPathLoss ){
                    PsmMessage pli = new PsmMessage( PrxMsg.PRXR_PATHLOSS_IND, this.connId );
                    pli.setByte( PrxMsg.PRXR_PATHLOSS_IND_B_LEVEL, (Byte)nextRnd(LEVEL) );
                    service.send(pli);
                }
                else {
                    PsmMessage lli = new PsmMessage( PrxMsg.PRXR_LINKLOSS_IND, this.connId );
                    lli.setByte( PrxMsg.PRXR_LINKLOSS_IND_B_LEVEL, (Byte)nextRnd(LEVEL) );
                    service.send(lli);
                }
            }
        }
    }
}
