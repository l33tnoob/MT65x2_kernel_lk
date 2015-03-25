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

#include "typedefs.h"
#include "platform.h"
#include "mt8193.h"

bool mt8193_CKGEN_AgtOnClk(e_CLK_T eAgt)
{
    u32 u4Tmp;

    printf("mt8193_CKGEN_AgtOnClk() %d\n", eAgt);

        
    switch (eAgt)
    {
       case e_CLK_NFI:
            u4Tmp = CKGEN_READ32(REG_RW_NFI_CKCFG);
            CKGEN_WRITE32(REG_RW_NFI_CKCFG, u4Tmp & (~CLK_PDN_NFI));
            break;
        case e_CLK_HDMIPLL:
            u4Tmp = CKGEN_READ32(REG_RW_HDMI_PLL_CKCFG);
            CKGEN_WRITE32(REG_RW_HDMI_PLL_CKCFG, u4Tmp & (~CLK_PDN_HDMI_PLL));
            break;
          case e_CLK_HDMIDISP:
            u4Tmp = CKGEN_READ32(REG_RW_HDMI_DISP_CKCFG);
            CKGEN_WRITE32(REG_RW_HDMI_DISP_CKCFG, u4Tmp & (~CLK_PDN_HDMI_DISP));
            break;
          case e_CLK_LVDSDISP:
            u4Tmp = CKGEN_READ32(REG_RW_LVDS_DISP_CKCFG);
            CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp & (~CLK_PDN_LVDS_DISP));
            break;
          case e_CLK_LVDSCTS:
            u4Tmp = CKGEN_READ32(REG_RW_LVDS_CTS_CKCFG);
            CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp & (~CLK_PDN_LVDS_CTS));
            break;
        default:
            return FALSE;
    }
        
       
    return TRUE;
}


bool mt8193_CKGEN_AgtOffClk(e_CLK_T eAgt)
{
      u32 u4Tmp;

      printf("mt8193_CKGEN_AgtOffClk() %d\n", eAgt);

    switch (eAgt)
    {
        case e_CLK_NFI:
            u4Tmp = CKGEN_READ32(REG_RW_NFI_CKCFG);
            CKGEN_WRITE32(REG_RW_NFI_CKCFG, u4Tmp | CLK_PDN_NFI);
            break;
        case e_CLK_HDMIPLL:
            u4Tmp = CKGEN_READ32(REG_RW_HDMI_PLL_CKCFG);
            CKGEN_WRITE32(REG_RW_HDMI_PLL_CKCFG, u4Tmp | CLK_PDN_HDMI_PLL);
            break;
        case e_CLK_HDMIDISP:
            u4Tmp = CKGEN_READ32(REG_RW_HDMI_DISP_CKCFG);
            CKGEN_WRITE32(REG_RW_HDMI_DISP_CKCFG, u4Tmp | CLK_PDN_HDMI_DISP);
            break;
        case e_CLK_LVDSDISP:
            u4Tmp = CKGEN_READ32(REG_RW_LVDS_DISP_CKCFG);
            CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp | CLK_PDN_LVDS_DISP);
            break;
        case e_CLK_LVDSCTS:
            u4Tmp = CKGEN_READ32(REG_RW_LVDS_CTS_CKCFG);
            CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp | CLK_PDN_LVDS_CTS);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

bool mt8193_CKGEN_AgtSelClk(e_CLK_T eAgt, u32 u4Sel)
{
      u32 u4Tmp;

      printf("mt8193_CKGEN_AgtSelClk() %d\n", eAgt);

      switch (eAgt)
      {
          case e_CLK_NFI:
              u4Tmp = CKGEN_READ32(REG_RW_NFI_CKCFG);
                CKGEN_WRITE32(REG_RW_NFI_CKCFG, u4Tmp | u4Sel);
              break;
          case e_CLK_HDMIPLL:
              u4Tmp = CKGEN_READ32(REG_RW_HDMI_PLL_CKCFG);
                CKGEN_WRITE32(REG_RW_HDMI_PLL_CKCFG, u4Tmp | u4Sel);
              break;
            case e_CLK_HDMIDISP:
              u4Tmp = CKGEN_READ32(REG_RW_HDMI_DISP_CKCFG);
                CKGEN_WRITE32(REG_RW_HDMI_DISP_CKCFG, u4Tmp | u4Sel);
              break;
            case e_CLK_LVDSDISP:
                u4Tmp = CKGEN_READ32(REG_RW_LVDS_DISP_CKCFG);
                CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp | u4Sel);
              break;
            case e_CLK_LVDSCTS:
              u4Tmp = CKGEN_READ32(REG_RW_LVDS_CTS_CKCFG);
                CKGEN_WRITE32(REG_RW_LVDS_DISP_CKCFG, u4Tmp | u4Sel);
              break;
          default:
              return FALSE;
      }
       
    return TRUE;
}

u32 mt8193_CKGEN_AgtGetClk(e_CLK_T eAgt)
{
    printf("mt8193_CKGEN_AgtGetClk() %d\n", eAgt);
    return 0;
}

