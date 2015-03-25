#define LOG_TAG "FlashlightDrv"
#include "flash_drv.h"
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include "kd_flashlight.h"

#define STROBE_DEV_NAME    "/dev/kd_camera_flashlight"

#include <time.h>

static int getMs()
{
	//	max:
	//	2147483648 digit
	//	2147483.648 second
	//	35791.39413 min
	//	596.5232356 hour
	//	24.85513481 day
	int t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);
	return t;
}

FlashSimpleDrv::FlashSimpleDrv()
{
	m_preOnTime=-1;
}
FlashSimpleDrv::~FlashSimpleDrv()
{
}
FlashSimpleDrv* FlashSimpleDrv::getInstance()
{
	static FlashSimpleDrv obj;
	return &obj;
}
int FlashSimpleDrv::init(unsigned long sensorDev)
{
	m_fdSTROBE = open(STROBE_DEV_NAME, O_RDWR);
	int value;
	ioctl(m_fdSTROBE,FLASH_IOC_GET_MAIN_PART_ID,&value);
	ioctl(m_fdSTROBE,FLASH_IOC_GET_SUB_PART_ID,&value);
	ioctl(m_fdSTROBE,FLASHLIGHTIOC_X_SET_DRIVER,sensorDev);
	return 0;
}
int FlashSimpleDrv::setOnOff(int a_isOn)
{
	ALOGD("setOnOff ln=%d isOn=%d", __LINE__,a_isOn);
	ioctl(m_fdSTROBE,FLASH_IOC_SET_TIME_OUT_TIME_MS,0);
	if(a_isOn==0)
	{
		//ALOGD("setOnOff ln=%d", __LINE__);
		ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,0);
		m_preOnTime=-1;
	}
	else		
	{
		//ALOGD("setOnOff ln=%d", __LINE__);
		int err;
		int minPreOnTime;
		err = getPreOnTimeMs(&minPreOnTime);
		if(err<0)
		{
			//ALOGD("setOnOff ln=%d", __LINE__);
			
		}
		else
		{
			//ALOGD("setOnOff ln=%d m_preOnTime=%d", __LINE__,m_preOnTime);			
			if(m_preOnTime==-1)
			{
				//ALOGD("setOnOff ln=%d", __LINE__);
				setPreOn();
				usleep(minPreOnTime*1000);
			}
			else
			{
				//ALOGD("setOnOff ln=%d", __LINE__);
				int curTime;
				int sleepTimeMs;
				curTime = getMs();
				sleepTimeMs = (minPreOnTime-(curTime-m_preOnTime));				
				if(sleepTimeMs>0)
				{
					//ALOGD("setOnOff ln=%d", __LINE__);
					usleep( sleepTimeMs*1000);
				}
			}
		}
		//ALOGD("setOnOff ln=%d", __LINE__);
		ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,1);
		//ALOGD("setOnOff ln=%d", __LINE__);
	}
	//ALOGD("setOnOff ln=%d", __LINE__);
	return 0;
}
int FlashSimpleDrv::uninit()
{
	close(m_fdSTROBE);
	return 0;
}


int FlashSimpleDrv::getPreOnTimeMs(int* ms)
{	
	int err;	
	*ms=0;
	err = ioctl(m_fdSTROBE,FLASH_IOC_GET_PRE_ON_TIME_MS, ms);	
	return err;
}

int FlashSimpleDrv::setPreOn()
{	
	int err = 0;	
	err = ioctl(m_fdSTROBE,FLASH_IOC_PRE_ON,0);		
	m_preOnTime=getMs();
	return err;
}

	
