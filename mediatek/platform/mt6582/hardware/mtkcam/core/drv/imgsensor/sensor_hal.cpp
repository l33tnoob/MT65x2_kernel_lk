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
#define LOG_TAG "SensorHal"

#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include "mtkcam/common.h"
#include <mtkcam/exif/IBaseCamExif.h>
#include <dbg_cam_param.h>
#include "sensor_hal_imp.h"
#include "sensor_drv.h"
#include "seninf_drv.h"


/*******************************************************************************
*
********************************************************************************/
#define ISP_RAW_WIDTH_PADD      (4)
#define ISP_RAW_HEIGHT_PADD     (6)
typedef struct  {
    MUINT32 u4GrabX;          // For input sensor width 
    MUINT32 u4GrabY;          // For input sensor height 
    MUINT32 u4CropW;		//TG crop width
	MUINT32 u4CropH;	    //TG crop height
}SENSOREXIFDEBUG_STURCT;
//

/*******************************************************************************
*
********************************************************************************/
static SensorDrv *pSensorDrv = NULL;
static SeninfDrv *pSeninfDrv = NULL;

#define SENSOR_NUM 2
static ACDK_SCENARIO_ID_ENUM scenarioId[2] = {ACDK_SCENARIO_ID_CAMERA_PREVIEW,ACDK_SCENARIO_ID_CAMERA_PREVIEW};// 1:for main/atv, 2:for main_2/sub
static ACDK_SENSOR_INFO_STRUCT sensorInfo[2];// 1:for main/atv, 2:for main_2/sub
static ACDK_SENSOR_CONFIG_STRUCT sensorCfg[2];// 1:for main/atv, 2:for main_2/sub;
static ACDK_SENSOR_RESOLUTION_INFO_STRUCT sensorResolution[2];// 1:for main/atv, 2:for main_2/sub
static ACDK_SCENARIO_ID_ENUM curScenario = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
static SENSOR_CROP_INFO sensorCropInfo[SENSOR_NUM];
MINT32 SensorHalImp::mSearchSensorDev = SENSOR_DEV_NONE;
static SENSOREXIFDEBUG_STURCT sensorDebug[2];
static ACDK_SCENARIO_ID_ENUM previousScenarioId[SENSOR_NUM]={ACDK_SCENARIO_ID_CAMERA_PREVIEW,ACDK_SCENARIO_ID_CAMERA_PREVIEW};//0:main,1:sub,2:main_2
static MUINT32 previousExposureTime[SENSOR_NUM] = {33000,33000};//unit: us 

//
static ACDK_SENSOR_RESOLUTION_INFO_STRUCT staticSensorResoultion[SENSOR_NUM+1];
static halSensorType_e staticSensorType[SENSOR_NUM+1];
static halSensorRawImageInfo_t staticSensorRawInfo[SENSOR_NUM+1][2];
static SENSOR_GRAB_INFO_STRUCT staticSensorGrabInfo[SENSOR_NUM+1][ACDK_SCENARIO_ID_MAX-1];//8 scenarios

/*******************************************************************************
*
********************************************************************************/
SensorHal* SensorHal::createInstance()
{
    return SensorHalImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SensorHal* SensorHalImp::getInstance()
{
    MINT32 ret;
    
    LOG_MSG("[SensorHalImp] getInstance \n");
    static SensorHalImp singleton;

    ret = singleton.createImp();
    if (ret < 0) {
        return NULL;
    }

    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void SensorHalImp::destroyInstance() 
{
    deleteImp();    
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::createImp()
{
    MINT32 ret = 0;

    LOG_MSG("[createImp]: %d \n", mUsers);
    //
    Mutex::Autolock lock(mImpLock);
    //
    if (mUsers > 0) {
        LOG_MSG("  Has created \n");
        android_atomic_inc(&mUsers);
        return 0;
    }
    //    
    mSensorDev = SENSOR_DEV_MAIN;
    mIspSensorType[0] = SENSOR_TYPE_UNKNOWN;
    mImageSensorType[0] = IMAGE_SENSOR_TYPE_UNKNOWN;
    mIspSensorType[1] = SENSOR_TYPE_UNKNOWN;
    mImageSensorType[1] = IMAGE_SENSOR_TYPE_UNKNOWN;

    pSeninfDrv = SeninfDrv::createInstance();
    if (!pSeninfDrv) {
        LOG_ERR("SeninfDrv::createInstance fail \n");
        ret = -1;
        goto createImp_exit;
    }
    //
    
    android_atomic_inc(&mUsers);

    return ret;
    //
createImp_exit:
    if (pSeninfDrv) {
        pSeninfDrv->destroyInstance();
        pSeninfDrv = NULL;
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::deleteImp()
{
    MINT32 ret = 0;

    LOG_MSG("[deleteImp]: %d \n", mUsers);
    //
    Mutex::Autolock lock(mImpLock);
    //
    if (mUsers <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if (mUsers == 0) {
        uninit();
        mInit = 0;
        if (pSeninfDrv) {
            pSeninfDrv->destroyInstance();
            pSeninfDrv = NULL;
        }
       
    }
    else {
        LOG_MSG("  Still users \n");
    }

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
SensorHalImp::SensorHalImp() :
    SensorHal()
{
}

/*******************************************************************************
*
********************************************************************************/
SensorHalImp::~SensorHalImp()
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::dumpReg()
{
    MINT32 ret = 0;
    
    LOG_MSG("[dumpReg]");

	ret = pSeninfDrv->dumpReg();
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::searchSensor()
{
    MINT32 sensorDevs = 0;
    MINT32 ret;
    MINT32 clkCnt = 1;
    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_search, CPTFlagStart); 
    #endif

    ret = pSeninfDrv->init();
    if (ret < 0) {
        LOG_ERR("pSeninfDrv->init() fail\n");
        return 0;
    }

    //ret = pSeninfDrv->autoDeskewCalibration(); //mipi calibration

    
    memset(&sensorInfo[0], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&sensorInfo[1], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));    
    memset(&sensorCfg[0], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    memset(&sensorCfg[1], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));    
    
    // Before searching sensor, need to turn on clock of TG
    // Config TG, always use Camera PLL, 1: 48MHz, clkCnt is 1,  48MHz /2 = 24MHz

    //ToDo: select  PLL group     
    ret = pSeninfDrv->setTg1PhaseCounter(
        1, CAM_PLL_48_GROUP, /*sensorInfo.SensorMasterClockSwitch,*/
        clkCnt,  sensorInfo[0].SensorClockPolarity ? 0 : 1,
        1, 0, 0);        
    if (ret < 0) {
        LOG_ERR("setTg1PhaseCounter fail\n");
        return 0;
    }

        
     
    // Search sensor
    mSearchSensorDev = SensorDrv::searchSensor(NULL);
    //
    //get sensor info
    querySensorInfo();


    ret = pSeninfDrv->uninit();
    if (ret < 0) {
        LOG_ERR("pSeninfDrv->uninit() fail\n");
    }
    LOG_MSG("[searchSensor] sensorDevs = 0x%x\n",mSearchSensorDev);

    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_search, CPTFlagEnd);
    #endif
    return mSearchSensorDev;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::init()
{
    MINT32 ret = 0;
        
    LOG_MSG("[init]: %d \n", mInit);
    //
    Mutex::Autolock lock(mLock);

    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_open, CPTFlagStart);
    #endif
    //
    if (mInit > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mInit);
        return 0;
    }
    //after set mSenseorDev, determine sensor or ATV
    pSensorDrv = SensorDrv::createInstance(mSensorDev); 

    //
    ret = pSeninfDrv->init();
    if (ret < 0) {
        LOG_ERR("pSeninfDrv->init() fail \n");
        return ret;
    }
    //
    if (mSensorDev == SENSOR_DEV_NONE) {
        LOG_MSG("mSensorDev is NONE \n");
        return ret;
    }
    //
    //pSensorDrv = SensorDrv::createInstance(mSensorDev); 
    // Before searching sensor, need to turn on TG
    ret = initSensor();
    if (ret < 0) {
        LOG_ERR("initSensor fail \n");
        return ret;
    }
    // Get sensor info before setting TG phase counter
    ret = getSensorInfo(scenarioId);
    if (ret < 0) {
        LOG_ERR("getSensorInfo fail \n");
        return ret;
    }
    //
    ret = setTgPhase();
    if (ret < 0) {
        LOG_ERR("setTgPhase fail \n");
        return ret;
    }
    //
    ret = setSensorIODrivingCurrent();
    if (ret < 0) {
        LOG_ERR("initial IO driving current fail \n");
        return ret;
    }
    //
    ret = initCSI2Peripheral(1);  // if the interface is mipi, enable the csi2
    if (ret < 0) {
        LOG_ERR("initial CSI2 peripheral fail \n");
        return ret;
    }
    //
    ret = setCSI2Config(1);     // enable and config CSI2.
    if (ret < 0) {
        LOG_ERR("set CSI2 config fail \n");
        return ret;
    }
#ifdef ATV_SUPPORT
#ifdef MTK_MATV_SERIAL_IF_SUPPORT
    pSeninfDrv->initTg1Serial(MFALSE);
#endif
#endif
    // Open sensor, try to open 3 time
    for (int i =0; i < 3; i++) {
        ret = pSensorDrv->open();
        if (ret < 0) {
            LOG_ERR("pSensorDrv->open fail, retry = %d \n", i);
        }
        else {
            break; 
        }
    }

    if (ret < 0) {
        LOG_ERR("pSensorDrv->open fail\n");
        return ret;
    }


    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_open, CPTFlagEnd);
    #endif
    
    android_atomic_inc(&mInit);

    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::uninit()
{
    MINT32 ret = 0;

    LOG_MSG("[uninit]: %d \n", mInit);
    //
    Mutex::Autolock lock(mLock);
    #ifndef USING_MTK_LDVT    
    //CPTLog(Event_Sensor_close, CPTFlagStart);
    #endif
    //
    if (mInit <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mInit);
    //
    if (mInit == 0) {
        //
        ret = initCSI2Peripheral(0);  // if the interface is mipi, disable the csi2
        if (ret < 0) {
            LOG_ERR("initial CSI2 peripheral fail \n");
            return ret;
        }
#ifdef ATV_SUPPORT
#ifdef MTK_MATV_SERIAL_IF_SUPPORT
        pSeninfDrv->initTg1Serial(MFALSE);
#endif
#endif
        //
        if (pSensorDrv) {
            pSensorDrv->close();
            pSensorDrv->uninit();
            pSensorDrv->destroyInstance();
            pSensorDrv = NULL;
        }
        //
        if ( pSeninfDrv ) {
            ret = pSeninfDrv->uninit();
            if (ret < 0) {
                LOG_ERR("pSeninfDrv->uninit() fail\n");
                return ret;
            }
        }
    }
    else {
        LOG_MSG("  Still users \n");
    }
    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_close, CPTFlagEnd);
    #endif
    
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::setATVStart()
{
    MINT32 ret = 0;
    
    LOG_MSG("[setMCLKEn]\n");

    
    if( mSensorDev == SENSOR_DEV_ATV )
    {
        ret = pSeninfDrv->setTg1MCLKEn(0);
        if (ret < 0) {
            LOG_ERR("CloseMclk fail\n");
            return ret;
        }
    }
    
    return ret;   
}



/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::setConf(halSensorIFParam_t halSensorIFParam[2])
{
    MINT32 ret = 0;
    MINT32 pixelX0[2], pixelY0[2], pixelX1[2], pixelY1[2];
    ACDK_SCENARIO_ID_ENUM sensorScenarioId[2]={ACDK_SCENARIO_ID_MAX,ACDK_SCENARIO_ID_MAX};
    MUINT32 inDataFmt = 0;
    SENSOR_DEV_ENUM eSensorDev = SENSOR_NONE;
	PAD2CAM_DATA_ENUM padSel = PAD_10BIT;
	SENINF_SOURCE_ENUM inSrcTypeSel = PARALLEL_SENSOR;
    TG_FORMAT_ENUM inDataType = RAW_10BIT_FMT;
    SENSOR_DATA_BITS_ENUM senInLsb = TG_10BIT;
    SENSOR_DEV_ENUM cameraId[2];

        
    LOG_MSG("[setConf] main/atv src: %d/%d , sub src: %d/%d\n", halSensorIFParam[0].u4SrcW, halSensorIFParam[0].u4SrcH, halSensorIFParam[1].u4SrcW, halSensorIFParam[1].u4SrcH);

    LOG_MSG(" main/atv u4IsBypassSensorScenario:%d, u4IsBypassSensorDelay:%d \n", halSensorIFParam[0].u4IsBypassSensorScenario, halSensorIFParam[0].u4IsBypassSensorDelay);
    LOG_MSG(" sub u4IsBypassSensorScenario:%d, u4IsBypassSensorDelay:%d \n", halSensorIFParam[1].u4IsBypassSensorScenario, halSensorIFParam[1].u4IsBypassSensorDelay);

    #ifndef USING_MTK_LDVT    
    //CPTLog(Event_Sensor_setScenario, CPTFlagStart);
    #endif

    
    if(mSensorDev & SENSOR_DEV_MAIN ) {
        sensorScenarioId[0] = halSensorIFParam[0].scenarioId;
        curScenario = halSensorIFParam[0].scenarioId;
        cameraId[0] = SENSOR_MAIN;     
        if(halSensorIFParam[0].u4IsBypassSensorScenario != 0) {
            goto setConf_exit;
        }
    }

    if(mSensorDev & SENSOR_DEV_ATV ) {
        sensorScenarioId[0] = halSensorIFParam[0].scenarioId;
        curScenario = halSensorIFParam[0].scenarioId;
        cameraId[0] = SENSOR_ATV;    
        if(halSensorIFParam[0].u4IsBypassSensorScenario != 0) {
            goto setConf_exit;
        }        
    }

    if(mSensorDev & SENSOR_DEV_SUB ) {
        sensorScenarioId[1] = halSensorIFParam[1].scenarioId;
        curScenario = halSensorIFParam[1].scenarioId;
        cameraId[1] = SENSOR_SUB;
        if(halSensorIFParam[1].u4IsBypassSensorScenario != 0) {
            goto setConf_exit;
        }
        
    }

    mSensorScenarioId[0] = sensorScenarioId[0];
    mSensorScenarioId[1] = sensorScenarioId[1];
    mCameraId[0] = cameraId[0];
    mCameraId[1] = cameraId[1]; 


    
    if (mSensorDev != SENSOR_DEV_NONE) {
        ret = getSensorInfo(sensorScenarioId);
        if (ret < 0) {
            LOG_ERR("getSensorInfo fail \n");
            goto setConf_exit;
        }
    }
    

    ret = pSensorDrv->setScenario(sensorScenarioId,cameraId);
    if (ret < 0) {
        LOG_ERR("halSensorSetScenario fail \n");
        goto setConf_exit;
    }

    pSeninfDrv->resetSeninf();

    //main  sensor
    if(mSensorDev & SENSOR_DEV_MAIN || mSensorDev & SENSOR_DEV_ATV) {
        eSensorDev = SENSOR_MAIN;
        
        if((halSensorIFParam[0].u4SrcW < halSensorIFParam[0].u4CropW)||(halSensorIFParam[0].u4SrcH < halSensorIFParam[0].u4CropH)||
            (halSensorIFParam[0].u4CropW == 0) || (halSensorIFParam[0].u4CropH == 0)) {
            LOG_ERR("SENSOR_DEV_MAIN  SetConf crop width or height incorrect, srcW =%d, srcH =%d, cropW =%d, cropH = %d \n",
                halSensorIFParam[0].u4SrcW,halSensorIFParam[0].u4SrcH,halSensorIFParam[0].u4CropW,halSensorIFParam[0].u4CropH);
            ret = -1;
            goto setConf_exit;
        }


        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, &inDataFmt);
        sensorCropInfo[0].u4GrabX = sensorInfo[0].SensorGrabStartX;
        sensorCropInfo[0].u4GrabY = sensorInfo[0].SensorGrabStartY;
        sensorCropInfo[0].u4SrcW = halSensorIFParam[0].u4SrcW;
        sensorCropInfo[0].u4SrcH = halSensorIFParam[0].u4SrcH;
        sensorCropInfo[0].u4CropW = halSensorIFParam[0].u4CropW;
        sensorCropInfo[0].u4CropH = halSensorIFParam[0].u4CropH;
        sensorCropInfo[0].DataFmt = (MUINT32)sensorInfo[0].SensorOutputDataFormat;
        // Source is from sensor
        if (mImageSensorType[0] == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + ((halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW)>>1);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		padSel = PAD_10BIT;
    		inDataType = RAW_10BIT_FMT;
    		senInLsb = TG_12BIT;
        }
        else if (mImageSensorType[0] == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + ((halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW)>>1);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RAW_8BIT_FMT;
    		senInLsb = TG_12BIT;
        }        
        else if ((mImageSensorType[0] == IMAGE_SENSOR_TYPE_YUV)||(mImageSensorType[0] == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + (halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW * 2;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = YUV422_FMT;
    		senInLsb = TG_8BIT;
        }
    	else if(mImageSensorType[0] == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + (halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW * 2;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RGB565_MIPI_FMT;
    		senInLsb = TG_8BIT;

    	}
    	else if(mImageSensorType[0] == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + (halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW * 2;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RGB888_MIPI_FMT;
    		senInLsb = TG_8BIT;

    	}
    	else if(mImageSensorType[0] == IMAGE_SENSOR_TYPE_JPEG) {
            pixelX0[0] = sensorInfo[0].SensorGrabStartX + (halSensorIFParam[0].u4SrcW - halSensorIFParam[0].u4CropW);
            pixelY0[0] = sensorInfo[0].SensorGrabStartY + ((halSensorIFParam[0].u4SrcH - halSensorIFParam[0].u4CropH)>>1);
            pixelX1[0] = pixelX0[0] + halSensorIFParam[0].u4CropW * 2;
            pixelY1[0] = pixelY0[0] + halSensorIFParam[0].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = JPEG_FMT;
    		senInLsb = TG_8BIT;

    	}	
    	else  {

    	}	

    	if (sensorInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
    		inSrcTypeSel = MIPI_SENSOR;
            //pSeninfDrv->autoDeskewCalibration(); //do calibration
    	}
    	else if (sensorInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
    		inSrcTypeSel = PARALLEL_SENSOR;
    	}
        else if (sensorInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            inSrcTypeSel = SERIAL_SENSOR;
        }
    	else {
    		inSrcTypeSel = TEST_MODEL;
    	}
    	

        ret = pSeninfDrv->setTg1GrabRange(pixelX0[0], pixelX1[0], pixelY0[0], pixelY1[0]);
        if (ret < 0) {
            LOG_ERR("setTg1GrabRange fail\n");
            goto setConf_exit;
        }
        //
        ret = pSeninfDrv->setTg1SensorModeCfg(sensorInfo[0].SensorHsyncPolarity ? 0 : 1, 
                                        sensorInfo[0].SensorVsyncPolarity ? 0 : 1);
        if (ret < 0) {
            LOG_ERR("setTg1SensorModeCfg fail\n");
            goto setConf_exit;
        }


   
        ret = pSeninfDrv->setTg1InputCfg(padSel, inSrcTypeSel, inDataType, senInLsb);
        if (ret < 0) {
            LOG_ERR("setTg1InputCfg fail\n");
            goto setConf_exit;
        }


        // Set view finder mode
        ret = pSeninfDrv->setTg1ViewFinderMode(halSensorIFParam[0].u4IsContinous ? 0 : 1, halSensorIFParam[0].u4IsBypassSensorDelay ? 0 : sensorInfo[0].CaptureDelayFrame);
        if (ret < 0) {
            LOG_ERR("setViewFinderMode fail\n");
            goto setConf_exit;
        }

        if((mImageSensorType[0] == IMAGE_SENSOR_TYPE_RAW) || (mImageSensorType[0] == IMAGE_SENSOR_TYPE_RAW8)){
            if(curScenario != previousScenarioId[0]) {
                sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_SET_SENSOR_EXP_TIME, (int)&previousExposureTime[0],NULL,NULL);
                LOG_MSG("set Exposure Time = %d, curScenario = %d, previousScenario = %d\n",previousExposureTime[0],curScenario,previousScenarioId[0]);

            }
        }
        previousScenarioId[0] = curScenario;
        
    }




    // SUB sensor
    if(mSensorDev & SENSOR_DEV_SUB ) {
        eSensorDev = SENSOR_SUB;
        if((halSensorIFParam[1].u4SrcW < halSensorIFParam[1].u4CropW)||(halSensorIFParam[1].u4SrcH < halSensorIFParam[1].u4CropH)||
            (halSensorIFParam[1].u4CropW == 0) || (halSensorIFParam[1].u4CropH == 0)) {
            LOG_ERR("SENSOR_DEV_MAIN_2 or SENSOR_DEV_SUB SetConf crop width or height incorrect, srcW =%d, srcH =%d, cropW =%d, cropH = %d \n",
                halSensorIFParam[1].u4SrcW,halSensorIFParam[1].u4SrcH,halSensorIFParam[1].u4CropW,halSensorIFParam[1].u4CropH);
            ret = -1;
            goto setConf_exit;        
        }        
        
        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, &inDataFmt);
        sensorCropInfo[1].u4GrabX = sensorInfo[1].SensorGrabStartX;
        sensorCropInfo[1].u4GrabY = sensorInfo[1].SensorGrabStartY;
        sensorCropInfo[1].u4SrcW = halSensorIFParam[1].u4SrcW;
        sensorCropInfo[1].u4SrcH = halSensorIFParam[1].u4SrcH;
        sensorCropInfo[1].u4CropW = halSensorIFParam[1].u4CropW;
        sensorCropInfo[1].u4CropH = halSensorIFParam[1].u4CropH;
        sensorCropInfo[1].DataFmt = (MUINT32)sensorInfo[1].SensorOutputDataFormat;
        // Source is from sensor
        if (mImageSensorType[1] == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            pixelX0[1] = sensorInfo[1].SensorGrabStartX + ((halSensorIFParam[1].u4SrcW - halSensorIFParam[1].u4CropW)>>1);
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		padSel = PAD_10BIT;
    		inDataType = RAW_10BIT_FMT;
    		senInLsb = TG_12BIT;
        }
        else if (mImageSensorType[1] == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            pixelX0[1] = sensorInfo[1].SensorGrabStartX + ((halSensorIFParam[1].u4SrcW - halSensorIFParam[1].u4CropW)>>1);
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RAW_8BIT_FMT;
    		senInLsb = TG_12BIT;
        }        
        else if ((mImageSensorType[1] == IMAGE_SENSOR_TYPE_YUV)||(mImageSensorType[1] == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            pixelX0[1] = sensorInfo[1].SensorGrabStartX;
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW * 2;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = YUV422_FMT;
    		senInLsb = TG_8BIT;
        }
    	else if(mImageSensorType[1] == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            pixelX0[1] = sensorInfo[1].SensorGrabStartX + (halSensorIFParam[1].u4SrcW - halSensorIFParam[1].u4CropW);
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW * 2;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RGB565_MIPI_FMT;
    		senInLsb = TG_8BIT;

    	}
    	else if(mImageSensorType[1] == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            pixelX0[1] = sensorInfo[1].SensorGrabStartX + (halSensorIFParam[1].u4SrcW - halSensorIFParam[1].u4CropW);
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW * 2;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = RGB888_MIPI_FMT;
    		senInLsb = TG_8BIT;

    	}
    	else if(mImageSensorType[1] == IMAGE_SENSOR_TYPE_JPEG) {
            pixelX0[1] = sensorInfo[1].SensorGrabStartX + (halSensorIFParam[1].u4SrcW - halSensorIFParam[1].u4CropW);
            pixelY0[1] = sensorInfo[1].SensorGrabStartY + ((halSensorIFParam[1].u4SrcH - halSensorIFParam[1].u4CropH)>>1);
            pixelX1[1] = pixelX0[1] + halSensorIFParam[1].u4CropW * 2;
            pixelY1[1] = pixelY0[1] + halSensorIFParam[1].u4CropH;
    		if (inDataFmt == 0) {
    	  		padSel = PAD_8BIT_9_2;
    		}
    		else {
    			padSel = PAD_8BIT_7_0;
    		}
    		inDataType = JPEG_FMT;
    		senInLsb = TG_8BIT;

    	}	
    	else  {

    	}	

    	if (sensorInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
    		inSrcTypeSel = MIPI_SENSOR;
            //pSeninfDrv->autoDeskewCalibration(); //do calibration
    	}
    	else if (sensorInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
    		inSrcTypeSel = PARALLEL_SENSOR;
    	}
    	else {
    		inSrcTypeSel = TEST_MODEL;
    	}
    	

        ret = pSeninfDrv->setTg1GrabRange(pixelX0[1], pixelX1[1], pixelY0[1], pixelY1[1]);
        if (ret < 0) {
            LOG_ERR("setTg1GrabRange fail\n");
            goto setConf_exit;
        }
        //
        ret = pSeninfDrv->setTg1SensorModeCfg(sensorInfo[1].SensorHsyncPolarity ? 0 : 1, 
                                        sensorInfo[1].SensorVsyncPolarity ? 0 : 1);
        if (ret < 0) {
            LOG_ERR("setTg1SensorModeCfg fail\n");
            goto setConf_exit;
        }

        //
        ret = pSeninfDrv->setTg1InputCfg(padSel, inSrcTypeSel, inDataType, senInLsb);
        if (ret < 0) {
            LOG_ERR("setTg1InputCfg fail\n");
            goto setConf_exit;
        }


        // Set view finder mode
        ret = pSeninfDrv->setTg1ViewFinderMode(halSensorIFParam[1].u4IsContinous ? 0 : 1, halSensorIFParam[1].u4IsBypassSensorDelay ? 0 : sensorInfo[1].CaptureDelayFrame);
        if (ret < 0) {
            LOG_ERR("setViewFinderMode fail\n");
            goto setConf_exit;
        }

        if((mImageSensorType[1] == IMAGE_SENSOR_TYPE_RAW) || (mImageSensorType[1] == IMAGE_SENSOR_TYPE_RAW8)) {        
            if(curScenario != previousScenarioId[1]) {
                sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_SET_SENSOR_EXP_TIME, (int)&previousExposureTime[1],NULL,NULL);
                LOG_MSG("set Exposure Time = %d, curScenario = %d, previousScenario = %d\n",previousExposureTime[1],curScenario,previousScenarioId[1]);

            }
        }
        previousScenarioId[1] = curScenario;

    }





    // ATV sensor
    if(mSensorDev & SENSOR_DEV_ATV ) {
        eSensorDev = SENSOR_ATV;
#ifdef ATV_SUPPORT
#ifdef MTK_MATV_SERIAL_IF_SUPPORT
        pSeninfDrv->initTg1Serial(MTRUE);         
        pSeninfDrv->setTg1Serial(sensorInfo[0].SensorClockPolarity, 320, 240, 1, 0);
#endif
#endif
    }


    ret = setCSI2Config(0);    // disable csi2
    if (ret < 0) {
        LOG_ERR("disable csi2 fail\n");
        goto setConf_exit;
    }

    //
    ret = setCSI2Config(1);    // enable csi2
    if (ret < 0) {
        LOG_ERR("enable csi2 fail\n");
        goto setConf_exit;
    }

    sensorDebug[0].u4GrabX = pixelX0[0];
    sensorDebug[0].u4GrabY = pixelY0[0];
    sensorDebug[0].u4CropW = halSensorIFParam[0].u4CropW;
    sensorDebug[0].u4CropH = halSensorIFParam[0].u4CropH;
    sensorDebug[1].u4GrabX = pixelX0[1];
    sensorDebug[1].u4GrabY = pixelY0[1];
    sensorDebug[1].u4CropW = halSensorIFParam[1].u4CropW;
    sensorDebug[1].u4CropH = halSensorIFParam[1].u4CropH;    



setConf_exit:
    #ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagEnd);
    #endif    
    return ret;   
}
  

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::initSensor()
{
    MINT32 ret = 0;
     ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResInfo[2];
     SENSOR_DEV_ENUM eSensorDev = SENSOR_NONE;
     MUINT32 u4PaddedWidth=0,u4PaddedHeight=0;

    LOG_MSG("[initSensor]\n");
    //
    switch (mSensorDev)
    {
        case SENSOR_DEV_MAIN:
            eSensorDev = SENSOR_MAIN;
            if(!(mSearchSensorDev & SENSOR_DEV_MAIN)) {
                LOG_ERR("initSensor fail,mSensorDev = 0x%x, mSearchSensorDev = 0x%x\n",mSensorDev,mSearchSensorDev);
                return -1;
            }
            break;
        case SENSOR_DEV_SUB:
            eSensorDev = SENSOR_SUB;
            if(!(mSearchSensorDev & SENSOR_DEV_SUB)) {
                LOG_ERR("initSensor fail,mSensorDev = 0x%x, mSearchSensorDev = 0x%x\n",mSensorDev,mSearchSensorDev);
                return -1;                
            }            
            break;

        case SENSOR_DEV_ATV:
            eSensorDev = SENSOR_ATV;
            if(!(mSearchSensorDev & SENSOR_DEV_ATV)) {
                LOG_ERR("initSensor fail,mSensorDev = 0x%x, mSearchSensorDev = 0x%x\n",mSensorDev,mSearchSensorDev);
                return -1;
            }              
            break;            
        default:
            break;
    }     
    ret = pSensorDrv->init(mSensorDev);
    if (ret < 0) {
        LOG_ERR("halSensorInit fail \n");
        return ret;
    }
    // Get Sensor Resolution
    pSensorResInfo[0] = &sensorResolution[0];
    pSensorResInfo[1] = &sensorResolution[1];
    ret = pSensorDrv->getResolution(pSensorResInfo);
    if (ret < 0) {
        LOG_ERR("halSensorGetResolution failn");
        return ret;
    }
    LOG_MSG("  Main/ATV sensor resolution, Preview: %d/%d, Full: %d/%d \n", 
        sensorResolution[0].SensorPreviewWidth, sensorResolution[0].SensorPreviewHeight,
        sensorResolution[0].SensorFullWidth, sensorResolution[0].SensorFullHeight);
    LOG_MSG("  Sub/MAIN_2 sensor resolution, Preview: %d/%d, Full: %d/%d \n", 
        sensorResolution[1].SensorPreviewWidth, sensorResolution[1].SensorPreviewHeight,
        sensorResolution[1].SensorFullWidth, sensorResolution[1].SensorFullHeight);    
    //
    /*
    According to "MT6575 ISP Hardware Limitation.xls"
        when CAM_PATH.OUTPATH_EN = 1:
            bayer8:   (width*height)%8=0
            bayer10:  (width*height)%6=0
            yuv444:   (width*height)%2=0
            yuv422:   (width*height)%4=0
    */
   
    
    if(eSensorDev == SENSOR_DEV_MAIN) {
        if  ( IMAGE_SENSOR_TYPE_RAW == pSensorDrv->getCurrentSensorType(eSensorDev) )
        {
            //  Full Resolution
            u4PaddedWidth = sensorResolution[0].SensorFullWidth + ISP_RAW_WIDTH_PADD;
            u4PaddedHeight= sensorResolution[0].SensorFullHeight + ISP_RAW_HEIGHT_PADD;
            if  ( 0 != ((u4PaddedWidth * u4PaddedHeight) % 6) )
            {
                sensorResolution[0].SensorFullHeight -= (u4PaddedHeight % 6);
                LOG_MSG("  Sensor resolution after fixing: Full: %d/%d\n", sensorResolution[0].SensorFullWidth, sensorResolution[0].SensorFullHeight);
            }
        }
    }

    if(eSensorDev == SENSOR_DEV_SUB) {
        if  ( IMAGE_SENSOR_TYPE_RAW == pSensorDrv->getCurrentSensorType(eSensorDev) )
        {
            //  Full Resolution
            u4PaddedWidth = sensorResolution[1].SensorFullWidth + ISP_RAW_WIDTH_PADD;
            u4PaddedHeight= sensorResolution[1].SensorFullHeight + ISP_RAW_HEIGHT_PADD;
            if  ( 0 != ((u4PaddedWidth * u4PaddedHeight) % 6) )
            {
                sensorResolution[1].SensorFullHeight -= (u4PaddedHeight % 6);
                LOG_MSG("  Sensor resolution after fixing: Full: %d/%d\n", sensorResolution[1].SensorFullWidth, sensorResolution[0].SensorFullHeight);
            }
        }
    }
    


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::getSensorInfo(ACDK_SCENARIO_ID_ENUM mode[2])
{
    MINT32 ret = 0;
    ACDK_SENSOR_INFO_STRUCT *pInfo[2];
    ACDK_SENSOR_CONFIG_STRUCT *pConfig[2];
    SENSOR_DEV_ENUM eSensorDev = SENSOR_NONE;
    
    LOG_MSG("[getSensorInfo]\n");
    
    scenarioId[0] = mode[0];
    scenarioId[1] = mode[1];    
     
    memset(&sensorInfo[0], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&sensorInfo[1], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&sensorCfg[0], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    memset(&sensorCfg[1], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));    
    //
    pInfo[0] = &sensorInfo[0];
    pInfo[1] = &sensorInfo[1];
    pConfig[0] = &sensorCfg[0];
    pConfig[1] = &sensorCfg[1];

    
    ret = pSensorDrv->getInfo(scenarioId, pInfo, pConfig);
  
    
    if (ret < 0) {
        LOG_ERR("getSensorInfo fail\n");
        return ret;       
    }
    //

    switch (mSensorDev)
    {
    case SENSOR_DEV_MAIN:
        eSensorDev = SENSOR_MAIN;
        break;
    case SENSOR_DEV_SUB:
        eSensorDev = SENSOR_SUB;
        break;
    case SENSOR_DEV_ATV:
        eSensorDev = SENSOR_ATV;
        break;
    default:
        break;
    }    

    if(eSensorDev == SENSOR_DEV_MAIN) {
         mImageSensorType[0] = pSensorDrv->getCurrentSensorType(eSensorDev);
        switch (mImageSensorType[0]) {
        case IMAGE_SENSOR_TYPE_RAW:
        case IMAGE_SENSOR_TYPE_RAW8:            
            mIspSensorType[0] = SENSOR_TYPE_RAW;    
            break;
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            mIspSensorType[0] = SENSOR_TYPE_YUV;
            break;
    	case IMAGE_SENSOR_TYPE_RGB565:
    		mIspSensorType[0] = SENSOR_TYPE_RGB565;
    		break;
        default:
            mIspSensorType[0] = SENSOR_TYPE_UNKNOWN;
            ret = -EINVAL;
            LOG_ERR("Unsupport Main Sensor Type \n");
            break;
        }
    }

    if(eSensorDev == SENSOR_DEV_SUB) {
         mImageSensorType[1] = pSensorDrv->getCurrentSensorType(eSensorDev);
        switch (mImageSensorType[1]) {
        case IMAGE_SENSOR_TYPE_RAW:
        case IMAGE_SENSOR_TYPE_RAW8:            
            mIspSensorType[1] = SENSOR_TYPE_RAW;    
            break;
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            mIspSensorType[1] = SENSOR_TYPE_YUV;
            break;
    	case IMAGE_SENSOR_TYPE_RGB565:
    		mIspSensorType[1] = SENSOR_TYPE_RGB565;
    		break;
        default:
            mIspSensorType[1] = SENSOR_TYPE_UNKNOWN;
            ret = -EINVAL;
            LOG_ERR("Unsupport Sub Sensor Type \n");
            break;
        }
    }

    if(eSensorDev == SENSOR_DEV_ATV) {
         mImageSensorType[0] = pSensorDrv->getCurrentSensorType(eSensorDev);
        switch (mImageSensorType[0]) {
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            mIspSensorType[0] = SENSOR_TYPE_YUV;
            break;
    	case IMAGE_SENSOR_TYPE_RGB565:
    		mIspSensorType[0] = SENSOR_TYPE_RGB565;
    		break;
        default:
            mIspSensorType[0] = SENSOR_TYPE_UNKNOWN;
            ret = -EINVAL;
            LOG_ERR("Unsupport Sensor Type \n");
            break;
        }
    }

    

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::setTgPhase() //CMMCLK: Main/sub, CMMCLK2:Main_2 (external signal design is not sync with internal signal in TG/I2C)
{
    MINT32 ret = 0, ret2 = 0;
    MUINT32 u4PadPclkInv1 = 0, u4PadPclkInv2 = 0;
    MINT32 clkInKHz1, clkCnt1, mclk1, mclkSel1;
    MINT32 clkInKHz2, clkCnt2, mclk2, mclkSel2;

    LOG_MSG("[setTgPhase] Tg1clk: %d, Tg2clk: %d \n", sensorInfo[0].SensorClockFreq, sensorInfo[1].SensorClockFreq);
    //
    if((mSensorDev & SENSOR_DEV_MAIN)||(mSensorDev & SENSOR_DEV_ATV)) {
        clkInKHz1 = sensorInfo[0].SensorClockFreq * 1000;     
   
        if ((clkInKHz1 < 3250) || (clkInKHz1 >= 104000)) {
            LOG_ERR("Err-Input clock rate error, %d \n", clkInKHz1);
            return -EINVAL;
        }
        //
        if ((clkInKHz1 % 48) == 0) {
            // Clock is in 48MHz group, original source is 48MHz
            mclk1 = 48000;
            mclkSel1 = CAM_PLL_48_GROUP;
        }
        else {
            // Clock is in 52MHz group
            mclk1 = 208000;//208000;
            mclkSel1 = CAM_PLL_52_GROUP;
        }

        //
        clkCnt1 = (mclk1 + (clkInKHz1 >> 1)) / clkInKHz1;
        // Maximum CLKCNT is 15
        clkCnt1 = clkCnt1 > 15 ? 15 : clkCnt1-1;
        LOG_MSG("  mclk1: %d, clkCnt1: %d \n", mclk1, clkCnt1);
    }

    if(mSensorDev & SENSOR_DEV_SUB) { 
        clkInKHz1 = sensorInfo[1].SensorClockFreq * 1000;


        if ((clkInKHz1 < 3250) || (clkInKHz1 >= 104000)) {
            LOG_ERR("Err-Input clock rate error, %d \n", clkInKHz1);
            return -EINVAL;
        }
        //
        if ((clkInKHz1 % 48) == 0) {
            // Clock is in 48MHz group, original source is 48MHz
            mclk1 = 48000;
            mclkSel1 = CAM_PLL_48_GROUP;
        }
        else {
            // Clock is in 52MHz group
            mclk1 = 208000;//208000;
            mclkSel1 = CAM_PLL_52_GROUP;
        }

        //
        clkCnt1 = (mclk1 + (clkInKHz1 >> 1)) / clkInKHz1;
        // Maximum CLKCNT is 15
        clkCnt1 = clkCnt1 > 15 ? 15 : clkCnt1-1;
        LOG_MSG("  mclk1: %d, clkCnt1: %d \n", mclk1, clkCnt1);
    }



    

    switch (mSensorDev)
    {
    case SENSOR_DEV_MAIN:
        ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, &u4PadPclkInv1);
        break;
    case SENSOR_DEV_SUB:
        ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_PAD_PCLK_INV, &u4PadPclkInv1);
        break;
    case SENSOR_DEV_ATV:
            ret = pSensorDrv->sendCommand(SENSOR_ATV, CMD_SENSOR_GET_PAD_PCLK_INV, &u4PadPclkInv1);
            break;
    default:
        u4PadPclkInv1 = 0;

        ret = 0;
        ret2 = 0;
        break;
    }
    if ((ret < 0)||(ret2 < 0)) {
        LOG_ERR("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x), err2(%x)\n", ret, ret2);
    }
    LOG_MSG("[setTgPhase] u4PadPclkInv_1(%d),u4PadPclkInv_2(%d) \n", u4PadPclkInv1,u4PadPclkInv2);

    // Config TG, always use Camera PLL, 1: 48MHz, 2: 208MHz

    if((mSensorDev & SENSOR_DEV_MAIN ) || (mSensorDev & SENSOR_DEV_ATV )){
        ret = pSeninfDrv->setTg1PhaseCounter(
            1, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
            clkCnt1, sensorInfo[0].SensorClockPolarity ? 0 : 1,
            sensorInfo[0].SensorClockFallingCount, sensorInfo[0].SensorClockRisingCount, u4PadPclkInv1);
    }
    
    if (ret < 0) {
        LOG_ERR("setTg1PhaseCounter fail\n");
        return ret;
    }

    //notice SUB sensorInfo[1] but use Tg1 mclk
    if(mSensorDev & SENSOR_DEV_SUB) {
        ret = pSeninfDrv->setTg1PhaseCounter(
            1, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
            clkCnt1, sensorInfo[1].SensorClockPolarity ? 0 : 1,
            sensorInfo[1].SensorClockFallingCount, sensorInfo[1].SensorClockRisingCount, u4PadPclkInv1);
    }
    
    if (ret < 0) {
        LOG_ERR("setTg1PhaseCounter fail\n");
        return ret;
    }


    


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::getRawInfo(halSensorDev_e sensorDevId,halSensorRawImageInfo_t *pinfo, MINT32 mode)
{
    MINT32 ret = 0;
    //const char *porder[4] = {"B", "Gb", "Gr", "R"};

    MINT32 W = 0, H = 0;

    memset(pinfo, 0, sizeof(halSensorRawImageInfo_t));

    if((SENSOR_DEV_MAIN==sensorDevId)|| (SENSOR_DEV_ATV==sensorDevId)) {
        if ( mode == 1 ) { // preview
            W = sensorResolution[0].SensorPreviewWidth;
            H = sensorResolution[0].SensorPreviewHeight;
        }
        else {//capture
            W = sensorResolution[0].SensorFullWidth;
            H = sensorResolution[0].SensorFullHeight;                    
        }
        switch (mImageSensorType[0]) {
            case IMAGE_SENSOR_TYPE_RAW:
                pinfo->u4BitDepth = 10; 
                pinfo->u4IsPacked = 1; 
                pinfo->u4Width = W + ISP_RAW_WIDTH_PADD; 
                pinfo->u4Height = H + ISP_RAW_HEIGHT_PADD; 
                pinfo->u4Size = (pinfo->u4Width * pinfo->u4Height * 4 + 2) / 3;  // 2 is for round up
                pinfo->u1Order = sensorInfo[0].SensorOutputDataFormat;
                break;
            case IMAGE_SENSOR_TYPE_RAW8:
                switch(sensorInfo[0].SensorOutputDataFormat) {
                    case SENSOR_OUTPUT_FORMAT_RAW8_B:
                        sensorInfo[0].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_B;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
                        sensorInfo[0].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_Gb;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
                        sensorInfo[0].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_Gr;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_R:
                        sensorInfo[0].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_R;
                        break;
                        
                    default:
                        break;
                }
                pinfo->u4BitDepth = 8; 
                pinfo->u4IsPacked = 1; 
                pinfo->u4Width = W + ISP_RAW_WIDTH_PADD; 
                pinfo->u4Height = H + ISP_RAW_HEIGHT_PADD; 
                pinfo->u4Size = (pinfo->u4Width * pinfo->u4Height ) ;  
                pinfo->u1Order = sensorInfo[0].SensorOutputDataFormat;
                break;                
            case IMAGE_SENSOR_TYPE_YUV:
                pinfo->u4BitDepth = 8; 
                pinfo->u4IsPacked = 0; 
                pinfo->u4Width = W;
                pinfo->u4Height = H; 
                pinfo->u4Size = pinfo->u4Width * pinfo->u4Height * 2;
                pinfo->u1Order = sensorInfo[0].SensorOutputDataFormat;
                break;
            default:
                LOG_ERR("[SensorHalImp] Err \n");
                break;        
        }
        // align size to 16x
        pinfo->u4Size = (pinfo->u4Size + 0xf) & (~0xf);         
    }
    else if (SENSOR_DEV_SUB==sensorDevId) {
        if ( mode == 1 ) { // preview
            W = sensorResolution[1].SensorPreviewWidth;
            H = sensorResolution[1].SensorPreviewHeight;
        }
        else {//capture
            W = sensorResolution[1].SensorFullWidth;
            H = sensorResolution[1].SensorFullHeight;
                    
        }
        switch (mImageSensorType[1]) {
            case IMAGE_SENSOR_TYPE_RAW:
                pinfo->u4BitDepth = 10; 
                pinfo->u4IsPacked = 1; 
                pinfo->u4Width = W + ISP_RAW_WIDTH_PADD; 
                pinfo->u4Height = H + ISP_RAW_HEIGHT_PADD; 
                pinfo->u4Size = (pinfo->u4Width * pinfo->u4Height * 4 + 2) / 3;  // 2 is for round up
                pinfo->u1Order = sensorInfo[1].SensorOutputDataFormat;
                break;
            case IMAGE_SENSOR_TYPE_RAW8:
                switch(sensorInfo[1].SensorOutputDataFormat) {
                    case SENSOR_OUTPUT_FORMAT_RAW8_B:
                        sensorInfo[1].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_B;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
                        sensorInfo[1].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_Gb;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
                        sensorInfo[1].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_Gr;
                        break;
                    case SENSOR_OUTPUT_FORMAT_RAW8_R:
                        sensorInfo[1].SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_RAW_R;
                        break;
                        
                    default:
                        break;
                }                
                pinfo->u4BitDepth = 8; 
                pinfo->u4IsPacked = 1; 
                pinfo->u4Width = W + ISP_RAW_WIDTH_PADD; 
                pinfo->u4Height = H + ISP_RAW_HEIGHT_PADD; 
                pinfo->u4Size = (pinfo->u4Width * pinfo->u4Height ) ;  
                pinfo->u1Order = sensorInfo[1].SensorOutputDataFormat;
                break;                
            case IMAGE_SENSOR_TYPE_YUV:
                pinfo->u4BitDepth = 8; 
                pinfo->u4IsPacked = 0; 
                pinfo->u4Width = W;
                pinfo->u4Height = H; 
                pinfo->u4Size = pinfo->u4Width * pinfo->u4Height * 2;
                pinfo->u1Order = sensorInfo[1].SensorOutputDataFormat;
                break;
            default:
                LOG_ERR("[SensorHalImp] Err \n");
                break;        
        }
        // align size to 16x
        pinfo->u4Size = (pinfo->u4Size + 0xf) & (~0xf);                
    }
    
  
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/       

MINT32 SensorHalImp::waitSensorEventDone(
    MUINT32 EventType,    
    MUINT32 Timeout
)
{
    MINT32 ret = 0;
    ret = pSensorDrv->waitSensorEventDone(EventType,Timeout);

    if(ret < 0)
    {
        LOG_MSG("waitSensorEventDone err, Event = %d", EventType);
    }
    return ret;    
}


/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::sendCommand(
    halSensorDev_e sensorDevId,
    int cmd,
    int arg1,
    int arg2,
    int arg3)
{
    MINT32 ret = 0;
    MUINT32 cmdId = 0;
    SensorDrv *pDeviceDrv = NULL;

    //LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    //
    pDeviceDrv = SensorDrv::createInstance(sensorDevId);
    switch (cmd) {
    //0x1000
    case SENSOR_CMD_SET_SENSOR_DEV:
        LOG_MSG("  Sensor Dev: %d \n", sensorDevId);
        mSensorDev = sensorDevId;    
        ret = pSeninfDrv->sendCommand(CMD_SET_DEVICE,mSensorDev,0,0);
        if(ret < 0) {
            LOG_ERR("[sendCommand] CMD_SET_DEVICE fail! \n");
        }
        break;    

    case SENSOR_CMD_SET_SENSOR_EXP_TIME:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        //LOG_MSG("  Exposure Time: %d \n", *(MUINT32 *) arg1);
        if(SENSOR_DEV_MAIN==sensorDevId) {
            previousExposureTime[0] = *(MUINT32 *) arg1;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            previousExposureTime[1] = *(MUINT32 *) arg1;
        }
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }        
        break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_GAIN;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;

    case SENSOR_CMD_SET_FLICKER_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_FLICKER_FRAME_RATE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;

	case SENSOR_CMD_SET_VIDEO_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_VIDEO_FRAME_RATE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);        
        break;  

    case SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC:
        cmdId = CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;
        
    case SENSOR_CMD_SET_CCT_FEATURE_CONTROL:
        cmdId = CMD_SENSOR_SET_CCT_FEATURE_CONTROL;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2, (MUINT32 *) arg3);
        break;

    case SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA:
        cmdId = CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;
    case SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);
        break;
    case SENSOR_CMD_SET_TEST_PATTERN_OUTPUT:
        cmdId = CMD_SENSOR_SET_TEST_PATTERN_OUTPUT;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_ESHUTTER_GAIN;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);
        break;        
    //0x2000
    case SENSOR_CMD_GET_SENSOR_DEV:
        *(MINT32 *) arg1 =mSensorDev;
        break;
        
    case SENSOR_CMD_GET_SENSOR_PRV_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_PRV_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].SensorPreviewWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].SensorPreviewHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].SensorPreviewWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].SensorPreviewHeight;
        }
        else if (SENSOR_DEV_ATV==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[2].SensorPreviewWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[2].SensorPreviewHeight;
            LOG_MSG("  test_atv   %dx%d", arg1, arg2);
        }
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }
        #ifdef HAVE_AEE_FEATURE
        AEE_ASSERT( arg1 != 0 && arg2 != 0 );
        #endif
        break;

    case SENSOR_CMD_GET_SENSOR_FULL_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_FULL_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].SensorFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].SensorFullHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].SensorFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].SensorFullHeight;
        }  
        else if (SENSOR_DEV_ATV==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[2].SensorFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[2].SensorFullHeight;
        } 
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }       
        #ifdef HAVE_AEE_FEATURE
        AEE_ASSERT( arg1 != 0 && arg2 != 0 );
        #endif
        break;

    case SENSOR_CMD_GET_SENSOR_VIDEO_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_VIDEO_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].SensorVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].SensorVideoHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].SensorVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].SensorVideoHeight;
        } 
        else if(SENSOR_DEV_ATV==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[2].SensorFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[2].SensorFullHeight;
        }
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }     
        #ifdef HAVE_AEE_FEATURE   
        AEE_ASSERT( arg1 != 0 && arg2 != 0 );
        #endif
        break;  
    case SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_VIDEO_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].SensorHighSpeedVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].SensorHighSpeedVideoHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].SensorHighSpeedVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].SensorHighSpeedVideoHeight;
        }  
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }        
        break;        
    case SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_PRV_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].Sensor3DPreviewWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].Sensor3DPreviewHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].Sensor3DPreviewWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].Sensor3DPreviewHeight;
        }
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }
        break;

    case SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_FULL_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].Sensor3DFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].Sensor3DFullHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].Sensor3DFullWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].Sensor3DFullHeight;
        }   
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }        
        break;

    case SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_VIDEO_RANGE \n");
        if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[0].Sensor3DVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[0].Sensor3DVideoHeight;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MUINT16 *) arg1 = staticSensorResoultion[1].Sensor3DVideoWidth;
            *(MUINT16 *) arg2 = staticSensorResoultion[1].Sensor3DVideoHeight;
        }  
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }        
        break;
    case SENSOR_CMD_GET_SENSOR_ID:
        switch(sensorDevId) {
            case SENSOR_DEV_MAIN:
                *(MUINT32 *)arg1 = pDeviceDrv->getMainSensorID();
            break;
            case SENSOR_DEV_SUB:
                *(MUINT32 *)arg1 = pDeviceDrv->getSubSensorID();
            break;
            default:
                LOG_ERR("  SENSOR_CMD_GET_SENSOR_ID wrong sensorDevId = 0x%x !\n",sensorDevId);
            break;
        }
        
        break;
    case SENSOR_CMD_GET_RAW_PADDING_RANGE:
        LOG_MSG("  ISP_CMD_GET_RAW_DUMMY_RANGE \n");
        *(MINT32 *) arg1 = ISP_RAW_WIDTH_PADD;
        *(MINT32 *) arg2 = ISP_RAW_HEIGHT_PADD;
        break;


    case SENSOR_CMD_GET_SENSOR_NUM:
        break;


    case SENSOR_CMD_GET_SENSOR_TYPE:
         if(SENSOR_DEV_MAIN==sensorDevId) {
            *(MINT32 *) arg1 = staticSensorType[0];
        }        
         else if (SENSOR_DEV_SUB==sensorDevId) {
            *(MINT32 *) arg1 = staticSensorType[1];
         }
         else if (SENSOR_DEV_ATV==sensorDevId) {
            LOG_MSG(" test_atv   atv: 0x%x", staticSensorType[2]);
            *(MINT32 *) arg1 = staticSensorType[2];
         }
        break;
  
    case SENSOR_CMD_GET_RAW_INFO:
        LOG_MSG("  SENSOR_CMD_GET_RAW_INFO \n");
        if(arg2 >= 2) {
            LOG_ERR("SENSOR_CMD_GET_RAW_INFO arg2 incorrect ! \n");
        }
        if(SENSOR_DEV_MAIN==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorRawInfo[0][arg2],sizeof(halSensorRawImageInfo_t));
        }        
        else if (SENSOR_DEV_SUB==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorRawInfo[1][arg2],sizeof(halSensorRawImageInfo_t));
        }               
        else if(SENSOR_DEV_ATV==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorRawInfo[2][arg2],sizeof(halSensorRawImageInfo_t));
        }          
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }

        break;

    case SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT:
        cmdId = CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2); //arg1 = mode
        break; 

    case SENSOR_CMD_GET_INPUT_BIT_ORDER:
        cmdId = CMD_SENSOR_GET_INPUT_BIT_ORDER;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);    
        break;
    
    case SENSOR_CMD_GET_PAD_PCLK_INV:
        cmdId = CMD_SENSOR_GET_PAD_PCLK_INV;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);         
        break;
        
    case SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE:
        cmdId = CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;
    
    case SENSOR_CMD_GET_SENSOR_FACING_DIRECTION:
        cmdId = CMD_SENSOR_GET_SENSOR_FACING_DIRECTION;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;

    case SENSOR_CMD_GET_PIXEL_CLOCK_FREQ:
        cmdId = CMD_SENSOR_GET_PIXEL_CLOCK_FREQ;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);         
        break;
        
    case SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        cmdId = CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;        
        
    case SENSOR_CMD_GET_SENSOR_FEATURE_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_FEATURE_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;
        
    case SENSOR_CMD_GET_ATV_DISP_DELAY_FRAME:
        cmdId = CMD_SENSOR_GET_ATV_DISP_DELAY_FRAME;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;        


    case SENSOR_CMD_GET_SENSOR_SCENARIO:
        if (arg1) {
            *((ACDK_SCENARIO_ID_ENUM *)arg1) = curScenario;
        }
        break;
    case SENSOR_CMD_GET_SENSOR_CROPINFO:
        if (arg1) {
            if(SENSOR_DEV_MAIN==sensorDevId) {
                ::memcpy((char *) arg1, &sensorCropInfo[0], sizeof(SENSOR_CROP_INFO));
            }
            else if (SENSOR_DEV_SUB==sensorDevId) {
                ::memcpy((char *) arg1, &sensorCropInfo[1], sizeof(SENSOR_CROP_INFO));
            }
        }
        break;
    case SENSOR_CMD_GET_SENSOR_GRAB_INFO:
        LOG_MSG("  SENSOR_CMD_GET_SENSOR_INFO \n");
        if(arg3 >= ACDK_SCENARIO_ID_MAX-1) {
            LOG_ERR("SENSOR_CMD_GET_SENSOR_INFO arg2 incorrect ! \n");
        }
        if(SENSOR_DEV_MAIN==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorGrabInfo[0][arg2], sizeof(SENSOR_GRAB_INFO_STRUCT));
            //*(MUINT16 *) arg1 = staticSensorGrabInfo[0][arg3].u4SensorGrabStartX;
            //*(MUINT16 *) arg2 = staticSensorGrabInfo[0][arg3].u4SensorGrabStartY;
        }
        else if (SENSOR_DEV_SUB==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorGrabInfo[1][arg2], sizeof(SENSOR_GRAB_INFO_STRUCT));
            //*(MUINT16 *) arg1 = staticSensorGrabInfo[1][arg3].u4SensorGrabStartX;
            //*(MUINT16 *) arg2 = staticSensorGrabInfo[1][arg3].u4SensorGrabStartY;
        }       
        else if (SENSOR_DEV_ATV==sensorDevId) {
            ::memcpy((char *) arg1, &staticSensorGrabInfo[2][arg2], sizeof(SENSOR_GRAB_INFO_STRUCT));
            //*(MUINT16 *) arg1 = staticSensorGrabInfo[2][arg3].u4SensorGrabStartX;
            //*(MUINT16 *) arg2 = staticSensorGrabInfo[2][arg3].u4SensorGrabStartY;
        }
        else{
            LOG_ERR("[sendCommand] sensorDevId is incorrect ! \n");
        }
        
        break;
    case SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);
        break;
        
    case SENSOR_CMD_GET_FAKE_ORIENTATION:
        cmdId = CMD_SENSOR_GET_FAKE_ORIENTATION;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;      

    case SENSOR_CMD_GET_SENSOR_VIEWANGLE:
        cmdId = CMD_SENSOR_GET_SENSOR_VIEWANGLE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);                       
        break; 
    case SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE:
        cmdId = CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;  
    case SENSOR_CMD_GET_SENSOR_CURRENT_TEMPERATURE:
        cmdId = CMD_SENSOR_GET_SENSOR_CURRENT_TEMPERATURE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);         
        break;
    //0x3000    
    case SENSOR_CMD_SET_YUV_FEATURE_CMD:
        cmdId = CMD_SENSOR_SET_YUV_FEATURE_CMD;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);         
        break;        
        
    case SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE:
        cmdId = CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId); 
        break;        
        
    case SENSOR_CMD_SET_YUV_CANCEL_AF:
        cmdId = CMD_SENSOR_SET_YUV_CANCEL_AF;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);                 
        break;        
        
    case SENSOR_CMD_SET_YUV_CONSTANT_AF:
        cmdId = CMD_SENSOR_SET_YUV_CONSTANT_AF;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);  
        break;        

    case SENSOR_CMD_SET_YUV_AF_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AF_WINDOW;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);        
        break;        
        
    case SENSOR_CMD_SET_YUV_AE_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AE_WINDOW;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;        
           


    //0x4000
    case SENSOR_CMD_GET_YUV_AF_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AF_STATUS;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);         
        break;        
        
    case SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);        
        break;        
        
    case SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);         
        break;        
        
    case SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;        
        
    case SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;        
        
    case SENSOR_CMD_GET_YUV_EXIF_INFO:
        cmdId = CMD_SENSOR_GET_YUV_EXIF_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;        
        
    case SENSOR_CMD_GET_YUV_DELAY_INFO:
        cmdId = CMD_SENSOR_GET_YUV_DELAY_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;   
    case SENSOR_CMD_GET_YUV_AE_AWB_LOCK:
        cmdId = CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);
        break;        
    case SENSOR_CMD_GET_YUV_STROBE_INFO:
        cmdId = CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1);          
        break;
    case SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        cmdId = CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;

    case SENSOR_CMD_SET_YUV_3A_CMD:
        cmdId = CMD_SENSOR_SET_YUV_3A_CMD;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1); 
        break;

    case SENSOR_CMD_SET_YUV_AUTOTEST:
        cmdId = CMD_SENSOR_SET_YUV_AUTOTEST;
        pDeviceDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINT32 *) arg1, (MUINT32 *) arg2);
        break;    
        
    default:
        ret = -1;
        LOG_MSG("[sendCommand] err: 0x%x \n", cmd);
        break;
    }
    //
    if (pDeviceDrv) {
        pDeviceDrv->destroyInstance();
        pDeviceDrv = NULL;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::initCSI2Peripheral(MINT32 initCSI2)
{
    MINT32 ret = 0;

    if((mSensorDev & SENSOR_DEV_MAIN ) || (mSensorDev & SENSOR_DEV_ATV )){
        if(sensorInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {   // enable peripheral
            //pSeninfDrv->sendCommand(CMD_SET_MIPI_TYPE,sensorInfo[0].MIPIsensorType,0,0);//select mipi type
            if(initCSI2) {
                ret = pSeninfDrv->initTg1CSI2(1);
                if (ret < 0) {
                    LOG_ERR("init Tg1 CSI2 peripheral fail\n");
                }
            } else {
                ret = pSeninfDrv->initTg1CSI2(0);
                if (ret < 0) {
                    LOG_ERR("uninit Tg1 CSI2 peripheral fail\n");
                }
            }
        }
        else {
            ret = pSeninfDrv->initTg1CSI2(0);
            if (ret < 0) {
                LOG_ERR("uninit Tg1 CSI2 peripheral fail\n");
            }
        }
    }

    if(mSensorDev & SENSOR_DEV_SUB) {
        if(sensorInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {   // enable peripheral
            //pSeninfDrv->sendCommand(CMD_SET_MIPI_TYPE,sensorInfo[1].MIPIsensorType,0,0);//select mipi type
            if(initCSI2) {
                ret = pSeninfDrv->initTg1CSI2(1);
                if (ret < 0) {
                    LOG_ERR("init Tg1 CSI2 peripheral fail\n");
                }
            } else {
                ret = pSeninfDrv->initTg1CSI2(0);
                if (ret < 0) {
                    LOG_ERR("uninit Tg1 CSI2 peripheral fail\n");
                }
            } 
        }
        else {
            ret = pSeninfDrv->initTg1CSI2(0);
            if (ret < 0) {
                LOG_ERR("uninit Tg2 CSI2 peripheral fail\n");
            }
        }        
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::setCSI2Config(MINT32 enableCSI2)
{
    MINT32 ret = 0;

    if((mSensorDev & SENSOR_DEV_MAIN) || (mSensorDev & SENSOR_DEV_ATV)){
        if((enableCSI2 ==  1) && (sensorInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI)) {   // enable csi2
            // Set mipi csi2         
            LOG_MSG("enable Tg1 CSI2Config\n");
            ret = pSeninfDrv->setTg1CSI2(sensorInfo[0].MIPIDataLowPwr2HighSpeedTermDelayCount, sensorInfo[0].MIPIDataLowPwr2HighSpeedSettleDelayCount, 
                      sensorInfo[0].MIPICLKLowPwr2HighSpeedTermDelayCount, sensorInfo[0].SensorVsyncPolarity, sensorInfo[0].SensorMIPILaneNumber, enableCSI2, sensorInfo[0].SensorPacketECCOrder, 0);
            if (ret < 0) {
                LOG_ERR("setTg1CSI2Config fail\n");
            }
        }
        else {
            LOG_MSG("disable Tg1 CSI2Config\n");
            ret = pSeninfDrv->setTg1CSI2(0, 0, 0, 0, 0, 0, 0, 0);
            if (ret < 0) {
                LOG_ERR("disable Tg1 CSI2Config fail\n");
            }

        }
    }

    if(mSensorDev & SENSOR_DEV_SUB) {
        if((enableCSI2 ==  1) && (sensorInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI)) {   // enable csi2
            // Set mipi csi2         
            LOG_MSG("enable Tg2 CSI2Config\n");
            ret = pSeninfDrv->setTg1CSI2(sensorInfo[1].MIPIDataLowPwr2HighSpeedTermDelayCount, sensorInfo[1].MIPIDataLowPwr2HighSpeedSettleDelayCount, 
                      sensorInfo[1].MIPICLKLowPwr2HighSpeedTermDelayCount, sensorInfo[1].SensorVsyncPolarity, sensorInfo[1].SensorMIPILaneNumber, enableCSI2, sensorInfo[1].SensorPacketECCOrder, 0);
            if (ret < 0) {
                LOG_ERR("setTg1CSI2Config fail\n");
            }
        }
        else {
            LOG_MSG("disable Tg1 CSI2Config\n");
            ret = pSeninfDrv->setTg1CSI2(0, 0, 0, 0, 0, 0, 0, 0);
            if (ret < 0) {
                LOG_ERR("disable Tg1 CSI2Config fail\n");
            }

        } 
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 SensorHalImp::setSensorIODrivingCurrent()
{
    MINT32 ret = 0;
    MINT32 increaseDivingCurrent1 = 0x08,increaseDivingCurrent2 = 0x08; // set to default 2mA and slew raw control

    if(mSensorDev & SENSOR_DEV_MAIN) { //Main/sub use TG1 mclk   
        switch(sensorInfo[0].SensorDrivingCurrent) {
            case ISP_DRIVING_2MA://4 //4mA
                increaseDivingCurrent1 = 0x0;
                break;
            case ISP_DRIVING_4MA:// 8mA
                increaseDivingCurrent1 = 0x2000;
                break;
            case ISP_DRIVING_6MA://12mA
                increaseDivingCurrent1 = 0x4000;
                break;
            case ISP_DRIVING_8MA://16mA
                increaseDivingCurrent1 = 0x6000;
                break;
            default:
                LOG_MSG("The driving current value is wrong\n");
                break;
        }
        
        ret = pSeninfDrv->setTg1IODrivingCurrent(increaseDivingCurrent1);
    }
    
    if (ret < 0) {
        LOG_ERR("The Tg1 driving current setting is wrong\n"); 
    }


    if(mSensorDev & SENSOR_DEV_SUB) {
        switch(sensorInfo[1].SensorDrivingCurrent) {
            case ISP_DRIVING_2MA:
                increaseDivingCurrent2 = 0x00;
                break;
            case ISP_DRIVING_4MA:
                increaseDivingCurrent2 = 0x2000;
                break;
            case ISP_DRIVING_6MA:
                increaseDivingCurrent2 = 0x4000;
                break;
            case ISP_DRIVING_8MA:
                increaseDivingCurrent2 = 0x6000;
                break;
            default:
                LOG_MSG("The driving current value is wrong\n");
                break;
        }
        
        ret = pSeninfDrv->setTg1IODrivingCurrent(increaseDivingCurrent2);
    }
    
    if (ret < 0) {
        LOG_ERR("The Tg2 driving current setting is wrong\n"); 
    }
    
    return ret;
}



MINT32 SensorHalImp::querySensorInfo()
{
    MINT32 ret = 0;
    MINT32 mode = 0;
    MUINT32 scenario = 0;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResInfo[2];
    ACDK_SCENARIO_ID_ENUM scenarioMode[2] = {ACDK_SCENARIO_ID_CAMERA_PREVIEW,ACDK_SCENARIO_ID_CAMERA_PREVIEW};


    pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN);//just indicate to imgsensor driver

    for (mSensorDev = SENSOR_MAIN; mSensorDev <= SENSOR_SUB; mSensorDev <<= 1) {

        ret = pSensorDrv->init(mSensorDev);
        if (ret < 0) {
            LOG_ERR("halSensorInit fail \n");
        }
        // Get Sensor Resolution
        pSensorResInfo[0] = &staticSensorResoultion[0];
        switch (mSensorDev) {
            case SENSOR_SUB:
                pSensorResInfo[1] = &staticSensorResoultion[1];
            break;           
            default:
                pSensorResInfo[1] = &staticSensorResoultion[1];
            break;
        }
        ret = pSensorDrv->getResolution(pSensorResInfo);
        if (ret < 0) {
            LOG_ERR("halSensorGetResolution failn");
        } 

        
        for(scenario = 0; scenario < ACDK_SCENARIO_ID_MAX-1; scenario++ ) {
            scenarioMode[0] = (ACDK_SCENARIO_ID_ENUM)scenario;
            scenarioMode[1] = (ACDK_SCENARIO_ID_ENUM)scenario;
            ret = getSensorInfo(scenarioMode);

            switch (mSensorDev) {
                case SENSOR_MAIN:
                    staticSensorGrabInfo[0][scenario].u4SensorGrabStartX = sensorInfo[0].SensorGrabStartX;
                    staticSensorGrabInfo[0][scenario].u4SensorGrabStartY = sensorInfo[0].SensorGrabStartY;                    
                    staticSensorGrabInfo[0][scenario].u2SensorSubSpW = sensorInfo[0].SensorWidthSampling;
                    staticSensorGrabInfo[0][scenario].u2SensorSubSpH = sensorInfo[0].SensorHightSampling;  
                break;
                case SENSOR_SUB:
                    staticSensorGrabInfo[1][scenario].u4SensorGrabStartX = sensorInfo[1].SensorGrabStartX;
                    staticSensorGrabInfo[1][scenario].u4SensorGrabStartY = sensorInfo[1].SensorGrabStartY;                    
                    staticSensorGrabInfo[1][scenario].u2SensorSubSpW = sensorInfo[1].SensorWidthSampling;
                    staticSensorGrabInfo[1][scenario].u2SensorSubSpH = sensorInfo[1].SensorHightSampling; 
                break;           
                default:
                break;
            }

            if (scenario == 0) {
                switch (mSensorDev) {
                    case SENSOR_MAIN:
                        staticSensorType[0] = mIspSensorType[0];
                    break;
                    case SENSOR_SUB:
                        staticSensorType[1] = mIspSensorType[1];
                    break;          
                    default:
                    break;
                }

            }
        }

        
 
        if (ret < 0) {
            LOG_ERR("SENSOR_CMD_GET_RAW_INFO: getSensorInfo fail \n");
        }
        for (mode = 0; mode <= 1; mode++) {
            switch (mSensorDev) {
                case SENSOR_MAIN:
                    ret = getRawInfo((halSensorDev_e)mSensorDev,&staticSensorRawInfo[0][mode] , mode);
                break;
                case SENSOR_SUB:
                    ret = getRawInfo((halSensorDev_e)mSensorDev,&staticSensorRawInfo[1][mode] , mode);
                break;
                default:
                    ret = getRawInfo((halSensorDev_e)mSensorDev,&staticSensorRawInfo[0][mode] , mode);
                break;                
            }
                
        }    
        pSensorDrv->uninit();
    }
    

    mSensorDev = SENSOR_NONE;    
    printf("[querySensorInfo] mSensorDev = SENSOR_NONE\n");

    if (pSensorDrv) {
        pSensorDrv->destroyInstance();
        pSensorDrv = NULL;
    }

#ifdef ATV_SUPPORT
    mSensorDev = SENSOR_ATV;
    pSensorDrv = SensorDrv::createInstance(SENSOR_ATV);//just indicate to imgsensor driver

    ret = pSensorDrv->init(mSensorDev);
    if (ret < 0) {
        LOG_ERR("halSensorInit fail \n");
    }
    // Get Sensor Resolution
    pSensorResInfo[0] = &staticSensorResoultion[2];
    ret = pSensorDrv->getResolution(pSensorResInfo);
    if (ret < 0) {
        LOG_ERR("halSensorGetResolution failn");
    } 

    for(scenario = 0; scenario < ACDK_SCENARIO_ID_MAX-1; scenario++ ) {
        scenarioMode[0] = (ACDK_SCENARIO_ID_ENUM)scenario;
        scenarioMode[1] = (ACDK_SCENARIO_ID_ENUM)scenario;
        ret = getSensorInfo(scenarioMode);

        staticSensorGrabInfo[2][scenario].u4SensorGrabStartX = sensorInfo[0].SensorGrabStartX;
        staticSensorGrabInfo[2][scenario].u4SensorGrabStartY = sensorInfo[0].SensorGrabStartY;  

        if (scenario == 0) 
        {
            staticSensorType[2] = mIspSensorType[0];
        }
    }

    if (ret < 0) 
    {
        LOG_ERR("SENSOR_CMD_GET_RAW_INFO: getSensorInfo fail \n");
    }
    
    for (mode = 0; mode <= 1; mode++) {
        ret = getRawInfo((halSensorDev_e)mSensorDev,&staticSensorRawInfo[2][mode] , mode);
    }

    pSensorDrv->uninit();

    mSensorDev = SENSOR_NONE;    
#endif 
    if (pSensorDrv) {
        pSensorDrv->destroyInstance();
        pSensorDrv = NULL;
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/


MINT32 SensorHalImp::reset()
{
    MINT32 ret = 0, tg1Ret = 0;


    ret = pSeninfDrv->checkSeninf1Input();
    
    //if (ret != 0) { 
        LOG_MSG("[reset] start reset!\n");
        ret = pSensorDrv->close();
        if (ret < 0) {
            LOG_ERR("[reset]pSensorDrv->close fail \n");
        }    
        usleep(100);
        pSeninfDrv->resetCSI2();//reset CSI2
        ret = pSensorDrv->open();
        if (ret < 0) {
            LOG_ERR("[reset] pSensorDrv->open fail \n");
        }    
        usleep(10);
        pSensorDrv->setScenario(mSensorScenarioId,mCameraId);
    //}
    //else {
    //    LOG_MSG("[reset] Seninf input normal, no need reset!\n");
    //}
    
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_SENSOR_INFO_S &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_SENSOR_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/

MINT32 SensorHalImp::setDebugInfo(IBaseCamExif *pIBaseCamExif) 
{

    DEBUG_SENSOR_INFO_T sensorDebugInfo;
    MUINT32 exifId;
    MINT32 ret = 0;

    //Exif debug info
#ifndef  USING_MTK_LDVT  
    
        setDebugTag(sensorDebugInfo, SENSOR_TAG_VERSION, (MUINT32)SENSOR_DEBUG_TAG_VERSION);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_COLORORDER, (MUINT32)sensorInfo[0].SensorOutputDataFormat);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_DATATYPE, (MUINT32)mImageSensorType[0]);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_HARDWARE_INTERFACE, (MUINT32)sensorInfo[0].SensroInterfaceType);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_X, (MUINT32)sensorDebug[0].u4GrabX);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_Y, (MUINT32)sensorDebug[0].u4GrabY);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_WIDTH, (MUINT32)sensorDebug[0].u4CropW);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_HEIGHT, (MUINT32)sensorDebug[0].u4CropH);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_COLORORDER, (MUINT32)sensorInfo[1].SensorOutputDataFormat);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_DATATYPE, (MUINT32)mImageSensorType[1]);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_HARDWARE_INTERFACE, (MUINT32)sensorInfo[1].SensroInterfaceType);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_X, (MUINT32)sensorDebug[1].u4GrabX);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_Y, (MUINT32)sensorDebug[1].u4GrabY);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_WIDTH, (MUINT32)sensorDebug[1].u4CropW);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_HEIGHT, (MUINT32)sensorDebug[1].u4CropH);

        ret = pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_CAM_SENSOR_MID, reinterpret_cast<MINT32>(&exifId));
        ret = pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, exifId, reinterpret_cast<MINT32>(&sensorDebugInfo), sizeof(DEBUG_SENSOR_INFO_T));        
       
#endif

    return ret;
}



