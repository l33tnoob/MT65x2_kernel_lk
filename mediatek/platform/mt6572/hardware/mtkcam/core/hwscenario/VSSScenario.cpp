#define LOG_TAG "MtkCam/VSSScen"
//
#include <vector>
using namespace std;
//
#include <utils/Vector.h>
#include <mtkcam/common.h>
#include <imageio/IPipe.h>
#include <imageio/ICamIOPipe.h>
#include <imageio/IPostProcPipe.h>
#include <imageio/ispio_stddef.h>
#include <drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
using namespace NSImageio;
using namespace NSIspio;
//

//
#include <hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <hwscenario/IhwScenario.h>
#include "hwUtility.h"
#include "VSSScenario.h"
//
#include <cutils/atomic.h>
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");
#define ERROR_LOG               MY_LOGE("Error");
//
#define _PASS1_CQ_CONTINUOUS_MODE_
#define ENABLE_LOG_PER_FRAME    (1)
#define CHECK_PASS1_ENQUE_SEQ   (0)
#define CHECK_PASS1_DEQUE_SEQ   (0)
//
/*******************************************************************************
*
********************************************************************************/
VSSScenario*
VSSScenario::createInstance(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
{
    return new VSSScenario(rSensorType, dev, bitorder);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::destroyInstance()
{
    //
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::VSSScenario(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
            : mpCamIOPipe(NULL)
            , mpPostProcPipe(NULL)
            , mSensorType(rSensorType)
            , mSensorDev(dev)
            , mSensorBitOrder(bitorder)
            , mModuleMtx()
{
    //
    mvPass1EnqueSeq.clear();
    mvPass1DequeSeq.clear();
    //
    MY_LOGD("mSensorBitOrder:%d", mSensorBitOrder);
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(VSSScenario));
}


/*******************************************************************************
*
********************************************************************************/
VSSScenario::~VSSScenario()
{
    MY_LOGD("");
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
init()
{
    FUNCTION_LOG_START;
    //
    //(1)
	
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS, mSensorType);
    if ( ! mpCamIOPipe || ! mpCamIOPipe->init())
    {
        MY_LOGE("ICamIOPipe init error");
        return MFALSE;
    }

    //(2)
    mpPostProcPipe = IPostProcPipe::createInstance(eScenarioID_VSS, mSensorType);
    if ( ! mpPostProcPipe || ! mpPostProcPipe->init())
    {
        MY_LOGE("IPostProcPipe init error");
        return MFALSE;
    }


    //(3)
    //Set the device ID
    mpCamIOPipe->sendCommand(EPIPECmd_SET_SENSOR_DEV,(MINT32)mSensorDev, 0, 0);
	
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS1_CQ0, 0, 0);

    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)EPIPE_PASS1_CQ0,
                            (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                            (MINT32)EPIPECQ_TRIG_BY_START);

    mpPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                               (MINT32)EPIPE_PASS2_CQ1, 0, 0);

    mpCamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);
    mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);


    //(3)
    //mpEis = EisHalBase::createInstance();
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::uninit()
{
     FUNCTION_LOG_START;
     //
     MBOOL ret = MTRUE;
     //
     //(1)
     if ( mpCamIOPipe )
     {
         if ( ! mpCamIOPipe->uninit())
         {
             MY_LOGE("mpCamIOPipe uninit fail");
             ret = MFALSE;
         }
         mpCamIOPipe->destroyInstance();
         mpCamIOPipe = NULL;
     }
     //
     //(2)
     if ( mpPostProcPipe )
     {
         if ( ! mpPostProcPipe->uninit())
         {
             MY_LOGE("mpPostProcPipe uninit fail");
             ret = MFALSE;
         }
         mpPostProcPipe->destroyInstance();
         mpPostProcPipe = NULL;
     }
     //
     //(3)
     //if (NULL != mpEis)
     //{
     //    mpEis->destroyInstance();
     //    mpEis = NULL;
     //}


     FUNCTION_LOG_END;
     //
     return ret;
}


/*******************************************************************************
* wait hardware interrupt
********************************************************************************/
MVOID
VSSScenario::wait(EWaitType rType)
{
    switch(rType)
    {
        case eIRQ_VS:
            if ((mSensorDev == SENSOR_DEV_MAIN) || (mSensorDev == SENSOR_DEV_ATV))
                mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
            else
                mpCamIOPipe->irq(EPipePass_PASS1_TG2, EPIPEIRQ_VSYNC);
        break;
        default:
        break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::start()
{
    FUNCTION_LOG_START;
    // (1)
    //eisHal_config_t eisCfg;
    //eisCfg.ISPSize_W = mSettingPorts.imgo.u4ImgWidth;
    //eisCfg.ISPSize_H = mSettingPorts.imgo.u4ImgHeight;
    //eisCfg.CDRZSize_W = mSettingPorts.imgo.u4ImgWidth;
    //eisCfg.CDRZSize_H = mSettingPorts.imgo.u4ImgHeight;
    //mpEis->configEIS(IhwScenario::eHW_VSS, eisCfg);

    // (2) start CQ
    mpCamIOPipe->startCQ0();
#if defined(_PASS1_CQ_CONTINUOUS_MODE_)
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
#else
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_CQ_NONE, 0, 0);
#endif


    // (3) pass1 start
    if ( ! mpCamIOPipe->start())
    {
        MY_LOGE("mpCamIOPipe->start() fail");
        return MFALSE;
    }

    // align to Vsync
    if ((mSensorDev == SENSOR_DEV_MAIN) || (mSensorDev == SENSOR_DEV_ATV))
    {
        mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
    }
    else
    {
        mpCamIOPipe->irq(EPipePass_PASS1_TG2, EPIPEIRQ_VSYNC);
    }
    MY_LOGD("- wait IRQ: ISP_DRV_IRQ_INT_STATUS_VS1_ST");


    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::stop()
{
    FUNCTION_LOG_START;
    //
    PortID rPortID;
    mapPortCfg(eID_Pass1Out, rPortID);
    PortQTBufInfo dummy(eID_Pass1Out);
    mpCamIOPipe->dequeOutBuf(rPortID, dummy.bufInfo);
    //
    if ( ! mpCamIOPipe->stop())
    {
        MY_LOGE("mpCamIOPipe->stop() fail");
        return MFALSE;
    }
    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
VSSScenario::
setConfig(vector<PortImgInfo> *pImgIn)
{
    if ( ! pImgIn )
    {
        MY_LOGE("pImgIn==NULL");
        return MFALSE;
    }

    bool isPass1 = false;

    defaultSetting();

    for (MUINT32 i = 0; i < pImgIn->size(); i++)
    {
        PortImgInfo rSrc = pImgIn->at(i);
        //
        // Pass 1 config will be fixed. Pass 2 config can be updated later.
        //
        if (rSrc.ePortIdx == eID_Pass1In)
        {
            isPass1 = true;
            mapFormat(rSrc.sFormat, mSettingPorts.tgi.eImgFmt);
            mSettingPorts.tgi.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.tgi.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.tgi.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.tgi.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.tgi.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.tgi.crop.x = rSrc.crop.x;
            mSettingPorts.tgi.crop.y = rSrc.crop.y;
            mSettingPorts.tgi.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.tgi.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.tgi.crop.w = rSrc.crop.w;
            mSettingPorts.tgi.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass1Out)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.imgo.eImgFmt);
            mSettingPorts.imgo.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.imgo.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.imgo.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.imgo.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.imgo.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.imgo.crop.x = rSrc.crop.x;
            mSettingPorts.imgo.crop.y = rSrc.crop.y;
            mSettingPorts.imgo.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.imgo.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.imgo.crop.w = rSrc.crop.w;
            mSettingPorts.imgo.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass2In)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.imgi.eImgFmt);
            mSettingPorts.imgi.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.imgi.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.imgi.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.imgi.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.imgi.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.imgi.crop.x = rSrc.crop.x;
            mSettingPorts.imgi.crop.y = rSrc.crop.y;
            mSettingPorts.imgi.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.imgi.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.imgi.crop.w = rSrc.crop.w;
            mSettingPorts.imgi.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass2DISPO)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.dispo.eImgFmt);
            mSettingPorts.dispo.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.dispo.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.dispo.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
#if (PLATFORM_VERSION_MAJOR == 2)
            mSettingPorts.dispo.eImgRot = rSrc.eRotate;
#endif
            mSettingPorts.dispo.crop.x = rSrc.crop.x;
            mSettingPorts.dispo.crop.y = rSrc.crop.y;
            mSettingPorts.dispo.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.dispo.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.dispo.crop.w = rSrc.crop.w;
            mSettingPorts.dispo.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass2VIDO)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.vido.eImgFmt);
            mSettingPorts.vido.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.vido.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.vido.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.vido.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.vido.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.vido.eImgRot = rSrc.eRotate;
            mSettingPorts.vido.crop.x = rSrc.crop.x;
            mSettingPorts.vido.crop.y = rSrc.crop.y;
            mSettingPorts.vido.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.vido.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.vido.crop.w = rSrc.crop.w;
            mSettingPorts.vido.crop.h = rSrc.crop.h;
        }
        else
        {
            MY_LOGE("Not done yet!!");
        }
    }

    //mSettingPorts.dump();
    
    if (isPass1)
    {
        // Note:: must to config cameraio pipe before irq
        //        since cameio pipe won't be changed later, do it here
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        vCamIOInPorts.push_back(&mSettingPorts.tgi);
        vCamIOOutPorts.push_back(&mSettingPorts.imgo);
        mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::sDefaultSetting_Ports::
dump()
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[TG]:F(%d),W(%d),H(%d),Str(%d)",
             tgi.eImgFmt, tgi.u4ImgWidth, tgi.u4ImgHeight, tgi.u4Stride[ESTRIDE_1ST_PLANE]);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[IMGI]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
             imgi.eImgFmt, imgi.u4ImgWidth, imgi.u4ImgHeight,
             imgi.u4Stride[ESTRIDE_1ST_PLANE], imgi.u4Stride[ESTRIDE_2ND_PLANE], imgi.u4Stride[ESTRIDE_3RD_PLANE]);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[DISPO]:F(%d),W(%d),H(%d),Str(%d,%d,%d)",
             dispo.eImgFmt, dispo.u4ImgWidth, dispo.u4ImgHeight,
             dispo.u4Stride[ESTRIDE_1ST_PLANE], dispo.u4Stride[ESTRIDE_2ND_PLANE], dispo.u4Stride[ESTRIDE_3RD_PLANE]);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"[VIDO]:F(%d),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
             vido.eImgFmt, vido.u4ImgWidth, vido.u4ImgHeight,
             vido.u4Stride[ESTRIDE_1ST_PLANE], vido.u4Stride[ESTRIDE_2ND_PLANE], vido.u4Stride[ESTRIDE_3RD_PLANE],
             vido.eImgRot);

}

/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::
defaultSetting()
{

    ////////////////////////////////////////////////////////////////////
    //      Pass 1 setting (default)                                  //
    ////////////////////////////////////////////////////////////////////

    // (1.1) tgi
    PortInfo &tgi = mSettingPorts.tgi;
    tgi.eRawPxlID = mSensorBitOrder; //only raw looks this
    tgi.type = EPortType_Sensor;
    tgi.inout  = EPortDirection_In;
    tgi.index = ((mSensorDev == SENSOR_DEV_MAIN) || (mSensorDev == SENSOR_DEV_ATV))?EPortIndex_TG1I:EPortIndex_TG1I; 
    
    // (1.2) imgo
    PortInfo &imgo = mSettingPorts.imgo;
    imgo.type = EPortType_Memory;
    imgo.index = EPortIndex_IMGO;
    imgo.inout  = EPortDirection_Out;


    ////////////////////////////////////////////////////////////////////
    //Pass 2 setting (default)
    ////////////////////////////////////////////////////////////////////

    //(2.1)
    PortInfo &imgi = mSettingPorts.imgi;
    imgi.eImgFmt = eImgFmt_UNKNOWN;
    imgi.type = EPortType_Memory;
    imgi.index = EPortIndex_IMGI;
    imgi.inout = EPortDirection_In;
    imgi.pipePass = EPipePass_PASS2;
    imgi.u4ImgWidth = 0;
    imgi.u4ImgHeight = 0;
    imgi.u4Offset = 0;
    imgi.u4Stride[0] = 0;
    imgi.u4Stride[1] = 0;
    imgi.u4Stride[2] = 0;

    //(2.2)
    PortInfo &dispo = mSettingPorts.dispo;
    dispo.eImgFmt = eImgFmt_UNKNOWN;
    dispo.eImgRot = eImgRot_0;                  //dispo NOT support rotation
    dispo.eImgFlip = eImgFlip_OFF;              //dispo NOT support flip
    dispo.type = EPortType_DISP_RDMA;           //EPortType
    dispo.index = EPortIndex_DISPO;
    dispo.inout  = EPortDirection_Out;
    dispo.u4ImgWidth = 0;
    dispo.u4ImgHeight = 0;
    dispo.u4Offset = 0;
    dispo.u4Stride[0] = 0;
    dispo.u4Stride[1] = 0;
    dispo.u4Stride[2] = 0;

    //(2.3)
    PortInfo &vido = mSettingPorts.vido;
    vido.eImgFmt = eImgFmt_UNKNOWN;
    vido.eImgRot = eImgRot_0;
    vido.eImgFlip = eImgFlip_OFF;
    vido.type = EPortType_VID_RDMA;
    vido.index = EPortIndex_VIDO;
    vido.inout  = EPortDirection_Out;
    vido.u4ImgWidth = 0;
    vido.u4ImgHeight = 0;
    vido.u4Offset = 0;
    vido.u4Stride[0] = 0;
    vido.u4Stride[1] = 0;
    vido.u4Stride[2] = 0;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::
dumpPass1EnqueSeq()
{
    MUINT32 i;
    //
    if(mvPass1EnqueSeq.empty())
    {
        MY_LOGD("Pass1EnqueSeq is empty");
    }
    else
    {
        for(i=0; i<mvPass1EnqueSeq.size(); i++)
        {
            MY_LOGD("Idx(%d),VA(0x%08X)",
                    i,
                    mvPass1EnqueSeq[i]);
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID
VSSScenario::
dumpPass1DequeSeq()
{
    MUINT32 i;
    //
    if(mvPass1DequeSeq.empty())
    {
        MY_LOGD("Pass1DequeSeq is empty");
    }
    else
    {
        for(i=0; i<mvPass1DequeSeq.size(); i++)
        {
            MY_LOGD("Idx(%d),VA(0x%08X)",
                    i,
                    mvPass1DequeSeq[i]);
        }
    }
}


/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque(vector<IhwScenario::PortQTBufInfo> const &in)
{
    MY_LOGW_IF(in.size() > 1, "in.size() > 1");  //shouldn't happen
    MY_LOGW_IF(in.at(0).bufInfo.vBufInfo.size() > 1, "in.at(0).bufInfo.vBufInfo.size() > 1"); //may happen
    //
    vector<IhwScenario::PortBufInfo> vEnBufPass1Out;

    for (MUINT32 i = 0; i < in.at(0).bufInfo.vBufInfo.size(); i++)
    {
        IhwScenario::PortBufInfo one(eID_Pass1Out,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufVA,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufPA,
                          in.at(0).bufInfo.vBufInfo.at(i).u4BufSize,
                          in.at(0).bufInfo.vBufInfo.at(i).memID,
                          in.at(0).bufInfo.vBufInfo.at(i).bufSecu,
                          in.at(0).bufInfo.vBufInfo.at(i).bufCohe);

        vEnBufPass1Out.push_back(one);
    };

    enque(NULL, &vEnBufPass1Out);

    return true;
}

/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
VSSScenario::
enque( vector<PortBufInfo> *pBufIn, vector<PortBufInfo> *pBufOut)
{
    if ( !pBufIn ) // pass 1
    {
        // Note:: can't update config, but address
        //
        MUINT32 size = pBufOut->size();
        for (MUINT32 i = 0; i < size; i++)
        {
            PortID rPortID;
            QBufInfo rQbufInfo;
            mapConfig(pBufOut->at(i), rPortID, rQbufInfo);
            mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);

            MY_LOGD_IF(0, "P1:Idx(%d),Id(%d),VA(0x%08X)",
                i,
                rQbufInfo.vBufInfo.at(0).memID,
                rQbufInfo.vBufInfo.at(0).u4BufVA);
            //
            #if CHECK_PASS1_ENQUE_SEQ
            mvPass1EnqueSeq.push_back(rQbufInfo.vBufInfo.at(0).u4BufVA);
            #endif
            //
            #if CHECK_PASS1_DEQUE_SEQ
            if(mvPass1DequeSeq.empty())
            {
                MY_LOGD("Pass1DequeSeq is empty");
            }
            else
            {
                if(rQbufInfo.vBufInfo.at(0).u4BufVA == mvPass1DequeSeq[0])
                {
                    mvPass1DequeSeq.erase(mvPass1DequeSeq.begin());
                }
                else
                {
                    MY_LOGE("VA(0x%08X) is not enque by seq VA(0x%08X)",
                            rQbufInfo.vBufInfo.at(0).u4BufVA,
                            mvPass1DequeSeq[0]);
                    dumpPass1DequeSeq();
                }
            }
            #endif
        }
    }
    else  // pass 2
    {
        //(1)


        // (2)
        // [pass 2 In]
        vector<PortInfo const*> vPostProcInPorts;
        vPostProcInPorts.push_back(&mSettingPorts.imgi);
        //
        MUINT32 size = pBufIn->size();
        for (MUINT32 i = 0; i < size; i++)
        {
            PortID rPortID;
            QBufInfo rQbufInfo;
            mapConfig(pBufIn->at(i), rPortID, rQbufInfo);
            mpPostProcPipe->enqueInBuf(rPortID, rQbufInfo);
            //
            MY_LOGD_IF(0, "P2 in:Idx(%d),Id(%d),VA(0x%08X) +",
                i, rQbufInfo.vBufInfo.at(0).memID, rQbufInfo.vBufInfo.at(0).u4BufVA);
        }

        // [pass 2 Out]
        vector<PortInfo const*> vPostProcOutPorts;
        size = pBufOut->size();
        for (MUINT32 i = 0; i < size; i++)
        {
            PortID rPortID;
            QBufInfo rQbufInfo;
            mapConfig(pBufOut->at(i), rPortID, rQbufInfo);
            mpPostProcPipe->enqueOutBuf(rPortID, rQbufInfo);
            //
            if (rPortID.index == EPortIndex_DISPO)
            {
                vPostProcOutPorts.push_back(&mSettingPorts.dispo);
            }
            else if (rPortID.index == EPortIndex_VIDO)
            {
                vPostProcOutPorts.push_back(&mSettingPorts.vido);
            }
            //
            MY_LOGD_IF(0, "P2 out:Idx(%d),Id(%d),VA(0x%08X) +",
                i, rQbufInfo.vBufInfo.at(0).memID, rQbufInfo.vBufInfo.at(0).u4BufVA);
        }
        //
        mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        // revise config to "update" mode after the first configPipe
        mpPostProcPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, (MINT32)eConfigSettingStage_UpdateTrigger, 0, 0);
    }
    return MTRUE;
}


/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
VSSScenario::
deque(EHwBufIdx port, vector<PortQTBufInfo> *pBufIn)
{
    if ( ! pBufIn )
    {
        MY_LOGE("pBufIn==NULL");
        return MFALSE;
    }

    if ( port == eID_Unknown )
    {
        MY_LOGE("port == eID_Unknown");
        return MFALSE;
    }

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+");

    //(1.1) wait pass 1 done
    if (port & eID_Pass1Out)
    {    
		
        PortID rPortID;
        mapPortCfg(eID_Pass1Out, rPortID);
        PortQTBufInfo one(eID_Pass1Out);
        
        if ( ! mpCamIOPipe->dequeOutBuf(rPortID, one.bufInfo))
        {
            MY_LOGE("mpCamIOPipe->dequeOutBuf fail");
            return false;
        }
        pBufIn->push_back(one);

        MY_LOGE_IF(one.bufInfo.vBufInfo.size()==0, "Pass 1 deque without buffer");

        for (MUINT32 i = 0; i < one.bufInfo.vBufInfo.size(); i++)
        {
            #if CHECK_PASS1_ENQUE_SEQ
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1:Idx(%d),Id(%d),VA(0x%08X/0x%08X),T(%d.%06d)",
                i,
                one.bufInfo.vBufInfo.at(i).memID,
                one.bufInfo.vBufInfo.at(i).u4BufVA,
                mvPass1EnqueSeq[0],
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_us);
            //
            if(mvPass1EnqueSeq.empty())
            {
                MY_LOGD("Pass1EnqueSeq is empty");
            }
            else
            {
                if(one.bufInfo.vBufInfo.at(i).u4BufVA == mvPass1EnqueSeq[0])
                {
                    mvPass1EnqueSeq.erase(mvPass1EnqueSeq.begin());
                }
                else
                {
                    MY_LOGE("VA(0x%08X) is not deque by seq VA(0x%08X)",
                            one.bufInfo.vBufInfo.at(i).u4BufVA,
                            mvPass1EnqueSeq[0]);
                    dumpPass1EnqueSeq();
                }
            }
            #else
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1:Idx(%d),Id(%d),VA(0x%08X),PA(0x%08X),S(%d),T(%d.%06d)",
                i,one.bufInfo.vBufInfo.at(i).memID,one.bufInfo.vBufInfo.at(i).u4BufVA, one.bufInfo.vBufInfo.at(i).u4BufPA,
                one.bufInfo.vBufInfo.at(i).u4BufSize,one.bufInfo.vBufInfo.at(i).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_us);
            #endif
            //
            #if CHECK_PASS1_DEQUE_SEQ
            mvPass1DequeSeq.push_back(one.bufInfo.vBufInfo.at(i).u4BufVA);
            #endif
        }
    }


    //(1.2) wait pass 2 done
    if ((port & eID_Pass2DISPO) || (port & eID_Pass2VIDO))
    {
        //int dummyX, dummyY;
        //mpEis->doEIS(dummyX, dummyY, mSettingPorts.dispo.u4ImgWidth, mSettingPorts.dispo.u4ImgHeight);
        mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);
        if (port & eID_Pass2DISPO)
        {
            mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
        }
        if (port & eID_Pass2VIDO)
        {
            mpPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);
        }
        mpPostProcPipe->start();

        mpPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE);
        //
        if (port & eID_Pass2DISPO) {
            PortID rPortID;
            mapPortCfg(eID_Pass2DISPO, rPortID);
            PortQTBufInfo one(eID_Pass2DISPO);
            mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo);
            pBufIn->push_back(one);
            //
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2DISPO:Id(%d),VA(0x%08X),PA(0x%08X),S(%d),T(%d.%06d)",
                one.bufInfo.vBufInfo.at(0).memID, one.bufInfo.vBufInfo.at(0).u4BufVA, one.bufInfo.vBufInfo.at(0).u4BufPA,
                one.bufInfo.vBufInfo.at(0).u4BufSize, one.bufInfo.vBufInfo.at(0).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(0).i4TimeStamp_us);
        }
        if (port & eID_Pass2VIDO){
            PortID rPortID;
            mapPortCfg(eID_Pass2VIDO, rPortID);
            PortQTBufInfo one(eID_Pass2VIDO);
            mpPostProcPipe->dequeOutBuf(rPortID, one.bufInfo);
            pBufIn->push_back(one);
            //
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2VIDO:Id(%d),VA(0x%08X),PA(0x%08X),S(%d),T(%d.%06d)",
                one.bufInfo.vBufInfo.at(0).memID, one.bufInfo.vBufInfo.at(0).u4BufVA, one.bufInfo.vBufInfo.at(0).u4BufPA,
                one.bufInfo.vBufInfo.at(0).u4BufSize, one.bufInfo.vBufInfo.at(0).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(0).i4TimeStamp_us);            
        }
        // deque out pass2 in buffer
        {
            PortID rPortID;
            mapPortCfg(eID_Pass2In, rPortID);
            QTimeStampBufInfo dummy;
            mpPostProcPipe->dequeInBuf(rPortID, dummy);
        }
        mpPostProcPipe->stop();
    }

    //
    return MTRUE;
}


/*******************************************************************************
*  replaceQue:
********************************************************************************/
MBOOL
VSSScenario::
replaceQue(vector<PortBufInfo> *pBufOld, vector<PortBufInfo> *pBufNew)
{
    PortID rPortID;
    QBufInfo rQbufInfo;
    //
    mapConfig(pBufOld->at(0), rPortID, rQbufInfo);
    mapConfig(pBufNew->at(0), rPortID, rQbufInfo);
    mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);
    //
    MY_LOGD("P1:VA:Old(0x%08X),New(0x%08X)",
            rQbufInfo.vBufInfo.at(0).u4BufVA,
            rQbufInfo.vBufInfo.at(1).u4BufVA);
    //
    return MTRUE;
}


/******************************************************************************
* This is used to check whether width or height exceed limitations of HW.
*******************************************************************************/
#define VSS_HW_LIMIT_LINE_BUF      (3264)
#define VSS_HW_LIMIT_FRAME_PIXEL   (3264*1836)
//
MVOID
VSSScenario::
getHwValidSize(MUINT32 &width, MUINT32 &height)
{
    MY_LOGD("In:W(%d),H(%d)",width,height);
    //
    if(width > VSS_HW_LIMIT_LINE_BUF)
    {
        MY_LOGW("W(%d) is larger than limitation(%d)",
                width,
                VSS_HW_LIMIT_LINE_BUF);
        width = VSS_HW_LIMIT_LINE_BUF;
        MY_LOGW("W is forced to set %d",width);
    }
    //
    if((width * height) > VSS_HW_LIMIT_FRAME_PIXEL)
    {
        MY_LOGW("Frame pixel(%d x %d = %d) is larger than limitation(%d)",
                width,
                height,
                (width * height),
                VSS_HW_LIMIT_FRAME_PIXEL);
        height = (width*9/16)&(~0x1);
        MY_LOGW("H is forced to set %d in 16:9 ratio",height);
    }
    //
    MY_LOGD("Out:W(%d),H(%d)",width,height);
}


