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
#define LOG_TAG "campipe/io"
//
#include <vector>
using namespace std;

#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
//
#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>
//
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/imageio/ispio_utility.h>
//
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>

#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/ICamIOPipe.h>

//
#include "../inc/PipeImp.h"
#include "../inc/CamIOPipe.h"
#include "../inc/CampipeImgioPipeMapper.h"
//


/*******************************************************************************
*
********************************************************************************/
namespace NSCamPipe {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* 
********************************************************************************/
CamIOPipe::
CamIOPipe(
    char const*const szPipeName, 
    EPipeID const ePipeID, 
    ESWScenarioID const eSWScenarioID, 
    EScenarioFmt const eScenarioFmt
)
    : PipeImp(szPipeName, ePipeID, eSWScenarioID, eScenarioFmt)
    , mpCamIOPipe(NULL)
    , mpSensorHal(NULL)
    , mrSensorPortInfo()
    , mfgIsYUVPortON(MFALSE)
    , mrRawQTBufInfo()
    , mrYuvQTBufInfo()
    , mu4DeviceID(0)
{
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
init()
{
    FUNCTION_LOG_START;
    //
    //(1) CameraIO pipe TG --> ISP --> Mem 
    mpCamIOPipe = NSImageio::NSIspio::ICamIOPipe::createInstance(
                                                       mapScenarioID(meSWScenarioID, mePipeID), 
                                                       mapScenarioFmt(meScenarioFmt));
    if (NULL == mpCamIOPipe || !mpCamIOPipe->init())
    {
        return MFALSE;
    }
        
    //FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
uninit()
{
    FUNCTION_LOG_START;
    //
    MBOOL ret = MTRUE;
    //
    if (NULL != mpSensorHal)
    {
         //mpSensorHal->uninit(); 
         mpSensorHal->destroyInstance(); 
         mpSensorHal = NULL; 
    }

    //
    if (NULL != mpCamIOPipe)
    {
        if (MTRUE != mpCamIOPipe->uninit())
        {
            ret = MFALSE;
        }
        mpCamIOPipe->destroyInstance(); 
        mpCamIOPipe = NULL;
    }
    //FUNCTION_LOG_END;
    //
    return ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
start()
{
    FUNCTION_LOG_START;
    MBOOL ret = MFALSE; 
    
    //
    // (1) set buffer to current buffer 
    ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER, 
                                   (MINT32)NSImageio::NSIspio::EPortIndex_IMGO,
                                    0,
                                    0
                                  );

    if (mfgIsYUVPortON) 
    {
       ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER, 
                                      (MINT32)NSImageio::NSIspio::EPortIndex_IMG2O,
                                       0,
                                       0
                                      ); 
    }

    // 
    // (2) start CQ
    ret = mpCamIOPipe->startCQ0(); 
    //
    // ! let commond queue trigger mode as continuous mode 
    ret = mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)NSImageio::NSIspio::EPIPE_CQ_NONE, 
                                  0, 
                                  0
                                   ); //[?]    
    //
    ret = mpCamIOPipe->start(); 
    // (3) sync vync, 82 has only TG1
    //ret = mpCamIOPipe->irq( NSImageio::NSIspio::EPipePass_PASS1_TG1,
    //                       NSImageio::NSIspio::EPIPEIRQ_VSYNC
    //                       );

    MY_LOGD(" Wait for stable frame:%d", mu4SkipFrame); 
    skipFrame(mu4SkipFrame); 
    
    handleNotifyCallback( ECamPipe_NOTIFY_MSG_SOF, 0, 0 );

    FUNCTION_LOG_END;
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
startOne()
{
    FUNCTION_LOG_START;    
    MY_LOGD("+ [TODO]");

   
    FUNCTION_LOG_END;
    return  MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
stop()
{
    FUNCTION_LOG_START;
    //
    if ( ! mpCamIOPipe->stop())
    {
       MY_LOGE("mCamIOPipe->stop() fail");
       return MFALSE;
    }
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
enqueBuf(PortID const ePortID, QBufInfo const& rQBufInfo)
{
    //FUNCTION_LOG_START;
    //MY_LOGD("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++) 
    {
        MY_LOGD("+ tid(%d) type(%d,%d,%d), (VA,PA,Size,ID)(%x, %x, %d, %d)", 
            gettid(), 
            ePortID.type, ePortID.index, ePortID.inout,
            rQBufInfo.vBufInfo.at(i).u4BufVA, rQBufInfo.vBufInfo.at(i).u4BufPA, 
            rQBufInfo.vBufInfo.at(i).u4BufSize, rQBufInfo.vBufInfo.at(i).i4MemID);
    }

    // 
    if (EPortType_MemoryOut != ePortID.type) 
    {
        MY_LOGE("enqueBuf only support memory out port type"); 
        return MFALSE;
    }

    // Note:: can't update config, but address
    //
    NSImageio::NSIspio::QBufInfo rOutBufInfo; 
    NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory, 
                                       NSImageio::NSIspio::EPortIndex_IMGO, 
                                       1); 

    // 
    if (1 == ePortID.index)      //yuv out buf 
    {
        rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O; 
    }

    //
    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++) 
    {
         NSImageio::NSIspio::BufInfo rBuf(rQBufInfo.vBufInfo.at(i).u4BufSize, 
                                          rQBufInfo.vBufInfo.at(i).u4BufVA, 
                                          rQBufInfo.vBufInfo.at(i).u4BufPA, 
                                          rQBufInfo.vBufInfo.at(i).i4MemID,
                                          rQBufInfo.vBufInfo.at(i).i4BufSecu,
                                          rQBufInfo.vBufInfo.at(i).i4BufCohe
                                         );  
         rOutBufInfo.vBufInfo.push_back(rBuf);                                                
    }
    //
    mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);         
    //FUNCTION_LOG_END;
    return  MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL 
CamIOPipe::
dequeHWBuf(MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //FUNCTION_LOG_START;
    MY_LOGD("+ tid(%d) tout)=(%d)", gettid(), u4TimeoutMs);
    //
    NSImageio::NSIspio::QTimeStampBufInfo rQTimeOutBufInfo; 
    NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory, 
                                       NSImageio::NSIspio::EPortIndex_IMGO, 
                                       1); 

    //Jackie, wait deque directly
    // (1). wait interrupt done 
    //mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);

    // (2.1). dequeue buffer , Raw 
    
    mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);         
      
    // (2.2). put buffer Raw in queue 
    mrRawQTBufInfo.u4User = rQTimeOutBufInfo.u4User; 
    mrRawQTBufInfo.u4Reserved = rQTimeOutBufInfo.u4Reserved;
    mrRawQTBufInfo.i4TimeStamp_sec = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_sec;
    mrRawQTBufInfo.i4TimeStamp_us = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_us; 
    
    for (MUINT32 i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++) 
    {
        BufInfo rBufInfo; 
        mapBufInfo(rBufInfo, rQTimeOutBufInfo.vBufInfo.at(i)); 

        mrRawQTBufInfo.vBufInfo.push_back(rBufInfo); 
    }

    if (mfgIsYUVPortON) 
    {
        // (3.1). dequeue buffer , Raw 
        rPortID.index =  NSImageio::NSIspio::EPortIndex_IMG2O; 
        mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);   
        // (3.2). put buffer yuv in queue 
        mrYuvQTBufInfo.u4User = rQTimeOutBufInfo.u4User; 
        mrYuvQTBufInfo.u4Reserved = rQTimeOutBufInfo.u4Reserved;
        mrYuvQTBufInfo.i4TimeStamp_sec = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_sec;
        mrYuvQTBufInfo.i4TimeStamp_us = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_us; 
        for (MUINT32 i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++) 
        {
            BufInfo rBufInfo; 
            mapBufInfo(rBufInfo, rQTimeOutBufInfo.vBufInfo.at(i));    
            mrYuvQTBufInfo.vBufInfo.push_back(rBufInfo);
        }
    }

    //FUNCTION_LOG_END;
    return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
dequeBuf(PortID const ePortID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //FUNCTION_LOG_START;
    MY_LOGD("+ tid(%d) type(%d, %d, %d, %d)", gettid(), ePortID.type, ePortID.index, ePortID.inout, u4TimeoutMs);
    // 
    // (1) check if the buffer already dequeue 
    // (2) if the buffer is not dequeue, dequeue from HW. 
    QTimeStampBufInfo *pQTBufInfo = NULL; 
    if (0 == ePortID.index)
    {
        pQTBufInfo = &mrRawQTBufInfo; 
    }
    else 
    {
        if (!mfgIsYUVPortON) 
        {
            MY_LOGE("The YUV por is not on \n");  
            return MFALSE; 
        }
        pQTBufInfo = &mrYuvQTBufInfo; 
    }
    
    if (pQTBufInfo->vBufInfo.size() == 0) 
    {
        dequeHWBuf(u4TimeoutMs); 
    }
    //
    rQBufInfo.u4User = pQTBufInfo->u4User; 
    rQBufInfo.u4Reserved = pQTBufInfo->u4Reserved;
    rQBufInfo.i4TimeStamp_sec = pQTBufInfo->i4TimeStamp_sec;
    rQBufInfo.i4TimeStamp_us = pQTBufInfo->i4TimeStamp_us; 
    for (MUINT32 i = 0; i < pQTBufInfo->vBufInfo.size(); i++) 
    {
        BufInfo rBufInfo(pQTBufInfo->vBufInfo.at(i).u4BufSize, 
                         pQTBufInfo->vBufInfo.at(i).u4BufVA, 
                         pQTBufInfo->vBufInfo.at(i).u4BufPA, 
                         pQTBufInfo->vBufInfo.at(i).i4MemID);  
        rQBufInfo.vBufInfo.push_back(rBufInfo); 
    }
    pQTBufInfo->vBufInfo.clear(); 

    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++) 
    {
        MY_LOGD("(VA,PA,Size,ID)(0x%x,0x%x,%d,%d), ts(%d.%06d)", 
            rQBufInfo.vBufInfo.at(i).u4BufVA,
            rQBufInfo.vBufInfo.at(i).u4BufPA, rQBufInfo.vBufInfo.at(i).u4BufSize, rQBufInfo.vBufInfo.at(i).i4MemID,
            rQBufInfo.i4TimeStamp_sec, rQBufInfo.i4TimeStamp_us);
    }

    handleNotifyCallback( ECamPipe_NOTIFY_MSG_EOF, 0, 0 );

    FUNCTION_LOG_END;
    return  MTRUE;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    //FUNCTION_LOG_START;
    MY_LOGD("+ tid(%d), %d in / %d out", gettid(), vInPorts.size(), vOutPorts.size());
    MBOOL ret = MTRUE; 

    // 
    if (0 == vInPorts.size() 
        || 0 == vOutPorts.size() 
        || vOutPorts.size() > 2) 
    {
        MY_LOGE("Port config error");
        return MFALSE; 
    }
    //
    if (EPortType_Sensor != vInPorts.at(0)->type) 
    {
        MY_LOGE("The IN port type should be sensor type"); 
        return MFALSE; 
    }
    //
    for (MUINT32 i = 0; i < vOutPorts.size(); i++) 
    {
        if (EPortType_MemoryOut != vOutPorts.at(i)->type) 
        {
            MY_LOGE("The OUT port type should be EPortType_MemoryOut");
            return MFALSE; 
        }
    } 
 
    // (1). callbacks 
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);
    // (2). command queue config 
    ret = ret 
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                   0,
                                   0
                                   )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)NSImageio::NSIspio::EPIPE_PASS1_CQ0,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                                  (MINT32)NSImageio::NSIspio::EPIPECQ_TRIG_BY_START
                                  )
            && mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                  (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,
                                   0,
                                   0
                                  );
    if (!ret) 
    {
        MY_LOGE("Cammand queue config fail:%d", mpCamIOPipe->getLastErrorCode()); 
        return ret; 
    }
    //
    // (3). In sensor port 
    vector<NSImageio::NSIspio::PortInfo const*> vCamIOInPorts; 
    SensorPortInfo const* const pSensorPort = reinterpret_cast<SensorPortInfo const*> (vInPorts.at(0)); 
    ::memcpy(&mrSensorPortInfo, const_cast<SensorPortInfo*>(pSensorPort),sizeof(SensorPortInfo)); 
    MUINT32 u4SensorWidth = 0, u4SensorHeight = 0; 
    MUINT32 u4RawPixelID = 0;
    EImageFormat eSensorFmt = eImgFmt_UNKNOWN; 
    // (3.1) Sensor instance 
    if (NULL == mpSensorHal) 
    {
        mpSensorHal = SensorHal::createInstance();
        if (NULL == mpSensorHal)
        {
            MY_LOGE("Null sensorHal object"); 
            return MFALSE; 
        }
    }
    //
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                 SENSOR_CMD_SET_SENSOR_DEV,
                                 0,
                                 0,
                                 0
                                 ); 
    //
    //mpSensorHal->init();

    ret = querySensorInfo( pSensorPort->u4DeviceID, pSensorPort->u4Scenario, pSensorPort->u4Bitdepth, eSensorFmt, u4SensorWidth, u4SensorHeight, u4RawPixelID);

    MY_LOGD("Sensor: (devID,scen)(%d,%d) (w,h,fmt,bits,pixID)(%d,%d,0x%x,%d,%d) (bpDelay,bpScen,rawType)(%d,%d,%d)", 
                       pSensorPort->u4DeviceID, pSensorPort->u4Scenario, 
                       u4SensorWidth, u4SensorHeight, eSensorFmt, pSensorPort->u4Bitdepth, u4RawPixelID,
                       pSensorPort->fgBypassDelay, pSensorPort->fgBypassScenaio, pSensorPort->u4RawType); 
    // 
    MUINT32 u4SensorStride = u4SensorWidth; 
    if (eImgFmt_BAYER8 == eSensorFmt || eImgFmt_BAYER10 == eSensorFmt || eImgFmt_BAYER12 == eSensorFmt)
    {
        u4SensorStride = NSImageio::NSIspio::queryRawStride(eSensorFmt, u4SensorWidth); 
    }

    //MY_LOGD("SensorPortInfo: (width, height, format, stride) = (%d, %d, 0x%x, %d, %d, %d)", 
    //                 u4SensorWidth, u4SensorHeight, eSensorFmt, u4SensorStride); 

    // 
    NSImageio::NSIspio::PortInfo tgi;    
    tgi.eImgFmt = eSensorFmt;     
    tgi.eRawPxlID = mapRawPixelID(u4RawPixelID);
    tgi.u4ImgWidth = u4SensorWidth; 
    tgi.u4ImgHeight = u4SensorHeight; 
    tgi.u4Stride[0] = u4SensorStride; 
    tgi.u4Stride[1] = 0; 
    tgi.u4Stride[2] = 0; 
    tgi.type = NSImageio::NSIspio::EPortType_Sensor;     
    mu4DeviceID = pSensorPort->u4DeviceID; 
    tgi.index = NSImageio::NSIspio::EPortIndex_TG1I;
    tgi.inout  = NSImageio::NSIspio::EPortDirection_In; 
    tgi.u4BufSize  = (MUINT32)0; 
    vCamIOInPorts.push_back(&tgi); 

    // The raw type, 0: pure raw, 1: pre-process raw 
    if(pSensorPort->u4RawType == 1)
    {
        ret = mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_IMGO_RAW_TYPE,
                                      (MINT32)NSImageio::NSIspio::eRawImageType_PreProc,
                                       0,
                                       0
                                      );
    }
    //
    // (4). Out Port    
    vector<NSImageio::NSIspio::PortInfo const*> vCamIOOutPorts; 
    NSImageio::NSIspio::PortInfo imgo;
    NSImageio::NSIspio::PortInfo img2o;   
    for (MUINT32 i = 0; i < vOutPorts.size(); i++) 
    {
        MemoryOutPortInfo const* const memOutPort= reinterpret_cast<MemoryOutPortInfo const*> (vOutPorts.at(i)); 
        //    
        if (0 == memOutPort->index) 
        {
            MY_LOGD("Out 0: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d)", 
                    tgi.eImgFmt, tgi.u4ImgWidth, tgi.u4ImgHeight, 
                    memOutPort->u4Stride[0],  memOutPort->u4Stride[1],  memOutPort->u4Stride[2]); 
            imgo.eImgFmt = tgi.eImgFmt;      
            imgo.u4ImgWidth = tgi.u4ImgWidth;     
            imgo.u4ImgHeight = tgi.u4ImgHeight;          
            // no crop 
            imgo.crop.y = 0; 
            imgo.crop.h = imgo.u4ImgHeight; 
            imgo.type = NSImageio::NSIspio::EPortType_Memory;
            imgo.index = NSImageio::NSIspio::EPortIndex_IMGO; 
            imgo.inout  = NSImageio::NSIspio::EPortDirection_Out; 
            imgo.u4Stride[0] = memOutPort->u4Stride[0]; 
            imgo.u4Stride[1] = memOutPort->u4Stride[1]; 
            imgo.u4Stride[2] = memOutPort->u4Stride[2]; 
            imgo.u4Offset = 0;  
            vCamIOOutPorts.push_back(&imgo); 
        }
#warning [TODO] Should check the port config by scenario 
        else if (1 == memOutPort->index) 
        {
            MY_LOGD("Out 1: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d)", 
                    memOutPort->eImgFmt,  memOutPort->u4ImgWidth, memOutPort->u4ImgHeight, 
                    memOutPort->u4Stride[0],  memOutPort->u4Stride[1],  memOutPort->u4Stride[2]); 

            img2o.eImgFmt = memOutPort->eImgFmt;  
            img2o.u4ImgWidth = memOutPort->u4ImgWidth;  
            img2o.u4ImgHeight = memOutPort->u4ImgHeight;
            img2o.crop.y = 0; 
            img2o.crop.h = img2o.u4ImgHeight; 
            img2o.type = NSImageio::NSIspio::EPortType_Memory;    
            img2o.index = NSImageio::NSIspio::EPortIndex_IMG2O;   
            img2o.inout  = NSImageio::NSIspio::EPortDirection_Out;
            img2o.u4Stride[0] = memOutPort->u4Stride[0]; 
            img2o.u4Stride[1] = memOutPort->u4Stride[1]; 
            img2o.u4Stride[2] = memOutPort->u4Stride[2]; 
            vCamIOOutPorts.push_back(&img2o); 
            mfgIsYUVPortON = MTRUE; 
        }
    }

    ret = mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);

    // 
    ret = configSensor(pSensorPort->u4DeviceID, pSensorPort->u4Scenario, u4SensorWidth, u4SensorHeight, pSensorPort->fgBypassDelay, pSensorPort->fgBypassScenaio, MTRUE);  

    // The raw type, 0: pure raw, 1: pre-process raw  2: sensor test pattern
    if(pSensorPort->u4RawType == 2)
    {
        MINT32 u32Enable = 1;
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                           SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                           (MINT32)&u32Enable,
                                           0,
                                           0);
        MY_LOGD("Sensor Test Pattern"); 
    }

    // query skip frame to wait for stable
    mu4SkipFrame = 0;     
    if (mrSensorPortInfo.fgBypassDelay == MFALSE) 
    {
        MUINT32 u4Mode = SENSOR_CAPTURE_DELAY; 
        //
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(mrSensorPortInfo.u4DeviceID), 
                                              static_cast<int>(SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT), 
                                              reinterpret_cast<int>(&mu4SkipFrame), 
                                              reinterpret_cast<int>(&u4Mode)); 
    }

    handleNotifyCallback( ECamPipe_NOTIFY_MSG_DROPFRAME, mu4SkipFrame, 0 );
    //FUNCTION_LOG_END;
    return  ret;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    //FUNCTION_LOG_START;
    MY_LOGD("+ tid(%d), (cmd, arg1, arg2, arg3) = (%d, %d, %d, %d)", 
                     gettid(), cmd, arg1, arg2, arg3) ;
    ECamIOPipeCmd eCmd = static_cast<ECamIOPipeCmd>(cmd); 
    
    switch (eCmd) 
    {
        case ECamIOPipeCmd_QUERY_BAYER_RAW_SRIDE:
        {
            EImageFormat eFmt = static_cast<EImageFormat>(arg1);             
            *(MINT32 *)arg3 = NSImageio::NSIspio::queryRawStride(eFmt, (MUINT32)arg2);                     
        }
        break; 
        default: 
            MY_LOGE("not support command"); 
        break; 
    }
    //FUNCTION_LOG_END;

   return MTRUE; 
}

/*******************************************************************************
* 
********************************************************************************/
MVOID   
CamIOPipe::
waitSignal(EPipeSignal ePipeSignal, MUINT32 const u4TimeoutMs )
{
    FUNCTION_LOG_START;
    switch(ePipeSignal)
    {
        case EPipeSignal_SOF:
            mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_VSYNC);
            break;
        case EPipeSignal_EOF:
            mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1, NSImageio::NSIspio::EPIPEIRQ_PATH_DONE); 
            break; 
        default:
            break;
    }    
    FUNCTION_LOG_END;
}

/*******************************************************************************
* 
********************************************************************************/
MBOOL
CamIOPipe::
queryPipeProperty(vector<PortProperty> &vInPorts, vector<PortProperty> &vOutPorts)
{    
    FUNCTION_LOG_START;
    PortID rSensorPortID(EPortType_Sensor, 0, 0); 
    PortID rRawPortID(EPortType_MemoryOut, 0, 1); 
    PortID rYuvPortID(EPortType_MemoryOut, 1, 1);   
    //
    PortProperty rSensorPortProperty(rSensorPortID, eImgFmt_UNKNOWN, MFALSE, MFALSE); 
    PortProperty rRawPortProperty(rRawPortID, eImgFmt_BAYER10|eImgFmt_YUY2, MFALSE, MFALSE); 
    PortProperty rYuvPortProperty(rYuvPortID, eImgFmt_YUY2, MFALSE, MFALSE); 

    vInPorts.clear(); 
    vOutPorts.clear(); 

    if(eSWScenarioID_MTK_PREVIEW == meSWScenarioID) 
    {
        vInPorts.push_back(rSensorPortProperty);     
        vOutPorts.push_back(rRawPortProperty);  
    }
    else if (eSWScenarioID_CAPTURE_NORMAL == meSWScenarioID) 
    {
        vInPorts.push_back(rSensorPortProperty);     
        vOutPorts.push_back(rRawPortProperty);  
        vOutPorts.push_back(rYuvPortProperty); 
    }

    dumpPipeProperty(vInPorts, vOutPorts); 
    FUNCTION_LOG_END;
    return  MTRUE;
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL 
CamIOPipe::
querySensorInfo(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth, EImageFormat &eFmt,  MUINT32 &u4Width, MUINT32 &u4Height, MUINT32 & u4RawPixelID)
{
    //MY_LOGD("+(id,scen,bits) = (%d,%d,%d)", u4DeviceID, u4Scenario, u4BitDepth); 

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
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID), 
                             SENSOR_CMD_GET_SENSOR_TYPE, 
                             reinterpret_cast<int>(&eSensorType), 
                             0, 
                             0
                            );

    //get sensor format info
    halSensorRawImageInfo_t rRawImgInfo; 
    memset(&rRawImgInfo, 0, sizeof(rRawImgInfo));

    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                                          SENSOR_CMD_GET_RAW_INFO,
                                          (MINT32)&rRawImgInfo,
                                          1,
                                          0
                                         );        



    switch(eSensorType) 
    {
        case SENSOR_TYPE_RAW:
            eFmt = mapRawFormat(u4BitDepth); 
            u4RawPixelID = mapRawPixelID(rRawImgInfo.u1Order); 
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
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(u4DeviceID),
                             cmd,
                             (int)&u4Width,
                             (int)&u4Height,
                             0
                            );    

    //MY_LOGD("-(fmt,w,h,pixID) = (0x%x,%d,%d,%d)", eFmt, u4Width, u4Height, u4RawPixelID); 

    return MTRUE; 
}


/*******************************************************************************
* 
********************************************************************************/
MBOOL 
CamIOPipe::
skipFrame(MUINT32 const u4SkipCount)
{
    MY_LOGD(" + (u4SkipCount) = (%d)", u4SkipCount); 
    MBOOL ret = MTRUE; 
    for (MUINT32 i = 0; i < u4SkipCount ; i++)
    {
        //
        NSImageio::NSIspio::QTimeStampBufInfo rQTimeOutBufInfo; 
        NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory, 
                                         NSImageio::NSIspio::EPortIndex_IMGO, 
                                        1); 
        //    
        ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);         
        if (!ret) 
        {
            MY_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMGO) fail "); 
            return ret; 
        }
        
        //
        mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);         
      
        if (mfgIsYUVPortON) 
        {
            rPortID.index =  NSImageio::NSIspio::EPortIndex_IMG2O; 
            ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);   
            //
            if (!ret) 
            {
                MY_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMG2O) fail "); 
                return ret; 
            }
            //
            mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);         
        }
        //ret = mpCamIOPipe->irq( (NSImageio::NSIspio::EPipePass_PASS1_TG1) ,
        //                   NSImageio::NSIspio::EPIPEIRQ_VSYNC
        //                   ); 
        handleNotifyCallback( ECamPipe_NOTIFY_MSG_DROPFRAME, u4SkipCount - i - 1, 0 );
    }

    return ret; 
}



/*******************************************************************************
* 
********************************************************************************/
MBOOL 
CamIOPipe::
configSensor(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4Width, MUINT32 const u4Height, MUINT32 const byPassDelay, MUINT32 const u4ByPassScenario,  MBOOL const fgIsContinuous)
{
    //FUNCTION_LOG_START;
    //MY_LOGD("+ (id, scen, w, h) = (%d, %d, %d, %d)", 
    //            u4DeviceID, u4Scenario, u4Width, u4Height); 
    //MY_LOGD(" configSensor(byPassDelay, u4ByPassScenario, fgIsContinuous) = (%d, %d, %d)", 
    //                   byPassDelay, u4ByPassScenario, fgIsContinuous); 

    MBOOL ret = MFALSE; 
    //
    halSensorIFParam_t halSensorIFParam[2];
    ::memset(halSensorIFParam, 0, sizeof(halSensorIFParam_t) * 2); 

    MUINT32 index = ((SENSOR_DEV_MAIN == u4DeviceID)||(SENSOR_DEV_ATV == u4DeviceID)) ? 0 : 1;

    halSensorIFParam[index].u4SrcW = u4Width; 
    halSensorIFParam[index].u4SrcH = u4Height; 
    halSensorIFParam[index].u4CropW = u4Width;
    halSensorIFParam[index].u4CropH = u4Height;
    halSensorIFParam[index].u4IsContinous = fgIsContinuous;
    halSensorIFParam[index].u4IsBypassSensorScenario = u4ByPassScenario;
#warning always set bypass sensor delay to true, due to hw not support delay frame currently. 
    halSensorIFParam[index].u4IsBypassSensorDelay = MTRUE;
    halSensorIFParam[index].scenarioId = static_cast<ACDK_SCENARIO_ID_ENUM>(u4Scenario); 
    ret = mpSensorHal->setConf(halSensorIFParam);

    //FUNCTION_LOG_END;
    return ret; 
} 

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamPipe



