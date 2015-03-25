/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
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
 *   imx179mipiraw_sensor.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:

 *============================================================================
 ****************************************************************************/
/* SENSOR FULL SIZE */
#ifndef __SENSOR_H
#define __SENSOR_H

typedef enum group_enum {
    PRE_GAIN=0,
    CMMCLK_CURRENT,
    FRAME_RATE_LIMITATION,
    REGISTER_EDITOR,
    GROUP_TOTAL_NUMS
} FACTORY_GROUP_ENUM;


#define ENGINEER_START_ADDR 10
#define FACTORY_START_ADDR 0
#define USE_PREVIW_FOR_VIDEO           // LINE <><20130922><use 720p to cut down current> Jiangde

typedef enum engineer_index
{
    CMMCLK_CURRENT_INDEX=ENGINEER_START_ADDR,
    ENGINEER_END
} FACTORY_ENGINEER_INDEX;
typedef enum register_index
{
	SENSOR_BASEGAIN=FACTORY_START_ADDR,
	PRE_GAIN_R_INDEX,
	PRE_GAIN_Gr_INDEX,
	PRE_GAIN_Gb_INDEX,
	PRE_GAIN_B_INDEX,
	FACTORY_END_ADDR
} FACTORY_REGISTER_INDEX;



typedef struct
{
    SENSOR_REG_STRUCT	Reg[ENGINEER_END];
    SENSOR_REG_STRUCT	CCT[FACTORY_END_ADDR];
} SENSOR_DATA_STRUCT, *PSENSOR_DATA_STRUCT;


// Important Note:
//     1. Make sure horizontal PV sensor output is larger than IMX179MIPI_REAL_PV_WIDTH  + 2 * IMX179MIPI_IMAGE_SENSOR_PV_STARTX + 4.
//     2. Make sure vertical   PV sensor output is larger than IMX179MIPI_REAL_PV_HEIGHT + 2 * IMX179MIPI_IMAGE_SENSOR_PV_STARTY + 6.
//     3. Make sure horizontal CAP sensor output is larger than IMX179MIPI_REAL_CAP_WIDTH  + 2 * IMX179MIPI_IMAGE_SENSOR_CAP_STARTX + IMAGE_SENSOR_H_DECIMATION*4.
//     4. Make sure vertical   CAP sensor output is larger than IMX179MIPI_REAL_CAP_HEIGHT + 2 * IMX179MIPI_IMAGE_SENSOR_CAP_STARTY + IMAGE_SENSOR_V_DECIMATION*6.
// Note:
//     1. The reason why we choose REAL_PV_WIDTH/HEIGHT as tuning starting point is
//        that if we choose REAL_CAP_WIDTH/HEIGHT as starting point, then:
//            REAL_PV_WIDTH  = REAL_CAP_WIDTH  / IMAGE_SENSOR_H_DECIMATION
//            REAL_PV_HEIGHT = REAL_CAP_HEIGHT / IMAGE_SENSOR_V_DECIMATION
//        There might be some truncation error when dividing, which may cause a little view angle difference.
//Macro for Resolution
#define IMAGE_SENSOR_H_DECIMATION				2	// For current PV mode, take 1 line for every 2 lines in horizontal direction.
#define IMAGE_SENSOR_V_DECIMATION				2	// For current PV mode, take 1 line for every 2 lines in vertical direction.

#if 0
#define IMX179MIPI_REAL_PV_WIDTH				1600//1640-40
#define IMX179MIPI_REAL_PV_HEIGHT				1020//1050-30
/* Real CAP Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */
#define IMX179MIPI_REAL_CAP_WIDTH				3200//3280-80
#define IMX179MIPI_REAL_CAP_HEIGHT				2400//2464-64
#define IMX179MIPI_REAL_VIDEO_WIDTH				3272//3280-8
#define IMX179MIPI_REAL_VIDEO_HEIGHT			1846//1852-6
#endif
#define IMX179MIPI_REAL_PV_WIDTH				1632//1640
#define IMX179MIPI_REAL_PV_HEIGHT				1224//1232
/* Real CAP Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */
#define IMX179MIPI_REAL_CAP_WIDTH				3264//3280
#define IMX179MIPI_REAL_CAP_HEIGHT				2448//2464
#ifndef USE_PREVIW_FOR_VIDEO
//#define IMX179MIPI_REAL_VIDEO_WIDTH				3272//3280-8
//#define IMX179MIPI_REAL_VIDEO_HEIGHT			1846//1852-6
#define IMX179MIPI_REAL_VIDEO_WIDTH			    3264//3272//3280-8//LINE <SNTZT-259> <DATE20130731> <dither when EIS recording> wupingzhou
#define IMX179MIPI_REAL_VIDEO_HEIGHT            1836//1846//1852-6//LINE <SNTZT-259> <DATE20130731> <dither when EIS recording> wupingzhou
#else
// Preview
// #define IMX179MIPI_REAL_VIDEO_WIDTH          1632 // 720p=(1288 - 8) // 3272
// #define IMX179MIPI_REAL_VIDEO_HEIGHT         1224 // 720p=(968 - 6)  // 1846
#define IMX179MIPI_REAL_VIDEO_WIDTH             1616 // (1632-16) // (1288 - 8) // 3272
#define IMX179MIPI_REAL_VIDEO_HEIGHT            1216 // (1224-8) // (968 - 6)  // 1846
#endif

/* X/Y Starting point */
#define IMX179MIPI_IMAGE_SENSOR_PV_STARTX       2
#define IMX179MIPI_IMAGE_SENSOR_PV_STARTY       2	// The value must bigger or equal than 1.
#define IMX179MIPI_IMAGE_SENSOR_CAP_STARTX		4   //(IMX179MIPI_IMAGE_SENSOR_PV_STARTX * IMAGE_SENSOR_H_DECIMATION)
#define IMX179MIPI_IMAGE_SENSOR_CAP_STARTY		4   //(IMX179MIPI_IMAGE_SENSOR_PV_STARTY * IMAGE_SENSOR_V_DECIMATION)		// The value must bigger or equal than 1.
#define IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTX       2
#define IMX179MIPI_IMAGE_SENSOR_VIDEO_STARTY       2	// The value must bigger or equal than 1.

#define IMX179MIPI_IMAGE_SENSOR_CCT_WIDTH		(3284)
#define IMX179MIPI_IMAGE_SENSOR_CCT_HEIGHT		(2462)
#if 0
#define IMX179MIPI_PV_LINE_LENGTH_PIXELS 						(3400)
#define IMX179MIPI_PV_FRAME_LENGTH_LINES						(2150)	
#define IMX179MIPI_FULL_LINE_LENGTH_PIXELS 						(3440)
#define IMX179MIPI_FULL_FRAME_LENGTH_LINES			            (2478)
#define IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS 					(3432)
#define IMX179MIPI_VIDEO_FRAME_LENGTH_LINES						(2140)
#endif
#define IMX179MIPI_PV_LINE_LENGTH_PIXELS 						(3440)
#define IMX179MIPI_PV_FRAME_LENGTH_LINES						(2512)	
#define IMX179MIPI_FULL_LINE_LENGTH_PIXELS 						(3440)
#define IMX179MIPI_FULL_FRAME_LENGTH_LINES			            (3140)

#ifndef USE_PREVIW_FOR_VIDEO
#define IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS 					(3440)
#define IMX179MIPI_VIDEO_FRAME_LENGTH_LINES						(2512)
#else
#define IMX179MIPI_VIDEO_LINE_LENGTH_PIXELS 					(3440) // 720p=(3400)
#define IMX179MIPI_VIDEO_FRAME_LENGTH_LINES						(2512) // 720p=(2510)
#endif


//#define IMX179MIPI_SHUTTER_LINES_GAP	  3

#define IMX179MIPI_WRITE_ID (0x20)
#define IMX179MIPI_READ_ID	(0x21)

/* SENSOR PRIVATE STRUCT */
struct IMX179_SENSOR_STRUCT
{
	kal_uint8 i2c_write_id;
	kal_uint8 i2c_read_id;

};
struct IMX179MIPI_sensor_STRUCT
	{	 
		  kal_uint16 i2c_write_id;
		  kal_uint16 i2c_read_id;
		  kal_bool first_init;
		  kal_bool fix_video_fps;
		  kal_bool pv_mode; 
		  kal_bool video_mode; 				
		  kal_bool capture_mode; 				//True: Preview Mode; False: Capture Mode
		  kal_bool night_mode;				//True: Night Mode; False: Auto Mode
		  kal_uint8 mirror_flip;
		  kal_uint32 pv_pclk;				//Preview Pclk
		  kal_uint32 video_pclk;				//video Pclk
		  kal_uint32 cp_pclk;				//Capture Pclk
		  kal_uint32 pv_shutter;		   
		  kal_uint32 video_shutter;		   
		  kal_uint32 cp_shutter;
		  kal_uint32 pv_gain;
		  kal_uint32 video_gain;
		  kal_uint32 cp_gain;
		  kal_uint32 pv_line_length;
		  kal_uint32 pv_frame_length;
		  kal_uint32 video_line_length;
		  kal_uint32 video_frame_length;
		  kal_uint32 cp_line_length;
		  kal_uint32 cp_frame_length;
		  kal_uint16 pv_dummy_pixels;		   //Dummy Pixels:must be 12s
		  kal_uint16 pv_dummy_lines;		   //Dummy Lines
		  kal_uint16 video_dummy_pixels;		   //Dummy Pixels:must be 12s
		  kal_uint16 video_dummy_lines;		   //Dummy Lines
		  kal_uint16 cp_dummy_pixels;		   //Dummy Pixels:must be 12s
		  kal_uint16 cp_dummy_lines;		   //Dummy Lines			
		  kal_uint16 video_current_frame_rate;
	};
// SENSOR CHIP VERSION
#define IMX179MIPI_SENSOR_ID            IMX179_SENSOR_ID
#define IMX179MIPI_PAGE_SETTING_REG    (0xFF)

UINT32 IMX179MIPIOpen(void);
UINT32 IMX179MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 IMX179MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 IMX179MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 IMX179MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 IMX179MIPIClose(void);

#endif /* __SENSOR_H */

