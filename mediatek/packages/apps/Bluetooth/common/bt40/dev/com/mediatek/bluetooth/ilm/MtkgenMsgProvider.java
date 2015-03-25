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
import java.util.ArrayList;
import java.util.List;

import com.mediatek.bluetooth.prx.PrxMsg;
import com.mediatek.bluetooth.util.BtLog;

public class MtkgenMsgProvider implements MsgProvider {

    public boolean compareMessage( Message m1, Message m2 ){

        ByteBuffer bb1 = m1.getBuffer();
        ByteBuffer bb2 = m2.getBuffer();
        BtLog.e( "b1:" + this.printByteBuffer(bb1) );
        BtLog.e( "b2:" + this.printByteBuffer(bb2) );
        return bb1.equals( bb2 );
    }
    private String printByteBuffer( ByteBuffer bb ){

        StringBuilder out = new StringBuilder();
        out.append( "pos[" ).append( bb.position() )
           .append( "], limit[" ).append( bb.limit() )
           .append( "], cap[" ).append( bb.capacity() ).append( "], data[" );
        for( int i=bb.position(); i<bb.limit(); i++ ){

            out.append( bb.get(i) ).append( "," );
        }
        out.append( "]" );
        return out.toString();
    }

    public Message createMessage( int msgId, ByteBuffer data ){

        return new Message( msgId, data );
    }

    public int getLoopCount(){

        return 5;
    }

    public List<Message> getMessageList(){

        ArrayList<Message> result = new ArrayList<Message>();

//        // msg1
//        Message msg1 = new Message( PrxMsg.PRXM_CONNECT_REQ );
//        msg1.setByte( PrxMsg.PRXM_CONNECT_REQ_B_INDEX, (byte)1 );
//        msg1.setByteArray( PrxMsg.PRXM_CONNECT_REQ_BA_ADDR, PrxMsg.PRXM_CONNECT_REQ_BL_ADDR, new byte[]{ 1, 2, 3, 4, 5, 6 } );
//        result.add( msg1 );
//
//        // msg2
//        Message msg2 = new Message( PrxMsg.PRXM_GET_CAPABILITY_CNF );
//        msg2.setByte( PrxMsg.PRXM_GET_CAPABILITY_CNF_B_INDEX, (byte)2);
//        msg2.setInt( PrxMsg.PRXM_GET_CAPABILITY_CNF_I_CAPABILITY, 222 );
//        result.add(msg2);
//
//        // msg3
//        Message msg3 = new Message( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF );
//        msg3.setByte( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF_B_INDEX, (byte)3 );
//        msg3.setInt( PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF_I_TXPOWER, 333 );
//        result.add(msg3);
//
//        // msg4
//        Message msg4 = new Message( PrxMsg.PRXM_GET_RSSI_CNF );
//        msg4.setByte( PrxMsg.PRXM_GET_RSSI_CNF_B_INDEX, (byte)4 );
//        msg4.setInt( PrxMsg.PRXM_GET_RSSI_CNF_I_RSSI, 444 );
//        result.add(msg4);
//
//        // msg5
//        Message msg5 = new Message( PrxMsg.PRXR_UPDATE_TXPOWER_CNF );
//        msg5.setByte( PrxMsg.PRXR_UPDATE_TXPOWER_CNF_B_INDEX, (byte)5 );
//        msg5.setInt( PrxMsg.PRXR_UPDATE_TXPOWER_CNF_I_TXPOWER, 555 );
//        result.add(msg5);

        Message msg;
        java.util.Random rnd = new java.util.Random( System.currentTimeMillis() );
        msg=new Message(PrxMsg.PRXM_CONNECT_REQ);
        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
        msg.setByteArray(5,6, new byte[]{61,71,109,11,84,113}); // ADDR
        result.add(msg);

        msg=new Message(PrxMsg.PRXM_CONNECT_CNF);
        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
        result.add(msg);

//        msg=new Message(PrxMsg.PRXM_DISCONNECT_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_DISCONNECT_IND);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_CAPABILITY_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_CAPABILITY_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setInt(2,(int)rnd.nextInt(Integer.MAX_VALUE)); // CAPABILITY
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_REMOTE_TXPOWER_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_REMOTE_TXPOWER_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setInt(2,(int)rnd.nextInt(Integer.MAX_VALUE)); // TXPOWER
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_SET_PATHLOSS_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // LEVEL
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_SET_PATHLOSS_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_SET_LINKLOSS_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // LEVEL
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_SET_LINKLOSS_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_RSSI_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXM_GET_RSSI_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setInt(2,(int)rnd.nextInt(Integer.MAX_VALUE)); // RSSI
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_REGISTER_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_REGISTER_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_UNREGISTER_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_UNREGISTER_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_CONNECT_IND);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByteArray(5,6, new byte[]{112,63,39,14,82,84}); // ADDR
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_CONNECT_RSP);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // RSPCODE
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_DISCONNECT_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_DISCONNECT_IND);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_PATHLOSS_IND);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // LEVEL
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_LINKLOSS_IND);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setByte(5,(byte)rnd.nextInt(Byte.MAX_VALUE)); // LEVEL
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_UPDATE_TXPOWER_REQ);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        result.add(msg);
//
//        msg=new Message(PrxMsg.PRXR_UPDATE_TXPOWER_CNF);
//        msg.setByte(4,(byte)rnd.nextInt(Byte.MAX_VALUE)); // INDEX
//        msg.setInt(2,(int)rnd.nextInt(Integer.MAX_VALUE)); // TXPOWER
//        result.add(msg);

        return result;
    }


}
