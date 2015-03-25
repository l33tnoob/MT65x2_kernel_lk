/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2012. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_NAND_CORE_H
#define _MTK_NAND_CORE_H

#include "typedefs.h"
#ifndef MTK_EMMC_SUPPORT
#include "nand_device_list.h"
#endif

#define CHIPVER_ECO_1               (0x8a00)
#define CHIPVER_ECO_2               (0x8a01)

/**************************************************************************
*  SIZE DEFINITION
**************************************************************************/
#define NFI_BUF_MAX_SIZE             (0x10000) 

/**************************************************************************
 * Buffer address define for NAND modules
 *************************************************************************/
#define MAX_MAIN_SIZE                (0x1000)
#define MAX_SPAR_SIZE                (0x80)

#define BMT_DAT_BUFFER              bmt_dat_buf 
#define BMT_DAT_BUFFER_SIZE         (MAX_MAIN_SIZE + MAX_SPAR_SIZE)

#define PMT_DAT_BUFFER              pmt_dat_buf 
#define PMT_DAT_BUFFER_SIZE         (MAX_MAIN_SIZE + MAX_SPAR_SIZE)

#define PMT_READ_BUFFER             pmt_read_buf
#define PMT_READ_BUFFER_SIZE        (MAX_MAIN_SIZE)

#define NAND_NFI_BUFFER             nand_nfi_buf
#define NAND_NFI_BUFFER_SIZE        (NFI_BUF_MAX_SIZE)

/******************************************************************************
*
* NFI & ECC Configuration 

*******************************************************************************/
//#define NFI_BASE                  (0x80032000)
//#define NFIECC_BASE               (0x80038000)

/******************************************************************************
*
* NFI Register Definition 

******************************************************************************/

#define NFI_CNFG_REG16              (NFI_BASE+0x0000)
#define NFI_PAGEFMT_REG16           (NFI_BASE+0x0004)
#define NFI_CON_REG16               (NFI_BASE+0x0008)
#define NFI_ACCCON_REG32            (NFI_BASE+0x000C)
#define NFI_INTR_EN_REG16           (NFI_BASE+0x0010)
#define NFI_INTR_REG16              (NFI_BASE+0x0014)

#define NFI_CMD_REG16               (NFI_BASE+0x0020)

#define NFI_ADDRNOB_REG16           (NFI_BASE+0x0030)
#define NFI_COLADDR_REG32           (NFI_BASE+0x0034)
#define NFI_ROWADDR_REG32           (NFI_BASE+0x0038)

#define NFI_STRDATA_REG16           (NFI_BASE+0x0040)

#define NFI_DATAW_REG32             (NFI_BASE+0x0050)
#define NFI_DATAR_REG32             (NFI_BASE+0x0054)
#define NFI_PIO_DIRDY_REG16         (NFI_BASE+0x0058)

#define NFI_STA_REG32               (NFI_BASE+0x0060)
#define NFI_FIFOSTA_REG16           (NFI_BASE+0x0064)
#define NFI_LOCKSTA_REG16           (NFI_BASE+0x0068)

#define NFI_ADDRCNTR_REG16          (NFI_BASE+0x0070)

#define NFI_STRADDR_REG32           (NFI_BASE+0x0080)
#define NFI_BYTELEN_REG16           (NFI_BASE+0x0084)

#define NFI_CSEL_REG16              (NFI_BASE+0x0090)
#define NFI_IOCON_REG16             (NFI_BASE+0x0094)

#define NFI_FDM0L_REG32             (NFI_BASE+0x00A0)
#define NFI_FDM0M_REG32             (NFI_BASE+0x00A4)

#define NFI_LOCK_REG16              (NFI_BASE+0x0100)
#define NFI_LOCKCON_REG32           (NFI_BASE+0x0104)
#define NFI_LOCKANOB_REG16          (NFI_BASE+0x0108)
#define NFI_LOCK00ADD_REG32         (NFI_BASE+0x0110)
#define NFI_LOCK00FMT_REG32         (NFI_BASE+0x0114)
#define NFI_LOCK01ADD_REG32         (NFI_BASE+0x0118)
#define NFI_LOCK01FMT_REG32         (NFI_BASE+0x011C)
#define NFI_LOCK02ADD_REG32         (NFI_BASE+0x0120)
#define NFI_LOCK02FMT_REG32         (NFI_BASE+0x0124)
#define NFI_LOCK03ADD_REG32         (NFI_BASE+0x0128)
#define NFI_LOCK03FMT_REG32         (NFI_BASE+0x012C)
#define NFI_LOCK04ADD_REG32         (NFI_BASE+0x0130)
#define NFI_LOCK04FMT_REG32         (NFI_BASE+0x0134)
#define NFI_LOCK05ADD_REG32         (NFI_BASE+0x0138)
#define NFI_LOCK05FMT_REG32         (NFI_BASE+0x013C)
#define NFI_LOCK06ADD_REG32         (NFI_BASE+0x0140)
#define NFI_LOCK06FMT_REG32         (NFI_BASE+0x0144)
#define NFI_LOCK07ADD_REG32         (NFI_BASE+0x0148)
#define NFI_LOCK07FMT_REG32         (NFI_BASE+0x014C)
#define NFI_LOCK08ADD_REG32         (NFI_BASE+0x0150)
#define NFI_LOCK08FMT_REG32         (NFI_BASE+0x0154)
#define NFI_LOCK09ADD_REG32         (NFI_BASE+0x0158)
#define NFI_LOCK09FMT_REG32         (NFI_BASE+0x015C)
#define NFI_LOCK10ADD_REG32         (NFI_BASE+0x0160)
#define NFI_LOCK10FMT_REG32         (NFI_BASE+0x0164)
#define NFI_LOCK11ADD_REG32         (NFI_BASE+0x0168)
#define NFI_LOCK11FMT_REG32         (NFI_BASE+0x016C)
#define NFI_LOCK12ADD_REG32         (NFI_BASE+0x0170)
#define NFI_LOCK12FMT_REG32         (NFI_BASE+0x0174)
#define NFI_LOCK13ADD_REG32         (NFI_BASE+0x0178)
#define NFI_LOCK13FMT_REG32         (NFI_BASE+0x017C)
#define NFI_LOCK14ADD_REG32         (NFI_BASE+0x0180)
#define NFI_LOCK14FMT_REG32         (NFI_BASE+0x0184)
#define NFI_LOCK15ADD_REG32         (NFI_BASE+0x0188)
#define NFI_LOCK15FMT_REG32         (NFI_BASE+0x018C)

#define NFI_FIFODATA0_REG32         (NFI_BASE+0x0190)
#define NFI_FIFODATA1_REG32         (NFI_BASE+0x0194)
#define NFI_FIFODATA2_REG32         (NFI_BASE+0x0198)
#define NFI_FIFODATA3_REG32         (NFI_BASE+0x019C)

#define NFI_MASTERSTA_REG16         (NFI_BASE+0x0210)
/******************************************************************************
*
* NFI Register Field Definition 

******************************************************************************/

/* NFI_CNFG */
#define CNFG_AHB                    (0x0001)
#define CNFG_READ_EN                (0x0002)
#define CNFG_BYTE_RW                (0x0040)
#define CNFG_HW_ECC_EN              (0x0100)
#define CNFG_AUTO_FMT_EN            (0x0200)
#define CNFG_OP_IDLE                (0x0000)
#define CNFG_OP_READ                (0x1000)
#define CNFG_OP_SRD                 (0x2000)
#define CNFG_OP_PRGM                (0x3000)
#define CNFG_OP_ERASE               (0x4000)
#define CNFG_OP_RESET               (0x5000)
#define CNFG_OP_CUST                (0x6000)

#define CNFG_OP_MODE_MASK           (0x7000)
#define CNFG_OP_MODE_SHIFT          (12)

/* NFI_CON */
#define CON_FIFO_FLUSH              (0x0001)
#define CON_NFI_RST                 (0x0002)
#define CON_NFI_SRD                 (0x0010)

#define CON_NOB_MASK                (0x00E0)
#define CON_NOB_SHIFT               (5)

#define CON_NFI_BRD                 (0x0100)
#define CON_NFI_BWR                 (0x0200)

#define CON_SEC_NUM_MASK            (0xF000)
#define CON_NFI_SEC_SHIFT           (12)

/* NFI_INTR_EN */
#define RD_DONE_EN                  (0x0001)
#define WR_DONE_EN                  (0x0002)
#define RST_DONE_EN                 (0x0004)
#define ERASE_DONE_EN               (0x0008)
#define BSY_RTN_EN                  (0x0010)
#define ACC_LOCK_EN                 (0x0020)
#define AHB_DONE_EN                 (0x0040)
#define ALL_INTR_DE                 (0x0000)
#define ALL_INTR_EN                 (0x007F)

/* NFI_INTR */
#define RD_COMPLETE                 (0x0001)
#define WR_COMPLETE                 (0x0002)
#define RESET_COMPLETE              (0x0004)
#define ERASE_COMPLETE              (0x0008)
#define BUSY_RETURN                 (0x0010)
#define ACCESS_LOCK_LOCK            (0x0020)
#define AHB_DONE                    (0x0040)

/* NFI_ADDRNOB */
#define ADDR_COL_NOB_MASK           (0x0007)
#define ADDR_COL_NOB_SHIFT          (0)
//#define ADDR_ROW_NOB_MASK           (0x0070)
#define ADDR_ROW_NOB_SHIFT          (4)

/* NFI_FIFO */
#define FIFO_RD_EMPTY               (0x0040)
#define FIFO_RD_FULL                (0x0080)
#define FIFO_WR_FULL                (0x8000)
#define FIFO_WR_EMPTY               (0x4000)
#define FIFO_RD_REMAIN(x)           (0x1F&(x))
#define FIFO_WR_REMAIN(x)           ((0x1F00&(x))>>8)

/* NFI_ADDRCNTR */
#define ADDRCNTR_CNTR(x)            ((0xF000&(x))>>12)
#define ADDRCNTR_OFFSET(x)          (0x03FF&(x))

/* NFI_LOCK */
#define NFI_LOCK_ON                 (0x0001)

/* NFI_LOCKANOB */
#define PROG_RADD_NOB_MASK          (0x7000)
#define PROG_RADD_NOB_SHIFT         (12)
#define PROG_CADD_NOB_MASK          (0x0300)
#define PROG_CADD_NOB_SHIFT         (8)
#define ERASE_RADD_NOB_MASK         (0x0070)
#define ERASE_RADD_NOB_SHIFT        (4)
#define ERASE_CADD_NOB_MASK         (0x0007)
#define ERASE_CADD_NOB_SHIFT        (0)

/* Nand flash command */
#define RD_1ST_CMD                  0x00
#define RANDOM_RD_1ST_CMD           0x05
#define RD_2ND_HALF_CMD             0x01    // only for 512 bytes page-size
#define RD_SPARE_CMD                0x50    // only for 512 bytes page-size
#define RD_2ND_CYCLE_CMD            0x30    // only for 2K  bytes page-size
#define RANDOM_RD_2ND_CMD           0xE0
#define RD_FOR_COPYBACK             0x35
#define COPY_1ST_CMD                0x85
#define COPY_2ND_CMD                0x10
#define COPY_PROGRAM                0x8A
#define INPUT_DATA_CMD              0x80
#define PLANE_INPUT_DATA_CMD        0x81
#define RANDOM_INPUT_DATA_CMD       0x85
#define PROG_DATA_CMD               0x10
#define PLANE_PROG_DATA_CMD         0x11
#define CACHE_PROG_CMD              0x15
#define BLOCK_ERASE1_CMD            0x60
#define BLOCK_ERASE2_CMD            0xD0
#define RD_ID_CMD                   0x90
#define RD_STATUS_CMD               0x70
#define RESET_CMD                   0xFF

/* NFI_PAGEFMT */
#define PAGEFMT_512                 (0x0000)
#define PAGEFMT_2K                  (0x0001)
#define PAGEFMT_4K                  (0x0002)

#define PAGEFMT_PAGE_MASK           (0x0003)

#define PAGEFMT_DBYTE_EN            (0x0008)

#define PAGEFMT_SPARE_16            (0x0000)
#define PAGEFMT_SPARE_26            (0x0001)
#define PAGEFMT_SPARE_27            (0x0002)
#define PAGEFMT_SPARE_28            (0x0003)
#define PAGEFMT_SPARE_MASK          (0x0030)
#define PAGEFMT_SPARE_SHIFT         (4)

#define PAGEFMT_FDM_MASK            (0x0F00)
#define PAGEFMT_FDM_SHIFT           (8)

#define PAGEFMT_FDM_ECC_MASK        (0xF000)
#define PAGEFMT_FDM_ECC_SHIFT       (12)

/******************************************************************************
*
* ECC Register Definition 
* 
*******************************************************************************/
#define ECC_ENCCON_REG16    ((volatile P_U16)(NFIECC_BASE+0x0000))
#define ECC_ENCCNFG_REG32   ((volatile P_U32)(NFIECC_BASE+0x0004))
#define ECC_ENCDIADDR_REG32 ((volatile P_U32)(NFIECC_BASE+0x0008))
#define ECC_ENCIDLE_REG32   ((volatile P_U32)(NFIECC_BASE+0x000C))
#define ECC_ENCPAR0_REG32   ((volatile P_U32)(NFIECC_BASE+0x0010))
#define ECC_ENCPAR1_REG32   ((volatile P_U32)(NFIECC_BASE+0x0014))
#define ECC_ENCPAR2_REG32   ((volatile P_U32)(NFIECC_BASE+0x0018))
#define ECC_ENCPAR3_REG32   ((volatile P_U32)(NFIECC_BASE+0x001C))
#define ECC_ENCPAR4_REG32   ((volatile P_U32)(NFIECC_BASE+0x0020))
#define ECC_ENCPAR5_REG32   ((volatile P_U32)(NFIECC_BASE+0x0024))
#define ECC_ENCPAR6_REG32   ((volatile P_U32)(NFIECC_BASE+0x0028))
#define ECC_ENCSTA_REG32    ((volatile P_U32)(NFIECC_BASE+0x002C))
#define ECC_ENCIRQEN_REG16  ((volatile P_U16)(NFIECC_BASE+0x0030))
#define ECC_ENCIRQSTA_REG16 ((volatile P_U16)(NFIECC_BASE+0x0034))

#define ECC_DECCON_REG16    ((volatile P_U16)(NFIECC_BASE+0x0100))
#define ECC_DECCNFG_REG32   ((volatile P_U32)(NFIECC_BASE+0x0104))
#define ECC_DECDIADDR_REG32 ((volatile P_U32)(NFIECC_BASE+0x0108))
#define ECC_DECIDLE_REG16   ((volatile P_U16)(NFIECC_BASE+0x010C))
#define ECC_DECFER_REG16    ((volatile P_U16)(NFIECC_BASE+0x0110))
#define ECC_DECENUM0_REG32   ((volatile P_U32)(NFIECC_BASE+0x0114))
#define ECC_DECENUM1_REG32   ((volatile P_U32)(NFIECC_BASE+0x0118))
#define ECC_DECDONE_REG16   ((volatile P_U16)(NFIECC_BASE+0x011C))
#define ECC_DECEL0_REG32    ((volatile P_U32)(NFIECC_BASE+0x0120))
#define ECC_DECEL1_REG32    ((volatile P_U32)(NFIECC_BASE+0x0124))
#define ECC_DECEL2_REG32    ((volatile P_U32)(NFIECC_BASE+0x0128))
#define ECC_DECEL3_REG32    ((volatile P_U32)(NFIECC_BASE+0x012C))
#define ECC_DECEL4_REG32    ((volatile P_U32)(NFIECC_BASE+0x0130))
#define ECC_DECEL5_REG32    ((volatile P_U32)(NFIECC_BASE+0x0134))
#define ECC_DECEL6_REG32    ((volatile P_U32)(NFIECC_BASE+0x0138))
#define ECC_DECEL7_REG32    ((volatile P_U32)(NFIECC_BASE+0x013C))
#define ECC_DECIRQEN_REG16  ((volatile P_U16)(NFIECC_BASE+0x0140))
#define ECC_DECIRQSTA_REG16 ((volatile P_U16)(NFIECC_BASE+0x0144))
#define ECC_FDMADDR_REG32   ((volatile P_U32)(NFIECC_BASE+0x0148))
#define ECC_DECFSM_REG32    ((volatile P_U32)(NFIECC_BASE+0x014C))
#define ECC_SYNSTA_REG32    ((volatile P_U32)(NFIECC_BASE+0x0150))
#define ECC_DECNFIDI_REG32  ((volatile P_U32)(NFIECC_BASE+0x0154))
#define ECC_SYN0_REG32      ((volatile P_U32)(NFIECC_BASE+0x0158))

/******************************************************************************
*
* ECC register definition
* 
*******************************************************************************
/
/* ECC_ENCON */
#define ENC_EN                  (0x0001)
#define ENC_DE                  (0x0000)

/* ECC_ENCCNFG */
#define ECC_CNFG_ECC4           (0x0000)
#define ECC_CNFG_ECC6           (0x0001)
#define ECC_CNFG_ECC8           (0x0002)
#define ECC_CNFG_ECC10          (0x0003)
#define ECC_CNFG_ECC12          (0x0004)
#define ECC_CNFG_ECC_MASK       (0x00000007)

#define ENC_CNFG_NFI            (0x0010)
#define ENC_CNFG_MODE_MASK      (0x0010)

#define ENC_CNFG_META6          (0x10300000)
#define ENC_CNFG_META8          (0x10400000)

#define ENC_CNFG_MSG_MASK       (0x1FFF0000)
#define ENC_CNFG_MSG_SHIFT      (0x10)

/* ECC_ENCIDLE */
#define ENC_IDLE                (0x0001)

/* ECC_ENCSTA */
#define STA_FSM                 (0x001F)
#define STA_COUNT_PS            (0xFF10)
#define STA_COUNT_MS            (0x3FFF0000)

/* ECC_ENCIRQEN */
#define ENC_IRQEN               (0x0001)

/* ECC_ENCIRQSTA */
#define ENC_IRQSTA              (0x0001)

/* ECC_DECCON */
#define DEC_EN                  (0x0001)
#define DEC_DE                  (0x0000)

/* ECC_ENCCNFG */
#define DEC_CNFG_ECC4          (0x0000)
//#define DEC_CNFG_ECC6          (0x0001)
//#define DEC_CNFG_ECC12         (0x0002)
#define DEC_CNFG_NFI           (0x0010)
//#define DEC_CNFG_META6         (0x10300000)
//#define DEC_CNFG_META8         (0x10400000)

#define DEC_CNFG_FER           (0x01000)
#define DEC_CNFG_EL            (0x02000)
#define DEC_CNFG_CORRECT       (0x03000)
#define DEC_CNFG_TYPE_MASK     (0x03000)

#define DEC_CNFG_EMPTY_EN      (0x80000000)

#define DEC_CNFG_CODE_MASK     (0x1FFF0000)
#define DEC_CNFG_CODE_SHIFT    (0x10)

/* ECC_DECIDLE */
#define DEC_IDLE                (0x0001)

/* ECC_DECFER */
#define DEC_FER0               (0x0001)
#define DEC_FER1               (0x0002)
#define DEC_FER2               (0x0004)
#define DEC_FER3               (0x0008)
#define DEC_FER4               (0x0010)
#define DEC_FER5               (0x0020)
#define DEC_FER6               (0x0040)
#define DEC_FER7               (0x0080)

/* ECC_DECENUM */
#define ERR_NUM0               (0x0000000F)
#define ERR_NUM1               (0x000000F0)
#define ERR_NUM2               (0x00000F00)
#define ERR_NUM3               (0x0000F000)
#define ERR_NUM4               (0x000F0000)
#define ERR_NUM5               (0x00F00000)
#define ERR_NUM6               (0x0F000000)
#define ERR_NUM7               (0xF0000000)

/* ECC_DECDONE */
#define DEC_DONE0               (0x0001)
#define DEC_DONE1               (0x0002)
#define DEC_DONE2               (0x0004)
#define DEC_DONE3               (0x0008)
#define DEC_DONE4               (0x0010)
#define DEC_DONE5               (0x0020)
#define DEC_DONE6               (0x0040)
#define DEC_DONE7               (0x0080)

/* ECC_DECIRQEN */
#define DEC_IRQEN               (0x0001)

/* ECC_DECIRQSTA */
#define DEC_IRQSTA              (0x0001)

/******************************************************************************
*
* NFI Register Field Definition 
* 
*******************************************************************************/

/* NFI_ACCCON */
#define ACCCON_SETTING       ()

/* NFI_INTR_EN */
#define INTR_RD_DONE_EN      (0x0001)
#define INTR_WR_DONE_EN      (0x0002)
#define INTR_RST_DONE_EN     (0x0004)
#define INTR_ERASE_DONE_EN   (0x0008)
#define INTR_BSY_RTN_EN      (0x0010)
#define INTR_ACC_LOCK_EN     (0x0020)
#define INTR_AHB_DONE_EN     (0x0040)
#define INTR_ALL_INTR_DE     (0x0000)
#define INTR_ALL_INTR_EN     (0x007F)

/* NFI_INTR */
#define INTR_RD_DONE         (0x0001)
#define INTR_WR_DONE         (0x0002)
#define INTR_RST_DONE        (0x0004)
#define INTR_ERASE_DONE      (0x0008)
#define INTR_BSY_RTN         (0x0010)
#define INTR_ACC_LOCK        (0x0020)
#define INTR_AHB_DONE        (0x0040)

/* NFI_ADDRNOB */
//#define ADDR_COL_NOB_MASK    (0x0003)
#define ADDR_COL_NOB_SHIFT   (0)
//#define ADDR_ROW_NOB_MASK    (0x0030)
#define ADDR_ROW_NOB_SHIFT   (4)

/* NFI_STA */
#define STA_READ_EMPTY       (0x00001000)
#define STA_ACC_LOCK         (0x00000010)
#define STA_CMD_STATE        (0x00000001)
#define STA_ADDR_STATE       (0x00000002)
#define STA_DATAR_STATE      (0x00000004)
#define STA_DATAW_STATE      (0x00000008)

#define STA_NAND_FSM_MASK    (0x1F000000)
#define STA_NAND_BUSY        (0x00000100)
#define STA_NAND_BUSY_RETURN (0x00000200)
#define STA_NFI_FSM_MASK     (0x000F0000)
#define STA_NFI_OP_MASK      (0x0000000F)

/* NFI_FIFOSTA */
#define FIFO_RD_EMPTY        (0x0040)
#define FIFO_RD_FULL         (0x0080)
#define FIFO_WR_FULL         (0x8000)
#define FIFO_WR_EMPTY        (0x4000)
#define FIFO_RD_REMAIN(x)    (0x1F&(x))
#define FIFO_WR_REMAIN(x)    ((0x1F00&(x))>>8)

/* NFI_ADDRCNTR */
#define ADDRCNTR_CNTR(x)     ((0xF000&(x))>>12)
#define ADDRCNTR_OFFSET(x)   (0x03FF&(x))

/* NFI_LOCK */
#define NFI_LOCK_ON          (0x0001)

/* NFI_LOCKANOB */
#define PROG_RADD_NOB_MASK   (0x7000)
#define PROG_RADD_NOB_SHIFT  (12)
#define PROG_CADD_NOB_MASK   (0x0300)
#define PROG_CADD_NOB_SHIFT  (8)
#define ERASE_RADD_NOB_MASK   (0x0070)
#define ERASE_RADD_NOB_SHIFT  (4)
#define ERASE_CADD_NOB_MASK   (0x0007)
#define ERASE_CADD_NOB_SHIFT  (0)

/******************************************************************************
*
* MTD command
* 
******************************************************************************/
#define NAND_CMD_READ0              0
#define NAND_CMD_READ1              1
#define NAND_CMD_PAGEPROG           0x10
#define NAND_CMD_READOOB            0x50
#define NAND_CMD_ERASE1             0x60
#define NAND_CMD_STATUS             0x70
#define NAND_CMD_SEQIN              0x80
#define NAND_CMD_READID             0x90
#define NAND_CMD_ERASE2             0xd0
#define NAND_CMD_RESET              0xff
#define NAND_CMD_READSTART          0x30
#define NAND_CMD_CACHEDPROG         0x15

/* Options */

#define NAND_NO_AUTOINCR    0x00000001

#define NAND_BUSW_16        0x00000002

#define NAND_NO_PADDING     0x00000004

#define NAND_CACHEPROGRAM   0x00000008

#define NAND_COPYBACK       0x00000010

#define NAND_IS_AND         0x00000020

#define NAND_4PAGE_ARRAY    0x00000040

#define NAND_SS_LOWPOWER_OPTIONS \
    (NAND_NO_PADDING | NAND_CACHEPROGRAM | NAND_COPYBACK)

#define _DEBUG_

/* Debug message event */
#define DBG_EVT_NONE        0x00000000  /* No event */
#define DBG_EVT_ERR         0x00000001  /* DMA related event */
#define DBG_EVT_CMD         0x00000002  /* NAND CMD related event */
#define DBG_EVT_BAD         0x00000004  /* NAND BAD BLOCK handling event */
#define DBG_EVT_INIT        0x00000008  /* NAND INT event */
#define DBG_EVT_READ        0x00000010  /* NAND READ event */
#define DBG_EVT_WRITE       0x00000020  /* NAND WRITE event */
#define DBG_EVT_ERASE       0x00000040  /* NAND ERASE event */

#define DBG_EVT_ALL         0xffffffff

#define DBG_EVT_MASK       (DBG_EVT_INIT | DBG_EVT_BAD )

#ifdef _DEBUG_
#define MSG(evt, fmt, args...) \
    do {    \
    if ((DBG_EVT_##evt) & DBG_EVT_MASK) { \
    print(fmt, ##args); \
    } \
    } while(0)

#define MSG_FUNC_ENTRY(f)   MSG(FUC, "<FUN_ENT>: %s\n", __FUNCTION__)
#else
#define MSG(evt, fmt, args...) do{}while(0)
#define MSG_FUNC_ENTRY(f)      do{}while(0)
#define dbg_print(a,...)
#endif

struct nand_chip
{
    int page_shift;
    unsigned int page_size;
    char ChipID;                /* Type of DiskOnChip */
    char *chips_name;
    unsigned long chipsize;
    unsigned long erasesize;
    unsigned long mfr;          /* Flash IDs - only one type of flash per device */
    unsigned long id;
    char *name;
    int numchips;
    int oobblock;               /* Size of OOB blocks (e.g. 512) */
    int oobsize;                /* Amount of OOB data per block (e.g. 16) */
    int eccsize;
    int bus16;
    int nand_ecc_mode;
};

struct nand_flash_device
{
    char *name;
    int nand_id;
    unsigned long page_size;
    unsigned long chip_size;
    unsigned long erase_size;
    unsigned long options;
};

/**
* struct nand_manufacturers - NAND Flash Manufacturer ID Structure
* @name:    Manufacturer name
* @id:      manufacturer ID code of device.
*/
struct nand_manufacturers
{
    int id;
    char *name;
};
/*
struct nand_oobinfo
{
    u32 useecc;
    u32 eccbytes;
    u32 oobfree[8][2];
    u32 eccpos[32];
};
*/

struct nand_oobfree
{
    u32 offset;
    u32 length;
};

#define MTD_MAX_OOBFREE_ENTRIES	8

struct nand_ecclayout
{
    u32 eccbytes;
    u32 eccpos[64];
    u32 oobavail;
    struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES];
};

/* ECC byte placement */
#define MTD_NANDECC_OFF         0   /* Switch off ECC (Not recommended) */
#define MTD_NANDECC_PLACE       1   /* Use the given placement in the structure (YAFFS1 legacy mode) */
#define MTD_NANDECC_AUTOPLACE   2   /* Use the default placement scheme */
#define MTD_NANDECC_PLACEONLY   3   /* Use the given placement in the structure (Do not store ecc result on read) */
#define MTD_NANDECC_AUTOPL_USR  4   /* Use the given autoplacement scheme rather than using the default */

/*
* NAND Flash Manufacturer ID Codes
*/
#define NAND_MANFR_TOSHIBA  0x98
#define NAND_MANFR_SAMSUNG  0xec
#define NAND_MANFR_FUJITSU  0x04
#define NAND_MANFR_NATIONAL 0x8f
#define NAND_MANFR_RENESAS  0x07
#define NAND_MANFR_STMICRO  0x20
#define NAND_MANFR_HYNIX    0xad
#define NAND_MANFR_MICRON   0x2c
#define NAND_MANFR_AMD      0x01

/******************************************************************************
*
* NAND APIs
* 
******************************************************************************/
extern void nand_device_init(void);
#endif
