//
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
int cust_getPrecapAfMode()
{
    XLOGD("cust_getPrecapAfMode");
    //return e_PrecapAf_AfterPreflash;
    return  e_PrecapAf_None;
}


int cust_isNeedDoPrecapAF_v2(int isFocused, int flashMode, int afLampMode, int isBvLowerTriger)
{
    if(isBvLowerTriger==1 && flashMode!=LIB3A_FLASH_MODE_FORCE_OFF)
        return 1;
    else
        return 0;
}