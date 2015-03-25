#ifndef _DDP_REG_H_
#define _DDP_REG_H_

#include <mach/mt_reg_base.h>
#include <mach/sync_write.h>



//TDODO: get base reg addr from system header
#define DDP_REG_BASE_MMSYS_CONFIG  0xf4000000
#define DDP_REG_BASE_DISP_OVL	   0xf4007000  
#define DDP_REG_BASE_DISP_RDMA	   0xf4008000
#define DDP_REG_BASE_DISP_WDMA	   0xf4009000
#define DDP_REG_BASE_DISP_BLS	   0xf400A000  
#define DDP_REG_BASE_DISP_COLOR	   0xf400B000
#define DDP_REG_BASE_DSI	       0xf400C000  
#define DDP_REG_BASE_DPI	       0xf400D000  
#define DDP_REG_BASE_MM_MUTEX	   0xf400E000  
#define DDP_REG_BASE_MM_CMDQ	   0xf400F000  
#define DDP_REG_BASE_SMI_LARB0	   0xf4010000
#define DDP_REG_BASE_SMI_COMMON	   0xf4011000
#define DDP_REG_BASE_DBI	       0xf500E000 

#define DISPSYS_CONFIG_BASE     DDP_REG_BASE_MMSYS_CONFIG		
#define DISPSYS_OVL_BASE        DDP_REG_BASE_DISP_OVL				
#define DISPSYS_WDMA0_BASE      DDP_REG_BASE_DISP_WDMA			
#define DISPSYS_RDMA1_BASE      DDP_REG_BASE_DISP_RDMA			
#define DISPSYS_BLS_BASE        DDP_REG_BASE_DISP_BLS				
#define DISPSYS_COLOR_BASE      DDP_REG_BASE_DISP_COLOR			
#define DISPSYS_DSI_BASE        DDP_REG_BASE_DSI				
#define DISPSYS_DPI0_BASE       DDP_REG_BASE_DPI
#define DISPSYS_MUTEX_BASE      DDP_REG_BASE_MM_MUTEX	
#define DISPSYS_CMDQ_BASE       DDP_REG_BASE_MM_CMDQ


#define DISPSYS_REG_ADDR_MIN    DISPSYS_CONFIG_BASE
#define DISPSYS_REG_ADDR_MAX    (DDP_REG_BASE_DBI+0x2000)
// ---------------------------------------------------------------------------
//  Register Field Access
// ---------------------------------------------------------------------------

#define REG_FLD(width, shift) \
    ((unsigned int)((((width) & 0xFF) << 16) | ((shift) & 0xFF)))

#define REG_FLD_WIDTH(field) \
    ((unsigned int)(((field) >> 16) & 0xFF))

#define REG_FLD_SHIFT(field) \
    ((unsigned int)((field) & 0xFF))

#define REG_FLD_MASK(field) \
    (((unsigned int)(1 << REG_FLD_WIDTH(field)) - 1) << REG_FLD_SHIFT(field))

#define REG_FLD_VAL(field, val) \
    (((val) << REG_FLD_SHIFT(field)) & REG_FLD_MASK(field))

#define REG_FLD_GET(field, reg32) \
    (((reg32) & REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field))

#define REG_FLD_SET(field, reg32, val)                  \
    do {                                                \
        (reg32) = (((reg32) & ~REG_FLD_MASK(field)) |   \
                   REG_FLD_VAL((field), (val)));        \
    } while (0)
    

#define DISP_REG_GET(reg32) (*(volatile unsigned int*)(reg32))
#define DISP_REG_GET_FIELD(field, reg32) \
        do{                           \
              ((*(volatile unsigned int*)(reg32) & REG_FLD_MASK(field)) >> REG_FLD_SHIFT(field)) \
          } while(0)         
      
#if 0
#define DISP_REG_SET(reg32, val) (*(volatile unsigned int*)(reg32) = val)
#define DISP_REG_SET_FIELD(field, reg32, val)  \
    do {                                \
           *(volatile unsigned int*)(reg32) = ((*(volatile unsigned int*)(reg32) & ~REG_FLD_MASK(field)) |  REG_FLD_VAL((field), (val)));  \
    } while (0)
#else  // use write_sync instead of write directly
#define DISP_REG_SET(reg32, val) mt65xx_reg_sync_writel(val, (volatile unsigned int*)(reg32))
/*
#define DISP_REG_SET_FIELD(field, reg32, val)  \
    do {                                \
           mt65xx_reg_sync_writel( (*(volatile unsigned int*)(reg32) & ~REG_FLD_MASK(field))|REG_FLD_VAL((field), (val)), reg32);  \
    } while (0)
*/
void DISP_REG_SET_FIELD(unsigned long field, unsigned long reg32, unsigned long val);
#endif

//-----------------------------------------------------------------
// CMDQ
#define CMDQ_THREAD_NUM 7
#define DISP_REG_CMDQ_IRQ_FLAG                           (DISPSYS_CMDQ_BASE + 0x10)
#define DISP_REG_CMDQ_LOADED_THR                         (DISPSYS_CMDQ_BASE + 0x18)
#define DISP_REG_CMDQ_THR_SLOT_CYCLES                    (DISPSYS_CMDQ_BASE + 0x30)
#define DISP_REG_CMDQ_BUS_CTRL                           (DISPSYS_CMDQ_BASE + 0x40)                                                  
#define DISP_REG_CMDQ_ABORT                              (DISPSYS_CMDQ_BASE + 0x50)
#define DISP_REG_CMDQ_SYNC_TOKEN_ID                      (DISPSYS_CMDQ_BASE + 0x60)
#define DISP_REG_CMDQ_SYNC_TOKEN_VALUE                   (DISPSYS_CMDQ_BASE + 0x64)
#define DISP_REG_CMDQ_THRx_RESET(idx)                    (DISPSYS_CMDQ_BASE + 0x100 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_EN(idx)                       (DISPSYS_CMDQ_BASE + 0x104 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_SUSPEND(idx)                  (DISPSYS_CMDQ_BASE + 0x108 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_STATUS(idx)                   (DISPSYS_CMDQ_BASE + 0x10c + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_IRQ_FLAG(idx)                 (DISPSYS_CMDQ_BASE + 0x110 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(idx)              (DISPSYS_CMDQ_BASE + 0x114 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_SECURITY(idx)                 (DISPSYS_CMDQ_BASE + 0x118 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_PC(idx)                       (DISPSYS_CMDQ_BASE + 0x120 + 0x80*idx) 
#define DISP_REG_CMDQ_THRx_END_ADDR(idx)                 (DISPSYS_CMDQ_BASE + 0x124 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(idx)            (DISPSYS_CMDQ_BASE + 0x128 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_WAIT_EVENTS0(idx)             (DISPSYS_CMDQ_BASE + 0x130 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_WAIT_EVENTS1(idx)             (DISPSYS_CMDQ_BASE + 0x134 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0(idx)         (DISPSYS_CMDQ_BASE + 0x140 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1(idx)         (DISPSYS_CMDQ_BASE + 0x144 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0_CLR(idx)     (DISPSYS_CMDQ_BASE + 0x148 + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1_CLR(idx)     (DISPSYS_CMDQ_BASE + 0x14c + 0x80*idx)   
#define DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(idx)     (DISPSYS_CMDQ_BASE + 0x150 + 0x80*idx)  

//-----------------------------------------------------------------
// CMDQ
#define DISP_REG_COLOR_START                (DISPSYS_COLOR_BASE + 0x0F00)        	  
#define DISP_REG_COLOR_INTEN	            (DISPSYS_COLOR_BASE + 0x0F04)
#define DISP_REG_COLOR_INTSTA	            (DISPSYS_COLOR_BASE + 0x0F08)
#define DISP_REG_COLOR_OUT_SEL	            (DISPSYS_COLOR_BASE + 0x0F0C)
#define DISP_REG_COLOR_FRAME_DONE_DEL       (DISPSYS_COLOR_BASE + 0x0F10)
#define DISP_REG_COLOR_CRC	                (DISPSYS_COLOR_BASE + 0x0F14)
#define DISP_REG_COLOR_SW_SCRATCH           (DISPSYS_COLOR_BASE + 0x0F18)
#define DISP_REG_COLOR_RDY_SEL              (DISPSYS_COLOR_BASE + 0x0F20)
#define DISP_REG_COLOR_RDY_SEL_EN           (DISPSYS_COLOR_BASE + 0x0F24)
#define DISP_REG_COLOR_CK_ON                (DISPSYS_COLOR_BASE + 0x0F28)
#define DISP_REG_COLOR_INTERNAL_IP_WIDTH    (DISPSYS_COLOR_BASE + 0x0F50)
#define DISP_REG_COLOR_INTERNAL_IP_HEIGHT   (DISPSYS_COLOR_BASE + 0x0F54)
#define DISP_REG_COLOR_CM1_EN               (DISPSYS_COLOR_BASE + 0x0F60)
#define DISP_REG_COLOR_H_CNT                (DISPSYS_COLOR_BASE + 0x0404)
#define DISP_REG_COLOR_L_CNT                (DISPSYS_COLOR_BASE + 0x0408)

//-----------------------------------------------------------------
// Config
#define DISP_REG_CONFIG_CAM_MDP_MOUT_EN           (DISPSYS_CONFIG_BASE + 0x01c)	 
#define DISP_REG_CONFIG_MDP_RDMA_MOUT_EN          (DISPSYS_CONFIG_BASE + 0x020)	
#define DISP_REG_CONFIG_MDP_RSZ0_MOUT_EN          (DISPSYS_CONFIG_BASE + 0x024)	
#define DISP_REG_CONFIG_MDP_RSZ1_MOUT_EN          (DISPSYS_CONFIG_BASE + 0x028)	
#define DISP_REG_CONFIG_MDP_TDSHP_MOUT_EN         (DISPSYS_CONFIG_BASE + 0x02c)	
#define DISP_REG_CONFIG_DISP_OVL_MOUT_EN          (DISPSYS_CONFIG_BASE + 0x030)	
#define DISP_REG_CONFIG_MMSYS_MOUT_RST            (DISPSYS_CONFIG_BASE + 0x034)	
#define DISP_REG_CONFIG_MDP_RSZ0_SEL              (DISPSYS_CONFIG_BASE + 0x038)	
#define DISP_REG_CONFIG_MDP_RSZ1_SEL              (DISPSYS_CONFIG_BASE + 0x03c)	
#define DISP_REG_CONFIG_MDP_TDSHP_SEL             (DISPSYS_CONFIG_BASE + 0x040)	
#define DISP_REG_CONFIG_MDP_WROT_SEL              (DISPSYS_CONFIG_BASE + 0x044)	
#define DISP_REG_CONFIG_MDP_WDMA_SEL              (DISPSYS_CONFIG_BASE + 0x048)	
#define DISP_REG_CONFIG_DISP_OUT_SEL              (DISPSYS_CONFIG_BASE + 0x04c)	
#define DISP_REG_CONFIG_MMSYS_CG_CON0             (DISPSYS_CONFIG_BASE + 0x100)	
#define DISP_REG_CONFIG_MMSYS_CG_SET0             (DISPSYS_CONFIG_BASE + 0x104)	
#define DISP_REG_CONFIG_MMSYS_CG_CLR0             (DISPSYS_CONFIG_BASE + 0x108)	
#define DISP_REG_CONFIG_MMSYS_CG_CON1             (DISPSYS_CONFIG_BASE + 0x110)	
#define DISP_REG_CONFIG_MMSYS_CG_SET1             (DISPSYS_CONFIG_BASE + 0x114)	
#define DISP_REG_CONFIG_MMSYS_CG_CLR1             (DISPSYS_CONFIG_BASE + 0x118)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS0         (DISPSYS_CONFIG_BASE + 0x120)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET0     (DISPSYS_CONFIG_BASE + 0x124)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR0     (DISPSYS_CONFIG_BASE + 0x128)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS1         (DISPSYS_CONFIG_BASE + 0x12c)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_SET1     (DISPSYS_CONFIG_BASE + 0x130)	
#define DISP_REG_CONFIG_MMSYS_HW_DCM_DIS_CLR1     (DISPSYS_CONFIG_BASE + 0x134)	
#define DISP_REG_CONFIG_MMSYS_SW_RST_B            (DISPSYS_CONFIG_BASE + 0x138)	
#define DISP_REG_CONFIG_MMSYS_LCM_RST_B           (DISPSYS_CONFIG_BASE + 0x13c)	
#define DISP_REG_CONFIG_MMSYS_MBIST_DONE          (DISPSYS_CONFIG_BASE + 0x800)	
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL0         (DISPSYS_CONFIG_BASE + 0x804)	
#define DISP_REG_CONFIG_MMSYS_MBIST_FAIL1         (DISPSYS_CONFIG_BASE + 0x808)	
#define DISP_REG_CONFIG_MMSYS_MBIST_HOLDB         (DISPSYS_CONFIG_BASE + 0x80C)	
#define DISP_REG_CONFIG_MMSYS_MBIST_MODE          (DISPSYS_CONFIG_BASE + 0x810)	
#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL0	      (DISPSYS_CONFIG_BASE + 0x814)	
#define DISP_REG_CONFIG_MMSYS_MBIST_BSEL1	      (DISPSYS_CONFIG_BASE + 0x818)	
#define DISP_REG_CONFIG_MMSYS_MBIST_CON	          (DISPSYS_CONFIG_BASE + 0x81c)	
#define DISP_REG_CONFIG_MMSYS_MEM_DELSEL0         (DISPSYS_CONFIG_BASE + 0x820)	
#define DISP_REG_CONFIG_MMSYS_MEM_DELSEL1         (DISPSYS_CONFIG_BASE + 0x824)	
#define DISP_REG_CONFIG_MMSYS_MEM_DELSEL2         (DISPSYS_CONFIG_BASE + 0x828)	
#define DISP_REG_CONFIG_MMSYS_MEM_DELSEL3         (DISPSYS_CONFIG_BASE + 0x82c)	
#define DISP_REG_CONFIG_MMSYS_DEBUG_OUT_SEL       (DISPSYS_CONFIG_BASE + 0x830)	
#define DISP_REG_CONFIG_MMSYS_DUMMY               (DISPSYS_CONFIG_BASE + 0x840)	
#define DISP_REG_CONFIG_CLOCK_DUMMY               (0xf0206040)	

#define DISP_REG_CONFIG_VALID               (DISPSYS_CONFIG_BASE + 0x860)	
#define DISP_REG_CONFIG_READY               (DISPSYS_CONFIG_BASE + 0x868)	
#define DISP_REG_CONFIG_GREQ                (DDP_REG_BASE_SMI_LARB0 + 0x450)	
//-----------------------------------------------------------------
// Mutex
#define MUTEX_RESOURCE_NUM                   6
#define DISP_REG_CONFIG_MUTEX_INTEN          (DISPSYS_MUTEX_BASE + 0x0)
#define DISP_REG_CONFIG_MUTEX_INTSTA         (DISPSYS_MUTEX_BASE + 0x4)
#define DISP_REG_CONFIG_REG_UPD_TIMEOUT      (DISPSYS_MUTEX_BASE + 0x8)
#define DISP_REG_CONFIG_REG_COMMIT           (DISPSYS_MUTEX_BASE + 0xC)
#define DISP_REG_CONFIG_MUTEX_EN(n)          (DISPSYS_MUTEX_BASE + 0x20 + (0x20 * n))
#define DISP_REG_CONFIG_MUTEX(n)             (DISPSYS_MUTEX_BASE + 0x24 + (0x20 * n))
#define DISP_REG_CONFIG_MUTEX_RST(n)         (DISPSYS_MUTEX_BASE + 0x28 + (0x20 * n))
#define DISP_REG_CONFIG_MUTEX_MOD(n)         (DISPSYS_MUTEX_BASE + 0x2C + (0x20 * n))
#define DISP_REG_CONFIG_MUTEX_SOF(n)         (DISPSYS_MUTEX_BASE + 0x30 + (0x20 * n))
#define DISP_REG_CONFIG_MUTEX0_EN            (DISPSYS_MUTEX_BASE + 0x20)    
#define DISP_REG_CONFIG_MUTEX0               (DISPSYS_MUTEX_BASE + 0x24)
#define DISP_REG_CONFIG_MUTEX0_RST           (DISPSYS_MUTEX_BASE + 0x28)
#define DISP_REG_CONFIG_MUTEX0_MOD           (DISPSYS_MUTEX_BASE + 0x2C)
#define DISP_REG_CONFIG_MUTEX0_SOF           (DISPSYS_MUTEX_BASE + 0x30)
#define DISP_REG_CONFIG_MUTEX1_EN            (DISPSYS_MUTEX_BASE + 0x40)
#define DISP_REG_CONFIG_MUTEX1               (DISPSYS_MUTEX_BASE + 0x44)
#define DISP_REG_CONFIG_MUTEX1_RST           (DISPSYS_MUTEX_BASE + 0x48)
#define DISP_REG_CONFIG_MUTEX1_MOD           (DISPSYS_MUTEX_BASE + 0x4C)
#define DISP_REG_CONFIG_MUTEX1_SOF           (DISPSYS_MUTEX_BASE + 0x50)
#define DISP_REG_CONFIG_MUTEX2_EN            (DISPSYS_MUTEX_BASE + 0x60)
#define DISP_REG_CONFIG_MUTEX2               (DISPSYS_MUTEX_BASE + 0x64)
#define DISP_REG_CONFIG_MUTEX2_RST           (DISPSYS_MUTEX_BASE + 0x68)
#define DISP_REG_CONFIG_MUTEX2_MOD           (DISPSYS_MUTEX_BASE + 0x6C)
#define DISP_REG_CONFIG_MUTEX2_SOF           (DISPSYS_MUTEX_BASE + 0x70)
#define DISP_REG_CONFIG_MUTEX3_EN            (DISPSYS_MUTEX_BASE + 0x80)
#define DISP_REG_CONFIG_MUTEX3               (DISPSYS_MUTEX_BASE + 0x84)
#define DISP_REG_CONFIG_MUTEX3_RST           (DISPSYS_MUTEX_BASE + 0x88)
#define DISP_REG_CONFIG_MUTEX3_MOD           (DISPSYS_MUTEX_BASE + 0x8C)
#define DISP_REG_CONFIG_MUTEX3_SOF           (DISPSYS_MUTEX_BASE + 0x90)
#define DISP_REG_CONFIG_MUTEX4_EN            (DISPSYS_MUTEX_BASE + 0xA0)
#define DISP_REG_CONFIG_MUTEX4               (DISPSYS_MUTEX_BASE + 0xA4)
#define DISP_REG_CONFIG_MUTEX4_RST           (DISPSYS_MUTEX_BASE + 0xA8)
#define DISP_REG_CONFIG_MUTEX4_MOD           (DISPSYS_MUTEX_BASE + 0xAC)
#define DISP_REG_CONFIG_MUTEX4_SOF           (DISPSYS_MUTEX_BASE + 0xB0)
#define DISP_REG_CONFIG_MUTEX5_EN            (DISPSYS_MUTEX_BASE + 0xC0)
#define DISP_REG_CONFIG_MUTEX5               (DISPSYS_MUTEX_BASE + 0xC4)
#define DISP_REG_CONFIG_MUTEX5_RST                      (DISPSYS_MUTEX_BASE + 0xC8)
#define DISP_REG_CONFIG_MUTEX5_MOD                      (DISPSYS_MUTEX_BASE + 0xCC)
#define DISP_REG_CONFIG_MUTEX5_SOF                      (DISPSYS_MUTEX_BASE + 0xD0)
#define DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL             (DISPSYS_MUTEX_BASE + 0x100)  

//-----------------------------------------------------------------
// OVL
#define DISP_REG_OVL_STA                         (DISPSYS_OVL_BASE + 0x0000)
#define DISP_REG_OVL_INTEN                       (DISPSYS_OVL_BASE + 0x0004)
#define DISP_REG_OVL_INTSTA                      (DISPSYS_OVL_BASE + 0x0008)
#define DISP_REG_OVL_EN                          (DISPSYS_OVL_BASE + 0x000C)
#define DISP_REG_OVL_TRIG                        (DISPSYS_OVL_BASE + 0x0010)
#define DISP_REG_OVL_RST                         (DISPSYS_OVL_BASE + 0x0014)
#define DISP_REG_OVL_ROI_SIZE                    (DISPSYS_OVL_BASE + 0x0020)
#define DISP_REG_OVL_DATAPATH_CON                (DISPSYS_OVL_BASE + 0x0024)
#define DISP_REG_OVL_ROI_BGCLR                   (DISPSYS_OVL_BASE + 0x0028)
#define DISP_REG_OVL_SRC_CON                     (DISPSYS_OVL_BASE + 0x002C)
#define DISP_REG_OVL_L0_CON                      (DISPSYS_OVL_BASE + 0x0030)
#define DISP_REG_OVL_L0_SRCKEY                   (DISPSYS_OVL_BASE + 0x0034)
#define DISP_REG_OVL_L0_SRC_SIZE                 (DISPSYS_OVL_BASE + 0x0038)
#define DISP_REG_OVL_L0_OFFSET                   (DISPSYS_OVL_BASE + 0x003C)
#define DISP_REG_OVL_L0_ADDR                     (DISPSYS_OVL_BASE + 0x0040)
#define DISP_REG_OVL_L0_PITCH                    (DISPSYS_OVL_BASE + 0x0044)
#define DISP_REG_OVL_L1_CON                      (DISPSYS_OVL_BASE + 0x0050)
#define DISP_REG_OVL_L1_SRCKEY                   (DISPSYS_OVL_BASE + 0x0054)
#define DISP_REG_OVL_L1_SRC_SIZE                 (DISPSYS_OVL_BASE + 0x0058)
#define DISP_REG_OVL_L1_OFFSET                   (DISPSYS_OVL_BASE + 0x005C)
#define DISP_REG_OVL_L1_ADDR                     (DISPSYS_OVL_BASE + 0x0060)
#define DISP_REG_OVL_L1_PITCH                    (DISPSYS_OVL_BASE + 0x0064)
#define DISP_REG_OVL_L2_CON                      (DISPSYS_OVL_BASE + 0x0070)
#define DISP_REG_OVL_L2_SRCKEY                   (DISPSYS_OVL_BASE + 0x0074)
#define DISP_REG_OVL_L2_SRC_SIZE                 (DISPSYS_OVL_BASE + 0x0078)
#define DISP_REG_OVL_L2_OFFSET                   (DISPSYS_OVL_BASE + 0x007C)
#define DISP_REG_OVL_L2_ADDR                     (DISPSYS_OVL_BASE + 0x0080)
#define DISP_REG_OVL_L2_PITCH                    (DISPSYS_OVL_BASE + 0x0084)
#define DISP_REG_OVL_L3_CON                      (DISPSYS_OVL_BASE + 0x0090)
#define DISP_REG_OVL_L3_SRCKEY                   (DISPSYS_OVL_BASE + 0x0094)
#define DISP_REG_OVL_L3_SRC_SIZE                 (DISPSYS_OVL_BASE + 0x0098)
#define DISP_REG_OVL_L3_OFFSET                   (DISPSYS_OVL_BASE + 0x009C)
#define DISP_REG_OVL_L3_ADDR                     (DISPSYS_OVL_BASE + 0x00A0)
#define DISP_REG_OVL_L3_PITCH                    (DISPSYS_OVL_BASE + 0x00A4)
#define DISP_REG_OVL_RDMA0_CTRL                  (DISPSYS_OVL_BASE + 0x00C0)
#define DISP_REG_OVL_RDMA0_MEM_START_TRIG        (DISPSYS_OVL_BASE + 0x00C4)
#define DISP_REG_OVL_RDMA0_MEM_GMC_SETTING       (DISPSYS_OVL_BASE + 0x00C8)
#define DISP_REG_OVL_RDMA0_MEM_SLOW_CON          (DISPSYS_OVL_BASE + 0x00CC)
#define DISP_REG_OVL_RDMA0_FIFO_CTRL             (DISPSYS_OVL_BASE + 0x00D0)
#define DISP_REG_OVL_RDMA1_CTRL                  (DISPSYS_OVL_BASE + 0x00E0)
#define DISP_REG_OVL_RDMA1_MEM_START_TRIG        (DISPSYS_OVL_BASE + 0x00E4)
#define DISP_REG_OVL_RDMA1_MEM_GMC_SETTING       (DISPSYS_OVL_BASE + 0x00E8)
#define DISP_REG_OVL_RDMA1_MEM_SLOW_CON          (DISPSYS_OVL_BASE + 0x00EC)
#define DISP_REG_OVL_RDMA1_FIFO_CTRL             (DISPSYS_OVL_BASE + 0x00F0)
#define DISP_REG_OVL_RDMA2_CTRL                  (DISPSYS_OVL_BASE + 0x0100)
#define DISP_REG_OVL_RDMA2_MEM_START_TRIG        (DISPSYS_OVL_BASE + 0x0104)
#define DISP_REG_OVL_RDMA2_MEM_GMC_SETTING       (DISPSYS_OVL_BASE + 0x0108)
#define DISP_REG_OVL_RDMA2_MEM_SLOW_CON          (DISPSYS_OVL_BASE + 0x010C)
#define DISP_REG_OVL_RDMA2_FIFO_CTRL             (DISPSYS_OVL_BASE + 0x0110)
#define DISP_REG_OVL_RDMA3_CTRL                  (DISPSYS_OVL_BASE + 0x0120)
#define DISP_REG_OVL_RDMA3_MEM_START_TRIG        (DISPSYS_OVL_BASE + 0x0124)
#define DISP_REG_OVL_RDMA3_MEM_GMC_SETTING       (DISPSYS_OVL_BASE + 0x0128)
#define DISP_REG_OVL_RDMA3_MEM_SLOW_CON          (DISPSYS_OVL_BASE + 0x012C)
#define DISP_REG_OVL_RDMA3_FIFO_CTRL             (DISPSYS_OVL_BASE + 0x0130)
#define DISP_REG_OVL_L0_Y2R_PARA_R0              (DISPSYS_OVL_BASE + 0x0134)
#define DISP_REG_OVL_L0_Y2R_PARA_R1              (DISPSYS_OVL_BASE + 0x0138)
#define DISP_REG_OVL_L0_Y2R_PARA_G0              (DISPSYS_OVL_BASE + 0x013C)
#define DISP_REG_OVL_L0_Y2R_PARA_G1              (DISPSYS_OVL_BASE + 0x0140)
#define DISP_REG_OVL_L0_Y2R_PARA_B0              (DISPSYS_OVL_BASE + 0x0144)
#define DISP_REG_OVL_L0_Y2R_PARA_B1              (DISPSYS_OVL_BASE + 0x0148)
#define DISP_REG_OVL_L0_Y2R_PARA_YUV_A_0         (DISPSYS_OVL_BASE + 0x014C)
#define DISP_REG_OVL_L0_Y2R_PARA_YUV_A_1         (DISPSYS_OVL_BASE + 0x0150)
#define DISP_REG_OVL_L0_Y2R_PARA_RGB_A_0         (DISPSYS_OVL_BASE + 0x0154)
#define DISP_REG_OVL_L0_Y2R_PARA_RGB_A_1         (DISPSYS_OVL_BASE + 0x0158)
#define DISP_REG_OVL_L1_Y2R_PARA_R0              (DISPSYS_OVL_BASE + 0x015C)
#define DISP_REG_OVL_L1_Y2R_PARA_R1              (DISPSYS_OVL_BASE + 0x0160)
#define DISP_REG_OVL_L1_Y2R_PARA_G0              (DISPSYS_OVL_BASE + 0x0164)
#define DISP_REG_OVL_L1_Y2R_PARA_G1              (DISPSYS_OVL_BASE + 0x0168)
#define DISP_REG_OVL_L1_Y2R_PARA_B0              (DISPSYS_OVL_BASE + 0x016C)
#define DISP_REG_OVL_L1_Y2R_PARA_B1              (DISPSYS_OVL_BASE + 0x0170)
#define DISP_REG_OVL_L1_Y2R_PARA_YUV_A_0         (DISPSYS_OVL_BASE + 0x0174)
#define DISP_REG_OVL_L1_Y2R_PARA_YUV_A_1         (DISPSYS_OVL_BASE + 0x0178)
#define DISP_REG_OVL_L1_Y2R_PARA_RGB_A_0         (DISPSYS_OVL_BASE + 0x017C)
#define DISP_REG_OVL_L1_Y2R_PARA_RGB_A_1         (DISPSYS_OVL_BASE + 0x0180)
#define DISP_REG_OVL_L2_Y2R_PARA_R0              (DISPSYS_OVL_BASE + 0x0184)
#define DISP_REG_OVL_L2_Y2R_PARA_R1              (DISPSYS_OVL_BASE + 0x0188)
#define DISP_REG_OVL_L2_Y2R_PARA_G0              (DISPSYS_OVL_BASE + 0x018C)
#define DISP_REG_OVL_L2_Y2R_PARA_G1              (DISPSYS_OVL_BASE + 0x0190)
#define DISP_REG_OVL_L2_Y2R_PARA_B0              (DISPSYS_OVL_BASE + 0x0194)
#define DISP_REG_OVL_L2_Y2R_PARA_B1              (DISPSYS_OVL_BASE + 0x0198)
#define DISP_REG_OVL_L2_Y2R_PARA_YUV_A_0         (DISPSYS_OVL_BASE + 0x019C)
#define DISP_REG_OVL_L2_Y2R_PARA_YUV_A_1         (DISPSYS_OVL_BASE + 0x01A0)
#define DISP_REG_OVL_L2_Y2R_PARA_RGB_A_0         (DISPSYS_OVL_BASE + 0x01A4)
#define DISP_REG_OVL_L2_Y2R_PARA_RGB_A_1         (DISPSYS_OVL_BASE + 0x01A8)
#define DISP_REG_OVL_L3_Y2R_PARA_R0              (DISPSYS_OVL_BASE + 0x01AC)
#define DISP_REG_OVL_L3_Y2R_PARA_R1              (DISPSYS_OVL_BASE + 0x01B0)
#define DISP_REG_OVL_L3_Y2R_PARA_G0              (DISPSYS_OVL_BASE + 0x01B4)
#define DISP_REG_OVL_L3_Y2R_PARA_G1              (DISPSYS_OVL_BASE + 0x01B8)
#define DISP_REG_OVL_L3_Y2R_PARA_B0              (DISPSYS_OVL_BASE + 0x01BC)
#define DISP_REG_OVL_L3_Y2R_PARA_B1              (DISPSYS_OVL_BASE + 0x01C0)
#define DISP_REG_OVL_L3_Y2R_PARA_YUV_A_0         (DISPSYS_OVL_BASE + 0x01C4)
#define DISP_REG_OVL_L3_Y2R_PARA_YUV_A_1         (DISPSYS_OVL_BASE + 0x01C8)
#define DISP_REG_OVL_L3_Y2R_PARA_RGB_A_0         (DISPSYS_OVL_BASE + 0x01CC)
#define DISP_REG_OVL_L3_Y2R_PARA_RGB_A_1         (DISPSYS_OVL_BASE + 0x01D0)
#define DISP_REG_OVL_DEBUG_MON_SEL               (DISPSYS_OVL_BASE + 0x01D4)
#define DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2      (DISPSYS_OVL_BASE + 0x01E0)
#define DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2      (DISPSYS_OVL_BASE + 0x01E4)
#define DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2      (DISPSYS_OVL_BASE + 0x01E8)
#define DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2      (DISPSYS_OVL_BASE + 0x01EC)
#define DISP_REG_OVL_DUMMY                       (DISPSYS_OVL_BASE + 0x0200)
#define DISP_REG_OVL_FLOW_CTRL_DBG               (DISPSYS_OVL_BASE + 0x0240)
#define DISP_REG_OVL_ADDCON_DBG                  (DISPSYS_OVL_BASE + 0x0244)
#define DISP_REG_OVL_OUTMUX_DBG                  (DISPSYS_OVL_BASE + 0x0248)
#define DISP_REG_OVL_RDMA0_DBG                   (DISPSYS_OVL_BASE + 0x024C)
#define DISP_REG_OVL_RDMA1_DBG                   (DISPSYS_OVL_BASE + 0x0250)
#define DISP_REG_OVL_RDMA2_DBG                   (DISPSYS_OVL_BASE + 0x0254)
#define DISP_REG_OVL_RDMA3_DBG                   (DISPSYS_OVL_BASE + 0x0258)

//-----------------------------------------------------------------
// BLS
#define DISP_REG_BLS_EN                         (DISPSYS_BLS_BASE + 0x0000)
#define DISP_REG_BLS_RST                        (DISPSYS_BLS_BASE + 0x0004)
#define DISP_REG_BLS_INTEN                      (DISPSYS_BLS_BASE + 0x0008)
#define DISP_REG_BLS_INTSTA                     (DISPSYS_BLS_BASE + 0x000C)
#define DISP_REG_BLS_BLS_SETTING                (DISPSYS_BLS_BASE + 0x0010)
#define DISP_REG_BLS_FANA_SETTING               (DISPSYS_BLS_BASE + 0x0014)
#define DISP_REG_BLS_SRC_SIZE	                (DISPSYS_BLS_BASE + 0x0018)
//#define DISP_REG_RDMA_TARGET_LINE               (DISPSYS_RDMA0_BASE + 0x001C)
#define DISP_REG_BLS_GAIN_SETTING               (DISPSYS_BLS_BASE + 0x0020)
#define DISP_REG_BLS_MANUAL_GAIN                (DISPSYS_BLS_BASE + 0x0024)
#define DISP_REG_BLS_MANUAL_MAXCLR              (DISPSYS_BLS_BASE + 0x0028)
#define DISP_REG_BLS_GAMMA_SETTING              (DISPSYS_BLS_BASE + 0x0030)
#define DISP_REG_BLS_GAMMA_BOUNDARY             (DISPSYS_BLS_BASE + 0x0034)
#define DISP_REG_BLS_LUT_UPDATE                 (DISPSYS_BLS_BASE + 0x0038)
#define DISP_REG_BLS_MAXCLR_THD                 (DISPSYS_BLS_BASE + 0x0060)
#define DISP_REG_BLS_DISTPT_THD                 (DISPSYS_BLS_BASE + 0x0064)
#define DISP_REG_BLS_MAXCLR_LIMIT               (DISPSYS_BLS_BASE + 0x0068)
#define DISP_REG_BLS_DISTPT_LIMIT               (DISPSYS_BLS_BASE + 0x006C)
#define DISP_REG_BLS_AVE_SETTING                (DISPSYS_BLS_BASE + 0x0070)
#define DISP_REG_BLS_AVE_LIMIT	                (DISPSYS_BLS_BASE + 0x0074)
#define DISP_REG_BLS_DISTPT_SETTING             (DISPSYS_BLS_BASE + 0x0078)
#define DISP_REG_BLS_HIS_CLEAR	                (DISPSYS_BLS_BASE + 0x007C)
#define DISP_REG_BLS_SC_DIFF_THD                (DISPSYS_BLS_BASE + 0x0080)
#define DISP_REG_BLS_SC_BIN_THD                 (DISPSYS_BLS_BASE + 0x0084)
#define DISP_REG_BLS_MAXCLR_GRADUAL             (DISPSYS_BLS_BASE + 0x0088)
#define DISP_REG_BLS_DISTPT_GRADUAL             (DISPSYS_BLS_BASE + 0x008C)
#define DISP_REG_BLS_FAST_IIR_XCOEFF            (DISPSYS_BLS_BASE + 0x0090)
#define DISP_REG_BLS_FAST_IIR_YCOEFF            (DISPSYS_BLS_BASE + 0x0094)
#define DISP_REG_BLS_SLOW_IIR_XCOEFF            (DISPSYS_BLS_BASE + 0x0098)
#define DISP_REG_BLS_SLOW_IIR_YCOEFF            (DISPSYS_BLS_BASE + 0x009C)
#define DISP_REG_BLS_PWM_DUTY	                (DISPSYS_BLS_BASE + 0x00A0)
#define DISP_REG_BLS_PWM_GRADUAL                (DISPSYS_BLS_BASE + 0x00A4)
#define DISP_REG_BLS_PWM_CON                    (DISPSYS_BLS_BASE + 0x00A8)
#define DISP_REG_BLS_PWM_MANUAL	                (DISPSYS_BLS_BASE + 0x00AC)
#define DISP_REG_BLS_DEBUG                      (DISPSYS_BLS_BASE + 0x00B0)
#define DISP_REG_BLS_PATTERN                    (DISPSYS_BLS_BASE + 0x00B4)
#define DISP_REG_BLS_CHKSUM                     (DISPSYS_BLS_BASE + 0x00B8)
#define DISP_REG_BLS_HIS_BIN(X)                 (DISPSYS_BLS_BASE + 0x0100 + (4*(X)))
#define DISP_REG_BLS_PWM_DUTY_RD                (DISPSYS_BLS_BASE + 0x0200)
#define DISP_REG_BLS_FRAME_AVE_RD               (DISPSYS_BLS_BASE + 0x0204)
#define DISP_REG_BLS_MAXCLR_RD	                (DISPSYS_BLS_BASE + 0x0208)
#define DISP_REG_BLS_DISTPT_RD	                (DISPSYS_BLS_BASE + 0x020C)
#define DISP_REG_BLS_GAIN_RD                    (DISPSYS_BLS_BASE + 0x0210)
#define DISP_REG_BLS_SC_RD                      (DISPSYS_BLS_BASE + 0x0214)
#define DISP_REG_BLS_LUMINANCE(X)               (DISPSYS_BLS_BASE + 0x0300 + (4*(X)))
#define DISP_REG_BLS_LUMINANCE_255              (DISPSYS_BLS_BASE + 0x0384)
#define DISP_REG_BLS_GAMMA_LUT(X)               (DISPSYS_BLS_BASE + 0x0400 + (4*(X)))
#define DISP_REG_BLS_DITHER(X)                  (DISPSYS_BLS_BASE + 0x0E00 + (4*(X)))

//-----------------------------------------------------------------
// RDMA
#define DISP_REG_RDMA_INT_ENABLE              (DISPSYS_RDMA1_BASE + 0x00)
#define DISP_REG_RDMA_INT_STATUS              (DISPSYS_RDMA1_BASE + 0x04)
#define DISP_REG_RDMA_GLOBAL_CON              (DISPSYS_RDMA1_BASE + 0x10)
#define DISP_REG_RDMA_SIZE_CON_0              (DISPSYS_RDMA1_BASE + 0x14)
#define DISP_REG_RDMA_SIZE_CON_1              (DISPSYS_RDMA1_BASE + 0x18)
#define DISP_REG_RDMA_TARGET_LINE             (DISPSYS_RDMA1_BASE + 0x1C)
#define DISP_REG_RDMA_MEM_CON                 (DISPSYS_RDMA1_BASE + 0x24)
#define DISP_REG_RDMA_MEM_START_ADDR          (DISPSYS_RDMA1_BASE + 0x28)
#define DISP_REG_RDMA_MEM_SRC_PITCH           (DISPSYS_RDMA1_BASE + 0x2C)
#define DISP_REG_RDMA_MEM_GMC_SETTING_0       (DISPSYS_RDMA1_BASE + 0x30)
#define DISP_REG_RDMA_MEM_SLOW_CON            (DISPSYS_RDMA1_BASE + 0x34)
#define DISP_REG_RDMA_MEM_GMC_SETTING_1       (DISPSYS_RDMA1_BASE + 0x38)
#define DISP_REG_RDMA_FIFO_CON                (DISPSYS_RDMA1_BASE + 0x40)
#define DISP_REG_RDMA_FIFO_LOG                (DISPSYS_RDMA1_BASE + 0x44)
#define DISP_REG_RDMA_CF_00                   (DISPSYS_RDMA1_BASE + 0x54)
#define DISP_REG_RDMA_CF_01                   (DISPSYS_RDMA1_BASE + 0x58)
#define DISP_REG_RDMA_CF_02                   (DISPSYS_RDMA1_BASE + 0x5C)
#define DISP_REG_RDMA_CF_10                   (DISPSYS_RDMA1_BASE + 0x60)
#define DISP_REG_RDMA_CF_11                   (DISPSYS_RDMA1_BASE + 0x64)
#define DISP_REG_RDMA_CF_12                   (DISPSYS_RDMA1_BASE + 0x68)
#define DISP_REG_RDMA_CF_20                   (DISPSYS_RDMA1_BASE + 0x6C)
#define DISP_REG_RDMA_CF_21                   (DISPSYS_RDMA1_BASE + 0x70)
#define DISP_REG_RDMA_CF_22                   (DISPSYS_RDMA1_BASE + 0x74)
#define DISP_REG_RDMA_CF_PRE_ADD0             (DISPSYS_RDMA1_BASE + 0x78)
#define DISP_REG_RDMA_CF_PRE_ADD1             (DISPSYS_RDMA1_BASE + 0x7C)
#define DISP_REG_RDMA_CF_PRE_ADD2             (DISPSYS_RDMA1_BASE + 0x80)
#define DISP_REG_RDMA_CF_POST_ADD0            (DISPSYS_RDMA1_BASE + 0x84)
#define DISP_REG_RDMA_CF_POST_ADD1            (DISPSYS_RDMA1_BASE + 0x88)
#define DISP_REG_RDMA_CF_POST_ADD2            (DISPSYS_RDMA1_BASE + 0x8C)
#define DISP_REG_RDMA_DUMMY                   (DISPSYS_RDMA1_BASE + 0x90)
#define DISP_REG_RDMA_DEBUG_OUT_SEL           (DISPSYS_RDMA1_BASE + 0x94)



//-----------------------------------------------------------------
// WDMA
#define DISP_REG_WDMA_INTEN               (DISPSYS_WDMA0_BASE+0x00)
#define DISP_REG_WDMA_INTSTA              (DISPSYS_WDMA0_BASE+0x04)
#define DISP_REG_WDMA_EN                  (DISPSYS_WDMA0_BASE+0x08)
#define DISP_REG_WDMA_RST                 (DISPSYS_WDMA0_BASE+0x0C)
#define DISP_REG_WDMA_SMI_CON             (DISPSYS_WDMA0_BASE+0x10)
#define DISP_REG_WDMA_CFG                 (DISPSYS_WDMA0_BASE+0x14)
#define DISP_REG_WDMA_SRC_SIZE            (DISPSYS_WDMA0_BASE+0x18)
#define DISP_REG_WDMA_CLIP_SIZE           (DISPSYS_WDMA0_BASE+0x1C)
#define DISP_REG_WDMA_CLIP_COORD          (DISPSYS_WDMA0_BASE+0x20)
#define DISP_REG_WDMA_DST_ADDR            (DISPSYS_WDMA0_BASE+0x24)
#define DISP_REG_WDMA_DST_W_IN_BYTE       (DISPSYS_WDMA0_BASE+0x28)
#define DISP_REG_WDMA_ALPHA               (DISPSYS_WDMA0_BASE+0x2C)
#define DISP_REG_WDMA_BUF_ADDR            (DISPSYS_WDMA0_BASE+0x30)
#define DISP_REG_WDMA_STA                 (DISPSYS_WDMA0_BASE+0x34)
#define DISP_REG_WDMA_BUF_CON1            (DISPSYS_WDMA0_BASE+0x38)
#define DISP_REG_WDMA_BUF_CON2            (DISPSYS_WDMA0_BASE+0x3C)
#define DISP_REG_WDMA_C00                 (DISPSYS_WDMA0_BASE+0x40)
#define DISP_REG_WDMA_C02                 (DISPSYS_WDMA0_BASE+0x44)
#define DISP_REG_WDMA_C10                 (DISPSYS_WDMA0_BASE+0x48)
#define DISP_REG_WDMA_C12                 (DISPSYS_WDMA0_BASE+0x4C)
#define DISP_REG_WDMA_C20                 (DISPSYS_WDMA0_BASE+0x50)
#define DISP_REG_WDMA_C22                 (DISPSYS_WDMA0_BASE+0x54)
#define DISP_REG_WDMA_PRE_ADD0            (DISPSYS_WDMA0_BASE+0x58)
#define DISP_REG_WDMA_PRE_ADD2            (DISPSYS_WDMA0_BASE+0x5C)
#define DISP_REG_WDMA_POST_ADD0           (DISPSYS_WDMA0_BASE+0x60)
#define DISP_REG_WDMA_POST_ADD2           (DISPSYS_WDMA0_BASE+0x64)
#define DISP_REG_WDMA_DST_U_ADDR          (DISPSYS_WDMA0_BASE+0x70)
#define DISP_REG_WDMA_DST_V_ADDR          (DISPSYS_WDMA0_BASE+0x74)
#define DISP_REG_WDMA_DST_UV_PITCH        (DISPSYS_WDMA0_BASE+0x78)
#define DISP_REG_WDMA_DITHER_CON          (DISPSYS_WDMA0_BASE+0x90)
#define DISP_REG_WDMA_FLOW_CTRL_DBG       (DISPSYS_WDMA0_BASE+0xA0)
#define DISP_REG_WDMA_EXEC_DBG            (DISPSYS_WDMA0_BASE+0xA4)
#define DISP_REG_WDMA_CLIP_DBG            (DISPSYS_WDMA0_BASE+0xA8)

//-----------------------------------------------------------------
// CMDQ
#define DISP_INT_MUTEX_BIT_MASK     0x00000002

//-----------------------------------------------------------------
// G2D
#define DISP_REG_G2D_START                (DISPSYS_G2D_BASE + 0x00)
#define DISP_REG_G2D_MODE_CON             (DISPSYS_G2D_BASE + 0x04)
#define DISP_REG_G2D_RESET                (DISPSYS_G2D_BASE + 0x08)
#define DISP_REG_G2D_STATUS               (DISPSYS_G2D_BASE + 0x0C)
#define DISP_REG_G2D_IRQ                  (DISPSYS_G2D_BASE + 0x10)

#define DISP_REG_G2D_ALP_CON              (DISPSYS_G2D_BASE + 0x18)

#define DISP_REG_G2D_W2M_CON              (DISPSYS_G2D_BASE + 0x40)
#define DISP_REG_G2D_W2M_ADDR             (DISPSYS_G2D_BASE + 0x44)
#define DISP_REG_G2D_W2M_PITCH            (DISPSYS_G2D_BASE + 0x48)
#define DISP_REG_G2D_W2M_SIZE             (DISPSYS_G2D_BASE + 0x50)

#define DISP_REG_G2D_DST_CON              (DISPSYS_G2D_BASE + 0x80)
#define DISP_REG_G2D_DST_ADDR             (DISPSYS_G2D_BASE + 0x84)
#define DISP_REG_G2D_DST_PITCH            (DISPSYS_G2D_BASE + 0x88)
#define DISP_REG_G2D_DST_COLOR            (DISPSYS_G2D_BASE + 0x94)

#define DISP_REG_G2D_SRC_CON              (DISPSYS_G2D_BASE + 0xC0)
#define DISP_REG_G2D_SRC_ADDR             (DISPSYS_G2D_BASE + 0xC4)
#define DISP_REG_G2D_SRC_PITCH            (DISPSYS_G2D_BASE + 0xC8)
#define DISP_REG_G2D_SRC_COLOR            (DISPSYS_G2D_BASE + 0xD4)

#define DISP_REG_G2D_DI_MAT_0             (DISPSYS_G2D_BASE + 0xD8)
#define DISP_REG_G2D_DI_MAT_1             (DISPSYS_G2D_BASE + 0xDC)

#define DISP_REG_G2D_PMC                  (DISPSYS_G2D_BASE + 0xE0)

#define G2D_RESET_APB_RESET_BIT                 0x0004
#define G2D_RESET_HARD_RESET_BIT                0x0002
#define G2D_RESET_WARM_RESET_BIT                0x0001
#define G2D_STATUS_BUSY_BIT                     0x0001
#define G2D_IRQ_STA_BIT			                0x0100
#define G2D_IRQ_ENABLE_BIT                      0x0001

#define DISP_REG_CMDQ_SYNC_TOKEN_UPDATE                  (DISPSYS_CMDQ_BASE + 0x68) //SL test for

#endif

