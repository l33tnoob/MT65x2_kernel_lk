/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief
 
#define LOG_TAG "CamPipeTest"

#include <vector>

using namespace std;

#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <common/hw/hwstddef.h>
//#include <campipe/_buffer.h>

#include <campipe/IPipe.h>
#include <campipe/ICamIOPipe.h>

//
#include <mtkcam/hal/sensor_hal.h>
#include <drv/imem_drv.h>

using namespace NSCamPipe; 


/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

#define BUF_NUM          3
#define PREVIEW_WIDTH    640
#define PREVIEW_HEIGHT   480 

static  IMemDrv *g_pIMemDrv;
static  SensorHal *g_pSensorHal; 
static  halSensorDev_e gSenDev = SENSOR_DEV_MAIN;


static  MUINT32 g_u4SensorFullWidth = 0; 
static  MUINT32 g_u4SensorFullHeight = 0; 
static  MUINT32 g_u4SensorHalfWidth = 0; 
static  MUINT32 g_u4SensorHalfHeight = 0; 
static  MUINT32 g_bytePerPixel = 2; // for YUV case


static void allocMem(IMEM_BUF_INFO &memBuf) 
{
    if (g_pIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (g_pIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
}

static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (g_pIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if (g_pIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }        
}


/******************************************************************************
* save the buffer to the file 
*******************************************************************************/
static  bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size); 
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true; 
}


/*******************************************************************************
*  Camio ZSD scenario , 1 IN , 2 OUT
********************************************************************************/
int main_testCamIO_ZSD() 
{
    MY_LOGD("+"); 
/*    
    // (1). Create Instance 
    ICamIOPipe    *pCamIOPipe = ICamIOPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, eScenarioFmt_RAW); 
    if (pCamIOPipe != NULL) 
    {
        MY_LOGD("Pipe (Name, ID) = (%s, %d)", pCamIOPipe->getPipeName(), pCamIOPipe->getPipeId()); 
    }
    
    // (2). Query port property
    vector<PortProperty> rInPortProperty; 
    vector<PortProperty> rOutPortProperty;     
    if (pCamIOPipe->queryPipeProperty(rInPortProperty,rOutPortProperty))
    {
        MY_LOGD("Port Property (IN, OUT): (%d, %d)", rInPortProperty.size(), rOutPortProperty.size()); 
        for (MUINT32 i = 0; i < rInPortProperty.size(); i++)
        {
            MY_LOGD("IN: (type,index,inout) = (%d, %d, %d)", rInPortProperty.at(i).type, rInPortProperty.at(i).index, rInPortProperty.at(i).inout); 
            MY_LOGD("IN: (fmt, rot, flip) = (%d, %d, %d)", rInPortProperty.at(i).u4SupportFmt,
                                         rInPortProperty.at(i).fgIsSupportRotate, rInPortProperty.at(i).fgIsSupportFlip); 
        }       
        for (MUINT32 i = 0; i < rOutPortProperty.size(); i++)
        {
            MY_LOGD("OUT: (type,index,inout) = (%d, %d, %d)", rOutPortProperty.at(i).type, rOutPortProperty.at(i).index, rOutPortProperty.at(i).inout); 
            MY_LOGD("OUT: (fmt, rot, flip) = (%d, %d, %d)", rOutPortProperty.at(i).u4SupportFmt,
                                         rOutPortProperty.at(i).fgIsSupportRotate, rOutPortProperty.at(i).fgIsSupportFlip); 
        } 
 
    }    

    // (3). init 
    pCamIOPipe->init(); 

    // (4). setCallback
    pCamIOPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    MUINT32 u4RawStride[3] = {g_u4SensorHalfWidth, 0, 0};
    SensorPortInfo rSensorPort(SENSOR_DEV_MAIN, ACDK_SCENARIO_ID_CAMERA_PREVIEW, 10, MTRUE, MFALSE, 0); 
    //

    MemoryOutPortInfo rRawPort(ImgInfo(eImgFmt_BAYER8, g_u4SensorHalfWidth, g_u4SensorHalfHeight), 
                               u4RawStride, 0, 0); 
    //
    MUINT32 u4YuvStride[3] = {PREVIEW_WIDTH, 0, 0}; 
    MemoryOutPortInfo rYuvPort(ImgInfo(eImgFmt_YUY2, PREVIEW_WIDTH, PREVIEW_HEIGHT), 
                               u4YuvStride, 0, 0); 
    rYuvPort.index = 1; 
    //
    vector<PortInfo const*> vCamIOInPorts;  
    vector<PortInfo const*> vCamIOOutPorts; 
    //
    vCamIOInPorts.push_back(&rSensorPort); 
    vCamIOOutPorts.push_back(&rRawPort); 
    vCamIOOutPorts.push_back(&rYuvPort); 
    //
    pCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts); 

    // (6). Enqueue, raw buf
    MUINT32 u4RawBufSize = (g_u4SensorHalfWidth * g_u4SensorHalfHeight + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
    vector<IMEM_BUF_INFO> vRawMem; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        IMEM_BUF_INFO rBuf; 
        rBuf.size = u4RawBufSize; 
        allocMem(rBuf); 
        vRawMem.push_back(rBuf); 
    }    
    // 
    QBufInfo rRawBuf; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        rRawBuf.vBufInfo.clear(); 
        BufInfo rBufInfo(vRawMem.at(i).size, vRawMem.at(i).virtAddr, vRawMem.at(i).phyAddr, vRawMem.at(i).memID);  
        rRawBuf.vBufInfo.push_back(rBufInfo); 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rRawBuf); 
    }
    // (6.1) Enqueue, yuv buf 
    MUINT32 u4YuvBufSize = (PREVIEW_WIDTH * PREVIEW_HEIGHT * 2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
    vector<IMEM_BUF_INFO> vYuvMem; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        IMEM_BUF_INFO rBuf; 
        rBuf.size = u4YuvBufSize; 
        allocMem(rBuf); 
        vYuvMem.push_back(rBuf); 
    }    
    // 
    QBufInfo rYuvBuf; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        rYuvBuf.vBufInfo.clear(); 
        BufInfo rBufInfo(vYuvMem.at(i).size, vYuvMem.at(i).virtAddr, vYuvMem.at(i).phyAddr, vYuvMem.at(i).memID);  
        rYuvBuf.vBufInfo.push_back(rBufInfo); 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rYuvBuf); 
    }
   
    // (7). start
    pCamIOPipe->start(); 

    // (8). enqueue --> dequeue --> enqueue        
    int count = 30; 
    do 
    {   
        //
        QTimeStampBufInfo rQRawOutBuf;         
        QTimeStampBufInfo rQYuvOutBuf; 
        pCamIOPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQRawOutBuf); 
        pCamIOPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQYuvOutBuf);      

        //
        rRawBuf.vBufInfo[0].u4BufVA = rQRawOutBuf.vBufInfo[0].u4BufVA ; 
        rRawBuf.vBufInfo[0].u4BufPA = rQRawOutBuf.vBufInfo[0].u4BufPA ;         
        rRawBuf.vBufInfo[0].i4MemID = rQRawOutBuf.vBufInfo[0].i4MemID; 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rRawBuf);

        //
        rYuvBuf.vBufInfo[0].u4BufVA = rQYuvOutBuf.vBufInfo[0].u4BufVA ; 
        rYuvBuf.vBufInfo[0].u4BufPA = rQYuvOutBuf.vBufInfo[0].u4BufPA ;         
        rYuvBuf.vBufInfo[0].i4MemID = rQYuvOutBuf.vBufInfo[0].i4MemID; 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rYuvBuf);

    }while(--count > 0); 
 
    // (9). Stop 
    pCamIOPipe->stop();
    // (10). uninit 
    pCamIOPipe->uninit(); 
    // (11). destory instance 
    pCamIOPipe->destroyInstance(); 

    for (int i = 0; i < BUF_NUM; i++) 
    {
        char filename[256]; 
        sprintf(filename, "/data/raw%dx%d_%02d.raw", g_u4SensorHalfWidth, g_u4SensorHalfHeight, i);         
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(vRawMem.at(i).virtAddr), g_u4SensorHalfWidth * g_u4SensorHalfHeight); 

        deallocMem(vRawMem.at(i)); 

        sprintf(filename, "/data/yuv%dx%d_%02d.yuv", PREVIEW_WIDTH, PREVIEW_HEIGHT, i); 
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(vYuvMem.at(i).virtAddr), PREVIEW_WIDTH * PREVIEW_HEIGHT * 2); 
        deallocMem(vYuvMem.at(i)); 
    }
*/    
    return 0; 
}



/*******************************************************************************
*  Camio VSS scenario , 1 IN , 1 OUT
********************************************************************************/
int main_testCamIO_VSS() 
{
    MY_LOGD("+"); 
    // (1). Create Instance 
    ICamIOPipe    *pCamIOPipe = ICamIOPipe::createInstance(eSWScenarioID_MTK_PREVIEW, eScenarioFmt_YUV); 
    if (pCamIOPipe != NULL) 
    {
        MY_LOGD("Pipe (Name, ID) = (%s, %d)", pCamIOPipe->getPipeName(), pCamIOPipe->getPipeId()); 
    }
    
    // (2). Query port property
    vector<PortProperty> rInPortProperty; 
    vector<PortProperty> rOutPortProperty;     
    if (pCamIOPipe->queryPipeProperty(rInPortProperty,rOutPortProperty))
    {
        MY_LOGD("Port Property (IN, OUT): (%d, %d)", rInPortProperty.size(), rOutPortProperty.size()); 
        for (MUINT32 i = 0; i < rInPortProperty.size(); i++)
        {
            MY_LOGD("IN: (type,index,inout) = (%d, %d, %d)", rInPortProperty.at(i).type, rInPortProperty.at(i).index, rInPortProperty.at(i).inout); 
            MY_LOGD("IN: (fmt, rot, flip) = (%d, %d, %d)", rInPortProperty.at(i).u4SupportFmt,
                                         rInPortProperty.at(i).fgIsSupportRotate, rInPortProperty.at(i).fgIsSupportFlip); 
        }       
        for (MUINT32 i = 0; i < rOutPortProperty.size(); i++)
        {
            MY_LOGD("OUT: (type,index,inout) = (%d, %d, %d)", rOutPortProperty.at(i).type, rOutPortProperty.at(i).index, rOutPortProperty.at(i).inout); 
            MY_LOGD("OUT: (fmt, rot, flip) = (%d, %d, %d)", rOutPortProperty.at(i).u4SupportFmt,
                                         rOutPortProperty.at(i).fgIsSupportRotate, rOutPortProperty.at(i).fgIsSupportFlip); 
        } 
 
    }    

    // (3). init 
    pCamIOPipe->init(); 

    // (4). setCallback
    pCamIOPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    //g_u4SensorHalfWidth = 320;
    //g_u4SensorHalfHeight = 240;
    MUINT32 u4Stride[3] = {g_u4SensorHalfWidth, 0, 0};
    SensorPortInfo rSensorPort(gSenDev, ACDK_SCENARIO_ID_CAMERA_PREVIEW, 8, MTRUE, MFALSE, 0); 

    
    MemoryOutPortInfo rYuvPort(ImgInfo(eImgFmt_UYVY, g_u4SensorHalfWidth, g_u4SensorHalfHeight), 
                               u4Stride, 0, 0); 
    //
    vector<PortInfo const*> vCamIOInPorts;  
    vector<PortInfo const*> vCamIOOutPorts; 
    //
    vCamIOInPorts.push_back(&rSensorPort); 
    vCamIOOutPorts.push_back(&rYuvPort); 
    //
    pCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts); 

    // (6). Enqueue, yuv buf
    MUINT32 uYuvBufSize = (g_u4SensorHalfWidth * g_u4SensorHalfHeight * g_bytePerPixel + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
    vector<IMEM_BUF_INFO> vYuvMem; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        IMEM_BUF_INFO rBuf; 
        rBuf.size = uYuvBufSize; 
        allocMem(rBuf); 
        vYuvMem.push_back(rBuf); 
    }    
    // 
    QBufInfo rYuvBuf; 
    for (int i = 0; i < BUF_NUM; i++) 
    {
        rYuvBuf.vBufInfo.clear(); 
        BufInfo rBufInfo(vYuvMem.at(i).size, vYuvMem.at(i).virtAddr, vYuvMem.at(i).phyAddr, vYuvMem.at(i).memID);  
        rYuvBuf.vBufInfo.push_back(rBufInfo); 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rYuvBuf); 
    }

    // (7). start
    pCamIOPipe->start(); 

    // (8). enqueue --> dequeue --> enqueue        
    int count = 0; 
    char filename[256];
    do 
    {   
        //char filename[256];
        //for(int i = 0 ; i < 100000 ; i++)
        //    filename[i&5] = 'a';         
        
        //g_pIMemDrv->cacheFlushAll();                
        //sprintf(filename, "/data/yuv%dx%d_%02d.yuv", g_u4SensorHalfWidth, g_u4SensorHalfHeight, count);     
        //saveBufToFile(filename, reinterpret_cast<MUINT8*>(vYuvMem.at(0).virtAddr), g_u4SensorHalfWidth * g_u4SensorHalfHeight * g_bytePerPixel);         
        //fflush(stdout);
        
        //
        MY_LOGD("start deque %d", count);
        QTimeStampBufInfo rQYuvOutBuf;         
        pCamIOPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQYuvOutBuf); 
        MY_LOGD("va/pa=%x/%x", rQYuvOutBuf.vBufInfo[0].u4BufVA, rQYuvOutBuf.vBufInfo[0].u4BufPA);
        MY_LOGD("deque done");

        MY_LOGD("before dump");                
        sprintf(filename, "/data/yuv%dx%d_%08x_%02d.yuv", g_u4SensorHalfWidth, g_u4SensorHalfHeight, rQYuvOutBuf.vBufInfo[0].u4BufVA, count);     
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(rQYuvOutBuf.vBufInfo[0].u4BufVA), g_u4SensorHalfWidth * g_u4SensorHalfHeight * g_bytePerPixel); 
        MY_LOGD("after dump");

        //
        MY_LOGD("start emque %d", count);
        rYuvBuf.vBufInfo[0].u4BufVA = rQYuvOutBuf.vBufInfo[0].u4BufVA ; 
        rYuvBuf.vBufInfo[0].u4BufPA = rQYuvOutBuf.vBufInfo[0].u4BufPA ;         
        rYuvBuf.vBufInfo[0].i4MemID = rQYuvOutBuf.vBufInfo[0].i4MemID; 
        pCamIOPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rYuvBuf);
        MY_LOGD("enque done");

    }while(++count < 10); 
 
    // (9). Stop 
    pCamIOPipe->stop();
    // (10). uninit 
    pCamIOPipe->uninit(); 
    // (11). destory instance 
    pCamIOPipe->destroyInstance(); 

    for (int i = 0; i < BUF_NUM; i++) 
    {        
        sprintf(filename, "/data/res_yuv%dx%d_%02d.yuv", g_u4SensorHalfWidth, g_u4SensorHalfHeight, i);     
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(vYuvMem.at(i).virtAddr), g_u4SensorHalfWidth * g_u4SensorHalfHeight * g_bytePerPixel); 

        deallocMem(vYuvMem.at(i)); 
    }
    return 0; 
}


/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_camio(int argc, char** argv)
{
    int ret = 0; 

    int testItem = 0; 
    if (argc != 2) 
    { 
        printf("please input the test item, 0: camio_vss(main), 1: camio_vss(sub), 2:camio_zsd\n"); 
        goto EXIT;
    }
    else 
    {
      testItem = atoi(argv[1]); 
    }
    

    g_pIMemDrv =  IMemDrv::createInstance(); 
    if (NULL == g_pIMemDrv)
    {
        MY_LOGE("g_pIMemDrv is NULL"); 
        return 0; 
    }
    g_pIMemDrv->init();

    g_pSensorHal = SensorHal:: createInstance();

    if (NULL == g_pSensorHal) 
    {
        MY_LOGE("g_pSensorHal is NULL"); 
        return 0; 
    }
    // search sensor 
    g_pSensorHal->searchSensor();


    printf("enter test item(0 for VSS main, 1 for VSS sub, else for bypass):");
    scanf("%d", &testItem);
    MY_LOGD("start testItem %d...\n", testItem);

    //
    // (1). init main sensor 
    if(testItem == 1)
        gSenDev = SENSOR_DEV_SUB;
    else
        gSenDev = SENSOR_DEV_MAIN;

    //
    ret = g_pSensorHal->sendCommand(gSenDev,
                                    SENSOR_CMD_SET_SENSOR_DEV,
                                    0,
                                    0,
                                    0); 
    //
    ret = g_pSensorHal->init(); 
    // 
    ret = g_pSensorHal->sendCommand(gSenDev, 
                                    SENSOR_CMD_GET_SENSOR_PRV_RANGE, 
                                    (int)&g_u4SensorHalfWidth, 
                                    (int)&g_u4SensorHalfHeight,
                                    0
                                   ); 
    //
    ret = g_pSensorHal->sendCommand(gSenDev, 
                                    SENSOR_CMD_GET_SENSOR_FULL_RANGE, 
                                    (int)&g_u4SensorFullWidth, 
                                    (int)&g_u4SensorFullHeight, 
                                    0
                                   ); 
    MY_LOGD("sensor prv width, height = (%d, %d)", g_u4SensorHalfWidth, g_u4SensorHalfHeight); 
    MY_LOGD("sensor full width, height = (%d, %d)", g_u4SensorFullWidth, g_u4SensorFullWidth);   

    MY_LOGD("SENSOR INIT DONE\n\n\n\n\nSET CAMIO\n");   
    switch(testItem) 
    {
        case 0:
        case 1:
            main_testCamIO_VSS(); 
        break; 
        case 2: 
            main_testCamIO_ZSD(); 
        break; 
    }
    
    g_pIMemDrv->uninit();
    g_pIMemDrv->destroyInstance(); 
    g_pSensorHal->uninit(); 
    g_pSensorHal->destroyInstance(); 

EXIT: 
    printf("press any key to exit \n"); 
    getchar(); 

    return ret; 
}
