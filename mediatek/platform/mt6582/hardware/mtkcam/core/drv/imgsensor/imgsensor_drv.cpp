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

#define LOG_TAG "ImgSensorDrv"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include <camera_custom_imgsensor_cfg.h>
#include "mtkcam/common.h"
#include "imgsensor_drv.h"
#include "kd_imgsensor.h"



/*******************************************************************************
*
********************************************************************************/
SensorDrv*
ImgSensorDrv::
getInstance()
{
    //LOG_MSG("[ImgSensorDrv] getInstance \n");
    static ImgSensorDrv singleton;

    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
ImgSensorDrv::
destroyInstance() 
{
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
ImgSensorDrv()
    : SensorDrv()
    , m_fdSensor(-1)
    , m_mainSensorId(SENSOR_DOES_NOT_EXIST)
    , m_subSensorId(SENSOR_DOES_NOT_EXIST)
    , m_mainSensorIdx(BAD_SENSOR_INDEX)
    , m_subSensorIdx(BAD_SENSOR_INDEX)
    , m_pMainSensorInfo(NULL)
    , m_pSubSensorInfo(NULL)
    , m_pstSensorInitFunc(NULL)
    , mUsers(0)

{
    m_LineTimeInus[0] = 31;
    m_LineTimeInus[1] = 31;    
    memset(&m_SenosrResInfo[0], 0, sizeof(m_SenosrResInfo));
    memset(&m_SenosrResInfo[1], 0, sizeof(m_SenosrResInfo));
    //    
    memset((void*)&m_mainSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    memset((void*)&m_subSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    m_mainSensorDrv.number = 0;
    m_subSensorDrv.number = 0;
    memset(&m_sensorInfo[0], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&m_sensorInfo[1], 0, sizeof(ACDK_SENSOR_INFO_STRUCT));    
    memset(&m_sensorConfigData[0], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    memset(&m_sensorConfigData[1], 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));   
    m_psensorInfo[0] = &m_sensorInfo[0];
    m_psensorInfo[1] = &m_sensorInfo[1];  
    m_psensorConfigData[0] = &m_sensorConfigData[0];
    m_psensorConfigData[1] = &m_sensorConfigData[1];    
    
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
~ImgSensorDrv()
{
    m_mainSensorIdx = BAD_SENSOR_INDEX; 
    m_subSensorIdx = BAD_SENSOR_INDEX;
    LOG_MSG ("[~ImgSensorDrv]\n");
}

/*******************************************************************************
*
********************************************************************************/

MINT32 
ImgSensorDrv::impSearchSensor(pfExIdChk pExIdChkCbf)
{
    MUINT32 SensorEnum = (MUINT32) DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 i,id[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    MBOOL SensorConnect=TRUE;
    UCHAR cBuf[64];
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 err2 = SENSOR_NO_ERROR;
    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;
    MINT32 sensorDevs = SENSOR_NONE;
    IMAGE_SENSOR_TYPE sensorType = IMAGE_SENSOR_TYPE_UNKNOWN;
    IMGSENSOR_SOCKET_POSITION_ENUM socketPos = IMGSENSOR_SOCKET_POS_NONE;


    //! If imp sensor search process already done before, 
    //! only need to return the sensorDevs, not need to 
    //! search again. 
    if (SENSOR_DOES_NOT_EXIST != m_mainSensorId) {
        //been processed.
        LOG_MSG("[impSearchSensor] Already processed \n"); 
        if (BAD_SENSOR_INDEX != m_mainSensorIdx) {
            sensorDevs |= SENSOR_MAIN;
        }

        if (BAD_SENSOR_INDEX != m_subSensorIdx) {
            sensorDevs |= SENSOR_SUB;
        }

        #ifdef  ATVCHIP_MTK_ENABLE

            sensorDevs |= SENSOR_ATV;

        #endif


        return sensorDevs; 
    }
    
    GetSensorInitFuncList(&m_pstSensorInitFunc);

    LOG_MSG("SENSOR search start \n");

    if (-1 != m_fdSensor) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }
    sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
    m_fdSensor = ::open(cBuf, O_RDWR);
    if (m_fdSensor < 0) {
         LOG_ERR("[impSearchSensor]: error opening %s: %s \n", cBuf, strerror(errno)); 
        return sensorDevs;
    }
    LOG_MSG("[impSearchSensor] m_fdSensor = %d  \n", m_fdSensor);

    // search main/main_2/sub 3 sockets
   #ifdef MTK_SUB_IMGSENSOR 
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum <= DUAL_CAMERA_SUB_SENSOR; SensorEnum <<= 1)  {
        LOG_MSG("impSearchSensor search to sub\n");
   #else
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum < DUAL_CAMERA_SUB_SENSOR; SensorEnum <<= 1)  {
        LOG_MSG("impSearchSensor search to main\n");   
   #endif     
        //skip atv case
        if ( 0x04 == SensorEnum ) continue;
        //
        for (i = 0; i < MAX_NUM_OF_SUPPORT_SENSOR; i++) {
            //end of driver list
            if (m_pstSensorInitFunc[i].getCameraDefault == NULL) {
                LOG_MSG("m_pstSensorInitFunc[i].getCameraDefault is NULL: %d \n", i);                
                break;
            }
                //set sensor driver
            id[KDIMGSENSOR_INVOKE_DRIVER_0] = (SensorEnum << KDIMGSENSOR_DUAL_SHIFT) | i;
            LOG_MSG("set sensor driver id =%x\n", id[KDIMGSENSOR_INVOKE_DRIVER_0]); 
            err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,&id[KDIMGSENSOR_INVOKE_DRIVER_0] );
                if (err < 0) {
                    LOG_ERR("ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
                }

           

                //err = open();
                err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CHECK_IS_ALIVE);
                

                
                if (err < 0) {
                    LOG_MSG("[impSearchSensor] Err-ctrlCode (%s) \n", strerror(errno));
                }
            //
            sensorType = this->getCurrentSensorType((SENSOR_DEV_ENUM)SensorEnum);
            //
            socketPos = this->getSocketPosition((CAMERA_DUAL_CAMERA_SENSOR_ENUM)SensorEnum);
                //check extra ID , from EEPROM maybe
                //may need to keep power here
                if (NULL != pExIdChkCbf) {
                    err2 = pExIdChkCbf();
                    if (err2 < 0) {
                        LOG_ERR("Error:pExIdChkCbf() \n");
                    }
                }

                //power off sensor
                close();

                if (err < 0 || err2 < 0) {
                    LOG_MSG("sensor ID mismatch\n");
                }
                else {
                    if (SensorEnum == DUAL_CAMERA_MAIN_SENSOR) {
                //m_mainSensorIdx = i;
                //m_mainSensorId = m_pstSensorInitFunc[m_mainSensorIdx].SensorId;
                m_mainSensorDrv.index[m_mainSensorDrv.number] = i;
                m_mainSensorDrv.type[m_mainSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstRawIndex ) {
                    m_mainSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstYuvIndex ) {
                    m_mainSensorDrv.firstYuvIndex = i;
                }
                m_mainSensorDrv.position = socketPos;
                m_mainSensorDrv.sensorID = m_pstSensorInitFunc[m_mainSensorDrv.index[m_mainSensorDrv.number]].SensorId;
                // LOG_MSG("MAIN sensor m_mainSensorDrv.number=%d, m_mainSensorDrv.index=%d\n",m_mainSensorDrv.number,m_mainSensorDrv.index[m_mainSensorDrv.number]);
                m_mainSensorDrv.number++;
                //
                m_pMainSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                if  ( m_pMainSensorInfo )
                {
                    NSFeature::SensorInfoBase* pSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                    LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
                }
                else
                {
                    LOG_WRN("m_pMainSensorInfo==NULL\n");
                }
                LOG_MSG("MAIN sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
            else if (SensorEnum == DUAL_CAMERA_SUB_SENSOR) {
                //m_subSensorIdx = i;
                //m_subSensorId = m_pstSensorInitFunc[m_subSensorIdx].SensorId;
                m_subSensorDrv.index[m_subSensorDrv.number] = i;
                m_subSensorDrv.type[m_subSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstRawIndex ) {
                    m_subSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstYuvIndex ) {
                    m_subSensorDrv.firstYuvIndex = i;
                }
                m_subSensorDrv.position = socketPos;
                m_subSensorDrv.sensorID = m_pstSensorInitFunc[m_subSensorDrv.index[m_subSensorDrv.number]].SensorId;
                //LOG_MSG("SUB sensor m_subSensorDrv.number=%d, m_subSensorDrv.index=%d\n",m_subSensorDrv.number,m_subSensorDrv.index[m_subSensorDrv.number]);
                m_subSensorDrv.number++;
                //
                m_pSubSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                if  ( m_pSubSensorInfo )
                {
                    NSFeature::SensorInfoBase* pSensorInfo = m_pstSensorInitFunc[i].pSensorInfo;
                    LOG_MSG("found <%#x/%s/%s>", pSensorInfo->GetID(), pSensorInfo->getDrvName(), pSensorInfo->getDrvMacroName());
                }
                else
                {
                    LOG_WRN("m_pSubSensorInfo==NULL\n");
                }
                LOG_MSG("SUB sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
        }//
        
        }
    }
    //close system call may be off sensor power. check first!!!
    ::close(m_fdSensor);
    m_fdSensor = -1;
    //
    if (BAD_SENSOR_INDEX != m_mainSensorDrv.index[0]) {
        m_mainSensorId = m_mainSensorDrv.sensorID;
        //init to choose first
        m_mainSensorIdx = m_mainSensorDrv.index[0];
        sensorDevs |= SENSOR_MAIN;
    }

    if (BAD_SENSOR_INDEX != m_subSensorDrv.index[0]) {
        m_subSensorId = m_subSensorDrv.sensorID;
        //init to choose first
        m_subSensorIdx = m_subSensorDrv.index[0];
        sensorDevs |= SENSOR_SUB;
    }

    #ifdef  ATVCHIP_MTK_ENABLE
    
        sensorDevs |= SENSOR_ATV;
    
    #endif


    if (sensorDevs == SENSOR_NONE) {
        LOG_ERR( "Error No sensor found!! \n");
    }
    //
    LOG_MSG("SENSOR search end: 0x%x /[0x%x][%d]/[0x%x][%d]\n", sensorDevs,
    m_mainSensorId,m_mainSensorIdx,m_subSensorId,m_subSensorIdx);

    return sensorDevs;
}//


/*******************************************************************************
*
********************************************************************************/

#define N2D_PRIORITY_DRIVER Yuv
#define N3D_PRIORITY_DRIVER Yuv
#define _SELECT_PRIORITY_DRIVER_(a,b)    do { if ( m_##a##SensorDrv.number > 1 && BAD_SENSOR_INDEX != m_##a##SensorDrv.first##b##Index ) { \
                                        m_##a##SensorIdx = m_##a##SensorDrv.first##b##Index; }}while(0)
#define SELECT_PRIORITY_DRIVER(a,b) _SELECT_PRIORITY_DRIVER_(a,b)

#define IMGSNESOR_FILL_SET_DRIVER_INFO(a) do{ \
        if ( DUAL_CAMERA_MAIN_SENSOR & a ) {    \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_MAIN_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_mainSensorIdx; \
        }   \
        if ( DUAL_CAMERA_SUB_SENSOR & a ) { \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_SUB_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_subSensorIdx;    \
        }\
    }while(0);

MINT32
ImgSensorDrv::init(
    MINT32 sensorIdx
)
{
    UCHAR cBuf[64];
    MINT32 ret = SENSOR_NO_ERROR;
    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;
    MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    IMAGE_SENSOR_TYPE sensorType_prioriy = IMAGE_SENSOR_TYPE_UNKNOWN;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResInfo[2];
    SENSOR_DEV_ENUM sensorDevId;

    //ONLY "main/main_2" can be ON simultaneously
    if ( ( 0 == sensorIdx ) ||
         ( 0x04 & sensorIdx ) ||
         ( ( DUAL_CAMERA_SUB_SENSOR & sensorIdx ) && ( DUAL_CAMERA_SUB_SENSOR ^ sensorIdx ) ) ) {
        LOG_ERR("invalid sensorIdx[0x%08x] \n",sensorIdx);
        return SENSOR_INVALID_PARA;
    }
    //select driver

        //2D mode
        SELECT_PRIORITY_DRIVER(main,N2D_PRIORITY_DRIVER);
        //SELECT_PRIORITY_DRIVER(main2,N2D_PRIORITY_DRIVER);
        SELECT_PRIORITY_DRIVER(sub,N2D_PRIORITY_DRIVER);

    //
    IMGSNESOR_FILL_SET_DRIVER_INFO(sensorIdx);
    //
    LOG_MSG("[init] mUsers = %d\n", mUsers); 
    Mutex::Autolock lock(mLock);

    if (mUsers == 0) {
        if (m_fdSensor == -1) {
            sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
            m_fdSensor = ::open(cBuf, O_RDWR);
            if (m_fdSensor < 0) {
                LOG_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));                
                return SENSOR_INVALID_DRIVER;
            }
        }
    }
    LOG_MSG("[init] m_fdSensor = %d  \n", m_fdSensor);

    //set sensor driver
    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInfo);
    if (ret < 0) {
       LOG_ERR("[init]: ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
    }

    android_atomic_inc(&mUsers); 

    //init. resolution
    pSensorResInfo[0] = &m_SenosrResInfo[0];
    pSensorResInfo[1] = &m_SenosrResInfo[1];

    ret = getResolution(pSensorResInfo);
    if (ret < 0) {
        LOG_ERR("[init]: Get Resolution error\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    if(SENSOR_MAIN & sensorIdx ) {
        sensorDevId = SENSOR_MAIN;

        //calculater g_LineTimeInus for exposure time convert.
        FeatureParaLen = sizeof(MUINT32);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
           LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
           return SENSOR_UNKNOWN_ERROR;
        }    

        FeatureParaLen = sizeof(pFeaturePara16);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
            LOG_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
            return SENSOR_UNKNOWN_ERROR;
        }

        if (FeaturePara32) {
            //in setting domain, use preview line time only
            //sensor drv will convert to capture line time when setting to capture mode.
            m_LineTimeInus[0] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
        }
        LOG_MSG("[init]: m_LineTimeInus[0] = %d\n", m_LineTimeInus[0] );
    }

    if(SENSOR_SUB & sensorIdx) {
            sensorDevId = SENSOR_SUB;


        //calculater g_LineTimeInus for exposure time convert.
        FeatureParaLen = sizeof(MUINT32);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
           LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
           return SENSOR_UNKNOWN_ERROR;
        }    

        FeatureParaLen = sizeof(pFeaturePara16);
        ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
            LOG_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
            return SENSOR_UNKNOWN_ERROR;
        }

        if (FeaturePara32) {
            //in setting domain, use preview line time only
            //sensor drv will convert to capture line time when setting to capture mode.
            m_LineTimeInus[1] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
        }
        LOG_MSG("[init]: m_LineTimeInus[1] = %d\n", m_LineTimeInus[1] );        
    }


    return SENSOR_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::uninit(
)
{
    //MHAL_LOG("[halSensorUninit] \n");
    MINT32 ret = SENSOR_NO_ERROR;

#if 0 
    ret = close();
    if (ret < 0)
    {
        LOG_ERR("[uninit] SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
        return MHAL_UNKNOWN_ERROR;
    }
#endif 

    //pthread_mutex_lock(&m_sensorMutex);        
    LOG_MSG("[uninit] mUsers = %d\n", mUsers);     

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return SENSOR_NO_ERROR;
    }

    if (mUsers == 1) {
        if (m_fdSensor > 0) {
            ::close(m_fdSensor);
        }
        m_fdSensor = -1;            
    }
    android_atomic_dec(&mUsers); 
    //m_userCnt --; 
    //pthread_mutex_unlock(&m_sensorMutex);        


    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setScenario(ACDK_SCENARIO_ID_ENUM sId[2],SENSOR_DEV_ENUM sensorDevId[2])
{
    MINT32 ret = SENSOR_NO_ERROR;
    MUINT32 i = 0;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow[2];
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData[2];
    
    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;

    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;

    for(i=0; i<KDIMGSENSOR_MAX_INVOKE_DRIVERS; i++) {
        if(ACDK_SCENARIO_ID_MAX != sId[i]) {
            SensorConfigData[i].SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;
            
            switch(sId[i])
            {        
                case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
                case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorPreviewWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorPreviewHeight;
                    break;
                case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
                case ACDK_SCENARIO_ID_CAMERA_ZSD:
                case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
                    SensorConfigData[i].EnableShutterTansfer = FALSE;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorFullWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorFullHeight;
                    break;
                case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
                    SensorConfigData[i].SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_VIDEO;
                    ImageWindow[i].ImageTargetWidth = m_SenosrResInfo[i].SensorVideoWidth;
                    ImageWindow[i].ImageTargetHeight = m_SenosrResInfo[i].SensorVideoHeight;
                    break;                    
                default:
                    LOG_ERR("[setScenario] error scenario id\n");
                    return SENSOR_UNKNOWN_ERROR;
            }

            //set sensor preview/capture mode

            sensorCtrl.ScenarioId = (MSDK_SCENARIO_ID_ENUM)sId[i];
            sensorCtrl.pImageWindow = &ImageWindow[i];
            sensorCtrl.pSensorConfigData = &SensorConfigData[i];

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_CONTROL , &sensorCtrl);
            if (ret < 0) {
                LOG_ERR("[setScenario]Err-ctrlCode (%s) \n", strerror(errno));
                return -errno;
            }
            //get exposure line time for each scenario
            FeatureParaLen = sizeof(MUINT32);
            ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId[i],SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
            if (ret < 0) {
               LOG_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
               return SENSOR_UNKNOWN_ERROR;
            }    
        
            FeatureParaLen = sizeof(pFeaturePara16);
            ret = featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId[i], SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
            if (ret < 0) {
                LOG_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error\n");
                return SENSOR_UNKNOWN_ERROR;
            }

            if (FeaturePara32) {
                if(FeaturePara32 >= 1000) {
                    m_LineTimeInus[i] = (MUINT32)(((MUINT64)pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
                } 
                else {
                    LOG_WRN("[setScenario]: Sensor clock too slow = %d %d\n", FeaturePara32, pFeaturePara16[0]);        
                }
                LOG_MSG("[setScenario] m_LineTimeInus[%d] = %d Scenario id = %d PixelClk = %d PixelInLine = %d\n", i,m_LineTimeInus[i], sId[i], FeaturePara32, pFeaturePara16[0]); 
            }

        }
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::start(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::stop(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/

MINT32 
ImgSensorDrv::getSensorDelayFrameCnt(
    SENSOR_DEV_ENUM sensorDevId,
    halSensorDelayFrame_e mode
)
{

    ACDK_SCENARIO_ID_ENUM scenarioId[2] = {ACDK_SCENARIO_ID_CAMERA_PREVIEW,ACDK_SCENARIO_ID_CAMERA_PREVIEW};


    
    LOG_MSG("[getSensorDelayFrameCnt] mode = %d\n", mode); 
    if (SENSOR_NO_ERROR != getInfo(scenarioId, m_psensorInfo, m_psensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo() \n");
       return 0;
    }    


    if(SENSOR_MAIN == sensorDevId) {
        if ( SENSOR_PREVIEW_DELAY == mode) {
            return m_psensorInfo[0]->PreviewDelayFrame; 
        }
        else if (SENSOR_VIDEO_DELAY == mode) {
            return m_psensorInfo[0]->VideoDelayFrame; 
        }
        else if (SENSOR_CAPTURE_DELAY == mode) {
            return m_psensorInfo[0]->CaptureDelayFrame;
        }
        else if (SENSOR_YUV_AWB_SETTING_DELAY == mode) {
            return m_psensorInfo[0]->YUVAwbDelayFrame; 
        }
        else if (SENSOR_YUV_EFFECT_SETTING_DELAY == mode) {
            return m_psensorInfo[0]->YUVEffectDelayFrame;
        }
        else if (SENSOR_AE_SHUTTER_DELAY == mode) {
            return m_psensorInfo[0]->AEShutDelayFrame;
        }
        else if (SENSOR_AE_GAIN_DELAY == mode) {
            return m_psensorInfo[0]->AESensorGainDelayFrame;
        }
        else if (SENSOR_AE_ISP_DELAY == mode) {
            return m_psensorInfo[0]->AEISPGainDelayFrame;
        }        
        else {
            return 0;
        }
    }
    else if(SENSOR_SUB == sensorDevId) {
        if ( SENSOR_PREVIEW_DELAY == mode) {
            return m_psensorInfo[1]->PreviewDelayFrame; 
        }
        else if (SENSOR_VIDEO_DELAY == mode) {
            return m_psensorInfo[1]->VideoDelayFrame; 
        }
        else if (SENSOR_CAPTURE_DELAY == mode) {
            return m_psensorInfo[1]->CaptureDelayFrame;
        }
        else if (SENSOR_YUV_AWB_SETTING_DELAY == mode) {
            return m_psensorInfo[1]->YUVAwbDelayFrame; 
        }
        else if (SENSOR_YUV_EFFECT_SETTING_DELAY == mode) {
            return m_psensorInfo[1]->YUVEffectDelayFrame;
        }  
        else if (SENSOR_AE_SHUTTER_DELAY == mode) {
            return m_psensorInfo[1]->AEShutDelayFrame;
        }
        else if (SENSOR_AE_GAIN_DELAY == mode) {
            return m_psensorInfo[1]->AESensorGainDelayFrame;
        }
        else if (SENSOR_AE_ISP_DELAY == mode) {
            return m_psensorInfo[1]->AEISPGainDelayFrame;
        }         
        else {
            return 0;
        }
    }
    else {
        LOG_ERR("[getSensorDelayFrameCnt] Error sensorDevId ! \n");
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/       


MINT32
ImgSensorDrv::waitSensorEventDone(
    MUINT32 EventType,    
    MUINT32 Timeout
)
{
    MINT32 ret = 0;
    LOG_MSG("[ImgSensorDrv]waitSensorEventDone: EventType = %d, Timeout=%d\n",EventType,Timeout);
    switch (EventType) {
        case WAIT_SENSOR_SET_SHUTTER_GAIN_DONE:
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE, &Timeout);
            break;
        default :
            break;           
    }
    if(ret < 0)
    {
        LOG_MSG("waitSensorEventDone err, Event = %d", EventType);
    }
    return ret;    
}


/*******************************************************************************
*
********************************************************************************/       


MINT32
ImgSensorDrv::sendCommand(
        SENSOR_DEV_ENUM sensorDevId,
        MUINT32 cmd, 
        MUINT32 *parg1, 
        MUINT32 *parg2,
        MUINT32 *parg3        
)
{ 
    MINT32 err = SENSOR_NO_ERROR;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT16 u2FeaturePara=0;
    MUINT8 *pFeaturePara = NULL; 
    MUINT32 u4FeaturePara[4];
    MUINT32 FeatureParaLen = 0;
    MUINT16 *pu2FeaturePara = NULL; 
    MUINT32 *pu4Feature = NULL; 
    MUINT32 *pu4FeaturePara = NULL;

    switch (cmd) {
    case CMD_SENSOR_SET_SENSOR_EXP_TIME:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        if(SENSOR_MAIN == sensorDevId) {
            u2FeaturePara = (MUINT16)((1000*(*parg1))/m_LineTimeInus[0]);
            if(u2FeaturePara ==0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %d %d\n", u2FeaturePara, m_LineTimeInus[0]);  
           	    u2FeaturePara = 1;        	
            }
            FeatureParaLen = sizeof(MUINT16);
            pFeaturePara = (MUINT8*)&u2FeaturePara;
        }
        else if(SENSOR_SUB == sensorDevId) {
            u2FeaturePara = (MUINT16)((1000*(*parg1))/m_LineTimeInus[1]);
            if(u2FeaturePara ==0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %d %d\n", u2FeaturePara, m_LineTimeInus[1]);  
               	u2FeaturePara = 1;        	
            }
            FeatureParaLen = sizeof(MUINT16);
            pFeaturePara = (MUINT8*)&u2FeaturePara;
        }
        else{
            LOG_ERR("sensorDevId is incorrect!!\n");
        }
            
        break;
        
    case CMD_SENSOR_SET_SENSOR_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        u2FeaturePara = (MUINT16)(*parg1);
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara = (MUINT8*)&u2FeaturePara;
        break;        
        
    case CMD_SENSOR_SET_SENSOR_GAIN:
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        u2FeaturePara=(MUINT16)(*parg1 >>= 4); //from 10b to 6b base
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara =  (MUINT8*)&u2FeaturePara;            
        break;
        
    case CMD_SENSOR_SET_FLICKER_FRAME_RATE:
       FeatureId = SENSOR_FEATURE_SET_AUTO_FLICKER_MODE;
       u2FeaturePara = (MUINT16)*parg1; 
       FeatureParaLen = sizeof(MUINT16);
       pFeaturePara =  (MUINT8*)&u2FeaturePara;            
       break;
       
        
    case CMD_SENSOR_SET_VIDEO_FRAME_RATE:
        FeatureId = SENSOR_FEATURE_SET_VIDEO_MODE;
        u2FeaturePara = (MUINT16)*parg1; 
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara =  (MUINT8*)&u2FeaturePara;            
        break; 


    case CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC;
        if(SENSOR_MAIN == sensorDevId) {
            u4FeaturePara[0] = *parg1; // RAW Gain R, Gr
            u4FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
            u4FeaturePara[2] = *(parg1+2);  // Exposure time
            u4FeaturePara[2] = ((1000 * u4FeaturePara[2] )/m_LineTimeInus[0]);
            if(u4FeaturePara[2]  ==0) {   // avoid the line number to zero
                LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] m_LineTime[0] = %d %d\n", u4FeaturePara[2] , m_LineTimeInus[0]);  
           	    u4FeaturePara[2]  = 1;        	
            }
            u4FeaturePara[2] = (u4FeaturePara[2] ) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
            u4FeaturePara[3] = *(parg1+4);  // Delay frame cnt
            FeatureParaLen = sizeof(MUINT32) * 4;
            pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        }
        else if(SENSOR_SUB == sensorDevId) {
            u4FeaturePara[0] = *parg1; // RAW Gain R, Gr
            u4FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
            u4FeaturePara[2] = *(parg1+2);  // Exposure time
            u4FeaturePara[2] = ((1000 * u4FeaturePara[2] )/m_LineTimeInus[1]);
            if(u4FeaturePara[2]  ==0) {   // avoid the line number to zero
               LOG_MSG("[CMD_SENSOR_SET_SENSOR_SYNC] m_LineTime[1] = %d %d\n", u4FeaturePara[2] , m_LineTimeInus[1]);  
           	   u4FeaturePara[2]  = 1;        	
            }
            u4FeaturePara[2] = (u4FeaturePara[2] ) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
            u4FeaturePara[3] = *(parg1+4);  // Delay frame cnt
            FeatureParaLen = sizeof(MUINT32) * 4;
            pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        }
        else{
            LOG_ERR("sensorDevId is incorrect!! \n");
        }        
        break;



    case CMD_SENSOR_SET_CCT_FEATURE_CONTROL:
        FeatureId = (ACDK_SENSOR_FEATURE_ENUM)*parg1;
        pFeaturePara = (MUINT8*)parg2;
        FeatureParaLen = (MUINT32)*parg3;
        break;

    case CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA:
        FeatureId = SENSOR_FEATURE_SET_CALIBRATION_DATA;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_CALIBRATION_DATA_STRUCT);
        break;

    case CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO;
        u4FeaturePara[0] = *parg1; 
        u4FeaturePara[1] = *parg2;         
        FeatureParaLen = sizeof(MUINT32) * 2; 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];  

        break;

    case CMD_SENSOR_SET_TEST_PATTERN_OUTPUT:
        FeatureId = SENSOR_FEATURE_SET_TEST_PATTERN;
        u2FeaturePara = (MUINT16)*parg1; 
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara =  (MUINT8*)&u2FeaturePara; 
        break;

    case CMD_SENSOR_SET_ESHUTTER_GAIN: 
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        u4FeaturePara[0] = *parg1; // exposure time (us)
        if(sensorDevId == SENSOR_MAIN) {
            u4FeaturePara[0] = ((1000 * u4FeaturePara[0] )/m_LineTimeInus[0]);
        }
        else if ((sensorDevId == SENSOR_SUB) || (sensorDevId == SENSOR_MAIN) ) {
            u4FeaturePara[0] = ((1000 * u4FeaturePara[0] )/m_LineTimeInus[1]);
        }
        else {
            LOG_ERR("Error sensorDevId !\n");
        }
        if(u4FeaturePara[0] == 0) {   // avoid the line number to zero
            LOG_MSG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %d %d\n", u4FeaturePara[0] , m_LineTimeInus);  
       	    u4FeaturePara[0]  = 1;        	
        }
        u4FeaturePara[2] = (u4FeaturePara[0] ) | (((*(parg1+1))>>4) << 16); // Sensor gain from 10b to 6b base
        u4FeaturePara[0] = 0; // RAW Gain R, Gr
        u4FeaturePara[1] = 0;  // RAW Gain Gb, B
        u4FeaturePara[3] = 0;  // Delay frame cnt
        LOG_MSG("CMD_SENSOR_SET_ESHUTTER_GAIN: Exp=%d, SensorGain=%d\n", u4FeaturePara[2]&0x0000FFFF, u4FeaturePara[2]>>16);
        FeatureParaLen = sizeof(MUINT32) * 4;
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];    	
    	 break;        
         
    case CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT:
        {
            *parg1 = getSensorDelayFrameCnt(sensorDevId,(halSensorDelayFrame_e)*parg2); 
            return err; 
        }
        break; 

    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        // Bit 0~7 as data input
        switch  (sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor1);
                err = 0;
                break;
             
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_INDATA_FORMAT> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return  err;
        break;

    case CMD_SENSOR_GET_PAD_PCLK_INV:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorPadPclkInv(eDevId_ImgSensor1);
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_PAD_PCLK_INV> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return  err;
        break;

    case CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorOrientation_T orientation;
        orientation = NSCamCustomSensor::getSensorOrientation();
        switch(sensorDevId)
        {
            case SENSOR_MAIN:
                *parg1 = orientation.u4Degree_0;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = orientation.u4Degree_1;
                err = 0;
                break;

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }      
        return err;
        break;
        
    case CMD_SENSOR_GET_SENSOR_FACING_DIRECTION:
        switch(sensorDevId)
        {
            using namespace NSCamCustomSensor;
            case SENSOR_MAIN:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor0);
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = getSensorFacingDirection(eDevId_ImgSensor1);
                err = 0;
                break;
  

            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FACING_DIRECTION> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }
        return  err;
        
        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;     
        break;

    case CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        FeatureId = SENSOR_FEATURE_GET_PERIOD;
        pu4FeaturePara = (MUINT32 *)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;    	    	
    	break;

    case CMD_SENSOR_GET_SENSOR_FEATURE_INFO:
        switch(sensorDevId)
        {
            using namespace NSFeature;
            case SENSOR_MAIN:
                *parg1 = (MUINT32)m_pMainSensorInfo;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = (MUINT32)m_pSubSensorInfo;
                err = 0;
                break;
    
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_FEATURE_INFO> - bad sensor id(%x)", (int)sensorDevId);
                parg1 = NULL;
                err = -1;
                break;
        }
        return  err;
        
        break;
     case CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        FeatureId = SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        u4FeaturePara[0] = *parg1; 
        u4FeaturePara[1] = (MUINT32)parg2;         
        FeatureParaLen = sizeof(MUINT32) * 2; 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];       
        break;
        
    case CMD_SENSOR_GET_FAKE_ORIENTATION:
         switch(sensorDevId)
         {
             using namespace NSCamCustomSensor;
             case SENSOR_MAIN:
                 *parg1 = isRetFakeMainOrientation(); 
                 err = 0;
                 break;
             case SENSOR_SUB:
                 *parg1 = isRetFakeSubOrientation();
                 err = 0;
                 break;

                 
             default:
                 LOG_ERR("[sendCommand]<CMD_SENSOR_GET_FAKE_ORIENTATION> - bad sensor id(%x)", (int)sensorDevId);
                 *parg1 = 0;
                 err = -1;
                 break;
         }
         return  err;
     break;
    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:
        using namespace NSCamCustomSensor;
        NSCamCustomSensor::SensorViewAngle_T viewangle;
        viewangle = NSCamCustomSensor::getSensorViewAngle();
        switch(sensorDevId)
        {
            case SENSOR_MAIN:
                *parg1 = viewangle.MainSensorHorFOV;
                *parg2 = viewangle.MainSensorVerFOV;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = viewangle.SubSensorHorFOV;
                *parg2 = viewangle.SubSensorVerFOV;
                err = 0;
                break;
            default:
                LOG_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)sensorDevId);
                *parg1 = 0;
                err = -1;
                break;
        }      
        return err;
       break;      
      
     case CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE:
        FeatureId = SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE; 
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;           
        break;
        
    case CMD_SENSOR_GET_SENSOR_CURRENT_TEMPERATURE:
       FeatureId = SENSOR_FEATURE_GET_SENSOR_CURRENT_TEMPERATURE; 
       pu4FeaturePara = (MUINT32*)parg1;
       FeatureParaLen = sizeof(MUINT32);
       pFeaturePara = (MUINT8*)pu4FeaturePara;           
       break;

     case CMD_SENSOR_SET_YUV_FEATURE_CMD:        
         FeatureId = SENSOR_FEATURE_SET_YUV_CMD; 
         u4FeaturePara[0] = *parg1; 
         u4FeaturePara[1] = *parg2;         
         FeatureParaLen = sizeof(MUINT32) * 2; 
         pFeaturePara = (MUINT8*)&u4FeaturePara[0];        
         break;

    case CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE:
        FeatureId = SENSOR_FEATURE_SINGLE_FOCUS_MODE;
        //LOG_MSG("CMD_SENSOR_SINGLE_FOCUS_MODE\n");
        break; 
        
        
    case CMD_SENSOR_SET_YUV_CANCEL_AF:
        FeatureId = SENSOR_FEATURE_CANCEL_AF; 
        //LOG_MSG("CMD_SENSOR_CANCEL_AF\n");
        break;

    case CMD_SENSOR_SET_YUV_CONSTANT_AF:
        FeatureId = SENSOR_FEATURE_CONSTANT_AF;
        break;

    case CMD_SENSOR_SET_YUV_AF_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AF_WINDOW;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        //LOG_MSG("zone_addr=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_YUV_AE_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AE_WINDOW;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        //LOG_MSG("AEzone_addr=0x%x\n", u4FeaturePara[0]);
        break;


    case CMD_SENSOR_GET_YUV_AF_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_STATUS,parg1=0x%x,FeatureParaLen=0x%x,pFeaturePara=0x%x\n",
        //parg1, FeatureParaLen, pFeaturePara);	
        break;


    case CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN:
        FeatureId = SENSOR_FEATURE_GET_EV_AWB_REF;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        LOG_MSG("p_ref=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        LOG_MSG("p_cur=0x%x\n", u4FeaturePara[0]);
        break;
        
    case CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;       

    case CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //LOG_MSG("CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;   
       
    case CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO;
        u4FeaturePara[0] = (MUINT32)parg1;
        u4FeaturePara[1] = (MUINT32)parg2;
        FeatureParaLen = sizeof(MUINT32)*2; 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];  
        break;
       
    case CMD_SENSOR_GET_YUV_EXIF_INFO:
        FeatureId = SENSOR_FEATURE_GET_EXIF_INFO;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        LOG_MSG("EXIF_addr=0x%x\n", u4FeaturePara[0]);
        break;   

    case CMD_SENSOR_GET_YUV_DELAY_INFO:
        FeatureId = SENSOR_FEATURE_GET_DELAY_INFO;
        u4FeaturePara[0] = (MUINT32)parg1; 
        FeatureParaLen = sizeof(MUINT32); 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];        
        LOG_MSG("DELAY_INFO=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO;
        u4FeaturePara[0] = (MUINT32)parg1; 
        FeatureParaLen = sizeof(MUINT32); 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];        
        LOG_MSG("FLASHLIGHT_INFO=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO;
        u4FeaturePara[0] = (MUINT32)parg1; 
        FeatureParaLen = sizeof(MUINT32); 
        pFeaturePara = (MUINT8*)parg1;  
        LOG_MSG("TRIGGER_FLASHLIGHT=0x%x\n", parg1);
        break;
       

    case CMD_SENSOR_SET_YUV_3A_CMD:
        FeatureId = SENSOR_FEATURE_SET_YUV_3A_CMD;
        u4FeaturePara[0] = (MUINT32)*parg1; 
        FeatureParaLen = sizeof(MUINT32); 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];  
        LOG_MSG("YUV_3A_CMD=0x%x\n", *parg1);
        break;

       
    case CMD_SENSOR_SET_YUV_AUTOTEST:
        FeatureId = SENSOR_FEATURE_AUTOTEST_CMD;
        u4FeaturePara[0] = (MUINT32)parg1;
        u4FeaturePara[1] = (MUINT32)parg2;
        FeatureParaLen = sizeof(MUINT32)*2; 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];        
        break;     

    default:
        LOG_ERR("[sendCommand]Command ID = %d is undefined\n",cmd);
        return SENSOR_UNKNOWN_ERROR; 
    }

    if (m_fdSensor == -1) {
        LOG_ERR("[sendCommand]m_fdSensor fail");
        return SENSOR_UNKNOWN_ERROR;
    }

    err= featureControl((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDevId, FeatureId,  (MUINT8*)pFeaturePara,(MUINT32*)&FeatureParaLen);
    if (err < 0) {
        LOG_ERR("[sendCommand] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
#if defined(YUV_TUNING_SUPPORT)
/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
char* 
ImgSensorDrv::getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

/*******************************************************************************
*
********************************************************************************/
void 
ImgSensorDrv::customerInit(void)
{
    FILE *fp = NULL; 

    fp = fopen("/data/sensor_init.txt", "r"); 
    if (fp == NULL)
    {
        printf("No Customer Sensor Init File Exist \n"); 
        return; 
    }

    ACDK_SENSOR_REG_INFO_STRUCT sensorReg; 
    memset (&sensorReg, 0, sizeof(sensorReg)); 
    UINT32 featureLen = sizeof(ACDK_SENSOR_REG_INFO_STRUCT); 
    
    char addrStr[20]; 
    char dataStr[20]; 
    LOG_MSG("[Write Customer Sensor Init Reg]:\n"); 
    fgets(dataStr, 20, fp); 
    if (strncmp(dataStr, "mt65xx_yuv_tuning", 17) != 0)
    {
        LOG_ERR("Error Password \n"); 
        fclose(fp); 
        return; 
    }

    while (!feof(fp))
    {
        fscanf(fp, "%s %s\n", addrStr, dataStr); 
        if (strlen(addrStr) != 0 && strlen(dataStr) != 0)
        {
            u32 addrVal = 0; 
            u32 dataVal = 0; 

            getHexToken(addrStr, &addrVal); 
            getHexToken(dataStr, &dataVal); 

            LOG_MSG("Addr:0x%x, data:0x%x\n", addrVal, dataVal); 
            sensorReg.RegAddr = addrVal; 
            sensorReg.RegData = dataVal;         

            featureControl(DUAL_CAMERA_MAIN_SENSOR,SENSOR_FEATURE_SET_REGISTER, (MUINT8 *)&sensorReg, (MUINT32 *)&featureLen); 
        }
    }
    
    fclose(fp); 
}
#endif 


/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::open()
{
    MINT32 err = SENSOR_NO_ERROR;


    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_OPEN);
    if (err < 0) {
        LOG_MSG("[open] Err-ctrlCode (%s) \n", strerror(errno));
 
        return -errno;
    }

#if  defined(YUV_TUNING_SUPPORT )
    //customerInit(); 
#endif 
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::close()
{
    MINT32 err = SENSOR_NO_ERROR;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CLOSE);
    if (err < 0) {
        LOG_ERR("[close] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }


    return err;
}

/*******************************************************************************
*
********************************************************************************/

MINT32 
ImgSensorDrv::getInfo(
    ACDK_SCENARIO_ID_ENUM ScenarioId[2],
    ACDK_SENSOR_INFO_STRUCT *pSensorInfo[2],
    ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData[2]
)
{
    ACDK_SENSOR_GETINFO_STRUCT getInfo;   
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 i=0;

    LOG_MSG("[getInfo] \n"); 

    
    if (NULL == pSensorInfo|| NULL == pSensorConfigData) {
        LOG_ERR("[getInfo] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    for(i=0; i<2; i++) {
        getInfo.ScenarioId[i] = (MSDK_SCENARIO_ID_ENUM)ScenarioId[i];
        getInfo.pInfo[i] = pSensorInfo[i];
        getInfo.pConfig[i] = pSensorConfigData[i];
          
    }

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO , &getInfo);
  
    
    if (err < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::getResolution(
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution[2]
)
{
    MINT32 err = SENSOR_NO_ERROR;
    if (NULL == pSensorResolution) {
        LOG_ERR("[getResolution] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETRESOLUTION , pSensorResolution);
    if (err < 0) {
        LOG_ERR("[getResolution] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    
    return err;
    
}//halSensorGetResolution

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::featureControl(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM InvokeCamera,
    ACDK_SENSOR_FEATURE_ENUM FeatureId,
    MUINT8 *pFeaturePara,
    MUINT32 *pFeatureParaLen
)
{
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MINT32 err = SENSOR_NO_ERROR;

    if(SENSOR_FEATURE_SINGLE_FOCUS_MODE == FeatureId || SENSOR_FEATURE_CANCEL_AF == FeatureId 
		|| SENSOR_FEATURE_CONSTANT_AF == FeatureId){
    //AF INIT || AF constant has no parameters
    }
    else{
		
        if (NULL == pFeaturePara || NULL == pFeatureParaLen) {
            return SENSOR_INVALID_PARA;
        }
    }		
    featureCtrl.InvokeCamera = InvokeCamera;
    featureCtrl.FeatureId = FeatureId;
    featureCtrl.pFeaturePara = pFeaturePara;
    featureCtrl.pFeatureParaLen = pFeatureParaLen;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    if (err < 0) {
        LOG_ERR("[featureControl] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}//halSensorFeatureControl

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::setFoundDrvsActive(
MUINT32 socketIdxes
)
{
    MINT32 err = SENSOR_NO_ERROR;
    MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};

    IMGSNESOR_FILL_SET_DRIVER_INFO(socketIdxes);

    LOG_MSG("[%s][0x%x] \n",__FUNCTION__,socketIdxes);
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInfo);
    if (err < 0) {
        LOG_ERR("ERROR:setFoundDrvsActive\n");
    }
    return err;
}
/*******************************************************************************
*
********************************************************************************/

IMAGE_SENSOR_TYPE 
ImgSensorDrv::getCurrentSensorType(
    SENSOR_DEV_ENUM sensorDevId
) 
{

    ACDK_SCENARIO_ID_ENUM scenarioId[2] = {ACDK_SCENARIO_ID_CAMERA_PREVIEW,ACDK_SCENARIO_ID_CAMERA_PREVIEW};

    LOG_MSG("[getCurrentSensorType] \n"); 

     
    if (SENSOR_NO_ERROR != getInfo(scenarioId, m_psensorInfo, m_psensorConfigData)) {
       LOG_ERR("[searchSensor] Error:getInfo() \n");
       return IMAGE_SENSOR_TYPE_UNKNOWN;
    }    

    if(SENSOR_MAIN == sensorDevId ) {

        if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B && 
             m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
            return IMAGE_SENSOR_TYPE_RAW;
        }
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B && 
             m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
            return IMAGE_SENSOR_TYPE_RAW8;
        }        
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY && 
                    m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
            return IMAGE_SENSOR_TYPE_YUV; 
        }
        else if (m_psensorInfo[0]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                    m_psensorInfo[0]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
            return IMAGE_SENSOR_TYPE_YCBCR; 
        }
        else {
            return IMAGE_SENSOR_TYPE_UNKNOWN; 
        }
    }
    else if(SENSOR_SUB == sensorDevId) {

        if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B && 
             m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
            return IMAGE_SENSOR_TYPE_RAW;
        }
        if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B && 
             m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
            return IMAGE_SENSOR_TYPE_RAW8;
        }        
        else if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY && 
                    m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
            return IMAGE_SENSOR_TYPE_YUV; 
        }
        else if (m_psensorInfo[1]->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                    m_psensorInfo[1]->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
            return IMAGE_SENSOR_TYPE_YCBCR; 
        }
        else {
            return IMAGE_SENSOR_TYPE_UNKNOWN; 
        }
    }
    else {
        LOG_ERR("[getCurrentSensorType] Error sensorDevId ! \n");
    }
    


    return IMAGE_SENSOR_TYPE_UNKNOWN; 
}



/*******************************************************************************
*
********************************************************************************/
IMGSENSOR_SOCKET_POSITION_ENUM
ImgSensorDrv::getSocketPosition(
CAMERA_DUAL_CAMERA_SENSOR_ENUM socket) {
MUINT32 socketPos = socket;
MINT32 err = SENSOR_NO_ERROR;
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_SOCKET_POS , &socketPos);
    if (err < 0) {
        LOG_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return IMGSENSOR_SOCKET_POS_NONE;
    }

    LOG_MSG("[%s]:[%d][%d] \n",__FUNCTION__,socket,socketPos);

    return (IMGSENSOR_SOCKET_POSITION_ENUM)socketPos;
}


