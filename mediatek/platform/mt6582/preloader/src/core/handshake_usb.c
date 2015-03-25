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

#include "usbtty.h"

#if CFG_USB_TOOL_HANDSHAKE

/*============================================================================*/
/* CONSTAND DEFINITIONS                                                       */
/*============================================================================*/
#define MOD                 "[TOOL]"
#define USB_SYNC_TIME        (CFG_USB_HANDSHAKE_TIMEOUT)
#define USB_PORT_DOWN_TIME   1000
#define CDC_DTR_MASK         0x01
/*============================================================================*/
/* EXTERNAL FUNCTIONS DECLARATION                                             */
/*============================================================================*/
extern int usbdl_init(void);
extern int usbdl_configured(void);
extern void service_interrupts(void);

/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/

extern int g_usbphy_ok;
extern int g_usb_port_state;



/*============================================================================*/
/* INTERNAL FUNCTIONS                                                         */
/*============================================================================*/
static bool usb_connect(u32 tmo)
{
    ulong start_time = get_timer(0);
    bool result = FALSE;
    u32 i = 1;

    mt_usb_disconnect_internal();
    mt_usb_connect_internal();

#if CFG_USBIF_COMPLIANCE
    /* USB compliance test: 100mA charging current when USB is unconfigured. */
    platform_set_chrg_cur(70);
#endif

    print("%s Enumeration(Start)\n", MOD);

    do {
        /* kick wdt to avoid cpu reset during usb driver installation if not present */
        platform_wdt_all_kick();
        service_interrupts();

        if (usbdl_configured()) {
            #if CFG_USBIF_COMPLIANCE
            /* USB compliance test: 500mA charging current when USB is configured but
             * we set the charging current to 450mA since 500mA doesn't support in the
             * platform.
             */
            platform_set_chrg_cur(450);
            #endif
            result = TRUE;
            break;
        }

        if (tmo) {
            /* enable timeout mechanism */
            if (get_timer(start_time) > tmo)
                break;
            #if !CFG_FPGA_PLATFORM
            /* cable plugged-out and power key detection each 1 second */
            if (get_timer(start_time) > i * 1000) {
                if (!usb_accessory_in() && !pmic_detect_powerkey())
                    pl_power_off();

		#if !CFG_EVB_PLATFORM
                /* check bypass power key from the 2nd second */
                if (i > 1 && pmic_detect_powerkey()) {
                    print("%s Enumeration(Skip): powerkey pressed\n", MOD);
                    break;
                }
                i++;
		#endif
            }
            #endif
        }
    } while(1);

    print("%s Enumeration(End): %s %dms \n", MOD, result == TRUE ? "OK" : "TMO",
        get_timer(start_time));

    return result;
}

static void usb_disconnect(void)
{
    mt_usb_disconnect_internal();
}

static int usb_send(u8 *buf, u32 len)
{
    mt_usbtty_putcn((int)len, (char*)buf, 0);
    mt_usbtty_flush();
    return 0;
}

static int usb_recv(u8 *buf, u32 size, u32 tmo_ms)
{
    ulong start_time = get_timer(0);
    u32 dsz;
    u32 tmo_en = (tmo_ms) ? 1 : 0;
    u8 *ptr = buf;

    if (!size)
        return 0;

    while (1) {
        if (tmo_en && (get_timer(start_time) > tmo_ms)){
            print("%s : usb receive timeout\n", MOD);
            return -1;
        }

        if (!tmo_en) {
            /* kick watchdog to avoid cpu reset but don't kick pmic wdt since
             * it could use i2c operations during a communication command protocl
             * that could break the atomic operation of 2 pmic i2c communication
             * commands. i2c operations should be not used during usb send or recv.
             * for example:
             *
             * i2c_write(pmic_addr) -> usb_recv() -> i2c_read(&pmic_data).
             */
            platform_wdt_kick();
        }

        dsz = mt_usbtty_query_data_size();
        if (dsz) {
            dsz = dsz < size ? dsz : size;
            mt_usbtty_getcn(dsz, (char*)ptr);
            ptr  += dsz;
            size -= dsz;
        }
        if (size == 0)
            break;
    }

    return 0;
}

static bool usb_listen(struct bldr_comport *comport, uint8 *data, uint32 size, uint32 tmo_ms)
{
    ulong  start_time = get_timer(0);
    uint32 dsz;
    uint32 tmo_en = (tmo_ms) ? 1 : 0;
    uint8 *ptr = data;

    if (!size)
        return FALSE;

    while (1) {
        if (tool_is_present())
            mt_usbtty_puts(HSHK_COM_READY); /* "READY" */

        if (tmo_en && (get_timer(start_time) > tmo_ms)){
            print("%s : usb listen timeout\n", MOD);
            return FALSE;
         }

        if (!tmo_en) {
            /* kick watchdog to avoid cpu reset */
            platform_wdt_all_kick();
        }

        dsz = mt_usbtty_query_data_size();
        if (dsz) {
            dsz = dsz < size ? dsz : size;
            mt_usbtty_getcn(dsz, (char*)ptr);
            #if CFG_USB_DOWNLOAD 
            if (*ptr == 0xa0) {
                print("%s sync time %dms\n", MOD, get_timer(start_time));
                usbdl_handler(comport, 300);
                print("%s : ignore %d bytes garbage data\n", MOD, dsz);
                continue; /* ingore received data */
            }
            #endif
            ptr  += dsz;
            size -= dsz;
        }
        if (size == 0)
            break;

        udelay(20000); /* 20ms */
    }

    print("%s sync time %dms\n", MOD, get_timer(start_time));

    return TRUE;
}

static bool usb_port_down(uint32 tmo_ms)
{
    ulong  start_time = get_timer(0);

    /* check if usb comport close */
    if (!(g_usb_port_state & CDC_DTR_MASK))
        return TRUE;

    while (1) {

        if (get_timer(start_time) > tmo_ms)
            return FALSE;

        /* kick watchdog to avoid cpu reset */
        platform_wdt_all_kick();

        mt_usbtty_query_data_size();

        /* check if usb comport close */
        if (!(g_usb_port_state & CDC_DTR_MASK))
            break;

        udelay(20000); /* 20ms */
    }

    print("%s usb port down: %dms\n", MOD, get_timer(start_time));

    return TRUE;
}

static bool usb_handshake_handler(struct bldr_command_handler *handler, uint32 tmo)
{
    uint8 buf[HSHK_TOKEN_SZ + 1] = {'\0'};
    struct bldr_comport comport;
    struct bldr_command cmd;
    struct comport_ops usb_ops = {usb_send, usb_recv};

    comport.type = COM_USB;
    comport.tmo  = tmo;
    comport.ops  = &usb_ops;

    if (FALSE == usb_listen(&comport, buf, HSHK_TOKEN_SZ, tmo)) {
        print("%s <USB> cannot detect tools!\n",MOD);
        return FALSE;
    }

    cmd.data = &buf[0];
    cmd.len  = HSHK_TOKEN_SZ;

    return handler->cb(handler, &cmd, &comport);
}

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/
bool usb_handshake(struct bldr_command_handler *handler)
{
    uint32 enum_tmo = CFG_USB_ENUM_TIMEOUT_EN ? USB_ENUM_TIMEOUT : 0;
    uint32 handshake_tmo = CFG_USB_HANDSHAKE_TIMEOUT_EN ? USB_SYNC_TIME : 0;
    bool result = FALSE;
    bool force_download = FALSE;

    platform_vusb_on();

    force_download = platform_com_wait_forever_check();

    if (TRUE == force_download) {
        enum_tmo = 0;
        handshake_tmo = 0;
    } else if (!usb_cable_in()) {
         print("%s PMIC not dectect usb cable!\n", MOD);
         return FALSE;
    }

    #if CFG_USB_AUTO_DETECT
    platform_usb_auto_detect_flow();	
	#endif

    print("%s USB enum timeout (%s), handshake timeout(%s)\n", MOD,
        enum_tmo ? "Yes" : "No",
        handshake_tmo ? "Yes" : "No");

    usbdl_init();
    udelay(1000);
    usb_disconnect();

    if (usb_connect(enum_tmo) == FALSE) {
        print("%s USB enum timeout!\n", MOD);
        /* USB enum fail when connecting to a standby PC, remove ASSERT */
    	/* ASSERT(g_usbphy_ok); */
        goto end;
    }

    udelay(1000);
    if (FALSE == usb_handshake_handler(handler, handshake_tmo)) {
        goto end;
    }

    result = TRUE;

    if (FALSE == usb_port_down(USB_PORT_DOWN_TIME)) {
        print("%s USB port down timeout!\n", MOD);
    }

end:
    usb_service_offline();

#if CFG_USBIF_COMPLIANCE
    /* USB compliance test: 100mA charging current when USB is unconfigured. */
    platform_set_chrg_cur(70);
#endif

    return result;
}

#endif /* CFG_USB_TOOL_HANDSHAKE */

