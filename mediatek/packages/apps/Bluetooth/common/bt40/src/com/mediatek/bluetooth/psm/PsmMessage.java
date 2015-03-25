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

import com.mediatek.bluetooth.ilm.Message;

/**
 * @author Jerry Hsu
 *
 * add common fields: ref_count / msg_len / index (connId)
 */
public class PsmMessage extends Message {

    private static final int PSM_MESSAGE_B_REF_COUNT = 0;    // common header field - ref_count
    private static final int PSM_MESSAGE_S_MSG_LEN = 1;    // common header field - msg_len
    private static final int PSM_MESSAGE_B_INDEX = 4;    // common header field - index

    /**
     * Constructor - from native to MMI
     *
     * @param id
     * @param content
     */
    public PsmMessage( int id, ByteBuffer content ){

        super( id, content );
    }

    /**
     * Constructor - from MMI to native
     *
     * @param messageDef
     */
    public PsmMessage( int[] messageDef, int connId ){

        super( messageDef );

        this.setRefCount( (byte)1 );
        this.setMsgLen( (short)this.size );
        this.setIndex( (byte)connId );
    }

    public byte getRefCount(){
        return this.getByte( PSM_MESSAGE_B_REF_COUNT );
    }
    public void setRefCount( byte refCount ){
        this.setByte( PSM_MESSAGE_B_REF_COUNT, refCount );
    }
    public short getMsgLen(){
        return this.getShort( PSM_MESSAGE_S_MSG_LEN );
    }
    public void setMsgLen( short msgLen ){
        this.setShort( PSM_MESSAGE_S_MSG_LEN, msgLen );
    }
    public byte getIndex(){
        return this.getByte( PSM_MESSAGE_B_INDEX );
    }
    public void setIndex( byte index ){
        this.setByte( PSM_MESSAGE_B_INDEX, index );
    }
}
