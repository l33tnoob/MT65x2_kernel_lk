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

package com.mediatek.activity;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnCancelListener;

import com.mediatek.bluetooth.util.BtLog;

/**
 * @author Jerry Hsu
 *
 * used to display a message to user => only single "confirm" button
 *
 * three parameters can be passed via Intent Extra:
 *
 *     1. MessageActivity.TITLE
 *     2. MessageActivity.MESSAGE
 *     3. MessageActivity.NEGATIVE_BUTTON
 *     4. MessageActivity.POSITIVE_BUTTON
 *     5. MessageActivity.POSITIVE_INTENT
 */
public class MessageActivity extends Activity {

    public static final int RESULT_ACTION_CLEAR = 0;
    public static final int RESULT_ACTION_RETRY = 1;
    public static final int RESULT_ACTION_CANCEL = 2;
    
    private static final String TITLE = "title";
    private static final String MESSAGE = "message";
    private static final String POSITIVE_BUTTON = "positiveButton";
    private static final String POSITIVE_INTENT = "positiveIntent";
    private static final String NEGATIVE_BUTTON = "negativeButton";

    @Override
    protected void onResume() {

        super.onResume();
        this.setVisible( false );
        this.showDialog(0);
    }

    @Override
    protected Dialog onCreateDialog( int id ){

        BtLog.d( "MessageActivity.onCreateDialog()[+]: task" + this.getTaskId() );

        Intent intent = getIntent();
        String title = intent.getStringExtra( TITLE );
        String message = intent.getStringExtra( MESSAGE );
        String positiveButton = intent.getStringExtra( POSITIVE_BUTTON );
        String negativeButton = intent.getStringExtra( NEGATIVE_BUTTON );

        // TODO check parameter

        // popup dialog to show message
        AlertDialog.Builder builder = new AlertDialog.Builder(this)
            .setTitle( title )
            .setMessage( message )
            .setNegativeButton( negativeButton, new DialogInterface.OnClickListener(){

                public void onClick( DialogInterface dialog, int which ){

                    setResult(RESULT_ACTION_CLEAR);
                    MessageActivity.this.finish();
                }
            })
            .setOnCancelListener( new OnCancelListener() {

                public void onCancel( DialogInterface dialog ){
                    
                    setResult(RESULT_ACTION_CANCEL);
                    MessageActivity.this.finish();
                }
            });

        // add positive button and intent if any
        if( positiveButton != null ){

            final Intent positiveIntent = intent.getParcelableExtra(POSITIVE_INTENT);
            builder.setPositiveButton( positiveButton, new DialogInterface.OnClickListener(){

                public void onClick( DialogInterface dialog, int which ){

                    setResult(RESULT_ACTION_RETRY);
                    MessageActivity.this.startActivity(positiveIntent);
                    MessageActivity.this.finish();
                }
            });
        }

        // return the AlertDialog
        return builder.create();
    }

    public static Intent createIntent( Context context, String title, String message, String button ){

        return createIntent( context, title, message, null, null, button );
    }

    public static Intent createIntent( Context context, String title, String message, String positiveButton, Intent positiveIntent, String negativeButton ){

        // don't config intent flags here because it's should be configed by case.
        Intent intent = new Intent( context, MessageActivity.class );
        intent.putExtra( MessageActivity.TITLE, title );
        intent.putExtra( MessageActivity.MESSAGE, message );
        intent.putExtra( MessageActivity.POSITIVE_BUTTON, positiveButton );
        intent.putExtra( MessageActivity.POSITIVE_INTENT, positiveIntent );
        intent.putExtra( MessageActivity.NEGATIVE_BUTTON, negativeButton );
        return intent;
    }
}
