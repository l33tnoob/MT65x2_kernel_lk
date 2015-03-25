#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5647mipiraw.h"
#include "camera_info_ov5647mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
//#include "camera_custom_flicker_table.h"
//#include "camera_flicker_table_ov5647mipiraw.h"

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
        82833, // i4R_AVG
        13766, // i4R_STD
        86433, // i4B_AVG
        22206, // i4B_STD
        { // i4P00[9]
            4763333, -2106667, -96667, -206667, 2716667, 50000,  353333, -1710000, 3916667 
        },
        { // i4P10[9]
            180577,  -172726, -7851, 133470,  -133470,  0, 134887,   -141321, 6434
        },
        { // i4P01[9]
            87181,  -83390, -3790, 64438, -64438,  0, -40371,  -68228,  108600
        },
        { // i4P20[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
        },
        { // i4P11[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
        },
        { // i4P02[9]
            0,  0,   0,  0,   0,  0, 0,  0,  0
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
            106,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            34,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            30,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            35,    // u4CapExpUnit 
            14,    // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            28,      // u4LensFno, Fno = 2.8
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
            {86, 108, 128, 148, 170},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            FALSE,            // bEnableCaptureThres  //fix flare
            FALSE,            // bEnableVideoThres
            FALSE,            // bEnableStrobeThres
            47,                // u4AETarget
            47,                // u4StrobeAETarget

            20,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -13,               // i4BVOffset delta BV = -2.3
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,                 // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            8,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            8,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            55    // u4FlatnessStrength
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
                777,    // i4R
                512,    // i4G
                520    // i4B
    		}
    	},
    	// Original XY coordinate of AWB light source
    	{
		    // Strobe
		    {
                148,    // i4X
                -160    // i4Y
		    },
    		// Horizon
    		{
                -415,    // i4X
                -272    // i4Y
    		},
    		// A
    		{
                -291,    // i4X
                -267    // i4Y
    		},
    		// TL84
    		{
                -115,    // i4X
                -261    // i4Y
    		},
    		// CWF
    		{
                -67,    // i4X
                -336    // i4Y
    		},
    		// DNP
    		{
                11,    // i4X
                -200    // i4Y
    		},
    		// D65
    		{
                148,    // i4X
                -160    // i4Y
    		},
		// DF
		{
			0,	// i4X
			0	// i4Y
    		}
    	},
    	// Rotated XY coordinate of AWB light source
    	{
		    // Strobe
		    {
                108,    // i4X
                -189    // i4Y
		    },
    		// Horizon
    		{
                -465,    // i4X
                -171    // i4Y
    		},
    		// A
    		{
                -344,    // i4X
                -194    // i4Y
    		},
    		// TL84
    		{
                -171,    // i4X
                -228    // i4Y
    		},
    		// CWF
    		{
                -141,    // i4X
                -312    // i4Y
    		},
    		// DNP
    		{
                -35,    // i4X
                -197    // i4Y
    		},
    		// D65
    		{
                108,    // i4X
                -189    // i4Y
    		},
		// DF
		{
			0,	// i4X
			0	// i4Y
    		}
    	},
	// AWB gain of AWB light source
	{
		// Strobe
		{
                777,    // i4R
                512,    // i4G
                520    // i4B
		},
		// Horizon
		{
                512,    // i4R
                621,    // i4G
                1574    // i4B
		},
		// A
		{
                512,    // i4R
                529,    // i4G
                1126    // i4B
		},
		// TL84
		{
                624,    // i4R
                512,    // i4G
                852    // i4B
		},
		// CWF
		{
                737,    // i4R
                512,    // i4G
                883    // i4B
		},
		// DNP
		{
                681,    // i4R
                512,    // i4G
                661    // i4B
		},
		// D65
		{
                777,    // i4R
                512,    // i4G
                520    // i4B
		},
		// DF
		{
			512,	// i4R
			512,	// i4G
			512     // i4B
		}
	},
    	// Rotation matrix parameter
    	{
            13,    // i4RotationAngle
            249,    // i4Cos
            58    // i4Sin
    	},
    	// Daylight locus parameter
    	{
            -205,    // i4SlopeNumerator
            128    // i4SlopeDenominator
    	},
    	// AWB light area
    	{
		    // Strobe:FIXME
		    {
            133,    // i4RightBound
            -85,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
		    },
    		// Tungsten
    		{
            -221,    // i4RightBound
            -871,    // i4LeftBound
            -92,    // i4UpperBound
            -232    // i4LowerBound
    		},
    		// Warm fluorescent
    		{
            -221,    // i4RightBound
            -871,    // i4LeftBound
            -232,    // i4UpperBound
            -352    // i4LowerBound
    		},
    		// Fluorescent
    		{
            -85,    // i4RightBound
            -221,    // i4LeftBound
            -120,    // i4UpperBound
            -270    // i4LowerBound
    		},
    		// CWF
    		{
            -85,    // i4RightBound
            -221,    // i4LeftBound
            -270,    // i4UpperBound
            -362    // i4LowerBound
    		},
    		// Daylight
    		{
            133,    // i4RightBound
            -85,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
    		// Shade
    		{
            493,    // i4RightBound
            133,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
		},
		// Daylight Fluorescent
		{
            133,    // i4RightBound
            -85,    // i4LeftBound
            -269,    // i4UpperBound
            -450    // i4LowerBound
    		}
    	},
    	// PWB light area
    	{
    		// Reference area
    		{
            493,    // i4RightBound
            -871,    // i4LeftBound
            -92,    // i4UpperBound
            -450    // i4LowerBound
    		},
    		// Daylight
    		{
            158,    // i4RightBound
            -85,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
    		// Cloudy daylight
    		{
            283,    // i4RightBound
            83,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
    		// Shade
    		{
            433,    // i4RightBound
            233,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
    		// Twilight
    		{
            -85,    // i4RightBound
            -245,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
    		// Fluorescent
    		{
            158,    // i4RightBound
            -271,    // i4LeftBound
            -139,    // i4UpperBound
            -362    // i4LowerBound
    		},
    		// Warm fluorescent
    		{
            -271,    // i4RightBound
            -444,    // i4LeftBound
            -139,    // i4UpperBound
            -362    // i4LowerBound
    		},
    		// Incandescent
    		{
            -271,    // i4RightBound
            -444,    // i4LeftBound
            -109,    // i4UpperBound
            -269    // i4LowerBound
    		},
            {// Gray World
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
            723,    // i4R
            512,    // i4G
            584    // i4B
    		},
    		// Cloudy daylight
    		{
            838,    // i4R
            512,    // i4G
            460    // i4B
    		},
    		// Shade
    		{
            976,    // i4R
            512,    // i4G
            361    // i4B
    		},
    		// Twilight
    		{
            589,    // i4R
            512,    // i4G
            811    // i4B
    		},
    		// Fluorescent
    		{
            727,    // i4R
            512,    // i4G
            723    // i4B
    		},
    		// Warm fluorescent
    		{
            536,    // i4R
            512,    // i4G
            1181    // i4B
    		},
    		// Incandescent
    		{
            485,    // i4R
            512,    // i4G
            1109    // i4B
		},
		// Gray World
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
    		}
    	},
    	// AWB preference color
    	{
    		// Tungsten
    		{
            0,    // i4SliderValue
            7475    // i4OffsetThr
    		},
    		// Warm fluorescent
    		{
            0,    // i4SliderValue
            5605    // i4OffsetThr
    		},
    		// Shade
    		{
            30,    // i4SliderValue
            750    // i4OffsetThr
    		},
    		// Daylight WB gain
    		{
            672,    // i4R
            512,    // i4G
            656    // i4B
		},
		// Preference gain: strobe
		{
			512,	// i4R
			512,	// i4G
			512	// i4B
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
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: CWF
		{
			512,	// i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight
		{
            512,    // i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: shade
		{
            512,    // i4R
			512,	// i4G
			512	    // i4B
		},
		// Preference gain: daylight fluorescent
		{
            512,    // i4R
			512,	// i4G
			512	    // i4B
    		}
    	},
    	// CCT estimation
    	{
    		// CCT
    		{
			    2300,	// i4CCT[0]
    			2850,	// i4CCT[1]
    			4100,	// i4CCT[2]
    			5100,	// i4CCT[3]
    			6500	// i4CCT[4]
    		},
    		// Rotated X coordinate
    		{
                -573,    // i4RotatedXCoordinate[0]
                -452,    // i4RotatedXCoordinate[1]
                -279,    // i4RotatedXCoordinate[2]
                -143,    // i4RotatedXCoordinate[3]
    			0	// i4RotatedXCoordinate[4]
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

