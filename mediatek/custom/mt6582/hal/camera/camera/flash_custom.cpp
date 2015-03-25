#define LOG_TAG "flash_custom.cpp"
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_nvram.h"
#include <cutils/xlog.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>



static int g_mainId=0;
static int g_subId=0;

void cust_setFlashPartId_main(int id)
{
	g_mainId=id;
}
void cust_setFlashPartId_sub(int id)
{
	g_subId=id;
}
//FLASH_PROJECT_PARA& cust_getFlashProjectPara_mainV2(int AEMode, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
FLASH_PROJECT_PARA& cust_getFlashProjectPara_V2(int sensorDev, int AEMode, NVRAM_CAMERA_STROBE_STRUCT* nvrame)
{
	if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
	{
		if(g_mainId==1)
			return cust_getFlashProjectPara(AEMode, nvrame);
		else //if(id==2)
			return cust_getFlashProjectPara_main2(AEMode, nvrame);
	}
	else
	{
		if(g_subId==1)
			return cust_getFlashProjectPara_sub(AEMode, nvrame);
		else //if(id==2)
			return cust_getFlashProjectPara_sub2(AEMode, nvrame);
	}
}
int cust_getDefaultStrobeNVRam_V2(int sensorDev, void* data, int* ret_size)
{
	if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
	{
		if(g_mainId==1)
		{
			XLOGD("devid main id1");
			return getDefaultStrobeNVRam(DUAL_CAMERA_MAIN_SENSOR, data, ret_size);
		}
		else
		{
			XLOGD("devid main id2");
			return getDefaultStrobeNVRam_main2(data, ret_size);
		}
	}
	else
	{
		if(g_subId==1)
		{
			XLOGD("devid sub id1");
			return getDefaultStrobeNVRam(DUAL_CAMERA_SUB_SENSOR, data, ret_size);
		}
		else
		{
			XLOGD("devid sub id2");
			return getDefaultStrobeNVRam_sub2(data, ret_size);
		}

	}
}


