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

#include "typedefs.h"
#include "platform.h"
#include "download.h"
#include "meta.h"
#include "sec.h"
#include "part.h"
#include "dram_buffer.h"

/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD "[BLDR]"

/*============================================================================*/
/* MACROS DEFINITIONS                                                         */
/*============================================================================*/
#define CMD_MATCH(cmd1,cmd2)  \
    (!strncmp((const char*)(cmd1->data), (cmd2), min(strlen(cmd2), cmd1->len)))

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/
#if CFG_BOOT_ARGUMENT
#define bootarg g_dram_buf->bootarg
#endif

#if CFG_TRUSTONIC_TEE_SUPPORT
extern void bldr_tee_jump(u32 addr, u32 arg1, u32 arg2);
u32 tee_boot_addr = 0;
extern u32 g_secure_dram_size;
#endif

extern unsigned int BOOT_ARGUMENT_LOCATION;
extern bl_param_t g_bl_param;

/*============================================================================*/
/* INTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static void bldr_pre_process(void)
{
    
    #if CFG_USB_AUTO_DETECT
	platform_usbdl_flag_check();	
	#endif

    #if CFG_EMERGENCY_DL_SUPPORT
    platform_safe_mode(1, CFG_EMERGENCY_DL_TIMEOUT_MS);
    #endif
    
    /* essential hardware initialization. e.g. timer, pll, uart... */
    platform_pre_init();

    print("\n%s Build Time: %s\n", MOD, BUILD_TIME);    

    /* Get the BROM parameter */
    memcpy(&g_bl_param, BOOT_ARGUMENT_LOCATION, sizeof(g_bl_param));
    
    g_boot_mode = NORMAL_BOOT;

#if CFG_UART_TOOL_HANDSHAKE
            /* init uart handshake for sending 'ready' to tool and receiving handshake
             * pattern from tool in the background and we'll see the pattern later.
             * this can reduce the handshake time.
             */
            uart_handshake_init();
#endif 

    /* hardware initialization */
    platform_init();

    part_init();
    part_dump();

    /* init security library */
    sec_lib_init();
}

static void bldr_post_process(void)
{
    platform_post_init();
}

static bool wait_for_discon(struct comport_ops *comm, u32 tmo_ms)
{
    bool ret;
    u8 discon[HSHK_DISCON_SZ];
    memset(discon, 0x0, HSHK_DISCON_SZ);

    print("[BLDR] DISCON...");
    if (ret = comm->recv(discon, HSHK_DISCON_SZ, tmo_ms)) {
	print("timeout\n");
	return ret;
    }

    if (0 == memcmp(discon, HSHK_DISCON, HSHK_DISCON_SZ))
	print("OK\n");
    else
	print("protocol mispatch\n");

    return ret;
}

int bldr_load_part(char *name, blkdev_t *bdev, u32 *addr)
{
    part_t *part = part_get(name);

    if (NULL == part) {
        print("%s %s partition not found\n", MOD, name);
        return -1;
    }

    return part_load(bdev, part, addr, 0, 0);
}

static bool bldr_cmd_handler(struct bldr_command_handler *handler, 
    struct bldr_command *cmd, struct bldr_comport *comport)
{
    struct comport_ops *comm = comport->ops;
    u32 attr = handler->attr;

#if CFG_DT_MD_DOWNLOAD
    if (CMD_MATCH(cmd, SWITCH_MD_REQ)) {
        /* SWITCHMD */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        comm->send((u8*)SWITCH_MD_ACK, strlen(SWITCH_MD_ACK));
        platform_modem_download();
        return TRUE;
    }
#endif

    if (CMD_MATCH(cmd, ATCMD_PREFIX)) {
        /* "AT+XXX" */

        if (CMD_MATCH(cmd, ATCMD_NBOOT_REQ)) {
            /* return "AT+OK" to tool */
            comm->send((u8*)ATCMD_OK, strlen(ATCMD_OK));

            g_boot_mode = NORMAL_BOOT;
            g_boot_reason = BR_TOOL_BY_PASS_PWK;

        } else {
            /* return "AT+UNKONWN" to ack tool */
            comm->send((u8*)ATCMD_UNKNOWN, strlen(ATCMD_UNKNOWN));

            return FALSE;
        }
    } else if (CMD_MATCH(cmd, META_STR_REQ)) {
        para_t param;

#if CFG_BOOT_ARGUMENT
	    bootarg.md_type[0] = 0;
	    bootarg.md_type[1] = 0;
#endif
        
        /* "METAMETA" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

	/* for backward compatibility */
	comm->recv((u8*)&param.v0001, sizeof(param.v0001), 2000);
#if CFG_WORLD_PHONE_SUPPORT
        comm->send((u8*)META_ARG_VER_STR, strlen(META_ARG_VER_STR));

        if (0 == comm->recv((u8*)&param.v0001, sizeof(param.v0001), 5000)) {
            g_meta_com_id = param.v0001.usb_type;
	    print("md_type[0] = %d \n", param.v0001.md0_type);
	    print("md_type[1] = %d \n", param.v0001.md1_type);
#if CFG_BOOT_ARGUMENT
	    bootarg.md_type[0] = param.v0001.md0_type;
	    bootarg.md_type[1] = param.v0001.md1_type;
#endif
        }
#endif
        comm->send((u8*)META_STR_ACK, strlen(META_STR_ACK));

#if CFG_WORLD_PHONE_SUPPORT
        wait_for_discon(comm, 1000);
#endif
        g_boot_mode = META_BOOT;
    } else if (CMD_MATCH(cmd, FACTORY_STR_REQ)) {
        para_t param;

        /* "FACTFACT" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v0001, sizeof(param.v0001), 5)) {
            g_meta_com_id = param.v0001.usb_type;
        }

        comm->send((u8*)FACTORY_STR_ACK, strlen(FACTORY_STR_ACK));

        g_boot_mode = FACTORY_BOOT;
    } else if (CMD_MATCH(cmd, META_ADV_REQ)) {
        /* "ADVEMETA" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        comm->send((u8*)META_ADV_ACK, strlen(META_ADV_ACK));

        wait_for_discon(comm, 1000);
	
        g_boot_mode = ADVMETA_BOOT;
    } else if (CMD_MATCH(cmd, ATE_STR_REQ)) {
        para_t param;

        /* "FACTORYM" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v0001, sizeof(param.v0001), 5)) {
            g_meta_com_id = param.v0001.usb_type;
        }

        comm->send((u8*)ATE_STR_ACK, strlen(ATE_STR_ACK));

        g_boot_mode = ATE_FACTORY_BOOT;
    } else if (CMD_MATCH(cmd, FB_STR_REQ)) {

	/* "FASTBOOT" */
	comm->send((u8 *)FB_STR_ACK, strlen(FB_STR_ACK));

	g_boot_mode = FASTBOOT;
    } else {
        print("%s unknown received: \'%s\'\n", MOD, cmd->data);

        return FALSE;
    }

    print("%s '%s' received!\n", MOD, cmd->data);

    return TRUE;

forbidden:
    comm->send((u8*)META_FORBIDDEN_ACK, strlen(META_FORBIDDEN_ACK));
    print("%s '%s' is forbidden!\n", MOD, cmd->data);
    return FALSE;
}

static int bldr_handshake(struct bldr_command_handler *handler)
{
    boot_mode_t mode = 0;
    bool isSBC = 0;
    
#ifdef MTK_SECURITY_SW_SUPPORT
    /* get mode type */
    mode = seclib_brom_meta_mode();
    isSBC = seclib_sbc_enabled();
#endif


    switch (mode) {
    case NORMAL_BOOT:
        /* ------------------------- */
        /* security check            */
        /* ------------------------- */
        if (TRUE == isSBC) {
            handler->attr |= CMD_HNDL_ATTR_COM_FORBIDDEN;
            print("%s META DIS\n", MOD);
        }

        #if CFG_USB_TOOL_HANDSHAKE
        if (TRUE == usb_handshake(handler))
            g_meta_com_type = META_USB_COM;
        #endif
        #if CFG_UART_TOOL_HANDSHAKE
        if (TRUE == uart_handshake(handler))
            g_meta_com_type = META_UART_COM;
        #endif

        break;
    
    case META_BOOT:
        print("%s BR META BOOT\n", MOD);
        g_boot_mode = META_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case FACTORY_BOOT:
        print("%s BR FACTORY BOOT\n", MOD);
        g_boot_mode = FACTORY_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case ADVMETA_BOOT:
        print("%s BR ADVMETA BOOT\n", MOD);
        g_boot_mode = ADVMETA_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    case ATE_FACTORY_BOOT:
        print("%s BR ATE FACTORY BOOT\n", MOD);
        g_boot_mode = ATE_FACTORY_BOOT;
        /* secure META is only enabled on USB connection */
        g_meta_com_type = META_USB_COM;
        break;

    default:
        print("%s UNKNOWN MODE\n", MOD);
        break;
    }

    return 0;
}

static void bldr_wait_forever(void)
{
    /* prevent wdt timeout and clear usbdl flag */
    mtk_wdt_disable();
    platform_safe_mode(0, 0);

    while(1);
}


static int bldr_load_images(u32 *jump_addr)
{
    int ret = 0;
    blkdev_t *bootdev;
    u32 addr = 0;
    char *name;
    u32 spare0 = 0;
    u32 spare1 = 0;
    
    if (NULL == (bootdev = blkdev_get(CFG_BOOT_DEV))) {
        print("%s can't find boot device(%d)\n", MOD, CFG_BOOT_DEV);
	    /* FIXME, should change to global error code */
        return -1;
    }

#if CFG_LOAD_MD_ROM
    /* do not check the correctness */
    addr = CFG_MD1_ROM_MEMADDR;
    bldr_load_part(PART_MD1_ROM, bootdev, &addr);

    addr = CFG_MD2_ROM_MEMADDR;
    bldr_load_part(PART_MD2_ROM, bootdev, &addr);
#endif

#if CFG_LOAD_MD_RAMDISK
    /* do not check the correctness */
    addr = CFG_MD1_RAMDISK_MEMADDR;
    bldr_load_part(PART_MD1_RAMDISK, bootdev, &addr);

    addr = CFG_MD2_RAMDISK_MEMADDR;
    bldr_load_part(PART_MD1_RAMDISK, bootdev, &addr);
#endif

#if CFG_LOAD_CONN_SYS 
    addr = CFG_CONN_SYS_MEMADDR;
    bldr_load_part(PART_CONN_SYS,bootdev, &addr);
#endif

#if CFG_LOAD_SLT_MD
    spare0 = seclib_get_devinfo_with_index(3);
    spare1 = seclib_get_devinfo_with_index(4);
    print("SPARE0:%x, SPARE1:%x\n",spare0,spare1);
    
    if( (spare0 & 0x00400000) == 0 ) // Check Modem enable
    {
      if(    ((spare1 & 0x00000001) == 0) && ((spare1 & 0x0000080) == 0)   ) // FDD + TDD_Only
        {
            //Load FDD + TDD ONLY load
             addr = CFG_FDD_MD_ROM_MEMADDR;
             bldr_load_part(PART_FDD_MD_ROM, bootdev, &addr);
             addr = CFG_TDD_ONLY_ROM_MEMADDR;
             bldr_load_part(PART_TDD_ONLY_ROM, bootdev, &addr);

        }
        else if( ((spare1 & 0x00000001) == 0) && ((spare1 & 0x0000080) == 0x80)  ) // HSPA enable
        {
            //Load FDD SLT load
             addr = CFG_FDD_MD_ROM_MEMADDR;
             bldr_load_part(PART_FDD_MD_ROM, bootdev, &addr);
        }
        else if( ((spare1 & 0x00000001) == 1) && ((spare1 & 0x00000080) == 0) ) // TDD enable
        {
            //Load TDD SLT load
             addr = CFG_TDD_MD_ROM_MEMADDR;
             bldr_load_part(PART_TDD_MD_ROM, bootdev, &addr);
        }
        else if( ((spare1 & 0x00000001) == 1) && ((spare1 & 0x00000080) == 0x80) ) //2G
        {
            //LOAD 2G SLT Load
            addr = CFG_2G_MD_ROM_MEMADDR;
            bldr_load_part(PART_TWOG_MD_ROM, bootdev, &addr);
        }
        else{
            print("SLT MD LOAD fail, SPARE0:%x, SPARE1:%x \n",spare0,spare1);
        }
    }
    else{
        print("Modem is not enable by EFUSE, SPARE0:%x, SPARE1:%x \n",spare0,spare1);
    }
#endif

#if CFG_LOAD_AP_ROM
    addr = CFG_AP_ROM_MEMADDR;
    ret = bldr_load_part(PART_AP_ROM, bootdev, &addr);
    if (ret)
	return ret;
    *jump_addr = addr;
#elif CFG_LOAD_UBOOT
    addr = CFG_UBOOT_MEMADDR;
    ret = bldr_load_part(PART_UBOOT, bootdev, &addr);
    if (ret)
       return ret; 
    *jump_addr = addr;
#endif

#if CFG_TRUSTONIC_TEE_SUPPORT
    addr = CFG_DRAM_ADDR + memory_size() - MAX_TEE_DRAM_SIZE;
    ret = bldr_load_part(PART_TEE1, bootdev, &addr);
    if (ret) {
	g_secure_dram_size = 0;
	addr = CFG_DRAM_ADDR + memory_size() - MAX_TEE_DRAM_SIZE;
	ret = bldr_load_part(PART_TEE2, bootdev, &addr);
	if (ret)
	    return ret;
    }

    tee_boot_addr = addr;
    /* TEE jump addr directly re-used in bldr_tee_jump */
    /* *jump_addr = addr; Directly computed in Jump fonction */
#endif

    return ret;
}

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/
void bldr_jump(u32 addr, u32 arg1, u32 arg2)
{
    platform_wdt_kick();

    /* disable preloader safe mode */
    platform_safe_mode(0, 0);

    print("\n%s jump to 0x%x\n", MOD, addr);
    print("%s <0x%x>=0x%x\n", MOD, addr, *(u32*)addr);
    print("%s <0x%x>=0x%x\n", MOD, addr + 4, *(u32*)(addr + 4));

#if CFG_TRUSTONIC_TEE_SUPPORT	
    if((DOWNLOAD_BOOT != g_boot_mode) && (UNKNOWN_BOOT != g_boot_mode))
    {
        /* In case of DOWNLOAD_BOOT (preloader reflash) we don't start t-base */
        bldr_tee_jump(addr, arg1, arg2);
    }
#endif

    jump(addr, arg1, arg2);
}

void main(void)
{
    struct bldr_command_handler handler;
    u32 jump_addr;

    bldr_pre_process();

    #ifdef HW_INIT_ONLY 
    bldr_wait_forever();
    #endif

    handler.priv = NULL;
    handler.attr = 0;
    handler.cb   = bldr_cmd_handler;

    bldr_handshake(&handler);

#if !CFG_FPGA_PLATFORM
    /* security check */
    sec_lib_read_secro();
    sec_boot_check();
    device_APC_dom_setup();
#endif

    if (0 != bldr_load_images(&jump_addr)) {
	    print("%s Second Bootloader Load Failed\n", MOD);
	    goto error;
    }

    bldr_post_process();
    bldr_jump(jump_addr, (u32)&bootarg, sizeof(boot_arg_t));

error:
    platform_error_handler();
}

