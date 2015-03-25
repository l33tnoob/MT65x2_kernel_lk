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

#define LOG_TAG "MtkCam/CapBufMgr"

#include <utils/List.h>
#include <utils/Vector.h>

#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;



#include <adapter/inc/ImgBufProvidersManager.h>
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>
using namespace NSHwScenario;
//
//#include <mtkcam/v1/IParamsManager.h>
//#include <inc/PreviewCmdQueThread.h>

#include <inc/ICaptureBufMgr.h>
#include <linux/cache.h>


using namespace NSMtkZsdCcCamAdapter;

//


/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt,  __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt,  __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")


namespace android {
namespace NSMtkZsdCcCamAdapter {

/******************************************************************************
 *
 ******************************************************************************/
class CaptureBufMgr : public ICaptureBufMgr
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPreviewBufMgr Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool            dequeProvider(CapBufQueNode& rNode);
    virtual bool            dequeProvider(list<CapBufQueNode>* pvNode);
    virtual bool            enqueProvider(CapBufQueNode& rNode, bool bIsFilled);
    virtual bool            enqueProvider(unsigned int va, bool bIsFilled) {return false;};

    virtual bool            dequeProcessor(CapBufQueNode& rNode, int index);
    virtual bool            enqueProcessor(CapBufQueNode& rNode);

    virtual void            allocBuffer(int w, int h, const char* format, int rotation,
                                        int _w, int _h, const char* _format, int cnt);
    virtual void            reallocBuffer(int w, int h, const char* format,
                                           int _w, int _h, const char* _format, int cnt);

    virtual void            freeBuffer();

    virtual int32_t            getStoredBufferCnt();
    virtual void            setStoredBufferCnt(int32_t const cnt);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    CaptureBufMgr();
    virtual ~CaptureBufMgr();
    virtual void destroyInstance();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 private:
    list<CapBufQueNode>           mImgBufQue; //raw or yuv

    mutable Mutex                 mImgBufQueMtx;
    Condition                     mImgBufQueCond; //  Condition to wait:

    sp<HwBuffProvider>            mspHwBufPvdr;
    int                           mStoredBufCnt;

    bool                          mbBufAlloced;
    int                           miBufWidth;
    int                           miBufHeight;
    int                           miBufCnt;
};

};
};


/*******************************************************************************
*
********************************************************************************/
void
CaptureBufMgr::
freeBuffer()
{

    MY_LOGD("free buffer");

    if (mImgBufQue.size() < miBufCnt) {
        MY_LOGW("to free cnt(%d) < total cnt(%d)", mImgBufQue.size(), miBufCnt);
    }
    mImgBufQue.clear();

    mbBufAlloced = false;
}


/*******************************************************************************
*
********************************************************************************/
void
CaptureBufMgr::
allocBuffer(int w, int h, const char* format, int rotation,
            int _w, int _h, const char* _format, int cnt)

{

    if (mbBufAlloced)
    {
        MY_LOGD("Buffer has been allocaed");
        if (w != miBufWidth || h != miBufHeight || cnt != miBufCnt)
        {
            MY_LOGE("Buffer is not same");
        }
        return;
    }
    miBufCnt = cnt;
    miBufWidth = w;
    miBufHeight = h;
    for (int i = 0; i < cnt; i++)
    {
        CapBufQueNode node;

        sp<CapBuffer> mainBuf = new CapBuffer(w, h, format);
        node.mainImgNode = ImgBufQueNode(mainBuf, ImgBufQueNode::eSTATUS_TODO);
        node.mainImgNode.setRotation(rotation);
               // thumb nail buffer
        if (_w != 0 && _h != 0 && _format != NULL) {
            sp<CapBuffer> subBuf = new CapBuffer(_w, _h, _format);
            node.subImgNode = ImgBufQueNode(subBuf, ImgBufQueNode::eSTATUS_TODO);
        }

        mImgBufQue.push_back(node);
    }
    mbBufAlloced = true;


}


/*******************************************************************************
*
********************************************************************************/
void
CaptureBufMgr::
reallocBuffer(int w, int h, const char* format,
               int _w, int _h, const char* _format, int cnt)
{
    freeBuffer();
    allocBuffer(w, h, format,0, _w, _h, _format, cnt);
}

/*******************************************************************************
*
********************************************************************************/
int32_t
CaptureBufMgr::
getStoredBufferCnt()
{
    return mStoredBufCnt;
}
/*******************************************************************************
*
********************************************************************************/
void
CaptureBufMgr::
setStoredBufferCnt(int32_t const cnt)
{
    mStoredBufCnt = cnt;
}



/******************************************************************************
*   REPEAT:[ dequeProvider() -> enqueProvider() ]
*   dequeProvider() returns false immediately if empty.
*******************************************************************************/
bool
CaptureBufMgr::
dequeProvider(CapBufQueNode& rNode)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mImgBufQueMtx);
    //
    if  ( ! mImgBufQue.empty() )
    {
        //  If the queue is not empty, take the first buffer from the queue.
        ret = true;
        rNode = *mImgBufQue.begin();
        rNode.mainImgNode.setStatus(ImgBufQueNode::eSTATUS_TODO);
        mImgBufQue.erase(mImgBufQue.begin());
        MY_LOGD_IF(
            ENABLE_LOG_PER_FRAME,
            "+ Que.size(%d); Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]",
            mImgBufQue.size(),
            rNode.mainImgNode->getBufName(), rNode.mainImgNode->getVirAddr(), rNode.mainImgNode->getBufSize(), rNode.mainImgNode->getImgFormat().string(),
            rNode.mainImgNode->getImgWidthStride(), rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight(),
            rNode.mainImgNode->getBitsPerPixel(), rNode.mainImgNode->getTimestamp()
        );

    }
    else
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Empty Que");
    }
    //
    return ret;
}


bool
CaptureBufMgr::
dequeProvider(list<CapBufQueNode>* pvNode)
{
    bool ret = false;
    CapBufQueNode rNode;
    //
    Mutex::Autolock _lock(mImgBufQueMtx);
    //
    if  ( ! mImgBufQue.empty() )
    {
        //  If the queue is not empty, take the first buffer from the queue.
        list<CapBufQueNode>::iterator it;
        for (it = mImgBufQue.begin(); it != mImgBufQue.end(); it++ ) {
            if ( mImgBufQue.size() > mStoredBufCnt){
                rNode = *mImgBufQue.begin();
                rNode.mainImgNode.setStatus(ImgBufQueNode::eSTATUS_TODO);
                mImgBufQue.erase(mImgBufQue.begin());
                pvNode->push_back(rNode);
                MY_LOGD_IF(
                    ENABLE_LOG_PER_FRAME,
                    "+ Que.size(%d); Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]",
                    mImgBufQue.size(),
                    rNode.mainImgNode->getBufName(), rNode.mainImgNode->getVirAddr(), rNode.mainImgNode->getBufSize(), rNode.mainImgNode->getImgFormat().string(),
                    rNode.mainImgNode->getImgWidthStride(), rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight(),
                    rNode.mainImgNode->getBitsPerPixel(), rNode.mainImgNode->getTimestamp()
                );
                ret = true;

            } else {
                break;
            }
        }
    }
    else
    {
        MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "Empty Que");
    }
    //
    return ret;
}


/******************************************************************************
*   REPEAT:[ dequeProvider() -> enqueProvider() ]
*   dequeProvider() returns false immediately if empty.
*******************************************************************************/
bool
CaptureBufMgr::
enqueProvider(CapBufQueNode& rNode, bool bIsFilled)
{
    if  ( ! rNode.mainImgNode ) {
        MY_LOGW("buffer is NULL");
        return  false;
    }
    //
    MY_LOGD_IF(
        ENABLE_LOG_PER_FRAME,
        "+ Que.size(%d); Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]",
        mImgBufQue.size(),
        rNode.mainImgNode->getBufName(), rNode.mainImgNode->getVirAddr(), rNode.mainImgNode->getBufSize(), rNode.mainImgNode->getImgFormat().string(),
        rNode.mainImgNode->getImgWidthStride(), rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight(),
        rNode.mainImgNode->getBitsPerPixel(), rNode.mainImgNode->getTimestamp()
    );
    //
    // If the buffer is filled by HW, put it to back of list
    // else put the buffer to front of the list.
    Mutex::Autolock _lock(mImgBufQueMtx);
    //
    if (bIsFilled) {
        rNode.mainImgNode.setStatus(ImgBufQueNode::eSTATUS_DONE);
        mImgBufQue.push_back(rNode);
        //
        mImgBufQueCond.broadcast();
        //
    } else {
        rNode.mainImgNode.setStatus(ImgBufQueNode::eSTATUS_TODO);
        mImgBufQue.push_front(rNode);
    }
    return  true;
}


/******************************************************************************
*
*******************************************************************************/
bool
CaptureBufMgr::
dequeProcessor(CapBufQueNode& rNode, int index=0)
{
    bool ret = false;

    //
    Mutex::Autolock _lock(mImgBufQueMtx);
    //
    // 1) no buffer is ready, wait
    while   ( !mImgBufQue.back().mainImgNode.isDONE() )
    {
        status_t status = mImgBufQueCond.wait(mImgBufQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), Que.size(%d)", status, mImgBufQue.size());
        }
    }

    //2) get index buffer and remove from list, set all old ready buffer's status as TODO

    list<CapBufQueNode>::iterator it;
    int cur_index = 0;
    for (it = mImgBufQue.begin(); it != mImgBufQue.end();) {
        if ((*it).mainImgNode.isDONE() && (cur_index >= (int)(mImgBufQue.size()-1 - index))){
            rNode = *it;
            it = mImgBufQue.erase(it);
            MY_LOGD_IF(
                ENABLE_LOG_PER_FRAME,
                "+ Que.size(%d);  Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]",
                mImgBufQue.size(),
                rNode.mainImgNode->getBufName(), rNode.mainImgNode->getVirAddr(), rNode.mainImgNode->getBufSize(), rNode.mainImgNode->getImgFormat().string(),
                rNode.mainImgNode->getImgWidthStride(), rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight(),
                rNode.mainImgNode->getBitsPerPixel(), rNode.mainImgNode->getTimestamp()
            );
            break;
        } else {
            if ((*it).mainImgNode.isDONE())  (*it).mainImgNode.setStatus(ImgBufQueNode::eSTATUS_TODO);
            it++;
        }
        cur_index++;
    }

    //
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
bool
CaptureBufMgr::
enqueProcessor(CapBufQueNode& rNode)
{
    if  ( ! rNode.mainImgNode ) {
        MY_LOGW("buffer is NULL");
        return  false;
    }
    //
    MY_LOGD_IF(
        ENABLE_LOG_PER_FRAME,
        "+ Que.size(%d);  Buffer[%s@0x%08X@%d@%s@(%d)%dx%d-%dBit@Timestamp(%lld)]",
        mImgBufQue.size(),
        rNode.mainImgNode->getBufName(), rNode.mainImgNode->getVirAddr(), rNode.mainImgNode->getBufSize(), rNode.mainImgNode->getImgFormat().string(),
        rNode.mainImgNode->getImgWidthStride(), rNode.mainImgNode->getImgWidth(), rNode.mainImgNode->getImgHeight(),
        rNode.mainImgNode->getBitsPerPixel(), rNode.mainImgNode->getTimestamp()
    );
    //
    Mutex::Autolock _lock(mImgBufQueMtx);
    //
    rNode.mainImgNode.setStatus(ImgBufQueNode::eSTATUS_TODO);
    mImgBufQue.push_front(rNode);
    //
    return  true;
}
/*******************************************************************************
*
********************************************************************************/
CaptureBufMgr::
CaptureBufMgr()
    :mspHwBufPvdr(new HwBuffProvider())
    , mStoredBufCnt(0)
    , mbBufAlloced(false)
    , miBufCnt(0)
    , miBufWidth(0)
    , miBufHeight(0)
{
}


/*******************************************************************************
*
********************************************************************************/
CaptureBufMgr::~CaptureBufMgr()
{
    MY_LOGD("this=%p, mspHwBufPvdr=%p, sizeof:%d",
             this,  &mspHwBufPvdr, sizeof(CaptureBufMgr));
}


/*******************************************************************************
*
********************************************************************************/
void
CaptureBufMgr::
destroyInstance()
{
    // let mspHwBufPvdr de-allocate by itself
}


/*******************************************************************************
*
********************************************************************************/
ICaptureBufMgr*
ICaptureBufMgr::
createInstance()
{
    return new CaptureBufMgr();
}



/******************************************************************************
*
*******************************************************************************/
void
CapBuffer::
createBuffer()
{
    FUNCTION_IN;
    //
    mbufSize = (mbufSize + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mbufInfo.size = mbufSize;
    //
    MY_LOGD("bufsize: %d", mbufSize);
    //
    mpIMemDrv = IMemDrv::createInstance();
    if ( ! mpIMemDrv || ! mpIMemDrv->init() ) {
        MY_LOGE("mpIMemDrv->init() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->allocVirtBuf(&mbufInfo) < 0) {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->mapPhyAddr(&mbufInfo) < 0) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
    //
    MY_LOGW_IF( mbufInfo.size & (L1_CACHE_BYTES-1), "bufSize(%d) not aligned!", mbufInfo.size);
    MY_LOGW_IF( mbufInfo.virtAddr & (L1_CACHE_BYTES-1), "bufAddr(%d) not aligned!", mbufInfo.virtAddr);
    //
    MY_LOGD("CapBuffer + 0x%x(0x%x)", mbufInfo.virtAddr, mbufInfo.size);
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void
CapBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    MY_LOGD("CapBuffer - 0x%x(0x%x)", mbufInfo.virtAddr, mbufInfo.size);
    if ( ! mbufInfo.virtAddr)
    {
        MY_LOGD("buffer doesn't exist");
        return;
    }

    if ( ! mpIMemDrv || mpIMemDrv->unmapPhyAddr(&mbufInfo) < 0) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->freeVirtBuf(&mbufInfo) < 0) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }

    if ( ! mpIMemDrv || ! mpIMemDrv->uninit() ) {
        MY_LOGE("m_pIMemDrv->uninit error");
    }
    //
    FUNCTION_OUT;

}

/******************************************************************************
*
*******************************************************************************/
void
CapBuffer::
update(uint32_t const u4Width,uint32_t const u4Height, uint32_t const u4Rotate)
{
    uint32_t rWidth = u4Width;
    uint32_t rHeight = u4Height;
    if (u4Rotate == 90 || u4Rotate == 270)
    {
        rWidth = u4Height;
        rHeight = u4Width;
    }
    uint32_t rbufSize = (FmtUtils::queryImgBufferSize(ms8format, rWidth, rHeight) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    MY_LOGD_IF((mbufSize != rbufSize),"(%dx%d,%d)->(%dx%d,%d)", mWidth, mHeight, mRotation,rWidth,rHeight,u4Rotate );
    mWidth = rWidth;
    mHeight = rHeight;
    mRotation = u4Rotate;

    if (mbufSize != rbufSize)
    {
        MY_LOGD("realloc (%d->%d)", mbufSize, rbufSize );
        destroyBuffer();
        mbufSize = rbufSize;
        createBuffer();
    }

#if 0
    bool isNeedToRotated = false;
    isNeedToRotated = (u4Rotate+ mRotation)/90 % 2 == 0 ? false : true;
    if ( isNeedToRotated )
    {
        uint32_t rbufSize = (FmtUtils::queryImgBufferSize(ms8format, rWidth, rHeight) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
        if (mbufSize != rbufSize)
        {
            MY_LOGD("update(S+R) (%dx%d,%d)->(%dx%d,%d)", mWidth, mHeight, mRotation,rWidth,rHeight,u4Rotate );
            destroyBuffer();
            mWidth = rWidth;
            mHeight = rHeight;
            mbufSize = rbufSize;
            createBuffer();
        }
        else
        {
            MY_LOGD("update(R) (%dx%d,%d)->(%dx%d,%d)", mWidth, mHeight, mRotation,rWidth,rHeight,u4Rotate );
            mWidth = rWidth;
            mHeight = rHeight;
        }
    }
    else
    {
        if (rWidth != mWidth || rHeight!= mHeight) {
            MY_LOGD("update(S) (%dx%d,%d)->(%dx%d,%d)", mWidth, mHeight, mRotation,rWidth,rHeight,u4Rotate );
            destroyBuffer();
            mWidth = u4Width;
            mHeight = u4Height;
            mbufSize = (FmtUtils::queryImgBufferSize(ms8format, u4Width, u4Height) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
            createBuffer();
        } else {
            MY_LOGD("not update (%dx%d,%d)->(%dx%d,%d)", mWidth, mHeight, mRotation,rWidth,rHeight,u4Rotate );
        }
    }
    mRotation = u4Rotate;
#endif
}
