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

#define LOG_TAG "MtkATV/PreviewCmdTh"

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
using namespace android::NSMtkAtvCamAdapter; 
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
#include <adapter/MtkAtv/inc/DisplayDelayThread.h>
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
#include <linux/mtkfb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#include <stdio.h>
extern "C" {
    #include "jpeglib.h"
    #include "jerror.h"
    #include <setjmp.h>
}


//
#define DUMP
#ifdef DUMP
#include <cutils/properties.h>
#endif
#define ENABLE_LOG_PER_FRAME        (0)
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt,  __FUNCTION__, ##arg)
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
static bool dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, int frmcnt);
static bool dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, int framecnt);
/******************************************************************************
*
*******************************************************************************/

namespace android {
namespace NSMtkAtvCamAdapter {
    
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
        mpSensor->setATVStart();

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
        int idx = 0;
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
        MY_LOGD("meSensorDev(%d), meSensorType(%d), mSensorBitOrder(%d), mFormat(%s))",
                 meSensorDev, meSensorType, mSensorBitOrder, mFormat);
        return true;
    }
    
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

    // [ATV]+
    uint32_t            getDisplayDelayTime()
    {          
        mpSensor->sendCommand(meSensorDev, SENSOR_CMD_GET_ATV_DISP_DELAY_FRAME, 
                              (int32_t)&muDispDelayTime, 0);
        return muDispDelayTime;
    }
    //[[ATV]-
    
    uint32_t            getImgWidthStride(uint_t const uPlaneIndex = 0) const     
    { 
        return FmtUtils::queryImgWidthStride(mFormat, getImgWidth(), uPlaneIndex);   
    }
    
public:    
    bool                isYUV()             const   { return meSensorType == SENSOR_TYPE_YUV 
                                                                            ? true : false; }
    bool                isSub()             const   { return meSensorDev == SENSOR_DEV_SUB 
                                                                            ? true : false; }
    
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
    //[ATV]+
    uint32_t                     			muDispDelayTime;
    //[[ATV]-
};


/******************************************************************************
*
*******************************************************************************/
#define PASS1BUFCNT      (3)

class PreviewCmdQueThread : public IPreviewCmdQueThread
                           //[ATV]+
                          , public IDisplayDelayThreadHandler
                          //[ATV]-
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
    virtual bool        setZoom(uint32_t zoomValue);
     
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
    
	//[ATV]+
	virtual bool 		getCurPrvBuf(sp<IImgBuf>& pCurPrvBuf);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Public to IIRQHandler 
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
    public:
        void                waitVSirq();
        //[ATV]-

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
    bool                dropFrame(uint32_t frameCnt);
    IhwScenario::Rect_t doCrop(IhwScenario::Rect_t const &rSrc, IhwScenario::Rect_t const &rDst, uint32_t ratio = 100);
    void                getCfg(int32_t port, vector<IhwScenario::PortImgInfo> &rvImginfo);
	//[ATV]+
    bool                enableFBImmediateUpdate(bool fgEnable);
	//[ATV]-
	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command-related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                getCommand(sp<PrvCmdCookie> &rCmd);
    bool                isNextCommand();
    List< sp<PrvCmdCookie> > mCmdCookieQ; 
    Mutex               mCmdMtx;
    Condition           mCmdCond;
    enum CAPTURE_STATUS{
        atv_capture_idle,
        atv_capture_start,
        atv_capture_done,
        };
    CAPTURE_STATUS mCaptureStatus;
    

//[ATV]+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public to IDisplayDelayThreadHandler
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  
    virtual bool        delayDisplay();
//[ATV]-
    
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
    sp<IParamsManager>           mspParamsMgr;
    sensorInfo                   mSensorInfo;
    //[ATV]+
    //  Display Dealy Thread
    sp<IDisplayDelayThread>      mpDisplayDelayThread;
    Vector<ImgBufQueNode> 		 mvDisplayQueNode;
    int32_t                      miAVSyncDelayTime;
    int32_t                      miATVDisplayTime;
	bool						 mIsATVDisplay;
    sp<IImgBuf>                  mpCurDispBuf;
    //[ATV]-

    int32_t                      mi4Tid;
    int32_t                      mbAWBIndicator;
    int32_t                      mbEFFECTIndicator;
    //
    #define ZOOM_SKIP_STEP      (2)
    Mutex                       mZoomMtx;
    Vector<uint32_t>            mvZoomRatio;
    uint32_t                    mCurZoomValue;
    //
   
};

}; // namespace NSMtkAtvCamAdapter
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
    //[ATV]+
    , mpDisplayDelayThread(NULL)
    , mvDisplayQueNode()
    , miAVSyncDelayTime(0)
    , miATVDisplayTime(0)
    , mIsATVDisplay(false)
    , mpCurDispBuf(NULL)
    //[ATV]-
    , mSensorInfo()
    , mi4Tid(0)
    , mbAWBIndicator(0)
    , mbEFFECTIndicator(0)
    , mZoomMtx()
    , mvZoomRatio()
    , mCaptureStatus(atv_capture_idle)
{
    gInfo.openId = rSensorid;
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
    if (getCommand(pCmdCookie))  // sam change to while
    {
        if(pCmdCookie != 0)
        {
            pCmdCookie->postSem(PrvCmdCookie::eSemBefore);
        }
        //
        bool isvalid = true;
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

//[ATV]+
/******************************************************************************
*
*******************************************************************************/
bool 
PreviewCmdQueThread::
getCurPrvBuf(sp<IImgBuf>& pCurPrvBuf)
{
    FUNCTION_IN;
    //
    if(mCaptureStatus!=atv_capture_idle)
        return true;
    
    mCaptureStatus = atv_capture_start;

    while(mCaptureStatus != atv_capture_done)
    {
        usleep(5000);
    }

#if 0    
    ImgBufQueNode dispNodeBegin = *mvDisplayQueNode.begin();
    sp<IImgBuf> pCurDispBuf = dispNodeBegin.getImgBuf();
    
    if (mpCurDispBuf->getImgWidth() != pCurDispBuf->getImgWidth()
        || mpCurDispBuf->getImgHeight() != pCurDispBuf->getImgHeight()
        || mpCurDispBuf->getImgFormat() != pCurDispBuf->getImgFormat())
    {
        // realloc current preview buffer
        mpCurDispBuf.clear();
        mpCurDispBuf = new HwBuffer(pCurDispBuf->getImgWidth(), pCurDispBuf->getImgHeight(), pCurDispBuf->getImgFormat());
    }
    memcpy(mpCurDispBuf->getVirAddr(), pCurDispBuf->getVirAddr(), pCurDispBuf->getBufSize());
#endif

    pCurPrvBuf = mpCurDispBuf;

    mCaptureStatus = atv_capture_idle;
    MY_LOGD("for capture copy done to idle");
    FUNCTION_OUT;
    return true;
}
//[ATV]-


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
//[ATV]+
/******************************************************************************
*
*******************************************************************************/
bool
PreviewCmdQueThread::
delayDisplay()
{
    FUNCTION_IN;

    if(mvDisplayQueNode.isEmpty())
    {
        MY_LOGW("mvDisplayQueNode is null");
        return true;
    }
    if(!mIsATVDisplay)
    {        
        
        ImgBufQueNode dispNodeBegin = *mvDisplayQueNode.begin();
        ImgBufQueNode dispNodeEnd = mvDisplayQueNode.itemAt(mvDisplayQueNode.size()-1);
        
        sp<IImgBuf> pCurDispBuf = dispNodeBegin.getImgBuf();

        if (mpCurDispBuf->getImgWidth() != pCurDispBuf->getImgWidth()
            || mpCurDispBuf->getImgHeight() != pCurDispBuf->getImgHeight()
            || mpCurDispBuf->getImgFormat() != pCurDispBuf->getImgFormat())
        {
            // realloc current preview buffer
            mpCurDispBuf.clear();
            mpCurDispBuf = new HwBuffer(pCurDispBuf->getImgWidth(), pCurDispBuf->getImgHeight(), pCurDispBuf->getImgFormat());
            MY_LOGD("for capture copy allocate");
        }

        int delta =dispNodeEnd.getImgBuf()->getTimestamp() - dispNodeBegin.getImgBuf()->getTimestamp();
        if (delta >= miAVSyncDelayTime) {
            miATVDisplayTime = mSensorInfo.getDisplayDelayTime();
            mIsATVDisplay = true;
            ///MY_LOGD("start to display miAVSyncDelayTime(%d), miATVDisplayTime(%d)", miAVSyncDelayTime, miATVDisplayTime);
        }
        else
        {
            ///MY_LOGD("For AV Sync, delay display, delta is %d, AV Sync delay time is %d", delta, miAVSyncDelayTime);            
            FUNCTION_OUT;
            return true;
        }
    }

    if(mCaptureStatus == atv_capture_start)
    {    
        ImgBufQueNode dispNodeBegin = *mvDisplayQueNode.begin();
        sp<IImgBuf> pCurDispBuf = dispNodeBegin.getImgBuf();
        
        if (mpCurDispBuf->getImgWidth() != pCurDispBuf->getImgWidth()
            || mpCurDispBuf->getImgHeight() != pCurDispBuf->getImgHeight()
            || mpCurDispBuf->getImgFormat() != pCurDispBuf->getImgFormat())
        {
            // realloc current preview buffer
            mpCurDispBuf.clear();
            mpCurDispBuf = new HwBuffer(pCurDispBuf->getImgWidth(), pCurDispBuf->getImgHeight(), pCurDispBuf->getImgFormat());
            MY_LOGD("for capture copy allocate");
        }
        
        memcpy(mpCurDispBuf->getVirAddr(), pCurDispBuf->getVirAddr(), pCurDispBuf->getBufSize());
        MY_LOGD("for capture copy done, pCurDispBuf %p, size %d, %d", mpCurDispBuf->getVirAddr(), 
                        mpCurDispBuf->getBufSize(), pCurDispBuf->getBufSize());
        
        mCaptureStatus = atv_capture_done;
    }
    
    ::usleep(miATVDisplayTime);

    MY_LOGD("PreveiewEnqDisp, avDely %d, dispDelay %d",miAVSyncDelayTime , miATVDisplayTime);
    mspPreviewBufHandler->enqueBuffer(*mvDisplayQueNode.begin());

    mvDisplayQueNode.erase(mvDisplayQueNode.begin());
    
    FUNCTION_OUT;
    return true;

}
//[ATV]-

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


    //(1) sensor (singleton)
    //
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init Sensor");
    //
    
    // [ATV]+
    // (0) create display thread
    status_t status = OK;
    mpDisplayDelayThread = IDisplayDelayThread::createInstance(this);
    if  (
            mpDisplayDelayThread == 0
        ||  OK != (status = mpDisplayDelayThread->run())
        )
    {
        MY_LOGE(
            "Fail to run DisplayDelayThread - mpDisplayDelayThread.get(%p), status[%s(%d)]", 
            mpDisplayDelayThread.get(), ::strerror(-status), -status
        );
        goto lbExit;
    }
    //[ATV]-  

    if( ! (ret = mSensorInfo.init(ACDK_SCENARIO_ID_CAMERA_PREVIEW)))
    {
        MY_LOGE("Init sensor fail!!");
        goto lbExit;
    }


    //(2) Hw scenario 
    //
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init Hw");
    //
    mpHwScenario = IhwScenario::createInstance(eHW_VSS, mSensorInfo.getSensorType(), 
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
    getCfg(eID_Pass1In|eID_Pass1Out, vimgInfo);
    getHw()->setConfig(&vimgInfo);

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


    //(3) 3A
    //!! must be set after hw->enque; otherwise, over-exposure. 
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Init 3A");   
    //
    mp3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(gInfo.openId)); 
    if ( ! mp3AHal )
    {
        MY_LOGE("init 3A fail!!");
        goto lbExit;
    }

    mp3AHal->setZoom(100, 0, 0, mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    mp3AHal->setIspProfile(EIspProfile_NormalPreview);
    mp3AHal->sendCommand(ECmd_CameraPreviewStart);

    // [ATV]+
    // (5) enable immediate update FB
    enableFBImmediateUpdate(true);
    // [ATV]-


    // [ATV]+
    // (5) alloc buffer for capture
    mpCurDispBuf = new HwBuffer(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight(), mSensorInfo.getImgFormat());
    MY_LOGD("for capture copy1 start");
    mIsATVDisplay = false;
    miAVSyncDelayTime = mspParamsMgr->getInt("tv-delay");
    // [ATV]-
    //
   

    // (5) hw start
    // !!enable pass1 SHOULD BE last step!!
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_start_init, CPTFlagSeparator, "Hw start");
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
    //[ATV]+
    /*

    //(1)
    switch (type)
    {
        case EQueryType_Init:
        {
            //(1) pure delay by AAA
            //
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            dropFrame(delay3A);
    
            //(2) delay by sensor driver
            // should send update to sw
            int32_t delaySensor = mSensorInfo.getDelayFrame(SENSOR_PREVIEW_DELAY);
            for (int32_t i = 0; i < delaySensor; i++)
            {
                dropFrame(1);
                mp3AHal->sendCommand(ECmd_Update);
            } 
            
            MY_LOGD("delay(Init): 3A(%d) + sensor(%d)", delay3A, delaySensor);
        }
        break;
        
        case EQueryType_Effect:
        {
            int32_t delay3A = mp3AHal->getDelayFrame(type);
            int32_t count = 0; 
            for (count; count < delay3A; count++)
            {
                if (::android_atomic_release_load(&mbEFFECTIndicator)) {
                    dropFrame(1);
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
                    dropFrame(1);
                }
                else {
                    break;
                }
            }
            
            MY_LOGD("delay(Awb): (%d), real: (%d)", delay3A, count);
        }
        break;
    }
       */
    //(1) wait atv frame stable  
    int32_t delay = 2;
    
    MY_LOGD("delay %d frame for ATV sensor stable", delay);
    
    ///dropFrame(delay);
    //[ATV]-
        
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
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop 3A");    
    //
    if ( mp3AHal != NULL )
    {
        mp3AHal->sendCommand(ECmd_CameraPreviewEnd);
        mp3AHal->destroyInstance();
        mp3AHal = NULL;
    }
    
    //(2) stop HW scenario
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop Hw");
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
    if ( mspPreviewBufHandler != 0 )
    {
        mspPreviewBufHandler->freeBuffer();  
        mspPreviewBufHandler = 0;
    }
    
    //[ATV]+
    sp<IDisplayDelayThread> pDisplayDelayThread;
    {
        pDisplayDelayThread = mpDisplayDelayThread;
        mpDisplayDelayThread = NULL;
    }
    //
    if  ( pDisplayDelayThread != 0 )
    {
        MY_LOGD(
            "DisplayDelayThread: (tid, getStrongCount, pDisplayDelayThread)=(%d, %d, %p)", 
            pDisplayDelayThread->getTid(), pDisplayDelayThread->getStrongCount(), pDisplayDelayThread.get()
        );
        //  Notes:
        //  requestExitAndWait() in ICS has bugs. Use requestExit()/join() instead.
        pDisplayDelayThread->requestExit();
        status_t status = OK;
        if  ( OK != (status = pDisplayDelayThread->join()) )
        {
            MY_LOGW("Not to wait DisplayDelayThread(tid:%d), status[%s(%d)]", pDisplayDelayThread->getTid(), ::strerror(-status), -status);
        }
        MY_LOGD("join() exit");
        pDisplayDelayThread = NULL;
    }
    //[ATV]-
    //

    //(3) stop sensor
    CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_stop, CPTFlagSeparator, "stop sensor");    
    if ( ! mSensorInfo.uninit() )
    {
        MY_LOGE("uninit sensor fail");
        ret = false;
    }


    //(5) change state to idle
    IStateManager::inst()->transitState(IState::eState_Idle); 
    //
    ::android_atomic_write(0, &mbAWBIndicator);     
    ::android_atomic_write(0, &mbEFFECTIndicator);     
    mvZoomRatio.clear();

    // [ATV]+
    enableFBImmediateUpdate(false);
    // [ATV]-
    
    // [ATV]+
    mpCurDispBuf.clear();
    mvDisplayQueNode.clear();
    // [ATV]-

    
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
    //
    //[ATV]+
    /*

    //(1) notify sw
    mp3AHal->sendCommand(ECmd_PrecaptureStart);    

    //(2) stay in preview until 3A permits 
    while ( ! mp3AHal->isReadyToCapture() ) 
    {
         CPTLogStr(Event_Hal_Adapter_MtkPhotoPreview_precap, CPTFlagSeparator, "precap_update");
         //
         updateOne();
    }
    
    //(3) notify sw
    mp3AHal->sendCommand(ECmd_PrecaptureEnd);
    */
    MY_LOGD("For ATV, directly change state to precapture state");
    //[ATV]-

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
    
    static int framecnt = 0;

    IStateManager::inst()->transitState(IState::eState_Preview);

    do{
        CPTLog(Event_Hal_Adapter_MtkPhotoPreview_proc, CPTFlagStart);         
        //(1)
        updateOne();
        
        MY_LOGD("frameCnt(%d)", framecnt++);
    
        //(2) check if need dalay
        //updateCheck();  [ATV]

        CPTLog(Event_Hal_Adapter_MtkPhotoPreview_proc, CPTFlagEnd);  
        
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
    
    //(1) change AWB/EFFECT delay
    //what if 'AWB and EFFECT mode change' are coming together?
    //only choose one delay? which one? larger one? 
    
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

    vector<IhwScenario::PortQTBufInfo> vDeBufPass1Out;
    vector<IhwScenario::PortQTBufInfo> vDeBufPass2Out;
    vector<IhwScenario::PortBufInfo> vEnBufPass2In;    
    vector<IhwScenario::PortBufInfo> vEnBufPass2Out;
    vector<IhwScenario::PortImgInfo> vPass2Cfg;


    //*************************************************************
    // (1) [PASS 1] sensor ---> ISP --> DRAM(IMGO)    
    //*************************************************************
    getHw()->deque(eID_Pass1Out, &vDeBufPass1Out);
    if ( ! mapQT2BufInfo(eID_Pass2In, vDeBufPass1Out, vEnBufPass2In) )
    {
        ///mSensorInfo.dumpIspReg();
        getHw()->enque(vDeBufPass1Out);   
        return false;           
    }    
    mp3AHal->sendCommand(ECmd_Update);
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
        flag = flag | eID_Pass2VIDO; 
        IhwScenario::PortBufInfo BufInfo;
        IhwScenario::PortImgInfo ImgInfo;
        mapNode2BufInfo(eID_Pass2VIDO, vidoNode, BufInfo);
        mapNode2ImgInfo(eID_Pass2VIDO, vidoNode, ImgInfo);
        vEnBufPass2Out.push_back(BufInfo);
        vPass2Cfg.push_back(ImgInfo); 
    }

    //(.3) no buffer ==> return immediately. 
    //
    MY_LOGD("P2(0x%X)", flag);
    
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
    getHw()->deque((EHwBufIdx)flag, &vDeBufPass2Out);
    
   
    //[ATV]+
    waitVSirq();
    //[ATV]-

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
#if 0
        mspPreviewBufHandler->enqueBuffer(dispNode);
#else        
        //[ATV]+
        /*
                mspPreviewBufHandler->enqueBuffer(dispNode);
              */
        mvDisplayQueNode.push_back(dispNode);

        mpDisplayDelayThread->postCommand(Command(Command::eID_DISPLAY_FRAME));
        //[ATV]-
#endif        
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
    static int32_t num = 0; 
    if (ret  )
    {
        ///MY_LOGD("raw fotmat %s,w %d,h %d frm %d ts: %lld", mSensorInfo.getImgFormat(), mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight(), num, pass1LatestTimeStamp);
        dumpBuffer(vDeBufPass1Out, "pass1", "raw", num);
        if (flag & eID_Pass2DISPO)
        {            
            MY_LOGD("dis fotmat %s,w %d,h %d ", dispNode.getImgBuf()->getImgFormat().string(), dispNode.getImgBuf()->getImgWidth(), dispNode.getImgBuf()->getImgHeight());
            dumpImg(
                (MUINT8*)(dispNode.getImgBuf()->getVirAddr()),
                dispNode.getImgBuf()->getBufSize(),
                "pass2_dispo",
                "yuv", num);
        }
        if (flag & eID_Pass2VIDO)
        {
            dumpImg(
                (MUINT8*)(vidoNode.getImgBuf()->getVirAddr()),
                vidoNode.getImgBuf()->getBufSize(),
                "pass2_vido",
                "yuv", num);
        }
        num++;
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
    //
    getParamsMgr()->getPreviewSize(&PrvWidth, &PrvHeight);

    IhwScenario::Rect_t Src(mSensorInfo.getImgWidth(), mSensorInfo.getImgHeight());
    IhwScenario::Rect_t Dst(PrvWidth, PrvHeight);
    IhwScenario::Rect_t Crop = doCrop(Src, Dst, mCurZoomValue);
    //
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

//[ATV]+
bool
PreviewCmdQueThread::
enableFBImmediateUpdate(bool fgEnable)
{
    int fd = open("/dev/graphics/fb0", O_RDWR, 0);
    
    MY_LOGD("[enableFBImmediateUpdate] %d \n", fgEnable);
   
    if (fd >= 0) {
        unsigned int value = (fgEnable ? 1 : 0); 
        if (ioctl(fd, MTKFB_CONFIG_IMMEDIATE_UPDATE, value) == -1) {
            MY_LOGE("[%s] ioctl failed, errno: %d", __func__, errno);
            return false;
        }
        close(fd);
    } else {
        MY_LOGE("[%s] open mtkfb failed. errno: %d", __func__, errno);
        return false;
    }
    
    return true;
}
//[ATV]-

/*******************************************************************************
*  
********************************************************************************/
bool
PreviewCmdQueThread::
dropFrame(uint32_t frameCnt)
{
    bool ret = true;

    //[T.B.D]
    //(1) deque in one time may get more than 1 frame 
    //(2) the first frame is filled 'before' or 'after' calling?
    //    in the following code, assume 'after'

    if ( frameCnt == 0 )
    {
        MY_LOGW("framecnt");    
        return false;
    }
    
    int32_t count = (int32_t)frameCnt;
    
    do {
        MY_LOGD("(%d) frames left need to be dropped", count);
        
        vector<IhwScenario::PortQTBufInfo> dummy; 
        
        getHw()->deque(eID_Pass1Out, &dummy);
        getHw()->enque(dummy);  
        if ( dummy.size() == 0 )
        {
            MY_LOGW("drop size == 0");
            continue;    
        }
        
        count = count - dummy.at(0).bufInfo.vBufInfo.size();
        
    } while (count > 0);
        
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

//[ATV]+
/******************************************************************************
*
*******************************************************************************/
void 
PreviewCmdQueThread::
waitVSirq()
{
    getHw()->wait(eIRQ_VS);
}
//ATV-

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
dumpBuffer(vector<IhwScenario::PortQTBufInfo> &src, char const*const tag, char const * const filetype, int frmcnt)
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
                  tag, filetype, frmcnt))
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
dumpImg(MUINT8 *addr, MUINT32 size, char const * const tag, char const * const filetype, int framecnt)
{
    ///return true;   ///[ATV]  
#if 1
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);    
    if (enable == 0) 
    {
        return false;
    }
#endif
    //
    char fileName[64];
    sprintf(fileName, "/sdcard/%s_%d.%s", tag, framecnt, filetype);
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

    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VA(0x%08X),S(%d),Idx(%d),Id(%d)", 
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
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "VA(0x%08X),S(%d),Idx(%d),Id(%d)", 
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
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Port(%d),F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)", 
                   dst.ePortIdx, dst.sFormat, dst.u4Width, dst.u4Height, 
                   dst.u4Stride[ESTRIDE_1ST_PLANE], dst.u4Stride[ESTRIDE_2ND_PLANE], dst.u4Stride[ESTRIDE_3RD_PLANE], dst.eRotate);
}
