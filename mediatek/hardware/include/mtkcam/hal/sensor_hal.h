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
#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_
class IBaseCamExif;
/**
* @file< Sensor_hal.h>
* @brief:This file is user space camera sensor driver provided interface definition
*
*/


/**
*@enum<ACDK_SCENARIO_ID_ENUM>
*@brief:Sensor operation scenario enum
*/
typedef enum
{
	ACDK_SCENARIO_ID_CAMERA_PREVIEW=0,		/*!<camera preview scenario*/
	ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,	/*!<camera capture scenario*/
	ACDK_SCENARIO_ID_VIDEO_PREVIEW,			/*!<video preview or record scenario*/
	ACDK_SCENARIO_ID_HIGH_SPEED_VIDEO,		/*!<high speed video or record scenario (30fps above)*/
	ACDK_SCENARIO_ID_CAMERA_ZSD,			/*!<zero shtter delay preview or capture scenario*/
	ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW,		/*!<3D camera preview scenario*/
	ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE,		/*!<3D camera capture scenario*/
	ACDK_SCENARIO_ID_CAMERA_3D_VIDEO,		/*!<3D video preview or record scenario*/
	ACDK_SCENARIO_ID_TV_OUT,				/*!<TV out scenario*/
	ACDK_SCENARIO_ID_MAX,	 				/*!<Max enum value*/
}	ACDK_SCENARIO_ID_ENUM;

/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned short MUINT16;
/*******************************************************************************
*
********************************************************************************/

/**
*@struct<halSensorIFParam_t>
*@brief:structure used in sensor hal provided setConf() api to control sensor setting
*/
typedef struct halSensorIFParam_s {
    MUINT32 u4SrcW;						/*!<sensor input source width*/
    MUINT32 u4SrcH;						/*!<sensor input source height*/ 
    MUINT32 u4CropW;					/*!<TG crop width for ISP input*/
	MUINT32 u4CropH;					/*!<TG crop height for ISP input*/
    MUINT32 u4IsContinous;				/*!<TG output to ISP coninuous frame or single frame*/
    MUINT32 u4IsBypassSensorScenario;	/*!<parameter to set bypass sensor operation scenario*/
    MUINT32 u4IsBypassSensorDelay;		/*!<parameter to set bypass sensor switch operation sceario delay frame*/
	ACDK_SCENARIO_ID_ENUM scenarioId;	/*!<operation scenario*/
} halSensorIFParam_t;
//

/**
*@struct<halSensorRawImageInfo_t>
*@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_RAW_INFO to get information from sensor
*/
typedef struct halSensorRawImageInfo_s {
    MUINT32 u4Width;			/*!<sensor output source width*/ 
    MUINT32 u4Height;			/*!<sensor output source height*/
    MUINT32 u4BitDepth; 		/*!<sensor output pixel data bit depth*/
    MUINT32 u4IsPacked; 		/*!<memory out raw data is packed or not*/
    MUINT32 u4Size;				/*!<sensor output frame data size in bytes*/
    MUINT32 u1Order;			/*!<sensor output data format , please reference ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM*/
} halSensorRawImageInfo_t; 
//

/**
*@struct<SENSOR_CROP_INFO>
*@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_SENSOR_CROPINFO to get information from sensor
*/
typedef struct {
    MUINT32 u4GrabX;	    /*!<TG crop window start x*/ 
    MUINT32 u4GrabY;        /*!<TG crop window start y*/ 
    MUINT32 u4SrcW;         /*!<sensor output source width*/ 
    MUINT32 u4SrcH;         /*!<sensor output source height*/ 
    MUINT32 u4CropW;        /*!<TG crop window width*/ 
    MUINT32 u4CropH;        /*!<TG crop window height*/ 
    MUINT32 u4SubSpW;       /*!<sensor Subsampling width*/
    MUINT32 u4SubSpH;       /*!<sensor Subsampling height*/
    MUINT32 DataFmt;	    /*!<sensor output data format*/ 
} SENSOR_CROP_INFO;


/**
*@struct<SENSOR_GRAB_INFO_STRUCT>
*@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_SENSOR_GRAB_INFO to get information from sensor
*/
typedef struct
{
    MUINT16 u4SensorGrabStartX;    /*!<sensor output data start X from its active window*/
    MUINT16 u4SensorGrabStartY;    /*!<sensor output data start Y from its active window*/
    MUINT16 u2SensorSubSpW;        /*!<sensor output subsampling data in X dir*/
    MUINT16 u2SensorSubSpH;        /*!<sensor output subsampling data in Y dir */
} SENSOR_GRAB_INFO_STRUCT;

//N3D config
/**
*@struct<SENSOR_N3D_CONFIG_STRUCT>
*@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_SET_N3D_CONFIG to set native 3D control configuration
*/
typedef struct halSensorN3dConfig_s {
    MUINT32 u4N3dEn;            /*!<N3D module enable control*/
    MUINT32 u4I2C1En;           /*!<N3D 1st I2C control enable control*/
    MUINT32 u4I2C2En;           /*!<N3D 2nd I2C control enable control*/
    MUINT32 u4N3dMode;          /*!<N3D I2C control mode*/
    MUINT32 u4DiffCntEn;        /*!<N3D sensor vsync difference count enable control */
    MUINT32 u4DiffCntThr;       /*!<N3D sensor vsync difference threshold */
} halSensorN3dConfig_t; 

//N3D different count
/**
*@struct<SENSOR_N3D_DIFFERENCE_COUNT_STRUCT>
*@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT to get native 3D vsync difference information
*/
typedef struct halSensorN3dDiffCnt_s {
    MUINT32 u4DiffCnt;          /*!<vsync difference count between vysnc1 & vsync2*/
    MUINT32 u4Vsync1Cnt;        /*!<*vsync1 valid count*/
    MUINT32 u4Vsync2Cnt;        /*!<vsync2 valid count*/
    MUINT32 u4DebugPort;        /*!<vsync2 count number sample at vysnc1*/
    MUINT32 u4Data[10];         /*!<reserve*/
} halSensorN3dDiffCnt_t; 

/**
*@enum<halSensorCmd_e>
*@brief:Sensor hal provide sendCommand() defined command id enum definition
*/
typedef enum halSensorCmd_s {
    SENSOR_CMD_SET_SENSOR_DEV          		= 0x1000,	/*!<Command to set camera operation device. arg1:N/A, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_SENSOR_EXP_TIME,						/*!<Command to set sensor exposure time in unit us. arg1:[input]exp time, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_SENSOR_EXP_LINE,						/*!<Command to set sensor exposure line. arg1:[input]exp line, arg2:N/A, arg3:N/A*/ 
	SENSOR_CMD_SET_SENSOR_GAIN,							/*!<Command to set sensor gain. arg1:[input]gain, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_FLICKER_FRAME_RATE,					/*!<Command to set sensor gain. arg1:[input]gain, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_VIDEO_FRAME_RATE,					/*!<Command to set video scenario sensor frame rate. arg1:[input]video frame rate, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC,				/*!<Command to set AE sync write shutter and gain. arg1:[input]raw gain R, Gr raw gain Gb, B exposure time, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_CCT_FEATURE_CONTROL,					/*!<Command to set CCT feature. arg1:[input]ACDK_SENSOR_FEATURE_ENUM, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,				/*!<Command to set sensor calibration data. arg1:[input]SET_SENSOR_CALIBRATION_DATA_STRUCT, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO,			/*!<Command to set sensor max output frame rate by scenario. arg1:[input] ACDK_SCENARIO_ID_ENUM, arg2:[input] frame rate (10base), arg3:N/A*/ 
    SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,					/*!<Command to set sensor test pattern output enable control. arg1:[input] on/off, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN,                /*!<Command to set sensor sensor shutter and sensor gain. arg1:[input] pointer of shutter and gain, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_N3D_CONFIG,                          /*!<Command to set native 3D mode configuration. arg1: [input]halSensorN3dConfig_t, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_N3D_I2C_POS,                         /*!<Command to set count in SMI clock rate for I2C1&2 delay arg1:[input]position count, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_I2C_TRIGGER,                     /*!<Command to set hardware trigger mode arg1:[input]trigger mode, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA,              /*!<Command to set I2C command to N3D I2C buffer used to control stream arg1:N/A, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_START_STREAMING,                 /*!<Command to set sensor driver to start straming out arg1:N/A, arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_SET_N3D_STOP_STREAMING,                  /*!<Command to set sensor driver to stop straming out arg1:N/A, arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_GET_SENSOR_DEV				= 0x2000,	/*!<Command to get camera operation device. arg1:[output]current sensor dev, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_PRV_RANGE,					/*!<Command to get sensor preview scenario output size. arg1:[output]preview width, arg2:[output]preview height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_FULL_RANGE,					/*!<Command to get sensor capture scenario output size. arg1:[output]full width, arg2:[output]full height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,					/*!<Command to get sensor video preview or record scenario output size. arg1:[output]video width, arg2:[output]video height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE,		/*!<Command to get sensor high speed video preview or record scenario output size. arg1:[output]video width, arg2:[output]video height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE,					/*!<Command to get sensor 3D preview scenario output size. arg1:[output]preview width, arg2:[output]preview height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE,				/*!<Command to get sensor 3D capture scenario output size. arg1:[output]full width, arg2:[output]full height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE,				/*!<Command to get sensor 3D video preview or record scenario output size. arg1:[output]video width, arg2:[output]video height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_ID,							/*!<Command to get sensor device ID . arg1:[output]Sensor Id, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_GET_RAW_PADDING_RANGE,					/*!<Command to get isp padding size. arg1:[output]isp padding width, arg2:[output]isp padding height, arg3:N/A*/ 
    SENSOR_CMD_GET_SENSOR_NUM,							/*!<Command invalid. arg1:N/A, arg2:N/A, arg3:N/A*/ 	
    SENSOR_CMD_GET_SENSOR_TYPE,							/*!<Command to get sensor output format type. arg1:[output] sensor type, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_GET_RAW_INFO,							/*!<Command to get sensor information. arg1:[output] halSensorRawImageInfo_t, arg2:[input] mode, arg3:N/A*/ 
    SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT,			/*!<Command to get sensor operation unstable frame number. arg1:[output] delay frame count, arg2:[input] operation mode, arg3:N/A*/ 			
    SENSOR_CMD_GET_INPUT_BIT_ORDER,						/*!<Command to get sensor pin connection bit order. arg1:[output] senosr bit order, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_PAD_PCLK_INV,						/*!<Command to get sensor pclk inverse setting. arg1:[output] pad pclk inverter, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_ORIENTATION_ANGLE,			/*!<Command to get sensor module orientation angle. arg1:[output] sensor orientation, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_FACING_DIRECTION,				/*!<Command to get sensor facing direction(0:back/1:front). arg1:[output] sensor facing direction, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,					/*!<Command to get sensor operating pixel clock. arg1:[output] pixel clock frequency, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,			/*!<Command to get sensor operating frame line length and line pxel value. arg1:[output] line pixels & frame line number, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_FEATURE_INFO,					/*!<Command to get sensor feature information. arg1:[output] feature info, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_ATV_DISP_DELAY_FRAME,				/*!<Command to get ATV required delay frame for display. arg1:[output] delay frame count, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_SCENARIO,						/*!<Command to get sensor current operating scenario. arg1:[output] ACDK_SCENARIO_ID_ENUM, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_CROPINFO,						/*!<Command to get TG cropping information. arg1:[output] SENSOR_CROP_INFO, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_GRAB_INFO,					/*!<Command to get sensor output grab information. arg1:[output] SENSOR_GRAB_INFO_STRUCT, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_SUBSAMPLING_INFO,             /*!<Command to get sensor sumsampling information. arg1:[output] subsample width, subsample height, arg3:N/A*/ 			
    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,		/*!<Command to get sensor default output frame rate by scenario. arg1:[input] ACDK_SCENARIO_ID_ENUM, arg2:[output] frame rate (10 base), arg3:N/A*/ 			
    SENSOR_CMD_GET_FAKE_ORIENTATION,					/*!<Command to get sensor module orientation is fake and need resizer to rotate information. arg1:[output] FAKE orientation, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_VIEWANGLE,    				/*!<Command to get sensor module viewangle. arg1:[output] horizontal view angle, arg2:[output] vertical view angle, arg3:N/A*/ 			
    SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,			/*!<Command to get sensor test pattern output check sum value. arg1:[output] test pattern check sum value, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_CURRENT_TEMPERATURE,			/*!<Command to get sensor current temerature. arg1:[output]sensor temperature, arg2:N/A, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_PRV_RANGE,			/*!<Command to get sensor maxumum preview active output size. arg1:[output] sensor effective preview width, arg2:[output] sensor effective preview height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_FULL_RANGE,			/*!<Command to get sensor maxumum capture active output size. arg1:[output] sensor effective full width, arg2:[output] sensor effective full height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_VIDEO_RANGE,		/*!<Command to get sensor maxumum video active output size. arg1:[output] sensor video full width, arg2:[output] sensor video full height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_HIGH_SPEED_VIDEO_RANGE,/*!<Command to get sensor maxumum high speed video active output size. arg1:[output] sensor effective video width, arg2:[output] sensor effective video height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_3D_PRV_RANGE,		/*!<Command to get sensor maxumum 3D preview active output size. arg1:[output] sensor effective preview width, arg2:[output] sensor effective preview height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_3D_FULL_RANGE,		/*!<Command to get sensor maxumum 3D capture active output size. arg1:[output] sensor effective full width, arg2:[output] sensor effective full height, arg3:N/A*/ 			
	SENSOR_CMD_GET_SENSOR_EFFECTIVE_3D_VIDEO_RANGE,		/*!<Command to get sensor maxumum 3D video active output size. arg1:[output] sensor effective video width, arg2:[output] sensor effective video height, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_PRV_RANGE_OFFSET,				/*!<Command to get sensor preview output offset from maximum active output size. arg1:[output] sensor preview output horizontal offset, arg2:[output] sensor preview output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_FULL_RANGE_OFFSET,			/*!<Command to get sensor capture output offset from maximum active output size. arg1:[output] sensor full output horizontal offset, arg2:[output] sensor full output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_VIDEO_RANGE_OFFSET,			/*!<Command to get sensor video output offset from maximum active output size. arg1:[output] sensor video output horizontal offset, arg2:[output] sensor video output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_HIGH_SPEED_VIDEO_RANGE_OFFSET,/*!<Command to get sensor high speed video output offset from maximum active output size. arg1:[output] sensor video output horizontal offset, arg2:[output] sensor video output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_3D_PRV_RANGE_OFFSET,			/*!<Command to get sensor 3D preview output offset from maximum active output size. arg1:[output] sensor preview output horizontal offset, arg2:[output] sensor preview output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_3D_FULL_RANGE_OFFSET,			/*!<Command to get sensor 3Dcapture output offset from maximum active output size. arg1:[output] sensor full output horizontal offset, arg2:[output] sensor full output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_3D_VIDEO_RANGE_OFFSET,    	/*!<Command to get sensor 3D video output offset from maximum active output size. arg1:[output] sensor video output horizontal offset, arg2:[output] sensor video output vertical offset, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,         /*!<Command to get native 3D module difference count value arg1:[output] halSensorN3dDiffCnt_t, arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME,     /*!<Command to get time between stream out command to vsync occur. arg1:[output] time(us), arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_SET_YUV_FEATURE_CMD			= 0x3000,	/*!<Command to set YUV sensor feature. arg1:[input] feature id, arg2:[input] feature value, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,				/*!<Command to set YUV sensor single focus. arg1:N/A, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_CANCEL_AF,						/*!<Command to cancel YUV sensor AF. arg1:N/A, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_CONSTANT_AF,						/*!<Command to set YUV sensor constant focus. arg1:N/A, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_AF_WINDOW,    					/*!<Command to set YUV sensor AF window. arg1:[input]AF window, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_AE_WINDOW,   					/*!<Command to set YUV sensor AE window. arg1:[input]AE window, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_SET_YUV_AUTOTEST,                        /*!<Command to set YUV sensor autotest. arg1:[input]YUV_AUTOTEST_T cmd, arg2:[input] param, arg3:N/A*/ 			 
    SENSOR_CMD_SET_YUV_3A_CMD,                          /*!<Command to set YUV sensor 3A cmd. arg1:[input]ACDK_SENSOR_3A_LOCK_ENUM cmd, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_AF_STATUS			= 0x4000,   /*!<Command to get YUV sensor AF status. arg1:[output] af status, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN,            /*!<Command to get YUV sensor AE shutter/gain for EV5 & 13 and AWB gain value. arg1:[output] pSensorAEAWBRefStruct, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN,   /*!<Command to get YUV sensor current AE shutter/gain and AWB gain value. arg1:[output] pSensorAEAWBCurStruct, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,          /*!<Command to get YUV sensor AF window number. arg1:[output] window number, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,       /*!<Command to get YUV sensor AE window number. arg1:[output] window number, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_EXIF_INFO,                       /*!<Command to get YUV sensor EXIF info. arg1:[output] SENSOR_EXIF_INFO_STRUCT, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_DELAY_INFO,                      /*!<Command to get YUV sensor delay info. arg1:[output] SENSOR_DELAY_INFO_STRUCT, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_AE_AWB_LOCK,                     /*!<Command to get YUV sensor AE/AWB lock capability. arg1:[output] AE lock support, arg2: AWB lock support, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_STROBE_INFO,                     /*!<Command to get YUV sensor current strobe info. arg1:[output] SENSOR_FLASHLIGHT_AE_INFO_STRUCT, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO,         /*!<Command to get YUV sensor current flashlight auto fire necessity. arg1:[output] fire or not, arg2:N/A, arg3:N/A*/ 			 
    SENSOR_CMD_GET_YUV_SENSOR_CAPTURE_OUTPUT_JPEG, //TRUE:JPEG,FALSE:original raw/yuv data
    SENSOR_CMD_SET_YUV_JPEG_PARA, //Set JPEG-Sensor parameters
    SENSOR_CMD_GET_YUV_JPEG_INFO, //Set JPEG-Sensor parameters
    SENSOR_CMD_SET_YUV_MSHOT_ENABLE, //notify sensor-hal, mShot is enable
    SENSOR_CMD_GET_YUV_SENSOR_FIXED_JPEG_ORIENTATION, //    
SENSOR_CMD_MAX                 = 0xFFFF
} halSensorCmd_e;
//
/**
*@enum<halSensorDev_e>
*@brief:camera operating device enum
*/
typedef enum halSensorDev_s {
    SENSOR_DEV_NONE = 0x00,		/*!<No camera device*/ 
    SENSOR_DEV_MAIN = 0x01,		/*!<Main camera device or Rear camera device*/ 
    SENSOR_DEV_SUB  = 0x02,		/*!<Sub camera device or Front camera device*/ 
    SENSOR_DEV_ATV  = 0x04,		/*!<Analog TV device */ 
    SENSOR_DEV_MAIN_2 = 0x08,	/*!<Main2 camera device (used in 3D scenario) */ 
    SENSOR_DEV_MAIN_3D = 0x09,	/*!<3D camera device (Main+Main2)*/ 
} halSensorDev_e;

/**
*@enum<halSensorType_e>
*@brief:Sensor output data format type
*/
typedef enum halSensorType_s {
    SENSOR_TYPE_RAW = 0,		/*!<RAW data format type*/
    SENSOR_TYPE_YUV = 1, 		/*!<YUV data format type*/
    SENSOR_TYPE_YCBCR = 2,		/*!<YCBCR data format type */
    SENSOR_TYPE_RGB565 = 3, 	/*!<RGB565 data format type*/
    SENSOR_TYPE_RGB888 = 4,		/*!<RGB888 data format type*/     
    SENSOR_TYPE_JPEG = 5,		/*!<JPEG data format type*/    
    SENSOR_TYPE_UNKNOWN = 0xFF,
} halSensorType_e; 

/**
*@enum<halSensorDelayFrame_e>
*@brief:Sensor operation valid required delay frame
*/
typedef enum halSensorDelayFrame_s {
	SENSOR_PREVIEW_DELAY = 0,			/*!<Request delay frame for sensor set to preview mode be valid */
	SENSOR_VIDEO_DELAY,					/*!<Request delay frame for sensor set to video mode be valid */
	SENSOR_CAPTURE_DELAY,				/*!<Request delay frame for sensor set to capture mode be valid */
	SENSOR_YUV_AWB_SETTING_DELAY,		/*!<Request delay frame for YUV sensor set WB setting be valid*/
	SENSOR_YUV_EFFECT_SETTING_DELAY,	/*!<Request delay frame for YUV sensor set color effect be valid */
	SENSOR_AE_SHUTTER_DELAY,			/*!<Request delay frame for sensor set AE shutter be valid */
	SENSOR_AE_GAIN_DELAY,				/*!<Request delay frame for sensor set AE gain be valid */
	SENSOR_AE_ISP_DELAY,				/*!<Request delay frame for sensor set AE ISP gain be valid */
}halSensorDelayFrame_e;
/**
*@enum<halSensorWaitEvent_e>
*@brief:Waiting event type for sensor operation
*/

typedef enum halSensorWaitEvent_s{ 	
	SENSOR_WAIT_SET_SHUTTER_GAIN_DONE = 0x0,	/*waiting for sensor driver set shutter and gain done*/
	SENSOR_WAIT_EVENT_MAX = 0xFFFF
}halSensorWaitEvent_e;

// for JPEG Sensor
typedef struct {
    MUINT32 u4FileSize;          // For input sensor width
    MUINT32 u4SrcW;          // For input sensor width
    MUINT32 u4SrcH;          // For input sensor height
} halSensorJpegInfo_t;


typedef struct
{
   MUINT32 tgtWidth;
   MUINT32 tgtHeight;
   MUINT32 quality;

}halSensorJpegConfigPara_t;

/*******************************************************************************
*
********************************************************************************/
/**
*@ user space sensor driver class used to define sensor hal provide interface for sensor driver user
*
*/

class SensorHal {

public:
    //
	/**
	 * @brief:create sensor hal instance when first time to connect sensor hal 
	 *
	 *
	 * @note:it must use in pair with destroyInstance()

	 *
	 * @return None
	 */
    static SensorHal* createInstance();
	/**
	 * @brief:destroy sensor hal instance when not use sensor hal anymore  
	 *
	 *
	 * @note:it must use in pair with createInstance()

	 *
	 * @return None
	 */

	
    virtual void destroyInstance() = 0;

protected:
    virtual ~SensorHal() {};

public:
	/**
	 * @brief:used to detect connected camera sensor device. 
	 *
	 * @note None
	 * 
	 * @return
	 * -  return value means connected camera sensor device represented in halSensorDev_e enum value.
	 */
    virtual MINT32 searchSensor() = 0;
	//
	/**
	 * @brief:initial sensor driver and related settings and then open assigned camera sensor device
	 *
	 * @details  
	 *
	 * @note:This API only need to be called once for the first user to open camera sensor device. The late coming user call will be redudant and useless. This must use in pair with uninit()
	 * 
	 * @return
	 * - MTRUE indicates success. 
	 * - MFALSE indicates failure. 	
	 */
    virtual MINT32 init() = 0;
    //
	/**
	 * @brief:uninitial sensor driver and restore default sensor driver settings and then close assigned camera sensor device 
	 *
	 * @details 
	 *
	 * @note:must used in pair with init()
	 *
	 * @return
	 * - MTRUE indicates success. 
	 * - MFALSE indicates failure. 	
	 */    
    virtual MINT32 uninit() = 0;
    //
	/**
	 * @brief:set mclk for ATV 
	 *
	 * @return
	 * - MTRUE indicates success.
	 * - MFALSE indicates failure. 	
	 */    
    virtual MINT32 setATVStart() = 0;
    //
	/**
	 * @brief:used to set sensor driver config setting for operating scenario 
	 *
	 * @details:set sensor input size, TG crop size, TG single or continous output frame data, if bypass set sensor scenario, if bypass delay frame for swtich scenario and scenario information
	 *
	 * @note 
	 * 
	 * @param [in]   halSensorIFParam_t, please see halSensorIFParam_t structure description
	 *
 	 * @return
	 * - MTRUE indicates success.  
	 * - MFALSE indicates failure. 	
	 */    
    virtual MINT32 setConf(halSensorIFParam_t halSensorIFParam[2]) = 0;
    //
	/**
	 * @brief:A genernal interface to access sensor driver by constructed command id
	 *
	 * @details 
	 *
	 * @note: Please check halSensorWaitEvent_e enum description to fill correct defined arguments
	 * 
	 * @param [in] EventType: waiting event enum defined in halSensorWaitEvent_e
	 * @param [in] Timeout: waiting time for timeout. unit is us
	 *
	 * @return
	 * - MTRUE indicates success. 
	 * - MFALSE indicates failure after timeout . 	
	 */  

	virtual MINT32 waitSensorEventDone(MUINT32 EventType, MUINT32 Timeout) = 0;	 

    //
	/**
	 * @brief:A general waiting sensor event type
	 *
	 * @details 
	 *
	 * @note :Please check halSensorCmd_e enum description to fill correct defined arguments
	 * 
	 * @param [in] sensorDevId: assigned camera sensor device for access
	 * @param [in] cmd: the command id for the sensor driver access
	 * @param [in] arg1: parameter needed by the assigned cmd, please reference halSensorCmd_e enum description for detail description
	 * @param [in] arg2: parameter needed by the assigned cmd, please reference halSensorCmd_e enum description for detail description
	 * @param [in] arg3: parameter needed by the assigned cmd, please reference halSensorCmd_e enum description for detail description
	 *
	 * @return
	 * - MTRUE indicates success. 
	 * - MFALSE indicates failure . 	
	 */    
  	
    virtual MINT32 sendCommand(
	    halSensorDev_e sensorDevId,
        int cmd,
        int arg1 = 0,
        int arg2 = 0,
        int arg3 = 0) = 0;
    //
	/**
	 * @brief:used to dump sensor interface module registers
	 *
	 * @details
	 *
	 * @note 
	 * 
	 * @return
	 * - MTRUE indicates success. 
	 * - MFALSE indicates failure. 	
	 */    
    virtual MINT32 dumpReg() = 0;
	//
	/**
	 * @brief:store exif information 
	 *
	 * @details:store sensor driver debug and specific information into exif tag
	 *
	 * 
	 * @param [in] pIBaseCamExif: structure pointer of IBaseCamExif.
	 *
	 * @return
	 * - MTRUE indicates success.
	 * - MFALSE indicates failure. 	
	 */	
	virtual MINT32 setDebugInfo(IBaseCamExif *pIBaseCamExif) = 0;
	//
	/**
	 * @brief:soft reset sensor device driver to output image data
	 *
	 * @return
	 * - MTRUE indicates success.
	 * - MFALSE indicates failure. 	
	 */	
	virtual MINT32 reset() = 0;		
	//
};

/*******************************************************************************
*
********************************************************************************/


#endif // _ISP_DRV_H_

