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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxBase.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX component base class
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/
 
#ifndef MTK_OMX_BASE
#define MTK_OMX_BASE

#include "OMX_Core.h"
#include "OMX_Component.h"

typedef enum MTK_OMX_PORT_INDEX
{
    MTK_OMX_INPUT_PORT     = OMX_DirInput,
    MTK_OMX_OUTPUT_PORT  = OMX_DirOutput,
#ifdef MTK_AIV_SUPPORT
    MTK_OMX_VIDEO_INPUT_PORT   = OMX_DirVideoInput,
    MTK_OMX_AUDIO_INPUT_PORT   = OMX_DirAudioInput,
#endif
    MTK_OMX_ALL_PORT         = OMX_ALL,
    MTK_OMX_INVALID_PORT = 0xFFFFFFFF
} MTK_OMX_PORT_INDEX;


typedef enum MTK_OMX_BUFFER_TYPE_ID
{
    MTK_OMX_EMPTY_THIS_BUFFER_TYPE,
    MTK_OMX_FILL_THIS_BUFFER_TYPE,
#if 0    
    MTK_OMX_EOMXMPTY_BUFFER_DONE_TYPE,   
    MTK_OMX_FILL_BUFFER_DONE_TYPE
#endif    
} MTK_OMX_BUFFER_TYPE_ID;


typedef enum MTK_OMX_COMMAND_CATEGORY
{
    MTK_OMX_GENERAL_COMMAND,
    MTK_OMX_BUFFER_COMMAND,
    MTK_OMX_STOP_COMMAND
} MTK_OMX_COMMAND_CATEGORY;


typedef enum MTK_OMX_PIPE_ID
{
    MTK_OMX_PIPE_ID_READ = 0,
    MTK_OMX_PIPE_ID_WRITE = 1,
} MTK_OMX_PIPE_ID;


#define MTK_OMX_IDLE_PENDING                           (1 << 0)
#define MTK_OMX_IN_PORT_ENABLE_PENDING      (1 << 1)
#define MTK_OMX_OUT_PORT_ENABLE_PENDING   (1 << 2)
#define MTK_OMX_LOADED_PENDING                      (1 << 3)
#ifdef MTK_AIV_SUPPORT
#define MTK_OMX_VIDEO_IN_PORT_ENABLE_PENDING      (1 << 4)
#define MTK_OMX_AUDIO_IN_PORT_ENABLE_PENDING      (1 << 5)
#endif

#define SET_PENDING(flag) (mPendingStatus |= flag);
#define IS_PENDING(flag) (mPendingStatus & flag)
#define CLEAR_PENDING(flag) (mPendingStatus &= ~flag)


class MtkOmxBase{
public:
    virtual ~MtkOmxBase() {}
    virtual OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                                                      OMX_IN OMX_STRING componentName) = 0;

    virtual OMX_ERRORTYPE  ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent) = 0;

    virtual OMX_ERRORTYPE  GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                                                                       OMX_IN OMX_STRING componentName,
                                                                                       OMX_OUT OMX_VERSIONTYPE* componentVersion,
                                                                                       OMX_OUT OMX_VERSIONTYPE* specVersion,
                                                                                       OMX_OUT OMX_UUIDTYPE* componentUUID) = 0;
    
    virtual OMX_ERRORTYPE  SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                                                        OMX_IN OMX_COMMANDTYPE Cmd,
                                                                        OMX_IN OMX_U32 nParam1,
                                                                        OMX_IN OMX_PTR pCmdData) = 0;

    virtual OMX_ERRORTYPE  GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                                                      OMX_IN  OMX_INDEXTYPE nParamIndex,
                                                                      OMX_INOUT OMX_PTR ComponentParameterStructure) = 0;

    virtual OMX_ERRORTYPE  SetParameter(OMX_IN OMX_HANDLETYPE hComp, 
                                                                     OMX_IN OMX_INDEXTYPE nParamIndex,
                                                                     OMX_IN OMX_PTR pCompParam) = 0;

    virtual OMX_ERRORTYPE  GetConfig(OMX_IN OMX_HANDLETYPE hComponent, 
                                                              OMX_IN OMX_INDEXTYPE nConfigIndex,
                                                              OMX_INOUT OMX_PTR ComponentConfigStructure) = 0;

    virtual OMX_ERRORTYPE  SetConfig(OMX_IN OMX_HANDLETYPE hComponent, 
                                                              OMX_IN OMX_INDEXTYPE nConfigIndex,
                                                              OMX_IN OMX_PTR ComponentConfigStructure) = 0;

    virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
    	                                                                       OMX_IN OMX_STRING parameterName,
    	                                                                       OMX_OUT OMX_INDEXTYPE* pIndexType) = 0;

    virtual OMX_ERRORTYPE  GetState(OMX_IN OMX_HANDLETYPE hComponent, 
                                                            OMX_INOUT OMX_STATETYPE* pState) = 0;

    virtual OMX_ERRORTYPE  UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                                              OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                                              OMX_IN OMX_U32 nPortIndex,
                                                              OMX_IN OMX_PTR pAppPrivate,
                                                              OMX_IN OMX_U32 nSizeBytes,
                                                              OMX_IN OMX_U8* pBuffer) = 0;


    virtual OMX_ERRORTYPE  AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                                                      OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffHead,
                                                                      OMX_IN OMX_U32 nPortIndex,
                                                                      OMX_IN OMX_PTR pAppPrivate,
                                                                      OMX_IN OMX_U32 nSizeBytes) = 0;


    virtual OMX_ERRORTYPE  FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                                               OMX_IN OMX_U32 nPortIndex,
                                                               OMX_IN OMX_BUFFERHEADERTYPE* pBuffHead) = 0;


    virtual OMX_ERRORTYPE  EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent, 
                                                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffHead) = 0;


    virtual OMX_ERRORTYPE  FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                                                    OMX_IN OMX_BUFFERHEADERTYPE* pBuffHead) = 0;

    virtual OMX_ERRORTYPE  SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent, 
                                                                   OMX_IN OMX_CALLBACKTYPE* pCallBacks, 
                                                                   OMX_IN OMX_PTR pAppData) = 0;

    virtual OMX_ERRORTYPE  ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                                                                     OMX_OUT OMX_U8 *cRole,
                                                                                     OMX_IN OMX_U32 nIndex) = 0;

    OMX_COMPONENTTYPE* GetComponentHandle() { return &mCompHandle; }

    void SetCoreGlobal(void* pData) { this->mCoreGlobal = pData; }
    void* GetCoreGlobal() { return this->mCoreGlobal; }

protected:
	OMX_COMPONENTTYPE mCompHandle;  // OMX component handle
	OMX_U8                       mCompName[OMX_MAX_STRINGNAME_SIZE];
       OMX_U8                       mCompRole[OMX_MAX_STRINGNAME_SIZE];
	OMX_CALLBACKTYPE    mCallback;        // {EmptyBufferDone, FillBufferDone, EventHandler} from IL client
	OMX_PTR                     mAppData;        // application specific data
	OMX_STATETYPE	       mState;             // component state
  OMX_MARKTYPE* mpMark;
  OMX_COMPONENTTYPE*  mpTargetComponent;
  OMX_COMPONENTTYPE*  mpPrevTargetComponent;
  OMX_PTR             mTargetMarkData;
  OMX_PTR             mPrevTargetMarkData;//Changqing

       OMX_PARAM_PORTDEFINITIONTYPE mInputPortDef;    // input port definition
       OMX_PARAM_PORTDEFINITIONTYPE mOutputPortDef;  // output port definition

#ifdef MTK_AIV_SUPPORT
       OMX_PARAM_PORTDEFINITIONTYPE mVideoInputPortDef;    // video input port definition
       OMX_PARAM_PORTDEFINITIONTYPE mAudioInputPortDef;    // video input port definition
#endif

       OMX_BUFFERHEADERTYPE** mInputBufferHdrs;	// OMX_BUFFERHEADERTYPE* array to keep the input buffers
       OMX_BUFFERHEADERTYPE** mOutputBufferHdrs;    // OMX_BUFFERHEADERTYPE* array to keep the output buffers

#ifdef MTK_AIV_SUPPORT
       OMX_BUFFERHEADERTYPE** mVideoInputBufferHdrs;	// OMX_BUFFERHEADERTYPE* array to keep the video input buffers
       OMX_BUFFERHEADERTYPE** mAudioInputBufferHdrs;	// OMX_BUFFERHEADERTYPE* array to keep the audio input buffers
#endif

       OMX_U32 mInputBufferPopulatedCnt;			// keep tracking how many input buffers have been populated
       OMX_U32 mOutputBufferPopulatedCnt;               // keep tracking how many output buffers have been populated

#ifdef MTK_AIV_SUPPORT
       OMX_U32 mVideoInputBufferPopulatedCnt;			// keep tracking how many video input buffers have been populated
       OMX_U32 mAudioInputBufferPopulatedCnt;			// keep tracking how many audio input buffers have been populated
#endif

       OMX_BOOL mPortReconfigInProgress;

    OMX_PTR mCoreGlobal;
};

#endif
