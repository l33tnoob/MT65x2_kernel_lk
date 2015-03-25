#ifndef __DDP_PATH_H__
#define __DDP_PATH_H__

#include "ddp_ovl.h"
#include "ddp_rdma.h"
#include "ddp_wdma.h"
#include "ddp_bls.h"

typedef enum DISP_MODULE_ENUM_
{
    DISP_MODULE_ROT = 0,
    DISP_MODULE_SCL,
    DISP_MODULE_OVL,
    DISP_MODULE_OVL_PQ,
    DISP_MODULE_COLOR,
    DISP_MODULE_TDSHP,  // 5
    DISP_MODULE_BLS,
    DISP_MODULE_WDMA0, 
    DISP_MODULE_WDMA1,
    DISP_MODULE_RDMA0,
    DISP_MODULE_RDMA1,  // 10
    DISP_MODULE_GAMMA,
    DISP_MODULE_DBI,
    DISP_MODULE_DPI0,
    DISP_MODULE_DSI,
    DISP_MODULE_DPI1,   // 15
    DISP_MODULE_CONFIG,
    DISP_MODULE_MUTEX,
    DISP_MODULE_CMDQ,
    DISP_MODULE_DSI_VDO,
    DISP_MODULE_DSI_CMD,
    DISP_MODULE_MAX
} DISP_MODULE_ENUM;

int disp_dump_reg(DISP_MODULE_ENUM module);

#define DDP_OVL_LAYER_MUN 4

struct DISP_REGION
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
};

struct disp_path_config_struct
{
    DISP_MODULE_ENUM srcModule;

	// if srcModule=RDMA0, set following value, else do not have to set following value
    unsigned int addr; 
    unsigned int inFormat; 
    unsigned int pitch;
    struct DISP_REGION srcROI;        // ROI

    OVL_CONFIG_STRUCT ovl_config;

    struct DISP_REGION bgROI;         // background ROI
    unsigned int bgColor;  // background color

    DISP_MODULE_ENUM dstModule;
    unsigned int outFormat; 
    unsigned int dstAddr;  // only take effect when dstModule=DISP_MODULE_WDMA1
};


int disp_wait_timeout(BOOL flag, unsigned int timeout);
int disp_delay_timeout(BOOL flag, unsigned int delay_ms);
int disp_path_config(struct disp_path_config_struct* pConfig);
int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig);
int disp_path_config_layer_addr(unsigned int layer, unsigned int addr);
int disp_path_get_mutex();
int disp_path_release_mutex();

int disp_path_ddp_clock_on();
int disp_path_ddp_clock_off();

#endif
