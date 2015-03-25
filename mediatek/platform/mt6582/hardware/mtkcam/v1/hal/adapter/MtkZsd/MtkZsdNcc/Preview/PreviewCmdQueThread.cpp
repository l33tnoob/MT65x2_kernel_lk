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

#define LOG_TAG "MtkCam/ZSDPrvCQT"

#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
#include <adapter/inc/ImgBufProvidersManager.h>
//
#include <utils/List.h>
#include <vector>
#include <list>
using namespace std;
//
#include <inc/IState.h>
#include <inc/PreviewCmdQueThread.h>
using namespace android::NSMtkZsdNccCamAdapter;
//
#include <mtkcam/featureio/eis_hal_base.h>
//
#include <mtkcam/hal/aaa_hal_base.h>
using namespace NS3A;
#include <mtkcam/hal/sensor_hal.h>
#include <kd_imgsensor_define.h>
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>
#include <mtkcam/imageio/ispio_stddef.h>
using namespace NSImageio::NSIspio;
#include <mtkcam/v1/hwscenario/IhwScenario.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>
//
#include <camera_custom_zsd.h>
//
#define DUMP
#ifdef DUMP
#include <cutils/properties.h>
#endif
#define ENABLE_LOG_PER_FRAME        (1)
//
#define EIS_ENABLE      (1)
#define __ENABLE_PASS2__
#define __ENABLE_3A__
#define ISP_REG_DUMP_ENABLE
//#define STORED_BUFFER_CNT        (3)
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN               MY_LOGD("+")
#define FUNCTION_OUT              MY_LOGD("-")

#define ROUND_TO_2X(x) ((x) & (~0x1))
/******************************************************************************
*
*******************************************************************************/
static IhwScenario::Rect_t calCrop(IhwScenario::Rect_t const &rSrc,
                                   IhwScenario::Rect_t const &rDst,
                                   uint32_t ratio = 100);
/******************************************************************************
*
*******************************************************************************/
static void mapNode2BufInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortBufInfo &dst);
static void mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst);
static bool mapQT2BufInfo(EHwBufIdx eInPort, EHwBufIdx eOutPort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst);
static bool dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, uint32_t filenum, uint32_t width, uint32_t height);
static bool dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, uint32_t filenum, uint32_t width, uint32_t height);
static bool mapQT2Node(vector<IhwScenario::PortQTBufInfo> &vQTBuf, list<CapBufQueNode> &vInNode, list<CapBufQueNode> &vOutNode);
/******************************************************************************
*
*******************************************************************************/

namespace android {
namespace NSMtkZsdNccCamAdapter {

/******************************************************************************
*
******************************************************************************/
struct globalInfo{
    globalInfo()
        : openId(-1)
    {}

    int32_t openId;
} gInfo;

/******************************************************************************
*
******************************************************************************/
class sensorInfo{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Construction interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    sensorInfo()
        : meSensorDev(SENSOR_DEV_NONE)
        , meSensorType(SENSOR_TYPE_UNKNOWN)
        , mu4TgOutW(0)
        , mu4TgOutH(0)
        , mu4MemOutW(0)
        , mu4MemOutH(0)
        , mu4SensorDelay(0)
        , mpSensor(NULL)
    {}

    bool init(ACDK_SCENARIO_ID_ENUM scenarioId)
    {
        //(1) init
        mpSensor = SensorHal::createInstance();
        if ( ! mpSensor ) {
            return false;
        }

        //(2) main or sub
        meSensorDev = (halSensorDev_e)DevMetaInfo::queryHalSensorDev(gInfo.openId);

        //
        mpSensor->sendCommand(meSensorDev, SENSOR_CMD_SET_SENSOR_DEV);
        mpSensor->init();

        //(3) raw or yuv
        mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (int32_t)&meSensorType);

        //(4) tg/mem size
        uint32_t  u4TgInW = 0;
        uint32_t  u4TgInH = 0;
        switch (scenarioId)
        {
            case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
            {
                mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_PRV_RANGE, (int32_t)&u4TgInW, (uint32_t)&u4TgInH);
                break;
            }
            case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
            {
                mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_VIDEO_RANGE, (int32_t)&u4TgInW, (int32_t)&u4TgInH);
                break;
            }
            // zsd added:
            case ACDK_SCENARIO_ID_CAMERA_ZSD:
            {
                mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_FULL_RANGE, (int32_t)&u4TgInW, (int32_t)&u4TgInH);
                break;
            }
            default:
                break;
        }
        //
        if( !( u4TgInW != 0 && u4TgInH != 0 ) )
        {
            return false;
        }
        //
        mu4TgOutW = ROUND_TO_2X(u4TgInW);  // in case sensor returns odd weight
        mu4TgOutH = ROUND_TO_2X(u4TgInH);  // in case senosr returns odd height
        mu4MemOutW = mu4TgOutW;
        mu4MemOutH = mu4TgOutH;
        //
        // zsd todo: need to check width & height?
        //getHwValidSize(mu4MemOutW, mu4MemOutH);
        //
        halSensorIFParam_t sensorCfg[2];
        int idx = meSensorDev == SENSOR_DEV_MAIN ? 0 : 1;
        sensorCfg[idx].u4SrcW = u4TgInW;
        sensorCfg[idx].u4SrcH = u4TgInH;
        sensorCfg[idx].u4CropW = mu4TgOutW;
        sensorCfg[idx].u4CropH = mu4TgOutH;
        sensorCfg[idx].u4IsContinous = 1;
        sensorCfg[idx].u4IsBypassSensorScenario = 0;
        sensorCfg[idx].u4IsBypassSensorDelay = 1;
        sensorCfg[idx].scenarioId = scenarioId;
        mpSensor->setConf(sensorCfg);
        //
        //(5) format
        halSensorRawImageInfo_t sensorFormatInfo;
        memset(&sensorFormatInfo, 0, sizeof(halSensorRawImageInfo_t));

        mpSensor->sendCommand(meSensorDev,
                              SENSOR_CMD_GET_RAW_INFO,
                              (MINT32)&sensorFormatInfo,
                              1,
                              0);
        mSensorBitOrder = (ERawPxlID)sensorFormatInfo.u1Order;
        if(meSensorType == SENSOR_TYPE_RAW)  // RAW
        {
            switch(sensorFormatInfo.u4BitDepth)
            {
                case 8 :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_BAYER8;
                break;
                case 10 :
                default :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_BAYER10;
                break;
            }
        }
        else if (meSensorType == SENSOR_TYPE_YUV){
            switch(sensorFormatInfo.u1Order)
            {
                case SENSOR_OUTPUT_FORMAT_UYVY :
                case SENSOR_OUTPUT_FORMAT_CbYCrY :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_UYVY;
                    break;
                case SENSOR_OUTPUT_FORMAT_VYUY :
                case SENSOR_OUTPUT_FORMAT_CrYCbY :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_VYUY;
                    break;
                case SENSOR_OUTPUT_FORMAT_YVYU :
                case SENSOR_OUTPUT_FORMAT_YCrYCb :
                    mFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I_YVYU;
                    break;
                case SENSOR_OUTPUT_FORMAT_YUYV :
                case SENSOR_OUTPUT_FORMAT_YCbYCr :
                default :
                    mFormat = CameraParameters::PIXEL_FORMAT_YUV422I;
                    break;
            }
        }
        else {
            MY_LOGE("Unknown sensor type: %d", meSensorType);
        }
        MY_LOGD("meSensorDev(%d), meSensorType(%d), mSensorBitOrder(%d), mFormat(%s)",
                 meSensorDev, meSensorType, mSensorBitOrder, mFormat);
        return true;
    }
    //
    bool dumpIspReg( )
    {
        mpSensor->dumpReg();
        return true;
    }
    //
    bool uninit()
    {
        if(mpSensor) {
            mpSensor->uninit();
            mpSensor->destroyInstance();
            return true;
        }
        return false;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member query interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    halSensorType_e     getSensorType()     const   { return meSensorType; }
    uint32_t            getImgWidth()       const   { return mu4MemOutW; }
    uint32_t            getImgHeight()      const   { return mu4MemOutH; }
    uint32_t            getSensorWidth()    const   { return mu4TgOutW; }
    uint32_t            getSensorHeight()   const   { return mu4TgOutH; }
    const char*         getImgFormat()      const   { return mFormat;}
    uint32_t            getDelayFrame(int32_t mode) const
    {
        mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT,
                              (int32_t)&mu4SensorDelay, (int32_t)&mode);
        return mu4SensorDelay;
    }

    uint32_t            getImgWidthStride(uint_t const uPlaneIndex = 0) const
    {
        return FmtUtils::queryImgWidthStride(mFormat, getImgWidth(), uPlaneIndex);
    }

public:
    bool                isYUV()             const   { return meSensorType == SENSOR_TYPE_YUV
                                                                            ? true : false; }
    bool                isSub()             const   { return meSensorDev == SENSOR_DEV_SUB
                                                                            ? true : false; }
    void                reset()
    {
        if (mpSensor)
        {
            mpSensor->reset();
        }
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Member variable
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    halSensorDev_e                          meSensorDev;
    halSensorType_e                         meSensorType;
    ERawPxlID                               mSensorBitOrder;
    uint32_t                                mu4TgOutW;
    uint32_t                                mu4TgOutH;
    uint32_t                                mu4MemOutW;
    uint32_t                                mu4MemOutH;
    uint32_t                                mu4SensorDelay;
    SensorHal*                              mpSensor;
    char const*                             mFormat;
};


/******************************************************************************
*
*******************************************************************************/
#define PASS1BUFCNT      (3)
//#define PASS1RAWBUFCNT   (STORED_BUFFER_CNT+PASS1BUFCNT)

class PreviewCmdQueThread : public IPreviewCmdQueThread
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Basic Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();


public:
    static PreviewCmdQueThread* getInstance(sp<IPreviewBufMgrHandler> pPHandler,
                                    sp<ICaptureBufMgrHandler> pCHandler,
                                    int32_t const & rSensorid,
                                    sp<IParamsManager> pParamsMgr);

    virtual             ~PreviewCmdQueThread();

    virtual bool        setParameters();
    virtual bool        setZoom(uint32_t zoomValue);


    virtual uint32_t    getShotMode(void){return mShotMode;};

protected:
                        PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pPHandler,
                                sp<ICaptureBufMgrHandler> pCHandler,
                                int32_t const & rSensorid,
                                sp<IParamsManager> pParamsMgr);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public to IPreviewCmdQueThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual int32_t     getTid()        const   { return mi4Tid; }
    virtual bool        isExitPending() const   { return exitPending(); }
    virtual bool        postCommand(PrvCmdCookie::ECmdType const rcmdType,
                                    PrvCmdCookie::ESemWait const rSemWait = PrvCmdCookie::eSemNone);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Detail operation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                start();
    bool                delay(EQueryType_T type);
    bool                update();
    bool                updateOne();
    bool                updateCheck();
    bool                stop();
    bool                precap();
    void                updateZoom(vector<IhwScenario::PortImgInfo> &pImgIn);
    uint32_t            getZoom();
    bool                dropFrame();
    IhwScenario::Rect_t doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio = 100);
    void                getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo);
    bool                queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height);
    bool                initCfg(void);
    void                dumpCfg(void);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command-related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                getCommand(sp<PrvCmdCookie> &rCmd);
    bool                isNextCommand();
    List< sp<PrvCmdCookie> > mCmdCookieQ;
    Mutex               mCmdMtx;
    Condition           mCmdCond;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  other modules
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    IhwScenario*                 getHw()        const    { return mpHwScenario; }
    sp<IParamsManager> const     getParamsMgr() const    { return mspParamsMgr; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    Hal3ABase*                   mp3AHal;
    IhwScenario*                 mpHwScenario;
    sp<IPreviewBufMgrHandler>    mspPreviewBufHandler;
    sp<ICaptureBufMgrHandler>    mspCaptureBufHandler;
    sp<IParamsManager>           mspParamsMgr;
    sensorInfo                   mSensorInfo;

    int32_t                      mi4Tid;
    int32_t                      mbAWBIndicator;
    int32_t                      mbEFFECTIndicator;
    //
    #define ZOOM_SKIP_STEP      (2)
    Mutex                       mZoomMtx;
    Vector<uint32_t>            mvZoomRatio;
    uint32_t                    mCurZoomValue;
    uint32_t                    mShotMode;
    uint32_t                    mFrameCnt;

    struct sPorts{
        IhwScenario::PortImgInfo pass1In;
        IhwScenario::PortImgInfo pass1DispOut;
        IhwScenario::PortImgInfo pass1RawOut;
        IhwScenario::PortImgInfo pass2In;
        //PortImgInfo vido;
        //PortImgInfo dispo;
    };
    sPorts mPorts;
    list<CapBufQueNode>       mvDequedImgBufNode;

    EisHalBase*                 mpEisHal;

};

}; // namespace NSMtkPhotoCamAdapter
}; // namespace android
/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pPHandler,
                                         sp<ICaptureBufMgrHandler> pCHandler,
                                         int32_t const & rSensorid,
                                         sp<IParamsManager> pParamsMgr)
    : mpHwScenario(NULL)
    , mspPreviewBufHandler(pPHandler)
    , mspCaptureBufHandler(pCHandler)
    , mspParamsMgr(pParamsMgr)
    , mSensorInfo()
    , mi4Tid(0)
    , mbAWBIndicator(0)
    , mbEFFECTIndicator(0)
    , mZoomMtx()
    , mvZoomRatio()
    , mpEisHal(NULL)
    , mShotMode(0)
{
    gInfo.openId = rSensorid;
}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::~PreviewCmdQueThread()
{
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(PreviewCmdQueThread));

    // zsd to do: alloc in preview thread, but dealloc in adpater, seems abnormal
    mspCaptureBufHandler->freeBuffer();

}




/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::requestExit()
{
    FUNCTION_IN;
    bool isIdle =  IStateManager::inst()->isState(IState::eState_Idle);
    if  ( !isIdle )
    {
        MY_LOGW("stop preview before exiting preview thread.");
        postCommand(PrvCmdCookie::eStop);
    }
    //
    Thread::requestExit();
    postCommand(PrvCmdCookie::eExit);
    mCmdCond.broadcast();
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
status_t
PreviewCmdQueThread::readyToRun()
{
    FUNCTION_IN;
    //
    // (1) set thread name
    ::prctl(PR_SET_NAME,"ZsdPreviewCmdQueThread", 0, 0, 0);

    // (2) set thread priority
    // [!!]Priority RR?
    int32_t const policy = SCHED_RR;
    int32_t const priority = PRIO_RT_CAMERA_PREVIEW;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //test
    mi4Tid = ::gettid();
    ::sched_getparam(0, &sched_p);
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d"
        , mi4Tid, ::sched_getscheduler(0)
        , sched_p.sched_priority
    );
    //
    mFrameCnt = 0;
    //
    FUNCTION_OUT;
    //
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::threadLoop()
{
    FUNCTION_IN;
    //
    bool ret = true;
    //
    sp<PrvCmdCookie> pCmdCookie;
    //
    if (getCommand(pCmdCookie))
    {
        if(pCmdCookie != 0)
        {
            pCmdCookie->postSem(PrvCmdCookie::eSemBefore);
        }
        //
        bool isvalid = true;
        //
        switch (pCmdCookie->getCmd())
        {
            case PrvCmdCookie::eStart:
                isvalid = start();
                break;
            case PrvCmdCookie::eDelay:
                isvalid = delay(EQueryType_Init);
                break;
            case PrvCmdCookie::eUpdate:
                isvalid = update();
                break;
            case PrvCmdCookie::ePrecap:
                isvalid = precap();
                break;
            case PrvCmdCookie::eStop:
                isvalid = stop();
                break;
            case PrvCmdCookie::eExit:
            default:
                break;
        }

        //
        if(pCmdCookie != 0)
        {
            pCmdCookie->setValid(isvalid);
            pCmdCookie->postSem(PrvCmdCookie::eSemAfter);
        }
    }
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
postCommand(PrvCmdCookie::ECmdType const cmdType, PrvCmdCookie::ESemWait const semWait)
{
    FUNCTION_IN;
    //
    bool ret = true;
    //
    sp<PrvCmdCookie> cmdCookie(new PrvCmdCookie(cmdType, semWait));
    //
    {
        Mutex::Autolock _l(mCmdMtx);
        //
        MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdCookieQ.size());

        if (!mCmdCookieQ.empty())
        {
            MY_LOGD("(%d) in the head of queue", (*mCmdCookieQ.begin())->getCmd());
        }

        mCmdCookieQ.push_back(cmdCookie);
        mCmdCond.broadcast();
        MY_LOGD("- new command added(%d):  tid(%d), que size(%d)", cmdType, ::gettid(), mCmdCookieQ.size());
    }
    //
    cmdCookie->waitSem();
    if (!cmdCookie->isValid())
    {
        ret = false;
    }
    //
    FUNCTION_OUT;
    //
    return ret;
}



/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
getCommand(sp<PrvCmdCookie> &rCmdCookie)
{
    FUNCTION_IN;
    //
    bool ret = false;
    //
    Mutex::Autolock _l(mCmdMtx);
    //
    MY_LOGD("+ tid(%d), que size(%d)", ::gettid(), mCmdCookieQ.size());
    //
    while ( mCmdCookieQ.empty() && ! exitPending() )
    {
        mCmdCond.wait(mCmdMtx);
    }
    //
    if ( ! mCmdCookieQ.empty() )
    {
        rCmdCookie = *mCmdCookieQ.begin();
        mCmdCookieQ.erase(mCmdCookieQ.begin());
        ret = true;
        MY_LOGD("Command: %d", rCmdCookie->getCmd());
    }
    //
    MY_LOGD("- tid(%d), que size(%d), ret(%d)", ::gettid(), mCmdCookieQ.size(), ret);
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
isNextCommand()
{
   Mutex::Autolock _l(mCmdMtx);
   //
   return mCmdCookieQ.empty()? false : true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
start()
{
    FUNCTION_IN;
    //
    bool ret = false;
    vector<IhwScenario::PortImgInfo> vimgInfo;
    vector<IhwScenario::PortBufInfo> vBufPass1Out;
    ImgBufQueNode Pass1Node;
    IhwScenario::PortBufInfo BufInfo;
    int storedBufCnt = 0;
    int pass1RAWBufCnt = 0;


    //(1) sensor (singleton)
    //
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init Sensor");
    //
    if( ! (ret = mSensorInfo.init(ACDK_SCENARIO_ID_CAMERA_ZSD)))
    {
        MY_LOGE("Init sensor fail!!");
        goto lbExit;
    }


    //(2) Hw scenario
    //
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init Hw");
    //
    mpHwScenario = IhwScenario::createInstance(eHW_ZSD, mSensorInfo.getSensorType(),
                                               mSensorInfo.meSensorDev,
                                               mSensorInfo.mSensorBitOrder);
    if(mpHwScenario != NULL)
    {
        if(!(mpHwScenario->init()))
        {
            MY_LOGE("init Hw Scenario fail!!");
            ret = false;
            goto lbExit;
        }
    }
    else
    {
        MY_LOGE("mpHwScenario is NULL!!");
        ret = false;
        goto lbExit;
    }

    // (2.1) hw config
    //
    if ( !initCfg() )
    {
        MY_LOGE("init Port Configure fail");
        ret = false;
        goto lbExit;
    }

    getCfg(eID_Pass1In|eID_Pass1DispOut|eID_Pass1RawOut, vimgInfo); // zsd modified
    getHw()->setConfig(&vimgInfo);


    // (2) enque pass 1 buffer
    //     must do this earlier than hw start
    mspPreviewBufHandler->allocBuffer(
                          mPorts.pass1DispOut.u4Width,
                          mPorts.pass1DispOut.u4Height,
                          mPorts.pass1DispOut.sFormat,
                          PASS1BUFCNT);
    for (int32_t i = 0; i < PASS1BUFCNT; i++)
    {
        mspPreviewBufHandler->dequeBuffer(eID_Pass1DispOut, Pass1Node);
        mapNode2BufInfo(eID_Pass1DispOut, Pass1Node, BufInfo);
        vBufPass1Out.push_back(BufInfo);
    }
    // (2.3) allocate capture buffer
    storedBufCnt = get_zsd_cap_stored_frame_cnt();
    if (storedBufCnt < 1)
    {
        MY_LOGE("Query ZSD buffer cnt error, ZSDNCC need at least 1 stored buffer, %d", storedBufCnt);
        goto lbExit;
    }
    pass1RAWBufCnt = storedBufCnt + PASS1BUFCNT;

    MY_LOGD("stored buffer:%d pass1 Raw buffer: %d",storedBufCnt, pass1RAWBufCnt );
    mspCaptureBufHandler->allocBuffer(
                      mPorts.pass1RawOut.u4Width,
                      mPorts.pass1RawOut.u4Height,
                      mPorts.pass1RawOut.sFormat,
                      0,0,0,NULL,
                      pass1RAWBufCnt);
    mspCaptureBufHandler->setStoredBufferCnt(storedBufCnt);

    // zsd added: buffer count of CameraIO two outport should be same
    //for (int32_t i = 0; i < PASS1RAWBUFCNT; i++)
    for (int32_t i = 0; i < PASS1BUFCNT; i++)
    {

        CapBufQueNode capNode;
        mspCaptureBufHandler->dequeProvider(capNode);
        mvDequedImgBufNode.push_back(capNode);
        mapNode2BufInfo(eID_Pass1RawOut, capNode.mainImgNode, BufInfo);
        vBufPass1Out.push_back(BufInfo);
    }

    getHw()->enque(NULL, &vBufPass1Out);


    //(3) 3A
    //!! must be set after hw->enque; otherwise, over-exposure.
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init 3A");
    //
    #ifdef __ENABLE_3A__
    mp3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId));
    if ( ! mp3AHal )
    {
        MY_LOGE("init 3A fail!!");
        goto lbExit;
    }
    #endif


    // (4) 3A start
    #ifdef __ENABLE_3A__
    //mp3AHal->setZoom(100, 0, 0, mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    //mp3AHal->setIspProfile(EIspProfile_NormalPreview);
    mp3AHal->setZoom(100, 0, 0, mPorts.pass2In.u4Width, mPorts.pass2In.u4Height);
    mp3AHal->setIspProfile(EIspProfile_ZsdPreview_NCC);
    mp3AHal->sendCommand(ECmd_CameraPreviewStart);
    #endif


    // (4) EIS
    //
#if EIS_ENABLE
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init EIS");
    mpEisHal = EisHalBase::createInstance("mtkzsdnccAdapter");
    if(mpEisHal != NULL)
    {
        eisHal_config_t eisHalConfig;
        eisHalConfig.imageWidth = mPorts.pass1DispOut.u4Width;
        eisHalConfig.imageHeight = mPorts.pass1DispOut.u4Height;
        //eisHalConfig.imageWidth = mPorts.pass1RawOut.u4Width;
        //eisHalConfig.imageHeight = mPorts.pass1RawOut.u4Height;
        mpEisHal->configEIS(
                    eHW_ZSD,
                    eisHalConfig);
    }
    else
    {
        MY_LOGE("mpEisHal is NULL");
        goto lbExit;
    }
#endif
    // (5) hw start
    // !!enable pass1 SHOULD BE last step!!
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Hw start");
    //
    if ( ! getHw()->start())
    {
        goto lbExit;
    }


    ret = true;

lbExit:
    //
    FUNCTION_OUT;
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
delay(EQueryType_T type)
{
    FUNCTION_IN;

    bool ret = true;

    //(1)
    switch (type)
    {
        case EQueryType_Init:
        {
            
            IStateManager::inst()->transitState(IState::eState_Preview);
#ifdef __ENABLE_3A__
            //
            //(1) delay by AAA and sensor driver
            //
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t delaySensor = mSensorInfo.getDelayFrame(SENSOR_PREVIEW_DELAY);
            int32_t delayCnt = 1;

            if(delay3A >= (delaySensor-1))
            {
                delayCnt += delay3A;
            }
            else
            {
                delayCnt += (delaySensor-1);
            }

            //(2) should send update to sw
            // Error Handling: If failure time accumulates up to 2 tims (which equals to 10 secs),
            // leave while loop and return fail.
            int failCnt = 0;
            for (int32_t i = 0; i < delayCnt; i++)
            {
                if ( ! dropFrame() )
                {
                    delayCnt++;
                    failCnt++;

                    if (failCnt >= 2)
            {
                        return false;
                    }
                    continue;
                }

                failCnt = 0;
            }

            MY_LOGD("delay(Init):delayCnt(%d),3A(%d),sensor(%d)",delayCnt,delay3A,delaySensor);
#endif
        }
        break;

        case EQueryType_Effect:
        {
#ifdef __ENABLE_3A__
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbEFFECTIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }

            MY_LOGD("delay(Effect): (%d), real: (%d)", delay3A, count);
#endif
        }

        case EQueryType_AWB:
        {
#ifdef __ENABLE_3A__
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0;
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbAWBIndicator)) {
                    dropFrame();
                }
                else {
                    break;
                }
            }

            MY_LOGD("delay(Awb): (%d), real: (%d)", delay3A, count);
#endif
        }
        break;
    }

    FUNCTION_OUT;

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
setParameters()
{
    FUNCTION_IN;

#define UPDATE_PARAMS(param, eMapXXX, key) \
    do { \
        String8 const s = mspParamsMgr->getStr(key); \
        if  ( ! s.isEmpty() ) { \
            param = PARAMSMANAGER_MAP_INST(eMapXXX)->valueFor(s); \
        } \
    } while (0)

#ifdef __ENABLE_3A__
    //(0)
    Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId)); 
    if ( ! p3AHal )
    {
        MY_LOGE("init 3A fail!!");
        return false;
    }


    //(1) Check awb mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapWhiteBalance, CameraParameters::KEY_WHITE_BALANCE);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4AwbMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbAWBIndicator);
            MY_LOGD("AWB mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }

    //(2) check effect mode change
    {
        int32_t newParam;
        UPDATE_PARAMS(newParam, eMapEffect, CameraParameters::KEY_EFFECT);
        Param_T oldParamAll;
        p3AHal->getParams(oldParamAll);
        int32_t oldParam = oldParamAll.u4EffectMode;
        if (newParam != oldParam)
        {
            ::android_atomic_write(1, &mbEFFECTIndicator);
            MY_LOGD("EFFECT mode changed (%d) --> (%d)", oldParam, newParam);
        }
    }
#endif

    //(3) Zoom
    setZoom(getParamsMgr()->getZoomRatio());

    //
#ifdef __ENABLE_3A__
    p3AHal->destroyInstance();
#endif

    //
    FUNCTION_OUT;

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height)
{
    getParamsMgr()->getPreviewSize(&ri4Width, &ri4Height);
    MY_LOGD("Preview size(%d %d)",ri4Width, ri4Height);
    return  true;
}

/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
setZoom(uint32_t zoomValue)
{
    FUNCTION_IN;
    //
    Mutex::Autolock _l(mZoomMtx);
    //
    if( mvZoomRatio.empty() ||
        (   !mvZoomRatio.empty() &&
            *mvZoomRatio.end() != zoomValue))
    {
        MY_LOGD("zoomValue(%d)",zoomValue);
        mvZoomRatio.push_back(zoomValue);
    }
    //
    FUNCTION_OUT;
    //
    return true;
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
PreviewCmdQueThread::
getZoom()
{
    //FUNCTION_IN;
    //
    Mutex::Autolock _l(mZoomMtx);
    //
    uint32_t i,ZoomSkip;
    uint32_t zoomValue;
    //
    if(!mvZoomRatio.empty())
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Size(%d)",mvZoomRatio.size());
        if(mvZoomRatio.size() > ZOOM_SKIP_STEP)
        {
            for(i=0; i<ZOOM_SKIP_STEP; i++)
            {
                ZoomSkip = *mvZoomRatio.begin();
                mvZoomRatio.erase(mvZoomRatio.begin());
                MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Skip(%d)",ZoomSkip);
            }
        }
        zoomValue = *mvZoomRatio.begin();
        mvZoomRatio.erase(mvZoomRatio.begin());
    }
    else
    {
        //MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Zoom from params");
        zoomValue = getParamsMgr()->getZoomRatio();
    }
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME,"Value(%d)",zoomValue);
    //
    //FUNCTION_OUT;
    //
    return zoomValue;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
stop()
{
    FUNCTION_IN;
    //
    bool ret = true;

    //(1) stop sw
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_stop, CPTFlagSeparator, "stop 3A");
    //
    #ifdef __ENABLE_3A__
    if ( mp3AHal != NULL )
    {
        mp3AHal->sendCommand(ECmd_CameraPreviewEnd);
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }
    #endif

    //(2) stop HW scenario
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_stop, CPTFlagSeparator, "stop Hw");
    //
    if ( mpHwScenario != NULL )
    {
        if ( ! (ret = getHw()->stop()) )
        {
            MY_LOGE("fail");
            ret = false;        
        }
        mpHwScenario->uninit();
        mpHwScenario->destroyInstance();
        mpHwScenario = NULL;
    }
    mspPreviewBufHandler->freeBuffer();
    //mspPreviewBufHandler = 0;

    list<CapBufQueNode>::iterator it;
    int dequedSize = mvDequedImgBufNode.size();
    for (int i = 0; i < dequedSize; i++  ) {
        mspCaptureBufHandler->enqueProvider(mvDequedImgBufNode.front(), false);
        mvDequedImgBufNode.pop_front();
    }

    // free capture buffer if not zsd capture.
    if (mShotMode != eShotMode_ZsdShot )
    {
        MY_LOGD("zsd free memory %d", mShotMode);
        mspCaptureBufHandler->freeBuffer();
    }


    //(3) stop sensor
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_stop, CPTFlagSeparator, "stop sensor");
    if ( ! mSensorInfo.uninit() )
    {
        MY_LOGE("uninit sensor fail");
        ret = false;
    }


    //(4) stop eis
#if EIS_ENABLE
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_stop, CPTFlagSeparator, "stop EIS");
    if(mpEisHal != NULL)
    {
        mpEisHal->destroyInstance("mtkzsdnccAdapter");
        mpEisHal = NULL;
    }
#endif


    //(5) change state to idle
    IStateManager::inst()->transitState(IState::eState_Idle);
    //
    ::android_atomic_write(0, &mbAWBIndicator);
    ::android_atomic_write(0, &mbEFFECTIndicator);
    mvZoomRatio.clear();


    FUNCTION_OUT;

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
precap()
{
    FUNCTION_IN;
    uint32_t bFlashOn = 0;
    String8 const s8ShotMode = getParamsMgr()->getShotModeStr();
    uint32_t u4ShotMode = getParamsMgr()->getShotMode();
    MY_LOGI("<shot mode> %#x(%s)", u4ShotMode, s8ShotMode.string());
    //
    mShotMode = u4ShotMode;
    #ifdef __ENABLE_3A__
    bFlashOn = mp3AHal->isNeedFiringFlash();
    #endif
    MY_LOGD("flash %s", bFlashOn == 1?"ON":"OFF");

    // ZSD shot only works when normal shot with strobe off
    if ( bFlashOn == 0 && mShotMode == eShotMode_NormalShot ) {
        mShotMode = eShotMode_ZsdShot;
    }

    if (mShotMode == eShotMode_ZsdShot )
    {
    #ifdef __ENABLE_3A__
        mp3AHal->sendCommand(ECmd_CaptureStart);
    #endif
    }
    else
    {
    #ifdef __ENABLE_3A__
        //(1) notify sw
        mp3AHal->sendCommand(ECmd_PrecaptureStart);

        //(2) stay in preview until 3A permits
        while ( ! mp3AHal->isReadyToCapture() )
        {
            CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_precap, CPTFlagSeparator, "precap_update");
            updateOne();
        }

        //(3) notify sw
        mp3AHal->sendCommand(ECmd_PrecaptureEnd);
    #endif
    }
    //(4) change state to precapture state
    IStateManager::inst()->transitState(IState::eState_PreCapture);
    //
    FUNCTION_OUT;
    //
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
update()
{
    // Loop: check if next command is comming
    // Next command can be {stop, precap}
    // Do at least 1 frame (in case of going to precapture directly)
    //  --> this works when AE updates in each frame (instead of in 3 frames)

    do{
        CPTLog(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagStart);
        //(1)
        updateOne();

        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "frameCnt(%d)",mFrameCnt);
        mFrameCnt++;

        //(2) check if need dalay
        updateCheck();
        CPTLog(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagEnd);

    } while( ! isNextCommand() );

    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
updateCheck()
{
    bool ret = false;

    //[T.B.D]
    //what if 'AWB and EFFECT mode change' are coming together?
    //only choose one delay? which one? larger one?
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "update Check");

    MY_LOGW_IF(::android_atomic_release_load(&mbAWBIndicator) &&
               ::android_atomic_release_load(&mbEFFECTIndicator),
               "AWB and effect mode are changing at the same time");

    if (::android_atomic_release_load(&mbAWBIndicator))
    {
        ret = delay(EQueryType_AWB);
        ::android_atomic_write(0, &mbAWBIndicator);
    }

    if (::android_atomic_release_load(&mbEFFECTIndicator))
    {
        ret = delay(EQueryType_Effect);
        ::android_atomic_write(0, &mbEFFECTIndicator);
    }

    //(2) BV value (3A --> AP)
#ifdef __ENABLE_3A__
    FrameOutputParam_T RTParams;
    mp3AHal->getRTParams(RTParams);
    int rt_BV = RTParams.i4BrightValue_x10;
    int rt_FPS = RTParams.u4FRameRate_x10;
    mspParamsMgr->updateBrightnessValue(rt_BV);
#endif
    
    return ret;
}


/******************************************************************************
*
*
*******************************************************************************/
bool
PreviewCmdQueThread::
updateOne()
{
    bool ret = true;
    int32_t pass1LatestBufIdx = -1;
    int64_t pass1LatestTimeStamp = 0;

    vector<IhwScenario::PortQTBufInfo> vDeBufPass1Out;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass1RawOut;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass2Out;
    vector<IhwScenario::PortBufInfo> vEnBufPass2In;
    vector<IhwScenario::PortBufInfo> vEnBufPass2Out;
    vector<IhwScenario::PortImgInfo> vPass2Cfg;

    list<CapBufQueNode> vDePass1RawOutNode;
    vector<IhwScenario::PortBufInfo> vDequedBuffer;
    vector<IhwScenario::PortBufInfo> vExchangedBuffer;

    int32_t pass1RawReadyCnt;

    ImgBufQueNode dispNode;
    ImgBufQueNode vidoNode;

#ifdef ISP_REG_DUMP_ENABLE
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpreg.enable", value, "0");
    int32_t newValue = atoi(value);
    static int32_t lastValue = 0;
    if (newValue != lastValue) {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "dump isp reg %d %d",newValue, lastValue );
        mSensorInfo.dumpIspReg();
        lastValue = newValue;
    }
#endif
    //*************************************************************
    // (1) [PASS 1] sensor ---> ISP --> DRAM(IMGO)
    //*************************************************************
    // (.1) get ready buffer from HW
    if ( ! getHw()->deque(eID_Pass1DispOut, &vDeBufPass1Out) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++)
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1DispOut, &vDeBufPass1Out))
            {
                MY_LOGD("success.");
                break;
            }
            else
            {
                MY_LOGE("still failed.");
            }
        }
        //
        if(i == tryCnt)
        {
            return false;
        }
    }
    //getHw()->deque(eID_Pass1DispOut, &vDeBufPass1Out);
    getHw()->deque(eID_Pass1RawOut, &vDeBufPass1RawOut);

    mapQT2BufInfo(eID_Pass1RawOut, eID_Pass1RawOut, vDeBufPass1RawOut, vDequedBuffer);
    //mapQT2Node(vDeBufPass1RawOut, mvDequedImgBufNode, vDePass1RawOutNode);
    pass1RawReadyCnt = vDequedBuffer.size();

    // (.2) enque ready buffer to capture provider
    for (MINT32 i =0; i< pass1RawReadyCnt; i++)
    {
        // enque
        //CapBufQueNode rNode;
        //rNode = vDePass1RawOutNode.front();
        mspCaptureBufHandler->enqueProvider(vDequedBuffer.at(i).virtAddr, true);
        list<CapBufQueNode>::iterator it;
        for (it = mvDequedImgBufNode.begin(); it != mvDequedImgBufNode.end(); ) {
            if ( (MUINT32)vDequedBuffer.at(i).virtAddr == (MUINT32)(*it).mainImgNode->getVirAddr()){
                it = mvDequedImgBufNode.erase(it);
            } else {
                 it++;
            }
        }
        //vDePass1RawOutNode.pop_front();

        // 1 replace 1
        CapBufQueNode rExNode;
        IhwScenario::PortBufInfo rExchangedBufInfo;
        mspCaptureBufHandler->dequeProvider(rExNode);
        mvDequedImgBufNode.push_back(rExNode);
        mapNode2BufInfo(eID_Pass1RawOut, rExNode.mainImgNode, rExchangedBufInfo);
        vExchangedBuffer.push_back(rExchangedBufInfo);

    }



    if ( ! mapQT2BufInfo(eID_Pass1DispOut, eID_Pass2In, vDeBufPass1Out, vEnBufPass2In) )
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "skip pass2");
        getHw()->enque(vDeBufPass1Out);

        // enque PASS1 raw
        getHw()->replaceQue(&vDequedBuffer, &vExchangedBuffer);

        return false;
    }
#ifdef __ENABLE_3A__
    mp3AHal->sendCommand(ECmd_Update);
#endif
    //
#if EIS_ENABLE
    mpEisHal->doEIS();
#endif
    //

    //*************************************************************
    //(2) [PASS 2] DRAM(IMGI) --> ISP --> CDP --> DRAM (DISPO, VIDO)
    //    if no buffer is available, return immediately.
    //*************************************************************
    int32_t flag = 0;


    //(.1) PASS2-IN
    //
    mCurZoomValue = getZoom();
    getCfg(eID_Pass2In , vPass2Cfg);

    //(.2) PASS2-OUT
    //
    //ImgBufQueNode dispNode;
    //ImgBufQueNode vidoNode;
    mspPreviewBufHandler->dequeBuffer(eID_Pass2DISPO, dispNode);
    mspPreviewBufHandler->dequeBuffer(eID_Pass2VIDO, vidoNode);
#ifdef __ENABLE_PASS2__

    if ( dispNode.getImgBuf() != 0)
    {
        flag |= eID_Pass2DISPO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2DISPO, dispNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2DISPO, dispNode, ImgInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
    }

    if ( vidoNode.getImgBuf() != 0)
    {
        flag = flag | eID_Pass2VIDO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2VIDO, vidoNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2VIDO, vidoNode, ImgInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
    }
#endif
    //(.3) no buffer ==> return immediately.
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2(0x%X)", flag);

    if ( ! flag )
    {
        getHw()->enque(vDeBufPass1Out);

        // enque PASS1 raw
        getHw()->replaceQue(&vDequedBuffer, &vExchangedBuffer);
        ret = false;
        goto lbExit;
    }

    //(.4) has buffer ==> do pass2 en/deque
    // Note: config must be set earlier than en/de-que
    //
    updateZoom(vPass2Cfg);
    getHw()->setConfig(&vPass2Cfg);


    getHw()->enque(&vEnBufPass2In, &vEnBufPass2Out);
    getHw()->deque((EHwBufIdx)flag, &vDeBufPass2Out);



    //*************************************************************
    // (3) return buffer
    //*************************************************************

    if( vDeBufPass1Out.size() > 0 &&
        vDeBufPass1Out[0].bufInfo.vBufInfo.size() > 0)
    {
         pass1LatestBufIdx = vDeBufPass1Out[0].bufInfo.vBufInfo.size()-1;
         pass1LatestTimeStamp = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].getTimeStamp_ns();
         //MY_LOGD("pass1LatestBufIdx(%d),pass1LatestTimeStamp(%lld)",pass1LatestBufIdx,pass1LatestTimeStamp);
    }

    // (.1) return PASS1
    getHw()->enque(vDeBufPass1Out);
    // enque PASS1 raw
    getHw()->replaceQue(&vDequedBuffer, &vExchangedBuffer);

    // (.2) return PASS2
    if (flag & eID_Pass2DISPO)
    {
        dispNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
        mspPreviewBufHandler->enqueBuffer(dispNode);
    }
    //
    if (flag & eID_Pass2VIDO)
    {
        vidoNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
        mspPreviewBufHandler->enqueBuffer(vidoNode);
    }
    //[T.B.D]
    //'0': "SUPPOSE" DISPO and VIDO gets the same timeStamp
    if( vDeBufPass2Out.size() > 1 )
    {
        MY_LOGW_IF(vDeBufPass2Out.at(0).bufInfo.getTimeStamp_ns() != vDeBufPass2Out.at(1).bufInfo.getTimeStamp_ns(),
        "DISP(%f),VIDO(%f)", vDeBufPass2Out.at(0).bufInfo.getTimeStamp_ns(), vDeBufPass2Out.at(1).bufInfo.getTimeStamp_ns());
    }
    //
    //
lbExit:

#ifdef DUMP
    if (ret)
    {
        dumpBuffer(vDeBufPass1Out, "pass1", "YUYV_Pack", mFrameCnt, mPorts.pass1DispOut.u4Width, mPorts.pass1DispOut.u4Height);
        //dumpBuffer(vDeBufPass1Out, "pass1", "raw", mFrameCnt);
        if (flag & eID_Pass2DISPO)
        {
            dumpImg(
                (MUINT8*)(dispNode.getImgBuf()->getVirAddr()),
                dispNode.getImgBuf()->getBufSize(),
                "pass2_dispo",
                "YUV420",
                mFrameCnt,
                dispNode.getImgBuf()->getImgWidth(),
                dispNode.getImgBuf()->getImgHeight());
        }
        if (flag & eID_Pass2VIDO)
        {
            dumpImg(
                (MUINT8*)(vidoNode.getImgBuf()->getVirAddr()),
                vidoNode.getImgBuf()->getBufSize(),
                "pass2_vido",
                "NV21",
                mFrameCnt,
                vidoNode.getImgBuf()->getImgWidth(),
                vidoNode.getImgBuf()->getImgHeight());
        }
    }
#endif

    return ret;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
updateZoom(vector<IhwScenario::PortImgInfo> &rImgIn)
{
    //   (1) calculate zoom
    //   by  src (from sensor output, or for video it's pass 1 out)
    //   and dst (preview size)
    int32_t PrvWidth = 0;
    int32_t PrvHeight = 0;
    //
    getParamsMgr()->getPreviewSize(&PrvWidth, &PrvHeight);

#if 0
    IhwScenario::Rect_t Src(mPorts.pass2In.u4Width, mPorts.pass2In.u4Height);
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, mCurZoomValue);
#ifdef __ENABLE_3A__
    //   (2) set to 3A
    mp3AHal->setZoom(mCurZoomValue, Crop.x, Crop.y, Crop.w, Crop.h);
#endif

#else
    //pass1 dispo out is "not" uniformly scaled to reduce memory usage.
    //Need to re-calculate the crop
    IhwScenario::Rect_t Src(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, mCurZoomValue);

#ifdef __ENABLE_3A__
    //   (2) set to 3A
    //   crop info is relative to the raw dump
    mp3AHal->setZoom(mCurZoomValue, Crop.x, Crop.y, Crop.w, Crop.h);
#endif

    Crop.w = ( (Crop.w * mPorts.pass2In.u4Width + (Src.w >> 1)) / Src.w ) & (~0x1);
    Crop.h = ( (Crop.h * mPorts.pass2In.u4Height + (Src.h >> 1)) / Src.h ) & (~0x1);
    Crop.x = (mPorts.pass2In.u4Width - Crop.w)/2;
    Crop.y = (mPorts.pass2In.u4Height - Crop.h)/2;

#endif

    //   (3) set to hw config
    rImgIn.at(0).crop = Crop;
}


/*******************************************************************************
*
********************************************************************************/
IhwScenario::Rect_t
PreviewCmdQueThread::
doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio)
{
    if (ratio < 100) {
        MY_LOGW("Attempt (%d) < min zoom(%d)" , ratio, 100);
        ratio = 100;
    }
    if (ratio > 800) {
        MY_LOGW("Attempt (%d) > max zoom(%d)" , ratio, 800);
        ratio = 800;
    }

    IhwScenario::Rect_t rCrop = calCrop(rSrc, rDst, ratio);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "S(%d/%d),D(%d/%d),Z(%d),C(%d,%d,%d,%d)",
                rSrc.w, rSrc.h,
                rDst.w, rDst.h,
                ratio,
                rCrop.x, rCrop.y, rCrop.w, rCrop.h);

    return rCrop;
}


/*******************************************************************************
*
********************************************************************************/
bool
PreviewCmdQueThread::
dropFrame()
{
    bool ret = true;

    vector<IhwScenario::PortQTBufInfo> dummy; 

/*
    if ( ! getHw()->deque(eID_Pass1DispOut, &dummy) )
    {
        MY_LOGE("drop frame failed");
        ret = false;
    }
*/
    if ( ! getHw()->deque(eID_Pass1DispOut, &dummy) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++)
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1DispOut, &dummy))
            {
                MY_LOGD("success.");
                break;
            }
            else
            {
                MY_LOGE("still failed.");
            }
        }
        //
        if(i == tryCnt)
        {
            return false;
        }
    }
    //
    getHw()->enque(dummy);
    mp3AHal->sendCommand(ECmd_Update);         
    vector<IhwScenario::PortQTBufInfo> raw_dummy;
    getHw()->deque(eID_Pass1RawOut, &raw_dummy);
    getHw()->enque(raw_dummy);
    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo)
{
#if 0
    if (port & eID_Pass1In)
    {
        IhwScenario::PortImgInfo imgInfoIn(
            eID_Pass1In,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getSensorWidth(),
            mSensorInfo.getSensorHeight()
        );
        imgInfoIn.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        //
        rvImginfo.push_back(imgInfoIn);
    }

    if (port & eID_Pass1DispOut)
    {
        IhwScenario::PortImgInfo imgInfoOut(
            eID_Pass1DispOut,
            "yuv422sp",
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoOut.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        IhwScenario::Rect_t SensorSize(mSensorInfo.getSensorWidth(), mSensorInfo.getSensorHeight());
        imgInfoOut.crop = calCrop(SensorSize, imgInfoOut.crop, 100);
        //
        rvImginfo.push_back(imgInfoOut);
    }

    // zsd added:
    if (port & eID_Pass1RawOut)
    {
        IhwScenario::PortImgInfo imgInfoRawOut(
            eID_Pass1RawOut,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoRawOut.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        IhwScenario::Rect_t SensorSize(mSensorInfo.getSensorWidth(), mSensorInfo.getSensorHeight());
        imgInfoRawOut.crop = calCrop(SensorSize, imgInfoRawOut.crop, 100);
        //
        rvImginfo.push_back(imgInfoRawOut);
    }

    if (port & eID_Pass2In)
    {
        IhwScenario::PortImgInfo imgInfoIn2(
            eID_Pass2In,
            "yuv422sp",
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoIn2.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        //
        rvImginfo.push_back(imgInfoIn2);
    }
#else
    if (port & eID_Pass1In)
    {
        //
        rvImginfo.push_back(mPorts.pass1In);
    }

    if (port & eID_Pass1DispOut)
    {
        //
        rvImginfo.push_back(mPorts.pass1DispOut);
    }

    if (port & eID_Pass1RawOut)
    {
        //
        rvImginfo.push_back(mPorts.pass1RawOut);
    }

    if (port & eID_Pass2In)
    {
        //
        rvImginfo.push_back(mPorts.pass2In);
    }

#endif
}

/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
initCfg()
{
    IhwScenario::Rect_t SensorSize(mSensorInfo.getSensorWidth(), mSensorInfo.getSensorHeight());

    int32_t i4PreviewWidth = 0, i4PreviewHeight = 0;
    if  ( ! queryPreviewSize(i4PreviewWidth, i4PreviewHeight) )
    {
        MY_LOGE("queryPreviewSize");
        return false;
    }


    //IhwScenario::Rect_t DispOutSize(i4PreviewWidth, 600);

    mPorts.pass1In.ePortIdx = eID_Pass1In;
    mPorts.pass1In.sFormat = mSensorInfo.getImgFormat();
    mPorts.pass1In.u4Width = mSensorInfo.getSensorWidth();
    mPorts.pass1In.u4Height = mSensorInfo.getSensorHeight();
    mPorts.pass1In.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();

    mPorts.pass1RawOut.ePortIdx = eID_Pass1RawOut;
    mPorts.pass1RawOut.sFormat = mSensorInfo.getImgFormat();
    mPorts.pass1RawOut.u4Width = mSensorInfo.getSensorWidth();
    mPorts.pass1RawOut.u4Height = mSensorInfo.getSensorHeight();
    mPorts.pass1RawOut.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
    mPorts.pass1RawOut.crop = calCrop(SensorSize, mPorts.pass1RawOut.crop, 100);


    mPorts.pass1DispOut.ePortIdx = eID_Pass1DispOut;
    mPorts.pass1DispOut.sFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I;

    if(mPorts.pass1RawOut.u4Width* i4PreviewHeight > mPorts.pass1RawOut.u4Height* i4PreviewWidth )
    {
        mPorts.pass1DispOut.u4Width = ((i4PreviewHeight *mPorts. pass1RawOut.u4Width + (mPorts. pass1RawOut.u4Height>>1))/mPorts. pass1RawOut.u4Height) & (~0x1);
        mPorts.pass1DispOut.u4Height = i4PreviewHeight;
    }else{
        mPorts.pass1DispOut.u4Width = i4PreviewWidth;
        mPorts.pass1DispOut.u4Height = ((i4PreviewWidth*mPorts. pass1RawOut.u4Height+(mPorts. pass1RawOut.u4Width>>1))/mPorts. pass1RawOut.u4Width) & (~0x1);
    }

    //mt6582 cdrz hw limit: h:1/32x~, v:1/2x~
    if( mPorts.pass1DispOut.u4Width * 32 < mPorts.pass1In.u4Width )
    {
        mPorts.pass1DispOut.u4Width = mPorts.pass1In.u4Width >> 5;
    }
    if( mPorts.pass1DispOut.u4Height * 2 < mPorts.pass1In.u4Height )
    {
        mPorts.pass1DispOut.u4Height = mPorts.pass1In.u4Height >> 1;
    }
    mPorts.pass1DispOut.u4Stride[ESTRIDE_1ST_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass1DispOut.sFormat, mPorts.pass1DispOut.u4Width, ESTRIDE_1ST_PLANE);
    mPorts.pass1DispOut.u4Stride[ESTRIDE_2ND_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass1DispOut.sFormat, mPorts.pass1DispOut.u4Width, ESTRIDE_2ND_PLANE);
    mPorts.pass1DispOut.u4Stride[ESTRIDE_3RD_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass1DispOut.sFormat, mPorts.pass1DispOut.u4Width, ESTRIDE_3RD_PLANE);
    IhwScenario::Rect_t DispOutSize(mPorts.pass1DispOut.u4Width, mPorts.pass1DispOut.u4Height);
    mPorts.pass1DispOut.crop = calCrop(DispOutSize, mPorts.pass1DispOut.crop, 100);

    mPorts.pass2In.ePortIdx = eID_Pass2In;
    mPorts.pass2In.sFormat = mPorts.pass1DispOut.sFormat;
    mPorts.pass2In.u4Width = mPorts.pass1DispOut.u4Width;
    mPorts.pass2In.u4Height = mPorts.pass1DispOut.u4Height;
    mPorts.pass2In.u4Stride[ESTRIDE_1ST_PLANE] = mPorts.pass1DispOut.u4Stride[ESTRIDE_1ST_PLANE];
    mPorts.pass2In.u4Stride[ESTRIDE_2ND_PLANE] = mPorts.pass1DispOut.u4Stride[ESTRIDE_2ND_PLANE];
    mPorts.pass2In.u4Stride[ESTRIDE_3RD_PLANE] = mPorts.pass1DispOut.u4Stride[ESTRIDE_3RD_PLANE];

    dumpCfg();

    return true;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
dumpCfg()
{
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "p1In: %s@%dx%d",
               mPorts.pass1In.sFormat,
               mPorts.pass1In.u4Width,
               mPorts.pass1In.u4Height);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "p1RawOut: %s@%dx%d crop(%d,%d,%d,%d)",
               mPorts.pass1RawOut.sFormat,
               mPorts.pass1RawOut.u4Width,
               mPorts.pass1RawOut.u4Height,
               mPorts.pass1RawOut.crop.x,
               mPorts.pass1RawOut.crop.y,
               mPorts.pass1RawOut.crop.w,
               mPorts.pass1RawOut.crop.h);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "p1DispOut: %s@%dx%d crop(%d,%d,%d,%d)",
               mPorts.pass1DispOut.sFormat,
               mPorts.pass1DispOut.u4Width,
               mPorts.pass1DispOut.u4Height,
               mPorts.pass1DispOut.crop.x,
               mPorts.pass1DispOut.crop.y,
               mPorts.pass1DispOut.crop.w,
               mPorts.pass1DispOut.crop.h);

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "p2In: %s@%dx%d",
               mPorts.pass2In.sFormat,
               mPorts.pass2In.u4Width,
               mPorts.pass2In.u4Height);

}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread*
PreviewCmdQueThread::
getInstance(sp<IPreviewBufMgrHandler> pPHandler, sp<ICaptureBufMgrHandler> pCHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    return  new PreviewCmdQueThread(pPHandler, pCHandler, rSensorid, pParamsMgr);
}


/******************************************************************************
*
*******************************************************************************/
IPreviewCmdQueThread*
IPreviewCmdQueThread::
createInstance(sp<IPreviewBufMgrHandler> pPHandler, sp<ICaptureBufMgrHandler> pCHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    if  ( pPHandler != 0 &&  pCHandler != 0) {
        return  PreviewCmdQueThread::getInstance(pPHandler, pCHandler, rSensorid, pParamsMgr);
    }

    MY_LOGE("pHandler==NULL");
    return  NULL;
}


/*******************************************************************************
*
********************************************************************************/
static
IhwScenario::Rect_t
calCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio)
{
#if 0
    IhwScenario::Rect_t rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.w * rDst.h < rDst.w * rSrc.h) {
        rCrop.w = rSrc.w;
        rCrop.h = rSrc.w * rDst.h / rDst.w;
    }
    //srcW/srcH > dstW/dstH
    else if (rSrc.w * rDst.h > rDst.w * rSrc.h) {
        rCrop.w = rSrc.h * rDst.w / rDst.h;
        rCrop.h = rSrc.h;
    }
    else {
        rCrop.w = rSrc.w;
        rCrop.h = rSrc.h;
    }
    //
    rCrop.w =  ROUND_TO_2X(rCrop.w * 100 / ratio);
    rCrop.h =  ROUND_TO_2X(rCrop.h * 100 / ratio);
    //
    rCrop.x = (rSrc.w - rCrop.w) / 2;
    rCrop.y = (rSrc.h - rCrop.h) / 2;
#else
    NSCamHW::Rect rHWSrc(rSrc.x, rSrc.y, rSrc.w, rSrc.h);
    NSCamHW::Rect rHWDst(rDst.x, rDst.y, rDst.w, rDst.h);
    NSCamHW::Rect rHWCrop = MtkCamUtils::calCrop(rHWSrc, rHWDst, ratio);

    IhwScenario::Rect_t rCrop(rHWCrop.w, rHWCrop.h, rHWCrop.x, rHWCrop.y );
#endif

    return rCrop;
}

#define ZSD_DUMP_PATH "/sdcard/zsd/"

/******************************************************************************
*
*******************************************************************************/
static
bool
dumpBuffer(
    vector<IhwScenario::PortQTBufInfo> &src,
    char const*const tag,
    char const * const filetype,
    uint32_t filenum,
    uint32_t width,
    uint32_t height)
{
#ifdef DUMP
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);
    if (enable == 0)
    {
        return false;
    }
#endif

    for (MUINT32 i = 0; i < src.size(); i++)
    {
        if ( ! src.at(i).bufInfo.vBufInfo.size() )
        {
            MY_LOGE("(%s) src.at(%d).bufInfo.vBufInfo.size() = 0", tag, i);
            continue;
        }

        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "addr: 0x%x, size: %d, time: %f",
           src.at(i).bufInfo.vBufInfo.at(0).u4BufVA,
           src.at(i).bufInfo.vBufInfo.at(0).u4BufSize,
           src.at(i).bufInfo.getTimeStamp_ns());

        if (!dumpImg((MUINT8*)src.at(i).bufInfo.vBufInfo.at(0).u4BufVA,
                  src.at(i).bufInfo.vBufInfo.at(0).u4BufSize,
                  tag, filetype, filenum,  width, height))
        {
            MY_LOGE("Dump buffer fail");
        }
    }
#endif

    return true;
}

/******************************************************************************
*
*******************************************************************************/
static
bool
dumpImg(
    MUINT8 *addr,
    MUINT32 size,
    char const * const tag,
    char const * const filetype,
    uint32_t filenum,
    MUINT32 width,
    MUINT32 height)
{
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);
    if (enable == 0)
    {
        return false;
    }
    MtkCamUtils::makePath(ZSD_DUMP_PATH, 0660);
#endif
    //
    char fileName[64];
    sprintf(fileName, ZSD_DUMP_PATH"%s_%d_[%s-%dx%d].bin", tag, filenum, filetype,width, height);
    FILE *fp = fopen(fileName, "w");
    if (NULL == fp)
    {
        MY_LOGE("fail to open file to save img: %s", fileName);
        return false;
    }

    fwrite(addr, 1, size, fp);
    fclose(fp);

    return true;
}


/******************************************************************************
*
*******************************************************************************/
static
bool
mapQT2BufInfo(EHwBufIdx eInPort, EHwBufIdx eOutPort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst)
{
    if ( src.size() <= 0 ) {
        MY_LOGE("vector size is 0!");
        return false;
    }

    vector< IhwScenario::PortQTBufInfo >::const_iterator it;
    bool isFindBuffer = false;
    for (it = src.begin(); it != src.end(); it++ ) {
        if ((*it).ePortIndex == eInPort) {
            MY_LOGD_IF(0, "[map] find pass 1 out buffer");
            isFindBuffer = true;
            break;
        }
    }
    if ( !isFindBuffer ) {
         MY_LOGE("[map] did not find pass 1 out buffer");
         return false;
    }

    if ( (*it).bufInfo.vBufInfo.empty() ) {
        MY_LOGE("Pass 1 buffer is 0!");
        return false;
    }

    int latest = (*it).bufInfo.vBufInfo.size()-1;
    MY_LOGD_IF(latest > 0, "deque size: %d", latest+1);

    IhwScenario::PortBufInfo one(
                     eOutPort,
                     (*it).bufInfo.vBufInfo.at(latest).u4BufVA,
                     (*it).bufInfo.vBufInfo.at(latest).u4BufPA,
                     (*it).bufInfo.vBufInfo.at(latest).u4BufSize,
                     (*it).bufInfo.vBufInfo.at(latest).memID
    );

    dst.push_back(one);

    MY_LOGD_IF(0, "VA(0x%08X),S(%d),Idx(%d),Id(%d)",
             one.virtAddr, one.bufSize, one.ePortIndex, one.memID);

    return true;
}


/******************************************************************************
*
*******************************************************************************/
static void
mapNode2BufInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortBufInfo &dst)
{
    dst.virtAddr   = (MUINT32)src.getImgBuf()->getVirAddr();
    dst.phyAddr    = (MUINT32)src.getImgBuf()->getPhyAddr();
    dst.bufSize    = src.getImgBuf()->getBufSize();
    dst.ePortIndex = idx;
    dst.memID      = src.getImgBuf()->getIonFd();
    MY_LOGD_IF(0, "VA(0x%08X),S(%d),Idx(%d),Id(%d)",
                   dst.virtAddr, dst.bufSize, dst.ePortIndex, dst.memID);
}


/******************************************************************************
*
*******************************************************************************/
static void
mapNode2ImgInfo(EHwBufIdx const &idx, ImgBufQueNode const &src, IhwScenario::PortImgInfo &dst)
{
    dst.ePortIdx = idx;
    dst.sFormat  = src.getImgBuf()->getImgFormat().string();
    dst.u4Width  = src.getImgBuf()->getImgWidth();
    dst.u4Height = src.getImgBuf()->getImgHeight();
    dst.u4Stride[ESTRIDE_1ST_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_1ST_PLANE);
    dst.u4Stride[ESTRIDE_2ND_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_2ND_PLANE);
    dst.u4Stride[ESTRIDE_3RD_PLANE] = src.getImgBuf()->getImgWidthStride(ESTRIDE_3RD_PLANE);

    //[T.B.D]
    dst.eRotate  = src.getRotation() == 0 ? eImgRot_0
                 : src.getRotation() == 90 ? eImgRot_90
                 : src.getRotation() == 180 ? eImgRot_180 : eImgRot_270;

    dst.eFlip    = eImgFlip_OFF;
    //
    MY_LOGD_IF(0, "Port(%d),F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
                   dst.ePortIdx, dst.sFormat, dst.u4Width, dst.u4Height,
                   dst.u4Stride[ESTRIDE_1ST_PLANE], dst.u4Stride[ESTRIDE_2ND_PLANE], dst.u4Stride[ESTRIDE_3RD_PLANE], dst.eRotate);
}
/******************************************************************************
*
*******************************************************************************/
static bool
mapQT2Node(         vector<IhwScenario::PortQTBufInfo> &vQTBuf,
                    list<CapBufQueNode> &vInNode,
                    list<CapBufQueNode> &vOutNode)
{
    if ( vQTBuf.size() <= 0 )
    {
        MY_LOGE("vector size is 0!");
        return false;
    }

    if ( vQTBuf.at(0).ePortIndex != eID_Pass1RawOut )
    {
        MY_LOGE("It is not raw out port!");
        return false;
    }

    if ( vQTBuf.at(0).bufInfo.vBufInfo.size() <= 0 )
    {
        MY_LOGE("no raw out buf!");
        return false;
    }

    MY_LOGD_IF(0, "raw cnt(%d)", vQTBuf.at(0).bufInfo.vBufInfo.size());

    list< CapBufQueNode >::iterator it;
    MINT32 size = vQTBuf.at(0).bufInfo.vBufInfo.size();
    for (MINT32 i =0; i < size; i++)
    {
        if (vInNode.empty())
        {
            MY_LOGE("fail to map, vInNode = 0");
            return false;
        }
        for (it = vInNode.begin(); it != vInNode.end(); it++ )
        {
            if ( vQTBuf.at(0).bufInfo.vBufInfo[i].u4BufVA == (MUINT32)(*it).mainImgNode->getVirAddr())
            {
                MY_LOGD_IF(0, "map QT->Node (0x%x)", vQTBuf.at(0).bufInfo.vBufInfo[i].u4BufVA);
                CapBufQueNode rNode = (*it);
                vInNode.erase(it);
                vOutNode.push_back(rNode);
                break;
            }
        }
    }
    return true;


}
