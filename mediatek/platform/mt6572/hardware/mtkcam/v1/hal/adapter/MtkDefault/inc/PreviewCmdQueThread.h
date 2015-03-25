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

#ifndef PREVIEW_CMDQ_THREAD_H
#define PREVIEW_CMDQ_THREAD_H
//
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <semaphore.h>
#include <inc/IPreviewBufMgr.h>
//
namespace android {
namespace NSMtkDefaultCamAdapter {
/******************************************************************************
*
*******************************************************************************/
class PrvCmdCookie : public virtual RefBase
{

public:
    enum ECmdType{
         eStart,               //prvStart()
         eDelay,               //prvDelay()
         eUpdate,              //prvUpdate()
         ePrecap,              //prvPrecap()
         eStop,                //prvStop()
         eExit,                
    };
    ///
    enum ESemWait{
        eSemNone    = 0,
        eSemBefore  = 0x01,
        eSemAfter   = 0x02,
    };
    ///
    
public:
    PrvCmdCookie(ECmdType _eType, ESemWait _eSem)
        : eType(_eType)
        , bsemBefore(false)
        , bsemAfter(false)
        , bvalid(true)
    {
        if (_eSem & eSemBefore)
        {
            bsemBefore = true;
            sem_init(&semBefore, 0, 0); 
        }
        if (_eSem & eSemAfter)
        {
            bsemAfter = true;
            sem_init(&semAfter, 0, 0);  
        } 
    }
    
    void waitSem(){
        if (isSemBefore()){
            sem_wait(&semBefore); 
        }
        if (isSemAfter()){
            sem_wait(&semAfter);
        }
    }
    
    void postSem(ESemWait _eSem){
        if ((_eSem & eSemBefore) && isSemBefore()) 
        {
            sem_post(&semBefore); 
        }
        if ((_eSem & eSemAfter) && isSemAfter())
        {
            sem_post(&semAfter); 
        }
    }

    ECmdType getCmd() const { return eType; }
    bool isValid() const { return bvalid;}
    void setValid(bool in) { bvalid = in;}
/////
private:
    bool isSemBefore() const {return bsemBefore;}
    bool isSemAfter()  const {return bsemAfter;}
    

private:
    ECmdType eType;
    sem_t semBefore;
    sem_t semAfter;
    bool  bsemBefore;
    bool  bsemAfter;
    bool  bvalid;
};


/******************************************************************************
*
*******************************************************************************/
class IPreviewCmdQueCallBack 
{

public:

    //  Notify Callback of Zoom
    //  Arguments:
    //
    //      _msgType: 
    //       
    //
    //      _ext1:

    virtual void        doNotifyCb (
                            int32_t _msgType, 
                            int32_t _ext1, 
                            int32_t _ext2,
                            int32_t _ext3
                        ) = 0;
    
    virtual             ~IPreviewCmdQueCallBack(){}
    
    enum ECallBack_T
    {
        eID_NOTIFY_Zoom,
    };
    
};


/******************************************************************************
*
*******************************************************************************/
class IPreviewCmdQueThread : public Thread 
{
public:
    static IPreviewCmdQueThread*    createInstance(
                                        sp<IPreviewBufMgrHandler> pHandler, 
                                        int const& rSensorid,
                                        sp<IParamsManager> pParamsMgr
                                    );
public:
    virtual int  getTid() const  = 0;
    virtual bool postCommand(PrvCmdCookie::ECmdType const rcmdType, 
                             PrvCmdCookie::ESemWait const rSemWait = PrvCmdCookie::eSemNone)= 0;
    virtual bool setParameters() = 0;
    virtual void startRecording() = 0;
    virtual void stopRecording() = 0;
    virtual void pushZoom(uint32_t zoomIdx) = 0;
    virtual int  popZoom() = 0;
    virtual void setZoomCallback(IPreviewCmdQueCallBack *pZoomCb) = 0;
};

}; // namespace NSMtkDefaultCamAdapter
}; // end of namespace
#endif

