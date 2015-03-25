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

#define LOG_TAG "AtvSensorDrv"

#ifdef ATVCHIP_MTK_ENABLE 
extern "C" {
#include "kal_release.h"
#include "matvctrl.h"
}

#include "ATVCtrl.h"
#include "ATVCtrlService.h"
///#include "RefBase.h"
///#include "threads.h"
#include <binder/IServiceManager.h>
using namespace android;
#endif 

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>
#include "atvsensor_drv.h"
#include <camera_custom_if.h>

/*******************************************************************************
*
********************************************************************************/
#ifdef ATVCHIP_MTK_ENABLE  
extern int is_factory_boot(void);
extern int matvdrv_exist();

/*
#define ATV_MODE_NTSC 30000
#define ATV_MODE_PAL  25000

******************************************************************
customized delay
******************************************************************
//unit: us
#define ATV_MODE_NTSC_DELAY 5000
#define ATV_MODE_PAL_DELAY  10000

unsigned int matv_cust_get_disp_delay(int mode) {
    return ((ATV_MODE_NTSC == mode)?ATV_MODE_NTSC_DELAY:((ATV_MODE_PAL == mode)?ATV_MODE_PAL_DELAY:0));
}
*/
#endif




/*******************************************************************************
*
********************************************************************************/
SensorDrv*
AtvSensorDrv::
getInstance()
{
    LOG_MSG("[AtvSensorDrv] getInstance \n");
    static AtvSensorDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
AtvSensorDrv::
destroyInstance() 
{
}

/*******************************************************************************
*
********************************************************************************/
AtvSensorDrv::
AtvSensorDrv()
    : SensorDrv()
    , m_SenosrResInfo()
{
    memset(&m_SenosrResInfo, 0, sizeof(m_SenosrResInfo));
}

/*******************************************************************************
*
********************************************************************************/
AtvSensorDrv::
~AtvSensorDrv()
{
}


/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::init(MINT32 sensorIdx)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::uninit()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::setScenario(ACDK_SCENARIO_ID_ENUM sId[2],SENSOR_DEV_ENUM sensorDevId[2])
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::start()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::stop()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::sendCommand(
    SENSOR_DEV_ENUM sensorDevId,
    MUINT32 cmd, 
    MUINT32 *parg1, 
    MUINT32 *parg2,
    MUINT32 *parg3
)
{ 
    MINT32 err = 0;

//add by tengfei mtk80343
    //TODO:recover later
    
    switch (cmd) {
        
#ifdef  ATVCHIP_MTK_ENABLE        
    case CMD_SENSOR_GET_INPUT_BIT_ORDER:
        {            
            MINT32 AtvInputdata = NSCamCustom::get_atv_input_data();
            ///*parg1 = 0;   ///1        
            if( 0 == AtvInputdata )
            {
                *parg1 = 0;
            }
            // Bit 0~7 as data input
            else
            {
                *parg1 = 1;
            }
        }  
        break;

    case CMD_SENSOR_GET_ATV_DISP_DELAY_FRAME:
        {
            char value[PROPERTY_VALUE_MAX] = {'\0'};     
            property_get("atv.disp.delay", value, "-1");
            MINT32 atvDelayTime = atoi(value); 

            if(atvDelayTime != -1)
            {
                if( atvDelayTime < 0)
                {
                    atvDelayTime = 0;
                }

                *parg1 = atvDelayTime;
                LOG_MSG("get atv display delay time through property_get() \n");
            }
            else
            {
                *parg1 = atvGetDispDelay();
                LOG_MSG("get atv display delay time from custom file definition \n");
            }

            LOG_MSG("atv display real delay time is %dus \n", *parg1);
        }
        break;
#endif
    case CMD_SENSOR_GET_FAKE_ORIENTATION:
             *parg1 = 0; 
     break;
    case CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT:   //unstable frame
        *(MUINT32*)parg1 = 0; 
        break; 
    case CMD_SENSOR_GET_PAD_PCLK_INV:
        *(MUINT32*)parg1 = 0; 
        break;
    default:
//Return OK for compatible with aaa_hal_yuv usage
//        err = -1;
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::open()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::close()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::getInfo(ACDK_SCENARIO_ID_ENUM ScenarioId[2],ACDK_SENSOR_INFO_STRUCT *pSensorInfo[2],ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData[2])
{
    
    if (NULL != pSensorInfo[0] && NULL != pSensorConfigData[0]) {

        //decide swapY swapCbCr
        pSensorInfo[0]->SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_YUYV;///SENSOR_OUTPUT_FORMAT_UYVY;    
        
        pSensorInfo[0]->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
        pSensorInfo[0]->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
        pSensorInfo[0]->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

#ifdef MTK_MATV_SERIAL_IF_SUPPORT		
        pSensorInfo[0]->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;//SENSOR_CLOCK_POLARITY_HIGH;
        pSensorInfo[0]->SensroInterfaceType = SENSOR_INTERFACE_TYPE_SERIAL;//SENSOR_INTERFACE_TYPE_PARALLEL;
#else
        pSensorInfo[0]->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
        pSensorInfo[0]->SensroInterfaceType = SENSOR_INTERFACE_TYPE_PARALLEL;
#endif
        pSensorInfo[0]->SensorMasterClockSwitch = 0;
        pSensorInfo[0]->SensorDrivingCurrent = ISP_DRIVING_2MA;
        pSensorInfo[0]->SensorClockFreq = 26;        
        pSensorInfo[0]->SensorClockDividCount=	3;
        pSensorInfo[0]->SensorClockRisingCount= 0;
        pSensorInfo[0]->SensorClockFallingCount= 2;
        pSensorInfo[0]->SensorPixelClockCount= 3;
        pSensorInfo[0]->SensorDataLatchCount= 2;
#ifdef MTK_MATV_SERIAL_IF_SUPPORT		
        pSensorInfo[0]->SensorGrabStartX = 2; 
        pSensorInfo[0]->SensorGrabStartY = 1;
#else
        pSensorInfo[0]->SensorGrabStartX = 1; 
        pSensorInfo[0]->SensorGrabStartY = 3;			   
#endif

        //to skip beginning unstable frame.
        pSensorInfo[0]->CaptureDelayFrame = 22; //11.11 by HP's comment
        pSensorInfo[0]->PreviewDelayFrame = 22;
        pSensorInfo[0]->VideoDelayFrame = 22;

        pSensorConfigData[0]->SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;
        pSensorConfigData[0]->SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW;
    }
    else {
        LOG_ERR("getInfo: NULL parameter \n");
        return SENSOR_UNKNOWN_ERROR;
    }
    
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::getResolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution[2])
{
    if (pSensorResolution[0]) {
        pSensorResolution[0]->SensorPreviewWidth = ISP_HAL_ATV_PREVIEW_WIDTH;
        pSensorResolution[0]->SensorPreviewHeight = ISP_HAL_ATV_PREVIEW_HEIGHT;
        pSensorResolution[0]->SensorFullWidth = ISP_HAL_ATV_FULL_WIDTH;
        pSensorResolution[0]->SensorFullHeight = ISP_HAL_ATV_FULLW_HEIGHT;
    }
    else {
        LOG_ERR("getResolution: NULL parameter \n");
        return SENSOR_UNKNOWN_ERROR;
    }
    
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
#ifdef ATVCHIP_MTK_ENABLE 
MINT32 
AtvSensorDrv::atvGetDispDelay()
{
    MINT32 atvMode =0;
    MINT32 atvDelay = 0;

    atvMode = matv_get_mode();

    if ( atvMode!=ATV_MODE_NTSC && atvMode!=ATV_MODE_PAL)
    {
        LOG_ERR("get wrong atv mode %d", atvMode);
    }
    else
    {
        atvDelay = NSCamCustom::get_atv_disp_delay(atvMode);    
        LOG_MSG("atvMode: %d atvDelay = %dus",atvMode, atvDelay);
    }
    
    return atvDelay;
}
#endif

