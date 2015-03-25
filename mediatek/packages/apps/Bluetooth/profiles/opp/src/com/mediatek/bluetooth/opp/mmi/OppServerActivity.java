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

package com.mediatek.bluetooth.opp.mmi;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.UserHandle;
import android.text.format.Formatter;
import android.widget.Toast;

import com.mediatek.activity.CancelableActivity;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.opp.adp.OppConstants;
import com.mediatek.bluetooth.opp.adp.OppManager;
import com.mediatek.bluetooth.opp.adp.OppService;
import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author mtk01635 1. process Intent with actions: ACTION_PUSH_REQUEST
 */
public class OppServerActivity extends CancelableActivity {

    private static final int DIALOG_PUSH_CONFIRMATION = 0;

    private static final String IS_ACTIVE = "is_active";

    /**
     * is under incoming request confirmation (true: before user confirmation or timeout)
     */
    private boolean mIsActive = false;

    @Override
    protected void onActivityCancel(int id) {

        // null cancel id => when service is stopping
        if (id == CancelableActivity.NULL_CANCEL_ID) {

            // cancel me
            BtLog.d("cancel OppServerActivity by NullCancelId broadcast.");
        } else {
            // check if current cancel request is for me
            // try {
            int taskId = this.getIntent().getIntExtra(OppConstants.OppsAccessRequest.EXTRA_TASK_ID, -1);
            if (taskId != id) {

                // skip any broadcast intent for other CancelableActivity
                return;
            }
        }

        // process cancel operation
        this.mIsActive = false;
        this.finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        OppLog.i("OppServerActivity.onCreate()[+]");

        super.onCreate(savedInstanceState);

        // get intent object and action
        Intent intent = this.getIntent();
        String action = intent.getAction();

        // restore flag
        if (savedInstanceState != null) {

            this.mIsActive = savedInstanceState.getBoolean(IS_ACTIVE, false);
        }

        // check if it's: launched from history && not active => skip it
        if ((intent.getFlags() & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY) != 0 && !this.mIsActive) {

            this.finish();
            return;
        }

        // OPPS Object Push
        if (OppConstants.OppsAccessRequest.ACTION_PUSH_REQUEST.equals(action)) {

            OppLog.d("opps puah request start...");

            this.mIsActive = true;
            /*
            String errMessage = OppManager.getInstance(this).oppsCheckCurrentTask();
            if (errMessage != null) {

                Toast.makeText(this, errMessage, Toast.LENGTH_LONG).show();
                this.finishPushActivity(false);
            }
            */ 

            // wait onResume() to display dialog
        } else if (ACTION_CANCEL_ACTIVITY.equals(action)) {

            // do nothing (needn't cancel anything)
            this.finish();
        } else {
            OppLog.w("unsupported OppsAccessRequest action: " + action);

            // for business card pull / business card exchanges
            this.finish();
        }
    }

    @Override
    protected void onResume() {

        super.onResume();
        this.setVisible(false);
        this.showDialog(DIALOG_PUSH_CONFIRMATION);
    }

    @Override
    protected void onStop() {

        super.onStop();

        // re-send incoming notification when incoming request is active
        // (undetermined)
        if (this.mIsActive) {

            //OppManager.getInstance(this).oppsSendCurrentIncomingNotification();
            Intent intent = new Intent(OppService.OPP_DIALOG_OPPS_RESEND_NOTIFICATION);
            sendBroadcastAsUser(intent, UserHandle.ALL);
        }
    }

    /**
     * save timeout info (for the restore from onCreate() )
     */
    @Override
    protected void onSaveInstanceState(Bundle outState) {

        super.onSaveInstanceState(outState);
        if (this.mIsActive) {

            outState.putBoolean(IS_ACTIVE, this.mIsActive);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {

        switch (id) {
            case DIALOG_PUSH_CONFIRMATION:

                Intent intent = this.getIntent();
                String peerName = intent.getStringExtra(OppConstants.OppsAccessRequest.EXTRA_PEER_NAME);
                String objectName = intent.getStringExtra(OppConstants.OppsAccessRequest.EXTRA_OBJECT_NAME);
                long totalBytes = intent.getLongExtra(OppConstants.OppsAccessRequest.EXTRA_TOTAL_BYTES, 0);

                // popup dialog for user confirmation
                return new AlertDialog.Builder(this).setTitle(R.string.bt_opps_confirm_push_title).setMessage(
                        this.getString(R.string.bt_opps_confirm_push_message, objectName, Formatter.formatFileSize(this,
                                totalBytes), peerName)).setPositiveButton(R.string.bt_opps_confirm_push_yes,
                        new DialogInterface.OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {

                                // accept access request
                                OppServerActivity.this.finishPushActivity(true);
                            }
                        }).setNegativeButton(R.string.bt_opps_confirm_push_no, new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {

                        // reject access request
                        OppServerActivity.this.finishPushActivity(false);
                    }
                }).setOnCancelListener(new OnCancelListener() {

                    public void onCancel(DialogInterface dialog) {

                        OppServerActivity.this.finishPushActivity(false);
                    }
                }).create();
            default:
                break;
        }
        return null;
    }

    /**
     * call will current opps task is confirmed (true/false)
     *
     * @param success
     */
    private synchronized void finishPushActivity(boolean success) {

        OppLog.d("OppServerActivity.finishPushActivity()[+]: " + success);

        this.mIsActive = false;
        int state = (success ? BluetoothShareTask.STATE_PENDING : BluetoothShareTask.STATE_REJECTING);
        Intent intent = new Intent(OppService.OPP_DIALOG_RECEIVER_RETURNS);
        intent.putExtra(OppService.OPP_DIALOG_RETURN_TYPE, OppService.OPP_DIALOG_OPPS_SUBMIT_TASK_ACTION);
        intent.putExtra(OppService.OPPS_DIALOG_RESULT, state);
        sendBroadcastAsUser(intent, UserHandle.ALL);
        this.finish();
    }
}
