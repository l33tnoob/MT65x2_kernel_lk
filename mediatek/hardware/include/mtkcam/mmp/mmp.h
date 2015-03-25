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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_MMP_MMP_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_MMP_MMP_H_


////////////////////////////////////////////////////////////////////////////////
/**
 * @file MMP.h 
 * @brief This file is for the camera hal profile using mmp tool.
 * @detail
 */


/******************************************************************************
 *  Camera Profiling Tool
 ******************************************************************************/
namespace CPTool
{


#define EVENT_CAMERA_ROOT                   0x00000000
#define EVENT_CAMERA_FRAMEWORK              0x10000000
#define EVENT_CAMERA_COMMON                 0x20000000
#define EVENT_CAMERA_PLATFORM               0x30000000


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
 * @brief Automatically Profiling Logging
 * @details 
 * The is used for automatically profiling logging.
 *
 */
class AutoCPTLog
{
protected:
    unsigned int mEvent;       /// camera profile event 
    unsigned int mData1;       /// data1 
    unsigned int mData2;

public:
    AutoCPTLog(unsigned int event, unsigned int data1 = 0, unsigned int data2 = 0);
    virtual ~AutoCPTLog();
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
    bool CPTRegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt);

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
    bool CPTEnableEvent(unsigned int event, bool enable);

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
    bool CPTLog(unsigned int event, CPT_LogType type);

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
    bool CPTLogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2);

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
    bool CPTLogStr(unsigned int event, CPT_LogType type, const char* str);

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
    bool CPTLogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str);

#else   //MTK_MMPROFILE_SUPPORT

    inline AutoCPTLog::AutoCPTLog(unsigned int event, unsigned int data1, unsigned int data2) {}
    inline AutoCPTLog::~AutoCPTLog() {}

    inline bool CPTRegisterEvents(const CPT_Event_Info *pCPTEventInfo, const unsigned int u4EventCnt)
    { return true; }

    inline bool CPTEnableEvent(unsigned int event, bool enable)
    { return true; }

    inline bool CPTLog(unsigned int event, CPT_LogType type)
    { return true; }

    inline bool CPTLogEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2)
    { return true; }

    inline bool CPTLogStr(unsigned int event, CPT_LogType type, const char* str)
    { return true; }

    inline bool CPTLogStrEx(unsigned int event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
    { return true; }

#endif  //MTK_MMPROFILE_SUPPORT

/******************************************************************************
 *
 ******************************************************************************/
};  // namespace CPTool
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_MMP_MMP_H_

