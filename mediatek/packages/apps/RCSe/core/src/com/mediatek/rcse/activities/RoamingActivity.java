/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences.Editor;
import android.content.Intent;
import android.os.Bundle;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;

/**
 * This class defined as a transparent activity to display the roaming
 * notification message.
 */
public class RoamingActivity extends Activity {

    private static final String TAG = "RoamingActivity";

    public static final String ROAMING_MESSAGE_DIALOG_ACTION =
            "com.mediatek.rcse.activities.ROAMING_STATE";

    public static final String RCSE_SETTING_ACTION = "com.mediatek.rcse.RCSE_SETTINGS";

    public static final String EXTRA_ACTION_TYPE = "action_type";

    public static final int ROAMING_WITH_ENABLE = 0x01;

    public static final int ROAMING_WITH_DISABLE = 0x02;

    public static final int UNROAMING = 0x03;

    public static final String ORANGELABS_RCS_PREFERENCES = "com.orangelabs.rcs_preferences";
    
    public static final String RCS_ROAMING = "rcs_roaming";

    @Override
    protected void onCreate(Bundle arg0) {
        super.onCreate(arg0);
        this.setContentView(R.layout.auto_config_layout);
        Intent intent = this.getIntent();
        if (intent != null) {
            int type = intent.getIntExtra(RoamingActivity.EXTRA_ACTION_TYPE, -1);
            RoamingMessageDialog roamingDialog = new RoamingMessageDialog();
            Bundle arguments = new Bundle();
            arguments.putInt(Utils.TYPE, type);
            roamingDialog.setArguments(arguments);
            roamingDialog.show(this.getFragmentManager(), TAG);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * This class defined to display the roaming message to user
     */
    public class RoamingMessageDialog extends DialogFragment {

        private int mType = -1;

        public RoamingMessageDialog() {

        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            mType = getArguments().getInt(Utils.TYPE);
            Builder dialogBuilder = new AlertDialog.Builder(RoamingActivity.this,
                    AlertDialog.THEME_HOLO_LIGHT);
            dialogBuilder
                    .setTitle(this.getResources().getString(R.string.rcs_message_dialog_title));
            // set display message
            if (mType == ROAMING_WITH_ENABLE) {
                Logger.d(TAG, "Currently is roaming, and the 'Roaming' check box is selected.");
                dialogBuilder.setMessage(this.getResources().getString(
                        R.string.rcs_roaming_message_with_enable_roaming));
            } else if (mType == ROAMING_WITH_DISABLE) {
                Logger.d(TAG, "Currently is roaming, and the 'Roaming' check box is unselected.");
                dialogBuilder.setMessage(this.getResources().getString(
                        R.string.rcs_roaming_message_with_disable_roaming));
            } else if (mType == UNROAMING) {
                Logger.d(TAG, "Currently is not roaming.");
                dialogBuilder.setMessage(this.getResources().getString(
                        R.string.rcs_unroaming_message));
            } else {
                Logger.d(TAG, "Invalid type.");
                return null;
            }

            if (mType == ROAMING_WITH_DISABLE) {
                dialogBuilder.setNegativeButton(this.getResources().getString(
                        R.string.rcs_dialog_negative_button), new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        finish();
                    }
                });
            }

            dialogBuilder.setPositiveButton(this.getResources().getString(
                    R.string.rcs_dialog_positive_button), new OnClickListener() {
                @Override
                public void onClick(DialogInterface arg0, int arg1) {
                    if (mType == ROAMING_WITH_ENABLE) {
                        Logger
                                .d(TAG,
                                        "onClick()-Currently is roaming, and the 'Roaming' check box is selected.");             
                    } else if (mType == ROAMING_WITH_DISABLE) {
                        Logger.d(TAG, "onClick()-Currently is roaming, and the 'Roaming' check box is unselected.");
                        // Start the RCS-e service automatically
                        SharedPreferences preferences =
                            getActivity().getSharedPreferences(ORANGELABS_RCS_PREFERENCES, Activity.MODE_PRIVATE);
                        if (preferences.contains(RCS_ROAMING)) {
                            Editor editor = preferences.edit();
                            editor.putBoolean(RCS_ROAMING, true);
                            editor.commit();
                        } 
                        RcsSettings.getInstance().setRoamingAuthorizationState(true);
                       // LauncherUtils.launchRcsCoreService(RoamingActivity.this.getApplicationContext());
                    } else if (mType == UNROAMING) {
                        Logger.d(TAG, "onClick()-Currently is not roaming");
                        // Start the RCS-e service automatically
                        LauncherUtils.forceLaunchRcsCoreService(RoamingActivity.this
                                .getApplicationContext());
                    } else {
                        Logger.d(TAG, "onClick()-Invalid type.");
                    }
                    finish();
                }
            });
            AlertDialog dialog = dialogBuilder.create();
            dialog.setCancelable(false);
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            this.dismissAllowingStateLoss();
            finish();
        }
    }
}
