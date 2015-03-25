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

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Date;

import android.app.Activity;
import android.content.Intent;
import android.widget.*;
import android.util.Log;
import android.view.*;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.os.Build;
import android.os.Bundle;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;



public class MTKThermalManagerActivity extends Activity implements OnItemClickListener, CompoundButton.OnCheckedChangeListener, OnClickListener{
  /** Called when the activity is first created. */
  
  private TextView thermal_main_tv;
  private Switch thermal_logger_switch;
  
  private List<String> mtc_file_list;
  private ArrayAdapter<String> mtc_file_adapter;
  private int count;
  private String selected_file_name;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    if (!Build.TYPE.equals("eng"))
    {
        Toast.makeText(MTKThermalManagerActivity.this, "Only supported in eng build.", Toast.LENGTH_LONG).show();
        finish();
    }
  
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);
    
    Button button = (Button)findViewById(R.id.button1);
    button.setOnClickListener((OnClickListener) this);
    
    /*
     *  Initialize switch
     */
    thermal_main_tv = (TextView) findViewById(R.id.textView1);
    
    thermal_logger_switch = (Switch) findViewById(R.id.switch_thermal_logger);
    if (thermal_logger_switch != null) {
      thermal_logger_switch.setOnCheckedChangeListener(this);
    }
    thermal_logger_switch.setChecked(isThermalLoggerEnabled());
    
    /*
     *  Initialize listView_main
     */
    ListView thermal_main_lv = (ListView) findViewById(R.id.listView_main);
    List<String> items = new ArrayList<String>();
    items.add(getString(R.string.thermal_sensors));
    items.add(getString(R.string.coolers));
    thermal_main_lv.setAdapter(new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, items));
    thermal_main_lv.setOnItemClickListener(this);
    
    /**
     *  Initialize thermal policy file spinner
     */
    {
    	Spinner s1 = (Spinner) findViewById(R.id.spinner1);
    	if (null == mtc_file_list)
    	{
    		mtc_file_list = new ArrayList<String>();
    	}
    	if (null == mtc_file_adapter)
    	{
    		mtc_file_adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, mtc_file_list);
    	}
        //ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
        //        this, R.array.colors, android.R.layout.simple_spinner_item);
    	mtc_file_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        s1.setAdapter(mtc_file_adapter);
        s1.setOnItemSelectedListener(
                new OnItemSelectedListener() {
                    public void onItemSelected(
                            AdapterView<?> parent, View view, int position, long id) {
                    	Xlog.d("MTKThermalManagerActivity", "Spinner1 OnItemSelectedListener.onItemSelected() position=" + position + " id=" + id + "\n");
                    	/**
                    	 *  Get file name from mtc_file_list
                    	 */
                    	if (0 == position)
                    	{
                    		selected_file_name = "/etc/.tp/thermal.conf";
                    	}
                    	else if (1 == position)
                    	{
                    		selected_file_name = "/etc/.tp/thermal.off.conf";
                    	}
                    	else if (2 == position)
                    	{
                    		selected_file_name = "/etc/.tp/.ht120.mtc";
                    	}
                    	else
                    	{
                    		selected_file_name = mtc_file_list.get(position);
                    	}
                    	//Toast.makeText(MTKThermalManagerActivity.this, "Spinner1: position=" + position + " id=" + id + " filepath=" + selected_file_name, Toast.LENGTH_SHORT).show();
                    }

                    public void onNothingSelected(AdapterView<?> parent) {
                    	Xlog.d("MTKThermalManagerActivity", "Spinner1 OnItemSelectedListener.onNothingSelected()\n");
                    }
                });
        s1.setOnTouchListener(new OnTouchListener() {  
      	  
			public boolean onTouch(View arg0, MotionEvent arg1) {
				// TODO Auto-generated method stub
				/**
				 * When ever touched, search for mtc files under /data folder
				 */
				if (arg1.getAction() == MotionEvent.ACTION_DOWN)
				{
					count++;
					Xlog.d("MTKThermalManagerActivity", "Spinner1 OnTouchListener.onTouch() cnt=" + count + "\n");
					mtc_file_list.clear();
					mtc_file_list.add("default");
					mtc_file_list.add("thermal protection only");
					mtc_file_list.add("high temp 120deg C");
					{
						File data_dir = new File("/data");
				        if (!data_dir.exists())
				        {
				            return false;
				        }
				          
				        File[] data_file_list = data_dir.listFiles();
				        if (null == data_file_list)
				        {
				            return false;
				        }
				        
				        for (int i = 0; i < data_file_list.length; i++)
				        {
				        	if (data_file_list[i].isFile() && 
				        		data_file_list[i].getName().endsWith(".mtc"))
				        	{
				        		mtc_file_list.add(data_file_list[i].getPath());
				        	}
				        }
					}
	            	mtc_file_adapter.notifyDataSetChanged();
				}
				return false;
			}  
        }); 
    }
  }
    
  public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
    Intent intent = new Intent();
    
    switch(arg2)
    {
    case 0: 
      Xlog.d("MTKThermalManagerActivity", "onItemClick() ThermalSensorActivity");
      if (isThermalLoggerEnabled())
      {
          Toast.makeText(this, "Thermal Logger started! Cannot enter!", Toast.LENGTH_SHORT).show();
      }
      else
      {
          intent.setClass(this, ThermalSensorActivity.class);
          this.startActivity(intent);
      }
      break;
    case 1: 
      Xlog.d("MTKThermalManagerActivity", "onItemClick() CoolersActivity");
      intent.setClass(this, CoolersActivity.class);
      this.startActivity(intent);
      break;
    }
      
  }
    
  public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
    // disable buttonView
    buttonView.setFocusable(false);
    buttonView.setEnabled(false);
    buttonView.setClickable(false);
    if (((CompoundButton) thermal_logger_switch).equals(buttonView))
    {
      if (isChecked)
      {
        if (isThermalLoggerEnabled())
        {
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() 1 logger already started");
        }
        else
        {
          // start logging
          // TODO: get execution duration.
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() start logging");
          Toast.makeText(this, "Starting Thermal Logger!", Toast.LENGTH_SHORT).show();
          //executeShellCommand("echo 0 0 0 0 > /proc/driver/storage_logger_config");
          writeFile("/proc/driver/storage_logger_config", "0 0 0 0");
          writeFile("/proc/driver/storage_logger_config", "0 0 1"); // enable proc name
          //executeShellCommand("echo 5 > /proc/driver/thermal_logger_config");
          writeFile("/proc/driver/thermal_logger_config", "5");
          //Expand storage logger buffer size to 10MB...
          //WARNING! this might fail.
          writeFile("/proc/driver/storage_logger_bufsize_malloc", "10485760");
          //executeShellCommand("echo ENABLE 1 > /proc/driver/storage_logger");
          writeFile("/proc/driver/storage_logger", "ENABLE 1");
          
          long timems = System.currentTimeMillis();
          Date curtime = new Date();
          curtime.setTime(timems);
          SimpleDateFormat sdf = new SimpleDateFormat("kkmmss");
          String ctstr = sdf.format(curtime);
          writeFile("/proc/driver/mtk_thermal_monitor", "1 "+ctstr);
          Toast.makeText(this, "Thermal Logger started!", Toast.LENGTH_SHORT).show();
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() logging started");
        }
      }
      else
      {
        if (isThermalLoggerEnabled())
        {
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() stop logging");
          
          // stop logging
          Toast.makeText(this, "Stopping Thermal Logger!", Toast.LENGTH_SHORT).show();
          writeFile("/proc/driver/mtk_thermal_monitor", "0");
          //executeShellCommand("echo ENABLE 0 > /proc/driver/storage_logger");
          writeFile("/proc/driver/storage_logger", "ENABLE 0");
          
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() disable storage logger");
          
          // check for /data folder, Android system process is not allowed to access SD card since it cannot be killed...
          if(new File("/data").exists())
          {
            long timems = System.currentTimeMillis();
            Date curtime = new Date();
            curtime.setTime(timems);
            SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd-kkmmssZ");
            String ctstr = sdf.format(curtime);
            ctstr = ctstr.replaceAll(" ", "");
            
            Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() test write to /data");
            writeFile("/data/storage_logger_dump_"+ctstr, "");
            try {
              appendFile(new File("/proc/driver/storage_logger_display"), new File("/data/storage_logger_dump_"+ctstr));
            }
            catch (IOException e)
            {
              Xlog.e("MTKThermalManagerActivity", "onCheckedChanged() append file exception!");
            }
            
            Toast.makeText(this, "Thermal Log storage_logger_dump_"+ctstr+" dumped to /data!", Toast.LENGTH_LONG).show();
          }
          else
          {
            Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() data does not exist!");
            Toast.makeText(this, "No storage to dump thermal Log!\n Please use adb command: \n @adb pull /proc/driver/storage_logger_display storage_logger_dump  ", Toast.LENGTH_LONG).show();
          }
          //Expand storage logger buffer size back to 2MB...
          //WARNING! this might fail.
          writeFile("/proc/driver/storage_logger_bufsize_malloc", "2097152");
          // TODO: get execution duration.
        }
      }
    }
    else
    {
      Xlog.e("MTKThermalManagerActivity", "onCheckedChanged() buttonView not found!");
    }
    // enable it after finished.
    buttonView.setClickable(true);
    buttonView.setEnabled(true);
    buttonView.setFocusable(true);
  }
    
  public void onClick(View v)
  {
    //EditText editText = (EditText) findViewById(R.id.editText1);
    //String newPolicyFilePath = editText.getText().toString();
	String newPolicyFilePath = selected_file_name;
	
	if (null == newPolicyFilePath)
	    return;
    
    if (new File(newPolicyFilePath).exists())
    {
      if (newPolicyFilePath.contains(" "))
      {
        Toast.makeText(
          this, newPolicyFilePath + " not applied.\n" + 
          "Please do not use space characters in file names.",
          Toast.LENGTH_LONG).show();
        return;
      }

      // stop thermal logging first
      boolean thermal_log_on = isThermalLoggerEnabled();
      if (true == thermal_log_on)
      {
        thermal_logger_switch.setChecked(false);
      }

      turnOnThermalProtection(newPolicyFilePath);

      if (newPolicyFilePath.contains("/etc/.tp/"))
      {
        newPolicyFilePath = "Applied...";
      }

      Toast.makeText(
          this, 
          newPolicyFilePath,
          Toast.LENGTH_LONG).show();
      
      // restart thermal logging
      if (true == thermal_log_on)
      {
        thermal_logger_switch.setChecked(true);
      }
    }
    else
    {
      Toast.makeText(
          this, 
          selected_file_name+" is not found!",
          Toast.LENGTH_LONG).show();
    }

    return;
  }
    
  private void executeShellCommand(String shellCommand)
  {
    Runtime runtime = Runtime.getRuntime();
    Process proc = null;
    OutputStreamWriter osw = null;
    String command = "";

    try
    {
      proc = runtime.exec(shellCommand);
      osw = new OutputStreamWriter(proc.getOutputStream());
      if (null != osw)
      {
        osw.write(shellCommand);
        osw.write("\n");
        osw.write("exit\n");
        osw.flush();
        osw.close();
      }
    }
    catch (IOException ex)
    {
      //Xlog.e("MTKThermalManagerActivity", "execCommandLine() IO Exception: " + shellCommand);
      Xlog.e("MTKThermalManagerActivity", "execCommandLine() IO Exception: ");
      return;
    }
    finally
    {
      if (null != osw)
      {
        try
        {
          osw.close();
        }
        catch (IOException e){}
      }
    }

    try 
    {
      proc.waitFor();
    }
    catch (InterruptedException e){}

    if (proc.exitValue() != 0)
    {
      //Xlog.e("MTKThermalManagerActivity", "execCommandLine() Err exit code: " + proc.exitValue() + " " + shellCommand);
      Xlog.e("MTKThermalManagerActivity", "execCommandLine() Err exit code: " + proc.exitValue());
    }
  }
    
  private void turnOffThermalProtection()
  {
    executeShellCommand("/system/bin/thermal_manager /etc/.tp/thermal.off.conf");
  }
  
  private void turnOnThermalProtection(String confFile)
  {
    executeShellCommand("/system/bin/thermal_manager "+confFile);
  }
  
  private void turnOffThermalLoggerAndDumpLog()
  {
      Xlog.d("MTKThermalManagerActivity", "turnOffThermalLoggerAndDumpLog() stop logging");
      
      // stop logging
      Toast.makeText(this, "Stopping Thermal Logger!", Toast.LENGTH_SHORT).show();
      writeFile("/proc/driver/mtk_thermal_monitor", "0");
      writeFile("/proc/driver/storage_logger", "ENABLE 0");
      
      Xlog.d("MTKThermalManagerActivity", "turnOffThermalLoggerAndDumpLog() disable storage logger");
      
      // check for SD card, if not exists, dump to /data/
      //if(new File("/mnt/sdcard").exists())
      // system process cannot access sdcard, so we use /data for now...
      {
        long timems = System.currentTimeMillis();
        Date curtime = new Date();
        curtime.setTime(timems);
        SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd-kkmmssZ");
        String ctstr = sdf.format(curtime);
        ctstr = ctstr.replaceAll(" ", "");
        
        Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() test write to /data");
        writeFile("/data/storage_logger_dump_"+ctstr, "");
        try {
          appendFile(new File("/proc/driver/storage_logger_display"), new File("/data/storage_logger_dump_"+ctstr));
        }
        catch (IOException e)
        {
          Xlog.e("MTKThermalManagerActivity", "onCheckedChanged() append file exception!");
        }
        
        Toast.makeText(this, "Thermal Log storage_logger_dump_"+ctstr+" dumped to /data!", Toast.LENGTH_LONG).show();
      }
      //Expand storage logger buffer size back to 2MB...
      //WARNING! this might fail.
      writeFile("/proc/driver/storage_logger_bufsize_malloc", "2097152");
      /*
      else
      {
        Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() sdcard does not exist!");
        Toast.makeText(this, "No storage to dump thermal Log!\n Please use adb command: \n @adb pull /proc/driver/storage_logger_display storage_logger_dump  ", Toast.LENGTH_LONG).show();
      }
       */
  }
  
  private void turnOnThermalLogger()
  {
    Toast.makeText(this, "Starting Thermal Logger!", Toast.LENGTH_SHORT).show();
    //executeShellCommand("echo 0 0 0 0 > /proc/driver/storage_logger_config");
    writeFile("/proc/driver/storage_logger_config", "0 0 0 0");
    writeFile("/proc/driver/storage_logger_config", "0 0 1"); // enable proc name
    //executeShellCommand("echo 5 > /proc/driver/thermal_logger_config");
    writeFile("/proc/driver/thermal_logger_config", "5");
    //Expand storage logger buffer size to 10MB...
    //WARNING! this might fail.
    writeFile("/proc/driver/storage_logger_bufsize_malloc", "10485760");
    //executeShellCommand("echo ENABLE 1 > /proc/driver/storage_logger");
    writeFile("/proc/driver/storage_logger", "ENABLE 1");
    long timems = System.currentTimeMillis();
    Date curtime = new Date();
    curtime.setTime(timems);
    SimpleDateFormat sdf = new SimpleDateFormat("kkmmss");
    String ctstr = sdf.format(curtime);
    writeFile("/proc/driver/mtk_thermal_monitor", "1 "+ctstr);
    Toast.makeText(this, "Thermal Logger started!", Toast.LENGTH_SHORT).show();
  }
  
  private void writeFile(String filePath, String line)
  {
    File a = new File(filePath);
    if (a.exists())
    {
      try {
        FileOutputStream fs = new FileOutputStream(a);
        DataOutputStream ds = new DataOutputStream(fs);
        ds.write(line.getBytes());
        ds.flush();
        ds.close();
        fs.close();
      }
      catch (Exception ex) {
        Xlog.e("MTKThermalManagerActivity", "writeFile() Exception: " + filePath);
      }
    }
    else
    {
      Xlog.d("MTKThermalManagerActivity", "writeFile() File not exist: " + filePath);
      try {
        if (a.createNewFile())
        {
          Xlog.d("MTKThermalManagerActivity", "writeFile() File created: " + filePath);
          try {
            FileOutputStream fs = new FileOutputStream(a);
            DataOutputStream ds = new DataOutputStream(fs);
            ds.write(line.getBytes());
            ds.flush();
            ds.close();
            fs.close();
          }
          catch (Exception ex) {
            Xlog.e("MTKThermalManagerActivity", "writeFile() Exception: " + filePath);
          }
        }
        else
        {
          Xlog.d("MTKThermalManagerActivity", "writeFile() Create file fail: " + filePath);
        }
      }
      catch (IOException e)
      {
          Xlog.e("MTKThermalManagerActivity", "writeFile() creatFile Exception: " + filePath);
      }
    }
  }
  
  private void appendFile(File src, File dst) throws IOException 
  {
    FileInputStream in = new FileInputStream(src); 
    FileOutputStream out = new FileOutputStream(dst, true); // Transfer bytes from in to out // `true` means append
    byte[] buf = new byte[1024];
    int len;
    while ((len = in.read(buf)) > 0)
    {
      out.write(buf, 0, len);
    }
    in.close();
    out.close();
  }
  
  private boolean isThermalLoggerEnabled()
  {
      // read /proc/driver/thermal_logger_config
      File thermal_logger_config = new File("/proc/driver/thermal_logger_config");
      if (thermal_logger_config.exists())
      {
          boolean result = false;
          try {
              FileReader fr = new FileReader("/proc/driver/thermal_logger_config");
              BufferedReader br = new BufferedReader(fr);
              String line = null;
              while (null != (line = br.readLine()))
              {
                  if (line.contains("Enable logger"))
                  {
                      if (line.contains("= 0"))
                      {
                          br.close();
                          fr.close();
                          return false;
                      }
                      else
                      {
                          result = true;
                          continue;
                      }
                  }
                  
                  if (line.contains("(Bit3)= 0"))
                  {
                      br.close();
                      fr.close();
                      return false;
                  }
                  else if (line.contains("(Bit3)= 1") && (true == result))
                  {
                      br.close();
                      fr.close();
                      return true;
                  }
              }
              br.close();
              fr.close();
          }
          catch (Exception e)
          {
            Xlog.e("MTKThermalManagerActivity", "isThermalLoggerEnabled() IOException");
          }
          
          return false;
      }
      else
      {
          return false;
      }
  }
}
