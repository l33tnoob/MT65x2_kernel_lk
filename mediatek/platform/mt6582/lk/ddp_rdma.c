#include <platform/mt_typedefs.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_rdma.h>
#include <printf.h>

#ifndef ASSERT
#define ASSERT(expr) do { if(!(expr)) printf("ASSERT error func: %s, line: %d\n", __func__, __LINE__);} while (0)
#endif

#define DISP_INDEX_OFFSET 0x0

int RDMAStart(unsigned idx) {
    ASSERT(idx <= 2);

    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0x3F);
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);

    return 0;
}

int RDMAStop(unsigned idx) {
    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_ENGINE_EN, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_ENABLE, 0);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS, 0);
    return 0;
}

int RDMAReset(unsigned idx) {
	unsigned int delay_cnt = 0;

    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 1);
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON)&0x700)==0x100) {
    	delay_cnt++;
    	if(delay_cnt > 10000) {
    		printf("[DDP] error, RDMAReset(%d) timeout, stage 1! DISP_REG_RDMA_GLOBAL_CON=0x%x \n", idx, DISP_REG_GET(idx * 0x1000 + DISP_REG_RDMA_GLOBAL_CON));
    		break;
    	}
    }
    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_SOFT_RESET, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, 0);

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

    unsigned bpp = 0;
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
	

    DISP_REG_SET_FIELD(GLOBAL_CON_FLD_MODE_SEL, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_GLOBAL_CON, mode);
    DISP_REG_SET_FIELD(MEM_CON_FLD_MEM_MODE_INPUT_FORMAT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_CON, inputFormat);
    
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_START_ADDR, address);
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
    
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_INPUT_BYTE_SWAP, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, isByteSwap);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FORMAT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, outputFormat);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, width);
    DISP_REG_SET_FIELD(SIZE_CON_0_FLD_OUTPUT_RGB_SWAP, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, isRGBSwap);
    DISP_REG_SET_FIELD(SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT, idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_1, height);

    return 0;
}

void RDMAWait(unsigned idx)
{
    // polling interrupt status
    unsigned int delay_cnt = 0;
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS) & 0x1) != 0x1) 
    {

		delay_cnt++;
		msleep(1);
		if(delay_cnt>100)
		{
			printk("[DDP] error:RDMA%dWait timeout \n", idx);
			break;
		}
    }
    DISP_REG_SET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS , 0x0);
}


void Wait_Rdma_Start(unsigned idx)
{
    // polling interrupt status
    while((DISP_REG_GET(idx * DISP_INDEX_OFFSET + DISP_REG_RDMA_INT_STATUS) & 0x2) != 0x2) ;
    DISP_REG_SET(DISP_REG_RDMA_INT_STATUS , 0x0);
}
