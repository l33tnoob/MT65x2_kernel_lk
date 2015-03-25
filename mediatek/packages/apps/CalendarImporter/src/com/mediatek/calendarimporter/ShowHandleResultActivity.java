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
 */
package com.mediatek.calendarimporter;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Bundle;
import android.provider.CalendarContract;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.calendarimporter.utils.LogUtils;
import com.mediatek.calendarimporter.utils.Utils;

public class ShowHandleResultActivity extends Activity {
    private static final String TAG = "ShowHandleResultActivity";
    private long mEventDtStart;

    private static final String KEY_VIEW_TYPE = "VIEW";
    private static final String MONTH_VIEW = "MONTH";
    private static final String EXTRA_BEGIN_TIME = "beginTime";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        LogUtils.d(TAG, "onCreate.");
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.import_dialog);

        int color = Utils.getThemeMainColor(this, Utils.DEFAULT_COLOR);
        if (color != Utils.DEFAULT_COLOR) {
            TextView view = (TextView) findViewById(R.id.result_title);
            view.setTextColor(color);
            findViewById(R.id.result_divide_line).setBackgroundColor(color);
        }

        ProgressBar progressBar = ((ProgressBar)findViewById(R.id.import_progress));
        progressBar.setProgress(100);

        TextView mtTextView = (TextView) findViewById(R.id.import_tip);
        Intent intent = getIntent();
        mEventDtStart = intent.getLongExtra("eventStartTime", System.currentTimeMillis());
        int success = intent.getIntExtra("SucceedCnt", 0);
        int total = intent.getIntExtra("totalCnt", 0);
        Button openBtn = (Button) findViewById(R.id.button_open);
        if (success <= 0 || success < total) {
            mtTextView.setText(R.string.import_vcs_failed);
            openBtn.setEnabled(false);
        } else {
            mtTextView.setText(R.string.import_complete);
            openBtn.setEnabled(true);
        }

        openBtn.setVisibility(View.VISIBLE);
        findViewById(R.id.button_done).setVisibility(View.VISIBLE);
        findViewById(R.id.button_open).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(CalendarContract.CONTENT_URI, "time/epoch");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                Bundle extras = new Bundle();
                extras.putString(KEY_VIEW_TYPE, MONTH_VIEW);
                intent.putExtra(EXTRA_BEGIN_TIME, mEventDtStart);
                intent.putExtras(extras);
                try {
                    startActivity(intent);
                } catch (ActivityNotFoundException e) {
                    Toast.makeText(ShowHandleResultActivity.this, R.string.open_calendar_failed, Toast.LENGTH_LONG)
                            .show();
                    LogUtils.e(TAG, "Start Activity failed! Maybe the Calendar App is closed.Exception:" + e);
                }
                finish();
            }
        });

        findViewById(R.id.button_done).setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                finish();
            }
        });
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        mEventDtStart = intent.getLongExtra("eventStartTime", System.currentTimeMillis());
    }
}
