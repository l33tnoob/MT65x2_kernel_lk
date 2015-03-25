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
#ifndef _SENSOR_HAL_IMP_H_
#define _SENSOR_HAL_IMP_H_


#include <utils/Errors.h>
#include <cutils/log.h>

#include "mtkcam/hal/sensor_hal.h"
#include "sensor_drv.h"
#include <mtkcam/exif/IBaseCamExif.h>


using namespace android;

#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#endif
#define AEE_ASSERT(exp) \
    do { \
        if( !(exp) ) { \
            LOG_ERR("ASSERT("#exp") fail"); \
            aee_system_exception("libcamdrv.so", NULL, DB_OPT_DEFAULT, "%s, %uL", __FILE__, __LINE__ ); \
        } \
    } while(0)
#else
#include "uvvf.h"

#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_ERRMSG("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)    
#define LOG_WRN(fmt, arg...)    
#define LOG_ERR(fmt, arg...)    
#endif   

#define AEE_ASSERT(exp)
#endif

/*******************************************************************************
*
********************************************************************************/
class SensorHalImp : public SensorHal {
public:
    static SensorHal* getInstance();
    virtual void destroyInstance();
//
private:
    SensorHalImp();
    virtual ~SensorHalImp();
//
public:
    virtual MINT32 searchSensor();
    //
    virtual MINT32 init();
    //
    virtual MINT32 uninit();
    //
    virtual MINT32 setATVStart();
	//
    virtual MINT32 setConf(halSensorIFParam_t halSensorIFParam[2]);
    //
	virtual MINT32 waitSensorEventDone(MUINT32 EventType, MUINT32 Timeout);
	//
    virtual MINT32 sendCommand(
    	halSensorDev_e cameraId,
        int cmd,
        int arg1 = 0,
        int arg2 = 0,
        int arg3 = 0);
    //
    virtual MINT32 dumpReg();  
	//
	virtual MINT32 setDebugInfo(IBaseCamExif *pIBaseCamExif);
//
	virtual MINT32 reset();	
//
private:
    MINT32 mSensorDev;
	static MINT32 mSearchSensorDev;
    halSensorType_e mIspSensorType[2];
    MINT32 mImageSensorType[2];
    mutable Mutex mImpLock;
    mutable Mutex mLock;
    volatile int mUsers;
    volatile int mInit;
	ACDK_SCENARIO_ID_ENUM mSensorScenarioId[2];
	SENSOR_DEV_ENUM mCameraId[2];	
    //
    MINT32 createImp();
    //
    MINT32 deleteImp();
    //
    MINT32 initSensor();
    //
    MINT32 openSensor();
    //
    MINT32 getSensorInfo(ACDK_SCENARIO_ID_ENUM mode[2]);
    //
    MINT32 setTgPhase();
    //
    MINT32 initCSI2Peripheral(MINT32 initCSI2);
    //
    MINT32 setCSI2Config(MINT32 enableCSI2);
    //
    MINT32 setSensorIODrivingCurrent();
    //
    MINT32 getRawInfo(halSensorDev_e sensorDevId,halSensorRawImageInfo_t *pinfo, MINT32 mode = 0);
    //
    MINT32 querySensorInfo();

};

#endif //_SENSOR_HAL_IMP_H_

