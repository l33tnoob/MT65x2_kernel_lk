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
import android.net.Uri;
import android.os.UserHandle;

import com.mediatek.activity.CancelableActivity;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.opp.adp.OppManager;
import com.mediatek.bluetooth.opp.adp.OppService;
import com.mediatek.bluetooth.util.BtLog;

/**
 * @author jerry.hsu popup 'Cancel' dialog and call OppManager to cancel task
 */
public class OppCancelActivity extends CancelableActivity {

    @Override
    protected void onResume() {

        super.onResume();
        this.setVisible(false);
        this.showDialog(0);
    }

    @Override
    protected Dialog onCreateDialog(int id) {

        return new AlertDialog.Builder(this).setTitle(R.string.bt_opp_cancel_task_title)
                .setMessage(R.string.bt_opp_cancel_task_message).setPositiveButton(
                        R.string.bt_opp_cancel_task_yes, new DialogInterface.OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {

                                OppCancelActivity.this.onTaskCancel();
                                OppCancelActivity.this.finish();
                            }
                        }).setNegativeButton(R.string.bt_opp_cancel_task_no,
                        new DialogInterface.OnClickListener() {

                            public void onClick(DialogInterface dialog, int which) {

                                OppCancelActivity.this.finish();
                            }
                        }).setOnCancelListener(new OnCancelListener() {

                    public void onCancel(DialogInterface dialog) {

                        OppCancelActivity.this.finish();
                    }
                }).create();
    }

    private void onTaskCancel() {

        Uri task = this.getIntent().getData();

        OppLog.d("cancel task - uri[" + task + "]");

        // send abort request to OppManager
        //OppManager.getInstance(this).oppAbortTask(task);
        Intent intent = new Intent(OppService.OPP_DIALOG_RECEIVER_RETURNS);
        intent.putExtra(OppService.OPP_DIALOG_RETURN_TYPE, OppService.OPP_DIALOG_OPPS_CANCEL_TASK);
        intent.putExtra(OppService.OPPS_DIALOG_RESULT, task.toString());
        sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    @Override
    protected void onActivityCancel(int id) {

        // null cancel id => when service is stopping
        if (id == CancelableActivity.NULL_CANCEL_ID) {

            // cancel me
            BtLog.d("cancel OppCancelActivity by NullCancelId broadcast.");
        } else {
            // check if current cancel request is for me
            try {
                int taskId = Integer.parseInt(this.getIntent().getData().getLastPathSegment());
                if (taskId != id) {

                    // broadcast intent is not for this Activity
                    return;
                }
            } catch (NumberFormatException ex) {

                // will be canceled
                OppLog.e("OppCancelActivity.onActivityCancel() error: intent[" + this.getIntent()
                        + "], exception[" + ex.getMessage() + "]");
            }
        }

        // only finish this Activity when id equals
        this.finish();
    }
}
