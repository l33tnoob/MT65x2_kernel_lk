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

package com.mediatek.bluetooth.le.demo;

import java.nio.ByteBuffer;

import com.mediatek.bluetooth.ilm.MessageListener;
import com.mediatek.bluetooth.ilm.MessageService;
import com.mediatek.bluetooth.prx.PrxMsg;
import com.mediatek.bluetooth.util.BtLog;

public class SystemDemo {

    public void demoInterlayerService(){

        MessageService ils = new MessageService();

        // init service
        ils.onCreate();

        // register listener(s)
        DefaultInterlayerMessageListener diml = new DefaultInterlayerMessageListener();
        ils.registerMessageListener( diml );

//        // send message
//        bt_prxm_connect_req_struct r1 = bt_prxm_connect_req_struct.create();
//        r1.index((byte)1);
//        r1.addr( "111111".getBytes() );
//        ils.send( r1 );
//        r1.index((byte)2);
//        r1.addr( "222222".getBytes() );
//        ils.send( r1 );
//
//        bt_prxm_connect_cnf_struct r2 = bt_prxm_connect_cnf_struct.create();
//        r2.index((byte)1);
//        r2.rspcode((byte)3);
//        ils.send( r2 );

        // TODO [L3] API need to sync each other (can run without this sleep)
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {

            e.printStackTrace();
        }

        // unregister listener(s)
        ils.unregisterMessageListener( diml );

        // deinit service
        ils.onDestroy();
    }
}
class DefaultInterlayerMessageListener implements MessageListener {

    public void onMessageReceived( int messageId, ByteBuffer content ){

        BtLog.i( "received message[" + messageId + "], size[" + content.capacity() + "]" );

        if( messageId == PrxMsg.MSG_ID_BT_PRXM_CONNECT_CNF ){

            //bt_prxm_connect_cnf_struct ccs = (bt_prxm_connect_cnf_struct)message;
            //BtLog.i( "bt_prxm_connect_cnf_struct: index[" + ccs.index() + "], rspcode[" + ccs.rspcode() + "]" );
        }
    }
}