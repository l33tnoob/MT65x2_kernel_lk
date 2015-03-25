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

package mediatek.StorageService.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.SystemClock;

/**
 * this class  receive alarm from AlarmManagerTest
 */
public class MockStorageReceiver extends BroadcastReceiver {
    public boolean mStorageLow = false;
    public boolean mStorageFull = false;
    public boolean mStorageNotFull = false;
    public boolean mStorageOk = false;
    private Object mSync = new Object();
    public static final String LOW = Intent.ACTION_DEVICE_STORAGE_LOW;
    public static final String FULL = Intent.ACTION_DEVICE_STORAGE_FULL;
    public static final String NOTFULL = Intent.ACTION_DEVICE_STORAGE_NOT_FULL;
    public static final String OK = Intent.ACTION_DEVICE_STORAGE_OK;

    @Override
    public void onReceive(Context context, Intent intent) {
        final String action = intent.getAction();
        if (action.equals(LOW)) {
            synchronized (mSync) {
                mStorageLow = true;
            }
        } else if (action.equals(FULL)) {
            synchronized (mSync) {
                mStorageFull = true;
            }
        } else if (action.equals(NOTFULL)) {
            synchronized (mSync) {
                mStorageNotFull = true;
            }
        } else if (action.equals(OK)) {
            synchronized (mSync) {
                mStorageOk = true;
            }
        }
    }

    public void setStorageFlagFalse() {
        synchronized (mSync) {
            mStorageLow = false;
            mStorageFull = false;
            mStorageNotFull = false;
            mStorageOk = false;
        }
    }
}
