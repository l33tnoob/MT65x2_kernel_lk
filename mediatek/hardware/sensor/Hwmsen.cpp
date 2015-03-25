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
#define LOG_TAG "Hwmsen_sensors"

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "Hwmsen.h"

#include <hwmsen_chip_info.h>


Hwmsen::Hwmsen()
    : SensorBase(LS_DEVICE_NAME, "hwmdata"),
      mPendingMask(0),
      mEnabled(0),
      mActiveSensors(0),
      mInputReader(4),
      mHasPendingEvent(false)
{
	mDelays[numSensors]={0};
	//memset(g_sensor_user_count,0,sizeof(g_sensor_user_count));
	ALOGD("Hwmsen mPendingEvents len(%d)\r\n",sizeof(mPendingEvents)/sizeof(sensors_event_t));
	memset(mPendingEvents, 0, sizeof(mPendingEvents));
    mPendingEvents[Accelerometer].version = sizeof(sensors_event_t);
    mPendingEvents[Accelerometer].sensor = ID_ACCELEROMETER;
    mPendingEvents[Accelerometer].type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvents[Accelerometer].acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    
    mPendingEvents[MagneticField].version = sizeof(sensors_event_t);
    mPendingEvents[MagneticField].sensor = ID_MAGNETIC;
    mPendingEvents[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvents[MagneticField].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;
    
    mPendingEvents[Orientation  ].version = sizeof(sensors_event_t);
    mPendingEvents[Orientation  ].sensor = ID_ORIENTATION;
    mPendingEvents[Orientation  ].type = SENSOR_TYPE_ORIENTATION;
    mPendingEvents[Orientation  ].orientation.status = SENSOR_STATUS_ACCURACY_HIGH;
   
	mPendingEvents[Gyro].version = sizeof(sensors_event_t);
    mPendingEvents[Gyro].sensor = ID_GYROSCOPE;
    mPendingEvents[Gyro].type = SENSOR_TYPE_GYROSCOPE;
    mPendingEvents[Gyro].orientation.status = SENSOR_STATUS_ACCURACY_HIGH;
    
	mPendingEvents[light].version = sizeof(sensors_event_t);
    mPendingEvents[light].sensor = ID_LIGHT;
    mPendingEvents[light].type = SENSOR_TYPE_LIGHT;
    
	mPendingEvents[proximity].version = sizeof(sensors_event_t);
    mPendingEvents[proximity].sensor = ID_PROXIMITY;
    mPendingEvents[proximity].type = SENSOR_TYPE_PROXIMITY;
	
    for (int i=0 ; i<numSensors ; i++)
    {
        mDelays[i] = 200000000; // 200 ms by default
    }
    open_device();
	ALOGD("Hwmsen Construct ok\r\n");

	//{@input value 0 stands for old hwmsen sensor,please refer to nusensor.cpp 
	//enmu in sensors_poll_context_t to see other sensors,this is for sensor debug log
	//mHwmSensorDebug = new SensorDebugObject((Hwmsen*)this, 0);
	//@}
}

Hwmsen::~Hwmsen() {
	close_device();
}

int Hwmsen::enableNoHALDataAcc(int en)
{
	int io_value = 0;
	if(1==en)
	{
		io_value = ID_ACCELEROMETER;
		if(ioctl(dev_fd, HWM_IO_ENABLE_SENSOR_NODATA, &io_value))
		{
			ALOGE("%s: Enable  nodata old acc error!",__func__);
			return -1;
		}	
	}
	if(0==en)
	{
		io_value = ID_ACCELEROMETER;
		if(ioctl(dev_fd, HWM_IO_DISABLE_SENSOR_NODATA, &io_value))
		{
			ALOGE("%s: disable  nodata old acc error!",__func__);
			return -1;
		}	
	}
	return 0;
}

int Hwmsen::enable(int32_t handle, int en) {
	//int sensor_type =0;
	int io_value = 0;
	int i=0;
    int flags = en ? 1 : 0;
    int err = 0;
	//uint32_t sensor = 0;
	uint32_t sensor = (1 << handle);	// new active/inactive sensor
	ALOGD("Hwmsen_Enable: handle:%d, en:%d \r\n",handle,en);
	if( handle != ID_ACCELEROMETER && handle != ID_MAGNETIC &&
		handle != ID_ORIENTATION && handle != ID_GYROSCOPE && 
		handle != ID_PROXIMITY && handle != ID_LIGHT)
	{
	    ALOGD("enable: (handle=%d) is not hwmsen driver command", handle);
		return 0;
	}
	   
	ALOGD("%s: handle %d, enable or disable %d!", __func__, handle, en);

	if(en == 1)
	{

       	
		// TODO:  Device IO control to enable sensor
		if(ioctl(dev_fd, HWM_IO_ENABLE_SENSOR, &handle))
		{
			ALOGE("%s: Enable sensor %d error!",__func__, handle);
			return -1;
		}

		// When start orientation sensor, should start the G and M first.
		if(((mActiveSensors & SENSOR_ORIENTATION) == 0) 	// hvae no orientation sensor
			&& (sensor & SENSOR_ORIENTATION))					// new orientation sensor start
		{
						
			io_value = ID_ACCELEROMETER;
			if(ioctl(dev_fd, HWM_IO_ENABLE_SENSOR_NODATA, &io_value))
			{
				ALOGE("%s: Enable ACCELEROMETR sensor error!",__func__);
				return -1;
			}
			

			io_value = ID_MAGNETIC;
			if(ioctl(dev_fd, HWM_IO_ENABLE_SENSOR_NODATA, &io_value))
			{
				ALOGE("%s: Enable MAGNETIC sensor error!",__func__);
				return -1;
			}
		}
		mEnabled = 1;
		mActiveSensors |= sensor;
	}
	else
	{
		mActiveSensors &= ~sensor;
		
		
		if(0==mActiveSensors)
		{
			mEnabled = 0;
		}

		// When stop Orientation, should stop G and M sensor if they are inactive
		if(((mActiveSensors & SENSOR_ORIENTATION) == 0)
			&& (sensor & SENSOR_ORIENTATION))
		{
			

			io_value = ID_ACCELEROMETER;
			if(ioctl(dev_fd, HWM_IO_DISABLE_SENSOR_NODATA, &io_value))
			{
				ALOGE("%s: Disable ACCELEROMETR sensor error!",__func__);
				return -1;
			}
			
					
			io_value = ID_MAGNETIC;
			if(ioctl(dev_fd, HWM_IO_DISABLE_SENSOR_NODATA, &io_value))
			{
				ALOGE("%s: Disable MAGNETIC sensor error!",__func__);
				return -1;
			}
			
		}		

		// TODO: Device IO control disable sensor
		if(ioctl(dev_fd, HWM_IO_DISABLE_SENSOR, &handle))
		{
			ALOGE("%s: Disable sensor %d error!",__func__, handle);
			return -1;
		}
   	}

	ALOGD("active_sensors =%x\r\n", mActiveSensors);

    return err;
}

bool Hwmsen::hasPendingEvents() const {
    return mHasPendingEvent;
}


int Hwmsen::readEvents(sensors_event_t* data, int count)
{
    int err=0;
	int i=0;
	
	
    if (count < 1)
    	{
    		//ALOGE("hwmsen: read event count:%d",count);
      		return -EINVAL;
      	}

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
    	{
    		//ALOGE("hwmsen: read event n:%d", n);
      		return n;
      	}

    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
		//ALOGE("hwmsen: read event (type=%d, code=%d value=%d)",type, event->code,event->value);
        if (type == EV_REL) 
		{
            processEvent(event->code, event->value);
            int64_t time = timevalToNano(event->time);

			//ALOGE("hwmsen:  ++mPendingMask:0x%x",mPendingMask);
			
            for (i=0 ; count && i<MAX_ANDROID_SENSOR_NUM ; i++) 
			{
			   if (mPendingMask & (1<<i)) 
			   {
                    mPendingMask &= ~(1<<i);
                    //mPendingEvents[i].timestamp = time;
                                 
                    *data++ = mPendingEvents[i];
                    count--;
                    numEventReceived++;
                   	//ALOGE("hwmsen:  count:%d, numEventReceived:%d",count,numEventReceived);
               }
                    
            }
        }  
		else if (type == EV_SYN) 
        {
           //ALOGE("hwmsen: EV_SYN event (type=%d, code=%d)",
                   // type, event->code);
            
        } else {
            ALOGE("hwmsen: unknown event (type=%d, code=%d)",
                    type, event->code);
            mInputReader.next();
        }
		mInputReader.next();
    }

    return numEventReceived;
}

void Hwmsen::processEvent(int code, int value)
{
    hwm_trans_data sensors_data;
	int err =0;
	int i=0;
	memset(&sensors_data, 0 , sizeof(hwm_trans_data));
	
    if(EVENT_TYPE_SENSOR != code)
    {
        ALOGE("hwmsen: processEvent code =%d",code);
        ALOGE("hwmsen: processEvent error!!!");
		return;
    }
	//ALOGE("hwmsen: processEvent value =%d",value);

    sensors_data.date_type = value;//set flag to read specified sensor
	err = ioctl(dev_fd, HWM_IO_GET_SENSORS_DATA, &sensors_data);

	for(i =0; i < MAX_ANDROID_SENSOR_NUM; i++)
	{
		//ALOGD("%s:get sensor value,id: %d, value1 %d, updata %d!\r\n", __func__,
					//i, sensors_data.data[i].values[0], sensors_data.data[i].update);
		if(sensors_data.data[i].update == 0)
		{
			continue;
		}
		switch (i) 
        {
          
        case ID_ORIENTATION:
			mPendingMask |= 1<<Orientation;
			mPendingEvents[Orientation].type = SENSOR_TYPE_ORIENTATION;
			mPendingEvents[Orientation].sensor = sensors_data.data[ID_ORIENTATION].sensor;
			mPendingEvents[Orientation].orientation.status = sensors_data.data[ID_ORIENTATION].status;
			mPendingEvents[Orientation].orientation.v[0] = (float)sensors_data.data[ID_ORIENTATION].values[0];
			mPendingEvents[Orientation].orientation.v[1] = (float)sensors_data.data[ID_ORIENTATION].values[1];
			mPendingEvents[Orientation].orientation.v[2] = (float)sensors_data.data[ID_ORIENTATION].values[2];

			mPendingEvents[Orientation].orientation.v[0]/=sensors_data.data[ID_ORIENTATION].value_divide;
			mPendingEvents[Orientation].orientation.v[1]/=sensors_data.data[ID_ORIENTATION].value_divide;
			mPendingEvents[Orientation].orientation.v[2]/=sensors_data.data[ID_ORIENTATION].value_divide;
            mPendingEvents[Orientation].timestamp = sensors_data.data[ID_ORIENTATION].time;
			break;
			
		case ID_MAGNETIC:
			mPendingMask |= 1<<MagneticField;
			mPendingEvents[MagneticField].type = SENSOR_TYPE_MAGNETIC_FIELD;
			mPendingEvents[MagneticField].sensor = sensors_data.data[ID_MAGNETIC].sensor;
			mPendingEvents[MagneticField].magnetic.status = sensors_data.data[ID_MAGNETIC].status;
			mPendingEvents[MagneticField].magnetic.v[0] = (float)sensors_data.data[ID_MAGNETIC].values[0];
			mPendingEvents[MagneticField].magnetic.v[1] = (float)sensors_data.data[ID_MAGNETIC].values[1];
			mPendingEvents[MagneticField].magnetic.v[2] = (float)sensors_data.data[ID_MAGNETIC].values[2];

			mPendingEvents[MagneticField].magnetic.v[0]/=sensors_data.data[ID_MAGNETIC].value_divide;
			mPendingEvents[MagneticField].magnetic.v[1]/=sensors_data.data[ID_MAGNETIC].value_divide;
			mPendingEvents[MagneticField].magnetic.v[2]/=sensors_data.data[ID_MAGNETIC].value_divide;
            mPendingEvents[MagneticField].timestamp = sensors_data.data[ID_MAGNETIC].time;
			//ALOGE("[ID_MAGNETIC](%f,%f,%f) \r\n",mPendingEvents[MagneticField].magnetic.v[0],mPendingEvents[MagneticField].magnetic.v[1],mPendingEvents[MagneticField].magnetic.v[2]);
			//{@input value 0 stands for old hwmsen sensor,please refer to nusensor.cpp enmu in sensors_poll_context_t to see other sensors
    		//mHwmSensorDebug->send_singnal(0);
			//@}
			break;
			
		case ID_ACCELEROMETER:
			
			mPendingMask |= 1<<Accelerometer;
			mPendingEvents[Accelerometer].type = SENSOR_TYPE_ACCELEROMETER;
			mPendingEvents[Accelerometer].sensor = sensors_data.data[ID_ACCELEROMETER].sensor;
			mPendingEvents[Accelerometer].acceleration.status = sensors_data.data[ID_ACCELEROMETER].status;
			mPendingEvents[Accelerometer].acceleration.v[0] = (float)sensors_data.data[ID_ACCELEROMETER].values[0];
			mPendingEvents[Accelerometer].acceleration.v[1] = (float)sensors_data.data[ID_ACCELEROMETER].values[1];
			mPendingEvents[Accelerometer].acceleration.v[2] = (float)sensors_data.data[ID_ACCELEROMETER].values[2];

			mPendingEvents[Accelerometer].acceleration.v[0]/=sensors_data.data[ID_ACCELEROMETER].value_divide;
			mPendingEvents[Accelerometer].acceleration.v[1]/=sensors_data.data[ID_ACCELEROMETER].value_divide;
			mPendingEvents[Accelerometer].acceleration.v[2]/=sensors_data.data[ID_ACCELEROMETER].value_divide;
            mPendingEvents[Accelerometer].timestamp = sensors_data.data[ID_ACCELEROMETER].time;
			//ALOGE("[ID_ACCELEROMETER](%f,%f,%f) \r\n",mPendingEvents[Accelerometer].acceleration.v[0],mPendingEvents[Accelerometer].acceleration.v[1],mPendingEvents[Accelerometer].acceleration.v[2]);
			break;
		case ID_GYROSCOPE:
			
			mPendingMask |= 1<<Gyro;
			mPendingEvents[Gyro].type = SENSOR_TYPE_GYROSCOPE;
			mPendingEvents[Gyro].sensor = sensors_data.data[ID_GYROSCOPE].sensor;
			mPendingEvents[Gyro].gyro.status = sensors_data.data[ID_GYROSCOPE].status;
			mPendingEvents[Gyro].gyro.v[0] = (float)sensors_data.data[ID_GYROSCOPE].values[0];
			mPendingEvents[Gyro].gyro.v[1] = (float)sensors_data.data[ID_GYROSCOPE].values[1];
			mPendingEvents[Gyro].gyro.v[2] = (float)sensors_data.data[ID_GYROSCOPE].values[2];

			mPendingEvents[Gyro].gyro.v[0]/=sensors_data.data[ID_GYROSCOPE].value_divide;
			mPendingEvents[Gyro].gyro.v[1]/=sensors_data.data[ID_GYROSCOPE].value_divide;
			mPendingEvents[Gyro].gyro.v[2]/=sensors_data.data[ID_GYROSCOPE].value_divide;
			mPendingEvents[Gyro].timestamp = sensors_data.data[ID_GYROSCOPE].time;
			//ALOGE("[ID_GYROSCOPE](%f,%f,%f) \r\n",mPendingEvents[Gyro].gyro.v[0],mPendingEvents[Gyro].gyro.v[1],mPendingEvents[Gyro].gyro.v[2]);
			break;
			
		case ID_PROXIMITY:
			mPendingMask |= 1<<proximity;
			mPendingEvents[proximity].type  = SENSOR_TYPE_PROXIMITY;
			mPendingEvents[proximity].sensor = sensors_data.data[ID_PROXIMITY].sensor;
			mPendingEvents[proximity].distance = (float)sensors_data.data[ID_PROXIMITY].values[0];
            mPendingEvents[proximity].timestamp = sensors_data.data[ID_PROXIMITY].time;
			break;
			
		case ID_LIGHT:
			 mPendingMask |= 1<<light;
			 mPendingEvents[light].type = SENSOR_TYPE_LIGHT;
			 mPendingEvents[light].sensor = sensors_data.data[ID_LIGHT].sensor;
			 mPendingEvents[light].light = (float)sensors_data.data[ID_LIGHT].values[0];
			 mPendingEvents[light].timestamp = sensors_data.data[ID_LIGHT].time;
			break;
       }				
		
	}

}

float Hwmsen::indexToValue(size_t index) const
{
    static const float luxValues[8] = {
            10.0, 160.0, 225.0, 320.0,
            640.0, 1280.0, 2600.0, 10240.0
    };

    const size_t maxIndex = sizeof(luxValues)/sizeof(*luxValues) - 1;
    if (index > maxIndex)
        index = maxIndex;
    return luxValues[index];
}

int Hwmsen::setDelay(int32_t handle, int64_t ns)
{
    int what = -1;
	ALOGD("setDelay: (handle=%d, ns=%d)",
                    handle, ns);

	if( handle != ID_ACCELEROMETER && handle != ID_MAGNETIC &&
		handle != ID_ORIENTATION && handle != ID_GYROSCOPE && 
		handle != ID_LIGHT)
	{
	    ALOGD("setDelay: (handle=%d, ns=%d) is not hwmsen driver command", handle, ns);
		return 0;
	}
	
    what = handle;
    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    if (ns < 0)
        return -EINVAL;
    mDelays[what] = ns;
    return update_delay(what);

}

int Hwmsen::update_delay(int what)
{

    struct sensor_delay delayPara;
	
    if (mEnabled) 
	{
		delayPara.delay = mDelays[what]/1000000;
		delayPara.handle = what;
        ALOGD("setDelay: (what=%d, ms=%d)",
                    delayPara.handle , delayPara.delay);
		if(delayPara.delay < 10)  //set max sampling rate = 100Hz
        {
           ALOGD("Control set delay %d ms is too small \n",delayPara.delay );
           delayPara.delay = 10;
           
        }
		ALOGD("really setDelay: (what=%d, ms=%d)",
                    delayPara.handle , delayPara.delay);
		if(ioctl(dev_fd, HWM_IO_SET_DELAY, &delayPara))
	    {
		  ALOGE("%s: Set delay %d ms error ", __func__, delayPara.delay);
		  return -errno;
	    }
    }
    return 0;
}

