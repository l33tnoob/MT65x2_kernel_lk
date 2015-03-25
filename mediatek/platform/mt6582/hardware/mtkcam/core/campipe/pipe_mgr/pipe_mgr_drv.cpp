/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "PipeMgrDrv"
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <camera_pipe_mgr.h>
#include <cutils/xlog.h>
#include <utils/threads.h>
#include "mtkcam/common.h"
#include <mtkcam/campipe/pipe_mgr_drv.h>
#include "pipe_mgr_drv_imp.h"
//-----------------------------------------------------------------------------
PipeMgrDrvImp::PipeMgrDrvImp()
{
    //LOG_MSG("");
    mUser = 0;
    mFd = -1;
    mLogMask = (    PIPE_MGR_DRV_PIPE_MASK_CAM_IO|
                    PIPE_MGR_DRV_PIPE_MASK_POST_PROC|
                    PIPE_MGR_DRV_PIPE_MASK_XDP_CAM);
}
//----------------------------------------------------------------------------
PipeMgrDrvImp::~PipeMgrDrvImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
PipeMgrDrv* PipeMgrDrv::CreateInstance(void)
{
    return PipeMgrDrvImp::GetInstance();
}
//-----------------------------------------------------------------------------
PipeMgrDrv* PipeMgrDrvImp::GetInstance(void)
{
    static PipeMgrDrvImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
MVOID PipeMgrDrvImp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MBOOL PipeMgrDrvImp::Init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        LOG_MSG("First user(%d)",mUser);
    }
    else
    {
        LOG_MSG("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //
    if(mFd < 0)
    {
        mFd = open(PIPE_MGR_DRV_DEVNAME, O_RDONLY, 0);
        if(mFd < 0)
        {
            LOG_ERR("CamPipeMgr kernel open fail, errno(%d):%s",errno,strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
    }
    else
    {
        LOG_MSG("CamPipeMgr kernel is opened already");
    }
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL PipeMgrDrvImp::Uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        LOG_WRN("No user(%d)",mUser);
        goto EXIT;
    }
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        LOG_MSG("Last user(%d)",mUser);
    }
    else
    {
        LOG_MSG("More user(%d)",mUser);
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        close(mFd);
        mFd = -1;
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL PipeMgrDrvImp::Lock(PIPE_MGR_DRV_LOCK_STRUCT* pLock)
{
    MBOOL Result = MTRUE;
    CAM_PIPE_MGR_LOCK_STRUCT CamPipeMgrLock;
    /*
    LOG_MSG("PipeMask(0x%08X),Timeount(%d)",
            pLock->PipeMask,
            pLock->Timeout);
    */
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        CamPipeMgrLock.PipeMask = pLock->PipeMask;
        CamPipeMgrLock.Timeout = pLock->Timeout;
        if(ioctl(mFd, CAM_PIPE_MGR_LOCK, &CamPipeMgrLock) == 0)
        {
            if(mLogMask & pLock->PipeMask)
            {
                LOG_MSG("OK,PipeMask(0x%08X),Timeount(%d)",
                        CamPipeMgrLock.PipeMask,
                        CamPipeMgrLock.Timeout);
            }
        }
        else
        {
            if(mLogMask & pLock->PipeMask)
            {

                LOG_ERR("Fail,PipeMask(0x%08X),Timeount(%d)",
                        CamPipeMgrLock.PipeMask,
                        CamPipeMgrLock.Timeout);
            }
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Kernel is not opened");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL PipeMgrDrvImp::Unlock(PIPE_MGR_DRV_UNLOCK_STRUCT* pUnlock)
{
    MBOOL Result = MTRUE;
    CAM_PIPE_MGR_UNLOCK_STRUCT CamPipeMgrUnlock;
    /*
    LOG_MSG("PipeMask(0x%08X),Timeount(%d)",
            pUnlock->PipeMask,
            pUnlock->Timeout);
    */
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        CamPipeMgrUnlock.PipeMask = pUnlock->PipeMask;
        if(ioctl(mFd, CAM_PIPE_MGR_UNLOCK, &CamPipeMgrUnlock) == 0)
        {
            if(mLogMask & pUnlock->PipeMask)
            {
                LOG_MSG("OK,PipeMask(0x%08X)",CamPipeMgrUnlock.PipeMask);
            }
        }
        else
        {
            if(mLogMask & pUnlock->PipeMask)
            {
                LOG_ERR("Fail,PipeMask(0x%08X)",CamPipeMgrUnlock.PipeMask);
            }
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Kernel is not opened");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL PipeMgrDrvImp::Dump(void)
{
    MBOOL Result = MTRUE;
    //
    //LOG_MSG("");
    //
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        if(ioctl(mFd, CAM_PIPE_MGR_DUMP) == 0)
        {
            LOG_MSG("OK");
        }
        else
        {
            LOG_ERR("Fail");
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Kernel is not opened");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------

