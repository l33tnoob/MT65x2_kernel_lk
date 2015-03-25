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

import java.io.File;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.widget.*;
import android.view.*;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.AdapterView.OnItemClickListener;
import android.os.Bundle;

public class CoolersActivity extends Activity implements OnItemClickListener{
    private ListView coolers_lv;
    private List<String> items;

    /** Called when the activity is first created. */
    
    private class CoolerLoader extends AsyncTask<Void, Void, Void> {
        @Override
        protected void onPostExecute(Void result) {
            if  (0 == items.size())
            {
                Toast.makeText(CoolersActivity.this, "No coolers found!", Toast.LENGTH_LONG).show();
            }
            coolers_lv.setAdapter(new ArrayAdapter<String>(CoolersActivity.this, android.R.layout.simple_list_item_1, items));
            return;
        }

        @Override
        protected Void doInBackground(Void... params) {
            items.clear();
            
            // TODO Auto-generated method stub
            File sys_class_thermal_dir = new File("/sys/class/thermal");
            if (!sys_class_thermal_dir.exists())
            {
                //Toast.makeText(CoolersActivity.this, "No coolers found!", Toast.LENGTH_LONG).show();
                return null;
            }
              
            File[] thermal_dev_file_list = sys_class_thermal_dir.listFiles();
            if (null == thermal_dev_file_list)
            {
                //Toast.makeText(CoolersActivity.this, "No coolers found!", Toast.LENGTH_LONG).show();
                return null;
            }

            for (int i = 0; i < thermal_dev_file_list.length; i++)
            {
                if( thermal_dev_file_list[i].getName().contains("cooling_device"))
                {
                    // get the type, cur_state, and max_state
                    // compose it to type -- cur_state/max_state
                    // add to list
                    //items.add(thermal_dev_file_list[i].getName());
                    String toshow = new String();
                  
                    // get type
                    File a = new File(thermal_dev_file_list[i].getPath()+"/type");
                    if (a.exists())
                    {
                        try {
                            FileInputStream fs = new FileInputStream(a);
                            DataInputStream ds = new DataInputStream(fs);
                            toshow+=ds.readLine();
                            ds.close();
                            fs.close();
                        }
                        catch (Exception ex) {

                        }
                    }
                  
                    // get cur_state
                    File b = new File(thermal_dev_file_list[i].getPath()+"/cur_state");
                    if (b.exists())
                    {
                        try {
                            FileInputStream fs = new FileInputStream(b);
                            DataInputStream ds = new DataInputStream(fs);
                            toshow+=" -- ";
                            toshow+=ds.readLine();
                            toshow+=" / ";
                            ds.close();
                            fs.close();
                        }
                        catch (Exception ex) {

                        }
                    }
                      
                    // get max_state
                    File c = new File(thermal_dev_file_list[i].getPath()+"/max_state");
                    if (c.exists())
                    {
                        try {
                            FileInputStream fs = new FileInputStream(c);
                            DataInputStream ds = new DataInputStream(fs);
                            toshow+=ds.readLine();
                            ds.close();
                            fs.close();
                        }
                        catch (Exception ex) {

                        }
                    }
                      
                    items.add(toshow);
                }
            }
            
            return null;
        }
        
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    
        // TODO Auto-generated method stub
        setContentView(R.layout.coolers);
        
        /*
         *  Initialize listView_main
         */
        items = new ArrayList<String>();
        coolers_lv = (ListView) findViewById(R.id.listView_coolers);
        coolers_lv.setOnItemClickListener(this);
        new CoolerLoader().execute();
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
    }
}
