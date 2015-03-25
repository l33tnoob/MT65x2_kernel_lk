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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_





namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *
 ******************************************************************************/
class ZsdShot : public ImpShot
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~ZsdShot();
                                    ZsdShot(
                                        char const*const pszShotName,
                                        uint32_t const u4ShotMode,
                                        int32_t const i4OpenId
                                    );

public:     ////                    Operations.

    //  This function is invoked when this object is firstly created.
    //  All resources can be allocated here.
    virtual bool                    onCreate();

    //  This function is invoked when this object is ready to destryoed in the
    //  destructor. All resources must be released before this returns.
    virtual void                    onDestroy();

    virtual bool                    sendCommand(
                                        uint32_t const  cmd,
                                        uint32_t const  arg1,
                                        uint32_t const  arg2
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_reset();
    virtual bool                    onCmd_capture();
    virtual void                    onCmd_cancel();


protected:  ////                    callbacks
    static MBOOL fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    static MBOOL fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg);

protected:
    MBOOL           handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size);
    MBOOL           handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize);

protected:
    bool                            onCmd_setCaptureBufHandler(uint32_t const arg1, uint32_t const arg2);
    bool                            onYuv_capture(CapBufQueNode & rNode);
    bool                            onRaw_capture(CapBufQueNode & rNode);
    bool                            checkIfNeedImgTransform(ImgBufInfo const & rSrcBufInfo, ImgBufInfo const & rDstBufInfo);
    bool                            checkIfImgTransformSupport(ImgBufInfo const & rSrcBufInfo, ImgBufInfo const & rDstBufInfo, int rot);

    bool                            createJpegImg(ImgBufInfo const & rSrcImgBufInfo,
                                                MUINT32 const u4u4Quality,
                                                MUINT32 const u4fgIsSOI,
                                                ImgBufInfo const & rJpgImgBufInfo,
                                                MUINT32 & u4JpegSize);

    bool                            imageTransform  (ImgBufInfo const & rSrcImgBufInfo, ImgBufInfo const & rDstImgBufInfo, int const & rot);
    void                            setImageBuf(EImageFormat eFmt, MUINT32 const u4Width, MUINT32 const u4Height, ImgBufInfo & rBuf, IMEM_BUF_INFO & rMem);
    void                            mapNodeToImageBuf(ImgBufQueNode & rNode, ImgBufInfo & rBuf);
    bool                            allocMem(IMEM_BUF_INFO & rMemBuf);
    bool                            deallocMem(IMEM_BUF_INFO & rMemBuf);
    bool                            onYuv_alloc(ImgBufInfo   & rJpgImgBufInfo, ImgBufInfo & rYuvImgBufInfo, ImgBufInfo & rThumbImgBufInfo);
    bool                            onYuv_free  ();
    MUINT32                         queryImgStride(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4PlaneIndex);
    MUINT32                         queryImgBufSize(EImageFormat const eFmt, MUINT32 const u4Width, MUINT32 const u4Height);
    sp<ICaptureBufMgrHandler>       mpCaptureBufHandler;

    MUINT32                         mShotMode; // 0: ZSDCC 1: ZSDNCC
    // for YUV capture
    IMEM_BUF_INFO mJpegMem;
    IMEM_BUF_INFO mYuvMem;
    IMEM_BUF_INFO mThumbnailMem;
    IMemDrv       *mpMemDrv;
    MUINT32         mu4DumpFlag;
};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_ZSDSHOT_H_

