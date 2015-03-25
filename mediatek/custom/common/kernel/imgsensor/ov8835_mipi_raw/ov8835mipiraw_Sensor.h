/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   CMOS sensor header file
 *
 ****************************************************************************/
#ifndef _OV8835MIPI_SENSOR_H
#define _OV8835MIPI_SENSOR_H

// #define USE_24FPS_SETTING // LINE <><2013.10.31><Use 24FPS to test blc> Jiangde
#define USE_1080P_VIDEO  // LINE <><2013.9.15><use 1080p to cut down video record current> Jiangde
#define OV8835MIPI_DEBUG
#define OV8835MIPI_DRIVER_TRACE
//#define OV8835MIPI_TEST_PATTEM
#ifdef OV8835MIPI_DEBUG
//#define SENSORDB printk
#else
//#define SENSORDB(x,...)
#endif

#define OV8835MIPI_FACTORY_START_ADDR 0
#define OV8835MIPI_ENGINEER_START_ADDR 10

//#define MIPI_INTERFACE

 
typedef enum OV8835MIPI_group_enum
{
  OV8835MIPI_PRE_GAIN = 0,
  OV8835MIPI_CMMCLK_CURRENT,
  OV8835MIPI_FRAME_RATE_LIMITATION,
  OV8835MIPI_REGISTER_EDITOR,
  OV8835MIPI_GROUP_TOTAL_NUMS
} OV8835MIPI_FACTORY_GROUP_ENUM;

typedef enum OV8835MIPI_register_index
{
  OV8835MIPI_SENSOR_BASEGAIN = OV8835MIPI_FACTORY_START_ADDR,
  OV8835MIPI_PRE_GAIN_R_INDEX,
  OV8835MIPI_PRE_GAIN_Gr_INDEX,
  OV8835MIPI_PRE_GAIN_Gb_INDEX,
  OV8835MIPI_PRE_GAIN_B_INDEX,
  OV8835MIPI_FACTORY_END_ADDR
} OV8835MIPI_FACTORY_REGISTER_INDEX;

typedef enum OV8835MIPI_engineer_index
{
  OV8835MIPI_CMMCLK_CURRENT_INDEX = OV8835MIPI_ENGINEER_START_ADDR,
  OV8835MIPI_ENGINEER_END
} OV8835MIPI_FACTORY_ENGINEER_INDEX;

typedef struct _sensor_data_struct
{
  SENSOR_REG_STRUCT reg[OV8835MIPI_ENGINEER_END];
  SENSOR_REG_STRUCT cct[OV8835MIPI_FACTORY_END_ADDR];
} sensor_data_struct;
typedef enum {
    OV8835MIPI_SENSOR_MODE_INIT = 0,
    OV8835MIPI_SENSOR_MODE_PREVIEW,
    OV8835MIPI_SENSOR_MODE_VIDEO,
    OV8835MIPI_SENSOR_MODE_CAPTURE
} OV8835MIPI_SENSOR_MODE;


/* SENSOR PREVIEW/CAPTURE VT CLOCK */
//#define OV8835MIPI_PREVIEW_CLK                     69333333  //48100000
//#define OV8835MIPI_CAPTURE_CLK                     69333333  //48100000

#define OV8835MIPI_COLOR_FORMAT                    SENSOR_OUTPUT_FORMAT_RAW_B //SENSOR_OUTPUT_FORMAT_RAW_R

#define OV8835MIPI_MIN_ANALOG_GAIN				1	/* 1x */
#define OV8835MIPI_MAX_ANALOG_GAIN				32	/* 32x */


/* FRAME RATE UNIT */
#define OV8835MIPI_FPS(x)                          (10 * (x))

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
//#define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          2700 /* 9 fps */

//#define OV8835MIPI_DEBUG_SETTING
#define OV8835_BINNING_SUM // binning: enable  for sum, disable for vertical averag, need to modify u4Cap2PreRatio from 1024 to 512	
#define OV8835MIPI_4LANE
#ifdef OV8835MIPI_4LANE
//#define OV8835MIPI_4LANE_CAP_30FPS
#endif

#ifdef OV8835MIPI_DEBUG_SETTING
#define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          3608  //3055 /* 8 fps */
#define OV8835MIPI_FULL_PERIOD_LINE_NUMS           2484   //1968
#define OV8835MIPI_PV_PERIOD_PIXEL_NUMS            3608 //1630 /* 30 fps */
#define OV8835MIPI_PV_PERIOD_LINE_NUMS             1260 //984

#else
#ifndef OV8835MIPI_4LANE
#define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          3608  //3055 /* 8 fps */
#define OV8835MIPI_FULL_PERIOD_LINE_NUMS           2586//2642//2586 //2484   //1968
#define OV8835MIPI_PV_PERIOD_PIXEL_NUMS            3608 //1630 /* 30 fps */
#define OV8835MIPI_PV_PERIOD_LINE_NUMS             1320//1260 //984
#define OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS            3608 //1630 /* 30 fps */
#define OV8835MIPI_VIDEO_PERIOD_LINE_NUMS             1320//1260 //984
#else
#ifdef OV8835MIPI_4LANE_CAP_30FPS
#define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          3608  //3055 /* 8 fps */
#define OV8835MIPI_FULL_PERIOD_LINE_NUMS           2522//2642//2586 //2484   //1968
#define OV8835MIPI_PV_PERIOD_PIXEL_NUMS            3608 //1630 /* 30 fps */
#define OV8835MIPI_PV_PERIOD_LINE_NUMS             1922//1260 //984
#define OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS            3608  //3055 /* 8 fps */
#define OV8835MIPI_VIDEO_PERIOD_LINE_NUMS             2522//2642//2586 //2484   //1968
#else
#ifdef USE_24FPS_SETTING
    #define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          3608 // test for 24fps  
    #define OV8835MIPI_FULL_PERIOD_LINE_NUMS           2504 // test for 24fps
#else
    #define OV8835MIPI_FULL_PERIOD_PIXEL_NUMS          3608  //3055 /* 8 fps */
    #define OV8835MIPI_FULL_PERIOD_LINE_NUMS           2522//2642//2586 //2484   //1968
#endif
    #define OV8835MIPI_PV_PERIOD_PIXEL_NUMS            3608 //1630 /* 30 fps */
    #define OV8835MIPI_PV_PERIOD_LINE_NUMS             1922 //1260 //984
    #define OV8835MIPI_VIDEO_PERIOD_PIXEL_NUMS         3608 //3055 /* 8 fps */
    #define OV8835MIPI_VIDEO_PERIOD_LINE_NUMS          2370 //for 30fps video//2400//2642//2586 //2484   //1968  //Jiangde video 2/3
#endif
#endif
#endif // OV8835MIPI_DEBUG_SETTING

/* SENSOR START/END POSITION */
#define OV8835MIPI_FULL_X_START                    9
#define OV8835MIPI_FULL_Y_START                    11
#define OV8835MIPI_IMAGE_SENSOR_FULL_WIDTH         (3264 - 64) /* 2560 */
#define OV8835MIPI_IMAGE_SENSOR_FULL_HEIGHT        (2448 - 48) /* 1920 */
#define OV8835MIPI_PV_X_START                      5
#define OV8835MIPI_PV_Y_START                      5
#define OV8835MIPI_IMAGE_SENSOR_PV_WIDTH           (1600)  //    (1280 - 16) /* 1264 */
#define OV8835MIPI_IMAGE_SENSOR_PV_HEIGHT          (1200)  //(960 - 12) /* 948 */

#ifndef OV8835MIPI_4LANE
#define OV8835MIPI_VIDEO_X_START                    9
#define OV8835MIPI_VIDEO_Y_START                    11
#define OV8835MIPI_IMAGE_SENSOR_VIDEO_WIDTH         1600//(3264 - 64) /* 2560 */
#define OV8835MIPI_IMAGE_SENSOR_VIDEO_HEIGHT        1200//(2448 - 48) /* 1920 */
#else
#define OV8835MIPI_VIDEO_X_START                    9
#define OV8835MIPI_VIDEO_Y_START                    11
#ifndef USE_1080P_VIDEO
 #define OV8835MIPI_IMAGE_SENSOR_VIDEO_WIDTH         (3264-64)//(3264 - 64) /* 2560 */
 #define OV8835MIPI_IMAGE_SENSOR_VIDEO_HEIGHT        (1836-36)//(2448 - 48) /* 1920 */
#else
 //#define OV8835MIPI_IMAGE_SENSOR_VIDEO_WIDTH         (1920 - 32) // (3264-64)//(3264 - 64) /* 2560 */ // Jiangde video 2/3
 //#define OV8835MIPI_IMAGE_SENSOR_VIDEO_HEIGHT        (1080 - 24) // (1836-36)//(2448 - 48) /* 1920 */
 #define OV8835MIPI_IMAGE_SENSOR_VIDEO_WIDTH         (1280-32)//(3264 - 64) /* 2560 */
 #define OV8835MIPI_IMAGE_SENSOR_VIDEO_HEIGHT        (720-24)//(2448 - 48) /* 1920 */
#endif
#endif




/* SENSOR READ/WRITE ID */
#define OV8835MIPI_SLAVE_WRITE_ID_1   (0x6c)
#define OV8835MIPI_SLAVE_WRITE_ID_2   (0x20)

#define OV8835MIPI_WRITE_ID   (0x20)  //(0x6c)
#define OV8835MIPI_READ_ID    (0x21)  //(0x6d)

/* SENSOR ID */
//#define OV8835MIPI_SENSOR_ID						(0x5647)


//added by mandrave
//#define OV8835MIPI_USE_OTP

#if defined(OV8835MIPI_USE_OTP)

struct ov8835mipi_otp_struct
{
    kal_uint16 customer_id;
	kal_uint16 module_integrator_id;
	kal_uint16 lens_id;
	kal_uint16 rg_ratio;
	kal_uint16 bg_ratio;
	kal_uint16 user_data[5];
	kal_uint16 lenc[63];

};

#define RG_TYPICAL 0x51
#define BG_TYPICAL 0x57


#endif




/* SENSOR PRIVATE STRUCT */
typedef struct OV8835MIPI_sensor_STRUCT
{
  MSDK_SENSOR_CONFIG_STRUCT cfg_data;
  sensor_data_struct eng; /* engineer mode */
  MSDK_SENSOR_ENG_INFO_STRUCT eng_info;
  kal_uint8 mirror;
  kal_bool pv_mode;
  kal_bool video_mode;
  kal_bool is_zsd;
  kal_bool is_zsd_cap;
  kal_bool is_autofliker;
  kal_uint16 normal_fps; /* video normal mode max fps */
  kal_uint16 night_fps; /* video night mode max fps */
  kal_uint16 shutter;
  kal_uint16 gain;
  kal_uint32 pv_pclk;
  kal_uint32 cap_pclk;
  kal_uint32 video_pclk;
  kal_uint32 pclk;
  kal_uint16 frame_length;
  kal_uint16 line_length;  
  kal_uint16 write_id;
  kal_uint16 read_id;
  kal_uint16 dummy_pixels;
  kal_uint16 dummy_lines;
  kal_uint32 max_exposure_lines;
  
  OV8835MIPI_SENSOR_MODE sensorMode;
} OV8835MIPI_sensor_struct;

//export functions
UINT32 OV8835MIPIOpen(void);
UINT32 OV8835MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV8835MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 OV8835MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 OV8835MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 OV8835MIPIClose(void);

#define Sleep(ms) mdelay(ms)

#endif 
