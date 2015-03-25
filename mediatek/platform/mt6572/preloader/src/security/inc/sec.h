/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2011
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef SEC_H
#define SEC_H

#include "sec_cust.h"
#include "sec_error.h"

/******************************************************************************
 * FPGA PORTING
 ******************************************************************************/
//#define SEC_FPGA_EARLY_PORTING

/******************************************************************************
 * SECURE CFG READ SIZE
 ******************************************************************************/
#define SEC_CFG_READ_SIZE               (0x4000)

/******************************************************************************
 * SECURE CFG PARTITION INFO
 ******************************************************************************/
typedef struct _SECURE_CFG_INFO
{   
    U32                                 addr;
    U32                                 len;
    
} SECURE_CFG_INFO;

/**************************************************************************
 * EXPORTED FUNCTIONS
 **************************************************************************/

/* configuration for PL download DA feature */
extern BOOL seclib_sbc_enabled(void);
extern BOOL seclib_daa_enabled(void);
extern BOOL seclib_sla_enabled(void);

/* secure boot */
extern void sec_boot_check (void);
extern BOOL seclib_sec_boot_enabled (BOOL bMsg);
extern U32 seclib_image_check (U8* image_name, BOOL bMsg);

 /* sec_cfg related */
extern U8* sec_util_image_read (U32 offset, U32 size);
extern U8* sec_util_get_img_buf (void);
extern U8* sec_util_get_working_buf (void);
extern void seclib_image_hash_compute (U8 *buf, U32 size);

/* library initialization */
extern U32 seclib_init ( CUSTOM_SEC_CFG *cust_cfg, U8* sec_cfg_cipher_data, U32 sec_cfg_read_size, BOOL bMsg, BOOL bAC);

/* region check */
extern U32 seclib_region_check (U32 addr, U32 len);

#endif /* SEC_H */

