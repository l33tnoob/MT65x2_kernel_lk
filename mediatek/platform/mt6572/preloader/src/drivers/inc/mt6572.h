/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef MT6572_H
#define MT6572_H

/*=======================================================================*/
/* Constant Definitions                                                  */
/*=======================================================================*/

#define IO_PHYS            	(0x10000000)
#define IO_SIZE            	(0x01000000)

#define VER_BASE            (0x08000000)

/*=======================================================================*/
/* Register Bases                                                        */
/*=======================================================================*/
#define TOP_CLKCTRL_BASE        (IO_PHYS + 0x00000000)
#define CONFIG_BASE             (IO_PHYS + 0x00001000)  //infracfg_ao
#define SRAMROM_BASE            (IO_PHYS + 0x00001400)
#define KP_BASE                 (IO_PHYS + 0x00002000)
#define EMI_BASE                (IO_PHYS + 0x00004000)
#define GPIO_BASE               (IO_PHYS + 0x00005000)
#define RGU_BASE                (IO_PHYS + 0x00007000)
#define GPT_BASE                (IO_PHYS + 0x00008000)
#define EFUSE_CTR_BASE          (IO_PHYS + 0x00009000)
#define PMIC_WRAP_BASE          (IO_PHYS + 0x0000F000)

#define APMCUSYS_CONFIG_BASE    (IO_PHYS + 0x00200000)
#define APMIXED_BASE            (IO_PHYS + 0x00205000)
#define IO_CFG_BOTTOM_BASE      (IO_PHYS + 0x00209000)
#define IO_CFG_LEFT_BASE        (IO_PHYS + 0x0020A000)
#define IO_CFG_RIGHT_BASE       (IO_PHYS + 0x0020B000)

#define CORTEA7MP_BASE          (IO_PHYS + 0x00210000)
#define GIC_DIST_BASE           (CORTEA7MP_BASE + 0x1000)
#define GIC_CPU_BASE            (CORTEA7MP_BASE + 0x2000)

#define NFI_BASE                (IO_PHYS + 0x01001000)
#define NFIECC_BASE             (IO_PHYS + 0x01002000)
#define UART0_BASE              (IO_PHYS + 0x01005000)
#define UART1_BASE              (IO_PHYS + 0x01006000)
#define I2C0_BASE               (IO_PHYS + 0x01009000)
#define I2C1_BASE               (IO_PHYS + 0x0100A000)
#define SPI_BASE                (IO_PHYS + 0x0100C000)
#define USB_BASE                (IO_PHYS + 0x01100000)
#define USBSIF_BASE             (IO_PHYS + 0x01110000)
#define MSDC0_BASE              (IO_PHYS + 0x01120000)
#define MSDC1_BASE              (IO_PHYS + 0x01130000)
/*=======================================================================*/
#define APHW_VER            (VER_BASE)
#define APSW_VER            (VER_BASE + 0x04)
#define APHW_CODE           (VER_BASE + 0x08)
#define APHW_SUBCODE        (VER_BASE + 0x0C)


#define AMCONFG_BASE        (0xFFFFFFFF)            /* CHECKME & FIXME */
/*=======================================================================*/
/* USB download control                                                  */
/*=======================================================================*/
// memory preserved mode
#define SLAVE_JUMP_REG          (SRAMROM_BASE + 0x0008) //0x10001408
#define SLAVE1_MAGIC_REG        (SRAMROM_BASE + 0x000C) //0x1000140C
// USB download control
#define SRAMROM_USBDL_REG       (SRAMROM_BASE + 0x001C)
#define SRAMROM_USBDL_MAGIC_REG (SRAMROM_BASE + 0x0020)
#define SRAMROM_DASIGLEN        (SRAMROM_BASE + 0x0030)

#define SRAMROM_USBDL_MAGIC (0x55534244) /* USBD */
#define USBDL_BIT_EN        (0x00000001) /* 1: download bit enabled */
#define USBDL_PL            (0x00000002) /* 0: usbdl by brom; 1: usbdl by bootloader */
#define USBDL_TIMEOUT_MASK  (0x0000FFFC) /* 14-bit timeout: 0x0000~0x3FFE: second; 0x3FFFF: no timeout */
#define USBDL_TIMEOUT_MAX   (USBDL_TIMEOUT_MASK >> 2) /* maximum timeout indicates no timeout */

#define SRAMROM_USBDL_TO_DIS    (SRAMROM_BASE + 0x002C)
#define USBDL_TO_DIS            (0x00000001)

/*=======================================================================*/
/* SRAM REPAIR                                                  */
/*=======================================================================*/
#define SRAM_REPAIR_REG             (EFUSE_CTR_BASE + 0x0170)
#define SRAM_REPAIR_ENABLE_BIT      (0x00200000)    //0x170 [21]

/*=======================================================================*/
/* Memory Presrved mode control                                                  */
/*=======================================================================*/
#define GIC_DIST_CTRL                   0x000
#define GIC_DIST_CTR                    0x004
#define GIC_DIST_ENABLE_SET             0x100
#define GIC_DIST_ENABLE_CLEAR           0x180
#define GIC_DIST_PENDING_SET            0x200
#define GIC_DIST_PENDING_CLEAR          0x280
#define GIC_DIST_ACTIVE_BIT             0x300
#define GIC_DIST_PRI                    0x400
#define GIC_DIST_TARGET                 0x800
#define GIC_DIST_CONFIG                 0xc00
#define GIC_DIST_SOFTINT                0xf00

#define GIC_CPU_CTRL                    0x00
#define GIC_CPU_PRIMASK                 0x04

#define GIC_SPI_START            (32)
#define NUM_SPI_SOURCES          (128)
#define NUM_IRQ_SOURCES          (GIC_SPI_START + NUM_SPI_SOURCES)

#define SLAVE1_MAGIC_NUM 0x534C4131

//bss_init_emi_baseaddr is define in link_mem_descriptio.ld
#define BSS_TCM_END   0x01003660

#define MTK_WDT_MCU_RG_DRAMC_SREF		(0x0100)
#if defined(CFG_MEM_PRESERVED_MODE)
//#define LOAD_NORMAL_BOOT_PRELOADER      (1)
#endif
/*=======================================================================*/
/* NAND Control                                                          */
/*=======================================================================*/
#define NAND_PAGE_SIZE                  (2048)  // (Bytes)
#define NAND_BLOCK_BLKS                 (64)    // 64 nand pages = 128KB
#define NAND_PAGE_SHIFT                 (9)
#define NAND_LARGE_PAGE                 (11)    // large page
#define NAND_SMALL_PAGE                 (9)     // small page
#define NAND_BUS_WIDTH_8                (8)
#define NAND_BUS_WIDTH_16               (16)
#define NAND_FDM_SIZE                   (8)
#define NAND_ECC_SW                     (0)
#define NAND_ECC_HW                     (1)

#define NFI_MAX_FDM_SIZE                (8)
#define NFI_MAX_FDM_SEC_NUM             (8)
#define NFI_MAX_LOCK_CHANNEL            (16)

#define ECC_MAX_CORRECTABLE_BITS        (12)
#define ECC_MAX_PARITY_SIZE             (20)    /* in bytes */

#define ECC_ERR_LOCATION_MASK           (0x1FFF)
#define ECC_ERR_LOCATION_SHIFT          (16)

#define NAND_FFBUF_SIZE                 (2048+64)

#endif
