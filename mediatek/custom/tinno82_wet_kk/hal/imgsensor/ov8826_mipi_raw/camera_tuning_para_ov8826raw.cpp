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
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8826raw.h"
#include "camera_info_ov8826raw.h"
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
        76300,    // i4R_AVG
        16384,    // i4R_STD
        87720,    // i4B_AVG
        22519,    // i4B_STD
        {  // i4P00[9]
            4130000, -1286000, -286000, -512000, 2968000, 104000, 158000, -1466000, 3866000
        },
        {  // i4P10[9]
            586695, -648589, 56788, -83328, -8377, 91705, -27627, 269616, -238394
        },
        {  // i4P01[9]
            254069, -335079, 75523, -147971, -77253, 225224, -54191, -167606, 228003
        },
        { // i4P20[9]
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
            1144,   // u4MinGain, 1024 base =  1x
            10240,  // u4MaxGain, 16x
            100,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            26,    // u4PreExpUnit 
            30,     // u4PreMaxFrameRate
            19,    // u4VideoExpUnit  
            30,     // u4VideoMaxFrameRate
            512,   // u4Video2PreRatio, 1024 base = 1x
            28,    // u4CapExpUnit 
            15,    // u4CapMaxFrameRate
            512,   // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
         },
         // rHistConfig
        {
            2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {86, 108, 128, 148, 170},    // 160 u4HistStretchThres[AE_CCT_STRENGTH_NUM] //170 180x
            {18, 22, 26, 30, 34}       //34 39// u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            FALSE,            // bEnableCaptureThres
            FALSE,            // bEnableVideoThres
            FALSE,            // bEnableStrobeThres
            47,                // u4AETarget
            0,    // u4StrobeAETarget

            50,                // u4InitIndex
			4, 				// 4 32 u4BackLightWeight
			32, 			   // 32 u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -5,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset 
            5,                 // u4VideoFlareThres
            64,                 // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            160,    // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            160,    // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            66                 // u4FlatnessStrength
         } 
    },

    // AWB NVRAM
    {
    	// AWB calibration data
    	{
    		// rUnitGain (unit gain: 1.0 = 512)
    		{
    			0,	// i4R
    			0,	// i4G
    			0	// i4B
    		},
    		// rGoldenGain (golden sample gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
    		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                862,    // i4R
                512,    // i4G
                573    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                64,    // i4X
                -399    // i4Y
            },
            // Horizon
            {
                -388,    // i4X
                -332    // i4Y
            },
            // A
            {
                -267,    // i4X
                -325    // i4Y
            },
            // TL84
            {
                -136,    // i4X
                -320    // i4Y
            },
            // CWF
            {
                -78,    // i4X
                -416    // i4Y
            },
            // DNP
            {
                7,    // i4X
                -281    // i4Y
            },
            // D65
            {
                151,    // i4X
                -234    // i4Y
            },
            // DF
            {
                107,    // i4X
                -339    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                -20,    // i4X
                -403    // i4Y
            },
            // Horizon
            {
                -448,    // i4X
                -244    // i4Y
            },
            // A
            {
                -328,    // i4X
                -262    // i4Y
            },
            // TL84
            {
                -199,    // i4X
                -284    // i4Y
            },
            // CWF
            {
                -162,    // i4X
                -390    // i4Y
            },
            // DNP
            {
                -51,    // i4X
                -276    // i4Y
            },
            // D65
            {
                99,    // i4X
                -260    // i4Y
            },
            // DF
            {
                34,    // i4X
                -353    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                959,    // i4R
                512,    // i4G
                805    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                552,    // i4G
                1465    // i4B
            },
            // A 
            {
                554,    // i4R
                512,    // i4G
                1142    // i4B
            },
            // TL84 
            {
                657,    // i4R
                512,    // i4G
                950    // i4B
            },
            // CWF 
            {
                809,    // i4R
                512,    // i4G
                1000    // i4B
            },
            // DNP 
            {
                756,    // i4R
                512,    // i4G
                741    // i4B
            },
            // D65 
            {
                862,    // i4R
                512,    // i4G
                573    // i4B
            },
            // DF 
            {
                936,    // i4R
                512,    // i4G
                701    // i4B
            }
        },
        // Rotation matrix parameter
        {
            12,    // i4RotationAngle
            250,    // i4Cos
            53    // i4Sin
        },
        // Daylight locus parameter
        {
            -196,    // i4SlopeNumerator
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
            -249,    // i4RightBound
            -899,    // i4LeftBound
            -203,    // i4UpperBound
            -303    // i4LowerBound
            },
            // Warm fluorescent
            {
            -249,    // i4RightBound
            -899,    // i4LeftBound
            -303,    // i4UpperBound
            -423    // i4LowerBound
            },
            // Fluorescent
            {
            -101,    // i4RightBound
            -249,    // i4LeftBound
            -191,    // i4UpperBound
            -337    // i4LowerBound
            },
            // CWF
            {
            -101,    // i4RightBound
            -249,    // i4LeftBound
            -337,    // i4UpperBound
            -440    // i4LowerBound
            },
            // Daylight
            {
            124,    // i4RightBound
            -101,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Shade
            {
            484,    // i4RightBound
            124,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            124,    // i4RightBound
            -101,    // i4LeftBound
            -340,    // i4UpperBound
            -440    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            484,    // i4RightBound
            -899,    // i4LeftBound
            0,    // i4UpperBound
            -440    // i4LowerBound
            },
            // Daylight
            {
            149,    // i4RightBound
            -101,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Cloudy daylight
            {
            249,    // i4RightBound
            74,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Shade
            {
            349,    // i4RightBound
            74,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Twilight
            {
            -101,    // i4RightBound
            -261,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
            },
            // Fluorescent
            {
            149,    // i4RightBound
            -299,    // i4LeftBound
            -210,    // i4UpperBound
            -440    // i4LowerBound
            },
            // Warm fluorescent
            {
            -228,    // i4RightBound
            -428,    // i4LeftBound
            -210,    // i4UpperBound
            -440    // i4LowerBound
            },
            // Incandescent
            {
            -228,    // i4RightBound
            -428,    // i4LeftBound
            -180,    // i4UpperBound
            -340    // i4LowerBound
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
            797,    // i4R
            512,    // i4G
            646    // i4B
            },
            // Cloudy daylight
            {
            921,    // i4R
            512,    // i4G
            518    // i4B
            },
            // Shade
            {
            970,    // i4R
            512,    // i4G
            478    // i4B
            },
            // Twilight
            {
            644,    // i4R
            512,    // i4G
            899    // i4B
            },
            // Fluorescent
            {
            798,    // i4R
            512,    // i4G
            811    // i4B
            },
            // Warm fluorescent
            {
            613,    // i4R
            512,    // i4G
            1219    // i4B
            },
            // Incandescent
            {
            552,    // i4R
            512,    // i4G
            1139    // i4B
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
            6678    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5845    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            70    // i4OffsetThr
            },
            // Daylight WB gain
            {
            780,    // i4R 737
            512,    // i4G
            680    // i4B 729 700
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            500,    // i4R 490 470 482 490 
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
            516    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            508,    // i4R
            512,    // i4G
            514    // i4B 508 512 514
            },
            // Preference gain: shade
            {
            506,    // i4R 512
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            495,    // i4R 512 507 502 500
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
                -547,    // i4RotatedXCoordinate[0]
                -427,    // i4RotatedXCoordinate[1]
                -298,    // i4RotatedXCoordinate[2]
                -150,    // i4RotatedXCoordinate[3]
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
}};  //  NSFeature


