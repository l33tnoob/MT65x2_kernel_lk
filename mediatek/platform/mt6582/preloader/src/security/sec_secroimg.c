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

/* for general operations */
#include "sec_platform.h"

/* import sec cfg partition info */
#include "sec_rom_info.h"

/* import secro image info */
#include "sec_secroimg.h"

/* for crypto operations */
#include "sec.h"
#include "sec_error.h"

/* for storage device operations */
#include "boot_device.h"
#include "nand_core.h"
#include "dram_buffer.h"
/**************************************************************************
*  MACRO
**************************************************************************/ 
#define MOD                             "SECRO"

/**************************************************************************
 * DEBUG
 **************************************************************************/ 
#define SEC_DEBUG                       (FALSE)
#define SMSG                            dbg_print
#if SEC_DEBUG
#define DMSG                            dbg_print
#else
#define DMSG 
#endif

/**************************************************************************
 *  GLOBAL VARIABLES
 **************************************************************************/
AND_SECROIMG_T                          *secroimg;           

/**************************************************************************
 *  EXTERNAL VARIABLES
 **************************************************************************/
extern AND_ROMINFO_T                    g_ROM_INFO;
#define bootarg g_dram_buf->bootarg

/******************************************************************************
 *  READ FACOTRY MODE CTRL
 ******************************************************************************/
BOOL factory_mode_valid (void)
{
    BOOL bFacEn = FALSE;

#ifdef MTK_SECURITY_SW_SUPPORT

    if(FACTORY_EN_CODE == secroimg->m_andro.factory_en)
    {
        SMSG("[%s] factory mode enabled\n",MOD);
        bFacEn = TRUE;
    }
    else
    {
        SMSG("[%s] factory mode disabled (0x%x)\n",MOD,secroimg->m_andro.factory_en);
    }
#endif

    return bFacEn;    
}

/**************************************************************************
 * [SECRO IMG] 
 **************************************************************************/
void sec_lib_read_secro (void)
{

#ifdef MTK_SECURITY_SW_SUPPORT


    U32 err;
    U8 *buf = sec_util_get_secro_buf();
    const U32 buf_len = sizeof(AND_SECROIMG_T);
    const U32 ac_offset = sizeof(AND_AC_HEADER_T);
    const U32 ac_len = sizeof(AND_AC_ANDRO_T) + sizeof(AND_AC_SV5_T);    
    const bAC = g_ROM_INFO.m_SEC_CTRL.m_secro_ac_en;
    secroimg = NULL;

    /* ---------------------- */
    /* check status           */
    /* ---------------------- */
    
    if(FALSE == seclib_sec_usbdl_enabled(TRUE))
    {
        SMSG("[%s] Don't read\n",MOD);
        return ;
    }

    /* ---------------------- */
    /* read secro image       */
    /* ---------------------- */

    if(SEC_OK != (err = seclib_secroimg_read (buf, buf_len, ac_offset, ac_len, bAC)))
    {
        SMSG("[%s] read secroimg fail '0x%x'\n",MOD,err);
        ASSERT (0);
    }
    else
    {
        secroimg = (AND_SECROIMG_T*) buf;
        SMSG("[%s] secroimg '0x%x'\n",MOD, secroimg->m_andro.magic_number);
        SMSG("[%s] secroimg '0x%x'\n",MOD, secroimg->m_sv5.magic_number);    
        

        /* ---------------------- */
        /* if valid SECRO format  */
        /* ---------------------- */
        if (AC_SV5_MAGIC == secroimg->m_sv5.magic_number)
        {
		    /* ---------------------- */
            /* check forbidden mode   */
            /* ---------------------- */
            if(FALSE == factory_mode_valid())
            {
                /* ---------------------- */
                /* set magic number       */
                /* ---------------------- */                
                bootarg.sec_limit.magic_num = SEC_LIMIT_MAGIC;
                bootarg.sec_limit.forbid_mode = F_FACTORY_MODE;
            }
        }
    }

#endif

}
