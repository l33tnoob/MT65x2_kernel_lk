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

#ifndef PLATFORM_H
#define PLATFORM_H

#include "print.h"
#include "string.h"
#include "boot_device.h"
#include "blkdev.h"
#include "preloader.h"

#include "mt6582.h"

#include "cust_bldr.h"
#include "cust_part.h"
#include "cust_rtc.h"
#include "cust_nand.h"
#include "cust_usb.h"
#include "sec_limit.h"

/*=======================================================================*/
/* Pre-Loader Internal Features                                          */
/*=======================================================================*/

/* if not defined in cust_usb.h, use default setting */
#if !defined(CFG_USB_ENUM_TIMEOUT)
#define CFG_USB_ENUM_TIMEOUT            (8000)           /* 8s */
#endif

/* if not defined in cust_usb.h, use default setting */
#if !defined(CFG_USB_HANDSHAKE_TIMEOUT)
#define CFG_USB_HANDSHAKE_TIMEOUT       (2500)           /* 2.5s */
#endif

/* support switch to modem com or not */
#ifdef MTK_DT_SUPPORT
#define CFG_DT_MD_DOWNLOAD              (1)
#else
#define CFG_DT_MD_DOWNLOAD              (0)
#endif

/*=======================================================================*/
/* Platform Setting                                                      */
/*=======================================================================*/
#if CFG_FPGA_PLATFORM
#define I2C_SRC_CLK_FRQ                 (12000000)
#define UART_SRC_CLK_FRQ                (12000000)
#define MSDC_SRC_CLK_FRQ                (12000000)

#else /* !CFG_FPGA_PLATFORM */
#define UART_SRC_CLK_FRQ                (0)         /* use default */
#endif

#define PART_MAX_NUM                    20

/*=======================================================================*/
/* Image Address                                                         */	
/*=======================================================================*/
#define CFG_DRAM_ADDR                   (0x80000000)
#define CFG_DA_RAM_ADDR                 (CFG_DRAM_ADDR + 0x00001000)
#if CFG_TRUSTONIC_TEE_SUPPORT
#define MAX_TEE_DRAM_SIZE               (160 * 1024 * 1024) /* TEE max DRAM size is 160MB */
#elif CFG_MEDIATEK_TEE_SUPPORT
#define MAX_TEE_DRAM_SIZE               (256 * 1024 * 1024) /* TEE max DRAM size is 256MB */
#else
#define MAX_TEE_DRAM_SIZE               (0) /* TEE max DRAM size is 0 if TEE is not enabled */
#endif

//ALPS00427972, implement the analog register formula
//Add here for eFuse, chip version checking -> analog register calibration
#define M_HW_RES3	                    0x10009170
//#define M_HW_RES3_PHY                   IO_PHYS+M_HW_RES3
#define RG_USB20_TERM_VREF_SEL_MASK     0xE000      //0b 1110,0000,0000,0000     15~13
#define RG_USB20_CLKREF_REF_MASK        0x1C00      //0b 0001,1100,0000,0000     12~10
#define RG_USB20_VRT_VREF_SEL_MASK      0x0380      //0b 0000,0011,1000,0000     9~7
//ALPS00427972, implement the analog register formula

#define RAM_CONSOLE_ADDR                (0x0010DC00)
#define RAM_CONSOLE_MAX_SIZE            (0x1C00)

typedef enum {
    NORMAL_BOOT         = 0,
    META_BOOT           = 1,
    RECOVERY_BOOT       = 2,
    SW_REBOOT           = 3,
    FACTORY_BOOT        = 4,
    ADVMETA_BOOT        = 5,
    ATE_FACTORY_BOOT    = 6,
    ALARM_BOOT          = 7,
    FASTBOOT            = 99,

    DOWNLOAD_BOOT       = 100,
    UNKNOWN_BOOT
} boot_mode_t;

typedef enum {
    BR_POWER_KEY = 0,
    BR_USB,
    BR_RTC,
    BR_WDT,
    BR_WDT_BY_PASS_PWK,
    BR_TOOL_BY_PASS_PWK,
    BR_2SEC_REBOOT,
    BR_UNKNOWN
} boot_reason_t;

typedef enum {
    META_UNKNOWN_COM = 0,
    META_UART_COM,
    META_USB_COM
} meta_com_t;

/* boot argument magic */
#define BOOT_ARGUMENT_MAGIC             (0x504c504c)
#define BA_FIELD_BYPASS_MAGIC           (0x5A5B5A5B)

typedef struct {
    u32 addr;    /* download agent address */
    u32 arg1;    /* download agent argument 1 */
    u32 arg2;    /* download agent argument 2 */
    u32 len;     /* length of DA */
    u32 sig_len; /* signature length of DA */
} da_info_t;

typedef struct {
    u32 magic;
    boot_mode_t mode;
    u32 e_flag;
    u32 log_port;
    u32 log_baudrate;
    u8  log_enable;
    u8  part_num;
    u8  reserved[2];
    u32 dram_rank_num;
    u32 dram_rank_size[4];
    u32 boot_reason;
    u32 meta_com_type;
    u32 meta_com_id;
    u32 boot_time;
    da_info_t da_info;
    SEC_LIMIT sec_limit;	
    part_hdr_t *part_info;
    u8  md_type[4];
    u32 ddr_reserve_enable;    
    u32 ddr_reserve_success;
    u32 chip_ver;
} boot_arg_t;

typedef enum {
    CHIP_VER_E1 = 0xca00,
    CHIP_VER_E2 = 0xcb00,
    CHIP_6583_E1 = CHIP_VER_E1,
    CHIP_6583_E2 = CHIP_VER_E2,
} chip_ver_t;

typedef enum {
    COM_UART = 0,
    COM_USB
} comport_t;

struct comport_ops {
    int (*send)(u8 *buf, u32 len);
    int (*recv)(u8 *buf, u32 len, u32 tmo);
};

struct bldr_comport {
    u32 type;
    u32 tmo;
    struct comport_ops *ops;
};

struct bldr_command {
    u8  *data;
    u32  len;
};

struct bldr_command_handler;

typedef bool (*bldr_cmd_handler_t)(struct bldr_command_handler *handler, struct bldr_command *cmd, struct bldr_comport *comm);

struct bldr_command_handler {
    void *priv;
    u32   attr;
    bldr_cmd_handler_t cb;
};

#define ME_IDENTITY_LEN         16  // 16B
#define MAX_RECORD_SIZE         512
#define MAX_BL_NUM              8

typedef struct {    
    u32                         m_bl_load_addr;
    u32                         m_bl_length;
    u32                         m_p_bl_desc;
    u8                          m_bl_index;
    u8                          m_state;
    u16                         m_code_1;
    u16                         m_code_2;
} BL_LoadInfo;

typedef struct {            
    u32                         m_raw_data[MAX_RECORD_SIZE>>2];
}Hdrblk_Info;

typedef struct {
    // BL param structure version
    u32                         m_ver;

    u32                         m_hdr_blk_dev_addr;
    Hdrblk_Info                 m_hdr_blk_info[4];       
    
    BL_LoadInfo                 m_bl_loadinfo[MAX_BL_NUM];
    
    u32                         m_brom_flags;
        
    u32                         MEID[ME_IDENTITY_LEN>>2];
}bl_param_t;

#define CMD_HNDL_ATTR_COM_FORBIDDEN               (1<<0)

extern int usb_cable_in(void);

extern int  platform_get_mcp_id(u8 *id, u32 len, u32 *fw_id_len);
extern void platform_vusb_on(void);
extern void platform_wdt_kick(void);
extern void platform_wdt_all_kick(void);
extern void platform_pre_init(void);
extern void platform_init(void);
extern void platform_post_init(void);
extern void platform_error_handler(void);
extern void platform_assert(char *file, int line, char *expr);
extern chip_ver_t platform_chip_ver(void);

extern void apmcu_dcache_clean_invalidate(void);
extern void apmcu_dsb(void);
extern void apmcu_isb(void);
extern void apmcu_disable_dcache(void);
extern void apmcu_disable_icache(void);    
extern void apmcu_icache_invalidate(void);
extern void apmcu_disable_smp(void);
extern void jump(u32 addr, u32 arg1, u32 arg2);

extern boot_mode_t g_boot_mode;
extern boot_dev_t  g_boot_dev;
extern boot_reason_t g_boot_reason;
extern meta_com_t g_meta_com_type;
extern u32 g_meta_com_id;

#endif /* PLATFORM_H */
