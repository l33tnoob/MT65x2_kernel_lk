#define LOG_TAG "MtkCam/ResourceLock"
//
#include <stdio.h>
#include <cutils/atomic.h>
#include <utils/threads.h>
//
#include <mtkcam/common.h>
#include <mtkcam/drv/res_mgr_drv.h>
#include <mtkcam/campipe/pipe_mgr_drv.h>
//
#include <inc/ResourceLock.h>
#include <ResourceLock_imp.h>
//
#include <inc/CamUtils.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//-----------------------------------------------------------------------------
ResourceLockImp::
ResourceLockImp()
    : mpPipeMgr(NULL)
    , mpResMgr(NULL)
    , mUser(0)
{
}
//-----------------------------------------------------------------------------
ResourceLockImp::
~ResourceLockImp()
{
}
//-----------------------------------------------------------------------------
ResourceLock*
ResourceLock::
CreateInstance(void)
{
    return ResourceLockImp::GetInstance();
}
//-----------------------------------------------------------------------------
ResourceLock*
ResourceLockImp::
GetInstance(void)
{
    static ResourceLockImp Singleton;
    //
    //MY_LOGD("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
MVOID
ResourceLockImp::
DestroyInstance(void) 
{
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
Init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        MY_LOGD("First user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //
    mpResMgr = ResMgrDrv::CreateInstance();
    if(mpResMgr != NULL)
    {
        if(mpResMgr->Init())
        {
            //MY_LOGD("mpResMgr->Init OK");
        }
        else
        {
            MY_LOGE("mpResMgr->Init fail");
            Result = MFALSE;
            goto EXIT;
        }
    } 
    else
    {
        MY_LOGE("mpResMgr is NULL");
        Result = MFALSE;
        goto EXIT;
    }
    //
    mpPipeMgr = PipeMgrDrv::CreateInstance();
    if(mpPipeMgr != NULL)
    {
        if(mpPipeMgr->Init())
        {
            //MY_LOGD("mpPipeMgr->Init OK");
        }
        else
        {
            MY_LOGE("mpPipeMgr->Init fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    else
    {
        MY_LOGE("mpPipeMgr is NULL");
        Result = MFALSE;
        goto EXIT;
    }
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
Uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        goto EXIT;
    }
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        MY_LOGD("Last user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        goto EXIT;
    }
    //
    if(mpResMgr != NULL)
    {
        mpResMgr->Uninit();
        mpResMgr->DestroyInstance();
        mpResMgr = NULL;
    }
    //
    if(mpPipeMgr != NULL)
    {
        mpPipeMgr->Uninit();
        mpPipeMgr->DestroyInstance();
        mpPipeMgr = NULL;    
    }
    //
    EXIT:
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL 
ResourceLockImp::
SetMode(ECamAdapter Type)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("Type(%d)",Type);
    //
    if(mUser <= 0)
    {
        MY_LOGE("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpResMgr != NULL)
    {
        RES_MGR_DRV_MODE_STRUCT rResMode;
        GetResMgr(Type, rResMode);
        mpResMgr->SetMode(&rResMode);
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
Lock(
    ECamAdapter Type,
    MUINT32     Timeout)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("Type(%d),Timeout(%d)",Type,Timeout);
    //
    if(mUser <= 0)
    {
        MY_LOGE("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpPipeMgr != NULL)
    {    
        PIPE_MGR_DRV_LOCK_STRUCT rPipeMode;
        rPipeMode.Timeout = Timeout;
        GetPipeMgr(Type, rPipeMode.PipeMask);
        //
        if(rPipeMode.PipeMask != 0)
        {
            if(mpPipeMgr->Lock(&rPipeMode))
            {
                MY_LOGD("Lock OK");
            }
            else
            {
                MY_LOGE("Lock fail");
                Result = MFALSE;
            }
        }
        else
        {
            MY_LOGD("PipeMask is 0");
        }
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
Unlock(ECamAdapter Type)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    MY_LOGD("Type(%d)",Type);
    //
    if(mUser <= 0)
    {
        MY_LOGE("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpPipeMgr != NULL)
    {    
        PIPE_MGR_DRV_UNLOCK_STRUCT rPipeMode;
        GetPipeMgr(Type, rPipeMode.PipeMask);
        //
        if(rPipeMode.PipeMask != 0)
        {
            if(mpPipeMgr->Unlock(&rPipeMode))
            {
                MY_LOGD("Unlock OK");
            }
            else
            {
                MY_LOGE("Unlock fail");
                Result = MFALSE;
            }
        }
        else
        {
            MY_LOGD("PipeMask is 0");
        }
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
GetResMgr(
    ECamAdapter                 Type,
    RES_MGR_DRV_MODE_STRUCT&    Dst)
{
    MBOOL Result = MTRUE;
    //
    if(mUser <= 0)
    {
        MY_LOGE("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    switch(Type)
    {
        case eMTK_NONE:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_NONE;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_NONE;
            break;
        }
        case eMTKCAM_IDLE:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_CAM_IDLE;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_NONE;
            break;
        }
        case eMTKPHOTO_PRV:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_CAM_PRV;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTKVIDEO_PRV:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_VIDEO_PRV;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTKVIDEO_REC:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_VIDEO_REC;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTKVIDEO_VSS:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_VIDEO_VSS;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTKZSDNCC_PRV:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_ZSD;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_ZSD;
            break;
        }
        case eMTKZSDCC_PRV:
        {
            Dst.Dev = RES_MGR_DRV_DEV_CAM;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_ZSD; // zsd am i right?
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTK_ATV:
        {
            Dst.Dev = RES_MGR_DRV_DEV_ATV;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_CAM_PRV;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        case eMTK_VT:
        {
            Dst.Dev = RES_MGR_DRV_DEV_VT;
            Dst.ScenSw = RES_MGR_DRV_SCEN_SW_CAM_PRV;
            Dst.ScenHw = RES_MGR_DRV_SCEN_HW_VSS;
            break;
        }
        default:
        {
            break;
        }
    }
    //
    MY_LOGD("Type(%d),ScenSw(%d),ScenHw(%d),Dev(%d)",
            Type,
            Dst.ScenSw,
            Dst.ScenHw,
            Dst.Dev);
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ResourceLockImp::
GetPipeMgr(
    ECamAdapter Type,
    MUINT32&    Dst)
{
    MBOOL Result = MTRUE;
    //
    if(mUser <= 0)
    {
        MY_LOGE("No user");
        Result = MFALSE;
        goto EXIT;
    }
    //
    switch(Type)
    {
        case eMTK_NONE:
        case eMTKCAM_IDLE:
        {
            Dst = 0 ;
            break;
        }
        case eMTKPHOTO_PRV:
        case eMTKVIDEO_PRV:
        case eMTKZSDCC_PRV:
        case eMTK_VT:
        case eMTK_ATV:
        {
            Dst = PIPE_MGR_DRV_PIPE_MASK_CAM_IO | PIPE_MGR_DRV_PIPE_MASK_POST_PROC;
            break;
        }
        case eMTKVIDEO_REC:
        {
            Dst = 0 ;
            break;
        }
        case eMTKVIDEO_VSS:
        {
            Dst = 0;//PIPE_MGR_DRV_PIPE_MASK_CDP_CAM;
            break;
        }
        case eMTKZSDNCC_PRV:
        {
            Dst = PIPE_MGR_DRV_PIPE_MASK_CAM_IO | PIPE_MGR_DRV_PIPE_MASK_XDP_CAM;
            break;
        }
        default:
        {
            break;
        }
    }
    //
    MY_LOGD("Type(%d),Dst(0x%08X)",Type,Dst);
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------

