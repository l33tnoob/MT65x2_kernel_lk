/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
** $Id: mt5911.cpp,v 1.4 2008/10/28 13:45:43 MTK01725 Exp $
*/

/*******************************************************************************
** Copyright (c) 2005 - 2007 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

/*
** $Log: mt5911.cpp,v $
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.4  2008/10/28 13:45:43  MTK01725
** Frog add IO Pin Test and other little modification.
**
** Revision 1.3  2008/06/12 02:34:19  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:48:58  MTK01385
** 1. setXtalTrimToCr(), queryThermoinfo() and setThermoEn().
**
** Revision 1.1  2008/05/26 14:04:36  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.5  2008/02/23 16:31:11  MTK01385
** 1 add support set Crystal Frequency Trim and RCPI offset to eeprom.
**
** Revision 1.3  2008/02/22 13:16:12  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.2  2008/01/03 07:53:50  MTK01385
** 1. Add Set Chip type function.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.4  2007/11/12 06:25:04  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.3  2007/10/17 09:28:51  MTK01267
** modify DAC function in mt5911, mt5912
**
** Revision 1.2  2007/10/17 08:50:34  MTK01267
** move DAC function to mt5911, mt5912
**
** Revision 1.1  2007/10/16 06:58:51  MTK01267
** Initial version
**
** Initial version
**
*/
#include "mt5911.h"
#include "param.h"

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
#define GROUP_CCK   0
#define GROUP_OFDM1 1
#define GROUP_OFDM2 2

typedef struct _TX_RATE_INFO {
    BOOLEAN cck;
    BOOLEAN b24G;   /* TRUE if 2.4G FALSE if 5G */
    int     rate;   /* in unit of 500K */
    int     index;  /* index for each modulation */
    int     offset;
} TX_RATE_INFO;

typedef struct _TX_MAX_POWER_OFFSET {
    int offset;
    int nChannel[2];
} TX_MAX_POWER_OFFSET;

/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/
/* system control */
#define MCR_SCR                         0x0000
#define MCR_ECSR                        0x0040
#define MCR_GPIOCR                      0x0050
#define MCR_PCICR                       0x0240
#define MCR_PPMCR                       0x0294
#define MCR_LCR                         0X02A8
#define MCR_WBCR                        0x03F8

/* System Control Register (SCR 0x0000) bit definitions */
#define SCR_EXTERNAL_INT_TRIG_EDGE      BIT(26)
#define SCR_EXTERNAL_INT_STATUS         BIT(25)
#define SCR_SLOW_CLK_DIS                BIT(22)
#define SCR_PAU_CLK_DYN_GATE_DIS        BIT(21)
#define SCR_SEC_CLK_DYN_GATE_DIS        BIT(20)
#define SCR_MAC_PAU_CLK_EN              BIT(19)
#define SCR_MAC_FIFO_CLK_EN             BIT(18)
#define SCR_MAC_HIF_CLK_EN              BIT(17)
#define SCR_CLK_ALL_EN                  (SCR_MAC_HIF_CLK_EN  | \
                                         SCR_MAC_FIFO_CLK_EN | \
                                         SCR_MAC_PAU_CLK_EN | SCR_SEC_CLK_DYN_GATE_DIS)
#define SCR_INT_POLARITY                BIT(12)
#define SCR_INT_CTRL_3                  BIT(11)
#define SCR_INT_CTRL_2                  BIT(10)
#define SCR_INT_CTRL_1                  BIT(9)
#define SCR_INT_CTRL_0                  BIT(8)

#define SCR_PAU_EN                      BIT(7)
#define SCR_MAC_REGRST                  BIT(3)
#define SCR_MAC_RST                     BIT(2)
#define SCR_BB_SIORST                   BIT(1)
#define SCR_BB_MRST                     BIT(0)
#define SCR_RST_ALL                     BITS(0,3)

/* EEPROM Control and Status Register (ECSR 0x0040) bit definitions */
#define ECSR_EE_TYPE                    BITS(4, 6)  /* EEPROM type */
#define ECSR_EE_NOT_PRESENT             0x00
#define ECSR_EE_128_BYTE                0x10
#define ECSR_EE_256_BYTE                0x20
#define ECSR_EE_512_BYTE                0x30
#define ECSR_EE_1024_BYTE               0x40
#define ECSR_EE_2048_BYTE               0x50
#define ECSR_EE_CHKSUM_ERR              BIT(3)      /* EEPROM checksum error */
#define ECSR_EE_RDY                     BIT(2)      /* EEPROM access complete */
#define ECSR_EE_RECALL                  BIT(1)      /* recall operation from EEPROM */
#define ECSR_EE_SERIAL                  BIT(0)      /* EEPROM serial access mode enable */

/* GPIO Control Register (GPIOCR) bit definitions */
#define GPIOCR_FIX_WAIT_EN              BIT(31)
#define GPIOCR_GPIO_1_IO_CTRL           BIT(18)
#define GPIOCR_GPIO_1_IO_CTRL_IN        BIT(18)
#define GPIOCR_GPIO_1_IO_CTRL_OUT       0
#define GPIOCR_GPIO_0_IO_CTRL           BIT(16)
#define GPIOCR_GPIO_0_IO_CTRL_IN        BIT(16)
#define GPIOCR_GPIO_0_IO_CTRL_OUT       0
#define GPIOCR_EXTERN_RF_PD_POLARITY    BIT(7)
#define GPIOCR_EXTERN_RF_PD_EN          BIT(6)
#define GPIOCR_GPIO_1                   BIT(2)
#define GPIOCR_GPIO_0                   BIT(0)

/* PHY Control Interface Configuration Register (PCICR 0x0240) bit definitions */
#define PCICR_BB_CTRL_TR                BIT(9)
#define PCICR_PA5_EN_POLARITY           BIT(8)
#define PCICR_PA2_EN_POLARITY           BIT(7)
#define PCICR_DIS_LP_SKIP               BIT(6)
#define PCICR_RF_BAND                   BIT(5)
#define PCICR_RF_BAND_24                0           /* 2.4 GHz band */
#define PCICR_RF_BAND_5                 BIT(5)      /* 5 GHz band */
#define PCICR_EEPROM_CONTENT            BITS(0,2)

/* PHY Power Management Control Register (PPMCR 0x0294) bit definitions */
#define PPMCR_GPIO_RF_SHDN              BIT(28)     /* GPIO for RF power control */
#define PPMCR_GPIO_PA2_EN               BIT(22)     /* GPIO for RF 2.4G PA enable */
#define PPMCR_GPIO_PA5_EN               BIT(21)     /* GPIO for RF 5G PA enable */
#define PPMCR_GPIO_RF_TX                BIT(20)     /* GPIO for RF Tx control */
#define PPMCR_GPIO_RF_RX                BIT(19)     /* GPIO for RF Rx control */
#define PPMCR_GPIO_TR_SW                BIT(18)     /* GPIO for BB TX */
#define PPMCR_BB_TX_PE                  BIT(17)     /* GPIO for RF Tx/Rx control */
#define PPMCR_BB_RX_PE                  BIT(16)     /* GPIO for RF Tx/Rx control */
#define PPMCR_CONT_TX (PPMCR_GPIO_RF_TX | PPMCR_GPIO_TR_SW | PPMCR_GPIO_RF_SHDN)

/* LED Configuration Register (LCR 0x02A8) bit definitions */
#define LCR_LED_RX_FILTER               BIT(21)
#define LCR_LED_TRBEACON_EN             BIT(20)     /* Enable LED When transmitting or receiving beacon  */
#define LCR_LED_GPIO_OUTPUT             BIT(19)     /* LED GPIO Output control */
#define LCR_LED_POLARITY                BIT(18)     /* LEDOn polarity */
#define LCR_LED_POLARITY_POSITIVE       0
#define LCR_LED_POLARITY_NEGATIVE       BIT(18)
#define LCR_LED_MODE                    BITS(16,17) /* LED mode */
#define LCR_LED_MODE_GPIO               0
#define LCR_LED_MODE_TX                 BIT(16)
#define LCR_LED_MODE_RX                 BIT(17)
#define LCR_LED_MODE_TX_RX              BITS(16,17)
#define LCR_LED_ON_CNT                  BITS(8,15)  /* LED On count, in unit of 4 ms */
#define LCR_LED_OFF_CNT                 BITS(0,7)   /* LED Off count, in unit of 4 ms */


/* WLAN and Bluetooth Co-Existence Register (Offset 0x03F8) bit definitions */
#define WBR_TX_DELAY                       BITS(8,10)
#define WBR_WLAN_ACTIVE_WHEN_CCA_ACTIVE_EN BIT(5)
#define WBR_BLUETOOTH_1ST_PRIORITY_EN      BIT(4)
#define WBR_ASSERT_BEFORE_BEACON_EN        BIT(3)
#define WBR_ASSERT_THROUGH_ACK_EN          BIT(2)
#define WBR_ASSERT_MIN_REQ_EN              BIT(1)
#define WBR_COEXIST_EN                     BIT(0)

/* EEPROM data locations (word offset) */
#define EEPROM_SIGNATURE                        0x00 /* EEPROM signature */

/* Host interface dependent  */
/* PCI */
#define EEPROM_PCI_DEVICE_ID                    0x01 /* PCI device ID */
#define EEPROM_PCI_VENDOR_ID                    0x02 /* PCI vendor ID */
#define EEPROM_PCI_CLASS_CODE_BYTE_1_2          0x03 /* PCI class code [31:16] */
#define EEPROM_PCI_CLASS_CODE_BYTE_0            0x04 /* PCI class code [8:15] */
#define EEPROM_PCI_SUBSYSTEM_ID                 0x05 /* PCI subsystem ID */
#define EEPROM_PCI_SUBSYSTEM_VENDOR_ID          0x06 /* PCI subsystem vendor ID */
#define EEPROM_PCI_MIN_GNT_MAX_LAT              0x07 /* PCI minimum grant timer & maximum latency timer */
#define EEPROM_CARDBUS_CIS_PTR                  0x08 /* CardBus CIS start/end pointer */
#define EEPROM_PCI_PWR_MGT_CTRL                 0x09 /* PCI power management control */

/* SDIO */
#define EEPROM_SDIO_CCCR                        0x01 /* SDIO[3:0], CCCR[3:0], 4'h0, SD[3:0] */
#define EEPROM_SDIO_SRW                         0x02 /* SRW, S4MI, SCSI, SBS, 3'h0, SMPC, 8'h0 */
#define EEPROM_SDIO_IO_CODE                     0x03 /* 4'h0, IO code[3:0], 8'h0 */
#define EEPROM_SDIO_SPS                         0x04 /* 7'h0, SPS, 8'h0 */
#define EEPROM_SDIO_OCR_LOWER                   0x05 /* OCR[7:0], OCR[15:8] */
#define EEPROM_SDIO_OCR_HIGHER                  0x06 /* OCR[23:16], 8'h0 */
#define EEPROM_SDIO_CIS_PTR                     0x08 /* [15:8]: CIS start, [7:0]: CIS end */
#define EEPROM_SDIO_CIS_LEN                     0x09 /* [15:8]: CIS 1 length, [7:0]: CIS 0 length */

/* CF */
#define EEPROM_CF_CIS_PTR                       0x08 /* CardBus CIS start/end pointer */
#define EEPROM_CF_CIS_LEN                       0x09 /* CIS Length*/

#define EEPROM_INTERFACE_CFG_CHKSUM             0x0A /* interface configuration & checksum */
#define EEPROM_LED_CFG_PHY_MODE                 0x0B /* LED configuration & PHY mode */
#define EEPROM_RF_ANT_MODE                      0x0C /* RF ANT mode */
#define EEPROM_OSC_STABLE_TIME                  0x0D /* OSC Stable Time in us */
#define EEPROM_DAC_IQ_OFFSET                    0x0E /* DAC offset IQ offset */
#define EEPROM_RF_INIT                          0x0F /* RF Init From EEPROM Bit definition */

#define EEPROM_MAC_ADDR_BYTE_0_1                0x10 /* MAC address [15:0] */
#define EEPROM_MAC_ADDR_BYTE_2_3                0x11 /* MAC address [31:16] */
#define EEPROM_MAC_ADDR_BYTE_4_5                0x12 /* MAC address [47:32] */
#define EEPROM_REG_DOMAIN                       0x13 /* regulatory domain */
#define EEPROM_LAYOUT_VERSION                   0x14 /* Layout version */

/* CCK TX Power Gain Table for 2.4G band */
#define EEPROM_2G_CCK_TXPWR_GAIN_START          0x16
#define EEPROM_2G_CCK_TXPWR_GAIN_END            0x1C

/* OFDM TX Power Gain Table for 2.4G band */
#define EEPROM_2G_OFDM_TXPWR_GAIN_START         0x1D
#define EEPROM_2G_OFDM_TXPWR_GAIN_END           0x23

/* EEPROM offset for 5G band OFDM Tx power gain table */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_8_12       0x2A /* Channel   8 ~  12 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_16_34      0x2B /* Channel  16 ~  34 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_36_38      0x2C /* Channel  36 ~  38 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_40_42      0x2D /* Channel  40 ~  42 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_44_46      0x2E /* Channel  44 ~  46 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_48_52      0x2F /* Channel  48 ~  52 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_56_60      0x30 /* Channel  56 ~  60 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_64_100     0x31 /* Channel  64 ~ 100 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_104_108    0x32 /* Channel 104 ~ 108 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_112_116    0x33 /* Channel 112 ~ 116 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_120_124    0x34 /* Channel 120 ~ 124 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_128_132    0x35 /* Channel 128 ~ 132 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_136_140    0x36 /* Channel 136 ~ 140 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_149_153    0x37 /* Channel 149 ~ 153 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_157_161    0x38 /* Channel 157 ~ 161 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_240_244    0x39 /* Channel 240 ~ 244 */
#define EEPROM_5G_OFDM_TXPWR_GAIN_CH_248_252    0x3A /* Channel 248 ~ 252 */

/* SLOPE RATIO 1 Available if 0x3B's BIT(2) is TRUE*/
#define EEPROM_SLOPE1_RATIO                     0x4E
/* SLOPE RATIO 2 Available if 0x3B's BIT(2) is TRUE*/
#define EEPROM_SLOPE2_RATIO                     0x4F

#define EEPROM_TX_PWR_MARK                      0x50 /* Tx power mark valid mask */
#define EEPROM_RF_INIT_SEQ_OFFSET               0x51

/* ALC information */
#define EEPROM_ALC_INFO                         0x8C
#define EEPROM_24G_CCK_ALC_OFFSET               0x8D
#define EEPROM_24G_OFDM_ALC_OFFSET_START        0x8E
#define EEPROM_24G_OFDM_ALC_OFFSET_END          0x95
#define EEPROM_5G_OFDM_ALC_OFFSET_START         0x96
#define EEPROM_5G_OFDM_ALC_OFFSET_END           0x9D

/* EEPROM offset for checksum */
#define EEPROM_NIC_CHKSUM                       0x9F

/* EEPROM signature field (word offset: 0x00) */
#define EEPROM_SIGNATURE_MT5911                 0x5911

/* EEPROM PCI minimum grant timer & maximum latency timer fields
   (word offset: 0x07) */
#define EEPROM_PCI_MIN_GNT_MASK                 BITS(0,7)
#define EEPROM_PCI_MAX_LAT_MASK                 BITS(8,15)

/* EEPROM CardBus CIS start/end pointer fields (word offset: 0x08) */
#define EEPROM_CARDBUS_CIS_END_PTR              BITS(0,7)
#define EEPROM_CARDBUS_CIS_START_PTR            BITS(8,15)

/* EEPROM interface configuration & checksum fields (word offset: 0x0A) */
#define EEPROM_INTERFACE_CFG_MASK               BITS(0,7)
#define EEPROM_CHECKSUM_MASK                    BITS(8,15)

/* EEPROM LED configuration & PHY mode (word offset: 0x0B) */
#define EEPROM_LED_MODE_MASK                    BITS(0,2)
#define EEPROM_LED_MODE_DEFAULT                 0x0000
#define EEPROM_LED_MODE_CUSTOM_1                0x0001
#define EEPROM_LED_MODE_CUSTOM_2                0x0002
#define EEPROM_LED_MODE_CUSTOM_3                0x0003

#define EEPROM_PHY_MODE_MASK                    BITS(8,9)
#define EEPROM_PHY_MODE_G                       0x0000
#define EEPROM_PHY_MODE_A_G                     0x0100
#define EEPROM_PHY_MODE_A                       0x0200
#define EEPROM_PHY_MODE_B                       0x0300

#define EEPROM_ANT_MASK                         BITS(0,3)
#define EEPROM_RX_ANT_PRESENT_MASK              BITS(0,1)
#define EEPROM_RX_ANT_0_PRESENT                 BIT(0)
#define EEPROM_RX_ANT_1_PRESENT                 BIT(1)
#define EEPROM_TX_ANT_PRESENT_MASK              BITS(2,3)
#define EEPROM_TX_ANT_0_PRESENT                 BIT(2)
#define EEPROM_TX_ANT_1_PRESENT                 BIT(3)

#define EEPROM_ANT_SEL_MODE_MASK                BIT(8)
#define EEPROM_ANT_SEL_MODE_1_MAIN              0
#define EEPROM_ANT_SEL_MODE_0_MAIN              BIT(8)

#define EEPROM_DAC_I_OFFSET BITS(8,12)
#define EEPROM_DAC_Q_OFFSET BITS(0,4)

#define EEPROM_RF_INIT_SEQ_NUM                  BITS(0,7)

#define EEPROM_RF_MODE_MASK                     BITS(4,7)
#define EEPROM_RF_MODE_5953                     0x00
#define EEPROM_RF_MODE_5952                     0x10
#define EEPROM_RF_MODE_5951                     0x20
#define EEPROM_RF_MODE_MAXIM_282527             0x30
#define EEPROM_RF_MODE_AIROHA_2232              0x40
#define EEPROM_RF_MODE_AIROHA_7230              0x50
#define EEPROM_RF_MODE_AIROHA_2230              0x60
#define EEPROM_RF_MODE_AIROHA_2236              0x70
#define EEPROM_RF_MODE_AIROHA_2238              0x80
#define EEPROM_RF_MODE_AIROHA_2237              0x90

#define EEPROM_RF_ANT_SEL_MODE_MASK             BIT(8)
#define EEPROM_RF_ANT_SEL_MAIN_1                0
#define EEPROM_RF_ANT_SEL_MAIN_0                1

#define EEPROM_DAC_OFFSET_I_MASK                BITS(8,12)
#define EEPROM_DAC_OFFSET_Q_MASK                BITS(0,4)

/* 0x14 EEPROM_LAYOUT_VERSION */
#define EEPROM_LAYOUT_VERSION_MASK              BITS(0,3)

/* 0x3B EEPROM_ALC_INFO */
#define EEPROM_ALC_INFO_24G_VALID               BIT(0)
#define EEPROM_ALC_INFO_5G_VALID                BIT(1)
#define EEPROM_ALC_INFO_USE_SLOPE_RATIO         BIT(2)
#define EEPROM_ALC_INFO_ENABLED                 BITS(0,2)

#define EEPROM_PWR_MARK_RF_SETTTING_MASK        BITS(0,7)
#define EEPROM_PWR_MARK_DB_MASK                 BITS(8,15)

/* 0x4E SLOPE RATIO 1 */
#define EEPROM_SLOPE1_RATIO_DIVIDER             BITS(0,7)
#define EEPROM_SLOPE1_RATIO_DIVIDEND            BITS(8,15)

/* 0x4F SLOPE RATIO 2 */
#define EEPROM_SLOPE2_RATIO_DIVIDER             BITS(0,7)
#define EEPROM_SLOPE2_RATIO_DIVIDEND            BITS(8,15)

#define EEPROM_TX_PWR_MARK_MASK                 BIT(0)
#define EEPROM_TX_PWR_MARK_INVALID              0
#define EEPROM_TX_PWR_MARK_VALID                BIT(0)

#define EEPROM_DB_ALC_TUPLE_ALC_MASK            BITS(0,7)
#define EEPROM_DB_ALC_TUPLE_DB_MASK             BITS(8,15)

#define EEPROM_NIC_CHKSUM_MASK                  BITS(8,15)

static TX_MAX_POWER_OFFSET maxPowerChannelOffsetTable[] = {
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_8_12, 8,12},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_16_34, 16,34},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_36_38, 36,38},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_40_42, 40,42},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_44_46, 44,46},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_48_52, 48,52},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_56_60, 56,60},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_64_100, 64,100},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_104_108, 104,108},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_112_116, 112,116},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_120_124, 120,124},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_128_132, 128,132},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_136_140, 136,140},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_149_153, 149,153},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_157_161, 157,161},
    {EEPROM_5G_OFDM_TXPWR_GAIN_CH_240_244, 240,244}
};

static TX_RATE_INFO dbAlcOffsetTale[] = {
    {TRUE,  TRUE,  RATE_1M,   0,    0},
    {TRUE,  TRUE,  RATE_2M,   1,    0},
    {TRUE,  TRUE,  RATE_5_5M, 2,    0},
    {TRUE,  TRUE,  RATE_11M,  3,    0},
    {FALSE, TRUE,  RATE_6M,   0,    0},
    {FALSE, TRUE,  RATE_9M,   1,    0},
    {FALSE, TRUE,  RATE_12M,  2,    0},
    {FALSE, TRUE,  RATE_18M,  3,    0},
    {FALSE, TRUE,  RATE_24M,  4,    0},
    {FALSE, TRUE,  RATE_36M,  5,    0},
    {FALSE, TRUE,  RATE_48M,  6,    0},
    {FALSE, TRUE,  RATE_54M,  7,    0},
    {FALSE, FALSE, RATE_6M,   0,    0},
    {FALSE, FALSE, RATE_9M,   1,    0},
    {FALSE, FALSE, RATE_12M,  2,    0},
    {FALSE, FALSE, RATE_18M,  3,    0},
    {FALSE, FALSE, RATE_24M,  4,    0},
    {FALSE, FALSE, RATE_36M,  5,    0},
    {FALSE, FALSE, RATE_48M,  6,    0},
    {FALSE, FALSE, RATE_54M,  7,    0}
 };

/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/
/*_________________________________________________________________________
**  readMCR32
**
**  descriptions: read 32-bit data from MCR
**  parameters:
**          nCardIndex: NIC index number
**          offset: address offset of the MCR
**  output:
**          value:  value read from the MCR (size: 4 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::readMCR32 (UINT_32 offset, UINT_32 * value)
{
	IPC_MCR_RW_STRUC m_sMACREG_RW;
	UINT_32 BytesRead;

	DEBUGFUNC("CMT5911::readMCR32");
	INITLOG((_T("\n")));

	m_sMACREG_RW.bRead = TRUE;
    m_sMACREG_RW.mcrIndex = offset;
    m_sMACREG_RW.mcrData = 0;  //clear before read

	if (!m_OID->setOID(this,
			OID_IPC_MCR_RW,
			(CHAR *)&m_sMACREG_RW,
			sizeof(IPC_MCR_RW_STRUC))
			) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	if (!m_OID->queryOID(this,
			OID_IPC_MCR_RW,
			(CHAR *)&m_sMACREG_RW,
			sizeof(IPC_MCR_RW_STRUC),
			&BytesRead)
			) {
        ERRORLOG((TEXT("Failed to read configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sMACREG_RW.mcrData;
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  writeMCR32
**
**  descriptions: write 32-bit data to MCR
**  parameters:
**          nCardIndex: NIC index number
**          offset: address offset of the MCR
**          value:  value set to the MCR (size: 4 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::writeMCR32 (UINT_32 offset, UINT_32 value)
{
	IPC_MCR_RW_STRUC m_sMACREG_RW;

	DEBUGFUNC("CMT5911::writeMCR32");
	INITLOG((_T("\n")));

	m_sMACREG_RW.bRead = FALSE;
    m_sMACREG_RW.mcrIndex = offset;
    m_sMACREG_RW.mcrData = value;

	if (!m_OID->setOID(this,
		OID_IPC_MCR_RW,
        (CHAR *)&m_sMACREG_RW,
        sizeof(IPC_MCR_RW_STRUC))
		) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  ReadBBReg8
**
**  descriptions: read 8-bit data in MMI interface
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the BB registers
**  output
**          value:  value read from the BB register (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::readBBCR8 (UINT_32 offset, UINT_32* value)
{
	IPC_BBCR_RW_STRUC m_sMMI_RW;
	UINT_32 BytesRead;

	m_sMMI_RW.bRead = TRUE;
	m_sMMI_RW.bbcrIndex = (UINT_8)offset;
	m_sMMI_RW.bbcrData = 0;    //clear before read

	if (!m_OID->setOID(this,
			OID_IPC_BBCR_RW,
			(CHAR *)&m_sMMI_RW,
			sizeof(IPC_BBCR_RW_STRUC))
			) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	if (!m_OID->queryOID(this,
			OID_IPC_BBCR_RW,
			(CHAR *)&m_sMMI_RW,
			sizeof(IPC_BBCR_RW_STRUC),
			&BytesRead)
			) {
        ERRORLOG((TEXT("Failed to read configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sMMI_RW.bbcrData;
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  WriteBBReg8
**
**  descriptions: write 8-bit data in MMI interface
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the BB registers
**          value:  value set to the BB register (size: 1 Byte)
**  return:
**          check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::writeBBCR8 (UINT_32 offset, UINT_32 value)
{
	IPC_BBCR_RW_STRUC m_sMMI_RW;

	m_sMMI_RW.bRead = FALSE;
    m_sMMI_RW.bbcrIndex = (UINT_8)offset;
    m_sMMI_RW.bbcrData = (UINT_8)value;

	if (!m_OID->setOID(this,
			OID_IPC_BBCR_RW,
	        (CHAR *)&m_sMMI_RW,
	        sizeof(IPC_BBCR_RW_STRUC))
			) {
//		ERRORLOG((TEXT("Failed to WriteBBReg8 CR%d value:0x%x"), offset, value));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  ReadEEPRom16
**
**  descriptions: read 16-bit data to EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the EEPRom offset
**  output:
**          value:  value read from the EEPRom offset (size: 2 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::readEEPRom16 (UINT_32 offset, UINT_32 * value)
{
	IPC_EEPROM_RW_STRUC m_sEEPRom_RW;
	UINT_32 BytesRead;

	m_sEEPRom_RW.bRead = TRUE;
    m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_RW;
    m_sEEPRom_RW.eepromIndex = (UINT_8)offset;
    m_sEEPRom_RW.eepromData = 0;   //clear before read

	if (!m_OID->setOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC))
	        ) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    if (!m_OID->queryOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC),
	        &BytesRead)
	        ) {
        ERRORLOG((TEXT("Failed to read configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sEEPRom_RW.eepromData;
    return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  WriteEEPRom16
**
**  descriptions: write 16-bit data to EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the EEPRom offset (0, 1, 2, 3......)
**          value:  value set to the EEPRom offset (size: 2 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::writeEEPRom16 (UINT_32 offset, UINT_32 value)
{
	IPC_EEPROM_RW_STRUC m_sEEPRom_RW;

	m_sEEPRom_RW.bRead = FALSE;
    m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_RW;
    m_sEEPRom_RW.eepromIndex = (UINT_8)offset;
    m_sEEPRom_RW.eepromData = (UINT_16)value;

	if (!m_OID->setOID(this,
            OID_IPC_EEPROM_RW,
            (CHAR *)&m_sEEPRom_RW,
            sizeof(IPC_EEPROM_RW_STRUC))
            ) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
	}
	return ERROR_RFTEST_SUCCESS;

}


INT_32
CMT5911::setTestMode(void)
{
    PARAM_RFTEST_INFO info;

	info.Length = sizeof(info.Length);
	if ( !m_OID->setOID(this,
			OID_IPC_TEST_MODE,
			(CHAR *) &info,
			info.Length)) {
		ERRORLOG((TEXT("Failed to set driver to test mode\n")));
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	// read CR with continuous Tx related configuration
    readBBCR8(BBCR_CR81,  &BBCRSave[0]);
    readBBCR8(BBCR_CR82,  &BBCRSave[1]);
    readBBCR8(BBCR_CR83,  &BBCRSave[2]);
    readBBCR8(BBCR_CR84,  &BBCRSave[3]);
    readBBCR8(BBCR_CR85,  &BBCRSave[4]);
    readBBCR8(BBCR_CR88,  &BBCRSave[5]);
    readBBCR8(BBCR_CR89,  &BBCRSave[6]);
    readBBCR8(BBCR_CR90,  &BBCRSave[7]);

    readMCR32(MCR_PPMCR,  &PPMCRSave);

    // Store original IO register setting
    readMCR32(MCR_GPIOCR,   &ioRegSave[0]);
    readMCR32(MCR_LCR,      &ioRegSave[1]);
    readMCR32(MCR_WBCR,    &ioRegSave[2]);

    // Init EEPROM ALC Offset Table
    if (getEEPRomVersion_() == 1) {
        updateALCOffset();
    }

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setDriverNormalMode
**
**  descriptions: Set the driver to normal mode
**  parameters:
**  return:
**          0, if successful
**         -1, otherwise
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setNormalMode(void)
{
    UINT_32 tmpU4;

	if ( !m_OID->setOID(this,
            OID_IPC_ABORT_TEST_MODE,
            (CHAR*) &tmpU4,
            sizeof(tmpU4))
            ) {
        ERRORLOG((TEXT("Failed to set driver to normal mode")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }


    //recover BB from force AD/DA Mode
    writeBBCR8(BBCR_CR81, BBCRSave[0]);
    writeBBCR8(BBCR_CR82, BBCRSave[1]);
    writeBBCR8(BBCR_CR84, BBCRSave[3]);
    writeBBCR8(BBCR_CR85, BBCRSave[4]);
    writeBBCR8(BBCR_CR88, BBCRSave[5]);
    writeBBCR8(BBCR_CR89, BBCRSave[6]);
    writeBBCR8(BBCR_CR90, BBCRSave[7]);

    writeMCR32(MCR_PPMCR, PPMCRSave);

    /* Restore original IO register setting */
    writeMCR32(MCR_GPIOCR,  ioRegSave[0]);
    writeMCR32(MCR_LCR,     ioRegSave[1]);
    writeMCR32(MCR_WBCR,   ioRegSave[2]);

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  StandBy
**
**  descriptions:   stop current status
**  parameters:
**  return:
**           0, if successful
**          -1, Failed to set stop pattern
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setStandBy(void)
{
    writeBBCR8(BBCR_CR81, BBCRSave[0]);
    writeBBCR8(BBCR_CR82, BBCRSave[1]);
    writeBBCR8(BBCR_CR84, BBCRSave[3]);
    writeBBCR8(BBCR_CR85, BBCRSave[4]);
    writeBBCR8(BBCR_CR88, BBCRSave[5]);
    writeBBCR8(BBCR_CR89, BBCRSave[6]);
    writeBBCR8(BBCR_CR90, BBCRSave[7]);

    writeMCR32(MCR_PPMCR, PPMCRSave);

	TX_PACKET_STRUC m_TxPacket;

	// stop Continuous Packet Tx Mode
	if (m_bEnableContiPktTx == TRUE) {
		// set the pattern to fill the structure which indicate to stop
		m_TxPacket.pktLength	= 0xF0F0F0F0;
		m_TxPacket.pktCount 	= 0xF0F0F0F0;
		m_TxPacket.pktInterval	= 0xF0F0F0F0;

		if ( !m_OID->setOID(this,
				OID_IPC_TEST_PACKET_TX,
			    (CHAR *)&m_TxPacket,
			    sizeof(TX_PACKET_STRUC))
			    ) {
			ERRORLOG((TEXT("Failed to set stop pattern")));
			return ERROR_RFTEST_NDIS_OID_FAILURE;
		}
		m_bEnableContiPktTx = FALSE;
	}

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setOutputPower
**
**  descriptions: Enable continuous tx. PAU disabled. Manually controlled the
**                  TX_RX, TX_PE, PA_PE, TR_SW on, RX_PE off.
**  parameters:
**          nTxRate: 0/1/2/3 fo 1M/2M/5.5M/11M bps
**  return:
**          0, if successful
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setOutputPower(
    INT_32 nTxRate,
    INT_32 txPower,
    INT_32 txAntenna
    )
{
	INT_32 status;

	//1. setBBTxPower
	status = this->setBBTxPower(txPower);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//2. setTxAnt
	status = this->setTxAnt(txAntenna);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//3. setOutputPower
    UINT_32 nCr8;

    //regist this card in output_power mode, return to stand by reference this setting
    setPPMCRAndSCR();

    readBBCR8(8, &nCr8);
    nCr8 &= ~BITS(2,7);
    nCr8 |= getTxRate(nTxRate, 0);
    writeBBCR8(8, nCr8);
    writeBBCR8(90, BITS(5,6));

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setCarrierSuppressionMeasure
**
**  descriptions:
**  parameters:
**              nModulationType: 0 CCK, 1 OFDM
**  return:
**          0, if successful
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setCarrierSuppression(
    INT_32 nModulationType,
    INT_32 txPower,
    INT_32 txAntenna
    )
{
	INT_32 status;

	//1. setBBTxPower
	status = this->setBBTxPower(txPower);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//2. setTxAnt
	status = this->setTxAnt(txAntenna);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//3. setCarrierSuppressionMeasure
    UINT_32 bbcrValue;

    setPPMCRAndSCR();

    readBBCR8(90, &bbcrValue);
    bbcrValue &= ~BITS(3,6);

    if (nModulationType) { //OFDM Mode
        /*set Test Mode continuous wave enable, OFDM long train symbol*/
        writeBBCR8(90, BIT(3)|BIT(6));
    }
	else { //CCK Mode
        /*set Test Mode continuous wave enable, QPSK 90 degree*/
        writeBBCR8(90, (BITS(3,4)|BIT(6)));
    }

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setLocalFrequecy
**
**  descriptions:
**  parameters:
**              nCardIndex: Which NIC
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setLocalFrequecy(INT_32 txPower, INT_32 txAntenna)
{
	UINT_32 status;

	//1. setBBTxPower
	status = this->setBBTxPower(txPower);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//2. setTxAnt
	status = this->setTxAnt(txAntenna);
	if(status) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	//3. setLocalFrequecyMeasure
	//regist this card in output_power mode, return to stand by reference this setting

    /*1 Tx_Rx: Enable(Hi)
	  2 TR_SW:Tx
	  3 PA_PE:Active
	  4 TX_PE:Active
	  5 TXCLK, TXD, TX_RDY: De_active
	  6 RX_PE, RXCLK, RXD, MD_RDY : De_active*/

    setPPMCRAndSCR();

    /* Configure BBP to forced DA mode,
       After test, should reset these BB CR to 0
    */
    // AFE AD/DA on
    writeBBCR8(BBCR_CR8, BIT(7));
    writeBBCR8(BBCR_CR82, BITS(2,5) | BIT(7));

    UINT_32 bbcrValue;
    readBBCR8(BBCR_CR84, &bbcrValue);
    bbcrValue |= BIT(7);
    writeBBCR8(BBCR_CR84, bbcrValue);

    // set CLK80M/40M clock
    writeBBCR8(BBCR_CR85, 0x7F);
    writeBBCR8(BBCR_CR88, BIT(7));

    return ERROR_RFTEST_SUCCESS;
}

CHAR *
CMT5911::getRFICType(void)
{
	UINT_32 output;
    CHAR * rfStr;

    readEEPRom16(EEPROM_RF_ANT_MODE, &output);
    switch (output & EEPROM_RF_MODE_MASK) {
        case EEPROM_RF_MODE_5953:
            rfStr = "MT5953";
            break;
        case EEPROM_RF_MODE_5952:
            rfStr = "MT2452";
            break;
        case EEPROM_RF_MODE_5951:
            rfStr = "MT5951";
            break;
        case EEPROM_RF_MODE_MAXIM_282527:
            rfStr = "Maxim 2825/27";
            break;
        case EEPROM_RF_MODE_AIROHA_2230:
            rfStr = "Airoha 2230";
            break;
        case EEPROM_RF_MODE_AIROHA_7230:
            rfStr = "Airoha 7230";
            break;
        case EEPROM_RF_MODE_AIROHA_2232:
            rfStr = "Airoha 2232";
            break;
        case EEPROM_RF_MODE_AIROHA_2236:
            rfStr = "Airoha 2236";
            break;
        case EEPROM_RF_MODE_AIROHA_2238:
            rfStr = "Airoha 2238";
            break;
        case EEPROM_RF_MODE_AIROHA_2237:
            rfStr = "Airoha 2237";
            break;
        default:
            rfStr = "Unknown RF control mode";
    }

    strcpy(rfType, rfStr);
    return rfType;
}

INT_32
CMT5911::readBBCRStatistic(
    UINT_16* ed,
    UINT_16* osd,
    UINT_16* sq1,
    UINT_16* sfd,
    UINT_16* fcs)
{
	UINT_32 nCr98, nCr99, nCr100, nCr101, nCr104, nCr105, nCr110, nCr111, nCr108, nCr109;

	writeBBCR8(BBCR_CR97, 0x00);
	readBBCR8(BBCR_CR98,  &nCr98);
	readBBCR8(BBCR_CR99,  &nCr99);
	readBBCR8(BBCR_CR100, &nCr100);
	readBBCR8(BBCR_CR101, &nCr101);
	readBBCR8(BBCR_CR104, &nCr104);
	readBBCR8(BBCR_CR105, &nCr105);
	readBBCR8(BBCR_CR110, &nCr110);
	readBBCR8(BBCR_CR111, &nCr111);
	readBBCR8(BBCR_CR108, &nCr108);
	readBBCR8(BBCR_CR109, &nCr109);

	*ed  = (UINT_16)(nCr98 * 256)   + (UINT_16)nCr99;
	*osd = (UINT_16)(nCr100 * 256)  + (UINT_16)nCr101;
	*sq1 = (UINT_16)(nCr104 * 256)  + (UINT_16)nCr105;
	*sfd = (UINT_16)(nCr110 * 256)  + (UINT_16)nCr111;
	*fcs = (UINT_16)(nCr108 * 256)  + (UINT_16)nCr109;

	writeBBCR8(BBCR_CR97, 0x40);
	writeBBCR8(BBCR_CR97, 0x80);

	return ERROR_RFTEST_SUCCESS;

}

INT_32
CMT5911::getRadarStatus(void)
{
	UINT_32 nBBCR2 = 0, nBBCR4 = 0;
	UINT_32 dwISAR = 0;

	//Initial, BBCR2 BIT0 SET 1
	nBBCR2 |= 1;
	writeBBCR8(2, nBBCR2);

	//Read MCR ISAR Bit20
	readMCR32(0x0004, &dwISAR);
	if ((dwISAR & BIT(20)) == (BIT(20))) {

		//Resume write 1 to bit 20
		dwISAR |= (1 << 20);
		writeMCR32(0x0004, dwISAR);

		//BBCR 4 bit 7 set 1
		readBBCR8(4, &nBBCR4);
		nBBCR4 |= (1 << 7);
		writeBBCR8(4, nBBCR4);

		return ERROR_RFTEST_SUCCESS;
	}

	return ERROR_RFTEST_GENERAL_ERROR;
}

/*_________________________________________________________________________
**  setAlcInfoToEeprom
**
**  descriptions:
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setAlcInfoToEeprom (
    INT_32 b24gAvailable,
    INT_32 b5gAvailable)
{
	UINT_32 alvInfo = 0, eepromValue = 0;

    if(readEEPRom16(EEPROM_LAYOUT_VERSION, &eepromValue))
        return -1;

    if((eepromValue & EEPROM_LAYOUT_VERSION_MASK) == 0 )
        return -1;

    if(b24gAvailable)
        alvInfo |= EEPROM_ALC_INFO_24G_VALID;

    if(b5gAvailable)
        alvInfo |= EEPROM_ALC_INFO_5G_VALID;

    if(b5gAvailable) //bUseSlope
        alvInfo |= EEPROM_ALC_INFO_USE_SLOPE_RATIO;

    return writeEEPRom16(EEPROM_ALC_INFO, alvInfo);
}

/*_________________________________________________________________________
**  getAlcInfo
**
**  descriptions: Return ALC info field
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::getAlcInfo(
    INT_32* b24gAvailable,
    INT_32* b5gAvailable,
    INT_32* bUseSlopeRate)
{
	UINT_32 alvInfo = 0, eepromValue = 0;

	if(readEEPRom16(EEPROM_LAYOUT_VERSION, &eepromValue))
		return ERROR_RFTEST_GENERAL_ERROR;

	if((eepromValue & EEPROM_LAYOUT_VERSION_MASK) == 0 )
		return ERROR_RFTEST_GENERAL_ERROR;

	if(readEEPRom16(EEPROM_ALC_INFO, &alvInfo))
		return ERROR_RFTEST_GENERAL_ERROR;

	if(alvInfo & EEPROM_ALC_INFO_24G_VALID)
		*b24gAvailable = TRUE;
	else
		*b24gAvailable = FALSE;

	if(alvInfo & EEPROM_ALC_INFO_5G_VALID)
		*b5gAvailable = TRUE;
	else
		*b5gAvailable = FALSE;

	if(alvInfo & EEPROM_ALC_INFO_USE_SLOPE_RATIO)
		*bUseSlopeRate = TRUE;
	else
		*bUseSlopeRate = FALSE;

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setAlcSlopeRatioToEeprom
**
**  descriptions: Update ALC Info Field
**  parameters:
**    slope1Divider:
**    slope1Dividend:
**    slope2Divider:
**    slope2Dividend:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setAlcSlopeRatioToEeprom (
    INT_32	slope1Divider,
    INT_32	slope1Dividend,
    INT_32	slope2Divider,
    INT_32	slope2Dividend
    )
{
	UINT_32 alvInfo = 0, eepromValue = 0;

	if(readEEPRom16(EEPROM_LAYOUT_VERSION, &eepromValue))
		return ERROR_RFTEST_GENERAL_ERROR;

	if((eepromValue & EEPROM_LAYOUT_VERSION_MASK) == 0 )
		return ERROR_RFTEST_GENERAL_ERROR;

	if(readEEPRom16(EEPROM_ALC_INFO, &alvInfo))
		return ERROR_RFTEST_GENERAL_ERROR;

	/* No slope information */
	if((alvInfo & EEPROM_ALC_INFO_USE_SLOPE_RATIO) == 0) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	eepromValue = ((slope1Dividend << 8) & EEPROM_SLOPE1_RATIO_DIVIDEND);
	eepromValue |= (slope1Divider & EEPROM_SLOPE1_RATIO_DIVIDER);
	if(writeEEPRom16(EEPROM_SLOPE1_RATIO, eepromValue)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	eepromValue = ((slope2Dividend << 8) & EEPROM_SLOPE2_RATIO_DIVIDEND);
	eepromValue |= (slope2Divider & EEPROM_SLOPE2_RATIO_DIVIDER);
	if(writeEEPRom16(EEPROM_SLOPE2_RATIO, eepromValue)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getAlcSlopeRatio
**
**  descriptions: Return ALC Slope Ration field
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::getAlcSlopeRatio (
    INT_32* slope1Divider,
    INT_32* slope1Dividend,
    INT_32* slope2Divider,
    INT_32* slope2Dividend
    )
{
	UINT_32 alvInfo = 0, eepromValue = 0;

	if(readEEPRom16(EEPROM_LAYOUT_VERSION, &eepromValue))
		return ERROR_RFTEST_GENERAL_ERROR;

	if((eepromValue & EEPROM_LAYOUT_VERSION_MASK) == 0 )
		return ERROR_RFTEST_GENERAL_ERROR;

	if(readEEPRom16(EEPROM_ALC_INFO, &alvInfo))
		return ERROR_RFTEST_GENERAL_ERROR;

	/* No slope information */
	if((alvInfo & EEPROM_ALC_INFO_USE_SLOPE_RATIO) == 0) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	if(readEEPRom16(EEPROM_SLOPE1_RATIO, &alvInfo))
		return ERROR_RFTEST_GENERAL_ERROR;

	*slope1Divider = alvInfo & EEPROM_SLOPE1_RATIO_DIVIDER;
	*slope1Dividend = (alvInfo & EEPROM_SLOPE1_RATIO_DIVIDEND) >> 8;

	if(readEEPRom16(EEPROM_SLOPE2_RATIO, &alvInfo))
		return ERROR_RFTEST_GENERAL_ERROR;

	*slope2Divider = alvInfo & EEPROM_SLOPE2_RATIO_DIVIDER;
	*slope2Dividend = (alvInfo & EEPROM_SLOPE2_RATIO_DIVIDEND) >> 8;

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setTXMaxPowerToEEProm
**
**  descriptions: Save TX Power from EEPROM
**  parameters:
**             nChannel:   Channel frequence in kHz
**             bCck:       1: CCK 0:OFDM
**             nTxPwr:     TX Power
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setTXMaxPowerToEEProm(INT_32 channelFreg, INT_32 bCck, INT_32 nTxPwr)
{
	INT_32 nChannel, band, b24G = FALSE;
	UINT_32 offset, nValue;

	if(getChannelBand(channelFreg, &nChannel, &band)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}
	if(band == 0)
		b24G = TRUE;

    if((nChannel < 1) || (nChannel > 252)) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    offset = findMaxPowerByteOffset(nChannel, bCck, b24G);
    if (offset < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    /* Offset is byte offset */
    readEEPRom16(offset/2, &nValue);

    /* Higher Byte */
    if ((offset & 1) == 1) {
        nValue &= 0x00FF;
        nValue |= (nTxPwr << 8) & BITS(8,15);
    }
	else {
        nValue &= 0xFF00;
        nValue |=  nTxPwr & BITS(0,7);
    }
    writeEEPRom16(offset/2, nValue);

    return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setTXMaxPowerToEEPromEx
**
**  descriptions: Save TX gain control value to EEPROM
**  parameters:
**             nCardIndex:  Which adapter.
**             nChannel:    Channel frequence in kHz
**             rate:        In unit of 500K
**             gainControl: Gain control value
**             outputPower: Measured output power
**             targetAlc:   Expected target ALC
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setTXMaxPowerToEEPromEx(
	INT_32 channelFreq,
	INT_32 rate,
	INT_32 gainControl,
	INT_32 outputPower,
	INT_32 targetAlc
	)
{
	INT_32 nChannel, band, b24G = FALSE;
	UINT_32 offset = 0, cck, i, value;

	if(getChannelBand(channelFreq, &nChannel, &band)){
        return ERROR_RFTEST_GENERAL_ERROR;
    }
    if(band == 0) {
        b24G = TRUE;
    }

	if((channelFreq < 1) || (channelFreq > 252)){
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	/* Check EEPROM version and valid mask */
	if(readEEPRom16(EEPROM_LAYOUT_VERSION, &value))
		return ERROR_RFTEST_GENERAL_ERROR;

	if((value & EEPROM_LAYOUT_VERSION_MASK) == 0)
		return ERROR_RFTEST_GENERAL_ERROR;

	if(readEEPRom16(EEPROM_ALC_INFO, &value))
		return ERROR_RFTEST_GENERAL_ERROR;

	if((value & EEPROM_ALC_INFO_ENABLED) == 0)
		return ERROR_RFTEST_GENERAL_ERROR;

	/* EEPROM does not supoort 5G yet. */
	if(b24G == FALSE)
		return ERROR_RFTEST_GENERAL_ERROR;

	if(outputPower > 255 || outputPower < 0)
		return ERROR_RFTEST_GENERAL_ERROR;

	if(targetAlc > 255 || targetAlc < 0)
		return ERROR_RFTEST_GENERAL_ERROR;

	for( i = 0 ; i < sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO) ; i ++ ){
		if((dbAlcOffsetTale[i].rate == rate) &&
			((dbAlcOffsetTale[i].b24G && b24G) ||
			((dbAlcOffsetTale[i].b24G == FALSE) && (b24G == 0)))){
			offset = dbAlcOffsetTale[i].offset;
			cck = dbAlcOffsetTale[i].cck;
			break;
		}
	}

	/* This rate is not found */
	if( i >= sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO)){
		ERRORLOG((TEXT("Can not find the corresponding offset\n")));
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	if(offset < EEPROM_LAYOUT_VERSION){
		ERRORLOG((TEXT("Strang Offset\n")));
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	if(setTXMaxPowerToEEProm(nChannel, cck, gainControl) != 0)
		return ERROR_RFTEST_GENERAL_ERROR;

	value = (targetAlc & EEPROM_DB_ALC_TUPLE_ALC_MASK) |
		((outputPower << 8) & EEPROM_DB_ALC_TUPLE_DB_MASK);
	offset += (nChannel - 1);

	if(writeEEPRom16(offset, value) != ERROR_RFTEST_SUCCESS)
		return ERROR_RFTEST_GENERAL_ERROR;

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  readTxPowerFromEEProm
**
**  descriptions: Retrieve TX Power from EEPROM
**  parameters:
**             nCardIndex: Which adapter.
**             nChannel:    Channel number
**             bCck:          1: CCK 0:OFDM
**             ndBindex:    0: MAX 1:0db 2:5db 3:10db
**  result:
**             nTxPwr:       Points to buffer of TX Power
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::readTxPowerFromEEProm (
    INT_32 channelFreq,
    INT_32 bCck,
    INT_32 *nTxPwr,
    INT_32 ndBindex
    )
{
    INT_32 nChannel, band = FALSE, b24G, result = 0;

	if(getChannelBand(channelFreq, &nChannel, &band)){
        return ERROR_RFTEST_GENERAL_ERROR;
    }

	if (band == 0) {
        b24G = TRUE;
	}

    if (ndBindex == 0) {

		UINT_32 offset, nValue;

	    if((nChannel < 1) || (nChannel > 252)){
	        return ERROR_RFTEST_GENERAL_ERROR;
	    }

	    offset = findMaxPowerByteOffset(nChannel, bCck, b24G);
	    if (offset < 0) {
	        return ERROR_RFTEST_GENERAL_ERROR;
	    }

	    /* offset is byte offset */
	    readEEPRom16(offset/2, &nValue);

	    /* Higher Byte */
	    if ((offset & 1) == 1) {
	        nValue &= 0xFF00;
	        nValue >>= 8;
	        nValue &= 0x00FF;
	        *nTxPwr  = nValue;
	    }
		else {
	        nValue &= 0x00FF;
	        *nTxPwr = nValue;
	    }
    }
	else {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  readTxPowerFromEEPromEx
**
**  descriptions: Retrieve TX Power from EEPROM
**  parameters:
**             nCardIndex:  Which adapter.
**             channelFreq: Frequency
**             rate:        Rate in unit of 500K
**  result:
**             nTxPwr:       Points to buffer of TX Power
**             outputPower:  Points to buffer of output power. In unit of dBm.
**             targetAlc:    Points to buffer of target ALC value.
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::readTxPowerFromEEPromEx(
    INT_32 channelFreq,
    INT_32 rate,
    INT_32 *nTxPwr,
    INT_32 *outputPower,
    INT_32 *targetAlc
    )
{
	INT_32 b24G;
	INT_32 nChannel, band = FALSE;

	if (getChannelBand(channelFreq, &nChannel, &band)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	if(band == 0) {
		b24G = TRUE;
	}

	UINT_32 offset, nValue, i, cck;

    if((nChannel < 1) || (nChannel > 252)) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    /* EEPROM does not supoort 5G yet. */
    if(b24G == FALSE) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    for( i = 0 ; i < sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO) ; i ++ ){
        if((dbAlcOffsetTale[i].rate == rate) &&
            ((dbAlcOffsetTale[i].b24G && b24G) ||
            ((dbAlcOffsetTale[i].b24G == FALSE) && (b24G == 0)))){
            offset = dbAlcOffsetTale[i].offset;
            cck = dbAlcOffsetTale[i].cck;
            break;
        }
    }

    /* This rate is not found */
    if( i >= sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO))
        return ERROR_RFTEST_GENERAL_ERROR;

    if(offset < EEPROM_LAYOUT_VERSION){
        ERRORLOG((TEXT("Strang Offset\n")));
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    if(readTxPowerFromEEProm(nChannel, cck, (INT_32*)&nValue, b24G) != 0)
        return ERROR_RFTEST_GENERAL_ERROR;

    *nTxPwr = nValue;

    offset += (nChannel - 1);

    if(readEEPRom16(offset, &nValue) != ERROR_RFTEST_SUCCESS)
        return ERROR_RFTEST_GENERAL_ERROR;

    *outputPower = (nValue & EEPROM_DB_ALC_TUPLE_DB_MASK) >> 8;
    *targetAlc = (nValue & EEPROM_DB_ALC_TUPLE_ALC_MASK);

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setPacketRx
**
**  descriptions:
**  parameters:
**         nCardIndex: NIC index number
**         condition, set RX condition:
**                    FALSE: disable
**                    TRUE: enable
**         nAntenna:  RX Ant
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setPacketRx(
    INT_32 condition,
    RX_ANT_SEL nAntenna
    )
{
	DEBUGFUNC("CMT5911::setPacketRx");
    INITLOG((_T("\n")));

	UINT_32 txmode;

	txmode = condition ? 1 : 0;

	UINT_32 bbcrValue;

    readBBCR8(BBCR_CR3, &bbcrValue);
    bbcrValue &= ~BITS(4,5);
    switch(nAntenna) {
        case AGC_RX_ANT_SEL:
            break;
        case MPDU_RX_ANT_SEL:
            bbcrValue |= BIT(4);
            break;
        case FIXED_0:
            bbcrValue |= BIT(5);
            break;
        case FIXED_1:
            bbcrValue |= BITS(4,5);
            break;
        default:
            return ERROR_RFTEST_GENERAL_ERROR;
    }
    writeBBCR8(BBCR_CR3, bbcrValue);

	if ( !m_OID->setOID(this,
			OID_IPC_TEST_PACKET_RX,
			(CHAR *)&txmode,
			sizeof(txmode))
			) {
		ERRORLOG((TEXT("Failed to set Start RX")));
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	return ERROR_RFTEST_SUCCESS;

}

INT_32
CMT5911::setEEPromCKSUpdated (void)
{
	UINT_8 checkSum;
	UINT_16 usEEPRDat[512] = {0};
	UINT_32 eepromData;

	for (int i = 0 ; i < 512 / 2 ; i++) {
		readEEPRom16(i, &eepromData);
		usEEPRDat[i] = (UINT_16)eepromData;
	}

	checkSum = nicCalculateChecksumByte(&usEEPRDat[EEPROM_LED_CFG_PHY_MODE],
		(EEPROM_NIC_CHKSUM - EEPROM_LED_CFG_PHY_MODE + 1) * 2 -1);

	UINT_16	eepromValue;

	eepromValue = (UINT_8)(~checkSum);
	eepromValue = (eepromValue << 8) | (UINT_8)usEEPRDat[EEPROM_NIC_CHKSUM];
	writeEEPRom16(EEPROM_NIC_CHKSUM, eepromValue);

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setEEPRomSize
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          value:  size of the EEPRom (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setEEPRomSize (
    INT_32 eepromSz
    )
{
	UINT_32  value;

	switch(eepromSz)
	{
		case 0:
			value = 0x00;
			break;
		case 128:
			value = 0x10;
			break;
		case 256:
			value = 0x20;
			break;
		case 512:
			value = 0x30;
			break;
		case 1024:
			value = 0x40;
			break;
		case 2048:
			value = 0x50;
			break;
		default:
			return ERROR_RFTEST_GENERAL_ERROR;/* illegal value */
	}

	if (writeMCR32(MCR_ECSR, value) == 0) {
		return ERROR_RFTEST_SUCCESS;
	}
	else {
		return ERROR_RFTEST_GENERAL_ERROR;
	}
}

/*_________________________________________________________________________
**  GetEEPromSize
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          value:  size of the EEPRom (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::getEEPRomSize (
	INT_32* eepromSz
	)
{
	DWORD BytesRead;
    IPC_EEPROM_RW_STRUC m_sEEPRom_RW;

	m_sEEPRom_RW.bRead          = TRUE;
    m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_SZ;
    m_sEEPRom_RW.eepromData     = 0;   //clear before read

	if(!m_OID->setOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC))
	        ) {
        ERRORLOG((TEXT("Failed to set configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    if(!m_OID->queryOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC),
	        &BytesRead)
	        ) {
        ERRORLOG((TEXT("Failed to read configuration")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    else {
        *eepromSz = (int)m_sEEPRom_RW.eepromData;
		return ERROR_RFTEST_SUCCESS;
    }
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setMACAddrToEEProm
**
**  descriptions: Save MAC Address to EEPROM
**  parameters:
**             nCardIndex: Which adapter.
**             macAddr:    MAC address
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setMACAddrToEEProm (
    UINT_8* macAddr
    )
{
	UINT_32 eepromValue, i, offset;

	if (!macAddr) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	offset = EEPROM_MAC_ADDR_BYTE_0_1;
	for (i = 0 ; i < 3; i++) {
		eepromValue = macAddr[i*2];
		eepromValue |= ((UINT_16)macAddr[i*2 + 1]) << 8;
		writeEEPRom16(offset, eepromValue);
		offset += 1;
	}

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getEEPRomVersion
**
**  descriptions: Return EEPROM version
**  parameters:
**         nCardIndex: NIC index number
**         fileName: a null terminated string indicate the file to be read
**  output:
**         version: Output buffer for version
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::getEEPRomVersion (
	INT_32* version
	)
{
	INT_32 ver;

	ver = this->getEEPRomVersion_();

	if(ver < 0)
        return ERROR_RFTEST_GENERAL_ERROR;

    *version = ver;
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setCountryToEEProm
**
**  descriptions:
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setCountryToEEProm (
	UINT_8* country
	)
{
	if (country == NULL) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	UINT_16 tmp16;

    if (!country) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    tmp16 = ((UINT_16)country[1])<<8;
    tmp16 |= country[0];

    if (writeEEPRom16(EEPROM_REG_DOMAIN, tmp16)) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setPhyModeToEEProm
**
**  descriptions: Update EEPROM phy mode
**  parameters:
**             nCardIndex: Which adapter.
**             mode:       Phy. mode
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setPhyModeToEEProm (
    INT_32 mode
    )
{
	UINT_32 eepromValue;

	if (readEEPRom16(EEPROM_LED_CFG_PHY_MODE, &eepromValue)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	eepromValue &= ~EEPROM_PHY_MODE_MASK;

	switch(mode)
	{
		case PHY_80211A:
			eepromValue |= EEPROM_PHY_MODE_A;
			break;
		case PHY_80211AG:
			eepromValue |= EEPROM_PHY_MODE_A_G;
			break;
		case PHY_80211G:
			eepromValue |= EEPROM_PHY_MODE_G;
			break;
		case PHY_80211B:
			eepromValue |= EEPROM_PHY_MODE_B;
			break;
		default:
			return ERROR_RFTEST_GENERAL_ERROR;
	}

	if (writeEEPRom16(EEPROM_LED_CFG_PHY_MODE, eepromValue)) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	return ERROR_RFTEST_SUCCESS;

}

INT_32
CMT5911::setDACOffsetToEEProm (
	UINT_16 dacOffset
	)
{
	UINT_16 offsetValue;

    offsetValue = dacOffset;
    offsetValue |= ((UINT_16)(dacOffset>>8)) << 8;

    if (writeEEPRom16(EEPROM_DAC_IQ_OFFSET, offsetValue)) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT5911::getDACOffset (
	UINT_16* dacOffset
	)
{
	UINT_8 dposi, dposq;
	UINT_32 m_2dadci1, m_2dadcq1;

	DACInit();
	DACSetep1(&dposi, &dposq);

	UINT_8	dneqi, dnegq;
	DACSetep2(&dneqi, &dnegq);

	INT_32 ddaci_int, ddacq_int;
	calDADCoffset(
		dposi, dposq, dneqi,  dnegq,
		&ddaci_int, &ddacq_int,
		&m_2dadci1, &m_2dadcq1);

	UINT_32 calID, calQD;
	calID = 15 - ddaci_int;
	calQD = 15 - ddacq_int;

	UINT_8	bestI, bestQ;
	interateDACAdjust(
		(UINT_8)calID, (UINT_8)calQD, m_2dadci1, m_2dadcq1, &bestI, &bestQ);

	UINT_16	result;
	result = bestQ;
	result |= ((UINT_16)bestI) << 8;

	*dacOffset = result;

	return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  setOutputPin
**
**  descriptions: This function sets the particular pin as an output one
**                and the output level
**  parameters:
**      index - card index
**      pinIndex - Pin index
**      outputLevel - output for band
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT5911::setOutputPin(
	INT_32 pinIndex,
	INT_32 outputLevel)
{
    INT_32 result = -1;
    UINT_32 u4ScrValue = 0, u4Value = 0;

	switch(pinIndex)
	{
		case 0: /* GPIO0 */
			//set GPIO setting
		    result = readMCR32(MCR_GPIOCR, &u4Value);
		    if(result)
		        break;

            u4Value &= ~(GPIOCR_GPIO_0_IO_CTRL | GPIOCR_GPIO_0);

		    if(outputLevel)
                u4Value |= GPIOCR_GPIO_0;

	        result = writeMCR32(MCR_GPIOCR, u4Value);
			break;

		case 1: /* GPIO1 */
		    result = readMCR32(MCR_GPIOCR, &u4Value);
		    if(result)
		        break;

            u4Value &= ~(GPIOCR_GPIO_1_IO_CTRL | GPIOCR_GPIO_1);

		    if(outputLevel)
                u4Value |= GPIOCR_GPIO_1;

	        result = writeMCR32(MCR_GPIOCR, u4Value);
			break;

		case 2: /* LED1 */
		    u4Value = LCR_LED_MODE_GPIO | LCR_LED_POLARITY_POSITIVE;
		    if(outputLevel)
                u4Value |= LCR_LED_GPIO_OUTPUT;

	        result = writeMCR32(MCR_LCR, u4Value);
			break;

		case 3: /* HI_INT_N */
		    result = readMCR32(MCR_SCR, &u4ScrValue);
		    if(result)
		        return result;

		    u4ScrValue &= ~(SCR_INT_CTRL_0 | SCR_INT_CTRL_1 | SCR_INT_CTRL_2 | SCR_INT_CTRL_3);
		    u4ScrValue |= SCR_INT_CTRL_0;
		    if(outputLevel){
		        u4ScrValue |= SCR_INT_CTRL_2;
            }

		    result = writeMCR32(MCR_SCR, u4ScrValue);
			break;

		case 4: /* SLOW_INT */

		    result = readMCR32(MCR_SCR, &u4ScrValue);
		    if(result)
		        return result;

		    u4ScrValue &= ~(SCR_INT_CTRL_0 | SCR_INT_CTRL_1 | SCR_INT_CTRL_2 | SCR_INT_CTRL_3);
		    if(outputLevel){
		        u4ScrValue |= SCR_INT_CTRL_3;
            }

		    result = writeMCR32(MCR_SCR, u4ScrValue);
			break;

		case 5: /* WLAN_ACT */
			u4Value = WBR_COEXIST_EN | WBR_ASSERT_MIN_REQ_EN |
		        WBR_ASSERT_THROUGH_ACK_EN | WBR_ASSERT_BEFORE_BEACON_EN |
		        WBR_BLUETOOTH_1ST_PRIORITY_EN;

            /* There is no way to control WLAN_ACT high or low via MCR */
		    result = writeMCR32(MCR_WBCR, u4Value);

			break;

		default:
			break;
	}
    return result;

}

/*******************************************************************************
**  getInputPin
**
**  descriptions: This function set the particular pin as an input pin and
**                returns the input level.
**  parameters:
**      nCardIndex - card index
**      pinIndex - Pin index
**  result:
**      inputLevel - Input level detected by hardware
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT5911::getInputPin(
	INT_32 pinIndex,
	INT_32* inputLevel)
{
	INT_32 result = -1;
	UINT_32 u4Value = 0;

	switch(pinIndex){
		case 0: /* GPIO0 */
			result = readMCR32(MCR_GPIOCR, &u4Value);
			if(result)
				break;

			u4Value &= ~(GPIOCR_GPIO_0_IO_CTRL | GPIOCR_GPIO_0);
			u4Value |= GPIOCR_GPIO_0_IO_CTRL_IN;

			result = writeMCR32(MCR_GPIOCR, u4Value);
			if(result)
				break;

			result = readMCR32(MCR_GPIOCR, &u4Value);
			if(result)
				break;

			if(u4Value & GPIOCR_GPIO_0)
				*inputLevel = 1;
			else
				*inputLevel = 0;
			break;
		case 1: /* GPIO1 */
			result = readMCR32(MCR_GPIOCR, &u4Value);
			if(result)
				break;

			u4Value &= ~(GPIOCR_GPIO_1_IO_CTRL | GPIOCR_GPIO_1);
			u4Value |= GPIOCR_GPIO_1_IO_CTRL_IN;

			result = writeMCR32(MCR_GPIOCR, u4Value);
			if(result)
				break;

			result = readMCR32(MCR_GPIOCR, &u4Value);
			if(result)
				break;

			if(u4Value & GPIOCR_GPIO_1)
				*inputLevel = 1;
			else
				*inputLevel = 0;
			break;

		default:
			break;
	}
	return result;

}

/*******************************************************************************
**  testInputPin
**
**  descriptions: This function sets the particular pin as an input for further tests
**  parameters:
**      nCardIndex - card index
**      pinIndex - Pin index
**      inputLevel - intended test level
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT5911::testInputPin(
	INT_32 pinIndex,
	INT_32	inputLevel)
{
	INT_32 result = -1;
	UINT_32 u4Value = 0;

	switch(pinIndex)
	{
		case 0: /* BT_PRIORITY */
			u4Value = WBR_COEXIST_EN | WBR_ASSERT_MIN_REQ_EN |
		        WBR_ASSERT_THROUGH_ACK_EN | WBR_ASSERT_BEFORE_BEACON_EN |
		        WBR_BLUETOOTH_1ST_PRIORITY_EN;

            /* There is no way to check BT_PR by observing TX done error only. */
	        result = writeMCR32(MCR_WBCR, u4Value);
	        if(result)
				break;
			break;
		default:
			break;
	}
	return result;

}

/*******************************************************************************
**  restoreIOPin
**
**  descriptions: This function retores I/O pin settings
**  parameters:
**      nCardIndex - card index
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT5911::restoreIOPin(void)
{
	INT_32 result = -1;

	result = writeMCR32(MCR_PPMCR, PPMCRSave);
	if(result)
		return result;

	/* Restore original IO register setting */
	result = writeMCR32(MCR_GPIOCR, ioRegSave[0]);
	if(result)
		return result;

	result = writeMCR32(MCR_LCR, ioRegSave[1]);
	if(result)
		return result;

	result = writeMCR32(MCR_WBCR, ioRegSave[2]);
	if(result)
		return result;

	return result;

}

//======================================================================
#if 1
#endif
/*_________________________________________________________________________
**  updateALCOffset
**
**  descriptions: Update ALC Offset table according to EEPROM content
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::updateALCOffset(void)
{
    UINT_32 i, eepromValue, offset, base, result = 0;
    UINT_32 alcInfo = 0;
    BOOL readCCK = FALSE, read24gOfdm = FALSE, read5gOfdm = FALSE;

    if (readEEPRom16(EEPROM_ALC_INFO, &alcInfo) != ERROR_RFTEST_SUCCESS) {
        return -1;
    }

    if (alcInfo & EEPROM_ALC_INFO_24G_VALID) {
        readCCK = TRUE;
        read24gOfdm = TRUE;
    }

    if (alcInfo & EEPROM_ALC_INFO_5G_VALID) {
        read5gOfdm = TRUE;
    }

    if (alcInfo & EEPROM_ALC_INFO_USE_SLOPE_RATIO) {
        readCCK = TRUE;
        read24gOfdm = TRUE;
        read5gOfdm = TRUE;
    }

    for (i = 0; i < sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO) ; i++) {
        if (dbAlcOffsetTale[i].cck && readCCK) {
            if(readEEPRom16(EEPROM_24G_CCK_ALC_OFFSET, &eepromValue)
                == ERROR_RFTEST_SUCCESS) {
                dbAlcOffsetTale[i].offset = eepromValue;
            }
			else {
                result = -1;
                break;
            }
        }
		else {
            if (dbAlcOffsetTale[i].b24G && read24gOfdm) {
                base = EEPROM_24G_OFDM_ALC_OFFSET_START;
            }
			else if(read5gOfdm) {
                base = EEPROM_5G_OFDM_ALC_OFFSET_START;
            } else {
                /* Both 2.4G and 5G are not valid */
                result = -1;
                break;
            }

            offset = base + dbAlcOffsetTale[i].index;
            if (readEEPRom16(offset, &eepromValue)
                == ERROR_RFTEST_SUCCESS) {
                dbAlcOffsetTale[i].offset = eepromValue;
            }
			else {
                result = -1;
                break;
            }
        }
    }

#if DBG
    for ( i = 0 ; i < sizeof(dbAlcOffsetTale)/sizeof(TX_RATE_INFO) ; i ++ ){
        TCHAR outputBuf[256];
        _stprintf(outputBuf, TEXT("index:%d for %s offset:0x%04x\n"),
            i, dbAlcOffsetTale[i].b24G?"2.4G":"5G",dbAlcOffsetTale[i].offset);
        OutputDebugString(outputBuf);
    }
#endif

    return result;
}

/*_________________________________________________________________________
**  findMaxPowerByteOffset
**
**  descriptions:  Given Channel, whether CCK, which turning point and band, to return EEPROM offset
**  parameters:
**              nChannel:    1 ~ 14 for 2.4G, 240 ~ 161
**              bCck:          for CCK for not, only valid when b24Gs is true
**              markIndex:
**  return:
**              -1: Failed to locate,
**              others: zero-based balue to represent turning point to describe TX Power curve
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::findMaxPowerByteOffset(INT_32 nChannel, INT_32 bCck, INT_32 b24Gs)
{
    INT_32 offset;
    INT_32 i;

    if (b24Gs) {
        if((nChannel > 14) || (nChannel < 1)) {
            return -1;
        }

        /* Byte Offset */
        if (bCck) {
            offset = EEPROM_2G_CCK_TXPWR_GAIN_START*2 + nChannel -1;
        }
		else {
            offset = EEPROM_2G_OFDM_TXPWR_GAIN_START*2 + nChannel -1;
        }
        return offset;
    }

    if(bCck)
        return -1;

    for( i=0; i < sizeof(maxPowerChannelOffsetTable)/sizeof(TX_MAX_POWER_OFFSET); i++) {
        if (nChannel == maxPowerChannelOffsetTable[i].nChannel[0]) {
            /* Table's offset is word offset */
            offset = maxPowerChannelOffsetTable[i].offset * 2;
            return offset;
        }
		else if (nChannel == maxPowerChannelOffsetTable[i].nChannel[1]) {
            offset = maxPowerChannelOffsetTable[i].offset * 2 +1;
            return offset;
        }
    }
    return -1;
}


INT_32
CMT5911::setPPMCRAndSCR(void)
{
    UINT_32 dscrTmp;
    UINT_32 dscrTmps;

    readMCR32(MCR_SCR, &dscrTmp);

    writeMCR32(MCR_SCR, dscrTmp&(~SCR_PAU_EN));

    /*1 Tx_Rx: Enable(Hi)
	  2 TR_SW:Tx
	  3 PA_PE:Active
	  4 TX_PE:Active
	  5 TXCLK, TXD, TX_RDY: De_active
	  6 RX_PE, RXCLK, RXD, MD_RDY : De_active*/
    /**/
    INT_32 pa2 = 0, pa5 = 0;

    INT_32 pctx = PPMCR_CONT_TX;

    readMCR32(MCR_PCICR, &dscrTmps);
    if (dscrTmps & PCICR_RF_BAND) {
        // 5 GHz band, enable PA5
        if(dscrTmps & PCICR_PA5_EN_POLARITY){
            pctx |= PPMCR_GPIO_PA5_EN;
        }
        // disable PA2
        if(!(dscrTmps & PCICR_PA2_EN_POLARITY)){
            pctx |= PPMCR_GPIO_PA2_EN;
        }
    }
    else {
        // 2.4 GHz band, enable PA2
        if(dscrTmps & PCICR_PA2_EN_POLARITY){
            pctx |= PPMCR_GPIO_PA2_EN;
        }
        // disable PA5
        if(!(dscrTmps & PCICR_PA5_EN_POLARITY)){
            pctx |= PPMCR_GPIO_PA5_EN;
        }
    }
    writeMCR32(MCR_PPMCR, pctx); /* enable TX_RX, PA_PE, TR_SW = T */
    return 0;
}

INT_32
CMT5911::setBBTxPower(INT_32 txPower)
{
    /* OFDM TX Power */
    writeBBCR8(14, txPower);
    /* CFK TX Power */
    writeBBCR8(15, txPower);
    return 0;
}

INT_32
CMT5911::setTxAnt(INT_32 antenna)
{
    UINT_32 nCr8;

    readBBCR8(8, &nCr8);

    /* set TX antenna from BBCR */
    if (antenna) {
        nCr8 = nCr8 & 0x0FF | 0x02;
        writeBBCR8(8, nCr8);
    } else {
        nCr8 = nCr8 & 0x0FD;
        writeBBCR8(8, nCr8);
    }
    return 0;
}
#if 0 //fifi_add
INT_32
CMT5911::resetBBCRStat(void)
{
    writeBBCR8(97, 0x40);
    writeBBCR8(97, 0x80);
    return 0;
}
#endif

UINT_8
CMT5911::getTxRate(INT_32 nTxRate, INT_32 nPreamble)
{
	UCHAR ucTxRate = 0;
	switch (nTxRate) {
	    default:
		case 2: ucTxRate = 0x00 << 2; break;
  		case 4: ucTxRate = 0x01 << 2; break;
		case 11: ucTxRate = 0x02 << 2; break;
		case 22: ucTxRate = 0x03 << 2; break;
		case 12: ucTxRate = (0x0b << 2)|0x80; break;//OFDM 6M
		case 18: ucTxRate = (0x0f << 2)|0x80; break;//OFDM 9M
		case 24: ucTxRate = (0x0a << 2)|0x80; break;//OFDM 12M
		case 36: ucTxRate = (0x0e << 2)|0x80; break;//OFDM 18M
		case 48: ucTxRate = (0x09 << 2)|0x80; break;//OFDM 24M
		case 72: ucTxRate = (0x0d << 2)|0x80; break;//OFDM 36M
		case 96: ucTxRate = (0x08 << 2)|0x80; break;//OFDM 48M
		case 108: ucTxRate = (0x0c << 2)|0x80; break;//OFDM 54M
    }

	if (nPreamble) {
		ucTxRate |= 0x40;
    }

    return ucTxRate;
}

UINT_8
CMT5911::nicCalculateChecksumByte(void * startingAddr_p,UINT_32 length)
{
    UINT_32 i;
    UINT_8 * cp;
    UINT_8 result;

    result = 0;
    cp = (UINT_8*) startingAddr_p;
    for (i = 0; i < length; i++, cp++) {
        result += *cp;
    }
    return result;
}

/*_________________________________________________________________________
**  getEEPRomVersion_
**
**  descriptions: Return EEPROM version
**  parameters:
**  return:
**      negative, failed
**      0 or posive: version number
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::getEEPRomVersion_(void)
{
    UINT_32 value;
    UINT_32 eepValue;

    if(readMCR32(MCR_ECSR, &value) != ERROR_RFTEST_SUCCESS)
        return -1;

    if((value & ECSR_EE_TYPE) == ECSR_EE_NOT_PRESENT)
        return -1;

    if(readEEPRom16(EEPROM_LAYOUT_VERSION, &eepValue) != ERROR_RFTEST_SUCCESS)
        return -1;

    return eepValue & EEPROM_LAYOUT_VERSION_MASK;
}


INT_32
CMT5911::SetATParam (UINT_32 offset, UINT_32 value)
{
	return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT5911::GetATParam (UINT_32 offset, UINT_32 * value)
{
	return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
*                     D A C   P U B L I C   D A T A
********************************************************************************
*/

NIC_BBCR_CONFIG_ENTRY dacInitTable_11[] = {
	{NULL, BBCR_CR82, 0xA1},
	{NULL, BBCR_CR88, 0x80},
	{NULL, BBCR_CR89, 0x80},
	{NULL, BBCR_CR8, 0x80},
	{NULL, BBCR_CR92, 0x61},
	{NULL, BBCR_CR81, 0x1B},
	{NULL, BBCR_CR83, 0xFB},
	{NULL, BBCR_CR85, 0x00}
};

NIC_BBCR_CONFIG_ENTRY dacStep1Table_11[] = {
	{NULL, BBCR_CR93, 0xAA},
	{NULL, BBCR_CR94, 0x10},
	{NULL, BBCR_CR89, 0x00}
};

NIC_BBCR_CONFIG_ENTRY dacStep2Table_11[] = {
	{NULL, BBCR_CR93, 0xA9},
	{NULL, BBCR_CR94, 0x10},
	{NULL, BBCR_CR89, 0x00}
};

UINT_32
diff(UINT_32 a, UINT_32 b)
{
	if(a>b)
		return a-b;
	return b - a;
}

/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/

void
CMT5911::DACInit(void)
{
    UINT_32 i, value;

    for(i =0 ; i < sizeof(dacInitTable_11)/sizeof(NIC_BBCR_CONFIG_ENTRY); i++){
        writeBBCR8(dacInitTable_11[i].offset, dacInitTable_11[i].value);
    }

    readBBCR8(BBCR_CR84, &value);

    value &= ~BITS(0,1);
    value |= BIT(7);
    writeBBCR8(BBCR_CR84, value);
}

void
CMT5911::DACSetep1(UINT_8* dposi, UINT_8* dposq)
{
    UINT_32 i, value;

    for(i =0 ; i < sizeof(dacStep1Table_11)/sizeof(NIC_BBCR_CONFIG_ENTRY); i++){
        writeBBCR8(dacStep1Table_11[i].offset, dacStep1Table_11[i].value);
    }

    readBBCR8(BBCR_CR106, &value);
    *dposi = (UINT_8)value;

    readBBCR8(BBCR_CR107, &value);
    *dposq = (UINT_8)value;

    writeBBCR8(BBCR_CR89,0x80);
}

void
CMT5911::DACSetep2(UINT_8* dnegi, UINT_8* dnegq)
{
    UINT_32 i, value;

    for(i =0 ; i < sizeof(dacStep2Table_11)/sizeof(NIC_BBCR_CONFIG_ENTRY); i++) {
        writeBBCR8(dacStep2Table_11[i].offset, dacStep2Table_11[i].value);
    }

   readBBCR8(BBCR_CR106, &value);
    *dnegi = (UINT_8)value;

    readBBCR8(BBCR_CR107, &value);
    *dnegq = (UINT_8)value;

    writeBBCR8(BBCR_CR89,0x80);
}

void
CMT5911::calDADCoffset(
    UINT_8 dposi,
    UINT_8 dposq,
    UINT_8 dnegi,
    UINT_8 dnegq,
    INT_32* ddaci_int,
    INT_32* ddacq_int,
    UINT_32* double_dadci1,
    UINT_32* double_dadcq1)
{
    UINT_32 value;
    INT_32 divider;

    readBBCR8(BBCR_CR84, &value);

    switch(value & BITS(4,5)){
        case 0x00:
            divider = 6;
            break;
        case 0x10:
            divider = 8;
            break;
        case 0x20:
            divider = 10;
            break;
        case 0x30:
            divider = 11;
            break;
    }

    *double_dadci1 = dposi + dnegi;
    *double_dadcq1 = dposq + dnegq;

#if DBG
    char output[256];
    sprintf(output, "ddaci: Int Deci %d", (dposi - dnegi)*5/(2* divider));
    OutputDebugString(output);
#endif

    INT_32 diff, result;

    if(dposi > dnegi)
        diff = dposi - dnegi;
    else
        diff = dnegi - dposi;

    result = (diff *5/divider);
    if(result % 1){
        result = diff/2 +1;
    }else
        result = diff/2;

    if((dposi < dnegi) && (result != 0))
        *ddaci_int = (result * -1);
    else
        *ddaci_int = result;

#if DBG
    sprintf(output, "ddaci: diff %d dir:%s", result, dposi > dnegi?"+":"-");
    OutputDebugString(output);
#endif

    if(dposq > dnegq)
        diff = dposq - dnegq;
    else
        diff = dnegq - dposq;

    result = (diff *5/divider);
    if(result % 1){
        result = diff/2 +1;
    }else
        result = diff/2;

    if((dposq < dnegq) && (result != 0))
        *ddacq_int = (result * -1);
    else{
        *ddacq_int = result;
    }

#if DBG
    sprintf(output, "ddaci: result %d ddacq_int:%d", result, *ddacq_int);
    OutputDebugString(output);
#endif
}


void
CMT5911::DACSetep4(
    UINT_8 txosID,
    UINT_8 txosQD,
    UINT_8* dadci,
    UINT_8* dadcq)
{
    UINT_32 cr81, cr83, cr84;

    readBBCR8(BBCR_CR81, &cr81);
    readBBCR8(BBCR_CR83, &cr83);
    readBBCR8(BBCR_CR84, &cr84);

    cr81 &= ~BITS(0,1);
    cr83 &= ~BITS(0,1);
    cr83 &= ~BITS(4,7);
    cr84 &= ~BITS(0,1);

    if(txosID & BIT(4))
        cr84 |= BIT(1);

    if(txosID & BIT(3))
        cr81 |= BIT(0);

    if(txosID & BIT(2))
        cr81 |= BIT(1);

    if(txosID & BIT(1))
        cr83 |= BIT(6);

    if(txosID & BIT(0))
        cr83 |= BIT(7);

    if(txosQD & BIT(4))
        cr84 |= BIT(0);

    if(txosQD & BIT(3))
        cr83 |= BIT(0);

    if(txosQD & BIT(2))
        cr83 |= BIT(1);

    if(txosQD & BIT(1))
        cr83 |= BIT(4);

    if(txosQD & BIT(0))
        cr83 |= BIT(5);

    writeBBCR8(BBCR_CR81,cr81);
    writeBBCR8(BBCR_CR83,cr83);
    writeBBCR8(BBCR_CR84,cr84);

    DACSetep1(dadci, dadcq);
}

void
CMT5911::interateDACAdjust(
    UINT_8 valueID,
    UINT_8 valueQD,
    UINT_32 targetIx2,
    UINT_32 targetQx2,
    UINT_8* bestI,
    UINT_8* bestQ)
{

    /* 0 for (0 0) 1 for (1, 1) 2 for (-1, +1) 3 for (-1, -1) 4 for (+1, -1)*/
    UINT_32 diff_i[5];
    UINT_32 diff_q[5];
    int mapI[] = {0, 1, -1, -1, 1};
    int mapQ[] = {0, 1, 1, -1, -1};
    UINT_8 dadci, dadcq;
    int i;

    for(i = 0 ; i < 5 ; i ++) {
        diff_i[i] = diff_q[i] = 0;

        /* Handling extreme cases, when initial guess is 0, then we won't test minus one. */
        if((valueID == 0) && mapI[i] < 0 ) {
                mapI[i] = 0;
        }
        if((valueQD == 0) && mapQ[i] < 0 ) {
                mapQ[i] = 0;
        }
    }

    for( i = 0 ; i < 10 ; i ++) {
        /* 0 , 0 */
        DACSetep4((UINT_8)valueID, (UINT_8)valueQD, &dadci, &dadcq);
        diff_i[0] += diff(targetIx2, 2 *(UINT)dadci);
        diff_q[0] += diff(targetQx2, 2 *(UINT)dadcq);

        DACSetep4( (UINT_8)valueID + mapI[1], (UINT_8)valueQD + mapQ[1], &dadci, &dadcq);
        diff_i[1] += diff(targetIx2, 2 *(UINT)dadci);
        diff_q[1] += diff(targetQx2, 2 *(UINT)dadcq);

        DACSetep4( (UINT_8)((UINT)valueID + mapI[2]), (UINT_8)valueQD + mapQ[2], &dadci, &dadcq);
        diff_i[2] += diff(targetIx2, 2 *(UINT)dadci);
        diff_q[2] += diff(targetQx2, 2 *(UINT)dadcq);

        DACSetep4( (UINT_8)((UINT)valueID + mapI[3]), (UINT_8)((UINT)valueQD + mapQ[3]), &dadci, &dadcq);
        diff_i[3] += diff(targetIx2, 2 *(UINT)dadci);
        diff_q[3] += diff(targetQx2, 2 *(UINT)dadcq);

        DACSetep4( (UINT_8)valueID + mapI[4], (UINT_8)((UINT)valueQD + mapQ[4]), &dadci, &dadcq);
        diff_i[4] += diff(targetIx2, 2 *(UINT)dadci);
        diff_q[4] += diff(targetQx2, 2 *(UINT)dadcq);
    }

    *bestI = 0;
    for(i = 1; i < 5 ; i ++) {
        if(diff_i[*bestI] > diff_i[i]) {
            *bestI = i;
        }
    }
    *bestI = valueID + mapI[*bestI] ;

    *bestQ = 0;
    for(i = 1; i < 5 ; i ++) {
        if(diff_q[*bestQ] > diff_q[i]) {
            *bestQ = i;
        }
    }
    *bestQ = valueQD+ mapQ[*bestQ];

#if DBG
    char input[256];
    sprintf(input, "Best I, %d Q :%d valueI %d valueQ %d\n",
        *bestI , *bestQ, valueID, valueQD);
    OutputDebugString(input);

    for(i = 0 ; i < 5; i ++) {
        sprintf(input, "Diff index:%d I %d Q%d\n", i, diff_i[i], diff_q[i]);
        OutputDebugString(input);
    }
#endif
}

/*_________________________________________________________________________
**  setChipType
**
**  descriptions: Config Chip type from UI for EEPROM chksum calculation.
**  parameters:
**  return:
**      negative, failed
**      0 or posive: version number
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setChipType (UINT_32 u4InChipType) 
{
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setEEPRomFromFile
**
**  descriptions: Burn EEPRom by reading the data from file
**  parameters:
**         nCardIndex: NIC index number
**         fileName: a null terminated string indicate the file to be read
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setEEPRomFromFile(TCHAR* fileName)
{
	INT_32 iSize, status;
	INT_32 i;
	UINT_16	dacOffset;
	UINT_16* usEEPRDat = NULL;

	status = getEEPRomSize(&iSize);
    if (status < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    usEEPRDat = (UINT_16*) new char[iSize];

    INT_32 nNum = ReadEepDataFromFile(fileName,
        usEEPRDat, iSize/sizeof(UINT_16));

    if (nNum <= 0 ) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    for (i = 0; i < iSize / 2; i++) {
        if ( writeEEPRom16(i, usEEPRDat[i]) ) {
            ERRORLOG((TEXT("Fail to read write")));
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }

    for (i = 0; i < iSize / 2; i++) {
        UINT_32 value;
        if ( readEEPRom16(i, &value) ) {
            ERRORLOG((TEXT("Fail to read EEPROM back")));
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }

        /* Fail  */
        if (value != usEEPRDat[i]) {
            ERRORLOG((TEXT("Read EEPROM value after write fails\n")));
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }

    if (getDACOffset(&dacOffset)) {
        ERRORLOG((TEXT("Fail to read DAC")));
        delete [] usEEPRDat;
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    if (setDACOffsetToEEProm(dacOffset)) {
        ERRORLOG((TEXT("Fail to update DAC in EEPROM")));
        delete [] usEEPRDat;
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    if (setEEPromCKSUpdated()) {
        ERRORLOG((TEXT("Fail to update NIC EEPROM checksum")));
        delete [] usEEPRDat;
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setXtalFreqTrimToEEProm
**
**  descriptions: Save Xtal Frequency offet to EEPROM
**  parameters:
**             i4FreqTrim:   offset set value
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setXtalFreqTrimToEEProm (
	INT_32 i4FreqTrim
	)
{
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setRcpiOffsetToEEProm
**
**  descriptions: Save Per Channel RCPI offset to EEPROM
**  parameters:
**             channelFreq:    Channel frequence in kHz
**             offset:        rcpi offset -128~+127
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setRcpiOffsetToEEProm (
	INT_32 channelFreq,
	INT_32 offset
	)
{
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setXtalTrimToCr
**
**  descriptions: Set Xtal Trim value to RFCR
**  parameters:
**             u4Value:       frequency trim value
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setXtalTrimToCr (
    UINT_32 u4Value
	)
{
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  queryThermoInfo
**
**  descriptions: Query Adapter's thermo information
**  parameters:
**             pi4Enable:       0: thermo disable. 1: thermo enable
**             pu4RawVal:       the thermo raw data
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::queryThermoInfo (
    INT_32 *  pi4Enable,
    UINT_32 * pu4RawVal
	)
{
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setThermoEn
**
**  descriptions: Set Adapter's thermo function enable/disable
**  parameters:
**             i4Enable:       0: thermo disable. 1: thermo enable
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT5911::setThermoEn (
    INT_32    i4Enable
	)
{
	return ERROR_RFTEST_SUCCESS;

}


/*_________________________________________________________________________
**  setAnritsu8860bTestSupportEn
**
**  descriptions: Set Adapter to enable/disable Anritsu test support
**  parameters:
**             i4Enable:       0: disable. 1: enable
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**        When enable, it will disable roaming and "No Ack when overflow".
**        When disable, it will restore original raoming and NoAck setting.
**__________________________________________________________________________*/
INT_32
CMT5911::setAnritsu8860bTestSupportEn (
    INT_32    i4Enable
	)
{
    return ERROR_RFTEST_SUCCESS;
}
