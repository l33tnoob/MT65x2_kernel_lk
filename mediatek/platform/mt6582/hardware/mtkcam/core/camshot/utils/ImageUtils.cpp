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
#define LOG_TAG "CamShot/ImageUtils"
//
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
//
#include <errno.h>
#include <fcntl.h>

//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>

// bayer raw stride
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/ICamIOPipe.h>
// Sensor info 
#include <mtkcam/hal/sensor_hal.h>
#include <kd_imgsensor_define.h>
 
#include "../inc/ImageUtils.h"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
#define ALIGN_16(x) ( (~15) & ((15) + (x)) )
#define ALIGN_32(x) ( (~31) & ((31) + (x)) )

namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
MUINT32  
queryImgStride(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4PlaneIndex)
{
    MUINT32 u4Stride = 0;
    // 
    switch(eFmt)
    {
        // YUV 420 format 
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            u4Stride = (u4PlaneIndex == 2) ? (0) : u4Width ; 
            break; 
        case eImgFmt_YV12:
            u4Stride = (u4PlaneIndex == 0) ? ALIGN_16( u4Width ) : ALIGN_16( u4Width>>1 );
            break; 
        case eImgFmt_I420:
            u4Stride = (u4PlaneIndex == 0) ? ALIGN_16( u4Width ) : ALIGN_16( u4Width>>1 );
            break; 
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
        case eImgFmt_YVYU:
        case eImgFmt_RGB565:
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : 0; 
            break; 
        case eImgFmt_YV16:
        case eImgFmt_I422:
        case eImgFmt_NV16:
        case eImgFmt_NV61:        
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : (u4Width >> 1); 
            break;         
        case eImgFmt_RGB888:         
            u4Stride = u4Width; 
            break; 
        case eImgFmt_ARGB888:
            u4Stride = u4Width; 
            break; 
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
            u4Stride = (u4PlaneIndex == 0 ) ? queryRawStride(eFmt, u4Width) : 0; 
            break; 
        case eImgFmt_JPEG:
            u4Stride = u4Width ; 
            break; 
        case eImgFmt_Y800:
            u4Stride = (u4PlaneIndex == 0) ? (u4Width) : (0); 
            break; 
        default:
            u4Stride = u4Width; 
            break; 
    } 
    return u4Stride; 
}



/*******************************************************************************
*
********************************************************************************/
MUINT32
queryRawStride(MUINT32 const imgFmt, MUINT32 const imgWidth)
{
    using namespace NSCamPipe; 
    ICamIOPipe *pCamIOPipe = ICamIOPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, eScenarioFmt_RAW); 
    CHECK_OBJECT(pCamIOPipe); 
    MUINT32 stride = 0;     

    pCamIOPipe->sendCommand(NSCamPipe::ECamIOPipeCmd_QUERY_BAYER_RAW_SRIDE, 
                            static_cast<MINT32>(imgFmt), 
                            static_cast<MINT32>(imgWidth),
                            reinterpret_cast<MINT32>(&stride)
                            );  
    pCamIOPipe->destroyInstance();

    //MY_LOGD("(fmt, w, stride) = (%d, %d, %d)", imgFmt, imgWidth, stride);
    return stride;
}


/*******************************************************************************
* 
********************************************************************************/
MUINT32 
queryImgBufSize(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height)
{
    MUINT32 u4BufSize = 0;

    // 
    switch(eFmt)
    {
        // YUV 420 format 
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK: 
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
            u4BufSize = u4Width * u4Height * 3 / 2; 
            break; 
        case eImgFmt_YV12:
        case eImgFmt_I420:
            u4BufSize = queryImgStride(eFmt, u4Width, 0) * u4Height +
                        queryImgStride(eFmt, u4Width, 1) * u4Height;
            break;
        // YUV 422 format , RGB565
        case eImgFmt_YUY2: 
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
        case eImgFmt_YVYU: 
        case eImgFmt_YV16:
        case eImgFmt_I422:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
        case eImgFmt_RGB565:
            u4BufSize = u4Width * u4Height * 2; 
            break;         
        case eImgFmt_RGB888:         
            u4BufSize = u4Width * u4Height * 3; 
            break; 
        case eImgFmt_ARGB888:
            u4BufSize = u4Width * u4Height * 4; 
            break; 
        case eImgFmt_BAYER8:
            u4BufSize = u4Width * u4Height; 
            break; 
        case eImgFmt_BAYER10:
        {
            // the stride is in pixel unit 
            MUINT32 u4Stride = queryImgStride(eFmt, u4Width, 0); 
            u4BufSize = u4Stride * u4Height * 5 / 4 ; 
        }  
            break; 
        case eImgFmt_BAYER12:
        {
            // the stride is in pixel unit 
            MUINT32 u4Stride = queryImgStride(eFmt, u4Width, 0); 
            u4BufSize = u4Stride * u4Height * 3 / 2 ; 
        }  
            break; 
        case eImgFmt_JPEG:
            u4BufSize = u4Width * u4Height * 6 / 5; // * 2 / 4;    //? assume the JPEG ratio is 1/4 
            break; 
        case eImgFmt_Y800:
            u4BufSize = u4Width * u4Height; 
            break; 
        default:
            u4BufSize = u4Width * u4Height * 2; 
            break; 
    } 
    return u4BufSize; 
}

/*******************************************************************************
* 
********************************************************************************/
static 
MUINT32 
mapRawPixeID(MUINT32 u4ColorOrder)
{
    switch (u4ColorOrder)
    {
        case SENSOR_OUTPUT_FORMAT_RAW_B:
            return 0; 
        break; 
        case SENSOR_OUTPUT_FORMAT_RAW_Gb:
            return 1; 
        break; 
        case SENSOR_OUTPUT_FORMAT_RAW_Gr: 
            return 2;
        break;
        case SENSOR_OUTPUT_FORMAT_RAW_R:
            return 3;
        break; 
        default:
            return 0; 
        break; 
    }
}

/*******************************************************************************
* 
********************************************************************************/
static 
EImageFormat 
mapRawFormat(MUINT32 u4BitDepth)
{
    switch (u4BitDepth)
    {
        case 8:
            return eImgFmt_BAYER8; 
        break; 
        case 10:
            return eImgFmt_BAYER10; 
        break; 
        case 12:
            return eImgFmt_BAYER12; 
        break; 
        default:
            return eImgFmt_BAYER8; 
            break; 
    }
}

/*******************************************************************************
* 
********************************************************************************/
static 
EImageFormat
mapYUVFormat(MUINT32 u4ColorOrder)
{
    switch (u4ColorOrder)
    {
        case SENSOR_OUTPUT_FORMAT_UYVY:
        case SENSOR_OUTPUT_FORMAT_CbYCrY:
            return eImgFmt_UYVY; 
        break; 
        case SENSOR_OUTPUT_FORMAT_VYUY:
        case SENSOR_OUTPUT_FORMAT_CrYCbY:
            return eImgFmt_VYUY; 
        break; 
        case SENSOR_OUTPUT_FORMAT_YUYV:
        case SENSOR_OUTPUT_FORMAT_YCbYCr:
            return eImgFmt_YUY2; 
        break; 
        case SENSOR_OUTPUT_FORMAT_YVYU:
        case SENSOR_OUTPUT_FORMAT_YCrYCb:
            return eImgFmt_YVYU; 
        break; 
        default:
            return eImgFmt_YUY2; 
        break; 
    }
}

/*******************************************************************************
* 
********************************************************************************/
EImageFormat  
mapBayerFormat(MUINT32 const u4BitDepth)
{
    if (8 == u4BitDepth) 
    {
        return eImgFmt_BAYER8; 
    }
    else if (10 == u4BitDepth)
    {
        return eImgFmt_BAYER10; 
    }
    else if (12 == u4BitDepth) 
    {
        return eImgFmt_BAYER12; 
    }
    else
    {
        return eImgFmt_BAYER8;
    }
}


/*******************************************************************************
* 
********************************************************************************/
EImageFormat  
querySensorFmt(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth)
{
    //MY_LOGD("+ (id, scenario, bitdepth) = (%d, %d, %d)", u4DeviceID, u4Scenario, u4BitDepth); 
    EImageFormat eFmt = eImgFmt_UNKNOWN; 

    SensorHal *pSensorHal = SensorHal:: createInstance();
    if (NULL == pSensorHal)
    {
        MY_LOGE(" Null pSensorHal Obj \n"); 
        return eFmt; 
    }

    pSensorHal->init(); 

    // Sensor type
    halSensorType_e eSensorType; 
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID), 
                             SENSOR_CMD_GET_SENSOR_TYPE, 
                             reinterpret_cast<int>(&eSensorType), 
                             0, 
                             0
                            );

    //get sensor format info
    halSensorRawImageInfo_t rRawImgInfo; 
    memset(&rRawImgInfo, 0, sizeof(rRawImgInfo));

    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                                          SENSOR_CMD_GET_RAW_INFO,
                                          (MINT32)&rRawImgInfo,
                                          1,
                                          0
                                         );        
    //
    switch(eSensorType) 
    {
        case SENSOR_TYPE_RAW:
            eFmt = mapRawFormat(u4BitDepth); 
        break; 
        case SENSOR_TYPE_YUV:
        case SENSOR_TYPE_YCBCR:  
            eFmt = mapYUVFormat(rRawImgInfo.u1Order);
        break; 
        break; 
        case SENSOR_TYPE_RGB565:
            eFmt = eImgFmt_RGB565; 
        break; 
        case SENSOR_TYPE_RGB888:
            eFmt = eImgFmt_RGB888; 
        break; 
        case SENSOR_TYPE_JPEG:   
            eFmt = eImgFmt_JPEG; 
        default:
            eFmt = eImgFmt_UNKNOWN; 
        break;       
    }

    pSensorHal->uninit(); 
    pSensorHal->destroyInstance(); 
    return eFmt; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
querySensorInfo(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth, EImageFormat &eFmt,  MUINT32 &u4Width, MUINT32 &u4Height, MUINT32 & u4RawPixelID)
{
    MY_LOGD("+ (id, scenario) = (%d, %d)", u4DeviceID, u4Scenario); 

    SensorHal *pSensorHal = SensorHal:: createInstance();
    CHECK_OBJECT(pSensorHal); 

    pSensorHal->init(); 
    MINT32 cmd = 0; 
    switch (u4Scenario) 
    {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
        break; 
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE; 
        break; 
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_VIDEO_RANGE; 
        break; 
    }

    // Sensor type
    halSensorType_e eSensorType; 
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID), 
                             SENSOR_CMD_GET_SENSOR_TYPE, 
                             reinterpret_cast<int>(&eSensorType), 
                             0, 
                             0
                            );

    //get sensor format info
    halSensorRawImageInfo_t rRawImgInfo; 
    memset(&rRawImgInfo, 0, sizeof(rRawImgInfo));

    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                                          SENSOR_CMD_GET_RAW_INFO,
                                          (MINT32)&rRawImgInfo,
                                          1,
                                          0
                                         );        



    switch(eSensorType) 
    {
        case SENSOR_TYPE_RAW:
            eFmt = mapRawFormat(u4BitDepth); 
            u4RawPixelID = mapRawPixeID(rRawImgInfo.u1Order); 
        break; 
        case SENSOR_TYPE_YUV:
        case SENSOR_TYPE_YCBCR:  
            eFmt = mapYUVFormat(rRawImgInfo.u1Order);
        break; 
        break; 
        case SENSOR_TYPE_RGB565:
            eFmt = eImgFmt_RGB565; 
        break; 
        case SENSOR_TYPE_RGB888:
            eFmt = eImgFmt_RGB888; 
        break; 
        case SENSOR_TYPE_JPEG:   
            eFmt = eImgFmt_JPEG; 
        default:
            eFmt = eImgFmt_UNKNOWN; 
        break;       
    }

    // resolution
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                             cmd,
                             (int)&u4Width,
                             (int)&u4Height,
                             0
                            );    

    pSensorHal->uninit(); 
    pSensorHal->destroyInstance(); 
    return MTRUE; 
}



/*******************************************************************************
* 
********************************************************************************/
MBOOL 
querySensorResolution(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 &u4Width, MUINT32 &u4Height)
{
    //MY_LOGD("+ (id, scenario) = (%d, %d)", u4DeviceID, u4Scenario); 

    SensorHal *pSensorHal = SensorHal:: createInstance();
    CHECK_OBJECT(pSensorHal); 

    pSensorHal->init(); 
    MINT32 cmd = 0; 
    switch (u4Scenario) 
    {
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
        break; 
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
            cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE; 
        break; 
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            cmd = SENSOR_CMD_GET_SENSOR_VIDEO_RANGE; 
        break; 
    }
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                             cmd,
                             (int)&u4Width,
                             (int)&u4Height,
                             0
                            );    
    pSensorHal->uninit(); 
    pSensorHal->destroyInstance(); 
    return MTRUE; 
}



////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

