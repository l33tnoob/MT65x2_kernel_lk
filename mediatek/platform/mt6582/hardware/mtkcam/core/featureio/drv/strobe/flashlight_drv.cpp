/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/








#define LOG_TAG "FlashlightDrv"
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>

//
#include "ae_feature.h"
#include "../inc/strobe_drv.h"
#include "flashlight_drv.h"
#include "kd_flashlight.h"

#include <mtkcam/Log.h>

#include <aaa_types.h>
#include <camera_custom_nvram.h>
#include <flash_feature.h>
#include <flash_param.h>
#include "flash_tuning_custom.h"



#include "camera_custom_AEPlinetable.h"
#include <cutils/xlog.h>
#include "flash_feature.h"
#include "flash_param.h"
#include <kd_camera_feature.h>
/*******************************************************************************
*
********************************************************************************/
#define STROBE_DEV_NAME    "/dev/kd_camera_flashlight"

#define DEBUG_STROBE_DRV
#ifdef  DEBUG_STROBE_DRV
#define DRV_DBG(fmt, arg...)  CAM_LOGD( fmt, ##arg)
#define DRV_ERR(fmt, arg...)  CAM_LOGD("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif


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



//#include <mtkcam/Log.h>
//#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
//#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)


//#define STROB_DRV_DBG(fmt, arg...) CAM_LOGD(LOG_TAG fmt, ##arg)
//#define STROB_DRV_ERR(fmt, arg...) CAM_LOGD(LOG_TAG "Err: %5d:, "fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
StrobeDrv* FlashlightDrv::getInstance()
{
	DRV_DBG("getInstance line=%d",__LINE__);
    static FlashlightDrv singleton;
    return &singleton;
}


/*******************************************************************************
*
********************************************************************************/
void FlashlightDrv::destroyInstance()
{
	DRV_DBG("destroyInstance line=%d",__LINE__);
}

int FlashlightDrv::getVBat(int* vbat)
{
	int err=0;
	int v;
	FILE* fp;
	fp = fopen("/sys/devuces/platform/mt6329-battery/FG_Battery_CurrentConsumption", "rb");
	if(fp!=0)
	{
		int ret;
		ret = fscanf(fp,"%d",&v);
		if(ret == 1) //read 1
		{
			*vbat = v;
		}
		else
			err = STROBE_FILE_ERR2;
		fclose(fp);
	}
	else
	{
		err = STROBE_FILE_ERR;
	}
	return err;
}

int FlashlightDrv::isOn(int* a_isOn)
{
	DRV_DBG("isOn()\n");
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("isOn() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	*a_isOn = m_isOn;

    /*
	if(a_isOn==1)
		err = ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,1);
	else if(a_isOn==0)
		err = ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,0);
	else
		err = STROBE_ERR_PARA_INVALID;

	if (err < 0)
	{
	    DRV_ERR("isOning() err=%d\n", err);
	}*/
	return err;

}
int FlashlightDrv::setOnOff(int a_isOn)
{
	DRV_DBG("setOnOff() isOn = %d\n",a_isOn);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("setOnOff() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}


	if(a_isOn==1)
	{
		int minPreOnTime;
		err = getPreOnTimeMs(&minPreOnTime);
		if(err<0)
		{
			DRV_DBG("no preon");
		}
		else
		{
			DRV_DBG("preon support");
			if(m_preOnTime==-1)
			{
				setPreOn();
				usleep(minPreOnTime*1000);
			}
			else
			{
				int curTime;
				int sleepTimeMs;
				curTime = getMs();
				sleepTimeMs = (minPreOnTime-(curTime-m_preOnTime));
				DRV_DBG("preon sleep %d ms", sleepTimeMs);
				if(sleepTimeMs>0)
				{
					usleep( sleepTimeMs*1000);
				}
			}
		}
		err = ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,1);
	}
	else if(a_isOn==0)
	{
		m_preOnTime=-1;
		err = ioctl(m_fdSTROBE,FLASH_IOC_SET_ONOFF,0);
	}
	else
		err = STROBE_ERR_PARA_INVALID;

	if (err < 0)
	{
	    DRV_ERR("setOnOff() err=%d\n", err);
	}

	if(err==0)
		m_isOn=a_isOn;
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::setStep(int step)
{
	DRV_DBG("setStep() step = %d\n",step);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("setStep() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	err = ioctl(m_fdSTROBE,FLASH_IOC_SET_STEP,step);
	if (err < 0)
	{
	    DRV_ERR("setOnOff() err=%d\n", err);
	    return err;
	}
	m_step = step;
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::setDuty(int duty)
{
	DRV_DBG("setDuty() duty = %d\n",duty);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("setDuty() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	err = ioctl(m_fdSTROBE,FLASH_IOC_SET_DUTY,duty);
	if (err < 0)
	{
	    DRV_ERR("setDuty() err=%d\n", err);
	    return err;
	}
	m_duty = duty;
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::getFlashReg(int shift, unsigned short* a_reg)
{
	DRV_DBG("getFlashReg() shift = %d",shift);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG(" [getFlashReg] m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	/*
	if(shift==0)
		err = ioctl(m_fdSTROBE,FLASHLIGHT_GET_REG0,a_reg);
	else if(shift==1)
		err = ioctl(m_fdSTROBE,FLASHLIGHT_GET_REG1,a_reg);
	else
		err = ioctl(m_fdSTROBE,FLASHLIGHT_GET_REG2,a_reg);
		*/
	if (err < 0)
	{
	    DRV_ERR("getFlashReg() err=%d\n", err);
	}
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::setFlashReg(int shift, unsigned short reg, unsigned short mask)
{
	DRV_DBG("setFlashSpReg() shift=%d, reg=%d, mak=0x%x",shift, reg, mask);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("setFlashSpReg() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	DRV_DBG("FlashErr setFlashSpReg() code not ready, line=%d",__LINE__);
	if (err < 0)
	{
	    DRV_ERR("setFlashSpReg() err=%d\n", err);
	}
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::setTimeOutTime(int ms)
{
	DRV_DBG("setTimeOutTime(ms=%d)", ms);
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("setTimeOutTime() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	err = ioctl(m_fdSTROBE,FLASH_IOC_SET_TIME_OUT_TIME_MS,ms);
	if (err < 0)
	{
	    DRV_ERR("setTimeOutTime() err=%d\n", err);
	}
	return err;
}
int FlashlightDrv::getCoolDownTime(int* ms)
{
	DRV_DBG("getCoolDownTime()");
	int err=0;
	DRV_DBG("FlashErr setFlashSpReg() code not ready, line=%d",__LINE__);
	*ms = 0;
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::lockSensedV()
{
	DRV_DBG("lockSensedV()");
	DRV_DBG("FlashErr lockSensedV() code not ready, line=%d",__LINE__);
	int err=0;
	return err;
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int FlashlightDrv::unlockSensedV()
{
	DRV_DBG("unlockSensedV()");
	DRV_DBG("FlashErr unlockSensedV() code not ready, line=%d",__LINE__);
	int err=0;
	return err;
}
int FlashlightDrv::mapDutyStep(int peakI, int aveI, int* duty, int* step)
{
	DRV_DBG("mapDutyStep()");
	DRV_DBG("FlashErr mapDutyStep() code not ready, line=%d",__LINE__);
	int err=0;
	return err;
}
//======================================
int FlashlightDrv::setPreOn()
{
	DRV_DBG("setPreOn()");
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_ERR("setPreOn() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	err = ioctl(m_fdSTROBE,FLASH_IOC_PRE_ON,0);
	if (err < 0)
	{
	    DRV_DBG("setPreOn() not support =%d\n", err);
	}
	m_preOnTime=getMs();
	return err;
}
//======================================
int FlashlightDrv::getPreOnTimeMs(int* ms)
{
	DRV_DBG("getPreOnTimeMs()");
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_ERR("getPreOnTimeMs() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	*ms=0;
	err = ioctl(m_fdSTROBE,FLASH_IOC_GET_PRE_ON_TIME_MS, ms);
	if (err < 0)
	{
	    DRV_DBG("getPreOnTimeMs() not support ret=%d\n", err);
	}
	DRV_DBG("getPreOnTimeMs=%d", *ms);
	return err;
}


int FlashlightDrv::setReg(int reg, int val)
{
    ioctl(m_fdSTROBE,FLASH_IOC_SET_REG_ADR,reg);
    ioctl(m_fdSTROBE,FLASH_IOC_SET_REG_VAL,val);
    ioctl(m_fdSTROBE,FLASH_IOC_SET_REG,0);
    return 0;
}
int FlashlightDrv::getReg(int reg, int* val)
{
	int ret;
    ret =  ioctl(m_fdSTROBE,FLASH_IOC_GET_REG,reg);
    *val = ret;
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::hasFlashHw()
{
  DRV_DBG("hasFlashHw line=%d",__LINE__);
  if (m_fdSTROBE < 0)
  {
      DRV_DBG(" [getFlashlightType] m_fdSTROBE < 0\n");
      return 0;
  }
  if(e_CAMERA_MAIN_SENSOR==m_sensorDev)
  {
#ifdef DUMMY_FLASHLIGHT
    return 0;
#else
    return 1;
#endif
  }
  else if(e_CAMERA_SUB_SENSOR==m_sensorDev)
  {
if(cust_isSubFlashSupport()==1)
    return 1;
else
    return 0;
  }
  else
    return 0;


}

StrobeDrv::FLASHLIGHT_TYPE_ENUM FlashlightDrv::getFlashlightType() const
{
	DRV_DBG("getFlashlightType line=%d",__LINE__);
    int err = 0;

    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [getFlashlightType] m_fdSTROBE < 0\n");
        return StrobeDrv::FLASHLIGHT_NONE;
    }

    DRV_DBG("[getFlashlightType] m_flashType=%d\n",m_flashType);
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_G_FLASHTYPE,&m_flashType);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_G_FLASHTYPE error:%d\n",m_flashType);
        return StrobeDrv::FLASHLIGHT_NONE;;
    }
    return (StrobeDrv::FLASHLIGHT_TYPE_ENUM)m_flashType;
}


/*******************************************************************************
*
********************************************************************************/
FlashlightDrv::FlashlightDrv()
	  : StrobeDrv()
    , m_fdSTROBE(-1)
    , m_flashType((int)StrobeDrv::FLASHLIGHT_NONE)
    , m_strobeMode(LIB3A_AE_STROBE_MODE_UNSUPPORTED)
    , mUsers(0)
{
    DRV_DBG("FlashlightDrv()\n");
    m_isOn=0;
    m_duty=0;
    m_step=0;
    m_bTempInit=0;
}


/*******************************************************************************
*
********************************************************************************/
FlashlightDrv::~FlashlightDrv()
{
	DRV_DBG("FlashlightDrv line=%d",__LINE__);

}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::init(unsigned long sensorDev)
{
	DRV_DBG("init line=%d",__LINE__);
    int err = 0;
    unsigned long flashlightIdx = 0; //default dummy driver


    //MHAL_LOG("[halSTROBEInit]: %s \n\n", __TIME__);
    DRV_DBG("[init] mUsers = %d\n", mUsers);
    Mutex::Autolock lock(mLock);

	int preSensorDev;
    preSensorDev = m_sensorDev;
	m_sensorDev = sensorDev;

	int bOpenTest=1;
    if(m_bTempInit==1)
    {
		if(preSensorDev==m_sensorDev)
        {
	        android_atomic_dec(&mUsers);
	        bOpenTest=0;
	    }
    	else
    	{
        uninitNoLock();
    	}
        m_bTempInit=0;
    }

    if (mUsers == 0 && bOpenTest==1)
    {
        if (m_fdSTROBE == -1)
        {
        	 int ta;
        	 int tb;
        		ta = getMs();

            m_fdSTROBE = open(STROBE_DEV_NAME, O_RDWR);

            tb = getMs();



            DRV_DBG("[init] m_fdSTROBE = %d t=%d\n", m_fdSTROBE,tb-ta);

            if (m_fdSTROBE < 0)
            {
                DRV_ERR("error opening %s: %s", STROBE_DEV_NAME, strerror(errno));
                 return StrobeDrv::STROBE_UNKNOWN_ERROR;
            }

            //set flashlight driver
            DRV_DBG("[init] sensorDev = %ld\n", sensorDev);

            err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_X_SET_DRIVER,sensorDev);
            if (err < 0)
            {
                DRV_ERR("FLASHLIGHTIOC_X_SET_DRIVER error\n");
                return err ;
            }
            m_isOn=0;
            m_duty=0;
    		m_step=0;
            m_preOnTime=-1;
        }
    }
    android_atomic_inc(&mUsers);
    return StrobeDrv::STROBE_NO_ERROR;
}

int FlashlightDrv::initTemp(unsigned long sensorDev)
{
	DRV_DBG("initTemp line=%d",__LINE__);
    int err = 0;
    unsigned long flashlightIdx = 0; //default dummy driver
    

    //MHAL_LOG("[halSTROBEInit]: %s \n\n", __TIME__);
    DRV_DBG("[initTemp] mUsers = %d\n", mUsers);
    Mutex::Autolock lock(mLock);
    if (mUsers > 0)
    {
    	int preSensorDev;
	    preSensorDev = m_sensorDev;
		m_sensorDev = sensorDev;	    
		if(preSensorDev==m_sensorDev)
	    {
	    		        
    }
    	else
    	{
    		uninitNoLock();
    	}
    }
    m_sensorDev = sensorDev;
    
    
    if (mUsers == 0)
    {
        if(sensorDev==0)
            sensorDev=e_CAMERA_MAIN_SENSOR;
        if (m_fdSTROBE == -1)
        {
        	 int ta;
        	 int tb;
        		ta = getMs();

            m_fdSTROBE = open(STROBE_DEV_NAME, O_RDWR);

            tb = getMs();



            DRV_DBG("[init] m_fdSTROBE = %d t=%d\n", m_fdSTROBE,tb-ta);

            if (m_fdSTROBE < 0)
            {
                DRV_ERR("error opening %s: %s", STROBE_DEV_NAME, strerror(errno));
                 return StrobeDrv::STROBE_UNKNOWN_ERROR;
            }

            //set flashlight driver
            DRV_DBG("[init] sensorDev = %ld\n", sensorDev);

            err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_X_SET_DRIVER,sensorDev);
            if (err < 0)
            {
                DRV_ERR("FLASHLIGHTIOC_X_SET_DRIVER error\n");
                return err ;
            }
            m_isOn=0;
            m_duty=0;
    		m_step=0;
            m_preOnTime=-1;
        }
        android_atomic_inc(&mUsers);
        m_bTempInit=1;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::uninit()
{
	//DRV_DBG("uninit line=%d",__LINE__);
    //MHAL_LOG("[halSTROBEUninit] \n");
    //DRV_DBG("[uninit] mUsers = %d\n", mUsers);

    Mutex::Autolock lock(mLock);
    return uninitNoLock();
}

int FlashlightDrv::uninitNoLock()
{
    DRV_DBG("uninitNoLock user=%d",mUsers);
    //MHAL_LOG("[halSTROBEUninit] \n");


    if (mUsers == 0)
    {
        DRV_DBG("[uninitNoLock] mUsers = %d\n", mUsers);
    }

     if (mUsers == 1)
     {
        if (m_fdSTROBE > 0)
        {
            close(m_fdSTROBE);
        }
        m_fdSTROBE = -1;
    }

    android_atomic_dec(&mUsers);
    m_bTempInit=0;

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setFire(unsigned long a_fire)
{
	DRV_DBG("setFire line=%d",__LINE__);
    int err = 0;
    unsigned long  fire = 0;

    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setFire] m_fdSTROBE < 0\n");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    DRV_DBG("[FlashlightDrv] setFire = %lu line=%d",a_fire,__LINE__);
    fire = (0 == a_fire) ? 0 : 1;
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_ENABLE,fire);

    if (err < 0)
    {
        //DRV_ERR("FLASHLIGHTIOC_T_ENABLE error\n");
        DRV_DBG("[FlashlightDrv] setFire ERR line=%d",__LINE__);
        return err ;
    }

    DRV_DBG("[FlashlightDrv] setFire OK line=%d",__LINE__);

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setLevel(unsigned long  a_level)
{
	DRV_DBG("setLevel line=%d",__LINE__);
    int err = 0;

    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setLevel] m_fdSTROBE < 0\n");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    DRV_DBG("[FlashlightDrv] setLevel = %lu\n",a_level);
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_LEVEL,a_level);

    DRV_DBG("[FlashlightDrv] setLevel err = %d, line=%d\n",err, __LINE__);


    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_ENABLE error;%ld\n",a_level);
        return err;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setTimeus(unsigned long a_timeus)
{
	DRV_DBG("setTimeus line=%d",__LINE__);
    int err = 0;
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setTimeus] m_fdSTROBE < 0\n");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_FLASHTIME,a_timeus);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_ENABLE error\n");
        return err;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setStartTimeus(unsigned long a_timeus)
{
	DRV_DBG("setStartTimeus line=%d",__LINE__);
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setState(unsigned long  a_state)
{
	DRV_DBG("setState line=%d",__LINE__);
    int err = 0;
    unsigned long strobeState = 0;

    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setState] m_fdSTROBE < 0\n");
        return 0;
    }

    strobeState = (0 == a_state) ? (unsigned long)FLASHLIGHTDRV_STATE_PREVIEW : (unsigned long)FLASHLIGHTDRV_STATE_STILL;
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_STATE,strobeState);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_STATE error\n");
        return err;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setFlashlightModeConf(unsigned long a_strobeMode)
{
	DRV_DBG("setFlashlightModeConf line=%d",__LINE__);
    int err = StrobeDrv::STROBE_NO_ERROR;
    static bool torchOn = false;    // cotta : added for 555 (torch) mode

    //cotta : modified for 555 (torch) mode
    if (StrobeDrv::FLASHLIGHT_NONE == m_flashType)
    {
        if(a_strobeMode != 555)
        {
            if(torchOn == true)
            {
                DRV_DBG("[setFlashlightModeConf] Torch mode off\n");
                torchOn = false;
                err = FlashlightDrv::setFire(0);
                return err;
            }
            else
            {
                DRV_DBG("[setFlashlightModeConf] FLASHLIGHT_NONE\n");
                return StrobeDrv::STROBE_NO_ERROR;
            }
        }
    }

    m_strobeMode = a_strobeMode;

    DRV_DBG("[setFlashlightModeConf] a_strobeMode: %d \n",m_strobeMode);

    switch(m_strobeMode)
    {
        case LIB3A_AE_STROBE_MODE_AUTO:
        //case LIB3A_AE_STROBE_MODE_SLOWSYNC:
            break;
        case LIB3A_AE_STROBE_MODE_FORCE_ON:
                //err = FlashlightDrv::setState(1); //still capture state
                //err = FlashlightDrv::setLevel(31); //max level
            if (StrobeDrv::FLASHLIGHT_LED_TORCH == m_flashType)
            {
                err = FlashlightDrv::setFire(1);
            }
            break;

        //FIXME: create an official control path for torch light
        case 555://LIB3A_AE_STROBE_MODE_FORCE_TORCH:
            err = FlashlightDrv::setLevel(1);
            err = FlashlightDrv::setFire(1);
            torchOn = true; //cotta : added for 555 (torch) mode
            break;

        case LIB3A_AE_STROBE_MODE_REDEYE:
            break;
        case LIB3A_AE_STROBE_MODE_FORCE_OFF:
        default:
            err = FlashlightDrv::setFire(0);

            if(torchOn == true) //cotta : added for 555 (torch) mode
            {
                torchOn = false;
            }

            break;
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setCaptureFlashlightConf(unsigned long a_strobeWidth)
{
	DRV_DBG("setCaptureFlashlightConf line=%d",__LINE__);
    int err = StrobeDrv::STROBE_NO_ERROR;

    DRV_DBG("[setCaptureFlashlightConf] m_flashType = %d, width=%lu\n",m_flashType,a_strobeWidth);

    if (StrobeDrv::FLASHLIGHT_LED_PEAK == m_flashType)
    {
        DRV_DBG("[setCaptureFlashlightConf] FLASHLIGHT_LED_PEAK\n");
        err = FlashlightDrv::setState(1); //still capture state
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setState fail\n");
        }
        err = FlashlightDrv::setLevel(a_strobeWidth);
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setLevel fail\n");
        }
    }
    else if (StrobeDrv::FLASHLIGHT_LED_CONSTANT == m_flashType && 0 != a_strobeWidth )
             /*LIB3A_AE_STROBE_MODE_FORCE_ON == m_strobeMode) */
    {
        err = FlashlightDrv::setState(1); //still capture state
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setState fail\n");
        }

        err = FlashlightDrv::setLevel(a_strobeWidth); //set level
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setLevel fail\n");
        }

        err = FlashlightDrv::setFire(1);
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setFire fail\n");
        }
    }
    else
    {
        DRV_DBG("[setCaptureFlashlightConf] No config\n");
    }

    return err;
}


/*******************************************************************************
 Author : Cotta
 Functionality : set value of sensor capture delay
********************************************************************************/
int FlashlightDrv::setCaptureDelay(unsigned int value)
{
	DRV_DBG("setCaptureDelay line=%d",__LINE__);
    int err = 0;

    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setCaptureDelay] m_fdSTROBE < 0\n");
        return 0;
    }

    DRV_DBG(" [setCaptureDelay] set capture delay : %u\n",value);

    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_DELAY,value);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_DELAY error : %u\n",value);
        return err;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
 Author : Cotta
 Functionality : get value of strobe watch dog timer
********************************************************************************/
int FlashlightDrv::getStrobeWDTValue(unsigned int *pValue)
{
	DRV_DBG("getStrobeWDTValue line=%d",__LINE__);
    *pValue = FLASH_LIGHT_WDT_TIMEOUT_MS;

    DRV_DBG("[getStrobeWDTValue] WDT value = %u\n",*pValue);
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
 Author : Cotta
 Functionality : command control
********************************************************************************/
int FlashlightDrv::sendCommand(unsigned int cmd, unsigned int pArg1, unsigned int *pArg2, unsigned int *pArg3)
{
	DRV_DBG("sendCommand line=%d",__LINE__);
    int err = 0;

    switch(cmd)
    {
        case CMD_STROBE_SET_CAP_DELAY :
            err = setCaptureDelay(pArg1);   // set capture delay to strobe kernel driver
            break;
        case CMD_STROBE_GET_WDT_VALUE :
            err = getStrobeWDTValue(pArg2); // get WDT value
            break;
        default :
            DRV_ERR("[strobe sendCommand] no commadn support\n");
            return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    if (err < 0)
    {
        DRV_ERR("[strobe sendCommand] Err-ctrlCode(%x)\n",err);
        return err;
    }

    return err;
}

int FlashlightDrv::getDuty(int* duty)
{
	DRV_DBG("getDuty()\n");
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("getDuty() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	*duty = m_duty;
	return err;

}

int FlashlightDrv::getStep(int* step)
{
	DRV_DBG("getStep()\n");
	int err = 0;
	if (m_fdSTROBE < 0)
	{
	    DRV_DBG("getStep() m_fdSTROBE < 0\n");
	    return StrobeDrv::STROBE_UNKNOWN_ERROR;
	}
	*step = m_step;
	return err;

}



int FlashlightDrv::getPartId(int sensorDev)
{
	Mutex::Autolock lock(mLock);
	int dev;
	int err;
	int value;
	dev = open(STROBE_DEV_NAME, O_RDWR);
	if(e_CAMERA_MAIN_SENSOR==sensorDev)
		err = ioctl(dev,FLASH_IOC_GET_MAIN_PART_ID,&value);
	else if(e_CAMERA_SUB_SENSOR==sensorDev)
		err = ioctl(dev,FLASH_IOC_GET_SUB_PART_ID,&value);
	else //MAIN2
		err = ioctl(dev,FLASH_IOC_GET_MAIN2_PART_ID,&value);
	DRV_DBG("getPartId dev=%d id=%d line=%d",sensorDev, value, __LINE__);
	close(dev);
	return value;
}


