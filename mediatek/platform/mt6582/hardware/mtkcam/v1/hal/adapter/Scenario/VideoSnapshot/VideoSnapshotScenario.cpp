#define LOG_TAG "MtkCam/VideoSnapshotScenario"
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <utils/threads.h>
#include <semaphore.h>
#include <cutils/atomic.h>
#include <sys/prctl.h>
//
using namespace android;
//
#include <cutils/properties.h>
//
#include <mtkcam/common.h>
#include <inc/CamUtils.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <mtkcam/drv/imem_drv.h>
#include <jpeg_hal.h>
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ICdpPipe.h>
#include <Shot/IShot.h>
//
#include <mtkcam/hal/aaa_hal_base.h>
using namespace NS3A;
#include <mtkcam/hal/sensor_hal.h>
#include <kd_imgsensor_define.h>
//
using namespace NSImageio;
using namespace NSIspio;
#include <mtkcam/vssimgtrans/vss_img_trans.h>
//
using namespace NSShot;
using namespace MtkCamUtils;
using namespace FmtUtils;
#include <VideoSnapshot/IVideoSnapshotScenario.h>
#include <VideoSnapshotScenario.h>
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
//-----------------------------------------------------------------------------
IVideoSnapshotScenario*
IVideoSnapshotScenario::
createInstance(void)
{
    return VideoSnapshotScenario::getInstance();
}
//-----------------------------------------------------------------------------
IVideoSnapshotScenario*
VideoSnapshotScenario::
getInstance(void)
{
    MY_LOGD("");
    static VideoSnapshotScenario singleton;
    return &singleton;
}
//-----------------------------------------------------------------------------
void
VideoSnapshotScenario::
destroyInstance(void) 
{
    mpShotCallback = NULL; 
}
//-----------------------------------------------------------------------------
VideoSnapshotScenario::
VideoSnapshotScenario()
{
    MUINT32 i;
    //MY_LOGD("");
    //
    mUsers = 0;
    mStatus = Status_Idle;
    mp3AHal = NULL;
    mpSensorHal = NULL;
    mpVssImgTrans = NULL;
    mpIMemDrv = NULL;
    mpShotCallback = NULL;
    mpParamsMgr = NULL;
    meSensorType = SENSOR_TYPE_RAW;
    //
    for(i=0; i<MemType_Amount; i++)
    {
        mIMemBufInfo[i].memID = -1;
        mIMemBufInfo[i].virtAddr = 0;
        mIMemBufInfo[i].phyAddr = 0;
        mIMemBufInfo[i].size = 0;
    }
    for(i=0; i<JpgType_Amount; i++)
    {
        mJpgInfo[i].width = 0;
        mJpgInfo[i].height = 0;
        mJpgInfo[i].bitStrSize = 0;
    }
    //
    mRotate = 0;
    mProcStep = ProcStep_Idle;
    mIsThumb = MFALSE;
    mPreAllocYuvWidth = 0;
    mPreAllocYuvHeight = 0;
    mImgiFormat = eImgFmt_BAYER10;
    mProcessCnt = 0;
}
//-----------------------------------------------------------------------------
VideoSnapshotScenario::
~VideoSnapshotScenario()
{
    //MY_LOGD("");
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
init(
    MINT32              sensorId,
    sp<IParamsManager>  pParamsMgr,
    Hal3ABase*          p3AHal,
    ImageSize*          pImageSize)
{
    MBOOL result = MTRUE;
    MUINT32 i;
    halSensorRawImageInfo_t sensorFormatInfo;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        goto EXIT;
    }
    //
    if(pParamsMgr == NULL)
    {
        MY_LOGE("pParamsMgr is NULL");
        goto EXIT;
    }
    //
    mpSensorHal = SensorHal::createInstance();
    if(mpSensorHal == NULL)
    {
        MY_LOGE("mpSensorHal is NULL");
        goto EXIT;
    }
    //
    mpIMemDrv = IMemDrv::createInstance();
    if(mpIMemDrv == NULL)
    {
        MY_LOGE("mpIMemDrv is NULL");
        goto EXIT;
    }
    //
    mSensorId = sensorId;
    mpParamsMgr = pParamsMgr;
    mp3AHal = p3AHal;
    //
    mpSensorHal->sendCommand(
        (mSensorId == 0) ? SENSOR_DEV_MAIN : SENSOR_DEV_SUB,
        SENSOR_CMD_GET_SENSOR_TYPE,
        (MINT32)&meSensorType);
    memset(&sensorFormatInfo, 0, sizeof(halSensorRawImageInfo_t));
    mpSensorHal->sendCommand(
        (mSensorId == 0) ? SENSOR_DEV_MAIN : SENSOR_DEV_SUB,
        SENSOR_CMD_GET_RAW_INFO,
        (MINT32)&sensorFormatInfo,
        1,
        0);
    //
    if(meSensorType == SENSOR_TYPE_RAW)
    {
        switch(sensorFormatInfo.u4BitDepth)
        {
            case 8:
            {
                mImgiFormat = eImgFmt_BAYER8;
                break;
            }
            case 10:
            default :
            {
                mImgiFormat = eImgFmt_BAYER10;
                break;
            }
        }            
    }
    else
    if(meSensorType == SENSOR_TYPE_YUV)
    {
        switch(sensorFormatInfo.u1Order)
        {
            case SENSOR_OUTPUT_FORMAT_UYVY: 
            case SENSOR_OUTPUT_FORMAT_CbYCrY:
            {
                mImgiFormat = eImgFmt_UYVY;
                break;
            }
            case SENSOR_OUTPUT_FORMAT_VYUY : 
            case SENSOR_OUTPUT_FORMAT_CrYCbY :
            {
                mImgiFormat = eImgFmt_VYUY;
                break;
            }
            case SENSOR_OUTPUT_FORMAT_YVYU : 
            case SENSOR_OUTPUT_FORMAT_YCrYCb : 
            {
                mImgiFormat = eImgFmt_YVYU;
                break;
            }
            case SENSOR_OUTPUT_FORMAT_YUYV : 
            case SENSOR_OUTPUT_FORMAT_YCbYCr :
            default :
            {
                mImgiFormat = eImgFmt_YUY2;
                break;
            }
        } 
    }
    else
    {
        MY_LOGE("Unknown sensor type(%d)", meSensorType);
    }
    MY_LOGD("Sensor:Type(%d),BitDepth(%d),Order(%d),Format(%d)",
            meSensorType,
            sensorFormatInfo.u4BitDepth,
            sensorFormatInfo.u1Order,
            mImgiFormat);
    //
    mpVssImgTrans = VssImgTrans::CreateInstance();
    //
    if(!mpIMemDrv->init())
    {
        MY_LOGE("mpIMemDrv->init() error");
    }
    //
    for(i=0; i<MemType_Amount; i++)
    {
        mIMemBufInfo[i].memID = -1;
        mIMemBufInfo[i].virtAddr = 0;
        mIMemBufInfo[i].phyAddr = 0;
        mIMemBufInfo[i].size = 0;
    }
    //
    if((pImageSize->width*pImageSize->height) < (YUV_PRE_ALLOC_WIDTH*YUV_PRE_ALLOC_HEIGHT))
    {
        mJpgInfo[JpgType_Main].width = YUV_PRE_ALLOC_WIDTH;
        mJpgInfo[JpgType_Main].height = YUV_PRE_ALLOC_HEIGHT;
    }
    else
    {
        mJpgInfo[JpgType_Main].width = ALIGN_SIZE(pImageSize->width,JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_Main].height = ALIGN_SIZE(pImageSize->height,JPG_IMG_ALIGN_SIZE);
    }
    mPreAllocYuvWidth = mJpgInfo[JpgType_Main].width;
    mPreAllocYuvHeight = mJpgInfo[JpgType_Main].height;
    MY_LOGD("Main:W(%d)xH(%d)",
            mPreAllocYuvWidth,
            mPreAllocYuvHeight);
    //
    if( mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH) == 0 ||
        mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT) == 0)
    {
        mIsThumb = MFALSE;
    }
    else
    {
        mIsThumb = MTRUE;
    }
    //
    if(mIsThumb)
    {
        mJpgInfo[JpgType_Thumb].width = ALIGN_SIZE(mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH),JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_Thumb].height = ALIGN_SIZE(mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT),JPG_IMG_ALIGN_SIZE);
        MY_LOGD("Thumb:W(%d)xH(%d)",
                mJpgInfo[JpgType_Thumb].width,
                mJpgInfo[JpgType_Thumb].height);
        //
        mJpgInfo[JpgType_ThumbRotate].width = ALIGN_SIZE(mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT),JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_ThumbRotate].height = ALIGN_SIZE(mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH),JPG_IMG_ALIGN_SIZE);
        //
        allocMem(MemType_YuvThumb);
        allocMem(MemType_YuvThumbRotate);
        allocMem(MemType_JpgThumb);
    }
    //
    allocMem(MemType_YuvMain);
    allocMem(MemType_JpgMain);
    allocMem(MemType_Jpg);
    //
    run();
    //
    android_atomic_inc(&mUsers);
    //
    EXIT:
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
uninit(void)
{
    MBOOL result = MTRUE;
    MUINT32 i;
    status_t status;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        goto EXIT;
    }
    //
    android_atomic_dec(&mUsers);
    //
    if(mUsers > 0)
    {
        goto EXIT;
    }
    //
    freeMem(MemType_YuvMain);
    freeMem(MemType_YuvThumb);
    freeMem(MemType_YuvThumbRotate);
    freeMem(MemType_JpgMain);
    freeMem(MemType_JpgThumb);
    freeMem(MemType_Jpg);
    //
    mp3AHal = NULL;
    //
    if(mpSensorHal != NULL)
    {
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
    }
    //
    if(mpVssImgTrans != NULL)
    {
        mpVssImgTrans->DestroyInstance();
        mpVssImgTrans = NULL;
    }
    //
    if(mpIMemDrv != NULL)
    {
        mpIMemDrv->uninit();
        mpIMemDrv = NULL;
    }
    //
    mpParamsMgr = NULL;
    mStatus = Status_Idle;
    meSensorType = SENSOR_TYPE_RAW;
    //
    for(i=0; i<MemType_Amount; i++)
    {
        mIMemBufInfo[i].memID = -1;
        mIMemBufInfo[i].virtAddr = 0;
        mIMemBufInfo[i].phyAddr = 0;
        mIMemBufInfo[i].size = 0;
    }
    for(i=0; i<JpgType_Amount; i++)
    {
        mJpgInfo[i].width = 0;
        mJpgInfo[i].height = 0;
        mJpgInfo[i].bitStrSize = 0;
    }
    //
    mRotate = 0;
    mProcStep = ProcStep_Idle;
    //
    requestExit();
    //
    EXIT:
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
setCallback(sp<IShotCallback> pShotCallback)
{
    MY_LOGD("+ rpShotCallback(%p), rpShotCallback->getStrongCount(%d)", pShotCallback.get(), pShotCallback->getStrongCount());
    mpShotCallback = pShotCallback;
    return  (mpShotCallback != NULL);
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
allocMem(MemTypeEnum type)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    MUINT32 bufSize = 0, bufSizeRot = 0;
    //
    MY_LOGD("type(%d)",type);
    //
    if(mpIMemDrv == NULL)
    {
        MY_LOGE("mpIMemDrv is NULL");
        goto EXIT;
    }
    //
    if(mIMemBufInfo[type].size != 0)
    {
        MY_LOGW("Buffer type(%d) has been allocated",type);
        goto EXIT;
    }
    //
    switch(type)
    {
        case MemType_Jpg:
        {
            mIMemBufInfo[type].size = JPG_EXIF_SIZE;
            mIMemBufInfo[type].size += FmtUtils::queryImgBufferSize(
                                                    CameraParameters::PIXEL_FORMAT_YUV420P,
                                                    mJpgInfo[JpgType_Thumb].width,
                                                    mJpgInfo[JpgType_Thumb].height)/JPG_COMPRESSION_RATIO;
            mIMemBufInfo[type].size += FmtUtils::queryImgBufferSize(
                                                    CameraParameters::PIXEL_FORMAT_YUV420P,
                                                    mJpgInfo[JpgType_Main].width,
                                                    mJpgInfo[JpgType_Main].height)/JPG_COMPRESSION_RATIO;
            break;
        }
        case MemType_JpgThumb:
        {
            mIMemBufInfo[type].size = FmtUtils::queryImgBufferSize(
                                                    CameraParameters::PIXEL_FORMAT_YUV420P,
                                                    mJpgInfo[JpgType_ThumbRotate].width,
                                                    mJpgInfo[JpgType_ThumbRotate].height)/JPG_COMPRESSION_RATIO;
            break;
        }
        case MemType_JpgMain:
        {
            mIMemBufInfo[type].size = FmtUtils::queryImgBufferSize(
                                                    CameraParameters::PIXEL_FORMAT_YUV420P,
                                                    mJpgInfo[JpgType_Main].width,
                                                    mJpgInfo[JpgType_Main].height)/JPG_COMPRESSION_RATIO;
            break;
        }
        case MemType_YuvThumb:
        {
            mIMemBufInfo[type].size =   ALIGN_SIZE(mJpgInfo[JpgType_Thumb].width,YUV_IMG_STRIDE_Y)*mJpgInfo[JpgType_Thumb].height+
                                        ALIGN_SIZE(mJpgInfo[JpgType_Thumb].width/2,YUV_IMG_STRIDE_U)*mJpgInfo[JpgType_Thumb].height/2+
                                        ALIGN_SIZE(mJpgInfo[JpgType_Thumb].width/2,YUV_IMG_STRIDE_V)*mJpgInfo[JpgType_Thumb].height/2;
            break;
        }
        case MemType_YuvThumbRotate:
        {
            mIMemBufInfo[type].size =   ALIGN_SIZE(mJpgInfo[JpgType_ThumbRotate].width,YUV_IMG_STRIDE_Y)*mJpgInfo[JpgType_ThumbRotate].height+
                                        ALIGN_SIZE(mJpgInfo[JpgType_ThumbRotate].width/2,YUV_IMG_STRIDE_U)*mJpgInfo[JpgType_ThumbRotate].height/2+
                                        ALIGN_SIZE(mJpgInfo[JpgType_ThumbRotate].width/2,YUV_IMG_STRIDE_V)*mJpgInfo[JpgType_ThumbRotate].height/2;
            break;
        }
        case MemType_YuvMain:
        {
            bufSize =       ALIGN_SIZE(mJpgInfo[JpgType_Main].width,YUV_IMG_STRIDE_Y)*mJpgInfo[JpgType_Main].height+
                            ALIGN_SIZE(mJpgInfo[JpgType_Main].width/2,YUV_IMG_STRIDE_U)*mJpgInfo[JpgType_Main].height/2+
                            ALIGN_SIZE(mJpgInfo[JpgType_Main].width/2,YUV_IMG_STRIDE_V)*mJpgInfo[JpgType_Main].height/2;
            bufSizeRot =    ALIGN_SIZE(mJpgInfo[JpgType_Main].height,YUV_IMG_STRIDE_Y)*mJpgInfo[JpgType_Main].width+
                            ALIGN_SIZE(mJpgInfo[JpgType_Main].height/2,YUV_IMG_STRIDE_U)*mJpgInfo[JpgType_Main].width/2+
                            ALIGN_SIZE(mJpgInfo[JpgType_Main].height/2,YUV_IMG_STRIDE_V)*mJpgInfo[JpgType_Main].width/2;
            MY_LOGD("bufSize(%d,%d)",
                    bufSize,
                    bufSizeRot);
            if( bufSize >= bufSizeRot)
            {
                mIMemBufInfo[type].size = bufSize;
            }
            else
            {
                mIMemBufInfo[type].size = bufSizeRot;
            }

            break;
        }
        default:
        {
            MY_LOGE("Un-supported type(%d)",type);
            break;
        }
    }
    //
    if(mpIMemDrv->allocVirtBuf(&mIMemBufInfo[type]) < 0)
    {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
    }
    if(mpIMemDrv->mapPhyAddr(&mIMemBufInfo[type]) < 0)
    {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
    MY_LOGD("T(%d),S(%d),I(%d),V(0x%08X~0x%08X),P(0x%08X~0x%08X)",
            type,
            mIMemBufInfo[type].size,
            mIMemBufInfo[type].memID,
            mIMemBufInfo[type].virtAddr,
            mIMemBufInfo[type].virtAddr+mIMemBufInfo[type].size,
            mIMemBufInfo[type].phyAddr,
            mIMemBufInfo[type].phyAddr+mIMemBufInfo[type].size);
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
freeMem(MemTypeEnum type)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    MY_LOGD("type(%d)",type);
    //
    if(mpIMemDrv == NULL)
    {
        MY_LOGE("mpIMemDrv is NULL");
        goto EXIT;
    }
    //
    if(mIMemBufInfo[type].size == 0)
    {
        MY_LOGW("Free un-allocated buffer type(%d)",type);
        goto EXIT;
    }
    //
    if(mpIMemDrv->unmapPhyAddr(&mIMemBufInfo[type]) < 0)
    {
        MY_LOGE("mpIMemDrv->unmapPhyAddr() error");
    }
    if(mpIMemDrv->freeVirtBuf(&mIMemBufInfo[type]) < 0)
    {
        MY_LOGE("mpIMemDrv->freeVirtBuf() error");
    }
    //
    mIMemBufInfo[type].size = 0;
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}

//-----------------------------------------------------------------------------
VideoSnapshotScenario::Status
VideoSnapshotScenario::
getStatus(void)
{
    return mStatus;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
enable(MBOOL en)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mStatus != Status_Idle)
    {
        MY_LOGE("Status(%d) is not idle",mStatus);
        result = MFALSE;
        goto EXIT;
    }
    //
    MY_LOGD("en(%d)",en);
    //
    if(en)
    {
        if( mJpgInfo[JpgType_Thumb].width != mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH) ||
            mJpgInfo[JpgType_Thumb].height != mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT))
        {
            MY_LOGW("Thumb size change W(%d)xH(%d) -> W(%d)xH(%d)",
                    mJpgInfo[JpgType_Thumb].width,
                    mJpgInfo[JpgType_Thumb].height,
                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH),
                    mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT));
            //
            freeMem(MemType_YuvThumb);
            freeMem(MemType_YuvThumbRotate);
            freeMem(MemType_JpgThumb);
            //
            if( mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH) == 0 ||
                mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT) == 0)
            {
                mIsThumb = MFALSE;
            }
            else
            {
                mIsThumb = MTRUE;
            }
            //
            if(mIsThumb)
            {
                mJpgInfo[JpgType_Thumb].width = mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
                mJpgInfo[JpgType_Thumb].height = mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
                //
                mJpgInfo[JpgType_ThumbRotate].width = mJpgInfo[JpgType_Thumb].width;
                mJpgInfo[JpgType_ThumbRotate].height = mJpgInfo[JpgType_Thumb].height;
                //
                allocMem(MemType_YuvThumb);
                allocMem(MemType_YuvThumbRotate);
                allocMem(MemType_JpgThumb);
            }
        }
        //
        mStatus = Status_WaitImage;
    }
    else
    {
        mStatus = Status_Idle;
    }
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
setImage(ImageInfo &img)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE, reAllocBuf = MFALSE;
    MINT32 picWidth,picHeight,thumbWidth,thumbHeight,recWidth,recHeight;
    NSCamHW::Rect rHWSrc, rHWDst, rHWCrop;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        MY_LOGE("Please init first");
        result = MFALSE;
        goto EXIT;
    }
    //
    if(mStatus != Status_WaitImage)
    {
        MY_LOGE("Status(%d) is not waiting image",mStatus);
        result = MFALSE;
        goto EXIT;
    }
    //
    MY_LOGD("Size:W(%d),H(%d),S(%d)",
            img.size.width,
            img.size.height,
            img.size.stride);
    MY_LOGD("Mem:I(%d),V(0x%08X),P(0x%08X)",
            img.mem.id,
            img.mem.vir,
            img.mem.phy);
    MY_LOGD("Crop:X(%d),Y(%d),W(%d),H(%d)",
            img.crop.x,
            img.crop.y,
            img.crop.w,
            img.crop.h);
    //
    memcpy(&mImage,&img,sizeof(ImageInfo));
    //
    mIMemBufInfo[MemType_Pass1Out].memID = mImage.mem.id;
    mIMemBufInfo[MemType_Pass1Out].virtAddr = mImage.mem.vir;
    mIMemBufInfo[MemType_Pass1Out].phyAddr = 0;
    mIMemBufInfo[MemType_Pass1Out].size = mImage.mem.size;
    //
    mRotate = mpParamsMgr->getInt(CameraParameters::KEY_ROTATION);
    //
    rHWSrc.x = 0;
    rHWSrc.y = 0;
    rHWSrc.w = img.size.width;
    rHWSrc.h = img.size.height;
    //
    rHWDst.x = 0;
    rHWDst.y = 0;
    rHWDst.w = img.crop.w;
    rHWDst.h = img.crop.h;
    //
    rHWCrop = MtkCamUtils::calCrop(rHWSrc, rHWDst, 100);
    //
    mpParamsMgr->getVideoSize(&recWidth, &recHeight);
    //
    if( rHWCrop.w < recWidth ||
        rHWCrop.h < recHeight)
    {
        MY_LOGW("Crop:W(%d)xH(%d) < Rec:W(%d)xH(%d)",
                rHWCrop.w,
                rHWCrop.h,
                recWidth,
                recHeight);
        picWidth = recWidth;
        picHeight = recHeight;
    }
    else
    {
        picWidth = rHWCrop.w;
        picHeight = rHWCrop.h;
    }
    //
    if((picWidth*picHeight) > (mPreAllocYuvWidth*mPreAllocYuvHeight))
    {
        MY_LOGW("Re-alloc:W(%d)xH(%d)",
                picWidth,
                picHeight);
        reAllocBuf = MTRUE;
        mPreAllocYuvWidth = picWidth;
        mPreAllocYuvHeight = picHeight;
    }
    //
    if( mRotate == 90 || 
        mRotate == 270)
    {
        mJpgInfo[JpgType_Main].width = ALIGN_SIZE(picHeight,JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_Main].height = ALIGN_SIZE(picWidth,JPG_IMG_ALIGN_SIZE);
    }
    else
    {
        mJpgInfo[JpgType_Main].width = ALIGN_SIZE(picWidth,JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_Main].height = ALIGN_SIZE(picHeight,JPG_IMG_ALIGN_SIZE);
    }
    //
    if(reAllocBuf)
    {
        freeMem(MemType_YuvMain);
        freeMem(MemType_JpgMain);
        freeMem(MemType_Jpg);
        allocMem(MemType_YuvMain);
        allocMem(MemType_JpgMain);
        allocMem(MemType_Jpg);
    }
    //
    if(mIsThumb)
    {
        thumbWidth = mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
        thumbHeight = mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
        //
        mJpgInfo[JpgType_Thumb].width = ALIGN_SIZE(thumbWidth,JPG_IMG_ALIGN_SIZE);
        mJpgInfo[JpgType_Thumb].height = ALIGN_SIZE(thumbHeight,JPG_IMG_ALIGN_SIZE);
        //
        if( mRotate == 90 || 
            mRotate == 270)
        {
            mJpgInfo[JpgType_ThumbRotate].width = ALIGN_SIZE(thumbHeight,JPG_IMG_ALIGN_SIZE);
            mJpgInfo[JpgType_ThumbRotate].height = ALIGN_SIZE(thumbWidth,JPG_IMG_ALIGN_SIZE);
        }
        else
        {
            mJpgInfo[JpgType_ThumbRotate].width = mJpgInfo[JpgType_Thumb].width;
            mJpgInfo[JpgType_ThumbRotate].height = mJpgInfo[JpgType_Thumb].height;
        }
    }
    //
    MY_LOGD("R(%d).Main:W(%d),H(%d).Thumb:W(%d),H(%d).ThumbRotate:W(%d),H(%d)",
            mRotate,
            mJpgInfo[JpgType_Main].width,
            mJpgInfo[JpgType_Main].height,
            mJpgInfo[JpgType_Thumb].width,
            mJpgInfo[JpgType_Thumb].height,
            mJpgInfo[JpgType_ThumbRotate].width,
            mJpgInfo[JpgType_ThumbRotate].height);
    //
    mProcStep = ProcStep_Main_Thumb_Init;
    //
    mStatus = Status_Process;
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
transMainThumb(MBOOL en)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    MY_LOGD("en(%d)",en);
    //
    if(en)
    {
        CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "transMainThumb start");
        //
        VssImgTrans::CONFIG_STRUCT Config;
        //
        mpIMemDrv->mapPhyAddr(&mIMemBufInfo[MemType_Pass1Out]);
        mImage.mem.phy = mIMemBufInfo[MemType_Pass1Out].phyAddr;
        //imgi
        Config.ImageIn.Size.Width = mImage.size.width;
        Config.ImageIn.Size.Height = mImage.size.height;
        Config.ImageIn.Size.Stride[0] = mImage.size.stride;
        Config.ImageIn.Size.Stride[1] = 0;
        Config.ImageIn.Size.Stride[2] = 0;
        Config.ImageIn.Mem.Id = mImage.mem.id;
        Config.ImageIn.Mem.Vir = mImage.mem.vir;
        Config.ImageIn.Mem.Phy = mImage.mem.phy;
        Config.ImageIn.Mem.Size = mImage.mem.size;
        Config.ImageIn.Format = mImgiFormat;
        Config.ImageIn.Crop.X = mImage.crop.x;
        Config.ImageIn.Crop.Y = mImage.crop.y;
        Config.ImageIn.Crop.W = mImage.crop.w;
        Config.ImageIn.Crop.H = mImage.crop.h;
        //dispo
        if(mIsThumb)
        {
            Config.DispoOut.Enable = MTRUE;
            //allocMem(MemType_YuvThumb);
            //
            if( mRotate == 90 || 
                mRotate == 270)
            {
                Config.DispoOut.Size.Width = mJpgInfo[JpgType_ThumbRotate].height;
                Config.DispoOut.Size.Height = mJpgInfo[JpgType_ThumbRotate].width;
            }
            else
            {
                
                Config.DispoOut.Size.Width = mJpgInfo[JpgType_Thumb].width;
                Config.DispoOut.Size.Height = mJpgInfo[JpgType_Thumb].height;
            }
            Config.DispoOut.Size.Stride[0] = Config.DispoOut.Size.Width;
            Config.DispoOut.Size.Stride[1] = ALIGN_SIZE(Config.DispoOut.Size.Width/2,YUV_IMG_STRIDE_U);
            Config.DispoOut.Size.Stride[2] = ALIGN_SIZE(Config.DispoOut.Size.Width/2,YUV_IMG_STRIDE_V);
            Config.DispoOut.Format = eImgFmt_YV12;
            Config.DispoOut.Mem.Id = mIMemBufInfo[MemType_YuvThumb].memID;
            Config.DispoOut.Mem.Vir = mIMemBufInfo[MemType_YuvThumb].virtAddr;
            Config.DispoOut.Mem.Phy = mIMemBufInfo[MemType_YuvThumb].phyAddr;
            Config.DispoOut.Mem.Size = mIMemBufInfo[MemType_YuvThumb].size;
        }
        else
        {
            Config.DispoOut.Enable = MFALSE;
        }
        //vido
        //allocMem(MemType_YuvMain);

        Config.VidoOut.Enable = MTRUE;
        Config.VidoOut.Rotate = mRotate;
        Config.VidoOut.Size.Width = mJpgInfo[JpgType_Main].width;
        Config.VidoOut.Size.Height = mJpgInfo[JpgType_Main].height;
        Config.VidoOut.Size.Stride[0] = Config.VidoOut.Size.Width;
        Config.VidoOut.Size.Stride[1] = ALIGN_SIZE(Config.VidoOut.Size.Width/2,YUV_IMG_STRIDE_U);
        Config.VidoOut.Size.Stride[2] = ALIGN_SIZE(Config.VidoOut.Size.Width/2,YUV_IMG_STRIDE_V);
        Config.VidoOut.Mem.Id = mIMemBufInfo[MemType_YuvMain].memID;
        Config.VidoOut.Mem.Vir = mIMemBufInfo[MemType_YuvMain].virtAddr;
        Config.VidoOut.Mem.Phy = mIMemBufInfo[MemType_YuvMain].phyAddr;
        Config.VidoOut.Mem.Size = mIMemBufInfo[MemType_YuvMain].size;
        Config.VidoOut.Format = eImgFmt_YV12;
        Config.VidoOut.Flip = MFALSE;
        //
        mpVssImgTrans->Init(Config);
        mpVssImgTrans->Start();
        mProcStep = ProcStep_Main_Thumb;
        //
        saveData(
            mImage.mem.vir,
            mImage.mem.size,
            "vss.raw");
    }
    else
    {
        mpVssImgTrans->Uninit();
        mpIMemDrv->unmapPhyAddr(&mIMemBufInfo[MemType_Pass1Out]);
        //
        CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "transMainThumb end");
        //
        saveData(
            mIMemBufInfo[MemType_YuvMain].virtAddr,
            mIMemBufInfo[MemType_YuvMain].size,
            "vss_main.yuv");
        //
        if(mIsThumb)
        {
            saveData(
                mIMemBufInfo[MemType_YuvThumb].virtAddr,
                mIMemBufInfo[MemType_YuvThumb].size,
                "vss_thumb.yuv");
        }
    }
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
rotateThumb(MBOOL en)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    if(!mIsThumb)
    {
        return MFALSE;
    }
    //
    MY_LOGD("en(%d)",en);
    //
    if(en)
    {
        CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "rotateThumb start");
        //
        VssImgTrans::CONFIG_STRUCT Config;
        //imgi
        if( mRotate == 90 || 
            mRotate == 270)
        {
            Config.ImageIn.Size.Width = mJpgInfo[JpgType_ThumbRotate].height;
            Config.ImageIn.Size.Height = mJpgInfo[JpgType_ThumbRotate].width;
        }
        else
        {
            Config.ImageIn.Size.Width = mJpgInfo[JpgType_Thumb].width;
            Config.ImageIn.Size.Height = mJpgInfo[JpgType_Thumb].height;
        }

        //
        Config.ImageIn.Size.Stride[0] = Config.ImageIn.Size.Width;
        Config.ImageIn.Size.Stride[1] = ALIGN_SIZE(Config.ImageIn.Size.Width/2,YUV_IMG_STRIDE_U);
        Config.ImageIn.Size.Stride[2] = ALIGN_SIZE(Config.ImageIn.Size.Width/2,YUV_IMG_STRIDE_V);
        Config.ImageIn.Mem.Id = mIMemBufInfo[MemType_YuvThumb].memID;
        Config.ImageIn.Mem.Vir = mIMemBufInfo[MemType_YuvThumb].virtAddr;
        Config.ImageIn.Mem.Phy = mIMemBufInfo[MemType_YuvThumb].phyAddr;
        //
        Config.ImageIn.Format = eImgFmt_YV12;
        Config.ImageIn.Crop.X = 0;
        Config.ImageIn.Crop.Y = 0;
        Config.ImageIn.Crop.W = Config.ImageIn.Size.Width;
        Config.ImageIn.Crop.H = Config.ImageIn.Size.Height;
        //dispo
        Config.DispoOut.Enable = MFALSE;    
        //vido
        //allocMem(MemType_YuvThumbRotate);
        Config.VidoOut.Enable = MTRUE;
        //
        if( mRotate == 90 || 
            mRotate == 270)
        {
            Config.VidoOut.Size.Width = Config.ImageIn.Size.Height;
            Config.VidoOut.Size.Height = Config.ImageIn.Size.Width;
        }
        else
        {
            Config.VidoOut.Size.Width = Config.ImageIn.Size.Width;
            Config.VidoOut.Size.Height = Config.ImageIn.Size.Height;
        }
        //
        Config.VidoOut.Size.Stride[0] = Config.VidoOut.Size.Width;
        Config.VidoOut.Size.Stride[1] = ALIGN_SIZE(Config.VidoOut.Size.Width/2,YUV_IMG_STRIDE_U);
        Config.VidoOut.Size.Stride[2] = ALIGN_SIZE(Config.VidoOut.Size.Width/2,YUV_IMG_STRIDE_V);
        Config.VidoOut.Mem.Id = mIMemBufInfo[MemType_YuvThumbRotate].memID;
        Config.VidoOut.Mem.Vir = mIMemBufInfo[MemType_YuvThumbRotate].virtAddr;
        Config.VidoOut.Mem.Phy = mIMemBufInfo[MemType_YuvThumbRotate].phyAddr;
        Config.VidoOut.Format = eImgFmt_YV12;
        Config.VidoOut.Rotate = mRotate;
        Config.VidoOut.Flip = MFALSE;
        //
        mpVssImgTrans->Init(Config);
        mpVssImgTrans->Start();
        mProcStep = ProcStep_ThumbRotate;
    }
    else
    {
        mpVssImgTrans->Uninit();
        //freeMem(MemType_YuvThumb);
        //
        CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "rotateThumb end");
        //
        saveData(
            mIMemBufInfo[MemType_YuvThumbRotate].virtAddr,
            mIMemBufInfo[MemType_YuvThumbRotate].size,
            "vss_thumb_rotate.yuv");
    }
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
encodeJpg(
    JpgTypeEnum     jpgType,
    MemTypeEnum     srcMemType,
    MemTypeEnum     dstMemType,
    MUINT32         quality,
    MBOOL           enableSOI)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    MUINT32 i, timeout = 0, yuvAddr[3],yuvSize[3];
    char fileName[100];
    //
    yuvSize[0] = ALIGN_SIZE(mJpgInfo[jpgType].width,YUV_IMG_STRIDE_Y)*mJpgInfo[jpgType].height;
    yuvSize[1] = ALIGN_SIZE(mJpgInfo[jpgType].width/2,YUV_IMG_STRIDE_U)*mJpgInfo[jpgType].height/2;
    yuvSize[2] = ALIGN_SIZE(mJpgInfo[jpgType].width/2,YUV_IMG_STRIDE_V)*mJpgInfo[jpgType].height/2;
    //
    yuvAddr[0] = mIMemBufInfo[srcMemType].virtAddr;
    yuvAddr[1] = yuvAddr[0]+yuvSize[0];
    yuvAddr[2] = yuvAddr[1]+yuvSize[1];
    //
    MY_LOGD("jpgType(%d),srcMemType(%d),dstMemType(%d),quality(%d)",jpgType,srcMemType,dstMemType,quality);
    MY_LOGD("VA:Src(0x%08X~0x%08X),Dst(0x%08X~0x%08X)",
            mIMemBufInfo[srcMemType].virtAddr,
            mIMemBufInfo[srcMemType].virtAddr+mIMemBufInfo[srcMemType].size,
            mIMemBufInfo[dstMemType].virtAddr,
            mIMemBufInfo[dstMemType].virtAddr+mIMemBufInfo[dstMemType].size);
    MY_LOGD("PA:Src(0x%08X~0x%08X),Dst(0x%08X~0x%08X)",
            mIMemBufInfo[srcMemType].phyAddr,
            mIMemBufInfo[srcMemType].phyAddr+mIMemBufInfo[srcMemType].size,
            mIMemBufInfo[dstMemType].phyAddr,
            mIMemBufInfo[dstMemType].phyAddr+mIMemBufInfo[dstMemType].size);
    for(i=0; i<3; i++)
    {
        MY_LOGD("YUV[%d]:A(0x%08X)S(%d)",
                i,
                yuvAddr[i],
                yuvSize[i]);
        //
        if(jpgType == JpgType_Main)
        {
            sprintf(fileName, "vss_main_%d.yuv", i);
        }
        else
        if(jpgType == JpgType_Thumb)
        {
            sprintf(fileName, "vss_thumb_%d.yuv", i);
        }
        else
        if(jpgType == JpgType_ThumbRotate)
        {
            sprintf(fileName, "vss_thumb_rotate_%d.yuv", i);
        }
        else
        {
            sprintf(fileName, "vss_%d_%d.yuv", jpgType, i);
        } 
        //
        saveData(
            yuvAddr[i],
            yuvSize[i],
            fileName);
    }
    //
    JpgEncHal* pJpgEncoder = new JpgEncHal();
    // (1). Lock 
    while(1)
    {
        if(pJpgEncoder->LevelLock(JpgEncHal::JPEG_ENC_LOCK_SW_ONLY))
        {
            MY_LOGD("Lock OK");
            break;
        }
        //
        timeout++;
        if(timeout > JPG_LOCK_TIMEOUT_CNT)
        {
            MY_LOGE("Lock fail");
            goto EXIT;
        }
        usleep(JPG_LOCK_TIMEOUT_SLEEP);
    }
    // (2). size, format, addr
    pJpgEncoder->setEncSize(
                    mJpgInfo[jpgType].width,
                    mJpgInfo[jpgType].height, 
                    JpgEncHal::kENC_YV12_Format);
    pJpgEncoder->setSrcAddr(
                    (void*)yuvAddr[0],
                    (void*)yuvAddr[1],
                    (void*)yuvAddr[2]);
    pJpgEncoder->setSrcBufSize(
                    mJpgInfo[jpgType].width,
                    yuvSize[0],
                    yuvSize[1],
                    yuvSize[2]);
    // (3). set quality
    pJpgEncoder->setQuality(quality);
    // (4). dst addr, size
    pJpgEncoder->setDstAddr((void*)(mIMemBufInfo[dstMemType].virtAddr));
    pJpgEncoder->setDstSize(mIMemBufInfo[dstMemType].size);
    // (6). set SOI
    pJpgEncoder->enableSOI(enableSOI);
    // (7). ION mode
    if(mIMemBufInfo[dstMemType].memID >= 0)
    {
        //pJpgEncoder->setIonMode(MTRUE);
        pJpgEncoder->setIonMode(MFALSE);
    }
    else
    {
        pJpgEncoder->setIonMode(MFALSE);
    }
    pJpgEncoder->setSrcFD(
                    mIMemBufInfo[srcMemType].memID,
                    mIMemBufInfo[srcMemType].memID);
    pJpgEncoder->setDstFD(mIMemBufInfo[dstMemType].memID);
    // (8).  Start 
    if(pJpgEncoder->start(&mJpgInfo[jpgType].bitStrSize))
    {
        MY_LOGD("Encode OK,size(%d)", mJpgInfo[jpgType].bitStrSize); 
    }
    else 
    {
        mJpgInfo[jpgType].bitStrSize = 0;
        MY_LOGE("Encode Fail");
    }
    //
    pJpgEncoder->unlock();
    //
    EXIT:
    delete pJpgEncoder;
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
encodeJpgMain(void)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    //allocMem(MemType_JpgMain);
    encodeJpg(
        JpgType_Main,
        MemType_YuvMain,
        MemType_JpgMain,
        mpParamsMgr->getInt(CameraParameters::KEY_JPEG_QUALITY),
        MFALSE);
    //freeMem(MemType_YuvMain);
    //
    saveData(
        mIMemBufInfo[MemType_JpgMain].virtAddr,
        mJpgInfo[JpgType_Main].bitStrSize,
        "vss_main.jpg");
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
encodeJpgThumb(void)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    JpgTypeEnum thumbJpgType;
    MemTypeEnum memType;
    //
    if(!mIsThumb)
    {
        return MFALSE;
    }
    //
    //allocMem(MemType_JpgThumb);
    //
    if( mRotate == 90 || 
        mRotate == 270)
    {
        thumbJpgType = JpgType_ThumbRotate;
        memType = MemType_YuvThumbRotate;
    }
    else
    {
        thumbJpgType = JpgType_Thumb;
        memType = MemType_YuvThumb;
    }
    //
    encodeJpg(
        thumbJpgType,
        memType,
        MemType_JpgThumb,
        mpParamsMgr->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY),
        MTRUE);
    //freeMem(memType);
    //
    saveData(
        mIMemBufInfo[MemType_JpgThumb].virtAddr,
        mJpgInfo[thumbJpgType].bitStrSize,
        "vss_thumb.jpg");
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
integrateJpg(void)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    JpgTypeEnum thumbJpgType;
    MUINT32 exifHeaderSize = 0, debugExifHeaderSize = 0;
    CamExif jpgExif;
    CamExifParam jpgExifParam;
    CamDbgParam jpgDbgParam;
    camera_info cameraInfo = MtkCamUtils::DevMetaInfo::queryCameraInfo(mSensorId);
    Hal3ABase* p3AHal = Hal3ABase::createInstance(MtkCamUtils::DevMetaInfo::queryHalSensorDev(mSensorId));
    //
    if( mRotate == 90 || 
        mRotate == 270)
    {
        thumbJpgType = JpgType_ThumbRotate;
    }
    else
    {
        thumbJpgType = JpgType_Thumb;
    }
    //
    //allocMem(MemType_Jpg);
    mJpgInfo[JpgType_Img].bitStrSize = 0;
    //
    if( !mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).isEmpty() &&
        !mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).isEmpty()) 
    {
        jpgExifParam.u4GpsIsOn = 1; 
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSLatitude),           mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).string(),           mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).length()); 
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSLongitude),          mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).string(),          mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).length()); 
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSTimeStamp),          mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).string(),          mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).length()); 
        ::strncpy(reinterpret_cast<char*>(jpgExifParam.uGPSProcessingMethod),   mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).string(),  mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).length()); 
        jpgExifParam.u4GPSAltitude = ::atoi(mpParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE).string()); 
    }
    else
    {
        MY_LOGD("No GPS data");
    }
    //
    jpgExifParam.u4Orientation = 0;
    jpgExifParam.u4ZoomRatio = 0;
    jpgExifParam.u4Facing = cameraInfo.facing;
    //
    jpgDbgParam.u4CamMode = eAppMode_VideoMode;
    jpgDbgParam.u4ShotMode = 0;
    //
    jpgExif.init(
        jpgExifParam,
        jpgDbgParam);
    //
    p3AHal->set3AEXIFInfo(&jpgExif);
    //
    if(mIsThumb)
    {
        jpgExif.makeExifApp1(
            mJpgInfo[JpgType_Main].width,
            mJpgInfo[JpgType_Main].height,
            mJpgInfo[thumbJpgType].bitStrSize,
            (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr),
            &exifHeaderSize);
    }
    else
    {
        jpgExif.makeExifApp1(
            mJpgInfo[JpgType_Main].width,
            mJpgInfo[JpgType_Main].height,
            0,
            (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr),
            &exifHeaderSize);
    }
    //
    mJpgInfo[JpgType_Img].bitStrSize += exifHeaderSize;
    MY_LOGD("JPG bitStrSize:Add exif(%d/%d)",
            exifHeaderSize,
            mJpgInfo[JpgType_Img].bitStrSize);
    //
    if(mIsThumb)
    {
        MY_LOGD("memcpy:DST(0x%08X + %d = 0x%08X),SRC(0x%08X),Size(%d),thumbJpgType(%d)",
                mIMemBufInfo[MemType_Jpg].virtAddr,
                mJpgInfo[JpgType_Img].bitStrSize,
                mIMemBufInfo[MemType_Jpg].virtAddr+mJpgInfo[JpgType_Img].bitStrSize,
                mIMemBufInfo[MemType_JpgThumb].virtAddr,
                mJpgInfo[thumbJpgType].bitStrSize,
                thumbJpgType);
        memcpy(
            (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr+mJpgInfo[JpgType_Img].bitStrSize),
            (MUINT8*)(mIMemBufInfo[MemType_JpgThumb].virtAddr),
            mJpgInfo[thumbJpgType].bitStrSize);
        //freeMem(MemType_JpgThumb);
        mJpgInfo[JpgType_Img].bitStrSize += mJpgInfo[thumbJpgType].bitStrSize;
        MY_LOGD("JPG bitStrSize:Add Thumbnail(%d/%d)",
                mJpgInfo[thumbJpgType].bitStrSize,
                mJpgInfo[JpgType_Img].bitStrSize);
    }
    //
    p3AHal->setDebugInfo(&jpgExif);
    mpSensorHal->setDebugInfo(&jpgExif);
    jpgExif.appendDebugExif(
        (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr+mJpgInfo[JpgType_Img].bitStrSize),
        &debugExifHeaderSize);
    mJpgInfo[JpgType_Img].bitStrSize += debugExifHeaderSize;
    MY_LOGD("JPG bitStrSize:Add debug exif(%d/%d)",
            debugExifHeaderSize,
            mJpgInfo[JpgType_Img].bitStrSize);
    //
    MY_LOGD("memcpy:DST(0x%08X + %d = 0x%08X),SRC(0x%08X),Size(%d)",
            mIMemBufInfo[MemType_Jpg].virtAddr,
            mJpgInfo[JpgType_Img].bitStrSize,
            mIMemBufInfo[MemType_Jpg].virtAddr+mJpgInfo[JpgType_Img].bitStrSize,
            mIMemBufInfo[MemType_JpgMain].virtAddr,
            mJpgInfo[JpgType_Main].bitStrSize);
    memcpy(
        (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr+mJpgInfo[JpgType_Img].bitStrSize),
        (MUINT8*)(mIMemBufInfo[MemType_JpgMain].virtAddr),
        mJpgInfo[JpgType_Main].bitStrSize);
    //freeMem(MemType_JpgMain);
    mJpgInfo[JpgType_Img].bitStrSize += mJpgInfo[JpgType_Main].bitStrSize;
    MY_LOGD("JPG bitStrSize:Add Main(%d/%d)",
            mJpgInfo[JpgType_Main].bitStrSize,
            mJpgInfo[JpgType_Img].bitStrSize);
    //
    saveData(
        mIMemBufInfo[MemType_Jpg].virtAddr,
        mJpgInfo[JpgType_Img].bitStrSize,
        "vss.jpg");
    //
    EXIT:
    p3AHal->destroyInstance();
    jpgExif.uninit();
    //
    MY_LOGD("JpgBitSize(%d)",mJpgInfo[JpgType_Img].bitStrSize);
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
callbackJpg(void)
{
    FUNCTION_IN;
    //
    MBOOL result = MTRUE;
    //
    if(mpShotCallback != NULL)
    {
        mpShotCallback->onCB_Shutter(true, 0);
        mpShotCallback->onCB_RawImage(0, 0, NULL);   
        mpShotCallback->onCB_CompressedImage(
                            0,
                            mJpgInfo[JpgType_Img].bitStrSize, 
                            (MUINT8*)(mIMemBufInfo[MemType_Jpg].virtAddr),
                            0,
                            NULL,  
                            0, 
                            MTRUE);
        //freeMem(MemType_Jpg);
    }
    else
    {
        MY_LOGE("mpShotCallback is NULL");
    }
    //
    EXIT:
    FUNCTION_OUT;
    return result;
}
//-----------------------------------------------------------------------------
MBOOL
VideoSnapshotScenario::
process(void)
{
    MBOOL result = MTRUE;
    //
    //Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        //MY_LOGE("Please init first!");
        result = MFALSE;
        goto EXIT;
    }
    //
    if(mStatus != Status_Idle)
    {
        MY_LOGD("mStatus(%d)",mStatus);
    }
    //
    if(mProcStep != ProcStep_Idle)
    {
        if(mProcStep == ProcStep_HandleJpg)
        {
            mCond.broadcast();
        }
        else
        {
            transImg();
        }
    }
    //
    EXIT:
    return result;
}
//-----------------------------------------------------------------------------
void
VideoSnapshotScenario::
transImg(void)
{
    if(mProcStep != ProcStep_Idle)
    {
        if(mProcStep == ProcStep_Main_Thumb_Init)
        {
            mp3AHal->setIspProfile(EIspProfile_VideoCapture);
            mp3AHal->setIspProfile(EIspProfile_VideoPreview);
            transMainThumb(MTRUE);
            mProcessCnt = 0;
        }
        //else
        if(mProcStep == ProcStep_ThumbRotate_Init)
        {
            rotateThumb(MTRUE);
            mProcessCnt = 0;
        }
        //else
        {
            if(mpVssImgTrans->WaitDone())
            {
                MY_LOGD("mProcStep(%d) is ready",mProcStep);
                switch(mProcStep)
                {
                    case ProcStep_Main_Thumb:
                    {
                        transMainThumb(MFALSE);
                        if(mIsThumb)
                        {
                            if( mRotate == 90 || 
                                mRotate == 270)
                            {
                                MY_LOGD("Start rotate thumbnail");
                                mProcStep = ProcStep_ThumbRotate_Init;
                            }
                            else
                            {
                                MY_LOGD("Start JPG without rotate");
                                mProcStep = ProcStep_HandleJpg;
                            }
                        }
                        else
                        {
                            MY_LOGD("Start JPG without thumbnail");
                            mProcStep = ProcStep_HandleJpg;
                        }
                        break;
                    }
                    case ProcStep_ThumbRotate:
                    {
                        MY_LOGD("Start JPG after rotate");
                        rotateThumb(MFALSE);
                        mProcStep = ProcStep_HandleJpg;
                        break;
                    }
                    default:
                    {
                        MY_LOGE("Un-supported ProcStep(%d)",mProcStep);
                        break;
                    }
                }
            }
            else
            {
                mProcessCnt++;
                MY_LOGD("ProcessCnt(%d)",mProcessCnt);
            }
        }
    }
}
//-----------------------------------------------------------------------------
MBOOL 
VideoSnapshotScenario::
saveData(
    MUINT32     addr,
    MUINT32     size,
    char*       pFileName)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.vss.dumpbuffer.enable", value, "0");
    int32_t enable = atoi(value);    
    if (enable == 0) 
    {
        return MFALSE;
    }
    //
    char* filePath = "/sdcard/camera_dump/VideoSnapshotScenario/";
    char fileName[100];
    sprintf(fileName, "%s%s", filePath, pFileName);
    //
    if(makePath(filePath,0660))
    {
        return saveBufToFile(
                    (char const *)fileName,
                    (MUINT8*)addr,
                    size);
    }
    //
    return MFALSE;
}
//-----------------------------------------------------------------------------
bool
VideoSnapshotScenario::
threadLoop()
{
    if(mpVssImgTrans == NULL)
    {
        MY_LOGE("mpVssImgTrans is NULL");
        return MFALSE;
    }
    //
    while(1)
    {
        MY_LOGD("Wait lock");
        Mutex::Autolock lock(mLock);
        mCond.wait(mLock);
        //
        MY_LOGD("mStatus(%d),mProcStep(%d)",mStatus,mProcStep);
        if(mProcStep == ProcStep_HandleJpg)
        {
            mStatus = Status_Done;
            CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "encodeJpgMain");
            encodeJpgMain();
            if(mIsThumb)
            {
                CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "encodeJpgThumb");
                encodeJpgThumb();
            }
            CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "integrateJpg");
            integrateJpg();
            CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "callbackJpg");
            callbackJpg();
            CPTLogStr(Event_Hal_Adapter_MtkDefaultPreview_vss, CPTFlagSeparator, "JPG end");
            //
            mProcStep = ProcStep_Idle;
            mStatus = Status_Idle;
        }
    }
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
void
VideoSnapshotScenario::
requestExit()
{
    FUNCTION_IN;
    Thread::requestExit();
    mCond.broadcast();
    FUNCTION_OUT;
}
//-----------------------------------------------------------------------------
status_t
VideoSnapshotScenario::
readyToRun()
{
    ::prctl(PR_SET_NAME,"VSS@Snapshot", 0, 0, 0);
    //
    mThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread 
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_VSS_THREAD;
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


