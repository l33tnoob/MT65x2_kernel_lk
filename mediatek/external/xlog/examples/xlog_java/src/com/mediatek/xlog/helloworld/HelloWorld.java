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

package com.mediatek.xlog.helloworld;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import android.util.Log;
import android.util.Slog;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class HelloWorld extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        // Inflate our UI from its XML layout description.
        setContentView(R.layout.activity);

        // Hook up button presses to the appropriate event handler.
        ((Button) findViewById(R.id.testlog)).setOnClickListener(mTestLogListener);
        ((Button) findViewById(R.id.testxlog)).setOnClickListener(mTestXLogListener);
        ((Button) findViewById(R.id.testperf)).setOnClickListener(mTestPerfListener);
    }

    public void xlogPerf()
    {
	long start = System.nanoTime();
	Xlog.v("xlog/perf", "Performance test init");
	long diff = System.nanoTime() - start;
	Xlog.v("xlog/perf-report", "Fist time " + diff);

	start = System.nanoTime();
	for (int i = 0; i < 1000; i++) {
	    Xlog.v("xlog/perf", "Performance test " + i);
	}
	diff = System.nanoTime() - start;
	Xlog.v("xlog/perf-report", "loop 1000 " + diff + " avg " + ((double) diff) / 1000);
    }


    /**
     * A call-back for when the user presses the back button.
     */
    OnClickListener mTestLogListener = new OnClickListener() {
        public void onClick(View v) {
	    Log.v("xlogtest/java", "Log Verbose");
	    Log.d("xlogtest/java", "Log Debug");
	    Log.i("xlogtest/java", "Log Info");
	    Log.w("xlogtest/java", "Log Warn");
	    Log.e("xlogtest/java", "Log Error");

	    Slog.v("xlogtest/java", "SLog Verbose");
	    Slog.d("xlogtest/java", "SLog Debug");
	    Slog.i("xlogtest/java", "SLog Info");
	    Slog.w("xlogtest/java", "SLog Warn");
	    Slog.e("xlogtest/java", "SLog Error");
        }
    };

    OnClickListener mTestXLogListener = new OnClickListener() {
        public void onClick(View v) {
	    Xlog.v("xlogtest/java", "XLog Verbose");
	    Xlog.d("xlogtest/java", "XLog Debug");
	    Xlog.i("xlogtest/java", "XLog Info");
	    Xlog.w("xlogtest/java", "XLog Warn");
	    Xlog.e("xlogtest/java", "XLog Error");

	    SXlog.v("xlogtest/java", "SXLog Verbose");
	    SXlog.d("xlogtest/java", "SXLog Debug");
	    SXlog.i("xlogtest/java", "SXLog Info");
	    SXlog.w("xlogtest/java", "SXLog Warn");
	    SXlog.e("xlogtest/java", "SXLog Error");
        }
    };

    OnClickListener mTestPerfListener = new OnClickListener() {
        public void onClick(View v) {
        }
    };

}
