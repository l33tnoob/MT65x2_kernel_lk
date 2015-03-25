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
import android.app.Dialog;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;

import com.mediatek.cts.activity.stub.R;

/**
 * A minimal "Hello, World!" application.
 */
public class AppStubActivity extends Activity {
    private Dialog mDialog;
    public boolean mOnPrepareDialog;
    public boolean mOnOptionsMenuClosedCalled;
    public boolean mOnPrepareOptionsMenuCalled;
    public boolean mOnOptionsItemSelectedCalled;
    public boolean mOnCreateOptionsMenu;
    public boolean mIndterminate = false;
    public boolean mIndterminatevisibility = false;
    public boolean mSecPro = false;
    public boolean mOnContextItemSelectedCalled;
    public boolean mOnCreateContextMenu;
    public boolean mApplyResourceCalled;
    public boolean mCreateContextMenuCalled;
    public boolean mRequestWinFeatureRet = false;

    public AppStubActivity() {

    }

    public void finalize() {
        try {
            super.finalize();
        } catch (Throwable exception) {
            System.err.print("exception!");
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState){
        super.onCreate(savedInstanceState);
        mRequestWinFeatureRet = requestWindowFeature(1);
        setContentView(R.layout.app_activity);
    }

    public Dialog getDialogById(int id) {
        return mDialog;
    }

    @Override
    public Dialog onCreateDialog(int id) {
        super.onCreateDialog(id);
        mDialog = new Dialog(this);
        return mDialog;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        super.onPrepareDialog(id, dialog);
        mOnPrepareDialog = true;
    }

    @Override
    public void onOptionsMenuClosed(Menu menu) {
        super.onOptionsMenuClosed(menu);
        mOnOptionsMenuClosedCalled = true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        mOnPrepareOptionsMenuCalled = true;
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mOnCreateOptionsMenu = true;
        if(menu != null)
            menu.add(0, 0, 0, "Fake Item");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        mOnOptionsItemSelectedCalled = true;
        return super.onOptionsItemSelected(item);
    }

    public boolean setProBarIndeterminate(boolean indeterminate){
        mIndterminate = indeterminate;
        super.setProgressBarIndeterminate(indeterminate);
        return mIndterminate;
    }

    public boolean setProBarIndeterminateVisibility(boolean visible){
        mIndterminatevisibility = visible;
        super.setProgressBarIndeterminateVisibility(visible);
        return mIndterminatevisibility;
    }

    public boolean setSecPro(int secPro){
        mSecPro = true;
        super.setSecondaryProgress(secPro);
        return mSecPro;
    }

    @Override
    public boolean onContextItemSelected(MenuItem item){
        mOnContextItemSelectedCalled = true;
        return super.onContextItemSelected(item);
    }

    @Override
    public void onApplyThemeResource( Resources.Theme theme,
                                      int resid,
                                      boolean first){
        super.onApplyThemeResource(theme,resid,first);
        mApplyResourceCalled = true;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu,v,menuInfo);
        mCreateContextMenuCalled = true;
    }
}

