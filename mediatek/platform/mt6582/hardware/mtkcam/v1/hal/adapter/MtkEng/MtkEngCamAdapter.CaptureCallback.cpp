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
#include <camera/MtkCamera.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
#include <mtkcam/v1/IParamsManager.h>

//
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkEngCamAdapter.h"
using namespace NSMtkEngCamAdapter;
//
#include <mtkcam/drv/hwutils.h>
//
#include <sys/prctl.h>
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] "fmt, ::gettid(), getName(), __FUNCTION__, ##arg)
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
//
//  Callback of Error (CAMERA_MSG_ERROR)
//
//  Arguments:
//      ext1
//          [I] extend argument 1.
//
//      ext2
//          [I] extend argument 2.
//
bool
CamAdapter::
onCB_Error(
    int32_t ext1,
    int32_t ext2
)
{
    MY_LOGW("CAMERA_MSG_ERROR %d %d", ext1, ext2);
    mpCamMsgCbInfo->mNotifyCb(CAMERA_MSG_ERROR, ext1, ext2, mpCamMsgCbInfo->mCbCookie);
    return  true;
}


/******************************************************************************
 *   
 ******************************************************************************/
namespace {
struct ShutterThread : public Thread
{
protected:  ////                Data Members.
    sp<CamMsgCbInfo>            mpCamMsgCbInfo;
    int32_t                     mi4PlayShutterSound;

public:
    ShutterThread(
        sp<CamMsgCbInfo> pCamMsgCbInfo, 
        int32_t i4PlayShutterSound
    )
        : Thread()
        , mpCamMsgCbInfo(pCamMsgCbInfo)
        , mi4PlayShutterSound(i4PlayShutterSound)
    {}

    // Good place to do one-time initializations
    status_t
    readyToRun()
    {
        ::prctl(PR_SET_NAME, "ShutterThread", 0, 0, 0);
        //
#if MTKCAM_HAVE_RR_PRIORITY
        int const expect_policy     = SCHED_RR;
        int const expect_priority   = PRIO_RT_CAMERA_SHUTTER_CB;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        CAM_LOGD(
            "[ShutterThread] policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%x, 0x%x)"
            , expect_policy, policy, expect_priority, priority
        );
#endif
        return OK;
    }

private:
    bool
    threadLoop()
    {
        CAM_LOGD("(%d)[ShutterThread] +", ::gettid());
#if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
        mpCamMsgCbInfo->mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_SHUTTER, mi4PlayShutterSound, mpCamMsgCbInfo->mCbCookie);
#else
        mpCamMsgCbInfo->mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mpCamMsgCbInfo->mCbCookie);
#endif
        CAM_LOGD("(%d)[ShutterThread] -", ::gettid());
        return  false;  // returns false, the thread will exit upon return.
    }
};
}; // namespace


/******************************************************************************
 *   
 ******************************************************************************/
//
//  Callback of Shutter (CAMERA_MSG_SHUTTER)
//
//      Invoking this callback may play a shutter sound.
//
//  Arguments:
//      bPlayShutterSound
//          [I] Play a shutter sound if ture; otherwise play no sound.
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one shutter callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
bool
CamAdapter::
onCB_Shutter(
    bool const bPlayShutterSound, 
    uint32_t const u4CallbackIndex
)
{
    if  ( msgTypeEnabled(CAMERA_MSG_SHUTTER) )
    {
        sp<Thread> pThread = new ShutterThread(mpCamMsgCbInfo, bPlayShutterSound);
        if  ( pThread == 0 || pThread->run() != OK )
        {
            MY_LOGW("Fail to run ShutterThread (%p)", pThread.get());
            return  false;
        }
    }
    return  true;
}


/******************************************************************************
 *   
 ******************************************************************************/
//
//  Callback of Postview for Display
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      u4PostviewSize
//          [I] Postview buffer size in bytes.
//
//      puPostviewBuf
//          [I] Postview buffer with its size = u4PostviewSize
//
bool
CamAdapter::
onCB_PostviewDisplay(
    int64_t const   i8Timestamp, 
    uint32_t const  u4PostviewSize, 
    uint8_t const*  puPostviewBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);
#if 1
    //
    if  ( ! u4PostviewSize || ! puPostviewBuf )
    {
        MY_LOGW("Bad callback: size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);
        return  false;
    }
    //
    sp<IImgBufProvider> pImgBufPvdr = mpImgBufProvidersMgr->getProvider(IImgBufProvider::eID_DISPLAY);
    if  ( pImgBufPvdr == 0 )
    {
        MY_LOGW("Bad IImgBufProvider");
        return  false;
    }
    //
    ImgBufQueNode node;
    if  ( ! pImgBufPvdr->dequeProvider(node) )
    {
        MY_LOGW("dequeProvider fail");
        return  false;
    }
    //
    sp<IImgBuf> pImgBuf = node.getImgBuf();
    if  ( u4PostviewSize != pImgBuf->getBufSize() )
    {
        MY_LOGW(
            "callback size(%d) != display:[%d %s %dx%d]", 
            u4PostviewSize, pImgBuf->getBufSize(), pImgBuf->getImgFormat().string(), 
            pImgBuf->getImgWidth(), pImgBuf->getImgHeight()
        );
        node.setStatus(ImgBufQueNode::eSTATUS_CANCEL);
    }
    else
    {
        ::memcpy(pImgBuf->getVirAddr(), puPostviewBuf, u4PostviewSize);
        globalcacheFlushAll();
        MY_LOGD_IF(1, "- globalcacheFlushAll()");
        //
        pImgBuf->setTimestamp(i8Timestamp);
        node.setStatus(ImgBufQueNode::eSTATUS_DONE);
    }
    //
    if  ( ! pImgBufPvdr->enqueProvider(node) )
    {
        MY_LOGW("enqueProvider fail");
        return  false;
    }
    //
#endif
    return  true;
}


/******************************************************************************
 *   
 ******************************************************************************/
//
//  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      u4PostviewSize
//          [I] Postview buffer size in bytes.
//
//      puPostviewBuf
//          [I] Postview buffer with its size = u4PostviewSize
//
bool
CamAdapter::
onCB_PostviewClient(
    int64_t const   i8Timestamp, 
    uint32_t const  u4PostviewSize, 
    uint8_t const*  puPostviewBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);

    MY_LOGW("Not implement yet");

    return  true;
}

namespace {
/******************************************************************************
 *   
 ******************************************************************************/
//
//  Callback of Raw Image (CAMERA_MSG_RAW_IMAGE/CAMERA_MSG_RAW_IMAGE_NOTIFY)
//
//  Arguments:
//      i8Timestamp
//          [I] Raw image timestamp
//
//      u4RawImgSize
//          [I] Raw image buffer size in bytes.
//
//      puRawImgBuf
//          [I] Raw image buffer with its size = u4RawImgSize
//
class Unpack_MT6589_RAW
{
private:
	int width; 
	int height;
	int bitdepth; 
	int pixelID;
    unsigned char *pu4SrcBuf;
    unsigned char *pucBuf;
    unsigned short *pu2DestBuf;

public:
//    Unpack_MT6589_RAW(int w = 0, int h = 0, int bit = 0, int pixel = 0, char *buf = NULL):
//        width(w), height(h), bitdepth(bit), pixelID(pixel), dstBuff(buf);
    Unpack_MT6589_RAW(int w, int h, int bit, int pixel, unsigned char *srcBuf, unsigned char *&pucRawBuf, int &rawSize);
};

Unpack_MT6589_RAW::Unpack_MT6589_RAW(int w, int h, int bit, int pixel, unsigned char *srcBuf, unsigned char *&pucRawBuf, int &rawSize)
    : width(w), height(h), bitdepth(bit), pixelID(pixel), pu4SrcBuf(srcBuf)
{
    CAM_LOGD("w, h, bit, pixel, src = %d, %d, %d, %d, %p", width, height, bitdepth, pixelID, pu4SrcBuf);

    if (NULL == pu4SrcBuf)
    {
        pucRawBuf = NULL;
        rawSize = 0;
        return;
    }

    if ((0 == w) || (0 == h))
    {
        pucRawBuf = NULL;
        rawSize = 0;
        return;
    }
    
    switch (bitdepth)
    {
        case 8:
        case 10:
        case 12:
            break;
        default:
            pucRawBuf = NULL;
            rawSize = 0;
            return;
    }
    
	MUINT32 stride = width; 

	if (bitdepth == 10)
	{
		// 10-bit the stride should align 8 bytes 
        if (width % 8)
        {
		    // align to 8 
            stride = width + 8 - (width % 8); 
        }
        stride = stride * 10 / 8; 
	}
	else if(bitdepth == 12) 
	{
		// 12-bit the stride should align 6 bytes 
		stride = (width * bitdepth + 7) / 8; 
		// align to 6
		stride =  (5 + stride) /6 * 6;         
	}
    CAM_LOGD("Raw stride = %d", stride);
    
	pucBuf = (unsigned char *) malloc (width * height  * 2 * sizeof(unsigned char));
    
    pu2DestBuf = (unsigned short  *)pucBuf;

	rawSize = width * height * 2; 
    pucRawBuf = pucBuf;
    
    if (8 == bitdepth) 
	{
		for (int i = 0; i < width * height; i++) 
		{
			unsigned char uPixel = *(pu4SrcBuf++); 
			*(pu2DestBuf++) = uPixel; 
		}
	}
	else if (10 == bitdepth)
	{
   	    for (int h = 0; h < height; h++) 
	    {
		   unsigned char *lineBuf = pu4SrcBuf + h * stride; 
		   
		   for (int w = 0; w < width/4; w++)
		   {
    		   unsigned char byte0 = (unsigned char)(*(lineBuf++)); 
	    	   unsigned char byte1 = (unsigned char)(*(lineBuf++)); 
		       unsigned char byte2 = (unsigned char)(*(lineBuf++)); 
		       unsigned char byte3 = (unsigned char)(*(lineBuf++)); 
			   unsigned char byte4 = (unsigned char)(*(lineBuf++)); 
               *(pu2DestBuf++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8)); 
			   *(pu2DestBuf++) = (unsigned short) (((byte1 & 0xFC) >> 2) + ((byte2 & 0xf) << 6)); 
			   *(pu2DestBuf++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 4)); 
		       *(pu2DestBuf++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2)); 
	       }
			// process last pixel in the width 
			if (width % 4 != 0) 
			{
    		   unsigned char byte0 = *(lineBuf++); 
	    	   unsigned char byte1 = *(lineBuf++);
		       unsigned char byte2 = *(lineBuf++);
		       unsigned char byte3 = *(lineBuf++);
			   unsigned char byte4 = *(lineBuf++); 
                for (int w = 0; w < width % 4; w++)
				{
					switch (w)
					{
					case 0:
                        *(pu2DestBuf++) = (unsigned short) (byte0 + ((byte1 & 0x3) << 8));
					    break; 
					case 1:
                        *(pu2DestBuf++) = (unsigned short) (((byte1 & 0x3F) >> 2) + ((byte2 & 0xf) << 6)); 
						break;
					case 2:
						*(pu2DestBuf++) = (unsigned short) (((byte2 & 0xf0) >> 4) + ((byte3 & 0x3f) << 6)); 
						break;
					case 3:
                        *(pu2DestBuf++) = (unsigned short) (((byte3 & 0xc0) >> 6) + (byte4 << 2)); 
						break;
					}
				}
			}
		}
	}	
	else if (12 == bitdepth)
	{
		for (int h = 0; h < height; h++)
		{
			unsigned char *lineBuf = pu4SrcBuf + h * stride; 
			int w = 0; 			
			for (int w = 0; w < width/4; w++)
			{
				unsigned char byte0 = *(lineBuf++); 
				unsigned char byte1 = *(lineBuf++); 
				unsigned char byte2 = *(lineBuf++); 
				unsigned char byte3 = *(lineBuf++); 
				unsigned char byte4 = *(lineBuf++); 
                unsigned char byte5 = *(lineBuf++); 
				*(pu2DestBuf++) = (unsigned short) (byte0 + ((byte1 & 0xf) << 8)); 
                *(pu2DestBuf++) = (unsigned short) ((byte1 >> 4) + (byte2 << 4));
				*(pu2DestBuf++) = (unsigned short) (byte3 + ((byte4 & 0xf) << 8)); 
                *(pu2DestBuf++) = (unsigned short) ((byte4 >> 4) + (byte5 << 4));									
			}	
			// process last pixel in the width 
			if (width % 4 != 0) 
			{
				unsigned char byte0 = *(lineBuf++); 
				unsigned char byte1 = *(lineBuf++); 
				unsigned char byte2 = *(lineBuf++); 
				unsigned char byte3 = *(lineBuf++); 
				unsigned char byte4 = *(lineBuf++); 
                unsigned char byte5 = *(lineBuf++); 
                for (int w = 0; w < width % 4; w++)
				{
					switch (w)
					{
					case 0:
                        *(pu2DestBuf++) = (unsigned short) (byte0 + ((byte1 & 0xf) << 8)); 
					    break; 
					case 1:
                        *(pu2DestBuf++) = (unsigned short) ((byte1 >> 4) + (byte2 << 4));
						break;
					case 2:
						*(pu2DestBuf++) = (unsigned short) (byte3 + ((byte4 & 0xf) << 8));    
						break;
					case 3:
                        *(pu2DestBuf++) = (unsigned short) ((byte4 >> 4) + (byte5 << 4));			
						break;
					}
				}
			}
		}
	}
}
} // namespace { // anonymous namespace

bool
CamAdapter::
onCB_RawImage(
    int64_t const   i8Timestamp, 
    uint32_t const  u4RawImgSize, 
    uint8_t const*  puRawImgBuf
)
{
    CAM_LOGD("Raw image size, buf = %d, 0x%X", u4RawImgSize, puRawImgBuf);
    
    sp<IParamsManager> pParamsMgr = getParamsManager();
    String8 ms8RawFilePath(pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH)); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0.raw
    String8 ms8SaveMode(pParamsMgr->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE));
    char const *mpsz8RawSaveMode = ms8SaveMode.string();
#warning "[TODO] Magic number: need to enumerize"
    switch (mpsz8RawSaveMode[0])
    {
        case '1': // 1: "Preview Mode", 
        case '2': // 2: "Capture Mode",
        case '4': // 4: "Video Preview Mode"
            if  ( (u4RawImgSize > 0) && (puRawImgBuf != 0) && (ms8RawFilePath != ""))
            {
                String8 ms8RawFileExt(ms8RawFilePath.getPathExtension()); // => .raw
                ms8RawFilePath = ms8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0

                String8 ms8IspMode = pParamsMgr->getStr(MtkCameraParameters::KEY_ISP_MODE);
                const char *strIspMode = ms8IspMode.string();
                String8 ms8RawType;
                switch (strIspMode[0])
                {
                    case EngParam::ENG_ISP_MODE_PROCESSED_RAW:
                        ms8RawType = "proc";
                        break;
                    case EngParam::ENG_ISP_MODE_PURE_RAW:
                        ms8RawType = "pure";
                        break;
                }

                char mpszSuffix[256] = {0};
                sprintf(mpszSuffix, "%s__%dx%d_%d_%d", ms8RawType.string(), engParam.u4SensorWidth, engParam.u4SensorHeight, engParam.u4Bitdepth, engParam.u4RawPixelID); /* info from EngShot::onCmd_capture */

                ms8RawFilePath.append(mpszSuffix);
                ms8RawFilePath.append(ms8RawFileExt);

                unsigned char *pucRawBuf = NULL;
                int rawSize = 0;
                Unpack_MT6589_RAW unpack(engParam.u4SensorWidth, engParam.u4SensorHeight, engParam.u4Bitdepth, engParam.u4RawPixelID, (const_cast<uint8_t*>(puRawImgBuf)), pucRawBuf, rawSize);
                // Unpack_MT6589_RAW(int w, int h, int bit, int pixel, unsigned char *srcBuf, unsigned char *pucRawBuf, int &rawSize);

                if (pucRawBuf)
                {
                    bool ret = saveBufToFile(ms8RawFilePath.string(), pucRawBuf, rawSize); // bool ret = saveBufToFile(ms8RawFilePath.string(), const_cast<uint8_t*>(puRawImgBuf), u4RawImgSize); 
                    // saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
                    free(pucRawBuf);
                    CAM_LOGD("Raw saved: %d: %s", ret, ms8RawFilePath.string());
                }

            }
            break;
        case '0': // 0: do not save
        case '3': // 3: "JPEG Only"
        default:
            break;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) )
    {
        MY_LOGD("CAMERA_MSG_RAW_IMAGE_NOTIFY");
        mpCamMsgCbInfo->mNotifyCb(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mpCamMsgCbInfo->mCbCookie);
        return  true;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) )
    {
        MY_LOGD("CAMERA_MSG_RAW_IMAGE");
        if  ( ! u4RawImgSize || ! puRawImgBuf )
        {
            MY_LOGD("dummy callback");
            camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, NULL);
            if  ( pmem )
            {
                mpCamMsgCbInfo->mDataCb(CAMERA_MSG_RAW_IMAGE, pmem, 0, NULL, mpCamMsgCbInfo->mCbCookie);
                pmem->release(pmem);
            }
        }
        else
        {
            camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4RawImgSize, 1, NULL);
            {
                ::memcpy(pmem->data, puRawImgBuf, u4RawImgSize);
                mpCamMsgCbInfo->mDataCb(CAMERA_MSG_RAW_IMAGE, pmem, 0, NULL, mpCamMsgCbInfo->mCbCookie);
                pmem->release(pmem);
            }
        }
    }
    //
    return  true;
}


/******************************************************************************
 *  ZIP (Compressed) Image Callback Thread
 ******************************************************************************/
namespace {
struct ZipImageCallbackThread : public Thread
{
protected:  ////                Data Members.
    char const*const            mpszThreadName;
    sp<CamMsgCbInfo>            mpCamMsgCbInfo;
    camera_memory*              mpImage;
    uint32_t const              mu4CallbackIndex;
    bool                        mfgIsFinalImage;
    uint32_t                    mu4ShotMode;

public:
    ZipImageCallbackThread(
        sp<CamMsgCbInfo> pCamMsgCbInfo, 
        camera_memory* image, 
        uint32_t const u4CallbackIndex, 
        bool const fgIsFinalImage, 
        uint32_t const u4ShotMode
    )
        : Thread()
        , mpszThreadName("ZipImageCallbackThread")
        , mpCamMsgCbInfo(pCamMsgCbInfo)
        , mpImage(image)
        , mu4CallbackIndex(u4CallbackIndex)
        , mfgIsFinalImage(fgIsFinalImage)
        , mu4ShotMode(u4ShotMode)
    {}

    // Good place to do one-time initializations
    status_t
    readyToRun()
    {
        ::prctl(PR_SET_NAME, mpszThreadName, 0, 0, 0);
        //
#if MTKCAM_HAVE_RR_PRIORITY
        int const expect_policy     = SCHED_RR;
        int const expect_priority   = PRIO_RT_CAMERA_ZIP_IMAGE_CB;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        CAM_LOGD(
            "[%s] policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%x, 0x%x)"
            , mpszThreadName, expect_policy, policy, expect_priority, priority
        );
#endif
        return OK;
    }

private:
    bool
    threadLoop()
    {
        if  ( mfgIsFinalImage )
        {
            CAM_LOGD("(%d)[%s] the final image: wait done before callback", ::gettid(), mpszThreadName);
            IStateManager* pStateManager = IStateManager::inst();
            IStateManager::StateObserver stateWaiter(pStateManager);
            pStateManager->registerOneShotObserver(&stateWaiter);
            if  ( OK != stateWaiter.waitState(IState::eState_Idle) )
            {
                CAM_LOGW(
                    "(%d)[%s] do nothing due to fail to wait - Index:%d ShotMode:%d", 
                    ::gettid(), mpszThreadName, mu4CallbackIndex, mu4ShotMode
                );
                return  false;
            }
        }

#if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
        CAM_LOGD("(%d)[%s] MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE - Index:%d ShotMode:%d", ::gettid(), mpszThreadName, mu4CallbackIndex, mu4ShotMode);
        mpCamMsgCbInfo->mDataCb(MTK_CAMERA_MSG_EXT_DATA, mpImage, 0, NULL, mpCamMsgCbInfo->mCbCookie);
        mpImage->release(mpImage);
#else
        CAM_LOGD("(%d)[%s] CAMERA_MSG_COMPRESSED_IMAGE - Index:%d ShotMode:%d", ::gettid(), mpszThreadName, mu4CallbackIndex, mu4ShotMode);
        mpCamMsgCbInfo->mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, mpImage, 0, NULL, mpCamMsgCbInfo->mCbCookie);
        mpImage->release(mpImage);
#endif
        if  ( mfgIsFinalImage )
        {
            mpCamMsgCbInfo->mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE, 0, mpCamMsgCbInfo->mCbCookie);
        }
        // Fix me this, make only continuous shot mode CB end msg
        if  ( mfgIsFinalImage && eShotMode_ContinuousShot == mu4ShotMode )
        {
#if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
            CAM_LOGD("Continuous shot end msg callback, total shot number is %d", mu4CallbackIndex);
            mpCamMsgCbInfo->mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_CONTINUOUS_END, mu4CallbackIndex, mpCamMsgCbInfo->mCbCookie);
#endif
        }

        CAM_LOGD("(%d)[%s] -", ::gettid(), mpszThreadName);
        return  false;  // returns false, the thread will exit upon return.
    }

};
}; // namespace


/******************************************************************************
 *   
 ******************************************************************************/
//
//  Callback of Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
//
//      [Compressed Image] = [Header] + [Bitstream], 
//      where 
//          Header may be jpeg exif (including thumbnail)
//
//  Arguments:
//      i8Timestamp
//          [I] Compressed image timestamp
//
//      u4BitstreamSize
//          [I] Bitstream buffer size in bytes.
//
//      puBitstreamBuf
//          [I] Bitstream buffer with its size = u4BitstreamSize
//
//      u4HeaderSize
//          [I] Header size in bytes; header may be jpeg exif.
//
//      puHeaderBuf
//          [I] Header buffer with its size = u4HeaderSize
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one compressed callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
//      fgIsFinalImage
//          [I] booliean value to indicate whether it is the final image.
//              true if this is the final image callback; otherwise false.
//              For single captures, this value must be true.
//
bool
CamAdapter::
onCB_CompressedImage(
    int64_t const   i8Timestamp,
    uint32_t const  u4BitstreamSize,
    uint8_t const*  puBitstreamBuf,
    uint32_t const  u4HeaderSize,
    uint8_t const*  puHeaderBuf,
    uint32_t const  u4CallbackIndex,
    bool            fgIsFinalImage,
    uint32_t const  msgType
)
{
    MY_LOGD(
        "timestamp(%lld), bitstream:size/buf=%d/%p, header:size/buf=%d/%p, index(%d), IsFinalImage(%d)", 
        i8Timestamp, u4BitstreamSize, puBitstreamBuf, u4HeaderSize, puHeaderBuf, u4CallbackIndex, fgIsFinalImage
    );
    //
    if  ( ! msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) )
    {
        MY_LOGW("msgTypeEnabled=%#x", msgTypeEnabled(0xFFFFFFFF));
        return  false;
    }
    //
    camera_memory* image = NULL;
    uint8_t* pImage = NULL;
    //
#if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
    uint32_t const u4DataSize = u4HeaderSize + u4BitstreamSize + sizeof(uint32_t)*(1+1);
    image = mpCamMsgCbInfo->mRequestMemory(-1, u4DataSize, 1, NULL);
    if  ( image )
    {
        uint32_t*const pCBData = reinterpret_cast<uint32_t*>(image->data);
        pCBData[0] = msgType;
        pCBData[1] = u4CallbackIndex;
        pImage = reinterpret_cast<uint8_t*>(&pCBData[2]);
    }
#else
    image = mpCamMsgCbInfo->mRequestMemory(-1, u4HeaderSize + u4BitstreamSize, 1, NULL);
    if  ( image )
    {
        pImage = reinterpret_cast<uint8_t*>(image->data);
    }
#endif
    if  ( ! image )
    {
        MY_LOGW("mRequestMemory fail");
        return  false;
    }
    //
    if  ( image )
    {
        if  ( 0 != u4HeaderSize && 0 != puHeaderBuf )
        {
            ::memcpy(pImage, puHeaderBuf, u4HeaderSize);
            pImage += u4HeaderSize;
        }
        if  ( 0 != u4BitstreamSize && 0 != puBitstreamBuf )
        {
            ::memcpy(pImage, puBitstreamBuf, u4BitstreamSize);
        }
        //
        sp<Thread> pThread = new ZipImageCallbackThread(mpCamMsgCbInfo, image, u4CallbackIndex, fgIsFinalImage, getParamsManager()->getShotMode());
        if  ( pThread == 0 || pThread->run() != OK )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            return  false;
        }
    }
    return  true;
}

