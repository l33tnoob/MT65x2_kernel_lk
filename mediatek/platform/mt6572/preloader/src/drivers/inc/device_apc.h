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


#ifndef _MTK_DEVICE_APC_H
#define _MTK_DEVICE_APC_H

#include "typedefs.h"

#define DEVAPC0_AO_BASE         0x10010000      // for AP
#define DEVAPC0_PD_BASE         0x10207000      // for AP
#define DEVAPC1_AO_BASE         0x10010100      // for AP
#define DEVAPC1_PD_BASE         0x10207100      // for AP
#define DEVAPC2_AO_BASE         0x10010200      // for AP
#define DEVAPC2_PD_BASE         0x10207200      // for AP
#define DEVAPC3_AO_BASE         0x10010300      // for MM
#define DEVAPC3_PD_BASE         0x10207300      // for MM
#define DEVAPC4_AO_BASE         0x10010400      // for MM
#define DEVAPC4_PD_BASE         0x10207400      // for MM


/*******************************************************************************
 * REGISTER ADDRESS DEFINATION
 ******************************************************************************/

#define DEVAPC0_D0_APC_0		    ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0000))
#define DEVAPC0_D0_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0004))
#define DEVAPC0_D1_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0008))
#define DEVAPC0_D1_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x000C))
#define DEVAPC0_D2_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0010))
#define DEVAPC0_D2_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0014))
#define DEVAPC0_D3_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0018))
#define DEVAPC0_D3_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x001C))
#define DEVAPC0_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0020))
#define DEVAPC0_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0024))
#define DEVAPC0_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0028))
#define DEVAPC0_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x002C))
#define DEVAPC0_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0030))
#define DEVAPC0_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0034))
#define DEVAPC0_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0038))
#define DEVAPC0_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x003C))
#define DEVAPC0_VIO_DBG0            ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0040))
#define DEVAPC0_VIO_DBG1            ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0044))
#define DEVAPC0_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0080))
#define DEVAPC0_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0084))
#define DEVAPC0_APC_CON             ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0090))
#define DEVAPC0_PD_APC_CON          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0090))
#define DEVAPC0_APC_LOCK            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0094))
#define DEVAPC0_MAS_DOM             ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x00A0))
#define DEVAPC0_MAS_SEC             ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x00A4))
#define DEVAPC0_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x00B4))
#define DEVAPC0_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x00B8))
#define DEVAPC0_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x00BC))

                                                                      
#define DEVAPC1_D0_APC_0		    ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0000))
#define DEVAPC1_D0_APC_1            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0004))
#define DEVAPC1_D1_APC_0            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0008))
#define DEVAPC1_D1_APC_1            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x000C))
#define DEVAPC1_D2_APC_0            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0010))
#define DEVAPC1_D2_APC_1            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0014))
#define DEVAPC1_D3_APC_0            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0018))
#define DEVAPC1_D3_APC_1            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x001C))
#define DEVAPC1_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0020))
#define DEVAPC1_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0024))
#define DEVAPC1_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0028))
#define DEVAPC1_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x002C))
#define DEVAPC1_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0030))
#define DEVAPC1_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0034))
#define DEVAPC1_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0038))
#define DEVAPC1_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x003C))
#define DEVAPC1_VIO_DBG0            ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0040))
#define DEVAPC1_VIO_DBG1            ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0044))
#define DEVAPC1_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0080))
#define DEVAPC1_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0084))
#define DEVAPC1_APC_CON             ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0090))
#define DEVAPC1_PD_APC_CON          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x0090))
#define DEVAPC1_APC_LOCK            ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x0094))
#define DEVAPC1_MAS_DOM             ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x00A0))
#define DEVAPC1_MAS_SEC             ((volatile unsigned int*)(DEVAPC1_AO_BASE+0x00A4))
#define DEVAPC1_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x00B4))
#define DEVAPC1_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x00B8))
#define DEVAPC1_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC1_PD_BASE+0x00BC))

#define DEVAPC2_D0_APC_0		    ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0000))
#define DEVAPC2_D0_APC_1            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0004))
#define DEVAPC2_D1_APC_0            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0008))
#define DEVAPC2_D1_APC_1            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x000C))
#define DEVAPC2_D2_APC_0            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0010))
#define DEVAPC2_D2_APC_1            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0014))
#define DEVAPC2_D3_APC_0            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0018))
#define DEVAPC2_D3_APC_1            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x001C))
#define DEVAPC2_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0020))
#define DEVAPC2_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0024))
#define DEVAPC2_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0028))
#define DEVAPC2_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x002C))
#define DEVAPC2_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0030))
#define DEVAPC2_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0034))
#define DEVAPC2_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0038))
#define DEVAPC2_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x003C))
#define DEVAPC2_VIO_DBG0            ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0040))
#define DEVAPC2_VIO_DBG1            ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0044))
#define DEVAPC2_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0080))
#define DEVAPC2_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0084))
#define DEVAPC2_APC_CON             ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0090))
#define DEVAPC2_PD_APC_CON          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x0090))
#define DEVAPC2_APC_LOCK            ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x0094))
#define DEVAPC2_MAS_DOM             ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x00A0))
#define DEVAPC2_MAS_SEC             ((volatile unsigned int*)(DEVAPC2_AO_BASE+0x00A4))
#define DEVAPC2_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x00B4))
#define DEVAPC2_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x00B8))
#define DEVAPC2_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC2_PD_BASE+0x00BC))

#define DEVAPC3_D0_APC_0		    ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0000))
#define DEVAPC3_D0_APC_1            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0004))
#define DEVAPC3_D1_APC_0            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0008))
#define DEVAPC3_D1_APC_1            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x000C))
#define DEVAPC3_D2_APC_0            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0010))
#define DEVAPC3_D2_APC_1            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0014))
#define DEVAPC3_D3_APC_0            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0018))
#define DEVAPC3_D3_APC_1            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x001C))
#define DEVAPC3_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0020))
#define DEVAPC3_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0024))
#define DEVAPC3_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0028))
#define DEVAPC3_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x002C))
#define DEVAPC3_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0030))
#define DEVAPC3_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0034))
#define DEVAPC3_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0038))
#define DEVAPC3_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x003C))
#define DEVAPC3_VIO_DBG0            ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0040))
#define DEVAPC3_VIO_DBG1            ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0044))
#define DEVAPC3_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0080))
#define DEVAPC3_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0084))
#define DEVAPC3_APC_CON             ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0090))
#define DEVAPC3_PD_APC_CON          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x0090))
#define DEVAPC3_APC_LOCK            ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x0094))
#define DEVAPC3_MAS_DOM             ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x00A0))
#define DEVAPC3_MAS_SEC             ((volatile unsigned int*)(DEVAPC3_AO_BASE+0x00A4))
#define DEVAPC3_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x00B4))
#define DEVAPC3_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x00B8))
#define DEVAPC3_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC3_PD_BASE+0x00BC))

#define DEVAPC4_D0_APC_0		    ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0000))
#define DEVAPC4_D0_APC_1            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0004))
#define DEVAPC4_D1_APC_0            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0008))
#define DEVAPC4_D1_APC_1            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x000C))
#define DEVAPC4_D2_APC_0            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0010))
#define DEVAPC4_D2_APC_1            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0014))
#define DEVAPC4_D3_APC_0            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0018))
#define DEVAPC4_D3_APC_1            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x001C))
#define DEVAPC4_D0_VIO_MASK         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0020))
#define DEVAPC4_D1_VIO_MASK         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0024))
#define DEVAPC4_D2_VIO_MASK         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0028))
#define DEVAPC4_D3_VIO_MASK         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x002C))
#define DEVAPC4_D0_VIO_STA          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0030))
#define DEVAPC4_D1_VIO_STA          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0034))
#define DEVAPC4_D2_VIO_STA          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0038))
#define DEVAPC4_D3_VIO_STA          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x003C))
#define DEVAPC4_VIO_DBG0            ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0040))
#define DEVAPC4_VIO_DBG1            ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0044))
#define DEVAPC4_DXS_VIO_MASK        ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0080))
#define DEVAPC4_DXS_VIO_STA         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0084))
#define DEVAPC4_APC_CON             ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0090))
#define DEVAPC4_PD_APC_CON          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x0090))
#define DEVAPC4_APC_LOCK            ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x0094))
#define DEVAPC4_MAS_DOM             ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x00A0))
#define DEVAPC4_MAS_SEC             ((volatile unsigned int*)(DEVAPC4_AO_BASE+0x00A4))
#define DEVAPC4_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x00B4))
#define DEVAPC4_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x00B8))
#define DEVAPC4_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC4_PD_BASE+0x00BC))

/* DOMAIN_SETUP */
#define DOMAIN_AP						0
#define DOMAIN_MD1	    				1
#define DOMAIN_MD2						2
#define DOMAIN_MM                       3

/* Masks for Domain Control for DEVAPC3 */
#define MD1_AHB_0                       (0x3 << 0)   
#define MD1_AXI_1                       (0x3 << 2)   
#define MD1_AXI_2                       (0x3 << 4)   
#define MD2_AHB_0                       (0x3 << 6)   
#define MD2_AXI_1                       (0x3 << 8)   
#define MD2_AXI_2                       (0x3 << 10)   
#define APMCU                           (0x3 << 12)   


static inline unsigned int uffs(unsigned int x)
{
    unsigned int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

#define reg_read16(reg)          __raw_readw(reg)
#define reg_read32(reg)          __raw_readl(reg)
#define reg_write16(reg,val)     __raw_writew(val,reg)
#define reg_write32(reg,val)     __raw_writel(val,reg)
 
#define reg_set_bits(reg,bs)     ((*(volatile u32*)(reg)) |= (u32)(bs))
#define reg_clr_bits(reg,bs)     ((*(volatile u32*)(reg)) &= ~((u32)(bs)))
 
#define reg_set_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         tv &= ~(field); \
         tv |= ((val) << (uffs((unsigned int)field) - 1)); \
         reg_write32(reg,tv); \
     } while(0)
     
#define reg_get_field(reg,field,val) \
     do {    \
         volatile unsigned int tv = reg_read32(reg); \
         val = ((tv & (field)) >> (uffs((unsigned int)field) - 1)); \
     } while(0)

#endif
