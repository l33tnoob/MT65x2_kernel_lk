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

#ifndef ANDROID_MAGNETIC_SENSOR_H
#define ANDROID_MAGNETIC_SENSOR_H

#include <stdint.h>
#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "sensordebug.h"
#include "nusensors.h"
#include "SensorBase.h"
#include "InputEventReader.h"
#include <linux/hwmsensor.h> 
#include "Acceleration.h"

/*****************************************************************************/
//#define MAG_DIV 1000
#define MAX_M_V_SENSOR  5

struct input_event;

class MagneticSensor : public SensorBase {
    int mEnabled;
    int mOrientationEnabled;
    InputEventCircularReader mInputReader;
	int64_t mEnabledTime;
	char input_sysfs_path[PATH_MAX];
	int input_sysfs_path_len;
	char mClassPath[PATH_MAX];
	uint32_t mPendingMask;
	enum {
        MagneticField   = 0,
        Orientation     = 1,
        numSensors
    };
	int mDataDiv_M;
	int mDataDiv_O;
	
public:

	sensors_event_t mPendingEvent[MAX_M_V_SENSOR];
	SensorDebugObject *mMagSensorDebug;


            MagneticSensor();
    virtual ~MagneticSensor();

    virtual int readEvents(sensors_event_t* data, int count);
    virtual int setDelay(int32_t handle, int64_t ns);
    virtual int enable(int32_t handle, int enabled);
    void processEvent(int code, int value);
	int write_attr(char* path, char* buf,int len);
};

/*****************************************************************************/

#endif  // ANDROID_MAGNETIC_SENSOR_H
