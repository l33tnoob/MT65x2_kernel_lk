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

#define LOG_TAG "MtkCam/PrvCB"
//
#include "PreviewClient.h"
//
using namespace NSCamClient;
using namespace NSPrvCbClient;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
ProfileTool::
ProfileTool(char const*const pszSubjectName)
    : ms8SubjectName(String8(pszSubjectName))
    , mfFps(0)
    , mnsDuration(0)
    , mi4Count(0)
    , mnsStart(::systemTime())
    , mnsEnd(::systemTime())
{
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
pulse()
{
    pulse(::systemTime());
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
pulse(nsecs_t nsTimestamp)
{
    mi4Count++;
    mnsEnd = nsTimestamp;
    mnsDuration = mnsEnd - mnsStart;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
reset()
{
    reset(::systemTime());
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
reset(nsecs_t nsInitTimestamp)
{
    mi4Count = 0;
    mnsStart = nsInitTimestamp;
    mnsEnd = nsInitTimestamp;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
updateFps()
{
    mfFps = (float)mi4Count / mnsDuration * 1000000000LL;
}


/******************************************************************************
 *
 ******************************************************************************/
void
ProfileTool::
showFps() const
{
    CAM_LOGD_IF(
        1, 
        "[%s] fps: %3f = %d / %lld ns", 
        ms8SubjectName.string(), mfFps, mi4Count, mnsDuration
    );
}


/******************************************************************************
*
*******************************************************************************/
namespace {
extern size_t getTestImageSize();
extern char const* getTestImageBase();


class TestThread : public Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop()
                        {
                            ImgBufQueNode   QueNode;
                            //
                            for (int i = 0; i < miLoopCount; i++)
                            {
                                if  ( mpImgBufQueue->dequeProvider(QueNode) )
                                {
                                    renderImgBuf(QueNode.getImgBuf());
                                    //
                                    mpImgBufQueue->enqueProvider(ImgBufQueNode(QueNode.getImgBuf(), ImgBufQueNode::eSTATUS_DONE));
                                }
                                //
                                ::usleep(miSleepInUs);
                            }
                            return  false;
                        }
                        //
    void                renderImgBuf(sp<IImgBuf>const& pImgBuf)
                        {
                            char const aColor[] = {0, 128, 255};
                            static int idxColor = 0;
                            MY_LOGD("[TestThread::renderImgBuf]");

                            ::memset(pImgBuf->getVirAddr(), aColor[idxColor], pImgBuf->getBufSize());
                            idxColor = (idxColor+1) % (sizeof(aColor)/sizeof(aColor[0]));
                        }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Instantiation.
                        TestThread(sp<IImgBufQueue> pImgBufQueue, int const iLoopCount = 30, int const iSleepInUs = 33333)
                            : mpImgBufQueue(pImgBufQueue)
                            , miLoopCount(iLoopCount)
                            , miSleepInUs(iSleepInUs)
                        {
                        }

protected:  ////
    sp<IImgBufQueue>    mpImgBufQueue;
    int                 miLoopCount;
    int                 miSleepInUs;
};
}


/******************************************************************************
*
*******************************************************************************/
void
PreviewClient::
dumpImgBuf_If(sp<ICameraImgBuf>const& rpImgBuf)
{
    //  Dump image if wanted.
    if  ( 0 < mi4DumpImgBufCount )
    {
        MY_LOGD("<dump image> mi4DumpImgBufCount(%d) > 0", mi4DumpImgBufCount);
        //
        String8 const format = rpImgBuf->getImgFormat();
        int const width = rpImgBuf->getImgWidth(), height = rpImgBuf->getImgHeight(), stride = rpImgBuf->getImgWidthStride();
        Vector<uint8_t> vBuf;
        vBuf.appendArray((uint8_t const*)rpImgBuf->getVirAddr(), rpImgBuf->getBufSize());
        //
        Mutex::Autolock _lock(mDumpMtx);
        mDumpImgBufList.push_back(vBuf);
        if  ( 1 == ::android_atomic_dec(&mi4DumpImgBufCount) )
        {
            for (List< Vector<uint8_t> >::iterator it = mDumpImgBufList.begin(); it != mDumpImgBufList.end(); it++)
            {
                saveBufToFile(
                    String8::format(
                        "%s_%s-(%d)%dx%d_%03d.yuv", 
                        ms8DumpImgBufPath.string(), 
                        format.string(), 
                        stride, 
                        width, 
                        height, 
                        mi4DumpImgBufIndex
                    ), 
                    (uint8_t*)it->array(), it->size()
                );
                ::android_atomic_inc(&mi4DumpImgBufIndex);
            }
            mDumpImgBufList.clear();
        }
    }
}


/******************************************************************************
 *
 *  @brief  <action> -loop-count=30 -sleep-in-us=33333 -dump-file-count=1 -dump-file-path=sdcard/DCIM -dump-file-name=prvcb
 *
 ******************************************************************************/
namespace
{
    struct  DumpArgument
    {
        int         miLoopCount;
        int         miSleepInUs;
        int         miDumpFileCount;
        String8     ms8DumpFilePath;
        String8     ms8DumpFileName;
                    //
                    DumpArgument()
                        : miLoopCount(30)
                        , miSleepInUs(33333)
                        , miDumpFileCount(1)
                        , ms8DumpFilePath("/sdcard/camera_dump")
                        , ms8DumpFileName("prvcb")
                    {
                    }
    };

    bool            parseOneCmdArgument(String8 const& rCmdArg, String8& rKey, String8& rVal)
                    {
                        const char *a = rCmdArg.string();
                        const char *b;
                        //
                        // Find the bounds of the key name.
                        b = ::strchr(a, '=');
                        if (b == 0)
                            return false;

                        // Create the key string.
                        rKey = String8(a, (size_t)(b-a));

                        // Find the value.
                        a = b+1;
                        rVal = String8(a);
                        return  true;
                    }

    bool            parseDumpArgument(Vector<String8>const& rvs8Args, DumpArgument& rArgs)
                    {
                        for (size_t i = 1; i < rvs8Args.size(); i++)
                        {
                            String8 const& s8Arg = rvs8Args[i];
                            String8 key, val;
                            if  ( ! parseOneCmdArgument(s8Arg, key, val) ) {
                                continue;
                            }
                            //
                            if  ( key == "-loop-count" ) {
                                rArgs.miLoopCount = ::atoi(val);
                                continue;
                            }
                            //
                            if  ( key == "-sleep-in-us" ) {
                                rArgs.miSleepInUs = ::atoi(val);
                                continue;
                            }
                            //
                            if  ( key == "-dump-file-count" ) {
                                rArgs.miDumpFileCount = ::atoi(val);
                                continue;
                            }
                            //
                            if  ( key == "-dump-file-path" ) {
                                rArgs.ms8DumpFilePath = val;
                                continue;
                            }
                            //
                            if  ( key == "-dump-file-name" ) {
                                rArgs.ms8DumpFileName = val;
                                continue;
                            }
                            //
                        }
                        return  true;
                    }

}


/******************************************************************************
*
*******************************************************************************/
status_t
PreviewClient::
dump(int fd, Vector<String8>& args)
{
    if  ( args.empty() ) {
        MY_LOGW("empty arguments");
        return  OK;
    }
    //
    MY_LOGD("args(%d)=%s", args.size(), (*args.begin()).string());
    //
    DumpArgument dumpArgument;
    parseDumpArgument(args, dumpArgument);
    //
    //  Command: "testImgBufProcessor <-loop-count=30> <-sleep-in-us=33333>"
    if  ( *args.begin() == "testImgBufProcessor" )
    {
        sp<Thread> pTestThread = new TestThread(mpImgBufQueue, dumpArgument.miLoopCount, dumpArgument.miSleepInUs);
        if  ( pTestThread != 0 )
        {
            pTestThread->run();
            pTestThread = 0;
        }
        return  OK;
    }
    //
    //  Command: "dumpImgBuf <-dump-file-count=1> <-dump-file-path=sdcard/camera_dump> <-dump-file-name=prvcb>"
    if  ( *args.begin() == "dumpImgBuf" )
    {
        makePath(dumpArgument.ms8DumpFilePath.string(), 0660);
        int const iDumpCount= dumpArgument.miDumpFileCount;
        String8 s8DumpPath  = dumpArgument.ms8DumpFilePath + "/" + dumpArgument.ms8DumpFileName;
        //
        Mutex::Autolock _lock(mDumpMtx);
        ::android_atomic_write(iDumpCount, &mi4DumpImgBufCount);
        ms8DumpImgBufPath = s8DumpPath;
        //
        return  OK;
    }
    //
    //  Command: "show"
    if  ( *args.begin() == "show" )
    {
        String8 s8OutputTemp;
        s8OutputTemp += String8::format("[Dump Image Buffer] %d buffers are not savaing... \n", mi4DumpImgBufCount);
        s8OutputTemp += String8::format("[performPreviewCallback] fps: %3f \n", mProfile_callback.getFps());
        s8OutputTemp += String8::format("[buffer timestamp] duration: %lld ms \n", ::ns2ms(mProfile_buffer_timestamp.getDuration()));
        s8OutputTemp += String8::format("[dequeProcessor] duration: %lld ms \n", ::ns2ms(mProfile_dequeProcessor.getDuration()));
        s8OutputTemp += "\n";
        ::write(fd, s8OutputTemp.string(), s8OutputTemp.size());
        //
        return  OK;
    }
    //
    //
    return  OK;
}

