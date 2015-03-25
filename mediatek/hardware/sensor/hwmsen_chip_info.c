/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "hwmsen_chip_info.h"
#include "hwmsen_custom.h"


/*--------------------------------------------------------*/
#ifdef CUSTOM_KERNEL_ACCELEROMETER
	#ifndef ACCELEROMETER
		#define ACCELEROMETER 			"ACCELEROMETER"
		#define ACCELEROMETER_VENDER 		"MTK"
	#endif
	#ifndef ACCELEROMETER_RANGE
		#define ACCELEROMETER_RANGE		32.0f
	#endif
	#ifndef ACCELEROMETER_RESOLUTION
		#define ACCELEROMETER_RESOLUTION	4.0f/1024.0f
	#endif
	#ifndef ACCELEROMETER_POWER
		#define ACCELEROMETER_POWER		130.0f/1000.0f
	#endif
#endif

#ifdef CUSTOM_KERNEL_ALSPS
	#ifndef PROXIMITY
		#define PROXIMITY 			"PROXIMITY"
		#define PROXIMITY_VENDER 		"MTK"
	#endif
	#ifndef PROXIMITY_RANGE
		#define PROXIMITY_RANGE 		1.00f
	#endif
	#ifndef PROXIMITY_RESOLUTION
		#define PROXIMITY_RESOLUTION  		1.0f	
	#endif
	#ifndef PROXIMITY_POWER			
		#define PROXIMITY_POWER			0.13f
	#endif

	#ifndef LIGHT
		#define LIGHT 				"LIGHT"
		#define LIGHT_VENDER 			"MTK"
	#endif
	#ifndef LIGHT_RANGE
		#define LIGHT_RANGE			10240.0f
	#endif
	#ifndef LIGHT_RESOLUTION
		#define LIGHT_RESOLUTION 		1.0f	
	#endif
	#ifndef LIGHT_POWER
		#define LIGHT_POWER			0.13f		
	#endif
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
	#ifndef MAGNETOMETER
		#define MAGNETOMETER 			"MAGNETOMETER"
		#define MAGNETOMETER_VENDER 		"MTK"
	#endif
	#ifndef MAGNETOMETER_RANGE
		#define MAGNETOMETER_RANGE 		600.0f
	#endif
	#ifndef MAGNETOMETER_RESOLUTION
		#define MAGNETOMETER_RESOLUTION		0.0016667f
	#endif
	#ifndef MAGNETOMETER_POWER
		#define MAGNETOMETER_POWER		0.25f
	#endif

	#ifndef ORIENTATION
		#define ORIENTATION 			"ORIENTATION"
		#define ORIENTATION_VENDER 		"MTK"
	#endif
	#ifndef ORIENTATION_RANGE
		#define ORIENTATION_RANGE		360.0f
	#endif
	#ifndef ORIENTATION_RESOLUTION
		#define ORIENTATION_RESOLUTION		1.0f
	#endif
	#ifndef ORIENTATION_POWER
		#define ORIENTATION_POWER		0.25f
	#endif
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
	#ifndef GYROSCOPE
		#define GYROSCOPE 			"GYROSCOPE"
		#define GYROSCOPE_VENDER 		"MTK"
	#endif
	#ifndef GYROSCOPE_RANGE
		#define GYROSCOPE_RANGE			34.91f
	#endif
	#ifndef GYROSCOPE_RESOLUTION
		#define GYROSCOPE_RESOLUTION		0.0107f
	#endif
	#ifndef GYROSCOPE_POWER
		#define GYROSCOPE_POWER			6.1f
	#endif
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
	#ifndef PRESSURE
		#define PRESSURE 			"PRESSURE"
		#define PRESSURE_VENDER			"MTK"
	#endif
	#ifndef PRESSURE_RANGE
		#define PRESSURE_RANGE 			1100.0f
	#endif
	#ifndef PRESSURE_RESOLUTION
		#define PRESSURE_RESOLUTION 		100.0f
	#endif
	#ifndef PRESSURE_POWER
		#define PRESSURE_POWER			0.5f
	#endif
#endif

#ifdef CUSTOM_KERNEL_TEMPURATURE
	#ifndef TEMPURATURE
		#define TEMPURATURE 			"TEMPURATURE"
		#define TEMPURATURE_VENDER		"MTK"
	#endif
	#ifndef TEMPURATURE_RANGE
		#define TEMPURATURE_RANGE 		85.0f
	#endif
	#ifndef TEMPURATURE_RESOLUTION
		#define TEMPURATURE_RESOLUTION 		0.1f
	#endif
	#ifndef TEMPURATURE_POWER
		#define TEMPURATURE_POWER 		0.5f
	#endif
#endif

/*--------------------------------------------------------*/

struct sensor_t sSensorList[MAX_NUM_SENSOR] = 
{
#ifdef CUSTOM_KERNEL_ACCELEROMETER
	{  
		.name       = ACCELEROMETER,
		.vendor     = ACCELEROMETER_VENDER,
		.version    = 1,
		.handle     = ID_ACCELEROMETER,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = ACCELEROMETER_RANGE,//32.0f,
		.resolution = ACCELEROMETER_RESOLUTION,//4.0f/1024.0f,
		.power      = ACCELEROMETER_POWER,//130.0f/1000.0f,
		.reserved   = {}
	},	
#endif

#ifdef CUSTOM_KERNEL_ALSPS
	{ 
		.name       = PROXIMITY,
		.vendor     = PROXIMITY_VENDER,
		.version    = 1,
		.handle     = ID_PROXIMITY,
		.type       = SENSOR_TYPE_PROXIMITY,
		.maxRange   = PROXIMITY_RANGE,//1.00f,
		.resolution = PROXIMITY_RESOLUTION,//1.0f,
		.power      = PROXIMITY_POWER,//0.13f,
		.reserved   = {}
	},

	{ 
		.name       = LIGHT,
		.vendor     = LIGHT_VENDER,
		.version    = 1,
		.handle     = ID_LIGHT,
		.type       = SENSOR_TYPE_LIGHT,
		.maxRange   = LIGHT_RANGE,//10240.0f,
		.resolution = LIGHT_RESOLUTION,//1.0f,
		.power      = LIGHT_POWER,//0.13f,
		.reserved   = {}
	},
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
	{ 
		.name       = GYROSCOPE,
		.vendor     = GYROSCOPE_VENDER,
		.version    = 1,
		.handle     = ID_GYROSCOPE,
		.type       = SENSOR_TYPE_GYROSCOPE,
		.maxRange   = GYROSCOPE_RANGE,//34.91f,
		.resolution = GYROSCOPE_RESOLUTION,//0.0107f,
		.power      = GYROSCOPE_POWER,//6.1f,
		.reserved   = {}
	},
#endif	

#ifdef CUSTOM_KERNEL_MAGNETOMETER
	{ 
		.name       = ORIENTATION,
		.vendor     = ORIENTATION_VENDER,
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = ORIENTATION_RANGE,//360.0f,
		.resolution = ORIENTATION_RESOLUTION,//1.0f,
		.power      = ORIENTATION_POWER,//0.25f,
		.reserved   = {}
	},

	{ 
		.name       = MAGNETOMETER,
		.vendor     = MAGNETOMETER_VENDER,
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = MAGNETOMETER_RANGE,//600.0f,
		.resolution = MAGNETOMETER_RESOLUTION,//0.0016667f,
		.power      = MAGNETOMETER_POWER,//0.25f,
		.reserved   = {}
	}, 
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
	{ 
		.name       = PRESSURE,
		.vendor     = PRESSURE_VENDER,
		.version    = 1,
		.handle     = ID_PRESSURE,
		.type       = SENSOR_TYPE_PRESSURE,
		.maxRange   = PRESSURE_RANGE,//360.0f,
		.resolution = PRESSURE_RESOLUTION,//1.0f,
		.power      = PRESSURE_POWER,//0.25f,
		.reserved   = {}
	},
#endif
	
#ifdef CUSTOM_KERNEL_TEMPURATURE
	{ 
		.name       = TEMPURATURE,
		.vendor     = TEMPURATURE_VENDER,
		.version    = 1,
		.handle     = ID_TEMPRERATURE,
		.type       = SENSOR_TYPE_TEMPERATURE,
		.maxRange   = TEMPURATURE_RANGE,//600.0f,
		.resolution = TEMPURATURE_RESOLUTION,//0.0016667f,
		.power      = TEMPURATURE_POWER,//0.25f,
		.reserved   = {}
	}, 
#endif

};

