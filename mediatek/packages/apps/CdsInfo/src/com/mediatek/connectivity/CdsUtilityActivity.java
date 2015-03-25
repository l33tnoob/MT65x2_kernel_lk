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
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.connectivity;


import java.net.InetAddress;
import org.apache.http.Header;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;
import android.app.ProgressDialog; 
import android.app.Activity;
import android.content.Context;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.mediatek.xlog.Xlog;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Spinner;
import android.os.SystemProperties;


public class CdsUtilityActivity extends Activity implements View.OnClickListener {
    private static final String TAG = "CDSINFO/CdsUtilityActivity";

    private static final String HTTPSTRING = "http://";
    private static final String PINGSTRING = "ping -c 5 www.google.com";
    private static final String PINGV6STRING = "ping6 -c 5 www.google.com";

    private ProgressDialog mDialog = null; 

    private Context mContext;
    private ConnectivityManager mConnMgr;
    private ProgressThread mProgressThread = null;
    private ArrayAdapter<String> mAutoCompleteAdapter;
    private AutoCompleteTextView mCmdLineList;

    private TextView mOutputScreen = null;
    private TextView mSysPropName = null;
    private TextView mSysPropValue = null;

    private int mCmdOption = 0;

    private static final String[] WEBSITES = new String[] {"netstat", "mtk_ifconfig", "iptables -t filter -L -n",
        "ping -c 1 -s 0 www.google.com",        
        "http://www.google.com","http://www.baidu.cn", "http://www.sina.cn", 
        "ps", "getprop", "setprop ",
        "8.8.8.8"};
    private static final String[] CMDTYPESTRING = new String[] {"SHELL", "PING", "PING IPV6", "DNS", "HTTP RESPONSE"};
    private static final String[] SYSPROP_LIST = new String[] {"media.wfd.video-format", "wlan.wfd.bt.exclude", "wfd.dumpts", "wfd.dumprgb", "wfd.slice.size"};
    
    private static final int RUN          = 0x1001;
    private static final int PING         = 0x1002;
    private static final int PINGV6       = 0x1003;
    private static final int DNS          = 0x1004;
    private static final int HTTPRESPONSNE =  0x1005;

    private static final int BASE          = RUN;

    private static final int MSG_UPDATE_UI = 0x3001;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.cds_network_tool);

        mContext = this.getBaseContext();
        if(mContext == null) {
            Xlog.e(TAG, "Could not get Conext of this activity");
        }

        mConnMgr = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        if(mConnMgr == null) {
            Xlog.e(TAG, "Could not get Connectivity Manager");
            return;
        }

        mAutoCompleteAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_dropdown_item_1line, WEBSITES);

        mCmdLineList = (AutoCompleteTextView) findViewById(R.id.cmdLine);
        mCmdLineList.setThreshold(3);
        mCmdLineList.setAdapter(mAutoCompleteAdapter);

        mOutputScreen = (TextView) findViewById(R.id.outputText);
        mSysPropName  = (EditText) findViewById(R.id.syspropName);
        mSysPropValue  = (EditText) findViewById(R.id.syspropValue);

        Spinner spinner = (Spinner) findViewById(R.id.cmdSpinnner);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, CMDTYPESTRING);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1,
            int position, long arg3) {
                // TODO Auto-generated method stub

                position += BASE;

                if(position == PING) {
                    mCmdLineList.setText(PINGSTRING);
                } else if(position == PINGV6) {
                    mCmdLineList.setText(PINGV6STRING);
                }else if(position == HTTPRESPONSNE) {
                    mCmdLineList.setText(HTTPSTRING);
                } else if(position == RUN) {
                    mCmdLineList.setText("");
                } else {
                    mCmdLineList.setText("www.google.com");
                }
                mCmdOption = position;
                mCmdLineList.requestFocus();
                mCmdLineList.setSelection(mCmdLineList.getText().length());
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub

            }
        });

        Button button = (Button)findViewById(R.id.runBtn);
        button.setOnClickListener(this);

        button = (Button)findViewById(R.id.setBtn);
        button.setOnClickListener(this);

        button = (Button)findViewById(R.id.getBtn);
        button.setOnClickListener(this);

        Spinner spinner2 = (Spinner) findViewById(R.id.sysPropSpinnner);
        ArrayAdapter<String> adapter2 = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_item, SYSPROP_LIST);
        adapter2.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner2.setAdapter(adapter2);
        spinner2.setOnItemSelectedListener(new Spinner.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View arg1,
            int position, long arg3) {
                 mSysPropName.setText(adapterView.getSelectedItem().toString());
            }
            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
                // TODO Auto-generated method stub

            }
        });

        Xlog.i(TAG, "CdsUtilityActivity is started");
    }

    @Override
    protected void onResume() {
        super.onResume();

    }


    @Override
    public void onPause() {
        super.onPause();


    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();

    }


    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();

    }

    public void onClick(View v) {
        int buttonId = v.getId();

        Xlog.d(TAG, "button id:" + buttonId);

        switch(buttonId) {
            case R.id.runBtn:
                handleRunCmd();
                break;
            case R.id.setBtn:
                handleSysProp(true);
                break;
            case R.id.getBtn:
                handleSysProp(false);
                break;
            default:
                break;
        }
    }

    private void handleSysProp(Boolean isSet){
        
        String name = mSysPropName.getText().toString();
        String value = mSysPropValue.getText().toString();
        
        if(name.length() <= 0 || name.length() > SystemProperties.PROP_NAME_MAX){
           String errMsg = "Please input the correct system property name";
           Xlog.e(TAG, errMsg);
           mOutputScreen.setText(errMsg);
           return;
        }
        
        if(isSet){
            if(name.length() <= 0 || name.length() > SystemProperties.PROP_VALUE_MAX){
                String errMsg = "Please input the correct system property value";
                Xlog.e(TAG, errMsg);
                mOutputScreen.setText(errMsg);
                return;
            }
            Xlog.i(TAG, "name:" + name + "/" + value);
            SystemProperties.set(name, value);
        }

        String result = SystemProperties.get(name);
        Xlog.i(TAG, "result:" + result);
        mSysPropValue.setText(result);
        mOutputScreen.setText(result);
    }

    private void handleRunCmd() {

        String cmdStr = mCmdLineList.getText().toString();
        Xlog.d(TAG, "" + cmdStr);

        if(mCmdOption == PING || mCmdOption == RUN || mCmdOption == PINGV6) {
            Xlog.i(TAG, "Run PING/RUN command");
            new Thread( new Runnable() {
                public void run() {
                    mProgressThread = new ProgressThread(mHandler);
                    try {
                        String cmdLineStr = mCmdLineList.getText().toString();
                        
                        mProgressThread.start();
                        CdsShellExe.execCommand(cmdLineStr);
                    } catch(Exception e) {
                        e.printStackTrace();
                    } finally {
                        mProgressThread.setState(ProgressThread.STATE_DONE);
                    }
                }
            }).start();
        } else if(mCmdOption == HTTPRESPONSNE) {
            Xlog.i(TAG, "Run HTTPRESPONSNE command");
            HttpTask httpTask = new HttpTask();
            httpTask.execute(cmdStr);
        } else if(mCmdOption == DNS) {
            Xlog.i(TAG, "Run DNS command");
            DnsTask dnsTask = new DnsTask();
            dnsTask.execute(cmdStr);
        }
    };

    // Define the Handler that receives messages from the thread and update the
    // progress
    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            String output = "";
            
            switch(msg.what) {
                case MSG_UPDATE_UI:
                    output = CdsShellExe.getOutput();
                    mOutputScreen.setText(output);
                    break;
                default:
                    break;
            }
        }
    };


    /** Nested class that performs screen update */
    private class ProgressThread extends Thread {
        Handler mHandler = null;
        private final static int STATE_DONE = 0;
        private final static int STATE_RUNNING = 1;
        private int mState = 0;

        ProgressThread(Handler h) {
            this.mHandler = h;
        }

        public void run() {
            setState(STATE_RUNNING);
            
            while (STATE_RUNNING == mState) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    Xlog.e(TAG, "Thread Interrupted");
                }

                Message msg = mHandler.obtainMessage();
                msg.what = MSG_UPDATE_UI;
                mHandler.sendMessage(msg);
            }
        }

        /**
        * sets the current state for the thread, used to stop the thread
        */
        public void setState(int state) {
            mState = state;
        }
    }

    private class HttpTask extends AsyncTask<String, Void, String> {
        protected String doInBackground(String... params) {
            StringBuilder mOutputString = new StringBuilder();

            try {
                String host = params[0];
                HttpClient httpClient = new DefaultHttpClient();
                HttpGet httpGet = new HttpGet(host);
                HttpResponse httpResponse = httpClient.execute(httpGet);
                Header[] headers = httpResponse.getAllHeaders();

                for(int i =0; i < headers.length; i++) {
                    mOutputString.append(headers[i].toString() + "\r\n");
                }
                Xlog.i(TAG, "Http result:" + mOutputString);
            } catch(Exception e) {
                e.printStackTrace();
                mOutputString.append(e.toString());
            }
            return mOutputString.toString();
        }

        protected void onPostExecute(String result) {
            mOutputScreen.setText(result);
        }

    }

    private class DnsTask extends AsyncTask<String, Void, String> {

        protected String doInBackground(String... params) {
            String mOutputString ="";

            try {
                String hostInfo = "";
                String host = params[0];
                InetAddress addresses[]  = InetAddress.getAllByName(host);

                for (int i=0; i<addresses.length; i++) {
                    hostInfo = i + ":" + "(" + addresses[i].getHostName() + "/" + addresses[i].getHostAddress() + ")\r\n" + addresses[i].getCanonicalHostName() + "\r\n";
                    mOutputString += hostInfo;
                }
                Xlog.i(TAG, "Dns result:" + mOutputString);
            } catch(Exception e) {
                e.printStackTrace();
                mOutputString = e.toString();
            }
            return mOutputString;
        }

        protected void onPostExecute(String result) {
            mOutputScreen.setText(result);
        }
    }

}
