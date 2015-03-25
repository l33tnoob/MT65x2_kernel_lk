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
//
#include <mtkcam/common.h>

//
#include <vector>
using namespace std;

//
#include <mtkcam/campipe/_scenario.h>
#include <mtkcam/campipe/_identity.h>
#include <mtkcam/common/hw/hwstddef.h>


// imageio/ispio, for mapper 
#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/imageio/ispio_stddef.h>
//
#include <kd_imgsensor_define.h>

namespace NSCamPipe {
/*******************************************************************************
* 
********************************************************************************/
NSImageio::NSIspio::EScenarioID 
mapScenarioID(NSCamPipe::ESWScenarioID const eSWScenarioID, NSCamPipe::EPipeID ePipeID)
{   
   if (NSCamPipe::ePipeID_1x2_Sensor_Tg_Isp_Mem == ePipeID) //camio 
   { 
       switch (eSWScenarioID) 
       {
            case NSCamPipe::eSWScenarioID_CAPTURE_NORMAL:       // Normal capture   
            case NSCamPipe::eSWScenarioID_CAPTURE_ZSD:          // ZSD capture 
                return NSImageio::NSIspio::eScenarioID_ZSD; 
            break; 
            case NSCamPipe::eSWScenarioID_MTK_PREVIEW:           // MTK preview 
            case NSCamPipe::eSWScenarioID_DEFAULT_PREVIEW:
            case NSCamPipe::eSWScenarioID_VIDEO:
            case NSCamPipe::eSWScenarioID_VSS:
            default:
                return NSImageio::NSIspio::eScenarioID_VSS;
            break; 
       }    
   }
   else if (NSCamPipe::ePipeID_1x2_Mem_Isp_Xdp_Mem == ePipeID)  //postproc
   {
       switch (eSWScenarioID) 
       {    //
            case NSCamPipe::eSWScenarioID_CAPTURE_NORMAL:       // Normal capture   
            case NSCamPipe::eSWScenarioID_CAPTURE_ZSD:          // ZSD capture 
                return NSImageio::NSIspio::eScenarioID_IP; 
            break; 
            case NSCamPipe::eSWScenarioID_MTK_PREVIEW:          // MTK preview 
            case NSCamPipe::eSWScenarioID_DEFAULT_PREVIEW:
            case NSCamPipe::eSWScenarioID_VIDEO:
            case NSCamPipe::eSWScenarioID_VSS:
            default:
                return NSImageio::NSIspio::eScenarioID_VSS;
            break; 
       } 
   }
   else if (NSCamPipe::ePipeID_1x2_Mem_Xdp_Mem == ePipeID)   // Xdp 
   {
       switch (eSWScenarioID) 
       {
            case NSCamPipe::eSWScenarioID_CAPTURE_NORMAL:       // Normal capture   
            case NSCamPipe::eSWScenarioID_CAPTURE_ZSD:          // ZSD capture 
                return NSImageio::NSIspio::eScenarioID_ZSD; 
            break; 
            case NSCamPipe::eSWScenarioID_PLAYBACK:
                return NSImageio::NSIspio::eScenarioID_IP;
            break; 
            case NSCamPipe::eSWScenarioID_MTK_PREVIEW:           // MTK preview 
            case NSCamPipe::eSWScenarioID_DEFAULT_PREVIEW:
            case NSCamPipe::eSWScenarioID_VIDEO:
            case NSCamPipe::eSWScenarioID_VSS:
            default:
                return NSImageio::NSIspio::eScenarioID_VSS;
            break; 
       }         
   }
   return NSImageio::NSIspio::eScenarioID_VSS; 
}

/*******************************************************************************
* 
********************************************************************************/
NSImageio::NSIspio::EScenarioFmt 
mapScenarioFmt(NSCamPipe::EScenarioFmt const eScenioFmt)
{
    switch (eScenioFmt) 
    {
        case NSCamPipe::eScenarioFmt_RAW:
            return NSImageio::NSIspio::eScenarioFmt_RAW; 
        break; 
        case NSCamPipe::eScenarioFmt_YUV:
            return NSImageio::NSIspio::eScenarioFmt_YUV;
        break; 
        case NSCamPipe::eScenarioFmt_RGB:
            return NSImageio::NSIspio::eScenarioFmt_RGB;
        break; 
        case NSCamPipe::eScenarioFmt_JPG:
            return NSImageio::NSIspio::eScenarioFmt_JPG;
        break;      
        default:
            return NSImageio::NSIspio::eScenarioFmt_UNKNOWN;
        break; 
    }
    
}



/*******************************************************************************
* 
********************************************************************************/
NSImageio::NSIspio::ERawPxlID 
mapRawPixelID(MUINT32 const u4PixelID)
{
    switch(u4PixelID) 
    {
        case 0:
            return NSImageio::NSIspio::ERawPxlID_B; 
        break; 
        case 1:
            return NSImageio::NSIspio::ERawPxlID_Gb;
        break; 
        case 2: 
            return NSImageio::NSIspio::ERawPxlID_Gr;
        break; 
        case 3:
            return NSImageio::NSIspio::ERawPxlID_R;        
        break; 
        default:
            return NSImageio::NSIspio::ERawPxlID_Gb;
        break; 
    }


}

/*******************************************************************************
* 
********************************************************************************/
void mapBufInfo(NSCamHW::BufInfo &rCamPipeBufInfo, NSImageio::NSIspio::BufInfo const &rBufInfo)
{    
    rCamPipeBufInfo.u4BufSize = rBufInfo.u4BufSize;
    rCamPipeBufInfo.u4BufVA = rBufInfo.u4BufVA;
    rCamPipeBufInfo.u4BufPA = rBufInfo.u4BufPA;
    rCamPipeBufInfo.i4MemID = rBufInfo.memID;

#warning [TODO] i4TimeStamp_us, i4TimeStamp_sec
}



/*******************************************************************************
* 
********************************************************************************/
EImageFormat 
mapRawFormat(MUINT32 u4BitDepth)
{
    switch (u4BitDepth)
    {
        case 8:
            return eImgFmt_BAYER8; 
        break; 
        case 10:
            return eImgFmt_BAYER10; 
        break; 
        case 12:
            return eImgFmt_BAYER12; 
        break; 
        default:
            return eImgFmt_BAYER8; 
            break; 
    }
}

/*******************************************************************************
* 
********************************************************************************/
EImageFormat
mapYUVFormat(MUINT32 u4ColorOrder)
{
    switch (u4ColorOrder)
    {
        case SENSOR_OUTPUT_FORMAT_UYVY:
        case SENSOR_OUTPUT_FORMAT_CbYCrY:
            return eImgFmt_UYVY; 
        break; 
        case SENSOR_OUTPUT_FORMAT_VYUY:
        case SENSOR_OUTPUT_FORMAT_CrYCbY:
            return eImgFmt_VYUY; 
        break; 
        case SENSOR_OUTPUT_FORMAT_YUYV:
        case SENSOR_OUTPUT_FORMAT_YCbYCr:
            return eImgFmt_YUY2; 
        break; 
        case SENSOR_OUTPUT_FORMAT_YVYU:
        case SENSOR_OUTPUT_FORMAT_YCrYCb:
            return eImgFmt_YVYU; 
        break; 
        default:
            return eImgFmt_YUY2; 
        break; 
    }
}



}; 
