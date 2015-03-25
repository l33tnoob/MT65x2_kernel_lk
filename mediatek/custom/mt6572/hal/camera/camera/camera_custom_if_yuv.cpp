#include <stdlib.h>
#include <stdio.h>
#include "camera_custom_if.h"
#include "camera_custom_if_yuv.h"

namespace NSCamCustom
{

void custom_GetYuvFLParam(MINT32 i4ParId, NSCamCustom::YUV_FL_PARAM_T& rYuvFlParam)
{
    switch (i4ParId)
    {
    default:
    case 1:
        rYuvFlParam.dFlashlightThreshold            = NSCamCustom::custom_GetYuvFlashlightThreshold();
        rYuvFlParam.i4FlashlightDuty                = NSCamCustom::custom_GetYuvFlashlightDuty();
        rYuvFlParam.i4FlashlightStep                = NSCamCustom::custom_GetYuvFlashlightStep();
        rYuvFlParam.i4FlashlightFrameCnt            = NSCamCustom::custom_GetYuvFlashlightFrameCnt();
        rYuvFlParam.i4FlashlightPreflashAF          = NSCamCustom::custom_GetYuvPreflashAF();
        rYuvFlParam.i4FlashlightGain10X             = NSCamCustom::custom_GetFlashlightGain10X();
        rYuvFlParam.i4FlashlightHighCurrentDuty     = NSCamCustom::custom_GetYuvFlashlightHighCurrentDuty();
        rYuvFlParam.i4FlashlightHighCurrentTimeout  = NSCamCustom::custom_GetYuvFlashlightHighCurrentTimeout();
        rYuvFlParam.i4FlashlightAfLampSupport       = NSCamCustom::custom_GetYuvAfLampSupport();
        break;
    case 2:
        rYuvFlParam.dFlashlightThreshold            = 3.0;
        rYuvFlParam.i4FlashlightDuty                = 10;
        rYuvFlParam.i4FlashlightStep                = 7;
        rYuvFlParam.i4FlashlightFrameCnt            = 8;
        rYuvFlParam.i4FlashlightPreflashAF          = 0;
        rYuvFlParam.i4FlashlightGain10X             = 10;
        rYuvFlParam.i4FlashlightHighCurrentDuty     = 0xFF;
        rYuvFlParam.i4FlashlightHighCurrentTimeout  = 400;
        rYuvFlParam.i4FlashlightAfLampSupport       = 1;
        break;
    };
}

}
