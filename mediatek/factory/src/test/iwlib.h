/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 *	Wireless Tools
 *
 *		Jean II - HPLB 97->99 - HPL 99->07
 *
 * Common header for the Wireless Extension library...
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2007 Jean Tourrilhes <jt@hpl.hp.com>
 */

#ifndef IWLIB_H
#define IWLIB_H

/*#include "CHANGELOG.h"*/

/***************************** INCLUDES *****************************/

/* Standard headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>		/* gethostbyname, getnetbyname */
#include <net/if_ether.h>	/* struct ether_addr */
#include <sys/time.h>		/* struct timeval */
#include <unistd.h>

/* This is our header selection. Try to hide the mess and the misery :-(
 * Don't look, you would go blind ;-)
 * Note : compatibility with *old* distributions has been removed,
 * you will need Glibc 2.2 and older to compile (which means 
 * Mandrake 8.0, Debian 2.3, RH 7.1 or older).
 */

/* Set of headers proposed by Dr. Michael Rietz <rietz@mail.amps.de>, 27.3.2 */
#include <net/if_arp.h>		/* For ARPHRD_ETHER */
#include <sys/socket.h>		/* For AF_INET & struct sockaddr */
#include <netinet/in.h>         /* For struct sockaddr_in */
#include <netinet/if_ether.h>

/* Fixup to be able to include kernel includes in userspace.
 * Basically, kill the sparse annotations... Jean II */
#ifndef __user
#define __user
#endif

#include <linux/types.h>		/* for "caddr_t" et al		*/

/* Glibc systems headers are supposedly less problematic than kernel ones */
#include <sys/socket.h>			/* for "struct sockaddr" et al	*/
#include <net/if.h>			/* for IFNAMSIZ and co... */

/* Private copy of Wireless extensions (in this directoty) */
#include "wireless.h"

/* Make gcc understant that when we say inline, we mean it.
 * I really hate when the compiler is trying to be more clever than me,
 * because in this case gcc is not able to figure out functions with a
 * single call site, so not only I have to tag those functions inline
 * by hand, but then it refuse to inline them properly.
 * Total saving for iwevent : 150B = 0.7%.
 * Fortunately, in gcc 3.4, they now automatically inline static functions
 * with a single call site. Hurrah !
 * Jean II */
#undef IW_GCC_HAS_BROKEN_INLINE
#if __GNUC__ == 3
#if __GNUC_MINOR__ >= 1 && __GNUC_MINOR__ < 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif	/* __GNUC_MINOR__ */
#endif	/* __GNUC__ */
/* However, gcc 4.0 has introduce a new "feature", when compiling with
 * '-Os', it does not want to inline iw_ether_cmp() and friends.
 * So, we need to fix inline again !
 * Jean II */
#if __GNUC__ == 4
#define IW_GCC_HAS_BROKEN_INLINE	1
#endif	/* __GNUC__ */
/* Now, really fix the inline */
#ifdef IW_GCC_HAS_BROKEN_INLINE
#ifdef inline
#undef inline
#endif	/* inline */
#define inline		inline		__attribute__((always_inline))
#endif	/* IW_GCC_HAS_BROKEN_INLINE */

#ifdef __cplusplus
extern "C" {
#endif

/****************************** DEBUG ******************************/

//#define DEBUG 1

/************************ CONSTANTS & MACROS ************************/

/* Various versions information */
/* Recommended Wireless Extension version */
#define WE_VERSION	21
/* Maximum forward compatibility built in this version of WT */
#define WE_MAX_VERSION	22
/* Version of Wireless Tools */
#define WT_VERSION	29

/* Paths */
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_NET_DEV		"/proc/net/dev"

/* Some usefull constants */
#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9
/* For doing log10/exp10 without libm */
#define LOG10_MAGIC	1.25892541179

/* Backward compatibility for network headers */
#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801		/* IEEE 802.11			*/
#endif /* ARPHRD_IEEE80211 */

#ifndef IW_EV_LCP_PK_LEN
/* Size of the Event prefix when packed in stream */
#define IW_EV_LCP_PK_LEN	(4)
/* Size of the various events when packed in stream */
#define IW_EV_CHAR_PK_LEN	(IW_EV_LCP_PK_LEN + IFNAMSIZ)
#define IW_EV_UINT_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(__u32))
#define IW_EV_FREQ_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_freq))
#define IW_EV_PARAM_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_param))
#define IW_EV_ADDR_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct sockaddr))
#define IW_EV_QUAL_PK_LEN	(IW_EV_LCP_PK_LEN + sizeof(struct iw_quality))
#define IW_EV_POINT_PK_LEN	(IW_EV_LCP_PK_LEN + 4)
#endif	/* IW_EV_LCP_PK_LEN */

struct iw_pk_event
{
	__u16		len;			/* Real lenght of this stuff */
	__u16		cmd;			/* Wireless IOCTL */
	union iwreq_data	u;		/* IOCTL fixed payload */
} __attribute__ ((packed));
struct	iw_pk_point
{
  void __user	*pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
} __attribute__ ((packed));

#define IW_EV_LCP_PK2_LEN	(sizeof(struct iw_pk_event) - sizeof(union iwreq_data))
#define IW_EV_POINT_PK2_LEN	(IW_EV_LCP_PK2_LEN + sizeof(struct iw_pk_point) - IW_EV_POINT_OFF)

/****************************** TYPES ******************************/

/* Shortcuts */
typedef struct iw_statistics	iwstats;
typedef struct iw_range		iwrange;
typedef struct iw_param		iwparam;
typedef struct iw_freq		iwfreq;
typedef struct iw_quality	iwqual;
typedef struct iw_priv_args	iwprivargs;
typedef struct sockaddr		sockaddr;

/* Structure for storing all wireless information for each device
 * This is a cut down version of the one above, containing only
 * the things *truly* needed to configure a card.
 * Don't add other junk, I'll remove it... */
typedef struct wireless_config
{
  char		name[IFNAMSIZ + 1];	/* Wireless/protocol name */
  int		has_nwid;
  iwparam	nwid;			/* Network ID */
  int		has_freq;
  double	freq;			/* Frequency/channel */
  int		freq_flags;
  int		has_key;
  unsigned char	key[IW_ENCODING_TOKEN_MAX];	/* Encoding key used */
  int		key_size;		/* Number of bytes */
  int		key_flags;		/* Various flags */
  int		has_essid;
  int		essid_on;
  char		essid[IW_ESSID_MAX_SIZE + 1];	/* ESSID (extended network) */
  int		has_mode;
  int		mode;			/* Operation mode */
} wireless_config;

/* Structure for storing all wireless information for each device
 * This is pretty exhaustive... */
typedef struct wireless_info
{
  struct wireless_config	b;	/* Basic information */

  int		has_sens;
  iwparam	sens;			/* sensitivity */
  int		has_nickname;
  char		nickname[IW_ESSID_MAX_SIZE + 1]; /* NickName */
  int		has_ap_addr;
  sockaddr	ap_addr;		/* Access point address */
  int		has_bitrate;
  iwparam	bitrate;		/* Bit rate in bps */
  int		has_rts;
  iwparam	rts;			/* RTS threshold in bytes */
  int		has_frag;
  iwparam	frag;			/* Fragmentation threshold in bytes */
  int		has_power;
  iwparam	power;			/* Power management parameters */
  int		has_txpower;
  iwparam	txpower;		/* Transmit Power in dBm */
  int		has_retry;
  iwparam	retry;			/* Retry limit or lifetime */

  /* Stats */
  iwstats	stats;
  int		has_stats;
  iwrange	range;
  int		has_range;

  /* Auth params for WPA/802.1x/802.11i */
  int		auth_key_mgmt;
  int		has_auth_key_mgmt;
  int		auth_cipher_pairwise;
  int		has_auth_cipher_pairwise;
  int		auth_cipher_group;
  int		has_auth_cipher_group;
} wireless_info;

/* Structure for storing an entry of a wireless scan.
 * This is only a subset of all possible information, the flexible
 * structure of scan results make it impossible to capture all
 * information in such a static structure. */
typedef struct wireless_scan
{
  /* Linked list */
  struct wireless_scan *	next;

  /* Cell identifiaction */
  int		has_ap_addr;
  sockaddr	ap_addr;		/* Access point address */

  /* Other information */
  struct wireless_config	b;	/* Basic information */
  iwstats	stats;			/* Signal strength */
  int		has_stats;
  iwparam	maxbitrate;		/* Max bit rate in bps */
  int		has_maxbitrate;
} wireless_scan;

/*
 * Context used for non-blocking scan.
 */
typedef struct wireless_scan_head
{
  wireless_scan *	result;		/* Result of the scan */
  int			retry;		/* Retry level */
} wireless_scan_head;

/* Structure used for parsing event streams, such as Wireless Events
 * and scan results */
typedef struct stream_descr
{
  char *	end;		/* End of the stream */
  char *	current;	/* Current event in stream of events */
  char *	value;		/* Current value in event */
} stream_descr;

/* Prototype for handling display of each single interface on the
 * system - see iw_enum_devices() */
typedef int (*iw_enum_handler)(int	skfd,
			       char *	ifname,
			       char *	args[],
			       int	count);

/* Describe a modulation */
typedef struct iw_modul_descr
{
  unsigned int		mask;		/* Modulation bitmask */
  char			cmd[8];		/* Short name */
  char *		verbose;	/* Verbose description */
} iw_modul_descr;
/* -------------------------- CONSTANTS -------------------------- */

/* Type of headers we know about (basically union iwreq_data) */
#define IW_HEADER_TYPE_NULL	0	/* Not available */
#define IW_HEADER_TYPE_CHAR	2	/* char [IFNAMSIZ] */
#define IW_HEADER_TYPE_UINT	4	/* __u32 */
#define IW_HEADER_TYPE_FREQ	5	/* struct iw_freq */
#define IW_HEADER_TYPE_ADDR	6	/* struct sockaddr */
#define IW_HEADER_TYPE_POINT	8	/* struct iw_point */
#define IW_HEADER_TYPE_PARAM	9	/* struct iw_param */
#define IW_HEADER_TYPE_QUAL	10	/* struct iw_quality */

/* Handling flags */
/* Most are not implemented. I just use them as a reminder of some
 * cool features we might need one day ;-) */
#define IW_DESCR_FLAG_NONE	0x0000	/* Obvious */
/* Wrapper level flags */
#define IW_DESCR_FLAG_DUMP	0x0001	/* Not part of the dump command */
#define IW_DESCR_FLAG_EVENT	0x0002	/* Generate an event on SET */
#define IW_DESCR_FLAG_RESTRICT	0x0004	/* GET : request is ROOT only */
				/* SET : Omit payload from generated iwevent */
#define IW_DESCR_FLAG_NOMAX	0x0008	/* GET : no limit on request size */
/* Driver level flags */
#define IW_DESCR_FLAG_WAIT	0x0100	/* Wait for driver event */

/* ---------------------------- TYPES ---------------------------- */

/*
 * Describe how a standard IOCTL looks like.
 */
struct iw_ioctl_description
{
	__u8	header_type;		/* NULL, iw_point or other */
	__u8	token_type;		/* Future */
	__u16	token_size;		/* Granularity of payload */
	__u16	min_tokens;		/* Min acceptable token number */
	__u16	max_tokens;		/* Max acceptable token number */
	__u32	flags;			/* Special handling of the request */
};

/* -------------------------- VARIABLES -------------------------- */

/*
 * Meta-data about all the standard Wireless Extension request we
 * know about.
 */
 #define SIOCSIWMODUL	0x8B2E		/* set Modulations settings */
#define SIOCGIWMODUL	0x8B2F		/* get Modulations settings */
static const struct iw_ioctl_description standard_ioctl_descr[] = {
	[SIOCSIWCOMMIT	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWNAME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_CHAR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWNWID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWFREQ	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWMODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWSENS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWRANGE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_range),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWPRIV	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWPRIV	- SIOCIWFIRST] = { /* (handled directly by us) */
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCSIWSTATS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[SIOCGIWSTATS	- SIOCIWFIRST] = { /* (handled directly by us) */
		.header_type	= IW_HEADER_TYPE_NULL,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCGIWSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_SPY,
	},
	[SIOCSIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCGIWTHRSPY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[SIOCSIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[SIOCGIWAP	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWMLME	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_mlme),
		.max_tokens	= sizeof(struct iw_mlme),
	},
	[SIOCGIWAPLIST	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_AP,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= 0,
		.max_tokens	= sizeof(struct iw_scan_req),
	},
	[SIOCGIWSCAN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_SCAN_MAX_DATA,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[SIOCSIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[SIOCGIWESSID	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[SIOCSIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
	},
	[SIOCGIWNICKN	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE + 1,
	},
	[SIOCSIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRATE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRTS	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWFRAG	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWTXPOW	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWRETRY	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCGIWENCODE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,
	},
	[SIOCSIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWPOWER	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWMODUL	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWMODUL	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCGIWGENIE	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[SIOCSIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCGIWAUTH	- SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[SIOCSIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCGIWENCODEEXT - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[SIOCSIWPMKSA - SIOCIWFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_pmksa),
		.max_tokens	= sizeof(struct iw_pmksa),
	},
};
static const unsigned int standard_ioctl_num = (sizeof(standard_ioctl_descr) /
						sizeof(struct iw_ioctl_description));

/*
 * Meta-data about all the additional standard Wireless Extension events
 * we know about.
 */
static const struct iw_ioctl_description standard_event_descr[] = {
	[IWEVTXDROP	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVQUAL	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_QUAL,
	},
	[IWEVCUSTOM	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_CUSTOM_MAX,
	},
	[IWEVREGISTERED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IWEVEXPIRED	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_ADDR, 
	},
	[IWEVGENIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVMICHAELMICFAILURE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT, 
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_michaelmicfailure),
	},
	[IWEVASSOCREQIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVASSOCRESPIE	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IWEVPMKIDCAND	- IWEVFIRST] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_pmkid_cand),
	},
};
static const unsigned int standard_event_num = (sizeof(standard_event_descr) /
						sizeof(struct iw_ioctl_description));

/* Size (in bytes) of various events */
static const int event_type_size[] = {
	IW_EV_LCP_PK_LEN,	/* IW_HEADER_TYPE_NULL */
	0,
	IW_EV_CHAR_PK_LEN,	/* IW_HEADER_TYPE_CHAR */
	0,
	IW_EV_UINT_PK_LEN,	/* IW_HEADER_TYPE_UINT */
	IW_EV_FREQ_PK_LEN,	/* IW_HEADER_TYPE_FREQ */
	IW_EV_ADDR_PK_LEN,	/* IW_HEADER_TYPE_ADDR */
	0,
	IW_EV_POINT_PK_LEN,	/* Without variable payload */
	IW_EV_PARAM_PK_LEN,	/* IW_HEADER_TYPE_PARAM */
	IW_EV_QUAL_PK_LEN,	/* IW_HEADER_TYPE_QUAL */
};
/************************ CONSTANTS & MACROS ************************/

/*
 * Constants fof WE-9->15
 */
#define IW15_MAX_FREQUENCIES	16
#define IW15_MAX_BITRATES	8
#define IW15_MAX_TXPOWER	8
#define IW15_MAX_ENCODING_SIZES	8
#define IW15_MAX_SPY		8
#define IW15_MAX_AP		8

/****************************** TYPES ******************************/

/*
 *	Struct iw_range up to WE-15
 */
struct	iw15_range
{
	__u32		throughput;
	__u32		min_nwid;
	__u32		max_nwid;
	__u16		num_channels;
	__u8		num_frequency;
	struct iw_freq	freq[IW15_MAX_FREQUENCIES];
	__s32		sensitivity;
	struct iw_quality	max_qual;
	__u8		num_bitrates;
	__s32		bitrate[IW15_MAX_BITRATES];
	__s32		min_rts;
	__s32		max_rts;
	__s32		min_frag;
	__s32		max_frag;
	__s32		min_pmp;
	__s32		max_pmp;
	__s32		min_pmt;
	__s32		max_pmt;
	__u16		pmp_flags;
	__u16		pmt_flags;
	__u16		pm_capa;
	__u16		encoding_size[IW15_MAX_ENCODING_SIZES];
	__u8		num_encoding_sizes;
	__u8		max_encoding_tokens;
	__u16		txpower_capa;
	__u8		num_txpower;
	__s32		txpower[IW15_MAX_TXPOWER];
	__u8		we_version_compiled;
	__u8		we_version_source;
	__u16		retry_capa;
	__u16		retry_flags;
	__u16		r_time_flags;
	__s32		min_retry;
	__s32		max_retry;
	__s32		min_r_time;
	__s32		max_r_time;
	struct iw_quality	avg_qual;
};

/*
 * Union for all the versions of iwrange.
 * Fortunately, I mostly only add fields at the end, and big-bang
 * reorganisations are few.
 */
union	iw_range_raw
{
	struct iw15_range	range15;	/* WE 9->15 */
	struct iw_range		range;		/* WE 16->current */
};

/*
 * Offsets in iw_range struct
 */
#define iwr15_off(f)	( ((char *) &(((struct iw15_range *) NULL)->f)) - \
			  (char *) NULL)
#define iwr_off(f)	( ((char *) &(((struct iw_range *) NULL)->f)) - \
			  (char *) NULL)

/**************************** PROTOTYPES ****************************/
/*
 * All the functions in iwcommon.c
 */

/* ---------------------- SOCKET SUBROUTINES -----------------------*/
int
	iw_sockets_open(void);
void
	iw_enum_devices(int		skfd,
			iw_enum_handler fn,
			char *		args[],
			int		count);
/* --------------------- WIRELESS SUBROUTINES ----------------------*/
int
	iw_get_kernel_we_version(void);
int
	iw_print_version_info(const char *	toolname);
int
	iw_get_range_info(int		skfd,
			  const char *	ifname,
			  iwrange *	range);
int
	iw_get_priv_info(int		skfd,
			 const char *	ifname,
			 iwprivargs **	ppriv);
int
	iw_get_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
int
	iw_set_basic_config(int			skfd,
			    const char *	ifname,
			    wireless_config *	info);
/* --------------------- PROTOCOL SUBROUTINES --------------------- */
int
	iw_protocol_compare(const char *	protocol1,
			    const char *	protocol2);
/* -------------------- FREQUENCY SUBROUTINES --------------------- */
void
	iw_float2freq(double	in,
		      iwfreq *	out);
double
	iw_freq2float(const iwfreq *	in);
void
	iw_print_freq_value(char *	buffer,
			    int		buflen,
			    double	freq);
void
	iw_print_freq(char *	buffer,
		      int	buflen,
		      double	freq,
		      int	channel,
		      int	freq_flags);
int
	iw_freq_to_channel(double			freq,
			   const struct iw_range *	range);
int
	iw_channel_to_freq(int				channel,
			   double *			pfreq,
			   const struct iw_range *	range);
void
	iw_print_bitrate(char *	buffer,
			 int	buflen,
			 int	bitrate);
/* ---------------------- POWER SUBROUTINES ----------------------- */
int
	iw_dbm2mwatt(int	in);
int
	iw_mwatt2dbm(int	in);
void
	iw_print_txpower(char *			buffer,
			 int			buflen,
			 struct iw_param *	txpower);
/* -------------------- STATISTICS SUBROUTINES -------------------- */
int
	iw_get_stats(int		skfd,
		     const char *	ifname,
		     iwstats *		stats,
		     const iwrange *	range,
		     int		has_range);
void
	iw_print_stats(char *		buffer,
		       int		buflen,
		       const iwqual *	qual,
		       const iwrange *	range,
		       int		has_range);
/* --------------------- ENCODING SUBROUTINES --------------------- */
void
	iw_print_key(char *			buffer,
		     int			buflen,
		     const unsigned char *	key,
		     int			key_size,
		     int			key_flags);
int
	iw_in_key(const char *		input,
		  unsigned char *	key);
int
	iw_in_key_full(int		skfd,
		       const char *	ifname,
		       const char *	input,
		       unsigned char *	key,
		       __u16 *		flags);
/* ----------------- POWER MANAGEMENT SUBROUTINES ----------------- */
void
	iw_print_pm_value(char *	buffer,
			  int		buflen,
			  int		value,
			  int		flags,
			  int		we_version);
void
	iw_print_pm_mode(char *		buffer,
			 int		buflen,
			 int		flags);
/* --------------- RETRY LIMIT/LIFETIME SUBROUTINES --------------- */
void
	iw_print_retry_value(char *	buffer,
			     int	buflen,
			     int	value,
			     int	flags,
			     int	we_version);
/* ----------------------- TIME SUBROUTINES ----------------------- */
void
	iw_print_timeval(char *				buffer,
			 int				buflen,
			 const struct timeval *		time,
			 const struct timezone *	tz);
/* --------------------- ADDRESS SUBROUTINES ---------------------- */
int
	iw_check_mac_addr_type(int		skfd,
			       const char *	ifname);
int
	iw_check_if_addr_type(int		skfd,
			      const char *	ifname);
#if 0
int
	iw_check_addr_type(int		skfd,
			   const char *	ifname);
#endif
#if 0
int
	iw_get_mac_addr(int			skfd,
			const char *		name,
			struct ether_addr *	eth,
			unsigned short *	ptype);
#endif
char *
	iw_mac_ntop(const unsigned char *	mac,
		    int				maclen,
		    char *			buf,
		    int				buflen);
void
	iw_ether_ntop(const struct ether_addr *	eth,
		      char *			buf);
char *
	iw_sawap_ntop(const struct sockaddr *	sap,
		      char *			buf);
int
	iw_mac_aton(const char *	orig,
		    unsigned char *	mac,
		    int			macmax);
int
	iw_ether_aton(const char* bufp, struct ether_addr* eth);
int
	iw_in_inet(char *bufp, struct sockaddr *sap);
int
	iw_in_addr(int			skfd,
		   const char *		ifname,
		   char *		bufp,
		   struct sockaddr *	sap);
/* ----------------------- MISC SUBROUTINES ------------------------ */
int
	iw_get_priv_size(int		args);

/* ---------------------- EVENT SUBROUTINES ---------------------- */
void
	iw_init_event_stream(struct stream_descr *	stream,
			     char *			data,
			     int			len);
int
	iw_extract_event_stream(struct stream_descr *	stream,
				struct iw_event *	iwe,
				int			we_version);
/* --------------------- SCANNING SUBROUTINES --------------------- */
int
	iw_process_scan(int			skfd,
			char *			ifname,
			int			we_version,
			wireless_scan_head *	context);
int
	iw_scan(int			skfd,
		char *			ifname,
		int			we_version,
		wireless_scan_head *	context);

/**************************** VARIABLES ****************************/

/* Modes as human readable strings */
extern const char * const	iw_operation_mode[];
#define IW_NUM_OPER_MODE	7
#define IW_NUM_OPER_MODE_EXT	8

/* Modulations as human readable strings */
extern const struct iw_modul_descr	iw_modul_list[];
#define IW_SIZE_MODUL_LIST	16

/************************* INLINE FUNTIONS *************************/
/*
 * Functions that are so simple that it's more efficient inlining them
 */

/*
 * Note : I've defined wrapper for the ioctl request so that
 * it will be easier to migrate to other kernel API if needed
 */

/*------------------------------------------------------------------*/
/*
 * Wrapper to push some Wireless Parameter in the driver
 */
static inline int
iw_set_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int			skfd,		/* Socket to the kernel */
	   const char *		ifname,		/* Device name */
	   int			request,	/* WE ID */
	   struct iwreq *	pwrq)		/* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}

/*------------------------------------------------------------------*/
/*
 * Close the socket used for ioctl.
 */
static inline void
iw_sockets_close(int	skfd)
{
  close(skfd);
}

/*------------------------------------------------------------------*/
/*
 * Display an Ethernet Socket Address in readable format.
 */
static inline char *
iw_saether_ntop(const struct sockaddr *sap, char* bufp)
{
  iw_ether_ntop((const struct ether_addr *) sap->sa_data, bufp);
  return bufp;
}
/*------------------------------------------------------------------*/
/*
 * Input an Ethernet Socket Address and convert to binary.
 */
static inline int
iw_saether_aton(const char *bufp, struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  return iw_ether_aton(bufp, (struct ether_addr *) sap->sa_data);
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet broadcast address
 */
static inline void
iw_broad_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0xFF, ETH_ALEN);
}

/*------------------------------------------------------------------*/
/*
 * Create an Ethernet NULL address
 */
static inline void
iw_null_ether(struct sockaddr *sap)
{
  sap->sa_family = ARPHRD_ETHER;
  memset((char *) sap->sa_data, 0x00, ETH_ALEN);
}

/*------------------------------------------------------------------*/
/*
 * Compare two ethernet addresses
 */
static inline int
iw_ether_cmp(const struct ether_addr* eth1, const struct ether_addr* eth2)
{
  return memcmp(eth1, eth2, sizeof(*eth1));
}

#ifdef __cplusplus
}
#endif

#endif	/* IWLIB_H */
