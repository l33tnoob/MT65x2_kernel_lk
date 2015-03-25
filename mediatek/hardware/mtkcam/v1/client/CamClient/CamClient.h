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

#ifndef _MTK_CAMERA_CLIENT_CAMCLIENT_CAMCLIENT_H_
#define _MTK_CAMERA_CLIENT_CAMCLIENT_CAMCLIENT_H_
//


namespace android {
namespace NSCamClient {


/******************************************************************************
*   Camera Client Callback Handler.
*******************************************************************************/
class CamClient : public ICamClient
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IClientCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual bool                    init();
    virtual bool                    uninit();
    /**
     *
     */
    virtual bool                    setImgBufProviderClient(
                                        sp<IImgBufProviderClient>const& rpClient
                                    );

    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo);

    virtual void                    enableMsgType(int32_t msgType);
    virtual void                    disableMsgType(int32_t msgType);
    virtual bool                    msgTypeEnabled(int32_t msgType);

    /**
     *
     */
    virtual bool                    startPreview();
    virtual void                    stopPreview();

    /**
     *
     */
    virtual void                    takePicture();

    /**
     * Start record mode. When a record image is available a CAMERA_MSG_VIDEO_FRAME
     * message is sent with the corresponding frame. Every record frame must be released
     * by a cameral hal client via releaseRecordingFrame() before the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME). After the client calls
     * disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's responsibility
     * to manage the life-cycle of the video recording frames, and the client must
     * not modify/access any video recording frames.
     */
    virtual bool                    startRecording();

    /**
     * Stop a previously started recording.
     */
    virtual void                    stopRecording();

    /**
     * Release a record frame previously returned by CAMERA_MSG_VIDEO_FRAME.
     *
     * It is camera hal client's responsibility to release video recording
     * frames sent out by the camera hal before the camera hal receives
     * a call to disableMsgType(CAMERA_MSG_VIDEO_FRAME). After it receives
     * the call to disableMsgType(CAMERA_MSG_VIDEO_FRAME), it is camera hal's
     * responsibility of managing the life-cycle of the video recording
     * frames.
     */
    virtual void                    releaseRecordingFrame(const void *opaque);

    /**
     * Send command to camera driver.
     */
    virtual status_t                sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

    /**
     *
     */
    virtual status_t                dump(int fd, Vector<String8>& args);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    //
                                    CamClient(sp<IParamsManager> pParamsMgr);
    virtual                         ~CamClient();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Common Info.
    mutable Mutex                   mModuleMtx;

    //  Pointer to Camera Message-Callback Info.
    sp<CamMsgCbInfo>                mpCamMsgCbInfo;

    //  Pointer to Parameters Manager.
    sp<IParamsManager>              mpParamsMgr;

    //  Pointer to the client of Image Buffer Provider (i.e. a client is a provider-IF user of IImgBufQueue).
    sp<IImgBufProviderClient>       mpImgBufPvdrClient;

protected:  ////                    Clients.
    sp<IPreviewClient>              mpPreviewClient;
    sp<IRecordClient>               mpRecordClient;
    sp<IFDClient>                   mpFDClient;
#if '1'==MTKCAM_HAVE_OT_CLIENT
    sp<IOTClient>                   mpOTClient;        
#endif
    sp<IPREFEATUREClient>           mpPreviewFeatureClient;
};


}; // namespace NSCamClient
}; // namespace android
#endif  //_MTK_CAMERA_CLIENT_CAMCLIENT_CAMCLIENT_H_

