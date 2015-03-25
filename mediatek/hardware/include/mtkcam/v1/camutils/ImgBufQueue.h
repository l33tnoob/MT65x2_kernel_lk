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

#ifndef _MTK_CAMERA_INC_COMMON_CAMUTILS_IMGBUFQUEUE_H_
#define _MTK_CAMERA_INC_COMMON_CAMUTILS_IMGBUFQUEUE_H_
//
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Vector.h>


/******************************************************************************
*
*******************************************************************************/


namespace android {
namespace MtkCamUtils {
/******************************************************************************
*
*******************************************************************************/


/******************************************************************************
*   Image Buffer Queue
*
*                   enqueProcessor --> [ ToDo Queue ] --> dequeProvider
*               -->                                                     -->
*   [Provider]                                                              [Processor]
*               <--                                                     <--
*                   dequeProcessor <-- [ Done Queue ] <-- enqueProvider
*
*******************************************************************************/
class ImgBufQueue : public IImgBufQueue
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            Instantiation.
                            ImgBufQueue(
                                int32_t const i4QueueId, 
                                char const*const pszQueueName
                            );

protected:  ////            Data Members.
    int32_t const           mi4QueueId;
    char const*const        mpszQueueName;
    char const*             getQueName() const      { return mpszQueueName; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  TODO Queue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    List<ImgBufQueNode>     mTodoImgBufQue;
    mutable Mutex           mTodoImgBufQueMtx;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DONE Queue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    Vector<ImgBufQueNode>   mDoneImgBufQue;
    mutable Mutex           mDoneImgBufQueMtx;
    Condition               mDoneImgBufQueCond; //  Condition to wait: [ mDoneImgBufQue.empty() && mbIsProcessorRunning ]

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBufProvider.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            Interface.
    virtual int32_t         getProviderId() const   { return mi4QueueId; }
    virtual char const*     getProviderName() const { return mpszQueueName; }

    /*
     *  REPEAT:[ dequeProvider() -> enqueProvider() ]
     *  dequeProvider() returns false immediately if empty.
     */
    virtual bool            dequeProvider(ImgBufQueNode& rNode);
    virtual bool            enqueProvider(ImgBufQueNode const& rNode);

    /*
     *  Arguments:
     *      rNode
     *          [I] If this function returns true, rNode is a copy of the first
     *          node in th queue. Unlike dequeProvider(), the first node is not
     *          removed from th equeue.
     *  Return:
     *      false if the queue is empty.
     *      true if the queue is not empty.
     */
    virtual bool            queryProvider(ImgBufQueNode& rNode);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IImgBufProcessor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            Interface.
    /*
     *  Usage:
     *         enqueProcessor()                     <============> if needed
     *      -> startProcessor()
     *      -> REPEAT:[ enqueProcessor() -> dequeProcessor() ]
     *      -> pauseProcessor() -> flushProcessor() <============> if needed
     *      -> stopProcessor()
     *  
     *  dequeProcessor() will block until:
     *  (1) queue is not empty, (2) pauseProcessor(), or (3) stopProcessor().
     */
    virtual bool            enqueProcessor(ImgBufQueNode const& rNode);
    virtual bool            dequeProcessor(Vector<ImgBufQueNode>& rvNode);
    //
    virtual bool            startProcessor();
    virtual bool            pauseProcessor();
    virtual bool            stopProcessor();
    /*
     *  Move all buffers from TODO queue to DONE queue.
     *  It returns false if Processor is Running; call pauseProcessor() firstly
     *  before flushProcessor().
     */
    virtual bool            flushProcessor();
    virtual bool            isProcessorRunning();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////            Data Members.
    bool volatile           mbIsProcessorRunning;

};


/******************************************************************************
*
*******************************************************************************/
};  // namespace MtkCamUtils
};  // namespace android
#endif  //_MTK_CAMERA_INC_COMMON_CAMUTILS_IMGBUFQUEUE_H_

