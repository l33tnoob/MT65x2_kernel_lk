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

#ifndef ANDROID_SENSOR_DEBUG_H
#define ANDROID_SENSOR_DEBUG_H

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <poll.h>
#include <pthread.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include <signal.h>
#include <pthread.h>

#include "SensorBase.h"

//for M-sensor Accurancy Debug log
#define SENSOR_LOG  "/data/msensor1.log"
#define SENSOR_LOG2 "/data/msensor2.log"
#define MAX_RECORD_LEN 1048576//5242880(5MB) //2097152 (2MB)/ 1048576(1MB)  
#define TEMP_BUFFER_SIZE 2048


/*****************************************************************************/

class SensorDebugObject {
protected:
	int m_G_active_log_path_id;
	int m_MSENSOR_ACCURANCY;
	int m_Mode_value;
	int m_Data_len;
	bool m_Is_old_m_driver;
	bool m_Is_old_g_driver;

	double m_MAG_DATA;

	char m_Data_buffer[TEMP_BUFFER_SIZE+100];
	char m_Record_path[2][20];

	static unsigned int SensorDebugBit;
	static SensorBase* mDebugsensorlist[3];

	pthread_t mThread;
	pthread_cond_t m_Sensor_event_cond;
	pthread_mutex_t m_Sensor_mutex;

	void write_sensor_log(char *buf,int len);
	void m_sensor_debug_func();
	void g_sensor_debug_func();
	bool is_old_structure(int sensor);
	static void * sensors_debug(void *para);
	
public:
	
            SensorDebugObject(SensorBase* sensorbaseList, int sensor);
    virtual ~SensorDebugObject();
    void send_singnal(int i);
};



/*****************************************************************************/

#endif  // ANDROID_SENSOR_DEBUG_H
