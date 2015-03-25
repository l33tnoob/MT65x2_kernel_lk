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
#define LOG_TAG "VssImgTrans"
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <cutils/xlog.h>
#include <utils/threads.h>
//
#include <mtkcam/common.h>
using namespace NSCam;
#include <imageio/IPipe.h>
#include <imageio/IPostProcPipe.h>
#include <imageio/ICamIOPipe.h>
#include <imageio/ispio_stddef.h>
#include <imageio/ICdpPipe.h>
//
using namespace NSImageio;
using namespace NSIspio;
#include <vss_img_trans.h>
#include <vss_img_trans_imp.h>
//-----------------------------------------------------------------------------
VssImgTransImp::VssImgTransImp()
{
    //LOG_MSG("");
    mUser = 0;
    mTpipeNum = 0;
    mStart = MFALSE;
    mpPipePass2 = NULL;
    mpPostProcPipe = NULL;
    mpCdpPipe = NULL;
    mvPortIn.clear();
    mvPortOut.clear();
}
//----------------------------------------------------------------------------
VssImgTransImp::~VssImgTransImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
VssImgTrans* VssImgTrans::CreateInstance(void)
{
    return VssImgTransImp::GetInstance();
}
//-----------------------------------------------------------------------------
VssImgTrans* VssImgTransImp::GetInstance(void)
{
    static VssImgTransImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
MVOID VssImgTransImp::DestroyInstance(void)
{
}
//----------------------------------------------------------------------------
MBOOL VssImgTransImp::Init(CONFIG_STRUCT& Config, MUINT32& TpipeNum)
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
    if( !(Config.DispoOut.Enable) &&
        !(Config.VidoOut.Enable))
    {
        LOG_ERR("DISPO(%d) or VIDO(%d) should be enabled",
                Config.DispoOut.Enable,
                Config.VidoOut.Enable);
        Result = MFALSE;
        goto EXIT;
    }
    //
    mConfig = Config;
    //
    mStart = MFALSE;
    mpPipePass2 = NULL;
    mpPostProcPipe = NULL;
    mpCdpPipe = NULL;
    mvPortIn.clear();
    mvPortOut.clear();
    //
    LOG_MSG("Fmt(%d)",mConfig.ImageIn.Format);
    switch(mConfig.ImageIn.Format)
    {
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        {
            mFormat = eScenarioFmt_RAW;
            mpPostProcPipe = IPostProcPipe::createInstance(eScenarioID_VSS, mFormat);
            if(mpPostProcPipe == NULL)
            {
                LOG_ERR("PostProcPipe is NULL");
                TpipeNum = 0;
                Result = MFALSE;
                goto EXIT;
            }
            mpPipePass2 = (IPipe*)mpPostProcPipe;
            break;
        }
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        {
            mFormat = eScenarioFmt_YUV;
            mpCdpPipe = ICdpPipe::createInstance(eScenarioID_VSS_CDP_CC, mFormat);
            if(mpCdpPipe == NULL)
            {
                LOG_ERR("PostProcPipe is NULL");
                TpipeNum = 0;
                Result = MFALSE;
                goto EXIT;
            }
            mpPipePass2 = (IPipe*)mpCdpPipe;
            break;
        }
        // RGB
        case eImgFmt_RGB565:
        case eImgFmt_RGB888:
        case eImgFmt_ARGB888:
        // YUV 2 plane
        case eImgFmt_YV12:
        case eImgFmt_NV21:
        case eImgFmt_NV21_BLK:
        case eImgFmt_NV12:
        case eImgFmt_NV12_BLK:
        // YUV 3 plane
        case eImgFmt_YV16:
        case eImgFmt_NV16:
        case eImgFmt_NV61:
        case eImgFmt_I420:
        //JPG
        case eImgFmt_JPEG:
        //
        default:
        {
            LOG_ERR("Not support fmt(%d)",mConfig.ImageIn.Format);
            TpipeNum = 0;
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    ConfigPass2();
    TpipeNum = mTpipeNum;
    LOG_MSG("Tpipe Num(%d)",mTpipeNum);
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL VssImgTransImp::Uninit(void)
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
    if(mpPipePass2 != NULL)
    {
        mpPipePass2->uninit();
        mpPipePass2->destroyInstance();
        mpPipePass2 = NULL;
        //
        mpPostProcPipe = NULL;
        mpCdpPipe = NULL;
        //
        LOG_MSG("Pass2 uninit");
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL VssImgTransImp::Start(MUINT32 TpipeIndex)
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
    //LOG_MSG("Fmt(%d)",mFormat);
    switch(mFormat)
    {
        case eScenarioFmt_RAW:
        case eScenarioFmt_YUV:
        {
            if(mStart)
            {
                LOG_ERR("Start already");
            }
            else
            if(TpipeIndex > (mTpipeNum-1))
            {
                LOG_ERR("Idx(%d) > MaxIdx(%d)",
                        TpipeIndex+1,
                        mTpipeNum);
            }
            else
            {
                mImgiPort.u4SegTpipeSimpleConfigIdx = TpipeIndex;
                mpPipePass2->configPipe(mvPortIn, mvPortOut);
                LOG_MSG("Fmt(%d),Idx(%d/%d)",
                        mFormat,
                        TpipeIndex+1,
                        mTpipeNum);
                mpPipePass2->start();
                mStart = MTRUE;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL VssImgTransImp::WaitDone(void)
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
    //LOG_MSG("Fmt(%d)",mFormat);
    switch(mFormat)
    {
        case eScenarioFmt_RAW:
        case eScenarioFmt_YUV:
        {
            if(mStart)
            {
                Result = mpPipePass2->irq(EPipePass_PASS2B,EPIPEIRQ_PATH_DONE);
                if(Result)
                {
                    LOG_MSG("Fmt(%d),Idx(%d/%d) OK",
                            mFormat,
                            mImgiPort.u4SegTpipeSimpleConfigIdx+1,
                            mTpipeNum);
                }
                else
                {
                    LOG_ERR("Fmt(%d),Idx(%d/%d) fail",
                            mFormat,
                            mImgiPort.u4SegTpipeSimpleConfigIdx+1,
                            mTpipeNum);
                }
                mStart = MFALSE;
            }
            else
            {
                LOG_ERR("Not start");
            }
            break;
        }
        default:
        {
            break;
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MVOID VssImgTransImp::ConfigImgi(PortInfo &ImgiPort)
{
    LOG_MSG("F(%d).Size:W(%d),H(%d),S(%d).Crop:X(%d),Y(%d),W(%d),H(%d).Addr:V(0x%08X),P(0x%08X)",
            mConfig.ImageIn.Format,
            mConfig.ImageIn.Size.Width,
            mConfig.ImageIn.Size.Height,
            mConfig.ImageIn.Size.Stride,
            mConfig.ImageIn.Crop.X,
            mConfig.ImageIn.Crop.Y,
            mConfig.ImageIn.Crop.W,
            mConfig.ImageIn.Crop.H,
            mConfig.ImageIn.Mem.Vir,
            mConfig.ImageIn.Mem.Phy);
    ImgiPort.eImgFmt = mConfig.ImageIn.Format;
    ImgiPort.u4ImgWidth = mConfig.ImageIn.Size.Width;
    ImgiPort.u4ImgHeight = mConfig.ImageIn.Size.Height;
    ImgiPort.u4Stride[ESTRIDE_1ST_PLANE] = mConfig.ImageIn.Size.Stride;
    ImgiPort.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    ImgiPort.u4Stride[ESTRIDE_3RD_PLANE] = 0;
    ImgiPort.crop.x = mConfig.ImageIn.Crop.X;
    ImgiPort.crop.y = mConfig.ImageIn.Crop.Y;
    ImgiPort.crop.floatX = 0;
    ImgiPort.crop.floatY = 0;
    ImgiPort.crop.w = mConfig.ImageIn.Crop.W;
    ImgiPort.crop.h = mConfig.ImageIn.Crop.H;
    ImgiPort.u4IsRunSegment = 1;
    ImgiPort.u4SegNumVa = (MUINT32)&mTpipeNum;
    ImgiPort.type = EPortType_Memory;
    ImgiPort.index = EPortIndex_IMGI;
    ImgiPort.inout  = EPortDirection_In;
    ImgiPort.pipePass = EPipePass_PASS2B;
	ImgiPort.memID = mConfig.ImageIn.Mem.id;
    ImgiPort.u4BufVA = mConfig.ImageIn.Mem.Vir;
    ImgiPort.u4BufPA = mConfig.ImageIn.Mem.Phy;
    LOG_MSG("X");
}
//-----------------------------------------------------------------------------
MVOID VssImgTransImp::ConfigDispo(PortInfo &DispoPort)
{
    LOG_MSG("F(%d).Size:W(%d),H(%d),S(%d).Addr:V(0x%08X),P(0x%08X)",
            mConfig.DispoOut.Format,
            mConfig.DispoOut.Size.Width,
            mConfig.DispoOut.Size.Height,
            mConfig.DispoOut.Size.Stride,
            mConfig.DispoOut.Mem.Vir,
            mConfig.DispoOut.Mem.Phy);
    DispoPort.eImgFmt = mConfig.DispoOut.Format;
    DispoPort.eImgRot = eImgRot_0;
    DispoPort.eImgFlip = eImgFlip_OFF;
    DispoPort.u4ImgWidth = mConfig.DispoOut.Size.Width;
    DispoPort.u4ImgHeight = mConfig.DispoOut.Size.Height;
    DispoPort.u4Stride[ESTRIDE_1ST_PLANE] = mConfig.DispoOut.Size.Stride;
    DispoPort.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    DispoPort.u4Stride[ESTRIDE_3RD_PLANE] = 0;
    DispoPort.type = EPortType_DISP_RDMA;
    DispoPort.index = EPortIndex_DISPO;
    DispoPort.inout = EPortDirection_Out;
	DispoPort.memID = mConfig.DispoOut.Mem.id;
    DispoPort.u4BufVA = mConfig.DispoOut.Mem.Vir;
    DispoPort.u4BufPA = mConfig.DispoOut.Mem.Phy;
    LOG_MSG("X");
}
//-----------------------------------------------------------------------------
MVOID VssImgTransImp::ConfigVido(PortInfo &VidoPort)
{
    LOG_MSG("F(%d),R(%d),F(%d).Size:W(%d),H(%d),S(%d).Addr:V(0x%08X),P(0x%08X)",
            mConfig.VidoOut.Format,
            mConfig.VidoOut.Rotate,
            mConfig.VidoOut.Flip,
            mConfig.VidoOut.Size.Width,
            mConfig.VidoOut.Size.Height,
            mConfig.VidoOut.Size.Stride,
            mConfig.VidoOut.Mem.Vir,
            mConfig.VidoOut.Mem.Phy);
    VidoPort.eImgFmt = mConfig.VidoOut.Format;
    VidoPort.eImgRot = mConfig.VidoOut.Rotate;
    VidoPort.eImgFlip = mConfig.VidoOut.Flip;
    VidoPort.u4ImgWidth = mConfig.VidoOut.Size.Width;
    VidoPort.u4ImgHeight = mConfig.VidoOut.Size.Height;
    VidoPort.u4Stride[ESTRIDE_1ST_PLANE] = mConfig.VidoOut.Size.Stride;
    VidoPort.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    VidoPort.u4Stride[ESTRIDE_3RD_PLANE] = 0;
    VidoPort.type = EPortType_VID_RDMA;
    VidoPort.index = EPortIndex_VIDO;
    VidoPort.inout  = EPortDirection_Out;
	VidoPort.memID = mConfig.VidoOut.Mem.id;
    VidoPort.u4BufVA = mConfig.VidoOut.Mem.Vir;
    VidoPort.u4BufPA = mConfig.VidoOut.Mem.Phy;
    LOG_MSG("X");
}
//-----------------------------------------------------------------------------
MVOID VssImgTransImp::ConfigPass2(void)
{
    LOG_MSG("Enable:DispoOut(%d),VidoOut(%d)",
            mConfig.DispoOut.Enable,
            mConfig.VidoOut.Enable);
    //
    mpPipePass2->init();
    mpPipePass2->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init,0,0);
    mpPipePass2->sendCommand(EPIPECmd_SET_IMG_PLANE_BY_IMGI,(MINT32)1,0,0);
    mpPipePass2->sendCommand(EPIPECmd_GET_GDMA,(MINT32)1,0,0);
    //
    mvPortIn.clear();
    mvPortOut.clear();
    //
    ConfigImgi(mImgiPort);
    mvPortIn.push_back(&mImgiPort);
    //
    if(mConfig.DispoOut.Enable)
    {
        ConfigDispo(mDispoPort);
        mvPortOut.push_back(&mDispoPort);
    }
    if(mConfig.VidoOut.Enable)
    {
        ConfigVido(mVidoPort);
        mvPortOut.push_back(&mVidoPort);
    }
    //
    mpPipePass2->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ2,0,0);
    mpPipePass2->configPipe(mvPortIn, mvPortOut);
    mpPipePass2->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_UpdateTrigger,0,0);
    //
    LOG_MSG("X");
}
//-----------------------------------------------------------------------------


