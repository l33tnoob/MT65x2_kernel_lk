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

/*============================================================================*/
/* INTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static void bldr_pre_process(void)
{
    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = 0;
    #endif

#if defined(CFG_USB_AUTO_DETECT)
	platform_usbdl_flag_check();
#endif

    /* enter preloader safe mode */
#if CFG_EMERGENCY_DL_SUPPORT
    platform_safe_mode(1, CFG_EMERGENCY_DL_TIMEOUT_MS);
#endif

    /* essential hardware initialization. e.g. timer, pll, uart... */
    platform_pre_init();

    #ifdef PL_PROFILING
    printf("#T#total_preplf_init=%d\n", get_timer(0));
    #endif
    print("\n%s Build Time: %s\n", MOD, BUILD_TIME);

    g_boot_mode = NORMAL_BOOT;

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* hardware initialization */
    platform_init();

    #ifdef PL_PROFILING
    printf("#T#total_plf_init=%d\n", get_timer(profiling_time));
    #endif
#if CFG_UART_TOOL_HANDSHAKE && (!defined(CFG_MEM_PRESERVED_MODE))
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* init uart handshake for sending 'ready' to tool and receiving handshake
     * pattern from tool in the background and we'll see the pattern later.
     * this can reduce the handshake time.
     */
    uart_handshake_init();

    #ifdef PL_PROFILING
    printf("#T#UART_hdshk=%d\n", get_timer(profiling_time));
    #endif
#endif  //#if CFG_UART_TOOL_HANDSHAKE && (!defined(CFG_MEM_PRESERVED_MODE))

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    part_init();
    part_dump();

    #ifdef PL_PROFILING
    printf("#T#part_init+dump=%d\n", get_timer(profiling_time));
    #endif

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    /* init security library */
    sec_lib_init();

    #ifdef PL_PROFILING
    printf("#T#sec_lib_init=%d\n", get_timer(profiling_time));
    #endif
}

static void bldr_post_process(void)
{
    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = get_timer(0);
    #endif
    platform_post_init();

    #ifdef PL_PROFILING
    printf("#T#total_plf_post_init=%d\n", get_timer(profiling_time));
    #endif
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

#if CFG_LEGACY_USB_DOWNLOAD
    /* "DOWNLOAD" */
    if (CMD_MATCH(cmd, DM_STR_DOWNLOAD_REQ) && (comport->type == COM_USB)) {
        comm->send((u8*)DM_STR_DOWNLOAD_ACK, strlen(DM_STR_DOWNLOAD_ACK));
        print("%s download mode detected!\n", MOD);
        download_handler();
        g_boot_mode = DOWNLOAD_BOOT;
        return TRUE;
    }
#endif

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

        /* "METAMETA" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
        }

        comm->send((u8*)META_STR_ACK, strlen(META_STR_ACK));
        wait_for_discon(comm, 1000);
        g_boot_mode = META_BOOT;
    } else if (CMD_MATCH(cmd, FACTORY_STR_REQ)) {
        para_t param;

        /* "FACTFACT" */
        if (attr & CMD_HNDL_ATTR_COM_FORBIDDEN)
            goto forbidden;

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
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

        if (0 == comm->recv((u8*)&param.v1, sizeof(param.v1), 5)) {
            g_meta_com_id = param.v1.usb_type;
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

    /* get mode type */
	/* Since entring META mode is from preloader not BROM,
	   we forcely set mode as NORMAL_BOOT */
    mode = seclib_brom_meta_mode();

    switch (mode) {
    case NORMAL_BOOT:
        /* ------------------------- */
        /* security check            */
        /* ------------------------- */
        if (TRUE == seclib_sbc_enabled()) {
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

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/
void bldr_jump(u32 addr, u32 arg1, u32 arg2)
{
    platform_wdt_kick();

    /* disable preloader safe mode */
    platform_safe_mode(0, 0);

    apmcu_disable_dcache();
    apmcu_dcache_clean_invalidate();
    apmcu_dsb();
    apmcu_icache_invalidate();
    apmcu_disable_icache();
    apmcu_isb();
    apmcu_disable_smp();

    print("\n%s jump to 0x%x\n", MOD, addr);
    print("%s <0x%x>=0x%x\n", MOD, addr, *(u32*)addr);
    print("%s <0x%x>=0x%x\n", MOD, addr + 4, *(u32*)(addr + 4));

    jump(addr, arg1, arg2);
}

void main(void)
{
#if !defined(CFG_MEM_PRESERVED_MODE)
    struct bldr_command_handler handler;
#endif

    blkdev_t *bootdev;
    u32 addr;
    char *name;

#if defined(CFG_SRAM_PRELOADER_MODE)
    //jump to mem preloader directly
    //mem_baseaddr is defined in link_sram_descriptor.ld
    addr = (u32) &mem_baseaddr;
    jump(addr, BOOT_ARGUMENT_ADDR, sizeof(boot_arg_t));
#else   //#if defined(CFG_SRAM_PRELOADER_MODE)

    #ifdef PL_PROFILING
    u32 profiling_time;
    profiling_time = 0;
    #endif

    //Change setting to improve L2 CACHE SRAM access stability
    //CACHE_MEM_DELSEL: 0x10200014
    //bit 3:0		l2data_delsel	Adjusts memory marco timing
    //change setting: default=0xA  new=0xB
    *(volatile unsigned int *)0x10200014 = 0xAAAB;

    addr = 0;
    bldr_pre_process();

#ifdef TINY_BOOTLOADER
    while(1);
#endif

#if !defined(CFG_MEM_PRESERVED_MODE)
    handler.priv = NULL;
    handler.attr = 0;
    handler.cb   = bldr_cmd_handler;

    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    bldr_handshake(&handler);

    #ifdef PL_PROFILING
    printf("#T#bldr_hdshk=%d\n", get_timer(profiling_time));
    #endif
#endif

    if (NULL == (bootdev = blkdev_get(CFG_BOOT_DEV))) {
        print("%s can't find boot device(%d)\n", MOD, CFG_BOOT_DEV);
        goto error;
    }

#if defined(LOAD_NORMAL_BOOT_PRELOADER)
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif

    {
        volatile u32 cache_cfg;

        #define L2C_SIZE_CFG_OFF 5
        //enable L2 sram for DA
        cache_cfg = DRV_Reg(APMCUSYS_CONFIG_BASE);
        cache_cfg &= ~(0x7 << L2C_SIZE_CFG_OFF);
        DRV_WriteReg(APMCUSYS_CONFIG_BASE, cache_cfg);

        //enable audio sysram clk for DA
        *(volatile unsigned int *)(0x10000084) = 0x02000000;
    }

    addr = CFG_UBOOT_MEMADDR;
    printf("load preloader=0x%x\n",addr);
    if (bldr_load_part(PART_PRELOADER, bootdev, &addr) != 0)
        goto error;

    addr = 0x02007200;
    printf("memcpy preloader=0x%x\n", addr);
    memcpy((void *)addr,(void *) CFG_UBOOT_MEMADDR,(int) 0x18E00);
    #ifdef PL_PROFILING
    printf("#T#ld_lk=%d\n", get_timer(profiling_time));
    #endif

    addr = 0x02007500;
    apmcu_disable_dcache();
    apmcu_dcache_clean_invalidate();
    apmcu_dsb();
    apmcu_icache_invalidate();
    apmcu_disable_icache();
    apmcu_isb();
    apmcu_disable_smp();
    printf("jump to preloader=0x%x\n", addr);

//    while( *(volatile unsigned int *)(0x10001428) != 0x000000AA)  ;

    jump((u32) addr, BOOT_ARGUMENT_ADDR, sizeof(boot_arg_t));
#endif


#if CFG_LOAD_DSP_ROM
    /* DSP is no more available in MT6589/MT6583 */
#endif

#if CFG_LOAD_MD_FS
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    addr = CFG_USE_HEADER_MEMADDR;
    name = PART_BOOTIMG;
    if (bldr_load_part(name, bootdev, &addr) != 0)
        ;   //goto error;
        // MD_FS partition may be empty

    #ifdef PL_PROFILING
    printf("#T#ld_MDFS=%d\n", get_timer(profiling_time));
    #endif
#endif

#if CFG_LOAD_MD_ROM
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    addr = CFG_USE_HEADER_MEMADDR;
    name = PART_RECOVERY;
    if (bldr_load_part(name, bootdev, &addr) != 0)
        goto error;

    #ifdef PL_PROFILING
    printf("#T#ld_MDROM=%d\n", get_timer(profiling_time));
    #endif
#endif

#if CFG_LOAD_AP_ROM
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    addr = CFG_USE_HEADER_MEMADDR;
    name = PART_UBOOT;
    if (bldr_load_part(name, bootdev, &addr) != 0)
        goto error;

    #ifdef PL_PROFILING
    printf("#T#ld_APROM=%d\n", get_timer(profiling_time));
    #endif
#endif

#if CFG_LOAD_UBOOT
    #ifdef PL_PROFILING
    profiling_time = get_timer(0);
    #endif
    addr = CFG_UBOOT_MEMADDR;
    if (bldr_load_part(PART_UBOOT, bootdev, &addr) != 0)
        goto error;

    #ifdef PL_PROFILING
    printf("#T#ld_lk=%d\n", get_timer(profiling_time));
    #endif
#endif

    bldr_post_process();
    bldr_jump(addr, BOOT_ARGUMENT_ADDR, sizeof(boot_arg_t));

error:
    platform_error_handler();
#endif  //end of #if !defined(CFG_SRAM_PRELOADER_MODE)

}


