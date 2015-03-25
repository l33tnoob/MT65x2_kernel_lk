
#ifndef __SENSOR_H
#define __SENSOR_H


#define HI256_SENSOR_ID         (0xc0)

#define HI256_WRITE_ID_0		(0x40)
#define HI256_READ_ID_0			(0x41)

#define HI256_WRITE_ID_1		(0x40)
#define HI256_READ_ID_1			(0x41)

#define HI256_PV_PERIOD_PIXEL_NUMS			(HI256_FULL_PERIOD_PIXEL_NUMS / 2)
#define HI256_PV_PERIOD_LINE_NUMS			(600)
#define HI256_FULL_PERIOD_PIXEL_NUMS		(1600)	
#define HI256_FULL_PERIOD_LINE_NUMS 		(1200)

#define HI256_PV_EXPOSURE_LIMITATION		(800)
#define HI256_FULL_EXPOSURE_LIMITATION		(1200)

#define HI256_IMAGE_SENSOR_PV_WIDTH   		(800) 
#define HI256_IMAGE_SENSOR_PV_HEIGHT  		(600) 

#define HI256_IMAGE_SENSOR_FULL_WIDTH     	(1600)
#define HI256_IMAGE_SENSOR_FULL_HEIGHT    	(1200)

/*ergate-004*/
#define HI256_PV_GRAB_START_X 				(4) //(1) 
#define HI256_PV_GRAB_START_Y  				(3) //(2) 
#define HI256_PV_GRAB_WIDTH					(HI256_IMAGE_SENSOR_PV_WIDTH - 8) 
#define HI256_PV_GRAB_HEIGHT				(HI256_IMAGE_SENSOR_PV_HEIGHT - 6) 

#define HI256_FULL_GRAB_START_X   			(16)
#define HI256_FULL_GRAB_START_Y	  			(12)
#define HI256_FULL_GRAB_WIDTH				(HI256_IMAGE_SENSOR_FULL_WIDTH - 32)
#define HI256_FULL_GRAB_HEIGHT				(HI256_IMAGE_SENSOR_FULL_HEIGHT - 24)

static kal_uint8 HI256_sccb_addr[] = 
{
	HI256_WRITE_ID_0,			
	HI256_WRITE_ID_1,			
};

typedef struct
{
kal_uint16	video_target_width;
kal_uint16	video_target_height;

kal_bool	MJPEG_encode_mode;			/* Motion JPEG */
kal_bool	MPEG4_encode_mode;			/* MJPEG4 JPEG */
kal_bool	FULLVIDEO_encode_mode;		/* 3G Video Call */

kal_bool	sensor_cap_state;			/* Preview or Capture mode */
kal_bool	is_PV_mode; 				/* PV size or Full size */
kal_bool	is_panorama_capturing;		/* 3G Video Call */

kal_uint32	curr_banding;				/* 50Hz/60Hz */
kal_bool	night_mode;
} HI256_OPERATION_STATE_ST;
	
typedef struct
{
kal_uint8	sccb_write_id;
kal_uint8	sccb_read_id;

kal_uint32	pv_shutter;
kal_uint32	pv_extra_shutter;
kal_uint32	pv_sensor_gain;

kal_uint32	pv_dummy_pixels;
kal_uint32	pv_dummy_lines;
kal_uint32	cap_dummy_pixels;
kal_uint32	cap_dummy_lines;

/* Preview & Capture Pixel Clock, 360 means 36.0MHz. Unit Multiple 10. */
kal_uint32	preview_pclk;
kal_uint32	capture_pclk;

/* Video frame rate 300 means 30.0fps. Unit Multiple 10. */
kal_uint32	video_frame_rate;	
}HI256_SENSOR_INFO_ST;

UINT32 HI256Open(void);
UINT32 HI256GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 HI256GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HI256Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HI256FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 HI256Close(void);

#endif 
