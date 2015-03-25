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
#include "camera_AE_PLineTable_s5k3h7ymipiraw.h"
#include "camera_info_s5k3h7ymipiraw.h"
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
        64425,    // i4R_AVG
        14068,    // i4R_STD
        108050,    // i4B_AVG
        29237,    // i4B_STD
        {  // i4P00[9]
            4837500, -2037500, -240000, -700000, 3172500, 90000, 172500, -2175000, 4565000
        },
        {  // i4P10[9]
            1023485, -953056, -70429, -137796, 151385, -6774, -31170, 281707, -243721
        },
        {  // i4P01[9]
            443399, -436070, -7329, -112167, -124718, 243035, -10651, -300356, 317156
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
            52,                // u4AETarget
            47,                // u4StrobeAETarget

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
            60,    // u4PreviewFlareOffset
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
                772,    // i4R
                512,    // i4G
                662    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                77,    // i4X
                -255    // i4Y
            },
            // Horizon
            {
                -537,    // i4X
                -324    // i4Y
            },
            // A
            {
                -389,    // i4X
                -323    // i4Y
            },
            // TL84
            {
                -226,    // i4X
                -347    // i4Y
            },
            // CWF
            {
                -164,    // i4X
                -457    // i4Y
            },
            // DNP
            {
                -114,    // i4X
                -271    // i4Y
            },
            // D65
            {
                57,    // i4X
                -246    // i4Y
            },
            // DF
            {
                -34,    // i4X
                -374    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                32,    // i4X
                -264    // i4Y
            },
            // Horizon
            {
                -584,    // i4X
                -227    // i4Y
            },
            // A
            {
                -438,    // i4X
                -251    // i4Y
            },
            // TL84
            {
                -282,    // i4X
                -303    // i4Y
            },
            // CWF
            {
                -240,    // i4X
                -422    // i4Y
            },
            // DNP
            {
                -159,    // i4X
                -247    // i4Y
            },
            // D65
            {
                14,    // i4X
                -252    // i4Y
            },
            // DF
            {
                -98,    // i4X
                -362    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                802,    // i4R
                512,    // i4G
                652    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                683,    // i4G
                2193    // i4B
            },
            // A 
            {
                512,    // i4R
                560,    // i4G
                1467    // i4B
            },
            // TL84 
            {
                603,    // i4R
                512,    // i4G
                1112    // i4B
            },
            // CWF 
            {
                761,    // i4R
                512,    // i4G
                1186    // i4B
            },
            // DNP 
            {
                633,    // i4R
                512,    // i4G
                862    // i4B
            },
            // D65 
            {
                772,    // i4R
                512,    // i4G
                662    // i4B
            },
            // DF 
            {
                810,    // i4R
                512,    // i4G
                889    // i4B
            }
        },
        // Rotation matrix parameter
        {
            10,    // i4RotationAngle
            252,    // i4Cos
            44    // i4Sin
        },
        // Daylight locus parameter
        {
            -179,    // i4SlopeNumerator
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
            -332,    // i4RightBound
            -982,    // i4LeftBound
            -150,    // i4UpperBound
            -240    // i4LowerBound
            },
            // Warm fluorescent
            {
            -332,    // i4RightBound
            -982,    // i4LeftBound
            -240,    // i4UpperBound
            -398    // i4LowerBound
            },
            // Fluorescent
            {
            -209,    // i4RightBound
            -332,    // i4LeftBound
            -180,    // i4UpperBound
            -362    // i4LowerBound
            },
            // CWF
            {
            -209,    // i4RightBound
            -332,    // i4LeftBound
            -362,    // i4UpperBound
            -472    // i4LowerBound
            },
            // Daylight
            {
            39,    // i4RightBound
            -209,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Shade
            {
            399,    // i4RightBound
            39,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            39,    // i4RightBound
            -209,    // i4LeftBound
            -332,    // i4UpperBound
            -420    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            399,    // i4RightBound
            -982,    // i4LeftBound
            0,    // i4UpperBound
            -472    // i4LowerBound
            },
            // Daylight
            {
            64,    // i4RightBound
            -209,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Cloudy daylight
            {
            164,    // i4RightBound
            -11,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Shade
            {
            264,    // i4RightBound
            -11,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Twilight
            {
            -209,    // i4RightBound
            -369,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
            },
            // Fluorescent
            {
            64,    // i4RightBound
            -382,    // i4LeftBound
            -202,    // i4UpperBound
            -472    // i4LowerBound
            },
            // Warm fluorescent
            {
            -338,    // i4RightBound
            -538,    // i4LeftBound
            -202,    // i4UpperBound
            -472    // i4LowerBound
            },
            // Incandescent
            {
            -338,    // i4RightBound
            -538,    // i4LeftBound
            -172,    // i4UpperBound
            -332    // i4LowerBound
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
            702,    // i4R
            512,    // i4G
            757    // i4B
            },
            // Cloudy daylight
            {
            827,    // i4R
            512,    // i4G
            599    // i4B
            },
            // Shade
            {
            874,    // i4R
            512,    // i4G
            554    // i4B
            },
            // Twilight
            {
            553,    // i4R
            512,    // i4G
            1063    // i4B
            },
            // Fluorescent
            {
            729,    // i4R
            512,    // i4G
            952    // i4B
            },
            // Warm fluorescent
            {
            536,    // i4R
            512,    // i4G
            1475    // i4B
            },
            // Incandescent
            {
            469,    // i4R
            512,    // i4G
            1343    // i4B
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
            7226    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5784    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            346    // i4OffsetThr
            },
            // Daylight WB gain
            {
            638,    // i4R
            512,    // i4G
            867    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            476,    // i4R
            512,    // i4G
            532    // i4B
            },
            // Preference gain: warm fluorescent
            {
            460,    // i4R
            512,    // i4G
            545    // i4B
            },
            // Preference gain: fluorescent
            {
            518,    // i4R
            512,    // i4G
            522    // i4B
            },
            // Preference gain: CWF
            {
            542,    // i4R
            512,    // i4G
            556    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            508    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            518,    // i4R
            512,    // i4G
            508    // i4B
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
                -598,    // i4RotatedXCoordinate[0]
                -452,    // i4RotatedXCoordinate[1]
                -296,    // i4RotatedXCoordinate[2]
                -173,    // i4RotatedXCoordinate[3]
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

    XLOGD("HJDDbgIQ, Darling3h7, impGetDefaultData, CameraDataType=%d\n", CameraDataType);
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


