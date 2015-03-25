/*****************************************************************************
 *
 * Filename:
 * ---------
 *   SP0A19yuv_Sensor.h
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver declare and macro define in the header file.
 *
 * Author:
 * -------
 *   Mormo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2011/10/25 Firsty Released By Mormo;
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
 
#ifndef __SP0A19_SENSOR_H
#define __SP0A19_SENSOR_H

typedef struct
{
  UINT16  iSensorVersion;
  UINT16  iNightMode;
  UINT16  iWB;
  UINT16  iEffect;
  UINT16  iEV;
  UINT16  iBanding;
  UINT16  iMirror;
  UINT16  iFrameRate;
  UINT16  iBrightness;
  UINT16  iIso;
} SP0A19_Status;
SP0A19_Status SP0A19_CurrentStatus;


#define SP0A19_TEST_PATTERN_CHECKSUM (0x7d732767)

#define SP0A19_VGA_PERIOD_PIXEL_NUMS					784
#define SP0A19_VGA_PERIOD_LINE_NUMS						510

#define SP0A19_IMAGE_SENSOR_START_PIXELS			2
#define SP0A19_IMAGE_SENSOR_START_LINES				2


#define SP0A19_IMAGE_SENSOR_PV_WIDTH					(640-8)
#define SP0A19_IMAGE_SENSOR_PV_HEIGHT					(480-6)

#define SP0A19_IMAGE_SENSOR_VIDEO_WIDTH					(640-8)
#define SP0A19_IMAGE_SENSOR_VIDEO_HEIGHT				(480-6)

#define SP0A19_IMAGE_SENSOR_FULL_WIDTH					(640-8)
#define SP0A19_IMAGE_SENSOR_FULL_HEIGHT					(480-6)

#define SP0A19_WRITE_ID							0x42
#define SP0A19_READ_ID							0x43
#define SP0A19_SENSOR_ID                        0xa6
typedef enum
{
	SP0A19_RGB_Gamma_m1 = 0,
	SP0A19_RGB_Gamma_m2,
	SP0A19_RGB_Gamma_m3,
	SP0A19_RGB_Gamma_m4,
	SP0A19_RGB_Gamma_m5, 
	SP0A19_RGB_Gamma_m6,
	SP0A19_RGB_Gamma_night
}SP0A19_GAMMA_TAG;

typedef enum
{
	CHT_806C_2 = 1,
	CHT_808C_2,
	LY_982A_H114,
	XY_046A,
	XY_0620,
	XY_078V,
	YG1001A_F
}SP0A19_LENS_TAG;

UINT32 SP0A19Open(void);
UINT32 SP0A19Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 SP0A19FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 SP0A19GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 SP0A19GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 SP0A19Close(void);

#endif /* __SENSOR_H */

