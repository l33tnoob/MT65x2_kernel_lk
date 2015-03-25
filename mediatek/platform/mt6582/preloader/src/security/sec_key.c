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

#include "sec_platform.h"
#include "sec_rom_info.h"
#include "sec_key.h"
#include "sec_error.h"

/**************************************************************************
*  MACRO
**************************************************************************/ 
#define MOD                             "SEC_K"

/**************************************************************************
*  DEFINITION
**************************************************************************/ 
#define SBC_PUBK_SEARCH_LEN             (0x300)  

/******************************************************************************
 * DEBUG
 ******************************************************************************/
#define SMSG                            dbg_print

/**************************************************************************
 *  GLOBAL VARIABLES
 **************************************************************************/
RSA_PUBK                                g_SBC_PUBK;

/**************************************************************************
 *  LOCAL VARIABLES
 **************************************************************************/

/**************************************************************************
 *  EXTERNAL VARIABLES
 **************************************************************************/
extern AND_ROMINFO_T  g_ROM_INFO;

#if 0
/**************************************************************************
 *  UTILITY
 **************************************************************************/
void dump_buf(U8* buf, U32 len)
{
    U32 i = 0;

    for (i =1; i <len+1; i++)
    {                
        SMSG("0x%x,",buf[i-1]);
        
        if(0 == (i%8))
            SMSG("\n");
    }

    if(0 != (len%8))
        SMSG("\n");    
}
#endif

/**************************************************************************
 *  INTERNAL FUNCTIONS
 **************************************************************************/
void sec_key_init (void)
{   
    U32 addr = 0;
    U32 ret = 0;
    U32 e_key_len = 2;

    /* ------------------ */
    /* check key length   */
    /* ------------------ */    
    COMPILE_ASSERT(AND_SEC_KEY_SIZE == sizeof(AND_SECKEY_T));          
    SMSG("[%s] SML KEY AC = %d\n",MOD,g_ROM_INFO.m_SEC_CTRL.m_sml_aes_key_ac_en);

    /* ------------------ */
    /* read sbc key       */
    /* ------------------ */    

    /* specify read address */
    addr = &g_ROM_INFO;
    addr = addr & 0xFFFFF000;
    addr = addr - SBC_PUBK_SEARCH_LEN;
    //SMSG("[%s] Search Addr '0x%x'\n",MOD,addr);
    
    ret = seclib_read_sbc_key (addr, SBC_PUBK_SEARCH_LEN, &g_SBC_PUBK);
    if(SEC_OK == ret)
    {        
        SMSG("[%s] SBC_PUBK Found\n",MOD);
        //SMSG("[%s] E_KEY :\n",MOD);    
        /* remove random data */
        memset(g_SBC_PUBK.E_Key + e_key_len, 0x0, sizeof(g_SBC_PUBK.E_Key) - 2*e_key_len);  
        //dump_buf(g_SBC_PUBK.E_Key,sizeof(g_SBC_PUBK.E_Key));
        //SMSG("[%s] N_KEY :\n",MOD);    
        //dump_buf(g_SBC_PUBK.N_Key,sizeof(g_SBC_PUBK.N_Key));        
    }
    else
    {
        SMSG("[%s] SBC_PUBK Not Found '0x%x'\n",MOD,ret);    
    }
    
}


