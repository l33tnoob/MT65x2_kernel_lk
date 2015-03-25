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
#define LOG_TAG "Sensors"

#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>

#include <poll.h>
#include <pthread.h>

#include <linux/input.h>

#include <cutils/atomic.h>
#include <cutils/log.h>

#include "nusensors.h"
#include "Hwmsen.h"
#include "Acceleration.h"
#include "Magnetic.h"


/*****************************************************************************/

struct sensors_poll_context_t {
    struct sensors_poll_device_t device; // must be first

        sensors_poll_context_t();
        ~sensors_poll_context_t();
    int activate(int handle, int enabled);
    int setDelay(int handle, int64_t ns);
    int pollEvents(sensors_event_t* data, int count);

private:
    enum {
        hwmsen        = 0, 
        accel,
        magnetic,
        //gyro,
        //light,
        //proximity,
        //pressure,
        numSensorDrivers,
        numFds,
    };

	int handleToDriver(int handle) const {
        switch (handle) {
            case ID_ACCELEROMETER:
				 return accel;
            case ID_MAGNETIC:
            case ID_ORIENTATION:
				 return magnetic;
            case ID_PROXIMITY:
				 //return proximity;
            case ID_LIGHT:
				 //return light;
            case ID_GYROSCOPE:
				 //return gyro;
			case ID_PRESSURE:
				 break;
				 //return pressure;
        }
        return -EINVAL;
    }

    static const size_t wake = numFds - 1;
    static const char WAKE_MESSAGE = 'W';
    struct pollfd mPollFds[numFds];
    int mWritePipeFd;
    SensorBase* mSensors[numSensorDrivers];
};

/*****************************************************************************/

sensors_poll_context_t::sensors_poll_context_t()
{
    mSensors[hwmsen] = new Hwmsen();
    mPollFds[hwmsen].fd = mSensors[hwmsen]->getFd();
    mPollFds[hwmsen].events = POLLIN;
    mPollFds[hwmsen].revents = 0;

	mSensors[accel] = new AccelerationSensor();
    mPollFds[accel].fd = mSensors[accel]->getFd();
    mPollFds[accel].events = POLLIN;
    mPollFds[accel].revents = 0;

	mSensors[magnetic] = new MagneticSensor();
    mPollFds[magnetic].fd = mSensors[magnetic]->getFd();
    mPollFds[magnetic].events = POLLIN;
    mPollFds[magnetic].revents = 0;

    int wakeFds[2];
    int result = pipe(wakeFds);
    ALOGE_IF(result<0, "error creating wake pipe (%s)", strerror(errno));
    fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
    fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
    mWritePipeFd = wakeFds[1];

    mPollFds[wake].fd = wakeFds[0];
    mPollFds[wake].events = POLLIN;
    mPollFds[wake].revents = 0;
}

sensors_poll_context_t::~sensors_poll_context_t() {
    for (int i=0 ; i<numSensorDrivers ; i++) {
        delete mSensors[i];
    }
    close(mPollFds[wake].fd);
    close(mWritePipeFd);
}

int sensors_poll_context_t::activate(int handle, int enabled) 
{
	ALOGD( "activate handle =%d, enable = %d",handle, enabled );
	int err=0;
	
	int index = handleToDriver(handle);

	//
	if(ID_ORIENTATION == handle)
	{
	     //ALOGD( "fwq1111" );
		((AccelerationSensor*)(mSensors[accel]))->enableNoHALDataAcc(enabled);
		((Hwmsen*)(mSensors[hwmsen]))->enableNoHALDataAcc(enabled);
	}
	
	if(NULL != mSensors[index] && index >0 )
	{
	   ALOGD( "use new sensor index=%d, mSensors[index](%x)", index, mSensors[index]);
	   err =  mSensors[index]->enable(handle, enabled);
	}
	
	if(err || index<0 )
	{
  		ALOGD("use old sensor err(%d),index(%d) go to old hwmsen\n",err,index);
		// notify to hwmsen sensor to support old architecture
		err =  mSensors[hwmsen]->enable(handle, enabled);
	}
	
    if (enabled && !err) {
        const char wakeMessage(WAKE_MESSAGE);
        int result = write(mWritePipeFd, &wakeMessage, 1);
        ALOGE_IF(result<0, "error sending wake message (%s)", strerror(errno));
    }
    return err;
}

int sensors_poll_context_t::setDelay(int handle, int64_t ns) 
{

    int err =0;
	
	
    int index = handleToDriver(handle);
	if(NULL != mSensors[index] && index >0)
	{
	   err = mSensors[index]->setDelay(handle, ns);
	   
	}
	if(err || index<0)
	{
		ALOGE("new acc setDelay handle(%d),ns(%lld) err! go to hwmsen\n",handle,ns);
		// notify to hwmsen sensor to support old architecture
	 	err = mSensors[hwmsen]->setDelay(handle, ns);
	}
    return err;
}

int sensors_poll_context_t::pollEvents(sensors_event_t* data, int count)
{
    int nbEvents = 0;
    int n = 0;
	//ALOGE("pollEvents count =%d",count );

    do {
        // see if we have some leftover from the last poll()
        for (int i=0 ; count && i<numSensorDrivers ; i++) {
            SensorBase* const sensor(mSensors[i]);
            if ((mPollFds[i].revents & POLLIN) || (sensor->hasPendingEvents())) {
                int nb = sensor->readEvents(data, count);
                if (nb < count) {
                    // no more data for this sensor
                    mPollFds[i].revents = 0;
                }
                //if(nb < 0||nb > count)
                //	ALOGE("pollEvents count error nb:%d, count:%d, nbEvents:%d", nb, count, nbEvents);//for sensor NE debug
                count -= nb;
                nbEvents += nb;
                data += nb;
                //if(nb < 0||nb > count)
                //	ALOGE("pollEvents count error nb:%d, count:%d, nbEvents:%d", nb, count, nbEvents);//for sensor NE debug
            }
        }

        
        if (count) {
            // we still have some room, so try to see if we can get
            // some events immediately or just wait if we don't have
            // anything to return
            
            n = poll(mPollFds, numFds, nbEvents ? 0 : -1);
            if (n<0) {
                ALOGE("poll() failed (%s)", strerror(errno));
                return -errno;
            }
            if (mPollFds[wake].revents & POLLIN) {
                char msg;
                int result = read(mPollFds[wake].fd, &msg, 1);
                ALOGE_IF(result<0, "error reading from wake pipe (%s)", strerror(errno));
                ALOGE_IF(msg != WAKE_MESSAGE, "unknown message on wake queue (0x%02x)", int(msg));
                mPollFds[wake].revents = 0;
            }
        }

		
		
        // if we have events and space, go read them
    } while (n && count);

    return nbEvents;
}

/*****************************************************************************/

static int poll__close(struct hw_device_t *dev)
{
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    if (ctx) {
        delete ctx;
    }
    return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->activate(handle, enabled);
}

static int poll__setDelay(struct sensors_poll_device_t *dev,
        int handle, int64_t ns) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->setDelay(handle, ns);
}

static int poll__poll(struct sensors_poll_device_t *dev,
        sensors_event_t* data, int count) {
    sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
    return ctx->pollEvents(data, count);
}

/*****************************************************************************/

int init_nusensors(hw_module_t const* module, hw_device_t** device)
{
    int status = -EINVAL;

    sensors_poll_context_t *dev = new sensors_poll_context_t();
    memset(&dev->device, 0, sizeof(sensors_poll_device_t));

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version  = 0;
    dev->device.common.module   = const_cast<hw_module_t*>(module);
    dev->device.common.close    = poll__close;
    dev->device.activate        = poll__activate;
    dev->device.setDelay        = poll__setDelay;
    dev->device.poll            = poll__poll;

    *device = &dev->device.common;
    status = 0;
    return status;
}
