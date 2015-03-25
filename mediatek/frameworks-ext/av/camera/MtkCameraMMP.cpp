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

#define LOG_TAG "MtkCameraMMP" 
//

//#undef MTK_MMPROFILE_SUPPORT
////////////////////////////////////////////////////////////////////////////////
#if defined(MTK_MMPROFILE_SUPPORT)
#if !defined(MTK_CAMERAMMP_SUPPORT)

#warning "MTK_CAMERAMMP_SUPPORT not defined"
#include <camera/MtkCameraMMP.h>

namespace CFPT {

    AutoLog::AutoLog(unsigned int event, unsigned int data1, unsigned int data2) {}
    AutoLog::~AutoLog() {}

    bool RegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt)
    { return true; }

    bool EnableEvent(unsigned int event, bool enable)
    { return true; }
    
    bool Log(unsigned int event, CPT_LogType type)
    { return true; }
    
    bool LogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2)
    { return true; }

    bool LogStr(unsigned int event, CPT_LogType type, const char* str)
    { return true; }

    bool LogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
    { return true; }

};  // namespace CFPT
#endif
#endif
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
#if defined(MTK_MMPROFILE_SUPPORT)
#if defined(MTK_CAMERAMMP_SUPPORT)

#include <utils/Log.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <linux/mmprofile.h>
#include <camera/MtkCameraMMP.h>

namespace CFPT {

typedef DefaultKeyedVector< unsigned int, MMP_Event >  MMPEventMap_t;

static MMPEventMap_t gCamMMPEventMap; 

static MMP_LogType gMMPLogType[CPTFlagMax] = {MMProfileFlagStart, MMProfileFlagEnd, MMProfileFlagPulse, MMProfileFlagEventSeparator};

static MMP_Event findMMPEvent (unsigned int cptEvent)
{
    MMP_Event mmpEvent = gCamMMPEventMap.valueFor(cptEvent); 
    return mmpEvent; 
}


/*******************************************************************************
 * 
 ******************************************************************************/
bool RegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt)
{
    ALOGD("[CPTRegisterEvents] EventCnt = %d", u4EventCnt);

    for (unsigned int  i = 0; i < u4EventCnt; i++)
    {
        MMP_Event mmpEvent = 0; 
        MMP_Event mmpParentEvent = findMMPEvent(pCPTEventInfo[i].parent); 
        if (0 == static_cast<unsigned int>(mmpParentEvent))
        {
            if (EVENT_CAMERA_ROOT == pCPTEventInfo[i].parent)
            {
                MMP_Event mmpRootEvent = MMProfileFindEvent(MMP_RootEvent, "Camera");
                if ( 0 == static_cast<unsigned int>(mmpRootEvent) )
                {
                    mmpRootEvent = MMProfileRegisterEvent(MMP_RootEvent, "Camera");
                }
                gCamMMPEventMap.add(pCPTEventInfo[i].parent, mmpRootEvent);
                mmpEvent = MMProfileRegisterEvent(mmpRootEvent, pCPTEventInfo[i].name); 
            }
            else 
            {
                ALOGW("[CPTRegisterEvents] parent event:0x%x regist by event:0x%x, name:%s not exist",
                        pCPTEventInfo[i].parent, pCPTEventInfo[i].event, pCPTEventInfo[i].name); 
                continue; 
            }
        }
        else 
        {
            if (NULL == pCPTEventInfo[i].name)  
            {
                continue; 
            }
            mmpEvent = MMProfileRegisterEvent(mmpParentEvent, pCPTEventInfo[i].name); 
        }
        
        if (mmpEvent != 0) 
        {
            gCamMMPEventMap.add(pCPTEventInfo[i].event, mmpEvent); 
            ALOGD("Event: %s is registered as id 0x%x", pCPTEventInfo[i].name, pCPTEventInfo[i].event);
        }    
        else
        {
            ALOGW("Event: %s is NOT registered. id=0x%x", pCPTEventInfo[i].name, pCPTEventInfo[i].event);
        }
    }
    return true; 
}


/*******************************************************************************
 * 
 ******************************************************************************/
bool EnableEvent(unsigned int event, bool enable)
{
    bool ret = false;
    MMP_Event mmpEvent = findMMPEvent(event);
    if (mmpEvent != 0)
    {
        MMProfileEnableEvent(mmpEvent, enable);
        ret = true;
    }
    return ret;
}


/******************************************************************************
 * 
 ******************************************************************************/
bool Log(unsigned int event, CPT_LogType type)
{    
    bool ret = false; 
    MMP_Event mmpEvent = findMMPEvent(event); 
    if (mmpEvent != 0 && type < CPTFlagMax) 
    {
        MMProfileLog(mmpEvent, gMMPLogType[type]); 
        ret = true;  
    }
    
    return ret;
}


/******************************************************************************
 * 
 ******************************************************************************/
bool LogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2)
{
    bool ret = false; 
    MMP_Event mmpEvent = findMMPEvent(event); 
    if (mmpEvent != 0 && type < CPTFlagMax) 
    {
        MMProfileLogEx(mmpEvent, gMMPLogType[type], data1, data2); 
        ret = true;  
    }
    
    return ret;
}


/******************************************************************************
 * 
 ******************************************************************************/
bool LogStr(unsigned int event, CPT_LogType type, const char* str)
{
    bool ret = false; 
    MMP_Event mmpEvent = findMMPEvent(event); 
    if (mmpEvent != 0 && type < CPTFlagMax) 
    {
        MMProfileLogMetaString(mmpEvent, gMMPLogType[type], str); 
        ret = true;  
    }
    
    return ret;
}


/******************************************************************************
 * 
 ******************************************************************************/
bool LogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
{
    bool ret = false; 
    MMP_Event mmpEvent = findMMPEvent(event); 
    if (mmpEvent != 0 && type < CPTFlagMax) 
    {
        MMProfileLogMetaStringEx(mmpEvent, gMMPLogType[type], data1, data2, str); 
        ret = true;  
    }
    
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
AutoLog::AutoLog(unsigned int event, unsigned int data1, unsigned int data2)
    :mEvent(event)
    ,mData1(data1)
    ,mData2(data2)
{
    MMP_Event mmpEvent = findMMPEvent(mEvent); 
    if (mmpEvent != 0) 
    {
        MMProfileLogEx(mmpEvent, MMProfileFlagStart, mData1, mData2);
    }  
}


/******************************************************************************
 * 
 ******************************************************************************/
AutoLog:: ~AutoLog()
{
    MMP_Event mmpEvent = findMMPEvent(mEvent); 
    if (mmpEvent != 0) 
    {
        MMProfileLogEx(mmpEvent, MMProfileFlagEnd, mData1, mData2);
    }
}

};  // namespace CFPT
#endif
#endif
////////////////////////////////////////////////////////////////////////////////

