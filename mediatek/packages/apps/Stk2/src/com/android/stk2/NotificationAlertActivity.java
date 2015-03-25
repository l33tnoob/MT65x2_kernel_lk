/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.android.stk2;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.cat.CatLog;

public class NotificationAlertActivity extends Activity {

    private String mNotificationMessage = "";
    private String mTitle = "";
    private static final String LOGTAG = "Stk-NA ";

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        requestWindowFeature(Window.FEATURE_LEFT_ICON);
        Window window = getWindow();
        setContentView(R.layout.stk_msg_dialog);

        TextView mMessageView = (TextView) findViewById(R.id.dialog_message);
        Button okButton = (Button) findViewById(R.id.button_ok);
        Button cancelButton = (Button) findViewById(R.id.button_cancel);

        // Bundle extras = getIntent().getExtras();
        // if (extras != null) {
        // mNotificationMessage =
        // extras.getString(StkAppService.NOTIFICATION_KEY);
        // mTitle = extras.getString(StkAppService.NOTIFICATION_TITLE);
        // }

        if ((null == mMessageView) || (null == okButton) || (null == cancelButton)) {
            CatLog.d(LOGTAG, "Error: null Point: mMessageView[" + mMessageView
                    + "] okButton[" + okButton + "] cancelButton[" + cancelButton + "]");
            finish();
        }

        mNotificationMessage = StkApp.mIdleMessage;
        mTitle = StkApp.mPLMN;

        window.setFeatureDrawableResource(Window.FEATURE_LEFT_ICON,
                com.android.internal.R.drawable.stat_notify_sim_toolkit);
        setTitle(mTitle);

        okButton.setOnClickListener(mButtonClicked);
        cancelButton.setOnClickListener(mButtonClicked);

        CatLog.d(LOGTAG, "Idle Text Title[" + mTitle + "]");
        CatLog.d(LOGTAG, "Idle Text[" + mNotificationMessage + "]");

        mMessageView.setText(mNotificationMessage);
    }

    private View.OnClickListener mButtonClicked = new View.OnClickListener() {
        public void onClick(View v) {
            CatLog.d(LOGTAG, "finished!");
            finish();
        }
    };
}
