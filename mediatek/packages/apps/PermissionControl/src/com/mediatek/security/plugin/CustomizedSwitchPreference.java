/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.security.plugin;

import android.content.Context;
import android.preference.SwitchPreference;
import android.util.AttributeSet;

import com.mediatek.xlog.Xlog;

public class CustomizedSwitchPreference extends SwitchPreference {
    private static final String TAG = "CustomizedSwitchPreference";

    public CustomizedSwitchPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public CustomizedSwitchPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomizedSwitchPreference(Context context) {
        super(context);
    }

    @Override
    protected void onClick() {
        Xlog.d(TAG,"onClick()");
    }
}
