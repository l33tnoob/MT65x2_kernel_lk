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

#ifdef __MUXD_FLOWCONTROL__
/******************************************************************************/

#include <errno.h>
#include <fcntl.h>
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

/******************************************************************************/

#define TCOOFF  0
#define TCOON   1
#define TCIOFF  2
#define TCION   3

/******************************************************************************/

static inline GSM0710_FrameList *_fl_init(GSM0710_FrameList *list);
static inline void _fl_destroy(GSM0710_FrameList *list);
static inline void _fl_cleanNodes(GSM0710_FrameList *list);
static inline int _fl_isEmpty(GSM0710_FrameList *list);
static inline GSM0710_FrameList *_fl_pushFrame(GSM0710_FrameList *list, GSM0710_Frame *frame);
static inline GSM0710_Frame *_fl_popFrame(GSM0710_FrameList *list);

/******************************************************************************/

void _fc_chokePty(Channel *channel);
void _fc_releasePty(Channel *channel);
void _fc_initContext(Channel *channel);
void _fc_closeContext(Channel *channel);
void _fc_cacheRemainingFrameData(Channel *channel, GSM0710_Frame *frame, int written);
void _fc_cacheFrameData(Channel *channel, GSM0710_Frame *frame);

/******************************************************************************/

extern int watchdog(Serial *serial);
extern void set_main_exit_signal(int signal);
extern int create_thread(pthread_t *thread_id, void *thread_function, void *thread_function_arg);
extern int write_frame(int channel, const unsigned char *input, int length, unsigned char type);
extern void destroy_frame(GSM0710_Frame *frame);

/******************************************************************************/

static inline GSM0710_FrameList *_fl_init(GSM0710_FrameList *list)
{
	if (list == NULL) {
		GSM0710_FrameList *head = (GSM0710_FrameList *)malloc(sizeof(GSM0710_FrameList));
		head->next = head;
		return head;
	} else {
		if (!(_fl_isEmpty(list))) _fl_cleanNodes(list);
		return list;
	}
}

static inline void _fl_destroy(GSM0710_FrameList *list)
{
	if (list != NULL) {
		if (!(_fl_isEmpty(list))) _fl_cleanNodes(list);
		free(list);
	}
}

GSM0710_FrameList *_fl_pushFrame(GSM0710_FrameList *	list,
				 GSM0710_Frame *	frame)
{
	GSM0710_FrameList *head = (GSM0710_FrameList *)malloc(sizeof(GSM0710_FrameList));

	head->next = list->next;
	head->frame = list->frame;

	list->next = head;
	list->frame = frame;

	return head;
}

GSM0710_Frame *_fl_popFrame(GSM0710_FrameList *list)
{
	if (_fl_isEmpty(list)) {
		return NULL;
	} else {
		GSM0710_FrameList *node = list->next;
		GSM0710_Frame *frame = node->frame;

		list->next = list->next->next;
		free(node);

		return frame;
	}
}

static inline void _fl_cleanNodes(GSM0710_FrameList *list)
{
	GSM0710_Frame *frame;

	while (!_fl_isEmpty(list)) {
		frame = _fl_popFrame(list);
		destroy_frame(frame);
	}
}

static inline int _fl_isEmpty(GSM0710_FrameList *list)
{
	if (list == NULL) return 1;
	else return list->next == list;
}

/******************************************************************************/

void _fc_chokePty(Channel *channel)
{
	if (channel->id != MUXD_VT_CH_NUM)
		LOGMUX(LOG_NOTICE, "Chock Pty of Channel:%d", channel->id);

	ioctl(channel->fd, TCXONC, TCIOFF);
}

void _fc_releasePty(Channel *channel)
{
	if (channel->id != MUXD_VT_CH_NUM)
		LOGMUX(LOG_NOTICE, "Release Pty of Channel:%d", channel->id);

	ioctl(channel->fd, TCXONC, TCION);
}

void _fc_initContext(Channel *channel)
{
	channel->tx_fc_off = 0;

	channel->rx_fc_off = 0;
	channel->rx_thread = 0;
	channel->rx_fl = NULL;
	channel->rx_fl_total = 0;
	channel->rx_fl_written = 0;

	/* Add by LS: Add the initialization of type pthread_cond_t by dynamic function call */
	/* If it uses channel->tx_fc_on_signal = PTHREAD_COND_INITIALIZER; Compiler Error happens! */
	pthread_cond_init(&channel->tx_fc_on_signal, NULL);
	pthread_cond_init(&channel->rx_fc_on_signal, NULL);
#if 0
	pthread_cond_init(&channel->rx_fc_off_rsp_signal, NULL);
	pthread_cond_init(&channel->rx_fc_on_req_signal, NULL);
#endif
}

void _fc_closeContext(Channel *channel)
{
	channel->tx_fc_off = 0;

	channel->rx_fc_off = 0;
	channel->rx_thread = 0;
	channel->rx_fl_total = 0;
	channel->rx_fl_written = 0;
	_fl_destroy(channel->rx_fl);
}

void _fc_cacheFrameData(
	Channel *	channel,
	GSM0710_Frame * frame)
{
	LOGMUX(LOG_DEBUG, "Enter");

	/* All data sent from the modem before receiving the MSC with FC OFF Rsp will be inserted into the link list */
	if ((channel->rx_fl_total + frame->length) > RX_FLOW_CTRL_HIGH_WATERMARK) {
		LOGMUX(LOG_DEBUG, "Accumulated_pending_frame_bytes is larger than mark val=%d, drop it",
		       RX_FLOW_CTRL_HIGH_WATERMARK);
		destroy_frame(frame);
		serial.in_buf->dropped_count++;
		//mtk02863
		//Gsm0710Muxd_Assert(19);
	} else {
		channel->rx_fl_total += frame->length;
		channel->rx_fl = _fl_pushFrame(channel->rx_fl, frame);
	}

	LOGMUX(LOG_INFO, "Case2:Frame List=0x%08X, pending_frame_bytes=%d, frame_len=%d",
	       (unsigned int)channel->rx_fl, channel->rx_fl_total, frame->length);
	return;
}

void _fc_cacheRemainingFrameData(
	Channel *	channel,
	GSM0710_Frame * frame,
	int		written)
{
	LOGMUX(LOG_INFO, "Enter");

	if (!_fl_isEmpty(channel->rx_fl))
		Gsm0710Muxd_Assert(GSM0710MUXD_FRAMELIST_INIT_ERR);

	channel->rx_fl = _fl_init(channel->rx_fl);

	channel->rx_fl_total = (frame->length - written);
	channel->rx_fl = _fl_pushFrame(channel->rx_fl, frame);
	//todo
	channel->rx_fl_written = written;

	/* This is 1st node due to pty channel's buffer is full */
	LOGMUX(LOG_INFO, "Case1: FrameList=0x%08X", (unsigned int)channel->rx_fl);
	return;
}

/******************************************************************************/

/*
 * Purpose:
 * Input:
 * Return:
 */
static void *retry_write_pty_thread(
	void *arg)
{
	Channel *channel = (Channel *)arg;
	GSM0710_Frame *frame;
	int write_result = 0, written = 0;
	unsigned char msc_cmd[4];
	unsigned int waiting = 0;
	unsigned long msec = 1000;

	LOGMUX(LOG_DEBUG, "Created:Chnl_Num=%d, Frame list ptr = 0x%08X", channel->id,
	       (unsigned int)channel->rx_fl);

	// Get first frame and retrieve the number of bytes that is already written
	pthread_mutex_lock(&channel->rx_fc_lock);
	channel->rx_thread = 1;
	frame = _fl_popFrame(channel->rx_fl);
	written = channel->rx_fl_written;
	channel->rx_fl_written = 0;
	pthread_mutex_unlock(&channel->rx_fc_lock);

	// First frame doesn't exist, ASSERT
	if (frame == NULL) Gsm0710Muxd_Assert(GSM0710MUXD_RXTHREAD_ERR);

	/* This function is thread execution function: It should runs as a while loop until some error happens!*/
	do {
		while (1) {
			LOGMUX(LOG_DEBUG, "written=%d, frame_ptr=0x%08X, frame_len=%d", written,
			       (unsigned int)frame, frame->length);

			if ((write_result = write(channel->fd, frame->data + written,
						  frame->length - written)) >= 0) {
				LOGMUX(LOG_INFO, "Write %dBytes to PTY Channel:%d", write_result, channel->id);
				//LOGMUX(LOG_DEBUG,"Write PTY data val=0x%02X", *(node->frame_data + already_written_len));
				//syslogdump("<P ", node->frame_data + already_written_len, written_len);
				written += write_result;
				waiting = 0;

				if (written < frame->length) {
					usleep(msec * (++waiting));
				} else {
					written = 0;
					break;
				}
			} else {
				switch (errno) {
				case EINTR:
					LOGMUX(LOG_NOTICE, "Interrupt signal EINTR caught");
					break;
				case EAGAIN:
					LOGMUX(LOG_NOTICE, "Interrupt signal EAGAIN caught");
					usleep(msec * ((waiting >= 10) ? waiting : waiting++));
					break;
				default:
					if (channel->reopen) {
						LOGMUX(LOG_ERR, "channel%d needs to be reopened\n", channel->id);
						watchdog(&serial);
					} else {
						LOGMUX(LOG_ERR, "Unknown interrupt signal errno=%d caught from write()\n"
						       , errno);
						Gsm0710Muxd_Assert(GSM0710MUXD_PTY_WRITE_ERR);
					}
					break;
				}
			}
		}

		pthread_mutex_lock(&channel->rx_fc_lock);
		frame = _fl_popFrame(channel->rx_fl);
		if (frame == NULL) channel->rx_fc_off = 0;
		pthread_mutex_unlock(&channel->rx_fc_lock);
	} while (frame != NULL);

	/* Send MSC with FC On to modem side: Enable RX again! */
	memcpy(msc_cmd, msc_channel_cmd, 4);
	msc_cmd[2] = (msc_cmd[2] | (channel->id << 2));

	/* The default value of FC bit (i.e.,msc_channel_cmd[3]) : Disable the FC Off flag */
#ifndef __MUX_UT__
	write_frame(0, msc_cmd, 4, GSM0710_TYPE_UIH | GSM0710_PF);
#else
	if (channel->id < (MAX_NON_GEMINI_NON_DATA_CHNL_NUM + 1)) {
		write_frame(0, msc_cmd, 4, GSM0710_TYPE_UIH | GSM0710_PF);
	} else {
		/* Channel_Num >= (MAX_NON_GEMINI_NON_DATA_CHNL_NUM+1) are used to UT Test: Its peer is a virtual serail device which will not parse the recv data from AP side */
		/* For UT Chnl: signal the rx_fc_on_req to ut_thread_serial */
#if 0
		pthread_cond_signal(&channel->rx_fc_on_req_signal);
#endif
	}
#endif

	LOGMUX(LOG_DEBUG, "Terminate Retry Write PTY Thread!");

	pthread_mutex_lock(&channel->rx_fc_lock);
	channel->rx_thread = 0;
	pthread_mutex_unlock(&channel->rx_fc_lock);
	pthread_cond_signal(&channel->rx_fc_on_signal);

	return NULL;
}

/*
 * Purpose:
 * Input:
 * Return:
 */
void start_retry_write_thread(
	Channel *channel
	)
{
	unsigned char msc_cmd[4];

	LOGMUX(LOG_DEBUG, "Enter");

	/* Send the MSC cmd with FC Off to the modem side */
	memcpy(msc_cmd, msc_channel_cmd, 4);
	msc_cmd[2] = (msc_cmd[2] | (channel->id << 2));
	msc_cmd[3] = (msc_cmd[3] | 0x02);
	LOGMUX(LOG_INFO, "Not all requested data are written into serial buffer at this time");

#ifndef __MUX_UT__
	write_frame(0, msc_cmd, 4, GSM0710_TYPE_UIH | GSM0710_PF);
#else
	if (channel->id <= MAX_NON_GEMINI_NON_DATA_CHNL_NUM)
		write_frame(0, msc_cmd, 4, GSM0710_TYPE_UIH | GSM0710_PF);
	else
		/* frame->channel > 5 are used as the UT Test Channel */
		set_mux_ut_rx_chnl_fc_flag(channel->id, FC_OFF_SENDING);

#endif

	/* After receving the MSC FC OFF Rsp from the modem, AP side should keep to store the data already sent into CCCI Driver */
	LOGMUX(LOG_INFO, "Set FC_OFF_SENDING and rx_fc_off as 1");
	pthread_mutex_lock(&channel->rx_fc_lock);
	if (channel->rx_thread)
		pthread_cond_wait(&channel->rx_fc_on_signal, &channel->rx_fc_lock);
	channel->rx_fc_off = 1;
	pthread_mutex_unlock(&channel->rx_fc_lock);

	// Start retry thread
	if (create_thread(&channel->push_thread_id, retry_write_pty_thread,
			  (void *)channel) != 0) {
		LOGMUX(LOG_ERR, "Could not create thread retry pty write thread for channel=%d",
		       channel->id);
		return;
	}
}

/******************************************************************************/
#endif /* __MUXD_FLOWCONTROL__ */
