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
import java.nio.ByteOrder;

import com.mediatek.bluetooth.util.ConvertUtils;

/**
 * @author Jerry Hsu
 *
 * - Requirements:
 * 1. create from null (java)
 * 2. create from ByteBuffer (c)
 * 3. getter()
 * 4. setter()
 * 5. size()
 * 6. id()
 * 7. get ByteBuffer (write to socket)
 */
public class Message {

    // definition for ilm_struct
    protected static final int[] ILM = {0, 2076};
    protected static final int ILM_B_USED=0;
    protected static final int ILM_I_MSG_ID=1;
    protected static final int ILM_I_LOCAL_PARA_PTR=2;
    protected static final int ILM_I_SRC_MOD_ID=3;
    protected static final int ILM_I_DEST_MOD_ID=4;
    protected static final int ILM_B_SAP_ID=20;
    protected static final int ILM_I_PEER_BUFF_PTR=6;
    protected static final int ILM_BA_ILM_DATA=28;
    protected static final int ILM_BL_ILM_DATA=2048;

    protected int id;        // message id
    protected int size;        // message size
    private ByteBuffer buffer;    // message content    // TODO [L2][message reuse] cache buffers

    /**
     * create from id + size (MMI)
     *
     * @param messageDef
     * @return
     */
    public Message( int[] messageDef ){

        this.id = messageDef[0];
        this.size = messageDef[1];
        this.buffer = ByteBuffer.allocateDirect( messageDef[1] ).order( ByteOrder.nativeOrder() );
    }

    /**
     * create from ByteBuffer (from native layer)
     *
     * @param messageId
     * @param content
     * @return
     */
    public Message( int id, ByteBuffer content ){

        this.id = id;
        this.size = content.capacity();
        this.buffer = content.order( ByteOrder.nativeOrder() );
    }

    public int getId(){

        return id;
    }
    public int size() {

        return this.size;
    }
    public String toPrintString(){

        return new StringBuilder()
            .append( "name[" ).append( this.id ).append( "], " )
            .append( "size[" ).append( size() ).append( "], " )
            .toString();
    }
    public String toHexString(){

        return ConvertUtils.toHexString( this.buffer );
    }
    protected ByteBuffer getBuffer(){
        return this.buffer;
    }
    protected ByteBuffer getBuffer( int index, int length ){
        this.buffer.position( index );
        this.buffer.limit( index + length );
        ByteBuffer result = this.buffer.slice();
        result.order( ByteOrder.nativeOrder() );
        this.buffer.position(0);
        this.buffer.limit( this.buffer.capacity() );
        return result;
    }

/******************************************************************************************
 * Type Specific Getter & Setter - BEG
 ******************************************************************************************/

    public byte getByte( int field ){
        return this.buffer.get( field );
    }
    public void setByte( int field, byte value ){
        this.buffer.put( field, value );
    }
    public short getShort( int field ){
        return this.buffer.asShortBuffer().get( field );
    }
    public void setShort( int field, short value ){
        this.buffer.asShortBuffer().put( field, value );
    }
    public int getInt( int field ){
        return this.buffer.asIntBuffer().get( field );
    }
    public void setInt( int field, int value ){
        this.buffer.asIntBuffer().put( field, value );
    }
    public long getLong( int field ){
        return this.buffer.asLongBuffer().get( field );
    }
    public void setLong( int field, long value ){
        this.buffer.asLongBuffer().put( field, value );
    }
    public byte[] getByteArray( int field, int length ){

        byte[] result = new byte[length];
        this.buffer.position( field );
        this.buffer.limit( field + length );

        ByteBuffer bb = this.buffer.slice().order( ByteOrder.nativeOrder() );
        bb.get( result );

        // TODO [L3] reset this.buffer position/limit ?
        this.buffer.position(0);
        this.buffer.limit( this.buffer.capacity() );
        return result;
    }
    public void setByteArray( int field, int length, byte[] value ){

        if ( value == null || value.length != length ){

            throw new IllegalArgumentException( "value[" + value + "] is null or length doesn't equal to [" + length + "]" );
        }
        this.buffer.position( field );
        this.buffer.put( value );
        this.buffer.rewind();
//        for( int i=0; i<length; i++ ){
//            this.setByte( index+i, value[i] );
//        }
    }

/******************************************************************************************
 * Type Specific Getter & Setter - END
 ******************************************************************************************/
}
