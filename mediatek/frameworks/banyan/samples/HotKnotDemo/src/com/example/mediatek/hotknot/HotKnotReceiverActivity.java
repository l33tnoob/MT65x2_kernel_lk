/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.example.mediatek.hotknot;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

import com.mediatek.hotknot.HotKnotAdapter;


public class HotKnotReceiverActivity extends Activity {
    HotKnotAdapter mHotKnotAdapter;
    TextView mInfoText;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mInfoText = (TextView) findViewById(R.id.textView);
    }

    @Override
    public void onResume() {
        super.onResume();
        // Check to see that the Activity started due to an HotKnot message.
        if (HotKnotAdapter.ACTION_MESSAGE_DISCOVERED.equals(getIntent().getAction())) {
            processIntent(getIntent());
        }
    }

    @Override
    public void onNewIntent(Intent intent) {
        // onResume gets called after this to handle the intent.
        setIntent(intent);
    }

    /**
     * Parses the HotKnot message from the intent and prints to the TextView
     */
    void processIntent(Intent intent) {
        byte[] rawMsgs = intent.getByteArrayExtra(HotKnotAdapter.EXTRA_DATA);
        mInfoText.setText(new String(rawMsgs));
    }
}
