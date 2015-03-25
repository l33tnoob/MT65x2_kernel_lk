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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <camera/MtkCamera.h>
//
#include <mtkcam/hal/sensor_hal.h>
//
#include <mtkcam/v1/IParamsManager.h>
//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkDefaultCamAdapter.h>
using namespace NSMtkDefaultCamAdapter;
//
#include <cutils/properties.h>
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
/******************************************************************************
*
*******************************************************************************/
namespace {

/**
 * [startSmoothZoom]
 *
 * Zooms to the requested value smoothly. The driver will notify {@link
 * OnZoomChangeListener} of the zoom value and whether zoom is stopped at
 * the time. For example, suppose the current zoom is 0 and startSmoothZoom
 * is called with value 3. The
 * {@link Camera.OnZoomChangeListener#onZoomChange(int, boolean, Camera)}
 * method will be called three times with zoom values 1, 2, and 3.
 * Applications can call {@link #stopSmoothZoom} to stop the zoom earlier.
 * Applications should not call startSmoothZoom again or change the zoom
 * value before zoom stops. If the supplied zoom value equals to the current
 * zoom value, no zoom callback will be generated. This method is supported
 * if {@link android.hardware.Camera.Parameters#isSmoothZoomSupported}
 * returns true.
 *
 * @param value zoom value. The valid range is 0 to {@link
 *              android.hardware.Camera.Parameters#getMaxZoom}.
 * @throws IllegalArgumentException if the zoom value is invalid.
 * @throws RuntimeException if the method fails.
 * @see #setZoomChangeListener(OnZoomChangeListener)
*/

/**
 * [stopSmoothZoom]
 *
 * Stops the smooth zoom. Applications should wait for the {@link
 * OnZoomChangeListener} to know when the zoom is actually stopped. This
 * method is supported if {@link
 * android.hardware.Camera.Parameters#isSmoothZoomSupported} is true.
 *
 * @throws RuntimeException if the method fails.
 */


// Destroy Criteria 
// (1) stopSmoothZoom (should wait at least one frame has been callback)
// (2) reach targetZoom naturally
// (3) uninit by camadapter
// (4) resett for startSmoothZoom

class ZoomCallBack : public IPreviewCmdQueCallBack 
{

private:
    mutable Mutex       mLock;
    sp<CamMsgCbInfo>    mspCamMsgCbInfo; 
    sp<IParamsManager>  mspParamsMgr;    
    int32_t             mTargetIdx;
    int32_t             mIniIdx;
    int32_t             mPrevIdx;
    int32_t             mCBcnt;
    bool                mRequestStop;
    Condition           mCondReqStop;
    
private:
    void                reset() 
                        {
                            mspCamMsgCbInfo = 0;
                            mspParamsMgr = 0;
                            mTargetIdx = mIniIdx = mCBcnt = 0; 
                            mPrevIdx = -1;
                            mRequestStop = false;
                        }

// for CTS case, if stopSmoothZoom follows with startSmoothZoom quickly,
// must quarentee one callback at least has been sent to AP. 
public:
    bool                requestStop();
        
public:

    //  Notify Callback of Zoom
    //  Arguments:
    //
    //      _msgType: 
    //       
    //
    //      _ext1:

    virtual void        doNotifyCb (
                            int32_t _msgType, 
                            int32_t _ext1, 
                            int32_t _ext2,
                            int32_t _ext3
                        );
    
// class member
public:
    
    virtual             ~ZoomCallBack(){}
                        ZoomCallBack()
                            : mLock()                            
                            , mspCamMsgCbInfo(0)
                            , mspParamsMgr(0)
                            , mTargetIdx(0)
                            , mIniIdx(0)
                            , mPrevIdx(-1)
                            , mRequestStop(false)
                        {}
    
    virtual void        setZoomTarget(int32_t cIdx, int32_t tIdx)
                        {
                            mIniIdx = cIdx;
                            mTargetIdx = tIdx; 
                            mPrevIdx = -1;
                        }   

    virtual void        setUser(sp<CamMsgCbInfo> const &rpCamMsgCbInfo,
                                sp<IParamsManager> const &rpParamsMgr) 
                        { 
                            Mutex::Autolock lock(mLock);
                            
                            reset(); //destroy criteria (4)
                            mspCamMsgCbInfo = rpCamMsgCbInfo; 
                            mspParamsMgr = rpParamsMgr;
                        }       
    
    virtual void        destroyUser() // destroy criteria (3) 
                        {
                            Mutex::Autolock lock(mLock);
                            reset();
                        }
};


/******************************************************************************
* 
*******************************************************************************/
bool
ZoomCallBack::
requestStop()
{
    Mutex::Autolock lock(mLock);
    
    mRequestStop = true;
    nsecs_t nsTimeoutToWait = 33LL*1000LL*1000LL;//wait 33 msecs.    
    status_t status = mCondReqStop.waitRelative(mLock, nsTimeoutToWait);

    return status == OK;
}


/******************************************************************************
* 
*******************************************************************************/

void
ZoomCallBack::
doNotifyCb(
    int32_t _msgType, 
    int32_t _ext1, 
    int32_t _ext2,
    int32_t _ext3
)
{
    Mutex::Autolock lock(mLock);

    if( mspCamMsgCbInfo == 0)
    {
        CAM_LOGW("mspCamMsgCbInfo == 0");
        return;
    }
    
    if (mPrevIdx == _ext1)
    {
        CAM_LOGW("same as previous idx, drop!");
        return;
    }
    //
    bool retStop = false;
    if ( _msgType == IPreviewCmdQueCallBack::eID_NOTIFY_Zoom &&
         _ext1 != mIniIdx )
    {
        retStop = (mRequestStop)||(_ext1 == mTargetIdx);
        CAM_LOGD("smoothZoom cb(#%d): (%d, %d)", mCBcnt++, _ext1, retStop); 
        mspParamsMgr->set(CameraParameters::KEY_ZOOM, _ext1);        
        mspCamMsgCbInfo->mNotifyCb(CAMERA_MSG_ZOOM, _ext1,  retStop, mspCamMsgCbInfo->mCbCookie);
        mPrevIdx = _ext1;
    }

    //
    if ( retStop ) // destroy criteria (1)&(2)
    {
        mCondReqStop.broadcast(); 
        reset();
    }    
}

static ZoomCallBack gZoomCallback;

}; // end of namespace


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::startSmoothZoom(int32_t arg1)
{
    MY_LOGD("+ (%d)", arg1);

    int32_t curIdx = mpParamsMgr->getInt(CameraParameters::KEY_ZOOM);

    if( arg1 < 0 || arg1 > mpParamsMgr->getInt(CameraParameters::KEY_MAX_ZOOM) )
    {
        MY_LOGE("return fail: smooth zoom(%d)", arg1);
        return BAD_VALUE;
    }

    if( arg1 == curIdx )
    {
        MY_LOGE("smooth zoom(%d) equals to current", arg1);
        return OK; 
    }
    
    // (0) initial setting
    gZoomCallback.setUser(mpCamMsgCbInfo, mpParamsMgr);
    gZoomCallback.setZoomTarget(curIdx, arg1);
        
    // (1) once callback has been set to PreviewCmdQueThread,
    //     it contains this pointer forever, 
    mpPreviewCmdQueThread->setZoomCallback(&gZoomCallback);

    //(2)
    while( mpPreviewCmdQueThread->popZoom() != -1 );

    // (3)
    if (curIdx < arg1)
    {
        for (int32_t i = curIdx+1; i <= arg1; i++)
        {
            mpPreviewCmdQueThread->pushZoom((uint32_t)i);
        }
    }
    else if (curIdx > arg1)
    {
        for (int32_t i = curIdx-1; i >= arg1; i--)
        {
            mpPreviewCmdQueThread->pushZoom((uint32_t)i);
        }
    }
    
    MY_LOGD("-");
    
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::stopSmoothZoom()
{
    MY_LOGD("+");

    //
    int popIdx = -1;
    Vector<int> vPopIdx;
    while( -1 != (popIdx = mpPreviewCmdQueThread->popZoom()) )
    {
        vPopIdx.push_back(popIdx);
    }


    // To guarentee:
    // (1) must send one callback if AP has ever sent startSmoothZoom;
    // (2) the last callback must indicate "stop";  
    while( ! vPopIdx.empty() && ! gZoomCallback.requestStop())
    {
        MY_LOGD("Push Idx(%d) for compensation", *(vPopIdx.end()-1));
        mpPreviewCmdQueThread->pushZoom(*(vPopIdx.end()-1));
        vPopIdx.erase(vPopIdx.end()-1);
    }
    
    MY_LOGD("-");    
    
    return OK;
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::uninitSmoothZoom()
{
    gZoomCallback.destroyUser();
}


