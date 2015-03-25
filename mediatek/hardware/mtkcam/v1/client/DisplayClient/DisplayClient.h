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

#ifndef _MTK_CAMERA_CLIENT_DISPLAYCLIENT_DISPLAYCLIENT_H_
#define _MTK_CAMERA_CLIENT_DISPLAYCLIENT_DISPLAYCLIENT_H_
//
#include <CamUtils.h>
#include <mtkcam/v1/ExtImgProc/IExtImgProc.h>
#include <mtkcam/v1/ExtImgProc/ExtImgProc.h>
using namespace android;
using namespace MtkCamUtils;
#include <mtkcam/v1/IDisplayClient.h>
#include "StreamImgBuf.h"
#include "DisplayThread.h"


namespace android {
namespace NSDisplayClient {
/******************************************************************************
*
*******************************************************************************/
class ProfileTool
{
protected:  ////        Data Members.
    String8             ms8SubjectName;
                        //
    float               mfFps;
    nsecs_t             mnsDuration;
                        //
    int32_t             mi4Count;
    nsecs_t             mnsStart;
    nsecs_t             mnsEnd;

public:     ////        Interfaces.
                        ProfileTool(char const*const pszSubjectName);
                        //
    nsecs_t             getDuration() const     { return mnsDuration; }
    float               getFps() const          { return mfFps; }
                        //
    void                pulse();
    void                pulse(nsecs_t nsTimestamp);
                        //
    void                reset();
    void                reset(nsecs_t nsInitTimestamp);
                        //
    void                updateFps();
                        //
    void                showFps() const;
};


/******************************************************************************
*
*******************************************************************************/
class DisplayClient : public IDisplayClient
                    , public IDisplayThreadHandler
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDisplayClient Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * Initialize the display adapter to create any resources required.
     */
    virtual bool                    init();
    /**
     * Uninitialize the display adapter.
     */
    virtual bool                    uninit();

    /**
     * Set the preview_stream_ops to which frames are sent.
     *
     * Notes:
     *  (1) When calling setWindow(), all preview parameters have been decided.
     *      [CameraService]
     *          mHardware->setParameters() -> mHardware->setPreviewWindow() -> mHardware->startPreview()
     *          --> enableDisplay during startPreview()
     *          mHardware->setParameters() -> mHardware->startPreview() -> mHardware->setPreviewWindow()
     *          --> enableDisplay during setPreviewWindow()
     *  (2) During inactive preview, window may be changed by setWindow().
     */
    virtual bool                    setWindow(
                                        preview_stream_ops*const window, 
                                        int32_t const   wndWidth, 
                                        int32_t const   wndHeight, 
                                        int32_t const   i4MaxImgBufCount
                                    );
    /**
     *
     */
    virtual bool                    setImgBufProviderClient(
                                        sp<IImgBufProviderClient>const& rpClient
                                    );

    /**
     *
     */
    virtual bool                    isDisplayEnabled() const;
    virtual bool                    disableDisplay();
    virtual bool                    enableDisplay();
    virtual bool                    enableDisplay(
                                        int32_t const   i4Width, 
                                        int32_t const   i4Height, 
                                        int32_t const   i4BufCount, 
                                        sp<IImgBufProviderClient>const& rpClient
                                    );
    virtual bool                    checkConfig(
                                        int32_t const   i4Width, 
                                        int32_t const   i4Height, 
                                        int32_t const   i4BufCount, 
                                        sp<IImgBufProviderClient>const& rpClient
                                    );

    virtual status_t                waitUntilDrained();

    /**
     *
     */
    virtual status_t                dump(int fd, Vector<String8>const& args);

public:     ////                    Instantiation.
                                    DisplayClient();
    virtual                         ~DisplayClient();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Display Thread.
    bool                            createDisplayThread();
    void                            destroyDisplayThread();

protected:  ////                    ImgBufQueue.
    bool                            createImgBufQueue();
    void                            destroyImgBufQueue();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDisplayThreadHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Interfaces
    // Derived class must implement the below function. The thread starts its
    // life here. There are two ways of using the Thread object:
    // 1) loop: if this function returns true, it will be called again if 
    //          requestExit() wasn't called.
    // 2) once: if this function returns false, the thread will exit upon return.
    virtual bool                    onThreadLoop(Command const& rCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Common Info.
    //
    mutable Mutex                   mModuleMtx;
    //
    //  Display Thread
    sp<IDisplayThread>              mpDisplayThread;
    //
    int32_t volatile                mIsDisplayEnabled;

                                    enum EState
                                    {
                                        eState_Suspend, 
                                        eState_Loop, 
                                    };
    EState volatile                 mState;
    Condition                       mStateCond;
    Mutex                           mStateMutex;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  mpStreamOps <-> mStreamBufList <-> mpImgBufQueue
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    //
//    mutable Mutex                 mStreamOpsMtx;

    //  Pointer to Image Info: w/h/fmt/...
    sp<ImgInfo>                     mpStreamImgInfo;

    //  Pointer to preview_stream_ops
    preview_stream_ops*             mpStreamOps;

    //  preview_stream_ops-related buffer list.
    //  Notes:
    //      mStreamBufList.size() <= CAMERA_WANTED_BUF_COUNT
    typedef List< sp<StreamImgBuf> > StreamBufList_t;
    StreamBufList_t                 mStreamBufList;

    //  Max image buffer count.
    int32_t                         mi4MaxImgBufCount;

    //  Pointer to IImgBufQueue.
    sp<IImgBufQueue>                mpImgBufQueue;
    //  Pointer to the client of Image Buffer Provider (i.e. a client is a provider-IF user of mpImgBufQueue).
    sp<IImgBufProviderClient>       mpImgBufPvdrClient;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    preview_stream_ops related Operations.
    void                            dumpDebug(
                                        StreamBufList_t const& rQue, 
                                        char const*const pszDbgText = ""
                                    );
    //
    bool                            set_preview_stream_ops(
                                        preview_stream_ops*const window, 
                                        int32_t const wndWidth, 
                                        int32_t const wndHeight, 
                                        int32_t const i4MaxImgBufCount
                                    );
    //
    bool                            dequePrvOps(sp<StreamImgBuf>& rpImgBuf);
    void                            enquePrvOps(sp<StreamImgBuf>const& rpImgBuf);
    void                            cancelPrvOps(sp<StreamImgBuf>const& rpImgBuf);
    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                    Buffer Operations.
    bool                            prepareOneTodoBuffer(sp<IImgBufQueue>const& rpBufQueue);
    bool                            prepareAllTodoBuffers(sp<IImgBufQueue>const& rpBufQueue);
    bool                            cancelAllUnreturnBuffers();
    bool                            waitAndHandleReturnBuffers(sp<IImgBufQueue>const& rpBufQueue);
    bool                            handleReturnBuffers(Vector<ImgBufQueNode>const& rvQueNode);
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Duration Tool
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    ProfileTool                     mProfile_enquePrvOps;
    ProfileTool                     mProfile_dequeProcessor;
    ProfileTool                     mProfile_buffer_timestamp;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Debug
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    logs
    int32_t                         miLogLevel; //0: silence, 1: debug1, 2: debug2

protected:  ////                    dump
    mutable Mutex                   mDumpMtx;
    List< Vector<uint8_t> >         mDumpImgBufList;
    //
    int32_t                         mi4DumpImgBufCount;
    int32_t                         mi4DumpImgBufIndex;
    String8                         ms8DumpImgBufPath;
    //
    ExtImgProc*                     mpExtImgProc;
protected:  ////                    dump
    void                            dumpImgBuf_If(sp<StreamImgBuf>const& rpImgBuf);
};


}; // namespace NSDisplayAdapter
}; // namespace android
#endif  //_MTK_CAMERA_CLIENT_DISPLAYCLIENT_DISPLAYCLIENT_H_

