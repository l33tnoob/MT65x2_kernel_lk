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
#define LOG_TAG "ResMgrDrv"
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
#include "bandwidth_control.h"
#include <linux/hdmitx.h>
#include "mtkcam/common.h"
#include <mtkcam/drv/res_mgr_drv.h>
#include "res_mgr_drv_imp.h"
//-----------------------------------------------------------------------------
ResMgrDrvImp::ResMgrDrvImp()
{
    //LOG_MSG("");
    mUser = 0;
    mFdCamPipeMgr = -1;
    mFdHdmiTx = -1;
}
//----------------------------------------------------------------------------
ResMgrDrvImp::~ResMgrDrvImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
ResMgrDrv* ResMgrDrv::CreateInstance(void)
{
    return ResMgrDrvImp::GetInstance();
}
//-----------------------------------------------------------------------------
ResMgrDrv* ResMgrDrvImp::GetInstance(void)
{
    static ResMgrDrvImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
MVOID ResMgrDrvImp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::Init(void)
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
    if(mFdCamPipeMgr < 0)
    {
        mFdCamPipeMgr = open(RES_MGR_DRV_DEVNAME_PIPE_MGR, O_RDONLY, 0);
        if(mFdCamPipeMgr < 0)
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
MBOOL ResMgrDrvImp::Uninit(void)
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
    if(mFdCamPipeMgr >= 0)
    {
        close(mFdCamPipeMgr);
        mFdCamPipeMgr = -1;
    }
    //
    if(mFdHdmiTx >= 0)
    {
        close(mFdHdmiTx);
        mFdHdmiTx = -1;
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::GetMode(RES_MGR_DRV_MODE_STRUCT* pMode)
{
    MBOOL Result = MTRUE;
    //
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(ioctl(mFdCamPipeMgr, CAM_PIPE_MGR_GET_MODE, (CAM_PIPE_MGR_MODE_STRUCT*)pMode) == 0)
    {
        LOG_MSG("ScenSw(%d),ScenHw(%d),Dev(%d)",
                pMode->ScenSw,
                pMode->ScenHw,
                pMode->Dev);
    }
    else
    {
        LOG_ERR("GET_MODE fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;

}
//-----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::SetMode(RES_MGR_DRV_MODE_STRUCT* pMode)
{
    MBOOL Result = MTRUE;
    RES_MGR_DRV_MODE_STRUCT     CurrMode;
    CAM_PIPE_MGR_LOCK_STRUCT    CamPipeMgrLock;
    CAM_PIPE_MGR_UNLOCK_STRUCT  CamPipeMgrUnlock;
    CAM_PIPE_MGR_DISABLE_STRUCT CamPipeMgrDisable;
    //
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFdCamPipeMgr < 0)
    {
        LOG_ERR("CamPipeMgr kernel is not opened");
        Result = MFALSE;
        goto EXIT;
    }
    //
    LOG_MSG("ScenSw(%d),ScenHw(%d),Dev(%d)",
            pMode->ScenSw,
            pMode->ScenHw,
            pMode->Dev);
    //
    GetMode(&CurrMode);
    //
    if( CurrMode.ScenSw == (pMode->ScenSw) &&
        CurrMode.ScenHw == (pMode->ScenHw) &&
        CurrMode.Dev == (pMode->Dev))
    {
        LOG_MSG("OK:Same");
        goto EXIT;
    }
    //
    if(Result)
    {
        if(ioctl(mFdCamPipeMgr, CAM_PIPE_MGR_SET_MODE, (CAM_PIPE_MGR_MODE_STRUCT*)pMode) == 0)
        {
            LOG_MSG("OK");
        }
        else
        {
            LOG_ERR("SET_MODE fail");
            Result = MFALSE;
        }
    }
    //
    if(Result)
    {
        BWC BwcIns;
        //
        if(CurrMode.ScenSw == (pMode->ScenSw))
        {
            LOG_MSG("Same SW(%d/%d)",CurrMode.ScenSw,pMode->ScenSw);
            goto EXIT;
        }
        //
        if(pMode->Dev == RES_MGR_DRV_DEV_VT)
        {
            if(pMode->ScenSw == RES_MGR_DRV_SCEN_SW_NONE)
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,false);
            }
            else
            {
                BwcIns.Profile_Change(BWCPT_VIDEO_TELEPHONY,true);
            }
        }
        else
        {
            switch(CurrMode.ScenSw)
            {
                case RES_MGR_DRV_SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_VIDEO_PRV:
                case RES_MGR_DRV_SCEN_SW_VIDEO_REC:
                case RES_MGR_DRV_SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,false);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,false);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,false);
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
            //
            switch(pMode->ScenSw)
            {
                case RES_MGR_DRV_SCEN_SW_NONE:
                case RES_MGR_DRV_SCEN_SW_CAM_IDLE:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,false);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_CAM_PRV:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_PREVIEW,true);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_VIDEO_PRV:
                case RES_MGR_DRV_SCEN_SW_VIDEO_REC:
                case RES_MGR_DRV_SCEN_SW_VIDEO_VSS:
                {
                    BwcIns.Profile_Change(BWCPT_VIDEO_RECORD_CAMERA,true);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_CAM_CAP:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_CAPTURE,true);
                    break;
                }
                case RES_MGR_DRV_SCEN_SW_ZSD:
                {
                    BwcIns.Profile_Change(BWCPT_CAMERA_ZSD,true);
                    break;
                }
                default:
                {
                    //do nothing.
                }
            }
            //
            switch(pMode->ScenSw)
            {
                case RES_MGR_DRV_SCEN_SW_NONE:
                {
                    CloseHdmi(MFALSE);
                    break;
                }
                default:
                {
                    CloseHdmi(MTRUE);
                    break;
                }
            }

        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;

}
//-----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::CloseHdmi(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    LOG_MSG("En(%d)",En);
    //
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFdHdmiTx < 0)
    {
        mFdHdmiTx = open(RES_MGR_DRV_DEVNAME_HDMITX, O_RDONLY, 0);
        if(mFdHdmiTx < 0)
        {
            LOG_WRN("HDMITX kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
    }
    //
    if(mFdHdmiTx >= 0)
    {
        if(En)
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_CLOSE, 0) < 0)
            {
                LOG_ERR("HDMI_FORCE_CLOSE fail, errno(%d):%s",errno,strerror(errno));
                Result = MFALSE;
            }
        }
        else
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_OPEN, 0) < 0)
            {
                LOG_ERR("HDMI_FORCE_OPEN fail, errno(%d):%s",errno,strerror(errno));
                Result = MFALSE;
            }
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------

