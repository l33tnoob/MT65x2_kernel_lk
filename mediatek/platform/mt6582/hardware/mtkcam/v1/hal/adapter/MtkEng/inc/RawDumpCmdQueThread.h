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

#ifndef RAW_DUMP_CMDQ_THREAD_H
#define RAW_DUMP_CMDQ_THREAD_H
//
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <semaphore.h>
#include <inc/IPreviewBufMgr.h>
//


namespace android {
namespace NSMtkEngCamAdapter {

#define BUFCNT      (75)

/******************************************************************************
*
*******************************************************************************/
class RawDumpCmdCookie : public virtual RefBase
{
public:
    RawDumpCmdCookie(MUINT32 frame_count, MUINT32 slot_index, MUINT32 buf_size)
        : mFrameCnt (frame_count)
        , mSlotIndex(slot_index)
        , mBufSize(buf_size)        
    {
    }

    MUINT32 getSlotIndex() const { return mSlotIndex; }
    MUINT32 getFrameCnt() const { return mFrameCnt; }
    MUINT32 getBufSize() const { return mBufSize; }
private:
    MUINT32 mFrameCnt;
    MUINT32 mSlotIndex;
    MUINT32 mBufSize;
};


/******************************************************************************
*
*******************************************************************************/
class IRawDumpCmdQueThread : public Thread 
{
public:
    static IRawDumpCmdQueThread*    createInstance(MUINT32 mem_out_width, MUINT32 mem_out_height, MUINT32 bitOrder, MUINT32 bitDepth, sp<IParamsManager> pParamsMgr);
public:
    virtual int  getTid() const  = 0;
    virtual bool postCommand(MUINT32 buf_addr, MUINT32 buf_size)= 0;
    virtual void setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo) = 0;
};

}; // namespace NSMtkEngCamAdapter
}; // end of namespace
#endif

