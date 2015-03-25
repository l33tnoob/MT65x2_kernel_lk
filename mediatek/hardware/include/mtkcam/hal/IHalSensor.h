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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALSENSOR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALSENSOR_H_
//
#include <mtkcam/common.h>
#include <mtkcam/metadata/IMetadata.h>

class IBaseCamExif;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Sensor Scenario ID
 ******************************************************************************/
enum
{
    SENSOR_SCENARIO_ID_NORMAL_PREVIEW, 
    SENSOR_SCENARIO_ID_NORMAL_CAPTURE, 
    SENSOR_SCENARIO_ID_NORMAL_VIDEO, 
    SENSOR_SCENARIO_ID_SLIM_VIDEO1,
    SENSOR_SCENARIO_ID_SLIM_VIDEO2,
    /**************************************************************************
     * All unnamed scenario id for a specific sensor must be started with 
     * values >= SENSOR_SCENARIO_ID_UNNAMED_START.
     **************************************************************************/
    SENSOR_SCENARIO_ID_UNNAMED_START = 0x100, 
};

enum 
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
};

enum
{
	CAM_TG_NONE = 0x0,
	CAM_TG_1    = 0x1,
	CAM_TG_2    = 0x2,
	CAM_SV_1    = 0x4,
	CAM_SV_2    = 0x8,
	CAM_TG_ERR = 0xFF
};

enum
{
	ONE_PIXEL_MODE = 0x0,
	TWO_PIXEL_MODE = 0x1,
};

enum
{
    SENSOR_TYPE_UNKNOWN = 0x0,
	SENSOR_TYPE_RAW,    
	SENSOR_TYPE_YUV,
	SENSOR_TYPE_RGB,
	SENSOR_TYPE_JPEG,
};

enum
{
    RAW_SENSOR_8BIT = 0x0,
	RAW_SENSOR_10BIT,
	RAW_SENSOR_12BIT,
	RAW_SENSOR_14BIT,
	RAW_SENSOR_ERROR = 0xFF,
};

enum
{
    SENSOR_FORMAT_ORDER_RAW_B = 0x0,
    SENSOR_FORMAT_ORDER_RAW_Gb,
    SENSOR_FORMAT_ORDER_RAW_Gr,
    SENSOR_FORMAT_ORDER_RAW_R,
    SENSOR_FORMAT_ORDER_UYVY,
    SENSOR_FORMAT_ORDER_VYUY,
    SENSOR_FORMAT_ORDER_YUYV,
    SENSOR_FORMAT_ORDER_YVYU,
    SENSOR_FORMAT_ORDER_NONE = 0xFF,
};

enum
{
	SENSOR_PREVIEW_DELAY = 0,			/*!<Request delay frame for sensor set to preview mode be valid */
	SENSOR_VIDEO_DELAY,					/*!<Request delay frame for sensor set to video mode be valid */
	SENSOR_CAPTURE_DELAY,				/*!<Request delay frame for sensor set to capture mode be valid */
	SENSOR_YUV_AWB_SETTING_DELAY,		/*!<Request delay frame for YUV sensor set WB setting be valid*/
	SENSOR_YUV_EFFECT_SETTING_DELAY,	/*!<Request delay frame for YUV sensor set color effect be valid */
	SENSOR_AE_SHUTTER_DELAY,			/*!<Request delay frame for sensor set AE shutter be valid */
	SENSOR_AE_GAIN_DELAY,				/*!<Request delay frame for sensor set AE gain be valid */
	SENSOR_AE_ISP_DELAY,
};


 enum {
    SENSOR_CMD_SET_SENSOR_EXP_TIME	        = 0x1000,	/*!<Command to set sensor exposure time in unit us. arg1:[input]exp time, arg2:N/A, arg3:N/A*/ 
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
    SENSOR_CMD_SET_N3D_CONFIG              = 0x1100,   /*!<Command to set native 3D mode configuration. arg1: [input]halSensorN3dConfig_t, arg2:N/A, arg3:N/A*/ 
    SENSOR_CMD_SET_N3D_I2C_POS,                         /*!<Command to set count in SMI clock rate for I2C1&2 delay arg1:[input]position count, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_I2C_TRIGGER,                     /*!<Command to set hardware trigger mode arg1:[input]trigger mode, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA,              /*!<Command to set I2C command to N3D I2C buffer used to control stream arg1:N/A, arg2:N/A, arg3:N/A */ 
    SENSOR_CMD_SET_N3D_START_STREAMING,                 /*!<Command to set sensor driver to start straming out arg1:N/A, arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_SET_N3D_STOP_STREAMING,                  /*!<Command to set sensor driver to stop straming out arg1:N/A, arg2:N/A, arg3:N/A  */ 
    SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT = 0x2000,	/*!<Command to get sensor operation unstable frame number. arg1:[output] delay frame count, arg2:[input] operation mode, arg3:N/A*/ 			
    SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,					/*!<Command to get sensor operating pixel clock. arg1:[output] pixel clock frequency, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,			/*!<Command to get sensor operating frame line length and line pxel value. arg1:[output] line pixels & frame line number, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_FEATURE_INFO,					/*!<Command to get sensor feature information. arg1:[output] feature info, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,		/*!<Command to get sensor default output frame rate by scenario. arg1:[input] ACDK_SCENARIO_ID_ENUM, arg2:[output] frame rate (10 base), arg3:N/A*/ 			
    SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,			/*!<Command to get sensor test pattern output check sum value. arg1:[output] test pattern check sum value, arg2:N/A, arg3:N/A*/ 			
    SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT = 0x2100,/*!<Command to get native 3D module difference count value arg1:[output] halSensorN3dDiffCnt_t, arg2:N/A, arg3:N/A  */ 
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
    SENSOR_CMD_MAX                 = 0xFFFF
} ;


 struct SensorStaticInfo{
    MUINT sensorDevID; //0x5642,....
    MUINT sensorType;  //SENSOR_TYPE_RAW, ....
    MUINT sensorFormatOrder; //SENSOR_FORMAT_ORDER_RAW_B,...
    MUINT rawSensorBit; //RAW_SENSOR_8BIT,....
    MBOOL iHDRSupport;
    MUINT previewWidth;
    MUINT previewHeight;
    MUINT captureWidth;
    MUINT captureHeight;
    MUINT videoWidth;
    MUINT videoHeight;
	MUINT video1Width;
	MUINT video1Height;
	MUINT video2Width;
	MUINT video2Height;
	MUINT previewDelayFrame;
	MUINT captureDelayFrame;
    MUINT videoDelayFrame;
	MUINT video1DelayFrame;
	MUINT video2DelayFrame;
	MUINT aeShutDelayFrame;
	MUINT aeSensorGainDelayFrame;
	MUINT aeISPGainDelayFrame;
    MUINT orientationAngle;
    MUINT facingDirection;
    MUINT previewFrameRate;//10 base
    MUINT captureFrameRate;//10 base
    MUINT videoFrameRate;//10 base
    MUINT video1FrameRate;//10 base
    MUINT video2FrameRate;//10 base
    MBOOL fakeOrientation;
    MUINT horizontalViewAngle;
    MUINT verticalViewAngle;
    MUINT previewActiveWidth;//3d use onlyl?
    MUINT previewActiveHeight;//3d use onlyl?
    MUINT captureActiveWidth;//3d use onlyl?
    MUINT captureActiveHeight;//3d use onlyl?
    MUINT videoActiveWidth;//3d use onlyl?
    MUINT videowActiveHeight;//3d use onlyl?
    MUINT previewHoizontalOutputOffset;//3d use onlyl?
    MUINT previewVerticalOutputOffset; //3d use onlyl?   
    MUINT captureHoizontalOutputOffset;//3d use onlyl?
    MUINT captureVerticalOutputOffset; //3d use onlyl?       
    MUINT videoHoizontalOutputOffset;//3d use onlyl?
    MUINT videoVerticalOutputOffset; //3d use onlyl?  
	MBOOL virtualChannelSupport;    
};


struct SensorDynamicInfo{
	MUINT TgInfo;       //TG_NONE,TG_1,...
	MUINT pixelMode;    //ONE_PIXEL_MODE, TWO_PIXEL_MODE
};

 //N3D config
 /**
 *@struct<SENSOR_N3D_CONFIG_STRUCT>
 *@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_SET_N3D_CONFIG to set native 3D control configuration
 */
 struct SensorN3dConfig{
	 MUINT32 u4N3dEn;			 /*!<N3D module enable control*/
	 MUINT32 u4I2C1En;			 /*!<N3D 1st I2C control enable control*/
	 MUINT32 u4I2C2En;			 /*!<N3D 2nd I2C control enable control*/
	 MUINT32 u4N3dMode; 		 /*!<N3D I2C control mode*/
	 MUINT32 u4DiffCntEn;		 /*!<N3D sensor vsync difference count enable control */
	 MUINT32 u4DiffCntThr;		 /*!<N3D sensor vsync difference threshold */
 } ; 

 //N3D different count
 /**
 *@struct<SENSOR_N3D_DIFFERENCE_COUNT_STRUCT>
 *@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT to get native 3D vsync difference information
 */
 struct SensorN3dDiffCnt{
	 MUINT32 u4DiffCnt; 		 /*!<vsync difference count between vysnc1 & vsync2*/
	 MUINT32 u4Vsync1Cnt;		 /*!<*vsync1 valid count*/
	 MUINT32 u4Vsync2Cnt;		 /*!<vsync2 valid count*/
	 MUINT32 u4DebugPort;		 /*!<vsync2 count number sample at vysnc1*/
	 MUINT32 u4Data[10];		 /*!<reserve*/
 } ; 


/******************************************************************************
 *  Hal Sensor Interface.
 ******************************************************************************/
class IHalSensor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    
    /**
     * @brief Used in configure.
     */
    struct  ConfigParam
    {
        MUINT                       index;              //  sensor index
        MSize                       crop;               //  TG crop size in pixels.
        MUINT                       scenarioId;         //  sensor scenario ID.
        MBOOL                       isBypassScenario;
        MBOOL                       isContinuous;
        MBOOL                       iHDROn;				//iHDR mode
        MUINT                       framerate;          //10based, 0: will run sensor mode default setting
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalSensor() {}

public:     ////                    Instantiation.

    /**
     * Destroy this instance created from IHalSensorList::createSensor.
     */
    virtual MVOID                   destroyInstance(
                                        char const* szCallerName = ""
                                    )                                           = 0;

public:     ////                    Operations.

    /**
     * Turn on/off the power of sensor(s).
     */
    virtual MBOOL                   powerOn(
                                           char const* szCallerName,           
	                                       MUINT const uCountOfIndex, 
                                           MUINT const*pArrayOfIndex
                                    )                                          = 0;
    virtual MBOOL                   powerOff(
                                          char const* szCallerName,			
                                          MUINT const uCountOfIndex, 
                                          MUINT const*pArrayOfIndex
                                    )			                               = 0;
	

    /**
     * Configure the sensor(s).
     */
    virtual MBOOL                   configure(
                                        MUINT const         uCountOfParam, 
                                        ConfigParam const*  pArrayOfParam
                                    ) 											= 0;

    /**
     * Common interface to access sensor for specified command index and sensorDevIdx .It must be used after powerOn
     */
    virtual MINT 					sendCommand(
								    		MUINT sensorDevIdx,
							        		MUINT cmd,
							        		MUINT arg1,
							        		MUINT arg2,
							        		MUINT arg3
						        	)											 = 0;
	/**
     * Query sensor dynamic information by sensorDevIdx..
      */
    virtual MBOOL                 querySensorDynamicInfo(
                                      MUINT32 sensorIdx,
                                      SensorDynamicInfo *pSensorDynamicInfo
                                   )                                             = 0;
	/**
     * Sensor set exif debug information.
      */
	
	virtual MINT32                setDebugInfo(IBaseCamExif *pIBaseCamExif)      = 0;

};


/******************************************************************************
 *  Hal Sensor List Interface.
 ******************************************************************************/
class IHalSensorList
{
public:     ////                    Instantiation.
    static  IHalSensorList*         get();

protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalSensorList() {}

public:     ////                    Attributes.

    /**
     * Query the number of image sensors.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryNumberOfSensors() const                = 0;

    /**
     * Query static information for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IMetadata const&        queryStaticInfo(MUINT const index) const    = 0;

    /**
     * Query the driver name for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual char const*             queryDriverName(MUINT const index) const    = 0;

    /**
     * Query the sensor type of NSSensorType::Type for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryType(MUINT const index) const          = 0;
    /**
     * Query SensorDev Index by sensor list index.
     * This call is legal only after searchSensors().
     * Return SENSOR_DEV_MAIN, SENSOR_DEV_SUB,...
     */
	virtual MUINT                   querySensorDevIdx(MUINT const index) const     = 0;
	
    /**
     * Query Sensor Information.
     * This call is legal only after searchSensors().
     */

	virtual MVOID				   querySensorStaticInfo(
										MUINT sensorDevIdx, 
										SensorStaticInfo *pSensorStaticInfo
									)												= 0;

public:     ////                    Operations.

    /**
     * Search sensors and return the number of image sensors.
     */
    virtual MUINT                   searchSensors()                             = 0;

    /**
     * Create an instance of IHalSensor for a single specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName, 
                                        MUINT const index
                                    )                                           = 0;

    /**
     * Create an instance of IHalSensor for multiple specific sensor indexes.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName, 
                                        MUINT const uCountOfIndex, 
                                        MUINT const*pArrayOfIndex
                                    )                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALSENSOR_H_

