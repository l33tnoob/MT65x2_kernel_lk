#ifndef _DDP_RDMA_API_H_
#define _DDP_RDMA_API_H_

#include <mach/mt_typedefs.h>

#include "ddp_drv.h"


#define RDMA_INSTANCES  2
#define RDMA_MAX_WIDTH  2047
#define RDMA_MAX_HEIGHT 2047



// initialize module
int RDMAInit(unsigned idx);

// start module
int RDMAStart(unsigned idx);

// stop module
int RDMAStop(unsigned idx);

// reset module
int RDMAReset(unsigned idx);

// configu module
int RDMAConfig(unsigned idx,
                    enum RDMA_MODE mode,
                    DpColorFormat inFormat, 
                    unsigned address, 
                    enum RDMA_OUTPUT_FORMAT outputFormat, 
                    unsigned pitch,
                    unsigned width, 
                    unsigned height, 
                    bool isByteSwap, // input setting
                    bool isRGBSwap); // ourput setting

void RDMAWait(unsigned idx);
void RDMASetAddress(unsigned int idx, unsigned int address);
void RDMASetTargetLine(unsigned int idx, unsigned int line);

void RDMAEnableIrq(unsigned int idx, unsigned int value);

enum RDMA_INPUT_FORMAT rdma_fmt_convert(DpColorFormat fmt);

//---------------------------------------------------------------------------------------
#define INT_ENABLE_FLD_FIFO_UNDERFLOW_INT_EN                   REG_FLD(1, 4)
#define INT_ENABLE_FLD_EOF_ABNORMAL_INT_EN                     REG_FLD(1, 3)
#define INT_ENABLE_FLD_FRAME_END_INT_EN                        REG_FLD(1, 2)
#define INT_ENABLE_FLD_FRAME_START_INT_EN                      REG_FLD(1, 1)
#define INT_ENABLE_FLD_REG_UPDATE_INT_EN                       REG_FLD(1, 0)

#define INT_STATUS_FLD_FIFO_UNDERFLOW_INT_FLAG                 REG_FLD(1, 4)
#define INT_STATUS_FLD_EOF_ABNORMAL_INT_FLAG                   REG_FLD(1, 3)
#define INT_STATUS_FLD_FRAME_END_INT_FLAG                      REG_FLD(1, 2)
#define INT_STATUS_FLD_FRAME_START_INT_FLAG                    REG_FLD(1, 1)
#define INT_STATUS_FLD_REG_UPDATE_INT_FLAG                     REG_FLD(1, 0)

#define GLOBAL_CON_FLD_SOFT_RESET                              REG_FLD(1, 4)
#define GLOBAL_CON_FLD_MODE_SEL                                REG_FLD(1, 1)
#define GLOBAL_CON_FLD_ENGINE_EN                               REG_FLD(1, 0)

#define SIZE_CON_0_FLD_OUTPUT_RGB_SWAP                         REG_FLD(1, 31)
#define SIZE_CON_0_FLD_INPUT_BYTE_SWAP                         REG_FLD(1, 30)
#define SIZE_CON_0_FLD_OUTPUT_FORMAT                           REG_FLD(1, 29)
#define SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL			           REG_FLD(4, 20)
#define SIZE_CON_0_FLD_MATRIX_ENABLE					       REG_FLD(1, 17)
#define SIZE_CON_0_FLD_MATRIX_EXT_MTX_ENABLE				   REG_FLD(1, 16)
#define SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH                      REG_FLD(12, 0)

#define SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT                     REG_FLD(20, 0)

//#define MEM_START_TRIG_FLD_MEM_MODE_START_TRIG                 REG_FLD(1, 0)

#define MEM_CON_FLD_MEM_MODE_HORI_BLOCK_NUM                    REG_FLD(8, 24)
#define MEM_CON_FLD_MEM_MODE_INPUT_FORMAT                      REG_FLD(5, 4)
#define MEM_CON_FLD_MEM_MODE_TILE_EN                           REG_FLD(1, 0)

#define MEM_START_ADDR_FLD_MEM_MODE_START_ADDR                 REG_FLD(32, 0)

#define MEM_SRC_PITCH_FLD_MEM_MODE_SRC_PITCH                   REG_FLD(16, 0)

#define MEM_GMC_SETTING_0_FLD_ULTRA_THRESHOLD_LOW              REG_FLD(8, 0)
#define MEM_GMC_SETTING_0_FLD_PRE_ULTRA_THRESHOLD_LOW          REG_FLD(8, 8)
#define MEM_GMC_SETTING_0_FLD_ULTRA_THRESHOLD_HIGH_OFS         REG_FLD(8, 16)
#define MEM_GMC_SETTING_0_FLD_PRE_ULTRA_THRESHOLD_HIGH_OFS     REG_FLD(8, 24)

#define MEM_GMC_SETTING_1_FLD_ISSUE_REQ_THRESHOLD              REG_FLD(32, 0)

#define MEM_SLOW_CON_FLD_MEM_MODE_SLOW_COUNT                   REG_FLD(16, 16)
#define MEM_SLOW_CON_FLD_MEM_MODE_SLOW_EN                      REG_FLD(1, 0)

#define FIFO_CON_FLD_FIFO_UNDERFLOW_EN                         REG_FLD(1, 31)
#define FIFO_CON_FLD_FIFO_SIZE                                 REG_FLD(10, 16)
#define FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD               REG_FLD(10, 0)

#define CF_FLD                                                 REG_FLD(13, 0)
#define CF_ADD_FLD                                             REG_FLD(9, 0)


#endif
