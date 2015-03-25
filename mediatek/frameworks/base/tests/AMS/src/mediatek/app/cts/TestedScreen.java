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

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package mediatek.app.cts;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue;
import android.os.SystemClock;

import android.util.Log;


public class TestedScreen extends Activity {
    public static final String WAIT_BEFORE_FINISH = "TestedScreen.WAIT_BEFORE_FINISH";
    public static final String DELIVER_RESULT = "TestedScreen.DELIVER_RESULT";
    public static final String CLEAR_TASK = "TestedScreen.CLEAR_TASK";

    public TestedScreen() {
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (LaunchpadActivity.FORWARD_RESULT.equals(getIntent().getAction())) {
            final Intent intent = new Intent(getIntent());
            intent.setAction(DELIVER_RESULT);
            intent.setFlags(Intent.FLAG_ACTIVITY_FORWARD_RESULT);
            startActivity(intent);
            finish();
        } else if (DELIVER_RESULT.equals(getIntent().getAction())) {
            setResult(RESULT_OK, new Intent().setAction(LaunchpadActivity.RETURNED_RESULT));
            finish();
        } else if (CLEAR_TASK.equals(getIntent().getAction())) {
            if (!getIntent().getBooleanExtra(ClearTop.WAIT_CLEAR_TASK, false)) {
                launchClearTask();
            }
        }
    }

    @Override
    protected void onRestoreInstanceState(Bundle state) {
        super.onRestoreInstanceState(state);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (CLEAR_TASK.equals(getIntent().getAction())) {
            if (getIntent().getBooleanExtra(ClearTop.WAIT_CLEAR_TASK, false)) {
                Looper.myLooper();
                Looper.myQueue().addIdleHandler(new Idler());
            }
        } else {
            Looper.myLooper();
            Looper.myQueue().addIdleHandler(new Idler());
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void launchClearTask() {
        final Intent intent = new Intent(getIntent()).addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                .setClass(this, ClearTop.class);
        startActivity(intent);
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (CLEAR_TASK.equals(getIntent().getAction())) {
                launchClearTask();
            } else {
                Log.v("hihi", "RESULT_OK");
                setResult(RESULT_OK);
                finish();
            }
        }
    };

    private class Idler implements MessageQueue.IdleHandler {
        public final boolean queueIdle() {
            if (WAIT_BEFORE_FINISH.equals(getIntent().getAction())) {
                final Message m = Message.obtain();
                Log.v("hihi", "sendMessageAtTime");
                mHandler.sendMessageAtTime(m, SystemClock.uptimeMillis() + 1000);
            } else if (CLEAR_TASK.equals(getIntent().getAction())) {
                final Message m = Message.obtain();
                mHandler.sendMessageAtTime(m, SystemClock.uptimeMillis() + 1000);
            } else {
                setResult(RESULT_OK);
                finish();
            }
            return false;
        }
    }
}
