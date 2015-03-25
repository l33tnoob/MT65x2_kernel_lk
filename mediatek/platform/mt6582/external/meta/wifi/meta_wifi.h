#ifndef __META_WIFI_H__
#define __META_WIFI_H__
#include "meta_common.h"
#include "meta_wifi_para.h"
#include "FT_Public.h"

typedef void (*WIFI_CNF_CB)(FT_WM_WIFI_CNF *cnf, void *buf, unsigned int size);

extern void META_WIFI_Register(WIFI_CNF_CB callback);

#endif 

