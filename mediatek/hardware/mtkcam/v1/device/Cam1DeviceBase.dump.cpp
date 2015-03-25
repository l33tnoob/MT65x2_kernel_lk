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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include <mtkcam/device/Cam1DeviceBase.h>
//
#include <utils/threads.h>
#include <cutils/properties.h>
//
using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[BaseCam1Device::%s] "fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  Dump state of the camera hardware
 ******************************************************************************/
#define DUMPSYS_COMMAND "camera.hal.dumpsys"
static
Vector<String8>
queryCommandVector(int fd)
{
    Vector<String8> vCommand;
    char dumpsys[PROPERTY_VALUE_MAX] = {0};
    //  (1) Get command from property.
    ::property_get(DUMPSYS_COMMAND, dumpsys, "");
    CAM_LOGD(DUMPSYS_COMMAND"=%s", dumpsys);
    //
    String8 s8Buffer = String8::format(DUMPSYS_COMMAND"=%s\n", dumpsys);
    ::write(fd, s8Buffer.string(), s8Buffer.size());
    //
    //  (2) Convert command from string to vector: vCommand
    char const *s1 = dumpsys, *s2 = 0;
    while ( 0 != s1 && 0 != (*s1) ) {
        if  ( ' ' == (*s1) ) {
            continue;
        }
        //
        s2 = ::strchr(s1, ' ');
        if  ( s2 == 0 ) {
            // If there's no space, this is the last item.
            vCommand.push_back(String8(s1));
            break;
        }
        vCommand.push_back(String8(s1, (size_t)(s2-s1)));
        s1 = s2 + 1;
    }
    //
    for (size_t i = 0; i < vCommand.size(); i++)
    {
        String8 s8Temp = String8::format("[%d] %s\n", i, vCommand[i].string());
        ::write(fd, s8Temp.string(), s8Temp.size());
    }
    //
    return  vCommand;
}


status_t
Cam1DeviceBase::
dump(int fd)
{
    Vector<String8> vCommand = queryCommandVector(fd);
    if  ( vCommand.empty() )
    {
        return  OK;
    }
    ////////////////////////////////////////////////////////////////////////////
    //  Parse Command:
    ////////////////////////////////////////////////////////////////////////////
    //
    //  <ParamsMgr>
    if  ( *vCommand.begin() == "ParamsMgr" )
    {
        vCommand.erase(vCommand.begin());
        mpParamsMgr->dump(fd, vCommand);
        return  OK;
    }
    //
    //  <DisplayClient>
    if  ( *vCommand.begin() == "DisplayClient" )
    {
        if  ( mpDisplayClient != 0 ) {
            vCommand.erase(vCommand.begin());
            mpDisplayClient->dump(fd, vCommand);
        }
        return  OK;
    }
    //
    //  <CamClient>
    if  ( *vCommand.begin() == "CamClient" )
    {
        if  ( mpCamClient != 0 ) {
            vCommand.erase(vCommand.begin());
            mpCamClient->dump(fd, vCommand);
        }
        return  OK;
    }
    //
    //  <CamAdapter>
    if  ( *vCommand.begin() == "CamAdapter" )
    {
        if  ( mpCamAdapter != 0 ) {
            vCommand.erase(vCommand.begin());
            mpCamAdapter->dump(fd, vCommand);
        }
        return  OK;
    }
    //
    //
    return OK;
}

