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

#ifndef SEC_ROMINFO_H
#define SEC_ROMINFO_H

#include "sec_boot.h"
#include "sec_key.h"
#include "sec_ctrl.h"
#include "sec_flashtool_cfg.h"


/**************************************************************************
*  INIT VARIABLES
**************************************************************************/ 
#define ROM_INFO_NAME                       "AND_ROMINFO_v"
/* VER1 - only ROM INFO region is provided */
/* VER2 - SECRO image anti-clone feature is supported */
#define ROM_INFO_VER                        0x2
#define ROM_INFO_SEC_RO_EXIST               0x1
#define ROM_INFO_ANTI_CLONE_OFFSET          0x54
#define ROM_INFO_ANTI_CLONE_LENGTH          0xE0

/**************************************************************************
*  ANDRIOD ROM INFO FORMAT
**************************************************************************/ 
/* this structure should always sync with FlashLib 
   becuase FlashLib will search storage to find ROM_INFO */
#define AND_ROM_INFO_SIZE                  (960)   
typedef struct {

    unsigned char                   m_identifier[16];   /* MTK */
    unsigned int                    m_rom_info_ver;     /* MTK */
    unsigned char                   m_platform_id[16];  /* CUSTOMER */
    unsigned char                   m_project_id[16];
    
    unsigned int                    m_sec_ro_exist;     /* MTK */
    unsigned int                    m_sec_ro_offset;    /* MTK */
    unsigned int                    m_sec_ro_length;    /* MTK */
    
    unsigned int                    m_ac_offset;        /* MTK : 
                                                            no use */

    unsigned int                    m_ac_length;        /* MTK : 
                                                            no use */
                                                            
    unsigned int                    m_sec_cfg_offset;   /* MTK :
                                                            part info. from 
                                                            parititon table.

                                                            tool will refer to
                                                            this setting to 
                                                            find SEC CFG */

    unsigned int                    m_sec_cfg_length;   /* MTK :
                                                            part info. from 
                                                            parititon table.
                                                            
                                                            tool will refer to
                                                            this setting to 
                                                            find SEC CFG */

    FLASHTOOL_SECCFG_T              m_flashtool_cfg;    /* CUSTOMER */
    FLASHTOOL_FORBID_DOWNLOAD_NSLA_T     m_flashtool_forbid_dl_nsla_cfg;    /* CUSTOMER */ 
    
    AND_SECCTRL_T                   m_SEC_CTRL;         /* CUSTOMER :
                                                            secure feature 
                                                            control */

    unsigned char                   m_reserve2[18];

                                                        /* CUSTOMER :
                                                            secure boot check 
                                                            partition */
    AND_SECBOOT_CHECK_PART_T        m_SEC_BOOT_CHECK_PART; 

    AND_SECKEY_T                    m_SEC_KEY;          /* CUSTOMER :
                                                            key */

} AND_ROMINFO_T;

#endif /* SEC_ROMINFO_H */

