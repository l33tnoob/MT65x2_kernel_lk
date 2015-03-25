/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_INC_CAMSHOT_PARAMS_H_
#define _MTK_CAMERA_INC_CAMSHOT_PARAMS_H_


using namespace NSCamHW; 
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

/**  
 * @enum ECamShotNotifyMsg
 * @brief The CamShot notify message type enum 
 *
 */
 enum ECamShotNotifyMsg 
{
    ECamShot_NOTIFY_MSG_NONE          = 0x0000,       /*!< none notify message */    	
    ECamShot_NOTIFY_MSG_SOF           = 0x0001,       /*!< start of frame notify message */ 
    ECamShot_NOTIFY_MSG_EOF           = 0x0002,    /*!< end of frame notify  message */
	ECamShot_NOTIFY_MSG_CSHOT_END 	  = 0x0004,    /*!< end of c-shot notify  message */
	ECamShot_NOTIFY_MSG_FOCUS_VALUE   = 0x0008,     /*!< focus value  notify  message */
    ECamShot_NOTIFY_MSG_ALL           = 0x000F,     /*!< all message notify  */
};

/**  
 * @enum ECamShotDataMsg
 * @brief The CamShot notify message type enum
 *
 */
enum ECamShotDataMsg 
{
    ECamShot_DATA_MSG_NONE           = 0x0000,           /*!< none data  callback message */    	
    ECamShot_DATA_MSG_BAYER          = 0x0001,           /*!< bayer data callback message */    	  
    ECamShot_DATA_MSG_YUV            = 0x0002,             /*!< yuv data callback message */    	
    ECamShot_DATA_MSG_POSTVIEW       = 0x0004,         /*!< postview data callback message */    	
    ECamShot_DATA_MSG_JPEG           = 0x0008,             /*!< jpeg data callback message */    	    
    ECamShot_DATA_MSG_ALL             = 0x000F,             /*!< all data callback message */    	
};

/**  
 * @enum ECamShotImgBufType
 * @brief The CamShot image buffer type enum 
 *
 */
enum ECamShotImgBufType {
    ECamShot_BUF_TYPE_BAYER          = 0,        /*!< buffer type for bayer data  */    
    ECamShot_BUF_TYPE_YUV            = 1,        /*!< buffer type for yuv data */     
    ECamShot_BUF_TYPE_POSTVIEW       = 2,        /*!< buffer type for postview data*/     
    ECamShot_BUF_TYPE_JPEG           = 3,        /*!< buffer type for jpeg data */      
    ECamShot_BUF_TYPE_THUMBNAIL      = 4,        /*!< buffer type for thumnail data*/    
    ECamShot_BUF_TYPE_MAX                             /*!< buffer type max */                                  
}; 

/**  
 * @enum ECamShotCmd
 * @brief The CamShot command type enum
 *
 */
enum ECamShotCmd 
{
    ECamShot_CMD_SET_CSHOT_SPEED     = 0x0000,           	/*!< set continuous shot speed */ 
	ECamShot_CMD_SET_CAPBUF_MGR		 = 0x0001,				/*!< set set capture buf mgr */ 
    ECamShot_CMD_ALL               	 = 0x0002,             	/*!< all command */    	
};


/**  
 * @struct SensorParam
 * @brief This structure is for the sensor parameter 
 *
 */
struct SensorParam
{
public:    //// fields.
    /**
      * @var u4DeviceID 
      * The device ID of the sensor 
      */ 
    MUINT32  u4DeviceID;          
    /**
      * @var u4Scenario 
      * The sensor scenario that get from sensorHal 
      */     
    MUINT32  u4Scenario;          
    /**
      * @var u4Bitdepth 
      * The bit depth of the raw data. 8, 10 , 12 
      */         
    MUINT32  u4Bitdepth;          
    /**
      * @var fgBypassDelay 
      * Bypass the sensor delay, used for skip frame 
      */             
    MBOOL    fgBypassDelay;       // by pass the delay frame count 
    /**
      * @var fgBypassScenaio 
      * Bypass to set the sensor scenario, this can save the sensor init time 
      */                 
    MBOOL    fgBypassScenaio;     // by pass to set the sensor scenaior
    /**
      * @var u4RawType 
      * The raw type, 0: pure raw, 1: pre-process raw 
      */                 
    MUINT32    u4RawType; 
            
public:    //// constructors. 
    SensorParam(
        MUINT32 const _u4DeviceID = 0, 
        MUINT32 const _u4Scenario = 0, 
        MUINT32 const _u4Bitdepth = 0, 
        MBOOL const _fgBypassDelay = 0, 
        MBOOL const _fgBypassScenaio = 0,
        MUINT32 const _u4RawType = 0
    )
        : u4DeviceID(_u4DeviceID)
        , u4Scenario(_u4Scenario)
        , u4Bitdepth(_u4Bitdepth)
        , fgBypassDelay(_fgBypassDelay)
        , fgBypassScenaio(_fgBypassScenaio) 
        , u4RawType(_u4RawType)
    {
    }
}; 


/**  
 * @struct ShotParam
 * @brief This structure is for the shot parameter 
 *
 */
struct ShotParam
{
public:    //// fields.   
    /**
      * @var ePictureFmt 
      * The picture the image format 
      */ 
    EImageFormat      ePictureFmt; 
    /**
      * @var u4PictureWidth 
      * The width of the picture image  
      */     
    MUINT32           u4PictureWidth; 
    /**
      * @var u4PictureHeight 
      * The height of the picture image 
      */     
    MUINT32           u4PictureHeight; 
    /**
      * @var u4PictureRotation 
      * The rotation operation of the picture image 
      */     
    MUINT32           u4PictureRotation; 
    /**
      * @var u4PictureFlip 
      * The flip operation of the picture image 
      */     
    MUINT32           u4PictureFlip; 
    /**
      * @var ePostViewFmt 
      * The image format of the postview image 
      */ 
    EImageFormat      ePostViewFmt; 
    /**
      * @var u4PostViewWidth 
      * The width of the postview image 
      */     
    MUINT32           u4PostViewWidth; 
    /**
      * @var u4PostViewHeight 
      * The height of the postview image 
      */     
    MUINT32           u4PostViewHeight; 
    /**
      * @var u4PostViewRotation 
      * The rotation operation of the postview image 
      */     
    MUINT32           u4PostViewRotation; 
    /**
      * @var u4PostViewFlip 
      * The flip operation of the postview image 
      */     
    MUINT32           u4PostViewFlip; 
    /**
      * @var u4ZoomRatio 
      * The zoom ratio of the shot operation, the value is x100 
      */     
    MUINT32           u4ZoomRatio; 
public:    //// constructors. 
    ShotParam(
        EImageFormat const _ePictureFmt = eImgFmt_YUY2, 
        MUINT32 const _u4PictureWidth = 640, 
        MUINT32 const _u4PictureHeight = 480, 
        MUINT32 const _u4PictureRotation = 0, 
        MUINT32 const _u4PictureFlip = 0, 
        EImageFormat const _ePostViewFmt = eImgFmt_YUY2, 
        MUINT32 const _u4PostViewWidth = 640, 
        MUINT32 const _u4PostViewHeight = 480, 
        MUINT32 const _u4PostViewRotation = 0, 
        MUINT32 const _u4PostViewFlip = 0, 
        MUINT32 const _u4ZoomRatio = 100
    )
        : ePictureFmt(_ePictureFmt)
        , u4PictureWidth(_u4PictureWidth)
        , u4PictureHeight(_u4PictureHeight)
        , u4PictureRotation(_u4PictureRotation) 
        , u4PictureFlip(_u4PictureFlip)
        , ePostViewFmt(_ePostViewFmt)
        , u4PostViewWidth(_u4PostViewWidth)
        , u4PostViewHeight(_u4PostViewHeight)
        , u4PostViewRotation(_u4PostViewRotation)
        , u4PostViewFlip(_u4PostViewFlip)
        , u4ZoomRatio(_u4ZoomRatio)
    {
    }
};


/**  
 * @struct ThumbnailParam
 * @brief This structure is for the thumbnail parameter 
 *
 */
struct ThumbnailParam
{
public: //// fields. 
    /**
      * @var u4ThumbWidth 
      * The width of the thumbnail image 
      */ 	
    MUINT32 u4ThumbWidth; 
    /**
      * @var u4ThumbHeight 
      * The height of the thumbnail image       
      */     
    MUINT32 u4ThumbHeight; 
    /**
      * @var u4ThumbQuality 
      * The jpeg quality factor of the thumbnail image 
      */     
    MUINT32 u4ThumbQuality; 
    /**
      * @var fgThumbIsSOI 
      * If enable to add start of image header in thumbnail image 
      */     
    MBOOL   fgThumbIsSOI; 

public:
    ThumbnailParam (
        MUINT32 const _u4ThumbWidth = 0,
        MUINT32 const _u4ThumbHeight = 0,
        MUINT32 const _u4ThumbQuality = 100,
        MBOOL   const _fgThumbIsSOI = MTRUE
    )  
        : u4ThumbWidth(_u4ThumbWidth)
        , u4ThumbHeight(_u4ThumbHeight)
        , u4ThumbQuality(_u4ThumbQuality)
        , fgThumbIsSOI(_fgThumbIsSOI)
    {
    }
}; 

/**  
 * @struct JpegParam
 * @brief This structure is for the jpeg parameter 
 *
 */
struct JpegParam : public ThumbnailParam
{
public:    //// fields. 
    /**
      * @var u4Quality 
      * The jpeg quality factor of the jpeg image 
      */ 
    MUINT32 u4Quality; 
    /**
      * @var fgIsSOI 
      * If enable to add start of image header in jpeg image 
      */     
    MBOOL   fgIsSOI; 

public:    //// constructors. 
    JpegParam(
        MUINT32 const _u4Quality = 100, 
        MBOOL   const _fgIsSOI = MFALSE
    )
        : ThumbnailParam()
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
    {
    }
    //
    JpegParam(
    	  ThumbnailParam _rParam, 
        MUINT32 const _u4Quality = 100, 
        MBOOL   const _fgIsSOI = MFALSE
    )
        : ThumbnailParam(_rParam.u4ThumbWidth,_rParam.u4ThumbHeight, _rParam.u4ThumbQuality, _rParam.fgThumbIsSOI)
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
    {
    }   
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_INC_CAMSHOT_BUFFERS_H_

