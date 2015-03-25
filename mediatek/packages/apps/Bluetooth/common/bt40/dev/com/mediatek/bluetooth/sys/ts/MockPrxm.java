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

import java.util.Random;

import com.mediatek.bluetooth.ilm.MessageService;
import com.mediatek.bluetooth.prx.PrxMsg;
import com.mediatek.bluetooth.psm.PsmMessage;
import com.mediatek.bluetooth.util.BtLog;

public class MockPrxm {

    private MessageService service;

    private Random rnd = new Random( System.currentTimeMillis() );

    private Byte[] CONN_RESP_CODE = { 0, 1, 0 };
    private Integer[] CAPABILITY = { 2, 2, 2, 2 };
    private Byte[] TX_POWER = { 5, 10, 15, 20 };
    private Byte[] RSSI = { -100, -60, -20, -18, -13, -8, 0, 1, 2, 3, 4, 8, 10, 11, 12, 13, 15, 18 };

    private Object nextRnd( Object[] src ){

        return src[rnd.nextInt( src.length )];
    }

    public MockPrxm( MessageService service ){

        this.service = service;
    }

    public void onMessageReceived( PsmMessage message ) {

        int mid = message.getId();
        switch( mid ){
            case PrxMsg.MSG_ID_BT_PRXM_CONNECT_REQ:
                PsmMessage cnf1 = new PsmMessage( PrxMsg.PRXM_CONNECT_CNF, message.getIndex() );
                cnf1.setByte( PrxMsg.PRXM_CONNECT_CNF_B_RSPCODE, (Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf1);
                BtLog.d( "Connect Rsp:" + cnf1.getByte( PrxMsg.PRXM_CONNECT_CNF_B_RSPCODE ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_GET_CAPABILITY_REQ:
                PsmMessage cnf2 = new PsmMessage( PrxMsg.PRXM_GET_CAPABILITY_CNF, message.getIndex() );
                cnf2.setInt( PrxMsg.PRXM_GET_CAPABILITY_CNF_I_CAPABILITY, (Integer)nextRnd(CAPABILITY) );
                this.service.send(cnf2);
                BtLog.d( "Capability:" + cnf2.getInt( PrxMsg.PRXM_GET_CAPABILITY_CNF_I_CAPABILITY ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_DISCONNECT_REQ:
                PsmMessage cnf3 = new PsmMessage( PrxMsg.PRXM_DISCONNECT_IND, message.getIndex() );
                cnf3.setByte( PrxMsg.PRXM_DISCONNECT_IND_B_RSPCODE,(Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf3);
                BtLog.d( "Disconnect Rsp:" + cnf3.getByte( PrxMsg.PRXM_DISCONNECT_IND_B_RSPCODE ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_REQ:
                PsmMessage cnf4 = new PsmMessage( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF, message.getIndex() );
                cnf4.setByte( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_TXPOWER, (Byte)nextRnd(TX_POWER) );
                this.service.send(cnf4);
                BtLog.d( "TxPower:" + cnf4.getByte( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_TXPOWER ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_GET_RSSI_REQ:
                PsmMessage cnf5 = new PsmMessage( PrxMsg.PRXM_GET_RSSI_CNF, message.getIndex() );
                cnf5.setByte( PrxMsg.PRXM_GET_RSSI_CNF_B_RSSI, (Byte)nextRnd(RSSI) );
                this.service.send(cnf5);
                BtLog.d( "TxPower:" + cnf5.getByte( PrxMsg.PRXM_GET_RSSI_CNF_B_RSSI ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_SET_LINKLOSS_REQ:
                PsmMessage cnf6 = new PsmMessage( PrxMsg.PRXM_SET_LINKLOSS_CNF, message.getIndex() );
                cnf6.setByte( PrxMsg.PRXM_SET_LINKLOSS_CNF_B_RSPCODE, (Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf6);
                BtLog.d( "Set Linkloss Rsp:" + cnf6.getByte( PrxMsg.PRXM_SET_LINKLOSS_CNF_B_RSPCODE ) );
                break;
            case PrxMsg.MSG_ID_BT_PRXM_SET_PATHLOSS_REQ:
                PsmMessage cnf7 = new PsmMessage( PrxMsg.PRXM_SET_PATHLOSS_CNF, message.getIndex() );
                cnf7.setByte( PrxMsg.PRXM_SET_PATHLOSS_CNF_B_RSPCODE, (Byte)nextRnd(CONN_RESP_CODE) );
                this.service.send(cnf7);
                BtLog.d( "Set Pathloss Rsp:" + cnf7.getByte( PrxMsg.PRXM_SET_PATHLOSS_CNF_B_RSPCODE ) );
                break;
            default:
                BtLog.w( "[PRXM] unsupported message: " + message.toPrintString() );
        }
    }
}
