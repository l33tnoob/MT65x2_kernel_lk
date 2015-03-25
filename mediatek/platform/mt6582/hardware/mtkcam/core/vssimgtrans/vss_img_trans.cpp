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
#include <mtkcam/common.h>
#include <mtkcam/campipe/IPipe.h>
#include <mtkcam/campipe/IPostProcPipe.h>
#include <mtkcam/campipe/IXdpPipe.h>
#include <mtkcam/campipe/_identity.h>
//
using namespace NSCamPipe;
#include <mtkcam/vssimgtrans/vss_img_trans.h>
#include <vss_img_trans_imp.h>
//-----------------------------------------------------------------------------
VssImgTransImp::VssImgTransImp()
{
    //LOG_MSG("");
    mUser = 0;
    mStart = MFALSE;
    mpPipePass2 = NULL;
    mpPostProcPipe = NULL;
    mpXdpPipe = NULL;
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
MBOOL VssImgTransImp::Init(CONFIG_STRUCT& Config)
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
    mpXdpPipe = NULL;
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
            mpPostProcPipe = IPostProcPipe::createInstance(eSWScenarioID_VSS, eScenarioFmt_RAW);
            if(mpPostProcPipe == NULL)
            {
                LOG_ERR("PostProcPipe is NULL");
                Result = MFALSE;
                goto EXIT;
            }
            mpPipePass2 = (IPipe*)mpPostProcPipe;
            break;
        }
        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
        case eImgFmt_YV12:
        {
            mpXdpPipe = IXdpPipe::createInstance(eSWScenarioID_VSS, eScenarioFmt_YUV);
            if(mpXdpPipe == NULL)
            {
                LOG_ERR("XdpPipe is NULL");
                Result = MFALSE;
                goto EXIT;
            }
            mpPipePass2 = (IPipe*)mpXdpPipe;
            break;
        }
        // RGB
        case eImgFmt_RGB565:
        case eImgFmt_RGB888:
        case eImgFmt_ARGB888:
        // YUV 2 plane
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
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    mpPipePass2->init();
    ConfigPass2();
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
        mpXdpPipe = NULL;
        //
        LOG_MSG("Pass2 uninit");
    }
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL VssImgTransImp::Start(void)
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
    if(mStart)
    {
        LOG_ERR("Start already");
    }
    else
    {
        LOG_MSG("Start");
        mpPipePass2->start();
        mStart = MTRUE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL VssImgTransImp::WaitDone(void)
{
    MBOOL Result = MFALSE;
    //
    if(mUser <= 0)
    {
        LOG_ERR("No user");
        goto EXIT;
    }
    //
    if(mConfig.DispoOut.Enable)
    {
        LOG_MSG("Wait DISPO");
        QTimeStampBufInfo rQDispOutBuf; 
        Result = mpPipePass2->dequeBuf(
                                PortID(
                                    EPortType_MemoryOut,
                                    0,
                                    1),
                                rQDispOutBuf);
    }
    //
    if(mConfig.VidoOut.Enable)
    {
        LOG_MSG("Wait VIDO");
        QTimeStampBufInfo rQVdoOutBuf; 
        Result = mpPipePass2->dequeBuf(
                                PortID(
                                    EPortType_MemoryOut,
                                    1,
                                    1),
                                rQVdoOutBuf);
    }
    //
    if(Result)
    {
        LOG_MSG("Done");
        QTimeStampBufInfo rQInBuf;
        mpPipePass2->dequeBuf(
                        PortID(
                            EPortType_MemoryIn,
                            0,
                            0),
                        rQInBuf);
        //
        LOG_MSG("stop");
        mpPipePass2->stop(); 
    }
    else
    {
        LOG_MSG("Not yet");
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//-----------------------------------------------------------------------------
MVOID VssImgTransImp::ConfigPass2(void)
{
    vector<PortInfo const*> vInPorts; 
    vector<PortInfo const*> vOutPorts;
    //
    LOG_MSG("IMGI:F(%d).Size:W(%d),H(%d),S(%d/%d/%d).Crop:X(%d),Y(%d),W(%d),H(%d).Addr:Id(%d),V(0x%08X),P(0x%08X),S(%d)",
            mConfig.ImageIn.Format,
            mConfig.ImageIn.Size.Width,
            mConfig.ImageIn.Size.Height,
            mConfig.ImageIn.Size.Stride[0],
            mConfig.ImageIn.Size.Stride[1],
            mConfig.ImageIn.Size.Stride[2],
            mConfig.ImageIn.Crop.X,
            mConfig.ImageIn.Crop.Y,
            mConfig.ImageIn.Crop.W,
            mConfig.ImageIn.Crop.H,
            mConfig.ImageIn.Mem.Id,
            mConfig.ImageIn.Mem.Vir,
            mConfig.ImageIn.Mem.Phy,
            mConfig.ImageIn.Mem.Size);
    LOG_MSG("DISPO:E(%d),F(%d).Size:W(%d),H(%d),S(%d/%d/%d).Addr:Id(%d),V(0x%08X),P(0x%08X),S(%d)",
            mConfig.DispoOut.Enable,
            mConfig.DispoOut.Format,
            mConfig.DispoOut.Size.Width,
            mConfig.DispoOut.Size.Height,
            mConfig.DispoOut.Size.Stride[0],
            mConfig.DispoOut.Size.Stride[1],
            mConfig.DispoOut.Size.Stride[2],
            mConfig.DispoOut.Mem.Id,
            mConfig.DispoOut.Mem.Vir,
            mConfig.DispoOut.Mem.Phy,
            mConfig.DispoOut.Mem.Size);
    LOG_MSG("VIDO:E(%d),F(%d),R(%d),F(%d).Size:W(%d),H(%d),S(%d/%d/%d).Addr:Id(%d),V(0x%08X),P(0x%08X),S(%d)",
            mConfig.VidoOut.Enable,
            mConfig.VidoOut.Format,
            mConfig.VidoOut.Rotate,
            mConfig.VidoOut.Flip,
            mConfig.VidoOut.Size.Width,
            mConfig.VidoOut.Size.Height,
            mConfig.VidoOut.Size.Stride[0],
            mConfig.VidoOut.Size.Stride[1],
            mConfig.VidoOut.Size.Stride[2],
            mConfig.VidoOut.Mem.Id,
            mConfig.VidoOut.Mem.Vir,
            mConfig.VidoOut.Mem.Phy,
            mConfig.VidoOut.Mem.Size);
    //
    mpPipePass2->setCallbacks(NULL, NULL, NULL); 
    //IMGI
    MemoryInPortInfo rMemInPort(
                        ImgInfo(
                            mConfig.ImageIn.Format,
                            mConfig.ImageIn.Size.Width,
                            mConfig.ImageIn.Size.Height),
                        0,
                        mConfig.ImageIn.Size.Stride,
                        Rect(
                            mConfig.ImageIn.Crop.X,
                            mConfig.ImageIn.Crop.Y,
                            mConfig.ImageIn.Crop.W,
                            mConfig.ImageIn.Crop.H));
    rMemInPort.u4Stride[0] = mConfig.ImageIn.Size.Stride[0];
    rMemInPort.u4Stride[1] = mConfig.ImageIn.Size.Stride[1];
    rMemInPort.u4Stride[2] = mConfig.ImageIn.Size.Stride[2];
    vInPorts.push_back(&rMemInPort);
    //DISPO
    MemoryOutPortInfo rDispPort(
                        ImgInfo(
                            mConfig.DispoOut.Format,
                            mConfig.DispoOut.Size.Width,
                            mConfig.DispoOut.Size.Height), 
                        mConfig.DispoOut.Size.Stride,
                        0,
                        0);
    rDispPort.u4Stride[0] = mConfig.DispoOut.Size.Stride[0];
    rDispPort.u4Stride[1] = mConfig.DispoOut.Size.Stride[1];
    rDispPort.u4Stride[2] = mConfig.DispoOut.Size.Stride[2];
    rDispPort.index = 0;
    if(mConfig.DispoOut.Enable)
    {
        vOutPorts.push_back(&rDispPort);
    }
    //VIDO
    MemoryOutPortInfo rVdoPort(
                        ImgInfo(
                            mConfig.VidoOut.Format,
                            mConfig.VidoOut.Size.Width,
                            mConfig.VidoOut.Size.Height), 
                        mConfig.VidoOut.Size.Stride,
                        mConfig.VidoOut.Rotate,
                        mConfig.VidoOut.Flip);
    rVdoPort.u4Stride[0] = mConfig.VidoOut.Size.Stride[0];
    rVdoPort.u4Stride[1] = mConfig.VidoOut.Size.Stride[1];
    rVdoPort.u4Stride[2] = mConfig.VidoOut.Size.Stride[2];
    rVdoPort.index = 1;
    if(mConfig.VidoOut.Enable)
    {
        vOutPorts.push_back(&rVdoPort);
    }
    //
    LOG_MSG("configPipe");
    mpPipePass2->configPipe(
                    vInPorts,
                    vOutPorts,
                    MTRUE); 
    //IMGI
    LOG_MSG("EQ:IMGI");
    QBufInfo rInQBuf;
    BufInfo rInBufInfo(
                mConfig.ImageIn.Mem.Size,
                mConfig.ImageIn.Mem.Vir,
                mConfig.ImageIn.Mem.Phy,
                mConfig.ImageIn.Mem.Id);  
    rInQBuf.vBufInfo.push_back(rInBufInfo);
    mpPipePass2->enqueBuf(
                    PortID(
                        EPortType_MemoryIn,
                        0,
                        0),
                    rInQBuf); 
    //DISPO
    QBufInfo rDispQBuf; 
    BufInfo rDispBufInfo(
                mConfig.DispoOut.Mem.Size,
                mConfig.DispoOut.Mem.Vir,
                mConfig.DispoOut.Mem.Phy,
                mConfig.DispoOut.Mem.Id);
    if(mConfig.DispoOut.Enable)
    {
        LOG_MSG("EQ:DISPO");  
        rDispQBuf.vBufInfo.push_back(rDispBufInfo); 
        mpPipePass2->enqueBuf(
                        PortID(
                            EPortType_MemoryOut,
                            0,
                            1),
                        rDispQBuf);
    }
    //VIDO
    QBufInfo rVdoQBuf; 
    BufInfo rVdoBufInfo(
                mConfig.VidoOut.Mem.Size,
                mConfig.VidoOut.Mem.Vir,
                mConfig.VidoOut.Mem.Phy,
                mConfig.VidoOut.Mem.Id);
    if(mConfig.VidoOut.Enable)
    {
        LOG_MSG("EQ:VIDO");
        rVdoQBuf.vBufInfo.push_back(rVdoBufInfo); 
        mpPipePass2->enqueBuf(
                        PortID(
                            EPortType_MemoryOut,
                            1,
                            1),
                        rVdoQBuf);
    }
    //
    LOG_MSG("X");
}
//-----------------------------------------------------------------------------


