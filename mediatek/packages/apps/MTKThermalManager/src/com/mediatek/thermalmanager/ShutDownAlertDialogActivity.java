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
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;

public class ShutDownAlertDialogActivity extends Activity {

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if (true == isThermalLoggerEnabled())
        {
            turnOffThermalLoggerAndDumpLog();
        }
        
        Intent it = new Intent(ShutDownAlertDialogActivity.this, ShutDownAlarm.class);
        PendingIntent sender = PendingIntent.getBroadcast(ShutDownAlertDialogActivity.this,
                0, it, 0);

        // We want the alarm to go off 30 seconds from now.
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis());
        calendar.add(Calendar.SECOND, 30);

        // Schedule the alarm!
        AlarmManager am = (AlarmManager)getSystemService(ALARM_SERVICE);
        // After KK use precise alarm
        am.setExact(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), sender);
        // Before KK
        am.set(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), sender);
        
          
        
        /* create ui */
        AlertDialog dialog = new AlertDialog.Builder(ShutDownAlertDialogActivity.this)
        .setIconAttribute(android.R.attr.alertDialogIcon)
        .setTitle(R.string.alert_dialog_two_buttons_title)
        .setMessage(R.string.alert_dialog_two_buttons_message)
        .setPositiveButton(R.string.alert_dialog_ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                /* User clicked OK so do some stuff */
                Intent intent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
                intent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                getApplicationContext().startActivity(intent);
            }
        })
        .setNegativeButton(R.string.alert_dialog_cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                //finish();
                /* User clicked Cancel so do some stuff */
                // Try not to do anything...
            }
        })
        .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();	
        // TODO Auto-generated method stub
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
        /*
        else
        {
          Xlog.d("MTKThermalManagerActivity", "onCheckedChanged() sdcard does not exist!");
          Toast.makeText(this, "No storage to dump thermal Log!\n Please use adb command: \n @adb pull /proc/driver/storage_logger_display storage_logger_dump  ", Toast.LENGTH_LONG).show();
        }
         */
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
}
