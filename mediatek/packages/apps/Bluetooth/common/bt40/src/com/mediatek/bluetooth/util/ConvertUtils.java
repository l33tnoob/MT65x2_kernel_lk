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

package com.mediatek.bluetooth.util;

import java.nio.ByteBuffer;

public class ConvertUtils {

    /**
     * convert bdaddr from string format (e.g. "7E:3F:4E:66:11:72") to byte[6]
     * accept upper case string only
     *
     * @param bdaddr
     * @return
     */
    public static byte[] convertBdAddr( String bdaddr ){

        // check parameter
        if( bdaddr == null || bdaddr.length() != 17 ){

            throw new IllegalArgumentException( "convertBdAddr() error: invalid bdaddr[" + bdaddr + "]" );
        }

        // convert to upper case
        //bdaddr = bdaddr.toUpperCase();

        // convert address
        short digital = 0;
        byte[] result = new byte[6];
        for( int i=0; i<17; i+=3 ){

            // character 1: e.g. '7'
            char c = bdaddr.charAt(i);
            if( '0' <= c && c <= '9' )    digital = (short)(c-'0');
            else if( 'A' <= c && c <= 'F' )    digital = (short)(c-'A'+10);
            else {
                throw new IllegalArgumentException( "convertBdAddr() error: invalid char[" + c + "] in [" + bdaddr + "]" );
            }

            // shit 4 bits
            digital <<= 4;

            // character 2: e.g. 'E'
            c = bdaddr.charAt(i+1);
            if( '0' <= c && c <= '9' )    digital += (short)(c-'0');
            else if( 'A' <= c && c <= 'F' )    digital += (short)(c-'A'+10);
            else {
                throw new IllegalArgumentException( "convertBdAddr() error: invalid char[" + c + "] in [" + bdaddr + "]" );
            }

            // keep result (from 5 ~ 0)
            result[5-i/3] = (byte)digital;
        }

        // return
        return result;
    }

    /**
     * convert bdaddr from byte[6] to string format (e.g. "7E:3F:4E:66:11:72")
     * @param bdaddr
     * @return
     */
    public static String convertBdAddr( byte[] bdaddr ){

        // check parameter
        if( bdaddr == null || bdaddr.length != 6 ){

            throw new IllegalArgumentException( "convertBdAddr() error: invalid bdaddr[" + bdaddr + "]" );
        }

        char[] result = new char[17];
        int v, t, j=0;
        for( int i=5; i>=0; i-- ){

            // process byte by byte
            v = bdaddr[i];

            // left 4 digital
            t = ( v >>> 4 ) & 15;
            if( t > 9 )    t = t - 10 + 'A';
            else        t += '0';
            result[j++] = (char)t;

            // right 4 digital
            t = v & 15;
            if( t > 9 )    t = t - 10 + 'A';
            else        t += '0';
            result[j++] = (char)t;

            // append ':'
            if( i>0 )    result[j++] = ':';
        }

        // return
        return new String(result);
    }

    private static final String HEX = "0123456789ABCDEF";
    public static String toHexString( ByteBuffer raw ){

        if( raw == null )    return "";

        byte b;
        int size = raw.capacity();
        StringBuilder res = new StringBuilder(size*2);
        for( int i=0; i<size; i++ ){

            b = raw.get(i);
            res.append( HEX.charAt( (b&0xF0)>>4 ) ).append( HEX.charAt( (b&0x0F) ) );
        }
        return res.toString();
    }
}
