#ifndef __CAMERA_COMMON_CALIBRATION_H__
#define __CAMERA_COMMON_CALIBRATION_H__



#include "camera_custom_eeprom.h"


////for SLIM TABLE>>>
/////////////Protected


typedef struct {
    unsigned int reg_mn;
	unsigned int reg_info0;
	unsigned int reg_info1;
	unsigned int *src_tbl_addr;
	unsigned int *dst_tbl_addr;
} TBL_INFO_T;

#define EEPROM_LSC_ID_SLIM 0x010200FF
#define EEPROM_LSC_ID_DYNAMIC 0x31520000
#define EEPROM_LSC_ID_FIX 0x39333236
#define EEPROM_LSC_ID_SENSOR 0x39333236
#define EEPROM_LSC_ID_SVD 0x010000FF

#define EEPROM_LSC_ID_NO 5

typedef struct
{
	MUINT32 ShadingID;
	MUINT32 (*LscConvert)(PEEPROM_SHADING_STRUCT pLscTbl);
} CAM_COMM_LSC_CONVERT;


MUINT32 CamCommShadingTableConvert(PEEPROM_SHADING_STRUCT pLscTbl);
/////////////Protected

#endif


///<<<


///for SLIM TABLE<<<
