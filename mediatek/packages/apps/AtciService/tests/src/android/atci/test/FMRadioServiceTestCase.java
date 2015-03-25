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

package android.atci.test;

import android.test.AndroidTestCase;
import android.test.ServiceTestCase;
import android.content.Intent;
import android.os.IBinder;
import android.os.Handler;
import android.util.Log;
import android.content.ServiceConnection;
import android.content.Context;
import android.content.ComponentName;
import android.os.RemoteException;
import com.mediatek.atci.service.AtciFMRadioNative;
import com.mediatek.atci.service.IFMRadioService;
import  com.mediatek.atci.service.FMRadioService;

public class FMRadioServiceTestCase extends
             ServiceTestCase<FMRadioService> {
    
    public static final String LOG_TAG = "FMRadioServiceTestCase";
    
    public static final int DEFAULT_FREQUENCY = 1000;
    private int mCurrentStation = DEFAULT_FREQUENCY;
    public static final int SECOND_FREQUENCY = 1063;
    private static final int FREQUENCY_CONVERT_RATE = 10;
    public static final int LOWEST_STATION = 875;
    public static final int HIGHEST_STATION = 1080;
    
    private IFMRadioService mFMRadioService = null;
    
    public FMRadioServiceTestCase() {
        super(FMRadioService.class);
    }
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testcase01_Startable() {
        try {
        	  Log.d(LOG_TAG, "[FMRADIOSERVICE]testcase01_Startable: start service.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), FMRadioService.class);
            startService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void testcase02_Bindable() {
    	  IBinder mService = null;
        try {
        	  Log.d(LOG_TAG, "[FMRADIOSERVICE]testcase02_Bindable: bindService.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), FMRadioService.class);
            mService = bindService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }

        Log.d(LOG_TAG, "[FMRADIOSERVICE]testcase02_get mFMRadioService.");
        mFMRadioService = IFMRadioService.Stub.asInterface(mService);
        Log.d(LOG_TAG, "[FMRADIOSERVICE]testcase02_RunFMRadioService.");
        boolean flag = false;
        try {
    		    flag = mFMRadioService.isServiceInit();
        } catch (RemoteException e) {}
      	
            //if (!flag) {
        	      Log.d(LOG_TAG, "initService.");
        		    try {
        	          mFMRadioService.initService(mCurrentStation);
        	      } catch (RemoteException e) {}
        	  //}
        	
        	Log.d(LOG_TAG, "openDevice.");
        	try {
        		  //flag = mFMRadioService.openDevice();
        	} catch (Exception e) {
				Log.d(LOG_TAG, "openDevice exception.");
            }
        		
        	if(!flag) {
        		  Log.d(LOG_TAG, "openDevice fail.");
        	    return;	
        	}
        	
        	Log.d(LOG_TAG, "powerUp.");
       try {
        	mFMRadioService.powerUp((float) mCurrentStation / FREQUENCY_CONVERT_RATE);
        	flag = mFMRadioService.isPowerUp();
        	Log.d(LOG_TAG, "isPowerUp ["+flag+"]");
       } catch (RemoteException e) {}
       	
        	if (!flag) {
        		  Log.d(LOG_TAG, "powerUp fail.");
        	    try {
        	        mFMRadioService.setMute(true);
                  mFMRadioService.powerDown();
                  mFMRadioService.closeDevice();
              } catch (RemoteException e) {}
              
              return;
          }
        	
        	Log.d(LOG_TAG, "setMute & switchAntenna.");
        	int frequency = DEFAULT_FREQUENCY;    
        	try {
        	    mFMRadioService.setMute(false);
              mFMRadioService.switchAntenna(1);
          
              Log.d(LOG_TAG, "getFrequency.");   
        	    frequency = mFMRadioService.getFrequency();
          } catch (RemoteException e) {}
        
          if (SECOND_FREQUENCY != frequency) {
          	  Log.d(LOG_TAG, "tune.");
          	  try {
                  mFMRadioService.tune((float) SECOND_FREQUENCY / FREQUENCY_CONVERT_RATE);
              } catch (RemoteException e) {}
          }
          
          Log.d(LOG_TAG, "seek.");
          float fStation = 0;
          try {
              fStation = mFMRadioService.seek((float) frequency / FREQUENCY_CONVERT_RATE, true);
          } catch (RemoteException e) {}
          	
          int iStation = (int) (fStation * FREQUENCY_CONVERT_RATE);
          	
          if (iStation >= HIGHEST_STATION || iStation <= LOWEST_STATION) {
          	  try {
          	  Log.d(LOG_TAG, "seek abnormal.");
          	  mFMRadioService.setMute(true);
              mFMRadioService.powerDown();
              mFMRadioService.closeDevice();
            } catch (RemoteException e) {}
              return;	
          }

          Log.d(LOG_TAG, " success.");
    }
    
    public void testcase03_StopService() {
        try {
        	  Log.d(LOG_TAG, "testcase03_StopService.");
            Intent startIntent = new Intent();
            startService(startIntent);
            FMRadioService service = getService();
            service.stopService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
