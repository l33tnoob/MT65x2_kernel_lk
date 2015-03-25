#ifndef _CAMERA_CUSTOM_IF_YUV_H_
#define _CAMERA_CUSTOM_IF_YUV_H_
//
#include "camera_custom_types.h"

//
namespace NSCamCustom
{

typedef struct
{
    double dFlashlightThreshold;
    MINT32 i4FlashlightDuty;
    MINT32 i4FlashlightStep;
    MINT32 i4FlashlightFrameCnt;
    MINT32 i4FlashlightPreflashAF;
    MINT32 i4FlashlightGain10X;
    MINT32 i4FlashlightHighCurrentDuty;
    MINT32 i4FlashlightHighCurrentTimeout;
    MINT32 i4FlashlightAfLampSupport;
} YUV_FL_PARAM_T;

void custom_GetYuvFLParam(MINT32 i4ParId, YUV_FL_PARAM_T& rYuvFlParam);

};  //NSCamCustom
#endif  //  _CAMERA_CUSTOM_IF_YUV_H_


