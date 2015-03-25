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

package android.content.cts;

import android.app.Service;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

/**
 * This class is used for {@link ContextWrapper}
 *
 * @see ContextWrapperTest
 */
public class MockContextWrapperService extends Service {
    private static boolean mHadCalledOnBind = false;
    private static boolean mHadCalledOnUnbind = false;
    private static boolean mHadCalledOnStart = false;
    private static boolean mHadCalledOnDestory = false;
    private static final int TEST_MESSAGE_WHAT = 1;

    private final IBinder mBinder = new Binder();

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            mHandler.sendMessageDelayed(mHandler.obtainMessage(TEST_MESSAGE_WHAT), 1000);
        }
    };

    @Override
    public void onCreate() {
        mHandler.sendMessageDelayed(mHandler.obtainMessage(TEST_MESSAGE_WHAT), 1000);
    }

    @Override
    public void onDestroy() {
        mHadCalledOnDestory = true;
        mHandler.removeMessages(1);
    }

    @Override
    public boolean onUnbind(Intent intent) {
        mHadCalledOnUnbind = true;
        return true;
    }

    @Override
    public IBinder onBind(Intent intent) {
        mHadCalledOnBind = true;
        return mBinder;
    }

    @Override
    public void onStart(Intent intent, int startId) {
        mHadCalledOnStart = true;
    }

    public static void reset() {
        mHadCalledOnBind = false;
        mHadCalledOnUnbind = false;
        mHadCalledOnStart = false;
        mHadCalledOnDestory = false;
    }

    public static boolean hadCalledOnBind() {
        return mHadCalledOnBind;
    }

    public static boolean hadCalledOnUnbind() {
        return mHadCalledOnUnbind;
    }

    public static boolean hadCalledOnStart() {
        return mHadCalledOnStart;
    }

    public static boolean hadCalledOnDestory() {
        return mHadCalledOnDestory;
    }
}

