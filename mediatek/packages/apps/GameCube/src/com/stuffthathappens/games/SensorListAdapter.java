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

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;
import android.util.Log;

import static android.hardware.SensorManager.*;

/**
 * Detects the list of sensors supported by the device. Adapts
 * this list for display in a ListView.
 * 
 * @author Eric M. Burke
 */
public class SensorListAdapter extends BaseAdapter {
	public static String TAG = "Game_List";
	private final LayoutInflater inflater;
	
	private final List<SensorInfo> sensors = new ArrayList<SensorInfo>();
	
	public SensorListAdapter(Context context) {
		// cache the inflater
		inflater = LayoutInflater.from(context);
		final int SENSOR_ID_ACCELEROMETER = (1 << Sensor.TYPE_ACCELEROMETER);
		final int SENSOR_ID_MAGNETIC_FIELD = (1 << Sensor.TYPE_MAGNETIC_FIELD);
		final int SENSOR_ID_ORIENTATION = (1 << Sensor.TYPE_ORIENTATION);
		final int SENSOR_ID_GYROSCOPE = (1 << Sensor.TYPE_GYROSCOPE);
		final int SENSOR_ID_LIGHT = (1 << Sensor.TYPE_LIGHT);
		final int SENSOR_ID_PRESSURE = (1 << Sensor.TYPE_PRESSURE);
		final int SENSOR_ID_TEMPERATURE = (1 << Sensor.TYPE_TEMPERATURE);
		final int SENSOR_ID_PROXIMITY = (1 << Sensor.TYPE_PROXIMITY);
		//add new sensors
		final int SENSOR_ID_GRAVITY = (1 << Sensor.TYPE_GRAVITY);
		final int SENSOR_ID_LINEARACC = (1 << Sensor.TYPE_LINEAR_ACCELERATION);
		final int SENSOR_ID_ROTATIONVEC = (1 << Sensor.TYPE_ROTATION_VECTOR);
		
		SensorManager sensorMgr = (SensorManager) 
				context.getSystemService(Context.SENSOR_SERVICE);
		int sensorIds = 0;
		
		for (Sensor sensor : sensorMgr.getSensorList(Sensor.TYPE_ALL)) 
			sensorIds |= (1 << sensor.getType());

		sensors.add(new SensorInfo("Accelerometer", (0 != (sensorIds & SENSOR_ID_ACCELEROMETER))));			
		sensors.add(new SensorInfo("Magnetic Field",(0 != (sensorIds & SENSOR_ID_MAGNETIC_FIELD))));
		sensors.add(new SensorInfo("Orientation", (0 != (sensorIds & SENSOR_ID_ORIENTATION))));
		sensors.add(new SensorInfo("GyroScope", (0 != (sensorIds & SENSOR_ID_GYROSCOPE))));
		sensors.add(new SensorInfo("Light",(0 != (sensorIds & SENSOR_ID_LIGHT))));
		sensors.add(new SensorInfo("Pressure",(0 != (sensorIds & SENSOR_ID_PRESSURE))));
		sensors.add(new SensorInfo("Temperature",(0 != (sensorIds & SENSOR_ID_TEMPERATURE))));
		sensors.add(new SensorInfo("Proximity",(0 != (sensorIds & SENSOR_ID_PROXIMITY))));
		//add new sensors
		sensors.add(new SensorInfo("Gravity",(0 != (sensorIds & SENSOR_ID_GRAVITY))));
		sensors.add(new SensorInfo("LinearAcceleration",(0 != (sensorIds & SENSOR_ID_LINEARACC))));
		sensors.add(new SensorInfo("RotationVector",(0 != (sensorIds & SENSOR_ID_ROTATIONVEC))));
	}

	public int getCount() {
		return sensors.size();
	}
	
	@Override
	public boolean hasStableIds() {
		return true;
	}

	public Object getItem(int position) {
		return sensors.get(position);
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {		
		ViewHolder holder;
		
		if (convertView == null) {
			convertView = inflater.inflate(R.layout.list_item_sensor, null);
			
			holder = new ViewHolder();
			holder.sensorName = (TextView) convertView.findViewById(R.id.sensor_name);
			holder.sensorSupported = (TextView) convertView.findViewById(R.id.sensor_supported);
			// store the holder for reuse later
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		
		SensorInfo si = sensors.get(position);
		holder.sensorName.setText(si.getName());
		holder.sensorSupported.setText(Boolean.toString(si.isSupported()));
		
		return convertView;
	}
	
	private static class ViewHolder {
		TextView sensorName;
		TextView sensorSupported;
	}

}
