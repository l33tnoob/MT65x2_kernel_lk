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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class MockANRReceiver extends BroadcastReceiver {

    // PendingIntent may return same instance or new instance, so static variable is needed.
    public static final String MOCKACTION = "android.content.cts.ANR.BROADCAST_MOCKTEST";

    /**
     * set the result as true when received alarm
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("MockANRReceiver", "onReceive");
        
        try {
            Thread.sleep(90 * 1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
