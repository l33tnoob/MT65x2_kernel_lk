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
#include <utils/Log.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_liteon3h7ymipiraw.h"
#include "camera_info_liteon3h7ymipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"

const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	}
    },
    ISPPca: {
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        68325,    // i4R_AVG
        14377,    // i4R_STD
        107600,    // i4B_AVG
        29212,    // i4B_STD
        {  // i4P00[9]
            4655000, -1867500, -230000, -647500, 3100000, 107500, 175000, -1857500, 4242500
        },
        {  // i4P10[9]
            992546, -992270, -6855, -145902, 209206, -63304, -25868, 196133, -177458
        },
        {  // i4P01[9]
            133497, -230426, 91013, -75469, -194981, 270450, 12873, -185976, 162007
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1136,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            51,    // u4MiniISOGain, ISOxx  
            256,    // u4GainStepUnit, 1x/8 
            14,    // u4PreExpUnit 
            29,    // u4PreMaxFrameRate
            14,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            14,    // u4CapExpUnit 
            24,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
         },
         // rHistConfig
        {
            4, // 2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {62, 70, 82, 128, 141},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 46}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            56,    // u4AETarget
            0,    // u4StrobeAETarget

            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            4,                 // u4BlackLightStrengthIndex
            3,    // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -8,    // i4BVOffset delta BV = value/10 
            75,    // u4PreviewFlareOffset
            60,    // u4CaptureFlareOffset
            3,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            3,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            3,                 // u4StrobeFlareThres // 0.5%
            160,                 // u4PrvMaxFlareThres //12 bit
            0,                 // u4PrvMinFlareThres
            160,                 // u4VideoMaxFlareThres // 12 bit
            0,                 // u4VideoMinFlareThres
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            75                 // u4FlatnessStrength
         }
    },

    // AWB NVRAM
    {
	// AWB calibration data
	{
		// rCalGain (calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rDefGain (Default calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rDefGain (Default calibration gain: 1.0 = 512)
		{
			0,	// u4R
			0,	// u4G
			0	// u4B
		},
		// rD65Gain (D65 WB gain: 1.0 = 512)
		{
                931,    // i4R
                512,    // i4G
                681    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                94,    // i4X
                -278    // i4Y
            },
            // Horizon
            {
                -498,    // i4X
                -357    // i4Y
            },
            // A
            {
                -349,    // i4X
                -340    // i4Y
            },
            // TL84
            {
                -195,    // i4X
                -405    // i4Y
            },
            // CWF
            {
                -169,    // i4X
                -428    // i4Y
            },
            // DNP
            {
                -20,    // i4X
                -350    // i4Y
            },
            // D65
            {
                116,    // i4X
                -326    // i4Y
            },
            // DF
            {
                -31,    // i4X
                -374    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                74,    // i4X
                -284    // i4Y
            },
            // Horizon
            {
                -521,    // i4X
                -321    // i4Y
            },
            // A
            {
                -372,    // i4X
                -314    // i4Y
            },
            // TL84
            {
                -223,    // i4X
                -390    // i4Y
            },
            // CWF
            {
                -198,    // i4X
                -414    // i4Y
            },
            // DNP
            {
                -45,    // i4X
                -347    // i4Y
            },
            // D65
            {
                93,    // i4X
                -333    // i4Y
            },
            // DF
            {
                -57,    // i4X
                -370    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                847,    // i4R
                512,    // i4G
                657    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                620,    // i4G
                1974    // i4B
            },
            // A 
            {
                512,    // i4R
                518,    // i4G
                1316    // i4B
            },
            // TL84 
            {
                680,    // i4R
                512,    // i4G
                1154    // i4B
            },
            // CWF 
            {
                727,    // i4R
                512,    // i4G
                1148    // i4B
            },
            // DNP 
            {
                801,    // i4R
                512,    // i4G
                845    // i4B
            },
            // D65 
            {
                931,    // i4R
                512,    // i4G
                681    // i4B
            },
            // DF 
            {
                814,    // i4R
                512,    // i4G
                886    // i4B
            }
        },
        // Rotation matrix parameter
        {
            4,    // i4RotationAngle
            255,    // i4Cos
            18    // i4Sin
        },
        // Daylight locus parameter
        {
            -146,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -273,    // i4RightBound
            -923,    // i4LeftBound
            -267,    // i4UpperBound
            -367    // i4LowerBound
            },
            // Warm fluorescent
            {
            -273,    // i4RightBound
            -923,    // i4LeftBound
            -367,    // i4UpperBound
            -487    // i4LowerBound
            },
            // Fluorescent
            {
            -95,    // i4RightBound
            -273,    // i4LeftBound
            -260,    // i4UpperBound
            -402    // i4LowerBound
            },
            // CWF
            {
            -95,    // i4RightBound
            -273,    // i4LeftBound
            -402,    // i4UpperBound
            -464    // i4LowerBound
            },
            // Daylight
            {
            118,    // i4RightBound
            -95,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Shade
            {
            478,    // i4RightBound
            118,    // i4LeftBound
            -253,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            118,    // i4RightBound
            -95,    // i4LeftBound
            -413,    // i4UpperBound
            -513    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            478,    // i4RightBound
            -923,    // i4LeftBound
            0,    // i4UpperBound
            -513    // i4LowerBound
            },
            // Daylight
            {
            143,    // i4RightBound
            -95,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Cloudy daylight
            {
            243,    // i4RightBound
            68,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Shade
            {
            343,    // i4RightBound
            68,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Twilight
            {
            -95,    // i4RightBound
            -255,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Fluorescent
            {
            143,    // i4RightBound
            -323,    // i4LeftBound
            -283,    // i4UpperBound
            -464    // i4LowerBound
            },
            // Warm fluorescent
            {
            -272,    // i4RightBound
            -472,    // i4LeftBound
            -283,    // i4UpperBound
            -464    // i4LowerBound
            },
            // Incandescent
            {
            -272,    // i4RightBound
            -472,    // i4LeftBound
            -213,    // i4UpperBound
            -413    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            830,    // i4R
            512,    // i4G
            733    // i4B
            },
            // Cloudy daylight
            {
            979,    // i4R
            512,    // i4G
            606    // i4B
            },
            // Shade
            {
            1043,    // i4R
            512,    // i4G
            564    // i4B
            },
            // Twilight
            {
            646,    // i4R
            512,    // i4G
            978    // i4B
            },
            // Fluorescent
            {
            785,    // i4R
            512,    // i4G
            933    // i4B
            },
            // Warm fluorescent
            {
            551,    // i4R
            512,    // i4G
            1403    // i4B
            },
            // Incandescent
            {
            505,    // i4R
            512,    // i4G
            1300    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            7253    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5999    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            341    // i4OffsetThr
            },
            // Daylight WB gain
            {
            784,    // i4R
            512,    // i4G
            831    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            500    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            500    // i4B
            },
            // Preference gain: daylight
            {
            500,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -614,    // i4RotatedXCoordinate[0]
                -465,    // i4RotatedXCoordinate[1]
                -316,    // i4RotatedXCoordinate[2]
                -138,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT)};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }
    
    XLOGD("HJDDbgIQ, LiteON3h7, impGetDefaultData, CameraDataType=%d\n", CameraDataType);
    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


