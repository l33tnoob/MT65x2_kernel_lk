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

#define LOG_TAG "MtkCam/Shot"
//
#include <utils/Timers.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/v1/camutils/CamMisc.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "TestShot.h"
//
#include "TestPostview.h"
#include "TestJpeg.h"
//
using namespace android;
using namespace NSShot;
using namespace NSTestShot;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] "fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
sp<IShot>
createInstance_TestShot(
    char const*const    pszShotName, 
    uint32_t const      u4ShotMode, 
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<MyShot>      pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new MyShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new MyShot <%s>", __FUNCTION__, pszShotName);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
MyShot::
onCreate()
{
    MY_LOGD("");
    return  true;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
MyShot::
onDestroy()
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
MyShot::
MyShot(
    char const*const pszShotName, 
    uint32_t const u4ShotMode, 
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    , mi4IsCancel(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MyShot::
~MyShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
MyShot::
sendCommand(
    uint32_t const  cmd, 
    uint32_t const  arg1, 
    uint32_t const  arg2
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset, 
    //  performing a new capture should work well, no matter whether previous 
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
MyShot::
onCmd_reset()
{
    MY_LOGD("");
    ::android_atomic_release_store(0, &mi4IsCancel);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
#include <stdio.h>
bool
MyShot::
onCmd_capture()
{
    uint32_t u4ShotCount = mShotParam.mu4ShotCount;
    MY_LOGD("+ ShotCount:%u", u4ShotCount);
    //
    for (size_t i = 0; i < u4ShotCount; i++)
    {
        MY_LOGD("[%d] sleep to simulate the shutter", i);
        ::usleep(200000);
        mpShotCallback->onCB_Shutter(true, 0);

        //  To simulate the H/W timestamp.
        nsecs_t const nsHWTimestamp = systemTime();

        MY_LOGD("[%d] usleep to simulate the postview", i);
        ::usleep(200000);
        mpShotCallback->onCB_PostviewDisplay(
            nsHWTimestamp, 
            sizeof(TestPostview_640x480), 
            TestPostview_640x480
        );
    
        MY_LOGD("[%d] usleep to simulate the jpeg encoding", i);
        ::usleep(200000);

        //  It's the final image if count-up end or cancel.
        bool fgIsFinalImage = (u4ShotCount==i+1) || mi4IsCancel;

        mpShotCallback->onCB_CompressedImage(
            nsHWTimestamp, 
            sizeof(TestJpegBitstream), 
            TestJpegBitstream, 
            sizeof(TestJpegHeader), 
            TestJpegHeader, 
            i, 
            fgIsFinalImage
        );

        if  ( 1 == mi4IsCancel )
        {
            MY_LOGD("[%d] break due to cancel", i);
            break;
        }
        ::sleep(1);
    }

/*
    uint8_t buf[640*480*3/2] = {0};
    MtkCamUtils::loadFileToBuf("data/testshot_postview_640x480.yuv", buf, sizeof(buf));

    FILE* f = NULL;
    f = fopen("sdcard/DCIM/Camera/testshot_postview_640x480.bin", "w");
    for (int i = 0; i < sizeof(bufQv); i++)
    {
        if  ( i%16 == 0) {
            fprintf(f, "\r\n");
        }
        fprintf(f, "0x%02x, ", buf[i]);
    }

    if  ( f ) {
        fclose(f);
    }
*/

    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
MyShot::
onCmd_cancel()
{
    MY_LOGD("");
    ::android_atomic_release_store(1, &mi4IsCancel);
}

