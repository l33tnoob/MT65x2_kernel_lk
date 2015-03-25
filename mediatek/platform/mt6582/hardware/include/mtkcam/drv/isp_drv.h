/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _ISP_DRV_H_
#define _ISP_DRV_H_

#include "utils/Mutex.h"    // For android::Mutex.
#include <mtkcam/drv/isp_reg.h>


//*******************************************************************
// Note: Please include following files before including isp_drv.h:
//      #include "utils/Mutex.h"    // For android::Mutex.
//      #include <asm/arch/mt6589_sync_write.h> // For dsb() in isp_drv.h.
//*******************************************************************


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#ifndef USING_MTK_LDVT   // Not using LDVT.
    #include <cutils/xlog.h>
    #define TEMP_LOG_DBG    XLOGD
    #define TEMP_LOG_INF    XLOGI
    #define TEMP_LOG_WRN    XLOGW
#else   // Using LDVT.
    #include "uvvf.h"
    #define TEMP_LOG_DBG    VV_MSG
#endif  // USING_MTK_LDVT

// ------------------ ISP Register Read/Write Macros ----------------------
// Out of date. Will be marked on 11/28 Wed.
// Use ISP_READ_BITS()/ISP_READ_REG()/ISP_WRITE_BITS()/ISP_WRITE_REG()/ISP_WRITE_ENABLE_BITS()/ISP_WRITE_ENABLE_REG() instead.
#define ISP_BITS(RegBase, RegName, FieldName)   (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName)               (RegBase->RegName.Raw)

// New macro for read ISP registers.
#define ISP_READ_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_READ_REG(RegBase, RegName)              (RegBase->RegName.Raw)

// New macro for write ISP registers "except" following registers:
//  CAM_CTL_START       0x00004000
//  CAM_CTL_EN1         0x00004004
//  CAM_CTL_EN2         0x00004008
//  CAM_CTL_DMA_EN      0x0000400C
//  CAM_CTL_EN1_SET     0x00004080
//  CAM_CTL_EN1_CLR     0x00004084
//  CAM_CTL_EN2_SET     0x00004088
//  CAM_CTL_EN2_CLR     0x0000408C
//  CAM_CTL_DMA_EN_SET  0x00004090
//  CAM_CTL_DMA_EN_CLR  0x00004094
// Note: for these registers, use ISP_WRITE_ENABLE_BITS()/ISP_WRITE_ENABLE_REG() instead.
#define ISP_WRITE_BITS(RegBase, RegName, FieldName, Value)              \
    do {                                                                \
        int regAddr = ((int)(&(RegBase->RegName.Raw)) -  (int)RegBase); \
        if (                                                            \
            ( regAddr == 0x00004000 ) ||                                \
            ( regAddr == 0x00004004 ) ||                                \
            ( regAddr == 0x00004008 ) ||                                \
            ( regAddr == 0x0000400C ) ||                                \
            ( regAddr == 0x00004018 ) ||                                \
            ( regAddr == 0x00004080 ) ||                                \
            ( regAddr == 0x00004084 ) ||                                \
            ( regAddr == 0x00004088 ) ||                                \
            ( regAddr == 0x0000408C ) ||                                \
            ( regAddr == 0x00004090 ) ||                                \
            ( regAddr == 0x00004094 ) ||                                \
            ( regAddr == 0x000040A0 ) ||                                \
            ( regAddr == 0x000040A4 )                                   \
        )                                                               \
        {                                                               \
            TEMP_LOG_DBG("[%s, %s, line%04d] WARNING: Addr 0x%08x is enable bit, please use ISP_WRITE_ENABLE_BITS().", __FILE__, __func__, __LINE__, (int)(&(RegBase->RegName.Raw)) - (int)RegBase);  \
        }                                                               \
        if( regAddr < 0x00008000 )                                      \
		{                                                               \
            (RegBase->RegName.Bits.FieldName) = (Value);                \
        }                                                               \
		else                                                            \
		{                                                               \
			TEMP_LOG_DBG("[%s, %s, line%04d] WARNING: Addr 0x%08x is seninf register, please use SENINF_WRITE_BITS().", __FILE__, __func__, __LINE__, (int)(&(RegBase->RegName.Raw)) - (int)RegBase);  \
		}                                                               \        
    } while (0)

#define ISP_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        int regAddr = ((int)(&(RegBase->RegName.Raw)) -  (int)RegBase); \
        if (                                                            \
            ( regAddr == 0x00004000 ) ||                                \
            ( regAddr == 0x00004004 ) ||                                \
            ( regAddr == 0x00004008 ) ||                                \
            ( regAddr == 0x0000400C ) ||                                \
            ( regAddr == 0x00004018 ) ||                                \
            ( regAddr == 0x00004080 ) ||                                \
            ( regAddr == 0x00004084 ) ||                                \
            ( regAddr == 0x00004088 ) ||                                \
            ( regAddr == 0x0000408C ) ||                                \
            ( regAddr == 0x00004090 ) ||                                \
            ( regAddr == 0x00004094 ) ||                                \
            ( regAddr == 0x000040A0 ) ||                                \
            ( regAddr == 0x000040A4 )                                   \
        )                                                               \
        {                                                               \
            TEMP_LOG_DBG("[%s, %s, line%04d] WARNING: Addr 0x%08x is enable bit, please use ISP_WRITE_ENABLE_REG().", __FILE__, __func__, __LINE__, (int)(&(RegBase->RegName.Raw)) - (int)RegBase);  \
        }                                                               \
		if( regAddr < 0x00008000 )                                      \
		{                                                               \
            (RegBase->RegName.Raw) = (Value);                           \
		}                                                               \
		else                                                            \
		{                                                               \
			TEMP_LOG_DBG("[%s, %s, line%04d] WARNING: Addr 0x%08x is seninf register, please use SENINF_WRITE_REG().", __FILE__, __func__, __LINE__, (int)(&(RegBase->RegName.Raw)) - (int)RegBase);  \
		}                                                               \        
    } while (0)

// New macro for write following registers:
//  CAM_CTL_START       0x00004000
//  CAM_CTL_EN1         0x00004004
//  CAM_CTL_EN2         0x00004008
//  CAM_CTL_DMA_EN      0x0000400C
//  CAM_CTL_EN1_SET     0x00004080
//  CAM_CTL_EN1_CLR     0x00004084
//  CAM_CTL_EN2_SET     0x00004088
//  CAM_CTL_EN2_CLR     0x0000408C
//  CAM_CTL_DMA_EN_SET  0x00004090
//  CAM_CTL_DMA_EN_CLR  0x00004094
#define ISP_WRITE_ENABLE_BITS(RegBase, RegName, FieldName, Value)   \
    do {                                                            \
        pthread_mutex_lock(&IspRegMutex);                           \
        (RegBase->RegName.Bits.FieldName) = (Value);                \        
        pthread_mutex_unlock(&IspRegMutex);                         \
    } while (0)
//        TEMP_LOG_DBG("[ISP_WRITE_ENABLE_BITS] %s.%s = 0x%08x.", #RegName, #FieldName, Value);  \

#define ISP_WRITE_ENABLE_REG(RegBase, RegName, Value)   \
    do {                                                \
        pthread_mutex_lock(&IspRegMutex);               \
        (RegBase->RegName.Raw) = (Value);               \        
        pthread_mutex_unlock(&IspRegMutex);             \
    } while (0)
//        TEMP_LOG_DBG("[ISP_WRITE_ENABLE_REG] %s = 0x%08x.", #RegName, Value);  \


// ------------------------------------------------------------------------
#define ISP_CHECK_ENABLE_BIT(RegBase,RegName) \
{\
    int regAddr = ((int)(&(RegBase->RegName.Raw)) -  (int)RegBase); \
    if (\
        ( regAddr == 0x00004000 ) || \
        ( regAddr == 0x00004004 ) || \
        ( regAddr == 0x00004008 ) || \
        ( regAddr == 0x0000400C ) || \
        ( regAddr == 0x00004018 ) || \
        ( regAddr == 0x00004080 ) || \
        ( regAddr == 0x00004084 ) || \
        ( regAddr == 0x00004088 ) || \
        ( regAddr == 0x0000408C ) || \
        ( regAddr == 0x00004090 ) || \
        ( regAddr == 0x00004094 ) || \
        ( regAddr == 0x000040A0 ) || \
        ( regAddr == 0x000040A4 ) \
    )\
    {\
        TEMP_LOG_DBG("[%s, %s, line%04d] WARNING: Addr 0x%08x is enable bit, please use ISP_WRITE_ENABLE_REG/BITS() or ISP_IOCTL_WRITE_ENABLE_REG/BITS().", __FILE__, __func__, __LINE__, (int)(&(RegBase->RegName.Raw)) - (int)RegBase); \
    }\
}
#define ISP_IOCTL_READ_REG(IspDrvPtr,RegBase,RegName)       IspDrvPtr->readReg((MUINT32)(&(RegBase->RegName))-(MUINT32)RegBase)
#define ISP_IOCTL_WRITE_REG(IspDrvPtr,RegBase,RegName,RegValue) \
{\
    ISP_CHECK_ENABLE_BIT(RegBase,RegName);\
    IspDrvPtr->writeReg((MUINT32)(&(RegBase->RegName))-(MUINT32)RegBase,RegValue);\
}
//
#define ISP_IOCTL_READ_BITS(IspDrvPtr,RegBase,RegName,FieldName,FieldValue) \
{\
    RegBase->RegName.Raw = ISP_IOCTL_READ_REG(IspDrvPtr,RegBase,RegName);\
    FieldValue = RegBase->RegName.Bits.FieldName;\
}
#define ISP_IOCTL_WRITE_BITS(IspDrvPtr,RegBase,RegName,FieldName,FieldValue) \
{\
    ISP_CHECK_ENABLE_BIT(RegBase,RegName);\
    RegBase->RegName.Raw = ISP_IOCTL_READ_REG(IspDrvPtr,RegBase,RegName);\
    RegBase->RegName.Bits.FieldName = FieldValue;\
    ISP_IOCTL_WRITE_REG(IspDrvPtr,RegBase,RegName,RegBase->RegName.Raw);\
}
//
#define ISP_IOCTL_WRITE_ENABLE_REG(IspDrvPtr,RegBase,RegName,RegValue) \
{\
    pthread_mutex_lock(&IspRegMutex);\
    IspDrvPtr->writeReg((MUINT32)(&(RegBase->RegName))-(MUINT32)RegBase, RegValue);\    
    pthread_mutex_unlock(&IspRegMutex);\
}
//    TEMP_LOG_DBG("[ISP_IOCTL_WRITE_ENABLE_REG] %s = 0x%08x.", #RegName, RegValue);\


// ISP_IOCTL_WRITE_REG(IspDrvPtr,RegBase,RegName,RegValue);\    //Vent: replaced.

#define ISP_IOCTL_WRITE_ENABLE_BITS(IspDrvPtr,RegBase,RegName,FieldName,FieldValue) \
{\
    pthread_mutex_lock(&IspRegMutex);\
    RegBase->RegName.Raw = ISP_IOCTL_READ_REG(IspDrvPtr,RegBase,RegName);\
    RegBase->RegName.Bits.FieldName = FieldValue;\
    IspDrvPtr->writeReg((MUINT32)(&(RegBase->RegName))-(MUINT32)RegBase, RegBase->RegName.Raw);\    
    pthread_mutex_unlock(&IspRegMutex);\
}
//    TEMP_LOG_DBG("[ISP_IOCTL_WRITE_ENABLE_BITS] %s.%s = 0x%08x.", #RegName, #FieldName, FieldValue);\

//     ISP_IOCTL_WRITE_REG(IspDrvPtr,RegBase,RegName,RegBase->RegName.Raw);\    //Vent: replaced.

// ------------------------------------------------------------------------
//CAM_CTL_INT_STATUS
#define ISP_DRV_IRQ_INT_STATUS_VS1_ST               ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ST1              ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ST2              ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_INT_STATUS_EXPDON1_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_INT_STATUS_TG1_ERR_ST           ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_INT_STATUS_VS2_ST               ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_INT_STATUS_TG2_ST1              ((MUINT32)1 << 6)
#define ISP_DRV_IRQ_INT_STATUS_TG2_ST2              ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_INT_STATUS_EXPDON2_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_INT_STATUS_TG2_ERR_ST           ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST     ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_INT_STATUS_PASS1_TG2_DON_ST     ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST          ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_INT_STATUS_CQ_ERR_ST            ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST         ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INT_STATUS_TPIPE_DON_ST         ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INT_STATUS_AF_DON_ST            ((MUINT32)1 << 16)
#define ISP_DRV_IRQ_INT_STATUS_FLK_DON_ST           ((MUINT32)1 << 17)
#define ISP_DRV_IRQ_INT_STATUS_FMT_DON_ST           ((MUINT32)1 << 18)
#define ISP_DRV_IRQ_INT_STATUS_CQ_DON_ST            ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INT_STATUS_IMGO_ERR_ST          ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INT_STATUS_AAO_ERR_ST           ((MUINT32)1 << 21)
#define ISP_DRV_IRQ_INT_STATUS_LCSO_ERR_ST          ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INT_STATUS_IMG2O_ERR_ST         ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INT_STATUS_ESFKO_ERR_ST         ((MUINT32)1 << 24)
#define ISP_DRV_IRQ_INT_STATUS_FLK_ERR_ST           ((MUINT32)1 << 25)
#define ISP_DRV_IRQ_INT_STATUS_LSC_ERR_ST           ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INT_STATUS_LSC2_ERR_ST          ((MUINT32)1 << 27)
#define ISP_DRV_IRQ_INT_STATUS_BPC_ERR_ST           ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INT_STATUS_LCE_ERR_ST           ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INT_STATUS_DMA_ERR_ST           ((MUINT32)1 << 30)
//CAM_CTL_DMA_INT
#define ISP_DRV_IRQ_DMA_INT_IMGO_DONE_ST            ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMA_INT_IMG2O_DONE_ST           ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMA_INT_AAO_DONE_ST             ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMA_INT_LCSO_DONE_ST            ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMA_INT_ESFKO_DONE_ST           ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMA_INT_DISPO_DONE_ST           ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMA_INT_VIDO_DONE_ST            ((MUINT32)1 << 6)
#define ISP_DRV_IRQ_DMA_INT_CQ0_VR_SNAP_ST          ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMA_INT_NR3O_DONE_ST            ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMA_INT_NR3O_ERR_ST             ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_DMA_INT_SOF2_INT_ST             ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_DMA_INT_BUF_OVL_ST              ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_DMA_INT_TG1_GBERR_ST            ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_DMA_INT_TG2_GBERR_ST            ((MUINT32)1 << 13)
//CAM_CTL_INTB_STATUS
#define ISP_DRV_IRQ_INTB_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTB_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTB_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTB_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTB_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTB_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTB_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTB_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTB_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMAB_INT
#define ISP_DRV_IRQ_DMAB_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAB_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAB_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAB_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAB_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAB_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAB_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAB_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAB_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAB_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
//CAM_CTL_INTC_STATUS
#define ISP_DRV_IRQ_INTC_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTC_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTC_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTC_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTC_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTC_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTC_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTC_STATUS_BPC_ERR_ST          ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INTC_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTC_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMAC_INT
#define ISP_DRV_IRQ_DMAC_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAC_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAC_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAC_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAC_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAC_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAC_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAC_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAC_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAC_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
//CAM_CTL_INT_STATUSX
#define ISP_DRV_IRQ_INTX_STATUS_VS1_ST              ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ST1             ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ST2             ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_INTX_STATUS_EXPDON1_ST          ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_INTX_STATUS_TG1_ERR_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_INTX_STATUS_VS2_ST              ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ST1             ((MUINT32)1 << 6)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ST2             ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_INTX_STATUS_EXPDON2_ST          ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_INTX_STATUS_TG2_ERR_ST          ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_INTX_STATUS_PASS1_TG1_DON_ST    ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_INTX_STATUS_PASS1_TG2_DON_ST    ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_INTX_STATUS_VEC_DON_ST          ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_INTX_STATUS_CQ_ERR_ST           ((MUINT32)1 << 13)
#define ISP_DRV_IRQ_INTX_STATUS_PASS2_DON_ST        ((MUINT32)1 << 14)
#define ISP_DRV_IRQ_INTX_STATUS_TPIPE_DON_ST        ((MUINT32)1 << 15)
#define ISP_DRV_IRQ_INTX_STATUS_AF_DON_ST           ((MUINT32)1 << 16)
#define ISP_DRV_IRQ_INTX_STATUS_FLK_DON_ST          ((MUINT32)1 << 17)
#define ISP_DRV_IRQ_INTX_STATUS_FMT_DON_ST          ((MUINT32)1 << 18)
#define ISP_DRV_IRQ_INTX_STATUS_CQ_DON_ST           ((MUINT32)1 << 19)
#define ISP_DRV_IRQ_INTX_STATUS_IMGO_ERR_ST         ((MUINT32)1 << 20)
#define ISP_DRV_IRQ_INTX_STATUS_AAO_ERR_ST          ((MUINT32)1 << 21)
#define ISP_DRV_IRQ_INTX_STATUS_LCSO_ERR_ST         ((MUINT32)1 << 22)
#define ISP_DRV_IRQ_INTX_STATUS_IMG2O_ERR_ST        ((MUINT32)1 << 23)
#define ISP_DRV_IRQ_INTX_STATUS_ESFKO_ERR_ST        ((MUINT32)1 << 24)
#define ISP_DRV_IRQ_INTX_STATUS_FLK_ERR_ST          ((MUINT32)1 << 25)
#define ISP_DRV_IRQ_INTX_STATUS_LSC_ERR_ST          ((MUINT32)1 << 26)
#define ISP_DRV_IRQ_INTX_STATUS_LSC2_ERR_ST         ((MUINT32)1 << 27)
#define ISP_DRV_IRQ_INTX_STATUS_BPC_ERR_ST          ((MUINT32)1 << 28)
#define ISP_DRV_IRQ_INTX_STATUS_LCE_ERR_ST          ((MUINT32)1 << 29)
#define ISP_DRV_IRQ_INTX_STATUS_DMA_ERR_ST          ((MUINT32)1 << 30)
//CAM_CTL_DMA_INTX
#define ISP_DRV_IRQ_DMAX_INT_IMGO_DONE_ST           ((MUINT32)1 << 0)
#define ISP_DRV_IRQ_DMAX_INT_IMG2O_DONE_ST          ((MUINT32)1 << 1)
#define ISP_DRV_IRQ_DMAX_INT_AAO_DONE_ST            ((MUINT32)1 << 2)
#define ISP_DRV_IRQ_DMAX_INT_LCSO_DONE_ST           ((MUINT32)1 << 3)
#define ISP_DRV_IRQ_DMAX_INT_ESFKO_DONE_ST          ((MUINT32)1 << 4)
#define ISP_DRV_IRQ_DMAX_INT_DISPO_DONE_ST          ((MUINT32)1 << 5)
#define ISP_DRV_IRQ_DMAX_INT_VIDO_DONE_ST           ((MUINT32)1 << 6)
//#define ISP_DRV_IRQ_DMAX_INT_VRZO_DONE_ST           ((MUINT32)1 << 7)
#define ISP_DRV_IRQ_DMAX_INT_NR3O_DONE_ST           ((MUINT32)1 << 8)
#define ISP_DRV_IRQ_DMAX_INT_NR3O_ERR_ST            ((MUINT32)1 << 9)
#define ISP_DRV_IRQ_DMAX_INT_CQ_ERR_ST              ((MUINT32)1 << 10)
#define ISP_DRV_IRQ_DMAX_INT_BUF_OVL_ST             ((MUINT32)1 << 11)
#define ISP_DRV_IRQ_DMAX_INT_TG1_GBERR_ST           ((MUINT32)1 << 12)
#define ISP_DRV_IRQ_DMAX_INT_TG2_GBERR_ST           ((MUINT32)1 << 13)
//
#define ISP_DRV_TURNING_TOP_RESET                   (0xffffffff)

/**************************************************************************
*     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N 
**************************************************************************/
#if 0 //js_test remove later
typedef enum
{
    ISP_DRV_INSTANCE_HW,
    ISP_DRV_INSTANCE_CQ
}ISP_DRV_INSTANCE_ENUM;
#endif

/**************************************************************************
*    CommandQ control
**************************************************************************/

/*
-DON'T program by CQ
    -0x04~18,0x74~0x7C
-config first in advance
    -CQ0_MODE
     CQ0B_MODE
     IMGI_EN
     FLKI_EN
     CQ0_BASEADDR
     CQ0B_BASEADDR
     CQ1_BASEADDR
     CQ2_BASEADDR
     CQ3_BASEADDR
*/

typedef enum
{
ISP_DRV_CQ03 = 0,
ISP_DRV_CQ0,
//    ISP_DRV_CQ0 = 0,
    ISP_DRV_CQ0B,
    ISP_DRV_CQ0C,
    ISP_DRV_CQ01,
    ISP_DRV_CQ01_SYNC,
    ISP_DRV_CQ02,
    ISP_DRV_CQ02_SYNC,
//    ISP_DRV_CQ03,
    ISP_DRV_CQ_NUM
}ISP_DRV_CQ_ENUM;

typedef enum
{
ISP_DRV_DESCRIPTOR_CQ03 = 0,
ISP_DRV_DESCRIPTOR_CQ0,
//    ISP_DRV_DESCRIPTOR_CQ0 = 0,
    ISP_DRV_DESCRIPTOR_CQ0B,
    ISP_DRV_DESCRIPTOR_CQ0C,
    ISP_DRV_DESCRIPTOR_CQ01,
    ISP_DRV_DESCRIPTOR_CQ02,
//    ISP_DRV_DESCRIPTOR_CQ03,
    ISP_DRV_DESCRIPTOR_CQ_NUM
}ISP_DRV_DESCRIPTOR_CQ_ENUM;

typedef enum
{
    CQ_SINGLE_IMMEDIATE_TRIGGER = 0,
    CQ_SINGLE_EVENT_TRIGGER,
    CQ_CONTINUOUS_EVENT_TRIGGER,
    CQ_TRIGGER_MODE_NUM
}ISP_DRV_CQ_TRIGGER_MODE_ENUM;

typedef enum
{
    CQ_TRIG_BY_START = 0,
    CQ_TRIG_BY_PASS1_DONE,
    CQ_TRIG_BY_PASS2_DONE,
    CQ_TRIG_BY_IMGO_DONE,
    CQ_TRIG_BY_IMG2O_DONE,
    CQ_TRIG_SRC_NUM
}ISP_DRV_CQ_TRIGGER_SOURCE_ENUM;

typedef enum
{
//TH    CAM_TOP_IMGSYS  = 0, // 99   15000000~15000188
    CAM_TOP_CTL = 0,     // 2    15004050~15004054
    CAM_TOP_CTL_01,      // 10   15004080~150040A4
    CAM_TOP_CTL_02,      // 8    150040C0~150040DC
    CAM_DMA_TDRI,        // 3    15004204~1500420C
    CAM_DMA_IMGI,        // 8    15004230~1500424C
    //CAM_DMA_IMGCI,       // 7    15004250~15004268
    CAM_DMA_LSCI,        // 7    1500426C~15004284
    //CAM_DMA_FLKI,        // 7    15004288~150042A0
    //CAM_DMA_LCEI,        // 7    150042A4~150042BC
    //CAM_DMA_VIPI,        // 8    150042C0~150042DC
    //CAM_DMA_VIP2I,       // 8    150042E0~150042FC
    CAM_DMA_IMGO,        // 7    15004304~1500431C  //remove base_addr
    CAM_DMA_IMG2O,       // 7    15004324~1500433C  //remove base_addr
    //CAM_DMA_LCSO,        // 7    15004340~15004358
    CAM_DMA_EISO,        // 2    1500435C~15004360
    CAM_DMA_AFO,         // 2    15004364~15004368
    CAM_DMA_ESFKO,       // 7    1500436C~15004384
    CAM_DMA_AAO,         // 7    15004388~150043A0
    CAM_RAW_TG1_TG2,     // 56   15004410~15004494
    CAM_ISP_BIN,         // 4    150044F0~150044F8
    CAM_ISP_OBC,         // 8    15004500~1500451C
    CAM_ISP_LSC,         // 8    15004530~1500454C
    CAM_ISP_HRZ,         // 2    15004580~15004584
    CAM_ISP_AWB,         // 36   150045B0~1500463C
    CAM_ISP_AE,          // 13   15004650~15004694 ..Modify 
    CAM_ISP_SGG,         // 2    150046A0~150046A4
    CAM_ISP_AF,          // 17   150046B0~15004708 ..Modify
    CAM_ISP_FLK,         // 4    15004770~1500477C
    //CAM_ISP_LCS,         // 6    15004780~15004794
    CAM_ISP_BNR,         // 18   15004800~15004844
    CAM_ISP_PGN,         // 6    15004880~15004894
    CAM_ISP_CFA,         // 22   150048A0~150048F4
    CAM_ISP_CCL,         // 3    15004910~15004918
    CAM_ISP_G2G,         // 7    15004920~15004938
    CAM_ISP_UNP,         // 1    15004948
    //CAM_ISP_C02,         // 1    15004950
    //CAM_ISP_MFB,         // 3    15004960~15004968
    //CAM_ISP_LCE_BASIC_1, // 2    150049C0~150049C4
    //CAM_ISP_LCE_CUSTOM,  // 2    150049C8~150049CC
    //CAM_ISP_LCE_BASIC_2, // 9    150049D0~150049F0
    CAM_ISP_G2C,         // 6    15004A00~15004A14
    CAM_ISP_C42,         // 1    15004A1C
    CAM_ISP_NBC,         // 32   15004A20~15004A9C
    CAM_ISP_SEEE,        // 24   15004AA0~15004AFC
    CAM_CDP_CDRZ,        // 15   14002B00~14002B38
    //CAM_CDP_CURZ,        // 15   15004B40~15004B78
    //CAM_CDP_PRZ,         // 15   15004BA0~15004BD8
    //CAM_CDP_FE,          // 4    15004C20~15004C2C
    //CAM_CDP_VIDO,        // 30   15004CC0~15004D34
    //CAM_CDP_DISPO,       // 30   15004D40~15004DB4
    CAM_ISP_EIS,         // 9    15004DC0~15004DE0
    //CAM_ISP_DGM,         // 6    15004E30~15004E44
    //CAM_GDMA_FMT,        // 36   15004E50~15004EDC
    //CAM_CDP_G2G2,        // 7    15004EE0~15004EF8
    //CAM_CDP_3DNR,        // 15   15004F00~15004F38
    CAM_CDP_SL2_FEATUREIO_1,    // 4    15004F40~15004F4C
    CAM_CDP_SL2_IMAGEIO,        // 1    15004F10~15004F10
    CAM_CDP_SL2_FEATUREIO_2,    // 2    15004F14~15004F18
    CAM_ISP_GGMRB,       // 144  15005000~1500523C
    CAM_ISP_GGMG,        // 144  15005300~1500553C
    CAM_ISP_GGM_CTL,     // 1    15005600
    CAM_ISP_PCA,         // 360  15005800~15005D9C
    CAM_ISP_PCA_CON,     // 2    15005E00~15005E04
//TH    CAM_CTL_VERIF,       // 1    15006000
/* CAN'T by CQ
    CAM_SENINF_TOP,      // 1    15008000
    CAM_SENINF_CTL,      // 45   15008010~150080C0
    CAM_CSI2_CTL,        // 48   15008100~150081BC
    CAM_SENINF_SCAM,     // 17   15008200~15008240
    CAM_SENINF_TG1,      // 5    15008300~15008310
    CAM_SENINF_TG2,      // 5    150083A0~150083B0
    CAM_SENINF_CCIR656,  // 10   15008400~15008424
    CAM_SENINF_N3D,      // 9    15008500~15008520
    CAM_CTL_MIPI_RX_CON, // 21   1500C000~1500C050
*/
    CAM_DUMMY_,
    CAM_MODULE_MAX
}CAM_MODULE_ENUM;

typedef struct cq_desc_t
{
    union 
    {
        struct 
        {
            MUINT32 osft_addr  :16;
            MUINT32 cnt        :10;
            MUINT32 inst       :6;
        } token;
        MUINT32 cmd;
    } u;
    MUINT32 v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_STRUCT;

typedef struct 
{
    MUINT32 cmd_set;
    MUINT32 v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_INIT_STRUCT;

typedef struct 
{
    unsigned int id;
    unsigned int addr_ofst;
    unsigned int reg_num;
}ISP_DRV_CQ_MODULE_INFO_STRUCT;

typedef struct
{
    CAM_MODULE_ENUM grop1;
}ISP_DRV_CQ_BIT_1GP_MAPPING;

typedef struct
{
    CAM_MODULE_ENUM grop1;
    CAM_MODULE_ENUM grop2;
}ISP_DRV_CQ_BIT_2GP_MAPPING;

typedef struct
{
    CAM_MODULE_ENUM grop1;
    CAM_MODULE_ENUM grop2;
    CAM_MODULE_ENUM grop3;
}ISP_DRV_CQ_BIT_3GP_MAPPING;

typedef enum
{
    ISP_DRV_IRQ_CLEAR_NONE,
    ISP_DRV_IRQ_CLEAR_WAIT,
    ISP_DRV_IRQ_CLEAR_ALL
}ISP_DRV_IRQ_CLEAR_ENUM;

typedef enum
{
    ISP_DRV_IRQ_TYPE_INT,
    ISP_DRV_IRQ_TYPE_DMA,
    ISP_DRV_IRQ_TYPE_INTB,
    ISP_DRV_IRQ_TYPE_DMAB,
    ISP_DRV_IRQ_TYPE_INTC,
    ISP_DRV_IRQ_TYPE_DMAC,
    ISP_DRV_IRQ_TYPE_INTX,
    ISP_DRV_IRQ_TYPE_DMAX,
    ISP_DRV_IRQ_TYPE_AMOUNT
}ISP_DRV_IRQ_TYPE_ENUM;

typedef enum
{
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_ENGINE_ONLY,
    ISP_DRV_TURNING_UPDATE_TYPE_TOP_ENGINE,
    ISP_DRV_TURNING_UPDATE_TYPE_NUM
}ISP_DRV_TURNING_UPDATE_TYPE_ENUM;

typedef struct
{
    ISP_DRV_IRQ_CLEAR_ENUM  Clear;
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status;
    MUINT32                 Timeout;
}ISP_DRV_WAIT_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Output
}ISP_DRV_READ_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Input
}ISP_DRV_CHECK_IRQ_STRUCT;

typedef struct
{
    ISP_DRV_IRQ_TYPE_ENUM   Type;
    MUINT32                 Status; //Input
}ISP_DRV_CLEAR_IRQ_STRUCT;

typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}ISP_DRV_REG_IO_STRUCT;

typedef struct
{
    MUINT32 updateCqDescriptorNum;  // store the CQ descriptor number that want to update
    MBOOL   mTurUpdTable[CAM_MODULE_MAX];
    MUINT32 topCtlEn1;
    MUINT32 topCtlEn2;
    MUINT32 topCtlDma;
    MUINT32 topCtlEn1Keep;
    MUINT32 topCtlEn1PrevKeep;
    MUINT32 topCtlEn2Keep;
    MUINT32 topCtlEn2PrevKeep;
    MUINT32 topCtlDmaKeep;
    MUINT32 topCtlDmaPrevKeep;

}ISP_TURNING_UPDATE_INFO;

typedef MINT32 (*CALLBACKFUNC_ISP_DRV)(MINT32 cqNum, MVOID *user);

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern pthread_mutex_t IspRegMutex; // Original IspRegMutex is defined in isp_drv.cpp.

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class IspDrv
{
    protected:
        IspDrv(){};
        virtual ~IspDrv() {};    
    public:
        //static IspDrv*  createInstance(ISP_DRV_INSTANCE_ENUM Instance);//js_test remove later
        static IspDrv *createInstance(MBOOL fgIsGdmaMode = MFALSE);
        virtual MVOID  destroyInstance() = 0;
        virtual MBOOL  init() = 0;
        virtual MBOOL  uninit() = 0;
        virtual MBOOL  waitIrq(ISP_DRV_WAIT_IRQ_STRUCT WaitIrq) = 0;
        virtual MBOOL  readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq) = 0;
        virtual MBOOL  checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq) = 0;
        virtual MBOOL  clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq) = 0;
        virtual MBOOL  reset() = 0;
        virtual MBOOL  resetBuf() = 0;
        virtual MUINT32 getRegAddr() = 0;
        virtual isp_reg_t *getRegAddrMap() = 0;
        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count) = 0;
        virtual MUINT32 readReg(MUINT32 Addr) = 0;
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT *pRegIo, MUINT32 Count) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr, MUINT32 Data) = 0;
        virtual MBOOL   holdReg(MBOOL En) = 0;
        virtual MBOOL   dumpReg() = 0;
        virtual MBOOL   IsReadOnlyMode() = 0;
//        virtual MBOOL   IsGdmaMode() = 0;
        
        //commandQ
        virtual IspDrv *getCQInstance(ISP_DRV_CQ_ENUM cq);
        virtual MBOOL   cqAddModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId);
        virtual MBOOL   cqDelModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId);
        virtual MUINT32 getCQDescBufPhyAddr(ISP_DRV_CQ_ENUM cq);
        virtual MBOOL   setCQTriggerMode(ISP_DRV_CQ_ENUM cq,
                                                ISP_DRV_CQ_TRIGGER_MODE_ENUM mode,
                                                ISP_DRV_CQ_TRIGGER_SOURCE_ENUM trig_src);
        
        virtual MBOOL rtBufCtrl(MVOID *pBuf_ctrl) = 0;
        virtual MUINT32 GlobalPipeCountInc() = 0;
        virtual MUINT32 GlobalPipeCountDec() = 0;

        // for turning update
        virtual MBOOL   updateTurningCq1();
        virtual MBOOL   updateTurningCq2();
        MVOID    lockSemaphoreCq1();
        MVOID    lockSemaphoreCq2();
        MVOID    unlockSemaphoreCq1();
        MVOID    unlockSemaphoreCq2();
        MUINT32 getTpipeMgrVaCq1(MUINT32 *pDescArray, MUINT32 *pDescNum, MUINT32 *pCtlEn1, MUINT32 *pCtlEn2, MUINT32 *pCtlDma);
        MUINT32 getTpipeMgrVaCq2(MUINT32 *pDescArray, MUINT32 *pDescNum, MUINT32 *pCtlEn1, MUINT32 *pCtlEn2, MUINT32 *pCtlDma);
        MVOID setCallbacks(CALLBACKFUNC_ISP_DRV pCameraIOCB, MVOID *user);
        MBOOL setTurnTopEn1(MINT32 cq, MINT32 en1);
        MBOOL setTurnTopEn2(MINT32 cq, MINT32 en2);
        MBOOL setTurnTopDma(MINT32 cq, MINT32 dma);
        MBOOL flushTurnCqTable(MINT32 cq);
        MUINT32 getTurnTopEn1(MINT32 cq);
        MUINT32 getTurnTopEn2(MINT32 cq);
        MUINT32 getTurnTopDma(MINT32 cq);

    private:
        mutable android::Mutex   cqPhyDesLock; // for cq phy address
        mutable android::Mutex   cqVirDesLock; // for cq virtual address
        mutable android::Mutex   turnKeepCq1Lock;
        mutable android::Mutex   turnKeepCq2Lock;
   
    public:
        MINT32          mFd;
        
        static MUINT32 *mpIspHwRegAddr;
        
        //for commandQ
        static MINT32  mIspVirRegFd;
        static MUINT32 *mpIspVirRegBuffer;
        static MUINT32 mIspVirRegSize;
        static MUINT32 *mpIspVirRegAddrVA[ISP_DRV_CQ_NUM];
        static MUINT32 *mpIspVirRegAddrPA[ISP_DRV_CQ_NUM];
        
        //CQ descriptor buffer
        static MINT32  mIspCQDescFd;
        static MUINT32 *mpIspCQDescBufferVirt;
        static MUINT32 mIspCQDescSize;
        static MUINT32 mpIspCQDescBufferPhy;
        
        // for turning update
        static ISP_TURNING_UPDATE_INFO tdriMgrInfoCq1;
        static ISP_TURNING_UPDATE_INFO tdriMgrInfoCq2;

        //friend class IspDrvShellImp;        
        //share same member
        static ISP_DRV_CQ_CMD_DESC_STRUCT *mpIspCQDescriptorVirt[ISP_DRV_DESCRIPTOR_CQ_NUM];
        static MUINT32                     mpIspCQDescriptorPhy[ISP_DRV_DESCRIPTOR_CQ_NUM];
        
        CALLBACKFUNC_ISP_DRV mpIspDrvCB;
        MVOID *mCallbackCookie;
        
        MUINT32 *m_pRTBufTbl;
        MUINT32 m_RTBufTblSize;
        
        MUINT32 *mpIspDrvRegMap;
        
        //MBOOL            m_IsGdmaMode;
};

#endif  // _ISP_DRV_H_

