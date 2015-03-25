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
#define LOG_TAG "CamExif"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>

#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
//
#include <camera_custom_exif.h>
#include <kd_camera_feature.h>
#include <IBaseExif.h>
#include <Exif.h>
//
#include <debug_exif/aaa/dbg_aaa_param.h>
//



/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)


/*******************************************************************************
*
********************************************************************************/
CamExif::
CamExif()
    : mCamExifParam()
    , mp3AEXIFInfo(NULL)
    , mCamDbgParam()
    , mDbgInfo()
    , mMapModuleID()
    , mi4DbgModuleType(0)
    , mpBaseExif(NULL)
{
    MY_LOGD("- this:%p", this);
}

/*******************************************************************************
*
********************************************************************************/
CamExif::
~CamExif()
{
    MY_LOGD("- this:%p", this);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
init(
    CamExifParam const& rCamExifParam,
    CamDbgParam const& rCamDbgParam = CamDbgParam(0, 0)
)
{
    // (0) CamExif: CamExifParam
    mCamExifParam = rCamExifParam;

    MY_LOGD("============================ mCamExifParam ================================\n");
    MY_LOGD("u4GpsIsOn: %d, u4GPSAltitude: %d, uGPSLatitude: %s, uGPSLongitude: %s, uGPSTimeStamp: %s, uGPSProcessingMethod: %s \n",
            mCamExifParam.u4GpsIsOn, mCamExifParam.u4GPSAltitude, mCamExifParam.uGPSLatitude,
            mCamExifParam.uGPSLongitude, mCamExifParam.uGPSTimeStamp, mCamExifParam.uGPSProcessingMethod);
    MY_LOGD("u4Orientation: %d, u4Facing: %d, u4ZoomRatio: %d, u4ImgIndex: %d, u4GroupId: %u, u4FocusH: %d, u4FocusL: %d \n", 
            mCamExifParam.u4Orientation, mCamExifParam.u4Facing, mCamExifParam.u4ZoomRatio,
            mCamExifParam.u4ImgIndex, mCamExifParam.u4GroupId, mCamExifParam.u4FocusH, mCamExifParam.u4FocusL);

    // (1) CamExif: 3A EXIF info
    mp3AEXIFInfo = new EXIF_INFO_T;

    // (2) DebugExif: CamDbgParam
    memset(&mCamDbgParam, 0, sizeof(CamDbgParam));
    mCamDbgParam = rCamDbgParam;
    MY_LOGD("============================ mCamDbgParam ================================\n");
    MY_LOGD("u4ShotMode: %d, u4CamMode: %d \n",
            mCamDbgParam.u4ShotMode, mCamDbgParam.u4CamMode);
    MY_LOGD("===========================================================================\n");

    
    // (3) DebugExif: reset debug information
    mDbgInfo.clear();
    mMapModuleID.clear();

    // (4) Exif Utilitis
    mpBaseExif = new ExifUtils();
    if ( !(mpBaseExif->init()) )
    {
        MY_LOGE("mpBaseExif->init() fail");
        return MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
uninit()
{
    //
    if ( mp3AEXIFInfo != NULL )
    {
        delete mp3AEXIFInfo;
        mp3AEXIFInfo = NULL;
    }
    //
    for (MUINT32 idx = 0; idx < mDbgInfo.size(); idx++)
    {
        if (NULL != mDbgInfo[idx].puDbgBuf)
        {
            MY_LOGI("[uninit] idx %d", idx);
            delete [] mDbgInfo[idx].puDbgBuf;
            mDbgInfo[idx].puDbgBuf = NULL;
        }
    }

    //
    mDbgInfo.clear();
    mMapModuleID.clear();

    //
    if ( mpBaseExif != NULL )
    {
        if ( !(mpBaseExif->uninit()) )
        {
            MY_LOGE("mpBaseExif->uninit() fail");
        }
        delete mpBaseExif;
        mpBaseExif = NULL;
    }

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
makeExifApp1(
    MUINT32 const u4ImgWidth,           //  [I] Image Width
    MUINT32 const u4ImgHeight,          //  [I] Image Height
    MUINT32 const u4ThumbSize,          //  [I] Thumb Size
    MUINT8* const puApp1Buf,            //  [O] Pointer to APP1 Buffer
    MUINT32*const pu4App1HeaderSize     //  [O] Pointer to APP1 Header Size
)
{
    MINT32  err = 0;
    //
    exifAPP1Info_t exifApp1Info;
    exifImageInfo_t exifImgInfo;
    //
    MUINT32 u4App1HeaderSize = 0;

    //  (0) Check arguments.
    if  ( ! puApp1Buf || 0 == u4ImgWidth || 0 == u4ImgHeight )
    {
        MY_LOGE(
            " invalid parameters:(puApp1Buf, u4ImgWidth, u4ImgHeight)=(%p, %d, %d)"
            , puApp1Buf, u4ImgWidth, u4ImgHeight
        );
        goto lbExit;
    }

    //  (1) Fill exifApp1Info
    ::memset(&exifApp1Info, 0, sizeof(exifAPP1Info_t));
    if  ( ! queryExifApp1Info(&exifApp1Info) )
    {
        MY_LOGE(" queryExifApp1Info");
        goto lbExit;
    }

    //  (2) Fill exifImgInfo
    ::memset(&exifImgInfo, 0, sizeof(exifImageInfo_t));
    exifImgInfo.bufAddr     = reinterpret_cast<MUINT32>(puApp1Buf);
    exifImgInfo.mainWidth   = u4ImgWidth;
    exifImgInfo.mainHeight  = u4ImgHeight;
    exifImgInfo.thumbSize   = u4ThumbSize;
    err = mpBaseExif->exifApp1Make(&exifImgInfo, &exifApp1Info, &u4App1HeaderSize);
    if  ( err != 0)
    {
        MY_LOGE(" exifApp1Make - err(%x)", err);
        goto lbExit;
    }

    if  (pu4App1HeaderSize)
    {
        *pu4App1HeaderSize = u4App1HeaderSize;
    }

lbExit:
    return  (0==err);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
makeExifApp3(
    MBOOL   const bIsN3dEnable,     //  [I] Native3D(AC) Enable
    MUINT8* const puApp3Buf,        //  [O] Pointer to APP3 Buffer
    MUINT32*const pu4App3Size       //  [O] Pointer to APP3 Size
)
{
    MINT32  err = 0;
    MUINT32 u4App3Size = 0;

    MUINT8* pDst = puApp3Buf;
    
    MUINT8* pJpsInfo = NULL;
    MUINT32 jpsSize = 0;
    MUINT32 app3ReturnSize = 0;

    exifAPP3Info_t exifAPP3Info;
    ::memset(&exifAPP3Info, 0, sizeof(exifAPP3Info_t));

    /*
     *  0xFF + Marker Number(1 byte) + Data size(2 bytes) + Data(n bytes)
     *  Data size(jpsSize) contains 'Data size' descriptor (plus 2 bytes).
     */
    jpsSize = sizeof(exifAPP3Info_t) + 2;
    MY_LOGD(" jpsSize  = %d", jpsSize);
    //
    const char jpsIdentifier[] = { 0x5F, 0x4A, 0x50, 0x53, 0x4A, 0x50, 0x53, 0x5F }; // _JPSJPS_
    ::memcpy(exifAPP3Info.identifier, jpsIdentifier, sizeof(jpsIdentifier));

#define swap16(x) (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8))
    /*
     *  length(2 bytes): the length of stereoscopic descriptor
     *                   it contains 'Length' descriptor (plus 2 bytes).
     *  this value should be written Most Significant Byte first.
     */
    MUINT16 length      = swap16((sizeof(exifAPP3Info.stereoDesc) + 2));
    ::memcpy(exifAPP3Info.length, &length, sizeof(length));
    //
    exifAPP3Info.stereoDesc[0].type[0]          = (MUINT8)SD_MTYPE_STEREOSCOPIC_IMAGE;
    exifAPP3Info.stereoDesc[0].layout[0]        = (MUINT8)SD_LAYOUT_SIDEBYSIDE;
    exifAPP3Info.stereoDesc[0].flags[0]         = (MUINT8)(SD_FULL_HEIGHT | SD_FULL_WIDTH | SD_LEFT_FIELD_FIRST);
    exifAPP3Info.stereoDesc[0].separation[0]    = (MUINT8)0x00;
    //
    MUINT16 cmtSize    = swap16(sizeof(exifAPP3Info.cmt));
    ::memcpy(exifAPP3Info.cmt.size, &cmtSize, sizeof(cmtSize));
    //
    if ( bIsN3dEnable )
    {
        const char jpsComments[] = { 0x5F, 0x4D, 0x54, 0x4B, 0x5F, 0x41, 0x43, 0x5F }; // _MTK_AC_
        ::memcpy(exifAPP3Info.cmt.comment, jpsComments, sizeof(jpsComments));
    }
    //
    err = mpBaseExif->exifAppnMake(3, pDst, (unsigned char*) &exifAPP3Info, jpsSize, &app3ReturnSize);
    if  ( err != 0)
    {
        MY_LOGE(" exifAppnMake - err(%x)", err);
        goto lbExit;
    }
    //
    pDst += app3ReturnSize;
    u4App3Size += app3ReturnSize;
    //
    if  (pu4App3Size)
    {
        *pu4App3Size = u4App3Size;
    }
    //
lbExit:
    return (0==err);
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapCapTypeIdx(MUINT32 const u4CapType)
{
    MUINT32 eCapTypeId = eCapTypeId_Standard;
    switch  (u4CapType)
    {
    case SCENE_MODE_OFF:
    case SCENE_MODE_NORMAL:
    case SCENE_MODE_NIGHTPORTRAIT:
    case SCENE_MODE_THEATRE:
    case SCENE_MODE_BEACH:
    case SCENE_MODE_SNOW:
    case SCENE_MODE_SUNSET:
    case SCENE_MODE_STEADYPHOTO:
    case SCENE_MODE_FIREWORKS:
    case SCENE_MODE_SPORTS:
    case SCENE_MODE_PARTY:
    case SCENE_MODE_CANDLELIGHT:
        eCapTypeId = eCapTypeId_Standard;
        break;
    case SCENE_MODE_PORTRAIT:
        eCapTypeId = eCapTypeId_Portrait;
        break;
    case SCENE_MODE_LANDSCAPE:
        eCapTypeId = eCapTypeId_Landscape;
        break;
    case SCENE_MODE_NIGHTSCENE:
        eCapTypeId = eCapTypeId_Night;
        break;
    default:
        eCapTypeId = eCapTypeId_Standard;
        break;
    }
    return  eCapTypeId;

}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapExpProgramIdx(MUINT32 const u4SceneMode)
{
    using namespace NSCamCustom;
    MUINT32 eExpProgramId   = eExpProgramId_NotDefined;
    customExif_t customExif;
    switch  (u4SceneMode)
    {
        case SCENE_MODE_PORTRAIT:
            eExpProgramId = eExpProgramId_Portrait;
            break;
        case SCENE_MODE_LANDSCAPE:
            eExpProgramId = eExpProgramId_Landscape;
            break;
        default:
        {
            customExif = getCustomExif();
            if ( customExif.bEnCustom )
            {
                MY_LOGD(" bEnCustom(%d), u4ExpProgram(%d)", customExif.bEnCustom, customExif.u4ExpProgram);
                switch (customExif.u4ExpProgram)
                {
                case eExpProgramId_Manual:
                    eExpProgramId = eExpProgramId_Manual;
                    break;
                case eExpProgramId_Normal:
                    eExpProgramId = eExpProgramId_Normal;
                    break;
                case eExpProgramId_NotDefined:
                default:
                    eExpProgramId = eExpProgramId_NotDefined;
                    break;
                }
            }
            else
            {
                eExpProgramId = eExpProgramId_NotDefined;
            }
        }
        break;
    }
    return  eExpProgramId;

}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapLightSourceIdx(MUINT32 const u4AwbMode)
{
    MUINT32 eLightSourceId = eLightSourceId_Other;
    switch  (u4AwbMode)
    {
    case AWB_MODE_AUTO:
    case AWB_MODE_WARM_FLUORESCENT:
    case AWB_MODE_TWILIGHT:
    case AWB_MODE_INCANDESCENT:
        eLightSourceId = eLightSourceId_Other;
        break;
    case AWB_MODE_DAYLIGHT:
        eLightSourceId = eLightSourceId_Daylight;
        break;
    case AWB_MODE_FLUORESCENT:
        eLightSourceId = eLightSourceId_Fluorescent;
        break;
    case AWB_MODE_TUNGSTEN:
        eLightSourceId = eLightSourceId_Tungsten;
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT:
        eLightSourceId = eLightSourceId_Cloudy;
        break;
    case AWB_MODE_SHADE:
        eLightSourceId = eLightSourceId_Shade;
        break;
    default:
        eLightSourceId = eLightSourceId_Other;
        break;
    }
    return  eLightSourceId;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapMeteringModeIdx(MUINT32 const u4AeMeterMode)
{
    MUINT32 eMeteringModeId = eMeteringMode_Other;
    switch  (u4AeMeterMode)
    {
    case AE_METERING_MODE_AVERAGE:
        eMeteringModeId = eMeteringMode_Average;
        break;
    case AE_METERING_MODE_CENTER_WEIGHT:
        eMeteringModeId = eMeteringMode_Center;
        break;
    case AE_METERING_MODE_SOPT:
        eMeteringModeId = eMeteringMode_Spot;
        break;
    default:
        eMeteringModeId = eMeteringMode_Other;
        break;
    }
    return  eMeteringModeId;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
queryExifApp1Info(exifAPP1Info_s*const pexifApp1Info)
{
    MY_LOGI("E - this:%p", this);

    if  ( ! pexifApp1Info )
    {
        MY_LOGE(" NULL pexifApp1Info");
        return  MFALSE;
    }

    time_t t;
    struct tm tm;

    /*********************************************************************************
                                           GPS
    **********************************************************************************/
    if  (mCamExifParam.u4GpsIsOn == 1)
    {
        float latitude = atof((char*)mCamExifParam.uGPSLatitude);
        float longitude = atof((char*)mCamExifParam.uGPSLongitude);
        long long timestamp = atol((char*)mCamExifParam.uGPSTimeStamp);
        char const*pgpsProcessingMethod = (char*)mCamExifParam.uGPSProcessingMethod;
        //
        // Set GPS Info
        if (latitude >= 0) {
            strcpy((char *) pexifApp1Info->gpsLatitudeRef, "N");
        }
        else {
            strcpy((char *) pexifApp1Info->gpsLatitudeRef, "S");
            latitude *= -1;     // make it positive
        }
        if (longitude >= 0) {
            strcpy((char *) pexifApp1Info->gpsLongitudeRef, "E");
        }
        else {
            strcpy((char *) pexifApp1Info->gpsLongitudeRef, "W");
            longitude *= -1;    // make it positive
        }
        pexifApp1Info->gpsIsOn = 1;
        // Altitude
        pexifApp1Info->gpsAltitude[0] = mCamExifParam.u4GPSAltitude;
        pexifApp1Info->gpsAltitude[1] = 1;
        // Latitude
        pexifApp1Info->gpsLatitude[0] = (int) latitude;
        pexifApp1Info->gpsLatitude[1] = 1;
        latitude -= pexifApp1Info->gpsLatitude[0];
        latitude *= 60;
        pexifApp1Info->gpsLatitude[2] = (int) latitude;
        pexifApp1Info->gpsLatitude[3] = 1;
        latitude -= pexifApp1Info->gpsLatitude[2];
        latitude *= 60;
        latitude *= 10000;
        pexifApp1Info->gpsLatitude[4] = (int) latitude;
        pexifApp1Info->gpsLatitude[5] = 10000;
        // Longtitude
        pexifApp1Info->gpsLongitude[0] = (int) longitude;
        pexifApp1Info->gpsLongitude[1] = 1;
        longitude -= pexifApp1Info->gpsLongitude[0];
        longitude *= 60;
        pexifApp1Info->gpsLongitude[2] = (int) longitude;
        pexifApp1Info->gpsLongitude[3] = 1;
        longitude -= pexifApp1Info->gpsLongitude[2];
        longitude *= 60;
        longitude *= 10000;
        pexifApp1Info->gpsLongitude[4] = (int) longitude;
        pexifApp1Info->gpsLongitude[5] = 10000;

        // Timestamp
        time_t tim = (time_t) timestamp;
        struct tm *ptime = gmtime(&tim);
        pexifApp1Info->gpsTimeStamp[0] = ptime->tm_hour;
        pexifApp1Info->gpsTimeStamp[1] = 1;
        pexifApp1Info->gpsTimeStamp[2] = ptime->tm_min;
        pexifApp1Info->gpsTimeStamp[3] = 1;
        pexifApp1Info->gpsTimeStamp[4] = ptime->tm_sec;
        pexifApp1Info->gpsTimeStamp[5] = 1;
        sprintf((char *) pexifApp1Info->gpsDateStamp, "%04d:%02d:%02d", ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday);
        // ProcessingMethod
        const char exifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 }; // ASCII
        int len1, len2, maxLen;
        len1 = sizeof(exifAsciiPrefix);
        memcpy(pexifApp1Info->gpsProcessingMethod, exifAsciiPrefix, len1);
        maxLen = sizeof(pexifApp1Info->gpsProcessingMethod) - len1;
        len2 = strlen(pgpsProcessingMethod);
        if (len2 > maxLen) {
            len2 = maxLen;
        }
        memcpy(&pexifApp1Info->gpsProcessingMethod[len1], pgpsProcessingMethod, len2);
    }

    /*********************************************************************************
                                           common
    **********************************************************************************/
    //software information
    memset(pexifApp1Info->strSoftware,0,32);
    strcpy((char *)pexifApp1Info->strSoftware, "MediaTek Camera Application");
    //get datetime
    tzset();
    time(&t);
    localtime_r(&t, &tm);
    strftime((char *)pexifApp1Info->strDateTime, 20, "%Y:%m:%d %H:%M:%S", &tm);

    /*********************************************************************************
                                       from CamExifParam
    **********************************************************************************/
    // [MFB: continuous shot image index, group ID]
    // [best shot: focus value(H+L)]
    pexifApp1Info->imgIndex     = mCamExifParam.u4ImgIndex;
    pexifApp1Info->groupID      = mCamExifParam.u4GroupId;
    pexifApp1Info->bestFocusH   = mCamExifParam.u4FocusH;
    pexifApp1Info->bestFocusL   = mCamExifParam.u4FocusL;


    // [digital zoom ratio]
    pexifApp1Info->digitalZoomRatio[0] = mCamExifParam.u4ZoomRatio;
    pexifApp1Info->digitalZoomRatio[1] = 100;
    // [orientation]
    pexifApp1Info->orientation = determineExifOrientation(
        mCamExifParam.u4Orientation, 
        mCamExifParam.u4Facing    //  1: front device
    );
    // [flashPixVer]
    memcpy(pexifApp1Info->strFlashPixVer,"0100 ",5);
    // [exposure mode]
    pexifApp1Info->exposureMode = 0;
    //

    /*********************************************************************************
                                           from 3A
    **********************************************************************************/
    if (mp3AEXIFInfo != NULL) {
        //
        // [f number]
        pexifApp1Info->fnumber[0] = mp3AEXIFInfo->u4FNumber;
        pexifApp1Info->fnumber[1] = 10;

        // [focal length]
        pexifApp1Info->focalLength[0] = mp3AEXIFInfo->u4FocalLength;
        pexifApp1Info->focalLength[1] = 100;

        // [iso speed]
        if (mp3AEXIFInfo->u4AEISOSpeed == AE_ISO_AUTO) {
            pexifApp1Info->isoSpeedRatings = (unsigned short)mp3AEXIFInfo->u4RealISOValue;
        }
        else {
            pexifApp1Info->isoSpeedRatings = (unsigned short)mp3AEXIFInfo->u4AEISOSpeed;
        }

        // [exposure time]
        if(mp3AEXIFInfo->u4CapExposureTime == 0){
            //YUV sensor
            pexifApp1Info->exposureTime[0] = 0;
            pexifApp1Info->exposureTime[1] = 0;
        }
        else{
            //RAW sensor
            if (mp3AEXIFInfo->u4CapExposureTime > 1000000) { //1 sec
                pexifApp1Info->exposureTime[0] = mp3AEXIFInfo->u4CapExposureTime/100000;
                pexifApp1Info->exposureTime[1] = 10;
            }
            else{ // us
                pexifApp1Info->exposureTime[0] = mp3AEXIFInfo->u4CapExposureTime;
                pexifApp1Info->exposureTime[1] = 1000000;
            }
        }

        // [flashlight]
        pexifApp1Info->flash = (0 != mp3AEXIFInfo->u4FlashLightTimeus)?1:0;

        // [white balance mode]
        pexifApp1Info->whiteBalanceMode = (AWB_MODE_AUTO != mp3AEXIFInfo->u4AWBMode)?1:0;

        // [light source]
        MUINT32 u4AwbMode = mp3AEXIFInfo->u4AWBMode;
        pexifApp1Info->lightSource = mapLightSourceIdx(u4AwbMode);

        // [metering mode]
        MUINT32 u4AeMeterMode = mp3AEXIFInfo->u4AEMeterMode;
        pexifApp1Info->meteringMode = mapMeteringModeIdx(u4AeMeterMode);

        // [exposure program] , [scene mode]
        MUINT32 u4SceneMode = mp3AEXIFInfo->u4SceneMode;
        if  ( SCENE_MODE_NUM <= u4SceneMode )
        {
            u4SceneMode = SCENE_MODE_OFF;
        }
        pexifApp1Info->exposureProgram  = mapExpProgramIdx(u4SceneMode);
        pexifApp1Info->sceneCaptureType = mapCapTypeIdx(u4SceneMode);

        // [Ev offset]
        pexifApp1Info->exposureBiasValue[0] = mp3AEXIFInfo->i4AEExpBias;
        pexifApp1Info->exposureBiasValue[1] = 10;

    } 
    else{
        MY_LOGE(" mp3AEXIFInfo == NULL");
    }

    /*********************************************************************************
                                           update customized exif
    **********************************************************************************/
    {
        using namespace NSCamCustom;
        customExifInfo_t* pCustomExifInfo = NULL;
        if  ( 0 == custom_SetExif((void **)&pCustomExifInfo) )
        {
            if  ( pCustomExifInfo )
            {
                // [Make]
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strMake) )
                {
                    ::strcpy((char*)pexifApp1Info->strMake, (const char*)pCustomExifInfo->strMake);
                }

                // [Model]
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strModel) )
                {
                    ::strcpy((char*)pexifApp1Info->strModel, (const char*)pCustomExifInfo->strModel);
                }

                // [software]
                ::memset(pexifApp1Info->strSoftware,0,32);
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strSoftware) )
                {
                    ::strcpy((char*)pexifApp1Info->strSoftware, (const char*)pCustomExifInfo->strSoftware);
                }
            }
        }
        else
        {
            MY_LOGW(" do not update customized exif");
        }
    }
    MY_LOGI("- X");

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
set3AEXIFInfo(EXIF_INFO_T* p3AEXIFInfo)
{
    MINT32  ret = MTRUE;

    if ( NULL != mp3AEXIFInfo )
    {
        memcpy(mp3AEXIFInfo, p3AEXIFInfo, sizeof(EXIF_INFO_T));

        MY_LOGD("=========== 3AEXIFInfo ===============\n");
        MY_LOGD("FNumber: %d/10,    FocalLength: %d/100,    Scene mode: %d, AWB mode: %d,   flash: %d \n",
            mp3AEXIFInfo->u4FNumber, mp3AEXIFInfo->u4FocalLength, mp3AEXIFInfo->u4SceneMode, mp3AEXIFInfo->u4AWBMode, mp3AEXIFInfo->u4FlashLightTimeus);
        MY_LOGD("AE meter mode: %d, AE ExpBias: %d, AE ExpTime: %d, ISO speed: %d,  Real ISO value: %d\n", 
            mp3AEXIFInfo->u4AEMeterMode, mp3AEXIFInfo->i4AEExpBias, mp3AEXIFInfo->u4CapExposureTime,
            mp3AEXIFInfo->u4AEISOSpeed, mp3AEXIFInfo->u4RealISOValue);
        MY_LOGD("======================================\n");
    }
    else
    {
        MY_LOGE(" mp3AEXIFInfo == NULL");
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
CamExif::
determineExifOrientation(
    MUINT32 const   u4DeviceOrientation, 
    MBOOL const     bIsFacing, 
    MBOOL const     bIsFacingFlip /*= MFALSE*/
)
{
    MINT32  result = -1;

    if  ( bIsFacing && bIsFacingFlip )
    {
        //  Front Camera with Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 8;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 6;
            break;
        default:
            result = 1;
            break;
        }
    }
    else
    {   //  Rear Camera or Front Camera without Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 6;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 8;
            break;
        default:
            result = 1;
            break;
        }
    }

    MY_LOGD(
        "[determineExifOrientation] - "
        "(u4DeviceOrientation, bIsFacing, bIsFacingFlip, result)=(%d, %d, %d, %d)"
        , u4DeviceOrientation, bIsFacing, bIsFacingFlip, result
    );

    return  result;
}

