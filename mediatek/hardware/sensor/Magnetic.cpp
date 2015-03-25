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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <cutils/log.h>

#include "Magnetic.h"
#define LOG_TAG "Magnetic"


#define IGNORE_EVENT_TIME 350000000
#define SYSFS_PATH           "/sys/class/input"


/*****************************************************************************/
MagneticSensor::MagneticSensor()
    : SensorBase(NULL, "m_mag_input"),//ACC_INPUTDEV_NAME
      mEnabled(0),
      mOrientationEnabled(0),
      mPendingMask(0),
      mInputReader(32)
{
    input_sysfs_path_len = 0;
	input_sysfs_path[PATH_MAX] = {0};
    mPendingEvent[0].version = sizeof(sensors_event_t);
    mPendingEvent[0].sensor = ID_MAGNETIC;
    mPendingEvent[0].type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvent[0].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent[0].data, 0x00, sizeof(mPendingEvent[0].data));

	mPendingEvent[1].version = sizeof(sensors_event_t);
    mPendingEvent[1].sensor = ID_ORIENTATION;
    mPendingEvent[1].type = SENSOR_TYPE_ORIENTATION;
    mPendingEvent[1].magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent[1].data, 0x00, sizeof(mPendingEvent[1].data));
	
	mDataDiv_M = 1;
	mDataDiv_O = 1;
    if (data_fd) {
        strcpy(input_sysfs_path, "/sys/class/misc/m_mag_misc/");
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
	ALOGD("mag misc path =%s", input_sysfs_path);

	
	char datapath1[64]={"/sys/class/misc/m_mag_misc/magactive"};
	int fd_m = open(datapath1, O_RDWR);
	char buf_m[64];
	int len_m;
	len_m = read(fd_m,buf_m,sizeof(buf_m));
	sscanf(buf_m, "%d", &mDataDiv_M);
	ALOGD("read div buf(%s)",datapath1 );
	ALOGD("mdiv_M %d",mDataDiv_M );
	if(len_m<=0)
	{
		ALOGD("read div err buf(%s)",buf_m );
	}
	close(fd_m);

	char datapath2[64]={"/sys/class/misc/m_mag_misc/magoactive"};
	int fd_o = open(datapath2, O_RDWR);
	char buf_o[64];
	int len_o;
	len_o = read(fd_o,buf_o,sizeof(buf_o));
	sscanf(buf_o, "%d", &mDataDiv_O);
	ALOGD("read div buf(%s)",datapath2 );
	ALOGD("mdiv_O %d",mDataDiv_O );
	if(len_o<=0)
	{
		ALOGD("read div err buf(%s)",buf_o );
	}
	close(fd_o);

	//{@input value 2 stands for MagneticField sensor,please refer to nusensor.cpp 
	//enmu in sensors_poll_context_t to see other sensors,this is for sensor debug log
	//mMagSensorDebug = new SensorDebugObject((SensorBase*)this, 2);
	//@}	
	
}

MagneticSensor::~MagneticSensor() {
}


int MagneticSensor::write_attr(char* path, char* buf,int len)
{
    int fd=0;
	int err=0;
	ALOGD("fwq write attr path %s   \n",path );
    fd = open(path, O_RDWR);
    if (fd >= 0) 
	{
        write(fd, buf, len);
        close(fd);      
    }
	else
	{
	    err =-1;
		ALOGD("fwq write attr %s fail \n",path );
	}
	
	return err;
   
}
int MagneticSensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;
	int err=0;
	char buf[2];
	int index;
	ALOGD("fwq enable: handle:%d, en:%d \r\n",handle,en);
	if(ID_ORIENTATION == handle)
	{
	   strcpy(&input_sysfs_path[input_sysfs_path_len], "magoactive");
	   index = Orientation;
	}
	if(ID_MAGNETIC== handle)
	{
	   strcpy(&input_sysfs_path[input_sysfs_path_len], "magactive");
	   index = MagneticField;
	}
    ALOGD("handle(%d),path:%s \r\n",handle,input_sysfs_path);
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	    ALOGD("no magntic enable attr \r\n");
		return -1;
	}
	
	if(0== en )
	{
	   mEnabled &= ~(1<<index);
	   buf[1] = 0;
	   buf[0] = '0';
	}
	
	if(1== en)
	{
	   mEnabled |= (1<<index);
	   buf[1] = 0;
	   buf[0] = '1';
	}

    write(fd, buf, sizeof(buf));
  	close(fd);
	
    ALOGD("mag(%d)  mEnabled(0x%x) ----\r\n",handle,mEnabled);    
    return 0;
}
int MagneticSensor::setDelay(int32_t handle, int64_t ns)
{
	//uint32_t ms=0;
	//ms = ns/1000000;
	int err;
	int fd;
	if(ID_ORIENTATION == handle)
	{
 		strcpy(&input_sysfs_path[input_sysfs_path_len], "magodelay");
	}
	if(ID_MAGNETIC == handle)
	{
		strcpy(&input_sysfs_path[input_sysfs_path_len], "magdelay");
	}
	
	fd = open(input_sysfs_path, O_RDWR);
	if(fd<0)
	{
	  	ALOGD("no MAG setDelay control attr\r\n" );
	  	return -1;
	}
		
    if (mEnabled) 
	{
        ALOGD("setDelay: (handle=%d, ms=%d)",handle , ns);
		char buf[80];
        sprintf(buf, "%lld", ns);
		write(fd, buf, strlen(buf)+1);
    }

	close(fd);

	ALOGD("really setDelay: (handle=%d, ns=%d)",handle , ns);
    return 0;

}
int MagneticSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
    {
        return -EINVAL;
    }

	ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
    {
        return n;
    }
    int numEventReceived = 0;
    input_event const* event;

    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            processEvent(event->code, event->value);
            mInputReader.next();
        } else if (type == EV_SYN) {
           
            int64_t time = timevalToNano(event->time);
			//ALOGE("fwqM1....\r\n");
            for (int j=0 ; count && mPendingMask && j<numSensors ; j++) 
			{
			    //ALOGE("fwqM2....\r\n");
                if (mPendingMask & (1<<j)) 
				{
				    //ALOGE("fwqM3....\r\n");
                    mPendingMask &= ~(1<<j);
                    mPendingEvent[j].timestamp = time;
            
                    *data++ = mPendingEvent[j]; 
                    count--;
                    numEventReceived++;
          
               }
            }
            if (!mPendingMask) {
                mInputReader.next();
            }
        } else {
            ALOGE("unknown event (type=%d, code=%d)",  type, event->code);
            mInputReader.next();
        }
    }

	//{@input value 2 stands for MagneticField sensor,please refer to nusensor.cpp enmu in sensors_poll_context_t to see other sensors
	//mMagSensorDebug->send_singnal(2);
	//@}
    return numEventReceived;
}

void MagneticSensor::processEvent(int code, int value)
{
   //ALOGD("processEvent code=%d,value=%d\r\n",code, value);
    switch (code) {
		 case ABS_WHEEL: 
		 	mPendingMask |= 1<<MagneticField;
		 	mPendingEvent[MagneticField].magnetic.status = value;
			break;
        case ABS_X:
			mPendingMask |= 1<<MagneticField;
            mPendingEvent[MagneticField].magnetic.x = (float)value / (float)mDataDiv_M;
            break;
        case ABS_Y:
			mPendingMask |= 1<<MagneticField;
            mPendingEvent[MagneticField].magnetic.y = (float)value / (float)mDataDiv_M;
            break;
        case ABS_Z:
			mPendingMask |= 1<<MagneticField;
            mPendingEvent[MagneticField].magnetic.z = (float)value / (float)mDataDiv_M;
            break;
        //for osensor
		case ABS_THROTTLE: 
			mPendingMask |= 1<<Orientation;
		 	mPendingEvent[Orientation].orientation.status = value;
			break;
        case ABS_RX:
			mPendingMask |= 1<<Orientation;
            mPendingEvent[Orientation].orientation.x = (float)value / (float)mDataDiv_O;
            break;
        case ABS_RY:
			mPendingMask |= 1<<Orientation;
            mPendingEvent[Orientation].orientation.y = (float)value / (float)mDataDiv_O;
            break;
        case ABS_RZ:
			mPendingMask |= 1<<Orientation;
            mPendingEvent[Orientation].orientation.z = (float)value / (float)mDataDiv_O;
            break;
    }
}
