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

package com.mediatek.thermalmanager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;

import android.app.Activity;
import android.app.ListActivity;
import android.content.Intent;
import android.widget.*;
import android.view.*;
import android.os.Bundle;
import android.widget.AdapterView.OnItemClickListener;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;


public class TzDeviceActivity extends Activity implements OnItemClickListener{
    private ArrayList<HashMap<String,String>> list = new ArrayList<HashMap<String,String>>();
    private SimpleAdapter adapter;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tz_device);
        
        Intent intent = this.getIntent();
        String type = "";
        File tz_dir;
        
        TextView textView_this = (TextView) findViewById(R.id.textView_tz_device);

        if (intent.getStringExtra("tz_sysfs_path") != null)
        {
            tz_dir = new File(intent.getStringExtra("tz_sysfs_path"));
            Xlog.d("TzDeviceActivity", "sysfs_path: " + intent.getStringExtra("tz_sysfs_path"));
        }
        else
        {
            Toast.makeText(TzDeviceActivity.this, "File temperarily unavailable!", Toast.LENGTH_SHORT).show();
            Xlog.d("TzDeviceActivity", "sysfs_path: null!!!\n");
            return;
        }


        
        // read type
        File a = new File(tz_dir.getPath()+"/type");
        if (a.exists())
        {
            try {
                FileInputStream fs = new FileInputStream(a);
                DataInputStream ds = new DataInputStream(fs);
                type+=ds.readLine();
                ds.close();
                fs.close();
            }
            catch (Exception ex) {

            }
        }
        else
        {
            Toast.makeText(TzDeviceActivity.this, "type unavailable!", Toast.LENGTH_SHORT).show();
            Xlog.d("TzDeviceActivity", "type not exists\n");
        }
        
        textView_this.setText(type);
        
        ListView tz_device_lv = (ListView) findViewById(R.id.listView_tz_device);
        
        // Read temp...
        File b = new File(tz_dir.getPath()+"/temp");
        if (b.exists())
        {
            try {
                FileInputStream fs = new FileInputStream(b);
                DataInputStream ds = new DataInputStream(fs);
                mFileContent[0] = ds.readLine()+ " mdegree C";
                ds.close();
                fs.close();
            }
            catch (Exception ex) {

            }
        }
        else
        {
            Toast.makeText(TzDeviceActivity.this, "temp unavailable!", Toast.LENGTH_SHORT).show();
            Xlog.d("TzDeviceActivity", "temp not exists\n");
        }
        
        // Read mode...
        File c = new File(tz_dir.getPath()+"/mode");
        if (c.exists())
        {
            try {
                FileInputStream fs = new FileInputStream(c);
                DataInputStream ds = new DataInputStream(fs);
                mFileContent[1] = ds.readLine();
                ds.close();
                fs.close();
            }
            catch (Exception ex) {

            }
        }
        else
        {
            Toast.makeText(TzDeviceActivity.this, "mode unavailable!", Toast.LENGTH_SHORT).show();
            Xlog.e("TzDeviceActivity", "mode not exists\n");
        }
        
        int i = 0;
        // Read all TZs
        for (; i < 12; i++) // TODO: fix magic number here...need to sync with LTF...current max is 12....
        {
            // trip point i
            File trip_point_i_temp = new File(new String(tz_dir.getPath()+"/trip_point_"+i+"_temp"));
            if (!trip_point_i_temp.exists())
            {
                break;
            }
            else
            {
                mFileName[2+i] = i + ": ";
                                
                try {
                    FileInputStream fs = new FileInputStream(trip_point_i_temp);
                    DataInputStream ds = new DataInputStream(fs);
                    mFileName[2+i] += ds.readLine();
                    ds.close();
                    fs.close();
                }
                catch (Exception ex) {
                }
                
                mFileName[2+i] += " mdeg C -- ";
                
                File trip_point_i_type = new File(new String(tz_dir.getPath()+"/trip_point_"+i+"_type"));
                try {
                    FileInputStream fs = new FileInputStream(trip_point_i_type);
                    DataInputStream ds = new DataInputStream(fs);
                    mFileName[2+i] += ds.readLine();
                    ds.close();
                    fs.close();
                }
                catch (Exception ex) {
                }
                
                // TODO: need to modify here...since cdev number does not match trip directly...
                //File cdev_i_type = new File(tz_dir.getPath()+"/cdev"+i+"/type");
                //try {
                //    FileInputStream fs = new FileInputStream(cdev_i_type);
                //    DataInputStream ds = new DataInputStream(fs);
                //    mFileContent[2+i] = "Cooler" + i + " " + ds.readLine();
                //    ds.close();
                //    fs.close();
                //}
                //catch (Exception ex) {
                //}
            }
        }
        
        for (i = 0; i < 12; i++)
        {
            File cdev_i_trip_point = new File(new String(tz_dir.getPath()+"/cdev"+i+"_trip_point"));
            if (!cdev_i_trip_point.exists())
            {
                break;
            }
            else
            {
                int trip_point;
                try {
                    FileInputStream fs = new FileInputStream(cdev_i_trip_point);
                    DataInputStream ds = new DataInputStream(fs);
                    trip_point = Integer.valueOf(ds.readLine());
                    ds.close();
                    fs.close();
                }
                catch (Exception ex) {
                    break;
                }
                
                File cdev_i_type = new File(tz_dir.getPath()+"/cdev"+i+"/type");
                try {
                    FileInputStream fs = new FileInputStream(cdev_i_type);
                    DataInputStream ds = new DataInputStream(fs);
                    mFileContent[2+trip_point] = "Cooler" + i + " " + ds.readLine();
                    ds.close();
                    fs.close();
                }
                catch (Exception ex) {
                }
            }
        }
        
        // Add data to ArrayList
        for (int j=0; j<(2+i); j++) {
            HashMap<String,String> item = new HashMap<String,String>();
            item.put("fileName", mFileName[j]);
            item.put("fileContent", mFileContent[j]);
            list.add(item);
        }
        
        // New a SimpleAdapter
        adapter = new SimpleAdapter( 
            this, 
            list,
            android.R.layout.simple_list_item_2,
            new String[] {"fileName", "fileContent"},
            new int[] {android.R.id.text1, android.R.id.text2});
        
        // Set Adapter to ListView
        tz_device_lv.setAdapter(adapter);
        
    }
    
    private String[] mFileName = new String[] {
         "Temperature: ", 
         "Mode: ", 
         "ZZZ mdegree C <type>", 
         "YYY mdegree C <type>", 
         "XXX mdegree C <type>",
         "WWW mdegree C <type>",
         "ZZZ mdegree C <type>", 
         "YYY mdegree C <type>", 
         "XXX mdegree C <type>",
         "WWW mdegree C <type>",
         "ZZZ mdegree C <type>", 
         "YYY mdegree C <type>", 
         "XXX mdegree C <type>",
         "WWW mdegree C <type>"
         };
         
    private String[] mFileContent = new String[] {
         "50 degree C", 
         "kernel", 
         "cooler 4", 
         "cooler 3", 
         "cooler 2",
         "cooler 1",
         "cooler 4", 
         "cooler 3", 
         "cooler 2",
         "cooler 1",
         "cooler 4", 
         "cooler 3", 
         "cooler 2",
         "cooler 1"
         };
    
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
    }
}
