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

/**
 * Displays values from the accelerometer sensor.
 * 
 * @author Eric M. Burke
 */
public class PressRawActivity extends Activity{
	private final static String TAG = "Game";
	private SensorManager sensorMgr;
	private TextView accuracyLabel;
	private TextView tempLabel, pressLabel;
	private TextView change_label;
	private TextView altChange_label;
	private TextView altitude_label;
	private Button calibrateButton;
	private Sensor mPressSensor;
	private Sensor mTempSensor;
	private long lastUpdate = -1;
	private boolean mShowLog = false;
	private long curTime;
	private long oldTime;
	private int timeflag=0;
    float   maxPress=0;
	float   minPress=1200;
	float   maxAltitude =-10000;
	float   minAltitude =10000;
	float[]  changeScale;
	float[]  changeAlt;
	int count =0;
	int sampleNum =3;
	float sumChangeScale=0;
	float sumAltChange =0;
	float altitude=0;

    

	//-----------------------------------------------------------------------------------------
//! @brief Converts the pressure to altitude
//!
//! @return altitude value
double pressure2altitude(double p)
{
    double i, j, h;

    if (p<700.0)              				// **** [300:700[    mbar **** 
    {
        if (p<499.0)          				// **** [300:499[    mbar **** 
        {
            if (p<400.5)      				// **** [300:400.5[  mbar **** 
            {
                if (p<349.0)  				// **** [300:349[    mbar ****
                {
                    i = 21.0;
                    j = 15458.0;
                }
                else                		// **** [349:400.5[  mbar **** 
                {
                    i = 18.6;
                    j = 14620.0;
                }
            }
            else                    		// **** [400.5:499[  mbar **** 
            {
                if (p<448.5)  				// **** [400.5:448.5[ mbar **** 
                {
                    i = 16.8;
                    j = 13899.0;
                }
                else                		// **** [448.5:499[  mbar **** 
                {
                    i = 15.4;
                    j = 13271.0;
                }
            }
        }
        else                        		// **** [499:700[    mbar **** 
        {
            if (p<599.0)      				// **** [499:599[    mbar **** 
            {
                if (p<549.0)  				// **** [499:549[    mbar **** 
                {
                    i = 14.2;
                    j = 12672.0;
                }
                else                		// **** [549:599[    mbar **** 
                {
                    i = 13.2;
                    j = 12123.0;
                }
            }
            else                    		// **** [599:700[    mbar **** 
            {
                if (p<648.5)  				// **** [599:648.5[  mbar **** 
                {
                    i = 12.3;
                    j = 11584.0;
                }
                else                		// **** [648.5:700[  mbar **** 
                {
                    i = 11.6;
                    j = 11130.0;
                }
            }
        }
    }
    else                            		// **** [700:1100[   mbar **** 
    {
        if (p<897.5)          				// **** [700:897.5[  mbar **** 
        {
            if (p<798.0)      				// **** [700:798[    mbar ****
            {
                if (p<744.0)  				// **** [700:744[    mbar **** 
                {
                    i = 10.9;
                    j = 10640.0;
                }
                else                		// **** [744:798[    mbar **** 
                {
                    i = 10.4;
                    j = 10266.0;
                }
            }
            else                    		// **** [798:897.5[  mbar **** 
            {
                if (p<850.0)  				// **** [798:850[    mbar **** 
                {
                    i = 9.8;
                    j = 9787.0;
                }
                else                		// **** [850:897.5[  mbar **** 
                {
                    i = 9.4;
                    j = 9447.0;
                }
            }
        }
        else                        		// **** [897.5:1100[ mbar **** 
        {
            if (p<1006.0)      				// **** [897.5:1006[ mbar **** 
            {
                if (p<945.0)  				// **** [897.5:945[  mbar **** 
                {
                    i = 9.0;
                    j = 9088.0;
                }
                else                		// **** [945:1006[   mbar **** 
                {
                    i = 8.6;
                    j = 8710.0;
                }
            }
            else                    		// **** [1006:1100[  mbar **** 
            {
                i = 8.1;
                j = 8207.0;
            }

        }
    }

    h = j - p * i;
    return(h);
}

	
	public void calcSensitivity(float pressValue , float h)
	{
	   float value =0;
	   String buffer;
	  // Log.d(TAG, "pressValue: "+ pressValue +"\n");
	   if(0 == sampleNum)
	   {
	      value = sumChangeScale/(count-1);
		  buffer = "avgChangeScale: " + value ;
	      change_label.setText(buffer);
		  altChange_label.setText("altchangeScale=" + sumAltChange/(count-1)+"\n" );
	      return;
	   }
	   curTime = System.currentTimeMillis();
	   if(0==timeflag)
	   {
		  oldTime = curTime;
		  timeflag =1;
	   }
	   maxPress = maxPress < pressValue? pressValue:maxPress;
	   minPress = minPress > pressValue? pressValue:minPress;

	   maxAltitude= maxAltitude < h? h:maxAltitude;
	   minAltitude= minAltitude > h? h:minAltitude;
	   
	   //Log.d(TAG, "maxPress="+ maxPress + " minPress=" + minPress +"\n");
	   if((curTime - oldTime)>60000)//every 5s caculate changeScale
	   {
	      Log.d(TAG, "maxPress="+ maxPress + " minPress=" + minPress +"\n");
		  Log.d(TAG, "max-min="+ (maxPress-minPress) + "\n");
		  
		  Log.d(TAG, "maxAlt="+ maxAltitude+ " minAlt=" + minAltitude+"\n");
		  Log.d(TAG, "maxAlt-minAlt="+ (maxAltitude-minAltitude) + "\n");
	      changeScale[count]= maxPress-minPress;
		  changeAlt[count]= maxAltitude-minAltitude;
		  Log.d(TAG, "changeScale "+ count + "=" + changeScale[count]+"\n");
		  Log.d(TAG, "changeAlt "+ count + "=" + changeAlt[count]+"\n");
		  if(count!=0)
		  {
		    // do not use fist 5s value
		    sumChangeScale +=changeScale[count];
			sumAltChange += changeAlt[count];
		  }

		  count++;
		  sampleNum--;
		  
		  
		  //re calcualte next 5s 
		  timeflag =0;
		  maxPress =0;
		  minPress=1200;
		  maxAltitude =-10000;
	      minAltitude =10000;
		  
	   }
	}
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);        
        setContentView(R.layout.alsps);
        accuracyLabel = (TextView) findViewById(R.id.accuracy_label);
        tempLabel = (TextView) findViewById(R.id.als_label);
        pressLabel = (TextView) findViewById(R.id.ps_label); 
		change_label = (TextView) findViewById(R.id.change_label);
		altitude_label = (TextView) findViewById(R.id.altitude_label);
		altChange_label = (TextView) findViewById(R.id.altChange_label);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);	
		changeScale = new float[100];
		changeAlt = new float[100];
    }

	@Override
	protected void onPause() {
		super.onPause();		
		sensorMgr.unregisterListener(mSensorEventListener);
		sensorMgr = null;
	}

	@Override
	protected void onResume() {
		super.onResume();
		
		sensorMgr = (SensorManager) getSystemService(SENSOR_SERVICE);
		mPressSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_PRESSURE);
		mTempSensor = sensorMgr.getDefaultSensor(Sensor.TYPE_TEMPERATURE);
		boolean tempSupported = sensorMgr.registerListener(mSensorEventListener, 
				mTempSensor, SENSOR_DELAY_UI);
		boolean pressSupported = sensorMgr.registerListener(mSensorEventListener, 
				mPressSensor, SENSOR_DELAY_UI);
		
		if (!tempSupported || !pressSupported) {
			// on accelerometer on this device
			//sensorMgr.unregisterListener(mSensorEventListener);
			
			if (!tempSupported && pressSupported)
				accuracyLabel.setText("no temperature");
			else if (tempSupported && !pressSupported)
				accuracyLabel.setText("no press");
			else 
				accuracyLabel.setText("no press and temperature");
		}
		Log.d(TAG,"tempSupported: " + tempSupported + "; pressSupported: "+pressSupported);		
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

	public final SensorEventListener mSensorEventListener = new SensorEventListener() {
		// from the android.hardware.SensorListener interface
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
			// this method is called very rarely, so we don't have to
			// limit our updates as we do in onSensorChanged(...)
			//Log.d(TAG, "onAccuracyChanged("+sensor+","+accuracy+")");
			int type = sensor.getType();
			if (type == Sensor.TYPE_PRESSURE|| type == Sensor.TYPE_TEMPERATURE) {
				switch (accuracy) {
				case SENSOR_STATUS_UNRELIABLE:
					accuracyLabel.setText(R.string.accuracy_unreliable);
					break;
				case SENSOR_STATUS_ACCURACY_LOW:
					accuracyLabel.setText(R.string.accuracy_low);
					break;
				case SENSOR_STATUS_ACCURACY_MEDIUM:
					accuracyLabel.setText(R.string.accuracy_medium);
					break;
				case SENSOR_STATUS_ACCURACY_HIGH:
					accuracyLabel.setText(R.string.accuracy_high);
					break;
				}
			}
		}
		


		// from the android.hardware.SensorEventListener interface
		private String mBuffer;
		public void onSensorChanged(SensorEvent event) {
			
			
			int type = event.sensor.getType();
			if (type == Sensor.TYPE_PRESSURE) {

				if (mShowLog)
				{
					Log.d(TAG, "("+type+")" + event.values[0]);
				}
				
				mBuffer = "press:  " + event.values[0];
				pressLabel.setText(mBuffer);
				//display altitude
				altitude = (float)pressure2altitude((double)event.values[0]); 
				altitude_label.setText("altitude=" + altitude + "\n");
				
				calcSensitivity(event.values[0],altitude);
				mBuffer = null;
			} else if (type == Sensor.TYPE_TEMPERATURE) {
				if (mShowLog)
				{
					Log.d(TAG, "("+type+")" + event.values[0]);
				}
				mBuffer = "Temp: " + event.values[0];
				tempLabel.setText(mBuffer);
				mBuffer = null;				
			}
		}
	};
}