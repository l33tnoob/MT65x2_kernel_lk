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

package com.mediatek.bluetooth;

/**
 * @author Jerry Hsu
 *
 * must be sync with: btcore/btstack/inc/bttypes.h
 */
public class ResponseCode {

    public static final byte STACK_END = 50;
    public static final byte UNKNOWN = 127;

    public static final byte INVALID_STATE = STACK_END + 1;
    public static final byte UNDEFINED_STATE = STACK_END + 2;


    public static final byte SUCCESS           = 0;     /* Successful and complete */
    public static final byte FAILED            = 1;     /* Operation failed */
    public static final byte PENDING           = 2;     /* Successfully started but pending */
    public static final byte DISCONNECT        = 3;     /* Link disconnected */
    public static final byte NO_LINK           = 4;     /* No Link layer Connection exists */
    public static final byte IN_USE            = 5;     /* Operation failed - already in use. */
    /* IrDA specific return codes */
    public static final byte MEDIA_BUSY        = 6;     /* IRDA: Media is busy */
    public static final byte MEDIA_NOT_BUSY    = 7;     /* IRDA: Media is not busy */
    public static final byte NO_PROGRESS       = 8;     /* IRDA: IrLAP not making progress */
    public static final byte LINK_OK           = 9;     /* IRDA: No progress condition cleared */
    public static final byte SDU_OVERRUN       = 10;    /* IRDA: Sent more data than current SDU size */
    /* Bluetooth specific return codes */
    public static final byte BUSY              = 11;
    public static final byte NO_RESOURCES      = 12;
    public static final byte NOT_FOUND         = 13;
    public static final byte DEVICE_NOT_FOUND  = 14;
    public static final byte CONNECTION_FAILED = 15;
    public static final byte TIMEOUT           = 16;
    public static final byte NO_CONNECTION     = 17;
    public static final byte INVALID_PARM      = 18;
    public static final byte IN_PROGRESS       = 19;
    public static final byte RESTRICTED        = 20;
    public static final byte INVALID_TYPE      = 21;
    public static final byte HCI_INIT_ERR      = 22;
    public static final byte NOT_SUPPORTED     = 23;
    public static final byte CONTINUE          = 24;
    public static final byte CANCELLED         = 25;
    public static final byte NOSERVICES        = 26;
    public static final byte SCO_REJECT        = 27;
    public static final byte CHIP_REASON       = 28;
    public static final byte BLOCK_LIST        = 29;
    public static final byte SCATTERNET_REJECT = 30;
    public static final byte REMOTE_REJECT     = 31;
    public static final byte KEY_ERR           = 32;
    public static final byte CONNECTION_EXIST  = 33;

    /*
    public static String getString( Context context, byte responseCode ){

        switch( responseCode ){
            case UNKNOWN:            return context.getString( R.string.rsp_unknown );
            case INVALID_STATE1:        return context.getString( R.string.rsp_invalid_state );
            case UNDEFINED_STATE:        return context.getString( R.string.rsp_undefined_state );
            case SUCCESS:            return context.getString( R.string.rsp_success );
            case FAILED:            return context.getString( R.string.rsp_failed );
            case PENDING:            return context.getString( R.string.rsp_pending );
            case DISCONNECT:        return context.getString( R.string.rsp_disconnect );
            case NO_LINK:            return context.getString( R.string.rsp_no_link );
            case IN_USE:            return context.getString( R.string.rsp_in_use );
            case NOT_FOUND:            return context.getString( R.string.rsp_not_found );
            case NO_CONNECTION:        return context.getString( R.string.rsp_no_connection );
            default:            return context.getString( R.string.rsp_undefined_code, Byte.toString( responseCode ) );
        }
    }
    */
}
