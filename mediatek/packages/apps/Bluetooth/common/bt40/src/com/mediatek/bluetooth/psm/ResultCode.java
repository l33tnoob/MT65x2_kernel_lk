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

/**
 * @author Jerry Hsu
 *
 *     ResultCode = StatusCode + ResponseCode
 *
 *     StatusCode: use to judge the result of PSM handleMessage.
 *     ResponseCode: response from stack (or fail reason from external layer)
 *
 */
public class ResultCode {

    /**********************************************************
     * Status Code: 0 ~ 32767
     **********************************************************/

    //public static final int STATUS_UNKNOWN1 = 0;
    public static final int STATUS_SUCCESS = 1;
    public static final int STATUS_PENDING = 2;
    public static final int STATUS_FAILED = 3;

    /**********************************************************
     * Reason Code: 0 ~ 32767 = statck response code
     **********************************************************/

    /**
     * get resultCode from statusCode
     *
     * @param statusCode
     * @param reasonCode
     * @return
     */
    public static int create( int statusCode ){

        return ( statusCode << 16 );
    }

    /**
     * get resultCode from statusCode + reasonCode
     *
     * @param statusCode
     * @param responseCode
     * @return
     */
    public static int create( int statusCode, int responseCode ){

        return ( statusCode << 16 ) | responseCode;
    }

    /**
     * get statusCode from resultCode
     *
     * @param resultCode
     * @return
     */
    public static int status( int resultCode ){

        return ( resultCode >> 16 );
    }

    /**
     * get reasonCode from resultCode
     *
     * @param resultCode
     * @return
     */
    public static int rspcode( int resultCode ){

        return ( resultCode & 0xFFFF );
    }
}

//case 0x10:    return "Continue";
//case 0x20:    return "OK, Success";
//case 0x21:    return "Created";
//case 0x22:    return "Accepted";
//case 0x23:    return "Non-Authoritative Information";
//case 0x24:    return "No Content";
//case 0x25:    return "Reset Content";
//case 0x26:    return "Partial Content";
//case 0x30:    return "Multiple Choices";
//case 0x31:    return "Moved Permanently";
//case 0x32:    return "Moved temporarily";
//case 0x33:    return "See Other";
//case 0x34:    return "Not modified";
//case 0x35:    return "Use Proxy";
//case 0x40:    return "Bad Request - server couldn't understand request";
//case 0x41:    return "Unauthorized";
//case 0x42:    return "Payment required";
//case 0x43:    return "Forbidden - operation is understood but refused";
//case 0x44:    return "Not Found";
//case 0x45:    return "Method not allowed";
//case 0x46:    return "Not Acceptable";
//case 0x47:    return "Proxy Authentication required";
//case 0x48:    return "Request Time Out";
//case 0x49:    return "Conflict";
//case 0x4A:    return "Gone";
//case 0x4B:    return "Length Required";
//case 0x4C:    return "Precondition failed";
//case 0x4D:    return "Requested entity too large";
//case 0x4E:    return "Request URL too large";
//case 0x4F:    return "Unsupported media type";
//case 0x50:    return "Internal Server Error";
//case 0x51:    return "Not Implemented";
//case 0x52:    return "Bad Gateway";
//case 0x53:    return "Service Unavailable";
//case 0x54:    return "Gateway Timeout";
//case 0x55:    return "HTTP version not supported";
//case 0x60:    return "Database Full";
//case 0x61:    return "Database Locked";