/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _FLASH_AWB_PARAM_H
#define _FLASH_AWB_PARAM_H

#include <camera_custom_nvram.h>

//#define FLASH_AWB_PC_SIM

typedef struct
{
    MUINT32 ForeGroundPercentage; //foreground percentage, 0~50
    MUINT32 BackGroundPercentage; //background percentage, 50~100
    
//Rule: map1_th1 < map1_th2 < map1_th3 < map1_th4 (//100~1000)
//Rule: map1_th1_val <= map1_th2_val <= map1_th3_val <= map1_th4_val (//100~1000)
    MUINT32 map1_th1;
    MUINT32 map1_th2;
    MUINT32 map1_th3;
    MUINT32 map1_th4;    
    MUINT32 map1_th1_val;
    MUINT32 map1_th2_val;
    MUINT32 map1_th3_val;
    MUINT32 map1_th4_val;

//Rule: map2_th1 < map2_th1 < map2_th3 < map2_th4 (//100~1000)
//Rule: map2_th1_val <= map2_th2_val <= map2_th3_val <= map2_th4_val (//100~1000) 
    MUINT32 map2_th1;
    MUINT32 map2_th2;
    MUINT32 map2_th3;
    MUINT32 map2_th4;
    MUINT32 map2_th1_val;
    MUINT32 map2_th2_val;
    MUINT32 map2_th3_val;
    MUINT32 map2_th4_val;

//Rule: location_map_th1 < location_map_th2 < location_map_th3 <location_map_th4 (//100~1000) 
//Rule: location_map_val1 <= location_map_val2 <= location_map_val3 <= location_map_val4 (//100~1000) 
	MUINT32 location_map_th1;
	MUINT32	location_map_th2;
	MUINT32 location_map_th3;
	MUINT32 location_map_th4;
	MUINT32 location_map_val1;
	MUINT32 location_map_val2;
	MUINT32 location_map_val3;
	MUINT32 location_map_val4;

    MUINT32 min_flash_y; //0~255

}FLASH_AWB_TUNING_PARAM_T;

typedef struct
{
    AWB_GAIN_T NoFlashWBGain;
    AWB_GAIN_T PureFlashWBGain;
    MUINT32 PureFlashWeight;
}FLASH_AWB_CAL_GAIN_INPUT_T;

typedef struct
{
    MINT32 flashDuty;
    MINT32 flashStep;
    MUINT32 flashAwbWeight;
}FLASH_AWB_PASS_FLASH_INFO_T;


#define FLASH_DUTY_NUM (64)
typedef struct
{
    FLASH_AWB_TUNING_PARAM_T flash_awb_tuning_param;
    //AWB_GAIN_T flashWBGain[FLASH_DUTY_NUM];   //Flash AWB calibration data
}FLASH_AWB_INIT_T, *PFLASH_AWB_INIT_T;

typedef struct
{
    AWB_GAIN_T flashWBGain[FLASH_DUTY_NUM]; // Flash AWB calibration data
} FLASH_AWB_CALIBRATION_DATA_STRUCT, *PFLASH_AWB_CALIBRATION_DATA_STRUCT;


typedef struct
{
    MUINT32 Hr;
    MUINT32 Mr;
    MUINT32 Lr;
    MUINT32 Midx;
}FLASH_AWB_DISTANCE_INFO_T;


typedef struct
{
    MUINT32 x1;
    MUINT32 x2;
    MUINT32 x3;
    MUINT32 x4;

    MUINT32 y1;
    MUINT32 y2;
    MUINT32 y3;
    MUINT32 y4;
}FLASH_AWB_FINAL_WEIGHT_LUT;


typedef struct
{
    int* pEstNoFlashY;
    int* pEstFlashY;
    double* aeCoef;
    double* pureCoef;
    
    int i4SceneLV; // current scene LV

    int FlashDuty;
    int FlashStep;
    
}FLASH_AWB_INPUT_T;



typedef struct
{
	AWB_GAIN_T rAWBGain;
} FLASH_AWB_OUTPUT_T;

#endif
