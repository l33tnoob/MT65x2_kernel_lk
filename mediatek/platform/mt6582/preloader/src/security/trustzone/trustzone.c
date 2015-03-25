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

/* Include header files */
#include "trustzone.h"
#include "typedefs.h"
#include "platform.h"
#include "dram_buffer.h"
#include "tz_sec_reg.h"
#include "device_apc.h"

#if CFG_BOOT_ARGUMENT
#define bootarg g_dram_buf->bootarg
#endif

#define teearg g_dram_buf->teearg

#define TBASE_MEM_SIZE      0x500000

/**************************************************************************
 *  EXTERNAL FUNCTIONS
 **************************************************************************/
extern void tz_sram_sec_init(void);
extern void tz_sec_mem_init(u32 start, u32 end);
extern void tz_dapc_sec_init(void);
extern void tz_set_module_apc(unsigned int module, E_MASK_DOM domain_num , APC_ATTR permission_control);
extern void tz_boot_share_page_init(void);
extern bl_param_t g_bl_param;
extern unsigned int BOOT_ARGUMENT_LOCATION;
extern u32 tee_boot_addr;
extern u32 g_secure_dram_size;

void spm_mtcmos_pdn_cpu1(void)
{   
    /* enable register control */
    WRITE_REGISTER_UINT32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    while ((READ_REGISTER_UINT32(SPM_SLEEP_TIMER_STA) & APMCU1_SLEEP) == 0);
    
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, READ_REGISTER_UINT32(SPM_FC1_PWR_CON) | SRAM_CKISO);
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, READ_REGISTER_UINT32(SPM_FC1_PWR_CON) & ~SRAM_ISOINT_B);
    WRITE_REGISTER_UINT32(SPM_CPU_FC1_L1_PDN, READ_REGISTER_UINT32(SPM_CPU_FC1_L1_PDN) | L1_PDN);
    while ((READ_REGISTER_UINT32(SPM_CPU_FC1_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
    
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, READ_REGISTER_UINT32(SPM_FC1_PWR_CON) | PWR_ISO);
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, (READ_REGISTER_UINT32(SPM_FC1_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
    
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, READ_REGISTER_UINT32(SPM_FC1_PWR_CON) & ~PWR_ON);
    WRITE_REGISTER_UINT32(SPM_FC1_PWR_CON, READ_REGISTER_UINT32(SPM_FC1_PWR_CON) & ~PWR_ON_S);
    while (((READ_REGISTER_UINT32(SPM_PWR_STATUS) & FC1) != 0) | ((READ_REGISTER_UINT32(SPM_PWR_STATUS_S) & FC1) != 0));
}

void spm_mtcmos_pdn_cpu2(void)
{   
    /* enable register control */
    WRITE_REGISTER_UINT32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
       
    while ((READ_REGISTER_UINT32(SPM_SLEEP_TIMER_STA) & APMCU2_SLEEP) == 0);
    
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, READ_REGISTER_UINT32(SPM_FC2_PWR_CON) | SRAM_CKISO);
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, READ_REGISTER_UINT32(SPM_FC2_PWR_CON) & ~SRAM_ISOINT_B);
    WRITE_REGISTER_UINT32(SPM_CPU_FC2_L1_PDN, READ_REGISTER_UINT32(SPM_CPU_FC2_L1_PDN) | L1_PDN);
    while ((READ_REGISTER_UINT32(SPM_CPU_FC2_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
    
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, READ_REGISTER_UINT32(SPM_FC2_PWR_CON) | PWR_ISO);
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, (READ_REGISTER_UINT32(SPM_FC2_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
    
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, READ_REGISTER_UINT32(SPM_FC2_PWR_CON) & ~PWR_ON);
    WRITE_REGISTER_UINT32(SPM_FC2_PWR_CON, READ_REGISTER_UINT32(SPM_FC2_PWR_CON) & ~PWR_ON_S);
    while (((READ_REGISTER_UINT32(SPM_PWR_STATUS) & FC2) != 0) | ((READ_REGISTER_UINT32(SPM_PWR_STATUS_S) & FC2) != 0));
}

void spm_mtcmos_pdn_cpu3(void)
{   
    /* enable register control */
    WRITE_REGISTER_UINT32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));

    while ((READ_REGISTER_UINT32(SPM_SLEEP_TIMER_STA) & APMCU3_SLEEP) == 0);
        
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, READ_REGISTER_UINT32(SPM_FC3_PWR_CON) | SRAM_CKISO);
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, READ_REGISTER_UINT32(SPM_FC3_PWR_CON) & ~SRAM_ISOINT_B);
    WRITE_REGISTER_UINT32(SPM_CPU_FC3_L1_PDN, READ_REGISTER_UINT32(SPM_CPU_FC3_L1_PDN) | L1_PDN);
    while ((READ_REGISTER_UINT32(SPM_CPU_FC3_L1_PDN) & L1_PDN_ACK) != L1_PDN_ACK);
    
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, READ_REGISTER_UINT32(SPM_FC3_PWR_CON) | PWR_ISO);
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, (READ_REGISTER_UINT32(SPM_FC3_PWR_CON) | PWR_CLK_DIS) & ~PWR_RST_B);
    
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, READ_REGISTER_UINT32(SPM_FC3_PWR_CON) & ~PWR_ON);
    WRITE_REGISTER_UINT32(SPM_FC3_PWR_CON, READ_REGISTER_UINT32(SPM_FC3_PWR_CON) & ~PWR_ON_S);
    while (((READ_REGISTER_UINT32(SPM_PWR_STATUS) & FC3) != 0) | ((READ_REGISTER_UINT32(SPM_PWR_STATUS_S) & FC3) != 0));
}

void tz_sec_brom_pdn(void)
{
    u32 cpu_info = seclib_get_devinfo_with_index(3);    
    cpu_info = (cpu_info << 4) >> 28;     
    
    print("%s (B)tz_sec_brom_pdn -- cpu info : 0x%x\n", MOD, cpu_info);

    switch(cpu_info)
    {
    case 0x0:
        spm_mtcmos_pdn_cpu1();
        spm_mtcmos_pdn_cpu2();
        spm_mtcmos_pdn_cpu3();
        break;
    case 0xC:
        spm_mtcmos_pdn_cpu1();
        break;
    case 0x8:
        spm_mtcmos_pdn_cpu1();
        spm_mtcmos_pdn_cpu2();
        break;
    default:
        break;
    }

    print("%s (B)tz_sec_brom_pdn is 0x%x.\n", MOD, READ_REGISTER_UINT32(BOOTROM_PWR_CTRL));
    WRITE_REGISTER_UINT32(BOOTROM_PWR_CTRL, (READ_REGISTER_UINT32(BOOTROM_PWR_CTRL)| 0x80000000));
    print("%s (E)tz_sec_brom_pdn is 0x%x.\n", MOD, READ_REGISTER_UINT32(BOOTROM_PWR_CTRL));
}

void tee_sec_config(void)
{
    u32 start, end;
    start = CFG_DRAM_ADDR + memory_size();
    end = start + g_secure_dram_size;

    tz_sram_sec_init();

    // disable MPU protect to secure OS secure memory before SQC(for debug)
//#if defined(TRUSTONIC_TEE_SUPPORT) && defined(MTK_SEC_VIDEO_PATH_SUPPORT)
#ifdef MTK_SEC_VIDEO_PATH_SUPPORT
    // dose not enable MPU protect to secure memory
#else
    tz_sec_mem_init(start, end);
#endif
    tz_sec_brom_pdn();
    tz_dapc_sec_init();
    tz_set_module_apc(MOD_INFRA_MCU_BIU_CONF, E_DOMAIN_0, E_L1);
    tz_boot_share_page_init();
}

/*
 * addr is NW Jump addr.
 * arg1 is NW Component bootargs.
 * arg2 is NW Component bootargs size
 */
void bldr_tee_jump(u32 addr, u32 arg1, u32 arg2)
{
    typedef void (*jump_func_type)(u32 addr ,u32 arg1, u32 arg2, u32 arg3) __attribute__ ((__noreturn__));
    jump_func_type jump_func;
    u32 full_memory_size = 0;
    sec_mem_arg_t sec_mem_arg;
    
    /* Configure platform's security settings */
    tee_sec_config();
    
    /* prepare trustonic's TEE arguments */    
    teearg.magic        = TEE_ARGUMENT_MAGIC;		/* Trustonic's TEE magic number */
    teearg.version      = TEE_ARGUMENT_VERSION;		/* Trustonic's TEE argument block version */
    teearg.NWEntry      = addr;                     /* NW Entry point after t-base */
    teearg.NWBootArgs   = arg1;                     /* NW boot args (propagated by t-base in r4 before jump) */
    teearg.NWBootArgsSize = arg2;                   /* NW boot args size (propagated by t-base in r5 before jump) */
    teearg.dRamBase     = CFG_DRAM_ADDR;			/* DRAM base address */
    teearg.dRamSize     = memory_size() + g_secure_dram_size;	/* Full DRAM size */
    teearg.secDRamBase  = tee_boot_addr;			/* Secure DRAM base address */    
    teearg.secDRamSize  = TBASE_MEM_SIZE;           /* Secure DRAM size */
    teearg.sRamBase     = 0x00000000;				/* SRAM base address */
    teearg.sRamSize     = 0x00000000;				/* SRAM size */
    teearg.secSRamBase  = 0x00000000;				/* Secure SRAM base address */
    teearg.secSRamSize  = 0x00000000;				/* Secure SRAM size */
    teearg.log_port     = UART0_BASE;				/* UART logging : UART base address. Can be same as preloader's one or not */
    teearg.log_baudrate = bootarg.log_baudrate;		/* UART logging : UART baud rate */
    
    print("KOSHI: boot argument addr (0x%x)\n", BOOT_ARGUMENT_LOCATION);
    
    /* Koshi modified { */
    /* Please note that if you use cmm file, the parameter is empty. */
    memcpy(teearg.hwuid, g_bl_param.MEID, sizeof(g_bl_param.MEID));
    /* Koshi modified } */    
    
    sec_mem_arg.magic = SEC_MEM_MAGIC;
    sec_mem_arg.version = SEC_MEM_VERSION;
    sec_mem_arg.svp_mem_start = tee_boot_addr + TBASE_MEM_SIZE;
    sec_mem_arg.svp_mem_end = tee_boot_addr + g_secure_dram_size;
    sec_mem_arg.tplay_table_size = SEC_MEM_TPLAY_TABLE_SIZE;
    memcpy(TEE_PARAMETER_ADDR, &sec_mem_arg, sizeof(sec_mem_arg_t));

    /* Jump to TEE */
    print("%s Mode(%d) Start world switch from secure to non-secure world.\n",MOD, g_boot_mode);
    print("%s SW jump addr is 0x%x.\n",MOD, tee_boot_addr);
    jump_func = (jump_func_type)tee_boot_addr;
    //(*jump_func)(addr, arg1, arg2, (u32)&teearg);
    (*jump_func)(0, (u32)&teearg, 0, 0);
    // Never return.
}
