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
#include <mtkcam/campipe/IXdpPipe.h>

//
//#include <mtkcam/hal/sensor_hal.h>
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
static int in_w = 0, in_h = 0; 
static int in_fmt = 0; 
static int out1_w = 0, out1_h = 0;
static EImageFormat out1_fmt = eImgFmt_UNKNOWN;
static unsigned int out1_rotation = 0, out1_flip = 0;
static int out2_w = 0, out2_h = 0;
static EImageFormat out2_fmt = eImgFmt_UNKNOWN;

static unsigned int getBitsPerPixel(EImageFormat fmt)
{
    switch(fmt)
    {
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
            return 16;
            break;
        case eImgFmt_YV12:
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK:
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
        case eImgFmt_I420:
            return 8;
            break;
        default:
            MY_LOGE("format not support 0x%x", fmt);
            break;
    }
    return 0;
}

static unsigned int getSize(EImageFormat fmt, unsigned int stride, unsigned int h)
{
    switch(fmt)
    {
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
            return stride * h * 2;
            break;
        case eImgFmt_YV12:
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK:
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
        case eImgFmt_I420:
            return stride * h + ( ( stride * h >> 2 ) << 1);
            break;
        default:
            MY_LOGE("format not support %i", fmt);
            break;

    }
}

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
*  Xdp 1 IN / 2 Out 
********************************************************************************/
int main_testXDP(IMEM_BUF_INFO rInMem, int width, int height, EImageFormat format) 
{
    MY_LOGD("+"); 
    // (1). Create Instance 
    IXdpPipe    *pXdpPipe = IXdpPipe::createInstance(eSWScenarioID_CAPTURE_NORMAL, eScenarioFmt_YUV); 
    if (pXdpPipe != NULL) 
    {
        MY_LOGD("Pipe (Name, ID, IDName) = (%s, %d)", pXdpPipe->getPipeName(), pXdpPipe->getPipeId()); 
    }
    
    // (2). Query port property
    vector<PortProperty> rInPortProperty; 
    vector<PortProperty> rOutPortProperty;     
    if (pXdpPipe->queryPipeProperty(rInPortProperty,rOutPortProperty))
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
    pXdpPipe->init(); 

    // (4). setCallback
    pXdpPipe->setCallbacks(NULL, NULL, NULL); 

    // (5). Config pipe 
    // 
    MUINT32 u4YuvStride[3] = {getBitsPerPixel(format) * width >> 3, 0, 0};//YUY2
    MemoryInPortInfo rMemInPort(ImgInfo(eImgFmt_YUY2, width, height), 
                                0, u4YuvStride, Rect(0, 0, width, height)); 
    //
    MUINT32 u4VdoStride[3] = {getBitsPerPixel(out1_fmt) * out1_w >> 3, 0, 0}; 
    MemoryOutPortInfo rVdoPort(ImgInfo(out1_fmt, out1_w, out1_h), 
                               u4VdoStride, 0, 0);   
    rVdoPort.index = 1;   
    rVdoPort.u4Flip = out1_flip;
    rVdoPort.u4Rotation = out1_rotation;

    MUINT32 u4DispStride[3] = {getBitsPerPixel(out2_fmt) * out2_w >> 3, 0, 0}; 
    MemoryOutPortInfo rDispPort(ImgInfo(out2_fmt, out2_w, out2_h), 
                               u4DispStride, 0, 0); 
    //
    vector<PortInfo const*> vXdpInPorts;  
    vector<PortInfo const*> vXdpOutPorts; 
    //
    vXdpInPorts.push_back(&rMemInPort); 
    vXdpOutPorts.push_back(&rDispPort); 
    vXdpOutPorts.push_back(&rVdoPort); 
    //
    pXdpPipe->configPipe(vXdpInPorts, vXdpOutPorts); 

    // (6). Enqueue, In buf
    // 
    QBufInfo rInBuf; 
    rInBuf.vBufInfo.clear(); 
    BufInfo rBufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID);  
    rInBuf.vBufInfo.push_back(rBufInfo); 
    pXdpPipe->enqueBuf(PortID(EPortType_MemoryIn, 0, 0), rInBuf); 

    // (6.1) Enqueue, Out Buf
    // 
    IMEM_BUF_INFO rDispMem; 
    rDispMem.size = ((out2_w * out2_h * 2) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);     
    allocMem(rDispMem);     
    // 
    QBufInfo rOutBuf; 
    rOutBuf.vBufInfo.clear(); 
    rBufInfo.u4BufSize = rDispMem.size; 
    rBufInfo.u4BufVA = rDispMem.virtAddr; 
    rBufInfo.u4BufPA = rDispMem.phyAddr; 
    rBufInfo.i4MemID = rDispMem.memID; 
    rOutBuf.vBufInfo.push_back(rBufInfo); 
    pXdpPipe->enqueBuf(PortID(EPortType_MemoryOut, 0, 1), rOutBuf); 

    //
    IMEM_BUF_INFO rVdoMem; 
    rVdoMem.size = ((out1_w * out1_h * 2)+ L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1); 
    allocMem(rVdoMem);    
    //
    rOutBuf.vBufInfo.at(0).u4BufSize = rVdoMem.size; 
    rOutBuf.vBufInfo.at(0).u4BufVA = rVdoMem.virtAddr; 
    rOutBuf.vBufInfo.at(0).u4BufPA = rVdoMem.phyAddr; 
    rOutBuf.vBufInfo.at(0).i4MemID = rVdoMem.memID; 
    pXdpPipe->enqueBuf(PortID(EPortType_MemoryOut, 1, 1), rOutBuf); 


    // (7). start
    pXdpPipe->start(); 

    // (8). Dequeue
    QTimeStampBufInfo rQDispOutBuf;         
    QTimeStampBufInfo rQVdoOutBuf; 
    QTimeStampBufInfo rQInBuf; 
    pXdpPipe->dequeBuf(PortID(EPortType_MemoryOut, 0, 1), rQDispOutBuf); 
    pXdpPipe->dequeBuf(PortID(EPortType_MemoryOut, 1, 1), rQVdoOutBuf); 
    pXdpPipe->dequeBuf(PortID(EPortType_MemoryIn, 0, 0), rQInBuf);  

 
    // (9). Stop 
    pXdpPipe->stop();
    // (10). uninit 
    pXdpPipe->uninit(); 
    // (11). destory instance 
    pXdpPipe->destroyInstance(); 

    char filename[256]; 
    sprintf(filename, "/data/disp%dx%d.yuv", out2_w, out2_h);     
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(rDispMem.virtAddr), out2_w * out2_h * 2); 
    deallocMem(rDispMem); 
    sprintf(filename, "/data/vdo%dx%d.yuv", out1_w, out1_h);     
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(rVdoMem.virtAddr), out1_w * out1_h * 2); 
    deallocMem(rVdoMem); 

    //
    return 0; 
}


/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_xdp(int argc, char** argv)
{
    int ret = 0; 
    char *filename; 

    IMEM_BUF_INFO rInMem; 
    //
    if (argc != 4 && argc != 12) 
    {
        printf("Usage: campipetest 2 <filename> <width> <height>");
        printf(" <out1_w> <out1_h> <out1_fmt> <rotation> <flip>");
        printf(" <out2_w> <out2_h> <out2_fmt> \n");
#define printf_fmt( fmt ) printf(" %s: %x\n", #fmt, fmt)
        printf(" fmt example:\n");
        printf_fmt( eImgFmt_YUY2 );
        printf_fmt( eImgFmt_NV21 );
        printf_fmt( eImgFmt_YV12 );
        goto EXIT; 
    }
 
    filename = argv[1]; 
    in_w = atoi(argv[2]); 
    in_h = atoi(argv[3]); 

    if (argc == 4 )
    {
        //default value
        out1_w = in_w;
        out1_h = in_h;
        out1_rotation = 0;
        out1_flip = 0;
        out1_fmt = eImgFmt_NV21;

        out2_w = 320;
        out2_h = 240;
        out2_fmt = eImgFmt_NV21;
    }
    else
    {
        out1_w = atoi(argv[4]);
        out1_h = atoi(argv[5]);
        //out1_fmt = EImageFormat(atoi(argv[6]));
        sscanf( argv[6], "%x", &out1_fmt );
        out1_rotation = atoi(argv[7]);
        out1_flip = atoi(argv[8]);

        out2_w = atoi(argv[9]);
        out2_h = atoi(argv[10]);
        //out2_fmt = EImageFormat(atoi(argv[11]));
        sscanf( argv[11], "%x", &out2_fmt );

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
    rInMem.size = (in_w * in_h * 2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);     
    allocMem(rInMem); 

    if (in_w > 5000 || in_w < 0)
    {
        in_w = 0; 
    }
    if (in_h > 5000 || in_h < 0) 
    {
        in_h = 0; 
    }

    //input is always in YUY2 format
    loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), in_w * in_h * 2), 
    main_testXDP(rInMem, in_w, in_h, eImgFmt_YUY2); 
 
    deallocMem(rInMem); 
    //
    g_pIMemDrv->destroyInstance(); 

EXIT:
    printf("press any key to exit \n"); 
    //getchar(); 

    return ret; 
}
