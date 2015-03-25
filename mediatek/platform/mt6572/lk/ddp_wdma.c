#include <assert.h>
#include <platform/mt_typedefs.h>
#include <platform/ddp_matrix_para.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_wdma.h>

#ifndef ASSERT
   #define ASSERT(expr)                                                    \
      do {                                                                \
         if(!(expr)) {                                                   \
            printf("<ASSERT> %s:line %d %s\n",                          \
            __FILE__,__LINE__,(int)(#expr));                        \
            while (1);                                                  \
         }                                                               \
      } while(0);
#endif

enum WDMA_COLOR_SPACE {
    WDMA_COLOR_SPACE_RGB = 0,
    WDMA_COLOR_SPACE_YUV,
};


int WDMAStart(unsigned idx) {
    
    
    DISP_REG_SET(DISP_REG_WDMA_INTEN, 0x01);
    DISP_REG_SET(DISP_REG_WDMA_EN, 0x01);

    return 0;
}

int WDMAStop(unsigned idx) {
        

    DISP_REG_SET(DISP_REG_WDMA_INTEN, 0x00);
    DISP_REG_SET(DISP_REG_WDMA_EN, 0x00);
    
    return 0;
}

int WDMAReset(unsigned idx) {
    
    

    //WDMA_RST = 0x00;
    DISP_REG_SET(DISP_REG_WDMA_RST , 0x01);            // soft reset
    DISP_REG_SET(DISP_REG_WDMA_RST , 0x00);
    
    DISP_REG_SET(DISP_REG_WDMA_CFG , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_SRC_SIZE , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_CLIP_SIZE , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_CLIP_COORD , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_DST_ADDR , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_DST_W_IN_BYTE , 0x00);
    DISP_REG_SET(DISP_REG_WDMA_ALPHA , 0x00);          // clear regs

    return 0;
}

int WDMAConfigUV(unsigned idx, unsigned int uAddr, unsigned int vAddr, unsigned int dstWidth)
{
    unsigned int bpp=1;

    
    

    DISP_REG_SET(DISP_REG_WDMA_DST_U_ADDR, uAddr);
    DISP_REG_SET(DISP_REG_WDMA_DST_V_ADDR, vAddr);
    DISP_REG_SET_FIELD(WDMA_BUF_ADDR_FLD_UV_Pitch, DISP_REG_WDMA_DST_UV_PITCH, dstWidth * bpp/2);

    return 0;
}

int WDMAConfig(unsigned idx,
               unsigned inputFormat, unsigned srcWidth, unsigned srcHeight,
               unsigned clipX, unsigned clipY, unsigned clipWidth, unsigned clipHeight,
               unsigned outputFormat, unsigned dstAddress, unsigned dstWidth,               
               BOOL useSpecifiedAlpha, unsigned char alpha) {


    
    
    
    ASSERT((WDMA_INPUT_FORMAT_ARGB == inputFormat) ||
           (WDMA_INPUT_FORMAT_YUV444 == inputFormat));    

    DISP_REG_SET(DISP_REG_WDMA_SRC_SIZE, srcHeight<<16 | srcWidth);
    DISP_REG_SET(DISP_REG_WDMA_CLIP_COORD, clipY<<16 | clipX);
    DISP_REG_SET(DISP_REG_WDMA_CLIP_SIZE, clipHeight<<16 | clipWidth);

    DISP_REG_SET_FIELD(WDMA_CFG_FLD_In_Format, DISP_REG_WDMA_CFG, inputFormat);

    unsigned int output_format=0;
    unsigned int byte_swap=0;
    unsigned int rgb_swap=0;
    unsigned int uv_swap=0;
    switch(outputFormat) {
        case WDMA_OUTPUT_FORMAT_RGB565:
        case WDMA_OUTPUT_FORMAT_RGB888:
        case WDMA_OUTPUT_FORMAT_ARGB:
        case WDMA_OUTPUT_FORMAT_XRGB:
        case WDMA_OUTPUT_FORMAT_UYVY:
        case WDMA_OUTPUT_FORMAT_YUV444:
        case WDMA_OUTPUT_FORMAT_UYVY_BLK:
        case WDMA_OUTPUT_FORMAT_YUV420_P:
            output_format = outputFormat;
            byte_swap =  0;
            rgb_swap =  0;
            uv_swap =  0;
            break;
        case WDMA_OUTPUT_FORMAT_BGR888:
            output_format =  WDMA_OUTPUT_FORMAT_RGB888;
            byte_swap =  0;
            rgb_swap =  1;
            uv_swap =  0;            
            break;
        case WDMA_OUTPUT_FORMAT_BGRA:
            output_format =  WDMA_OUTPUT_FORMAT_ARGB;
            byte_swap =  1;
            rgb_swap =  0;
            uv_swap =  0;
            break;
        case WDMA_OUTPUT_FORMAT_ABGR:
            output_format =  WDMA_OUTPUT_FORMAT_ARGB;
            byte_swap =  0;
            rgb_swap =  1;
            uv_swap =  0;
            break;
        case WDMA_OUTPUT_FORMAT_RGBA:
            output_format =  WDMA_OUTPUT_FORMAT_ARGB;
            byte_swap =  1;
            rgb_swap =  1;
            uv_swap =  0;
            break;
        default:
            ASSERT(0);       // invalid format
    }
    DISP_REG_SET_FIELD(WDMA_CFG_FLD_Out_Format, DISP_REG_WDMA_CFG, output_format);
    DISP_REG_SET_FIELD(WDMA_CFG_FLD_BYTE_SWAP,  DISP_REG_WDMA_CFG, byte_swap);
    DISP_REG_SET_FIELD(WDMA_CFG_FLD_RGB_SWAP,   DISP_REG_WDMA_CFG, rgb_swap);
    DISP_REG_SET_FIELD(WDMA_CFG_FLD_UV_SWAP,    DISP_REG_WDMA_CFG, uv_swap);


    // set DNSP for UYVY and YUV_3P format for better quality
    if(outputFormat==WDMA_OUTPUT_FORMAT_UYVY ||
       outputFormat==WDMA_OUTPUT_FORMAT_YUV420_P)
    {
       DISP_REG_SET_FIELD(WDMA_CFG_FLD_DNSP_SEL, DISP_REG_WDMA_CFG, 1);
    }
    else
    {
        DISP_REG_SET_FIELD(WDMA_CFG_FLD_DNSP_SEL, DISP_REG_WDMA_CFG, 0);
    }
    
    unsigned input_color_space;                         // check input format color space
    switch (inputFormat) {
        case WDMA_INPUT_FORMAT_ARGB:
            input_color_space = WDMA_COLOR_SPACE_RGB;
            break;
        case WDMA_INPUT_FORMAT_YUV444:
            input_color_space = WDMA_COLOR_SPACE_YUV;
            break;
        default:
            ASSERT(0);
    }

    unsigned output_color_space;                        // check output format color space
    switch (outputFormat) {
        case WDMA_OUTPUT_FORMAT_RGB565:
        case WDMA_OUTPUT_FORMAT_RGB888:
        case WDMA_OUTPUT_FORMAT_ARGB:
        case WDMA_OUTPUT_FORMAT_XRGB:
        case WDMA_OUTPUT_FORMAT_BGR888:
        case WDMA_OUTPUT_FORMAT_BGRA:
        case WDMA_OUTPUT_FORMAT_ABGR:
        case WDMA_OUTPUT_FORMAT_RGBA:
            output_color_space = WDMA_COLOR_SPACE_RGB;
            break;
        case WDMA_OUTPUT_FORMAT_UYVY:
        case WDMA_OUTPUT_FORMAT_YUV444:
        case WDMA_OUTPUT_FORMAT_UYVY_BLK:
        case WDMA_OUTPUT_FORMAT_YUV420_P:
            output_color_space = WDMA_COLOR_SPACE_YUV;
            break;
        default:
            ASSERT(0);
    }

    unsigned mode = 0xdeaddead;
    if(WDMA_COLOR_SPACE_RGB == input_color_space &&
       WDMA_COLOR_SPACE_YUV == output_color_space) {        // RGB to YUV required       
        mode = RGB2YUV_601;
    }
    else if(WDMA_COLOR_SPACE_YUV == input_color_space &&    // YUV to RGB required
            WDMA_COLOR_SPACE_RGB == output_color_space) {        
        mode = YUV2RGB_601_16_16;
    }

    if(TABLE_NO > mode) {                                           // set matrix as mode
#if 1
        DISP_REG_SET_FIELD(WDMA_CFG_FLD_CT_EN, DISP_REG_WDMA_CFG, 1);
        DISP_REG_SET_FIELD(WDMA_CFG_FLD_EXT_MTX_EN, DISP_REG_WDMA_CFG, 1);

        DISP_REG_SET_FIELD(WDMA_C00_FLD_C00, DISP_REG_WDMA_C00, coef[mode][0][0]);
        DISP_REG_SET_FIELD(WDMA_C00_FLD_C01, DISP_REG_WDMA_C00, coef[mode][0][1]);
        DISP_REG_SET(DISP_REG_WDMA_C02, coef[mode][0][2]);

        DISP_REG_SET_FIELD(WDMA_C10_FLD_C10, DISP_REG_WDMA_C10, coef[mode][1][0]);
        DISP_REG_SET_FIELD(WDMA_C10_FLD_C11, DISP_REG_WDMA_C10, coef[mode][1][1]);
        DISP_REG_SET(DISP_REG_WDMA_C12 , coef[mode][1][2]);

        DISP_REG_SET_FIELD(WDMA_C20_FLD_C20, DISP_REG_WDMA_C20, coef[mode][2][0]);
        DISP_REG_SET_FIELD(WDMA_C20_FLD_C21, DISP_REG_WDMA_C20, coef[mode][2][1]);
        DISP_REG_SET(DISP_REG_WDMA_C22 , coef[mode][2][2]);

        DISP_REG_SET_FIELD(WDMA_PRE_ADD0_FLD_PRE_ADD_0, DISP_REG_WDMA_PRE_ADD0, coef[mode][3][0]);
        DISP_REG_SET_FIELD(WDMA_PRE_ADD0_FLD_SIGNED_0, DISP_REG_WDMA_PRE_ADD0, 0);
        DISP_REG_SET_FIELD(WDMA_PRE_ADD0_FLD_PRE_ADD_1, DISP_REG_WDMA_PRE_ADD0, coef[mode][3][1]);
        DISP_REG_SET_FIELD(WDMA_PRE_ADD0_FLD_SIGNED_1, DISP_REG_WDMA_PRE_ADD0, 0);

        DISP_REG_SET_FIELD(WDMA_PRE_ADD2_FLD_PRE_ADD_2, DISP_REG_WDMA_PRE_ADD2, coef[mode][3][2]); 
        DISP_REG_SET_FIELD(WDMA_PRE_ADD2_FLD_SIGNED_2, DISP_REG_WDMA_PRE_ADD2, 0);

        DISP_REG_SET_FIELD(WDMA_POST_ADD0_FLD_POST_ADD_0, DISP_REG_WDMA_POST_ADD0, coef[mode][4][0]);
        DISP_REG_SET_FIELD(WDMA_POST_ADD0_FLD_POST_ADD_1, DISP_REG_WDMA_POST_ADD0, coef[mode][4][1]);
        DISP_REG_SET(DISP_REG_WDMA_POST_ADD2 , coef[mode][4][2]);
#else
        DISP_REG_SET_FIELD(WDMA_CFG_FLD_CT_EN, DISP_REG_WDMA_CFG, 1);
        DISP_REG_SET_FIELD(WDMA_CFG_FLD_INT_MTX_SEL, DISP_REG_WDMA_CFG, 2);
#endif
    }
    
    unsigned bpp;
    switch(outputFormat) {
        case WDMA_OUTPUT_FORMAT_RGB565:
        case WDMA_OUTPUT_FORMAT_UYVY_BLK:
        case WDMA_OUTPUT_FORMAT_UYVY:
            bpp = 2;
            break;
        case WDMA_OUTPUT_FORMAT_YUV420_P:
            bpp = 1;
            break;            
        case WDMA_OUTPUT_FORMAT_RGB888:
        case WDMA_OUTPUT_FORMAT_BGR888:
        case WDMA_OUTPUT_FORMAT_YUV444:
            bpp = 3;
            break;
        case WDMA_OUTPUT_FORMAT_ARGB:
        case WDMA_OUTPUT_FORMAT_XRGB:
        case WDMA_OUTPUT_FORMAT_BGRA:
        case WDMA_OUTPUT_FORMAT_ABGR:            
        case WDMA_OUTPUT_FORMAT_RGBA:
            bpp = 4;
            break;
        default:
            ASSERT(0);  // invalid format
    }
    DISP_REG_SET(DISP_REG_WDMA_DST_ADDR, dstAddress);
    DISP_REG_SET(DISP_REG_WDMA_DST_W_IN_BYTE, dstWidth * bpp);
    DISP_REG_SET_FIELD(WDMA_ALPHA_FLD_A_Sel,   DISP_REG_WDMA_ALPHA, useSpecifiedAlpha);
    DISP_REG_SET_FIELD(WDMA_ALPHA_FLD_A_Value, DISP_REG_WDMA_ALPHA, alpha);
    
    return 0;
}

void WDMAWait(unsigned idx)
{
    
    // polling interrupt status
    while((DISP_REG_GET(DISP_REG_WDMA_INTSTA) & 0x1) != 0x1) ;
    DISP_REG_SET(DISP_REG_WDMA_INTSTA , 0x0);
}

void WDMASlowMode(unsigned int idx, 
                          unsigned int enable, 
                          unsigned int level, 
                          unsigned int cnt,
                          unsigned int threadhold)
{
    
    DISP_REG_SET_FIELD(WDMA_SMI_CON_FLD_Slow_Enable, DISP_REG_WDMA_SMI_CON, enable&0x01);
    DISP_REG_SET_FIELD(WDMA_SMI_CON_FLD_Slow_Count,  DISP_REG_WDMA_SMI_CON, cnt&0xff);
    DISP_REG_SET_FIELD(WDMA_SMI_CON_FLD_Slow_Level,  DISP_REG_WDMA_SMI_CON, level&0x7);
    DISP_REG_SET_FIELD(WDMA_SMI_CON_FLD_Threshold,   DISP_REG_WDMA_SMI_CON, threadhold&0xf);

}
