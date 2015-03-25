/******************************************************************************
 * camera_isp.c - MT6572 Linux ISP Device Driver
 *
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 *
 * DESCRIPTION:
 *     This file provid the other drivers ISP relative functions
 *
 ******************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <asm/io.h>
//#include <asm/tcm.h>
#include <linux/proc_fs.h>  	//proc file use
//
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/xlog.h> 		// For xlog_printk().
//
#include <mach/hardware.h>
#include <mach/camera_isp.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>		// For clock mgr APIS. enable_clock()/disable_clock()
#include <mach/mt_smi.h>		// For SMI clock mgr APIS
#include <mach/mt_spm_api.h>	// For screen on deep idle
#include <mach/mt_gpio.h>		// For gpio control
//#include <mach/sync_write.h>    // For mt65xx_reg_sync_writel().

typedef unsigned char           MUINT8;
typedef unsigned int            MUINT32;
typedef signed char             MINT8;
typedef signed int              MINT32;
typedef bool                    MBOOL;
#ifndef MTRUE
    #define MTRUE               1
#endif
#ifndef MFALSE
    #define MFALSE              0
#endif
//----------------------------------------------------------------------------
//#define LOG_MSG(fmt, arg...)    printk(KERN_ERR "[ISP][%s]" fmt,__FUNCTION__, ##arg)
//#define LOG_DBG(fmt, arg...)    printk(KERN_ERR  "[ISP][%s]" fmt,__FUNCTION__, ##arg)
//#define LOG_WRN(fmt, arg...)    printk(KERN_ERR "[ISP][%s]Warning" fmt,__FUNCTION__, ##arg)
//#define LOG_ERR(fmt, arg...)    printk(KERN_ERR   "[ISP][%s]Err(%5d):" fmt, __FUNCTION__,__LINE__, ##arg)
#define K_ISP_TAG "K_ISP"
//#define LOG_VRB(format, args...)    xlog_printk(ANDROID_LOG_VERBOSE, K_ISP_TAG, "[%s] " format, __FUNCTION__, ##args)
//#define LOG_DBG(format, args...)    xlog_printk(ANDROID_LOG_DEBUG  , K_ISP_TAG, "[%s] " format, __FUNCTION__, ##args)
//#define LOG_INF(format, args...)    xlog_printk(ANDROID_LOG_INFO   , K_ISP_TAG, "[%s] " format, __FUNCTION__, ##args)
//#define LOG_WRN(format, args...)    xlog_printk(ANDROID_LOG_WARN   , K_ISP_TAG, "[%s] WARNING: " format, __FUNCTION__, ##args)
//#define LOG_ERR(format, args...)    xlog_printk(ANDROID_LOG_ERROR  , K_ISP_TAG, "[%s, line%04d] ERROR: " format, __FUNCTION__, __LINE__, ##args)
//#define LOG_AST(format, args...)    xlog_printk(ANDROID_LOG_ASSERT , K_ISP_TAG, "[%s, line%04d] ASSERT: " format, __FUNCTION__, __LINE__, ##args)

#define LOG_VRB(format, args...)    //xlog_printk(ANDROID_LOG_VERBOSE, K_ISP_TAG, "[%s] " format, __FUNCTION__, ##args)
#define LOG_DBG(format, args...)    //xlog_printk(ANDROID_LOG_DEBUG  , K_ISP_TAG, "[%s] " format, __FUNCTION__, ##args)
#define LOG_INF(format, args...)    printk(KERN_DEBUG K_ISP_TAG format, ##args)
#define LOG_WRN(format, args...)    //xlog_printk(ANDROID_LOG_WARN   , K_ISP_TAG, "[%s] WARNING: " format, __FUNCTION__, ##args)
#define LOG_ERR(format, args...)    xlog_printk(ANDROID_LOG_ERROR  , K_ISP_TAG, "[%s, line%04d] ERROR: " format, __FUNCTION__, __LINE__, ##args)
#define LOG_AST(format, args...)    //xlog_printk(ANDROID_LOG_ASSERT , K_ISP_TAG, "[%s, line%04d] ASSERT: " format, __FUNCTION__, __LINE__, ##args)

#define __ISP_PROC_TEST_ENABLE__
/*******************************************************************************
*
********************************************************************************/
//#define ISP_WR32(addr, data)    mt65xx_reg_sync_writel(data, addr)    // For 89 Only.   // NEED_TUNING_BY_PROJECT
#define ISP_WR32(addr, data)    iowrite32(data, addr) // For other projects.
#define ISP_RD32(addr)          ioread32(addr)
#define ISP_SET_BIT(reg, bit)   ((*(volatile MUINT32*)(reg)) |= (MUINT32)(1 << (bit)))
#define ISP_CLR_BIT(reg, bit)   ((*(volatile MUINT32*)(reg)) &= ~((MUINT32)(1 << (bit))))
/*******************************************************************************
*
********************************************************************************/
#define ISP_DEV_NAME                "camera-isp"
/*******************************************************************************
*
********************************************************************************/
#define ISP_DBG_INT                 (0x00000001)
#define ISP_DBG_HOLD_REG            (0x00000002)
#define ISP_DBG_READ_REG            (0x00000004)
#define ISP_DBG_WRITE_REG           (0x00000008)
#define ISP_DBG_CLK                 (0x00000010)
#define ISP_DBG_TASKLET             (0x00000020)
#define ISP_DBG_SCHEDULE_WORK       (0x00000040)
#define ISP_DBG_BUF_WRITE           (0x00000080)
#define ISP_DBG_RT_BUF_CTRL         (0x00000100)
#define ISP_DBG_REF_CNT_CTRL        (0x00000200)
/*******************************************************************************
*
********************************************************************************/
//#define ISP_ADDR                        (CAMINF_BASE + 0x4000)
//#define ISP_ADDR_CAMINF                 CAMINF_BASE
//#define ISP_ADDR_CAMINF                 (CAM_BASE)
#define ISP_BASE_ADDR                  	(CAM_BASE)
#define SENINF_BASE_ADDR				(SENINF_BASE)
#define ISP_REG_MODULE_EN               (ISP_BASE_ADDR + 0x0)
#define ISP_REG_INT_STATUS         		(ISP_BASE_ADDR + 0x14)
#define ISP_REG_SW_CTL             		(ISP_BASE_ADDR + 0x20)
#define ISP_REG_IMGO_FBC           		(ISP_BASE_ADDR + 0x40)
#define ISP_REG_IMGO_BASE_ADDR     		(ISP_BASE_ADDR + 0x204)
#define ISP_REG_TG_VF_CON          		(ISP_BASE_ADDR + 0x414)

//#define ISP_TPIPE_ADDR                  (0x15004000)

//CAM_SW_CTL
#define ISP_REG_SW_CTL_IMGO_RST_TRIG   	(0x00000001)
#define ISP_REG_SW_CTL_IMGO_RST_ST    	(0x00000002)
#define ISP_REG_SW_CTL_SW_RST           (0x00000004)

//CAM_CTL_INT_STATUS
#define ISP_REG_MASK_INT_STATUS         (ISP_INT_VS1 |\
                                        ISP_INT_TG1_INT1 |\
                                        ISP_INT_TG1_INT2 |\
                                        ISP_INT_EXPDON1 |\
                                        ISP_INT_TG1_SOF |\
                                        ISP_INT_PASS1_TG1_DON)
                                        
#define ISP_REG_MASK_INT_STATUS_ERR 	(ISP_INT_TG1_ERR |\
                                        ISP_INT_TG1_DROP |\
										ISP_INT_TG1_GBERR |\
										ISP_INT_IMGO_ERR |\
                                        ISP_INT_IMGO_OVERR |\
                                        ISP_INT_IMGO_DROP)                                        
// for ISP_Irq
#define __tcmfunc
/*******************************************************************************
*
********************************************************************************/
// internal data
static int* pTbl_RTBuf = NULL;		// pointer to the kmalloc'd area, rounded up to a page boundary
static void* pBuf_kmalloc = NULL;	// original pointer for kmalloc'd area as returned by kmalloc/
static ISP_RT_BUF_INFO_STRUCT* pstRTBuf = NULL;
//static ISP_DEQUE_BUF_INFO_STRUCT g_deque_buf = {0,{}};    // Marked to remove build warning.

unsigned long g_Flash_SpinLock;

unsigned int G_u4EnableClockCount = 0;
/*******************************************************************************
*
********************************************************************************/
#define ISP_BUF_SIZE            (4096)
#define ISP_BUF_SIZE_WRITE      (1024)
#define ISP_BUF_WRITE_AMOUNT    (6)

//isp driver
#define MAP_ISP_RTBUF_REG_RANGE      0x10000
#define MAP_ISP_BASE_HW              0x14013000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_ISP_BASE_RANGE           0x1000 

//seninf driver
#define MAP_SENINF_BASE_ADDR         0x14014000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_SENINF_REG_RANGE         (0x1000)      //0x100,the same with the value in seninf_reg.h and page-aligned
#define MAP_CAM_MMSYS_CONFIG_BASE    0x14000000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_CAM_MMSYS_CONFIG_RANGE   (0x1000)      //0x100,the same with the value in seninf_drv.cpp and page-aligned
#define MAP_CAM_MIPI_CONFIG_BASE     0x10011000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_CAM_MIPI_CONFIG_RANGE    (0x1000)      //0x100,the same with the value in seninf_drv.cpp and page-aligned
#define MAP_CAM_MIPI_RX_CONFIG_BASE  0x14015000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_CAM_MIPI_RX_CONFIG_RANGE (0x1000)      //0x100,the same with the value in seninf_drv.cpp and page-aligned
#define MAP_CAM_GPIO_CFG_BASE         0x1020B000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_CAM_GPIO_CFG_RANGE       (0x1000)      //0x100,the same with the value in seninf_reg.h and page-aligned

#define MAP_CAM_EFUSE_BASE           0x10009000   //the same with the value in seninf_drv.cpp(chip-dependent)
#define MAP_CAM_EFUSE_RANGE         (0x1000)      //0x100,the same with the value in seninf_reg.h and page-aligned



typedef enum
{
    ISP_BUF_STATUS_EMPTY,
    ISP_BUF_STATUS_HOLD,
    ISP_BUF_STATUS_READY
}ISP_BUF_STATUS_ENUM;

typedef struct
{
    volatile ISP_BUF_STATUS_ENUM    Status;
    volatile MUINT32                Size;
    MUINT8*                         pData;
}ISP_BUF_STRUCT;

typedef struct
{
    ISP_BUF_STRUCT      Read;
    ISP_BUF_STRUCT      Write[ISP_BUF_WRITE_AMOUNT];
}ISP_BUF_INFO_STRUCT;
/*******************************************************************************
*
********************************************************************************/
typedef struct
{
    MUINT32     Status;
    MUINT32     Mask;
    MUINT32     ErrMask;
}ISP_IRQ_INFO_STRUCT;

typedef struct
{
    atomic_t            HoldEnable;
    atomic_t            WriteEnable;
    ISP_HOLD_TIME_ENUM  Time;
}ISP_HOLD_INFO_STRUCT;

typedef struct
{
    MUINT32     Vd;
    MUINT32     Expdone;
    MUINT32     WorkQueueVd;
    MUINT32     WorkQueueExpdone;
    MUINT32     TaskletVd;
    MUINT32     TaskletExpdone;
}ISP_TIME_LOG_STRUCT;

typedef struct
{
    spinlock_t              SpinLockIspRef;
    spinlock_t              SpinLockIsp;
    spinlock_t              SpinLockIrq;
    spinlock_t              SpinLockHold;
    spinlock_t              SpinLockRTBC;
    wait_queue_head_t       WaitQueueHead;
    struct work_struct      ScheduleWorkVD;
    struct work_struct      ScheduleWorkEXPDONE;
    MUINT32                 UserCount;
    MUINT32                 DebugMask;
    MINT32                  IrqNum;
    ISP_IRQ_INFO_STRUCT     IrqInfo;
    ISP_HOLD_INFO_STRUCT    HoldInfo;
    ISP_BUF_INFO_STRUCT     BufInfo;
    ISP_TIME_LOG_STRUCT     TimeLog;
    ISP_CALLBACK_STRUCT     Callback[ISP_CALLBACK_AMOUNT];
}ISP_INFO_STRUCT;

static ISP_INFO_STRUCT IspInfo;
/*******************************************************************************
*
********************************************************************************/
typedef struct
{
    pid_t   Pid;
    pid_t   Tid;
}ISP_USER_INFO_STRUCT;
/*******************************************************************************
*
********************************************************************************/
static __inline MUINT32 ISP_MsToJiffies(MUINT32 Ms)
{
    return ((Ms * HZ + 512) >> 10);
}

static __inline MUINT32 ISP_JiffiesToMs(MUINT32 Jiffies)
{
    return ((Jiffies*1000)/HZ);
}

static __inline MUINT32 ISP_GetIRQState(MUINT32 stus)
{
    MUINT32 ret;
    unsigned long flags;
    //
    spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
    ret = (IspInfo.IrqInfo.Status & stus);
    spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
    //
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_DumpReg(void)
{
    MINT32 Ret = 0;
    MINT32 i;

    LOG_INF(" +");	    
    //spin_lock_irq(&(IspInfo.SpinLock));
    
    //CAM
    for(i = 0x0; i <= 0x84; i += 4)
    {
        LOG_INF("0x%08X %08X ", ISP_BASE_ADDR + i, ISP_RD32(ISP_BASE_ADDR + i));
    }	
    //DMA
    for( i = 0x200; i <= 0x22C; i += 4)
    {
        LOG_INF("0x%08X %08X ", ISP_BASE_ADDR + i, ISP_RD32(ISP_BASE_ADDR + i));
    }
    //TG1
    for( i = 0x410; i <= 0x44C; i += 4)
    {
        LOG_INF("0x%08X %08X ", ISP_BASE_ADDR + i, ISP_RD32(ISP_BASE_ADDR + i));
    }
    //CDRZ
    for( i = 0xB00; i <= 0xB38; i += 4)
    {
        LOG_INF("0x%08X %08X ", ISP_BASE_ADDR + i, ISP_RD32(ISP_BASE_ADDR + i));
    }		
	
	//SENINF1_TOP
	LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + 0x0, ISP_RD32(ISP_BASE_ADDR + 0x0));
    //SENINF1
    for( i = 0x10; i <= 0x40; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }
   	//SENINF1_CSI2
    for( i = 0x100; i <= 0x13C; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }
	//SCAM1
    for( i = 0x200; i <= 0x240; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }
    //SENINF_TG1
    for( i = 0x300; i <= 0x310; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }
    //SENINF1_CCIR656
    for( i = 0x400; i <= 0x424; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }
    //SENINF1_NCSI2
    for( i = 0x600; i <= 0x644; i += 4)
    {
        LOG_INF("0x%08X %08X ", SENINF_BASE_ADDR + i, ISP_RD32(SENINF_BASE_ADDR + i));
    }

    //spin_unlock_irq(&(IspInfo.SpinLock));
    LOG_INF(" -");
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_EnableClock(MBOOL En)
{
	LOG_DBG(" +");
	
    LOG_DBG("En: %d. G_u4EnableClockCount: %d.", En, G_u4EnableClockCount);
    if (En) // Enable clock.
    {
		//*** for low power concern ***//
		// pull en CMPCLK/CMMCLK/CMDAT0/CMDAT1/CMDAT2/CMDAT3 ; 0x1020B040 = 0x0FC0 (mask 0x0FC0)  
		// 1020B040//1020B044//1020B048 ; PULLEN_CFG0//SET//CLR 	
		(*(volatile MUINT32*)(IO_CFG_RIGHT_BASE+ 0x048)) = 0x0FC0; //CLR bit 6~11 to not pull en
		
		/*
	    // IES_CFG_SET, CAM(bit 2,3)=1 ; (mask 0xC) ; CAMPCLK/CMMCLK/CMDAT0/CMDAT1/CMDAT2/CMDAT3
		(*(volatile MUINT32*)(IO_CFG_RIGHT_BASE+ 0x004)) =0xC; //|= 0x0000003C; 
		// set bit 0,6,12,18,24, for MIPI IES = 1
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x84C)) |= 0x01041041; 
		// set bit 0, for MIPI IES = 1
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x850)) |= 0x00000001; 	
		*/
		//*** for low power concern ***//
		
        enable_clock(MT_CG_MM_CAM_SW_CG, "CAMERA");
        enable_clock(MT_CG_MM_CAMTG_SW_CG,  "CAMERA");
        enable_clock(MT_CG_MM_SENINF_SW_CG, "CAMERA");
		larb_clock_on(0, "CAMERA");		// for SMI related CG
		//enable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");
        //enable_clock(MT_CG_IMAGE_LARB3_SMI, "CAMERA");
        //enable_clock(MT_CG_IMAGE_LARB4_SMI, "CAMERA");
        //enable_clock(MT_CG_IMAGE_COMMN_SMI, "CAMERA");
        G_u4EnableClockCount++;
        LOG_DBG("Camera clock enbled. G_u4EnableClockCount: %d.", G_u4EnableClockCount);
    }
    else    // Disable clock.
    {        
        disable_clock(MT_CG_MM_CAM_SW_CG, "CAMERA");
        disable_clock(MT_CG_MM_CAMTG_SW_CG,  "CAMERA");
        disable_clock(MT_CG_MM_SENINF_SW_CG, "CAMERA");
		larb_clock_off(0, "CAMERA");	// for SMI related CG
		//disable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");		
        //disable_clock(MT_CG_IMAGE_LARB3_SMI, "CAMERA");
        //disable_clock(MT_CG_IMAGE_LARB4_SMI, "CAMERA");
        //disable_clock(MT_CG_IMAGE_COMMN_SMI, "CAMERA");

		//*** for low power concern ***//
		// pull en CMPCLK/CMMCLK/CMDAT0/CMDAT1/CMDAT2/CMDAT3 ; 0x1020B040 = 0x0FC0 (mask 0x0FC0)  
		// 1020B040//1020B044//1020B048 ; PULLEN_CFG0//SET//CLR		
		(*(volatile MUINT32*)(IO_CFG_RIGHT_BASE+ 0x044)) = 0x0FC0; //SET bit 6~11 to pull en
		
		// pull down CMPCLK/CMMCLK/CMDAT0/CMDAT1/CMDAT2/CMDAT3 ; 0x1020B050 = 0x0000 (mask 0x0FC0)  
		// 1020B050//1020B054//1020B058 ; PULLSEL_CFG0//SET//CLR
		(*(volatile MUINT32*)(IO_CFG_RIGHT_BASE+ 0x058)) = 0x0FC0; //CLR bit 6~11 to pull down

		// pull down RDP0_A(CMDAT4)/RDN0_A(CMDAT5)/RDP1_A(CMDAT6)/RDN1_A(CMDAT7)/RCP_A(CMHSYNC)
		// 0x1001184C = 0x8208208 (mask 0x8208208)
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x84C)) |= 0x08208208; 	
		
		// pul down RCN_A(CMVSYNC)
		// 0x10011850 = 0x08 (mask 0x08)
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x850)) |= 0x00000008; 

		/*
	    // IES_CFG_CLR, CAM(bit 2,3)=0 ; (mask 0xC) ; CAMPCLK/CMMCLK/CMDAT0/CMDAT1/CMDAT2/CMDAT3
		(*(volatile MUINT32*)(IO_CFG_RIGHT_BASE+ 0x008)) = 0xC; // clear bit 2-5, for CAM IES = 0		
		// 0x1001184C (mask 0x1041041) IES=0, RDP0_A(CMDAT4)/RDN0_A(CMDAT5)/RDP1_A(CMDAT6)/RDN1_A(CMDAT7)/RCP_A(CMHSYNC)
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x84C)) &= 0xFEFBEFBE; 				
		// 0x10011850 (mask 0x01) IES=0, RCN_A(CMVSYNC)
		(*(volatile MUINT32*)(MIPI_CONFIG_BASE + 0x850)) &= 0xFFFFFFFE; 
		*/
		//*** for low power concern ***//		
	
        G_u4EnableClockCount--;
        LOG_DBG("Camera clock disabled. G_u4EnableClockCount: %d.", G_u4EnableClockCount);
    }

    LOG_DBG(" -");
	return;
}
/*******************************************************************************
*
********************************************************************************/
static inline void ISP_Reset(void)
{    
    MUINT32 Reg;
    unsigned long  flags;

    LOG_DBG(" +");
    //spin_lock_irq(&(IspInfo.SpinLockHold));

	// ensure the view finder is disabe. 0: take_picture
    ISP_CLR_BIT(ISP_REG_MODULE_EN, 0);
    ISP_CLR_BIT(ISP_REG_TG_VF_CON, 0);
	
    //TODO: MUST remove later
    //imgsys clk on
    //ISP_WR32(ISP_BASE_ADDR_CAMINF, 0);
	//ISP_EnableClock(MTRUE);
    //LOG_DBG("isp gate clk(0x%x)",ISP_RD32(ISP_BASE_ADDR_CAMINF));

	// Jason TODO Reg
    //bandwidth limitor for TG
    Reg = ISP_RD32((EMI_BASE+0x120));
    Reg |= 0x3F;
    ISP_WR32((EMI_BASE+0x120), Reg);	
	// Jason TODO Reg

    // do SW_RST
    ISP_WR32(ISP_REG_SW_CTL, ISP_REG_SW_CTL_IMGO_RST_TRIG);
	do {
	    Reg = ISP_RD32(ISP_REG_SW_CTL);
	} while ((!Reg) & ISP_REG_SW_CTL_IMGO_RST_ST);
    ISP_WR32(ISP_REG_SW_CTL, ISP_REG_SW_CTL_IMGO_RST_TRIG|ISP_REG_SW_CTL_SW_RST); //0x5
    ISP_WR32(ISP_REG_SW_CTL, ISP_REG_SW_CTL_SW_RST); //0x4
    ISP_WR32(ISP_REG_SW_CTL, 0);
	LOG_DBG("after sw reset, %08X", ISP_RD32(ISP_REG_SW_CTL));

	// clear irq status 
    spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
	IspInfo.IrqInfo.Status = 0;
    spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
    
    //spin_unlock_irq(&(IspInfo.SpinLockHold));
    LOG_DBG(" -");
	return;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_WriteRegToHw(
    ISP_REG_STRUCT* pReg,
    MUINT32         Count)
{
    MINT32 Ret = 0;
    MUINT32 i;

   	//
    spin_lock(&(IspInfo.SpinLockIsp));
    for(i = 0; i < Count; i++)
    {
        if(IspInfo.DebugMask & ISP_DBG_WRITE_REG)
        {
            LOG_DBG("Addr(0x%08X), Val(0x%08X)", (MUINT32)(ISP_BASE_ADDR + pReg[i].Addr), (MUINT32)(pReg[i].Val));
        }
        ISP_WR32(ISP_BASE_ADDR + pReg[i].Addr, pReg[i].Val);
    }
    spin_unlock(&(IspInfo.SpinLockIsp));
    //

    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_BufWrite_Dump(void)
{
    MUINT32 i;
	
    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        LOG_DBG("i=%d, Status=%d, Size=%d",i,IspInfo.BufInfo.Write[i].Status,IspInfo.BufInfo.Write[i].Size);
        IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_EMPTY;
        IspInfo.BufInfo.Write[i].Size = 0;
        IspInfo.BufInfo.Write[i].pData = NULL;
    }

	return;
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_BufWrite_Free(void)
{
    MUINT32 i;
        
    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_EMPTY;
        IspInfo.BufInfo.Write[i].Size = 0;
        if(IspInfo.BufInfo.Write[i].pData != NULL)
        {
            kfree(IspInfo.BufInfo.Write[i].pData);
            IspInfo.BufInfo.Write[i].pData = NULL;
        }
    }

	return;
}
/*******************************************************************************
*
********************************************************************************/
static MBOOL ISP_BufWrite_Alloc(void)
{
    MUINT32 i;

    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_EMPTY;
        IspInfo.BufInfo.Write[i].Size = 0;
        IspInfo.BufInfo.Write[i].pData = (MUINT8*)kmalloc(ISP_BUF_SIZE_WRITE, GFP_ATOMIC);
        if(IspInfo.BufInfo.Write[i].pData == NULL)
        {
            LOG_DBG("ERROR: i = %d, pData is NULL",i);
            ISP_BufWrite_Free();
            return false;
        }
    }

    return true;
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_BufWrite_Reset(void)
{
    MUINT32 i;

    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_EMPTY;
        IspInfo.BufInfo.Write[i].Size = 0;
    }

	return;
}
/*******************************************************************************
*
********************************************************************************/
static __inline MUINT32 ISP_BufWrite_GetAmount(void)
{
    MUINT32 i, Count = 0;
	
    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        if(IspInfo.BufInfo.Write[i].Status == ISP_BUF_STATUS_READY)
        {
            Count++;
        }
    }
    if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
    {
        LOG_DBG("Count = %d",Count);
    }

    return Count;
}
/*******************************************************************************
*
********************************************************************************/
static MBOOL ISP_BufWrite_Add(
    MUINT32     Size,
    MUINT8*     pData)
{
    MUINT32 i;

    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        if(IspInfo.BufInfo.Write[i].Status == ISP_BUF_STATUS_HOLD)
        {
            if((IspInfo.BufInfo.Write[i].Size+Size) > ISP_BUF_SIZE_WRITE)
            {
                LOG_ERR("i = %d, BufWriteSize(%d)+Size(%d) > %d",i,IspInfo.BufInfo.Write[i].Size,Size,ISP_BUF_SIZE_WRITE);
                return false;
            }
            //
            if(copy_from_user((MUINT8*)(IspInfo.BufInfo.Write[i].pData+IspInfo.BufInfo.Write[i].Size), (MUINT8*)pData, Size) != 0)
            {
                LOG_ERR("copy_from_user failed");
                return false;
            }
            //
            if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
            {
                LOG_DBG("i = %d, BufSize = %d, Size = %d",i,IspInfo.BufInfo.Write[i].Size,Size);
            }
            //
            IspInfo.BufInfo.Write[i].Size += Size;
            return true;
        }
    }
    //
    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        if(IspInfo.BufInfo.Write[i].Status == ISP_BUF_STATUS_EMPTY)
        {
            if(Size > ISP_BUF_SIZE_WRITE)
            {
                LOG_ERR("i = %d, Size(%d) > %d",i,Size,ISP_BUF_SIZE_WRITE);
                return false;
            }
            //
            if(copy_from_user((MUINT8*)(IspInfo.BufInfo.Write[i].pData), (MUINT8*)pData, Size) != 0)
            {
                LOG_ERR("copy_from_user failed");
                return false;
            }
            //
            if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
            {
                LOG_DBG("i = %d, Size = %d",i,Size);
            }
            //
            IspInfo.BufInfo.Write[i].Size = Size;
            //
            IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_HOLD;
            return true;
        }
    }

    //
    LOG_ERR("All write buffer are full of data!");
    return false;
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_BufWrite_SetReady(void)
{
    MUINT32 i;

    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        if(IspInfo.BufInfo.Write[i].Status == ISP_BUF_STATUS_HOLD)
        {
            if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
            {
                LOG_DBG("i = %d, Size = %d",i,IspInfo.BufInfo.Write[i].Size);
            }
            IspInfo.BufInfo.Write[i].Status = ISP_BUF_STATUS_READY;
        }
    }
}
/*******************************************************************************
*
********************************************************************************/
static MBOOL ISP_BufWrite_Get(
    MUINT32*    pIndex,
    MUINT32*    pSize,
    MUINT8**    ppData)
{
    MUINT32 i;

    for(i=0; i<ISP_BUF_WRITE_AMOUNT; i++)
    {
        if(IspInfo.BufInfo.Write[i].Status == ISP_BUF_STATUS_READY)
        {
            if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
            {
                LOG_DBG("i = %d, Size = %d",i,IspInfo.BufInfo.Write[i].Size);
            }
            *pIndex = i;
            *pSize = IspInfo.BufInfo.Write[i].Size;
            *ppData = IspInfo.BufInfo.Write[i].pData;
            return true;
        }
    }
    //
    if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
    {
        LOG_DBG("No buf is ready!");
    }
    return false;
}
/*******************************************************************************
*
********************************************************************************/
static MBOOL ISP_BufWrite_Clear(MUINT32  Index)
{
    if(IspInfo.BufInfo.Write[Index].Status == ISP_BUF_STATUS_READY)
    {
        if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
        {
            LOG_DBG("Index = %d, Size = %d",Index,IspInfo.BufInfo.Write[Index].Size);
        }
        IspInfo.BufInfo.Write[Index].Size = 0;
        IspInfo.BufInfo.Write[Index].Status = ISP_BUF_STATUS_EMPTY;
        return true;
    }
    else
    {
        LOG_DBG("WARNING: Index(%d) is not ready! Status = %d",Index,IspInfo.BufInfo.Write[Index].Status);
        return false;
    }
}
/*******************************************************************************
*
********************************************************************************/
static void ISP_BufWrite_WriteToHw(void)
{
    MUINT8* pBuf;
    MUINT32 Index, BufSize;
    //
    spin_lock(&(IspInfo.SpinLockHold));
    //
    while(ISP_BufWrite_Get(&Index,&BufSize,&pBuf))
    {
        if(IspInfo.DebugMask & ISP_DBG_BUF_WRITE)
        {
            LOG_DBG("Index = %d, BufSize = %d ", Index, BufSize);
        }
        ISP_WriteRegToHw((ISP_REG_STRUCT*)pBuf, BufSize/sizeof(ISP_REG_STRUCT));
        ISP_BufWrite_Clear(Index);
    }
    //LOG_DBG("No more buf.");
    atomic_set(&(IspInfo.HoldInfo.WriteEnable), 0);
    wake_up_interruptible(&(IspInfo.WaitQueueHead));
    //
    spin_unlock(&(IspInfo.SpinLockHold));
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_WriteReg(ISP_REG_IO_STRUCT*   pRegIo)
{
    MINT32 Ret = 0;
    MINT32 TimeVd = 0;
    MINT32 TimeExpdone = 0;
    MINT32 TimeTasklet = 0;

    //
    if(IspInfo.DebugMask & ISP_DBG_WRITE_REG)
    {
        LOG_DBG("Data(0x%08X), Count(%d)", (MUINT32)(pRegIo->Data), (MUINT32)(pRegIo->Count));
    }
    //
    if(atomic_read(&(IspInfo.HoldInfo.HoldEnable)))
    {
        if(ISP_BufWrite_Add((pRegIo->Count)*sizeof(ISP_REG_STRUCT), (MUINT8*)(pRegIo->Data)))
        {
            //LOG_DBG("Add write buffer OK");
        }
        else
        {
            LOG_ERR("Add write buffer fail");
            TimeVd = ISP_JiffiesToMs(jiffies)-IspInfo.TimeLog.Vd;
            TimeExpdone = ISP_JiffiesToMs(jiffies)-IspInfo.TimeLog.Expdone;
            TimeTasklet = ISP_JiffiesToMs(jiffies)-IspInfo.TimeLog.TaskletExpdone;
            LOG_ERR("HoldTime(%d), VD(%d ms), Expdone(%d ms), Tasklet(%d ms)",IspInfo.HoldInfo.Time,TimeVd,TimeExpdone,TimeTasklet);
            ISP_BufWrite_Dump();
            ISP_DumpReg();
            Ret = -EFAULT;
            goto EXIT;
        }
    }
    else
    {
        Ret = ISP_WriteRegToHw(
                (ISP_REG_STRUCT*)(pRegIo->Data),
                pRegIo->Count);
    }
    //
    EXIT:
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_ReadReg(ISP_REG_IO_STRUCT* pRegIo)
{
    MUINT32 i;
    MINT32 Ret = 0, Size = (pRegIo->Count)*sizeof(ISP_REG_STRUCT);
    ISP_REG_STRUCT* pReg = (ISP_REG_STRUCT*)(IspInfo.BufInfo.Read.pData);
    //
    if(IspInfo.DebugMask & ISP_DBG_READ_REG)
    {
        LOG_DBG("Data(0x%08X), Count(%d)", (MUINT32)(pRegIo->Data), (MUINT32)(pRegIo->Count));
    }
    //
    if(Size > ISP_BUF_SIZE)
    {
        LOG_ERR("Size too big");
    }
    //
    if(copy_from_user((MUINT8*)pReg, (MUINT8*)(pRegIo->Data), Size) != 0)
    {
        LOG_ERR("copy_from_user failed");
        Ret = -EFAULT;
        goto EXIT;
    }
    //
    for(i = 0; i < pRegIo->Count; i++)
    {
        pReg[i].Val = ISP_RD32(ISP_BASE_ADDR + pReg[i].Addr);
        if(IspInfo.DebugMask & ISP_DBG_READ_REG)
        {
            LOG_DBG("Addr(0x%08X), Val(0x%08X)", (MUINT32)(ISP_BASE_ADDR + pReg[i].Addr), (MUINT32)(pReg[i].Val));
        }
    }
    //
    if(copy_to_user((MUINT8*)(pRegIo->Data), (MUINT8*)pReg, Size) != 0)
    {
        LOG_ERR("copy_to_user failed");
        Ret = -EFAULT;
        goto EXIT;
    }
    //
    EXIT:
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
void ISP_ScheduleWork_VD(struct work_struct *data)
{
    if(IspInfo.DebugMask & ISP_DBG_SCHEDULE_WORK)
    {
        LOG_DBG(" +");
    }
	
    //
    IspInfo.TimeLog.WorkQueueVd = ISP_JiffiesToMs(jiffies);
    //
    if(IspInfo.Callback[ISP_CALLBACK_WORKQUEUE_VD].Func != NULL)
    {
        IspInfo.Callback[ISP_CALLBACK_WORKQUEUE_VD].Func();
    }

	if(IspInfo.DebugMask & ISP_DBG_SCHEDULE_WORK)
    {
        LOG_DBG(" -");
    }
	return;
}
/*******************************************************************************
*
********************************************************************************/
void ISP_ScheduleWork_EXPDONE(struct work_struct *data)
{
    if(IspInfo.DebugMask & ISP_DBG_SCHEDULE_WORK)
    {
        LOG_DBG(" +");
    }
	
    //
    IspInfo.TimeLog.WorkQueueExpdone = ISP_JiffiesToMs(jiffies);
    //
    if(IspInfo.Callback[ISP_CALLBACK_WORKQUEUE_EXPDONE].Func != NULL)
    {
        IspInfo.Callback[ISP_CALLBACK_WORKQUEUE_EXPDONE].Func();
    }

	if(IspInfo.DebugMask & ISP_DBG_SCHEDULE_WORK)
    {
        LOG_DBG(" -");
    }
	return;
}
/*******************************************************************************
*
********************************************************************************/
void ISP_Tasklet_VD(unsigned long Param)
{
    if(IspInfo.DebugMask & ISP_DBG_TASKLET)
    {
        LOG_DBG(" +");
    }
	
    //
    IspInfo.TimeLog.TaskletVd = ISP_JiffiesToMs(jiffies);
    //
    if(IspInfo.Callback[ISP_CALLBACK_TASKLET_VD].Func != NULL)
    {
        IspInfo.Callback[ISP_CALLBACK_TASKLET_VD].Func();
    }
    //
    if(IspInfo.HoldInfo.Time == ISP_HOLD_TIME_VD)
    {
        ISP_BufWrite_WriteToHw();
    }

	if(IspInfo.DebugMask & ISP_DBG_TASKLET)
    {
        LOG_DBG(" -");
    }
	return;
}
DECLARE_TASKLET(IspTaskletVD, ISP_Tasklet_VD, 0);
/*******************************************************************************
*
********************************************************************************/
void ISP_Tasklet_EXPDONE(unsigned long Param)
{
    if(IspInfo.DebugMask & ISP_DBG_TASKLET)
    {
        LOG_DBG(" +");
    }
	
    //
    IspInfo.TimeLog.TaskletExpdone = ISP_JiffiesToMs(jiffies);
    //
    if(IspInfo.Callback[ISP_CALLBACK_TASKLET_EXPDONE].Func != NULL)
    {
        IspInfo.Callback[ISP_CALLBACK_TASKLET_EXPDONE].Func();
    }
    //
    if(IspInfo.HoldInfo.Time == ISP_HOLD_TIME_EXPDONE)
    {
        ISP_BufWrite_WriteToHw();
    }

	if(IspInfo.DebugMask & ISP_DBG_TASKLET)
    {
        LOG_DBG(" -");
    }
	return;
}
DECLARE_TASKLET(IspTaskletEXPDONE, ISP_Tasklet_EXPDONE, 0);
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_SetHoldTime(ISP_HOLD_TIME_ENUM HoldTime)
{
    LOG_DBG("HoldTime(%d)", HoldTime);
    IspInfo.HoldInfo.Time = HoldTime;
    //
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_ResetBuf(void)
{
	LOG_DBG(" +");
	
    LOG_DBG("hold_reg(%d), BufAmount(%d)", atomic_read(&(IspInfo.HoldInfo.HoldEnable)), ISP_BufWrite_GetAmount());
    //
    ISP_BufWrite_Reset();
    atomic_set(&(IspInfo.HoldInfo.HoldEnable), 0);
    atomic_set(&(IspInfo.HoldInfo.WriteEnable), 0);
	
    LOG_DBG(" -");
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_EnableHoldReg(MBOOL En)
{
    MINT32 Ret = 0;
    MUINT32 BufAmount = 0;
    //
    if(IspInfo.DebugMask & ISP_DBG_HOLD_REG)
    {
        LOG_DBG("En(%d), HoldEnable(%d)", En, atomic_read(&(IspInfo.HoldInfo.HoldEnable)));
    }
    //
    if(!spin_trylock_bh(&(IspInfo.SpinLockHold)))
    {
        //  Should wait until tasklet done.
        MINT32 Timeout;
        MINT32 IsLock = 0;
        //
        if(IspInfo.DebugMask & ISP_DBG_TASKLET)
        {
            LOG_DBG("Start wait ... ");
        }
        //
        Timeout = wait_event_interruptible_timeout(
                    IspInfo.WaitQueueHead,
                    (IsLock = spin_trylock_bh(&(IspInfo.SpinLockHold))),
                    ISP_MsToJiffies(500));
        //
        if(IspInfo.DebugMask & ISP_DBG_TASKLET)
        {
            LOG_DBG("End wait ");
        }
        //
        if(IsLock == 0)
        {
            LOG_ERR("Should not happen, Timeout & IsLock is 0");
            Ret = -EFAULT;
            goto EXIT;
        }
    }
    //  Here we get the lock.
    if(En == MFALSE)
    {
        ISP_BufWrite_SetReady();
        BufAmount = ISP_BufWrite_GetAmount();
        //
        if(BufAmount)
        {
            atomic_set(&(IspInfo.HoldInfo.WriteEnable), 1);
        }
    }
    //
    if(IspInfo.DebugMask & ISP_DBG_HOLD_REG)
    {
        LOG_DBG("En(%d), HoldEnable(%d), BufAmount(%d)", En, atomic_read(&(IspInfo.HoldInfo.HoldEnable)),BufAmount);
    }
    //
    atomic_set(&(IspInfo.HoldInfo.HoldEnable), En);
    //
    spin_unlock_bh(&(IspInfo.SpinLockHold));
    //
    EXIT:
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static atomic_t g_ref_cnt[ISP_REF_CNT_ID_MAX];
//
static long ISP_REF_CNT_CTRL_FUNC(MUINT32 Param)
{
    MINT32 Ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt_ctrl;
    MINT32 t_ref_cnt = 0;
    
    if(IspInfo.DebugMask & ISP_DBG_REF_CNT_CTRL) 
	{
        LOG_DBG(" +");
    }
	
    //
    if (NULL == (void*)Param)  
	{
        LOG_ERR("NULL Param");
		LOG_DBG(" -");
        return -EFAULT;
    }
    //
    if(copy_from_user(&ref_cnt_ctrl, (void*)Param, sizeof(ISP_REF_CNT_CTRL_STRUCT)) == 0)
    {
        if(IspInfo.DebugMask & ISP_DBG_REF_CNT_CTRL) 
		{
            LOG_DBG("ctrl(%d),id(%d)",ref_cnt_ctrl.ctrl,ref_cnt_ctrl.id);
        }
        //
        if ( ISP_REF_CNT_ID_MAX > ref_cnt_ctrl.id ) 
		{
            //
            spin_lock(&(IspInfo.SpinLockIspRef));
            //            
            switch(ref_cnt_ctrl.ctrl) 
			{
                case ISP_REF_CNT_GET:
                    break;
                case ISP_REF_CNT_INC:
                    atomic_inc(&g_ref_cnt[ref_cnt_ctrl.id]);
                    //g_ref_cnt++;
                    break;
                case ISP_REF_CNT_DEC:
                case ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE:
                    atomic_dec(&g_ref_cnt[ref_cnt_ctrl.id]);
                    //g_ref_cnt--;
                    break;
                default:
                case ISP_REF_CNT_MAX:   // Add this to remove build warning.
                    // Do nothing.
                    break;
            }
            //
            t_ref_cnt = (MINT32)atomic_read(&g_ref_cnt[ref_cnt_ctrl.id]);
            LOG_DBG("g_ref_cnt[%d]: %d.", ref_cnt_ctrl.id, t_ref_cnt);

			// No user left and ctrl is RESET_IF_LAST_ONE, do ISP reset.
            if ( (t_ref_cnt == 0) && (ref_cnt_ctrl.ctrl == ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE) )   
            {
                ISP_Reset();
                LOG_DBG("ISP_REF_CNT_DEC_AND_RESET_IF_LAST_ONE. Do ISP_Reset.");
            }
            //
            spin_unlock(&(IspInfo.SpinLockIspRef));
            //
            if(IspInfo.DebugMask & ISP_DBG_REF_CNT_CTRL) 
			{
                LOG_DBG("ref_cnt(%d)",t_ref_cnt);
            }    
            //
            if(copy_to_user((void*)ref_cnt_ctrl.data_ptr, &t_ref_cnt, sizeof(MINT32)) != 0)
            {
                LOG_ERR("copy_to_user failed");
                Ret = -EFAULT;
            }
        }
        else 
		{
            LOG_ERR("id(%d) exceed",ref_cnt_ctrl.id);
            Ret = -EFAULT;
        }
    }
    else
    {
        LOG_ERR("copy_from_user failed");
        Ret = -EFAULT;
    }

    if(IspInfo.DebugMask & ISP_DBG_REF_CNT_CTRL) 
    {
        LOG_DBG(" -");
    }
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static MUINT32 frmCnt = 0;
static MUINT32 deqCnt = 0;
static MUINT32 enqCnt = 0;
static MINT32 dropCnt = 0;
static MINT32 irqSOFCnt = 0;
static MINT32 irqPASS1Cnt = 0;
static unsigned long long	gSofPreSec;
static unsigned long		gSofPreUsec;
static unsigned long long	gPass1PreSec;
static unsigned long		gPass1PreUsec;
static unsigned long		gDeqPreUsec;

#define IRQ_LOG_SIZE 200
typedef struct {
	unsigned int irq_type;
	unsigned int cnt;
	unsigned long long sec;
	unsigned long usec;
	unsigned int period;
} CAMERA_ISP_IRQ_LOG_STRUCT;
static unsigned gIrqLogCnt = 0;
static CAMERA_ISP_IRQ_LOG_STRUCT gCameraIspIrqLog[IRQ_LOG_SIZE];

//
static ISP_RT_BUF_STRUCT       	  rt_buf;
static ISP_RT_DEQUE_BUF_STRUCT    deque_buf;
static MUINT32 bEnqBuf = 0;
static MUINT32 bDeqBuf = 0;
static MINT32 rtbc_enq_dma = ISP_RT_BUF_DMAMAX;
static MINT32 rtbc_deq_dma = ISP_RT_BUF_DMAMAX;
// 
MUINT32 USE_NEW_RTBC = 1;
MUINT32 DEQ_SYNC_SOF = 0;
MUINT32 ENQ_SYNC_SOF = 0;
MUINT32 SOF_PASS1DONE_PAIRED = 0;

MINT32 DevId = 1;
#if 0
static void ISP_RTBC_PRINTBUF()
{
	MUINT32 i = 0;
	LOG_WRN("[rtbc][print]***total(%d), empty(%d), start(%d)", 
		pstRTBuf->ring_buf.total_count, 
		pstRTBuf->ring_buf.empty_count,
		pstRTBuf->ring_buf.start);
	for(i = 0 ; i < pstRTBuf->ring_buf.total_count ; i++)
	{
		LOG_WRN("[rtbc][print]***(%d) state<%d> pA:0x%x(0x%x)", 
			i, pstRTBuf->ring_buf.data[i].bFilled, 
			pstRTBuf->ring_buf.data[i].base_pAddr,
			pstRTBuf->ring_buf.data[i].size);				
	}
	LOG_WRN("[rtbc][print]***");
}
#endif
static MINT32 ISP_RTBC_ENQUE(MINT32 dma)
{
    MINT32 Ret = 0;
    MINT32 rt_dma = dma;
    MUINT32 buffer_exist = 0;
    MUINT32 reg_val = 0;
    MUINT32 i = 0;
    MUINT32 index = 0;    

	//
    //spin_lock_irqsave(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
    //   
    enqCnt++;
	if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
		LOG_DBG("[rtbc][E]frmCnt(%d)enqCnt(%d) + vvvvvvvvvv", frmCnt, enqCnt);
	}
    
	//check max
    if ( ISP_RT_BUF_SIZE == pstRTBuf->ring_buf.total_count ) 
	{
        LOG_ERR("[rtbc][E]:real time buffer number FULL");
        Ret = -EFAULT;
        //break;
    }        	
	
    //check if buffer exist
    for (i=0;i<ISP_RT_BUF_SIZE;i++) 
	{
        if ( pstRTBuf->ring_buf.data[i].base_pAddr == rt_buf.base_pAddr ) 
		{
            buffer_exist = 1;
            break;
        }
        if ( pstRTBuf->ring_buf.data[i].base_pAddr == 0 ) 
		{
            break;
        }
    }
	
    //put enq buf into rt_buf
    if (buffer_exist) 
	{
		//reset exist buff's status to empty
        if ( ISP_RT_BUF_EMPTY != pstRTBuf->ring_buf.data[i].bFilled ) 
		{
			if(ISP_RT_BUF_FILLED == pstRTBuf->ring_buf.data[i].bFilled)
			{
				LOG_ERR("[rtbc][E]frmCnt(%d)enqCnt(%d):idx(%d) enq again without deq!!!");
			}
            pstRTBuf->ring_buf.data[i].bFilled = ISP_RT_BUF_EMPTY;
            pstRTBuf->ring_buf.empty_count++;
            index = i;
        }
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			if(3 == pstRTBuf->ring_buf.total_count)
			{
            	LOG_INF("[rtbc][E]fC(%d)eC(%d)idx(%d):B<%d%d%d>/W(%d)/R(%d)/E(%d)",
					frmCnt, enqCnt, index, 
					pstRTBuf->ring_buf.data[0].bFilled, 
					pstRTBuf->ring_buf.data[1].bFilled, 
					pstRTBuf->ring_buf.data[2].bFilled, 
					pstRTBuf->ring_buf.start, 
					pstRTBuf->ring_buf.read, 
					pstRTBuf->ring_buf.empty_count); 
			}
			else if(1 == pstRTBuf->ring_buf.total_count)
			{
            	LOG_INF("[rtbc][E]fC(%d)eC(%d)idx(%d):B<%d>/W(%d)/R(%d)/E(%d)",
					frmCnt, enqCnt, index, 
					pstRTBuf->ring_buf.data[0].bFilled, 
					pstRTBuf->ring_buf.start, 
					pstRTBuf->ring_buf.read, 
					pstRTBuf->ring_buf.empty_count); 
			}
        }
    }
    else	//first time add, overwrite oldest element if buffer is full   
	{        
        if ( pstRTBuf->ring_buf.total_count == ISP_RT_BUF_SIZE) 
		{
            LOG_ERR("[rtbc][E]:buffer full(%d)", pstRTBuf->ring_buf.total_count);
        }
        else 
		{            
            index = pstRTBuf->ring_buf.total_count % ISP_RT_BUF_SIZE;            
            pstRTBuf->ring_buf.data[index].memID      = rt_buf.memID;
            pstRTBuf->ring_buf.data[index].size       = rt_buf.size;
            pstRTBuf->ring_buf.data[index].base_vAddr = rt_buf.base_vAddr;
            pstRTBuf->ring_buf.data[index].base_pAddr = rt_buf.base_pAddr;
            pstRTBuf->ring_buf.data[index].bFilled    = ISP_RT_BUF_EMPTY;            
            pstRTBuf->ring_buf.empty_count++;			
            pstRTBuf->ring_buf.total_count++;            
            if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
			{
                LOG_INF("[rtbc][E]:buff first in/idx(%d)/enqPA(0x%x)/empty(%d)/total(%d)",
                    index, pstRTBuf->ring_buf.data[index].base_pAddr,
                    pstRTBuf->ring_buf.empty_count, pstRTBuf->ring_buf.total_count);
            }
			//first enque, set base_addr at beginning before VF_EN
			if(pstRTBuf->ring_buf.total_count == 1)
			{				
				ISP_WR32(ISP_REG_IMGO_BASE_ADDR, pstRTBuf->ring_buf.data[0].base_pAddr);

				if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
				{
					LOG_INF("[rtbc][E]:set first buff to IMGO_BASE_ADDR(0x%x) before VF_EN(0x%x)", 
						ISP_RD32(ISP_REG_IMGO_BASE_ADDR), ISP_RD32(ISP_REG_TG_VF_CON));
				}
            }
        }
    }
	
if(!USE_NEW_RTBC)
{	
    //empty_count==1 means before enque it's 0, DMA stalled already or NOT start yet
    if ( 1 == pstRTBuf->ring_buf.empty_count) 
	{
        if (ISP_RT_BUF_IMGO == rt_dma) 
		{
            //set base_addr at beginning before VF_EN
            ISP_WR32(ISP_REG_IMGO_BASE_ADDR,pstRTBuf->ring_buf.data[index].base_pAddr);
        }
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][E]:set base_pAddr(0x%x), because empty_count(%d) is 1",
                pstRTBuf->ring_buf.data[index].base_pAddr,
                pstRTBuf->ring_buf.empty_count);
        }
		
        //disable FBC control to go on download
        if (ISP_RT_BUF_IMGO == rt_dma) 
		{
            reg_val = ISP_RD32(ISP_REG_IMGO_FBC);
            reg_val &= ~0x4000;
            ISP_WR32(ISP_REG_IMGO_FBC,reg_val);
        }
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][E]:disable fbc(0x%x)", ISP_RD32(ISP_REG_IMGO_FBC));
        }		
        pstRTBuf->ring_buf.pre_empty_count = pstRTBuf->ring_buf.empty_count;
    }
}
    
    if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
        LOG_DBG("[rtbc][E]:after enque to idx(%d) with pAddr(0x%x), start(%d), empty_count(%d)",
			index, pstRTBuf->ring_buf.data[index].base_pAddr, 
            pstRTBuf->ring_buf.start, pstRTBuf->ring_buf.empty_count);
    }
		
	if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
		//ISP_RTBC_PRINTBUF();
        LOG_DBG("[rtbc][E] -");
    }
	
	//
    //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
    //
    return Ret;
}

static MINT32 ISP_RTBC_DEQUE(MINT32 dma)
{
    MINT32 Ret = 0;
    //MINT32 rt_dma = dma;
    MUINT32 i=0;
    MUINT32 index = 0;

	//
    //spin_lock_irqsave(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
    //    
	deqCnt++;
	if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
		LOG_DBG("[rtbc][D]frmCnt(%d)deqCnt(%d) + ^^^^^^^^^^", frmCnt, deqCnt);	
	}
	
    //in SOF, "start" is next buffer index
    deque_buf.count = 0;    
    for ( i=0 ; i < pstRTBuf->ring_buf.total_count ; i++ ) 
	{    
        index = ( pstRTBuf->ring_buf.read + i ) % pstRTBuf->ring_buf.total_count;     
        if ( ISP_RT_BUF_FILLED == pstRTBuf->ring_buf.data[index].bFilled ) 
		{
            pstRTBuf->ring_buf.data[index].bFilled = ISP_RT_BUF_LOCKED;
            deque_buf.count= 1;

			if(index != pstRTBuf->ring_buf.read)
				LOG_ERR("[rtbc][D]:!! suppose read(%d), get Filled index(%d)", pstRTBuf->ring_buf.read, index);			
			
			//pstRTBuf->ring_buf.read = (pstRTBuf->ring_buf.read + 1)%pstRTBuf->ring_buf.total_count;			
			pstRTBuf->ring_buf.read = (index + 1)%pstRTBuf->ring_buf.total_count;			
            break;
        }
    }
	
    // deque but no buff got
    if (0==deque_buf.count)
	{
        //queue buffer status
        LOG_ERR("[rtbc][D]:deque no buff, deque_buf.count(%d), start(%d)/total_count(%d)/empty_count(%d)",
        	deque_buf.count, pstRTBuf->ring_buf.start, 
            pstRTBuf->ring_buf.total_count, pstRTBuf->ring_buf.empty_count);
        for ( i=0;i<pstRTBuf->ring_buf.total_count;i++ ) 
		{
            LOG_ERR("[rtbc][D]:buf(%d) id=%d/size=0x%x/va=0x%x/pa=0x%x/bFilled=%d",
                i, pstRTBuf->ring_buf.data[i].memID, pstRTBuf->ring_buf.data[i].size,
                pstRTBuf->ring_buf.data[i].base_vAddr, pstRTBuf->ring_buf.data[i].base_pAddr,
                pstRTBuf->ring_buf.data[i].bFilled);
        }
    }    
    if (deque_buf.count) 
	{
		unsigned long period;
        //Fill buffer head, "start" is current working index
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			if(pstRTBuf->ring_buf.data[index].timeStampUs > gDeqPreUsec)
				period = pstRTBuf->ring_buf.data[index].timeStampUs - gDeqPreUsec;
			else
				period = (pstRTBuf->ring_buf.data[index].timeStampUs+1000000) - gDeqPreUsec;
			gDeqPreUsec = pstRTBuf->ring_buf.data[index].timeStampUs;			
		  
			if(3 == pstRTBuf->ring_buf.total_count)
			{
            	LOG_INF("[rtbc][D]fC(%d)dC(%d)idx(%d):B<%d%d%d>/W(%d)/R(%d)/E(%d), t(%ld)/p(%ld)",
					frmCnt, deqCnt, index,
					pstRTBuf->ring_buf.data[0].bFilled, 
					pstRTBuf->ring_buf.data[1].bFilled, 
					pstRTBuf->ring_buf.data[2].bFilled, 					 					
					pstRTBuf->ring_buf.start, 
					pstRTBuf->ring_buf.read, 
					pstRTBuf->ring_buf.empty_count,
					gDeqPreUsec, period);  

				if(period > 34000)
					dropCnt++;
			}
			else if(1 == pstRTBuf->ring_buf.total_count)
			{
            	LOG_INF("[rtbc][D]fC(%d)dC(%d)idx(%d):B<%d>/W(%d)/R(%d)/E(%d), t(%ld)/p(%ld)",
					frmCnt, deqCnt, index,
					pstRTBuf->ring_buf.data[0].bFilled, 					 					
					pstRTBuf->ring_buf.start, 
					pstRTBuf->ring_buf.read, 
					pstRTBuf->ring_buf.empty_count,
					gDeqPreUsec, period);  
			}						
        }
        //
        for (i=0;i<deque_buf.count;i++) 
		{
            deque_buf.data[i].memID         = pstRTBuf->ring_buf.data[index+i].memID;
            deque_buf.data[i].size          = pstRTBuf->ring_buf.data[index+i].size;
            deque_buf.data[i].base_vAddr    = pstRTBuf->ring_buf.data[index+i].base_vAddr;
            deque_buf.data[i].base_pAddr    = pstRTBuf->ring_buf.data[index+i].base_pAddr;
            deque_buf.data[i].timeStampS    = pstRTBuf->ring_buf.data[index+i].timeStampS;
            deque_buf.data[i].timeStampUs   = pstRTBuf->ring_buf.data[index+i].timeStampUs;
            //
            if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
			{
                LOG_DBG("[rtbc][D]:idx(%d)/PA(0x%x)/memID(%d)/size(0x%x)/VA(0x%x)",
                    index+i,
                    deque_buf.data[i].base_pAddr,
                    deque_buf.data[i].memID,
                    deque_buf.data[i].size,
                    deque_buf.data[i].base_vAddr);
            }
        }     
    }
    else 
	{
        LOG_ERR("[rtbc][D]:no filled buffer");
        Ret = -EFAULT;
    }	

	if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
		//ISP_RTBC_PRINTBUF();
		LOG_DBG("[rtbc][D] -");
	}

	//
	//spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
	//
    return Ret;
}
/*******************************************************************************
* ISP_RT_BUF_CTRL_ENQUE: copy buff addr from user, and wait until enque done at SOF irq
* ISP_RT_BUF_CTRL_DEQUE: wait utill deque done at SOF irq, and copy buff addr to user
* ISP_RT_BUF_CTRL_IS_RDY: buff all empty => return 1; else => return 0
* ISP_RT_BUF_CTRL_GET_SIZE: return buff total count
* ISP_RT_BUF_CTRL_CLEAR: clear pstRTBuf contain as 0
********************************************************************************/
static long ISP_Buf_CTRL_FUNC(MUINT32 Param)
{
    MINT32 Ret = 0;
    MINT32 rt_dma;
    MUINT32 reg_val = 0;
    MUINT32 i = 0;
    //MUINT32 iBuf = 0;
    MUINT32 size = 0;
    MUINT32 bWaitBufRdy = 0;
    ISP_RT_BUF_CTRL_STRUCT rt_buf_ctrl;
    //MUINT32 buffer_exist = 0;

    MINT32 Timeout = 5000; //ms
    //MUINT32 tstamp = 0;
    
    //
    if (NULL == pstRTBuf)  
	{
        LOG_ERR("[rtbc][CTRL]:NULL pstRTBuf");
        return -EFAULT;
    }
    //
    if(copy_from_user(&rt_buf_ctrl, (void*)Param, sizeof(ISP_RT_BUF_CTRL_STRUCT)) == 0)
    {
        rt_dma = rt_buf_ctrl.buf_id;
        //
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][CTRL]:ctrl(0x%x)/buf_id(0x%x)/data_ptr(0x%x)", rt_buf_ctrl.ctrl,rt_buf_ctrl.buf_id,rt_buf_ctrl.data_ptr);
        }
        //
        if (ISP_RT_BUF_IMGO != rt_dma) 
		{
            LOG_ERR("[rtbc][CTRL]:invalid dma channel(%d)", rt_dma);
            return -EFAULT;
        }
        //
        switch(rt_buf_ctrl.ctrl) 
		{
			case ISP_RT_BUF_CTRL_ENQUE:
            case ISP_RT_BUF_CTRL_EXCHANGE_ENQUE:                
                if(copy_from_user(&rt_buf, (void*)rt_buf_ctrl.data_ptr, sizeof(ISP_RT_BUF_STRUCT)) == 0) 
				{
                    reg_val  = ISP_RD32(ISP_REG_TG_VF_CON);                                   
                    //VF start already
                    if ((reg_val & 0x01) && ENQ_SYNC_SOF)
					{		                     
                        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
						{
                            LOG_DBG("[rtbc][CTRL]:<queue> wait E done, reg_val(0x%x)", reg_val);
                        }
                        //wait till enq done in SOF
                        rtbc_enq_dma = rt_dma;
                        bEnqBuf = 1;
                        Timeout = wait_event_interruptible_timeout(
                                    IspInfo.WaitQueueHead,
                                    (0==bEnqBuf),
                                    ISP_MsToJiffies(Timeout));
                        //
                        if(Timeout == 0)
                        {
                            LOG_ERR("[rtbc][CTRL]:<queue> enque timeout(%d)", bEnqBuf);
                            Ret = -EFAULT;
                        }
                        //
                        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
						{
                            LOG_DBG("[rtbc][CTRL]:<queue> E done");
                        }
                    }
                    else 
					{
						LOG_DBG("[rtbc][CTRL]:<queue> direct call E");
                        ISP_RTBC_ENQUE(rt_dma);
                    }
                }
                else 
				{
                    LOG_ERR("[rtbc][CTRL]:<queue> enque copy_from_user fail");
                    return -EFAULT;
                }
                break;
				
            case ISP_RT_BUF_CTRL_DEQUE:
                reg_val  = ISP_RD32(ISP_REG_TG_VF_CON);
                //VF start already
                if ((reg_val & 0x01) && DEQ_SYNC_SOF)
				{					                 
                    if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
					{
                        LOG_DBG("[rtbc][CTRL]:<queue> wait D done, reg_val(0x%x)", reg_val);
                    }
                    //wait till deq done in SOF
                    rtbc_deq_dma = rt_dma;
                    bDeqBuf = 1;
                    Timeout = wait_event_interruptible_timeout(
                                IspInfo.WaitQueueHead,
                                (0==bDeqBuf),
                                ISP_MsToJiffies(Timeout));
                    //
                    if(Timeout == 0)
                    {
                        LOG_ERR("[rtbc][CTRL]:<queue> deque timeout(%d)", bDeqBuf);
                        LOG_ERR("[rtbc][CTRL]:<queue> ISP_IRQ_INT:IrqStatus(0x%08X)", IspInfo.IrqInfo.Status);
                        ISP_DumpReg();
                        Ret = -EFAULT;
                    }
                    if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
					{
                        LOG_DBG("[rtbc][CTRL]:<queue> D done");
                    }
                }
                else 
				{
					LOG_DBG("[rtbc][CTRL]:<queue> direct call D");
                    ISP_RTBC_DEQUE(rt_dma);
                }
                if (deque_buf.count) 
				{
                    if(copy_to_user((void*)rt_buf_ctrl.data_ptr, &deque_buf, sizeof(ISP_RT_DEQUE_BUF_STRUCT)) != 0)
                    {
                        LOG_ERR("[rtbc][CTRL]:<queue> deque copy_to_user failed");
                        Ret = -EFAULT;
                    }
                }
                else 
				{
                    //
                    //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
                    LOG_ERR("[rtbc][CTRL]:<queue> deque no filled buffer");
                    Ret = -EFAULT;
                }
                break;
				
            case ISP_RT_BUF_CTRL_IS_RDY:
                //
                //spin_lock_irqsave(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
                //
                bWaitBufRdy = 1;				
                for ( i=0 ; i < pstRTBuf->ring_buf.total_count ; i++ ) 
				{                 
                    if ( ISP_RT_BUF_FILLED == pstRTBuf->ring_buf.data[i].bFilled ) 
					{
                        bWaitBufRdy = 0;
						if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
						{
							LOG_INF("[rtbc][CTRL]:check buff(%d) is filled", i);
						}
                        break;
                    }
                }
                //
                if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
				{
					if(i==pstRTBuf->ring_buf.total_count)
	                    LOG_INF("[rtbc][CTRL]:check buff not rdy, bWaitBufRdy(%d)", bWaitBufRdy);			
                }

                //
                //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
                //
                if(copy_to_user((void*)rt_buf_ctrl.data_ptr, &bWaitBufRdy, sizeof(MUINT32)) != 0)
                {
                    LOG_ERR("[rtbc][CTRL]:check rdy, copy_to_user failed");
                    Ret = -EFAULT;
                }
                //
                //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC), g_Flash_SpinLock);
                //
                break;
				
            case ISP_RT_BUF_CTRL_GET_SIZE:
                //
                size = pstRTBuf->ring_buf.total_count;
                //
                if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
				{
                    LOG_DBG("[rtbc][CTRL]:getsize(%d)", size);
                }
                if(copy_to_user((void*)rt_buf_ctrl.data_ptr, &size, sizeof(MUINT32)) != 0)
                {
                    LOG_ERR("[rtbc][CTRL]:getsize, copy_to_user failed");
                    Ret = -EFAULT;
                }
                break;
				
            case ISP_RT_BUF_CTRL_CLEAR:
                //
                if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
				{
                    LOG_DBG("[rtbc][CTRL]:clear");
                }
                //
				#if 0
                pstRTBuf->ring_buf.total_count= 0;
                pstRTBuf->ring_buf.start    = 0;
                pstRTBuf->ring_buf.empty_count= 0;
                pstRTBuf->ring_buf.active   = 0;

                for (i=0;i<ISP_RT_BUF_SIZE;i++) {
                    if ( pstRTBuf->ring_buf.data[i].base_pAddr == rt_buf.base_pAddr ) {
                        buffer_exist = 1;
                        break;
                    }
                    //
                    if ( pstRTBuf->ring_buf.data[i].base_pAddr == 0 ) {
                        break;
                    }
                }
				#else
				LOG_INF("[rtbc][PRINT_LOG]frmCnt=%d, deqCnt=%d, enqCnt=%d, dropCnt=%d", 
							frmCnt, deqCnt, enqCnt, dropCnt);
				irqSOFCnt = irqPASS1Cnt = 0;	
				gSofPreSec = gPass1PreSec = 0;
				gSofPreUsec = gPass1PreUsec = 0;
				frmCnt = deqCnt = enqCnt = dropCnt = 0;		
				gIrqLogCnt = 0;
                SOF_PASS1DONE_PAIRED = 0;
				memset((char*)(&gCameraIspIrqLog),0x00,sizeof(CAMERA_ISP_IRQ_LOG_STRUCT));				
                memset((char*)pstRTBuf,0x00,sizeof(ISP_RT_BUF_INFO_STRUCT));
                //prv_tstamp = 0;
				#endif
                break;

            case ISP_RT_BUF_CTRL_MAX:   // Add this to remove build warning.
                // Do nothing.
                break;
        }
        //
    }
    else
    {
        LOG_ERR("[rtbc][CTRL]:copy_from_user failed");
        Ret = -EFAULT;
    }

    return Ret;
}
/*******************************************************************************
* Update next buff addr to ISP reg at SOF irq
* Do Deque|Enque and wake up waitqueue
********************************************************************************/
static MINT32 ISP_SOF_Buf_Get(unsigned long long sec,unsigned long usec)
{
	int i;
	int i_dma;
	unsigned int pAddr = 0;
	unsigned int dma_base_addr = 0;
	unsigned int next = 0;
	//unsigned int reg_val = 0;
    unsigned int error = 0;
    unsigned int tempvalue = 0;
    //
    //spin_lock_irqsave(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
    //

	//if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	//{
    //    LOG_WRN("[rtbc][irq][S]frmCnt(%d) + ////////// sec=%lld usec=%ld", irqSOFCnt, sec, usec);
    //}

    i_dma = ISP_RT_BUF_IMGO;
    dma_base_addr = ISP_REG_IMGO_BASE_ADDR;

if(!USE_NEW_RTBC)
{	
    //queue buffer status
    if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
        LOG_DBG("[rtbc][S]:real time buffer info -> start(%d), total_count(%d), empty_count(%d)",
            pstRTBuf->ring_buf.start, pstRTBuf->ring_buf.total_count, pstRTBuf->ring_buf.empty_count);

        for ( i=0 ; i < pstRTBuf->ring_buf.total_count ; i++ ) 
		{
            LOG_DBG("[rtbc][S]:buf(%d) id=%d/size=0x%x/va=0x%x/pa=0x%x/bFilled=%d",
                i, pstRTBuf->ring_buf.data[i].memID, pstRTBuf->ring_buf.data[i].size,
                pstRTBuf->ring_buf.data[i].base_vAddr, pstRTBuf->ring_buf.data[i].base_pAddr,
                pstRTBuf->ring_buf.data[i].bFilled);
        }
    }

    //ring buffer get next buffer
    if (0 == pstRTBuf->ring_buf.empty_count) 
	{
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][S]:real time buffer number empty, start(%d)", pstRTBuf->ring_buf.start);
        }

		//once if buffer put into queue between SOF and ISP_DONE.
        pstRTBuf->ring_buf.active = MFALSE;
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][S]:set active = %d", pstRTBuf->ring_buf.active);
        }		
    }
    else 
	{     
        if (2 <= pstRTBuf->ring_buf.empty_count) 
		{
            //next buffer
            next = (pstRTBuf->ring_buf.start+1)%pstRTBuf->ring_buf.total_count;
            pAddr = pstRTBuf->ring_buf.data[next].base_pAddr;
            //
            ISP_WR32(dma_base_addr, pAddr);
            //
            if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
			{
                LOG_DBG("[rtbc][S]:start(%d), empty_count(%d), next(%d), nextPA(0x%x) ",
                    pstRTBuf->ring_buf.start, pstRTBuf->ring_buf.empty_count, 
                    next, pAddr);
            }
        }
        else
		{
            if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
			{
                LOG_DBG("[rtbc][S]:real time buffer number is running out");
            }
        }

        //once if buffer put into queue between SOF and ISP_DONE.
        pstRTBuf->ring_buf.active = MTRUE;
		if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][S]:set active = %d", pstRTBuf->ring_buf.active);
    }
    }
	
    //
    if( bEnqBuf ) 
	{
        ISP_RTBC_ENQUE(rtbc_enq_dma);
        bEnqBuf = 0;
        wake_up_interruptible(&IspInfo.WaitQueueHead);
    }
    //
    if (bDeqBuf) 
	{
        ISP_RTBC_DEQUE(rtbc_deq_dma);
        bDeqBuf = 0;
        wake_up_interruptible(&IspInfo.WaitQueueHead);
    }
}

	// fixed timing issue in post-MP; update next buffer to ISP when VF open
	if( ISP_RD32(ISP_REG_TG_VF_CON) & 0x00000001 )
	{
        if (1 == SOF_PASS1DONE_PAIRED)
        {
            //Error handling, if the sensor send the frame start, but no any frame done.
           error = 1;
           tempvalue = *((unsigned int*)(0xF4014018));
           LOG_ERR("isp ovrun");
        }
        SOF_PASS1DONE_PAIRED = 1;
        if (0x04 == DevId) //0x01 main sensor, 0x02 sub sensor, 0x04 atv sensor
        {
		   dma_base_addr = ISP_REG_IMGO_BASE_ADDR;
    	   pAddr = pstRTBuf->ring_buf.data[(pstRTBuf->ring_buf.start+1)%pstRTBuf->ring_buf.total_count].base_pAddr;
		   ISP_WR32(dma_base_addr, pAddr);
		}
    }

    //if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	//{
		//ISP_RTBC_PRINTBUF();
	//	LOG_WRN("[rtbc][irq][S]: -");        
    //}
    pstRTBuf->state = ISP_RT_BUF_INFO_SOF;
	
	//
    //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
	//
    return 0;
}
/*******************************************************************************
* Switch index to the next empty buffer at Pass1Done irq
* If no any empty buffer, use ISP_REG_IMGO_FBC to stall DMA
********************************************************************************/
static MINT32 ISP_DONE_Buf_Time(unsigned long long sec,unsigned long usec)
{
	//int k;
	int i;
	int i_dma;
	unsigned int curr, next;
	unsigned int pAddr = 0;
	unsigned int reg_fbc;
	MUINT32 reg_val = 0;
	#if 0
    if ( spin_trylock_bh(&(IspInfo.SpinLockRTBC)) ) {
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) {
            LOG_DBG("[rtbc]:unlock state");
        }
    }
    else {
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) {
            LOG_DBG("[rtbc]:locked state");
        }
    }
	#endif
	//
    //spin_lock_irqsave(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
    //
	 
	//if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	//{
    //    LOG_WRN("[rtbc][irq][N]frmCnt(%d) + ---------- sec=%lld usec=%ld", irqPASS1Cnt, sec, usec);
		frmCnt = irqPASS1Cnt;
    //}

    i_dma = ISP_RT_BUF_IMGO;
    reg_fbc = ISP_REG_IMGO_FBC;
	   
if(USE_NEW_RTBC)
{
	//set curr buf as filled
	SOF_PASS1DONE_PAIRED = 0;
	curr = pstRTBuf->ring_buf.start;
	if(0x0 == pstRTBuf->ring_buf.data[curr].base_pAddr)
	{
		/*
		if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			LOG_DBG("[rtbc][irq][N]:no buffer enque, total_count(%d)", pstRTBuf->ring_buf.total_count);
    	}
    	*/
		goto EXIT;
	}
	if(pstRTBuf->ring_buf.data[curr].bFilled == ISP_RT_BUF_EMPTY)
	{
		pAddr = pstRTBuf->ring_buf.data[curr].base_pAddr;
		pstRTBuf->ring_buf.data[curr].bFilled = ISP_RT_BUF_FILLED;
		pstRTBuf->ring_buf.empty_count--;		
		pstRTBuf->ring_buf.data[curr].timeStampS = sec;
    	pstRTBuf->ring_buf.data[curr].timeStampUs = usec;
		/*
    	if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			LOG_WRN("[rtbc][irq][N]frmCnt(%d):filled cur buff(%d)/pA(0x%08X), timeStamp=sec(%lld),usec(%ld)", 
				frmCnt, curr, pAddr, sec, usec);
    	}
    	*/
	}
	
	//find next empty buf
	next = (curr)%pstRTBuf->ring_buf.total_count;
	for(i = 0 ; i < pstRTBuf->ring_buf.total_count ; i++)
	{
		if(pstRTBuf->ring_buf.data[next].bFilled == ISP_RT_BUF_EMPTY)
        {            	        
        	break;
		}
		next = (next+1)%pstRTBuf->ring_buf.total_count;
	}		
	if(next != curr)
	{	//update to ISP HW
		//reg_val = ISP_RD32(ISP_REG_IMGO_FBC);			
		//reg_val &= 0xBFFF;
		//ISP_WR32(ISP_REG_IMGO_FBC,reg_val);
		
		pAddr = pstRTBuf->ring_buf.data[next].base_pAddr;
		ISP_WR32(ISP_REG_IMGO_BASE_ADDR, pAddr);
		
		pstRTBuf->ring_buf.start = next;
		/*
		if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			LOG_DBG("[rtbc][irq][N]frmCnt(%d):set next buff(%d)/pA(0x%08X) to CAM_WDMA, fbc(0x%08X)", 
				frmCnt, next, pAddr, reg_val);
		}
		*/
	}
	else
	{	//stall DMA by FBC
		//reg_val = ISP_RD32(ISP_REG_IMGO_FBC);			
		//reg_val |= 0x4000;
		//ISP_WR32(ISP_REG_IMGO_FBC,reg_val);		
		LOG_ERR("[rtbc][N]frmCnt(%d):stall at cur buff(%d)!!!", frmCnt, curr);
        
		if(pstRTBuf->ring_buf.data[curr].bFilled == ISP_RT_BUF_LOCKED)
        {      
            next = (curr + pstRTBuf->ring_buf.total_count - 1)%pstRTBuf->ring_buf.total_count;
			pstRTBuf->ring_buf.data[next].bFilled = ISP_RT_BUF_EMPTY;
			pstRTBuf->ring_buf.empty_count++;
        }
        else if(pstRTBuf->ring_buf.data[curr].bFilled == ISP_RT_BUF_FILLED)
        {
            next = curr;
			pstRTBuf->ring_buf.data[next].bFilled = ISP_RT_BUF_EMPTY;			
			pstRTBuf->ring_buf.empty_count++;
        }

        pAddr = pstRTBuf->ring_buf.data[next].base_pAddr;
		ISP_WR32(ISP_REG_IMGO_BASE_ADDR, pAddr);
        pstRTBuf->ring_buf.start = next;

		/*
		if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
			LOG_DBG("[rtbc][irq][N]frmCnt(%d):stall at cur buff(%d)/pA(0x%08X) since no empty buff left, fbc(0x%08X)", 
				frmCnt, curr, pAddr, reg_val);
		}
		*/
	}
}
else
{ 	
	//exit if no empty buff
    if (0 == pstRTBuf->ring_buf.empty_count) 
	{    
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
		   LOG_DBG("[rtbc][N]:real time buffer number empty, start(%d) ", pstRTBuf->ring_buf.start);
        }     
        goto EXIT;
    }
    //once if buffer put into queue between SOF and ISP_DONE.
    if (MFALSE == pstRTBuf->ring_buf.active ) 
	{    
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
		   LOG_DBG("[rtbc][N]:real time buffer active false, missing SOF ");
        }     
        goto EXIT;
    }

	// search empty buff
    while(1)
    {
        MUINT32 loopCount = 0;
        curr = pstRTBuf->ring_buf.start;

		//
        if(pstRTBuf->ring_buf.data[curr].bFilled == ISP_RT_BUF_EMPTY)
        {           			
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            	LOG_DBG("[rtbc][N]:curr(%d).bFilled(%d) is ISP_RTBC_BUF_EMPTY, OK", 
					curr, pstRTBuf->ring_buf.data[curr].bFilled);
        }        
            pstRTBuf->ring_buf.data[curr].bFilled = ISP_RT_BUF_FILLED;
			pstRTBuf->ring_buf.empty_count--;			
            //start + 1
            pstRTBuf->ring_buf.start = (curr+1)%pstRTBuf->ring_buf.total_count;            
            break;
        }
        else
        {
            if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
			{
                LOG_DBG("[rtbc][N]:curr(%d).bFilled(%d) != ISP_RTBC_BUF_EMPTY, try next one", 
					curr, pstRTBuf->ring_buf.data[curr].bFilled);
            }
            //start + 1
            pstRTBuf->ring_buf.start = (curr+1)%pstRTBuf->ring_buf.total_count;
        }

        loopCount++;
        if(loopCount > pstRTBuf->ring_buf.total_count)
        {
            LOG_ERR("[rtbc][N]:Can't find empty buf in total_count(%d)",
                    pstRTBuf->ring_buf.total_count);
            break;
        }
    }
	pstRTBuf->ring_buf.data[curr].timeStampS = sec;
    pstRTBuf->ring_buf.data[curr].timeStampUs = usec;
    if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	{
		LOG_DBG("[rtbc][N]:curr(%d), sec(%lld), usec(%ld)", 
			curr, sec, usec);
        LOG_DBG("[rtbc][N]:start(%d), empty_count(%d)", 
			pstRTBuf->ring_buf.start, pstRTBuf->ring_buf.empty_count);
    }

    //enable fbc to stall DMA
    LOG_DBG("[rtbc][N]:fbc(0x%x)", ISP_RD32(reg_fbc));	
    if ( 0 == pstRTBuf->ring_buf.empty_count) 
	{
        if (ISP_RT_BUF_IMGO == i_dma) 
		{
            reg_val = ISP_RD32(ISP_REG_IMGO_FBC);
			//reg_val |= (pstRTBuf->ring_buf.total_count<<16);
            reg_val |= 0x4000;
            //ISP_WR32(ISP_REG_IMGO_FBC,reg_val);
        }            
        if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
		{
            LOG_DBG("[rtbc][N]:en fbc(0x%x) to stall DMA out", ISP_RD32(reg_fbc));
        }
    }  
}

	EXIT:
    //if(IspInfo.DebugMask & ISP_DBG_RT_BUF_CTRL) 
	//{
		//ISP_RTBC_PRINTBUF();
    //    LOG_WRN("[rtbc][irq][N]: -");
    //}
    pstRTBuf->state = ISP_RT_BUF_INFO_DONE;
	
	//
    //spin_unlock_irqrestore(&(IspInfo.SpinLockRTBC),g_Flash_SpinLock);
	//
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_WaitIrq(ISP_WAIT_IRQ_STRUCT WaitIrq)
{
    MINT32 Ret = 0, Timeout = WaitIrq.Timeout;
    //MUINT32 i;
    unsigned long flags;
    //
    if(IspInfo.DebugMask & ISP_DBG_INT)
    {
        LOG_DBG("Wait: Clear(%d), Status(0x%08X), Timeout(%d)",
                    WaitIrq.Clear,
                    WaitIrq.Status,
                    WaitIrq.Timeout);
    }
    //
    if(WaitIrq.Clear == ISP_IRQ_CLEAR_WAIT)
    {
        spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
        if(IspInfo.IrqInfo.Status & WaitIrq.Status)
        {            
            LOG_DBG("WARNING: Clear(%d): IrqStatus(0x%08X) has been cleared", WaitIrq.Clear, IspInfo.IrqInfo.Status & WaitIrq.Status);
            IspInfo.IrqInfo.Status &= (~WaitIrq.Status);            
        }
        spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
    }
    else if(WaitIrq.Clear == ISP_IRQ_CLEAR_ALL)
    {
        spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
        LOG_DBG("WARNING: Clear(%d): IrqStatus(0x%08X) has been cleared", WaitIrq.Clear, IspInfo.IrqInfo.Status);
        IspInfo.IrqInfo.Status = 0;
        spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
    }
    //    
    Timeout = wait_event_interruptible_timeout(
                IspInfo.WaitQueueHead,
                ISP_GetIRQState(WaitIrq.Status),//(IspInfo.IrqInfo.Status & WaitIrq.Status),
                ISP_MsToJiffies(WaitIrq.Timeout));
    //
    if(Timeout == 0)
    {
        LOG_ERR("Timeout: Clear(%d), IrqStatus(0x%08X), WaitStatus(0x%08X), Timeout(%d)",
                    WaitIrq.Clear,
                    IspInfo.IrqInfo.Status,
                    WaitIrq.Status,
                    WaitIrq.Timeout);
		
		ISP_DumpReg();
        
        Ret = -EFAULT;
        goto EXIT;
    }
    //
    spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
    //
    if(IspInfo.DebugMask & ISP_DBG_INT)
    {
		LOG_DBG("After wait: IrqStatus(0x%08X)", IspInfo.IrqInfo.Status);     
    }
    //
    IspInfo.IrqInfo.Status &= (~WaitIrq.Status);
    //
    spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
    //
    EXIT:
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static __tcmfunc irqreturn_t ISP_Irq(
    MINT32  Irq,
    void*   DeviceId)
{
    //MUINT32 i;
    MUINT32 IrqStatus;
    // Read irq status
    IrqStatus = (ISP_RD32(ISP_REG_INT_STATUS)&(IspInfo.IrqInfo.Mask|IspInfo.IrqInfo.ErrMask));
    //
    spin_lock(&(IspInfo.SpinLockIrq));
    if(IspInfo.IrqInfo.ErrMask & IrqStatus)
    {
        LOG_DBG("Error IRQ, Status(0x%08X)",IspInfo.IrqInfo.ErrMask & IrqStatus);
        //TODO: Add error handler...
    }
    //
    if(IspInfo.DebugMask & ISP_DBG_INT)
    {
        LOG_DBG("IrqStatus(0x%08X | 0x%08X)",IspInfo.IrqInfo.Status, IrqStatus);
    }
    IspInfo.IrqInfo.Status |= (IrqStatus & IspInfo.IrqInfo.Mask);
    spin_unlock(&(IspInfo.SpinLockIrq));

    //service pass1_done first once if SOF/PASS1_DONE are coming together.
    
    //get time stamp
    //push hw filled buffer to sw list
    if(IrqStatus & ISP_INT_PASS1_TG1_DON)
    {
		unsigned long long  sec;
		unsigned long       usec;
		unsigned long  		period;		
		sec = cpu_clock(0);				// get ns from cpu_clock()
		do_div(sec, 1000);				// calc us and save to "sec"
		usec = do_div( sec, 1000000);	// quotient is saved to sec, and remainder is saved to usec

		irqPASS1Cnt++;	
		if(sec > gPass1PreSec)
			period = (usec + 1000000) - gPass1PreUsec;
		else
			period = usec - gPass1PreUsec;
		//LOG_WRN("[rtbc]<FPS check> irq PASS1 %d, time:%ld (sec=%lld usec=%ld)", irqPASS1Cnt, period, sec, usec);

		ISP_DONE_Buf_Time(sec,usec);
		gPass1PreSec = sec;
		gPass1PreUsec = usec;

		// fps log
		gCameraIspIrqLog[gIrqLogCnt].cnt = irqPASS1Cnt;
		gCameraIspIrqLog[gIrqLogCnt].period = period;
		gCameraIspIrqLog[gIrqLogCnt].sec = sec;
		gCameraIspIrqLog[gIrqLogCnt].usec = usec;
		gCameraIspIrqLog[gIrqLogCnt].irq_type = 1;		
		gIrqLogCnt++;
		if(IRQ_LOG_SIZE==gIrqLogCnt)
		{
			gIrqLogCnt = 0;
		}
    }
    //switch pass1 WDMA buffer
    //fill time stamp
    if(IrqStatus & ISP_INT_TG1_SOF)
    {
        unsigned long long  sec;
        unsigned long       usec;
		unsigned long  		period;	
		sec = cpu_clock(0);				// get ns from cpu_clock()
		do_div(sec, 1000);				// calc us and save to "sec"
		usec = do_div( sec, 1000000);	// quotient is saved to sec, and remainder is saved to usecec

		irqSOFCnt++;
		if(sec > gSofPreSec)
			period = (usec + 1000000) - gSofPreUsec;
		else
			period = usec - gSofPreUsec;
		//LOG_WRN("[rtbc]<FPS check> irq SOF %d, time:%ld (sec=%lld usec=%ld)", irqSOFCnt, period, sec, usec);
		
        ISP_SOF_Buf_Get(sec,usec);
		gSofPreSec = sec;
		gSofPreUsec = usec;

		// fps log
		gCameraIspIrqLog[gIrqLogCnt].cnt = irqSOFCnt;
		gCameraIspIrqLog[gIrqLogCnt].period = period;
		gCameraIspIrqLog[gIrqLogCnt].sec = sec;
		gCameraIspIrqLog[gIrqLogCnt].usec = usec;
		gCameraIspIrqLog[gIrqLogCnt].irq_type = 0;		
		gIrqLogCnt++;
		if(IRQ_LOG_SIZE==gIrqLogCnt)
		{
			gIrqLogCnt = 0;
		}
    }  
    wake_up_interruptible(&IspInfo.WaitQueueHead);
	
    //schedule_work: Work queue. It is process context, interruptible, so there can be "Sleep" in work queue function.
    //tasklet_schedule: Tasklet. It is interrupt context, uninterrupted, so there can NOT be "Sleep" in tasklet function.
    if(IrqStatus & ISP_INT_VS1)
    {
        IspInfo.TimeLog.Vd = ISP_JiffiesToMs(jiffies);
        schedule_work(&IspInfo.ScheduleWorkVD);
        tasklet_schedule(&IspTaskletVD);
    }
    if(IrqStatus & ISP_INT_EXPDON1)
    {
        IspInfo.TimeLog.Expdone = ISP_JiffiesToMs(jiffies);
        schedule_work(&IspInfo.ScheduleWorkEXPDONE);
        tasklet_schedule(&IspTaskletEXPDONE);
    }
	
    return IRQ_HANDLED;
}
/*******************************************************************************
*
********************************************************************************/
static long ISP_ioctl(
    struct file*    pFile,
    unsigned int    Cmd,
    unsigned long   Param)
{
    MINT32 Ret = 0;
    //
    MBOOL   HoldEnable = MFALSE;
    MUINT32 DebugFlag = 0;
    ISP_REG_IO_STRUCT       RegIo;
    ISP_HOLD_TIME_ENUM      HoldTime;
    ISP_WAIT_IRQ_STRUCT     WaitIrq;
    MUINT32     			ReadIrqStatus;
    MUINT32    				ClearIrqStatus;
    MINT32                 DevIdTmp;
    
    ISP_USER_INFO_STRUCT*   pUserInfo;
    unsigned long flags;
    //
    if(pFile->private_data == NULL)
    {
        LOG_WRN("private_data is NULL,(process, pid, tgid)=(%s, %d, %d)", current->comm , current->pid, current->tgid);
        return -EFAULT;
    }
    //
    pUserInfo = (ISP_USER_INFO_STRUCT*)(pFile->private_data);
    //
    switch(Cmd)
    {
        case ISP_IOC_RESET:
        {
            spin_lock(&(IspInfo.SpinLockIsp));
            ISP_Reset();
            spin_unlock(&(IspInfo.SpinLockIsp));
            break;
        }
        case ISP_IOC_RESET_BUF:
        {
            spin_lock_bh(&(IspInfo.SpinLockHold));
            ISP_ResetBuf();
            spin_unlock_bh(&(IspInfo.SpinLockHold));
            break;
        }
        case ISP_IOC_READ_REG:
        {
            if(copy_from_user(&RegIo, (void*)Param, sizeof(ISP_REG_IO_STRUCT)) == 0)
            {
                Ret = ISP_ReadReg(&RegIo);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_WRITE_REG:
        {
            if(copy_from_user(&RegIo, (void*)Param, sizeof(ISP_REG_IO_STRUCT)) == 0)
            {
                Ret = ISP_WriteReg(&RegIo);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_HOLD_REG_TIME:
        {
            if(copy_from_user(&HoldTime, (void*)Param, sizeof(ISP_HOLD_TIME_ENUM)) == 0)
            {
                spin_lock(&(IspInfo.SpinLockIsp));
                Ret = ISP_SetHoldTime(HoldTime);
                spin_unlock(&(IspInfo.SpinLockIsp));
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_HOLD_REG:
        {
            if(copy_from_user(&HoldEnable, (void*)Param, sizeof(MBOOL)) == 0)
            {
                Ret = ISP_EnableHoldReg(HoldEnable);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_WAIT_IRQ:
        {
            if(copy_from_user(&WaitIrq, (void*)Param, sizeof(ISP_WAIT_IRQ_STRUCT)) == 0)
            {
                Ret = ISP_WaitIrq(WaitIrq);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_READ_IRQ:
        {
            if(copy_from_user(&ReadIrqStatus, (void*)Param, sizeof(MUINT32)) == 0)
            {
                //
                ReadIrqStatus = IspInfo.IrqInfo.Status;
                //
                if(copy_to_user((void*)Param, &ReadIrqStatus, sizeof(MUINT32)) != 0)
                {
                    LOG_ERR("copy_to_user failed");
                    Ret = -EFAULT;
                }
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_CLEAR_IRQ:
        {
            if(copy_from_user(&ClearIrqStatus, (void*)Param, sizeof(MUINT32)) == 0)
            {
                spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
                //
                LOG_DBG("ISP_CLEAR_IRQ:Status(0x%08X),IrqStatus(0x%08X)",ClearIrqStatus,IspInfo.IrqInfo.Status);
                IspInfo.IrqInfo.Status &= (~ClearIrqStatus);
                spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_SET_DEVICE_ID:
        {
            if(copy_from_user(&DevIdTmp, (void*)Param, sizeof(MINT32)) == 0)
            {
                DevId = DevIdTmp;
                LOG_ERR("camera device id: %d",DevId);
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
            break;
        }
        case ISP_IOC_DUMP_REG:
        {
            Ret = ISP_DumpReg();
            break;
        }        
		case ISP_IOC_PLL_SEL_IRQ:
		{			
			ISP_PLL_SEL_STRUCT	PllCtrl;
			if(copy_from_user(&PllCtrl, (void*)Param, sizeof(ISP_PLL_SEL_STRUCT)) == 0)
			{
				if(MCLK_USING_UNIV_48M == PllCtrl.MclkSel)
					clkmux_sel(MT_CLKMUX_CAM_MUX_SEL, MT_CG_UNIV_48M, "CAMERA");
				else if(MCLK_USING_UNIV_208M == PllCtrl.MclkSel)					
					clkmux_sel(MT_CLKMUX_CAM_MUX_SEL, MT_CG_UPLL_D6, "CAMERA");
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
			break;
		}
		case ISP_IOC_GPIO_SEL_IRQ:
		{
						
			ISP_GPIO_SEL_STRUCT	GpioCtrl;
			if(copy_from_user(&GpioCtrl, (void*)Param, sizeof(ISP_GPIO_SEL_STRUCT)) == 0)
            {
            	mt_set_gpio_mode(GpioCtrl.Pin, GpioCtrl.Mode);				     
            }
            else
            {
                LOG_ERR("copy_from_user failed");
                Ret = -EFAULT;
            }
			break;
		}					
        case ISP_IOC_BUFFER_CTRL:
            Ret = ISP_Buf_CTRL_FUNC(Param);
            break;
        case ISP_IOC_REF_CNT_CTRL:
            Ret = ISP_REF_CNT_CTRL_FUNC(Param);
            break;
		case ISP_IOC_DEBUG_FLAG:
		{
			if(copy_from_user(&DebugFlag, (void*)Param, sizeof(MUINT32)) == 0)
			{
                                spin_lock_irqsave(&(IspInfo.SpinLockIrq), flags);
				IspInfo.DebugMask = DebugFlag;
				spin_unlock_irqrestore(&(IspInfo.SpinLockIrq), flags);
			}
			else
			{
				LOG_ERR("copy_from_user failed");
				Ret = -EFAULT;
			}
			break;
		}
        default:
        {
            LOG_ERR("Unknown Cmd(%d)",Cmd);
            Ret = -EPERM;
            break;
        }
    }
    //
    if(Ret != 0)
    {
        LOG_ERR("Fail, Cmd(%d), Pid(%d), (process, pid, tgid)=(%s, %d, %d)",Cmd, pUserInfo->Pid, current->comm , current->pid, current->tgid);
    }
    //
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_open(
    struct inode*   pInode,
    struct file*    pFile)
{
    MINT32 Ret = 0;
    MUINT32 i;
    ISP_USER_INFO_STRUCT* pUserInfo;
	
    LOG_DBG("- E. UserCount: %d.", IspInfo.UserCount);
    //
    spin_lock(&(IspInfo.SpinLockIspRef));
    //
    pFile->private_data = NULL;
    pFile->private_data = kmalloc(sizeof(ISP_USER_INFO_STRUCT) , GFP_ATOMIC);
    if(pFile->private_data == NULL)
    {
        LOG_DBG("ERROR: kmalloc failed, (process, pid, tgid)=(%s, %d, %d)", current->comm, current->pid, current->tgid);
        Ret = -ENOMEM;
    }
    else
    {
        pUserInfo = (ISP_USER_INFO_STRUCT*)pFile->private_data;
        pUserInfo->Pid = current->pid;
        pUserInfo->Tid = current->tgid;
    }
    //
    if(IspInfo.UserCount > 0)
    {
        IspInfo.UserCount++;
        LOG_DBG("Curr UserCount(%d), (process, pid, tgid)=(%s, %d, %d), users exist",IspInfo.UserCount,current->comm, current->pid, current->tgid);
        goto EXIT;
    }
    //
    IspInfo.BufInfo.Read.pData = (MUINT8 *) kmalloc(ISP_BUF_SIZE, GFP_ATOMIC);
    IspInfo.BufInfo.Read.Size = ISP_BUF_SIZE;
    IspInfo.BufInfo.Read.Status = ISP_BUF_STATUS_EMPTY;
    if(IspInfo.BufInfo.Read.pData == NULL)
    {
        LOG_DBG("ERROR: BufRead kmalloc failed");
        Ret = -ENOMEM;
        goto EXIT;
    }
    //
    if(!ISP_BufWrite_Alloc())
    {
        LOG_DBG("ERROR: BufWrite kmalloc failed");
        Ret = -ENOMEM;
        goto EXIT;
    }
    //
    atomic_set(&(IspInfo.HoldInfo.HoldEnable), 0);
    atomic_set(&(IspInfo.HoldInfo.WriteEnable), 0);
    for (i=0;i<ISP_REF_CNT_ID_MAX;i++) 
	{
        atomic_set(&g_ref_cnt[i],0);    
    }
    // Enable clock
    ISP_EnableClock(MTRUE);
    //    
	IspInfo.IrqInfo.Status = 0;		
    for(i=0; i<ISP_CALLBACK_AMOUNT; i++)
    {
        IspInfo.Callback[i].Func = NULL;
    }
    //
    IspInfo.UserCount++;
    LOG_DBG("Curr UserCount(%d), (process, pid, tgid)=(%s, %d, %d), first user",IspInfo.UserCount,current->comm, current->pid, current->tgid);
    //
	IspInfo.DebugMask = ISP_DBG_RT_BUF_CTRL|ISP_DBG_INT;
    //
    EXIT:
    if(Ret < 0)
    {
    	//
        if(IspInfo.BufInfo.Read.pData != NULL)
        {
            kfree(IspInfo.BufInfo.Read.pData);
            IspInfo.BufInfo.Read.pData = NULL;
			IspInfo.BufInfo.Read.Size = 0;
	        IspInfo.BufInfo.Read.Status = ISP_BUF_STATUS_EMPTY;
        }
        //
        ISP_BufWrite_Free();
    }
    //
    spin_unlock(&(IspInfo.SpinLockIspRef));
    //

	spm_disable_sodi();
	
    LOG_DBG("- X. Ret: %d. UserCount: %d.", Ret, IspInfo.UserCount);
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_release(
    struct inode*   pInode,
    struct file*    pFile)
{
    ISP_USER_INFO_STRUCT* pUserInfo;
    LOG_DBG("- E. UserCount: %d.", IspInfo.UserCount);
    //
    spin_lock(&(IspInfo.SpinLockIspRef));
    //
    //LOG_DBG("UserCount(%d)",IspInfo.UserCount);
    //
    if(pFile->private_data != NULL)
    {
        pUserInfo = (ISP_USER_INFO_STRUCT*)pFile->private_data;
        kfree(pFile->private_data);
        pFile->private_data = NULL;
    }
    //
    IspInfo.UserCount--;
    if(IspInfo.UserCount > 0)
    {
        LOG_DBG("Curr UserCount(%d), (process, pid, tgid)=(%s, %d, %d), users exist",IspInfo.UserCount,current->comm, current->pid, current->tgid);
        goto EXIT;
    }
    //
    LOG_DBG("Curr UserCount(%d), (process, pid, tgid)=(%s, %d, %d), last user",IspInfo.UserCount,current->comm, current->pid, current->tgid);
    //
    ISP_EnableClock(MFALSE);
    //
    if(IspInfo.BufInfo.Read.pData != NULL)
    {
        kfree(IspInfo.BufInfo.Read.pData);
        IspInfo.BufInfo.Read.pData = NULL;
        IspInfo.BufInfo.Read.Size = 0;
        IspInfo.BufInfo.Read.Status = ISP_BUF_STATUS_EMPTY;
    }
    //
    ISP_BufWrite_Free();
    //
    EXIT:
    spin_unlock(&(IspInfo.SpinLockIspRef));
    //

	spm_enable_sodi();
	
    LOG_DBG("- X. UserCount: %d.", IspInfo.UserCount);
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
// helper function, mmap's the kmalloc'd area which is physically contiguous
static MINT32 mmap_kmem(struct file *filp, struct vm_area_struct *vma)
{
        int ret;
        long length = vma->vm_end - vma->vm_start;

        /* check length - do not allow larger mappings than the number of
           pages allocated */
        if (length > ISP_RT_BUF_TBL_NPAGES * PAGE_SIZE)
                return -EIO;

        /* map the whole physically contiguous area in one piece */
		LOG_INF("Vma->vm_pgoff(0x%x),Vma->vm_start(0x%x),Vma->vm_end(0x%x),length(0x%x)",\
			vma->vm_pgoff,vma->vm_start,vma->vm_end,length);
		if(length>MAP_ISP_RTBUF_REG_RANGE)
		{
			LOG_ERR("mmap range error! : length(0x%x),MAP_ISP_RTBUF_REG_RANGE(0x%x)!",length,MAP_ISP_RTBUF_REG_RANGE);
			return -EAGAIN;
		}

        if ((ret = remap_pfn_range(vma,
                                   vma->vm_start,
                                   virt_to_phys((void *)pTbl_RTBuf) >> PAGE_SHIFT,
                                   length,
                                   vma->vm_page_prot)) < 0) {
                return ret;
        }
        return 0;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_mmap(
    struct file*            pFile,
    struct vm_area_struct*  pVma)
{
    LOG_DBG("- E.");
    long length = pVma->vm_end - pVma->vm_start;
    /* at offset ISP_RT_BUF_TBL_NPAGES we map the kmalloc'd area */
    if (pVma->vm_pgoff == ISP_RT_BUF_TBL_NPAGES) 
    {
        return mmap_kmem(pFile, pVma);
    }
    else 
    {
        LOG_INF("pVma->vm_pgoff(0x%x),phy(0x%x),pVmapVma->vm_start(0x%x),pVma->vm_end(0x%x),length(0x%x)",\
            pVma->vm_pgoff,pVma->vm_pgoff<<PAGE_SHIFT,pVma->vm_start,pVma->vm_end,length);
        MUINT32 pfn=pVma->vm_pgoff<<PAGE_SHIFT;//page from number, physical address of kernel memory
        switch(pfn)
        {
            case MAP_ISP_BASE_HW:
                if(length>MAP_ISP_BASE_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_ISP_BASE_RANGE(0x%x)!",length,MAP_ISP_BASE_RANGE);
                    return -EAGAIN;
                }
                break;

            case MAP_SENINF_BASE_ADDR:
                if(length>MAP_SENINF_REG_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_SENINF_REG_RANGE(0x%x)!",length,MAP_SENINF_REG_RANGE);
                    return -EAGAIN;
                }
                break;

            case MAP_CAM_MMSYS_CONFIG_BASE:
                if(length>MAP_CAM_MMSYS_CONFIG_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_CAM_MMSYS_CONFIG_RANGE(0x%x)!",length,MAP_CAM_MMSYS_CONFIG_RANGE);
                    return -EAGAIN;
                }
                break;

            case MAP_CAM_MIPI_CONFIG_BASE:
                if(length>MAP_CAM_MIPI_CONFIG_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_CAM_MIPI_CONFIG_RANGE(0x%x)!",length,MAP_CAM_MIPI_CONFIG_RANGE);
                    return -EAGAIN;
                }
                break;
            case MAP_CAM_MIPI_RX_CONFIG_BASE:
                if(length>MAP_CAM_MIPI_RX_CONFIG_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_CAM_MIPI_RX_CONFIG_RANGE(0x%x)!",length,MAP_CAM_MIPI_RX_CONFIG_RANGE);
                    return -EAGAIN;
                }
                break;
            case MAP_CAM_GPIO_CFG_BASE:
                if(length>MAP_CAM_GPIO_CFG_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_CAM_GPIO_CFG_RANGE(0x%x)!",length,MAP_CAM_GPIO_CFG_RANGE);
                    return -EAGAIN;
                }
                break;
            case MAP_CAM_EFUSE_BASE:
                if(length>MAP_CAM_EFUSE_RANGE)
                {
                    LOG_ERR("mmap range error : length(0x%x),MAP_CAM_EFUSE_RANGE(0x%x)!",length,MAP_CAM_EFUSE_RANGE);
                    return -EAGAIN;
                }
                break;
            default:
                LOG_ERR("Illegal starting HW addr for mmap!");
                return -EAGAIN;
                break;
        }
        pVma->vm_page_prot = pgprot_noncached(pVma->vm_page_prot);
        if(remap_pfn_range(pVma, pVma->vm_start, pVma->vm_pgoff,pVma->vm_end - pVma->vm_start, pVma->vm_page_prot))
        {
            return -EAGAIN;
        }
    }
    //
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
static dev_t IspDevNo;
static struct cdev *pIspCharDrv = NULL;
static struct class *pIspClass = NULL;

static const struct file_operations IspFileOper =
{
    .owner   = THIS_MODULE,
    .open    = ISP_open,
    .release = ISP_release,
    //.flush   = mt_isp_flush,
    .mmap    = ISP_mmap,
    .unlocked_ioctl   = ISP_ioctl
};
/*******************************************************************************
*
********************************************************************************/
inline static void ISP_UnregCharDev(void)
{
    LOG_DBG("- E.");
    //
    //Release char driver
    if(pIspCharDrv != NULL)
    {
        cdev_del(pIspCharDrv);
        pIspCharDrv = NULL;
    }
    //
    unregister_chrdev_region(IspDevNo, 1);
}
/*******************************************************************************
*
********************************************************************************/
inline static MINT32 ISP_RegCharDev(void)
{
    MINT32 Ret = 0;
    //
    LOG_DBG("- E.");
    //
    if((Ret = alloc_chrdev_region(&IspDevNo, 0, 1, ISP_DEV_NAME)) < 0)
    {
        LOG_ERR("alloc_chrdev_region failed, %d", Ret);
        return Ret;
    }
    //Allocate driver
    pIspCharDrv = cdev_alloc();
    if(pIspCharDrv == NULL)
    {
        LOG_ERR("cdev_alloc failed");
        Ret = -ENOMEM;
        goto EXIT;
    }
    //Attatch file operation.
    cdev_init(pIspCharDrv, &IspFileOper);
    //
    pIspCharDrv->owner = THIS_MODULE;
    //Add to system
    if((Ret = cdev_add(pIspCharDrv, IspDevNo, 1)) < 0)
    {
        LOG_ERR("Attatch file operation failed, %d", Ret);
        goto EXIT;
    }
    //
    EXIT:
    if(Ret < 0)
    {
        ISP_UnregCharDev();
    }
    //

    LOG_DBG("- X.");
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_probe(struct platform_device* pDev)
{
    MINT32 Ret = 0;
    struct resource *pRes = NULL;
    MINT32 i;
    //
    LOG_DBG("- E.");
    // Check platform_device parameters
    if(pDev == NULL)
    {
        dev_err(&pDev->dev, "pDev is NULL");
        return -ENXIO;
    }
    // Register char driver
    if((Ret = ISP_RegCharDev()))
    {
        dev_err(&pDev->dev, "register char failed");
        return Ret;
    }
    // Mapping CAM_REGISTERS
    for(i = 0; i < 1; i++)  // NEED_TUNING_BY_CHIP. 1: Only one IORESOURCE_MEM type resource in kernel\mt_devs.c\mt_resource_isp[].
    {
        LOG_DBG("Mapping CAM_REGISTERS. i: %d.", i);
        pRes = platform_get_resource(pDev, IORESOURCE_MEM, i);
        if(pRes == NULL)
        {
            dev_err(&pDev->dev, "platform_get_resource failed");
            Ret = -ENOMEM;
            goto EXIT;
        }
        pRes = request_mem_region(pRes->start, pRes->end - pRes->start + 1, pDev->name);
        if(pRes == NULL)
        {
            dev_err(&pDev->dev, "request_mem_region failed");
            Ret = -ENOMEM;
            goto EXIT;
        }
    }
    // Create class register
    pIspClass = class_create(THIS_MODULE, "ispdrv");
    if(IS_ERR(pIspClass))
    {
        Ret = PTR_ERR(pIspClass);
        LOG_ERR("Unable to create class, err = %d", Ret);
        return Ret;
    }
    // FIXME: error handling
    device_create(pIspClass, NULL, IspDevNo, NULL, ISP_DEV_NAME);
    //
    init_waitqueue_head(&IspInfo.WaitQueueHead);
    //
    INIT_WORK(&IspInfo.ScheduleWorkVD,       ISP_ScheduleWork_VD);
    INIT_WORK(&IspInfo.ScheduleWorkEXPDONE,  ISP_ScheduleWork_EXPDONE);
    //
    spin_lock_init(&(IspInfo.SpinLockIspRef));
    spin_lock_init(&(IspInfo.SpinLockIsp));
    spin_lock_init(&(IspInfo.SpinLockIrq));
    spin_lock_init(&(IspInfo.SpinLockHold));
    spin_lock_init(&(IspInfo.SpinLockRTBC));
    //
    IspInfo.UserCount = 0;
    IspInfo.HoldInfo.Time = ISP_HOLD_TIME_EXPDONE;
    //
    IspInfo.IrqInfo.Mask = ISP_REG_MASK_INT_STATUS;    
    //
	IspInfo.IrqInfo.ErrMask = ISP_REG_MASK_INT_STATUS_ERR;
    //

    // Request CAM_ISP IRQ
    // check whether ISP_REG_SW_CTL is default value for isp exist or not
    if(ISP_REG_SW_CTL_IMGO_RST_ST == ISP_RD32(ISP_REG_SW_CTL))	
    {
        //if (request_irq(MT_CAMERA_IRQ_ID, (irq_handler_t)ISP_Irq, IRQF_TRIGGER_LOW, "isp", NULL))
    	if (request_irq(MT_CAMERA_IRQ_ID, (irq_handler_t)ISP_Irq, IRQF_TRIGGER_LOW, "isp", NULL))
        {
            LOG_ERR("MT6572_CAM_IRQ_LINE IRQ LINE NOT AVAILABLE!!\n");
            goto EXIT;
        }
        //mt_irq_unmask(MT_CAMERA_IRQ_ID);
    }

    EXIT:
    if(Ret < 0)
    {
        ISP_UnregCharDev();
    }
    //
    LOG_DBG("- X.");
    //
    return Ret;
}

/*******************************************************************************
* Called when the device is being detached from the driver
********************************************************************************/
static MINT32 ISP_remove(struct platform_device *pDev)
{
    struct resource *pRes;
    MINT32 i;
    MINT32 IrqNum;
    //
    LOG_DBG("- E.");
    // unregister char driver.
    ISP_UnregCharDev();
    // unmaping ISP CAM_REGISTER registers
    for(i = 0; i < 2; i++)
    {
        pRes = platform_get_resource(pDev, IORESOURCE_MEM, 0);
        release_mem_region(pRes->start, (pRes->end - pRes->start + 1));
    }
    // Release IRQ
    disable_irq(IspInfo.IrqNum);
    IrqNum = platform_get_irq(pDev, 0);
    free_irq(IrqNum , NULL);
    //
    device_destroy(pIspClass, IspDevNo);
    //
    class_destroy(pIspClass);
    pIspClass = NULL;
    //
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL bPass1_On_In_Resume_TG1 = MFALSE;
static MINT32 ISP_suspend(
    struct platform_device* pDev,
    pm_message_t            Mesg)
{
    ISP_WAIT_IRQ_STRUCT waitirq;
    MINT32 ret = 0;
    // VFDATA_EN. TG1 Take Picture Request.
    MUINT32 regTG1Val = ISP_RD32(ISP_REG_TG_VF_CON);
    LOG_DBG("bPass1_On_In_Resume_TG1(%d), regTG1Val(0x%08x)", bPass1_On_In_Resume_TG1, regTG1Val);

    bPass1_On_In_Resume_TG1 = MFALSE;
    if ( regTG1Val & 0x01 )    
    {
        bPass1_On_In_Resume_TG1 = MTRUE;
		ISP_WR32(ISP_REG_TG_VF_CON, (regTG1Val&(~0x01)));
		
        //wait p1 done
        waitirq.Clear=ISP_IRQ_CLEAR_WAIT;
        waitirq.Status=ISP_INT_PASS1_TG1_DON;
        waitirq.Timeout=100;
        ret=ISP_WaitIrq(waitirq);
		
    }
   
    return 0;
}
static MINT32 ISP_resume(struct platform_device* pDev)
{
    // VFDATA_EN. TG1 Take Picture Request.
    MUINT32 regTG1Val = ISP_RD32(ISP_REG_TG_VF_CON);
    LOG_DBG("bPass1_On_In_Resume_TG1(%d), regTG1Val(0x%08x)", bPass1_On_In_Resume_TG1, regTG1Val);

    if ( bPass1_On_In_Resume_TG1 ) 
	{
        bPass1_On_In_Resume_TG1 = MFALSE;
        ISP_WR32(ISP_REG_TG_VF_CON, (regTG1Val|0x01) );    
    }
	
    return 0;
}
/*******************************************************************************
* IPO suspend/resume
********************************************************************************/
#ifdef CONFIG_PM
int ISP_pm_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    pr_debug("calling %s()\n", __func__);

    return ISP_suspend(pdev, PMSG_SUSPEND);
}

int ISP_pm_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    pr_debug("calling %s()\n", __func__);

    return ISP_resume(pdev);
}
extern void mt_irq_set_sens(unsigned int irq, unsigned int sens);
extern void mt_irq_set_polarity(unsigned int irq, unsigned int polarity);
int ISP_pm_restore_noirq(struct device *device)
{
    pr_debug("calling %s()\n", __func__);

    mt_irq_set_sens(MT_CAMERA_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt_irq_set_polarity(MT_CAMERA_IRQ_ID, MT65xx_POLARITY_LOW);

    return 0;
}
#else /*CONFIG_PM*/
#define ISP_pm_suspend NULL
#define ISP_pm_resume  NULL
#define ISP_pm_restore_noirq NULL
#endif /*CONFIG_PM*/
/*---------------------------------------------------------------------------*/
struct dev_pm_ops ISP_pm_ops = {
    .suspend = ISP_pm_suspend,
    .resume = ISP_pm_resume,
    .freeze = ISP_pm_suspend,
    .thaw = ISP_pm_resume,
    .poweroff = ISP_pm_suspend,
    .restore = ISP_pm_resume,
    .restore_noirq = ISP_pm_restore_noirq,
};
/*******************************************************************************
*
********************************************************************************/
static struct platform_driver IspDriver =
{
    .probe   = ISP_probe,
    .remove  = ISP_remove,
    .suspend = ISP_suspend,
    .resume  = ISP_resume,
    .driver  = {
        .name  = ISP_DEV_NAME,
        .owner = THIS_MODULE,
		#ifdef CONFIG_PM
		.pm 	= &ISP_pm_ops,
		#endif	        
    }
};
/*******************************************************************************
*
********************************************************************************/
static MINT32 ISP_DumpRegToProc(
    char*   pPage,
    char**  ppStart,
    off_t   off,
    MINT32  Count,
    MINT32* pEof,
    void*   pData)
{
    char *p = pPage;
    MINT32 Length = 0;
    MUINT32 i = 0;
    MINT32 ret = 0;
    //
    LOG_DBG("- E. pPage: 0x%08x. off: %d. Count: %d.", (unsigned int)pPage, (unsigned int)off, Count);
    //
    p += sprintf(p, " MT6572 ISP Register\n");
    p += sprintf(p, "====== cam ======\n");
    for(i = 0x0; i <= 0x084; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", ISP_BASE_ADDR + i, (unsigned int)ISP_RD32(ISP_BASE_ADDR + i));
    }
    p += sprintf(p,"====== dma ======\n");
    for(i = 0x200; i <= 0x22C; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", ISP_BASE_ADDR + i, (unsigned int)ISP_RD32(ISP_BASE_ADDR + i));
    }
    p += sprintf(p,"====== tg ======\n");
    for(i = 0x410; i <= 0x44C; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", ISP_BASE_ADDR + i, (unsigned int)ISP_RD32(ISP_BASE_ADDR + i));
    }
    p += sprintf(p,"====== cdrz ======\n");
    for(i = 0xB00; i <= 0xB38; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", ISP_BASE_ADDR + i, (unsigned int)ISP_RD32(ISP_BASE_ADDR + i));
    }
    p += sprintf(p,"====== seninf ======\n");
    for(i = 0x0; i <= 0x40; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }
    for(i = 0x100; i <= 0x13C; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }
    for(i = 0x200; i <= 0x240; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }
    for(i = 0x300; i <= 0x310; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }
    for(i = 0x400; i <= 0x424; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }
    for(i = 0x600; i <= 0x644; i += 4)
    {
        p += sprintf(p,"+0x%08x 0x%08x\n", SENINF_BASE_ADDR + i, (unsigned int)ISP_RD32(SENINF_BASE_ADDR + i));
    }     
    //
    *ppStart = pPage + off;
    //
    Length = p - pPage;
    if(Length > off)
    {
        Length -= off;
    }
    else
    {
        Length = 0;
    }
    //

    ret = Length < Count ? Length : Count;

    LOG_DBG("- X. ret: %d.", ret);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32  ISP_RegDebug(
    struct file*    pFile,
    const char*     pBuffer,
    unsigned long   Count,
    void*           pData)
{
    char RegBuf[64];
    MUINT32 CopyBufSize = (Count < (sizeof(RegBuf) - 1)) ? (Count) : (sizeof(RegBuf) - 1);
    MUINT32 Addr = 0;
    MUINT32 Data = 0;

    LOG_DBG("- E. pFile: 0x%08x. pBuffer: 0x%08x. Count: %d.", (unsigned int)pFile, (unsigned int)pBuffer, (int)Count);
    //
    if(copy_from_user(RegBuf, pBuffer, CopyBufSize))
    {
        LOG_ERR("copy_from_user() fail.");
        return -EFAULT;
    }

    //
    if (sscanf(RegBuf, "%x %x",  &Addr, &Data) == 2)
    {
        ISP_WR32(Data, ISP_BASE_ADDR + Addr);
        LOG_DBG("Write => Addr: 0x%08X, Write Data: 0x%08X. Read Data: 0x%08X.", ISP_BASE_ADDR + Addr, Data, ioread32(ISP_BASE_ADDR + Addr));
    }
    else if (sscanf(RegBuf, "%x", &Addr) == 1)
    {
        LOG_DBG("Read => Addr: 0x%08X, Read Data: 0x%08X.", ISP_BASE_ADDR + Addr, ioread32(ISP_BASE_ADDR + Addr));
    }
    //
    LOG_DBG("- X. Count: %d.", (int)Count);
    return Count;
}
#if 0
/*******************************************************************************
*
********************************************************************************/
static MINT32 CAMIO_DumpRegToProc(
    char*   pPage,
    char**  ppStart,
    off_t   off,
    MINT32  Count,
    MINT32* pEof,
    void*   pData)
{
    char *p = pPage;
    MINT32 Length = 0;
    MINT32 ret = 0;
    //
    LOG_DBG("- E. pPage: 0x%08x. off: %d. Count: %d.", (unsigned int)pPage, (int)off, Count);
    //
    p += sprintf(p, " MT6572 CAMIO Register");
    p += sprintf(p,"====== CAM MCLK DRIVING SETTING ====");
    p += sprintf(p,"+0x%08x 0x%08x", GPIO_BASE+0x5B0, ioread32(GPIO_BASE + 0x5B0));


    *ppStart = pPage + off;
    //
    Length = p - pPage;
    if(Length > off)
    {
        Length -= off;
    }
    else
    {
        Length = 0;
    }
    //

    ret = Length < Count ? Length : Count;

    LOG_DBG("- X. ret: %d.", ret);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
static MINT32  CAMIO_RegDebug(
    struct file*    pFile,
    const char*     pBuffer,
    unsigned long   Count,
    void*           pData)
{
    char RegBuf[64];
    MUINT32 CopyBufSize = (Count < (sizeof(RegBuf) - 1)) ? (Count) : (sizeof(RegBuf) - 1);
    MUINT32 Addr = 0;
    MUINT32 Data = 0;
    LOG_DBG("- E. pFile: 0x%08x. pBuffer: 0x%08x. Count: %d.", (unsigned int)pFile, (unsigned int)pBuffer, (int)Count);

    //
    if(copy_from_user(RegBuf, pBuffer, CopyBufSize))
    {
        LOG_ERR("copy_from_user() fail.");
        return -EFAULT;
    }

    //
    if (sscanf(RegBuf, "%x %x",  &Addr, &Data) == 2)
    {
        ISP_WR32(Data, GPIO_BASE + Addr);
        LOG_DBG("Write => Addr: 0x%08X, Write Data: 0x%08X. Read Data: 0x%08X.", GPIO_BASE + Addr, Data, ioread32(GPIO_BASE + Addr));
    }
    else if (sscanf(RegBuf, "%x", &Addr) == 1)
    {
        LOG_DBG("Read => Addr: 0x%08X, Read Data: 0x%08X.", GPIO_BASE + Addr, ioread32(GPIO_BASE + Addr));
    }
    //
    LOG_DBG("- X. Count: %d.", (int)Count);
    return Count;
}
#endif

#if defined(__ISP_PROC_TEST_ENABLE__)
/*******************************************************************************
*
********************************************************************************/
static ISP_RT_BUF_STRUCT rt_buf1, rt_buf2, rt_buf3;
static ISP_RT_BUF_STRUCT rt_buf_d1, rt_buf_d2;
char* q_printf(char *p)
{
	int i;
	p += sprintf(p,"\n&& queue total:%d, start:%d\n", pstRTBuf->ring_buf.total_count,
												pstRTBuf->ring_buf.start);
	for(i = 0 ; i < pstRTBuf->ring_buf.total_count ; i++)
	{
		p += sprintf(p,"[%d]-pa(0x%08x),filled(%d)\n", i, pstRTBuf->ring_buf.data[i].base_pAddr
													  ,	pstRTBuf->ring_buf.data[i].bFilled);
	}
	p += sprintf(p,"\n");

	return p;
}
static MINT32 ISP_RTBCTestToProc(
    char*   pPage,
    char**  ppStart,
    off_t   off,
    MINT32  Count,
    MINT32* pEof,
    void*   pData)
{
    char *p = pPage;
    MINT32 Length = 0;
    MINT32 ret = 0;
    //
    LOG_DBG("- E. pPage: 0x%08x. off: %d. Count: %d.", (unsigned int)pPage, (int)off, Count);
    //
    p += sprintf(p," MT6572 ISP RTBC test\n");
	//
	rtbc_enq_dma = rtbc_deq_dma = 1;

	rt_buf1.memID = 0;
	rt_buf1.size = 100;
	rt_buf1.base_vAddr = 0xFFFF0000;	
	rt_buf1.base_pAddr = 0x000A0000;
	rt_buf2.memID = 1;
	rt_buf2.size = 100;
	rt_buf2.base_vAddr = 0xFFFF1000;	
	rt_buf2.base_pAddr = 0x000A1000;
	rt_buf3.memID = 2;
	rt_buf3.size = 100;
	rt_buf3.base_vAddr = 0xFFFF2000;	
	rt_buf3.base_pAddr = 0x000A2000;

	rt_buf = rt_buf1;
	ISP_RTBC_ENQUE(rtbc_enq_dma);
	p += sprintf(p,"enque 1 buff, pa(0x%08x)\n", rt_buf.base_pAddr);

	rt_buf = rt_buf2;
	ISP_RTBC_ENQUE(rtbc_enq_dma);
	p += sprintf(p,"enque 1 buff, pa(0x%08x)\n", rt_buf.base_pAddr);

	rt_buf = rt_buf3;
	ISP_RTBC_ENQUE(rtbc_enq_dma);
	p += sprintf(p,"enque 1 buff, pa(0x%08x)\n", rt_buf.base_pAddr);

	p = q_printf(p); 	// list queue

	pstRTBuf->ring_buf.data[pstRTBuf->ring_buf.start].bFilled = ISP_RT_BUF_FILLED;		
	pstRTBuf->ring_buf.empty_count--;
	
	ISP_RTBC_DEQUE(rtbc_deq_dma);
	p += sprintf(p,"deque %d buff, pa(0x%08x)\n", deque_buf.count, deque_buf.data[0].base_pAddr);
	if(deque_buf.count)
	{
		pstRTBuf->ring_buf.start = (pstRTBuf->ring_buf.start+1)%pstRTBuf->ring_buf.total_count;
		rt_buf_d1 = deque_buf.data[0];		
	}
		
	p = q_printf(p);	// list queue

	pstRTBuf->ring_buf.data[pstRTBuf->ring_buf.start].bFilled = ISP_RT_BUF_FILLED;		
	pstRTBuf->ring_buf.empty_count--;
	
	ISP_RTBC_DEQUE(rtbc_deq_dma);
	p += sprintf(p,"deque %d buff, pa(0x%08x)\n", deque_buf.count, deque_buf.data[0].base_pAddr);
	if(deque_buf.count)
	{
		pstRTBuf->ring_buf.start = (pstRTBuf->ring_buf.start+1)%pstRTBuf->ring_buf.total_count;
		rt_buf_d2 = deque_buf.data[0];
	}
			
	p = q_printf(p);	// list queue

	rt_buf = rt_buf_d1;		
	ISP_RTBC_ENQUE(rtbc_enq_dma);
	p += sprintf(p,"enque 1 buff, pa(0x%08x)\n", rt_buf.base_pAddr);

	p = q_printf(p);	// list queue

	rt_buf = rt_buf_d2;		
	ISP_RTBC_ENQUE(rtbc_enq_dma);
	p += sprintf(p,"enque 1 buff, pa(0x%08x)\n", rt_buf.base_pAddr);

	p = q_printf(p);	// list queue


	//
    *ppStart = pPage + off;
    //
    Length = p - pPage;
    if(Length > off)
    {
        Length -= off;
    }
    else
    {
        Length = 0;
    }
    //

    ret = Length < Count ? Length : Count;

    LOG_DBG("- X. ret: %d.", ret);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
unsigned int CEILING(unsigned int A, unsigned int B)
{
    if(B == 0)
        return A;
    if(A%B == 0)
        return A/B;
    else
        return A/B + 1;
}
unsigned int ROUND(unsigned int A, unsigned int B)
{
    if(B == 0)
        return A;
    else
        return (2*A+B)/(2*B);
}
char* fpga_write_reg(char* p, char* regname, unsigned int offset, unsigned int value)
{
    //if(preg == NULL)
    //    return;
    unsigned int data;
	
	data = ISP_RD32(ISP_BASE_ADDR+offset);
	ISP_WR32(ISP_BASE_ADDR+offset, value);

    if(regname[0] != '~')
		p += sprintf(p,"%30s, RegBase[0x%04X]: 0x%08X->0x%08X \n", regname, offset, data, ISP_RD32(ISP_BASE_ADDR+offset));
	
	return p;
}

//Mark by hungwen, because of build warning!!
#if 0
void fpga_reset_cam_tg()
{
	char *p;
    fpga_write_reg(p, "~CAM_MODULE_EN",         0x0000, 0x00000008);
    fpga_write_reg(p, "~CAM_FMT_SEL",           0x0004, 0x01000000);    
    fpga_write_reg(p, "~CAM_INT_EN",            0x0010, 0x00000000);
	fpga_write_reg(p, "~CAM_INT_STATUS",		0x0014, 0xFFFFFFFF);	// write clear
    //preg[0x0014>>2] = preg[0x0014>>2];// read clear
    
    fpga_write_reg(p, "~CAM_CRDZ_CONTROL",      0x0B00, 0x00000000);    
    fpga_write_reg(p, "~CAM_CRDZ_INPUT_IMAGE",  0x0B04, 0x00000000);
    fpga_write_reg(p, "~CAM_CRDZ_OUTPUT_IMAGE", 0x0B08, 0x00000000);    
    fpga_write_reg(p, "~CAM_CRDZ_HORIZONTAL_COEFF_STEP",    0x0B0C, 0x00000000);        
    fpga_write_reg(p, "~CAM_CRDZ_VERTICAL_COEFF_STEP",      0x0B10, 0x00000000);     

    fpga_write_reg(p, "~CAM_IMGO_BASE_ADDR",    0x0204, 0x00000000);     
    fpga_write_reg(p, "~CAM_IMGO_XSIZE",        0x020C, 0x00000000);     
    fpga_write_reg(p, "~CAM_IMGO_YSIZE",        0x0210, 0x00000000);       
    fpga_write_reg(p, "~CAM_IMGO_STRIDE",       0x0214, 0x00000000);       
            
    fpga_write_reg(p, "~CAM_TG_SEN_GRAB_PXL",   0x0418, 0x7FFF0000);
    fpga_write_reg(p, "~CAM_TG_SEN_GRAB_LIN",   0x041C, 0x1FFF0000);
    fpga_write_reg(p, "~CAM_TG_SEN_MODE",       0x0410, 0x00000000);       
    fpga_write_reg(p, "~CAM_TG_VF_CON",         0x0414, 0x00000000);          

    fpga_write_reg(p, "~SENINF1_CTRL",          0x1010, 0x06DF0080);
    fpga_write_reg(p, "~SENINF1_SIZE",          0x101C, 0x00000000);    
    
    fpga_write_reg(p, "~SENINF_TG1_TM_CTL",     0x1308, 0x00300004);
    fpga_write_reg(p, "~SENINF_TG1_TM_SIZE",    0x130C, 0x00000000);
}
#endif

//#include <asm/io.h>
static MINT32 ISP_Pass1TestToProc(
    char*   pPage,
    char**  ppStart,
    off_t   off,
    MINT32  Count,
    MINT32* pEof,
    void*   pData)
{
    char *p = pPage;
    MINT32 Length = 0;
    MINT32 ret = 0;
	MUINT32 i = 0;

    unsigned long* imgoAddr;
    MUINT32 imgiLog, imgiPhy, imgiSize;
    //MINT32   imgiMemId;
    int flag, InW, InH, OutW, OutH;
    int testW, testH;
    int grabW, grabH, grabX, grabY;
    int cdrzInW, cdrzInH;
    int cdrzOutW, cdrzOutH;    
    int coefW, coefH;           // for cdrz
    int testdummy, testpattern; // for sen tg test
    int ckimgW, ckimgH;         // for sen tg check    

	//int mispfd, msensorfd;
	
    //
    LOG_DBG("- E. pPage: 0x%08x. off: %d. Count: %d.", (unsigned int)pPage, (int)off, Count);
    //
    p += sprintf(p," MT6572 ISP Pass1 test\n");
	//

/*
	printk("11111\n");
	msensorfd = open("/dev/kd_camera_hw", O_RDWR);
    if (msensorfd < 0) {
        LOG_DBG("[init]: error opening %s: \n", "/dev/kd_camera_hw");
        return -1;
    }
	printk("22222\n");
	mispfd = open(ISP_DEV_NAME, O_RDWR);
	if (mispfd < 0) {
		LOG_DBG("open(ISP_DEV_NAME, O_RDWR) FAILED!!!\n");
		return -1;
	}
	printk("33333\n");
*/	

	InW = 1280;
	InH = 960;
	OutW = 320;
	OutH = 240;
	flag = 0;
	grabW = testW = InW*2;
	grabH = testH = InH;
	grabX = 0;
	grabY = 0;
	cdrzInW = InW;
	cdrzInH = InH;
	cdrzOutW = OutW;
	cdrzOutH = OutH;        
	coefW = CEILING( (cdrzOutW-1)*1048576, (cdrzInW-1) ); // CEILING(a,b) => CEILING(a/b)
    coefH = ROUND( (cdrzInH-1)*32768, (cdrzOutH-1) );     // ROUND(a,b) => ROUND(a/b)
	testpattern = 12;
	testdummy = 0xA;
	ckimgW = 0xAA;
    ckimgH = 0xB;

	imgiSize = cdrzOutW*2*cdrzOutH;   
    //imgiLog = (MUINT32) pmem_alloc_sync(imgiSize, &imgiMemId);
    //imgiPhy = (MUINT32) pmem_get_phys(imgiMemId);
    imgiLog = (MUINT32)kmalloc(imgiSize, __GFP_DMA);
	imgiPhy = virt_to_phys((void *)imgiLog); 	//imgiLog-0xC0000000;
    imgoAddr = (unsigned long*)imgiLog;    
    for(i = 0 ; i < imgiSize>>2 ; i++)
        imgoAddr[i] = 0xF0F0F0F0;    
	
	p += sprintf(p,"test(%d,%d) => grab(%d,%d) => cdrz(%d,%d) \n", 
												testW/2, testH, grabW/2, grabH, cdrzOutW, cdrzOutH);
    p += sprintf(p,"cdrz coefW=0x%08X, coefH=0x%08X \n", coefW, coefH);
	p += sprintf(p,"pmem allocation: size=0x%X, imgiLog= 0x%08X , imgiPhy= 0x%08X \n", imgiSize, imgiLog, imgiPhy);
	p += sprintf(p,"imgoAddr = 0x%08X \n", (MUINT32)imgoAddr);

	// start config isp/seninf
    //Mark by hung-wen
    //fpga_reset_cam_tg();

//
	p = fpga_write_reg(p, "SENINF1_CTRL",          0x1010, 0x86DF3180);    // enable seninf mux and
    p = fpga_write_reg(p, "SENINF_TG1_PH_CNT",     0x1300, 0x00000010);    // pwn 0 & rst 1 for OV5642
	
//	p = fpga_write_reg(p, "SENINF1_CTRL",			0x1010, 0x86DF1080);	// enable seninf mux and set src as testpattern
//	p = fpga_write_reg(p, "SENINF1_SIZE",			0x101C, (ckimgH<<16) + ckimgW);   // for check only 

	p = fpga_write_reg(p, "CAM_TG_SEN_MODE",		0x0410, 0x3);		
	p = fpga_write_reg(p, "CAM_TG_VF_CON", 			0x0414, 0x1);			// single:3, conti:1	
	p = fpga_write_reg(p, "CAM_TG_SEN_GRAB_PXL",	0x0418, ((grabX+grabW)<<16)+grabX);
	p = fpga_write_reg(p, "CAM_TG_SEN_GRAB_LIN",	0x041C, ((grabY+grabH)<<16)+grabY);

	p = fpga_write_reg(p, "CAM_MODULE_EN", 			0x0000, 0xF);
	p = fpga_write_reg(p, "CAM_FMT_SEL",			0x0004, 0x01010003);	// UYVY422 format
	p = fpga_write_reg(p, "CAM_INT_EN",				0x0010, 0x80000401);	// 0x8...: write clear
	
	if(flag == 1)	// bypass CRDZ
		p = fpga_write_reg(p, "CAM_CRDZ_CONTROL",	0x0B00, 0x0);
	else			// enable CRDZ, using horizontal algo. = 2
		p = fpga_write_reg(p, "CAM_CRDZ_CONTROL",	0x0B00, 0x43);	  
	p = fpga_write_reg(p, "CAM_CRDZ_INPUT_IMAGE",	0x0B04, (cdrzInH<<16) + cdrzInW);
	p = fpga_write_reg(p, "CAM_CRDZ_OUTPUT_IMAGE", 	0x0B08, (cdrzOutH<<16) + cdrzOutW); 	   
	p = fpga_write_reg(p, "CAM_CRDZ_HORIZONTAL_COEFF_STEP",		0x0B0C, coefW); 	   
	p = fpga_write_reg(p, "CAM_CRDZ_VERTICAL_COEFF_STEP",		0x0B10, coefH);   
	 
	p = fpga_write_reg(p, "CAM_IMGO_BASE_ADDR",		0x0204, (unsigned int)imgiPhy);   
	p = fpga_write_reg(p, "CAM_IMGO_XSIZE",			0x020C, cdrzOutW*2-1);	   
	p = fpga_write_reg(p, "CAM_IMGO_YSIZE",			0x0210, cdrzOutH-1);	   
	p = fpga_write_reg(p, "CAM_IMGO_STRIDE",		0x0214, cdrzOutW*2);	   
			 
	//p = fpga_write_reg(p, "SENINF_TG1_TM_CTL", 		0x1308, 0x00300006);	// reset tg
	//p = fpga_write_reg(p, "SENINF_TG1_TM_CTL", 		0x1308, 0x00300004);	// reset tg  
	//p = fpga_write_reg(p, "SENINF_TG1_TM_CTL", 		0x1308, (testdummy<<16) + (testpattern<<4) + 4);	// set dummy and pattern
	//p = fpga_write_reg(p, "SENINF_TG1_TM_SIZE",		0x130C, (testH<<16) + testW);	
	
	p += sprintf(p,"~~~ Check INT status before test: 0x%08X \n", ISP_RD32(ISP_BASE_ADDR+0x0014));		 
	
	//p = fpga_write_reg(p, "SENINF_TG1_TM_CTL", 		0x1308, (testdummy<<16) + (testpattern<<4) + 5);	// start test pattern

	i = 0;
    while( ((ISP_RD32(ISP_BASE_ADDR+0x0014))&0x00000400) == 0 )     // check vsync
    {
        i++;
		if(i == 100000)
			break;
    } 
	p += sprintf(p,"~~~ Check INT status after test: 0x%08X \n", ISP_RD32(ISP_BASE_ADDR+0x0014));		 
	p += sprintf(p,"Result: i = %d\n",i);

	
	//
	*ppStart = pPage + off;
	//
	Length = p - pPage;
	if(Length > off)
	{
		Length -= off;
	}
	else
	{
		Length = 0;
	}
	//

	ret = Length < Count ? Length : Count;

	LOG_DBG("- X. ret: %d.", ret);

	return ret;
}
/*******************************************************************************
*
********************************************************************************/
#if 0
static MINT32 ISP_TestToProc(
    char*   pPage,
    char**  ppStart,
    off_t   off,
    MINT32  Count,
    MINT32* pEof,
    void*   pData)
{
    char *p = pPage;
    MINT32 Length = 0;
    MINT32 ret = 0;
	MUINT32 i = 0;
    //
    LOG_DBG("- E. pPage: 0x%08x. off: %d. Count: %d.", (unsigned int)pPage, (int)off, Count);
    //
    p += sprintf(p," MT6572 ISP test\n");
	//
	
	// Test here
	
	//
	*ppStart = pPage + off;
	//
	Length = p - pPage;
	if(Length > off)
	{
		Length -= off;
	}
	else
	{
		Length = 0;
	}
	//

	ret = Length < Count ? Length : Count;

	LOG_DBG("- X. ret: %d.", ret);

	return ret;
}
#endif
#endif //__ISP_PROC_TEST_ENABLE__
/*******************************************************************************
*
********************************************************************************/
static MINT32 __init ISP_Init(void)
{
    MINT32 Ret = 0;
    struct proc_dir_entry*  pEntry;
    int i;
    //
    LOG_DBG("- E.");
    //
    if((Ret = platform_driver_register(&IspDriver)) < 0)
    {
        LOG_ERR("platform_driver_register fail");
        return Ret;
    }
    //
    pEntry = create_proc_entry("driver/isp_reg", 0, NULL);
    if(pEntry)
    {
        pEntry->read_proc = ISP_DumpRegToProc;
        pEntry->write_proc = ISP_RegDebug;
    }
    else
    {
        LOG_ERR("add /proc/driver/isp_reg entry fail");
    }
	#if 0 // Jason test
    pEntry = create_proc_entry("driver/camio_reg", 0, NULL);
    if(pEntry)
    {
        pEntry->read_proc = CAMIO_DumpRegToProc;
        pEntry->write_proc = CAMIO_RegDebug;
    }
    else
    {
        LOG_ERR("add /proc/driver/camio_reg entry fail");
    }
	#endif
	#if defined(__ISP_PROC_TEST_ENABLE__)// Jason test
	pEntry = create_proc_entry("driver/isp_rtbc_test", 0, NULL);
    if(pEntry)
    {
        pEntry->read_proc = ISP_RTBCTestToProc;
        pEntry->write_proc = NULL;
    }
    else
    {
        LOG_ERR("add /proc/driver/isp_rtbc_test entry fail");
    }
	pEntry = create_proc_entry("driver/isp_pass1_test", 0, NULL);
    if(pEntry)
    {
        pEntry->read_proc = ISP_Pass1TestToProc;
        pEntry->write_proc = NULL;
    }
    else
    {
        LOG_ERR("add /proc/driver/isp_pass1_test entry fail");
    }			
	#endif	//__ISP_PROC_TEST_ENABLE__
    //
    /* allocate a memory area with kmalloc. Will be rounded up to a page boundary */
    //ISP_RT_BUF_TBL_NPAGES*4096(1page) = 64k Bytes
    if ((pBuf_kmalloc = kmalloc((ISP_RT_BUF_TBL_NPAGES + 2) * PAGE_SIZE, GFP_KERNEL)) == NULL) 
	{
            return -ENOMEM;
    }
    memset(pBuf_kmalloc,0x00,ISP_RT_BUF_TBL_NPAGES*PAGE_SIZE);
	//
    /* round it up to the page bondary */
    pTbl_RTBuf = (int *)((((unsigned long)pBuf_kmalloc) + PAGE_SIZE - 1) & PAGE_MASK);
    pstRTBuf = (ISP_RT_BUF_INFO_STRUCT*)pTbl_RTBuf;
    pstRTBuf->state = ISP_RT_BUF_INFO_INIT;
	//
	/* mark the pages reserved */
    for (i = 0; i < ISP_RT_BUF_TBL_NPAGES * PAGE_SIZE; i+= PAGE_SIZE) 
	{
        SetPageReserved(virt_to_page(((unsigned long)pTbl_RTBuf) + i));
    }
    //
    LOG_DBG("- X. Ret: %d.", Ret);
    return Ret;
}
/*******************************************************************************
*
********************************************************************************/
static void __exit ISP_Exit(void)
{
    int i;
    LOG_DBG("- E.");
    //
    platform_driver_unregister(&IspDriver);

    //
    /* unreserve the pages */
    for (i = 0; i < ISP_RT_BUF_TBL_NPAGES * PAGE_SIZE; i+= PAGE_SIZE) {
            SetPageReserved(virt_to_page(((unsigned long)pTbl_RTBuf) + i));
    }
    /* free the memory areas */
    kfree(pBuf_kmalloc);
    //
}
/*******************************************************************************
*
********************************************************************************/
MBOOL ISP_RegCallback(ISP_CALLBACK_STRUCT* pCallback)
{
    //
    if(pCallback == NULL)
    {
        LOG_ERR("pCallback is null");
        return MFALSE;
    }
    //
    if(pCallback->Func == NULL)
    {
        LOG_ERR("Func is null");
        return MFALSE;
    }
    //
    LOG_DBG("Type(%d)",pCallback->Type);
    IspInfo.Callback[pCallback->Type].Func = pCallback->Func;
    //
    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL ISP_UnregCallback(ISP_CALLBACK_ENUM Type)
{
    if(Type > ISP_CALLBACK_AMOUNT)
    {
        LOG_ERR("Type(%d) must smaller than %d",Type,ISP_CALLBACK_AMOUNT);
        return MFALSE;
    }
    //
    LOG_DBG("Type(%d)",Type);
    IspInfo.Callback[Type].Func = NULL;
    //
    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
void ISP_MCLK1_EN(MBOOL En)
{
    MUINT32 temp=0;
    temp = ISP_RD32(SENINF_BASE + 0x300);
    if(En)
    {
        temp |= 0x20000000;
        ISP_WR32(SENINF_BASE + 0x300,temp);
    }
    else
    {
        temp &= 0xDFFFFFFF;
        ISP_WR32(SENINF_BASE + 0x300,temp);
    }

}
/*******************************************************************************
*
********************************************************************************/
module_init(ISP_Init);
module_exit(ISP_Exit);
MODULE_DESCRIPTION("Camera ISP driver");
MODULE_AUTHOR("ME");
MODULE_LICENSE("GPL");
EXPORT_SYMBOL(ISP_RegCallback);
EXPORT_SYMBOL(ISP_UnregCallback);
