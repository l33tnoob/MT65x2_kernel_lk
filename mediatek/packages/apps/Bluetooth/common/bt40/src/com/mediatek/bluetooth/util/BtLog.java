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

import com.mediatek.bluetooth.Options;

import android.util.Log;

/**
 * @author Jerry Hsu
 *
 * TODO [L4] integrate with slf4j/log4j and utility log viewer: http://logging.apache.org/chainsaw/index.html
 */
public class BtLog {

    private static final boolean DEVELOPMENT = Options.LM_DEVELOPMENT;
    private static final String TAG = "BT";
    private static final String LAYER = "[MMI]";

    public static void doLog( int priority, StackTraceElement stack, String tag, String message ){

        // get classname (for package and class)
        String classname = stack.getClassName();
        int idx = classname.lastIndexOf('.');
        classname = classname.substring(++idx);

        // update message
        message = new StringBuilder()
             .append( LAYER )
            .append( "[" )
            .append( classname )
            .append( "." )
            .append( stack.getMethodName() )
            .append( "][" )
            .append( message )
            .append( "]").toString();

        // use caller's class name as tag when it's under development mode
        tag = classname;

        // do log
        android.util.Log.println( priority, tag, message );
    }

    public static void printStackTrace(){

        StackTraceElement[] ste = Thread.currentThread().getStackTrace();
        w( "=================== Stack Trace =====================" );
        for( StackTraceElement e : ste ){

            StringBuilder msg  = new StringBuilder();
            msg.append( e.getClassName() )
                .append(".").append( e.getMethodName() )
                .append("(").append( e.getLineNumber() ).append(")");
            w( msg.toString() );
        }
    }

    public static void v( String message ){

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.VERBOSE, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.VERBOSE, TAG, message );
        }
    }

    public static void d( String message ){

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.DEBUG, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.DEBUG, TAG, message );
        }
    }

    public static void i( String message ){

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.INFO, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.INFO, TAG, message );
        }
    }

    public static void w( String message ){

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.WARN, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.WARN, TAG, message );
        }
    }

    public static void e( String message ){

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.ERROR, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.ERROR, TAG, message );
        }
    }

    public static void e( String message, Throwable throwable ){

        message += Log.getStackTraceString(throwable);

        if( DEVELOPMENT ){

            BtLog.doLog( android.util.Log.ERROR, Thread.currentThread().getStackTrace()[3], TAG, message );
        }
        else {
            android.util.Log.println( android.util.Log.ERROR, TAG, message );
        }
    }
}
