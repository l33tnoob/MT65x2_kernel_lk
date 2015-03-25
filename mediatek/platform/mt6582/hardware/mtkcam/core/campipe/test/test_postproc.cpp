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
#include <mtkcam/common/hw/hwstddef.h>
//#include <mtkcam/campipe/_buffer.h>

#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/IPostProcPipe.h>

//
#include <mtkcam/hal/sensor_hal.h>
#include <mtkcam/drv/imem_drv.h>

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

/******************************************************************************
* 
*******************************************************************************/
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

/******************************************************************************
* 
*******************************************************************************/
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
static bool
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


/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    MY_LOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            MY_LOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    MY_LOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}

/*******************************************************************************
*  Postproc 1 IN / 2 Out 
********************************************************************************/
#define VIDEO_PORT_ON     0
int main_testPostProc(IMEM_BUF_INFO rInMem, int width, int height, int format) 
{
    MY_LOGD("+"); 
    MUINT32 u4DispW = 640;//1280; 
    MUINT32 u4DispH = 480;//960; 

    // (1). Create Instance 
    IPostProcPipe    *pPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, eScenarioFmt_RAW); 
    if (pPostProcPipe != NULL) 
    {
        MY_LOGD("Pipe (Name, ID, IDName) = (%s, %d)", pPostProcPipe->getPipeName(), pPostProcPipe->getPipeId()); 
    }
    
    // (2). Query port property
    vector<PortProperty> rInPortProperty; 
    vector<PortProperty> rOutPortProperty;     
    if (pPostProcPipe->queryPipeProperty(rInPortProperty,rOutPortProperty))
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
    pPostProcPipe->init(); 

    // (4). setCallback
    pPostProcPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    MUINT32 u4RawStride[3] = {width, 0, 0}; 
    MemoryInPortInfo rMemInPort(ImgInfo(eImgFmt_BAYER10, width, height), 
                                0, u4RawStride, Rect(0, 0, width, height)); 
    //
    MUINT32 u4DispStride[3] = {u4DispW, 0, 0}; 
    MemoryOutPortInfo rDispPort(ImgInfo(eImgFmt_YUY2, u4DispW, u4DispH), 
                               u4DispStride, 0, 0); 
    //
    vector<PortInfo const*> vCdpInPorts;  
    vector<PortInfo const*> vCdpOutPorts; 
    //
    vCdpInPorts.push_back(&rMemInPort); 
    vCdpOutPorts.push_back(&rDispPort); 
#if VIDEO_PORT_ON
    MUINT32 u4VdoStride[3] = {u4DispW, 0, 0}; 
    MemoryOutPortInfo rVdoPort(ImgInfo(eImgFmt_YUY2, u4DispW, u4DispH), 
                               u4VdoStride, 0, 0);   
    rVdoPort.index = 1;   
    vCdpOutPorts.push_back(&rVdoPort); 
#endif 
    //
    pPostProcPipe->configPipe(vCdpInPorts, vCdpOutPorts); 

    // (6). Enqueue, In buf
    // 
    QBufInfo rInBuf; 
    rInBuf.vBufInfo.clear(); 
    BufInfo rBufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID);  
    rInBuf.vBufInfo.push_back(rBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryIn, 0, 0), rInBuf); 

    // (6.1) Enqueue, Out Buf
    // 
    IMEM_BUF_INFO rDispMem; 
    rDispMem.size = ((u4DispW * u4DispH * 2) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);     
    allocMem(rDispMem);     
    // 
    QBufInfo rOutBuf; 
    rOutBuf.vBufInfo.clear(); 
    rBufInfo.u4BufSize = rDispMem.size; 
    rBufInfo.u4BufVA = rDispMem.virtAddr; 
    rBufInfo.u4BufPA = rDispMem.phyAddr; 
    rBufInfo.i4MemID = rDispMem.memID; 
    rOutBuf.vBufInfo.push_back(rBufInfo); 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rOutBuf); 

    //
#if  VIDEO_PORT_ON
    IMEM_BUF_INFO rVdoMem; 
    rVdoMem.size = ((u4DispW * u4DispH * 2)+ L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1); 
    allocMem(rVdoMem);    
    //
    rOutBuf.vBufInfo.at(0).u4BufSize = rVdoMem.size; 
    rOutBuf.vBufInfo.at(0).u4BufVA = rVdoMem.virtAddr; 
    rOutBuf.vBufInfo.at(0).u4BufPA = rVdoMem.phyAddr; 
    rOutBuf.vBufInfo.at(0).i4MemID = rVdoMem.memID; 
    pPostProcPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rOutBuf); 
#endif 

    // (7). start
    pPostProcPipe->start(); 

    // (8). Dequeue
    QTimeStampBufInfo rQDispOutBuf;         
    QTimeStampBufInfo rQVdoOutBuf; 
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQDispOutBuf); 

#if VIDEO_PORT_ON
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 
#endif 
    // (8.1) Dequeue In Buf 
    QTimeStampBufInfo rQInBuf;             
    pPostProcPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf); 
 
    // (9). Stop 
    pPostProcPipe->stop();
    // (10). uninit 
    pPostProcPipe->uninit(); 
    // (11). destory instance 
    pPostProcPipe->destroyInstance(); 

    char filename[256]; 
    sprintf(filename, "/data/disp%dx%d.yuv", u4DispW, u4DispH);     
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(rDispMem.virtAddr), u4DispW * u4DispH * 2); 
    deallocMem(rDispMem); 
#if VIDEO_PORT_ON
    sprintf(filename, "/data/vdo%dx%d.yuv", u4DispW, u4DispH);     
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(rVdoMem.virtAddr), u4DispW * u4DispH * 2); 
    deallocMem(rVdoMem); 
#endif 
    //
    return 0; 
}


/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_postproc(int argc, char** argv)
{
    int ret = 0; 
    char *filename; 
    int width = 0, height = 0; 
    int format = 0; 
    IMEM_BUF_INFO rInMem;
    //
    if (argc != 5) 
    {
        printf("Usage: campipetest 1 <filename> <width> <height> <format>\n"); 
#define printf_fmt( fmt ) printf(" %s: %i\n", #fmt, fmt)
        printf(" fmt example:\n");
        printf_fmt( eImgFmt_YUY2 );
        printf_fmt( eImgFmt_NV21 );
        printf_fmt( eImgFmt_YV12 );
        printf_fmt( eImgFmt_BAYER8 );
        printf_fmt( eImgFmt_BAYER10 );
        goto EXIT; 
    }
 
    filename = argv[1]; 
    width = atoi(argv[2]); 
    height = atoi(argv[3]); 
    format = atoi(argv[4]); 

    if (width > 5000 || width < 0)
    {
        width = 0; 
    }
    if (height > 5000 || height < 0) 
    {
        height = 0; 
    }
    //
    g_pIMemDrv =  IMemDrv::createInstance(); 
    if (NULL == g_pIMemDrv)
    {
        MY_LOGE("g_pIMemDrv is NULL"); 
        return 0; 
    }

    if ( !g_pIMemDrv->init() )
    {
        MY_LOGE("g_pIMemDrv init fail");
    }

    //  
    rInMem.size = (width * height * 2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);     
    allocMem(rInMem); 

    loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), 0 ), 
    main_testPostProc(rInMem, width, height, format); 
 
    deallocMem(rInMem); 
    //
    g_pIMemDrv->destroyInstance(); 

EXIT:
    printf("press any key to exit \n"); 
    getchar(); 

    return ret; 
}
