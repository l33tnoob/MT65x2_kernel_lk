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

#define LOG_TAG "MtkCam/PrvCQT"

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
using namespace android::NSMtkDefaultCamAdapter;
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
#include <Scenario/Shot/IShot.h>
using namespace NSShot;
//
#include <Scenario/VideoSnapshot/IVideoSnapshotScenario.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>
//
#include <cutils/properties.h>
//
#define ENABLE_LOG_PER_FRAME        (1)
//
#define VSS_ENABLE      (1)
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
static bool checkDumpPass1(void);
static bool checkDumpPass2Dispo(void);
static bool checkDumpPass2Vido(void);
static bool dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, uint32_t filenum);
static bool dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, uint32_t filenum);
/******************************************************************************
*
*******************************************************************************/

namespace android {
namespace NSMtkDefaultCamAdapter {
    
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
        IhwScenario* pHwScenario = IhwScenario::createInstance(
                                                    eHW_VSS,
                                                    meSensorType,
                                                    meSensorDev,
                                                    mSensorBitOrder); 
        pHwScenario->getHwValidSize(mu4MemOutW,mu4MemOutH);
        pHwScenario->destroyInstance();
        pHwScenario = NULL;
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
#define PASS1BUFCNT      (5)
//
#if VSS_ENABLE
#define PASS1BUFCNT_VSS  (1)
#else
#define PASS1BUFCNT_VSS  (0)
#endif

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
    static PreviewCmdQueThread* getInstance(sp<IPreviewBufMgrHandler> pHandler, 
                                    int32_t const & rSensorid,
                                    sp<IParamsManager> pParamsMgr);
    
    virtual             ~PreviewCmdQueThread();

    virtual bool        setParameters();
    void                pushZoom(uint32_t zoomIdx);
    int                 popZoom();
    virtual void        setZoomCallback(IPreviewCmdQueCallBack *pZoomCb);

    virtual void        startRecording();
    virtual void        stopRecording();
     
protected:
                        PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pHandler, 
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
    void                updateCheck1();
    void                updateCheck2();
    void                handleCallback();
    bool                stop();
    bool                precap();
    bool                dropFrame();
    IhwScenario::Rect_t doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio = 100);
    void                getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo);
    //
    void                updateZoom(vector<IhwScenario::PortImgInfo> &pImgIn);
    uint32_t            getZoomValue();


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
    Hal3ABase*                  mp3AHal;
    IhwScenario*                mpHwScenario;
    sp<IPreviewBufMgrHandler>   mspPreviewBufHandler;
    sp<IParamsManager>          mspParamsMgr;
    sensorInfo                  mSensorInfo;

    int32_t                     mi4Tid;
    int32_t                     mbAWBIndicator;
    int32_t                     mbEFFECTIndicator;
    //
    Mutex                       mZoomMtx;
    Vector<uint32_t>            mvZoomIdx;
    IPreviewCmdQueCallBack*     mpZoomCB;
    uint32_t                    mCurZoomIdx;
    //
    uint32_t                    mFrameCnt;
    //
    EisHalBase*                 mpEisHal;
    //
    Condition                   mCondUpdateOne;
    volatile bool               mbRecording;
    bool                        mbRecordingHint;
    //
    #if VSS_ENABLE
    IVideoSnapshotScenario*     mpVideoSnapshotScenario;
    vector<IhwScenario::PortBufInfo>    mvBufPass1OutVss;
    #endif
};

}; // namespace NSMtkDefaultCamAdapter
}; // namespace android
/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::PreviewCmdQueThread(sp<IPreviewBufMgrHandler> pHandler, 
                                         int32_t const & rSensorid,
                                         sp<IParamsManager> pParamsMgr)
    : mpHwScenario(NULL)
    , mspPreviewBufHandler(pHandler)
    , mspParamsMgr(pParamsMgr)
    , mSensorInfo()
    , mi4Tid(0)
    , mbAWBIndicator(0)
    , mbEFFECTIndicator(0)
    , mZoomMtx()
    , mvZoomIdx()
    , mpZoomCB(NULL)
    , mpEisHal(NULL)
    , mbRecording(false)
    , mbRecordingHint(false)
{
    gInfo.openId = rSensorid;
    //
    #if VSS_ENABLE
    mpVideoSnapshotScenario = NULL;
    mvBufPass1OutVss.clear();
    #endif
}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread::~PreviewCmdQueThread()
{
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(PreviewCmdQueThread));
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
    ::prctl(PR_SET_NAME,"PreviewCmdQueThread", 0, 0, 0);

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
    //
    EIspProfile_T eIspProfile = ( mspParamsMgr->getRecordingHint() ) 
                ? EIspProfile_VideoPreview : EIspProfile_NormalPreview;
    ECmd_T eCmd = ( mspParamsMgr->getRecordingHint() ) 
                ? ECmd_CamcorderPreviewStart : ECmd_CameraPreviewStart;
    mbRecordingHint = ( mspParamsMgr->getRecordingHint() ) 
                ? true : false;

    //(0) scenario ID is decided by recording hint
    //
    int32_t eScenarioId = ( mspParamsMgr->getRecordingHint() ) 
                ? ACDK_SCENARIO_ID_VIDEO_PREVIEW : ACDK_SCENARIO_ID_CAMERA_PREVIEW;

    //(1) sensor (singleton)
    //
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "Init Sensor");
    //
    if ( ! (ret = mSensorInfo.init((ACDK_SCENARIO_ID_ENUM)eScenarioId)))
    {
        MY_LOGE("Init sensor fail!!");
        goto lbExit;
    }


    //(2) Hw scenario 
    //
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "Init Hw");
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
    getCfg(eID_Pass1In|eID_Pass1Out, vimgInfo);
    getHw()->setConfig(&vimgInfo);

    // (2.2) enque pass 1 buffer
    //     must do this earlier than hw start
    mspPreviewBufHandler->allocBuffer(
                          mSensorInfo.getImgWidth(),
                          mSensorInfo.getImgHeight(),
                          mSensorInfo.getImgFormat(),
                          PASS1BUFCNT+PASS1BUFCNT_VSS);
    
    for (int32_t i = 0; i < PASS1BUFCNT; i++)
    {  
        mspPreviewBufHandler->dequeBuffer(eID_Pass1Out, Pass1Node);
        mapNode2BufInfo(eID_Pass1Out, Pass1Node, BufInfo);
        vBufPass1Out.push_back(BufInfo);      
    }
    getHw()->enque(NULL, &vBufPass1Out);    
    //
    #if VSS_ENABLE
    mspPreviewBufHandler->dequeBuffer(eID_Pass1Out, Pass1Node);
    mapNode2BufInfo(eID_Pass1Out, Pass1Node, BufInfo);
    mvBufPass1OutVss.clear();
    mvBufPass1OutVss.push_back(BufInfo);
    #endif

    //(3) 3A
    //!! must be set after hw->enque; otherwise, over-exposure. 
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "Init 3A");   
    //
    mp3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId)); 
    if ( ! mp3AHal )
    {
        MY_LOGE("init 3A fail!!");
        goto lbExit;
    }

    mp3AHal->setZoom(100, 0, 0, mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    mp3AHal->setIspProfile(eIspProfile);
    mp3AHal->sendCommand(eCmd);

    // (4) EIS
    //
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "Init EIS");
    mpEisHal = EisHalBase::createInstance("mtkdefaultAdapter");
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
    //
    #if VSS_ENABLE
    mpVideoSnapshotScenario = IVideoSnapshotScenario::createInstance();
    #endif
    // (5) hw start
    //!!enable pass1 SHOULD BE last step!!
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "Hw start");
    //
    if ( ! getHw()->start())
    {
        goto lbExit;
    }
    //
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
    //setZoom(getParamsMgr()->getInt(CameraParameters::KEY_ZOOM));

    //
    p3AHal->destroyInstance();
    
    //
    FUNCTION_OUT;
    
    return true;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::setZoomCallback(IPreviewCmdQueCallBack *pZoomCb)
{
    mpZoomCB = pZoomCb;
}


/******************************************************************************
*
*******************************************************************************/
int
PreviewCmdQueThread::
popZoom()
{
    Mutex::Autolock _l(mZoomMtx);

    if ( mvZoomIdx.empty() )
    {
        MY_LOGD("ZoomQ is []");
        return -1;
    }

    int popIdx = *(mvZoomIdx.end()-1);
    MY_LOGD("popZoom (%d)", popIdx);
    mvZoomIdx.erase(mvZoomIdx.end()-1);

    return popIdx;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
pushZoom(uint32_t zoomIdx)
{
    Mutex::Autolock _l(mZoomMtx);

    MY_LOGD("pushZoom (%d)", zoomIdx);
    mvZoomIdx.push_back(zoomIdx);   
}


/******************************************************************************
*
*******************************************************************************/
uint32_t
PreviewCmdQueThread::
getZoomValue()
{
    //
    Mutex::Autolock _l(mZoomMtx);
    //
    uint32_t zoomIdx; 
    //
    if( ! mvZoomIdx.empty() )
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Size(%d)", mvZoomIdx.size());
        zoomIdx = *mvZoomIdx.begin();
        mvZoomIdx.erase(mvZoomIdx.begin());
        
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Idx(%d)", zoomIdx);        
    }
    else
    {
        zoomIdx = getParamsMgr()->getInt(CameraParameters::KEY_ZOOM);
    }
    
    mCurZoomIdx = zoomIdx;
    
    return getParamsMgr()->getZoomRatioByIndex(zoomIdx);
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
    ECmd_T eCmd = mbRecordingHint 
                ? ECmd_CamcorderPreviewEnd : ECmd_CameraPreviewEnd;

    //(1) stop sw 
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_stop, CPTFlagSeparator, "stop 3A");    
    //
    if ( mp3AHal != NULL )
    {
        mp3AHal->sendCommand(eCmd);
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }
    
    //(2) stop HW scenario
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_stop, CPTFlagSeparator, "stop Hw");
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
    

    //(3) stop sensor
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_stop, CPTFlagSeparator, "stop sensor");    
    if ( ! mSensorInfo.uninit() )
    {
        MY_LOGE("uninit sensor fail");
        ret = false;
    }


    //(4) stop eis
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_stop, CPTFlagSeparator, "stop EIS");
    if(mpEisHal != NULL)
    {
        mpEisHal->destroyInstance("mtkdefaultAdapter");
        mpEisHal = NULL;
    }

    //
    #if VSS_ENABLE
    if(mpVideoSnapshotScenario != NULL)
    {
        mpVideoSnapshotScenario->destroyInstance();
        mpVideoSnapshotScenario = NULL;
    }
    #endif
    //(5) change state to idle
    IStateManager::inst()->transitState(IState::eState_Idle); 
    //
    ::android_atomic_write(0, &mbAWBIndicator);     
    ::android_atomic_write(0, &mbEFFECTIndicator);     
    mvZoomIdx.clear();

    
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

    //(1) notify sw
    mp3AHal->sendCommand(ECmd_PrecaptureStart);    

    //(2) stay in preview until 3A permits 
    while ( ! mp3AHal->isReadyToCapture() ) 
    {
         CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_precap, CPTFlagSeparator, "precap_update");
         //
         updateOne();
    }
    
    //(3) notify sw
    mp3AHal->sendCommand(ECmd_PrecaptureEnd);

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

    if (mpHwScenario == NULL ||
        mp3AHal == NULL)
    {
        MY_LOGW("mpHwScenario or mp3AHal is NULL");
        return true;
    }

    do{
        CPTLog(Event_Hal_Adapter_MtkDefaultPreview_proc, CPTFlagStart);         
        //(1)
        updateOne();
        
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "frameCnt(%d)",mFrameCnt);
        mFrameCnt++;

        //(2) handle callback
        handleCallback();

        //(3) update check
        updateCheck1();
        updateCheck2();

        CPTLog(Event_Hal_Adapter_MtkDefaultPreview_proc, CPTFlagEnd);  
        
    } while( ! isNextCommand() );

    return true;
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
handleCallback()
{
    // zoom callback for smoothZoom
    if (mpZoomCB != NULL)
    {
        mpZoomCB->doNotifyCb(IPreviewCmdQueCallBack::eID_NOTIFY_Zoom, 
                             mCurZoomIdx, 0, 0);
    }
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
updateCheck1()
{
    //(1) BV value (3A --> AP)
    FrameOutputParam_T RTParams;
    mp3AHal->getRTParams(RTParams);
    int rt_BV = RTParams.i4BrightValue_x10;
    int rt_FPS = RTParams.u4FRameRate_x10;
    getParamsMgr()->updateBrightnessValue(rt_BV);
}

    
/******************************************************************************
*
*******************************************************************************/
void
PreviewCmdQueThread::
updateCheck2()
{
    //change AWB/EFFECT delay
    //what if 'AWB and EFFECT mode change' are coming together?
    //only choose one delay? which one? larger one? 
    
    MY_LOGW_IF(::android_atomic_release_load(&mbAWBIndicator) &&
               ::android_atomic_release_load(&mbEFFECTIndicator),
               "AWB and effect mode are changing at the same time");

    if (::android_atomic_release_load(&mbAWBIndicator))
    {
        delay(EQueryType_AWB);
        ::android_atomic_write(0, &mbAWBIndicator);
    }
    
    if (::android_atomic_release_load(&mbEFFECTIndicator))
    {
        delay(EQueryType_Effect);
        ::android_atomic_write(0, &mbEFFECTIndicator);    
    }
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
    nsecs_t passTime = 0,pass1Time = 0,pass2Time = 0,vssTime = 0;

    #if VSS_ENABLE
    vector<IhwScenario::PortBufInfo> vDeBufPass1OutVss;
    #endif
    vector<IhwScenario::PortQTBufInfo> vDeBufPass1Out;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass2Out;
    vector<IhwScenario::PortBufInfo> vEnBufPass2In;    
    vector<IhwScenario::PortBufInfo> vEnBufPass2Out;
    vector<IhwScenario::PortImgInfo> vPass2Cfg;

    if (mpHwScenario == NULL)
    {
        MY_LOGW("mpHwScenario is NULL");
        return false;
    }

    passTime = systemTime();
    //*************************************************************
    // (1) [PASS 1] sensor ---> ISP --> DRAM(IMGO)    
    //*************************************************************
    pass1Time = passTime;
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
    pass1Time = systemTime()-pass1Time;
    //
    mapQT2BufInfo(eID_Pass2In, vDeBufPass1Out, vEnBufPass2In);

    mp3AHal->sendCommand(ECmd_Update);
    //
    mpEisHal->doEIS();

    //
    //*************************************************************
    //(2) [PASS 2] DRAM(IMGI) --> ISP --> CDP --> DRAM (DISPO, VIDO)
    //    if no buffer is available, return immediately. 
    //*************************************************************
    int32_t flag = 0;


    //(.1) PASS2-IN
    //
    getCfg(eID_Pass2In , vPass2Cfg); 
    
    //(.2) PASS2-OUT
    //
    ImgBufQueNode dispNode;
    ImgBufQueNode vidoNode;
    mspPreviewBufHandler->dequeBuffer(eID_Pass2DISPO, dispNode);
    mspPreviewBufHandler->dequeBuffer(eID_Pass2VIDO, vidoNode);

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
        if( vidoNode.getCookieDE() == IPreviewBufMgr::eBuf_Rec &&
            !mbRecording)
        {
            MY_LOGW("VR has been stopped, do not enque buf to pass 2");
        }
        else
        {
            flag = flag | eID_Pass2VIDO; 
            IhwScenario::PortBufInfo BufInfo;
            IhwScenario::PortImgInfo ImgInfo;
            mapNode2BufInfo(eID_Pass2VIDO, vidoNode, BufInfo);
            mapNode2ImgInfo(eID_Pass2VIDO, vidoNode, ImgInfo);
            vEnBufPass2Out.push_back(BufInfo);
            vPass2Cfg.push_back(ImgInfo);
        }
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

    pass2Time = systemTime();
    getHw()->deque((EHwBufIdx)flag, &vDeBufPass2Out);
    pass2Time = systemTime()-pass2Time;

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
    //
    if (ret)
    {
        if(checkDumpPass1())
        {
            dumpBuffer(vDeBufPass1Out, "pass1", "raw", mFrameCnt);
        }
        if (flag & eID_Pass2DISPO)
        {
            if(checkDumpPass2Dispo())
            {
                dumpImg(
                    (MUINT8*)(dispNode.getImgBuf()->getVirAddr()),
                    dispNode.getImgBuf()->getBufSize(),
                    "pass2_dispo",
                    "yuv",
                    mFrameCnt);
            }
        }
        if (flag & eID_Pass2VIDO)
        {
            if(checkDumpPass2Vido())
            {
                dumpImg(
                    (MUINT8*)(vidoNode.getImgBuf()->getVirAddr()),
                    vidoNode.getImgBuf()->getBufSize(),
                    "pass2_vido",
                    "yuv",
                    mFrameCnt);
            }
        }
    }
    // (.1) return PASS1
    #if VSS_ENABLE
    if( mpVideoSnapshotScenario->getStatus() == IVideoSnapshotScenario::Status_WaitImage &&
        pass1LatestBufIdx >= 0)
    {
        vector<IhwScenario::PortImgInfo> vPass1OutCfg;
        IVideoSnapshotScenario::ImageInfo vssImage;
        //
        getCfg(eID_Pass1Out, vPass1OutCfg);
        //
        vssImage.size.width = vPass1OutCfg[0].u4Width;
        vssImage.size.height = vPass1OutCfg[0].u4Height;
        vssImage.size.stride = vPass1OutCfg[0].u4Stride[0];
        vssImage.mem.id = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].memID;
        vssImage.mem.vir = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].u4BufVA;
        vssImage.mem.phy = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].u4BufPA;
        vssImage.mem.size = vDeBufPass1Out[0].bufInfo.vBufInfo[pass1LatestBufIdx].u4BufSize; 
        vssImage.crop.x = vPass2Cfg[0].crop.x;
        vssImage.crop.y = vPass2Cfg[0].crop.y;
        vssImage.crop.w = vPass2Cfg[0].crop.w;
        vssImage.crop.h = vPass2Cfg[0].crop.h;
        mpVideoSnapshotScenario->setImage(vssImage);
        //
        mapQT2BufInfo(eID_Pass1Out, vDeBufPass1Out, vDeBufPass1OutVss);
        getHw()->replaceQue(&vDeBufPass1OutVss, &mvBufPass1OutVss);
        //Save the current pass 1 buffer for next VSS.
        mvBufPass1OutVss.clear();
        mvBufPass1OutVss.push_back(vDeBufPass1OutVss[0]);
    }
    else
    {
    #endif
        getHw()->enque(vDeBufPass1Out);
    #if VSS_ENABLE
    }
    #endif
    //
    #if VSS_ENABLE
    vssTime = systemTime();
    mpVideoSnapshotScenario->process();
    vssTime = systemTime()-vssTime;
    #endif
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
    //
    passTime = systemTime()-passTime;
    mCondUpdateOne.broadcast();
    MY_LOGD("Time(%3lld/%3lld/%3lld/%3lld)ms",
            passTime/1000000,
            pass1Time/1000000,
            pass2Time/1000000,
            vssTime/1000000);
    //
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
    MUINT32 EisXInt = 0, EisYInt = 0;
    MUINT32 EisXFlt = 0, EisYFlt = 0;
    MUINT32 EisTarWidth = 0;
    MUINT32 EisTarHeight = 0;
    MFLOAT EisScaleFactor = 100.0/EIS_FACTOR;
    //
    uint32_t  curZoomValue = getZoomValue();    
    getParamsMgr()->getPreviewSize(&PrvWidth, &PrvHeight);

    IhwScenario::Rect_t Src(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, curZoomValue);
    //   (2) set to 3A
    mp3AHal->setZoom(curZoomValue, Crop.x, Crop.y, Crop.w, Crop.h);
    //
    if(mspParamsMgr->getVideoStabilization())
    {
        mpEisHal->getEISResult(
                    EisXInt,
                    EisXFlt,
                    EisYInt,
                    EisYFlt,
                    EisTarWidth,
                    EisTarHeight);
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "EIS:X(%d/%10d),Y(%d/%10d),S(%d/%d)",
                    EisXInt, EisXFlt,
                    EisYInt, EisYFlt,
                    EisTarWidth, EisTarHeight);
        Crop.x = Crop.x * EisScaleFactor + EisXInt;
        Crop.y = Crop.y * EisScaleFactor + EisYInt;
        Crop.w = Crop.w * EisScaleFactor;
        Crop.h = Crop.h * EisScaleFactor;
        Crop.w = Crop.w & (~0x1);
        Crop.h = Crop.h & (~0x1);
        Crop.floatX = EisXFlt;
        Crop.floatY = EisYFlt;
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Crop(EIS):X(%d/%10d),Y(%d/%10d),S(%d/%d)",
                    Crop.x, Crop.floatX,
                    Crop.y, Crop.floatY,
                    Crop.w, Crop.h);
    }
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
}


/******************************************************************************
*
*******************************************************************************/
void 
PreviewCmdQueThread::
startRecording()
{
    FUNCTION_IN;
    //
    mbRecording = true;
    mp3AHal->sendCommand(ECmd_RecordingStart);
    //
    #if VSS_ENABLE
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_start_init, CPTFlagSeparator, "VSS start");
    IVideoSnapshotScenario::ImageSize vssImgSize;
    vssImgSize.width = mSensorInfo.getImgWidth();
    vssImgSize.height = mSensorInfo.getImgHeight();
    vssImgSize.stride = vssImgSize.width;
    if(mpVideoSnapshotScenario != NULL)
    {
        mpVideoSnapshotScenario->init(
                                    gInfo.openId,
                                    mspParamsMgr,
                                    mp3AHal,
                                    &vssImgSize);
    }
    else
    {
        MY_LOGE("mpVideoSnapshotScenario is NULL");
    }
    #endif
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void 
PreviewCmdQueThread::
stopRecording()
{
    FUNCTION_IN;
    //
    Mutex::Autolock lock(mCmdMtx);
    //
    mbRecording = false;
    mCondUpdateOne.wait(mCmdMtx);
    //
    mp3AHal->sendCommand(ECmd_RecordingEnd);
    //
    #if VSS_ENABLE
    CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_stop, CPTFlagSeparator, "VSS stop");
    if(mpVideoSnapshotScenario != NULL)
    {
        mpVideoSnapshotScenario->uninit();
    }
    #endif
    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
PreviewCmdQueThread*
PreviewCmdQueThread::
getInstance(sp<IPreviewBufMgrHandler> pHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    return  new PreviewCmdQueThread(pHandler, rSensorid, pParamsMgr);
}
    

/******************************************************************************
*
*******************************************************************************/
IPreviewCmdQueThread*
IPreviewCmdQueThread::
createInstance(sp<IPreviewBufMgrHandler> pHandler, int32_t const & rSensorid, sp<IParamsManager> pParamsMgr)
{
    if  ( pHandler != 0 ) {
        return  PreviewCmdQueThread::getInstance(pHandler, rSensorid, pParamsMgr); 
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


/******************************************************************************
*
*******************************************************************************/
static 
bool
checkDumpPass1(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);    
    //
    if (enable & 0x1) 
    {
        return true;
    }
    return false;
}

/******************************************************************************
*
*******************************************************************************/
static 
bool
checkDumpPass2Dispo(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);    
    //
    if (enable & 0x2) 
    {
        return true;
    }
    return false;
}

/******************************************************************************
*
*******************************************************************************/
static 
bool
checkDumpPass2Vido(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);    
    //
    if (enable & 0x4) 
    {
        return true;
    }
    return false;
}


/******************************************************************************
*
*******************************************************************************/
static 
bool
dumpBuffer(
    vector<IhwScenario::PortQTBufInfo> &src,
    char const*const tag,
    char const * const filetype,
    uint32_t filenum)
{
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
                  tag, filetype, filenum))
        {
            MY_LOGE("Dump buffer fail");       
        } 
    }
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
    uint32_t filenum)
{
    char* filePath = "/sdcard/camera_dump/MtkDefault/";
    char fileName[100];
    sprintf(fileName, "%s%s_%d.%s", filePath, tag, filenum, filetype);
    //
    if(makePath(filePath,0660))
    {
        return saveBufToFile(
                    (char const *)fileName,
                    addr,
                    size);
    }
    //
    return false;
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
    dst.bufSecu    = src.getImgBuf()->getBufSecu();
    dst.bufCohe    = src.getImgBuf()->getBufCohe();
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
