#define LOG_TAG "MtkCam/ZSDScen"
//
#include <vector>
using namespace std;
//
#include <utils/Vector.h>
#include <mtkcam/common.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/sensor_hal.h>
using namespace NSImageio;
using namespace NSIspio;
//use MDP
#include "DpIspStream.h"
//for map mva
#include <mtkcam/drv/imem_drv.h>
//
//#include <mtkcam/featureio/eis_hal_base.h>
//
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
#include "hwUtility.h"

#include "ZSDScenario.h"
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
#define __ENABLE_TGOUT__
//
/*******************************************************************************
*
********************************************************************************/
ZSDScenario*
ZSDScenario::createInstance(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
{
    return new ZSDScenario(rSensorType, dev, bitorder);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ZSDScenario::destroyInstance()
{
    //
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
ZSDScenario::ZSDScenario(EScenarioFmt rSensorType, halSensorDev_e const &dev, ERawPxlID const &bitorder)
            : mpCamIOPipe(NULL)
            , mpDpStream(NULL)
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
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(ZSDScenario));
}


/*******************************************************************************
*
********************************************************************************/
ZSDScenario::~ZSDScenario()
{
    MY_LOGD("");
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZSDScenario::
init()
{
    FUNCTION_LOG_START;
    //
    //(1)

	
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_ZSD, mSensorType);
    if ( ! mpCamIOPipe || ! mpCamIOPipe->init())
    {
        MY_LOGE("ICamIOPipe init error");
        return MFALSE;
    }
    //(1)
    mpDpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    if ( ! mpDpStream )
    {
        MY_LOGE("DpIspStream init error");
        return MFALSE;
    }
    //(3)
    //init imem for dpStream
    mIMemDrv = IMemDrv::createInstance();

    if ( ! mIMemDrv || ! mIMemDrv->init() )
    {
        MY_LOGE("IMemDrv init fail.");
        return MFALSE;
    }

    //(3)
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS1_CQ0, 0, 0);

    mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                            (MINT32)EPIPE_PASS1_CQ0,
                            (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                            (MINT32)EPIPECQ_TRIG_BY_START);

    //
    mpCamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE,(MINT32)eConfigSettingStage_Init, 0, 0);


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
ZSDScenario::uninit()
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
     if ( mpDpStream )
     {
         delete mpDpStream;
         mpDpStream = NULL;
     }

     if ( mIMemDrv )
     {
         if ( !mIMemDrv->uninit() )
         {
             MY_LOGE("IMemDrv uninit fail.");
         }
         mIMemDrv->destroyInstance();
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
ZSDScenario::wait(EWaitType rType)
{
    switch(rType)
    {
        case eIRQ_VS:
            mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
        break;
        default:
        break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZSDScenario::start()
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
    mpCamIOPipe->irq(EPipePass_PASS1_TG1, EPIPEIRQ_VSYNC);
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
ZSDScenario::stop()
{
    FUNCTION_LOG_START;
    //
    PortID rPortID;
    mapPortCfg(eID_Pass1DispOut, rPortID);
    PortQTBufInfo dummy(eID_Pass1DispOut);
    mpCamIOPipe->dequeOutBuf(rPortID, dummy.bufInfo);
    // zsd need to add
    //
    mapPortCfg(eID_Pass1RawOut, rPortID);
    PortQTBufInfo dummyRaw(eID_Pass1RawOut);
    mpCamIOPipe->dequeOutBuf(rPortID, dummyRaw.bufInfo);

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
ZSDScenario::
setConfig(vector<PortImgInfo> *pImgIn)
{
    FUNCTION_LOG_START;
    //
    if ( ! pImgIn )
    {
        MY_LOGE("pImgIn==NULL");
        return MFALSE;
    }

    defaultSetting();

    bool isPass1 = false;
    
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
        #ifdef __ENABLE_TGOUT__
        // zsd added
        else if (rSrc.ePortIdx == eID_Pass1RawOut)
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
        #endif
        else if (rSrc.ePortIdx == eID_Pass1DispOut)
        {
            mapFormat(rSrc.sFormat, mSettingPorts.img2o.eImgFmt);
            MY_LOGD("Disp out fmt %d +", mSettingPorts.img2o.eImgFmt);
            //mSettingPorts.img2o.eImgFmt = eImgFmt_YUY2; // zsd need to remove
            //MY_LOGD("Disp out fmt %d -", mSettingPorts.img2o.eImgFmt);
            mSettingPorts.img2o.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.img2o.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.img2o.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.img2o.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.img2o.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.img2o.crop.x = rSrc.crop.x;
            mSettingPorts.img2o.crop.y = rSrc.crop.y;
            mSettingPorts.img2o.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.img2o.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.img2o.crop.w = rSrc.crop.w;
            mSettingPorts.img2o.crop.h = rSrc.crop.h;
        }
        else if (rSrc.ePortIdx == eID_Pass2In)
        {
            mapDpPortInfo(rSrc, &mSettingPorts.imgi); 
#if 0
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
#endif
        }
        else if (rSrc.ePortIdx == eID_Pass2DISPO)
        {
            mapDpPortInfo(rSrc, &mSettingPorts.dispo );
#if 0
            mapFormat(rSrc.sFormat, mSettingPorts.dispo.eImgFmt);
            mSettingPorts.dispo.u4ImgWidth = rSrc.u4Width;
            mSettingPorts.dispo.u4ImgHeight = rSrc.u4Height;
            mSettingPorts.dispo.u4Stride[ESTRIDE_1ST_PLANE] = rSrc.u4Stride[ESTRIDE_1ST_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_2ND_PLANE] = rSrc.u4Stride[ESTRIDE_2ND_PLANE];
            mSettingPorts.dispo.u4Stride[ESTRIDE_3RD_PLANE] = rSrc.u4Stride[ESTRIDE_3RD_PLANE];
            mSettingPorts.dispo.crop.x = rSrc.crop.x;
            mSettingPorts.dispo.crop.y = rSrc.crop.y;
            mSettingPorts.dispo.crop.floatX = rSrc.crop.floatX;
            mSettingPorts.dispo.crop.floatY = rSrc.crop.floatY;
            mSettingPorts.dispo.crop.w = rSrc.crop.w;
            mSettingPorts.dispo.crop.h = rSrc.crop.h;
#endif
        }
        else if (rSrc.ePortIdx == eID_Pass2VIDO)
        {
            mapDpPortInfo(rSrc, &mSettingPorts.vido);
#if 0
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
#endif
        }
        else
        {
            MY_LOGE("Not done yet!!");
        }
    }

    mSettingPorts.dump();

    if(isPass1)
    {
        // Note:: must to config cameraio pipe before irq   
        //              since cameio pipe won't be changed later, do it here
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        vCamIOInPorts.push_back(&mSettingPorts.tgi);
        vCamIOOutPorts.push_back(&mSettingPorts.imgo);
        // zsd added
        vCamIOOutPorts.push_back(&mSettingPorts.img2o);
        mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);
    }

    //
    FUNCTION_LOG_END;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ZSDScenario::sDefaultSetting_Ports::
dump()
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[TG]:F(%d),W(%d),H(%d),Str(%d)",
             tgi.eImgFmt, tgi.u4ImgWidth, tgi.u4ImgHeight, tgi.u4Stride[ESTRIDE_1ST_PLANE]);

    imgi.dump();
    vido.dump();
    dispo.dump();

}

/*******************************************************************************
*
********************************************************************************/
MVOID
ZSDScenario::
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
    tgi.index = EPortIndex_TG1I;
#ifdef __ENABLE_TGOUT__

    // (1.2) imgo
    PortInfo &imgo = mSettingPorts.imgo;
    imgo.type = EPortType_Memory;
    imgo.index = EPortIndex_IMGO;
    imgo.inout  = EPortDirection_Out;
#endif
    // (1.3) img2o
    PortInfo &img2o = mSettingPorts.img2o;
    img2o.type = EPortType_Memory;
    img2o.index = EPortIndex_IMG2O;
    img2o.inout  = EPortDirection_Out;

    ////////////////////////////////////////////////////////////////////
    //Pass 2 setting (default)
    ////////////////////////////////////////////////////////////////////
    //(2.1) imgi
    mSettingPorts.imgi.portID = 0;
    //(2.2) dispo
    mSettingPorts.dispo.portID = 0;
    //(2.3) vido 
    mSettingPorts.vido.portID = 1;

}


/*******************************************************************************
*
********************************************************************************/
MVOID
ZSDScenario::
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
ZSDScenario::
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
ZSDScenario::
enque(vector<IhwScenario::PortQTBufInfo> const &in)
{
    MY_LOGW_IF(in.size() > 1, "in.size() > 1");  //shouldn't happen
    MY_LOGW_IF(in.at(0).bufInfo.vBufInfo.size() > 1, "in.at(0).bufInfo.vBufInfo.size() > 1"); //may happen
    //
    vector<IhwScenario::PortBufInfo> vEnBufPass1Out;

    vector< IhwScenario::PortQTBufInfo >::const_iterator it;
    bool isFindBuffer = false;
    for (it = in.begin(); it != in.end(); it++ ) {
        if ((*it).ePortIndex == eID_Pass1DispOut) {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[map] enque pass 1 out buffer");
            for (MUINT32 i = 0; i < (*it).bufInfo.vBufInfo.size(); i++)
            {
                IhwScenario::PortBufInfo one(eID_Pass1DispOut,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufVA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufPA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufSize,
                                  (*it).bufInfo.vBufInfo.at(i).memID,
                                  (*it).bufInfo.vBufInfo.at(i).bufSecu,
                                  (*it).bufInfo.vBufInfo.at(i).bufCohe);

                vEnBufPass1Out.push_back(one);
            };
            continue;
        }
        else if ((*it).ePortIndex == eID_Pass1RawOut) {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "[map] enque pass 1 raw out buffer");
            for (MUINT32 i = 0; i < (*it).bufInfo.vBufInfo.size(); i++)
            {
                IhwScenario::PortBufInfo one(eID_Pass1RawOut,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufVA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufPA,
                                  (*it).bufInfo.vBufInfo.at(i).u4BufSize,
                                  (*it).bufInfo.vBufInfo.at(i).memID,
                                  (*it).bufInfo.vBufInfo.at(i).bufSecu,
                                  (*it).bufInfo.vBufInfo.at(i).bufCohe);

                vEnBufPass1Out.push_back(one);
            };

            continue;
        }
    }
    enque(NULL, &vEnBufPass1Out);

    return true;
}

/*******************************************************************************
*  enque:
********************************************************************************/
MBOOL
ZSDScenario::
enque( vector<PortBufInfo> *pBufIn, vector<PortBufInfo> *pBufOut)
{
    FUNCTION_LOG_START;
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

            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1:Idx(%d),port(%d),Id(%d),VA(0x%08X) +",
                i,rPortID.index,
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

        // [pass 2 In]
       
        DpPortInfo* pDpInPort = &mSettingPorts.imgi;
        //set config
        mpDpStream->setSrcConfig( pDpInPort->width,
                                  pDpInPort->height,
                                  pDpInPort->stride[0],
                                  pDpInPort->stride[1],
                                  pDpInPort->fmt);

        mpDpStream->setSrcCrop( pDpInPort->crop_x,
                                pDpInPort->crop_fx,
                                pDpInPort->crop_y,
                                pDpInPort->crop_fy,
                                pDpInPort->crop_w,
                                pDpInPort->crop_h);
        
        //input: suppose only one input buf.
        std::vector<PortBufInfo>::iterator it = pBufIn->begin();
        //map physical addr
        mapPhyAddr( it->bufSize, it->memID, it->virtAddr, 
                    it->bufSecu, it->bufCohe, it->phyAddr );
    
        MUINT32 srcVA[3];
        MUINT32 srcMVA[3];
        srcVA[0] = it->virtAddr;
        srcVA[1] = srcVA[0] + pDpInPort->planesize[0];
        srcVA[2] = srcVA[1] + pDpInPort->planesize[1];

        srcMVA[0] = it->phyAddr;
        srcMVA[1] = srcMVA[0] + pDpInPort->planesize[0];
        srcMVA[2] = srcMVA[1] + pDpInPort->planesize[1];

        mpDpStream->queueSrcBuffer( (void**)srcVA,
                                    srcMVA,
                                    pDpInPort->planesize,
                                    pDpInPort->planenum);
        
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2 IN Id(%d),VA(0x%08x),PA(0x%08x) +", \
                   it->memID, \
                   it->virtAddr, \
                   it->phyAddr);

        //output
        for( it = pBufOut->begin(); it != pBufOut->end(); it++ )
        {
            PortID rPortID;
            MUINT32 portIndex = 0;
            mapPortCfg(it->ePortIndex, rPortID);

            //map physical addr
            mapPhyAddr( it->bufSize, it->memID, it->virtAddr, 
                        it->bufSecu, it->bufCohe, it->phyAddr );

            DpPortInfo* pDpOutPort;
            if (rPortID.index == EPortIndex_DISPO)
            {
                pDpOutPort = &mSettingPorts.dispo;
                //check previous output
                if( mQbufDispo.vBufInfo.size() > 0 )
                {
                    MY_LOGE("pass2 dispo queue not empty: do clear");
                    mQbufDispo.vBufInfo.clear();
                }
                mapBufCfg( *it, mQbufDispo );
            }
            else if (rPortID.index == EPortIndex_VIDO)
            {
                pDpOutPort = &mSettingPorts.vido;
                //check previous output
                if( mQbufVido.vBufInfo.size() > 0 )
                {
                    MY_LOGE("pass2 vido queue not empty: do clear");
                    mQbufVido.vBufInfo.clear();
                }
                mapBufCfg( *it, mQbufVido );
            }
            else
            {
                pDpOutPort = NULL;
            }

            if( pDpOutPort )
            {
                //set config
                mpDpStream->setDstConfig( pDpOutPort->portID, 
                                          pDpOutPort->width,
                                          pDpOutPort->height,
                                          pDpOutPort->stride[0],
                                          pDpOutPort->stride[1],
                                          pDpOutPort->fmt);
                //set rot, flip
                if( pDpOutPort->portID == 1 )
                {
                    mpDpStream->setRotation( 1, pDpOutPort->rot );
                    mpDpStream->setFlipStatus( 1, pDpOutPort->flip );
                }
                else
                {
                    mpDpStream->setRotation( 0, 0 );
                    mpDpStream->setFlipStatus( 0, 0 );
                }

                //enque
                MUINT32 dstVA[3];
                MUINT32 dstMVA[3];

                dstVA[0] = it->virtAddr;
                dstVA[1] = dstVA[0] + pDpOutPort->planesize[0];
                dstVA[2] = dstVA[1] + pDpOutPort->planesize[1];

                dstMVA[0] = it->phyAddr;
                dstMVA[1] = dstMVA[0] + pDpOutPort->planesize[0];
                dstMVA[2] = dstMVA[1] + pDpOutPort->planesize[1];

                mpDpStream->queueDstBuffer( pDpOutPort->portID, 
                                            (void**)dstVA,
                                            dstMVA,
                                            pDpOutPort->planesize,
                                            pDpOutPort->planenum);

                MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2 OUT(%d) Id(%d),VA(0x%08x),PA(0x%08x) +", \
                        pDpOutPort->portID, \
                        it->memID, \
                        it->virtAddr, \
                        it->phyAddr);
            }
        }
    }
    FUNCTION_LOG_END;
    return MTRUE;
}


/*******************************************************************************
*  deque:
********************************************************************************/
MBOOL
ZSDScenario::
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
    if (port & eID_Pass1DispOut || port & eID_Pass1RawOut)
    {
        PortID rPortID;
        if (port & eID_Pass1RawOut) {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "eID_Pass1RawOut");
            mapPortCfg(eID_Pass1RawOut, rPortID);
            PortQTBufInfo one(eID_Pass1RawOut);
            if ( ! mpCamIOPipe->dequeOutBuf(rPortID, one.bufInfo) )
            {
                MY_LOGE("mpCamIOPipe->dequeOutBuf fail");
                return false;                
            }
            pBufIn->push_back(one);
            MY_LOGE_IF(one.bufInfo.vBufInfo.size()==0, "Pass 1 deque without buffer");
            for (MUINT32 i = 0; i < one.bufInfo.vBufInfo.size(); i++)
            {
                MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "(va:0x%x) - ", one.bufInfo.vBufInfo.at(i).u4BufVA);
            }
        }
        if (port & eID_Pass1DispOut) {
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "eID_Pass1DispOut");
            mapPortCfg(eID_Pass1DispOut, rPortID);
            PortQTBufInfo one(eID_Pass1DispOut);
            mpCamIOPipe->dequeOutBuf(rPortID, one.bufInfo);
            pBufIn->push_back(one);
            MY_LOGE_IF(one.bufInfo.vBufInfo.size()==0, "Pass 1 deque without buffer");
            for (MUINT32 i = 0; i < one.bufInfo.vBufInfo.size(); i++)
            {
                MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "(va:0x%x) - ", one.bufInfo.vBufInfo.at(i).u4BufVA);
            }
        }
        #if 0
        for (MUINT32 i = 0; i < one.bufInfo.vBufInfo.size(); i++)
        {
            #if CHECK_PASS1_ENQUE_SEQ
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1:Idx(%d),Id(%d),VA(0x%08X/0x%08X),Time(%d.%06d)",
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
            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1:Idx(%d),Id(%d),VA(0x%08X),Time(%d.%06d)",
                i,
                one.bufInfo.vBufInfo.at(i).memID,
                one.bufInfo.vBufInfo.at(i).u4BufVA,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_sec,
                one.bufInfo.vBufInfo.at(i).i4TimeStamp_us);
            #endif
            //
            #if CHECK_PASS1_DEQUE_SEQ
            mvPass1DequeSeq.push_back(one.bufInfo.vBufInfo.at(i).u4BufVA);
            #endif
        }
        #endif
    }


    //(1.2) wait pass 2 done
    if ((port & eID_Pass2DISPO) || (port & eID_Pass2VIDO))
    {
        //int dummyX, dummyY;
        //mpEis->doEIS(dummyX, dummyY, mSettingPorts.dispo.u4ImgWidth, mSettingPorts.dispo.u4ImgHeight);

        mpDpStream->startStream();
        
        //dequeue
        MY_LOGD("Wait P2 done");
        if (port & eID_Pass2DISPO) {
            MUINT32 va[3];
            mpDpStream->dequeueDstBuffer( 0, (void**)&va ); 

            //check if buf is correct, suppose only one buf
            std::vector<BufInfo>::iterator it = mQbufDispo.vBufInfo.begin();
            if( it->u4BufVA == va[0] )
            {
                unmapPhyAddr( it->u4BufSize, it->memID, it->u4BufVA, 
                              it->bufSecu, it->bufCohe, it->u4BufPA );

                BufInfo buf;
                PortID rPortID;
                mapPortCfg(eID_Pass2DISPO, rPortID);
                PortQTBufInfo one(eID_Pass2DISPO);
                buf = *it;
                mQbufDispo.vBufInfo.erase(it);
                one.bufInfo.vBufInfo.push_back(buf);
                pBufIn->push_back( one ); 
                MY_LOGD("va(0x%x)-", va[0]);
            }
            else
            {
                MY_LOGE( "dispo: va(0x%08x) != mQ(0x%08x)", va[0], it->u4BufVA );
            }
        }

        if (port & eID_Pass2VIDO){
            MUINT32 va[3];
            mpDpStream->dequeueDstBuffer( 1, (void**)&va ); 

            //check if buf is correct, suppose only one buf
            std::vector<BufInfo>::iterator it = mQbufVido.vBufInfo.begin();
            if( it->u4BufVA == va[0] )
            {
                unmapPhyAddr( it->u4BufSize, it->memID, it->u4BufVA, 
                              it->bufSecu, it->bufCohe, it->u4BufPA );

                BufInfo buf;
                PortID rPortID;
                mapPortCfg(eID_Pass2VIDO, rPortID);
                PortQTBufInfo one(eID_Pass2VIDO);
                buf = *it;
                mQbufVido.vBufInfo.erase(it);
                one.bufInfo.vBufInfo.push_back(buf);
                pBufIn->push_back( one ); 
                MY_LOGD("va(0x%x)-", va[0]);
            }
            else
            {
                MY_LOGE( "vido: va(0x%08x) != mQ(0x%08x)", va[0], it->u4BufVA );
            }
        }

        {
            mpDpStream->dequeueSrcBuffer(); 
        }

        mpDpStream->stopStream();

    }

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "-");
    //
    return MTRUE;
}


/*******************************************************************************
*  replaceQue:
********************************************************************************/
MBOOL
ZSDScenario::
replaceQue(vector<PortBufInfo> *pBufOld, vector<PortBufInfo> *pBufNew)
{
    if (pBufOld->size() != pBufNew->size())
    {
        MY_LOGE("Buffer size not same D=%d E=%d", pBufOld->size(), pBufNew->size());
        return MFALSE;
    }

    for ( int i=0; i < pBufOld->size(); i++) {
        PortBufInfo rBufDeqed = pBufOld->at(i);
        PortBufInfo rBufExchanged = pBufNew->at(i);
        PortID rPortID;
        QBufInfo rQbufInfo;

        mapPortCfg(rBufDeqed.ePortIndex, rPortID);
        mapBufCfg(rBufDeqed, rQbufInfo);
        mapBufCfg(rBufExchanged, rQbufInfo);

        mpCamIOPipe->enqueOutBuf(rPortID, rQbufInfo);

        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P1 RAW: port(%d),Id(%d),VA(0x%08X) Ex: Id(%d),VA(0x%08x)+",
            rPortID.index,
            rQbufInfo.vBufInfo.at(0).memID,
            rQbufInfo.vBufInfo.at(0).u4BufVA,
            rQbufInfo.vBufInfo.at(1).memID,
            rQbufInfo.vBufInfo.at(1).u4BufVA);
    }
    return MTRUE;
}


/******************************************************************************
* This is used to check whether width or height exceed limitations of HW.
*******************************************************************************/
MVOID
ZSDScenario::
getHwValidSize(MUINT32 &width, MUINT32 &height)
{
    MY_LOGD("In:W(%d),H(%d)",width,height);
    //
    //Do nothing for now.
    //If ZSD has size limitation, it should add some code here.
    //
    MY_LOGD("Out:W(%d),H(%d)",width,height);
}


inline MVOID
ZSDScenario::
DpPortInfo::
dump( )
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "fmt(0x%x) port(%d) wxh(%d,%d) p_num(%d) str(%d,%d,%d) size(%d,%d,%d)", \
                fmt, portID,  \
                width, height, \
                planenum, \
                stride[0], stride[1], stride[2], \
                planesize[0], planesize[1], planesize[2]);
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "crop(%d,%d,%d,%d,%d,%d), ro(%d), flip(%d)", \
               crop_x, crop_y, crop_fx, crop_fy, crop_w, crop_h, \
               rot, flip ); 
}
/******************************************************************************
* map format to dpframe's one
*******************************************************************************/
MBOOL
ZSDScenario::
mapDpPortInfo(const PortImgInfo& port, DpPortInfo* pDpPort)
{
    MUINT32 bit_pixel;
    //unsigned int uv_h_shift = 0;
    MUINT32 uv_v_shift = 0;
    DpColorFormat dp_fmt;
    MUINT32 planenum = 0;
    EImageFormat fmt;
    mapFormat(port.sFormat, fmt);
    
    switch( fmt )
    {
#define FMTCASE( fmt, dpfmt, bpp, n_plane, v_shift ) \
        case fmt: \
            dp_fmt = dpfmt; \
            bit_pixel = bpp; \
            planenum = n_plane; \
            uv_v_shift = v_shift; \
            break
        FMTCASE( eImgFmt_YUY2, DP_COLOR_YUYV, 16, 1, 0 );
        FMTCASE( eImgFmt_UYVY, DP_COLOR_UYVY, 16, 1, 0 );
        FMTCASE( eImgFmt_YVYU, DP_COLOR_YVYU, 16, 1, 0 );
        FMTCASE( eImgFmt_VYUY, DP_COLOR_VYUY, 16, 1, 0 );
        FMTCASE( eImgFmt_NV16, DP_COLOR_NV16,  8, 2, 0 );
        FMTCASE( eImgFmt_NV61, DP_COLOR_NV61,  8, 2, 0 );
        FMTCASE( eImgFmt_NV21, DP_COLOR_NV21,  8, 2, 1 );
        FMTCASE( eImgFmt_NV12, DP_COLOR_NV12,  8, 2, 1 );
        FMTCASE( eImgFmt_YV16, DP_COLOR_YV16,  8, 3, 0 );
        FMTCASE( eImgFmt_I422, DP_COLOR_I422,  8, 3, 0 );
        FMTCASE( eImgFmt_YV12, DP_COLOR_YV12,  8, 3, 1 );
        FMTCASE( eImgFmt_I420, DP_COLOR_I420,  8, 3, 1 );
        FMTCASE( eImgFmt_Y800, DP_COLOR_GREY,  8, 1, 0 );
        FMTCASE( eImgFmt_RGB565 , DP_COLOR_RGB565, 16, 1, 0);
        FMTCASE( eImgFmt_RGB888 , DP_COLOR_RGB888, 24, 1, 0);
        FMTCASE( eImgFmt_ARGB888, DP_COLOR_ARGB8888, 32, 1, 0);
#undef  FMTCASE
        case eImgFmt_NV21_BLK: // 420, 2 plane (Y), (VU)
        case eImgFmt_BAYER8:
        case eImgFmt_BAYER10:
        case eImgFmt_BAYER12:
        case eImgFmt_JPEG:
        case eImgFmt_NV12_BLK: // 420, 2 plane (Y), (VU)
            MY_LOGW("fmt(0x%x) not support in DP", fmt);
        default:
            MY_LOGW("fmt (0x%x) is not supported");
            return false;
            break;
    }

    pDpPort->fmt = dp_fmt;
    //pDpPort->portID; //already set
    pDpPort->width = port.u4Width;
    pDpPort->height = port.u4Height;
    pDpPort->planenum = planenum;

    const MUINT32* stride_p = port.u4Stride;
    pDpPort->stride[0] = (stride_p[0] * bit_pixel) >> 3;
    pDpPort->stride[1] = planenum > 1 ? (stride_p[1] * bit_pixel) >> 3 : 0;
    pDpPort->stride[2] = planenum > 2 ? (stride_p[2] * bit_pixel) >> 3 : 0;

    pDpPort->planesize[0] = pDpPort->stride[0] * port.u4Height;
    pDpPort->planesize[1] = planenum > 1 ? pDpPort->stride[1] * (port.u4Height >> uv_v_shift) : 0;
    pDpPort->planesize[2] = planenum > 2 ? pDpPort->stride[2] * (port.u4Height >> uv_v_shift) : 0;

    pDpPort->crop_x = port.crop.x;
    pDpPort->crop_y = port.crop.y;
    pDpPort->crop_fx = port.crop.floatX;
    pDpPort->crop_fy = port.crop.floatY;
    pDpPort->crop_w = port.crop.w;
    pDpPort->crop_h = port.crop.h;

    pDpPort->rot = port.eRotate * 90;
    pDpPort->flip = port.eFlip;
    
    return true;
}

/******************************************************************************
* mapPhyAddr
*******************************************************************************/
MBOOL ZSDScenario::
mapPhyAddr( MUINT32 size, MINT32 id, MUINT32 va, MINT32 secu, MINT32 cohe, MUINT32& pa )
{
    IMEM_BUF_INFO imembuf( size, id, va, pa, secu, cohe );

    if( mIMemDrv->mapPhyAddr( &imembuf ) < 0 )
    {
        MY_LOGE("mapPhyAddr failed.");
        return MFALSE;
    }

    pa = imembuf.phyAddr;
    return MTRUE;
}


/******************************************************************************
* unmapPhyAddr
*******************************************************************************/
MBOOL ZSDScenario::
unmapPhyAddr( MUINT32 size, MINT32 id, MUINT32 va, MINT32 secu, MINT32 cohe, MUINT32& pa )
{
    IMEM_BUF_INFO imembuf( size, id, va, pa, secu, cohe );

    if( mIMemDrv->unmapPhyAddr( &imembuf ) < 0 )
    {
        MY_LOGE("unmapPhyAddr failed.");
        return MFALSE;
    }
    return MTRUE;
}
