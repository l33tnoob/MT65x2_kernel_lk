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
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.text.format.Time;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.hotknot.HotKnotAdapter.OnHotKnotCompleteCallback;
import com.mediatek.hotknot.HotKnotMessage;

import java.io.File;


public class HotKnotSendActivity extends Activity implements OnHotKnotCompleteCallback,
        View.OnClickListener{
    private static final String TAG = "HotKnotSendActivity";
    private static final String MIMETYPE ="application/com.example.mediatek.hotknot";
    private static final int MESSAGE_SENT_SUCCESS = 0;
    private static final int MESSAGE_SENT_FAIL = 1;
    private final static int MENU_SHOW_SYSTEM_SETTINGS = 0;
    
    private HotKnotAdapter mHotKnotAdapter;
    private TextView mInfoText;
    private Button mSendMessageButton;
    private Button mSetMsgCallbackBtn;
    private Button mSendBeamUriButton;
    private Button mSetBeamCallbackBtn;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hotknot_send);
        
        mSendMessageButton = (Button) findViewById(R.id.sendMessageBtn);
        mSetMsgCallbackBtn = (Button) findViewById(R.id.setMessageCbBtn);
        mSendBeamUriButton = (Button) findViewById(R.id.sendBeamUriBtn);
        mSetBeamCallbackBtn = (Button) findViewById(R.id.setBeamUriCbBtn);
        
        mInfoText = (TextView) findViewById(R.id.textView);
        Log.d(TAG, "MineType is " + MIMETYPE);
        // Check for available HotKnotAdapter.
        mHotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
        if (mHotKnotAdapter == null) {
            mInfoText.setText("HotKnot is not available on this device.");
            mSendMessageButton.setEnabled(false);
            mSetMsgCallbackBtn.setEnabled(false);
            mSetBeamCallbackBtn.setEnabled(false);
            mSendBeamUriButton.setEnabled(false);
        } else {
            mSendMessageButton.setOnClickListener(this);
            mSetMsgCallbackBtn.setOnClickListener(this);
            mSendBeamUriButton.setOnClickListener(this);
            mSetBeamCallbackBtn.setOnClickListener(this);
            
            if(!mHotKnotAdapter.isEnabled()){
                mInfoText.setText("HotKnot is disabled, please press menu key and go to system settings to enable it");
            }
        }
    }

    @Override
    public void onClick(View v) {
        if(mHotKnotAdapter != null && !mHotKnotAdapter.isEnabled()) {
            Intent intent = new Intent(HotKnotAdapter.ACTION_HOTKNOT_SETTINGS);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
            startActivity(intent);
            return;
        }
        if(v == mSendMessageButton) {
            sendHotknotMessageData();
        } else if(v == mSetMsgCallbackBtn) {
            setHotknotMessageCallback();
        } else if(v == mSendBeamUriButton) {
            sendHotknotBeamFiles();
        } else if(v == mSetBeamCallbackBtn) {
            setHotKnotBeamCallback();
        }
    }
    
    private void sendHotknotMessageData() {
    	// Remove HotKnot exchange registrations that have higher priorities.
    	// setHotKnotBeamUrisCallback() > setHotKnotBeamUris() >
    	// setHotknotMessageCallback() > setHotKnotMessage().
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUris(null, this);
        mHotKnotAdapter.setHotKnotMessageCallback(null, this);

        // Register new HotKnot message for sending.
        mHotKnotAdapter.setHotKnotMessage(new HotKnotMessage(MIMETYPE, gerenateDate().getBytes()), this);
        // Register callback to listen for message-sent success.
        mHotKnotAdapter.setOnHotKnotCompleteCallback(this, this);
    }

    private String gerenateDate() {
        String data = null;
        Time time = new Time();
        time.setToNow();
        data = ("HotKnot me up!\n\n" + "HotKnot Time: " + time.format("%H:%M:%S"));
        return data;
    }
    
    /**
     * Implementation for the createHotKnotMessage interface.
     */
    private class MessageCallback  implements HotKnotAdapter.CreateHotKnotMessageCallback {
        public MessageCallback() {
        }
        
        @Override
        public HotKnotMessage createHotKnotMessage() {
            HotKnotMessage msg = new HotKnotMessage(MIMETYPE, gerenateDate().getBytes());
            return msg;
        }
    }
    
    private void setHotknotMessageCallback() {
    	// Remove HotKnot exchange registrations that have higher priorities.
    	// setHotKnotBeamUrisCallback() > setHotKnotBeamUris() >
    	// setHotknotMessageCallback().
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        mHotKnotAdapter.setHotKnotBeamUris(null, this);
        
        // Register callback to set HotKnot message.
        MessageCallback messageCallback = new MessageCallback();
        mHotKnotAdapter.setHotKnotMessageCallback(messageCallback, this);
        // Register callback to listen for message-sent success.
        mHotKnotAdapter.setOnHotKnotCompleteCallback(this, this);
    }

    /**
     * Implementation for the onHotKnotComplete interface
     */
    @Override
    public void onHotKnotComplete(int reason) {
        // A handler is needed to send messages to the activity when this
        // callback occurs, because it happens from a binder thread.
        mHandler.obtainMessage(reason == 0 ? MESSAGE_SENT_SUCCESS : MESSAGE_SENT_FAIL).sendToTarget();
    }
    
    /**
     * Send HotKnot Beam files, because beam callback is higher than send beam files,
     * so firstly set setHotKnotBeamUrisCallback as null
     */
    private void sendHotknotBeamFiles() {
        Uri[] uris = getFileUris();
        if(uris == null){
            return;
        }        
    	// Remove HotKnot exchange registrations that have higher priorities.
    	// setHotKnotBeamUrisCallback() > setHotKnotBeamUris().
        mHotKnotAdapter.setHotKnotBeamUrisCallback(null, this);
        // Register new file (URI) for sending.
        mHotKnotAdapter.setHotKnotBeamUris(uris, this);
    }

    /**
     * Callback when another device capable of HotKnot Beam transfer is within range.
     */
    private class FileUriCallback implements HotKnotAdapter.CreateHotKnotBeamUrisCallback {
        public FileUriCallback() {
        }
        /**
         * Create content URIs as needed to share with another device
         */
        @Override
        public Uri[] createHotKnotBeamUris() {
            return getFileUris();
        }
    }
    private void setHotKnotBeamCallback() {
        // Instantiate a new FileUriCallback to handle requests for URIs.
        FileUriCallback fileUriCallback = new FileUriCallback();
        // Set the dynamic callback for URI requests.
        mHotKnotAdapter.setHotKnotBeamUrisCallback(fileUriCallback, this);
    }

    /*
     * Create a list of URIs, get a File, and set its permissions.
     */
    private Uri[] getFileUris() {
        Uri[] fileUris = new Uri[1];
        String transferFile = "transferimage.jpg";
        File extDir = Environment.getExternalStorageDirectory();
        File requestFile = new File(extDir, transferFile);
        Log.d(TAG, "request file is " + requestFile.getPath());
        if (!requestFile.exists()) {
        	Toast.makeText(this, "File does not exist,  please push an \"trasnferimage.jpg\" to " + extDir, Toast.LENGTH_LONG).show();
        	return null;
        }
        requestFile.setReadable(true, false);
        // Get a URI for the File and add it to the list of URIs.
        Uri fileUri = Uri.fromFile(requestFile);
        if (fileUri != null) {
            fileUris[0] = fileUri;
        } else {
            Log.e(TAG, "No File URI available for file.");
        }
        return fileUris;
    }

    @Override
    public void onResume() {
        super.onResume();
        if(mHotKnotAdapter != null && mHotKnotAdapter.isEnabled()){
            mInfoText.setText(R.string.info);
        } else {
            mInfoText.setText("HotKnot is disabled, please press menu key and go to system settings to enable it");
        }
    }
    
    /** This handler receives a message from onHotKnotComplete */
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MESSAGE_SENT_SUCCESS:
                Toast.makeText(getApplicationContext(), "Message sent successfully!", Toast.LENGTH_LONG).show();
                break;
            case MESSAGE_SENT_FAIL:
                Toast.makeText(getApplicationContext(), "Failed to send message!", Toast.LENGTH_LONG).show();
                break;
            }
        }
    };
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        if(mHotKnotAdapter != null) {
            menu.add(Menu.NONE, MENU_SHOW_SYSTEM_SETTINGS, 0, R.string.hotknot_menu_description)
            .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            return true;
        }
        return false;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if(item.getItemId() == MENU_SHOW_SYSTEM_SETTINGS) {
            // Start the HotKnot settings activity to enable HotKnot.
            Intent intent = new Intent(HotKnotAdapter.ACTION_HOTKNOT_SETTINGS);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
            startActivity(intent);
        }
        return super.onOptionsItemSelected(item);
    }
}
