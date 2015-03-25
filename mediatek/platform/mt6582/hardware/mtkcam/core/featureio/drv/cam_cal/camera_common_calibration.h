#ifndef __CAMERA_COMMON_CALIBRATION_H__
#define __CAMERA_COMMON_CALIBRATION_H__



#include "camera_custom_cam_cal.h"


////for SLIM TABLE>>>
/////////////Protected
#if 0 //121017 for 658x

typedef struct {
	unsigned int reg_info0;
	unsigned int reg_info1;
	unsigned int *src_tbl_addr;
	unsigned int *dst_tbl_addr;
} TBL_INFO_T;

#define CAM_CAL_LSC_ID_SLIM 0x010200FF
#define CAM_CAL_LSC_ID_DYNAMIC 0x31520000
#define CAM_CAL_LSC_ID_FIX 0x39333236
#define CAM_CAL_LSC_ID_SENSOR 0x39333236
#define CAM_CAL_LSC_ID_SVD 0x010000FF

#define CAM_CAL_LSC_ID_NO 5

typedef struct
{
	MUINT32 ShadingID; 
	MUINT32 (*LscConvert)(PCAM_CAL_SHADING_STRUCT pLscTbl);
} CAM_COMM_LSC_CONVERT;


MUINT32 CamCommShadingTableConvert(PCAM_CAL_SHADING_STRUCT pLscTbl);
/////////////Protected
#endif

#endif
///<<<
///for SLIM TABLE<<<
