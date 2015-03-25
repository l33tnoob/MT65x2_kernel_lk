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

/*
 * This is sample code for WLAN test library implementation
 */

#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <cutils/log.h>
#include <cutils/misc.h> /* load_file() */
#include <libnvram.h>
#include <Custom_NvRam_LID.h>
#include <CFG_Wifi_File.h>

#include "iwlib.h"
#include "libwifitest.h"

#define DBG 1

extern int init_module(void *, unsigned long, const char *);

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#ifndef BIT
#define BIT(n)          ((uint32_t) 1 << (n))
#define BITS(m,n)       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

#define MAC_ADDR_LEN                            6

#define IS_BMCAST_MAC_ADDR(_pucDestAddr)            \
        ((bool) ( ((uint8_t *)(_pucDestAddr))[0] & BIT(0) ))

#define EQUAL_MAC_ADDR(_pucDestAddr, _pucSrcAddr)   \
        (!memcmp(_pucDestAddr, _pucSrcAddr, MAC_ADDR_LEN))

/* Debug print format string for the MAC Address */
#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"

/* Debug print argument for the MAC Address */
#define MAC2STR(a)  ((uint8_t *)a)[0], ((uint8_t *)a)[1], ((uint8_t *)a)[2], \
    ((uint8_t *)a)[3], ((uint8_t *)a)[4], ((uint8_t *)a)[5]

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#ifndef WIFI_DRIVER_MODULE_PATH
#define WIFI_DRIVER_MODULE_PATH     "/system/lib/modules/wlan.ko"
#endif
#ifndef WIFI_DRIVER_MODULE_NAME
#define WIFI_DRIVER_MODULE_NAME     "wlan"
#endif
#ifndef WIFI_DRIVER_MODULE_ARG
#define WIFI_DRIVER_MODULE_ARG      ""
#endif

#ifndef WIFI_IF_NAME
#define WIFI_IF_NAME                "wlan0"
#endif

static const char DRIVER_PROP_NAME[]    = "wlan.driver.status";
static const char DRIVER_MODULE_NAME[]  = WIFI_DRIVER_MODULE_NAME;
static const char DRIVER_MODULE_TAG[]   = WIFI_DRIVER_MODULE_NAME " ";
static const char DRIVER_MODULE_PATH[]  = WIFI_DRIVER_MODULE_PATH;
static const char DRIVER_MODULE_ARG[]   = WIFI_DRIVER_MODULE_ARG;
static const char MODULE_FILE[]         = "/proc/modules";

#define IOCTL_SET_STRUCT                (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT                (SIOCIWFIRSTPRIV + 9)
#define IOCTL_SET_STRUCT_FOR_EM         (SIOCIWFIRSTPRIV + 11)

#define PRIV_CMD_OID                    15

/* RF Test specific OIDs */
#define OID_CUSTOM_TEST_MODE                            0xFFA0C901
#define OID_CUSTOM_ABORT_TEST_MODE                      0xFFA0C906
#define OID_CUSTOM_MTK_WIFI_TEST_                       0xFFA0C911

/* command mask */
#define TEST_FUNC_IDX_MASK              BITS(0,7)
#define TEST_SET_CMD_OFFSET_MASK        BITS(16,31)
#define TEST_SET_CMD_OFFSET             16

/* RF Test Properties */
#define RF_AT_PARAM_RATE_MCS_MASK   BIT(31)
#define RF_AT_PARAM_RATE_MASK       BITS(0,7)
#define RF_AT_PARAM_RATE_1M         0
#define RF_AT_PARAM_RATE_2M         1
#define RF_AT_PARAM_RATE_5_5M       2
#define RF_AT_PARAM_RATE_11M        3
#define RF_AT_PARAM_RATE_6M         4
#define RF_AT_PARAM_RATE_9M         5
#define RF_AT_PARAM_RATE_12M        6
#define RF_AT_PARAM_RATE_18M        7
#define RF_AT_PARAM_RATE_24M        8
#define RF_AT_PARAM_RATE_36M        9
#define RF_AT_PARAM_RATE_48M        10
#define RF_AT_PARAM_RATE_54M        11

#define NULL_MAC_ADDR               {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
typedef struct _NDIS_TRANSPORT_STRUCT {
    uint32_t    ndisOidCmd;
    uint32_t    inNdisOidlength;
    uint32_t    outNdisOidLength;
    uint8_t     ndisOidContent[16];
} NDIS_TRANSPORT_STRUCT, *P_NDIS_TRANSPORT_STRUCT;

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
    uint32_t u4FuncIndex;
    uint32_t u4FuncData;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef enum _ENUM_RF_AT_FUNCID_T {

    RF_AT_FUNCID_VERSION = 0,
    RF_AT_FUNCID_COMMAND,
    RF_AT_FUNCID_POWER,
    RF_AT_FUNCID_RATE,
    RF_AT_FUNCID_PREAMBLE,
    RF_AT_FUNCID_ANTENNA,
    RF_AT_FUNCID_PKTLEN,
    RF_AT_FUNCID_PKTCNT,
    RF_AT_FUNCID_PKTINTERVAL,
    RF_AT_FUNCID_TEMP_COMPEN,
    RF_AT_FUNCID_TXOPLIMIT,
    RF_AT_FUNCID_ACKPOLICY,
    RF_AT_FUNCID_PKTCONTENT,
    RF_AT_FUNCID_RETRYLIMIT,
    RF_AT_FUNCID_QUEUE,
    RF_AT_FUNCID_BANDWIDTH,
    RF_AT_FUNCID_GI,
    RF_AT_FUNCID_STBC,
    RF_AT_FUNCID_CHNL_FREQ,
    RF_AT_FUNCID_RIFS,
    RF_AT_FUNCID_TRSW_TYPE,
    RF_AT_FUNCID_RF_SX_SHUTDOWN,
    RF_AT_FUNCID_PLL_SHUTDOWN,
    RF_AT_FUNCID_SLOW_CLK_MODE,
    RF_AT_FUNCID_ADC_CLK_MODE,
    RF_AT_FUNCID_MEASURE_MODE,
    RF_AT_FUNCID_VOLT_COMPEN,
    RF_AT_FUNCID_DPD_TX_GAIN,
    RF_AT_FUNCID_DPD_MODE,
    RF_AT_FUNCID_TSSI_MODE,
    RF_AT_FUNCID_TX_GAIN_CODE,
    RF_AT_FUNCID_TX_PWR_MODE,

    /* Query command */
    RF_AT_FUNCID_TXED_COUNT = 32,
    RF_AT_FUNCID_TXOK_COUNT,
    RF_AT_FUNCID_RXOK_COUNT,
    RF_AT_FUNCID_RXERROR_COUNT,
    RF_AT_FUNCID_RESULT_INFO,
    RF_AT_FUNCID_TRX_IQ_RESULT,
    RF_AT_FUNCID_TSSI_RESULT,
    RF_AT_FUNCID_DPD_RESULT,
    RF_AT_FUNCID_RXV_DUMP,
    RF_AT_FUNCID_RX_PHY_STATIS,
    RF_AT_FUNCID_MEASURE_RESULT,
    RF_AT_FUNCID_TEMP_SENSOR,
    RF_AT_FUNCID_VOLT_SENSOR,
    RF_AT_FUNCID_READ_EFUSE,
    RF_AT_FUNCID_RX_RSSI,

    /* Set command */
    RF_AT_FUNCID_SET_DPD_RESULT = 64,
    RF_AT_FUNCID_SET_CW_MODE,
    RF_AT_FUNCID_SET_JAPAN_CH14_FILTER,
    RF_AT_FUNCID_WRITE_EFUSE,
    RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
    RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
    RF_AT_FUNCID_SET_RXOK_MATCH_RULE,

} ENUM_RF_AT_FUNCID_T;

typedef enum _ENUM_RF_AT_COMMAND_T {

    RF_AT_COMMAND_STOPTEST = 0,
    RF_AT_COMMAND_STARTTX,
    RF_AT_COMMAND_STARTRX,
    RF_AT_COMMAND_RESET,
    RF_AT_COMMAND_OUTPUT_POWER,     /* Payload */
    RF_AT_COMMAND_LO_LEAKAGE,       /* Local freq is renamed to Local leakage */
    RF_AT_COMMAND_CARRIER_SUPPR,    /* OFDM (LTF/STF), CCK (PI,PI/2) */
    RF_AT_COMMAND_TRX_IQ_CAL,
    RF_AT_COMMAND_TSSI_CAL,
    RF_AT_COMMAND_DPD_CAL,
    RF_AT_COMMAND_CW,
    RF_AT_COMMAND_NUM

} ENUM_RF_AT_COMMAND_T;

typedef enum _ENUM_RF_AT_PREAMBLE_T {

    RF_AT_PREAMBLE_NORMAL = 0,
    RF_AT_PREAMBLE_CCK_SHORT,
    RF_AT_PREAMBLE_11N_MM,
    RF_AT_PREAMBLE_11N_GF,
    RF_AT_PREAMBLE_NUM

} ENUM_RF_AT_PREAMBLE_T;

typedef enum _ENUM_RF_AT_BW_T {

    RF_AT_BW_20 = 0,
    RF_AT_BW_40,
    RF_AT_BW_U20,
    RF_AT_BW_L20,
    RF_AT_BW_NUM

} ENUM_RF_AT_BW_T, *P_ENUM_RF_AT_BW_T;

typedef enum _ENUM_RF_AT_RXOK_MATCH_RULE_T {

    RF_AT_RXOK_DISABLED = 0,
    RF_AT_RXOK_MATCH_RA_ONLY,
    RF_AT_RXOK_MATCH_TA_ONLY,
    RF_AT_RXOK_MATCH_RA_TA,
    RF_AT_RXOK_NUM

} ENUM_RF_AT_RXOK_MATCH_RULE_T, *P_ENUM_RF_AT_RXOK_MATCH_RULE_T;



/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
#if 0
static int wifi_rfkill_id = -1;
static char *wifi_rfkill_state_path = NULL;
#endif
static int skfd = -1;
static bool fgIsTesting = false;

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to initial rfkill variables
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_init_rfkill(
    void
    )
{
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;

    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            ALOGW("[%s] open(%s) failed: %s (%d)\n", __func__, path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
            wifi_rfkill_id = id;
            break;
        }
    }

    asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state",
            wifi_rfkill_id);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to turn on/off Wi-Fi interface
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
#if 0
static int
wifi_set_power(
    int on
    )
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    ALOGD("[%s] %d", __func__, on);

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) {
            goto out;
        }
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    ALOGD("[%s] %s", __func__, wifi_rfkill_state_path);
    if (fd < 0) {
        ALOGE("open(%s) for write failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("write(%s) failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    ret = 0;

out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}
#else
#define WIFI_POWER_PATH                 "/dev/wmtWifi"

int wifi_set_power(int enable) {
    int sz;
    int fd = -1;
    const char buffer = (enable ? '1' : '0');

    fd = open(WIFI_POWER_PATH, O_WRONLY);
    if (fd < 0) {
        ALOGE("Open \"%s\" failed", WIFI_POWER_PATH);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        ALOGE("Set \"%s\" [%c] failed", WIFI_POWER_PATH, buffer);
        goto out;
    }

out:
    if (fd >= 0) close(fd);
    return 0;
}

#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to insert module
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
insmod(
    const char *filename,
    const char *args
    )
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check if driver is loaded or not
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
check_driver_loaded(
    void
    )
{
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG)+10];

    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        ALOGW("Could not open %s: %s", MODULE_FILE, strerror(errno));
        return 0;
    }

    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }

    fclose(proc);
    return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to extract interface name
 * 
 * @param   name    pointer to name buffer
 *          nsize   size of name buffer
 *          buf     current position in buffer
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline char *
iw_get_ifname(
    char *name,
    int	  nsize,
    char *buf
    )
{
    char *end;

    /* Skip leading spaces */
    while(isspace(*buf))
        buf++;

    end = strrchr(buf, ':');

    /* Not found ??? To big ??? */
    if((end == NULL) || (((end - buf) + 1) > nsize))
        return(NULL);

    /* Copy */
    memcpy(name, buf, (end - buf));
    name[end - buf] = '\0';

    /* Return value currently unused, just make sure it's non-NULL */
    return(end);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to check whether wlan0 has been spawn or not
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
find_wifi_device(
    void
    )
{
    FILE *fh;
    char  buff[1024];
    int   ret = -1;

    fh = fopen(PROC_NET_DEV, "r");

    if(fh != NULL) {
        /* Success : use data from /proc/net/wireless */
        /* Eat 2 lines of header */
        fgets(buff, sizeof(buff), fh);
        fgets(buff, sizeof(buff), fh);

        /* Read each device line */
        while(fgets(buff, sizeof(buff), fh)) {
            char  name[IFNAMSIZ + 1];
            char *s;

            /* Skip empty or almost empty lines. It seems that in some
             * *        * cases fgets return a line with only a newline. */
            if ((buff[0] == '\0') || (buff[1] == '\0'))
                continue;
            /* Extract interface name */
            s = iw_get_ifname(name, sizeof(name), buff);

            if(s) {
                ALOGD("[%s] %s", __func__, name);
                if (strcmp(name, WIFI_IF_NAME) == 0 ){
                    ret = 0;
                    break;
                }
            }
        }

        fclose(fh);
    }

    return ret;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief   Wrapper to push some Wireless Parameter in the driver
 * 
 * @param   request Wireless extension identifier
 *          pwrq    Pointer to wireless extension request
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static inline int
ioctl_iw_ext(
    int             request,
    struct iwreq   *pwrq
    )
{
    if(skfd > 0) {
        /* Set device name */
        strncpy(pwrq->ifr_name, WIFI_IF_NAME, IFNAMSIZ);

        /* Do the request */
        return(ioctl(skfd, request, pwrq));
    }
    else {
        return -1;
    }
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API is to ask underlying software to enter/leave RF test mode
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int 
wifi_switch_test_mode(
    int on
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure NDIS_TRANSPORT_STRUC */
    if(on == 1) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_TEST_MODE;
    }
    else if(on == 0) {
        rNdisStruct.ndisOidCmd = OID_CUSTOM_ABORT_TEST_MODE;
    }
    else {
        return -1;
    }

    rNdisStruct.inNdisOidlength = 0;
    rNdisStruct.outNdisOidLength = 0;

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT, &wrq);

    if(retval == 0) {
        if(on == 1) {
            fgIsTesting = true;
        }
        else {
            fgIsTesting = false;
        }
    }

    return retval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test set commands
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int 
WIFI_TEST_set(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_SET_STRUCT, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
        }
    }

    return retval;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This API provided a generic service for RF test query commands
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int 
WIFI_TEST_get(
    uint32_t  u4FuncIndex,
    uint32_t  u4FuncData,
    uint32_t *pu4FuncIndex,
    uint32_t *pu4FuncData
    )
{
    int retval;
    struct iwreq wrq;
    NDIS_TRANSPORT_STRUCT rNdisStruct;
    P_PARAM_MTK_WIFI_TEST_STRUC_T prTestStruct;

    prTestStruct = (P_PARAM_MTK_WIFI_TEST_STRUC_T)rNdisStruct.ndisOidContent;

    /* zeroize */
    memset(&wrq, 0, sizeof(struct iwreq));

    /* configure TEST_STRUCT */
    prTestStruct->u4FuncIndex = u4FuncIndex;
    prTestStruct->u4FuncData = u4FuncData;

    /* configure NDIS_TRANSPORT_STRUC */
    rNdisStruct.ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST_;
    rNdisStruct.inNdisOidlength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);
    rNdisStruct.outNdisOidLength = sizeof(PARAM_MTK_WIFI_TEST_STRUC_T);

    /* configure struct iwreq */
    wrq.u.data.pointer = &rNdisStruct;
    wrq.u.data.length = sizeof(NDIS_TRANSPORT_STRUCT);
    wrq.u.data.flags = PRIV_CMD_OID;

    retval = ioctl_iw_ext(IOCTL_GET_STRUCT, &wrq);

    if(retval == 0) {
        if(pu4FuncIndex) {
            *pu4FuncIndex = prTestStruct->u4FuncIndex;
        }

        if(pu4FuncData) {
            *pu4FuncData = prTestStruct->u4FuncData;
        }
    }

    return retval;
}


/* API
 * ========================================================================== */
bool WIFI_TEST_OpenDUT(void)
{
    int count = 60;
    bool retval = false;

    ALOGD("[%s] entry\n", __func__);

    wifi_set_power(1);

    if (!check_driver_loaded()) {
        ALOGD("[%s] loading wifi driver ... ...\n", __func__);

        if (insmod(DRIVER_MODULE_PATH, DRIVER_MODULE_ARG) < 0) {
            ALOGD("[%s] failed to load wifi driver!!\n", __func__);
            goto error;
        }
    }

    sched_yield();

    while(count -- > 0) {
        if(find_wifi_device()==0) {
            retval = true;
            ALOGD("[%s] find wifi device\n", __func__);
            break;
        }
        usleep(50000);
    }

    if (retval == false) {
        goto error;
    }
    else {
        /* initialize skfd */
        if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            ALOGE("[%s] failed to open net socket\n", __func__);
            goto error;
        }
    }

    /* switch into test mode */
    if(wifi_switch_test_mode(1) != 0) {
        goto error;
    }

    return true;

error:
    ALOGD("[%s] failure", __func__);

    WIFI_TEST_CloseDUT();

    return false;
}

bool WIFI_TEST_CloseDUT(void)
{
    /* turn off test mode */
    wifi_switch_test_mode(0);

    /* close socket if necessary */
    if(skfd > 0) {
        close(skfd);
        skfd = -1;
    }

    /* no need to remove module, just turn off host power via rfkill */
    wifi_set_power(0);

    return true;
}

bool WIFI_TEST_TxDataRate(int TxDataRate)
{
    int retval;

	switch (TxDataRate) {
	case WIFI_TEST_RATE_AUTO:
        return false;   //@FIXME

	case WIFI_TEST_RATE_1MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_1M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_2MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_2M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_5_5MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_5_5M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_6MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_6M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_9MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_9M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_11MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_11M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_12MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_12M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_18MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_18M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_24MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_24M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_36MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_36M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_48MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_48M,
                NULL,
                NULL);
        break;

	case WIFI_TEST_RATE_54MBPS:
        retval = WIFI_TEST_set(RF_AT_FUNCID_RATE,
                RF_AT_PARAM_RATE_54M,
                NULL,
                NULL);
        break;

	default:
        return false;

	}

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetPreamble(PreambleType_t PreambleType)
{
	int retval;

	switch (PreambleType) {
	case WIFI_TEST_PREAMBLE_LONG:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_NORMAL,
                NULL,
                NULL);
		break;

	case WIFI_TEST_PREAMBLE_SHORT:
        retval = WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                RF_AT_PREAMBLE_CCK_SHORT,
                NULL,
                NULL);
		break;

	default:
        return false;
	}

    /* return value checking */
    if(retval == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_Channel(int ChannelNo)
{
    uint32_t u4Freq;

    if(ChannelNo < 0) {
        return false; /* invalid channel number */
    }
    /* 2.4GHz band */
    else if(ChannelNo <= 13) {
        u4Freq = 2412000 + (ChannelNo - 1) * 5000;
    }
    else if(ChannelNo == 14) {
        u4Freq = 2484000;
    }
    /* 5GHz band */
    else if(ChannelNo >= 36) {
        u4Freq = 5180000 + (ChannelNo - 36) * 5000;
    }
    else {
        return false; /* invalid channel number */
    }

    if(WIFI_TEST_set(RF_AT_FUNCID_CHNL_FREQ,
            u4Freq,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxGain(int TxGain)
{
    /* assign TX power gain */
    if(WIFI_TEST_set(RF_AT_FUNCID_POWER,
            TxGain * 2, // in unit of 0.5dBm
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxBurstInterval(int SIFS)
{
	if (SIFS < 20 || SIFS > 1000)
		return false;

    /* specify packet interval */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTINTERVAL,
            SIFS,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxPayloadLength(int TxPayLength)
{
	if (TxPayLength <= 0)
		return false;

    /* specify packet length */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTLEN,
            TxPayLength,
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxBurstFrames(int Frames)
{
	if (Frames < 0)
		return false;

    /* specify packet count */
    if(WIFI_TEST_set(RF_AT_FUNCID_PKTCNT,
                Frames,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxDestAddress(unsigned char *addr)
{
    uint8_t aucMacAddr[4];
    uint32_t u4MacAddr;

    /* specify MAC address[0:3] */
    memcpy(aucMacAddr, addr, sizeof(uint8_t) * 4);
    u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify MAC address[4:5] */
    memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
    memcpy(aucMacAddr, addr + 4, sizeof(uint8_t) * 2);
    u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
    if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                u4MacAddr,
                NULL,
                NULL) != 0) {
        return false;
    }

    return true;
}

bool WIFI_TEST_TxStart(void)
{
	/* tx start: without ack, async mode */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTTX,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_TxStop(void)
{
    /* checking for testing mode */
    if(WIFI_TEST_IsRunning() == false) {
        WIFI_TEST_OpenDUT();
    }

	/* tx stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RxStart(void)
{
	/* rx start: without ack, async mode */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STARTRX,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RxStop(void)
{
    /* checking for testing mode */
    if(WIFI_TEST_IsRunning() == false) {
        WIFI_TEST_OpenDUT();
    }

	/* rx stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_FRError(int *FError)
{
	if (!FError)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                (uint32_t *)FError) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_FRGood(int *FRGood)
{
	if (!FRGood)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                (uint32_t *)FRGood) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_RSSI(int *RSSI)
{
    uint32_t u4Result;

	if (!RSSI)
		return false;

    if(WIFI_TEST_get(RF_AT_FUNCID_RX_RSSI,
                0,
                NULL,
                &u4Result) == 0) {
        /* u4Result[0:7]    Average RSSI (dbM)
         * u4Result[8:15]   Minimum RCPI
         * u4Result[16:23]  Maximum RCPI
         * u4Result[24:31]  Last RCPI
         */
        *RSSI = (int)(u4Result & BITS(0,7));
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_IsRunning(void)
{
	return fgIsTesting;
}

bool WIFI_TEST_IsUp(void)
{
    struct ifreq ifr;
    int sk, up;

    /* NULL ptr checking */
    sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk == -1) {
        return false;
    }

    strncpy(ifr.ifr_name, WIFI_IF_NAME, sizeof(ifr.ifr_name)-1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1]='\0';

    /* retrieve hardware address */
    if (ioctl(sk, SIOCGIFFLAGS, &ifr) == -1) {
        close(sk);
        return false;
    }

    up = ifr.ifr_flags & IFF_UP;

    close(sk);

    if(up) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
// FrameFormat        : 1 (Mixed Mode), 2 (Green field mode)
// GI (Gard interval) : 1 (Long GI), 2 (Short GI)
bool WIFI_TEST_TxDataRate11n(
    ENUM_WIFI_TEST_MCS_RATE eTxDataRate11n,
    ENUM_WIFI_TEST_PREAMBLE_TYPE ePreambleType,
    ENUM_WIFI_TEST_GI_TYPE eGIType
    )
{
    uint32_t u4Rate, u4Preamble;
	
	if( eTxDataRate11n > WIFI_TEST_MCS_RATE_7 && eTxDataRate11n != WIFI_TEST_MCS_RATE_32 ) 
	{
		return false;
	}
	
	if( (ePreambleType != 0) && (ePreambleType != 1) ) 
	{
		return false;
	}
	
	if( (eGIType != 0) && (eGIType != 1) ) 
	{
		return false;
	}

    /* specify 11n rate */
    u4Rate = RF_AT_PARAM_RATE_MCS_MASK | (uint32_t)(eTxDataRate11n);
    if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
                u4Rate,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify preamble type */
    switch(ePreambleType) {
    case WIFI_TEST_PREAMBLE_TYPE_MIXED_MODE:
        u4Preamble = RF_AT_PREAMBLE_11N_MM;
        break;

    case WIFI_TEST_PREAMBLE_TYPE_GREENFIELD:
        u4Preamble = RF_AT_PREAMBLE_11N_GF;
        break;

    default:
        return false;
    }

    if(WIFI_TEST_set(RF_AT_FUNCID_PREAMBLE,
                u4Preamble,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* specify Guard Interval type */
    if(WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

	return true;
}

/* for 11n test */
bool WIFI_TEST_FrequencyAccuracy(int ChannelNo)
{
    /* set channel */
    if(WIFI_TEST_Channel(ChannelNo) == false) {
        return false;
    }

    /* start carrier tone */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_LO_LEAKAGE,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/* for 11n test */
bool WIFI_TEST_FrequencyAccuracy_Stop()
{
    /* checking for testing mode */
    if(WIFI_TEST_IsRunning() == false) {
        WIFI_TEST_OpenDUT();
    }

	/* stop */
    if(WIFI_TEST_set(RF_AT_FUNCID_COMMAND,
                RF_AT_COMMAND_STOPTEST,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}


/*******************************************************************************
 * STYLE #B Control API
 *******************************************************************************
 */
/* static variables for Style #B Control API */
static ENUM_WIFI_TEST_MODE eWifiTestMode = WIFI_TEST_MODE_BY_API_CONTROL;
static ENUM_WIFI_BANDWIDTH eWifiBw = WIFI_TEST_BW_20MHZ;
static uint32_t u4NumGoodFrames = 0;
static uint32_t u4NumBadFrames = 0;

bool WIFI_TEST_SetMode(ENUM_WIFI_TEST_MODE eMode)
{
    if(eMode >= WIFI_TEST_MODE_NUM) {
        return false;
    }
    else {
        eWifiTestMode = eMode;
        return true;
    }
}

bool WIFI_TEST_GetSupportedMode(uint32_t *pu4SupportedMode) {
    if(pu4SupportedMode == NULL) {
        return false;
    }
    else {
#if defined(MT6620) || defined(MT5931) || defined(MT6628)
        /* read fgEnable5GBand / fgSupport5GBand field from NVRAM */
        uint8_t ucSupport5GBand = 0, ucEnable5GBand = 0;
        F_INFO rFileInfo;
        int fd;
        
        rFileInfo = NVM_ReadFileVerInfo(AP_CFG_RDEB_FILE_WIFI_LID);
        fd = open(rFileInfo.cFileName, O_RDONLY, S_IRUSR);
        if(fd) {
            lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, ucSupport5GBand), SEEK_SET);
            read(fd, &ucSupport5GBand, sizeof(uint8_t));

            lseek(fd, offsetof(WIFI_CFG_PARAM_STRUCT, ucEnable5GBand), SEEK_SET);
            read(fd, &ucEnable5GBand, sizeof(uint8_t));

            close(fd);
        }

        if(ucSupport5GBand && ucEnable5GBand) {
            *pu4SupportedMode = BIT((int)WIFI_TEST_MODE_80211A_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211B_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211G_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211N_ONLY);
        }
        else {
            *pu4SupportedMode = BIT((int)WIFI_TEST_MODE_80211B_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211G_ONLY)
                                    | BIT((int)WIFI_TEST_MODE_80211N_ONLY);
        }
#else
        *pu4SupportedMode = 0;
#endif

        return true;
    }
}

bool WIFI_TEST_SetBandwidth(ENUM_WIFI_BANDWIDTH eBandwidth)
{
    if(eBandwidth >= WIFI_TEST_BW_NUM) {
        return false;
    }
    else if(WIFI_TEST_set(RF_AT_FUNCID_BANDWIDTH,
                (uint32_t)eBandwidth,
                NULL,
                NULL) == 0) {
        eWifiBw = eBandwidth;
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetFrequency(uint32_t u4FreqInMHz, uint32_t u4OffsetInKHz)
{
    if(u4OffsetInKHz != 0) {
        /* not supporting offset frequency */
        return false;
    }
    else {
        switch(eWifiTestMode) {
        case WIFI_TEST_MODE_80211B_ONLY:
        case WIFI_TEST_MODE_80211G_ONLY:
            /* check for channel #1 ~ channel #14 */
            switch(u4FreqInMHz) {
            case 2412:
                return WIFI_TEST_Channel(1);
            case 2417:
                return WIFI_TEST_Channel(2);
            case 2422:
                return WIFI_TEST_Channel(3);
            case 2427:
                return WIFI_TEST_Channel(4);
            case 2432:
                return WIFI_TEST_Channel(5);
            case 2437:
                return WIFI_TEST_Channel(6);
            case 2442:
                return WIFI_TEST_Channel(7);
            case 2447:
                return WIFI_TEST_Channel(8);
            case 2452:
                return WIFI_TEST_Channel(9);
            case 2457:
                return WIFI_TEST_Channel(10);
            case 2462:
                return WIFI_TEST_Channel(11);
            case 2467:
                return WIFI_TEST_Channel(12);
            case 2472:
                return WIFI_TEST_Channel(13);
            case 2484:
                return WIFI_TEST_Channel(14);
            default:
                break;
            }
            break;

        case WIFI_TEST_MODE_80211A_ONLY:
            switch(u4FreqInMHz) {
            case 4915:
                return WIFI_TEST_Channel(183);
            case 4920:
                return WIFI_TEST_Channel(184);
            case 4925:
                return WIFI_TEST_Channel(185);
            case 4935:
                return WIFI_TEST_Channel(187);
            case 4940:
                return WIFI_TEST_Channel(188);
            case 4945:
                return WIFI_TEST_Channel(189);
            case 4960:
                return WIFI_TEST_Channel(192);
            case 4980:
                return WIFI_TEST_Channel(196);
            case 5170:
                return WIFI_TEST_Channel(34);
            case 5180:
                return WIFI_TEST_Channel(36);
            case 5190:
                return WIFI_TEST_Channel(38);
            case 5200:
                return WIFI_TEST_Channel(40);
            case 5210:
                return WIFI_TEST_Channel(42);
            case 5220:
                return WIFI_TEST_Channel(44);
            case 5230:
                return WIFI_TEST_Channel(46);
            case 5240:
                return WIFI_TEST_Channel(48);
            case 5260:
                return WIFI_TEST_Channel(52);
            case 5280:
                return WIFI_TEST_Channel(56);
            case 5300:
                return WIFI_TEST_Channel(60);
            case 5320:
                return WIFI_TEST_Channel(64);
            case 5500:
                return WIFI_TEST_Channel(100);
            case 5520:
                return WIFI_TEST_Channel(104);
            case 5540:
                return WIFI_TEST_Channel(108);
            case 5560:
                return WIFI_TEST_Channel(112);
            case 5580:
                return WIFI_TEST_Channel(116);
            case 5600:
                return WIFI_TEST_Channel(120);
            case 5620:
                return WIFI_TEST_Channel(124);
            case 5640:
                return WIFI_TEST_Channel(128);
            case 5660:
                return WIFI_TEST_Channel(132);
            case 5680:
                return WIFI_TEST_Channel(136);
            case 5700:
                return WIFI_TEST_Channel(140);
            case 5745:
                return WIFI_TEST_Channel(149);
            case 5765:
                return WIFI_TEST_Channel(153);
            case 5785:
                return WIFI_TEST_Channel(157);
            case 5805:
                return WIFI_TEST_Channel(161);
            case 5825:
                return WIFI_TEST_Channel(165);
            default:
                break;
            }
            break;

        case WIFI_TEST_MODE_80211N_ONLY:
            switch(u4FreqInMHz) {
            case 2412:
                return WIFI_TEST_Channel(1);
            case 2417:
                return WIFI_TEST_Channel(2);
            case 2422:
                return WIFI_TEST_Channel(3);
            case 2427:
                return WIFI_TEST_Channel(4);
            case 2432:
                return WIFI_TEST_Channel(5);
            case 2437:
                return WIFI_TEST_Channel(6);
            case 2442:
                return WIFI_TEST_Channel(7);
            case 2447:
                return WIFI_TEST_Channel(8);
            case 2452:
                return WIFI_TEST_Channel(9);
            case 2457:
                return WIFI_TEST_Channel(10);
            case 2462:
                return WIFI_TEST_Channel(11);
            case 2467:
                return WIFI_TEST_Channel(12);
            case 2472:
                return WIFI_TEST_Channel(13);
            case 2484:
                return WIFI_TEST_Channel(14);
            case 4915:
                return WIFI_TEST_Channel(183);
            case 4920:
                return WIFI_TEST_Channel(184);
            case 4925:
                return WIFI_TEST_Channel(185);
            case 4935:
                return WIFI_TEST_Channel(187);
            case 4940:
                return WIFI_TEST_Channel(188);
            case 4945:
                return WIFI_TEST_Channel(189);
            case 4960:
                return WIFI_TEST_Channel(192);
            case 4980:
                return WIFI_TEST_Channel(196);
            case 5170:
                return WIFI_TEST_Channel(34);
            case 5180:
                return WIFI_TEST_Channel(36);
            case 5190:
                return WIFI_TEST_Channel(38);
            case 5200:
                return WIFI_TEST_Channel(40);
            case 5210:
                return WIFI_TEST_Channel(42);
            case 5220:
                return WIFI_TEST_Channel(44);
            case 5230:
                return WIFI_TEST_Channel(46);
            case 5240:
                return WIFI_TEST_Channel(48);
            case 5260:
                return WIFI_TEST_Channel(52);
            case 5280:
                return WIFI_TEST_Channel(56);
            case 5300:
                return WIFI_TEST_Channel(60);
            case 5320:
                return WIFI_TEST_Channel(64);
            case 5500:
                return WIFI_TEST_Channel(100);
            case 5520:
                return WIFI_TEST_Channel(104);
            case 5540:
                return WIFI_TEST_Channel(108);
            case 5560:
                return WIFI_TEST_Channel(112);
            case 5580:
                return WIFI_TEST_Channel(116);
            case 5600:
                return WIFI_TEST_Channel(120);
            case 5620:
                return WIFI_TEST_Channel(124);
            case 5640:
                return WIFI_TEST_Channel(128);
            case 5660:
                return WIFI_TEST_Channel(132);
            case 5680:
                return WIFI_TEST_Channel(136);
            case 5700:
                return WIFI_TEST_Channel(140);
            case 5745:
                return WIFI_TEST_Channel(149);
            case 5765:
                return WIFI_TEST_Channel(153);
            case 5785:
                return WIFI_TEST_Channel(157);
            case 5805:
                return WIFI_TEST_Channel(161);
            case 5825:
                return WIFI_TEST_Channel(165);
            default:
                break;
            }
            break;

        default:
            break;
        }

        return false;
    }
}

bool WIFI_TEST_SetRate(uint32_t u4Rate)
{
    ENUM_WIFI_TEST_GI_TYPE eGIType;
    uint32_t u4RateCode;

    /* validate rate by mode/bandwidth settings */
    if(eWifiTestMode == WIFI_TEST_MODE_80211B_ONLY) {
        switch(u4Rate) {
        case 100:
            u4RateCode = RF_AT_PARAM_RATE_1M;
            break;

        case 200:
            u4RateCode = RF_AT_PARAM_RATE_2M;
            break;

        case 550:
            u4RateCode = RF_AT_PARAM_RATE_5_5M;
            break;

        case 1100:
            u4RateCode = RF_AT_PARAM_RATE_11M;
            break;

        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211G_ONLY || eWifiTestMode == WIFI_TEST_MODE_80211A_ONLY) {
        switch(u4Rate) {
        case 600:
            u4RateCode = RF_AT_PARAM_RATE_6M;
            break;

        case 900:
            u4RateCode = RF_AT_PARAM_RATE_9M;
            break;

        case 1200:
            u4RateCode = RF_AT_PARAM_RATE_12M;
            break;

        case 1800:
            u4RateCode = RF_AT_PARAM_RATE_18M;
            break;

        case 2400:
            u4RateCode = RF_AT_PARAM_RATE_24M;
            break;

        case 3600:
            u4RateCode = RF_AT_PARAM_RATE_36M;
            break;

        case 4800:
            u4RateCode = RF_AT_PARAM_RATE_48M;
            break;

        case 5400:
            u4RateCode = RF_AT_PARAM_RATE_54M;
            break;

        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && eWifiBw == WIFI_TEST_BW_20MHZ) {
        switch(u4Rate) {
        case 650:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 720:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 1300:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 1440:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 1950:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 2170:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 2600:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 2890:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 3900:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 4330:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5200:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 5780:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5850:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 6500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 7200:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        default:
            return false;
        }
    }
    else if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && eWifiBw == WIFI_TEST_BW_40MHZ) {
        switch(u4Rate) {
        case 1350:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 1500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_0;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 2700:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 3000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_1;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 4050:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 4500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_2;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 5400:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 6000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_3;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 8100:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 9000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_4;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 10800:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 12000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_5;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        case 12150:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_6;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 13500:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_NORMAL_GI;
            break;
        case 15000:
            u4RateCode = RF_AT_PARAM_RATE_MCS_MASK | WIFI_TEST_MCS_RATE_7;
            eGIType = WIFI_TEST_GI_TYPE_SHORT_GI;
            break;
        default:
            return false;
        }
    }
    else {
        return false;
    }

    /* set GI type */
    if(eWifiTestMode == WIFI_TEST_MODE_80211N_ONLY && 
            WIFI_TEST_set(RF_AT_FUNCID_GI,
                eGIType,
                NULL,
                NULL) != 0) {
        return false;
    }

    /* set rate code */
    if(WIFI_TEST_set(RF_AT_FUNCID_RATE,
                u4RateCode,
                NULL,
                NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetTXPower(uint32_t u4Gain)
{
    if(u4Gain > 6350) {
        return false;
    }
    else if(WIFI_TEST_set(RF_AT_FUNCID_POWER,
            u4Gain / 50, // convert to unit of 0.5dBm
            NULL,
            NULL) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_SetTX(bool fgEnable)
{
    if(fgEnable == true) {
        return WIFI_TEST_TxStart();
    }
    else {
        return WIFI_TEST_TxStop();
    }
}

bool WIFI_TEST_SetRX(bool fgEnable, char *aucSrcAddr, char *aucDstAddr)
{
    if(fgEnable == true) {
        uint8_t aucMacAddr[4];
        uint32_t u4MacAddr;
        ENUM_RF_AT_RXOK_MATCH_RULE_T eRxRule;

        if(aucSrcAddr && aucDstAddr) {
            eRxRule = RF_AT_RXOK_DISABLED;
        }
        else if(aucDstAddr) {
            eRxRule = RF_AT_RXOK_MATCH_RA_ONLY;
        }
        else if(aucSrcAddr) {
            eRxRule = RF_AT_RXOK_MATCH_TA_ONLY;
        }
        else {
            eRxRule = RF_AT_RXOK_DISABLED;
        }

        /* specify policy */
        if(WIFI_TEST_set(RF_AT_FUNCID_SET_RXOK_MATCH_RULE,
                    eRxRule,
                    NULL,
                    NULL) != 0) {
            return false;
        }

        if(aucDstAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucDstAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        
            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucDstAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_DST_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        if(aucSrcAddr) {
            /* specify MAC address[0:3] */
            memcpy(aucMacAddr, aucSrcAddr, sizeof(uint8_t) * 4);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS,
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        
            /* specify MAC address[4:5] */
            memset(aucMacAddr, 0, sizeof(uint8_t) * 4);
            memcpy(aucMacAddr, &(aucSrcAddr[4]), sizeof(uint8_t) * 2);
            u4MacAddr = *(uint32_t *)(&(aucMacAddr[0]));
            if(WIFI_TEST_set(RF_AT_FUNCID_SET_MAC_SRC_ADDRESS | (4 << TEST_SET_CMD_OFFSET),
                        u4MacAddr,
                        NULL,
                        NULL) != 0) {
                return false;
            }
        }

        /* Start RX */
        return WIFI_TEST_RxStart();
    }
    else {
        return WIFI_TEST_RxStop();
    }
}

bool WIFI_TEST_ClearResult(void) 
{
    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4NumGoodFrames) == 0 &&
            WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4NumBadFrames) == 0) {
        return true;
    }
    else {
        return false;
    }
}

bool WIFI_TEST_GetResult(uint32_t *pu4GoodFrameCount, uint32_t *pu4BadFrameCount)
{
    uint32_t u4GoodCnt, u4BadCnt;

    if(WIFI_TEST_get(RF_AT_FUNCID_RXOK_COUNT,
                0,
                NULL,
                &u4GoodCnt) == 0 &&
            WIFI_TEST_get(RF_AT_FUNCID_RXERROR_COUNT,
                0,
                NULL,
                &u4BadCnt) == 0) {

        if(pu4GoodFrameCount) {
            *pu4GoodFrameCount = u4GoodCnt - u4NumGoodFrames;
        }
        if(pu4BadFrameCount) {
            *pu4BadFrameCount = u4BadCnt - u4NumBadFrames;
        }

        return true;
    }
    else {
        return false;
    }
}

