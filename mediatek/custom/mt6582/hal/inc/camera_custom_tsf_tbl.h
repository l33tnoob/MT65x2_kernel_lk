#ifndef _CAMERA_CUSTOM_TSF_TBL_H_
#define _CAMERA_CUSTOM_TSF_TBL_H_

typedef struct
{
    int TSF_para[409];
} CAMERA_TSF_PARA_STRUCT, *PCAMERA_TSF_PARA_STRUCT;


typedef struct
{
    unsigned int tsf_data[16000];
} CAMERA_TSF_DATA_STRUCT, *PCAMERA_TSF_DATA_STRUCT;

typedef struct
{
    int TSF_PARA[409];
    unsigned int TSF_DATA[16000];
} CAMERA_TSF_TBL_STRUCT, *PCAMERA_TSF_TBL_STRUCT;

#endif //_CAMERA_CUSTOM_TSF_TBL_H_

