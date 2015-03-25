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

package com.mediatek.storage;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.os.storage.StorageManager;
import android.content.BroadcastReceiver;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
/*
this class for user to set default storage path when the SD plug in.
*/
public class StorageDefaultPathDialog extends AlertActivity implements DialogInterface.OnClickListener {
    
	private static final String TAG = "StorageDefaultPathDialog";
    private static final String SD_ACTION = "android.intent.action.MEDIA_BAD_REMOVAL";
    private static final String INSERT_OTG = "insert_otg";
    String path = null;  
    private IntentFilter mSDCardStateFilter;
    private BroadcastReceiver mReceiver;
    private Boolean mInsertOtg = false;
    
    private final BroadcastReceiver mSDStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(SD_ACTION)) {
            	finish();
            }
        }
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG,"StorageDefaultPathDialog onCreate()");
        mSDCardStateFilter = new IntentFilter(SD_ACTION);
        mSDCardStateFilter.addDataScheme("file");
        mReceiver = mSDStateReceiver;
        mInsertOtg = getIntent().getBooleanExtra(INSERT_OTG, false);
        createDialog();
    }

    private void createDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = mInsertOtg ? getString(com.mediatek.internal.R.string.usb_storage_ready_title) : 
                getString(com.mediatek.internal.R.string.sdcard_ready_title);
        p.mView = createView();
        p.mViewSpacingSpecified=true;
        p.mViewSpacingLeft=15;
        p.mViewSpacingRight=15;
        p.mViewSpacingTop=5;
        p.mViewSpacingBottom=5;
        p.mPositiveButtonText = getString(android.R.string.yes);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(android.R.string.no);
        p.mNegativeButtonListener = this;
        setupAlert();
    }

    private View createView() {
        TextView messageView = new TextView(this);
        messageView.setTextAppearance(messageView.getContext(), android.R.style.TextAppearance_Medium);
        messageView.setText(com.mediatek.internal.R.string.sdcard_select_default_path);
        return messageView;
    }
    
    @Override
    protected void onResume() {
        super.onResume();

        registerReceiver(mReceiver, mSDCardStateFilter);
    }
    @Override
    protected void onDestroy() {
        Log.d(TAG,"onDestroy()");
        super.onDestroy();
    }

    @Override
    protected void onPause(){
        super.onPause();
        
        Log.e(TAG, "onPause entry");
        unregisterReceiver(mReceiver);
                   
    }
    
    private void onOK() {
        Intent intent = new Intent();
        intent.setAction("android.settings.INTERNAL_STORAGE_SETTINGS");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        Log.d(TAG,"onOK() start activity");
        startActivity(intent);
        finish();
        
    }

    private void onCancel() {
		/*when the sd card plug out, framework will modify the default storage path.
		so, we need to do nothing*/
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                onOK();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                onCancel();
                break;
        }
    }
}

