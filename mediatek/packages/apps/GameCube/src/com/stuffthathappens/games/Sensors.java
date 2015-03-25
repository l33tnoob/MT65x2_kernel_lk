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

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.hardware.SensorManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;

/**
 * Shows a list of all Sensors on this phone.
 * 
 * @author Eric M. Burke
 */
public class Sensors extends Activity implements OnItemClickListener{
    private final static String TAG = "Game_sensors";
	
	private List<Sensor> mSensorList;
	private ListView mSensorView;
	private SensorManager mSensorManager;
	private Sensor mSensor[] = new Sensor[16];
	private final String mSensorName[] = new String[] {
			"Accelerometer",//0
			"Magnetometer",
			"Orientation",
			"Gyroscope",
			"Light",
			"Pressure",
			"Temperature",
			"Proximity", //7
			"Gravity",
			"LinearAcceleration",
			"RotationVector",//10
	};	
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sensors);
        
		mSensorView = (ListView) findViewById(R.id.sensor_list);
		mSensorView.setAdapter(new SensorListAdapter(this));
		mSensorView.setOnItemClickListener(this);
		mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		mSensorList = mSensorManager.getSensorList(Sensor.TYPE_ALL);
		
		for (int idx = 0; idx < mSensor.length; idx++)
			mSensor[idx] = null;
		Log.d(TAG, ":"+mSensorList.size());	
		for (Sensor sensor : mSensorList) {
			if (sensor.getType() >= 1)
				mSensor[sensor.getType()-1] = sensor;
			else
				Log.d(TAG, "unknown type: "+sensor.getType());
		}	
    }
    public void onStop() {
    	super.onStop();
    	mSensorView = null;
    	mSensorList = null;
    	mSensorManager = null;
    	for (int idx = 0; idx < mSensor.length; idx++)
    		mSensor[idx] = null;
    }
    
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
    	showDialog(position);
    }    
    
	@Override
	protected Dialog onCreateDialog(int id)
	{
		Builder builder;
		AlertDialog alertDlg;	
		Sensor sensor = mSensor[id];

		Log.d(TAG,"id = " + id+"\n");

		builder = new AlertDialog.Builder(Sensors.this);
		builder.setTitle(mSensorName[id]);
		builder.setAdapter(new InfoListAdapter(this, sensor, mSensorName[id]), new DialogInterface.OnClickListener() {				
			public void onClick(DialogInterface dialog, int whichButton) {				
			}
		});	
		builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {				
			public void onClick(DialogInterface dialog, int whichButton) {
				// TODO Auto-generated method stub
				//use to judge whether the click is correctly done!
			}
		});
		alertDlg = builder.create();
		return alertDlg;
	}
    
}
/*---------------------------------------------------------------------------*/
class SensorProperty {
	private final String propery;
	private final String value;
	
	public SensorProperty(String propery, String value) {
		this.propery = propery;
		this.value = value;
	}

	public String getProperty() {
		return propery;
	}

	public String getValue() {
		return value;
	}
	
}
/*---------------------------------------------------------------------------*/
class InfoListAdapter extends BaseAdapter {	
	private final LayoutInflater inflater;
	private final List<SensorProperty> mProperty = new ArrayList<SensorProperty>();

	
	public InfoListAdapter(Context context, Sensor sensor, String name) {
		// cache the inflater
		inflater = LayoutInflater.from(context);
		if (sensor == null) {
			mProperty.add(new SensorProperty("",name+" Not found"));
		} else {
			mProperty.add(new SensorProperty("MaxRange", Float.toString(sensor.getMaximumRange())));
			mProperty.add(new SensorProperty("Name", sensor.getName()));
			mProperty.add(new SensorProperty("Power", Float.toString(sensor.getPower())));
			mProperty.add(new SensorProperty("Resolution", Float.toString(sensor.getResolution())));
			mProperty.add(new SensorProperty("Type", name+"("+sensor.getType()+")"));
			mProperty.add(new SensorProperty("Vendor", sensor.getVendor()));
			mProperty.add(new SensorProperty("Version", Integer.toString(sensor.getVersion())));
		}

	}

	public int getCount() {
		return mProperty.size();
	}
	
	@Override
	public boolean hasStableIds() {
		return true;
	}

	public Object getItem(int position) {
		return mProperty.get(position);
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {		
		ViewHolder holder;
		
		if (convertView == null) {
			convertView = inflater.inflate(R.layout.info_item, null);
			
			holder = new ViewHolder();
			holder.property = (TextView) convertView.findViewById(R.id.property);
			holder.value = (TextView) convertView.findViewById(R.id.value);
			
			// store the holder for reuse later
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		SensorProperty sp = mProperty.get(position);
		holder.property.setText(sp.getProperty());
		holder.value.setText(sp.getValue());
		
		return convertView;
	}
	
	private static class ViewHolder {
		TextView value;
		TextView property;
	}

}