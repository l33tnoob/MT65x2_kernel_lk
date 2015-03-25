#include <assert.h>
#include <platform/mt_typedefs.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_rdma.h>

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


int RDMAStart(unsigned idx) {
    ASSERT(idx <= 2);
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, DISP_REG_RDMA_GLOBAL_CON, 1);

    return 0;
}

int RDMAStop(unsigned idx) {
    ASSERT(idx <= 2);
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, DISP_REG_RDMA_GLOBAL_CON, 0);

    return 0;
}

int RDMAReset(unsigned idx) {
    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, DISP_REG_RDMA_GLOBAL_CON, 1); 
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, DISP_REG_RDMA_GLOBAL_CON, 0); 

    DISP_REG_SET(DISP_REG_RDMA_GLOBAL_CON     , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_SIZE_CON_0     , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_SIZE_CON_1     , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_MEM_CON         , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_MEM_START_ADDR , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_MEM_SRC_PITCH     , 0x00);
    DISP_REG_SET(DISP_REG_RDMA_MEM_GMC_SETTING_1 , 0x20);     ///TODO: need check
    DISP_REG_SET(DISP_REG_RDMA_FIFO_CON , 0x81000040);        ///TODO: need check
    
    return 0;
}

int RDMAConfig(unsigned idx,
                    enum RDMA_MODE mode,
                    enum RDMA_INPUT_FORMAT inputFormat, 
                    unsigned address, 
                    enum RDMA_OUTPUT_FORMAT outputFormat, 
                    unsigned pitch,
                    unsigned width, 
                    unsigned height, 
                    BOOL isByteSwap, // input setting
                    BOOL isRGBSwap)  // ourput setting
{
    ASSERT(idx <= 2);
    ASSERT((width <= RDMA_MAX_WIDTH) && (height <= RDMA_MAX_HEIGHT));

    unsigned bpp;

    switch(inputFormat) {
       case RDMA_INPUT_FORMAT_YUYV:
       case RDMA_INPUT_FORMAT_UYVY:
       case RDMA_INPUT_FORMAT_YVYU:
       case RDMA_INPUT_FORMAT_VYUY:
       case RDMA_INPUT_FORMAT_RGB565:
           bpp = 2;
           break;
       case RDMA_INPUT_FORMAT_RGB888:
           bpp = 3;
           break;
       case RDMA_INPUT_FORMAT_ARGB:
           bpp = 4;
           break;
       default:
           ASSERT(0);
    }

    printf("RDMA: w=%d, h=%d, pitch=%d, mode=%d \n", width, height, width*bpp, mode);
	

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_MODE_SEL, DISP_REG_RDMA_GLOBAL_CON, mode);
    DISP_REG_SET_FIELD(MEM_CON_FLD_MEM_MODE_INPUT_FORMAT, DISP_REG_RDMA_MEM_CON, inputFormat);
    
    DISP_REG_SET(DISP_REG_RDMA_MEM_START_ADDR, address);
    DISP_REG_SET(DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
    
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_INPUT_BYTE_SWAP, DISP_REG_RDMA_SIZE_CON_0, isByteSwap);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FORMAT, DISP_REG_RDMA_SIZE_CON_0, outputFormat);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH, DISP_REG_RDMA_SIZE_CON_0, width);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_RGB_SWAP, DISP_REG_RDMA_SIZE_CON_0, isRGBSwap);
    DISP_REG_SET_FIELD(SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT, DISP_REG_RDMA_SIZE_CON_1, height);

    return 0;
}

void RDMAWait(unsigned idx)
{
    // polling interrupt status
    while((DISP_REG_GET(DISP_REG_RDMA_INT_STATUS) & 0x1) != 0x1) ;
    DISP_REG_SET(DISP_REG_RDMA_INT_STATUS , 0x0);
}



