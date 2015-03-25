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

#define LOG_TAG "MtkCam/ZSDCCPrvCQT"

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
using namespace std;
//
#include <inc/IState.h>
#include <inc/PreviewCmdQueThread.h>
#include <inc/IPreviewBufMgr.h>
using namespace android::NSMtkZsdCcCamAdapter;
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
static bool mapQT2BufInfo(EHwBufIdx ePort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst);
static bool dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, uint32_t filenum, uint32_t width, uint32_t height);
static bool dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, uint32_t filenum, uint32_t width, uint32_t height);
/******************************************************************************
*
*******************************************************************************/

namespace android {
namespace NSMtkZsdCcCamAdapter {

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
        // zsd to do: ZSDCC, <8M@24, no need to check this size.
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
//#define PASS2YUVBUFCNT   (STORED_BUFFER_CNT>3?STORED_BUFFER_CNT:3)

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
    bool                captureUpdate();
    bool                updateOne();
    bool                updateCheck();
    bool                stop();
    bool                precap();
    void                updateZoom(vector<IhwScenario::PortImgInfo> &pImgIn);
    uint32_t            getZoom();
    bool                dropFrame();
    IhwScenario::Rect_t doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio = 100);
    void                getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo);
    bool                initCfg(void);
    bool                queryThumbnailSize(int32_t& ri4Width, int32_t& ri4Height);
    bool                queryPreviewSize(int32_t& ri4Width, int32_t& ri4Height);
    bool                queryCaptureSize(       int32_t& ri4CapWidth, int32_t& ri4CapHeight, int32_t& ri4Rotation);

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
    //
    EisHalBase*                 mpEisHal;
    struct sPorts{
        IhwScenario::PortImgInfo pass1In;
        IhwScenario::PortImgInfo pass1Out;
        IhwScenario::PortImgInfo pass2In;
        IhwScenario::PortImgInfo pass2YuvOut;
        IhwScenario::PortImgInfo pass2DumyPVOut; // Post view, use SW convertor, dump port
    };
    sPorts mPorts;
    FmtUtils::CamFormatTransform mConvertor;

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
    ::prctl(PR_SET_NAME,"ZsdCcPreviewCmdQueThread", 0, 0, 0);

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
		    case PrvCmdCookie::eCaptureUpdate:
				isvalid = captureUpdate();
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
    int pass2YUVBufCnt = 0;


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
    mpHwScenario = IhwScenario::createInstance(eHW_VSS, mSensorInfo.getSensorType(),
                                               mSensorInfo.meSensorDev,
                                               mSensorInfo.mSensorBitOrder);
    if(mpHwScenario != NULL)
    {
        if(!(mpHwScenario->init()))
        {
            MY_LOGE("init Hw Scenario fail!!");
            goto lbExit;
        }
    }
    else
    {
        MY_LOGE("mpHwScenario is NULL!!");
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

    getCfg(eID_Pass1In|eID_Pass1Out, vimgInfo);
    getHw()->setConfig(&vimgInfo);
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "alloc preview buffer");

    // (2.2) enque pass 1 buffer
    //     must do this earlier than hw start
    mspPreviewBufHandler->allocBuffer(
                          mSensorInfo.getImgWidth(),
                          mSensorInfo.getImgHeight(),
                          mSensorInfo.getImgFormat(),
                          PASS1BUFCNT);
    for (int32_t i = 0; i < PASS1BUFCNT; i++)
    {
        mspPreviewBufHandler->dequeBuffer(eID_Pass1Out, Pass1Node);
        mapNode2BufInfo(eID_Pass1Out, Pass1Node, BufInfo);
        vBufPass1Out.push_back(BufInfo);
    }
    getHw()->enque(NULL, &vBufPass1Out);
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "alloc capture buffer");

    // (2.3) allocate capture buffer
    storedBufCnt = get_zsd_cap_stored_frame_cnt();
    if (storedBufCnt < 0)
    {
        MY_LOGE("Query ZSD buffer cnt error");
        goto lbExit;
    }
    pass2YUVBufCnt = storedBufCnt > 3 ? storedBufCnt : 3;
    MY_LOGD("stored buffer:%d pass2 YUV buffer: %d",storedBufCnt, pass2YUVBufCnt );

    mspCaptureBufHandler->allocBuffer(
                      mPorts.pass2YuvOut.u4Width,
                      mPorts.pass2YuvOut.u4Height,
                      mPorts.pass2YuvOut.sFormat,
                      mPorts.pass2YuvOut.eRotate,
                      mPorts.pass2DumyPVOut.u4Width,
                      mPorts.pass2DumyPVOut.u4Height,
                      mPorts.pass2DumyPVOut.sFormat,
                      pass2YUVBufCnt);
    mspCaptureBufHandler->setStoredBufferCnt(storedBufCnt);

    //(3) 3A
    //!! must be set after hw->enque; otherwise, over-exposure.
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init 3A");
    //
    mp3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId));
    if ( ! mp3AHal )
    {
        MY_LOGE("init 3A fail!!");
        goto lbExit;
    }

    mp3AHal->setZoom(100, 0, 0, mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    mp3AHal->setIspProfile(EIspProfile_ZsdPreview_CC);
    mp3AHal->sendCommand(ECmd_CameraPreviewStart);


    // (4) EIS
    //
#if EIS_ENABLE
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_start_init, CPTFlagSeparator, "Init EIS");
    mpEisHal = EisHalBase::createInstance("mtkzsdccAdapter");
    if(mpEisHal != NULL)
    {
        eisHal_config_t eisHalConfig;
        eisHalConfig.imageWidth = mSensorInfo.getImgWidth();
        eisHalConfig.imageHeight = mSensorInfo.getImgHeight();
        mpEisHal->configEIS(
                    eHW_VSS,
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
        }
        break;
        
        case EQueryType_Effect:
        {
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
        }

        case EQueryType_AWB:
        {
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
    
    //(3) Zoom
    setZoom(getParamsMgr()->getZoomRatio());

    //
    p3AHal->destroyInstance();

    //
    FUNCTION_OUT;
    
    return true;
}


/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
queryThumbnailSize(int32_t& ri4Width, int32_t& ri4Height)
{
    ri4Width     = getParamsMgr()->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
    ri4Height    = getParamsMgr()->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);

    MY_LOGD("Thumb nail size(%d %d)",ri4Width, ri4Height);
    return  true;
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
queryCaptureSize(       int32_t& ri4CapWidth, int32_t& ri4CapHeight, int32_t& ri4Rotation)
{
    static int32_t lastRoation = 0;
    static int32_t lastWidth = 0;
    static int32_t lastHeight = 0;

    if (mShotMode == eShotMode_ContinuousShotCc)
    {
        ri4CapWidth = lastWidth;
        ri4CapHeight = lastHeight;
        ri4Rotation = lastRoation;

        MY_LOGD("Capture size (%d,%d,%d), ContinuousShotCc not update",ri4CapWidth, ri4CapHeight, ri4Rotation);
        return  true;
    }


    getParamsMgr()->getPictureSize(&ri4CapWidth, &ri4CapHeight);
    ri4Rotation = getParamsMgr()->getInt(CameraParameters::KEY_ROTATION);

    lastWidth = ri4CapWidth;
    lastHeight = ri4CapHeight;
    lastRoation = ri4Rotation;

    MY_LOGD("Capture size (%d,%d,%d)",ri4CapWidth, ri4CapHeight, ri4Rotation);
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
    if ( mp3AHal != NULL )
    {
        mp3AHal->sendCommand(ECmd_CameraPreviewEnd);
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }

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

    if (mShotMode != eShotMode_ZsdShot && mShotMode != eShotMode_ContinuousShotCc)
    {
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
        mpEisHal->destroyInstance("mtkzsdccAdapter");
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
    bFlashOn = mp3AHal->isNeedFiringFlash();

    MY_LOGD("flash %s", bFlashOn == 1?"ON":"OFF");

    // ZSD shot only works when normal shot with strobe off
    if ( bFlashOn == 0 && mShotMode == eShotMode_NormalShot ) {
        mShotMode = eShotMode_ZsdShot;
    }

    // C-Chot CC only works when strobe off
    if ( bFlashOn == 0 && mShotMode == eShotMode_ContinuousShot ) {
        mShotMode = eShotMode_ContinuousShotCc;
    }

    if (mShotMode != eShotMode_ZsdShot && mShotMode != eShotMode_ContinuousShotCc) {
        //(1) notify sw
        mp3AHal->sendCommand(ECmd_PrecaptureStart);

        //(2) stay in preview until 3A permits
        while ( ! mp3AHal->isReadyToCapture() )
        {
             CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_precap, CPTFlagSeparator, "precap_update");
             updateOne();
             MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "frameCnt(%d)",mFrameCnt);
             mFrameCnt++;
        }

        //(3) notify sw
        mp3AHal->sendCommand(ECmd_PrecaptureEnd);
    }
    //(4) change state to precapture state
    IStateManager::inst()->transitState(IState::eState_PreCapture);
    //
    FUNCTION_OUT;
    //
    return true;
}

bool
PreviewCmdQueThread::
captureUpdate()
{
    if ((mShotMode == eShotMode_ZsdShot || mShotMode == eShotMode_ContinuousShotCc)) {
        do{
            CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagStart, "captureUpdate");
            //(1)
            updateOne();

            MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "frameCnt(%d)",mFrameCnt);
            mFrameCnt++;

            //(2) check if need dalay
            updateCheck();
            CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagEnd, "captureUpdate");

        } while( ! isNextCommand() );

        if (mShotMode == eShotMode_ContinuousShotCc) {

            MY_LOGD("capture done, reset mShotmode");
            mShotMode = 0;
        }
    }

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
    FrameOutputParam_T RTParams;
    mp3AHal->getRTParams(RTParams);
    int rt_BV = RTParams.i4BrightValue_x10;
    int rt_FPS = RTParams.u4FRameRate_x10;
    mspParamsMgr->updateBrightnessValue(rt_BV);
    
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
    bool isGenaric = false;

    vector<IhwScenario::PortQTBufInfo> vDeBufPass1Out;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass2Out;
    vector<IhwScenario::PortBufInfo> vEnBufPass2In;
    vector<IhwScenario::PortBufInfo> vEnBufPass2Out;
    vector<IhwScenario::PortImgInfo> vPass2Cfg;

    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "deque PASS1");

    //*************************************************************
    // (1) [PASS 1] sensor ---> ISP --> DRAM(IMGO)
    //*************************************************************
    if ( ! getHw()->deque(eID_Pass1Out, &vDeBufPass1Out) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++) 
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1Out, &vDeBufPass1Out))
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

    mapQT2BufInfo(eID_Pass2In, vDeBufPass1Out, vEnBufPass2In);
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "3A update");
    mp3AHal->sendCommand(ECmd_Update);

    FeatureParam_T rFeatureParam;
    mp3AHal->getSupportedParams(rFeatureParam);
    uint32_t u4FocusValH = rFeatureParam.i8BSSVlu >> 32;
    uint32_t u4FocusValL = (MUINT32)(rFeatureParam.i8BSSVlu);
    MY_LOGD("Focus value(i8BSSVlu, u4FocusValH, u4FocusValL) = (%lld, %d, %d)", rFeatureParam.i8BSSVlu, u4FocusValH, u4FocusValL); 

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
    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "prepare PASS2 Buffer");
    ImgBufQueNode dispNode;
    ImgBufQueNode vidoNode;
    CapBufQueNode capNode;
    mspPreviewBufHandler->dequeBuffer(eID_Pass2DISPO, dispNode);
    mspPreviewBufHandler->dequeBuffer(eID_Pass2VIDO, vidoNode);
    mspCaptureBufHandler->dequeProvider(capNode);

    CPTLogStrEx(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator,(MUINT32)capNode.mainImgNode->getVirAddr(),0, "dequeProvider");
/*
    updateCfg();
    if (mPorts.pass2YuvOut.u4Width != capNode.mainImgNode->getImgWidth() ||
        mPorts.pass2YuvOut.u4Height != capNode.mainImgNode->getImgHeight())
    {
        mspCaptureBufHandler->reallocBuffer(
                        mPorts.pass2YuvOut.u4Width,
                        mPorts.pass2YuvOut.u4Height,
                        mPorts.pass2YuvOut.sFormat,
                        mPorts.pass2DispOut.u4Width,
                        mPorts.pass2DispOut.u4Height,
                        mPorts.pass2DispOut.sFormat,
                        PASS2YUVBUFCNT);
        mspCaptureBufHandler->setStoredBufferCnt(STORED_BUFFER_CNT);
        mspCaptureBufHandler->dequeProvider(capNode);
    }
    */


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

    // Generic is for Panorama or MAV
    // it only works in preview mode
    // thus no need to genarate capture buffer.
    if ( vidoNode.getImgBuf() != 0 && IPreviewBufMgr::eBuf_Generic == vidoNode.getCookieDE())
    {
        flag = flag | eID_Pass2VIDO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2VIDO, vidoNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2VIDO, vidoNode, ImgInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
        isGenaric = true;
    }
    else if ( capNode.mainImgNode.getImgBuf() != 0)
    {
        int32_t i4CaptureWidth = 0, i4CaptureHeight = 0, i4Rotation = 0;
        queryCaptureSize(i4CaptureWidth, i4CaptureHeight, i4Rotation);

        capNode.mainImgNode.setRotation(i4Rotation);
        CapBuffer* capBuf = reinterpret_cast<CapBuffer*> (capNode.mainImgNode.getImgBuf().get());
        capBuf->update(i4CaptureWidth, i4CaptureHeight, i4Rotation);


        flag = flag | eID_Pass2VIDO;
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2VIDO, capNode.mainImgNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2VIDO, capNode.mainImgNode, ImgInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo);
    }

    //(.3) no buffer ==> return immediately.
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "P2(0x%X)", flag);

    if ( ! flag )
    {
        getHw()->enque(vDeBufPass1Out);
        ret = false;
        goto lbExit;
    }

    //(.4) has buffer ==> do pass2 en/deque
    // Note: config must be set earlier than en/de-que
    //
    updateZoom(vPass2Cfg);
    getHw()->setConfig(&vPass2Cfg);


    getHw()->enque(&vEnBufPass2In, &vEnBufPass2Out);


    CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "deque PASS2");
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

    // (.2) return PASS2
    if (flag & eID_Pass2DISPO)
    {
        dispNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);

        if (!isGenaric && capNode.subImgNode.getImgBuf() != 0) {

            CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "SubImgNode");
            //memcpy(capNode.subImgNode->getVirAddr(),dispNode->getVirAddr(),dispNode->getBufSize());
#if 1
            // 1. sub image node
            mConvertor.setSrc(dispNode->getImgFormat(),
                             (unsigned char*)dispNode->getVirAddr(),
                             dispNode->getBufSize(),
                             dispNode->getImgWidth(),
                             dispNode->getImgHeight(),
                             dispNode->getImgWidthStride(0),
                             dispNode->getImgWidthStride(1),
                             dispNode->getImgWidthStride(2));

            mConvertor.setDst(capNode.subImgNode->getImgFormat(),
                             (unsigned char*)capNode.subImgNode->getVirAddr(),
                             capNode.subImgNode->getBufSize(),
                             capNode.subImgNode->getImgWidth(),
                             capNode.subImgNode->getImgHeight(),
                             capNode.subImgNode->getImgWidthStride(0),
                             capNode.subImgNode->getImgWidthStride(1),
                             capNode.subImgNode->getImgWidthStride(2));
            MY_LOGD_IF(0, "Sub %s@%dx%d -> %s@%dx%d",
                       dispNode->getImgFormat().string(),
                       dispNode->getImgWidth(),
                       dispNode->getImgHeight(),
                       capNode.subImgNode->getImgFormat().string(),
                       capNode.subImgNode->getImgWidth(),
                       capNode.subImgNode->getImgHeight());
            mConvertor.convert();
            // 2. FD buffer
            CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "FD buffer");
            if (vidoNode.getImgBuf() != 0)
            {
                mConvertor.setSrc(dispNode->getImgFormat(),
                                 (unsigned char*)dispNode->getVirAddr(),
                                 dispNode->getBufSize(),
                                 dispNode->getImgWidth(),
                                 dispNode->getImgHeight(),
                                 dispNode->getImgWidthStride(0),
                                 dispNode->getImgWidthStride(1),
                                 dispNode->getImgWidthStride(2));



                mConvertor.setDst(vidoNode->getImgFormat(),
                                 (unsigned char*)vidoNode->getVirAddr(),
                                 vidoNode->getBufSize(),
                                 vidoNode->getImgWidth(),
                                 vidoNode->getImgHeight(),
                                 vidoNode->getImgWidthStride(0),
                                 vidoNode->getImgWidthStride(1),
                                 vidoNode->getImgWidthStride(2));
                MY_LOGD_IF(0, "FD %s@%dx%d -> %s@%dx%d",
                       capNode.subImgNode->getImgFormat().string(),
                       capNode.subImgNode->getImgWidth(),
                       capNode.subImgNode->getImgHeight(),
                       vidoNode->getImgFormat().string(),
                       vidoNode->getImgWidth(),
                       vidoNode->getImgHeight());
                mConvertor.convert();
            }
#endif
        }
        mspPreviewBufHandler->enqueBuffer(dispNode);
    }
    //
    if (flag & eID_Pass2VIDO)
    {
        if ( vidoNode.getImgBuf() != 0 ) {
            vidoNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
            mspPreviewBufHandler->enqueBuffer(vidoNode);
        }

        capNode.mainImgNode.getImgBuf()->setTimestamp(pass1LatestTimeStamp);
        //mspPreviewBufHandler->enqueBuffer(vidoNode);
        capNode.u4FocusValH = u4FocusValH;
        capNode.u4FocusValL = u4FocusValL;
        mspCaptureBufHandler->enqueProvider(capNode, true);
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
        CPTLogStr(Event_Hal_Adapter_MtkZsdPreview_proc, CPTFlagSeparator, "dump buffer");
        //dumpBuffer(vDeBufPass1Out, "pass1", "raw", mFrameCnt);
        //dumpBuffer(vDeBufPass1Out, "pass1", "YUYV_Pack", mFrameCnt, mPorts.pass1Out.u4Width, mPorts.pass1Out.u4Height);
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
                (MUINT8*)(capNode.subImgNode.getImgBuf()->getVirAddr()),
                capNode.subImgNode.getImgBuf()->getBufSize(),
                "pass2_sub",
                "YUV420",
                mFrameCnt,
                capNode.subImgNode.getImgBuf()->getImgWidth(),
                capNode.subImgNode.getImgBuf()->getImgHeight());
            if (vidoNode.getImgBuf() !=0 ){
            dumpImg(
                (MUINT8*)(vidoNode.getImgBuf()->getVirAddr()),
                vidoNode.getImgBuf()->getBufSize(),
                "pass2_vido",
                "YUV420",
                mFrameCnt,
                vidoNode.getImgBuf()->getImgWidth(),
                vidoNode.getImgBuf()->getImgHeight());
            }
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

    IhwScenario::Rect_t Src(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, mCurZoomValue);
    //   (2) set to 3A
    mp3AHal->setZoom(mCurZoomValue, Crop.x, Crop.y, Crop.w, Crop.h);

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

    if ( ! getHw()->deque(eID_Pass1Out, &dummy) )
    {
        int i, tryCnt = 1;
        for (i = 0; i < tryCnt; i++) 
        {
            MY_LOGW("drop frame failed. try reset sensor(%d)", i);
            mSensorInfo.reset();
            if (getHw()->deque(eID_Pass1Out, &dummy))
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

    if (port & eID_Pass1Out)
    {
        IhwScenario::PortImgInfo imgInfoOut(
            eID_Pass1Out,
            mSensorInfo.getImgFormat(),
            mSensorInfo.getImgWidth(),
            mSensorInfo.getImgHeight()
        );
        imgInfoOut.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();
        IhwScenario::Rect_t SensorSize(mSensorInfo.getSensorWidth(), mSensorInfo.getSensorHeight());
        imgInfoOut.crop = calCrop(SensorSize, imgInfoOut.crop, 100);
        //
        rvImginfo.push_back(imgInfoOut);
    }

    if (port & eID_Pass2In)
    {
        IhwScenario::PortImgInfo imgInfoIn2(
            eID_Pass2In,
            mSensorInfo.getImgFormat(),
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

    if (port & eID_Pass1Out)
    {
        //
        rvImginfo.push_back(mPorts.pass1Out);
    }

    if (port & eID_Pass2In)
    {
        //
        rvImginfo.push_back(mPorts.pass2In);
    }

    if (port & eID_Pass2VIDO)
    {
        //
        rvImginfo.push_back(mPorts.pass2YuvOut);
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

    mPorts.pass1In.ePortIdx = eID_Pass1In;
    mPorts.pass1In.sFormat = mSensorInfo.getImgFormat();
    mPorts.pass1In.u4Width = mSensorInfo.getSensorWidth();
    mPorts.pass1In.u4Height = mSensorInfo.getSensorHeight();
    mPorts.pass1In.u4Stride[ESTRIDE_1ST_PLANE] = mSensorInfo.getImgWidthStride();

    mPorts.pass1Out.ePortIdx = eID_Pass1Out;
    mPorts.pass1Out.sFormat = mPorts.pass1In.sFormat;
    mPorts.pass1Out.u4Width = mPorts.pass1In.u4Width;
    mPorts.pass1Out.u4Height = mPorts.pass1In.u4Height;
    mPorts.pass1Out.u4Stride[ESTRIDE_1ST_PLANE] = mPorts.pass1In.u4Stride[ESTRIDE_1ST_PLANE];
    //mPorts.pass1Out.crop = calCrop(DispOutSize, mPorts.pass1Out.crop, 100);

    mPorts.pass2In.ePortIdx = eID_Pass2In;
    mPorts.pass2In.sFormat = mPorts.pass1Out.sFormat;
    mPorts.pass2In.u4Width = mPorts.pass1Out.u4Width;
    mPorts.pass2In.u4Height = mPorts.pass1Out.u4Height;
    mPorts.pass2In.u4Stride[ESTRIDE_1ST_PLANE] = mPorts.pass1Out.u4Stride[ESTRIDE_1ST_PLANE];

    int32_t i4CaptureWidth = 0, i4CaptureHeight = 0, i4Rotation = 0;
    if  ( ! queryCaptureSize(i4CaptureWidth, i4CaptureHeight, i4Rotation) )
    {
        MY_LOGE("queryCaptureSize");
        return false;
    }

    mPorts.pass2YuvOut.ePortIdx = eID_Pass2VIDO;
    mPorts.pass2YuvOut.sFormat = MtkCameraParameters::PIXEL_FORMAT_YUV422I;

    mPorts.pass2YuvOut.eRotate = i4Rotation == 0 ? eImgRot_0
                               : i4Rotation == 90 ? eImgRot_90
                               : i4Rotation == 180 ? eImgRot_180 : eImgRot_270;
    if (i4Rotation == 90 || i4Rotation == 270) {
        mPorts.pass2YuvOut.u4Width = i4CaptureHeight;//640;//mPorts.pass1Out.u4Width;
        mPorts.pass2YuvOut.u4Height = i4CaptureWidth;//480;//mPorts.pass1Out.u4Height;
    }
    else {
        mPorts.pass2YuvOut.u4Width = i4CaptureWidth;//640;//mPorts.pass1Out.u4Width;
        mPorts.pass2YuvOut.u4Height = i4CaptureHeight;//480;//mPorts.pass1Out.u4Height;
    }
    mPorts.pass2YuvOut.u4Stride[ESTRIDE_1ST_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass2YuvOut.sFormat, mPorts.pass2YuvOut.u4Width, ESTRIDE_1ST_PLANE);

    // zsd: fix me, how to get display port format?
    int32_t i4PreviewWidth = 0, i4PreviewHeight = 0;
    if  ( ! queryPreviewSize(i4PreviewWidth, i4PreviewHeight) )
    {
        MY_LOGE("queryPreviewSize");
        return false;
    }

    mPorts.pass2DumyPVOut.ePortIdx = eID_Unknown;
    //mPorts.pass2DumyPVOut.sFormat = MtkCameraParameters::PIXEL_FORMAT_YV12_GPU;
    mPorts.pass2DumyPVOut.sFormat = MtkCameraParameters::PIXEL_FORMAT_YUV420P;
    mPorts.pass2DumyPVOut.u4Width = i4PreviewWidth;
    mPorts.pass2DumyPVOut.u4Height = i4PreviewHeight;
    mPorts.pass2DumyPVOut.u4Stride[ESTRIDE_1ST_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass2DumyPVOut.sFormat, mPorts.pass2DumyPVOut.u4Width, 0);//mPorts.pass2DispOut.u4Width;
    mPorts.pass2DumyPVOut.u4Stride[ESTRIDE_2ND_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass2DumyPVOut.sFormat, mPorts.pass2DumyPVOut.u4Width, 1);//mPorts.pass2DispOut.u4Width;
    mPorts.pass2DumyPVOut.u4Stride[ESTRIDE_3RD_PLANE] = FmtUtils::queryImgWidthStride(mPorts.pass2DumyPVOut.sFormat, mPorts.pass2DumyPVOut.u4Width, 2);//mPorts.pass2DispOut.u4Width;

    return true;
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
mapQT2BufInfo(EHwBufIdx ePort, vector<IhwScenario::PortQTBufInfo> const &src, vector<IhwScenario::PortBufInfo> &dst)
{
    if ( src.size() <= 0 ) {
        MY_LOGE("vector size is 0!");
        return false;
    }

    if ( src.at(0).bufInfo.vBufInfo.empty() ) {
        MY_LOGE("Pass 1 buffer is 0!");
        return false;
    }

    int latest = src.at(0).bufInfo.vBufInfo.size()-1;
    MY_LOGD_IF(latest > 0, "deque size: %d", latest+1);

    IhwScenario::PortBufInfo one(
                     ePort,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufVA,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufPA,
                     src.at(0).bufInfo.vBufInfo.at(latest).u4BufSize,
                     src.at(0).bufInfo.vBufInfo.at(latest).memID
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
