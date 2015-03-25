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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkBase.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMain.h

#ifndef _ACDKMAIN_H_
#define _ACDKMAIN_H_

//====== Define Value ======
#define SURFACE_NUM 2

//====== Declaration ======

namespace NSACDK
{   
    /**  
         *@struct acdkMainSensorInfo_s
         *@brief  Used for store sensor infomation
       */
    typedef struct acdkMainSensorInfo_s
    {
        MINT32 facing;
        MINT32 devType;
    }acdkMainSensorInfo_t; 

    /**
         *@enum acdkMainState_e
         *@brief State enum of AcdkMain
       */
    typedef enum 
    {
        ACDK_MAIN_NONE           = 0x0000,        
        ACDK_MAIN_INIT           = 0x0001,
        ACDK_MAIN_IDLE           = 0x0002,
        //
        ACDK_MAIN_PREVIEW_MASK   = 0x0100,
        ACDK_MAIN_PREVIEW        = 0x0100,
        //
        ACDK_MAIN_CAPTURE_MASK   = 0x0200,        
        ACDK_MAIN_CAPTURE        = 0x0201,       
        //
        ACDK_MAIN_UNINIT         = 0x0400,
        //
        ACDK_MAIN_ERROR          = 0x8000
    } acdkMainState_e;

    /**
         *@class AcdkMain
         *@brief This class takes charge of flow control of ACDK and implements AcdkBase
       */
    class AcdkMain : public AcdkBase
    {
    public:

        /**
                *@brief AcdkMain constructor
              */
        AcdkMain ();

        /**
                *@brief Destroy AcdkMain object
              */
        virtual void destroyInstance();

        /**
                *@brief Initialize function
                *@note Must call this function right after createInstance and before other functions
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 init ();
        
        /**
                *@brief Uninitialize function
                *@note Must call this function before destroyInstance
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 uninit();
        
        /**
                *@brief Start preview
                *
                *@param[in] prvCb : preview callback function
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 startPreview(Func_CB prvCb);

        /**
                *@brief Stop preview
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 stopPreview();

        /**
                *@brief Capture
                *@note If parameter width and height are not spcified, ACDK will decide by itself based on capture mode
                *
                *@param[in] mode : capture mode
                *@param[in] imgType : capture image type
                *@param[in] capCb : capture callback function
                *@param[in] width : width of capture image
                *@param[in] height : height of capture image
                *@param[in] captureCnt : capture times. default is 1
                *@param[in] isSaveImg : save capture image or not. 0-no save, 1-save. default is 0
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 takePicture(
                MUINT32 const mode,
                MUINT32 const imgType,
                Func_CB const capCb = 0,
                MUINT32 const width = 0,
                MUINT32 const height = 0,
                MUINT32 const captureCnt = 1,
                MINT32  const isSaveImg = 0);
        
        /**
                *@brief Get preview frame number
                *
                *@param[in,out] frameCnt : will set to preview frame number
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 getFrameCnt(MUINT32 &frameCnt);

        /**
                *@brief Setting sensor device
                *@note Sensor device is default setted to main sensor
                *
                *@param[in] srcDev : 0x1-main sensor, 0x2-sub sensor, 0x8-main2 sensor
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 setSrcDev(MINT32 srcDev);

        /**
                *@brief Show quick-view image
                *@note It will be called after takePicture automatically
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 quickViewImg(MUINT32 qvFormat);

        /**
                *@brief Command interface
                *
                *@param[in] a_u4Ioctl : command
                *@param[in] puParaIn : input parameter
                *@param[in] u4ParaInLen : input parameter length
                *@param[in] puParaOut : output parameter
                *@param[in] u4ParaOutLen : output parameter length
                *@param[in] pu4RealParaOutLen
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen);

    protected:

        /**
                *@brief AcdkMain destructor
              */
        virtual ~AcdkMain();

    private:

        /**
                *@brief Set current sate of AcdkMain to newState
                *
                *@param[in] newState : new state
              */
        virtual MVOID acdkMainSetState(acdkMainState_e newState);

        /**
                *@brief Get current sate of AcdkMain
              */
        virtual acdkMainState_e acdkMainGetState();

        /**
                *@brief Check state to avoid wrong command from user
              */
        virtual MINT32 stateMgr(acdkMainState_e nextState);

        /**
                *@brief Capture callback function
                *@details It is used to get data buffer from Single Shot
                *
                *@param[in] user : object pointer 
                *@param[in] msg : capture callback info structure
                *
                *@return
                *-MTRUE indicates success, otherwise indicates fail
              */
        static MBOOL camShotDataCB(MVOID *user, CamShotDataInfo const msg);

        /**
                *@brief Camera callback function
                *@details It is used to get data buffer from AcdkMhalBase
                *
                *@param[in] param : pointer of acdkCBInfo structure
              */
        static void cameraCallback(MVOID *param);

        /**
                *@brief Dispatch camera callback to right handle function depends on callback type
              */
        void dispatchCallback(MVOID *param);

        /**
                *@brief Handle preview callback
                *
                *@param[in] param : pointer of acdkCallbackParam_t structure
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 handlePreviewCB(MVOID *param);

        /**
                *@brief Handle RAW capture callback
                *
                *@param[in] param : pointer of acdkCallbackParam_t structure
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 handleRAWCB(MVOID *param);

        /**
                *@brief Handle JPEG capture callback
                *
                *@param[in] param : pointer of acdkCallbackParam_t structure
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 handleJPEGCB(MVOID *param);

        /**
                *@brief Handle QV callback
                *
                *@param[in] param : pointer of acdkCallbackParam_t structure
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 handleQVCB(MVOID *param);

        /**
                *@brief Save JPEG image
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 saveJPEGImg();

        /**
                *@brief Save RAW image
                *
                *@param[in] mode : Capture mode
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 saveRAWImg(MINT32 mode);

        /**
                *@brief Initialize sensor
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 sensorInit();

        /**
                *@brief Get sensor infomation
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 getSensorInfo();

        /**
                *@brief Calculate preview size based on screen size and orientation
                *
                *@param[in] surfaceWidth : screen width
                *@param[in] surfaceHeight : screen height
                *@param[in,out] x : will set to x-direction start cooridinate
                *@param[in,out] y : will set to y-direction start cooridinate
                *@param[in,out] width : will set to final preview width
                *@param[in,out] height : will set to fianl preview height
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 calcPreviewWin(
               MUINT32 const surfaceWidth,
               MUINT32 const surfaceHeight,
               MUINT32 &x,
               MUINT32 &y,
               MUINT32 &width,
               MUINT32 &height);
        
        /**
                *@brief Create memory by using IMem
                *
                *@param[in,out] memSize : input already calculated size and will set to alingSize
                *@param[in] bufCnt : how many memory need to be created
                *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo);

        /**
                *@brief Destroy memory by using IMem
                *
                *@param[in] bufCnt : how many memory need to be destroyed
                *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo);

        /**
                *@brief Setting sensor related infomation
                *
                *@param[in] mode : sensor mode
                *@param[in,out] imgFormat : will set to correct sensor format
                *@param[in,out] imgSize : will set to correct sensor image size
                *@param[in,out] imgStride : will set to correct stride
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MINT32 sensorFormatSetting(MUINT32 mode, MUINT32 &imgFormat, MUINT32 &imgSize, MUINT32 *imgStride = NULL);

        /**
                *@brief Setting shutter time
                *@note this is for factory-camera auto-testing usage
                *
                *@param[in] a_time : specific shutter time
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        virtual MINT32 setShutterTime(MUINT32 a_time);

        /**
                *@brief MakeExif Header
                *@note this is for cct jpeg exif header
                *
                *@param[in] a_time : Jpeg Exif Header
                *
                *@return
                *-0 indicates success, otherwise indicates fail
              */
        MBOOL makeExifHeader(MUINT32 const u4CamMode, 
                                MUINT8* const puThumbBuf, 
                                MUINT32 const u4ThumbSize, 
                                MUINT8* puExifBuf, 
                                MUINT32 &u4FinalExifSize, 
                                MUINT32 u4ImgIndex, 
                                MUINT32 u4GroupId);

        /********************************************************************************/
        
        AcdkMhalBase *m_pAcdkMhalObj;
        AcdkUtility *m_pAcdkUtilityObj;

        mutable Mutex mLock;

        // state
        acdkMainState_e m_eAcdkMainState;

        // operation mode
        eACDK_OPERA_MODE mOperaMode;

        //acdkMhal parametner
        acdkMhalPrvParam_t mAcdkMhalPrvParam;

        // AF related variable
        MINT32 mFocusDone;

        // IMEM
        IMemDrv *m_pIMemDrv;
       
        //preview related variable
        MUINT32 mFrameCnt;
        MUINT32 mPrvWidth;
        MUINT32 mPrvHeight;
        MUINT32 mPrvStartX;
        MUINT32 mPrvStartY;
        MUINT32 mOrientation;
        MBOOL   mIsFacotory;
        MUINT16 mTestPatternOut;

        //capture related variable
        ISingleShot *m_pSingleShot;

        MUINT32 mCapWidth;
        MUINT32 mCapHeight;
        MUINT32 mCapType;
        MUINT32 mQVWidth;
        MUINT32 mQVHeight;
        MBOOL   mUnPack;
        MBOOL   mIsSOI; 
        // preview & display buffer
        IMEM_BUF_INFO mPrvIMemInfo[OVERLAY_BUFFER_CNT];
        IMEM_BUF_INFO mDispIMemInfo[OVERLAY_BUFFER_CNT];

        //capture buffer
        IMEM_BUF_INFO mRawIMemInfo;
        IMEM_BUF_INFO mJpgIMemInfo;
        IMEM_BUF_INFO mQvIMemInfo;
      
        //surface View
        AcdkSurfaceView *m_pAcdkSurfaceViewObj;
        IMEM_BUF_INFO mSurfaceIMemInfo[SURFACE_NUM];
        MUINT32 mLCMOrientation;
        MUINT32 mSurfaceIndex;

        //sensor Info
        SensorHal *m_pSensorHalObj;
        MBOOL   mSensorInit;
        MUINT32 mu4SensorDelay;
        MINT32  mSupportedSensorDev;
        MINT32  mSensorDev;
        MINT32  mSensorType;
        MUINT32 mSensorOrientation;
        MUINT32 mSensorVFlip;
        MUINT32 mSensorHFlip;
        ACDK_SENSOR_RESOLUTION_INFO_STRUCT mSensorResolution;
        halSensorRawImageInfo_t mSensorFormatInfo;  // Although name is RawImageInfo, but can get both raw/yuv type

        //factory-camera auto-testing
        MUINT32 mSetShutTime;
        MUINT32 mGetShutTime;
        MUINT32 mGetCheckSumValue;
        //AF information
        MUINT32 mGetAFInfo;

    };     
};
#endif //end AcdkMain.h 

