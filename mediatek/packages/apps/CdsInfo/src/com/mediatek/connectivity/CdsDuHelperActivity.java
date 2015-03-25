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

package com.mediatek.connectivity;

import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Xml;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Bundle;
import android.content.pm.ActivityInfo;

public class CdsDuHelperActivity extends Activity {
    private Button mBtnSnap;
    private Button mBtnExit;
    private Button mBtnClear;
    private TextView mTv;
    private ListView mListview;
    private ArrayAdapter<String> mMyArrayAdapter;
    private Toast mToast;
    private CdsDuHelperActivity mThis;
    ArrayList<HashMap<String,String>> mList = new ArrayList<HashMap<String,String>>();
    private SimpleAdapter mAdapter;



    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cds_du_main);

        mListview = (ListView)findViewById(R.id.listview);
        mToast = Toast.makeText(this, null, Toast.LENGTH_SHORT);
        mThis = this;
        
        mAdapter = new SimpleAdapter(
            this,
            mList,
            R.layout.simple_list_item2,
            new String[] { "name","data" },
            new int[] { android.R.id.text1, android.R.id.text2 } );
        mListview.setAdapter( mAdapter );




        // SnapShot button
        this.mBtnSnap = (Button)this.findViewById(R.id.snapshot);
        this.mBtnSnap.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                Intent intent = new Intent("mediatek.net.datausg.DATA_INFO_WRITE");
                intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY);
                getApplicationContext().sendBroadcast(intent);
            }
        });
        // Exit button
        this.mBtnExit = (Button)this.findViewById(R.id.exit);
        this.mBtnExit.setOnClickListener(new OnClickListener() {
            public void onClick(View arg0) {
                finish();
            }
        });
        // Clear button
        this.mBtnClear = (Button)this.findViewById(R.id.clear);
        this.mBtnClear.setOnClickListener(new OnClickListener() {

            public void onClick(View arg0) {

                AlertDialog.Builder builder = new AlertDialog.Builder(mThis);
                builder.setMessage("WARNING: Data Usage history data will all be deleted and the device will be reboot")
                .setCancelable(false)
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent("mediatek.net.datausg.DATA_INFO_DEL");
                        intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY);
                        getApplicationContext().sendBroadcast(intent);

                        mToast.setText("Data Usage statistics are deleted");
                        mToast.show();
                    }
                }).setNegativeButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });
                AlertDialog alert = builder.create();
                alert.show();
            }
        });
    }

    @Override
    protected void onResume() {
        IntentFilter filter = new IntentFilter();
        filter.addAction("mediatek.net.datausg.DATA_INFO_DONE");
        registerReceiver(mAppUsageReceiver, filter);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(mAppUsageReceiver);
        super.onDestroy();
    }
    
    @Override
    public void onPause() {
        super.onPause();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);

    }   
    

    private BroadcastReceiver mAppUsageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            HashMap<String,String> item;
            
            mToast.setText("Log is saved");
            mToast.show();
            String name = "";
            String data = "";

            int i = 1;
            String info=intent.getStringExtra("extra");
            if (info!= null) {
                String [] show = info.split(">");

                //mMyArrayAdapter.clear();
                mList.clear();
                
                name = "[File Path]";
                data = "/data/user/duhelper_XXXXXXX_YYYYYY.log";
                item = new HashMap<String,String>();
                item.put( "name", name );
                item.put( "data", data );
                mList.add( item );

                
                for (String s: show) {
                    if (s.startsWith("<?") || s.startsWith("</"))
                        continue;

                    s = s.replace("/", "");
                    s = s.replace("<", "");
                    if (s.startsWith("datausage")) {
                        s = s.replace("datausage-helper version=\"1\" DateTime=\"_", "");
                        name = "[DataTime]";
                        data = s.replace("\"", "");
                    }
                    else if (s.startsWith("App")) {
                        s = s.replace("App", "("+i+")");
                        String [] app = s.split("BgData");
                        name = app[0];
                        name = name.replace("unknown","android:uid:system:0");
                        data = "Background Data" + app[1] + " Bytes";
                        i++;
                    }
                    //mMyArrayAdapter.add(s);
                    item = new HashMap<String,String>();
                    item.put( "name", name );
                    item.put( "data", data );
                    mList.add( item );

                }
                mAdapter.notifyDataSetChanged();
                //mMyArrayAdapter.notifyDataSetChanged();
            }
        }
    };
}