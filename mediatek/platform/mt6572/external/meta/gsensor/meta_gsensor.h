#ifndef __META_GPIO_H__
#define __META_GPIO_H__
#include "meta_common.h"
#include "FT_Public.h"
#include "meta_gsensor_para.h"
#include "WM2Linux.h"
#endif 

typedef void (*GS_CNF_CB)(GS_CNF *cnf);
extern void Meta_GSensor_Register(GS_CNF_CB callback);

