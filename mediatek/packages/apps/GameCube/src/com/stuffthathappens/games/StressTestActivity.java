/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.stuffthathappens.games;

import static android.hardware.SensorManager.DATA_X;
import static android.hardware.SensorManager.DATA_Y;
import static android.hardware.SensorManager.DATA_Z;
import static android.hardware.SensorManager.SENSOR_DELAY_UI;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_HIGH;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_LOW;
import static android.hardware.SensorManager.SENSOR_STATUS_ACCURACY_MEDIUM;
import static android.hardware.SensorManager.SENSOR_STATUS_UNRELIABLE;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.util.Log;

import android.os.Message;
import android.os.SystemClock;
import java.util.Random;
import android.os.IPowerManager;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.os.PowerManager;
import android.content.Context;
import android.os.PowerManager.WakeLock;
import android.os.Handler;

/**
 * Displays values from the accelerometer sensor.
 * 
 * @author Eric M. Burke
 */
public class StressTestActivity extends Activity implements OnClickListener {
    private final static String TAG = "stress";
	SensorManager sensorManager = null;
    Sensor sensor = null;
    static TextView TextGSensor = null;
    static TextView TextLightSensor = null;
    static TextView TextProximity = null;
    static TextView TextMSensor = null;
    static TextView TextOrientation= null;
    static TextView TextSensorList = null;
    static int sensorIds=0;
	static Thread TestThread=null;
    static Thread GThread = null;
    
    static int SENSOR_ID_GSENSOR = (1 << Sensor.TYPE_ACCELEROMETER);
    static int SENSOR_ID_MSENSOR = (1 << Sensor.TYPE_MAGNETIC_FIELD);
    static int SENSOR_ID_ORIENTATION = (1 << Sensor.TYPE_ORIENTATION);
    static int SENSOR_ID_PROXIMITY = (1 << Sensor.TYPE_PROXIMITY);
    static int SENSOR_ID_LIGHT = (1 << Sensor.TYPE_LIGHT);
    
    Random random = new Random();
    static float f_proximity_v = 0 ;
    static float f_lv=0;
    static int stateOn =0;
    static float gx = 0 ;
    static float gy = 0 ;
    static float gz = 0 ;

    static float f_msensor_x =0;
    static float f_msensor_y =0;
    static float f_msensor_z =0;
    
    static float g_orientation_x=0;
    static float g_orientation_y=0;
    static float g_orientation_z=0;
    //handle ui
    private final static int UPDATE_Gsensor = 1;
    private final static int UPDATE_Msensor = 2;
    private final static int UPDATE_Lsensor = 3;
    private final static int UPDATE_Psensor = 4;
    private final static int UPDATE_Osensor = 5;
    
    //handle data ui
    private final static int UPDATE_GsensorData = 6;
    private final static int UPDATE_MsensorData = 7;
    private final static int UPDATE_LsensorData = 8;
    private final static int UPDATE_PsensorData = 9;
    private final static int UPDATE_OsensorData = 10;
    
    //rate record
    static int P_rate_choose =0 ;
    static int L_rate_choose =0 ;
    static int M_rate_choose =0 ;
    static int G_rate_choose =0 ;
    static int O_rate_choose =0 ;
    
    static boolean initFlag =false;
	private boolean mShowLog = false;

	static boolean bakThreadRun =false ;
	static boolean UIThreadRun =false ;
    
    
    
    static private Handler mHandler = new MainHandler();;
    
    static PowerManager pm = null;
    static IPowerManager ipm = null;
    static WakeLock GsensorWake = null;

	static private class MainHandler extends Handler {
      @Override
          public void handleMessage(Message msg) {
          switch (msg.what) 
          {
              case UPDATE_Gsensor:
            	   
            	   TextGSensor.setText("Gsensor close!");
                  break;
              case UPDATE_Msensor:
            	  TextMSensor.setText("Msensor close!");
            	  break;
              case UPDATE_Lsensor:
            	  TextLightSensor.setText("Lightsensor close!");
            	  break;
              case UPDATE_Psensor:
            	  TextProximity.setText("Proximity close!");
            	  break;
              case UPDATE_Osensor:
            	  TextOrientation.setText("Orientation close!");
            	  break;
             
              default:
                  break;
          }
      }
    }
    //end handle ui

	 //handler UI get data
   
   static private Handler mHandDataUI= new mHandlerrefreshData();
   
   static  private class mHandlerrefreshData extends Handler {
      @Override
          public void handleMessage(Message msg) {
          switch (msg.what) 
          {
              case UPDATE_GsensorData:
            	  switch(G_rate_choose)
                  {
                  case 0:
                	  TextGSensor.setText("Gsensor rate: SENSOR_DELAY_NORMAL \n "+"Accelerometer:\n x= " + gx + ",\ny=  " + gy + ",\nz=  " + gz);
                	  break;
                  case 1:
                	  TextGSensor.setText("Gsensor rate: SENSOR_DELAY_GAME \n "+"Accelerometer:\n x= " + gx + ",\ny=  " + gy + ",\nz=  " + gz);
                	  break;
                  case 2:
                	  TextGSensor.setText("Gsensor rate: SENSOR_DELAY_FASTEST \n "+"Accelerometer:\n x= " + gx + ",\ny=  " + gy + ",\nz=  " + gz);
                	  break;
                  case 3:
                	  TextGSensor.setText("Gsensor rate: SENSOR_DELAY_UI \n "+"Accelerometer:\n x= " + gx + ",\ny=  " + gy + ",\nz=  " + gz);
                	  break;
                	  
                  }
                  break;
              case UPDATE_MsensorData:
            	  switch(M_rate_choose)
                  {
                  case 0:
                	  TextMSensor.setText("Msensor rate: SENSOR_DELAY_NORMAL \n "+"Msensor:\n x= " + f_msensor_x + ",\ny=  " + f_msensor_y + ",\nz=  " + f_msensor_z);
                	  break;
                  case 1:
               	   TextMSensor.setText("Msensor rate: SENSOR_DELAY_GAME \n "+"Msensor:\n x= " + f_msensor_x + ",\ny=  " + f_msensor_y + ",\nz=  " + f_msensor_z);
                	  break;
                  case 2:
               	   TextMSensor.setText("Msensor rate: SENSOR_DELAY_FASTEST \n "+"Msensor:\n x= " + f_msensor_x + ",\ny=  " + f_msensor_y + ",\nz=  " + f_msensor_z);
                	  break;
                  case 3:
               	   TextMSensor.setText("Msensor rate: SENSOR_DELAY_UI \n "+"Orientation:\n x= " + "Msensor:\n x= " + f_msensor_x + ",\ny=  " + f_msensor_y + ",\nz=  " + f_msensor_z);
                	  break;
                	  
                  }
            	  break;
              case UPDATE_LsensorData:
            	  switch(L_rate_choose)
                  {
                  case 0:
                	  TextLightSensor.setText("Light rate: SENSOR_DELAY_NORMAL \n "+"LightSesnor:  = " + f_lv );
                	  break;
                  case 1:
                    TextLightSensor.setText("Light rate: SENSOR_DELAY_GAME \n "+"LightSesnor:  = " + f_lv );
                	  break;
                  case 2:
                  	TextLightSensor.setText("Light rate: SENSOR_DELAY_FASTEST \n "+"LightSesnor:  = " + f_lv );
                	  break;
                  case 3:
                  	TextLightSensor.setText("Light rate: SENSOR_DELAY_UI \n "+"LightSesnor:  = " + f_lv );
                	  break;
                	  
                  }
            	  break;
              case UPDATE_PsensorData:
            	  switch(P_rate_choose)
                  {
                  case 0:
                	  TextProximity.setText("Proximity rate: SENSOR_DELAY_NORMAL \n "+"Proximity: " + f_proximity_v+"\n");
                	  break;
                  case 1:
                	  TextProximity.setText("Proximity rate: SENSOR_DELAY_GAME \n "+"Proximity: " + f_proximity_v+"\n");
                	  break;
                  case 2:
                	  TextProximity.setText("Proximity rate: SENSOR_DELAY_FASTEST \n "+"Proximity: " + f_proximity_v+"\n");
                	  break;
                  case 3:
                	  TextProximity.setText("Proximity rate: SENSOR_DELAY_UI \n "+"Proximity: " + f_proximity_v+"\n");
                	  break;
                	  
                  }
            	  break;
              case UPDATE_OsensorData:
            	  switch(O_rate_choose)
                  {
                  case 0:
                	  TextOrientation.setText("Orientation rate: SENSOR_DELAY_NORMAL \n "+"Orientation:\n x= " + g_orientation_x + ",\ny=  " + g_orientation_y + ",\nz=  " + g_orientation_z);
                	  break;
                  case 1:
                  	TextOrientation.setText("Orientation rate: SENSOR_DELAY_GAME \n "+"Orientation:\n x= " + g_orientation_x + ",\ny=  " + g_orientation_y + ",\nz=  " + g_orientation_z);
                	  break;
                  case 2:
                  	TextOrientation.setText("Orientation rate: SENSOR_DELAY_FASTEST \n "+"Orientation:\n x= " + g_orientation_x + ",\ny=  " + g_orientation_y + ",\nz=  " + g_orientation_z);
                	  break;
                  case 3:
                  	TextOrientation.setText("Orientation rate: SENSOR_DELAY_UI \n "+"Orientation:\n x= " + g_orientation_x + ",\ny=  " + g_orientation_y + ",\nz=  " + g_orientation_z);
                	  break;
                	  
                  }
            	  break;
             
              default:
                  break;
          }
      }
    }
     //Gsensor listener
    SensorEventListener Gsensor_listener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent e) {
        	 
          gx = e.values[SensorManager.DATA_X];
          gy = e.values[SensorManager.DATA_Y];
          gz = e.values[SensorManager.DATA_Z];
          
          //TextGSensor.setText("Accelerometer:\n x= " + gx + ",\ny=  " + gy + ",\nz=  " + gz);
        	
        }
        public void onAccuracyChanged(Sensor s, int accuracy) {
        }
    };
    //
  //LightSensor listener
    SensorEventListener LightSensor_listener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent e) {
        	 
        	f_lv = e.values[SensorManager.DATA_X];
            //TextLightSensor.setText("LightSesnor:  = " + f_lv );
            
        	
        }
        public void onAccuracyChanged(Sensor s, int accuracy) {
        }
    };
    //Proximity sensor
    SensorEventListener Proximity_listener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent e) {
        	 
          f_proximity_v = e.values[SensorManager.DATA_X];
         // TextProximity.setText("Proximity: " + f_proximity_v+"\n");
          
          
        	
        }
        public void onAccuracyChanged(Sensor s, int accuracy) {
        }
    };
  //Orientation listener
    SensorEventListener Orientation_listener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent e) {
        	 
        	g_orientation_x = e.values[SensorManager.DATA_X];
        	g_orientation_y = e.values[SensorManager.DATA_Y];
        	g_orientation_z = e.values[SensorManager.DATA_Z];
        	//TextOrientation.setText("Orientation:\n x= " + g_orientation_x + ",\ny=  " + g_orientation_y + ",\nz=  " + g_orientation_z);
        	
        }
        public void onAccuracyChanged(Sensor s, int accuracy) {
        }
    };
    //MSensor listener
    SensorEventListener MsensorSensor_listener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent e) {
        	 
           f_msensor_x = e.values[SensorManager.DATA_X];
           f_msensor_y = e.values[SensorManager.DATA_Y];
           f_msensor_z = e.values[SensorManager.DATA_Z];
           
          // TextMSensor.setText("Msensor:\n x= " + f_msensor_x + ",\ny=  " + f_msensor_y + ",\nz=  " + f_msensor_z);	
        }
        public void onAccuracyChanged(Sensor s, int accuracy) {
        }
    };
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);   
		setContentView(R.layout.stress);

		//set text view
        TextGSensor = (TextView) findViewById(R.id.Gsensor_prompt);
        TextMSensor = (TextView) findViewById(R.id.Msensor_prompt);
        TextProximity = (TextView) findViewById(R.id.Proximity_prompt);
        TextLightSensor = (TextView) findViewById(R.id.LightSensor_prompt);
        TextOrientation = (TextView) findViewById(R.id.Orientation_prompt);
        // get sensor service
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        //
        TextSensorList  = (TextView) findViewById(R.id.SensorList_prompt);
        // get Manger
        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        //get sensor list
        for(Sensor sensor_list : sensorManager.getSensorList(Sensor.TYPE_ALL))
        {
        	sensorIds |= (1 << sensor_list.getType());
        	
        }
        
        //TextSensorList.setText("SensorList: "+sensorIds+"\n");
        //add power
        pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
        ipm = IPowerManager.Stub.asInterface(ServiceManager.getService(POWER_SERVICE));
        GsensorWake = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Gsensor_wake");
        
       
    }

	//
    // register sensors
    //
    int gRegisterF_Proximity =0;
    int gRegisterF_Light =0;
    int gRegisterF_Gsensor =0;
    int gRegisterF_Msensor =0;
    int gRegisterF_Orientation=0;
    
    private void registerProximitySensor() {
		// TODO Auto-generated method stub
    	
    	
    	gRegisterF_Proximity=1;
    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
    	
    	P_rate_choose = Math.abs(random.nextInt());
    	P_rate_choose = P_rate_choose%4;
    	switch(P_rate_choose)
    	{
    	case 0:
    		sensorManager.registerListener(Proximity_listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    		break;
    	case 1:
    		sensorManager.registerListener(Proximity_listener, sensor, SensorManager.SENSOR_DELAY_GAME);
    		break;
    	case 2:
    		sensorManager.registerListener(Proximity_listener, sensor, SensorManager.SENSOR_DELAY_FASTEST);
    		break;
    	case 3:
    		sensorManager.registerListener(Proximity_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    		break;
    	default:
    			break;
    	}
        
	}
    private void registerGsensor() {
		// TODO Auto-generated method stub
    	
    	gRegisterF_Gsensor = 1;
    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
    	
    	G_rate_choose = Math.abs(random.nextInt());
    	G_rate_choose = G_rate_choose%4;
    	switch(G_rate_choose)
    	{
    	case 0:
    		sensorManager.registerListener(Gsensor_listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    		break;
    	case 1:
    		sensorManager.registerListener(Gsensor_listener, sensor, SensorManager.SENSOR_DELAY_GAME);
    		break;
    	case 2:
    		sensorManager.registerListener(Gsensor_listener, sensor, SensorManager.SENSOR_DELAY_FASTEST);
    		break;
    	case 3:
    		sensorManager.registerListener(Gsensor_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    		break;
    	default:
    			break;
    	}
        
	}
    private void registerLightSensor()
    {
    	
    	gRegisterF_Light =1;
    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
    	
    	L_rate_choose = Math.abs(random.nextInt());
    	L_rate_choose = L_rate_choose%4;
    	switch(L_rate_choose)
    	{
    	case 0:
    		sensorManager.registerListener(LightSensor_listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    		break;
    	case 1:
    		sensorManager.registerListener(LightSensor_listener, sensor, SensorManager.SENSOR_DELAY_GAME);
    		break;
    	case 2:
    		sensorManager.registerListener(LightSensor_listener, sensor, SensorManager.SENSOR_DELAY_FASTEST);
    		break;
    	case 3:
    		sensorManager.registerListener(LightSensor_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    		break;
    	default:
    			break;
    	}
    	
    }
    private void registerOrientationSensor() {
		// TODO Auto-generated method stub
    	
    	gRegisterF_Orientation = 1;
    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
    	
    	O_rate_choose = Math.abs(random.nextInt());
    	O_rate_choose = O_rate_choose%4;
    	switch(L_rate_choose)
    	{
    	case 0:
    		sensorManager.registerListener(Orientation_listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    		break;
    	case 1:
    		sensorManager.registerListener(Orientation_listener, sensor, SensorManager.SENSOR_DELAY_GAME);
    		break;
    	case 2:
    		sensorManager.registerListener(Orientation_listener, sensor, SensorManager.SENSOR_DELAY_FASTEST);
    		break;
    	case 3:
    		sensorManager.registerListener(Orientation_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    		break;
    	default:
    			break;
    	}
        //sensorManager.registerListener(Orientation_listener, sensor, SensorManager.SENSOR_DELAY_UI);
	}
    private void registerMSensor()
    {
    	
    	gRegisterF_Msensor = 1;
    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
    	
    	M_rate_choose = Math.abs(random.nextInt());
    	M_rate_choose = M_rate_choose%4;
    	switch(M_rate_choose)
    	{
    	case 0:
    		sensorManager.registerListener(MsensorSensor_listener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
    		break;
    	case 1:
    		sensorManager.registerListener(MsensorSensor_listener, sensor, SensorManager.SENSOR_DELAY_GAME);
    		break;
    	case 2:
    		sensorManager.registerListener(MsensorSensor_listener, sensor, SensorManager.SENSOR_DELAY_FASTEST);
    		break;
    	case 3:
    		sensorManager.registerListener(MsensorSensor_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    		break;
    	default:
    			break;
    	}
    	//sensorManager.registerListener(MsensorSensor_listener, sensor, SensorManager.SENSOR_DELAY_UI);
    }
    //
    // unregister sensor
    //
   public void unregisterGsensor() 
    {
    	gRegisterF_Gsensor =0;
        sensorManager.unregisterListener(Gsensor_listener);
        

    }
    void unregisterLightSensor() 
    {
    	gRegisterF_Light =0;
        sensorManager.unregisterListener(LightSensor_listener);
        
    }
    void unregisterProximitySensor() 
    {
    	gRegisterF_Proximity = 0;
        sensorManager.unregisterListener(Proximity_listener);
        
    }
    void unregisterOrientationSensor() 
    {
    	gRegisterF_Orientation = 0;
        sensorManager.unregisterListener(Orientation_listener);
        
    }
    void unregisterMSensor() 
    {
    	gRegisterF_Msensor  = 0;
        sensorManager.unregisterListener(MsensorSensor_listener);
       
    }
    
    Runnable doBackgroundThreadProcessing = new Runnable() 
    { 
         public void run() 
         {    
        	 int rand =0; 
        	 int choose =0;
        	 
  
        	 while(bakThreadRun)
       	     {
       	         Log.d(TAG,"bakThreadRunning .... \n");
        		 rand = Math.abs(random.nextInt());
				 Log.d(TAG,"rand= " + rand+"\n");
        		 choose =rand%12;
				 Log.d(TAG,"choose= " + choose+"\n");
        		 
        		 switch(choose)
        		 {
        		 //gsensor
        		 case 0:
        			 if(1 == gRegisterF_Gsensor)
        			 {
        				 unregisterGsensor();
        				 mHandler.sendEmptyMessageDelayed(UPDATE_Gsensor, 0);

        			 }
        			 break;
        		 case 1:
        			 if(0 == gRegisterF_Gsensor)
        			 {
        				 if((sensorIds&SENSOR_ID_GSENSOR)!= 0)
        				 {
        					 registerGsensor();
        				 }
        			 }
        			 break;
        			 
        			 //proximtiy
        		 case 2:
        			 if(0== gRegisterF_Proximity)
        			 {
        				 if((sensorIds&SENSOR_ID_PROXIMITY)!= 0)
        				 {
        				    registerProximitySensor();
        				 }
        			 }
        			 break;
        		 case 3:
        			 if(1 == gRegisterF_Proximity)
        			 {
        				 unregisterProximitySensor();
        				 mHandler.sendEmptyMessageDelayed(UPDATE_Psensor, 0);
        			 }
        			 break;
        			 
        			 //light sensor
        		 case 4:
        			 if(0== gRegisterF_Light)
        			 {
        				 if((sensorIds&SENSOR_ID_LIGHT)!= 0)
        				 {
        				   registerLightSensor();
        				 }
        			 }
        			 break;
        		 case 5:
        			 if(1 == gRegisterF_Light)
        			 {
        				 unregisterLightSensor();
        				 mHandler.sendEmptyMessageDelayed(UPDATE_Lsensor, 0);
        			 }
        			 
        			 //Msensor
        		 case 6:
        			 if(0== gRegisterF_Msensor)
        			 {
        				 if((sensorIds&SENSOR_ID_MSENSOR)!= 0)
        				 {
        				    registerMSensor();
        				 }
        			 }
        			 break;
        		 case 7:
        			 if(1 == gRegisterF_Msensor)
        			 {
        				 unregisterMSensor();
        				 mHandler.sendEmptyMessageDelayed(UPDATE_Msensor, 0);
        			 }
        			 
        			 
        			 //Orientation
        		 case 8:
        			 if(0== gRegisterF_Orientation)
        			 {
        				 if((sensorIds&SENSOR_ID_ORIENTATION)!= 0)
        				 {
        				    registerOrientationSensor();
        				 }
        			 }
        			 break;
        		 case 9:
        			 if(1 == gRegisterF_Orientation)
        			 {
        				 unregisterOrientationSensor();
        				 mHandler.sendEmptyMessageDelayed(UPDATE_Osensor, 0);
        			 }	 
        			 break;
        		 case 10:
        			 
        			 //early suspend
        			 try
        		     {
        			   pm.goToSleep(SystemClock.uptimeMillis());
        		     }
        		     catch(SecurityException e)
        		     {
        		       Log.d(TAG,"GsensorTest exception \r\n");
        			 
        		     }
        			 stateOn = 0;
        			 try
        		     {
        		       GsensorWake.acquire();
        		       SystemClock.sleep(3000);
        		       GsensorWake.release();
        		     }catch(SecurityException e)
        		     {
        			 
				Log.d(TAG,"wake lock exception \r\n");
        		     }
        			 //late resume
        			 try
        		     {
				pm.wakeUp(SystemClock.uptimeMillis());
        		     }
        		     catch(Exception e)
        		     {
        		       Log.d(TAG,"userActivity exception  \r\n");
        		     }
        			 stateOn = 1;
        			 
        			 break;
        		 case 11:
        			 
        			 break;
        			 
        		 default:
        			 //log("swithc  default !!!!\n");
        			break;
        		 }
				 
        		 SystemClock.sleep(5000);
       	     }
   	    
         }
    };

	Runnable updataUIThread = new Runnable() 
    { 
    	public void run() 
    	{
    		while(UIThreadRun)
    		{
    			//log("mHandDataUI = "+ mHandDataUI+"\r\n");
    			if(1 == gRegisterF_Gsensor)
    			{mHandDataUI.sendEmptyMessageDelayed(UPDATE_GsensorData, 0);}
    			if(1 == gRegisterF_Msensor)
    			{mHandDataUI.sendEmptyMessageDelayed(UPDATE_MsensorData, 0);}
    			if(1 == gRegisterF_Orientation)
    			{mHandDataUI.sendEmptyMessageDelayed(UPDATE_OsensorData, 0);}
    			if(1 == gRegisterF_Light)
    			{mHandDataUI.sendEmptyMessageDelayed(UPDATE_LsensorData, 0);}
    			if(1 == gRegisterF_Proximity)
    			{mHandDataUI.sendEmptyMessageDelayed(UPDATE_PsensorData, 0);}
        		SystemClock.sleep(1000);
    		}
    		
    	}
    };

	@Override
	protected void onPause() {
		super.onPause();
		
		//sensorMgr.unregisterListener(mSensorEventListener);
		//sensorMgr = null;
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		Log.d(TAG,"destroy ,only destroy once \r\n" );
		
		//bakThreadRun =false ;
	    //UIThreadRun =false ;
		//initFlag = false;
		
		//unregisterGsensor();
		//unregisterLightSensor();
		//unregisterProximitySensor();
		//unregisterOrientationSensor();
		//unregisterMSensor();
	}

	void init()
    {
    	//View container = findViewById(R.id.Gsensor_container);
    	bakThreadRun =true ;
	    UIThreadRun =true ;
        Log.d(TAG,"init ,only init once \r\n" );
		if(null == TestThread)
    	{
    	    Log.d(TAG,"creat background thread \r\n" );
    		TestThread = new Thread(doBackgroundThreadProcessing);
        	TestThread.start();
			
    	}
    	if(null == GThread)
    	{
    	    Log.d(TAG,"creat updataUI thread \r\n" );
    		GThread = new Thread(updataUIThread);
       	    GThread.start();
    	}
    	
    	//final Thread thread = new Thread(doBackgroundThreadProcessing);
   	  //  thread.start();
   	    
   	   // final Thread thr = new Thread(updataUIThread);
   	   // thr.start();
   	    
   	    
   	 
   	   // final Thread Gsensorthread = new Thread(GsensorThread);
   	    //Gsensorthread.start();
    }

	@Override
	protected void onResume() {
		super.onResume();
		
          init();
        
		
		Log.d(TAG,"onResume\r\n" );		
	}

	public void onClick(View v) {
		/*
		if (v == calibrateButton) {
			cx = -x;
			cy = -y;
			cz = -z;
		}
		*/
	}
	
	@Override 
	public boolean onCreateOptionsMenu(Menu menu) { 
		boolean supRetVal = super.onCreateOptionsMenu(menu);
		if (mShowLog)
			menu.add(0, 0, 0, getString(R.string.hide_log));
		else
			menu.add(0, 0, 0, getString(R.string.show_log));
		return supRetVal; 
	} 

	@Override 
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) { 
		case 0: 
			if (mShowLog) {
				mShowLog = false;
				item.setTitle(R.string.show_log);
			} else {
				mShowLog = true;
				item.setTitle(R.string.hide_log);
			}
			return true;			
		}
		return false; 
	}	

}
