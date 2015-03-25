/*
 * Driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2002-2010, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef _MTK_DRIVER_NL80211_H_
#define _MTK_DRIVER_NL80211_H_

#ifndef BITS
/* Eddie */
/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m,n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

struct	iw_point
{
  void __user	*pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
};
struct	iw_param
{
  __s32		value;		/* The value of the parameter itself */
  __u8		fixed;		/* Hardware should not use auto select */
  __u8		disabled;	/* Disable the feature */
  __u16		flags;		/* Various specifc flags (if any) */
};
struct	iw_freq
{
	__s32		m;		/* Mantissa */
	__s16		e;		/* Exponent */
	__u8		i;		/* List index (when in range struct) */
	__u8		flags;		/* Flags (fixed/auto) */
};
struct	iw_quality
{
	__u8		qual;		/* link quality (%retries, SNR,
					   %missed beacons or better...) */
	__u8		level;		/* signal level (dBm) */
	__u8		noise;		/* noise level (dBm) */
	__u8		updated;	/* Flags to know if updated */
};

union	iwreq_data
{
	/* Config - generic */
	char		name[IFNAMSIZ];
	/* Name : used to verify the presence of  wireless extensions.
	 * Name of the protocol/provider... */

	struct iw_point	essid;		/* Extended network name */
	struct iw_param	nwid;		/* network id (or domain - the cell) */
	struct iw_freq	freq;		/* frequency or channel :
					 * 0-1000 = channel
					 * > 1000 = frequency in Hz */

	struct iw_param	sens;		/* signal level threshold */
	struct iw_param	bitrate;	/* default bit rate */
	struct iw_param	txpower;	/* default transmit power */
	struct iw_param	rts;		/* RTS threshold threshold */
	struct iw_param	frag;		/* Fragmentation threshold */
	__u32		mode;		/* Operation mode */
	struct iw_param	retry;		/* Retry limits & lifetime */

	struct iw_point	encoding;	/* Encoding stuff : tokens */
	struct iw_param	power;		/* PM duration/timeout */
	struct iw_quality qual;		/* Quality part of statistics */

	struct sockaddr	ap_addr;	/* Access point address */
	struct sockaddr	addr;		/* Destination address (hw/mac) */

	struct iw_param	param;		/* Other small parameters */
	struct iw_point	data;		/* Other large parameters */
};
struct	iwreq 
{
	union
	{
		char	ifrn_name[IFNAMSIZ];	/* if name, e.g. "eth0" */
	} ifr_ifrn;

	/* Data part (defined just above) */
	union	iwreq_data	u;
};

enum nl80211_testmode_sta_link_statistics_attr{
    __NL80211_TESTMODE_STA_STATISTICS_INVALID = 0,
	NL80211_TESTMODE_STA_STATISTICS_VERSION,
	NL80211_TESTMODE_STA_STATISTICS_MAC,
    NL80211_TESTMODE_STA_STATISTICS_LINK_SCORE,
    NL80211_TESTMODE_STA_STATISTICS_FLAG,

    NL80211_TESTMODE_STA_STATISTICS_PER,
    NL80211_TESTMODE_STA_STATISTICS_RSSI,
    NL80211_TESTMODE_STA_STATISTICS_PHY_MODE,
	NL80211_TESTMODE_STA_STATISTICS_TX_RATE,
	
	NL80211_TESTMODE_STA_STATISTICS_TOTAL_CNT,
	NL80211_TESTMODE_STA_STATISTICS_THRESHOLD_CNT,
	NL80211_TESTMODE_STA_STATISTICS_AVG_PROCESS_TIME,
	
	NL80211_TESTMODE_STA_STATISTICS_FAIL_CNT,
	NL80211_TESTMODE_STA_STATISTICS_TIMEOUT_CNT,
	NL80211_TESTMODE_STA_STATISTICS_AVG_AIR_TIME,

    NL80211_TESTMODE_STA_STATISTICS_TC_EMPTY_CNT_ARRAY,
    NL80211_TESTMODE_STA_STATISTICS_TC_QUE_LEN_ARRAY,

    NL80211_TESTMODE_STA_STATISTICS_TC_AVG_QUE_LEN_ARRAY,
    NL80211_TESTMODE_STA_STATISTICS_TC_CUR_QUE_LEN_ARRAY,

    NL80211_TESTMODE_STA_STATISTICS_RESERVED_ARRAY,
    
    NL80211_TESTMODE_STA_STATISTICS_NUM,
    NL80211_TESTMODE_STA_STATISTICS_MAX = NL80211_TESTMODE_STA_STATISTICS_NUM - 1
};

enum nl80211_testmode_link_detect_attr{
	NL80211_TESTMODE_LINK_INVALID = 0,
	NL80211_TESTMODE_LINK_TX_FAIL_CNT,
	NL80211_TESTMODE_LINK_TX_RETRY_CNT,
	NL80211_TESTMODE_LINK_TX_MULTI_RETRY_CNT,
	NL80211_TESTMODE_LINK_ACK_FAIL_CNT,
	NL80211_TESTMODE_LINK_FCS_ERR_CNT,
	NL80211_TESTMODE_LINK_TX_CNT,
	NL80211_TESTMODE_LINK_TX_OK_CNT,
	NL80211_TESTMODE_LINK_TX_OS_CNT,
    
	NL80211_TESTMODE_LINK_DETECT_NUM,
	NL80211_TESTMODE_LINK_DETECT_MAX = NL80211_TESTMODE_LINK_DETECT_NUM - 1
};

typedef enum _ENUM_TRAFFIC_CLASS_INDEX_T {
    TC0_INDEX = 0,  
    TC1_INDEX,       
    TC2_INDEX,       
    TC3_INDEX,       
    TC_DATA_NUM,
    TC4_INDEX = TC_DATA_NUM,
    TC5_INDEX,       
    TC_NUM           
} ENUM_TRAFFIC_CLASS_INDEX_T;

struct wpa_driver_sta_statistics_s {
    u8      version;
    u8      addr[ETH_ALEN];
    u32     flag;
    
    u8      link_score;
    u8      per;
    int     rssi;
    u32     phy_mode;
    double  tx_rate;
    
    u32     tx_total_cnt;
    u32     tx_exc_threshold_cnt;
    u32     tx_avg_process_time;
    u32     tx_fail_cnt;
    u32     tx_timeout_cnt;
    u32     tx_avg_air_time;

    u32     tc_buf_full_cnt[TC_DATA_NUM];
    u32     tc_que_len[TC_DATA_NUM];
    
    u32     tc_avg_que_len[TC_DATA_NUM];
    u32     tc_cur_que_len[TC_DATA_NUM];

    u8      reserved[32];
};

struct wpa_driver_sta_link_detect_s {    
    u64     tx_fail_cnt;
    u64     tx_retry_cnt;
    u64     tx_multi_retry_cnt;
    u64     ack_fail_cnt;
    u64     fcs_err_cnt;
    u64     tx_cnt;
    u64     tx_ok_cnt;
    u64     tx_os_cnt;
};
#endif
