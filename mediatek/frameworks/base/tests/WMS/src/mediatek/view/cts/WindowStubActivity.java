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

package com.mediatek.cts.window;

import com.mediatek.cts.window.stub.R;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.Window;
import android.view.WindowManager;
import android.util.Log;

public class WindowStubActivity extends Activity {

    private static boolean mIsOnCreateOptionsMenuCalled;
    private static boolean mIsOnOptionsMenuClosedCalled;
    private static boolean mIsOnKeyDownCalled;
    private static boolean mIsOnHomeKeyDownCalled;
    private static boolean mIsOnHomeKeyUpCalled;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().requestFeature(Window.FEATURE_LEFT_ICON);
        setContentView(R.layout.windowstub_layout);
        
        Window win = getWindow();
        WindowManager.LayoutParams lp = win.getAttributes();
        lp.flags |= WindowManager.LayoutParams.FLAG_HOMEKEY_DISPATCHED;
        win.setAttributes(lp);        
        
        mIsOnHomeKeyDownCalled = false;
        mIsOnHomeKeyUpCalled = false;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, Menu.NONE, Menu.NONE, "Quit").setShortcut('1', 'q');
        menu.add(Menu.NONE, Menu.NONE, Menu.NONE, "Action").setShortcut('2', 'a');
        mIsOnCreateOptionsMenuCalled = true;
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public void onOptionsMenuClosed(Menu menu) {
        super.onOptionsMenuClosed(menu);
        mIsOnOptionsMenuClosedCalled = true;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        mIsOnKeyDownCalled = true;
    	  if (keyCode == KeyEvent.KEYCODE_HOME) {
    	  	  Log.d("WindowStubActivity", "home key is pressed");
    	      mIsOnHomeKeyDownCalled = true;	
    	  }        
        return super.onKeyDown(keyCode, event);
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
    	  if (keyCode == KeyEvent.KEYCODE_HOME) {
    	  	  Log.d("WindowStubActivity", "home key is up");
    	      mIsOnHomeKeyUpCalled = true;	
    	  }        
        return super.onKeyDown(keyCode, event);
    }

    public boolean isOnCreateOptionsMenuCalled() {
        return mIsOnCreateOptionsMenuCalled;
    }

    public boolean isOnOptionsMenuClosedCalled() {
        return mIsOnOptionsMenuClosedCalled;
    }

    public boolean isOnKeyDownCalled() {
        return mIsOnKeyDownCalled;
    }

    public boolean isOnHomeKeyDownCalled() {
        return mIsOnHomeKeyDownCalled;
    }
    
    public boolean isOnHomeKeyUpCalled() {
        return mIsOnHomeKeyUpCalled;
    }
    
    public void setFlagFalse() {
        mIsOnCreateOptionsMenuCalled = false;
        mIsOnOptionsMenuClosedCalled = false;
    }
}
