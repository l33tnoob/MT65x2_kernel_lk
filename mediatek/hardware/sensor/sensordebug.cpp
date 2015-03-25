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


#include <cutils/log.h>

#include "Hwmsen.h"
#include "Acceleration.h"
#include "Magnetic.h"

#include "sensordebug.h"

#define LOG_TAG "sensordebug"

/*****************************************************************************/
unsigned int SensorDebugObject::SensorDebugBit=0;

SensorBase* SensorDebugObject::mDebugsensorlist[3]={0,0,0};

SensorDebugObject::SensorDebugObject(SensorBase* sensorbaseList, int sensor)
    : m_G_active_log_path_id(0), m_MSENSOR_ACCURANCY(0), m_MAG_DATA(0), m_Mode_value(0),
      m_Data_len(0), m_Is_old_m_driver(false), m_Is_old_g_driver(false)
{
	mDebugsensorlist[sensor] = sensorbaseList;
	m_Is_old_m_driver = is_old_structure(2);//check where sensor data from
	m_Sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

	int ret = pthread_create(&mThread, NULL, sensors_debug, (void*)this);
	if (ret) { 
		ALOGD( "Could not create thread for debug");
	}
	else
	{
		ALOGD("debug Thread is running and listening on ");
	}
	if (pthread_cond_init(&m_Sensor_event_cond, NULL))
	{
		ALOGD("debug Thread pthread_cond_init error ");
	}

	
	//{@init sensor log path
	char p[20] ={SENSOR_LOG};
	char q[20] ={SENSOR_LOG2};	
	strcpy(&m_Record_path[0][0],p);
	strcpy(&m_Record_path[1][0],q);
	//@}
}


SensorDebugObject::~SensorDebugObject() {

}


void SensorDebugObject::g_sensor_debug_func(){
	//do gsensor debug operations
}




void SensorDebugObject::m_sensor_debug_func(){
	sensors_event_t *tempdata;
	sensors_event_t data;	
    int i=1;
    
	if(m_Is_old_m_driver){
		if(mDebugsensorlist[0] == 0){
			ALOGE("mDebugsensorlist[0] 0 pointer");
			return;
			}
		tempdata = ((Hwmsen*)mDebugsensorlist[0])->mPendingEvents;
		data = *(tempdata+1);
	}else{
		if(mDebugsensorlist[2] == 0){
			ALOGE("mDebugsensorlist[2] 0 pointer");
			return;
			}
		tempdata = ((MagneticSensor*)mDebugsensorlist[2])->mPendingEvent;
		data = *tempdata;
	}

	if(m_MSENSOR_ACCURANCY != data.magnetic.status)
	{
		float time = (float)((float)(data.timestamp)/(float)1000000000);
		//ALOGD("M_ACC: %d, %.3f",data.magnetic.status,time);
		//{@ write data to file
		char buf[50];
		int len = 0;
		len = sprintf(buf,"M_ACC:%d,%.3f\n",data.magnetic.status,time);
		//ALOGD("M_ACC len: %d",len);					
		write_sensor_log(buf,len);
		//write data to file @}
		m_MSENSOR_ACCURANCY = data.magnetic.status;
	}
	
	double x = 0,y = 0,z = 0, result = 0;
	int mode_temp = 0;
	x = (double)(data.magnetic.x);
	y = (double)(data.magnetic.y);
	z = (double)(data.magnetic.z);
	
	x = pow(x,2);
	y = pow(y,2);
	z = pow(z,2);
	
	result = x+y+z;
	result = sqrt(result);
	mode_temp =result;
	if(abs(m_Mode_value-mode_temp)> 25)
	{
		float time = (float)((float)(data.timestamp)/(float)1000000000);
		//ALOGD("M_DA: %.3f, %.3f",result,time);
		//{@ write data to file
		char buf[50];
		int len = 0;
		len = sprintf(buf,"M_DA:%.3f,%.3f\n",result,time);
		//ALOGD("M_DA len: %d",len);
		write_sensor_log(buf,len);
		//write data to file @} 							
		m_Mode_value = mode_temp;
	}	
}

void SensorDebugObject::write_sensor_log(char *buf,int len){
	struct stat buffer;
	int fd = 0;
	if(m_Data_len < TEMP_BUFFER_SIZE){
		strcpy(&m_Data_buffer[m_Data_len], buf);
		m_Data_len += len;
		//ALOGD("%s: yucong data lenth:%d", __func__, m_Data_len);
	}else{
		//ALOGD("%s: begin data saving!", __func__);
		FILE* f1;
		//ALOGD("debug %s:!", m_Record_path[m_G_active_log_path_id]);
		if((f1 = fopen(m_Record_path[m_G_active_log_path_id], "at+")) == NULL)
		{
			ALOGD("%s: open file: %s err!", __func__, SENSOR_LOG);
		}
		else
		{
			fwrite(&m_Data_buffer[0], sizeof(char), m_Data_len, f1);
		}
		fclose(f1);
		m_Data_len = 0;
		
		if((fd = open(m_Record_path[m_G_active_log_path_id], O_RDWR))<0){
			ALOGD("%s: open file: %s err!", __func__, SENSOR_LOG);	
		}
		else
		{
			if(fstat(fd, &buffer)<0){
				ALOGD("get file size error!");
			}
			ALOGD("file size: %lld", buffer.st_size);	
		}
		close(fd);

		if(buffer.st_size > MAX_RECORD_LEN)
		{

		   ALOGD("size > MAX_RECORD_LEN!");
		   //recored in other file
		   m_G_active_log_path_id++;
		   if(2 == m_G_active_log_path_id)
		   {
			 m_G_active_log_path_id = 0;
		   }
		   ALOGD("m_G_active_log_path_id=%d",m_G_active_log_path_id);
		   ALOGD("going to record to %s ",m_Record_path[m_G_active_log_path_id]);
		   //clear the file we going to recored log
		   if((f1 = fopen(m_Record_path[m_G_active_log_path_id], "w+"))<0)
		   {
			  ALOGD("%s: open file: %s err!", __func__, SENSOR_LOG);	
		   }
		   fclose(f1);
		   
		 
		}
	}
}

bool SensorDebugObject::is_old_structure(int sensor){
	if(sensor == 1){
		char input_sysfs_path[4096];
		strcpy(input_sysfs_path, "/sys/class/misc/m_acc_misc/accactive");
		int fd = 0;
		fd = open(&input_sysfs_path[0], O_RDWR);
		if(fd < 0)
		{
		    ALOGD("old msensor driver will be used\r\n");
			return true;
		}
		//ALOGD("new msensor driver will be used \r\n");
	  	close(fd);
		}

	if(sensor == 2){
		char input_sysfs_path[4096];
		strcpy(input_sysfs_path, "/sys/class/misc/m_mag_misc/magactive");
		int fd = 0;
		fd = open(&input_sysfs_path[0], O_RDWR);
		if(fd < 0)
		{
		    ALOGD("old msensor driver will be used\r\n");
			return true;
		}
		//ALOGD("new msensor driver will be used \r\n");
	  	close(fd);
		}

	return false;
}

void SensorDebugObject::send_singnal(int i){
	if((2 == i)||(0 == i))
	{
		pthread_mutex_lock(&m_Sensor_mutex);
		if (pthread_cond_signal(&m_Sensor_event_cond))
			{
			ALOGE("%s: set signal error",__func__);
			}
		pthread_mutex_unlock(&m_Sensor_mutex);
		//ALOGD("%s: set signal  \r\n",__func__);
	}
}


void * SensorDebugObject::sensors_debug(void *para){
	SensorDebugObject *dev = (SensorDebugObject *) para;
	while(1){
		pthread_mutex_lock(&dev->m_Sensor_mutex);
		if(pthread_cond_wait(&dev->m_Sensor_event_cond, &dev->m_Sensor_mutex))
			{
			ALOGD("%s: wait error\r\n",__func__);
			}
		pthread_mutex_unlock(&dev->m_Sensor_mutex);
		//ALOGD("%s: begin debug  \r\n",__func__);
		dev->m_sensor_debug_func();
		dev->g_sensor_debug_func();
		}
	return 0;
}

