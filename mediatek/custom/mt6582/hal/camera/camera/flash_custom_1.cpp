
#include "camera_custom_nvram.h"
#include "camera_custom_types.h"

#include "camera_custom_AEPlinetable.h"
#include "camera_custom_nvram.h"

#include <cutils/xlog.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int cust_getFlashModeStyle(int sensorType, int flashMode)
{

    //if(sensorType==(int)DUAL_CAMERA_MAIN_SENSOR)

    /*
    e_FLASH_STYLE_OFF_AUTO = 0,
    e_FLASH_STYLE_OFF_ON,
    e_FLASH_STYLE_OFF_OFF,
    e_FLASH_STYLE_ON_ON,
    e_FLASH_STYLE_ON_TORCH, */

    if(flashMode==LIB3A_FLASH_MODE_AUTO)
	{
		return e_FLASH_STYLE_OFF_AUTO;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_ON)
	{
		return e_FLASH_STYLE_OFF_ON;
		//return e_FLASH_STYLE_ON_ON;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_OFF)
	{
		return e_FLASH_STYLE_OFF_OFF;
	}
	else if(flashMode==LIB3A_FLASH_MODE_REDEYE)
	{
		return e_FLASH_STYLE_OFF_AUTO;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_TORCH)
	{
		return e_FLASH_STYLE_ON_ON;
	}
	return e_FLASH_STYLE_OFF_AUTO;
}
int cust_getVideoFlashModeStyle(int sensorType, int flashMode)
{
    /*
    e_FLASH_STYLE_OFF_AUTO = 0,
    e_FLASH_STYLE_OFF_ON,
    e_FLASH_STYLE_OFF_OFF,
    e_FLASH_STYLE_ON_ON,
    e_FLASH_STYLE_ON_TORCH, */

    if(flashMode==LIB3A_FLASH_MODE_AUTO)
	{
		return e_FLASH_STYLE_OFF_AUTO;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_ON)
	{
		return e_FLASH_STYLE_ON_ON;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_OFF)
	{
		return e_FLASH_STYLE_OFF_OFF;
	}
	else if(flashMode==LIB3A_FLASH_MODE_REDEYE)
	{
		return e_FLASH_STYLE_OFF_OFF;
	}
	else if(flashMode==LIB3A_FLASH_MODE_FORCE_TORCH)
	{
		return e_FLASH_STYLE_ON_ON;
	}
	return e_FLASH_STYLE_OFF_OFF;
}


float evX[5] = {-3, -1.5, 0, 1.5, 3};
float evY[5] = {-2, -1, 0, 1, 2};
float evL[5] = {0,   0,   0, 3, 5};
void cust_getEvCompPara(int& maxEvTar10Bit, int& indNum, float*& evIndTab, float*& evTab, float*& evLevel)
{
    maxEvTar10Bit=600;
    indNum = 5;
    evIndTab = evX;
    evTab = evY;
    evLevel = evL;

}
