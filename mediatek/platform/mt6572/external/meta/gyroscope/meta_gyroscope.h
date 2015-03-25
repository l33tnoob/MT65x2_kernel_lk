#ifndef __META_GYROSCOPE_H__
#define __META_GYROSCOPE_H__
#include "meta_common.h"
#include "FT_Public.h"
#include "meta_gyroscope_para.h"
#include "WM2Linux.h"


typedef void (*GYRO_CNF_CB)(GYRO_CNF *cnf);
extern void Meta_Gyroscope_Register(GYRO_CNF_CB callback);

#endif 

