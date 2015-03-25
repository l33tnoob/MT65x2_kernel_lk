/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.security.ui;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import com.mediatek.security.R;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.xlog.Xlog;


/**
 * RequestPermissionHelperActivity asks the user whether to enable discovery.
 * This is usually started by RequestPermissionActivity.
 */
public class SwitchOffAlertActivity extends AlertActivity implements DialogInterface.OnClickListener {
    private static final String TAG = "SwitchOffAlertActivity";
    private CheckBox mCheckBox;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createDialog();
    }

    void createDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mIconId = android.R.drawable.ic_dialog_alert;
        p.mTitle = getString(R.string.alert_dlg_title);
        p.mPositiveButtonText = getString(com.android.internal.R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(com.android.internal.R.string.no);
        p.mNegativeButtonListener = this;

        LayoutInflater inflater = LayoutInflater.from(this);
        View view = inflater.inflate(R.layout.notify_dialog_customview, null);

        mCheckBox = (CheckBox)view.findViewById(R.id.checkbox);

        TextView msgTxt = (TextView)view.findViewById(R.id.message);
        msgTxt.setText(R.string.alert_dlg_mgs);

        // over use the layout , so hide the timer
        TextView timer = (TextView)view.findViewById(R.id.count_timer);
        timer.setVisibility(View.GONE);

        p.mView = view;
                
        setupAlert();
    }

    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            int state = mCheckBox.isChecked() ? 1 : 0;
            Xlog.d(TAG, "OK button , turn off the switch , checkbox state = "
                    + state);
            PermControlUtils.enablePermissionControl(false, this);
            Settings.System.putInt(getContentResolver(),
                    PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, state);

        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
            Xlog.d(TAG, "Cancle button , nothing to do");
        }
    }
    
}
