#ifndef _DDP_WDMA_API_H_
#define _DDP_WDMA_API_H_

#include <mach/mt_typedefs.h>

#include "ddp_drv.h"


#define WDMA_INSTANCES  2
#define WDMA_MAX_WIDTH  1920
#define WDMA_MAX_HEIGHT 1080

enum WDMA_INPUT_FORMAT {
    WDMA_INPUT_FORMAT_ARGB = 0x00,      // from overlay
    WDMA_INPUT_FORMAT_YUV444 = 0x01,    // from direct link
};

typedef struct _WDMA_CONFIG_STRUCT
{
    unsigned idx;           // module idx
    unsigned inputFormat; 
    unsigned srcWidth; 
    unsigned srcHeight;     // input
    unsigned clipX; 
    unsigned clipY; 
    unsigned clipWidth; 
    unsigned clipHeight;    // clip
    unsigned outputFormat; 
    unsigned dstAddress; 
    unsigned dstWidth;     // output
    bool useSpecifiedAlpha; 
    unsigned char alpha;
}WDMA_CONFIG_STRUCT;


// initialize module
int WDMAInit(unsigned idx);

// start module
int WDMAStart(unsigned idx);

// stop module
int WDMAStop(unsigned idx);

// soft reset and to default value
int WDMAReset(unsigned idx);

// config write settings
int WDMAConfig(unsigned idx,                                                            // module idx
               unsigned inputFormat, 
               unsigned srcWidth, 
               unsigned srcHeight,
               unsigned clipX, 
               unsigned clipY, 
               unsigned clipWidth, 
               unsigned clipHeight,
               DpColorFormat  out_formt, 
               unsigned dstAddress, 
               unsigned dstWidth,               
               bool useSpecifiedAlpha, 
               unsigned char alpha);                            // alpha

void WDMAWait(unsigned idx);

void WDMASlowMode(unsigned int idx, 
                          unsigned int enable, 
                          unsigned int level, 
                          unsigned int cnt,
                          unsigned int threadhold);

enum WDMA_OUTPUT_FORMAT wdma_fmt_convert(DpColorFormat fmt);
void WDMAConfigAddress(unsigned int idx, unsigned int addr);


///----------------------------------------------------------
#define WDMA_INTEN_FLD_Frame_Underrun                          REG_FLD(1, 1)
#define WDMA_INTEN_FLD_Frame_Complete                          REG_FLD(1, 0)

#define WDMA_INTSTA_FLD_Frame_Underrun                         REG_FLD(1, 1)
#define WDMA_INTSTA_FLD_Frame_Complete                         REG_FLD(1, 0)

#define WDMA_EN_FLD_Enable                                     REG_FLD(1, 0)

#define WDMA_RST_FLD_Soft_Reset                                REG_FLD(1, 0)

#define WDMA_SMI_CON_FLD_Slow_Count                            REG_FLD(8, 8)
#define WDMA_SMI_CON_FLD_Slow_Level                            REG_FLD(3, 5)
#define WDMA_SMI_CON_FLD_Slow_Enable                           REG_FLD(1, 4)
#define WDMA_SMI_CON_FLD_Threshold                             REG_FLD(4, 0)

#define WDMA_CFG_FLD_ERR_DIF_EN                                REG_FLD(1, 25)
#define WDMA_CFG_FLD_DITHER_EN                                 REG_FLD(1, 24)
#define WDMA_CFG_FLD_DNSP_SEL                                  REG_FLD(1, 15)
#define WDMA_CFG_FLD_UV_SWAP                                   REG_FLD(1, 10)
#define WDMA_CFG_FLD_RGB_SWAP                                  REG_FLD(1, 9)
#define WDMA_CFG_FLD_BYTE_SWAP                                 REG_FLD(1, 8)
#define WDMA_CFG_FLD_Out_Format                                REG_FLD(4, 4)
#define WDMA_CFG_FLD_In_Format                                 REG_FLD(4, 0)

#define WDMA_SRC_SIZE_FLD_Height                               REG_FLD(16, 16)
#define WDMA_SRC_SIZE_FLD_Width                                REG_FLD(16, 0)

#define WDMA_CLIP_SIZE_FLD_Height                              REG_FLD(16, 16)
#define WDMA_CLIP_SIZE_FLD_Width                               REG_FLD(16, 0)

#define WDMA_CLIP_COORD_FLD_Y_coord                            REG_FLD(16, 16)
#define WDMA_CLIP_COORD_FLD_X_coord                            REG_FLD(16, 0)

#define WDMA_DST_ADDR_FLD_Address                              REG_FLD(32, 0)
#define WDMA_BUF_ADDR_FLD_U_Buf_Address                          REG_FLD(32, 0)
#define WDMA_BUF_ADDR_FLD_V_Buf_Address                          REG_FLD(32, 0)
#define WDMA_BUF_ADDR_FLD_UV_Pitch                             REG_FLD(14, 0)

#define WDMA_DST_W_IN_BYTE_FLD_Dst_W_in_Byte                   REG_FLD(32, 0)

#define WDMA_ALPHA_FLD_A_Sel                                   REG_FLD(1, 31)
#define WDMA_ALPHA_FLD_A_Value                                 REG_FLD(8, 0)

#define WDMA_BUF_ADDR_FLD_Buf_Address                          REG_FLD(32, 0)


#define WDMA_STA_FLD_Status                                    REG_FLD(32, 0)

#define WDMA_BUF_CON1_FLD_Ultra_Enable                        REG_FLD(1, 31)
#define WDMA_BUF_CON1_FLD_Ultra_Threshold_H                   REG_FLD(9, 0)
#define WDMA_BUF_CON1_FLD_Ultra_Threshold_L                   REG_FLD(9, 16)

#define WDMA_BUF_CON2_FLD_Ultra_Threshold_H                   REG_FLD(8, 0)
#define WDMA_BUF_CON2_FLD_Ultra_Threshold_L                   REG_FLD(8, 8)
#define WDMA_BUF_CON2_FLD_Pre_Ultra_Th_High_Ofs               REG_FLD(8, 16)
#define WDMA_BUF_CON2_FLD_Ultra_Enable                        REG_FLD(8, 24)

#define WDMA_C00_FLD_C01                                       REG_FLD(13, 16)
#define WDMA_C00_FLD_C00                                       REG_FLD(13, 0)

#define WDMA_C02_FLD_C02                                       REG_FLD(13, 0)

#define WDMA_C10_FLD_C11                                       REG_FLD(13, 16)
#define WDMA_C10_FLD_C10                                       REG_FLD(13, 0)

#define WDMA_C12_FLD_C12                                       REG_FLD(13, 0)

#define WDMA_C20_FLD_C21                                       REG_FLD(13, 16)
#define WDMA_C20_FLD_C20                                       REG_FLD(13, 0)

#define WDMA_C22_FLD_C22                                       REG_FLD(13, 0)

#define WDMA_PRE_ADD0_FLD_SIGNED_1                             REG_FLD(1, 31)
#define WDMA_PRE_ADD0_FLD_PRE_ADD_1                            REG_FLD(9, 16)
#define WDMA_PRE_ADD0_FLD_SIGNED_0                             REG_FLD(1, 15)
#define WDMA_PRE_ADD0_FLD_PRE_ADD_0                            REG_FLD(9, 0)

#define WDMA_PRE_ADD2_FLD_SIGNED_2                             REG_FLD(1, 15)
#define WDMA_PRE_ADD2_FLD_PRE_ADD_2                            REG_FLD(9, 0)

#define WDMA_POST_ADD0_FLD_POST_ADD_1                          REG_FLD(9, 16)
#define WDMA_POST_ADD0_FLD_POST_ADD_0                          REG_FLD(9, 0)

#define WDMA_POST_ADD2_FLD_POST_ADD_2                          REG_FLD(9, 0)

#define WDMA_DITHER_CON_FLD_LFSR_Seed_R                        REG_FLD(4, 24)
#define WDMA_DITHER_CON_FLD_LFSR_Seed_G                        REG_FLD(4, 20)
#define WDMA_DITHER_CON_FLD_LFSR_Seed_B                        REG_FLD(4, 16)
#define WDMA_DITHER_CON_FLD_Dither_Bit_R                       REG_FLD(2, 8)
#define WDMA_DITHER_CON_FLD_Dither_Bit_G                       REG_FLD(2, 4)
#define WDMA_DITHER_CON_FLD_Dither_Bit_B                       REG_FLD(2, 0)

#define WDMA_FLOW_CTRL_DBG_FLD_WDMA_STA_FLOW_CTRL              REG_FLD(32, 0)

#define WDMA_EXEC_DBG_FLD_WDMA_STA_EXEC                        REG_FLD(32, 0)

#define WDMA_CLIP_DBG_FLD_WDMA_STA_CLIP                        REG_FLD(32, 0)


#endif
