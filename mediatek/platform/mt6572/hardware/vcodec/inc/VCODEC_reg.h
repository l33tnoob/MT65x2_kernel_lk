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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _VCODEC_REG__
#define _VCODEC_REG__
#define VDEC_BASE                                           0xC2084000

#define VDEC_RESET                                          *(volatile unsigned int *)(VDEC_BASE +0x0)
#define VDEC_START                                          *(volatile unsigned int *)(VDEC_BASE +0x4)
#define VDEC_INT_MSK                                        *(volatile unsigned int *)(VDEC_BASE +0x8)
#define VDEC_INT_ACK                                        *(volatile unsigned int *)(VDEC_BASE +0xC)
#define VDEC_INT_STA                                        *(volatile unsigned int *)(VDEC_BASE +0x10)
#define VDEC_CONFIG                                         *(volatile unsigned int *)(VDEC_BASE +0x14)
#define VDEC_PIC_SIZE                                       *(volatile unsigned int *)(VDEC_BASE +0x18)
#define VDEC_MBIF_SIZE                                      *(volatile unsigned int *)(VDEC_BASE +0x1C)
#define VDEC_POST_BS                                        *(volatile unsigned int *)(VDEC_BASE +0x20)
#define VDEC_DMACFG                                         *(volatile unsigned int *)(VDEC_BASE +0x24)
#define VDEC_CMDCFG                                         *(volatile unsigned int *)(VDEC_BASE +0x28)
#define VDEC_REC_YADDR                                      *(volatile unsigned int *)(VDEC_BASE +0x30)
#define VDEC_REC_UADDR                                      *(volatile unsigned int *)(VDEC_BASE +0x34)
#define VDEC_REC_VADDR                                      *(volatile unsigned int *)(VDEC_BASE +0x38)
#define VDEC_DB_YADDR                                       *(volatile unsigned int *)(VDEC_BASE +0x3C)
#define VDEC_DB_UADDR                                       *(volatile unsigned int *)(VDEC_BASE +0x40)
#define VDEC_DB_VADDR                                       *(volatile unsigned int *)(VDEC_BASE +0x44)
#define VDEC_MBIF_ADDR                                      *(volatile unsigned int *)(VDEC_BASE +0x48)
#define VDEC_CTRL_STA                                       *(volatile unsigned int *)(VDEC_BASE +0x50)
#define VDEC_PROC_STA                                       *(volatile unsigned int *)(VDEC_BASE +0x54)
#define VDEC_POST_STA0                                      *(volatile unsigned int *)(VDEC_BASE +0x58)
#define VDEC_POST_STA1                                      *(volatile unsigned int *)(VDEC_BASE +0x5C)
#define VDEC_POST_STA2                                      *(volatile unsigned int *)(VDEC_BASE +0x60)
#define VDEC_POST_STA3                                      *(volatile unsigned int *)(VDEC_BASE +0x64)
#define VDEC_IDMA_START                                     *(volatile unsigned int *)(VDEC_BASE +0x80)
#define VDEC_IDMA_PAC_ADDR                                  *(volatile unsigned int *)(VDEC_BASE +0x84)
#define VDEC_IDMA_EXT_ADDR                                  *(volatile unsigned int *)(VDEC_BASE +0x88)
#define VDEC_IDMA_STA0                                      *(volatile unsigned int *)(VDEC_BASE +0x8C)
#define VDEC_IDMA_STA1                                      *(volatile unsigned int *)(VDEC_BASE +0x90)
#define VDEC_IDMA_STA2                                      *(volatile unsigned int *)(VDEC_BASE +0x94)
#define VDEC_IDMA_GADDR                                     *(volatile unsigned int *)(VDEC_BASE +0x98)
#define VDEC_POST_GR                                        *(volatile unsigned int *)(VDEC_BASE +0x400)
#define VDEC_POST_DM                                        *(volatile unsigned int *)(VDEC_BASE +0x800)

#define VDEC_PITCH                                          *(volatile unsigned int *)(VDEC_BASE +0x300)
#define VDEC_MSG_LOG                                        *(volatile unsigned int *)(VDEC_BASE +0x310)

#define VDEC_VDEC_PRINTF_0                                  *(volatile unsigned int *)(VDEC_BASE +0x320)
#define VDEC_VDEC_PRINTF_1                                  *(volatile unsigned int *)(VDEC_BASE +0x324)
#define VDEC_VDEC_PRINTF_2                                  *(volatile unsigned int *)(VDEC_BASE +0x328)
#define VDEC_VDEC_PRINTF_3                                  *(volatile unsigned int *)(VDEC_BASE +0x32c)
#define VDEC_VDEC_PRINTF_4                                  *(volatile unsigned int *)(VDEC_BASE +0x330)
#define VDEC_VDEC_PRINTF_5                                  *(volatile unsigned int *)(VDEC_BASE +0x334)
#define VDEC_VDEC_PRINTF_6                                  *(volatile unsigned int *)(VDEC_BASE +0x338)
#define VDEC_VDEC_PRINTF_7                                  *(volatile unsigned int *)(VDEC_BASE +0x33c)
#define VDEC_VDEC_PRINTF_8                                  *(volatile unsigned int *)(VDEC_BASE +0x340)
#define VDEC_VDEC_PRINTF_9                                  *(volatile unsigned int *)(VDEC_BASE +0x344)
#define VDEC_VDEC_PRINTF_10                                 *(volatile unsigned int *)(VDEC_BASE +0x348)
#define VDEC_VDEC_PRINTF_11                                 *(volatile unsigned int *)(VDEC_BASE +0x34c)
#define VDEC_VDEC_PRINTF_12                                 *(volatile unsigned int *)(VDEC_BASE +0x350)
#define VDEC_VDEC_PRINTF_13                                 *(volatile unsigned int *)(VDEC_BASE +0x354)
#define VDEC_VDEC_PRINTF_14                                 *(volatile unsigned int *)(VDEC_BASE +0x358)
#define VDEC_VDEC_PRINTF_15                                 *(volatile unsigned int *)(VDEC_BASE +0x35c)
#define VDEC_VDEC_PRINTF_16                                 *(volatile unsigned int *)(VDEC_BASE +0x360)
#define VDEC_VDEC_PRINTF_17                                 *(volatile unsigned int *)(VDEC_BASE +0x364)
#define VDEC_VDEC_PRINTF_18                                 *(volatile unsigned int *)(VDEC_BASE +0x368)
#define VDEC_VDEC_PRINTF_19                                 *(volatile unsigned int *)(VDEC_BASE +0x36c)
#define VDEC_VDEC_PRINTF_20                                 *(volatile unsigned int *)(VDEC_BASE +0x370)
#define VDEC_VDEC_PRINTF_21                                 *(volatile unsigned int *)(VDEC_BASE +0x374)
#define VDEC_VDEC_PRINTF_22                                 *(volatile unsigned int *)(VDEC_BASE +0x378)
#define VDEC_VDEC_PRINTF_23                                 *(volatile unsigned int *)(VDEC_BASE +0x37c)
#define VDEC_VDEC_PRINTF_24                                 *(volatile unsigned int *)(VDEC_BASE +0x380)
#define VDEC_VDEC_PRINTF_25                                 *(volatile unsigned int *)(VDEC_BASE +0x384)
#define VDEC_VDEC_PRINTF_26                                 *(volatile unsigned int *)(VDEC_BASE +0x388)
#define VDEC_VDEC_PRINTF_27                                 *(volatile unsigned int *)(VDEC_BASE +0x38c)
#define VDEC_VDEC_PRINTF_28                                 *(volatile unsigned int *)(VDEC_BASE +0x390)
#define VDEC_VDEC_PRINTF_29                                 *(volatile unsigned int *)(VDEC_BASE +0x394)
#define VDEC_VDEC_PRINTF_30                                 *(volatile unsigned int *)(VDEC_BASE +0x398)
#define VDEC_VDEC_PRINTF_31                                 *(volatile unsigned int *)(VDEC_BASE +0x39c)


#define VENC_BASE                                           0xc2085000

#define VENC_FRAME_START                                    *(volatile unsigned int *)(VENC_BASE +0x0)
#define VENC_SLICE_START                                    *(volatile unsigned int *)(VENC_BASE +0x4)
#define VENC_MBX_LMT                                        *(volatile unsigned int *)(VENC_BASE +0x8)
#define VENC_MBY_LMT                                        *(volatile unsigned int *)(VENC_BASE +0xC)
#define VENC_MBX_STOP                                       *(volatile unsigned int *)(VENC_BASE +0x10)
#define VENC_MBY_STOP                                       *(volatile unsigned int *)(VENC_BASE +0x14)
#define VENC_VOP_TYPE                                       *(volatile unsigned int *)(VENC_BASE +0x18)
#define VENC_SHRT_VDO                                       *(volatile unsigned int *)(VENC_BASE +0x1C)
#define VENC_FCODE                                          *(volatile unsigned int *)(VENC_BASE +0x20)
#define VENC_BCODE                                          *(volatile unsigned int *)(VENC_BASE +0x24)
#define VENC_RND_CTRL                                       *(volatile unsigned int *)(VENC_BASE +0x28)
#define VENC_INIT_BITH                                      *(volatile unsigned int *)(VENC_BASE +0x2C)
#define VENC_INIT_BITL                                      *(volatile unsigned int *)(VENC_BASE +0x30)
#define VENC_BIT_IDX                                        *(volatile unsigned int *)(VENC_BASE +0x34)
#define VENC_SIDE_ADDR                                      *(volatile unsigned int *)(VENC_BASE +0x38)
#define VENC_SRCADR_Y                                       *(volatile unsigned int *)(VENC_BASE +0x3C)
#define VENC_SRCADR_CB                                      *(volatile unsigned int *)(VENC_BASE +0x40)
#define VENC_SRCADR_CR                                      *(volatile unsigned int *)(VENC_BASE +0x44)
#define VENC_RECADR_Y                                       *(volatile unsigned int *)(VENC_BASE +0x48)
#define VENC_RECADR_CB                                      *(volatile unsigned int *)(VENC_BASE +0x4C)
#define VENC_RECADR_CR                                      *(volatile unsigned int *)(VENC_BASE +0x50)
#define VENC_REFADR_Y                                       *(volatile unsigned int *)(VENC_BASE +0x54)
#define VENC_REFADR_CB                                      *(volatile unsigned int *)(VENC_BASE +0x58)
#define VENC_REFADR_CR                                      *(volatile unsigned int *)(VENC_BASE +0x5C)
#define VENC_BITADR                                         *(volatile unsigned int *)(VENC_BASE +0x60)
#define VENC_ENC_STATUS                                     *(volatile unsigned int *)(VENC_BASE +0x64)
#define VENC_IRQ_EN                                         *(volatile unsigned int *)(VENC_BASE +0x68)
#define VENC_MC_CTRL                                        *(volatile unsigned int *)(VENC_BASE +0x6C)
#define VENC_WDMA_CTRL                                      *(volatile unsigned int *)(VENC_BASE +0x70)
#define VENC_IRQ_ACK                                        *(volatile unsigned int *)(VENC_BASE +0x78)
#define VENC_IRQ_STATUS                                     *(volatile unsigned int *)(VENC_BASE +0x7C)
#define VENC_BYTE_COUNT                                     *(volatile unsigned int *)(VENC_BASE +0x80)
#define VENC_BIT_COUNT                                      *(volatile unsigned int *)(VENC_BASE +0x84)
#define VENC_ZERO_COEF_COUNT                                *(volatile unsigned int *)(VENC_BASE +0x88)
#define VENC_QP                                             *(volatile unsigned int *)(VENC_BASE +0x8C)
#define VENC_DEBUG0                                         *(volatile unsigned int *)(VENC_BASE +0xA0)
#define VENC_DEBUG1                                         *(volatile unsigned int *)(VENC_BASE +0xA4)
#define VENC_DEBUG2                                         *(volatile unsigned int *)(VENC_BASE +0xA8)
#define VENC_DEBUG3                                         *(volatile unsigned int *)(VENC_BASE +0xAC)
#define VENC_DEBUG4                                         *(volatile unsigned int *)(VENC_BASE +0xB0)
#define VENC_DEBUG5                                         *(volatile unsigned int *)(VENC_BASE +0xB4)
#define VENC_CHECKSUM0                                      *(volatile unsigned int *)(VENC_BASE +0xB8)
#define VENC_CHECKSUM1                                      *(volatile unsigned int *)(VENC_BASE +0xBC)
#define VENC_CHECKSUM2                                      *(volatile unsigned int *)(VENC_BASE +0xC0)
#define VENC_CHECKSUM3                                      *(volatile unsigned int *)(VENC_BASE +0xC4)
#define VENC_CHECKSUM4                                      *(volatile unsigned int *)(VENC_BASE +0xC8)
#define VENC_CHECKSUM5                                      *(volatile unsigned int *)(VENC_BASE +0xCC)




#define VDEC_RESET_addr                                     (VDEC_BASE +0x0)
#define VDEC_START_addr                                     (VDEC_BASE +0x4)
#define VDEC_INT_MSK_addr                                   (VDEC_BASE +0x8)
#define VDEC_INT_ACK_addr                                   (VDEC_BASE +0xC)
#define VDEC_INT_STA_addr                                   (VDEC_BASE +0x10)
#define VDEC_CONFIG_addr                                    (VDEC_BASE +0x14)
#define VDEC_PIC_SIZE_addr                                  (VDEC_BASE +0x18)
#define VDEC_MBIF_SIZE_addr                                 (VDEC_BASE +0x1C)
#define VDEC_POST_BS_addr                                   (VDEC_BASE +0x20)
#define VDEC_DMACFG_addr                                    (VDEC_BASE +0x24)
#define VDEC_CMDCFG_addr                                    (VDEC_BASE +0x28)
#define VDEC_REC_YADDR_addr                                 (VDEC_BASE +0x30)
#define VDEC_REC_UADDR_addr                                 (VDEC_BASE +0x34)
#define VDEC_REC_VADDR_addr                                 (VDEC_BASE +0x38)
#define VDEC_DB_YADDR_addr                                  (VDEC_BASE +0x3C)
#define VDEC_DB_UADDR_addr                                  (VDEC_BASE +0x40)
#define VDEC_DB_VADDR_addr                                  (VDEC_BASE +0x44)
#define VDEC_MBIF_ADDR_addr                                 (VDEC_BASE +0x48)
#define VDEC_CTRL_STA_addr                                  (VDEC_BASE +0x50)
#define VDEC_PROC_STA_addr                                  (VDEC_BASE +0x54)
#define VDEC_POST_STA0_addr                                 (VDEC_BASE +0x58)
#define VDEC_POST_STA1_addr                                 (VDEC_BASE +0x5C)
#define VDEC_POST_STA2_addr                                 (VDEC_BASE +0x60)
#define VDEC_POST_STA3_addr                                 (VDEC_BASE +0x64)
#define VDEC_IDMA_START_addr                                (VDEC_BASE +0x80)
#define VDEC_IDMA_PAC_ADDR_addr                             (VDEC_BASE +0x84)
#define VDEC_IDMA_EXT_ADDR_addr                             (VDEC_BASE +0x88)
#define VDEC_IDMA_STA0_addr                                 (VDEC_BASE +0x8C)
#define VDEC_IDMA_STA1_addr                                 (VDEC_BASE +0x90)
#define VDEC_IDMA_STA2_addr                                 (VDEC_BASE +0x94)
#define VDEC_IDMA_GADDR_addr                                (VDEC_BASE +0x98)
#define VDEC_POST_GR_addr                                   (VDEC_BASE +0x400)
#define VDEC_POST_DM_addr                                   (VDEC_BASE +0x800)

#define VDEC_PITCH_addr                                     (VDEC_BASE +0x300)
#define VDEC_MSG_LOG_addr                                   (VDEC_BASE +0x310)
#define VDEC_VDEC_PRINTF_addr                               (VDEC_BASE +0x320)

#define VENC_FRAME_START_addr                               (VENC_BASE +0x0)
#define VENC_SLICE_START_addr                               (VENC_BASE +0x4)
#define VENC_MBX_LMT_addr                                   (VENC_BASE +0x8)
#define VENC_MBY_LMT_addr                                   (VENC_BASE +0xC)
#define VENC_MBX_STOP_addr                                  (VENC_BASE +0x10)
#define VENC_MBY_STOP_addr                                  (VENC_BASE +0x14)
#define VENC_VOP_TYPE_addr                                  (VENC_BASE +0x18)
#define VENC_SHRT_VDO_addr                                  (VENC_BASE +0x1C)
#define VENC_FCODE_addr                                     (VENC_BASE +0x20)
#define VENC_BCODE_addr                                     (VENC_BASE +0x24)
#define VENC_RND_CTRL_addr                                  (VENC_BASE +0x28)
#define VENC_INIT_BITH_addr                                 (VENC_BASE +0x2C)
#define VENC_INIT_BITL_addr                                 (VENC_BASE +0x30)
#define VENC_BIT_IDX_addr                                   (VENC_BASE +0x34)
#define VENC_SIDE_ADDR_addr                                 (VENC_BASE +0x38)
#define VENC_SRCADR_Y_addr                                  (VENC_BASE +0x3C)
#define VENC_SRCADR_CB_addr                                 (VENC_BASE +0x40)
#define VENC_SRCADR_CR_addr                                 (VENC_BASE +0x44)
#define VENC_RECADR_Y_addr                                  (VENC_BASE +0x48)
#define VENC_RECADR_CB_addr                                 (VENC_BASE +0x4C)
#define VENC_RECADR_CR_addr                                 (VENC_BASE +0x50)
#define VENC_REFADR_Y_addr                                  (VENC_BASE +0x54)
#define VENC_REFADR_CB_addr                                 (VENC_BASE +0x58)
#define VENC_REFADR_CR_addr                                 (VENC_BASE +0x5C)
#define VENC_BITADR_addr                                    (VENC_BASE +0x60)
#define VENC_ENC_STATUS_addr                                (VENC_BASE +0x64)
#define VENC_IRQ_EN_addr                                    (VENC_BASE +0x68)
#define VENC_MC_CTRL_addr                                   (VENC_BASE +0x6C)
#define VENC_WDMA_CTRL_addr                                 (VENC_BASE +0x70)
#define VENC_IRQ_ACK_addr                                   (VENC_BASE +0x78)
#define VENC_IRQ_STATUS_addr                                (VENC_BASE +0x7C)
#define VENC_BYTE_COUNT_addr                                (VENC_BASE +0x80)
#define VENC_BIT_COUNT_addr                                 (VENC_BASE +0x84)
#define VENC_ZERO_COEF_COUNT_addr                           (VENC_BASE +0x88)
#define VENC_QP_addr                                        (VENC_BASE +0x8C)
#define VENC_DEBUG0_addr                                    (VENC_BASE +0xA0)
#define VENC_DEBUG1_addr                                    (VENC_BASE +0xA4)
#define VENC_DEBUG2_addr                                    (VENC_BASE +0xA8)
#define VENC_DEBUG3_addr                                    (VENC_BASE +0xAC)
#define VENC_DEBUG4_addr                                    (VENC_BASE +0xB0)
#define VENC_DEBUG5_addr                                    (VENC_BASE +0xB4)
#define VENC_CHECKSUM0_addr                                 (VENC_BASE +0xB8)
#define VENC_CHECKSUM1_addr                                 (VENC_BASE +0xBC)
#define VENC_CHECKSUM2_addr                                 (VENC_BASE +0xC0)
#define VENC_CHECKSUM3_addr                                 (VENC_BASE +0xC4)
#define VENC_CHECKSUM4_addr                                 (VENC_BASE +0xC8)
#define VENC_CHECKSUM5_addr                                 (VENC_BASE +0xCC)
/*
// Traffic Generator
#define TG_MMSYS1_addr                                      0xc2098000
#define TG_MD_PERI_addr                                     0xc20a0000

#ifdef WIN32
#if defined(__cplusplus) || defined(c_plusplus)
  extern "C" void WRITE_REG(unsigned int addr, unsigned int value);
  extern "C" unsigned int READ_REG(unsigned int addr);
#else
  void WRITE_REG(unsigned int addr, unsigned int value);
  unsigned int READ_REG(unsigned int addr);
#endif
#else
  #define WRITE_REG(addr, value)      (*(volatile unsigned int *)(addr) = (unsigned int)(value))
  #define READ_REG(addr)              (*(volatile unsigned int *)(addr))
  #define VDEC_VDEC_PRINTF(index, value)    VDEC_VDEC_PRINTF_##index = (unsigned int)(value);
#endif
*/
#endif

