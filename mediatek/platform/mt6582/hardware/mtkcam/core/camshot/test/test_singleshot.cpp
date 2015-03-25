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
 
#define LOG_TAG "CamShotTest"


#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>

//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>

extern "C" {
#include <pthread.h>
}

//
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/hal/sensor_hal.h>


using namespace NSCamShot; 
using namespace NSCamHW; 

/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)

#define POSTVIEW_WIDTH  640
#define POSTVEIW_HEIGHT 480 

static  IMemDrv *g_pIMemDrv;
static SensorHal *g_pSensorHal = NULL; 

static pthread_t g_CliKeyThreadHandle; 
static MBOOL g_bIsCLITest = MTRUE;

static EImageFormat g_eImgFmt[] = {eImgFmt_YUY2, eImgFmt_NV21, eImgFmt_I420, eImgFmt_YV16, eImgFmt_JPEG} ; 

static IMEM_BUF_INFO g_rRawMem; 
static IMEM_BUF_INFO g_rYuvMem; 
static IMEM_BUF_INFO g_rPostViewMem; 
static IMEM_BUF_INFO g_rJpegMem; 

static ImgBufInfo g_rRawBufInfo; 
static ImgBufInfo g_rYuvBufInfo; 
static ImgBufInfo g_rPostViewBufInfo; 
static ImgBufInfo g_rJpegBufInfo; 

static MUINT32 u4CapCnt = 0; 
static MUINT32 g_u4Width = 1280; 
static MUINT32 g_u4Height = 960; 
static MUINT32 g_u4SensorWidth = 0; 
static MUINT32 g_u4SensorHeight = 0; 

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
* 
*******************************************************************************/
static MBOOL reallocMem(IMEM_BUF_INFO & rMemBuf, MUINT32 const u4Size )
{       
    //
    deallocMem(rMemBuf); 
    rMemBuf.size = u4Size; 
    //
    allocMem(rMemBuf);     
    return MTRUE; 
}


/******************************************************************************
* 
*******************************************************************************/
static MBOOL allocImgMem(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height, IMEM_BUF_INFO & rMem)
{
    //
    MY_LOGD("[allocImgMem], (format, width, height) = (0x%x, %d, %d)",  eFmt, u4Width, u4Height); 
#warning [TODO] buffer size 
    // 422 format
    MUINT32 u4BufSize = u4Width * u4Height * 2;
    // 420 format
    if (eImgFmt_YV12 == eFmt 
       || eImgFmt_NV21 == eFmt 
       || eImgFmt_NV12 == eFmt
       || eImgFmt_I420 == eFmt)
    {
        u4BufSize = u4Width * u4Height * 3 / 2; 
    }
    else if (eImgFmt_RGB888 == eFmt) 
    {
        u4BufSize = u4Width * u4Height * 3; 
    }  
    else if (eImgFmt_ARGB888 == eFmt) 
    {
        u4BufSize = u4Width * u4Height * 4; 
    }
    //
    if (0 == rMem.size) 
    {        
        rMem.size = (u4BufSize  + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);    
        allocMem(rMem); 
        MY_LOGD("[allocImgMem] (va, pa, size) = (0x%x, 0x%x, %d)",  rMem.virtAddr, rMem.phyAddr, rMem.size);  
    }
    else 
    {
        if (rMem.size < u4BufSize) 
        {          
            reallocMem(rMem, u4BufSize); 
            MY_LOGD("[allocImgMem] re-allocate (va, pa, size) = (0x%x, 0x%x, %d)", rMem.virtAddr, rMem.phyAddr, rMem.size);  
        }
    }  
    return MTRUE; 
}



/******************************************************************************
* save the buffer to the file 
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, 0x%x, %d)", fname, buf, size); 
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


/////////////////////////////////////////////////////////////////////////
//! Nucamera commands
/////////////////////////////////////////////////////////////////////////
typedef struct CLICmd_t
{
    //! Command string, include shortcut key
    const char *pucCmdStr;

    //! Help string, include functionality and parameter description
    const char *pucHelpStr;

    //! Handling function
    //! \param a_u4Argc  [IN] Number of arguments plus 1
    //! \param a_pprArgv [IN] Array of command and arguments, element 0 is
    //!                       command string
    //! \return error code
    //FIXME: return MRESULT is good?
    MUINT32 (*handleCmd)(const int argc, char** argv);

} CLICmd;



/******************************************************************************
* 
*******************************************************************************/
static MBOOL fgCamShotNotifyCb(MVOID* user, CamShotNotifyInfo const msg)
{
    return MTRUE;
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleBayerDataCallback(CamShotDataInfo const msg) 
{
    MY_LOGD("handleBayerDataCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/bayer_%ux%u_%02d.raw", g_u4SensorWidth, g_u4SensorHeight, u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 

    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleYuvDataCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleYuvDataCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/yuv_%ux%u_%02d.yuv", g_u4Width, g_u4Height, u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handlePostViewCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handlePostViewCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/postview%02d.yuv", u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
    return 0; 
}

/******************************************************************************
* 
*******************************************************************************/
static MBOOL handleJpegCallback(CamShotDataInfo const msg)
{
    MY_LOGD("handleJpegCallback"); 
    char fileName[256] = {'\0'}; 
    sprintf(fileName, "/data/jpeg%02d.jpg", u4CapCnt); 
    saveBufToFile(fileName, msg.puData, msg.u4Size); 
 
    memset(fileName, '\0', 256); 
    sprintf(fileName, "/data/thumb%02d.jpg", u4CapCnt); 
    saveBufToFile(fileName, reinterpret_cast<MUINT8*> (msg.ext1), msg.ext2); 
 
    return 0; 
}



/******************************************************************************
* 
*******************************************************************************/
static MBOOL fgCamShotDataCb(MVOID* user, CamShotDataInfo const msg)
{
    CamShotDataInfo rDataInfo = msg; 

    switch (rDataInfo.msgType) 
    {
        case ECamShot_DATA_MSG_BAYER:
            handleBayerDataCallback(msg); 
        break; 
        case ECamShot_DATA_MSG_YUV:
            handleYuvDataCallback(msg); 
        break; 
        case ECamShot_DATA_MSG_POSTVIEW:
            handlePostViewCallback(msg);             
        break; 
        case ECamShot_DATA_MSG_JPEG:
            handleJpegCallback(msg); 
        break; 
  
    }

    return MTRUE;
}


static MVOID allocateMem(MUINT32 u4EnableMsg)
{
#warning [TODO] allocate memory according to the format
    if (u4EnableMsg & ECamShot_DATA_MSG_BAYER) 
    {
        allocImgMem(eImgFmt_YUY2,g_u4SensorWidth ,g_u4SensorHeight , g_rRawMem); 
        
        g_rRawBufInfo.u4ImgWidth = g_u4SensorWidth;
        g_rRawBufInfo.u4ImgHeight = g_u4SensorHeight; 
        g_rRawBufInfo.eImgFmt = eImgFmt_BAYER8; 
        g_rRawBufInfo.u4Stride[0] = g_u4SensorWidth; 
        g_rRawBufInfo.u4BufSize = g_rRawMem.size; 
        g_rRawBufInfo.u4BufVA = g_rRawMem.virtAddr;
        g_rRawBufInfo.u4BufPA = g_rRawMem.phyAddr;
        g_rRawBufInfo.i4MemID = g_rRawMem.memID; 

    }
    
    if (u4EnableMsg & ECamShot_DATA_MSG_YUV) 
    {
        EImageFormat eFmt = eImgFmt_YUY2; 
        allocImgMem(eFmt, g_u4Width, g_u4Height, g_rYuvMem);
        g_rYuvBufInfo.u4ImgWidth = g_u4Width;
        g_rYuvBufInfo.u4ImgHeight = g_u4Height; 
        g_rYuvBufInfo.eImgFmt = eFmt; 
        g_rYuvBufInfo.u4Stride[0] = g_u4Width; 
        g_rYuvBufInfo.u4Stride[1] = g_u4Width ; 
        g_rYuvBufInfo.u4Stride[2] = 0; 
        g_rYuvBufInfo.u4BufSize = g_rYuvMem.size; 
        g_rYuvBufInfo.u4BufVA = g_rYuvMem.virtAddr;
        g_rYuvBufInfo.u4BufPA = g_rYuvMem.phyAddr;
        g_rYuvBufInfo.i4MemID = g_rYuvMem.memID; 
    }
   
    if (u4EnableMsg & ECamShot_DATA_MSG_POSTVIEW) 
    {
        //allocImgMem(eImgFmt_YUY2, 640, 480, g_rPostViewMem); 
        MUINT32 u4Width = 640; 
        MUINT32 u4Height = 480; 
        EImageFormat eFmt = eImgFmt_YUY2; 
        allocImgMem(eFmt, u4Width, u4Height, g_rPostViewMem); 
        g_rPostViewBufInfo.u4ImgWidth = u4Width; ;
        g_rPostViewBufInfo.u4ImgHeight = u4Height; ; 
        g_rPostViewBufInfo.eImgFmt = eFmt; 
        //g_rPostViewBufInfo.u4Stride[0] = (~31) & (31 + u4Width); 
        //g_rPostViewBufInfo.u4Stride[1] = (~15) & (15 + (u4Width >> 1)); 
        //g_rPostViewBufInfo.u4Stride[2] = (~15) & (15 + (u4Width >> 1)); 
        g_rPostViewBufInfo.u4Stride[0] = u4Width; 
        g_rPostViewBufInfo.u4Stride[1] = u4Width ; 
        g_rPostViewBufInfo.u4Stride[2] = u4Width ;

        g_rPostViewBufInfo.u4BufSize = g_rPostViewMem.size; 
        g_rPostViewBufInfo.u4BufVA = g_rPostViewMem.virtAddr;
        g_rPostViewBufInfo.u4BufPA = g_rPostViewMem.phyAddr;
        g_rPostViewBufInfo.i4MemID = g_rPostViewMem.memID; 
    }
    
    if (u4EnableMsg & ECamShot_DATA_MSG_JPEG) 
    {
        allocImgMem(eImgFmt_YUY2, g_u4Width/2, g_u4Height/2, g_rJpegMem);   
        g_rJpegBufInfo.u4ImgWidth = g_u4Width;
        g_rJpegBufInfo.u4ImgHeight = g_u4Height; 
        g_rJpegBufInfo.eImgFmt = eImgFmt_JPEG; 
        g_rJpegBufInfo.u4Stride[0] = g_u4Width; 
        g_rJpegBufInfo.u4BufSize = g_rJpegMem.size; 
        g_rJpegBufInfo.u4BufVA = g_rJpegMem.virtAddr;
        g_rJpegBufInfo.u4BufPA = g_rJpegMem.phyAddr;
        g_rJpegBufInfo.i4MemID = g_rJpegMem.memID; 
    }
}


/******************************************************************************
* 
*******************************************************************************/
#define ALLOCA_MEM    0
static MUINT32 u4Capture_Cmd(int argc, char** argv)
{
    MUINT32 u4Mode = ACDK_SCENARIO_ID_CAMERA_PREVIEW; 
    MUINT32 u4EnableMsg = 0xf;    //jpeg only
    MUINT32 u4Rot = 0; 
    MUINT32 u4ShotCnt = 1; 
    u4CapCnt = 0; 

    if (argc == 1) 
    {
        u4Mode = atoi(argv[0]); 
    }
    else if (argc == 3) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
    }
    else if (argc == 4) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
    }
    else if (argc == 5) 
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
        u4Rot = atoi(argv[4]); 
    }
    else if (argc == 6)
    {
        u4Mode = atoi(argv[0]); 
        g_u4Width = atoi(argv[1]); 
        g_u4Height = atoi(argv[2]); 
        sscanf( argv[3],"%x", &u4EnableMsg); 
        u4Rot = atoi(argv[4]); 
        u4ShotCnt = atoi(argv[5]); 

    }

    MUINT32 cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
    if (u4Mode == 0)  
    {
        cmd = SENSOR_CMD_GET_SENSOR_PRV_RANGE; 
    }
    else if (1 == u4Mode) 
    {
        cmd = SENSOR_CMD_GET_SENSOR_FULL_RANGE;    
    } 

    g_pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                             cmd,
                             (int)&g_u4SensorWidth,
                             (int)&g_u4SensorHeight,
                             0
                            );    

    MY_LOGD("sensor width:%d, height:%d\n", g_u4SensorWidth, g_u4SensorHeight); 
    MY_LOGD("capture width:%d, height:%d\n, mode:%d, image:0x%x, count:%d\n", g_u4Width, g_u4Height, u4Mode, u4EnableMsg, u4ShotCnt); 

    ISingleShot *pSingleShot = ISingleShot::createInstance(eShotMode_NormalShot, "testshot"); 
    // 
    pSingleShot->init(); 
 
    // 
    pSingleShot->enableDataMsg(u4EnableMsg ); 
    // set buffer 
    //
#if ALLOCA_MEM
    allocateMem(u4EnableMsg); 
    //
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_BAYER, g_rRawBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, g_rYuvBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_POSTVIEW, g_rPostViewBufInfo); 
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_JPEG, g_rJpegBufInfo); 
#endif 
   

    // shot param 
    ShotParam rShotParam(eImgFmt_YUY2,           //yuv format 
                         g_u4Width,              //picutre width 
                         g_u4Height,             //picture height
                         u4Rot * 90,                      //picutre rotation 
                         0,                      //picutre flip 
                         eImgFmt_YV12,           //postview format 
                         800,                    //postview width 
                         480,                    //postview height 
                         0,                      //postview rotation 
                         0,                      //postview flip 
                         100                     //zoom   
                        );                                  
 
    // jpeg param 
    JpegParam rJpegParam(ThumbnailParam(160, 128, 100, MTRUE),
                         90,                     //Quality 
                         MTRUE                   //isSOI 
                        ); 
 
    // thumbnail param 
    ThumbnailParam rThumbnailParam(160,          // thumbnail width
                                   128,          // thumbnail height
                                   100,          // quality 
                                   MTRUE         // isSOI     
                                  ); 

    // sensor param 
    SensorParam rSensorParam(SENSOR_DEV_MAIN,                         //Device ID 
               u4Mode == 0 ? ACDK_SCENARIO_ID_CAMERA_PREVIEW : ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio 
                             10,                                       //bit depth 
                             MTRUE,                                   //bypass delay 
                             MFALSE                                   //bypass scenario 
                            );  
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, NULL); 
    //     
    pSingleShot->setShotParam(rShotParam); 
    //
    pSingleShot->setJpegParam(rJpegParam); 
    // 
    // 
    for (MUINT32 i = 0 ; i < u4ShotCnt; i++) 
    {
        pSingleShot->startOne(rSensorParam); 
        u4CapCnt++; 
    }
    //
    pSingleShot->uninit(); 
    //
    pSingleShot->destroyInstance(); 



    return 0; 
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_rTest_Cmds[] =
{
    {"cap", "cap <mode:0:prv, 1:cap> <width> <height> <img:0x1:raw, 0x2:yuv, 0x4:postview, 0x8:jpeg> <rot:0, 1:90, 2:180, 3:270> <Count>", u4Capture_Cmd},
    {NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param sig: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void thread_exit_handler(int sig)
{ 
    printf("This signal is %d \n", sig);
    pthread_exit(0);
}    

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;
    
    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


//  vHelp () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
static void vHelp()
{
    printf("\n***********************************************************\n");
    printf("* CamShot SingleShot CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n"); 
    printf("===========================================================\n");    

    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");

    int i = 0; 
    for (i = 0; ; i++)
    {
        if (NULL == g_rTest_Cmds[i].pucCmdStr) 
        {
            break; 
        } 
        printf("%s    [%s]\n", g_rTest_Cmds[i].pucCmdStr, 
                               g_rTest_Cmds[i].pucHelpStr);        
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () - 
//! @brief the CLI key input thread, wait for CLI command 
//! @param a_pArg: The input arguments 
/////////////////////////////////////////////////////////////////////////
static void* cliKeyThread (void *a_pArg)
{
    char urCmds[256] = {0}; 

    //! ************************************************
    //! Set the signal for kill self thread 
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal 
    //! to kill self 
    //! ************************************************    
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions)); 
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0; 
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);
    
    while (1)
    {
        printf("Input Cmd#"); 
        fgets(urCmds, 256, stdin);

        //remove the '\n' 
        urCmds[strlen(urCmds)-1] = '\0';         
        char *pCmds = &urCmds[0];         
        //remove the space in the front of the string 
        vSkipSpace(&pCmds); 
       
        //Ignore blank command 
        if (*pCmds == '\0')
        {
            continue; 
        }
        
        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        char  *pucStrToken, *pucCmdToken;
        char  *pucArgValues[25];
        
        pucStrToken = (char *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(char*) pucStrToken;
            pucStrToken = (char*)strtok (NULL, " ");                
        }

        if (u4ArgCount == 0)
        {
            continue; 
        }
        
        pucCmdToken = (char*) pucArgValues[0]; 

        //parse the command 
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp(); 
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) || 
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            printf("Exit From CLI\n"); 
            g_bIsCLITest = MFALSE; 
            break; 
        }
        else
        {
            MBOOL bIsFoundCmd = MFALSE;
            for (MUINT32 u4CmdIndex = 0; ; u4CmdIndex++)
            {
                if(NULL == g_rTest_Cmds[u4CmdIndex].pucCmdStr)
                {
                    break; 
                }  
                if (strcmp((char *)pucCmdToken, g_rTest_Cmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE; 
                    g_rTest_Cmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);                     
                    break;
                }                
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n"); 
            }            
        }        
            
    }

    return 0; 
}




/*******************************************************************************
*  Main Function 
********************************************************************************/
int main_singleshot(int argc, char** argv)
{
    printf("SingleShot Test \n");     

    vHelp(); 
    
    g_pIMemDrv =  IMemDrv::createInstance(); 
    if (NULL == g_pIMemDrv)
    {
        MY_LOGE("g_pIMemDrv is NULL"); 
        return 0; 
    }
    g_pIMemDrv->init(); 


    // init sensor first 
    g_pSensorHal = SensorHal:: createInstance();

    if (NULL == g_pSensorHal) 
    {
        MY_LOGE("pSensorHal is NULL"); 
        return 0; 
    }
    // search sensor 
    g_pSensorHal->searchSensor();

    //
    // (1). init main sensor  
    //
    g_pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                    SENSOR_CMD_SET_SENSOR_DEV,
                                    0,
                                    0,
                                    0); 
    //
    g_pSensorHal->init(); 



    
    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL); 

    //!***************************************************
    //! Main thread wait for exit 
    //!***************************************************    
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000); 
    }


    //
    //
    g_pSensorHal->uninit(); 
    //
    g_pSensorHal->destroyInstance(); 
    //
    if (0 != g_rRawMem.size)
    {
        deallocMem(g_rRawMem); 
    }

    if (0 != g_rYuvMem.size)
    {
        deallocMem(g_rYuvMem); 
    }

    if (0 != g_rPostViewMem.size)
    {
        deallocMem(g_rPostViewMem); 
    }

    if (0 != g_rJpegMem.size)
    {
        deallocMem(g_rJpegMem); 
    }

    g_pIMemDrv->uninit(); 
    g_pIMemDrv->destroyInstance(); 



    return 0; 
}
