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

#define LOG_TAG "MtkCam/MtkCamUtils"
//
#include <camera/MtkCameraParameters.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common/camutils/CamFormat.h>
//

/******************************************************************************
*
*******************************************************************************/
#define SUPPORT_IMAGEIO_FORMAT      (1)
#if SUPPORT_IMAGEIO_FORMAT
    #include <mtkcam/common.h>
    #include <mtkcam/common.h>
using namespace NSCam;
    #include <mtkcam/imageio/ispio_utility.h>
    using namespace NSImageio::NSIspio;
#endif


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] "fmt, __FUNCTION__, ##arg)
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
*******************************************************************************/
namespace android {
namespace MtkCamUtils {
namespace FmtUtils {


/******************************************************************************
 *   Camera Format Interface
 ******************************************************************************/
class ICamFmt
{
public:     ////            Instantiation.
    virtual                 ~ICamFmt() {}

public:     ////            Attributes.
    virtual int             getBitsPerPixel() const                                         = 0;
    virtual size_t          getPlaneCount() const                                           = 0;
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const       = 0;
    virtual size_t          getImgWidthStride(size_t imgWidth, size_t planeIndex) const     = 0;
    virtual size_t          getImgHeightStride(size_t imgHeight, size_t planeIndex) const   = 0;
};


/******************************************************************************
 *   Camera Format Base
 ******************************************************************************/
class CamFmtBase : public ICamFmt
{
protected:  ////            Data Members.
    String8                 ms8About;           //  "About" this class
    String8                 ms8Format;          //  CameraParameters::PIXEL_FORMAT_xxx
    int                     miBitsPerPixel;
    size_t                  mPlaneCount;
#if SUPPORT_IMAGEIO_FORMAT
    EImageFormat            meImageioFormat;    //  EImageFormat in imageio.
#endif

public:     ////            Instantiation.
                            CamFmtBase(
                                char const* szAbout, 
                                char const* szFormat, 
                                int const iBitsPerPixel, 
                                int const PlaneCount
                                )
                                : ms8About(String8(szAbout))
                                , ms8Format(String8(szFormat))
                                , miBitsPerPixel(iBitsPerPixel)
                                , mPlaneCount(PlaneCount)
                            #if SUPPORT_IMAGEIO_FORMAT
                                , meImageioFormat(eImgFmt_UNKNOWN)
                            #endif
                            {
                            }

public:     ////            Attributes.

#if SUPPORT_IMAGEIO_FORMAT
    virtual uint_t          getImageioFormat() const    { return meImageioFormat; }
#else
    virtual uint_t          getImageioFormat() const    { return 0; }
#endif

    virtual int             getBitsPerPixel() const     { return miBitsPerPixel; }
    virtual size_t          getPlaneCount() const       { return mPlaneCount; }

    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const   = 0;

    virtual size_t          getImgWidthStride(size_t imgWidth, size_t planeIndex) const
                            {
                                return  planeIndex < getPlaneCount()
                                    ?   getWidthStride(imgWidth, planeIndex)
                                    :   0
                                        ;
                            }

    virtual size_t          getImgHeightStride(size_t imgHeight, size_t planeIndex) const
                            {
                                return  planeIndex < getPlaneCount()
                                    ?   getHeightStride(imgHeight, planeIndex)
                                    :   0
                                        ;
                            }

protected:  ////            Implementations.
    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const    = 0;
    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const  = 0;

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV420P : public CamFmtBase
{
    typedef CamFmt_YUV420P  classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV420P()
                                : CamFmtBase(
                                    "(Android) YV12 - 420p 3 planar", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV420P, 
                                    12, 
                                    3
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_YV12;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const
                            {
                                size_t const y_size = getWidthStride(imgWidth, 0) * getHeightStride(imgHeight, 0);
                                size_t const v_size = getWidthStride(imgWidth, 1) * getHeightStride(imgHeight, 1);
                                size_t const u_size = getWidthStride(imgWidth, 2) * getHeightStride(imgHeight, 2);
                                //
                                return  y_size + v_size + u_size;
                            }

    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const
                            {
                                //  Y:V:U/16:16:16
                                return  (0 == planeIndex)
                                    ?   (~15) & (15 + (imgWidth))   //  Y: 16-alignment
                                    :   (~15) & (15 + (imgWidth>>1))//  VU:16-alignment
                                        ;
                            }

    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const
                            {
                                return  (0 == planeIndex)
                                    ?   (imgHeight)         //  Y
                                    :   (imgHeight) >> 1    //  VU
                                        ;
                            }

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YV12_GPU : public CamFmtBase
{
    typedef CamFmt_YV12_GPU classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YV12_GPU()
                                : CamFmtBase(
                                    "(GPU) YV12 - 3 planar", 
                                    MtkCameraParameters::PIXEL_FORMAT_YV12_GPU, 
                                    12, 
                                    3
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_YV12;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const
                            {
                                size_t const y_size = getWidthStride(imgWidth, 0) * getHeightStride(imgHeight, 0);
                                size_t const v_size = getWidthStride(imgWidth, 1) * getHeightStride(imgHeight, 1);
                                size_t const u_size = getWidthStride(imgWidth, 2) * getHeightStride(imgHeight, 2);
                                //
                                return  y_size + v_size + u_size;
                            }

    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const
                            {
                                //  Y:V:U/16:16:16
                                return  (0 == planeIndex)
                                    ?   (~15) & (15 + (imgWidth))   //  Y: 16-alignment
                                    :   (~15) & (15 + (imgWidth>>1))//  VU:16-alignment
                                        ;
                            }

    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const
                            {
                                return  (0 == planeIndex)
                                    ?   (imgHeight)         //  Y
                                    :   (imgHeight) >> 1    //  VU
                                        ;
                            }

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV420I : public CamFmtBase
{
    typedef CamFmt_YUV420I     classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV420I()
                                : CamFmtBase(
                                    "I420 - 3 planar", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV420I, 
                                    12, 
                                    3
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_I420;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const
                            {
                                size_t const y_size = getWidthStride(imgWidth, 0) * getHeightStride(imgHeight, 0);
                                size_t const u_size = getWidthStride(imgWidth, 1) * getHeightStride(imgHeight, 1);
                                size_t const v_size = getWidthStride(imgWidth, 2) * getHeightStride(imgHeight, 2);
                                //
                                return  y_size + u_size + v_size;
                            }

    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const
                            {
                                //  Y:U:V/16:16:16
                                return  (0 == planeIndex)
                                    ?   (~15) & (15 + (imgWidth))   //  Y: 16-alignment
                                    :   (~15) & (15 + (imgWidth>>1))//  UV:16-alignment
                                        ;
                            }

    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const
                            {
                                return  (0 == planeIndex)
                                    ?   (imgHeight)         //  Y
                                    :   (imgHeight) >> 1    //  UV
                                        ;
                            }

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV420SP : public CamFmtBase
{
    typedef CamFmt_YUV420SP classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV420SP()
                                : CamFmtBase(
                                    "NV21 - 420sp 2 planar", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV420SP, 
                                    12, 
                                    2
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_NV21;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const   { return (getBitsPerPixel() * getImgWidthStride(imgWidth, 0) * getImgHeightStride(imgHeight, 0)) >> 3; }
    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const    { return imgWidth; }
    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const
                            {
                                return  (0 == planeIndex)
                                    ?   (imgHeight)         //  Y
                                    :   (imgHeight) >> 1    //  VU
                                        ;
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV422SP : public CamFmtBase
{
    typedef CamFmt_YUV422SP classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV422SP()
                                : CamFmtBase(
                                    "NV16 - yuv422sp", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV422SP, 
                                    16, 
                                    2
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_NV16;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const   { return (getBitsPerPixel() * getImgWidthStride(imgWidth, 0) * getImgHeightStride(imgHeight, 0)) >> 3; }
    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const    { return imgWidth; }
    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const  { return imgHeight; }

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_1Plane : public CamFmtBase
{
public:     ////            Instantiation.
                            CamFmt_1Plane(char const* szAbout, char const* szFormat, int const iBitsPerPixel)
                                : CamFmtBase(szAbout, szFormat, iBitsPerPixel, 1)
                            {}

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const   { return (getBitsPerPixel() * getImgWidthStride(imgWidth, 0) * getImgHeightStride(imgHeight, 0)) >> 3; }
    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const    { return imgWidth; }
    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const  { return imgHeight; }

};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV422I : public CamFmt_1Plane
{
    typedef CamFmt_YUV422I  classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV422I()
                                : CamFmt_1Plane(
                                    "yuv422i - YUY2(yuyv)", 
                                    CameraParameters::PIXEL_FORMAT_YUV422I, 
                                    16
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_YUY2;
                            #endif
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV422I_YVYU : public CamFmt_1Plane
{
    typedef CamFmt_YUV422I_YVYU  classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV422I_YVYU()
                                : CamFmt_1Plane(
                                    "yuv422i - YVYU", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV422I_YVYU, 
                                    16
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_YVYU;
                            #endif
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV422I_UYVY : public CamFmt_1Plane
{
    typedef CamFmt_YUV422I_UYVY  classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV422I_UYVY()
                                : CamFmt_1Plane(
                                    "yuv422i - UYVY", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV422I_UYVY, 
                                    16
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_UYVY;
                            #endif
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_YUV422I_VYUY : public CamFmt_1Plane
{
    typedef CamFmt_YUV422I_VYUY  classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_YUV422I_VYUY()
                                : CamFmt_1Plane(
                                    "yuv422i - VYUY", 
                                    MtkCameraParameters::PIXEL_FORMAT_YUV422I_VYUY, 
                                    16
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_VYUY;
                            #endif
                            }
};

/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
class CamFmt_RGB565 : public CamFmt_1Plane
{
    typedef CamFmt_RGB565   classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_RGB565()
                                : CamFmt_1Plane(
                                    "rgb565", 
                                    MtkCameraParameters::PIXEL_FORMAT_RGB565, 
                                    16
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_RGB565;
                            #endif
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
struct CamFmt_RGBA8888 : public CamFmt_1Plane
{
    typedef CamFmt_RGBA8888 classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_RGBA8888()
                                : CamFmt_1Plane(
                                    "rgba8888 - R:LSB, A:MSB", 
                                    MtkCameraParameters::PIXEL_FORMAT_RGBA8888, 
                                    32
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
#warning "[FIXME] RGBA8888 --> eImgFmt_UNKNOWN"
                                meImageioFormat = eImgFmt_UNKNOWN;
                            #endif
                            }
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
struct CamFmt_BAYER8 : public CamFmt_1Plane
{
    typedef CamFmt_BAYER8 classType;
public:
    static classType*       inst() { static classType o; return &o; }
                            CamFmt_BAYER8()
                                : CamFmt_1Plane(
                                    "bayer 8 - ",
                                    MtkCameraParameters::PIXEL_FORMAT_BAYER8,
                                    8
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_BAYER8;
                                miBitsPerPixel = queryRawBitPerPixel(meImageioFormat);
                            #endif 
                            }
                            
   virtual size_t           getImgWidthStride(size_t imgWidth, size_t planeIndex) const
                            { 
                                 return queryRawStride(meImageioFormat, imgWidth); 
                            }    
};


/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
struct CamFmt_BAYER10 : public CamFmt_1Plane
{
    typedef CamFmt_BAYER10 classType;
public:
    static classType*       inst() { static classType o; return &o; }
                            CamFmt_BAYER10()
                                : CamFmt_1Plane(
                                    "bayer 10 - ",
                                    MtkCameraParameters::PIXEL_FORMAT_BAYER10,
                                    10
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_BAYER10;
                                miBitsPerPixel = queryRawBitPerPixel(meImageioFormat);
                            #endif 
                            }
                            
   virtual size_t           getImgWidthStride(size_t imgWidth, size_t planeIndex) const
                            { 
                                 return queryRawStride(meImageioFormat, imgWidth); 
                            }    
};



/******************************************************************************
 *   Camera Format - 
 ******************************************************************************/
struct CamFmt_JPEG : public CamFmtBase
{
    typedef CamFmt_JPEG     classType;
public:     ////            Instantiation.
    static  classType*      inst() { static classType o; return &o; }
                            CamFmt_JPEG()
                                : CamFmtBase(
                                    "jpeg", 
                                    MtkCameraParameters::PIXEL_FORMAT_JPEG, 
                                    0, 
                                    1
                                )
                            {
                            #if SUPPORT_IMAGEIO_FORMAT
                                meImageioFormat = eImgFmt_JPEG;
                            #endif
                            }

public:     ////            Attributes.
    virtual size_t          getImgBufferSize(size_t imgWidth, size_t imgHeight) const   { return 0; }
    virtual size_t          getWidthStride(size_t imgWidth, size_t planeIndex) const    { return imgWidth; }
    virtual size_t          getHeightStride(size_t imgHeight, size_t planeIndex) const  { return imgHeight; }

};


/******************************************************************************
 *   Camera Format Map Implementation
 ******************************************************************************/
struct CamFmtMapMgr
{
public:     ////            Data Members.
    typedef DefaultKeyedVector<String8, CamFmtBase*>    MapType;
    MapType                 mvFmtInstMap;

public:     ////            Instantiation.
                            CamFmtMapMgr()
                                : mvFmtInstMap(NULL)
                            {
                                mvFmtInstMap.clear();
                                //
#define DO_MAP(_format_)    \
            do { \
                mvFmtInstMap.add(String8(MtkCameraParameters::PIXEL_FORMAT_##_format_), CamFmt_##_format_::inst()); \
            } while (0)
                            //
                            DO_MAP(YUV420SP);
                            DO_MAP(YUV420P);
                            DO_MAP(YV12_GPU);
                            DO_MAP(YUV420I);
                            DO_MAP(YUV422SP);
                            DO_MAP(YUV422I);
                            DO_MAP(YUV422I_YVYU);
                            DO_MAP(YUV422I_UYVY);
                            DO_MAP(YUV422I_VYUY);                            
                            DO_MAP(RGB565);
                            DO_MAP(RGBA8888);
                            DO_MAP(BAYER8);
                            DO_MAP(BAYER10);
                            DO_MAP(JPEG);
                            //
#undef  DO_MAP
                            }
};


CamFmtMapMgr&
getCamFmtMapMgr()
{
    static CamFmtMapMgr inst;
    return inst;
}


CamFmtMapMgr::MapType
getCamFmtMap()
{
    return  getCamFmtMapMgr().mvFmtInstMap;
}


/*****************************************************************************
 * @brief Query the imageio format.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * imageio format.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding imageio format (i.e. EImageFormat)
 *
 ******************************************************************************/
uint_t
queryImageioFormat(char const* szPixelFormat)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        uint_t const uFormat = p->getImageioFormat();
        MY_LOGV("[%s] imageio format: 0x%08X", szPixelFormat, uFormat);
        return  uFormat;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the bits per pixel.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * bits per pixel.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding bits per pixel.
 *
 ******************************************************************************/
int
queryBitsPerPixel(char const* szPixelFormat)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        int const bitsPerPixel = p->getBitsPerPixel();
        MY_LOGV("[%s] bit per pixel: %d", szPixelFormat, bitsPerPixel);
        return  bitsPerPixel;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the plane count.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, return its corresponding
 * plane count.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding plane count.
 *
 ******************************************************************************/
size_t
queryPlaneCount(char const* szPixelFormat)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        size_t const PlaneCount = p->getPlaneCount();
        MY_LOGV("[%s] Plane Count: %d", szPixelFormat, PlaneCount);
        return  PlaneCount;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query a specific plane's width stride.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image width and plane's 
 * index, return its corresponding plane's stride.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgWidth: image width, in pixel.
 *
 * @param[in] planeIndex: plane's index; 0, 1, and 2 refer to 1st-, 2nd-, and 
 * 3rd plane, respectively.
 *
 * @return its corresponding plane's stride, in pixel
 *
 ******************************************************************************/
size_t
queryImgWidthStride(char const* szPixelFormat, size_t imgWidth, size_t planeIndex)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        size_t const WidthStride = p->getImgWidthStride(imgWidth, planeIndex);
        MY_LOGV("[%s] %dth-plane stride(%d) for image width(%d)", szPixelFormat, planeIndex, WidthStride, imgWidth);
        return  WidthStride;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query a specific plane's height stride.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image height and plane's 
 * index, return its corresponding plane's stride.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgHeight: image height, in pixel.
 *
 * @param[in] planeIndex: plane's index; 0, 1, and 2 refer to 1st-, 2nd-, and 
 * 3rd plane, respectively.
 *
 * @return its corresponding plane's stride, in pixel
 *
 ******************************************************************************/
size_t
queryImgHeightStride(char const* szPixelFormat, size_t imgHeight, size_t planeIndex)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        size_t const HeightStride = p->getImgHeightStride(imgHeight, planeIndex);
        MY_LOGV("[%s] %dth-plane stride(%d) for image height(%d)", szPixelFormat, planeIndex, HeightStride, imgHeight);
        return  HeightStride;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query a striding buffer size, in bytes.
 *
 * @details Given a CameraParameters::PIXEL_FORMAT_xxx, image width/height, 
 * return its corresponding striding buffer size, in bytes.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e. 
 * CameraParameters::PIXEL_FORMAT_xxx)
 *
 * @param[in] imgWidth: image width, in pixel.
 *
 * @param[in] imgHeight: image height, in pixel.
 *
 * @return its corresponding striding buffer size, in bytes
 *
 ******************************************************************************/
size_t
queryImgBufferSize(char const* szPixelFormat, size_t imgWidth, size_t imgHeight)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    //
    if  ( CamFmtBase* p = getCamFmtMap().valueFor(String8(szPixelFormat)) )
    {
        size_t const bufsize = p->getImgBufferSize(imgWidth, imgHeight);
        MY_LOGD("[%s] %dx%d image buffer size: %d", szPixelFormat, imgWidth, imgHeight, bufsize);
        return  bufsize;
    }
    //
    MY_LOGW("Unsupported CameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat);
    return  0;
}


};  // namespace FmtUtils
};  // namespace MtkCamUtils
};  // namespace android

