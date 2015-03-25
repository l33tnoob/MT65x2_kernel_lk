#ifndef __DROVL_H__
#define __DROVL_H__

#include "drDpDataType.h"

// ddp_ovl.h
#define OVL_MAX_WIDTH  1920
#define OVL_MAX_HEIGHT 1920
#define OVL_LAYER_NUM  4

// ddp_hal.h
enum OVL_LAYER_SOURCE {
    OVL_LAYER_SOURCE_MEM    = 0,
    OVL_LAYER_SOURCE_RESERVED = 1,
    OVL_LAYER_SOURCE_SCL     = 2,
    OVL_LAYER_SOURCE_PQ     = 3,
};

typedef struct{
    unsigned int layer_en[OVL_LAYER_NUM];
    unsigned int addr[OVL_LAYER_NUM];
    unsigned int size[OVL_LAYER_NUM];
} OVL_LAYER_INFO;

// ddp_hal.h
typedef struct _OVL_CONFIG_STRUCT
{
    unsigned int layer;
    unsigned int layer_en;
    enum OVL_LAYER_SOURCE source;
    DpColorFormat fmt;
    unsigned int addr;
    unsigned int vaddr;
    unsigned int src_x;
    unsigned int src_y;
    unsigned int src_w;
    unsigned int src_h;
    unsigned int src_pitch;
    unsigned int dst_x;
    unsigned int dst_y;
    unsigned int dst_w;
    unsigned int dst_h;                  // clip region
    unsigned int keyEn;
    unsigned int key;
    unsigned int aen;
    unsigned char alpha;

    unsigned int isTdshp;
    unsigned int isDirty;

    int buff_idx;
    int identity;
    int connected_type;
    unsigned int security;
} OVL_CONFIG_STRUCT;

// translate the input buffer to OVL_CONFIG_STRUCT value
// and call OVLLayerConfig
int OVLLayerConfig_svp(void* data, unsigned int len);

// call OVLLayerSwitch
int OVLLayerSwitch_svp(unsigned int layer, int en);

int OVLLayerConfig_debug_buffer(void* buffer_mva, unsigned int len);

int OVLReset(void);
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
#define STA_FLD_RDMA3_RST_PERIOD                               REG_FLD(1, 4)
#define STA_FLD_RDMA2_RST_PERIOD                               REG_FLD(1, 3)
#define STA_FLD_RDMA1_RST_PERIOD                               REG_FLD(1, 2)
#define STA_FLD_RDMA0_RST_PERIOD                               REG_FLD(1, 1)
#define STA_FLD_OVL_RUN                                        REG_FLD(1, 0)

#define INTEN_FLD_RDMA3_FIFO_UNDERFLOW_INTEN                   REG_FLD(1, 11)
#define INTEN_FLD_RDMA2_FIFO_UNDERFLOW_INTEN                   REG_FLD(1, 10)
#define INTEN_FLD_RDMA1_FIFO_UNDERFLOW_INTEN                   REG_FLD(1, 9)
#define INTEN_FLD_RDMA0_FIFO_UNDERFLOW_INTEN                   REG_FLD(1, 8)
#define INTEN_FLD_RDMA3_EOF_ABNORMAL_INTEN                     REG_FLD(1, 7)
#define INTEN_FLD_RDMA2_EOF_ABNORMAL_INTEN                     REG_FLD(1, 6)
#define INTEN_FLD_RDMA1_EOF_ABNORMAL_INTEN                     REG_FLD(1, 5)
#define INTEN_FLD_RDMA0_EOF_ABNORMAL_INTEN                     REG_FLD(1, 4)
#define INTEN_FLD_OVL_FME_SWRST_DONE_INTEN                     REG_FLD(1, 3)
#define INTEN_FLD_OVL_FME_UND_INTEN                            REG_FLD(1, 2)
#define INTEN_FLD_OVL_FME_CPL_INTEN                            REG_FLD(1, 1)
#define INTEN_FLD_OVL_REG_CMT_INTEN                            REG_FLD(1, 0)

#define INTSTA_FLD_RDMA3_FIFO_UNDERFLOW_INTSTA                 REG_FLD(1, 11)
#define INTSTA_FLD_RDMA2_FIFO_UNDERFLOW_INTSTA                 REG_FLD(1, 10)
#define INTSTA_FLD_RDMA1_FIFO_UNDERFLOW_INTSTA                 REG_FLD(1, 9)
#define INTSTA_FLD_RDMA0_FIFO_UNDERFLOW_INTSTA                 REG_FLD(1, 8)
#define INTSTA_FLD_RDMA3_EOF_ABNORMAL_INTSTA                   REG_FLD(1, 7)
#define INTSTA_FLD_RDMA2_EOF_ABNORMAL_INTSTA                   REG_FLD(1, 6)
#define INTSTA_FLD_RDMA1_EOF_ABNORMAL_INTSTA                   REG_FLD(1, 5)
#define INTSTA_FLD_RDMA0_EOF_ABNORMAL_INTSTA                   REG_FLD(1, 4)
#define INTSTA_FLD_OVL_FME_SWRST_DONE_INTSTA                   REG_FLD(1, 3)
#define INTSTA_FLD_OVL_FME_UND_INTSTA                          REG_FLD(1, 2)
#define INTSTA_FLD_OVL_FME_CPL_INTSTA                          REG_FLD(1, 1)
#define INTSTA_FLD_OVL_REG_CMT_INTSTA                          REG_FLD(1, 0)

#define EN_FLD_OVL_EN                                          REG_FLD(1, 0)

#define TRIG_FLD_OVL_SW_TRIG                                   REG_FLD(1, 0)

#define RST_FLD_OVL_RSTB                                       REG_FLD(1, 0)

#define ROI_SIZE_FLD_ROI_H                                     REG_FLD(12, 16)
#define ROI_SIZE_FLD_ROI_W                                     REG_FLD(12, 0)

#define ROI_BGCLR_FLD_ALPHA                                    REG_FLD(8, 24)
#define ROI_BGCLR_FLD_RED                                      REG_FLD(8, 16)
#define ROI_BGCLR_FLD_GREEN                                    REG_FLD(8, 8)
#define ROI_BGCLR_FLD_BLUE                                     REG_FLD(8, 0)

#define SRC_CON_FLD_L3_EN                                      REG_FLD(1, 3)
#define SRC_CON_FLD_L2_EN                                      REG_FLD(1, 2)
#define SRC_CON_FLD_L1_EN                                      REG_FLD(1, 1)
#define SRC_CON_FLD_L0_EN                                      REG_FLD(1, 0)

#define L0_CON_FLD_DSTKEY_EN                                   REG_FLD(1, 31)
#define L0_CON_FLD_SRCKEY_EN                                   REG_FLD(1, 30)
#define L0_CON_FLD_LAYER_SRC                                   REG_FLD(2, 28)
#define L0_CON_FLD_RGB_SWAP                                    REG_FLD(1, 25)
#define L0_CON_FLD_BYTE_SWAP                                   REG_FLD(1, 24)
#define L0_CON_FLD_R_FIRST                                     REG_FLD(1, 22)
#define L0_CON_FLD_LANDSCAPE                                   REG_FLD(1, 21)
#define L0_CON_FLD_EN_3D                                       REG_FLD(1, 20)
#define L0_CON_FLD_C_CF_SEL                                    REG_FLD(3, 16)
#define L0_CON_FLD_CLRFMT                                      REG_FLD(4, 12)
#define L0_CON_FLD_EXT_MTX_EN                                  REG_FLD(1, 11)
#define L0_CON_FLD_ALPHA_EN                                    REG_FLD(1, 8)
#define L0_CON_FLD_ALPHA                                       REG_FLD(8, 0)

#define L0_SRCKEY_FLD_SRCKEY                                   REG_FLD(32, 0)

#define L0_SRC_SIZE_FLD_L0_SRC_H                               REG_FLD(12, 16)
#define L0_SRC_SIZE_FLD_L0_SRC_W                               REG_FLD(12, 0)

#define L0_OFFSET_FLD_L0_YOFF                                  REG_FLD(12, 16)
#define L0_OFFSET_FLD_L0_XOFF                                  REG_FLD(12, 0)

#define L0_ADDR_FLD_L0_ADDR                                    REG_FLD(32, 0)

#define L0_PITCH_FLD_L0_SRC_PITCH                              REG_FLD(16, 0)

#define L1_CON_FLD_DSTKEY_EN                                   REG_FLD(1, 31)
#define L1_CON_FLD_SRCKEY_EN                                   REG_FLD(1, 30)
#define L1_CON_FLD_LAYER_SRC                                   REG_FLD(2, 28)
#define L1_CON_FLD_RGB_SWAP                                    REG_FLD(1, 25)
#define L1_CON_FLD_BYTE_SWAP                                   REG_FLD(1, 24)
#define L1_CON_FLD_R_FIRST                                     REG_FLD(1, 22)
#define L1_CON_FLD_LANDSCAPE                                   REG_FLD(1, 21)
#define L1_CON_FLD_EN_3D                                       REG_FLD(1, 20)
#define L1_CON_FLD_C_CF_SEL                                    REG_FLD(3, 16)
#define L1_CON_FLD_CLRFMT                                      REG_FLD(4, 12)
#define L1_CON_FLD_EXT_MTX_EN                                  REG_FLD(1, 11)
#define L1_CON_FLD_ALPHA_EN                                    REG_FLD(1, 8)
#define L1_CON_FLD_ALPHA                                       REG_FLD(8, 0)

#define L1_SRCKEY_FLD_SRCKEY                                   REG_FLD(32, 0)

#define L1_SRC_SIZE_FLD_L1_SRC_H                               REG_FLD(12, 16)
#define L1_SRC_SIZE_FLD_L1_SRC_W                               REG_FLD(12, 0)

#define L1_OFFSET_FLD_L1_YOFF                                  REG_FLD(12, 16)
#define L1_OFFSET_FLD_L1_XOFF                                  REG_FLD(12, 0)

#define L1_ADDR_FLD_L1_ADDR                                    REG_FLD(32, 0)

#define L1_PITCH_FLD_L1_SRC_PITCH                              REG_FLD(16, 0)

#define L2_CON_FLD_DSTKEY_EN                                   REG_FLD(1, 31)
#define L2_CON_FLD_SRCKEY_EN                                   REG_FLD(1, 30)
#define L2_CON_FLD_LAYER_SRC                                   REG_FLD(2, 28)
#define L2_CON_FLD_RGB_SWAP                                    REG_FLD(1, 25)
#define L2_CON_FLD_BYTE_SWAP                                   REG_FLD(1, 24)
#define L2_CON_FLD_R_FIRST                                     REG_FLD(1, 22)
#define L2_CON_FLD_LANDSCAPE                                   REG_FLD(1, 21)
#define L2_CON_FLD_EN_3D                                       REG_FLD(1, 20)
#define L2_CON_FLD_C_CF_SEL                                    REG_FLD(3, 16)
#define L2_CON_FLD_CLRFMT                                      REG_FLD(4, 12)
#define L2_CON_FLD_EXT_MTX_EN                                  REG_FLD(1, 11)
#define L2_CON_FLD_ALPHA_EN                                    REG_FLD(1, 8)
#define L2_CON_FLD_ALPHA                                       REG_FLD(8, 0)

#define L2_SRCKEY_FLD_SRCKEY                                   REG_FLD(32, 0)

#define L2_SRC_SIZE_FLD_L2_SRC_H                               REG_FLD(12, 16)
#define L2_SRC_SIZE_FLD_L2_SRC_W                               REG_FLD(12, 0)

#define L2_OFFSET_FLD_L2_YOFF                                  REG_FLD(12, 16)
#define L2_OFFSET_FLD_L2_XOFF                                  REG_FLD(12, 0)

#define L2_ADDR_FLD_L2_ADDR                                    REG_FLD(32, 0)

#define L2_PITCH_FLD_L2_SRC_PITCH                              REG_FLD(16, 0)

#define L3_CON_FLD_DSTKEY_EN                                   REG_FLD(1, 31)
#define L3_CON_FLD_SRCKEY_EN                                   REG_FLD(1, 30)
#define L3_CON_FLD_LAYER_SRC                                   REG_FLD(2, 28)
#define L3_CON_FLD_RGB_SWAP                                    REG_FLD(1, 25)
#define L3_CON_FLD_BYTE_SWAP                                   REG_FLD(1, 24)
#define L3_CON_FLD_R_FIRST                                     REG_FLD(1, 22)
#define L3_CON_FLD_LANDSCAPE                                   REG_FLD(1, 21)
#define L3_CON_FLD_EN_3D                                       REG_FLD(1, 20)
#define L3_CON_FLD_C_CF_SEL                                    REG_FLD(3, 16)
#define L3_CON_FLD_CLRFMT                                      REG_FLD(4, 12)
#define L3_CON_FLD_EXT_MTX_EN                                  REG_FLD(1, 11)
#define L3_CON_FLD_ALPHA_EN                                    REG_FLD(1, 8)
#define L3_CON_FLD_ALPHA                                       REG_FLD(8, 0)

#define L3_SRCKEY_FLD_SRCKEY                                   REG_FLD(32, 0)

#define L3_SRC_SIZE_FLD_L3_SRC_H                               REG_FLD(12, 16)
#define L3_SRC_SIZE_FLD_L3_SRC_W                               REG_FLD(12, 0)

#define L3_OFFSET_FLD_L3_YOFF                                  REG_FLD(12, 16)
#define L3_OFFSET_FLD_L3_XOFF                                  REG_FLD(12, 0)

#define L3_ADDR_FLD_L3_ADDR                                    REG_FLD(32, 0)

#define L3_PITCH_FLD_L3_SRC_PITCH                              REG_FLD(16, 0)

#define RDMA0_CTRL_FLD_RDMA0_TRIG_TYPE                         REG_FLD(1, 8)
#define RDMA0_CTRL_FLD_RDMA0_EN                                REG_FLD(1, 0)

#define RDMA0_MEM_START_TRIG_FLD_RDMA0_START_TRIG              REG_FLD(1, 0)

#define RDMA0_MEM_GMC_SETTING_FLD_RDMA0_DISEN_THRD             REG_FLD(10, 16)
#define RDMA0_MEM_GMC_SETTING_FLD_RDMA0_EN_THRD                REG_FLD(10, 0)

#define RDMA0_MEM_SLOW_CON_FLD_RDMA0_SLOW_CNT                  REG_FLD(16, 16)
#define RDMA0_MEM_SLOW_CON_FLD_RDMA0_SLOW_EN                   REG_FLD(1, 0)

#define RDMA0_FIFO_CTRL_FLD_RDMA0_FIFO_UND_EN                  REG_FLD(1, 31)
#define RDMA0_FIFO_CTRL_FLD_RDMA0_FIFO_SIZE                    REG_FLD(10, 16)
#define RDMA0_FIFO_CTRL_FLD_RDMA0_FIFO_THRD                    REG_FLD(10, 0)

#define RDMA1_CTRL_FLD_RDMA1_TRIG_TYPE                         REG_FLD(1, 8)
#define RDMA1_CTRL_FLD_RDMA1_EN                                REG_FLD(1, 0)

#define RDMA1_MEM_START_TRIG_FLD_RDMA1_START_TRIG              REG_FLD(1, 0)

#define RDMA1_MEM_GMC_SETTING_FLD_RDMA1_DISEN_THRD             REG_FLD(10, 16)
#define RDMA1_MEM_GMC_SETTING_FLD_RDMA1_EN_THRD                REG_FLD(10, 0)

#define RDMA1_MEM_SLOW_CON_FLD_RDMA1_SLOW_CNT                  REG_FLD(16, 16)
#define RDMA1_MEM_SLOW_CON_FLD_RDMA1_SLOW_EN                   REG_FLD(1, 0)

#define RDMA1_FIFO_CTRL_FLD_RDMA1_FIFO_UND_EN                  REG_FLD(1, 31)
#define RDMA1_FIFO_CTRL_FLD_RDMA1_FIFO_SIZE                    REG_FLD(10, 16)
#define RDMA1_FIFO_CTRL_FLD_RDMA1_FIFO_THRD                    REG_FLD(10, 0)

#define RDMA2_CTRL_FLD_RDMA2_TRIG_TYPE                         REG_FLD(1, 8)
#define RDMA2_CTRL_FLD_RDMA2_EN                                REG_FLD(1, 0)

#define RDMA2_MEM_START_TRIG_FLD_RDMA2_START_TRIG              REG_FLD(1, 0)

#define RDMA2_MEM_GMC_SETTING_FLD_RDMA2_DISEN_THRD             REG_FLD(10, 16)
#define RDMA2_MEM_GMC_SETTING_FLD_RDMA2_EN_THRD                REG_FLD(10, 0)

#define RDMA2_MEM_SLOW_CON_FLD_RDMA2_SLOW_CNT                  REG_FLD(16, 16)
#define RDMA2_MEM_SLOW_CON_FLD_RDMA2_SLOW_EN                   REG_FLD(1, 0)

#define RDMA2_FIFO_CTRL_FLD_RDMA2_FIFO_UND_EN                  REG_FLD(1, 31)
#define RDMA2_FIFO_CTRL_FLD_RDMA2_FIFO_SIZE                    REG_FLD(10, 16)
#define RDMA2_FIFO_CTRL_FLD_RDMA2_FIFO_THRD                    REG_FLD(10, 0)

#define RDMA3_CTRL_FLD_RDMA3_TRIG_TYPE                         REG_FLD(1, 8)
#define RDMA3_CTRL_FLD_RDMA3_EN                                REG_FLD(1, 0)

#define RDMA3_MEM_START_TRIG_FLD_RDMA3_START_TRIG              REG_FLD(1, 0)

#define RDMA3_MEM_GMC_SETTING_FLD_RDMA3_DISEN_THRD             REG_FLD(10, 16)
#define RDMA3_MEM_GMC_SETTING_FLD_RDMA3_EN_THRD                REG_FLD(10, 0)

#define RDMA3_MEM_SLOW_CON_FLD_RDMA3_SLOW_CNT                  REG_FLD(16, 16)
#define RDMA3_MEM_SLOW_CON_FLD_RDMA3_SLOW_EN                   REG_FLD(1, 0)

#define RDMA3_FIFO_CTRL_FLD_RDMA3_FIFO_UND_EN                  REG_FLD(1, 31)
#define RDMA3_FIFO_CTRL_FLD_RDMA3_FIFO_SIZE                    REG_FLD(10, 16)
#define RDMA3_FIFO_CTRL_FLD_RDMA3_FIFO_THRD                    REG_FLD(10, 0)

#define L0_Y2R_PARA_R0_FLD_C_CF_RMU                            REG_FLD(13, 16)
#define L0_Y2R_PARA_R0_FLD_C_CF_RMY                            REG_FLD(13, 0)

#define L0_Y2R_PARA_R1_FLD_C_CF_RMV                            REG_FLD(13, 0)

#define L0_Y2R_PARA_G0_FLD_C_CF_GMU                            REG_FLD(13, 16)
#define L0_Y2R_PARA_G0_FLD_C_CF_GMY                            REG_FLD(13, 0)

#define L0_Y2R_PARA_G1_FLD_C_CF_GMV                            REG_FLD(13, 0)

#define L0_Y2R_PARA_B0_FLD_C_CF_BMU                            REG_FLD(13, 16)
#define L0_Y2R_PARA_B0_FLD_C_CF_BMY                            REG_FLD(13, 0)

#define L0_Y2R_PARA_B1_FLD_C_CF_BMV                            REG_FLD(13, 0)

#define L0_Y2R_PARA_YUV_A_0_FLD_C_CF_UA                        REG_FLD(13, 16)
#define L0_Y2R_PARA_YUV_A_0_FLD_C_CF_YA                        REG_FLD(9, 0)

#define L0_Y2R_PARA_YUV_A_1_FLD_C_CF_VA                        REG_FLD(9, 0)

#define L0_Y2R_PARA_RGB_A_0_FLD_C_CF_GA                        REG_FLD(13, 16)
#define L0_Y2R_PARA_RGB_A_0_FLD_C_CF_RA                        REG_FLD(9, 0)

#define L0_Y2R_PARA_RGB_A_1_FLD_C_CF_BA                        REG_FLD(9, 0)

#define L1_Y2R_PARA_R0_FLD_C_CF_RMU                            REG_FLD(13, 16)
#define L1_Y2R_PARA_R0_FLD_C_CF_RMY                            REG_FLD(13, 0)

#define L1_Y2R_PARA_R1_FLD_C_CF_RMV                            REG_FLD(13, 0)

#define L1_Y2R_PARA_G0_FLD_C_CF_GMU                            REG_FLD(13, 16)
#define L1_Y2R_PARA_G0_FLD_C_CF_GMY                            REG_FLD(13, 0)

#define L1_Y2R_PARA_G1_FLD_C_CF_GMV                            REG_FLD(13, 0)

#define L1_Y2R_PARA_B0_FLD_C_CF_BMU                            REG_FLD(13, 16)
#define L1_Y2R_PARA_B0_FLD_C_CF_BMY                            REG_FLD(13, 0)

#define L1_Y2R_PARA_B1_FLD_C_CF_BMV                            REG_FLD(13, 0)

#define L1_Y2R_PARA_YUV_A_0_FLD_C_CF_UA                        REG_FLD(13, 16)
#define L1_Y2R_PARA_YUV_A_0_FLD_C_CF_YA                        REG_FLD(9, 0)

#define L1_Y2R_PARA_YUV_A_1_FLD_C_CF_VA                        REG_FLD(9, 0)

#define L1_Y2R_PARA_RGB_A_0_FLD_C_CF_GA                        REG_FLD(13, 16)
#define L1_Y2R_PARA_RGB_A_0_FLD_C_CF_RA                        REG_FLD(9, 0)

#define L1_Y2R_PARA_RGB_A_1_FLD_C_CF_BA                        REG_FLD(9, 0)

#define L2_Y2R_PARA_R0_FLD_C_CF_RMU                            REG_FLD(13, 16)
#define L2_Y2R_PARA_R0_FLD_C_CF_RMY                            REG_FLD(13, 0)

#define L2_Y2R_PARA_R1_FLD_C_CF_RMV                            REG_FLD(13, 0)

#define L2_Y2R_PARA_G0_FLD_C_CF_GMU                            REG_FLD(13, 16)
#define L2_Y2R_PARA_G0_FLD_C_CF_GMY                            REG_FLD(13, 0)

#define L2_Y2R_PARA_G1_FLD_C_CF_GMV                            REG_FLD(13, 0)

#define L2_Y2R_PARA_B0_FLD_C_CF_BMU                            REG_FLD(13, 16)
#define L2_Y2R_PARA_B0_FLD_C_CF_BMY                            REG_FLD(13, 0)

#define L2_Y2R_PARA_B1_FLD_C_CF_BMV                            REG_FLD(13, 0)

#define L2_Y2R_PARA_YUV_A_0_FLD_C_CF_UA                        REG_FLD(13, 16)
#define L2_Y2R_PARA_YUV_A_0_FLD_C_CF_YA                        REG_FLD(9, 0)

#define L2_Y2R_PARA_YUV_A_1_FLD_C_CF_VA                        REG_FLD(9, 0)

#define L2_Y2R_PARA_RGB_A_0_FLD_C_CF_GA                        REG_FLD(13, 16)
#define L2_Y2R_PARA_RGB_A_0_FLD_C_CF_RA                        REG_FLD(9, 0)

#define L2_Y2R_PARA_RGB_A_1_FLD_C_CF_BA                        REG_FLD(9, 0)

#define L3_Y2R_PARA_R0_FLD_C_CF_RMU                            REG_FLD(13, 16)
#define L3_Y2R_PARA_R0_FLD_C_CF_RMY                            REG_FLD(13, 0)

#define L3_Y2R_PARA_R1_FLD_C_CF_RMV                            REG_FLD(13, 0)

#define L3_Y2R_PARA_G0_FLD_C_CF_GMU                            REG_FLD(13, 16)
#define L3_Y2R_PARA_G0_FLD_C_CF_GMY                            REG_FLD(13, 0)

#define L3_Y2R_PARA_G1_FLD_C_CF_GMV                            REG_FLD(13, 0)

#define L3_Y2R_PARA_B0_FLD_C_CF_BMU                            REG_FLD(13, 16)
#define L3_Y2R_PARA_B0_FLD_C_CF_BMY                            REG_FLD(13, 0)

#define L3_Y2R_PARA_B1_FLD_C_CF_BMV                            REG_FLD(13, 0)

#define L3_Y2R_PARA_YUV_A_0_FLD_C_CF_UA                        REG_FLD(13, 16)
#define L3_Y2R_PARA_YUV_A_0_FLD_C_CF_YA                        REG_FLD(9, 0)

#define L3_Y2R_PARA_YUV_A_1_FLD_C_CF_VA                        REG_FLD(9, 0)

#define L3_Y2R_PARA_RGB_A_0_FLD_C_CF_GA                        REG_FLD(13, 16)
#define L3_Y2R_PARA_RGB_A_0_FLD_C_CF_RA                        REG_FLD(9, 0)

#define L3_Y2R_PARA_RGB_A_1_FLD_C_CF_BA                        REG_FLD(9, 0)

#define FLOW_CTRL_DBG_FLD_FLOW_DBG                             REG_FLD(32, 0)

#define ADDCON_DBG_FLD_ROI_Y                                   REG_FLD(13, 16)
#define ADDCON_DBG_FLD_ROI_X                                   REG_FLD(13, 0)

#define OUTMUX_DBG_FLD_OUT_DATA                                REG_FLD(24, 8)
#define OUTMUX_DBG_FLD_OUT_VALID                               REG_FLD(1, 1)
#define OUTMUX_DBG_FLD_OUT_READY                               REG_FLD(1, 0)

#define RDMA0_DBG_FLD_CUR_Y0                                   REG_FLD(16, 16)
#define RDMA0_DBG_FLD_CUR_X0                                   REG_FLD(16, 0)

#define RDMA1_DBG_FLD_CUR_Y1                                   REG_FLD(16, 16)
#define RDMA1_DBG_FLD_CUR_X1                                   REG_FLD(16, 0)

#define RDMA2_DBG_FLD_CUR_Y2                                   REG_FLD(16, 16)
#define RDMA2_DBG_FLD_CUR_X2                                   REG_FLD(16, 0)

#define RDMA3_DBG_FLD_CUR_Y3                                   REG_FLD(16, 16)
#define RDMA3_DBG_FLD_CUR_X3                                   REG_FLD(16, 0)

#endif // __DROVL_H__
