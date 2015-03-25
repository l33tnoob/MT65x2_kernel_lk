#ifndef _IWLIB_STUB_H
#define _IWLIB_STUB_H

/*******************************************************************************
** Copyright (c) 2005 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

#include "type.h"

#if WIRELESS_EXT >= 12
/* New wireless extensions API - SET/GET convention (even ioctl numbers are
 * root only)
 */
#define IOCTL_SET_INT       (SIOCIWFIRSTPRIV + 0)
#define IOCTL_GET_INT       (SIOCIWFIRSTPRIV + 1)
#define IOCTL_SET_ADDRESS   (SIOCIWFIRSTPRIV + 2)
#define IOCTL_GET_ADDRESS   (SIOCIWFIRSTPRIV + 3)
#define IOCTL_SET_STR       (SIOCIWFIRSTPRIV + 4)
#define IOCTL_GET_STR       (SIOCIWFIRSTPRIV + 5)
#define IOCTL_SET_KEY       (SIOCIWFIRSTPRIV + 6)
#define IOCTL_GET_KEY       (SIOCIWFIRSTPRIV + 7)
#define IOCTL_SET_STRUCT    (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT    (SIOCIWFIRSTPRIV + 9)
#endif /* WIRELESS_EXT >= 12 */

#if 1
/* MT5921 Glue Layer Private IOCTL IDs */
#define PRIV_CMD_REG_DOMAIN             0
#define PRIV_CMD_BEACON_PERIOD          1
#define PRIV_CMD_ADHOC_MODE             2

#define PRIV_CMD_CSUM_OFFLOAD       3

#define PRIV_CMD_ROAMING                4
#define PRIV_CMD_VOIP_DELAY             5
#define PRIV_CMD_POWER_MODE             6

#define PRIV_CMD_WMM_PS                 7
#define PRIV_CMD_BT_COEXIST             8
#define PRIV_GPIO2_MODE                 9

#define PRIV_CUSTOM_SET_PTA        		10
#define PRIV_CUSTOM_CONTINUOUS_POLL     11
#define PRIV_CUSTOM_SINGLE_ANTENNA		12
#define PRIV_CUSTOM_BWCS_CMD			13
#define PRIV_CUSTOM_DISABLE_BEACON_DETECTION	14 // later
#define PRIV_CMD_OID                15
#define PRIV_CMD_MAX                16

#else
/* MT5911/12 Glue Layer Private IOCTL IDs */
#define PRIV_CMD_OID                0
#define PRIV_CMD_CHIPID             1
#define PRIV_CMD_MCR                2
#define PRIV_CMD_BBCR               3
#define PRIV_CMD_EEPROM             4
#define PRIV_CMD_EEPROM_SIZE        5
#define PRIV_CMD_EEPROM_MAC         6
#define PRIV_CMD_EEPROM_CHKSUM      7
#define PRIV_CMD_EEPROM_COUNTRY     8
#define PRIV_CMD_EEPROM_SET_POWER   9
#define PRIV_CMD_RFTEST_MODE        10
#define PRIV_CMD_RFTEST_TX          11
#define PRIV_CMD_RFTEST_RX          12
#define PRIV_CMD_RFTEST_RST_RX_CNT  13
#define PRIV_CMD_RFTEST_CNT_POWER   14
#define PRIV_CMD_RFTEST_LOCAL_FREQ  15
#define PRIV_CMD_RFTEST_CAR_SUP     16
#define PRIV_CMD_RFTEST_LOW_POWER   17
#define PRIV_CMD_QOS_UAPSD          18
#define PRIV_CMD_QOS_UAPSD_TEST     19
#define PRIV_CMD_PSP_PROFILE        20
#define PRIV_CMD_PM_TX_TIMEOUT      21
#define PRIV_CMD_PM_ASSOC_TIMEOUT   22
#define PRIV_CMD_PM_DTIM_PERIOD     23
#define PRIV_CMD_CONN_KEEP_PERIOD   24
#define PRIV_CMD_CONN_TX_NULL     25
#define PRIV_CMD_MAX                26
#endif

typedef int (*_enum_handler)(int	skfd,
			       char *	ifname,
			       void*    arg);

typedef struct _NDIS_TRANSPORT_STRUCT{
	unsigned long	ndisOidCmd;
	unsigned long	inNdisOidlength;
	unsigned long	outNdisOidLength;
	unsigned char	ndisOidContent[16];
} NDIS_TRANSPORT_STRUCT;


int openNetHandle(void);
void closeNetHandle(int skfd);
int enumNetIf(int skfd, _enum_handler fn, void* argc);

int setIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen);

int getIWreq(int skfd, char* if_name,
    unsigned long ndisOid,
    unsigned char* ndisData,
    unsigned long bufLen,
    unsigned long* outputBufLen);
#endif
