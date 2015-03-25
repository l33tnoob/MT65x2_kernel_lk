#ifndef __META_SDCARD_H__
#define __META_SDCARD_H__

#include "meta_common.h"
#include "meta_sdcard_para.h"
#include "FT_Public.h"

#ifdef MTK_EMMC_SUPPORT
#define MAX_NUM_SDCARDS     (4)
#define MIN_SDCARD_IDX      (0)
#else
#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (1)

#endif

#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "META_SDCARD"



#define META_SDCARD_LOG(...) \
    do { \
        LOGD(__VA_ARGS__); \
    } while (0)

#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)


typedef void (*SDCARD_CNF_CB)(SDCARD_CNF *cnf);

extern void Meta_SDcard_Register(SDCARD_CNF_CB callback);

#endif 

