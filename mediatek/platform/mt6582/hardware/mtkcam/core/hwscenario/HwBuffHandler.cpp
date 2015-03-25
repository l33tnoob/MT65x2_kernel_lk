#define LOG_TAG "MtkCam/HwBuffhandler"
//
#include <adapter/inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h> 
#include <linux/cache.h>
//
#include <mtkcam/v1/hwscenario/HwBuffHandler.h>
//
#include <cutils/atomic.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/Log.h>
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

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

/******************************************************************************
*
*******************************************************************************/
void 
HwBuffer::
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
    FUNCTION_OUT;  
}


/******************************************************************************
*
*******************************************************************************/
void
HwBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
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
HwBuffProvider::
HwBuffProvider()
    : mbufCnt(0)
    , mIdxMtx()
    , mvHwBufQ()
    , mvDequedBuf()
{
}


/******************************************************************************
*
*******************************************************************************/
HwBuffProvider::
~HwBuffProvider()
{
    Mutex::Autolock autoLock(mIdxMtx);
    
    FUNCTION_IN;    

    mvHwBufQ.clear();
    mvDequedBuf.clear();

    FUNCTION_OUT;   
}


/******************************************************************************
*
*******************************************************************************/
void
HwBuffProvider::
removeBuf()
{
    FUNCTION_IN; 

    Mutex::Autolock autoLock(mIdxMtx);

    MY_LOGW_IF(mvHwBufQ.size() < (uint32_t)mbufCnt, 
               "the number of to-be-de-allocated buffer is not enough: %d ", mvHwBufQ.size());

    MY_LOGD("clear Hwbuff with size: %d, %d", mvHwBufQ.size(), mvDequedBuf.size());
    

    mvHwBufQ.clear();
    mvDequedBuf.clear();
    
    FUNCTION_OUT;     
}


/******************************************************************************
*
*******************************************************************************/
void
HwBuffProvider::
addBuf(sp<HwBuffer> const &_buf)
{
    FUNCTION_IN;

    Mutex::Autolock autoLock(mIdxMtx);

    mvHwBufQ.push_back(_buf);
    ::android_atomic_inc(&mbufCnt);
    
    FUNCTION_OUT; 
}        


/******************************************************************************
*
*******************************************************************************/
void
HwBuffProvider::
deque(sp<IImgBuf> &rbuf)
{
    Mutex::Autolock autoLock(mIdxMtx);

    if ( ! mvHwBufQ.empty() )
    {
        sp<IImgBuf> buf = *mvHwBufQ.begin();
        mvDequedBuf.push_back(buf);        
        mvHwBufQ.erase(mvHwBufQ.begin());
        rbuf = buf;
    }
    else
    {
        MY_LOGD("no buffer in Q");
    }
} 


/******************************************************************************
*
*******************************************************************************/
void 
HwBuffProvider::
enque(sp<IImgBuf> const &rbuf)
{
    if ( ! rbuf->getVirAddr() )
    {
        MY_LOGD("to-be-enque buffer is empty");
        return;
    }
    
    Mutex::Autolock autoLock(mIdxMtx);
    
    // find the corresponding buffer from mvDequedBuf to mvHwBufQ
    vector< sp<IImgBuf> >::iterator it;
    for (it = mvDequedBuf.begin(); it < mvDequedBuf.end(); it++ )
    {
        if ((*it)->getVirAddr() == rbuf->getVirAddr())
        {
            sp<IImgBuf> buf = (*it);
            mvHwBufQ.push_back(buf);
            mvDequedBuf.erase(it);
            return;
        }
    }
}

