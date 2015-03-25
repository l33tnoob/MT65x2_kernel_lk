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

#define LOG_TAG "ImageCreateThread"
//
#include <utils/List.h>>
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include "../inc/ImageCreateThread.h"
//
#include <linux/rtpm_prio.h>
#include <sys/prctl.h>
//
using namespace android;
using namespace NSCamShot;


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#include "mtkcam/Log.h"
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


/*******************************************************************************
*   ImageCreateThread Class
*******************************************************************************/
class ImageCreateThread : public IImageCreateThread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in IImageCreateThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Attributes.
    virtual int32_t     getTid() const          { return mi4Tid; }
    virtual bool        isExitPending() const   { return exitPending(); }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////        Interfaces.
    virtual void        postCommand(Command const& rCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Instantiation.
                        ImageCreateThread(IImageCreateThreadHandler*const pHandler);

protected:  ////        Data Members.
    IImageCreateThreadHandler*   mpThreadHandler;
    int32_t             mi4Tid;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Commands.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Operations.
    virtual bool        getCommand(Command& rCmd);

protected:  ////        Data Members.
    List<Command>       mCmdQue;
    Mutex               mCmdQueMtx;
    Condition           mCmdQueCond;    //  Condition to wait: [ ! exitPending() && mCmdQue.empty() ]
};



/******************************************************************************
*
*******************************************************************************/
ImageCreateThread::
ImageCreateThread(IImageCreateThreadHandler*const pHandler)
    : IImageCreateThread()
    //
    , mpThreadHandler(pHandler)
    , mi4Tid(0)
    //
    , mCmdQue()
    , mCmdQueMtx()
    , mCmdQueCond()
{
}


/******************************************************************************
*
*******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
ImageCreateThread::
requestExit()
{
    MY_LOGD("+");
    Thread::requestExit();
    //
    postCommand(Command(Command::eID_EXIT));
    //
    MY_LOGD("-");
}


/******************************************************************************
*
*******************************************************************************/
// Good place to do one-time initializations
status_t
ImageCreateThread::
readyToRun() 
{
    MY_LOGD("ImageCreateThread::readyToRun");

    ::prctl(PR_SET_NAME,"MShot@CreateImage", 0, 0, 0);
    //
    mi4Tid = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = RTPM_PRIO_CAMERA_PREVIEW;
#warning "[FIXME] ImageCreateThread::readyToRun() thread priority & policy"
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
#endif 
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
void
ImageCreateThread::
postCommand(Command const& rCmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    if  ( ! mCmdQue.empty() )
    {
        Command const& rBegCmd = *mCmdQue.begin();
        MY_LOGW("que size:%d > 0 with begin Command::%s", mCmdQue.size(), rBegCmd.name());
    }
    //
    mCmdQue.push_back(rCmd);
    mCmdQueCond.broadcast();
    //
    //MY_LOGD("- new command::%s", rCmd.name());
}


/******************************************************************************
*
*******************************************************************************/
bool
ImageCreateThread::
getCommand(Command& rCmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        rCmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%s", rCmd.name());
    }
    //
    //MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
ImageCreateThread::
threadLoop()
{

    //MY_LOGD("ImageCreateThread::threadLoop");

    bool ret = false;
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_EXIT:
            MY_LOGD("Command::%s", cmd.name());
            break;
        //
        case Command::eID_WAKEUP:
        default:
            if  ( mpThreadHandler != NULL )
            {
                ret = mpThreadHandler->onThreadLoop(IMAGE_CREATE);  
            }
            else
            {
                MY_LOGE("cannot handle cmd(%s) due to mpThreadHandler==NULL", cmd.name());
            }
            break;
            MY_LOGD("Command::%s", cmd.name());
            break;
        }
    }
   
    //
    //MY_LOGD("- mpThreadHandler.get(%p)", mpThreadHandler);
    return  ret;
}

/*******************************************************************************
*   YuvImageCreateThread Class
*******************************************************************************/
class YuvImageCreateThread : public ImageCreateThread
{
public: 
    YuvImageCreateThread(IImageCreateThreadHandler*const pHandler);
    // Good place to do one-time initializations
    virtual status_t    readyToRun();
    virtual bool        threadLoop();
    bool                createYuvImage();
    bool                isNextCommand();
};

/******************************************************************************
*
*******************************************************************************/
YuvImageCreateThread::
YuvImageCreateThread(IImageCreateThreadHandler*const pHandler)
    : ImageCreateThread(pHandler)
{
}
// Good place to do one-time initializations
status_t
YuvImageCreateThread::
readyToRun()
{
    MY_LOGD("NSCamShot@CreateYuv ready to run");

    ::prctl(PR_SET_NAME,"MShot@CreateYuv", 0, 0, 0);
    //
    mi4Tid = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_CAMERA_CAPTURE;
#warning "[FIXME] ImageCreateThread::readyToRun() thread priority & policy"
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
bool
YuvImageCreateThread::
threadLoop()
{
    //MY_LOGD("YuvImageCreateThread::threadLoop");

    bool ret = false;
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_EXIT:
            MY_LOGD("Command::%s", cmd.name());
            break;
        //
        case Command::eID_WAKEUP:
        default:
            ret = createYuvImage();
            break;
        }
    }
    //
    //MY_LOGD("- mpThreadHandler.get(%p), ret", mpThreadHandler, ret);
    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
YuvImageCreateThread::
createYuvImage()
{
    MBOOL ret = MTRUE;
    int yuvCnt = 0;
    do{
        //(1)
        if  ( mpThreadHandler != NULL )
        {
            ret = mpThreadHandler->onThreadLoop(YUV_IMAGE_CREATE); 
        }
        else
        {
            MY_LOGE("cannot create yuv image due to mpThreadHandler==NULL");
            ret = MFALSE;
            break;
        }
        
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "yuvCnt(%d)", yuvCnt++);
            
    } while( ! isNextCommand() && ret);

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
YuvImageCreateThread::
isNextCommand()
{
   Mutex::Autolock _l(mCmdQueMtx);
   //
   return mCmdQue.empty()? false : true;
}

/*******************************************************************************
*   ThumbnailImageCreateThread Class
*******************************************************************************/
class ThumbnailImageCreateThread : public ImageCreateThread
{
public: 
    ThumbnailImageCreateThread(IImageCreateThreadHandler*const pHandler);
    // Good place to do one-time initializations
    virtual status_t    readyToRun();
    virtual bool        threadLoop();
};

/******************************************************************************
*
*******************************************************************************/
ThumbnailImageCreateThread::
ThumbnailImageCreateThread(IImageCreateThreadHandler*const pHandler)
    : ImageCreateThread(pHandler)
{
}
// Good place to do one-time initializations
status_t
ThumbnailImageCreateThread::
readyToRun()
{
    MY_LOGD("NSCamShot@CreateThumbnail ready to run");

    ::prctl(PR_SET_NAME,"MShot@CreateThumbnail", 0, 0, 0);
    //
    mi4Tid = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_CAMERA_CAPTURE;
#warning "[FIXME] ImageCreateThread::readyToRun() thread priority & policy"
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
#endif 
    return NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
bool
ThumbnailImageCreateThread::
threadLoop()
{
    //MY_LOGD("ThumbnailImageCreateThread::threadLoop");

    bool ret = false;
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_EXIT:
            MY_LOGD("Command::%s", cmd.name());
            break;
        //
        case Command::eID_POSTVIEW_BUF:
        default:
            if  ( mpThreadHandler != NULL )
            {
                 ret = mpThreadHandler->onThreadLoop(THUMBNAIL_IMAGE_CREATE);  
            }
            else
            {
                MY_LOGE("cannot handle cmd(%s) due to mpThreadHandler==NULL", cmd.name());
            }
            break;
        }
    }
    //
    //MY_LOGD("- mpThreadHandler.get(%p)", mpThreadHandler);
    return  ret;
}

/*******************************************************************************
*   JpegImageCreateThread Class
*******************************************************************************/
class JpegImageCreateThread : public ImageCreateThread
{
public: 
    JpegImageCreateThread(IImageCreateThreadHandler*const pHandler);
    // Good place to do one-time initializations
    virtual status_t    readyToRun();
    virtual bool        threadLoop();
};

/******************************************************************************
*
*******************************************************************************/
JpegImageCreateThread::
JpegImageCreateThread(IImageCreateThreadHandler*const pHandler)
    : ImageCreateThread(pHandler)
{
}

// Good place to do one-time initializations
status_t
JpegImageCreateThread::
readyToRun()
{
    MY_LOGD("NSCamShot@CreateJpeg ready to run");

    ::prctl(PR_SET_NAME,"MShot@CreateJpeg", 0, 0, 0);
    //
    mi4Tid = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
#if MTKCAM_HAVE_RR_PRIORITY
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_CAMERA_CAPTURE;
#warning "[FIXME] ImageCreateThread::readyToRun() thread priority & policy"
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
#endif 
    return NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
bool
JpegImageCreateThread::
threadLoop()
{
    //MY_LOGD("JpegImageCreateThread::threadLoop");

    bool ret = false;
    Command cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
        case Command::eID_EXIT:
            MY_LOGD("Command::%s", cmd.name());
            break;
        //
        case Command::eID_YUV_BUF:
        default:
            if  ( mpThreadHandler != NULL )
            {
                ret = mpThreadHandler->onThreadLoop(JPEG_IMAGE_CREATE);  
            }
            else
            {
                MY_LOGE("cannot handle cmd(%s) due to mpThreadHandler==NULL", cmd.name());
            }
            break;
        }
    }
    //
    //MY_LOGD("- mpThreadHandler.get(%p)", mpThreadHandler);
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
IImageCreateThread*
IImageCreateThread::
createInstance(IMAGE_TYPE imgType, IImageCreateThreadHandler*const pHandler)
{
    if  ( ! pHandler ) {
        MY_LOGE("pHandler==NULL");
        return  NULL;
    }

    IImageCreateThread* pImageCreateThread = NULL;

    switch  (imgType)
    {
    case IMAGE_CREATE: 
        pImageCreateThread = new ImageCreateThread(pHandler);
        break;
    case YUV_IMAGE_CREATE: 
        pImageCreateThread = new YuvImageCreateThread(pHandler);
        break;
    case THUMBNAIL_IMAGE_CREATE:
        pImageCreateThread = new ThumbnailImageCreateThread(pHandler);
        break;
    case JPEG_IMAGE_CREATE:
        pImageCreateThread = new JpegImageCreateThread(pHandler);
        break;
    default:
        break;
    }

    return pImageCreateThread;
}

