/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.util;

import android.content.Context;
import android.content.Intent;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.UserHandle;
import android.util.Log;


public class MediaScanner implements MediaScannerConnectionClient {

    private static final String TAG = "MediaScanner";

    public static final int MEDIA_SCANNED = 0;
    public static final int MEDIA_SCAN_FAILED = 1;

    private Context context;
    private String path;
    private String mimeType;
    private Handler callback;
    private int callbackId;
    private static final String CURRENT_USER_ID = "currentUserId";

    private MediaScannerConnection msc;

    public MediaScanner( Context context, String path, String mimeType, Handler callback, int callbackId ) {

        this.context = context;
        this.path = path;
        this.mimeType = mimeType;
        this.callback = callback;
        this.callbackId = callbackId;
        this.msc = new MediaScannerConnection( this.context, this );

        Log.d( TAG, "[BT][MMI][MediaScanner]: connecting to Media Scanner Service" );
        this.msc.connect();
    }

    public void onMediaScannerConnected(){

        try {
            Log.d( TAG, "[BT][MMI][onMediaScannerConnected]: MediaScanner connected." );
            this.msc.scanFile( this.path, this.mimeType );
        }
        catch( Exception ex ){

            Log.i( TAG, "[BT][MMI][onMediaScannerConnected]: MediaScanner exception: " + ex );
        }
    }

    public void onScanCompleted( String path, Uri uri ){

        Log.d( TAG, "[BT][MMI][onScanCompleted]: path=" + path + ", uri=" + uri );

        try {
            if( this.callback != null ){

                Message msg = Message.obtain( this.callback );
                msg.arg1 = this.callbackId;
                msg.obj = uri;
                msg.what = ( uri != null ) ? MEDIA_SCANNED : MEDIA_SCAN_FAILED;
                msg.sendToTarget();
            }
        }
        catch( Exception ex ){

            Log.i( TAG, "[BT][MMI][onScanCompleted]: MediaScanner exception: " + ex );
        }
        finally {

            this.msc.disconnect();
        }
    }
}
