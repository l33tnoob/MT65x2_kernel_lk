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
 * GSM 07.10 Implementation with User Space Serial Ports
 *
 * Code heavily based on gsmMuxd written by
 * Copyright (C) 2003 Tuukka Karvonen <tkarvone@iki.fi>
 * Modified November 2004 by David Jander <david@protonic.nl>
 * Modified January 2006 by Tuukka Karvonen <tkarvone@iki.fi>
 * Modified January 2006 by Antti Haapakoski <antti.haapakoski@iki.fi>
 * Modified March 2006 by Tuukka Karvonen <tkarvone@iki.fi>
 * Modified October 2006 by Vasiliy Novikov <vn@hotbox.ru>
 *
 * Copyright (C) 2008 M. Dietrich <mdt@emdete.de>
 * Modified January 2009 by Ulrik Bech Hald <ubh@ti.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* If compiled with the MUX_ANDROID flag this mux will be enabled to run under Android */

/******************************************************************************/
/* INCLUDES                                                                                                                           */
/******************************************************************************/

#include <errno.h>
#include <fcntl.h>
//#include <features.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#ifdef MUX_ANDROID
#include <pathconf.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#endif

#include "gsm0710muxd.h"
#include <linux/capability.h>
#include <linux/prctl.h>
#include <private/android_filesystem_config.h>
#include "hardware/ccci_intf.h"
#include <DfoDefines.h>


/******************************************************************************/
/* DEFINES                                                                                                                             */
/******************************************************************************/

//type: Frame Structure-> Information Filed -> type field; command(e.g., MSC,PN...etc)
#define GSM0710_COMMAND_IS(type, command) ((type & ~GSM0710_CR) == command)
//frame->control: Frame Structure -> Control Field; type: Frame Type(e.g., SABM,UIH...etc)
#define GSM0710_FRAME_IS(type, frame) ((frame->control & ~GSM0710_PF) == type)

#define GSM0710_DECODE_ADDRESS(byte) (((byte) & 252) >> 2)

/******************************************************************************/
/* FUNCTION PROTOTYPES                                                                                                        */
/******************************************************************************/

static int close_devices();
static int thread_serial_device_read(void *vargp);
static int pseudo_device_read(void *vargp);
static void *poll_thread(void *vargp);
static void *assemble_frame_thread(void *vargp);
static int setup_pty_interface(Channel *channel);
static int logical_channel_close(Channel *channel);
static void *malloc_r(int size);
void set_main_exit_signal(int signal);
int watchdog(Serial *serial);

/******************************************************************************/

int create_thread(pthread_t *thread_id, void *thread_function, void *thread_function_arg);
int write_frame(int channel, const unsigned char *input, int length, unsigned char type);
void destroy_frame(GSM0710_Frame *frame);

/******************************************************************************/

#ifdef __MUXD_FLOWCONTROL__
extern void _fc_chokePty(Channel *channel);
extern void _fc_releasePty(Channel *channel);
extern void _fc_initContext(Channel *channel);
extern void _fc_closeContext(Channel *channel);
extern void _fc_cacheRemainingFrameData(Channel *channel, GSM0710_Frame *frame, int written);
extern void _fc_cacheFrameData(Channel *channel, GSM0710_Frame *frame);
extern void start_retry_write_thread(Channel *channel);
#endif /* __MUXD_FLOWCONTROL__ */

/******************************************************************************/

#ifdef __MUX_UT__
int mux_ut_init_properties();
extern void set_mux_ut_rx_chnl_fc_flag(int chnl_num, int value);
extern int ut_thread_serial_device_read(void *vargp);
extern void *ut_poll_thread_serial(void *vargp);
extern void *ut_assemble_frame_thread(void *vargp);
extern int get_mux_ut_channel_n1(void);
#endif /* __MUX_UT__ */

/******************************************************************************/
/* CONSTANTS & GLOBALS                                                                                                        */
/******************************************************************************/

unsigned char close_channel_cmd[] = { GSM0710_CONTROL_CLD | GSM0710_CR, GSM0710_EA | (0 << 1) };
unsigned char test_channel_cmd[] = { GSM0710_CONTROL_TEST | GSM0710_CR, GSM0710_EA | (6 << 1), 'P', 'I', 'N', 'G', '\r', '\n', };

/* MSC cmd pattern */
unsigned char msc_channel_cmd[] = {
	GSM0710_CONTROL_MSC | GSM0710_CR,       /* byte#0: Type field */
	GSM0710_EA | (2 << 1),                  /* byte#1: Length field, its value=2 (2 parms in MSC: One byte is DLCI, the other is V.24 control signals) */
	0x03,                                   /* Value#1: DLCI: non-zero value - Bit#8-Bit#3: DLCI;Bit#2 1 Bit#1 E/A: 1 => 0x03 */
	0x8D                                    /* Value#2: V.24 control signals- (0xD: FC On; 0xF: FC Off) */
	/* Bit#8 DV(Data Valid) Bit#7 IC(Incoming Call) Bit#6 Reserved Bit#5 Reserved :0x8 */
	/* Bit#4 RTR(Ready To Receive) Bit#3 RTC(Ready To Communicate) Bit#2 FC Off Bit#1 E/A :0xD */
};
//static unsigned char psc_channel_cmd[] = { GSM0710_CONTROL_PSC | GSM0710_CR, GSM0710_EA | (0 << 1), };
//static unsigned char wakeup_sequence[] = { GSM0710_FRAME_FLAG, GSM0710_FRAME_FLAG, };

/* crc table from gsm0710 spec */
static const unsigned char r_crctable[] = { //reversed, 8-bit, poly=0x07
	0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED,
	0x7C, 0x09, 0x98, 0xEA, 0x7B, 0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A,
	0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67, 0x38,
	0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44,
	0x31, 0xA0, 0xD2, 0x43, 0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0,
	0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F, 0x70, 0xE1,
	0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79,
	0xE8, 0x9A, 0x0B, 0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
	0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17, 0x48, 0xD9, 0xAB,
	0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0,
	0xA2, 0x33, 0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A,
	0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F, 0xE0, 0x71, 0x03, 0x92,
	0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A,
	0x9B, 0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63,
	0x11, 0x80, 0xF5, 0x64, 0x16, 0x87, 0xD8, 0x49, 0x3B, 0xAA, 0xDF,
	0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
	0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29,
	0xB8, 0xCD, 0x5C, 0x2E, 0xBF, 0x90, 0x01, 0x73, 0xE2, 0x97, 0x06,
	0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB, 0x8C,
	0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0,
	0x85, 0x14, 0x66, 0xF7, 0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C,
	0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3, 0xB4, 0x25,
	0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD,
	0x2C, 0x5E, 0xCF,
};

/******************************************************************************/

// muxd contexts
Serial serial;
Channel channellist[GSM0710_MAX_CHANNELS]; // remember: [0] is not used acticly because it's the control channel

// config stuff
static char *revision = "$Rev: 1 $";
static int no_daemon = 0;
static int pin_code = -1;
static int use_ping = 0;
static int use_timeout = 0;
static int vir_ports = 2; /* number of virtual ports to create */

// debug
int syslog_level = LOG_INFO;
int logtofile = 0;
FILE *muxlogfile;

/*misc global vars */
static int main_exit_signal = 0;        /* 1:main() received exit signal */
static int uih_pf_bit_received = 0;
static unsigned int pts_reopen = 0;     /*If != 0,  signals watchdog that one cahnnel needs to be reopened */
/* Add by LS: this variable "close_down_send_disc_for_all_chnl" is used with ua_rsp_for_disc_lock */
static int close_down_send_disc_for_all_chnl = 0;
static int mux_setup_chnl_complete = 0;
static int stop_muxd = 0;

// some state
// +CMUX=<mode>[,<subset>[,<port_speed>[,<N1>[,<T1>[,<N2>[,<T2>[,<T3>[,<k>]]]]]]]]
static int cmux_mode = 1;
static int cmux_subset = 0;
static int cmux_port_speed = 5; // 115200 baud rate
static int cmux_N1 = 1509;      // Maximum Frame Size (N1): 64/31
#if 0
// Acknowledgement Timer (T1) sec/100: 10
static int cmux_T1 = 10;
// Maximum number of retransmissions (N2): 3
static int cmux_N2 = 3;
// Response Timer for multiplexer control channel (T2) sec/100: 30
static int cmux_T2 = 30;
// Response Timer for wake-up procedure(T3) sec: 10
static int cmux_T3 = 10;
// Window Size (k): 2
static int cmux_k = 2;
#endif

/******************************************************************************/

/*pthread */
/* Thread_ID:ser_read_thread -> Thread_Func: poll_thread_serial() -> Thread_Read_Ptr: thread_serial_device_read() => Read data from /dev/ttyC0 and put data into serial->in_buf */
pthread_t ser_read_thread;
/* Thread_ID:frame_assembly_thread -> Thread_Func: assemble_frame_thread() -> Thread_Read_Ptr: thread_serial_device_read() => Read data from serial->inbuf */
pthread_t frame_assembly_thread;

#if 0                                                   //move following pthread variables to channellist[]
/* Thread_ID:pseudo_terminal[i] -> Thread_Func: pseudo_device_read()  => Read data from master pty then write data to /dev/ttyC0 via write_frame */
pthread_t pseudo_terminal[GSM0710_MAX_CHANNELS - 1];    /* -1 because control channel cannot be mapped to pseudo-terminal dev/pts */
#endif

pthread_cond_t setup_chnl_complete_signal = PTHREAD_COND_INITIALIZER;

/* Add by LS: main() process waits for all UA Rsp of every non-control channel in the CLD procedure */
pthread_cond_t ua_rsp_for_disc_signal = PTHREAD_COND_INITIALIZER;
pthread_attr_t thread_attr;
pthread_mutex_t syslogdump_lock;
pthread_mutex_t write_frame_lock;
pthread_mutex_t main_exit_signal_lock;
pthread_mutex_t pts_reopen_lock;
pthread_mutex_t setup_chnl_complete_lock;
pthread_mutex_t ua_rsp_for_disc_lock;
pthread_mutex_t watch_lock;

#if 0 //move to GSM0710_Buffer
pthread_cond_t newdataready_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t bufferready_signal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t datacount_lock;
pthread_mutex_t newdataready_lock;
pthread_mutex_t bufferready_lock;
#endif

#ifdef __MUX_UT__
pthread_t ut_ser_read_thread;
pthread_t ut_frame_assembly_thread;
#endif

/******************************************************************************/
/*
 * The following arrays must have equal length and the values must
 * correspond. also it has to correspond to the gsm0710 spec regarding
 * baud id of CMUX the command.
 */
static int baud_rates[] = {
	0, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};
static speed_t baud_bits[] = {
	0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600
};

/******************************************************************************/

#ifdef MUX_ANDROID
/* Android's log level are in opposite order of syslog.h */
int android_log_lvl_convert[8] = { ANDROID_LOG_SILENT,  /*8*/
				   ANDROID_LOG_SILENT,  /*7*/
				   ANDROID_LOG_FATAL,   /*6*/
				   ANDROID_LOG_ERROR,   /*5*/
				   ANDROID_LOG_WARN,    /*4*/
				   ANDROID_LOG_INFO,    /*3*/
				   ANDROID_LOG_INFO,    /*2*/
				   ANDROID_LOG_DEBUG }; /*1*/
#endif /*MUX_ANDROID*/

/******************************************************************************/

static const Channel_Config ch_cfg[] =
{
	{ 0,  512, NULL		    },
        
#ifdef MTK_RIL_MD2
	{ 1,  512, "/dev/radio/pttycmd4-md2"  }, /* ALPS00337548 split data and nw command channel */
	{ 2,  512, "/dev/radio/pttynoti-md2"  },
	{ 3,  512, "/dev/radio/pttycmd1-md2"  },
	{ 4,  512, "/dev/radio/pttycmd2-md2"  },
	{ 5,  512, "/dev/radio/pttycmd3-md2"  }, 
	{ 26, 512, "/dev/radio/atci1-md2" },
#ifdef __ANDROID_GEMINI_SUPPORT__ 
	{ 6,  512, "/dev/radio/ptty2cmd4-md2" }, /* ALPS00337548 split data and nw command channel */
	{ 7,  512, "/dev/radio/ptty2noti-md2" },
	{ 8,  512, "/dev/radio/ptty2cmd1-md2" },
	{ 9,  512, "/dev/radio/ptty2cmd2-md2" },
	{ 10, 512, "/dev/radio/ptty2cmd3-md2" }, 
	{ 27, 512, "/dev/radio/atci2-md2" },
#endif
#ifdef __ANDROID_VT_SUPPORT__
	{ 24, 512, "/dev/radio/pttyvt-md2"    },
#endif

#else
	{ 1,  512, "/dev/radio/pttycmd4"  }, /* ALPS00337548 split data and nw command channel */
	{ 2,  512, "/dev/radio/pttynoti"  },
	{ 3,  512, "/dev/radio/pttycmd1"  },
	{ 4,  512, "/dev/radio/pttycmd2"  },
	{ 5,  512, "/dev/radio/pttycmd3"  }, 
	{ 26, 512, "/dev/radio/atci1" },
#ifdef __ANDROID_GEMINI_SUPPORT__
	{ 6,  512, "/dev/radio/ptty2cmd4" }, /* ALPS00337548 split data and nw command channel */
	{ 7,  512, "/dev/radio/ptty2noti" },
	{ 8,  512, "/dev/radio/ptty2cmd1" },
	{ 9,  512, "/dev/radio/ptty2cmd2" },
	{ 10, 512, "/dev/radio/ptty2cmd3" }, 
	{ 27, 512, "/dev/radio/atci2" },
#endif

#if (MTK_GEMINI_SIM_NUM >= 3)
	{ 11,  512, "/dev/radio/ptty3cmd4" }, /* Gemini plus 3 SIM*/
	{ 12,  512, "/dev/radio/ptty3noti" },
	{ 13,  512, "/dev/radio/ptty3cmd1" },
	{ 14,  512, "/dev/radio/ptty3cmd2" },
	{ 15, 512, "/dev/radio/ptty3cmd3" },
	{ 28, 512, "/dev/radio/atci3" },
#endif

#if (MTK_GEMINI_SIM_NUM >= 4)
	{ 16,  512, "/dev/radio/ptty4cmd4" }, /* Gemini plus 4SIM*/
	{ 17,  512, "/dev/radio/ptty4noti" },
	{ 18,  512, "/dev/radio/ptty4cmd1" },
	{ 19,  512, "/dev/radio/ptty4cmd2" },
	{ 20, 512, "/dev/radio/ptty4cmd3" },
	{ 29, 512, "/dev/radio/atci4" },
#endif


#ifdef __ANDROID_VT_SUPPORT__
	{ 24, 512, "/dev/radio/pttyvt"    },
#endif

#endif

#ifdef MTK_CSD_DIALER_SUPPORT 
    { 30, 512, "/dev/pttycsd" },
#endif
#if defined(__CCMNI_SUPPORT__) && defined(__MUX_UT__)
	{ 28, 512, "/dev/pttyiutlb" },
	{ 29, 512, "/dev/pttyiuttx" },
	{ 30, 512, "/dev/pttyiutrx" },
#endif

};

/******************************************************************************/
/* MAIN CODE                                                                                                                         */
/******************************************************************************/

/*
 * Purpose:
 * Input:
 * Return:
 */
void *malloc_r(int size)
{
	void *mem = NULL;

	int i;
	for (i = 0; i < MUXD_MALLOC_RETRY; ++i) {
		if ((mem = malloc(size)) != NULL) {
			return mem;
		} else {
			LOGMUX(LOG_WARNING, "Out of memory!!");
			sleep(1);
		}
	}

	// Out of memory, Assert!!
	Gsm0710Muxd_Assert(16);

	return mem;
}

/*
 * Purpose:
 * Input:
 * Return:
 */
void reset_cntx_and_terminate_threads()
{
	/* Currently, Muxd should not receive CLD command from modem */
	/* Left this function for further implementation */
	LOGMUX(LOG_ERR, "Empty and Not implement yet");
}

/*
 * Purpose:  Determine baud-rate index for CMUX command
 * Input:	 baud_rate - baud rate (eg. 460800)
 * Return:   -1 if fail, i - baud rate index if success
 */
static int baud_rate_index(int baud_rate)
{
	unsigned int i;
	for (i = 0; i < sizeof(baud_rates) / sizeof(*baud_rates); ++i)
		if (baud_rates[i] == baud_rate)
			return i;
	return -1;
}

/*
 * Purpose:  Calculates frame check sequence from given characters.
 * Input:	 input - character array
 *           length - number of characters in array (that are included)
 * Return:   frame check sequence
 */
unsigned char frame_calc_crc(const unsigned char * input, int length)
{
	unsigned char fcs = 0xFF;

	int i;
	for (i = 0; i < length; i++)
		fcs = r_crctable[fcs ^ input[i]];

	return 0xFF - fcs;
}

/*
 * Purpose:  Escapes GSM0710_FRAME_ADV_ESCAPED_SYMS characters.
 * Input:	 adv_buf - pointer to the new buffer with the escaped content
 *           data - pointer to the char buffer to be parsed
 *           length - the length of the data char buffer
 * Return:   adv_i - number of added escape chars
 */
static int fill_adv_frame_buf(unsigned char * adv_buf, const unsigned char * data, int length)
{
	static const unsigned char esc[] = GSM0710_FRAME_ADV_ESCAPED_SYMS;
	int i, adv_i = 0;
	unsigned int esc_i;

	for (i = 0; i < length; ++i, ++adv_i) {
		adv_buf[adv_i] = data[i];
		for (esc_i = 0; esc_i < sizeof(esc) / sizeof(esc[0]); ++esc_i)
			if (data[i] == esc[esc_i]) {
				adv_buf[adv_i] = GSM0710_FRAME_ADV_ESC;
				adv_i++;
				adv_buf[adv_i] = data[i] ^ GSM0710_FRAME_ADV_ESC_COPML;
				break;
			}
	}
	return adv_i;
}

/*
 * Purpose:  ascii/hexdump a byte buffer
 * Input:	 prefix - string to printed before hex data on every line
 *           ptr - the string to be dumped
 *           length - the length of the string to be dumped
 * Return:   0
 */
static int syslogdump(
	const char *		prefix,
	const unsigned char *	ptr,
	unsigned int		length)
{
	if (LOG_DEBUG > syslog_level) /*No need for all frame logging if it's not to be seen */
		return 0;

	char buffer[100];
	unsigned int offset = 0l;
	int i;

	pthread_mutex_lock(&syslogdump_lock);   //new lock
	while (offset < length) {
		int off;
		strcpy(buffer, prefix);
		off = strlen(buffer);
		SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%08x: ", offset));
		off = strlen(buffer);
		for (i = 0; i < 16; i++) {
			if (offset + i < length)
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%02x%c", ptr[offset + i], i == 7 ? '-' : ' '));
			else
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " .%c", i == 7 ? '-' : ' '));
			off = strlen(buffer);
		}
		SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " "));
		off = strlen(buffer);
		for (i = 0; i < 16; i++)
			if (offset + i < length) {
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%c", (ptr[offset + i] < ' ') ? '.' : ptr[offset + i]));
				off = strlen(buffer);
			}
		offset += 16;
		LOGMUX(LOG_DEBUG, "%s", buffer);
	}
	pthread_mutex_unlock(&syslogdump_lock); /*new lock*/
	return 0;
}

/*
 * Purpose:  Writes a frame to a logical channel. C/R bit is set to 1.
 *                Doesn't support FCS counting for GSM0710_TYPE_UI frames.
 * Input:	    channel - channel number (0 = control)
 *                input - the data to be written
 *                length - the length of the data
 *                type - the type of the frame (with possible P/F-bit)
 * Return:    number of characters written
 */
int write_frame(
	int			channel,
	const unsigned char *	input,
	int			length,
	unsigned char		type)
{
	/* new lock */
	pthread_mutex_lock(&write_frame_lock);
	LOGMUX(LOG_DEBUG, "Enter");

    if(stop_muxd == 1){
        LOGMUX(LOG_WARNING, "stop_muxd write");
        return 1;
    }

	/* flag, GSM0710_EA=1 C channel, frame type, length 1-2 */
	unsigned char prefix[5] = { GSM0710_FRAME_FLAG, GSM0710_EA | GSM0710_CR, 0, 0, 0 };
	unsigned char postfix[2] = { 0xFF, GSM0710_FRAME_FLAG };
	/* Default prefix_length = 1 (Frame flag) + 1 (Address field) + 1 (Control field) + 1 (Length field) */
	int prefix_length = 4;
	int c;

#if 0
//	char w = 0;
//	int count = 0;
//	do
//	{
//commented out wakeup sequence:
//              syslogdump(">s ", (unsigned char *)wakeup_sequence, sizeof(wakeup_sequence));
//		write(serial.fd, wakeup_sequence, sizeof(wakeup_sequence));
//		SYSCHECK(tcdrain(serial.fd));
//		fd_set rfds;
//		FD_ZERO(&rfds);
//		FD_SET(serial.fd, &rfds);
//		struct timeval timeout;
//		timeout.tv_sec = 0;
//		timeout.tv_usec = 1000 / 100 * cmux_T2;
//		int sel = select(serial.fd + 1, &rfds, NULL, NULL, &timeout);
//		if (sel > 0 && FD_ISSET(serial.fd, &rfds))
//			read(serial.fd, &w, 1);
//		else
//			count++;
//	} while (w != wakeup_sequence[0] && count < cmux_N2);
//	if (w != wakeup_sequence[0])
//		LOGMUX(LOG_WARNING, "Didn't get frame-flag after wakeup");
#endif

#if 0
#if defined(__CCMNI_SUPPORT__) && (defined(__MUX_IT__) || defined(__MUX_UT__))
	if (channel >= (MAX_NON_GEMINI_CHNL_NUM - 2)) {
		/* Change the channel number from 9,10,11 to 28,29,30 (28-30: Modem MUX defined Test Channels) */
		LOGMUX(LOG_DEBUG, "Sending frame to non-modified channel num = %d", channel);
		channel += MUX_TEST_CHNL_OFFSET;
	}
#endif
#endif
	if (channel != MUXD_VT_CH_NUM)
		LOGMUX(LOG_INFO, "Sending frame to channel %d", channel);

	/* prefix[1]: already set GSM0710_EA=1 and GSM0710_CR bit as 1, Command, let's add address: DLCI */
	prefix[1] = prefix[1] | ((63 & (unsigned char)channel) << 2);
	/* let's set control field */
	prefix[2] = type;
	if ((type == GSM0710_TYPE_UIH || type == GSM0710_TYPE_UI) &&
	    uih_pf_bit_received == 1 &&
	    GSM0710_COMMAND_IS(input[0], GSM0710_CONTROL_MSC)) {
		prefix[2] = prefix[2] | GSM0710_PF;     //Set the P/F bit in Response if Command from modem had it set
		uih_pf_bit_received = 0;                //Reset the variable, so it is ready for next command
	}

	/* let's not use too big frames */
	/* Note by LS: It should use channel->negotiated_N1 instead of cmux_N1 */
	length = min(channellist[channel].negotiated_N1, length);
	if (!cmux_mode) {
		/* Modified acording PATCH CRC checksum */
		/* postfix[0] = frame_calc_crc (prefix + 1, prefix_length - 1); */
		/* length */
		if (length > 127) {
			prefix_length = 5;
			/* Two bytes Length field with EA-bit as 0 */
			prefix[3] = (0x007F & length) << 1;
			prefix[4] = (0x7F80 & length) >> 7;
		} else {
			/* prefix[3]: 1 byte Length field with EA-bit as 1 */
			prefix[3] = 1 | (length << 1);
		}

		/* Using Address filed, Control field and Length field to calculate the FCS and store it in postfix[0] */
		postfix[0] = frame_calc_crc(prefix + 1, prefix_length - 1);
		syslogdump(">s ", prefix,prefix_length); /* syslogdump for basic mode */
		c = write(serial.fd, prefix, prefix_length);
		LOGMUX(LOG_DEBUG, "Write prefix len=%d written=%d", prefix_length, c);
		if (c != prefix_length) {
			LOGMUX(LOG_WARNING, "Couldn't write the whole prefix to the serial port for the virtual port %d. Wrote only %d bytes",
			       channel, c);
			/* Note by LS: It should return 0 due to no any information field data is written */
			/* In c_alloc_channel(): this fd is opened with O_NONBLOCK */
			/* But the current implementation, it will not record which index of next prefix data shoud be written to the pty */
			/* Another potential problem: If it returns from here directly, original completed frame maybe discontinuous with other subsystem's frame data */
			if (c < 0) {
				/* Check Error cause */
			    if (errno==ETXTBSY || errno==ENODEV) {
			        LOGMUX(LOG_DEBUG, "write to MD get ETXTBSY/ENODEV (%d), drop data (%d)", errno, length);
			        pthread_mutex_unlock(&write_frame_lock);
			        return length;
			    }
			}
            pthread_mutex_unlock(&write_frame_lock);
			return 0;
		}

		// Wirte information into buffer
		if (length > 0) {
			syslogdump(">s ", input,length); /* syslogdump for basic mode */
			c = write(serial.fd, input, length);
			LOGMUX(LOG_DEBUG, "Write data len=%d written=%d", length, c);
			if (length != c) {
				LOGMUX(LOG_WARNING, "Couldn't write all data to the serial port from the virtual port %d. Wrote only %d bytes",
				       channel, c);
				/* Note by LS: Potential Bug - When re-entering this function, it always sends from the prefix data again */
				/* If the remaining data exists due to previous write_frame(), it should not write the prefix data again */
				if (c < 0) {
					/* Check Error Cause */
			            if (errno==ETXTBSY || errno==ENODEV) {
			                LOGMUX(LOG_DEBUG, "write to MD get ETXTBSY/ENODEV (%d), drop data (%d)", errno, length);
			                pthread_mutex_unlock(&write_frame_lock);
			                return length;
			            }
				}
                pthread_mutex_unlock(&write_frame_lock);
				return 0;
			}
		}

		// Write postfix
		syslogdump(">s ", postfix,2); /* syslogdump for basic mode */
		c = write(serial.fd, postfix, 2);
		LOGMUX(LOG_DEBUG, "Write postfix len=%d written=%d", 2, c);
		if (c != 2) {
			LOGMUX(LOG_WARNING, "Couldn't write the whole postfix to the serial port for the virtual port %d. Wrote only %d bytes",
			       channel, c);
			/* Note by LS: Potential Bug - When re-entering this function, it always sends from the prefix data again */
			/* If the remaining data exists due to previous write_frame(), it should not write the prefix data again */
		    /* Check Error Cause */
		    if (errno==ETXTBSY || errno==ENODEV) {
		        LOGMUX(LOG_DEBUG, "write to MD get ETXTBSY/ENODEV (%d), drop data (%d)", errno, length);
		        pthread_mutex_unlock(&write_frame_lock);
		        return length;
		    }
            pthread_mutex_unlock(&write_frame_lock);
			return 0;
		}

		if (channel != MUXD_VT_CH_NUM)
			LOGMUX(LOG_INFO, "Write a frame on channel %d with length %d", channel, length);
	} else { /* cmux_mode */
		int offs = 1;
		serial.adv_frame_buf[0] = GSM0710_FRAME_ADV_FLAG;
		offs += fill_adv_frame_buf(serial.adv_frame_buf + offs, prefix + 1, 2); /* address, control */
		offs += fill_adv_frame_buf(serial.adv_frame_buf + offs, input, length); /* data */
		/* CRC checksum */
		postfix[0] = frame_calc_crc(prefix + 1, 2);
		offs += fill_adv_frame_buf(serial.adv_frame_buf + offs, postfix, 1); /* fcs */
		serial.adv_frame_buf[offs] = GSM0710_FRAME_ADV_FLAG;
		offs++;
		syslogdump(">s ", (unsigned char *)serial.adv_frame_buf, offs);
		c = write(serial.fd, serial.adv_frame_buf, offs);
		LOGMUX(LOG_DEBUG, "return from write()case=%d:wanted_written_len=%d,actual_written_len=%d", 4, offs, c);
		if (c != offs) {
			LOGMUX(LOG_WARNING, "Couldn't write the whole advanced packet to the serial port for the virtual port %d. Wrote only %d bytes",
			       channel, c);
            pthread_mutex_unlock(&write_frame_lock);
			return 0;
		}
	}
	//LOGMUX(LOG_DEBUG, "Leave");
	//new lock
	pthread_mutex_unlock(&write_frame_lock);
	return length;
}

/* Add by LS */
static int rx_thread_write_frame(
	int			channel,
	const unsigned char *	input,
	int			length,
	unsigned char		type)
{
#ifndef RX_THREAD_NONBLOCKING_WRITE_FRAME
	write_frame(channel, input, length, type);
#else
	/* Start a new thread to execute this BLOCKING write_frame: In this way, RX thread can handle other channel's RX activity continuously */
#endif  /* RX_THREAD_NONBLOCKING_WRITE_FRAME */

	return length;
}


/*
 * Purpose:  Handles received data from pseudo terminal device (application)
 * Input:	 buf - buffer, which contains received data
 *           len - the length of the buffer channel
 *           channel - logical channel id where data was received
 * Return:   The number of remaining bytes in partial packet
 */
static int handle_channel_data(unsigned char * buf, int len, int channel)
{
	int written = 0;
	int i = 0;
	int last = 0;

	/* try to write 5 times */
	while (written != len && i < GSM0710_WRITE_RETRIES) {
		last = write_frame(channel, buf + written, len - written, GSM0710_TYPE_UIH);
		written += last;
		if (last == 0)
			i++;
	}
#if 0
	if (i == GSM0710_WRITE_RETRIES)
		LOGMUX(LOG_WARNING, "Couldn't write data to channel %d. Wrote only %d bytes, when should have written %d", channel, written, len);
#endif

	/* Note by LS: Bug - It should not always return 0 : It may reach the max write_retries to exit the while loop */
	/* return 0; */
	return len - written;
}

/*
 * Purpose:  Close mux logical channel
 * Input:      channel - logical channel struct
 * Return:    0
 */
static int logical_channel_close(Channel *channel)
{
	if (channel->fd >= 0) {
#ifdef  MUX_ANDROID
		/* Remove the symbolic link first */
		if (channel->ptslink != NULL)
			unlink(channel->ptslink);
#endif  /* MUX_ANDROID */
		close(channel->fd);
		LOGMUX(LOG_DEBUG, "loical_channel_close close(channel->fd);");
		channel->fd = -1;
	}

	if (channel->ptsname != NULL)
		free(channel->ptsname);
	channel->ptsname = NULL;

	channel->opened = 0;
	channel->v24_signals = 0;

#if 0   //remove remainging variable, use local varaible rather than global one
	channel->remaining = 0;
#endif
	/* Add by LS */
	channel->sabm_ua_pending = 0;
	channel->disc_ua_pending = 0;
	//channel->retry_malloc_count = 0;

	channel->negotiated_N1 = 0;

#ifdef __MUXD_FLOWCONTROL__
	_fc_closeContext(channel);
#endif  /* __MUXD_FLOWCONTROL__ */

	return 0;
}

/*
 * Purpose:  Initialize mux logical channel
 * Input:    channel - logical channel struct
 *           id - logical channel id number
 * Return:   0
 */
static int logical_channel_init(Channel * channel, int id)
{
	int i;

	// clean channel and reset the context of channel
	logical_channel_close(channel);

	channel->id = id; // connected channel-id
	/* Comment by LS:Because channel num#0 is used by Gsm0710Muxd locally and not exported to RILD or network interface driver: */
	/* It is not necessary to open with /dev/ptmx to create a pair of ptys */
	channel->devicename = id ? "/dev/ptmx" : NULL; // TODO do we need this to be dynamic anymore?
	// clear reopen flag
	channel->reopen = 0;

#ifdef MUX_ANDROID
	// find configuration in ch_cfg
	for (i = 0; i < MUXD_CH_NUM_ALL; i++) {
		if (id == ch_cfg[i].pn_dlci) {
			//ptslink direct to pty symbolic link
			channel->ptslink = ch_cfg[i].s_path;
			//negotiated_N1 field must be discussed and pre-defined with modem side's MUX
			channel->negotiated_N1 = ch_cfg[i].pn_n1;
			break;
		}
	}

	if (i == MUXD_CH_NUM_ALL) {
		channel->ptslink = NULL;
		channel->negotiated_N1 = cmux_N1;
	}
#endif

#ifdef __MUXD_FLOWCONTROL__
	_fc_initContext(channel);
#endif  /* __MUXD_FLOWCONTROL__ */

	return 0;
}

/*
 * Purpose:
 * Input:
 * Return:
 */
static int logical_channel_establish(Channel *channel)
{
	if (setup_pty_interface(channel) != 0) {
		//Exit main function if channel couldn't be allocated
		Gsm0710Muxd_Assert(GSM0710MUXD_SETUP_PTY_ERR);
	} else {
		// Set Contexts
		channel->v24_signals = GSM0710_SIGNAL_DV | GSM0710_SIGNAL_RTR | GSM0710_SIGNAL_RTC | GSM0710_EA;

		/* Only send SABM for each MUX's channel*/
		channel->sabm_ua_pending = 1;

		write_frame(channel->id, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF);

		LOGMUX(LOG_INFO, "Connecting %s to virtual channel %d on %s", channel->ptsname, channel->id, serial.devicename);
	}
	return GSM0710MUXD_SUCCESS;
}

/*
 * Purpose: Add this function for c_alloc_channel() and pty_restart_interface() to open a pty channel and send the SABM command for it
 * Input:
 * Return:
 */
//int setup_one_pty_channel( const char* origin, pthread_t * thread_id, Channel* channel )
static int setup_pty_interface(Channel *channel)
{
	struct termios options;
	char *pts;

	// LOGMUX(LOG_DEBUG, "Found free channel %d fd %d on %s", channel->id, channel->fd, channel->devicename);
	/* Comment by LS: Only channel num is larger than zero is necessary to create a pair of ptys */
	SYSCHECK(channel->fd = open(channel->devicename, O_RDWR | O_NONBLOCK)); //open pseudo terminal devices from /dev/ptmx master
	LOGMUX(LOG_DEBUG, "setup_pty_interface SYSCHECK(channel->fd = open(channel->devicename, O_RDWR | O_NONBLOCK));");
	pts = ptsname(channel->fd);
	if (pts == NULL) SYSCHECK(-1);
	channel->ptsname = strdup(pts);
	tcgetattr(channel->fd, &options); //get the parameters
	bzero(&options, sizeof(options));
	//options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //set raw input
	//options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
	//options.c_oflag &= ~(OPOST| OLCUC| ONLRET| ONOCR| OCRNL); //set raw output
	options.c_cflag = BAUDRATE | CREAD | CLOCAL | CS8;
	//options.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
	//options.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL | IXON | IXOFF);
	options.c_iflag = IGNPAR;
	options.c_lflag = 0;    /* disable CANON, ECHO*, etc */
	options.c_oflag = 0;    /* set raw output */
	/* no timeout but request at least one character per read */
	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 1;

	SYSCHECK(tcflush(channel->fd, TCIFLUSH));
	SYSCHECK(tcsetattr(channel->fd, TCSANOW, &options));
	if (!strcmp(channel->devicename, "/dev/ptmx")) {
		//Otherwise programs cannot access the pseudo terminals
		SYSCHECK(grantpt(channel->fd));
		SYSCHECK(unlockpt(channel->fd));

#ifdef  MUX_ANDROID
		if (channel->ptslink != NULL) {
                        chown(channel->ptsname, AID_RADIO, AID_RADIO);	
			unlink(channel->ptslink);
			SYSCHECK(chmod(channel->ptsname, 0660));
			SYSCHECK(symlink(channel->ptsname, channel->ptslink));
                        LOGMUX(LOG_ERR,"symlink done");                   
		}
#if 0
		if (channel_id < vir_ports && restarted == 0) {
			LOGMUX(LOG_INFO, "Unlock %s on Link %s", channel->ptsname, s_path[channel_id]);
			SYSCHECK(chmod(channel->ptsname, 0666));
			SYSCHECK(symlink(channel->ptsname, s_path[channel_id]));
			channel_id++;
		} else if (channel->id < vir_ports && restarted == 1) {
			LOGMUX(LOG_INFO, "Unlock %s on Link %s", channel->ptsname, s_path[channel->id - 1]);
			/* Remove the previous symbolic link before setting it up again: It will get a different ptmx name at each time's open() */
			unlink(s_path[channel->id - 1]);
			SYSCHECK(chmod(channel->ptsname, 0666));
			SYSCHECK(symlink(channel->ptsname, s_path[channel->id - 1]));
		} else {
			LOGMUX(LOG_ERR, "MTK only support 8 channels");
			return GSM0710MUXD_EXCEED_SUPPORTED_VP_NUM;
		}
#endif
#endif  /* MUX_ANDROID */
	}

	//create thread
	LOGMUX(LOG_ERR, "New channel properties: number: %d fd: %d device: %s", channel->id, channel->fd, channel->devicename);

	//iniitialize pointer to thread args
	Poll_Thread_Arg *poll_thread_arg = (Poll_Thread_Arg *)malloc(sizeof(Poll_Thread_Arg));
	poll_thread_arg->fd = channel->fd;
	poll_thread_arg->read_function_ptr = &pseudo_device_read;
	poll_thread_arg->read_function_arg = (void *)(channel);

	//create thread for reading input from virtual port
	if (create_thread(&channel->poll_thread_id, poll_thread, (void *)poll_thread_arg) != 0) {
		LOGMUX(LOG_ERR, "Could not create thread for listening on %s", channel->ptsname);
		return GSM0710MUXD_CREATE_THREAD_ERR;
	}
	LOGMUX(LOG_DEBUG, "Thread is running and listening on %s", channel->ptsname);

	return GSM0710MUXD_SUCCESS;
}

/*
 * Purpose:  Read from line discipline buffer. (input from application connected to pseudo terminal)
 * Input:      void pointer to Channel struct
 * Return:    1 if fail, 0 if success
 */
int pseudo_device_read(void *vargp)
{
	LOGMUX(LOG_INFO, "Enter");
	Channel *channel = (Channel *)vargp;
	/* Note by LS: Because the pseudo_device_read() is ivoked due to newly incoming data */
	/* The buffer size of N_TTY is defined as 4096 in tty.h: The maximum bytes of newly incoming data maybe 4096 */
	/* Change the buf[4096] to buf[1024]: It should use 4 times read() to achieve the original procedure */
	unsigned char buf[1024];
	int len = 0, read_action_count = 0;
	/* information from virtual port */
	/* channel->remaining records the num of bytes of data not written to the pty in the previous write_frame() */
	/* And these remaining data are stored at the memory address pointed by channel->tmp and not copied to the local buf yet */
	/* So, this time's newly read data should be stored to the local buf from the position buf + channel->remaining */

	if (!channel->opened) {
		/* Note by LS: In our implementation: each non-control channel must be opened at the initialization stage */
		Gsm0710Muxd_Assert(GSM0710MUXD_TXTHREAD_ERR);
#if 0
		LOGMUX(LOG_WARNING, "Write to a channel which wasn't acked to be open.");
		/* [Bug] Note by LS:But it already copied some data to the local buf !*/
		/* Possible solution#2: Copy the already read data to channel->tmp buffer */
		/* [Question] When to write the data stored in the channel->tmp buffer to the modem? and it may re-enter this function but UA is not received from the modem yet */
		/*
		 * if (len > 0)
		 * {
		 *  char* previous_channel_tmp = channel->tmp;
		 *  channel->tmp = malloc(len + channel->remaining);
		 *  memcpy(channel->tmp, previous_channel_tmp, channel->remaining);
		 *  memcpy(channel->tmp + channel->remaining, buf, len);
		 *  channel->remaining = len;
		 * }
		 */
		write_frame(channel->id, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF);
		LOGMUX(LOG_DEBUG, "Leave");
		return 1;
#endif
	}

	/* Modify by LS */
	for (read_action_count = 0; read_action_count < MUXD_PTY_READ_ROUND; read_action_count++) {
		memset(buf, 0, sizeof(buf));

#ifdef  __MUXD_FLOWCONTROL__
		pthread_mutex_lock(&channel->tx_fc_lock);
		if (channel->tx_fc_off) {
			/* If tx_fc is off, it should block here and wait for tx_fc_on siganl to allow to send data! */
			LOGMUX(LOG_DEBUG, "Wait for FC On siganl");
			pthread_cond_wait(&channel->tx_fc_on_signal, &channel->tx_fc_lock);
			/* It receives the MSC cmd with FC On again: tx_fc_off should be reset as zero when recving FC ON again */
			//channel->tx_fc_off = 0;
			LOGMUX(LOG_DEBUG, "Notify by FC On siganl,try to read data and to send it");
		}
		pthread_mutex_unlock(&channel->tx_fc_lock);
#endif  /* __MUXD_FLOWCONTROL__ */

		if ((len = read(channel->fd, buf, sizeof(buf))) >= 0) {
			LOGMUX(LOG_DEBUG, "pseudo_device_read read(channel->fd, buf, sizeof(buf))");
			LOGMUX(LOG_DEBUG, "pseudo_device_read len=%d", len);
			unsigned int remaining = 0;
			LOGMUX(LOG_DEBUG, "Data from channel %d, wanted_read=%d, actual_read=%d bytes with read_action_count=%d", channel->id, sizeof(buf), len, read_action_count);

#ifdef  __MUX_UT__
			if (channel->id == DEFAULT_MUX_UT_LB_TEST_CHNL_NUM) {
				/* Write data back to the sender */
				if (len > 0) {
					int lb_accumulated_write_len = 0, lb_this_write_len = 0;
					while (lb_accumulated_write_len < len) {
						lb_this_write_len = write(channel->fd, buf + lb_accumulated_write_len, len - lb_accumulated_write_len);
						if (lb_this_write_len >= 0) {
							LOGMUX(LOG_DEBUG, "LB this time write back data len=%d, total read len=%d", lb_this_write_len, len);
							lb_accumulated_write_len += lb_this_write_len;
						} else {
							switch (errno) {
							case EINTR:
								LOGMUX(LOG_ERR, "Interrupt signal EINTR caught");
								break;
							case EAGAIN:
								LOGMUX(LOG_ERR, "Interrupt signal EAGAIN(i.e.,Nonblocing mode read but no data in buf now) caught");
								break;
#if 0
							case EBADF:
								LOGMUXUTAPP(LOG_ERR, "Interrupt signal EBADF caught");
								break;
							case EINVAL:
								LOGMUXUTAPP(LOG_ERR, "Interrupt signal EINVAL caught");
								break;
							case EFAULT:
								LOGMUXUTAPP(LOG_ERR, "Interrupt signal EFAULT caught");
								break;
							case EIO:
								LOGMUXUTAPP(LOG_ERR, "Interrupt signal EIO caught");
								break;
							case EISDIR:
								LOGMUXUTAPP(LOG_ERR, "Interrupt signal EISDIR caught");
								break;
#endif
							default:
								LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught\n", errno);
								goto close_pty_channel;
							}
						}
					}
					lb_accumulated_write_len = 0;
					/* Simulate this time's read data are all written into the PTY for modem: It should return from this function */
				}
			} else if (channel->id == DEFAULT_MUX_UT_TX_TEST_CHNL_NUM) {
				LOGMUX(LOG_DEBUG, "UT mode Simulate wirte data to modem with len=%d", len);
			} else {
#endif
			if (len > 0)
				remaining = handle_channel_data(buf, len, channel->id);

			/* copy remaining bytes from last packet into tmp */
			if (remaining > 0) {
				/* Modify by LS: Because the serial.in_buf is opened as BLOCKING mode, the value returned from handle_channel_data() must be zero! */
				LOGMUX(LOG_ERR, "%d bytes can't be written to channel %d", channel->id,
				       remaining);
				Gsm0710Muxd_Assert(GSM0710MUXD_SERIAL_WRITE_ERR);
			}

			LOGMUX(LOG_DEBUG, "Leave");
			return 0;

#ifdef __MUX_UT__
		}
#endif
		} else {
			//When RILD crashes, read function returns error with errno other than EINTR and EAGAIN
			switch (errno) {
			case EINTR:
				LOGMUX(LOG_ERR, "Interrupt signal EINTR caught");
				/* It should retry the read action again! This failure should not be counted */
				read_action_count--;
				break;
			case EAGAIN:
				LOGMUX(LOG_ERR, "Interrupt signal EAGAIN(i.e.,Nonblocing mode read but no data in buf now) caught");
				if (read_action_count == 0) {
					/* pseudo_device_read() is invoked due to FDISSET, it should not no data in the buffer when 1st read() is invoked */
					Gsm0710Muxd_Assert(GSM0710MUXD_PTY_READ_ERR);
				} else {
					/* Add by LS: It alread read data from the buffer due to FDISSET, it can return now due to no data in the buffer now */
					//read_action_count--;
					/* Because read_action_count is not zero: It means at least one read action is performed!*/
					/* If there is no data in the pty buffer, it can exit this for-loop then waits for next read_fd enabled via select() */
					/* read_action_count is 3, after break statement: read_action_count is updated to 4 then exit this for-loop */
					read_action_count = (MUXD_PTY_READ_ROUND - 1);
				}
				break;
			case EIO:
				LOGMUX(LOG_ERR, "Interrupt signal EIO caught");
				goto close_pty_channel;
				break;
            #if 0
			case EBADF:
				LOGMUX(LOG_ERR, "Interrupt signal EBADF caught");
				break;
			case EINVAL:
				LOGMUX(LOG_ERR, "Interrupt signal EINVAL caught");
				break;
			case EFAULT:
				LOGMUX(LOG_ERR, "Interrupt signal EFAULT caught");
				break;
			case EISDIR:
				LOGMUX(LOG_ERR, "Interrupt signal EISDIR caught");
				break;
            #endif
			default:
				LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught\n", errno);
				goto close_pty_channel;
			}
			// dropped connection. close channel but re-open afterwards
		}
	}

	if (read_action_count != MUXD_PTY_READ_ROUND)
		Gsm0710Muxd_Assert(GSM0710MUXD_PTY_READ_ERR);

	/* Total 4 read() action and each time try to read maximum size is 1024 bytes */
	/* Because N_TTY BUF SIZE is 4096: If each time read with buf's size 1024, it should execute 4 times read action */
	/* PTY's peer side may write 4096 bytes at one write action into the PTY Buffer: 4096 bytes data to be processed due to FD_READ_SET is on */
	LOGMUX(LOG_DEBUG, "End of Execute 4 read actions for this time's read_fd is select");
	return 0;

close_pty_channel:
	LOGMUX(LOG_INFO, "Appl. dropped connection, device %s shutting down. Set to be reopened", channel->ptsname);
	/*disconnect channel from pty*/
	if (channel->ptslink != NULL) unlink(channel->ptslink);
	close(channel->fd);
	LOGMUX(LOG_DEBUG, "pseudo_device_read close(channel->fd);");
	channel->fd = -1;
	free(channel->ptsname);
	channel->ptsname = NULL;

	/* set channel to be reopened. this will not be cleared when doing a channel close */
	channel->reopen = 1;

	/*global flag to signal at least one channel needs to be reopened */
	pthread_mutex_lock(&pts_reopen_lock);
	pts_reopen = 1;
	pthread_mutex_unlock(&pts_reopen_lock);

	//LOGMUX(LOG_DEBUG, "Leave");
	return GSM0710MUXD_PTY_READ_ERR;
}


/*
 * Purpose:  Allocate a channel and corresponding virtual port and a start a reading thread on that port
 * Input:      origin - string to define origin of allocation
 * Return:    1 if fail, 0 if success
 */
static int c_alloc_channel()
{
	int i = 1, id = 0, rc = -1;

	LOGMUX(LOG_DEBUG, "Enter");

	if (serial.state == MUX_STATE_MUXING) {
#ifdef  MUX_ANDROID
		// create channels in ch_cfg first
		for (i = 1; i < MUXD_CH_NUM_ALL; i++) {
			id = ch_cfg[i].pn_dlci;

			if (channellist[id].fd < 0) {
				rc = logical_channel_establish(&channellist[id]);
				return rc;
			}
		}
#endif
		// if all channels in ch_cfg are established, find a free slot for new channel
		for (i = 1; i < GSM0710_MAX_CHANNELS; i++) {
			if (channellist[i].fd < 0) {
				rc = logical_channel_establish(&channellist[id]);
				return rc;
			}
		}
	}

	LOGMUX(LOG_ERR, "Not muxing or no free channel found");
	return GSM0710MUXD_ALLOC_CHANNEL_ERR;
}

/*
 * Purpose:  Allocates memory for a new buffer and initializes it.
 * Input:      -
 * Return:    pointer to a new buffer
 */
static GSM0710_Buffer *gsm0710_buffer_init()
{
	GSM0710_Buffer *buf = (GSM0710_Buffer *)malloc(sizeof(GSM0710_Buffer));

	if (buf != NULL) {
		memset(buf, 0, sizeof(GSM0710_Buffer));
		buf->readp = buf->data;
		buf->writep = buf->data;
		buf->endp = buf->data + GSM0710_BUFFER_SIZE;

		buf->bufferready_signal.value = 0;
		buf->newdataready_signal.value = 0;
	}
	return buf;
}

/*
 * Purpose:  Destroys the buffer (i.e. frees up the memory
 * Input:      buf - buffer to be destroyed
 * Return:    -
 */
static void gsm0710_buffer_destroy(GSM0710_Buffer *buf)
{
	free(buf);
}

/*
 * Purpose:  Writes data to the buffer
 * Input:      buf - pointer to the buffer
 *                input - input data (in user memory)
 *                length - how many characters should be written
 * Return:    number of characters written
 */
int gsm0710_buffer_write(
	GSM0710_Buffer *	buf,
	const unsigned char *	input,
	int			length)
{
	LOGMUX(LOG_DEBUG, "Enter");
	LOGMUX(LOG_DEBUG, "GSM0710 buffer (up-to-date): free %d, stored %d", gsm0710_buffer_free(buf), gsm0710_buffer_length(buf));
	int c = buf->endp - buf->writep;
	length = min(length, (int)gsm0710_buffer_free(buf));
	/* If buf->readp is located between the buf->writep and buf->endp: the free space will be less than c */
	/* It also means that the "min(length,free space)" must be less than c */
	if (length > c) {
		/* In this case: buf->readp is located in the front of the buf->writep */
		/* After data is written to the end of buf, it will be wrapped around to the start of the buf */
		memcpy(buf->writep, input, c);
		memcpy(buf->data, input + c, length - c);
		buf->writep = buf->data + (length - c);
	} else {
		/* In this case: buf->readp is located between the buf->writep and buf->endp */
		memcpy(buf->writep, input, length);
		buf->writep += length;
		if (buf->writep == buf->endp)
			buf->writep = buf->data;
	}

	pthread_mutex_lock(&buf->datacount_lock);
	/* After copying the data to the serial->in_buf, it is time to update datacount to avoid read thread to get invalid data */
	buf->datacount += length; /*updating the data-not-yet-read counter*/
	LOGMUX(LOG_DEBUG, "GSM0710 buffer (up-to-date): written %d, free %d, stored %d", length, gsm0710_buffer_free(buf), gsm0710_buffer_length(buf));
	pthread_mutex_unlock(&buf->datacount_lock);

	pthread_mutex_lock(&buf->newdataready_lock);
	buf->newdataready = 1; /*signal assemble_frame_thread that new buffer data is ready and stored in serial->in_buf */
	pthread_mutex_unlock(&buf->newdataready_lock);
	pthread_cond_signal(&buf->newdataready_signal);

	LOGMUX(LOG_DEBUG, "Leave");
	return length;
}

/*
 * Purpose:  destroys a frame
 * Input:      frame - pointer to the frame
 * Return:    -
 */
void destroy_frame(GSM0710_Frame *frame)
{
	if (frame->length > 0) {
		LOGMUX(LOG_DEBUG, "frame_data ptr=0x%02X, frame_len=%d", (unsigned int)frame->data, frame->length);

		if (frame->data != NULL)
			free(frame->data);
	}
	free(frame);
}

/*
 * Purpose:  Gets a complete basic mode frame from buffer. You have to remember to free this frame
 *                when it's not needed anymore
 * Input:      buf - the buffer, where the frame is extracted
 * Return:    frame or null, if there isn't ready frame with given index
 */
static GSM0710_Frame *gsm0710_base_buffer_get_frame(GSM0710_Buffer *buf, int passed_depth_level)
{
	int end, depth_level;
	unsigned int length_needed = 5; // channel(Address Field), type(Control Field), length(Lengthe Field), fcs, flag
	unsigned char fcs = 0xFF;
	GSM0710_Frame *frame = NULL;
	unsigned char *local_readp;
	unsigned int local_datacount, local_datacount_backup;

	LOGMUX(LOG_DEBUG, "Enter: depth_level=%d", passed_depth_level);

	/* Add by LS: Check the recursive call depth to avoid the stack overflow problem */
	if (passed_depth_level < MAX_RESYNC_GET_FRAME_COUNT)
		depth_level = passed_depth_level;
	else
		Gsm0710Muxd_Assert(13);

	/*Find start flag*/
	while (!buf->flag_found && gsm0710_buffer_length(buf) > 0) {
		if (*buf->readp == GSM0710_FRAME_FLAG) {
			buf->flag_found = 1;
			LOGMUX(LOG_DEBUG, "Find Start Flag");
		}

		pthread_mutex_lock(&buf->datacount_lock); /* need to lock to operate on buf->datacount*/
		LOGMUX(LOG_DEBUG, "buf_readp val=0x%02X", *(buf->readp));
		gsm0710_buffer_inc(buf->readp, buf->datacount);
		LOGMUX(LOG_DEBUG, "current buf_readp val=0x%02X", *(buf->readp));
		pthread_mutex_unlock(&buf->datacount_lock);
	}
	if (!buf->flag_found) { // no frame started
		LOGMUX(LOG_DEBUG, "Leave. No start frame 0xf9 found in bytes stored in GSM0710 buffer");
		/* Add by LS: Once out of sync- After it checks all invalid bytes in buffer, it should signal another thread to put data into buffer again */
		/* In order to find the Start Flag then sync again! */

		pthread_mutex_lock(&buf->bufferready_lock);
		if (buf->input_sleeping == 1) {
			/* Because the serial->inbuf's data are copied to the frame->data, it is time to signal the thread_serial_device_read */
			//pthread_cond_signal(&newdataready_signal);
			/* [Bug] Note by LS: It should signal the thread waits for the bufferready_signal instead of newdataready_signal */
			LOGMUX(LOG_DEBUG, "Signal thread serial device read(): case2");
			pthread_cond_signal(&buf->bufferready_signal);
		}
		pthread_mutex_unlock(&buf->bufferready_lock);

		return NULL;
	}
	/*skip empty frames (this causes troubles if we're using DLC 62) - skipping frame start flags*/
	while (gsm0710_buffer_length(buf) > 0 && (*buf->readp == GSM0710_FRAME_FLAG)) {
		pthread_mutex_lock(&buf->datacount_lock); /* need to lock to operate on buf->datacount*/
		gsm0710_buffer_inc(buf->readp, buf->datacount);
		LOGMUX(LOG_DEBUG, "skip continus start flag:current buf_readp=0x%02X", *(buf->readp));
		pthread_mutex_unlock(&buf->datacount_lock);
	}

	/* Okay, we're ready to analyze a proper frame header */
	/*Make local copy of buffer pointer and data counter. They are shared between 2 threads, so we want to update them only after a frame extraction */
	/*From now on, only operate on these local copies */
	LOGMUX(LOG_DEBUG, "Ready to check Addr Field:buf_readp val=0x%02X", *(buf->readp));
	local_readp = buf->readp;
	local_datacount = local_datacount_backup = buf->datacount;      /* current no. of stored bytes in buffer */

	if (local_datacount >= length_needed) {                         /* enough data stored for 0710 frame header+footer? */
		if ((frame = (GSM0710_Frame *)malloc_r(sizeof(GSM0710_Frame))) != NULL) {
			/* Add by LS */
			/* It it does not set zero: when it encounters an error before allocating mem for frame->data */
			/* In destroy_frame(): It will free the memory pointed by frame->data abnormally */
			memset(frame, 0, sizeof(GSM0710_Frame));

			/* Parse Address Field */
			LOGMUX(LOG_DEBUG, "UIH Addr Filed value=0x%02X", *local_readp);

			/* Enter this function due to incoming data */
			frame->channel = GSM0710_DECODE_ADDRESS(*local_readp); /*frame header address-byte read*/

			LOGMUX(LOG_DEBUG, "UIH Addr Filed chnl value=%d", frame->channel);
            #if defined(__CCMNI_SUPPORT__) && (defined(__MUX_IT__) || defined(__MUX_UT__))
			if (frame->channel >= (MAX_NON_GEMINI_CHNL_NUM - 2 + MUX_TEST_CHNL_OFFSET)) {
				/* Modify the received channel number from 28-30 (Modem MUX defined IT channels) to channel num values (used for channelist[]) 9-11 */
				frame->channel -= MUX_TEST_CHNL_OFFSET;
				LOGMUX(LOG_DEBUG, "UIH Addr Filed modified chnl num =%d", frame->channel);
			}
            #endif  /* __CCMNI_SUPPORT__ && __MUX_IT__ && __MUX_UT__ */
            
			/* Note by LS: Although the frame->channel is invalid, but */
			/* [Q] Why does it not parse the length field? In this way, it can throw the whole frame data by moving ptrs */
			/* [A] Because once it enters this function again, it will use a while-loop to find the 1st F9: In this way, The previous frame's data can be regarded as thrown correctly */
            #if 0
			if (frame->channel > vir_ports) { /* Field Sanity check if channel ID actually exists */
				LOGMUX(LOG_WARNING, "Dropping frame: Corrupt! Channel Addr. field indicated %d, which does not exist", frame->channel);
				free(frame);
				buf->flag_found = 0;
				buf->dropped_count++;
				/* If this DLCI is an invalid one, it will find the next start flag to obtain a frame by invoking this function again */
				goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
			}
            #endif

			fcs = r_crctable[fcs ^ *local_readp];
			gsm0710_buffer_inc(local_readp, local_datacount);
			length_needed--;
			frame->control = *local_readp; /*frame header type-byte read*/
			fcs = r_crctable[fcs ^ *local_readp];
			gsm0710_buffer_inc(local_readp, local_datacount);
			length_needed--;
			/* Length field: LSB bit is E/A bit */
			frame->length = (*local_readp & 254) >> 1; /*Frame header 1st length-byte read*/
			LOGMUX(LOG_DEBUG, "UIH Length Filed Byte#1=0x%02X,frame_length=%d", *local_readp, frame->length);
			fcs = r_crctable[fcs ^ *local_readp];
			/* Note that: After check the length field value, length_needed and local_datacount are not discounted by one */
			/*length_needed : 1 length byte + 1 fcs byte + 1 end frame flag */
		}

		/* Note by LS: Check if the maximum Length field occupies two bytes or not - According to Spec: The Max Length field is 2-bytes */
		/* if frame payload length byte E/A bit not set, a 2nd length byte is in header */
		if ((*local_readp & 1) == 0) {
			//Current spec (version 7.1.0) states these kind of
			//frames to be invalid Long lost of sync might be
			//caused if we would expect a long frame because of an
			//error in length field.

			/* Because the default length_needed only counts 1 byte for Length field: in this case */
			/* Only local_datacount needs to discount by one then buf->readp will be pointed to the 1st byte of the Information field */
			/* Now, local_datacount = 1 length byte + payload + 1 fcs byte + 1 end frame flag ; it should be equal to length_needed */
			gsm0710_buffer_inc(local_readp, local_datacount);
			/* 2nd byte: Its valus should shift left 7 bit and then add with 1st length field's value => original value + 2nd byte-value x128 */
			frame->length += (*local_readp * 128); /*Frame header 2nd length-byte read*/
			LOGMUX(LOG_DEBUG, "UIH Length Filed Byte#2=0x%02X,frame_length=%d", *local_readp, frame->length);
			fcs = r_crctable[fcs ^ *local_readp];
		}

		LOGMUX(LOG_DEBUG, "Before: frame_length=%d", frame->length);
		length_needed += frame->length; /*length_needed : 1 length byte + payload + 1 fcs byte + 1 end frame flag */
		LOGMUX(LOG_DEBUG, "length_needed: %d, available in local_datacount: %d, frame_len=%d", length_needed, local_datacount, frame->length);

		/* [Bug] Note by LS: It should check each channel's negotiated_N1 value instead of cmux_N1 */
        #ifndef __MUX_UT__
		if (frame->length > channellist[frame->channel].negotiated_N1 /*|| frame->channel > vir_ports*/) /* Field Sanity check if payload is bigger than the max size negotiated in +CMUX */
        #else   /* __MUX_UT__ */
		if ((frame->length > channellist[frame->channel].negotiated_N1 && frame->channel <= MAX_NON_GEMINI_NON_DATA_CHNL_NUM)
		    || (frame->length > get_mux_ut_channel_n1() && frame->channel > MAX_NON_GEMINI_NON_DATA_CHNL_NUM))
		/* Case#1:Normal cmd & urc chnl: Frame size must be less than negotiated N1 ; Case#2: Test chnl: Frame size must be less than setting value */
        #endif  /* __MUX_UT__ */
		{
            #ifndef __MUX_UT__
			LOGMUX(LOG_WARNING, "Dropping frame: Chnl_Num=%d,Corrupt! Length field indicated=%d. Max=%d allowed", frame->channel, frame->length, channellist[frame->channel].negotiated_N1);
            #else   /* __MUX_UT__ */
			if (frame->channel <= MAX_NON_GEMINI_NON_DATA_CHNL_NUM)
				LOGMUX(LOG_WARNING, "Dropping frame: Chnl_Num=%d,Corrupt! Length field indicated=%d. Max=%d allowed", frame->channel, frame->length, channellist[frame->channel].negotiated_N1);
			else
				LOGMUX(LOG_WARNING, "Dropping frame: Chnl_Num=%d, Corrupt! Length field indicated=%d. MAX UT N1=%d allowed", frame->channel, frame->length, get_mux_ut_channel_n1());

            #endif  /* __MUX_UT__ */

            #if 0
			/* Note by LS: Before throwing this frame, it should update the local_readp and local_datacount to consume this frame and move ptrs to the next frame's start position */
			/* Note by LS: The following byte of FCS field may be F9 (It may only one F9 or two F9 F9) */
			/* Because it resets buf->flag_found as zero, it should parse this F9 at next time to enter gsm0710_base_buffer_get_frame() */
			while ((frame->length + 1) > 0) { /* 1: One byte is FCS field */
				gsm0710_buffer_inc(local_readp, local_datacount);
				frame->length--;
			}
            #endif
			destroy_frame(frame);
			/* Note by LS: The following byte of FCS field may be F9 (It may only one F9 or two F9 F9) */
			/* Because it resets buf->flag_found as zero, it should parse this F9 at next time to enter gsm0710_base_buffer_get_frame() */
			buf->flag_found = 0;
			buf->dropped_count++;
			goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
		}

		if (!(local_datacount >= length_needed)) {
			destroy_frame(frame);
			/* Comment by LS: buf->readp will not be updated and still pointed to the Address field; buf->flag_found is still remainded as 1 */
			/* At this time, there is no any frame data written to the pty because no a completed frame can be obtained from the serial->in_buf */
			LOGMUX(LOG_DEBUG, "Leave, frame extraction cancelled. Frame not completely stored in re-assembly buffer yet");
			/* Becuase it does not have enough data, this thread will be notified again while data is read from the modem side */
			return NULL;
		}
		/* Pass the condition "local_datacount>=length_needed" check: It can get a completed frame */
		gsm0710_buffer_inc(local_readp, local_datacount);

		/*Okay, done with the frame header. Start extracting the payload data */
		if (frame->length > 0) {
			/* Now, local_readp is pointed to the 1st byte of the Information field and all data for this completed frame are stored in buf */
			if ((frame->data = malloc_r(sizeof(char) * frame->length)) != NULL) {
				end = buf->endp - local_readp;
				if (frame->length > end) { /*wrap-around necessary*/
					/* In this case: the buf->writep is not possible located between the buf->readp and buf->end */
					/* 1st: From local_readp to end */
					memcpy(frame->data, local_readp, end);
					/* 2nd: From start position(i.e.,buf->data) of buf to (frame->length - end) */
					memcpy(frame->data + end, buf->data, frame->length - end);
					/* Update the pointer local_readp to the FCS field */
					local_readp = buf->data + (frame->length - end);
					local_datacount -= frame->length;
				} else {
					memcpy(frame->data, local_readp, frame->length);
					local_readp += frame->length;
					local_datacount -= frame->length;
					if (local_readp == buf->endp)
						/* Update the pointer local_readp to the FCS field */
						local_readp = buf->data;
				}
				if (GSM0710_FRAME_IS(GSM0710_TYPE_UI, frame))
					for (end = 0; end < frame->length; end++)
						fcs = r_crctable[fcs ^ (frame->data[end])];
			}
		}
		/*Okay, check FCS*/
		if (r_crctable[fcs ^ (*local_readp)] != 0xCF) {
			/* Fail to the FCS check */
			gsm0710_buffer_inc(local_readp, local_datacount);
			if (*local_readp != GSM0710_FRAME_FLAG) { /* the FCS didn't match, but the next byte may not even be an end-frame-flag*/
				LOGMUX(LOG_WARNING, "Dropping frame: Corrupt! End flag not present and FCS mismatch.");
				destroy_frame(frame);
				buf->flag_found = 0;
				buf->dropped_count++;
				goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
			} else {
				/* Received FCS is matched the calculated one and the next byte is also F9 */
				LOGMUX(LOG_WARNING, "Dropping frame: FCS doesn't match");
				destroy_frame(frame);
				buf->flag_found = 0;
				buf->dropped_count++;
				goto update_buffer_dropping_frame; /* throw whole frame away, up until and incl. local_readp */
			}
		} else {
			/*Pass the FCS check: Okay, check end flag */
			gsm0710_buffer_inc(local_readp, local_datacount);
			if (*local_readp != GSM0710_FRAME_FLAG) {
				LOGMUX(LOG_WARNING, "Dropping frame: End flag not present. Instead: %d", *local_readp);
				destroy_frame(frame);
				buf->flag_found = 0;
				buf->dropped_count++;
				goto update_buffer_dropping_frame;
			} else {
				buf->received_count++;
			}
			gsm0710_buffer_inc(local_readp, local_datacount); /* prepare readp for next frame extraction */
		}
	} else {
		LOGMUX(LOG_DEBUG, "Leave, not enough bytes stored in buffer for header information yet");
		return NULL;
	}
	/* Everything went fine, update GSM0710 buffer pointer and counter */
	pthread_mutex_lock(&buf->datacount_lock);
	buf->readp = local_readp;
	buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
	pthread_mutex_unlock(&buf->datacount_lock);

	buf->flag_found = 0; /* prepare for any future frame processing*/

	if (frame->channel != MUXD_VT_CH_NUM)
		LOGMUX(LOG_INFO, "Get a complete frame. ch:%d, ctrl:%d, len:%d", frame->channel,
		       frame->control, frame->length);
	return frame;

update_buffer_dropping_frame:
	/*Update GSM0710 buffer pointer and counter */
	LOGMUX(LOG_WARNING, "update_buffer_dropping_frame");

	pthread_mutex_lock(&buf->datacount_lock);
	buf->readp = local_readp;
	buf->datacount -= (local_datacount_backup - local_datacount); /* subtract whatever we analyzed */
	pthread_mutex_unlock(&buf->datacount_lock);
	depth_level++;

	return gsm0710_base_buffer_get_frame(buf, depth_level); /*continue extracting more frames if any*/
}

/*
 * Purpose:  Gets a advanced option frame from buffer. You have to remember to free this frame
 *                when it's not needed anymore
 * Input:      buf - the buffer, where the frame is extracted
 * Return:    frame or null, if there isn't ready frame with given index
 */
static GSM0710_Frame *gsm0710_advanced_buffer_get_frame(GSM0710_Buffer *buf)
{
	LOGMUX(LOG_DEBUG, "Enter");
l_begin:
	/* Okay, find start flag in buffer*/
	while (!buf->flag_found && gsm0710_buffer_length(buf) > 0) {
		if (*buf->readp == GSM0710_FRAME_ADV_FLAG) {
			buf->flag_found = 1;
			buf->adv_length = 0;
			buf->adv_found_esc = 0;
		}
		pthread_mutex_lock(&buf->datacount_lock); /* need lock to operate on buf->datacount*/
		gsm0710_buffer_inc(buf->readp, buf->datacount);
		pthread_mutex_unlock(&buf->datacount_lock);
	}

	if (!buf->flag_found) // no frame started
		return NULL;

	/* skip empty frames (this causes troubles if we're using DLC 62) */
	if (0 == buf->adv_length) {
		while (gsm0710_buffer_length(buf) > 0 && (*buf->readp == GSM0710_FRAME_ADV_FLAG)) {
			pthread_mutex_lock(&buf->datacount_lock); /* need to lock to operate on buf->datacount*/
			gsm0710_buffer_inc(buf->readp, buf->datacount);
			pthread_mutex_unlock(&buf->datacount_lock);
		}
	}

	/* Okay, we're ready to start analyzing the frame and filter out any escape char */
	while (gsm0710_buffer_length(buf) > 0) {
		if (!buf->adv_found_esc && GSM0710_FRAME_ADV_FLAG == *(buf->readp)) { /* Whole frame parsed for escape chars, closing flag found */
			GSM0710_Frame *frame = NULL;
			unsigned char *data = buf->adv_data;
			unsigned char fcs = 0xFF;
			pthread_mutex_lock(&buf->datacount_lock); /* need to lock to operate on buf->datacount*/
			gsm0710_buffer_inc(buf->readp, buf->datacount);
			pthread_mutex_unlock(&buf->datacount_lock);

			if (buf->adv_length < 3) {
				LOGMUX(LOG_WARNING, "Too short adv frame, length:%d", buf->adv_length);
				buf->flag_found = 0;
				goto l_begin; /* throw away current frame and start looking for new frame start flag */
			}
			/* Okay, extract the header information */
			if ((frame = (GSM0710_Frame *)malloc(sizeof(GSM0710_Frame))) != NULL) { /* frame is sane, allocate memory for it */
				frame->channel = ((data[0] & 252) >> 2);                        /* the channel address field */
				fcs = r_crctable[fcs ^ data[0]];
				frame->control = data[1];                                       /* the frame type field */
				fcs = r_crctable[fcs ^ data[1]];
				frame->length = buf->adv_length - 3;                            /* the frame length field (total - address field - type field - fcs field) */
			} else {
				LOGMUX(LOG_ERR, "Out of memory, when allocating space for frame");
			}
			/* Okay, extract the payload data */
			if (frame->length > 0) {
				if ((frame->data = (unsigned char *)malloc(sizeof(char) * frame->length))) {
					memcpy(frame->data, data + 2, frame->length); /*copy data from first payload field*/
					if (GSM0710_FRAME_IS(GSM0710_TYPE_UI, frame)) {
						int i;
						for (i = 0; i < frame->length; ++i)
							fcs = r_crctable[fcs ^ (frame->data[i])];
					}
				} else {
					LOGMUX(LOG_ERR, "Out of memory, when allocating space for frame data");
					buf->flag_found = 0;
					goto l_begin;
				}
			}
			/* Okay, check FCS field */
			if (r_crctable[fcs ^ data[buf->adv_length - 1]] != 0xCF) {
				LOGMUX(LOG_WARNING, "Dropping frame: FCS doesn't match");
				destroy_frame(frame);
				buf->flag_found = 0;
				buf->dropped_count++;
				goto l_begin;
			} else {
				buf->received_count++;
				buf->flag_found = 0;
				LOGMUX(LOG_DEBUG, "Leave success");
				return frame;
			}
		}

		if (buf->adv_length >= sizeof(buf->adv_data)) { /* frame data too much for buffer.. increase buffer size? */
			LOGMUX(LOG_WARNING, "Too long adv frame, length:%d", buf->adv_length);
			buf->flag_found = 0;
			buf->dropped_count++;
			goto l_begin;
		}

		if (buf->adv_found_esc) { /* Treat found escape char (throw it away and complement 6th bit in next field */
			buf->adv_data[buf->adv_length] = *(buf->readp) ^ GSM0710_FRAME_ADV_ESC_COPML;
			buf->adv_length++;
			buf->adv_found_esc = 0;
		} else if (GSM0710_FRAME_ADV_ESC == *(buf->readp)) {    /* no untreated escape char. if current field is escape char, note it down */
			buf->adv_found_esc = 1;
		} else {                                                /*field is regular payload char, store it*/
			buf->adv_data[buf->adv_length] = *(buf->readp);
			buf->adv_length++;
		}

		/* need to lock to operate on buf->datacount*/
		pthread_mutex_lock(&buf->datacount_lock);
		gsm0710_buffer_inc(buf->readp, buf->datacount);
		pthread_mutex_unlock(&buf->datacount_lock);
	}
	return NULL;
}

/*
 * Purpose:  Compares two strings.
 *                strstr might not work because WebBox sends garbage before the first OK
 *                when it's not needed anymore
 * Input:      haystack - string to check
 *                length - length of string to check
 *                needle - reference string to compare to. must be null-terminated.
 * Return:    1 if comparison was success, else 0
 */
static int memstr(const char *haystack,	int length,	const char *needle)
{
	int i;
	int j = 0;

	if (needle[0] == '\0')
		return 1;
	for (i = 0; i < length; i++)
		if (needle[j] == haystack[i]) {
			j++;
			if (needle[j] == '\0') // Entire needle was found
				return 1;
		} else {
			j = 0;
		}
	return 0;
}

/*
 * Purpose:  Wait for URC +EIND: 128
 * Input:    fd - file descriptor
 *           to - how many seconds to wait for response
 * Return:   0 on success (OK-response), -1 otherwise
 */
static int poll_modem_ready(int	serial_device_fd, int to)
{
	/* Note by LS: buf[] is used to store the +EIND: 128; it may not use so large buf-size (e.g., 1024) */
	unsigned char buf[1024];
	int sel;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(serial_device_fd, &rfds);
	struct timeval timeout;
	timeout.tv_sec = (time_t)to;
	timeout.tv_usec = 0;
	int len;

	LOGMUX(LOG_DEBUG, "gsm0710muxd: wait for modem boot up");

	do {
		sel = select(serial_device_fd + 1, &rfds, NULL, NULL, &timeout);
		if (sel < 0) {
			if (errno == EINTR) {
				sel = 1;
				continue;
			} else {
				LOGMUX(LOG_ERR, "select failure!");
				break;
			}
		} else if (sel == 0) {
			sel = 1;
			LOGMUX(LOG_ERR, "select timeout");
			return -1;
			//continue;
		}

		LOGMUX(LOG_DEBUG, "Selected %d", sel);
		if (FD_ISSET(serial_device_fd, &rfds)) {
			memset(buf, 0, sizeof(buf));
			len = read(serial_device_fd, buf, sizeof(buf));
			SYSCHECK(len);
			LOGMUX(LOG_DEBUG, "Read %d bytes from serial device", len);
			syslogdump("<s ", buf, len);
			errno = 0;
			if (memstr((char *)buf, len, "+EIND: 128")) {
				LOGMUX(LOG_DEBUG, "Received +EIND: 128");
				return 0;
			}
		}
	} while (sel);
	return -1;
}

/*
 * Purpose:  Sends an AT-command to a given serial port and waits for reply.
 * Input:      fd - file descriptor
 *                cmd - command
 *                to - how many seconds to wait for response
 * Return:   0 on success (OK-response), -1 otherwise
 */
static int chat(int	serial_device_fd, char * cmd, int to)
{
	LOGMUX(LOG_DEBUG, "Enter");
	/* Note by LS: buf[] is used to store the AT cmd; it may not use so large buf-size (e.g., 1024) */
	/* Currently, the maximum size passed to chat is start_muxer(): gsm_command[100] */
	unsigned char buf[1024];
	int wrote = 0;
	int cur = 0;
	int len = strlen(cmd);
	syslogdump(">s ", (unsigned char *)cmd, strlen(cmd));

#ifndef REVERSE_MTK_CHANGE
	/* the main string */
	while (cur < len) {
		do {
			wrote = write(serial_device_fd, cmd + cur, len - cur);
		} while (wrote < 0 && errno == EINTR);

		if (wrote < 0) {
			LOGMUX(LOG_ERR, "Wrote fail");
			return -1;
		}

		cur += wrote;
	}

	/* the \r  */

	do {
		wrote = write(serial_device_fd, "\r", 1);
    } while ((wrote < 0 && errno == EINTR) || (wrote == 0));

	if (wrote < 0) {
		LOGMUX(LOG_ERR, "Wrote CR fail");
		return -1;
	} else {
		wrote += cur;
	}
#else   /* REVERSE_MTK_CHANGE */
	SYSCHECK(wrote = write(serial_device_fd, cmd, strlen(cmd)));
#endif  /* REVERSE_MTK_CHANGE */
	LOGMUX(LOG_DEBUG, "Wrote %d bytes", wrote);

#ifdef  MUX_ANDROID
	/* tcdrain not available on ANDROID */
	//ioctl(serial_device_fd, TCSBRK, 1); //equivalent to tcdrain(). perhaps permanent replacement?
#else   /* MUX_ANDROID */
	SYSCHECK(tcdrain(serial_device_fd));
#endif  /* MUX_ANDROID */

	fd_set rfds;
	struct timeval timeout;
	int sel;

	while (1) {
		FD_ZERO(&rfds);
		FD_SET(serial_device_fd, &rfds);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
        #if 0
		SYSCHECK(sel = select(serial_device_fd + 1, &rfds, NULL, NULL, &timeout));
        #endif
		sel = select(serial_device_fd + 1, &rfds, NULL, NULL, &timeout);
		if (sel < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				LOGMUX(LOG_ERR, "select failure!");
				break;
			}
		} else if (sel == 0) {
			LOGMUX(LOG_ERR, "select timeout!");
			break;
		} else {
			LOGMUX(LOG_NOTICE, "Selected %d", sel);
			if (FD_ISSET(serial_device_fd, &rfds)) {
				memset(buf, 0, sizeof(buf));
				len = read(serial_device_fd, buf, sizeof(buf));
				SYSCHECK(len);
				LOGMUX(LOG_DEBUG, "Read %d bytes from serial device", len);
				syslogdump("<s ", buf, len);
				//errno = 0;
				if (memstr((char *)buf, len, "OK")) {
					LOGMUX(LOG_NOTICE, "Received OK");
					return 0;
				} else if (memstr((char *)buf, len, "ERROR")) {
					LOGMUX(LOG_WARNING, "Received ERROR");
					return -1;
				}
			} else { break; }
		}
	}
	return -1;
}

/*
 * Purpose:  Handles commands received from the control channel.
 * Input:    frame - the mux frame struct
 * Return:   0
 */
static int handle_command(GSM0710_Frame *frame)
{
	LOGMUX(LOG_DEBUG, "Enter");
	unsigned char type, signals;
	/* type_length: number of bytes of Message_Type field */
	int length = 0, i, type_length, id, supported = 1;
	unsigned char *response;
	//struct ussp_operation op;
	if (frame->length > 0) {
		/* UIH frame Information field: 1st byte: message_type: e.g., MSC, PN, RPN ...etc. */
		type = frame->data[0]; // only a byte long types are handled now skip extra bytes
		/* Note by LS: If it assumes that Message_Type is 1 byte, the following codes should be skipped */
		for (i = 0; (frame->length > i && (frame->data[i] & GSM0710_EA) == 0); i++) ;
		i++;
		type_length = i;

		if ((type & GSM0710_CR) == GSM0710_CR) {
			//command not ack extract frame length
			while (frame->length > i) {
				/* length (declare as int: max 4 bytes): number of bytes of Length field =  ((frame->data[i] & 11111110)>>1): LSB bit of frame->data[i] is E/A bit  */
				length = (length * 128) + ((frame->data[i] & 254) >> 1);
				if ((frame->data[i] & 1) == 1)
					break;
				i++;
			}
			i++;
			/* Now: i is the index of the 1st Value field */
			switch ((type & ~GSM0710_CR)) {
			case GSM0710_CONTROL_CLD:
				LOGMUX(LOG_INFO, "The mobile station requested mux-mode termination");
				/* Modify by LS */
				serial.state = MUX_STATE_PEER_CLOSING;
				break;
			case GSM0710_CONTROL_PSC:
			//LOGMUX(LOG_DEBUG, "Power Service Control command: ***");
			//LOGMUX(LOG_DEBUG, "Frame->data = %s / frame->length = %d", frame->data + i, frame->length - i);
			//break;
			case GSM0710_CONTROL_TEST:
			//LOGMUX(LOG_DEBUG, "Test command: ");
			//LOGMUX(LOG_DEBUG, "Frame->data = %s / frame->length = %d", frame->data + i, frame->length - i);
			//serial->ping_number = 0;
			//break;
			case GSM0710_CONTROL_PN:
				/* It will enter the if(supported) decision then send the PN rsp to the modem */
				break;

            #ifdef __MUXD_FLOWCONTROL__
			case GSM0710_CONTROL_MSC:
				/* For MSC command, it has at least two bytea of data follow it (One byte is DLCI and the other byte is V.24 signal) */
				if (i + 1 < frame->length) {
					Channel *channel;
					int fc_switched = 0;

					/* DLCI only occupies the bit#2-#7; bit#0 is E/A and bit#1 is always set as 1 */
					id = GSM0710_DECODE_ADDRESS(frame->data[i]);
					i++;
					/* V.24 signals: Only 1 byte */
					signals = (frame->data[i]);
					//op.op = USSP_MSC;
					//op.arg = USSP_RTS;
					//op.len = 0;
					LOGMUX(LOG_DEBUG, "Modem status command on channel %d", id);

					// Get channel context and validate it
					channel = &channellist[id];
					// Check if channel is opened
					if (!channel->opened) break;

					if ((signals & GSM0710_SIGNAL_FC) == GSM0710_SIGNAL_FC) {
                                                LOGMUX(LOG_INFO, "No frames allowed, channel id=%d,tx_fc_off=%d",channel->id,channel->tx_fc_lock);                         

						/* TX from the AP side to Modem is disallowed */
						pthread_mutex_lock(&channel->tx_fc_lock);
						if (channel->tx_fc_off == 0) {
							channel->tx_fc_off = 1;
							fc_switched = 1;
						}
						pthread_mutex_unlock(&channel->tx_fc_lock);

						// Choke Peer
						if (fc_switched)
							_fc_chokePty(channel);
					} else {
						//op.arg |= USSP_CTS;
                                                LOGMUX(LOG_INFO, "Frames allowed, channel id=%d,tx_fc_off=%d",channel->id,channel->tx_fc_lock);

						/* Receive TX Flow Control On */
						pthread_mutex_lock(&channel->tx_fc_lock);
						if (channel->tx_fc_off == 1) {
							LOGMUX(LOG_DEBUG, "Tell blocked thread: Recv FC On!");
							/* Add by LS: pseudo_device_read() thread may not have chance to check this variable even FC is on again */
							/* It should reset tx_fc_off as zero: In this way, when pseudo_device_read() thread to check (and actually FC is on) */
							/* pseudo_device_read() will read data from pty channel and write into pty again */
							/* without blocking to wait for cond signal's notification */
							channel->tx_fc_off = 0;
							/* This cond signal may be not affected: */
							/* due to pseudo_device_read thread still has no chance to execute the code "check tx_fc_off variable" */
							pthread_cond_signal(&channel->tx_fc_on_signal);

							fc_switched = 1;
						}
						pthread_mutex_unlock(&channel->tx_fc_lock);

						//Release Pty
						if (fc_switched)
							_fc_releasePty(channel);
					}

                    #if 0   //Doesn't handle following bytes
					if ((signals & GSM0710_SIGNAL_RTC) == GSM0710_SIGNAL_RTC)
						//op.arg |= USSP_DSR;
						LOGMUX(LOG_DEBUG, "Signal RTC");

					if ((signals & GSM0710_SIGNAL_IC) == GSM0710_SIGNAL_IC)
						//op.arg |= USSP_RI;
						LOGMUX(LOG_DEBUG, "Signal Ring");

					if ((signals & GSM0710_SIGNAL_DV) == GSM0710_SIGNAL_DV)
						//op.arg |= USSP_DCD;
						LOGMUX(LOG_DEBUG, "Signal DV");
                    #endif
				} else {
					LOGMUX(LOG_ERR, "Modem status command, but no info. i: %d, len: %d, data-len: %d",
					       i, length, frame->length);
				}
				break;
            #endif  /* __MUXD_FLOWCONTROL__ */

			default:
				LOGMUX(LOG_ERR, "Unknown command (%d) from the control channel", type);
				/* 2 + type_length: 2= 1 byte Message_Type field + 1byte Length field */
				if ((response = malloc(sizeof(char) * (2 + type_length))) != NULL) {
					i = 0;
					/* 1 byte Message_Type field: NSC(Non Supported Command) */
					response[i++] = GSM0710_CONTROL_NSC;
					type_length &= 127; //supposes that type length is less than 128
					/* 1 byte Length field */
					response[i++] = GSM0710_EA | (type_length << 1);
					while (type_length--) {
						/* Value field: Copy the received Message type field of request (i.e., from the byte of frame->data[0]) */
						response[i] = frame->data[i - 2];
						i++;
					}
					//write_frame(0, response, i, GSM0710_TYPE_UIH);
					rx_thread_write_frame(0, response, i, GSM0710_TYPE_UIH);
					free(response);
					supported = 0;
				} else {
					LOGMUX(LOG_ERR, "Out of memory, when allocating space for response");
				}
				break;
			}

			if (supported) {
				//acknowledge the command
				frame->data[0] = frame->data[0] & ~GSM0710_CR;
				/* Send the response of this received request frame back to the peer side (i.e., Modem) */
				/* [Bug] Note by LS: It seems that use the received value to respond directly */
				//write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);
				rx_thread_write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);

                #if 0   //Echo MSC message back doesn't match spec
				switch ((type & ~GSM0710_CR)) {
				case GSM0710_CONTROL_MSC:
					/* frame->control: This is the Control field */
					if (frame->control & GSM0710_PF) //Check if the P/F var needs to be set again (cleared in write_frame)
						uih_pf_bit_received = 1;
					LOGMUX(LOG_DEBUG, "Sending 1st MSC command App->Modem");
					/* After receiving the MSC cmd and sending MSC response, it will also send an MSC command from Ap to Modem */
					frame->data[0] = frame->data[0] | GSM0710_CR; //setting the C/R bit to "command"
					//write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);
					rx_thread_write_frame(0, frame->data, frame->length, GSM0710_TYPE_UIH);
					break;
				default:
					break;
				}
                #endif
			}
		} else {
			//received ack (i.e. response: i.e., CR-bit is 0) for a command
			switch ((type & ~GSM0710_CR)) {
			case GSM0710_CONTROL_NSC:
				LOGMUX(LOG_WARNING, "The mobile station didn't support the command sent");
				break;
			case GSM0710_CONTROL_MSC:
				LOGMUX(LOG_WARNING, "The mobile station receives acknowledgment of MSC msg");
				break;

                #if 0   //Reaction doesn't depends on peer's ack
				int local_rx_fc_cmd_sending = -1;
				int local_rx_fc_off = -1;

				/* [Bug] Note by LS: It should handle the UIH frame carried the MSC response (e.g., send the MSC cmd to stop receive data from modem) */
				/* Because our sent MSC cmd with FC Off only 4-bytes data: DLCI value is located 3-rd byte */
				id = GSM0710_DECODE_ADDRESS(frame->data[2]);
				channel = &channellist[id];

				pthread_mutex_lock(&channel->rx_fc_lock);
				local_rx_fc_cmd_sending = channel->rx_fc_cmd_sending;
				pthread_mutex_unlock(&channel->rx_fc_lock);
				if (local_rx_fc_cmd_sending == FC_OFF_SENDING) {
					pthread_mutex_lock(&channel->rx_fc_lock);
					channel->rx_fc_cmd_sending = FC_NONE;
					pthread_mutex_unlock(&channel->rx_fc_lock);

					LOGMUX(LOG_DEBUG, "Recv MSC with FC OFF Rsp: signal rx_fc_off_rsp to retry write thread");
					pthread_cond_signal(&channel->rx_fc_off_rsp_signal);
				} else if (local_rx_fc_cmd_sending == FC_ON_SENDING) {
					pthread_mutex_lock(&channel->rx_fc_lock);
					channel->rx_fc_cmd_sending = FC_NONE;
					/* AP side is ready to receive incoming data again! */
					channel->rx_fc_off = 0;
					pthread_mutex_unlock(&channel->rx_fc_lock);
					LOGMUX(LOG_DEBUG, "Recv MSC with FC ON Rsp: reset rx_fc_off as 0");
				} else {
					/* Not expect to receive this MSC Rsp */
					LOGMUX(LOG_WARNING, "Recv unexpected MSC Rsp");
				}
                #endif
			default:
				LOGMUX(LOG_DEBUG, "Command Acknowledged by the mobile station");
				break;
			}
		}
	}

	return 0;
}

/*
 * Purpose:  Extracts and assembles frames from the mux GSM0710 buffer
 * Input:    buf - the receiver buffer
 * Return:   number of frames extracted
 */
int extract_frames(GSM0710_Buffer *buf)
{
	static unsigned int sabm_ua_received = 0;
	int frames_extracted = 0;
	GSM0710_Frame *frame;
	Channel *channel;

	LOGMUX(LOG_DEBUG, "Enter");

	while ((frame = cmux_mode ? gsm0710_advanced_buffer_get_frame(buf) : gsm0710_base_buffer_get_frame(buf, 0))) {
		/* cmux_mode=0: basic mode; frame = gsm0710_base_buffer_get_frame(buf) */
		/* If it can't obtain a completed frame(e.g.,f9,......,f9) in gsm0710_base_buffer_get_frame(), NULL will be returned */
		frames_extracted++;
		/*Okay, go ahead and signal ser_read_thread to wake up if it is sleeping because reassembly buffer was full before */
		pthread_mutex_lock(&buf->bufferready_lock);
		if (buf->input_sleeping == 1) {
			/* Because the serial->inbuf's data are copied to the frame->data, it is time to signal the thread_serial_device_read */
			//pthread_cond_signal(&newdataready_signal);
			/* [Bug] Note by LS: It should signal the thread waits for the bufferready_signal instead of newdataready_signal */
			LOGMUX(LOG_DEBUG, "Signal thread serial device read(): case1");
			pthread_cond_signal(&buf->bufferready_signal);
		}
		pthread_mutex_unlock(&buf->bufferready_lock);

		if ((GSM0710_FRAME_IS(GSM0710_TYPE_UI, frame) || GSM0710_FRAME_IS(GSM0710_TYPE_UIH, frame))) {
			LOGMUX(LOG_DEBUG, "Frame is UI or UIH");
			if (frame->control & GSM0710_PF)
				uih_pf_bit_received = 1;

			if (frame->channel > 0) {
				channel = &channellist[frame->channel];
				//if (!channel->opened) TODO

				/* frame->channel is the DLCI#i; in this case: it is the non-control channel (i.e., DLCI#0) */
				LOGMUX(LOG_INFO, "Writing %d byte frame received on channel %d to %s", frame->length, frame->channel, channel->ptsname);
				//data from logical channel
				syslogdump("Frame:", frame->data, frame->length);

                #ifdef  __MUXD_FLOWCONTROL__
				// Cached this frame if FC is currently off
				pthread_mutex_lock(&channel->rx_fc_lock);
				if (channel->rx_fc_off) {
					_fc_cacheFrameData(channel, frame);
					frame = NULL;
				}
				pthread_mutex_unlock(&channel->rx_fc_lock);
                #endif  /* __MUXD_FLOWCONTROL__ */

				if (frame != NULL) {
					int write_result = -1;

					while (1) {
						if ((write_result = write(channel->fd, frame->data, frame->length)) >= 0) {
							LOGMUX(LOG_INFO, "write() returned. Written %d/%d bytes of frame to %s", write_result, frame->length, channel->ptsname);
							/* Ref Linux Man Page: fsync() transfers ("flushes") all modified in-core data of (i.e., modified buffer cache pages for) the file referred to by the file descriptor fd to the disk device (or other permanent storage device) where that file resides */
							/* The call blocks until the device reports that the transfer has completed */
							fsync(channel->fd); /*push to /dev/pts device */

                            #ifdef __MUXD_FLOWCONTROL__
							if ((frame->length - write_result) > 0) {
								/* Cache remaining data into a linked list */
								_fc_cacheRemainingFrameData(channel, frame, write_result);

								/* Start another thread to retry the action - writte data into the channel*/
								start_retry_write_thread(channel);
								frame = NULL;
							}
                            #endif  /* __MUXD_FLOWCONTROL__ */

							// Break while(1)
							break;
						} else {
							switch (errno) {
							case EINTR:
								LOGMUX(LOG_ERR, "Interrupt signal EINTR caught");
								break;
							case EAGAIN:
								LOGMUX(LOG_ERR, "Interrupt signal EAGAIN caught");
                                #ifdef  __MUXD_FLOWCONTROL__
								//Add by MTK03594
								//Disable RX flow control for VT call
								if (frame->channel != MUXD_VT_CH_NUM) {
									/* Cache remaining data into a linked list */
									_fc_cacheRemainingFrameData(channel, frame, 0);

									/* Start another thread to retry the action - writte data into the channel*/
									start_retry_write_thread(channel);
								} else {
									LOGMUX(LOG_ERR, "Discard VT frame");
								}

								frame = NULL;
								goto uih_process_done;
                                #endif  /* __MUXD_FLOWCONTROL__ */
								break;
                            #if 0
							case EBADF:
								LOGMUX(LOG_ERR, "Interrupt signal EBADF caught");
								break;
							case EINVAL:
								LOGMUX(LOG_ERR, "Interrupt signal EINVAL caught");
								break;
							case EFAULT:
								LOGMUX(LOG_ERR, "Interrupt signal EFAULT caught");
								break;
							case EIO:
								LOGMUX(LOG_ERR, "Interrupt signal EIO caught");
								break;
							case EFBIG:
								LOGMUX(LOG_ERR, "Interrupt signal EFBIG caught");
								break;
							case ENOSPC:
								LOGMUX(LOG_ERR, "Interrupt signal ENOSPC caught");
								break;
							case EPIPE:
								LOGMUX(LOG_ERR, "Interrupt signal EPIPE caught");
								break;
                            #endif
							default:
								if (channel->reopen) {
									LOGMUX(LOG_ERR, "channel%d needs to be reopened\n", channel->id);
									watchdog(&serial);
								} else {
									LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught from write()\n", errno);
									Gsm0710Muxd_Assert(9);
								}
								break;
							}
						}
					}

uih_process_done:
					{}
				}
			} else {
				//control channel command (i.e., UIH Frame with the control command sent on frame->channel#0)
				LOGMUX(LOG_DEBUG, "Frame channel == 0, control channel command");
				handle_command(frame);
			}
		} else {
			//not an information frame (e.g., SABM,UA,DISC and DM)
			LOGMUX(LOG_DEBUG, "Not an information frame");
			switch ((frame->control & ~GSM0710_PF)) {
			case GSM0710_TYPE_UA:
				LOGMUX(LOG_DEBUG, "Frame is UA");
				/* [Bug] Note by LS: If Muxd does not send the DISC previously, it should ignores this unexpected UA instead of closing the pty */
				if (channellist[frame->channel].opened) {
					/* Note by LS: It seems not to send DISC from Gsm0710Muxd! */
					/* pseudo_device_read() just reads data (i.e., UIH payload; but not control channel cmd) from master-pty and writes to the modem via serial */
					int check_ua_iterator = 0;

					pthread_mutex_lock(&ua_rsp_for_disc_lock);
					/* After all disc are sent to the modem side, it is allowed to wait for the UA Rsp to check */
					if (channellist[frame->channel].disc_ua_pending == 1) {
						/* Note by LS: Check if the channel num is larger than zero */
						if (frame->channel == 0) {
							/* AP side does not send DLCI on chnl#0 to close down the MUX: It should not receive UA for chnl#0 */
							//assert(0);
							Gsm0710Muxd_Assert(22);
						}

						LOGMUX(LOG_INFO, "Logical channel %d closed", frame->channel);

						/* Note by LS: origin, fd ptsname, v24_signals and symbolic link are setup in c_alloc_channel => reset in logical_channel_close() */
						SYSCHECK(logical_channel_close(channellist + frame->channel));
						channellist[frame->channel].disc_ua_pending = 0;
					} else {
						LOGMUX(LOG_INFO, "Channel Opened:ignore this UA frame for channel=%d", frame->channel);
					}

					/* Note by LS:Only UA Rsp is triggered by close_devices(): It needs to do the following check */
					if (close_down_send_disc_for_all_chnl == 1) {
						/* Add by LS: Check if all UA Rsp for non-control channel's DISC are received or not */
						for (check_ua_iterator = 1; check_ua_iterator < GSM0710_MAX_CHANNELS; check_ua_iterator++)
							if (channellist[check_ua_iterator].disc_ua_pending == 1)
								break;

						/* If the variable close_down_send_disc_for_all_chnl is not 1: It means that this UA for DISC is not triggered by close_devices() -> Not necessary to signal */
						if (check_ua_iterator == GSM0710_MAX_CHANNELS)
							/* It means that all UA Rsp for non-control channel's DISC are received */
							pthread_cond_signal(&ua_rsp_for_disc_signal);
					}
					pthread_mutex_unlock(&ua_rsp_for_disc_lock);
				} else {
					if (channellist[frame->channel].sabm_ua_pending == 1) {
						/* Note by LS: Should it check if this UA is triggered due to previous SABM first? */
						channellist[frame->channel].opened = 1;
						channellist[frame->channel].sabm_ua_pending = 0;

						if (frame->channel == 0) {
							LOGMUX(LOG_ERR, "Control channel opened");
							//send version Siemens version test
							//static unsigned char version_test[] = "\x23\x21\x04TEMUXVERSION2\0";
							//write_frame(0, version_test, sizeof(version_test), GSM0710_TYPE_UIH);
						} else {
							LOGMUX(LOG_ERR, "Logical channel %d opened", frame->channel);
						}

                        #ifdef  MUX_ANDROID
						// If all channels are established, set flag and send signal, as well
                        #ifdef  __ANDROID_VT_SUPPORT__
						if ((++sabm_ua_received) == (MUXD_CH_NUM_ALL - 1))
                        #else   /* __ANDROID_VT_SUPPORT__ */
						if ((++sabm_ua_received) == (MUXD_CH_NUM_ALL))
                        #endif
						{
							pthread_mutex_lock(&setup_chnl_complete_lock);
							mux_setup_chnl_complete = 1;
							pthread_mutex_unlock(&setup_chnl_complete_lock);

							LOGMUX(LOG_ERR, "Finish MUX Channel Setup Procedure");
							pthread_cond_signal(&setup_chnl_complete_signal);
						} else if (frame->channel == 24) {
							LOGMUX(LOG_ERR, "Finish MUX Channel Setup Procedure for VT");
						}
                        #endif /* MUX_ANDROID */
					} else if (channellist[frame->channel].disc_ua_pending == 1) {
						LOGMUX(LOG_INFO, "UA to acknowledgde DISC on channel %d received", frame->channel);
						channellist[frame->channel].disc_ua_pending = 0;
					} else {
						LOGMUX(LOG_INFO, "Channel Closed:ignore this UA frame for channel=%d", frame->channel);
					}
				}
				break;
			case GSM0710_TYPE_DM:
				if (channellist[frame->channel].opened) {
					SYSCHECK(logical_channel_close(channellist + frame->channel));
					LOGMUX(LOG_INFO, "DM received, so the channel %d was already closed",
					       frame->channel);
				} else {
					if (frame->channel == 0) {
						/* Note by LS: It seems that DM is triggered due to previous SABM on control channel */
						LOGMUX(LOG_INFO, "Couldn't open control channel.\n->Terminating");
						serial.state = MUX_STATE_CLOSING;
						//close channels
					} else {
						LOGMUX(LOG_INFO, "Logical channel %d couldn't be opened", frame->channel);
					}
				}
				break;
			case GSM0710_TYPE_DISC:
				if (channellist[frame->channel].opened) {
					channellist[frame->channel].opened = 0;
					//write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
					rx_thread_write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
					if (frame->channel == 0) {
						/* Note by LS */
						/* 0710Spec: Section 5.3.4:DISC command sent at DLCI 0 have the same meaning as the Multiplexer Close Down command */
						serial.state = MUX_STATE_PEER_CLOSING;
						LOGMUX(LOG_INFO, "Control channel closed");
					} else {
						LOGMUX(LOG_INFO, "Logical channel %d closed", frame->channel);
						/* Add by LS: Only disconnect one specific non-control channel */
						SYSCHECK(logical_channel_close(channellist + frame->channel));
					}
				} else {
					//channel already closed
					LOGMUX(LOG_WARNING, "Received DISC even though channel %d was already closed", frame->channel);
					//write_frame(frame->channel, NULL, 0, GSM0710_TYPE_DM | GSM0710_PF);
					rx_thread_write_frame(frame->channel, NULL, 0, GSM0710_TYPE_DM | GSM0710_PF);
				}
				break;
			case GSM0710_TYPE_SABM:
				//channel open request
				if (channellist[frame->channel].opened) {
					if (frame->channel == 0)
						LOGMUX(LOG_INFO, "Control channel opened");
					else
						LOGMUX(LOG_INFO, "Logical channel %d opened", frame->channel);
				} else {
					//channel already closed
					LOGMUX(LOG_WARNING, "Received SABM even though channel %d was already closed", frame->channel);
				}
				channellist[frame->channel].opened = 1;
				//write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
				rx_thread_write_frame(frame->channel, NULL, 0, GSM0710_TYPE_UA | GSM0710_PF);
				break;
			}
		}
		/* Memory allocation from frame and frame->data may be done in gsm0710_base_buffer_get_frame() */
		if (frame != NULL) destroy_frame(frame);
	}
	LOGMUX(LOG_DEBUG, "Leave");
	return frames_extracted;
}

/*
 * Purpose:  Thread function. Will constantly check GSM0710_Buffer for mux frames, and, if any,
 *                assemble them in Frame struct and send them to the appropriate pseudo terminal
 * Input:      vargp - void pointer to the receiver buffer
 * Return:    NULL - when buffer is destroyed
 */
void *assemble_frame_thread(void *vargp)
{
	GSM0710_Buffer *buf = (GSM0710_Buffer *)vargp;

	while (buf != NULL) {
		pthread_mutex_lock(&buf->newdataready_lock);
		while (!(buf->datacount > 0) || !buf->newdataready) {   /* true if no data available in buffer or no new data written since last extract_frames() call */
			if (buf->input_sleeping != 1) {                 /* when is sleeping, thread_serial_device_read is wait */
				LOGMUX(LOG_DEBUG, "assemble_frame_thread put to sleep. GSM0710 buffer stored %d, newdataready: %d", buf->datacount, buf->newdataready);
				/* It should be signaled via pthread_cond_signal(&newdataready_signal); via thread_serial_device_read() -> gsm0710_buffer_write() */
				pthread_cond_wait(&buf->newdataready_signal, &buf->newdataready_lock); /* sleep until signalled by thread_serial_device_read() */
				LOGMUX(LOG_DEBUG, "assemble_frame_thread awoken. GSM0710 buffer stored %d, newdataready: %d", buf->datacount, buf->newdataready);
			} else {
				LOGMUX(LOG_WARNING, "not newdataready but skip wait due to input is sleeping");
				break;
			}
		}

		buf->newdataready = 0; /*reset newdataready since buffer will be processed in extract_frames()*/
		pthread_mutex_unlock(&buf->newdataready_lock);

		// Extract frames from buffer
		extract_frames(buf);
	}

	LOGMUX(LOG_ERR, "assemble_frame_thread terminated");
	return NULL;
}

/* Add by LS: It is invoked :
 * (1) When receiving CLD command or DISC frame on control channel
 * (2) Receive exit signal to execute the finalization procedure
 */
static int shutdown_devices(int shutdown_type)
{
	LOGMUX(LOG_INFO, "GSM0710_MAX_CHANNELS=%d,serial.fd=%d",GSM0710_MAX_CHANNELS,serial.fd);
	int i, num_disc_ua_pending = 0;
	for (i = 1; i < GSM0710_MAX_CHANNELS; i++) {
		if (channellist[i].fd >= 0) {
			if (channellist[i].opened)
				/* When receiving the CLD or DISC frame on the control channel:
				 * It will think all DLCI are sent from the peer side: For any opened channel: Just close and reset its context
				 */
				logical_channel_close(&channellist[i]);
		}
	}

	if (serial.fd >= 0) {
		if (shutdown_type == SHUTDOWN_DEV_WO_ACTIVE_FINALIZED) {
			/* In this case: It means that AP side recevies the CLD or DISC frame on control channel */
			/* For receiving the exit signal situation:
			 * It means that the lower layer below MUX (e.g., CCCI-TTY driver) has something wrong:
			 * No any cmd can be sent over this bearer anymore: It only closes the serial.fd !
			 */
			unsigned char close_down_rsp[2];
			close_down_rsp[0] = (close_channel_cmd[0] & ~GSM0710_CR);
			close_down_rsp[1] = close_channel_cmd[1];

			/* Send CLD rsp on the control channel back to the peer side */
			write_frame(0, close_down_rsp, 2, GSM0710_TYPE_UIH);
		}
		SYSCHECK(close(serial.fd));
		LOGMUX(LOG_INFO, "shutdown_devices SYSCHECK(close(serial.fd));;");
		serial.fd = -1;
	}

	serial.state = MUX_STATE_OFF;
#if 0
	property_set("ctl.stop", "ril-daemon");
	rild_started = 0;
#endif
	return 0;
}

/*
 * Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
 * Input:      param - signal ID
 * Return:    -
 */
void signal_treatment(int param)
{
	LOGMUX(LOG_ERR, "signal_no=%d", param);
	switch (param) {
	case SIGPIPE:
		exit(0);
		break;
	case SIGHUP:
		//reread the configuration files
		break;
	case SIGINT:
		set_main_exit_signal(param);
		break;
	case SIGTERM:
        #ifdef __PRODUCTION_RELEASE__
		if (g_set_force_assert_flag == 1 && g_set_alarm_flag == 1) {
			/* Cancel this alarm and report flag */
			LOGMUX(LOG_ERR, "Recv SIGTERM:Cancel alarm and report flag");
			alarm(0);
			g_set_force_assert_flag = 0;
			g_set_alarm_flag = 0;
			exit(0);
		} else if (g_set_force_assert_flag == 1 && g_set_alarm_flag == 0) {
			g_set_force_assert_flag = 0;
			LOGMUX(LOG_ERR, "Recv SIGTERM before main process executes force assert and report");
			exit(0);
		} else
        #endif  /* __PRODUCTION_RELEASE__ */
		{
			/* It means that it needs to report actively */
			LOGMUX(LOG_ERR, "Not Muxd Force Assert To Log Case:recv SIGTERM");
			set_main_exit_signal(param);
		}
		break;
	case SIGUSR1:
		//exit(0);
		//sig_term(param);
		set_main_exit_signal(1);
		break;

	case SIGUSR2:
        LOGMUX(LOG_WARNING, "MUXD recv SIGUSR2,stop_muxd=%d",stop_muxd);
        if(stop_muxd != 1){
            stop_muxd = 1;	
            LOGMUX(LOG_ERR, "Close CCCI port,fd=%d",serial.fd);			
            shutdown_devices(SHUTDOWN_DEV_W_ACTIVE_FINALIZED);
        }			
        break;

    #ifdef MUX_ANDROID
    #ifdef __PRODUCTION_RELEASE__
	case SIGALRM:
		LOGMUX(LOG_ERR, "Recv SIGALRM: force_assert_flag=%d,alarm_flag=%d", g_set_force_assert_flag, g_set_alarm_flag);
		if (g_set_force_assert_flag == 1) {
			g_set_force_assert_flag = 0;
			g_set_alarm_flag = 0;

			/* Re-try report */
			/* Plug-in to report abnormal case2 by Android property_set() to launch another process */
#ifdef MTK_RIL_MD2
            property_set("mux.report.case", "6");
#else			
			property_set("mux.report.case", "2");
#endif
			property_set("ctl.start", "muxreport-daemon");
		}
		break;
    #endif  /* __PRODUCTION_RELEASE__ */
    #endif  /* MUX_ANDROID */

    case SIGKILL:
        LOGMUX(LOG_WARNING, "MUXD recv SIGKILL,stop_muxd=%d",stop_muxd);
    default:
        LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught\n", errno);
        if(stop_muxd != 1){
            stop_muxd = 1;
            LOGMUX(LOG_ERR, "Close CCCI port,fd=%d",serial.fd); 
            SYSCHECK(shutdown_devices(SHUTDOWN_DEV_W_ACTIVE_FINALIZED));
        }
        exit(0);
        break;
    }
}

/*
 * Purpose:  Poll a device without select(). read() will do the blocking if VMIN=1.
 *           call a reading function for the particular device
 * Input:    vargp - a pointer to a Poll_Thread_Arg struct.
 * Return:   NULL if error
 */
void *poll_thread_serial(void *vargp)
{
	LOGMUX(LOG_DEBUG, "Enter");
	Poll_Thread_Arg *poll_thread_arg = (Poll_Thread_Arg *)vargp;
	if (poll_thread_arg->fd == -1) {
		LOGMUX(LOG_ERR, "Serial port not initialized");
		goto terminate;
	}

	while (1) { /*Call reading function*/
		if ((*(poll_thread_arg->read_function_ptr))(poll_thread_arg->read_function_arg) != 0) {
			LOGMUX(LOG_WARNING, "Device read function returned error");
			goto terminate;
		}
	}
	goto terminate;

terminate:
	LOGMUX(LOG_ERR, "Device polling thread terminated");
	/* The memory required for poll_thread_arg is allocated while creating this thread */
	free(poll_thread_arg); //free the memory allocated for the thread args before exiting thread
	return NULL;
}

/*
 * Purpose:  Thread function. Reads whatever data is in the line discipline (coming from modem)
 * Input:    vargp - void pointer the serial struct
 * Return:   0 if data successfully read, else read error
 */
int thread_serial_device_read(void *vargp)
{
	Serial *serial = (Serial *)vargp;
	GSM0710_Buffer *buf = serial->in_buf;

	LOGMUX(LOG_DEBUG, "Enter");
	{
            if(stop_muxd == 1){
                LOGMUX(LOG_WARNING, "stop_muxd read");
                return 1;
            }
		
		switch (serial->state) {
		case MUX_STATE_MUXING:
		{
			/* Note by LS: It may not declare a local array with so large size; it can sync with CCCI TTY driver buf's size */
			unsigned char buffer[4096];
			int len;
			//input from serial port
			LOGMUX(LOG_DEBUG, "Serial Data");
			unsigned int length = 0;

			if ((length = gsm0710_buffer_free(buf)) > 0) { /*available space in buffer (not locked since we want to utilize all available space)*/
				if ((len = read(serial->fd, buffer, min(length, sizeof(buffer)))) > 0) {
				        syslogdump("<s ", buffer, len);
					/* Copy data from local buf to the serial->in_buf */
					gsm0710_buffer_write(buf, buffer, len);
				} else if ((length > 0) && (len == 0)) {
					LOGMUX(LOG_DEBUG, "Waiting for data from serial device");
				} else {
					switch (errno) {
					case EINTR:
						LOGMUX(LOG_ERR, "Interrupt signal EINTR caught");
						break;
					case EAGAIN:
						LOGMUX(LOG_ERR, "Interrupt signal EAGAIN caught");
						break;
                    #if 0
					case EBADF:
						LOGMUX(LOG_ERR, "Interrupt signal EBADF caught");
						break;
					case EINVAL:
						LOGMUX(LOG_ERR, "Interrupt signal EINVAL caught");
						break;
					case EFAULT:
						LOGMUX(LOG_ERR, "Interrupt signal EFAULT caught");
						break;
					case EIO:
						LOGMUX(LOG_ERR, "Interrupt signal EIO caught");
						break;
					case EISDIR:
						LOGMUX(LOG_ERR, "Interrupt signal EISDIR caught");
						break;
                    #endif
					default:
						LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught\n", errno);
						//set_main_exit_signal(1);
						Gsm0710Muxd_Assert(1);
					}
					/* Note by LS: How to handle this error case? */
				}
			} else {
				/* Okay, internal buffer is full. we need to wait for the assembly thread to deliver a frame to the app(s). and free-up space */
				pthread_mutex_lock(&buf->bufferready_lock);
				LOGMUX(LOG_WARNING, "Internal re-assembly buffer is full, waiting for flush to appl!");
				buf->input_sleeping = 1; /* set sleeping flag - must be inside lock*/
				while (!(gsm0710_buffer_free(buf)) > 0) {
					LOGMUX(LOG_DEBUG, "ser_read_thread put to sleep. GSM0710 buffer has %d bytes free", gsm0710_buffer_free(buf));
					/* Comment by LS: Once the data is removed from the serial->inbuf to frame->data in extract_frames(): it will invoke pthread_cond_wait(&bufferready_signal,&bufferready_lock) */
					pthread_cond_wait(&buf->bufferready_signal, &buf->bufferready_lock); /* sleep until signalled by assembly thread() */
					LOGMUX(LOG_DEBUG, "ser_read_thread awoken");
				}
				buf->input_sleeping = 0; /* unset sleeping flag - must be inside lock*/
				LOGMUX(LOG_WARNING, "Internal re-assembly buffer partly flushed, free space: %d", gsm0710_buffer_free(buf));
				pthread_mutex_unlock(&buf->bufferready_lock);
			}
			LOGMUX(LOG_DEBUG, "Leave, keep watching");
			return 0;
			break;
		}
		default:
			LOGMUX(LOG_WARNING, "Don't know how to handle reading in state %d", serial->state);
			break;
		}
	}
	LOGMUX(LOG_DEBUG, "Leave, stop watching");
	return 1;
}

/*
 * Purpose:  Open and initialize the serial device used.
 * Input:    serial - the serial struct
 * Return:   0 if port successfully opened, else 1.
 */
int open_serial_device(Serial *serial)
{
	LOGMUX(LOG_DEBUG, "Enter");
	unsigned int i;
	for (i = 0; i < GSM0710_MAX_CHANNELS; i++)
		SYSCHECK(logical_channel_init(channellist + i, i));

	/* open the serial port */
#if (defined(MUX_ANDROID) && !defined(REVERSE_MTK_CHANGE))
	serial->is_socket = 0;
	if (!strcmp(serial->devicename, "/dev/socket/qemud")) {
		/* Qemu-specific control socket */
		SYSCHECK(serial->fd = socket_local_client("qemud", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM));
		if (serial->fd >= 0) {
			char answer[2];

			if (write(serial->fd, "gsm", 3) != 3 ||
			    read(serial->fd, answer, 2) != 2 ||
			    memcmp(answer, "OK", 2) != 0) 
			{
				close(serial->fd);
				LOGMUX(LOG_DEBUG, "open_serial_device close(serial->fd);");
				serial->fd = -1;
				LOGMUX(LOG_ERR, "serial closed!");
			}
		}
		LOGMUX(LOG_INFO, "Opened qemud socket");
		serial->is_socket = 1;
	} else {
		/* Comment by LS: The O_NOCTTY flag tells UNIX that this program doesn't want to be the "controlling terminal" for that port */
		/* If you don't specify this then any input (such as keyboard abort signals and so forth) will affect your process */
		/* See: http://www.faqs.org/docs/Linux-HOWTO/Serial-Programming-HOWTO.html */
		SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));
		LOGMUX(LOG_DEBUG, "open_serial_device SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));");

		if (serial->fd < 0) {
			/* There is no such device! Try to handle it as socket */
			SYSCHECK(serial->fd = socket_local_client(serial->devicename, ANDROID_SOCKET_NAMESPACE_FILESYSTEM, SOCK_STREAM));
			LOGMUX(LOG_INFO, "Opened socket");
			serial->is_socket = 1;
		} else {
			LOGMUX(LOG_INFO, "Opened serial port,serial->fd=%d",serial->fd);
			int fdflags;
			SYSCHECK(fdflags = fcntl(serial->fd, F_GETFL));
			/* Note by LS: Why does it open the serial device with NON-BLOCK in open() then disable this flag via fcntl() later? */
			SYSCHECK(fcntl(serial->fd, F_SETFL, fdflags & ~O_NONBLOCK));

            /* ALPS00574862 */
            #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            struct termios uart_cfg_opt;
            tcgetattr(serial->fd, &uart_cfg_opt);
            tcflush(serial->fd, TCIOFLUSH);
            
        	/*set standard buadrate setting*/
            speed_t speed = baud_bits[cmux_port_speed];
        	cfsetospeed(&uart_cfg_opt, speed);
        	cfsetispeed(&uart_cfg_opt, speed);

        	/* Raw data */
        	uart_cfg_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        	uart_cfg_opt.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
        	uart_cfg_opt.c_oflag &=~(INLCR|IGNCR|ICRNL);
        	uart_cfg_opt.c_oflag &=~(ONLCR|OCRNL);

        	uart_cfg_opt.c_cflag &= ~CRTSCTS;                /*clear flags for hardware flow control*/

        	////uart_cfg_opt.c_cflag |= CRTSCTS; /* Enable HW flow control */
        	uart_cfg_opt.c_iflag &= ~(IXON | IXOFF | IXANY); /*clear flags for software flow control*/        

            // Set time out
            uart_cfg_opt.c_cc[VMIN] = 1;
            uart_cfg_opt.c_cc[VTIME] = 0;
            
            /* Apply new settings */
            SYSCHECK(tcsetattr(serial->fd, TCSANOW, &uart_cfg_opt));
            #endif

            #if 0
			struct termios t;
			tcgetattr(serial->fd, &t);
			t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
			t.c_cflag |= CREAD | CLOCAL | CS8;
			t.c_cflag &= ~(CRTSCTS);
			t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
			t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
			t.c_iflag &= ~(IXON | IXOFF);
			t.c_oflag &= ~(OPOST | OCRNL);
			t.c_cc[VMIN] = 1;
			t.c_cc[VTIME] = 0;

			//Android does not directly define _POSIX_VDISABLE. It can be fetched using pathconf()
			long posix_vdisable;
			char cur_path[FILENAME_MAX];
			if (!getcwd(cur_path, sizeof(cur_path))) {
				LOGMUX(LOG_ERR, "_getcwd returned errno %d", errno);
				return 1;
			}
			posix_vdisable = pathconf(cur_path, _PC_VDISABLE);
			t.c_cc[VINTR] = posix_vdisable;
			t.c_cc[VQUIT] = posix_vdisable;
			t.c_cc[VSTART] = posix_vdisable;
			t.c_cc[VSTOP] = posix_vdisable;
			t.c_cc[VSUSP] = posix_vdisable;

			speed_t speed = baud_bits[cmux_port_speed];
			cfsetispeed(&t, speed);
			cfsetospeed(&t, speed);
			SYSCHECK(tcsetattr(serial->fd, TCSANOW, &t));
			int status = TIOCM_DTR | TIOCM_RTS;
			ioctl(serial->fd, TIOCMBIS, &status);
            #endif
			LOGMUX(LOG_INFO, "Configured serial device");
		}
	}
#else
	SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));
	LOGMUX(LOG_DEBUG, "open_serial_device 1 SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));");

	LOGMUX(LOG_INFO, "Opened serial port");
	int fdflags;
	SYSCHECK(fdflags = fcntl(serial->fd, F_GETFL));
	SYSCHECK(fcntl(serial->fd, F_SETFL, fdflags & ~O_NONBLOCK));
	struct termios t;
	tcgetattr(serial->fd, &t);
	t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
	t.c_cflag |= CREAD | CLOCAL | CS8;
	t.c_cflag &= ~(CRTSCTS);
	t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
	t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
	t.c_iflag &= ~(IXON | IXOFF);
	t.c_oflag &= ~(OPOST | OCRNL);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

#ifdef  MUX_ANDROID
	//Android does not directly define _POSIX_VDISABLE. It can be fetched using pathconf()
	long posix_vdisable;
	char cur_path[FILENAME_MAX];
	if (!getcwd(cur_path, sizeof(cur_path))) {
		LOGMUX(LOG_ERR, "_getcwd returned errno %d", errno);
		return GSM0710MUXD_GET_CWD_ERR;
	}
	posix_vdisable = pathconf(cur_path, _PC_VDISABLE);
	t.c_cc[VINTR] = posix_vdisable;
	t.c_cc[VQUIT] = posix_vdisable;
	t.c_cc[VSTART] = posix_vdisable;
	t.c_cc[VSTOP] = posix_vdisable;
	t.c_cc[VSUSP] = posix_vdisable;
#else   /* MUX_ANDROID */
	t.c_cc[VINTR] = _POSIX_VDISABLE;
	t.c_cc[VQUIT] = _POSIX_VDISABLE;
	t.c_cc[VSTART] = _POSIX_VDISABLE;
	t.c_cc[VSTOP] = _POSIX_VDISABLE;
	t.c_cc[VSUSP] = _POSIX_VDISABLE;
#endif  /* MUX_ANDROID */

	speed_t speed = baud_bits[cmux_port_speed];
	cfsetispeed(&t, speed);
	cfsetospeed(&t, speed);
	SYSCHECK(tcsetattr(serial->fd, TCSANOW, &t));
	int status = TIOCM_DTR | TIOCM_RTS;
	ioctl(serial->fd, TIOCMBIS, &status);
	LOGMUX(LOG_INFO, "Configured serial device");
#endif  /* REVERSE_MTK_CHANGE */

	serial->ping_number = 0;
	time(&serial->frame_receive_time); //get the current time
	serial->state = MUX_STATE_INITILIZING;
	LOGMUX(LOG_DEBUG, "Switched Mux state to %d ", serial->state);
	return 0;
}

/*
 * Purpose:  Initialize mux connection with modem.
 * Input:    serial - the serial struct
 * Return:   0
 */
int start_muxer(Serial *serial)
{
	LOGMUX(LOG_INFO, "Configuring modem");
	char gsm_command[100];
	//check if communication with modem is online
	/* job000291 wait for 10 secs for first +EIND: 128 to start AT+CMUX */
	/* The timeout will affect the bootup time on Rachael35 because the first +EIND: 128 will always lost. */
#ifdef POLL_MODEM_LONGER
    if (poll_modem_ready(serial->fd, 30) < 0)
#else
	if (poll_modem_ready(serial->fd, 10) < 0)
#endif
		LOGMUX(LOG_WARNING, "Wait +EIND: 128 timeout! Continue...");

	if (chat(serial->fd, "AT", 1) < 0) {
		LOGMUX(LOG_WARNING, "Modem does not respond to AT commands, trying close mux mode");
        #if 0
		/* [Bug] Note by LS: But the mux is not opened yet, SABM is sent after start_muxer and creating threads successfully */
		/* It seems to modify the codes here */
		/* [Bug] */
		//if (cmux_mode) we do not know now so write both
		write_frame(0, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR);
		//else
		write_frame(0, close_channel_cmd, 2, GSM0710_TYPE_UIH);
        #endif
		SYSCHECK(chat(serial->fd, "AT", 1));
	}

	SYSCHECK(chat(serial->fd, "ATZ", 3));
	SYSCHECK(chat(serial->fd, "ATE0", 1));
    
    #if 0
	if (0) { // additional siemens c35 init
		SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+IPR=%d", baud_rates[cmux_port_speed]));
		SYSCHECK(chat(serial->fd, gsm_command, 1));
		SYSCHECK(chat(serial->fd, "AT", 1));
		SYSCHECK(chat(serial->fd, "AT&S0", 1));
		SYSCHECK(chat(serial->fd, "AT\\Q3", 1));
	}
    #endif

	if (pin_code >= 0) {
		LOGMUX(LOG_DEBUG, "send pin %04d", pin_code);
		//Some modems, such as webbox, will sometimes hang if SIM code is given in virtual channel
		SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CPIN=%04d", pin_code));
		SYSCHECK(chat(serial->fd, gsm_command, 10));
	}

	if (cmux_mode) {
		SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CMUX=1"));
	} else {
		/* Basic mode */
		SYSCHECK(snprintf(gsm_command, sizeof(gsm_command), "AT+CMUX=%d,%d,%d,%d", cmux_mode, cmux_subset, cmux_port_speed, cmux_N1));
	}
	LOGMUX(LOG_INFO, "Starting mux mode");
	SYSCHECK(chat(serial->fd, gsm_command, 3));
	serial->state = MUX_STATE_MUXING;
	LOGMUX(LOG_DEBUG, "Switched Mux state to %d ", serial->state);
	LOGMUX(LOG_INFO, "Waiting for mux-mode");
	sleep(1);
    #if 0
	LOGMUX(LOG_INFO, "Init control channel");
    #endif
	return 0;
}

/*
 * Purpose:  Close all devices, send mux termination frames (Close Down Procedure is triggered by AP side)
 * Input:    -
 * Return:   0
 */
static int close_devices()
{
	LOGMUX(LOG_INFO, "Enter,GSM0710_MAX_CHANNELS=%d,serial.fd=%d",GSM0710_MAX_CHANNELS,serial.fd);
	int i, num_disc_sent = 0;

	pthread_mutex_lock(&ua_rsp_for_disc_lock);
	/* close_down_send_disc_for_all_chnl should be wrapped by mutex and unlock this mutex after all DISC comds are sent! */
	close_down_send_disc_for_all_chnl = 1;
	for (i = 1; i < GSM0710_MAX_CHANNELS; i++) {
		//terminate command given. Close all non-control channels one by one and finaly close the control channel(i.e., channel num#0)
		//the mux mode		
		if (channellist[i].fd >= 0) {
			if (channellist[i].opened) {
				LOGMUX(LOG_INFO, "Closing down the logical channel %d", i);
				if (cmux_mode) {
					write_frame(i, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR); /* advance mode */
				} else {
					/* [Bug] Note by LS: It should use DISC frame to disconnect each non-control channel first */
#if 0
					write_frame(i, close_channel_cmd, 2, GSM0710_TYPE_UIH); /* basic mode */
#endif
					/* [Bug-add this line] channellist[i].disc_ua_pending = 1; */
					channellist[i].disc_ua_pending = 1;
					num_disc_sent++;
					write_frame(i, NULL, 0, GSM0710_TYPE_DISC | GSM0710_PF);
					/* In logical_channel_close(): it will set channel->opened = 0; In extract_frames():
					 * After receiving the UA rsp frame of this DISC, it will invoke the logical_channel_close() in extract_frames() to handle this UA
					 */
				}
#if 0
				SYSCHECK(logical_channel_close(channellist + i));
#endif
			}
			LOGMUX(LOG_INFO, "Send DISC cmd for the Logical channel %d", channellist[i].id);
		}
	}

	if (serial.fd >= 0) {
		/* Note by LS: It should wait for all UA for each non-contol channel are received then send CLD on contorl channel */
		if (num_disc_sent > 0) {
			int rc = 0;
			struct timespec to;
			/* Block to wait for the other thread's signal of all UA Rsp received */

			/* Note by LS: Set the timeout value as ACK_T1_TIMEOUT seconds:
			 * But it must get the current time first then add ACK_T1_TIMEOUT seconds to obtain the absolute time in the future
			 */
			to.tv_sec = time(NULL) + ACK_T1_TIMEOUT;
			to.tv_nsec = 0;

			/* Because the peer side may not send UA Rsp for each DISC: it should set a timer to guarantee to send the CLD command */
			/* Note by LS: 3rd parm of the pthread_cond_timedwait(): It will be the absolute time in the future */
			/* It still wakes up from the blocking when the timer is expired or signal by the other thread */

			rc = pthread_cond_timedwait(&ua_rsp_for_disc_signal, &ua_rsp_for_disc_lock, &to);
			close_down_send_disc_for_all_chnl = 0;
			if (rc == ETIMEDOUT)
				LOGMUX(LOG_DEBUG, "Fail to wait for all UA Rsp for DISC");
		}
		pthread_mutex_unlock(&ua_rsp_for_disc_lock);

		if (cmux_mode)
			write_frame(0, NULL, 0, GSM0710_CONTROL_CLD | GSM0710_CR);
		else
			/* Note by LS: Should this CLD cmd be sent after receiving all UA resposes for each DLC from the peer device? */
			write_frame(0, close_channel_cmd, 2, GSM0710_TYPE_UIH);  /* basic mode */

		/* Note by LS: Does modem support the AT@POFF ? Ans: NO */
#if 0
		/* Note by LS: If the CLD cmd is sent by modem side: Its MUX will be reset correctly */
		/* AP side is not necessary to send AT+EPOFF to the modem side! */
		static const char *poff = "AT@POFF\r\n";
		syslogdump(">s ", (unsigned char *)poff, strlen(poff));
		write(serial.fd, poff, strlen(poff));
#endif
		SYSCHECK(close(serial.fd));
		LOGMUX(LOG_INFO, "close_devices SYSCHECK(close(serial.fd));;");

		serial.fd = -1;
	} else {
		pthread_mutex_unlock(&ua_rsp_for_disc_lock);
	}
	serial.state = MUX_STATE_OFF;
	/* Note by LS: If it stops the rild process by property_set("ctl.stop","ril-daemon"), it should also set rild_started as 0 */
#if 0
	property_set("ctl.stop", "ril-daemon");
	rild_started = 0;
#endif
	return 0;
}




/*
 * Purpose:  The watchdog state machine restarted every x seconds
 * Input:      serial - the serial struct
 * Return:    1 if error, 0 if success
 */
#ifdef MUX_ANDROID
static int rild_started = 0;
#endif /* MUX_ANDROID */

/* watchdog()
 * Returned-value: Zero-Execution successfully; Non-zero: something wrong!
 */
int watchdog(Serial *serial)
{
	LOGMUX(LOG_DEBUG, "Enter");
	int i;
	/* Comment by LS: serial->state is set as MUX_STATE_OPENING at the initialization in main() */
	LOGMUX(LOG_DEBUG, "Serial state is %d", serial->state);

	pthread_mutex_lock(&watch_lock);

	switch (serial->state) {
	case MUX_STATE_OPENING:
		if (open_serial_device(serial) != 0) {
			LOGMUX(LOG_ERR, "Could not open serial device and start muxer");
			return GSM0710MUXD_OPEN_SERIAL_DEV_ERR;
		}
		/* In open_serial_device(): serial->state will be changed to MUX_STATE_INITILIZING */
		LOGMUX(LOG_INFO, "Watchdog started");
	case MUX_STATE_INITILIZING:
		if (start_muxer(serial) < 0) {
			/* Note by LS: Should it return directly for error handling? */
			LOGMUX(LOG_WARNING, "Could not open all devices and start muxer");
            //BEGIN mtk03923 [ALPS00243691][Daily Use]The 2 cards have no signal
            Gsm0710Muxd_Assert(GSM0710MUXD_START_MUXER_ERR);
            //END   mtk03923 [ALPS00243691][Daily Use]The 2 cards have no signal
			return GSM0710MUXD_START_MUXER_ERR;
		}

		/*If the returned value of start_muxer() is equal to zero: It means that serial->state has been switched to MUX_STATE_MUXING */
		/* Create thread for assemble of frames from data in GSM0710 mux buffer */
		if (create_thread(&frame_assembly_thread, assemble_frame_thread, (void *)serial->in_buf) != 0) {
			LOGMUX(LOG_ERR, "Could not create thread for frame-assmbly");
			return GSM0710MUXD_CREATE_THREAD_ERR;
		}

		/* Create thread for polling on serial device (mux input) and writing to GSM0710 mux buffer */
		Poll_Thread_Arg *poll_thread_arg = (Poll_Thread_Arg *)malloc(sizeof(Poll_Thread_Arg)); //iniitialize pointer to thread args ;
		poll_thread_arg->fd = serial->fd;
		poll_thread_arg->read_function_ptr = &thread_serial_device_read;
		poll_thread_arg->read_function_arg = (void *)serial;
		if (create_thread(&ser_read_thread, poll_thread_serial, (void *)poll_thread_arg) != 0) { //create thread for reading input from serial device
			LOGMUX(LOG_ERR, "Could not create thread for listening on %s", serial->devicename);
			return GSM0710MUXD_CREATE_THREAD_ERR;
		}
		LOGMUX(LOG_DEBUG, "Thread is running and listening on %s", serial->devicename); //listening on serial port

#ifdef __MUX_UT__
		/* Create thread for assemble of frames from data in GSM0710 mux buffer */
		if (create_thread(&ut_frame_assembly_thread, assemble_frame_thread, (void *)serial->ut_in_buf) != 0) {
			LOGMUX(LOG_ERR, "Could not create thread for ut frame-assmbly");
			return GSM0710MUXD_CREATE_THREAD_ERR;
		}

		/* Create thread for polling on serial device (mux input) and writing to GSM0710 mux buffer */
		Poll_Thread_Arg *ut_poll_thread_arg = (Poll_Thread_Arg *)malloc(sizeof(Poll_Thread_Arg)); //iniitialize pointer to thread args ;
		ut_poll_thread_arg->read_function_ptr = &ut_thread_serial_device_read;
		ut_poll_thread_arg->read_function_arg = (void *)serial;
		if (create_thread(&ut_ser_read_thread, ut_poll_thread_serial, (void *)ut_poll_thread_arg) != 0) { //create thread for reading input from serial device
			LOGMUX(LOG_ERR, "Could not create ut simulate thread for listening on %s", "ut_simulated_serail_device");
			return GSM0710MUXD_CREATE_THREAD_ERR;
		}
#endif  /* __MUX_UT__ */
		// Establish CCH
		LOGMUX(LOG_NOTICE, "Init control channel");
		channellist[0].sabm_ua_pending = 1;
		write_frame(0, NULL, 0, GSM0710_TYPE_SABM | GSM0710_PF); //need to move? messy

		//tempary solution. call to allocate virtual port(s)
		int rc = 0;
		for (i = 1; i <= vir_ports; i++) {
			LOGMUX(LOG_ERR, "Allocating logical channel %d/%d ", i, vir_ports);
			if ((rc = c_alloc_channel()) > 0)
				return rc;
			//sleep(1);
		}

#ifdef MUX_ANDROID
		pthread_mutex_lock(&setup_chnl_complete_lock);
		if (mux_setup_chnl_complete == 0) {
			LOGMUX(LOG_DEBUG, "Wait for Channel Setup Procedure to finish");
			pthread_cond_wait(&setup_chnl_complete_signal, &setup_chnl_complete_lock);
			mux_setup_chnl_complete = 0;
			LOGMUX(LOG_NOTICE, "Main Thread is notified Channel Setup Procedure Complete");
		} else {
			mux_setup_chnl_complete = 0;
			LOGMUX(LOG_WARNING, "Channel Setup Procedure is already completed");
		}
		pthread_mutex_unlock(&setup_chnl_complete_lock);

		if (rild_started == 0) {
			/* In this case, serial->state has been switched to MUX_STATE_MUXING when returning from start_muxer() successfully */
			rild_started++;
#ifdef MTK_RIL_MD2
			property_set("ctl.start", "ril-daemon-md2");
			LOGMUX(LOG_ERR, "ril-daemon-md2 started!");
#else
            property_set("ctl.start", "ril-daemon");
			LOGMUX(LOG_ERR, "ril-daemon started!");
#endif
            prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);
            setuid(AID_RADIO);			
            LOGMUX(LOG_ERR, "muxd switch to user radio");	
		}
#endif
		break;

	case MUX_STATE_MUXING:
		/* Re-establish previously closed logical channel and pseudo terminal */
		/* Comment by LS: If something wrong happens in pseudo_device_read()[e.g., read failure]: pts_reopen will be set as 1 */
		if (pts_reopen == 1) {
			for (i = 1; i < GSM0710_MAX_CHANNELS; i++) {
				if (channellist[i].reopen == 1) {
					if (setup_pty_interface(&channellist[i]) != 0) {
						//Exit main function if channel couldn't be allocated
						Gsm0710Muxd_Assert(GSM0710MUXD_RESTART_PTY_ERR);
					} else {
						LOGMUX(LOG_INFO, "Restarted thread listening on %s", channellist[i].ptsname);

						channellist[i].reopen = 0;
						pthread_mutex_lock(&pts_reopen_lock);
						pts_reopen = 0;
						pthread_mutex_unlock(&pts_reopen_lock);
					}
				}
			}
		}
		if (use_ping) {
			if (serial->ping_number > use_ping) {
				LOGMUX(LOG_DEBUG, "no ping reply for %d times, switch to CLOSING state", serial->ping_number);
				serial->state = MUX_STATE_CLOSING;
				LOGMUX(LOG_DEBUG, "Switched Mux state to %d ", serial->state);
			} else {
				LOGMUX(LOG_DEBUG, "Sending PING to the modem");
				//write_frame(0, psc_channel_cmd, sizeof(psc_channel_cmd), GSM0710_TYPE_UI);
				write_frame(0, test_channel_cmd, sizeof(test_channel_cmd), GSM0710_TYPE_UI);
				serial->ping_number++;
			}
		}
		if (use_timeout) {
			time_t current_time;
			time(&current_time); //get the current time
			if (current_time - serial->frame_receive_time > use_timeout) {
				LOGMUX(LOG_DEBUG, "timeout, switch to CLOSING state");
				serial->state = MUX_STATE_CLOSING;
				LOGMUX(LOG_DEBUG, "Switched Mux state to %d ", serial->state);
			}
		}
		break;

	case MUX_STATE_CLOSING:
		/* In close_devices(): it will send the GSM0710_TYPE_DISC to each non-control DLC channel + GSM0710_CONTROL_DISC to control channel; then serial->state will be set as MUX_STATE_OFF */
		close_devices();
		reset_cntx_and_terminate_threads();
		/* [Q] by LS: Does the modem side send CLD command to AP side ? Ans: NO */
		/* Roll back the serial->state to MUX_STATE_OPENING as it sets in the main() */
		serial->state = MUX_STATE_OPENING;
		LOGMUX(LOG_DEBUG, "Switched Mux state from CLOSING to %d ", serial->state);
		break;
	/* Add by LS */
	case MUX_STATE_PEER_CLOSING:
		/* In this case: It already received the CLD cmd or DISC on control channel */
		shutdown_devices(SHUTDOWN_DEV_WO_ACTIVE_FINALIZED);
		reset_cntx_and_terminate_threads();
		serial->state = MUX_STATE_OPENING;
		LOGMUX(LOG_DEBUG, "Switched Mux state from PEER_CLOSING to %d ", serial->state);
		break;
	default:
		LOGMUX(LOG_WARNING, "Don't know how to handle state %d", serial->state);
		break;
	}

	pthread_mutex_unlock(&watch_lock);

	return 0;
}

/*
 * Purpose:  shows how to use this program
 * Input:    name - string containing name of program
 * Return:   -1
 */
static int usage(char *_name)
{
	//TODO: Need to be reviewed and rewrited
	fprintf(stderr, "\tUsage: %s [options]\n", _name);
	fprintf(stderr, "Options:\n");
	// process control
	fprintf(stderr, "\t-d: Fork, get a daemon [%s]\n", no_daemon ? "no" : "yes");
	fprintf(stderr, "\t-v: Set verbose logging level. 0 (Silent) - 7 (Debug) [%d]\n", syslog_level);
	// modem control
	fprintf(stderr, "\t-s <serial port name>: Serial port device to connect to [%s]\n", serial.devicename);
	fprintf(stderr, "\t-t <timeout>: reset modem after this number of seconds of silence [%d]\n", use_timeout);
	fprintf(stderr, "\t-P <pin-code>: PIN code to unlock SIM [%d]\n", pin_code);
	fprintf(stderr, "\t-p <number>: use ping and reset modem after this number of unanswered pings [%d]\n", use_ping);
	// legacy - will be removed
	fprintf(stderr, "\t-b <baudrate>: mode baudrate [%d]\n", baud_rates[cmux_port_speed]);
	fprintf(stderr, "\t-m <modem>: Mode (basic, advanced) [%s]\n", cmux_mode ? "advanced" : "basic");
	fprintf(stderr, "\t-f <framsize>: Frame size [%d]\n", cmux_N1);
	fprintf(stderr, "\t-n <number of ports>: Number of virtual ports to create, must be in range 1-31 [%d]\n", vir_ports);
	fprintf(stderr, "\t-o <output log to file>: Output log to /tmp/gsm0710muxd.log [%s]\n", logtofile ? "yes" : "no");
	fprintf(stderr, "\t-h: Show this help message and show current settings.\n");
	return -1;
}

/*
 * Purpose:  function to set global flag main_exit_signal
 * Input:      signal - ID number of signal
 * Return:    -
 */
void set_main_exit_signal(int signal)
{
	//new lock
	pthread_mutex_lock(&main_exit_signal_lock);
	LOGMUX(LOG_ERR, "Current main_exit_signal=%d", main_exit_signal);
	if (main_exit_signal == SIGTERM && signal == SIGUSR2) {
		LOGMUX(LOG_ERR, "Recv SIGTERM first,ignore SIGUSR2");
		/* To avoid the main proccess exit while-loop due to SIGTERM but executes force assert due to invalid flag */
#ifdef __PRODUCTION_RELEASE__
		g_set_force_assert_flag = 0;
#endif
	} else {
		main_exit_signal = signal;
	}
	pthread_mutex_unlock(&main_exit_signal_lock);
}

/*
 * Purpose:  Creates a detached thread. also checks for errors on exit.
 * Input:      thread_id - pointer to pthread_t id
 *                thread_function - void pointer to thread function
 *                thread_function_arg - void pointer to thread function args
 * Return:    0 if success, 1 if fail
 */
int create_thread(pthread_t *   thread_id,
                  void *        thread_function,
                  void *        thread_function_arg)
{
	LOGMUX(LOG_DEBUG, "Enter");
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg) != 0) {
		switch (errno) {
		case EAGAIN:
			LOGMUX(LOG_ERR, "Interrupt signal EAGAIN caught");
			break;
		case EINVAL:
			LOGMUX(LOG_ERR, "Interrupt signal EINVAL caught");
			break;
		default:
			LOGMUX(LOG_ERR, "Unknown interrupt signal caught");
		}
		LOGMUX(LOG_ERR, "Could not create thread");
		//set_main_exit_signal(1); //exit main function if thread couldn't be created
		Gsm0710Muxd_Assert(4);
		return 1;
	}
	pthread_attr_destroy(&thread_attr);     /* Not strictly necessary */
	return 0;                               //thread created successfully
}

/*
 * Purpose:  Poll a device (file descriptor) using select()
 *           if select returns data to be read. call a reading function for the particular device
 * Input:    vargp - a pointer to a Poll_Thread_Arg struct.
 * Return:   NULL if error
 */
void *poll_thread(void *vargp)
{
	LOGMUX(LOG_DEBUG, "Enter");
	Poll_Thread_Arg *poll_thread_arg = (Poll_Thread_Arg *)vargp;
	if (poll_thread_arg->fd == -1) {
		LOGMUX(LOG_ERR, "Serial port not initialized");
		goto terminate;
	}
	/* Note by LS: In pseudo_device_read(): It may have remaining data due to previous write_frame() */
	/* If the application does not write data anymore, these remaining data will not be sent due to blocking in select() */
	while (1) {
		fd_set fdr, fdw;
		FD_ZERO(&fdr);
		FD_ZERO(&fdw);
		/* Monitor the poll_thread_arg->fd in read set */
		FD_SET(poll_thread_arg->fd, &fdr);
		if (select((poll_thread_arg->fd) + 1, &fdr, &fdw, NULL, NULL) > 0) {
			LOGMUX(LOG_INFO, "poll_thread select found");
			/* Return from the select(): it means that at least one fd is ready for I/O */
			if (FD_ISSET(poll_thread_arg->fd, &fdr)) {
				/*Call reading function*/
				if ((*(poll_thread_arg->read_function_ptr))(poll_thread_arg->read_function_arg) != 0) {
					LOGMUX(LOG_WARNING, "Device read function returned error");
					goto terminate;
				}
			}
		} else { //No need to evaluate retval=0 case, since no use of timeout in select()
			switch (errno) {
			case EINTR:
				LOGMUX(LOG_ERR, "Interrupt signal EINTR caught");
				break;
			case EAGAIN:
				LOGMUX(LOG_ERR, "Interrupt signal EAGAIN caught");
				break;
            case EBADF:
                if(stop_muxd == 1){
                    LOGMUX(LOG_ERR, "Interrupt signal EBADF caught because devices is already shutdown by self");
                    goto terminate;
                } else {
                    LOGMUX(LOG_ERR, "Interrupt signal EBADF caught");
                }
            #if 0
			case EINVAL:
				LOGMUX(LOG_ERR, "Interrupt signal EINVAL caught");
				break;
            #endif
			default:
				LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught\n", errno);
				Gsm0710Muxd_Assert(GSM0710MUXD_TXTHREAD_SELECT_ERR);
			}
		}
	}
	goto terminate;

terminate:
	LOGMUX(LOG_ERR, "Device polling thread terminated");
	watchdog(&serial);
	free(poll_thread_arg); //free the memory allocated for the thread args before exiting thread
	return NULL;
}

/*
 * Purpose:  The main program loop
 * Input:    argc - number of input arguments
 *           argv - array of strings (input arguments)
 * Return:   0
 */
int main(int argc, char *argv[])
{
	LOGMUX(LOG_ERR, "Enter");
	int opt, rc = -1;
	char dev_node[32] = {0};

	char prop_value[PROPERTY_VALUE_MAX] = { 0 };
	property_get("mux.debuglog.enable", prop_value, NULL);
	if (prop_value[0] == '1') {
	    LOGMUX(LOG_INFO, "Enable full log");
            syslog_level = LOG_DEBUG;
	}	
	if (MTK_ENABLE_MD1) {
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA, MD_SYS1));
    LOGMUX(LOG_ERR, "ccci_get_node_name=%s", dev_node);
  } else if (MTK_ENABLE_MD2) {
  	snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA, MD_SYS2));
  	LOGMUX(LOG_ERR, "ccci_get_node_name=%s", dev_node);
  } else if (MTK_ENABLE_MD5) {
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_MUXD_DATA, MD_SYS5));
    LOGMUX(LOG_ERR, "ccci_get_node_name=%s", dev_node);
  } else {
  	snprintf(dev_node, 32, "%s", "/dev/ttyUSB1");
    LOGMUX(LOG_ERR, "ccci_get_node_name unknown, default tablet=%s", dev_node);    
  }
  
	serial.devicename = dev_node;
	while ((opt = getopt(argc, argv, "dov:s:t:p:f:n:h?m:b:P:")) > 0) {
		switch (opt) {
		case 'v':
			syslog_level = atoi(optarg);
			if ((syslog_level > LOG_DEBUG) || (syslog_level < 0)) {
				usage(argv[0]);
				exit(0);
#ifdef MUX_ANDROID
				//syslog_level=android_log_lvl_convert[syslog_level];
#endif
			}
			break;
		case 'o':
			logtofile = 1;
			if ((muxlogfile = fopen("/tmp/gsm0710muxd.log", "w+")) == NULL) {
				fprintf(stderr, "Error: %s.\n", strerror(errno));
				usage(argv[0]);
				exit(0);
			} else {
				fprintf(stderr, "gsm0710muxd log is output to /tmp/gsm0710muxd.log\n");
			}
			break;
		case 'd':
			no_daemon = !no_daemon;
			break;
		case 's':
			break;
		case 't':
			use_timeout = atoi(optarg);
			break;
		case 'p':
			use_ping = atoi(optarg);
			break;
		case 'P':
			pin_code = atoi(optarg);
			break;
		// will be removed if +CMUX? works
		case 'f':
			cmux_N1 = atoi(optarg);
			break;
		case 'n':
			vir_ports = atoi(optarg);
			vir_ports = MUXD_CH_NUM_ALL - 1;

			// Validate vir_prorts
			if ((vir_ports > GSM0710_MAX_CHANNELS - 1) || (vir_ports < 1)) {
				LOGMUX(LOG_ERR, "Cannot allocate %d virtual ports", vir_ports);
				usage(argv[0]);
				exit(0);
			}
			break;
		case 'm':
			if (!strcmp(optarg, "basic"))
				cmux_mode = 0;
			else if (!strcmp(optarg, "advanced"))
				cmux_mode = 1;
			else
				cmux_mode = 0;
			break;
		case 'b':
			cmux_port_speed = baud_rate_index(atoi(optarg));
			break;
		default:
		case '?':
		case 'h':
			usage(argv[0]);
			exit(0);
			break;
		}
	}

	/* unmask is used to obtain the correct permission of a newly created file or directory */
	/* But the newly created file can't have the execution permission but directory can have it */
	/* For example: If the unmask value is 022(unmask is presented as ): the newly-created file's permission is (~022)=664; directory's permission is (~022)=775 */
	umask(0);
	//signals treatment
	signal(SIGHUP, signal_treatment);
	signal(SIGPIPE, signal_treatment);
	signal(SIGKILL, signal_treatment);
	signal(SIGINT, signal_treatment);
	signal(SIGUSR1, signal_treatment);
	signal(SIGUSR2, signal_treatment);	
	signal(SIGTERM, signal_treatment);
	signal(SIGALRM, signal_treatment);

#ifndef MUX_ANDROID
	if (no_daemon)
		openlog(argv[0], LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_LOCAL0);
	else
		openlog(argv[0], LOG_NDELAY | LOG_PID, LOG_LOCAL0);
#endif
	//allocate memory for data structures
	//adv_frame_buf is not necessary in basic mode
	if ((serial.in_buf = gsm0710_buffer_init()) == NULL ||
	    (cmux_mode == 1 &&
	     (serial.adv_frame_buf = (unsigned char *)malloc((cmux_N1 + 3) * 2 + 2)) == NULL)) {
		LOGMUX(LOG_ERR, "Out of memory");
		exit(-1);
	}

#ifdef __MUX_UT__
	/* Add by LS: Init some properties and serial buffer(i.e., ut_in_buf) used in MUX UT */
	mux_ut_init_properties();

	if ((serial.ut_in_buf = gsm0710_buffer_init()) == NULL) {
		LOGMUX(LOG_ERR, "Out of memory");
		exit(-1);
	}
#endif

	LOGMUX(LOG_ERR, "%s %s starting", *argv, revision);
	//Initialize modem and virtual ports
	serial.state = MUX_STATE_OPENING;

	LOGMUX(LOG_NOTICE, "Called with following options:");
	LOGMUX(LOG_NOTICE, "\t-d: Fork, get a daemon [%s]", no_daemon ? "no" : "yes");
	LOGMUX(LOG_NOTICE, "\t-v: Set verbose logging level. 0 (Silent) - 7 (Debug) [%d]", syslog_level);
	LOGMUX(LOG_NOTICE, "\t-s <serial port name>: Serial port device to connect to [%s]", serial.devicename);
	LOGMUX(LOG_NOTICE, "\t-t <timeout>: reset modem after this number of seconds of silence [%d]", use_timeout);
	LOGMUX(LOG_NOTICE, "\t-P <pin-code>: PIN code to unlock SIM [%d]", pin_code);
	LOGMUX(LOG_NOTICE, "\t-p <number>: use ping and reset modem after this number of unanswered pings [%d]", use_ping);
	LOGMUX(LOG_NOTICE, "\t-b <baudrate>: mode baudrate [%d]", baud_rates[cmux_port_speed]);
	LOGMUX(LOG_NOTICE, "\t-m <modem>: Mode (basic, advanced) [%s]", cmux_mode ? "advanced" : "basic");
	LOGMUX(LOG_NOTICE, "\t-f <framsize>: Frame size [%d]", cmux_N1);
	LOGMUX(LOG_NOTICE, "\t-n <number of ports>: Number of virtual ports to create, must be in range 1-31 [%d]", vir_ports);
	LOGMUX(LOG_NOTICE, "\t-o <output log to file>: Output log to /tmp/gsm0710muxd.log [%s]", logtofile ? "yes" : "no");

	/* main_exit_signal is set via the function set_main_exit_signal() */
	while ((main_exit_signal == 0) && ((rc = watchdog(&serial)) == 0)) {
		/* Monitor and print statistics */
		//LOGMUX(LOG_INFO, "GSM0710 buffer. Stored %d", gsm0710_buffer_length(serial.in_buf));
		LOGMUX(LOG_INFO, "Frames received/dropped: %lu/%lu", serial.in_buf->received_count, serial.in_buf->dropped_count);

		sleep(5);
        
#ifdef  MUX_ANDROID
#ifdef  __MUX_REPORT_IT__
		/* Add codes for test force assert functionality by LS */
		{
			char property_value[2] = { 0 };
			property_get("mux.force.assert.flag", property_value, NULL);
			if (prop_value[0] == 'Y') {
				LOGMUX(LOG_ERR, "Detect Muxd Force Assert Flag as Y");
				property_set("mux.force.assert.flag", "N");
				Gsm0710Muxd_Assert(99);
			}
		}
#endif  /* __MUX_REPORT_IT__ */
#endif  /* MUX_ANDROID */
	}
	//finalize everything: serial.in_buf and serial_adv_frame_buf are allocated previously - It's time to free
	/* Note by LS: If the main process exists the while-loop, it means that something is wrong */
	/* Two types of error causes returned from watchdog(): GSM0710MUXD_OPEN_SERIAL_DEV_ERR and GSM0710MUXD_CREATE_THREAD_ERR */
	if (main_exit_signal != 0) {
		/* MUXd receives the abnormal termination signal, it should finalize its context */
		SYSCHECK(shutdown_devices(SHUTDOWN_DEV_W_ACTIVE_FINALIZED));
	} else if (rc == GSM0710MUXD_OPEN_SERIAL_DEV_ERR || rc == GSM0710MUXD_CREATE_THREAD_ERR) {
		/* SHUTDOWN_DEV_WO_ACTIVE_FINALIZED: It will send a CLD Rsp to the modem side */
		SYSCHECK(shutdown_devices(SHUTDOWN_DEV_W_ACTIVE_FINALIZED));
	} else {
		/* Returned value from the watchdog() is not equal to zero due to  */
		/* It should return the error code from the watchdog exactly - Because some error is failed to oper serial device, some is failed to create the thread */
		/* Not all MUX's state is necessary to invoke the close_devices() */
		/* [Bug] by LS: Need to be fixed */
		//SYSCHECK(close_devices());
	}

	if (cmux_mode) free(serial.adv_frame_buf);
	gsm0710_buffer_destroy(serial.in_buf);
	LOGMUX(LOG_INFO, "Received %ld frames and dropped %ld received frames during the mux-mode", serial.in_buf->received_count, serial.in_buf->dropped_count);
	//LOGMUX(LOG_DEBUG, "%s finished", argv[0]);
	LOGMUX(LOG_ERR, "Terminating Gsm0710Muxd process ...");
#ifndef MUX_ANDROID
	closelog(); // close syslog
#endif  /* MUX_ANDROID */

	if (logtofile)
		fclose(muxlogfile);

#ifdef MUX_ANDROID
#ifdef __PRODUCTION_RELEASE__
	if (g_set_force_assert_flag == 1) {
		/* It means that this main_exit_signal triggered by Gsm0710Muxd_Assert() instead of recv a signal actually */
		/* Plug-in to report abnormal case1 by Android property_set() to launch another process */
#ifdef MTK_RIL_MD2
        property_set("mux.report.case", "5");
#else		
		property_set("mux.report.case", "1");
#endif
		property_set("ctl.start", "muxreport-daemon");
		/* Set a 5 seconds alarm to check if the force assert for logging procedure is executed or not */
		g_set_alarm_flag = 1;
		alarm(5);
		LOGMUX(LOG_ERR, "return from alarm() then  wait for signal to wake up from pause()");
		/* Pause this process until it receives a signal */
		pause();
	}
#endif  /* __PRODUCTION_RELEASE__ */
#endif  /* MUX_ANDROID */

	LOGMUX(LOG_ERR, "%s finished", argv[0]);
	return 0;
}

/******************************************************************************/
