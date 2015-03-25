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

#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>

extern "C" {
#include <pthread.h>
}

//
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/hal/sensor_hal.h>

//kidd
#include "Hdr.h"
#include "MyHdr.h"


using namespace NSCamShot;
using namespace NSCamHW;
using namespace android::NSShot;

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

#define	WORKAROUND_IMEM	1

static  IMemDrv *g_pIMemDrv;

static pthread_t g_CliKeyThreadHandle;
static MBOOL g_bIsCLITest = MTRUE;

static EImageFormat g_eImgFmt[] = {eImgFmt_YUY2, eImgFmt_NV21, eImgFmt_I420, eImgFmt_YV16, eImgFmt_JPEG, eImgFmt_YV12} ;



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
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
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
        if (nr == 0) {
            MY_LOGE("can't read from file [%s]", fname);
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
* test capture
*******************************************************************************/
static MUINT32 u4Capture_Cmd(int argc, char** argv)
{
    // init sensor first
    SensorHal *pSensorHal = SensorHal:: createInstance();

    if (NULL == pSensorHal)
    {
        MY_LOGE("pSensorHal is NULL");
        return 0;
    }
    // search sensor
    pSensorHal->searchSensor();

    //
    // (1). init main sensor
    //
    printf("init main sensor\n");
    pSensorHal->sendCommand(SENSOR_DEV_MAIN,
                                    SENSOR_CMD_SET_SENSOR_DEV,
                                    0,
                                    0,
                                    0);
    //
    printf("pSensorHal->init\n");
    pSensorHal->init();

    //insert HDR
    printf("insert HDR\n");
    //sp<HdrShot> pShot = NULL;
    HdrShot *pShot = NULL;
    pShot = new HdrShot("HdrShot", 0, 0);
	pShot->mTestMode = 1;	//test only
	//for ImpShit interface
    printf("pShot->onCmd_capture\n");
	pShot->sendCommand(eCmd_capture, 0, 0);

    printf("delete pShot\n");
	delete pShot;
    //
    printf("pSensorHal->destroyInstance\n");
    pSensorHal->destroyInstance();

    return 0;
}


static MUINT32 u4CDPNV21_Cmd(int argc, char** argv)
{
	//1. allocate buffer
	printf("allocate buffer\n");
	IMemDrv *mpIMemDrv = IMemDrv::createInstance();
	mpIMemDrv->init();	//check this, see fd


	//2. source image
	printf("source image\n");
	IMEM_BUF_INFO   mpSourceImgBuf;
	mpSourceImgBuf.size = 1280 * 960 * 1.5;
	if(mpIMemDrv->allocVirtBuf(&mpSourceImgBuf))
		return MFALSE;
	uint32_t nReadSize = loadFileToBuf("source.nv21.yuv", (uint8_t*)mpSourceImgBuf.virtAddr, mpSourceImgBuf.size);


	//3. target image
	printf("target image\n");
	IMEM_BUF_INFO   mpTargetImgBuf;
	mpTargetImgBuf.size = 160 * 120 * 1.5;
	if(mpIMemDrv->allocVirtBuf(&mpTargetImgBuf))
		return MFALSE;


	//4. resize
	printf("resize\n");
	HdrShot::CDPResize_simple(
			&mpSourceImgBuf, 1280, 960, eImgFmt_NV21,
			&mpTargetImgBuf, 160, 120, eImgFmt_NV21, 0);
	saveBufToFile("/sdcard/target.nv21.yuv", (MUINT8 *)mpTargetImgBuf.virtAddr, mpTargetImgBuf.size);


	//5. final
	printf("final\n");
    return 0;
}


static MUINT32 u4CDPYUY2_Cmd(int argc, char** argv)
{
	MUINT32 u4SrcWidth = 1280;
	MUINT32 u4SrcHeight = 960;
	MUINT32 u4TargetWidth = 160;
	MUINT32 u4TargetHeight = 120;
	int srcFmt = 0;
	char filename[128];
	sprintf(filename, "/sdcard/source.yuy2.yuv");

    printf("SImager Test \n");
    //
    IMemDrv *pIMemDrv =  IMemDrv::createInstance();
    pIMemDrv->init();
    if (NULL == pIMemDrv)
    {
        printf("g_pIMemDrv is NULL");
        return 0;
    }
    //
    IMEM_BUF_INFO rInMem;
    rInMem.size = (u4SrcWidth * u4SrcHeight * 2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rInMem)) {
        printf("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rInMem.virtAddr, 0 , rInMem.size);
    if (pIMemDrv->mapPhyAddr(&rInMem)) {
        printf("mpIMemDrv->mapPhyAddr() error");
    }
#endif

    if(!loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), u4SrcWidth * u4SrcHeight * 2)) {
	    printf("can't load image:%s\n", filename);
		return 0;
	}
    printf("load image:%s\n", filename);

    //
	IMEM_BUF_INFO rOutMem;
    rOutMem.size = (u4TargetWidth * u4TargetHeight * 2 + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rOutMem)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rOutMem.virtAddr, 0 , rOutMem.size);
    if (pIMemDrv->mapPhyAddr(&rOutMem)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
#endif
#if 0
    //
    //MUINT32 u4InStride[3] = {u4SrcWidth, 0, 0};
    MUINT32 u4InStride[3] = {u4SrcWidth, u4SrcWidth, u4SrcWidth};
    ImgBufInfo rSrcImgInfo(ImgInfo(eImgFmt_YUY2, u4SrcWidth, u4SrcHeight),
                           BufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID), u4InStride);
#endif

	saveBufToFile("/sdcard/input.yuy2.yuv", (MUINT8 *)rInMem.virtAddr, rInMem.size);
	HdrShot::CDPResize_simple(
			&rInMem, 1280, 960, eImgFmt_YUY2,
			&rOutMem, 160, 120, eImgFmt_YUY2, 0);
	saveBufToFile("/sdcard/output.yuy2.yuv", (MUINT8 *)rOutMem.virtAddr, rOutMem.size);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************

    //
    //deallocMem(rInMem);
    //deallocMem(rOutMem);
    if (pIMemDrv->unmapPhyAddr(&rInMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rInMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
    if (pIMemDrv->unmapPhyAddr(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }


    //
    pIMemDrv->uninit();
    pIMemDrv->destroyInstance();
    return 0;
}


static MUINT32 u4NV212YUY2_Cmd(int argc, char** argv)
{
	MUINT32 u4SrcWidth = 1592;
	MUINT32 u4SrcHeight = 1200;
	MUINT32 u4TargetWidth = 1600;
	MUINT32 u4TargetHeight = 1200;
	int srcFmt = 0;
	char filename[128];
	sprintf(filename, "/sdcard/output/0001_8_HdrResult_1592x1200_r1.nv21");

    printf("SImager Test \n");
    //
    IMemDrv *pIMemDrv =  IMemDrv::createInstance();
    pIMemDrv->init();
    if (NULL == pIMemDrv)
    {
        printf("g_pIMemDrv is NULL");
        return 0;
    }
    //
    IMEM_BUF_INFO rInMem;
    rInMem.size = (u4SrcWidth * u4SrcHeight * 3/2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rInMem)) {
        printf("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rInMem.virtAddr, 0 , rInMem.size);
    if (pIMemDrv->mapPhyAddr(&rInMem)) {
        printf("mpIMemDrv->mapPhyAddr() error");
    }
#endif

    if(!loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), u4SrcWidth * u4SrcHeight * 2)) {
	    printf("can't load image:%s\n", filename);
		return 0;
	}
    printf("load image:%s\n", filename);

    //
	IMEM_BUF_INFO rOutMem;
    rOutMem.size = (u4TargetWidth * u4TargetHeight * 2 + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rOutMem)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rOutMem.virtAddr, 0 , rOutMem.size);
    if (pIMemDrv->mapPhyAddr(&rOutMem)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
#endif
#if 0
    //
    MUINT32 u4InStride[3] = {u4SrcWidth, u4SrcWidth, u4SrcWidth};
    ImgBufInfo rSrcImgInfo(ImgInfo(eImgFmt_YUY2, u4SrcWidth, u4SrcHeight),
                           BufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID), u4InStride);
#endif

	saveBufToFile("/sdcard/input_1592x1200.nv21", (MUINT8 *)rInMem.virtAddr, rInMem.size);
	HdrShot::CDPResize_simple(
			&rInMem, u4SrcWidth, u4SrcHeight, eImgFmt_NV21,
			&rOutMem, u4TargetWidth, u4TargetHeight, eImgFmt_YUY2, 0);
	saveBufToFile("/sdcard/output_1600x1200.yuy2", (MUINT8 *)rOutMem.virtAddr, rOutMem.size);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************

    //
    //deallocMem(rInMem);
    //deallocMem(rOutMem);
    if (pIMemDrv->unmapPhyAddr(&rInMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rInMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
    if (pIMemDrv->unmapPhyAddr(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }


    //
    pIMemDrv->uninit();
    pIMemDrv->destroyInstance();
    return 0;
}


static MUINT32 u4NV212Y800_Cmd(int argc, char** argv)
{
	MUINT32 u4SrcWidth = 1592;
	MUINT32 u4SrcHeight = 1200;
	MUINT32 u4TargetWidth = 1600;
	MUINT32 u4TargetHeight = 1200;
	int srcFmt = 0;
	char filename[128];
	sprintf(filename, "/sdcard/output/0001_8_HdrResult_1592x1200_r1.nv21");

    printf("SImager Test \n");
    //
    IMemDrv *pIMemDrv =  IMemDrv::createInstance();
    pIMemDrv->init();
    if (NULL == pIMemDrv)
    {
        printf("g_pIMemDrv is NULL");
        return 0;
    }
    //
    IMEM_BUF_INFO rInMem;
    rInMem.size = (u4SrcWidth * u4SrcHeight * 3/2 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rInMem)) {
        printf("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rInMem.virtAddr, 0 , rInMem.size);
    if (pIMemDrv->mapPhyAddr(&rInMem)) {
        printf("mpIMemDrv->mapPhyAddr() error");
    }
#endif

    if(!loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), u4SrcWidth * u4SrcHeight * 2)) {
	    printf("can't load image:%s\n", filename);
		return 0;
	}
    printf("load image:%s\n", filename);

    //
	IMEM_BUF_INFO rOutMem;
    rOutMem.size = (u4TargetWidth * u4TargetHeight + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rOutMem)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    //memset((void*)rOutMem.virtAddr, 0 , rOutMem.size);
    memset((void*)rOutMem.virtAddr, 128, rOutMem.size);
    if (pIMemDrv->mapPhyAddr(&rOutMem)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
#endif
#if 0
    //
    MUINT32 u4InStride[3] = {u4SrcWidth, u4SrcWidth, u4SrcWidth};
    ImgBufInfo rSrcImgInfo(ImgInfo(eImgFmt_YUY2, u4SrcWidth, u4SrcHeight),
                           BufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID), u4InStride);
#endif
	saveBufToFile("/sdcard/input_1592x1200.nv21", (MUINT8 *)rInMem.virtAddr, rInMem.size);
	HdrShot::CDPResize_simple(
			&rInMem, u4SrcWidth, u4SrcHeight, eImgFmt_NV21,
			&rOutMem, u4TargetWidth, u4TargetHeight, eImgFmt_Y800, 0);
	saveBufToFile("/sdcard/output_1600x1200.y", (MUINT8 *)rOutMem.virtAddr, rOutMem.size);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************

    //
    //deallocMem(rInMem);
    //deallocMem(rOutMem);
    if (pIMemDrv->unmapPhyAddr(&rInMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rInMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
    if (pIMemDrv->unmapPhyAddr(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }


    //
    pIMemDrv->uninit();
    pIMemDrv->destroyInstance();
    return 0;
}


static MUINT32 u4Y8002Y800_Cmd(int argc, char** argv)
{
	MUINT32 u4SrcWidth = 1600;
	MUINT32 u4SrcHeight = 1200;
	MUINT32 u4TargetWidth = 160;
	MUINT32 u4TargetHeight = 120;
	int srcFmt = 0;
	char filename[128];
	sprintf(filename, "/sdcard/hdr_sample_1600x1200_%d.y", 1);


    printf("SImager Test \n");
    //
    IMemDrv *pIMemDrv =  IMemDrv::createInstance();
    pIMemDrv->init();
    if (NULL == pIMemDrv)
    {
        printf("g_pIMemDrv is NULL");
        return 0;
    }
    //
    IMEM_BUF_INFO rInMem;
    rInMem.size = (u4SrcWidth * u4SrcHeight + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rInMem)) {
        printf("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rInMem.virtAddr, 0 , rInMem.size);
    if (pIMemDrv->mapPhyAddr(&rInMem)) {
        printf("mpIMemDrv->mapPhyAddr() error");
    }
#endif

    if(!loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), u4SrcWidth * u4SrcHeight)) {
	    printf("can't load image:%s\n", filename);
		return 0;
	}
    printf("load image:%s\n", filename);

    //
	IMEM_BUF_INFO rOutMem;
    rOutMem.size = (u4TargetWidth * u4TargetHeight + L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rOutMem)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    //memset((void*)rOutMem.virtAddr, 0 , rOutMem.size);
    memset((void*)rOutMem.virtAddr, 128, rOutMem.size);
    if (pIMemDrv->mapPhyAddr(&rOutMem)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
#endif
#if 0
    //
    MUINT32 u4InStride[3] = {u4SrcWidth, u4SrcWidth, u4SrcWidth};
    ImgBufInfo rSrcImgInfo(ImgInfo(eImgFmt_YUY2, u4SrcWidth, u4SrcHeight),
                           BufInfo(rInMem.size, rInMem.virtAddr, rInMem.phyAddr, rInMem.memID), u4InStride);
#endif
	saveBufToFile("/sdcard/0001_input_1600x1200.y", (MUINT8 *)rInMem.virtAddr, rInMem.size);
	HdrShot::CDPResize_simple(
			&rInMem, u4SrcWidth, u4SrcHeight, eImgFmt_Y800,
			&rOutMem, u4TargetWidth, u4TargetHeight, eImgFmt_Y800, 0);
	saveBufToFile("/sdcard/0001_output_160x120.y", (MUINT8 *)rOutMem.virtAddr, rOutMem.size);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************

    //
    //deallocMem(rInMem);
    //deallocMem(rOutMem);
    if (pIMemDrv->unmapPhyAddr(&rInMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rInMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
    if (pIMemDrv->unmapPhyAddr(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }


    //
    pIMemDrv->uninit();
    pIMemDrv->destroyInstance();
    return 0;
}


static MUINT32 u4SImager_Cmd(int argc, char** argv)
{
	MUINT32 u4SrcWidth = 1600;
	MUINT32 u4SrcHeight = 1200;
	MUINT32 u4TargetWidth = 800;
	MUINT32 u4TargetHeight = 600;
	int srcFmt = 0;
	int dstFmt = 0;


	char *filename = argv[0];
	u4SrcWidth = atoi(argv[1]);
	u4SrcHeight = atoi(argv[2]);
	srcFmt = atoi(argv[3]);
	u4TargetWidth = atoi(argv[4]);
	u4TargetHeight = atoi(argv[5]);
	dstFmt = atoi(argv[6]);

	printf("filename = %s\n", filename);
	printf("size in(%d,%d), out(%d,%d)\n", u4SrcWidth, u4SrcHeight, u4TargetWidth, u4TargetHeight);
	printf("fmt in=%x, out=%x\n", srcFmt, dstFmt);

    printf("SImager Test\n");
    //
    IMemDrv *pIMemDrv =  IMemDrv::createInstance();
    pIMemDrv->init();
    if (NULL == pIMemDrv)
    {
        printf("g_pIMemDrv is NULL");
        return 0;
    }
    //
    IMEM_BUF_INFO rInMem;
    rInMem.size = (u4SrcWidth * u4SrcHeight * 2+ L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rInMem)) {
        printf("g_pIMemDrv->allocVirtBuf() error");
    }
#if WORKAROUND_IMEM
    memset((void*)rInMem.virtAddr, 0 , rInMem.size);
    if (pIMemDrv->mapPhyAddr(&rInMem)) {
        printf("mpIMemDrv->mapPhyAddr() error");
    }
#endif

    if(!loadFileToBuf(filename, reinterpret_cast<MUINT8*>(rInMem.virtAddr), u4SrcWidth * u4SrcHeight)) {
	    printf("can't load image:%s\n", filename);
		return 0;
	}
    printf("load image:%s\n", filename);

    //
	IMEM_BUF_INFO rOutMem;
    rOutMem.size = (u4TargetWidth * u4TargetHeight * 2+ L1_CACHE_BYTES -1) & ~(L1_CACHE_BYTES-1);
    if (pIMemDrv->allocVirtBuf(&rOutMem)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)rOutMem.virtAddr, 128, rOutMem.size);
    if (pIMemDrv->mapPhyAddr(&rOutMem)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
	HdrShot::CDPResize_simple(
			&rInMem, u4SrcWidth, u4SrcHeight, g_eImgFmt[srcFmt],
			&rOutMem, u4TargetWidth, u4TargetHeight, g_eImgFmt[dstFmt], 0);

	char suffix[8];
	char prefix[8];
	switch(g_eImgFmt[srcFmt]) {
		case eImgFmt_YUY2: sprintf(prefix, "%s", "yuy2"); break;
		case eImgFmt_NV21: sprintf(prefix, "%s", "nv21"); break;
		case eImgFmt_I420: sprintf(prefix, "%s", "i420"); break;
		case eImgFmt_YV16: sprintf(prefix, "%s", "yv16"); break;
		case eImgFmt_JPEG: sprintf(prefix, "%s", "jpg"); break;
		case eImgFmt_YV12: sprintf(prefix, "%s", "yv12"); break;
	}
	switch(g_eImgFmt[dstFmt]) {
		case eImgFmt_YUY2: sprintf(suffix, "%s", "yuy2"); break;
		case eImgFmt_NV21: sprintf(suffix, "%s", "nv21"); break;
		case eImgFmt_I420: sprintf(suffix, "%s", "i420"); break;
		case eImgFmt_YV16: sprintf(suffix, "%s", "yv16"); break;
		case eImgFmt_JPEG: sprintf(suffix, "%s", "jpg"); break;
		case eImgFmt_YV12: sprintf(suffix, "%s", "yv12"); break;
	}
	char outfile[128];
	sprintf(outfile, "/sdcard/0001_%s_%dx%d.%s", prefix, u4TargetWidth, u4TargetHeight, suffix);
	saveBufToFile(outfile, (MUINT8 *)rOutMem.virtAddr, rOutMem.size);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************

    //
    //deallocMem(rInMem);
    //deallocMem(rOutMem);
    if (pIMemDrv->unmapPhyAddr(&rInMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rInMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
    if (pIMemDrv->unmapPhyAddr(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }
    if (pIMemDrv->freeVirtBuf(&rOutMem)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }


    //
    pIMemDrv->uninit();
    pIMemDrv->destroyInstance();
    return 0;
}


#if 1
static MUINT32 u4X3_Cmd(int argc, char** argv)
{
	//1. allocate buffer
	printf("allocate buffer\n");
	IMemDrv *mpIMemDrv = IMemDrv::createInstance();
	mpIMemDrv->init();	//check this, see fd

	IMEM_BUF_INFO   mpSourceImgBuf[3];
	IMEM_BUF_INFO   mpTargetImgBuf[3];

	for(int i=0; i<3; i++) {
		//2. source image
		printf("source image\n");
		mpSourceImgBuf[i].size = 794 * 600 * 1.5;
		if(mpIMemDrv->allocVirtBuf(&mpSourceImgBuf[i]))
			return MFALSE;
		uint32_t nReadSize = loadFileToBuf("/sdcard/source794x600.nv21.yuv", (uint8_t*)mpSourceImgBuf[i].virtAddr, mpSourceImgBuf[i].size);

		//3. target image
		printf("target image\n");
		mpTargetImgBuf[i].size = 20 * 16 * 1.5;
		if(mpIMemDrv->allocVirtBuf(&mpTargetImgBuf[i]))
			return MFALSE;

		//4. resize
		printf("resize\n");
		HdrShot::CDPResize_simple(
				&mpSourceImgBuf[i], 794, 600, eImgFmt_NV21,
				&mpTargetImgBuf[i], 20, 16, eImgFmt_NV21, 0);
		char outfile[128];
		sprintf(outfile, "/sdcard/target[%i].nv21.yuv.pgm", i);
		saveBufToFile(outfile, (MUINT8 *)mpTargetImgBuf[i].virtAddr, mpTargetImgBuf[i].size);
	}


	//5. final
	printf("final\n");
    return 0;
}
#endif
static MUINT32 u4CDPX3_Cmd(int argc, char** argv)
{
	MUINT32 ret = 0;
	MUINT32 mu4RunningNumber = 1;
	MUINT32 u4OutputFrameNum = 3;

	MUINT32 weight_table_width = 794;
	MUINT32 weight_table_height = 600;

	MUINT32 mu4W_dsmap = 20;
	MUINT32 mu4H_dsmap = 16;
	MUINT32 mu4DownSizedWeightMapSize = mu4W_dsmap * mu4H_dsmap * 1.5;

	IMEM_BUF_INFO   mpWeightMapBuf[3];
	IMEM_BUF_INFO   mpDownSizedWeightMapBuf[3];

	IMemDrv *mpIMemDrv = IMemDrv::createInstance();
	mpIMemDrv->init();	//check this, see fd

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		//alloc mpWeightMapBuf
		mpWeightMapBuf[i].size = weight_table_width * weight_table_height * 1.5;
		mpIMemDrv->allocVirtBuf(&mpWeightMapBuf[i]);
		#if WORKAROUND_IMEM
		if (mpIMemDrv->mapPhyAddr(&mpWeightMapBuf[i])) {
	        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    	}
		#endif

		loadFileToBuf("/sdcard/source794x600.nv21.yuv", (uint8_t*)mpWeightMapBuf[i].virtAddr, mpWeightMapBuf[i].size);

		MY_DBG("[do_DownScaleWeightMap] CDPResize %d/%d", i, u4OutputFrameNum);

		//alloc mpDownSizedWeightMapBuf
		mpDownSizedWeightMapBuf[i].size = mu4W_dsmap*mu4H_dsmap*1.5;
		mpIMemDrv->allocVirtBuf(&mpDownSizedWeightMapBuf[i]);
		#if WORKAROUND_IMEM
		if (mpIMemDrv->mapPhyAddr(&mpDownSizedWeightMapBuf[i])) {
	        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    	}
		#endif

		//set default value
		::memset((void*)mpDownSizedWeightMapBuf[i].virtAddr, 128, mu4W_dsmap*mu4H_dsmap*1.5);

		ret = HdrShot::CDPResize_simple(
				&mpWeightMapBuf[i], weight_table_width, weight_table_height, eImgFmt_NV21,
				&mpDownSizedWeightMapBuf[i], mu4W_dsmap, mu4H_dsmap, eImgFmt_NV21, 0);
		#if WORKAROUND_IMEM
	    if (mpIMemDrv->unmapPhyAddr(&mpWeightMapBuf[i])) {
	        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
	    }
		#endif
		mpIMemDrv->freeVirtBuf(&mpWeightMapBuf[i]);	//????
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_6_mpDownSizedWeightMapBuf[%d]_%dx%d.raw", mu4RunningNumber, i, mu4W_dsmap, mu4H_dsmap);
		saveBufToFile(szFileName, (MUINT8*)mpDownSizedWeightMapBuf[i].virtAddr, mu4DownSizedWeightMapSize);
		MY_VERB("[do_DownScaleWeightMap] Save %s done.", szFileName);
		#if WORKAROUND_IMEM
	    if (mpIMemDrv->unmapPhyAddr(&mpDownSizedWeightMapBuf[i])) {
	        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
	    }
		#endif
		mpIMemDrv->freeVirtBuf(&mpDownSizedWeightMapBuf[i]);
	}

	printf("[do_DownScaleWeightMap] - t7.");
	printf("[do_DownScaleWeightMap] - X. ret: %d.", ret);

	mpIMemDrv->destroyInstance();

	return	ret;
}


static MUINT32 u4CDPUp_Cmd(int argc, char** argv)
{
	MUINT32 ret = 0;
	MUINT32 mu4RunningNumber = 1;
	MUINT32 u4OutputFrameNum = 3;

	MUINT32 weight_table_width = 794;
	MUINT32 weight_table_height = 600;

	MUINT32 mu4W_dsmap = 1280;
	MUINT32 mu4H_dsmap = 960;
	MUINT32 mu4DownSizedWeightMapSize = mu4W_dsmap * mu4H_dsmap * 1.5;

	IMEM_BUF_INFO   mpWeightMapBuf[3];
	IMEM_BUF_INFO   mpDownSizedWeightMapBuf[3];

	IMemDrv *mpIMemDrv = IMemDrv::createInstance();
	mpIMemDrv->init();	//check this, see fd

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		//alloc mpWeightMapBuf
		mpWeightMapBuf[i].size = weight_table_width * weight_table_height * 1.5;
		mpIMemDrv->allocVirtBuf(&mpWeightMapBuf[i]);
		#if WORKAROUND_IMEM
		if (mpIMemDrv->mapPhyAddr(&mpWeightMapBuf[i])) {
	        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    	}
		#endif

		loadFileToBuf("/sdcard/source794x600.nv21.yuv", (uint8_t*)mpWeightMapBuf[i].virtAddr, mpWeightMapBuf[i].size);

		MY_DBG("[do_DownScaleWeightMap] CDPResize %d/%d", i, u4OutputFrameNum);

		//alloc mpDownSizedWeightMapBuf
		mpDownSizedWeightMapBuf[i].size = mu4W_dsmap*mu4H_dsmap*1.5;
		mpIMemDrv->allocVirtBuf(&mpDownSizedWeightMapBuf[i]);
		#if WORKAROUND_IMEM
		if (mpIMemDrv->mapPhyAddr(&mpDownSizedWeightMapBuf[i])) {
	        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    	}
		#endif

		//set default value
		::memset((void*)mpDownSizedWeightMapBuf[i].virtAddr, 128, mu4W_dsmap*mu4H_dsmap*1.5);

		ret = HdrShot::CDPResize_simple(
				&mpWeightMapBuf[i], weight_table_width, weight_table_height, eImgFmt_NV21,
				&mpDownSizedWeightMapBuf[i], mu4W_dsmap, mu4H_dsmap, eImgFmt_NV21, 0);
		#if WORKAROUND_IMEM
	    if (mpIMemDrv->unmapPhyAddr(&mpWeightMapBuf[i])) {
	        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
	    }
		#endif
		mpIMemDrv->freeVirtBuf(&mpWeightMapBuf[i]);	//????
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_6_mpDownSizedWeightMapBuf[%d]_%dx%d.raw", mu4RunningNumber, i, mu4W_dsmap, mu4H_dsmap);
		saveBufToFile(szFileName, (MUINT8*)mpDownSizedWeightMapBuf[i].virtAddr, mu4DownSizedWeightMapSize);
		MY_VERB("[do_DownScaleWeightMap] Save %s done.", szFileName);
		#if WORKAROUND_IMEM
	    if (mpIMemDrv->unmapPhyAddr(&mpDownSizedWeightMapBuf[i])) {
	        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
	    }
		#endif
		mpIMemDrv->freeVirtBuf(&mpDownSizedWeightMapBuf[i]);
	}

	printf("[do_DownScaleWeightMap] - t8.");
	printf("[do_DownScaleWeightMap] - X. ret: %d.", ret);

	mpIMemDrv->destroyInstance();

	return	ret;
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_rTest_Cmds[] =
{
    {"hdr", "hdr", u4Capture_Cmd},
    {"cdpnv21", "cdpnv21", u4CDPNV21_Cmd},
    {"cdpyuy2", "cdpyuy2", u4CDPYUY2_Cmd},
    {"cdpx3", "cdpx3", u4CDPX3_Cmd},
    {"x3", "x3", u4X3_Cmd},
    {"up", "up", u4CDPUp_Cmd},
    {"nv212yuy2", "nv212yuy2", u4NV212YUY2_Cmd},
    {"nv212y800", "nv212y800", u4NV212Y800_Cmd},
    {"y8002y800", "y8002y800", u4Y8002Y800_Cmd},
    {"simager", "<file> <srcW> <srcH> <srcFmt> <dstW> <dstH> <dstFmt>", u4SImager_Cmd},
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
int main_hdrshot(int argc, char** argv)
{
    printf("HdrShot Test \n");

    vHelp();

    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000);
    }
    //
    return 0;
}
