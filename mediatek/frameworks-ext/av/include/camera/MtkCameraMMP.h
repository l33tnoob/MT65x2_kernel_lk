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

#ifndef _MTK_FRAMEWORKS_EXT_AV_INCLUDE_CAMERA_MTKCAMERAMMP_H_
#define _MTK_FRAMEWORKS_EXT_AV_INCLUDE_CAMERA_MTKCAMERAMMP_H_

////////////////////////////////////////////////////////////////////////////////
/**
 * @file MtkCameraMMP.h 
 * @brief This file is for the camera framework profile using mmp tool.
 * @detail
 */


/******************************************************************************
 *  Camera Framework Profiling Tool
 ******************************************************************************/
namespace CFPT
{


#define EVENT_CAMERA_ROOT                   0x00000000
#define EVENT_CAMERA_FRAMEWORK              0x10000000


/**
 * @brief Camera profile Event Info
 */
typedef struct
{
    unsigned int event;             /// Log event 
    unsigned int parent;            /// Log event's parent 
    const char* name;               /// Log event's name
}CPT_Event_Info;


/**
 * @brief Camera profile log type enum 
 */
typedef enum
{
    CPTFlagStart = 0,               /// Log Start 
    CPTFlagEnd,                     /// Log End 
    CPTFlagPulse,                   /// Log pulse 
    CPTFlagSeparator,               /// Log Separator                          
    CPTFlagMax                      /// Log Flag Max 
}CPT_LogType;


/**
 * @brief Interface of pipe command 
 * @details 
 * The is used for new send command style. 
 *
 */
class AutoLog
{
protected:
    unsigned int mEvent;       /// camera profile event 
    unsigned int mData1;       /// data1 
    unsigned int mData2;

public:
    AutoLog(unsigned int event, unsigned int data1 = 0, unsigned int data2 = 0);
    virtual ~AutoLog();
};


//#undef MTK_MMPROFILE_SUPPORT
#if defined(MTK_MMPROFILE_SUPPORT)

    /**
     * @brief register camera profile event in mmp profile tool 
     *
     * @details 
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] enable: enable or disable 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool RegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt);


    /**
     * @brief enable camera profile event in mmp profile tool
     *
     * @details 
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] enable: enable or disable 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool EnableEvent(unsigned int event, bool enable);
    
    
    /**
     * @brief camera profile log 
     *
     * @details      
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] type: The camera profile log type 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool Log(unsigned int event, CPT_LogType type);
    
    
    /**
     * @brief camera profile log w/ extend parameter
     *
     * @details      
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] type: The camera profile log type 
     * 
     * @param[in] data1: The extend parameter 1
     *
     * @param[in] data2: The extend parameter 2 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool LogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2);
    
    
    /**
     * @brief camera profile log w/ extend string parameter
     *
     * @details      
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] type: The camera profile log type 
     * 
     * @param[in] str: The extend string parameter 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool LogStr(unsigned int event, CPT_LogType type, const char* str);

    /**
     * @brief camera profile log w/ extend parameter
     *
     * @details      
     *
     * @note 
     *
     * @param[in] event: The camera profile event 
     *
     * @param[in] type: The camera profile log type 
     * 
     * @param[in] data1: The extend parameter 1  
     *
     * @param[in] data2: The extend parameter 2
     *
     * @param[in] str: The extend string parameter 
     *
     * @return
     * - MTRUE indicates success. 
     * - MFALSE indicates failure
     *
     */
    bool LogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str);


#else   //MTK_MMPROFILE_SUPPORT

#warning "MTK_MMPROFILE_SUPPORT Not Defined"

    inline AutoLog::AutoLog(unsigned int event, unsigned int data1, unsigned int data2) {}
    inline AutoLog::~AutoLog() {}

    inline bool RegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt)
    { return true; }

    inline bool EnableEvent(unsigned int event, bool enable)
    { return true; }

    inline bool Log(unsigned int event, CPT_LogType type)
    { return true; }

    inline bool LogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2)
    { return true; }

    inline bool LogStr(unsigned int event, CPT_LogType type, const char* str)
    { return true; }

    inline bool LogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
    { return true; }

#endif  //MTK_MMPROFILE_SUPPORT

/******************************************************************************
 *
 ******************************************************************************/
};  // namespace CFPT
#endif

