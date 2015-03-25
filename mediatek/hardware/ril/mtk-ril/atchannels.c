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

/* //$(MTK_PATH_SOURCE)/hardware/ril/mtk-ril/atchannels.c
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#include <telephony/ril.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <termios.h>

#define LOG_NDEBUG 0

#ifdef MTK_RIL_MD1
#define LOG_TAG "AT"
#else
#define LOG_TAG "ATMD2"
#endif
#include <utils/Log.h>


#include "misc.h"
#include "atchannels.h"
#include "at_tok.h"


#ifdef MTK_RIL

#ifdef HAVE_ANDROID_OS
#define USE_NP 1
#endif /* HAVE_ANDROID_OS */

#if AT_DEBUG
void AT_DUMP(const char *prefix, const char *buff, int len)
{
	if (len < 0)
		len = strlen(buff);
	LOGD("%.*s", len, buff);
}
#endif  /* AT_DEBUG */

/*
 * for current pending command
 * these are protected by s_commandmutex
 */

static void (*s_onTimeout)(RILChannelCtx *p_channel) = NULL;
static void (*s_onReaderClosed)(RILChannelCtx *p_channel) = NULL;

static void onReaderClosed();
static int writeCtrlZ(const char *s, RILChannelCtx *p_channel);
static int writeline(const char *s, RILChannelCtx *p_channel);

static RILChannelCtx s_RILChannel[RIL_SUPPORT_CHANNELS];

const char *channelIdToString(RILChannelId id);

extern void initRILChannels(void)
{
	RILChannelCtx *channel;

	int i;
	for (i = 0; i < RIL_SUPPORT_CHANNELS; i++) {
		channel = &s_RILChannel[i];
		memset(channel, 0, sizeof(RILChannelCtx));

		channel->fd = -1; /* fd of the AT channel */
		channel->ATBufferCur = channel->ATBuffer;
		channel->myName = channelIdToString(i);
		channel->id = i;
	}
}

#ifndef USE_NP
static void setTimespecRelative(struct timespec *p_ts, long long msec)
{
	struct timeval tv;

	gettimeofday(&tv, (struct timezone *)NULL);

	/* what's really funny about this is that I know
	 * pthread_cond_timedwait just turns around and makes this
	 * a relative time again */
	p_ts->tv_sec = tv.tv_sec + (msec / 1000);
	p_ts->tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L) * 1000L;
}
#endif  /* USE_NP */

void sleepMsec(long long msec)
{
	struct timespec ts;
	int err;

	ts.tv_sec = (msec / 1000);
	ts.tv_nsec = (msec % 1000) * 1000 * 1000;

	do
		err = nanosleep(&ts, &ts);
	while (err < 0 && errno == EINTR);
}

/** add an intermediate response to sp_response*/
static void addIntermediate(const char *line, RILChannelCtx *p_channel)
{
	ATResponse *p_response = p_channel->p_response;

	ATLine *p_new;
	p_new = (ATLine *) malloc(sizeof(ATLine));
	p_new->line = strdup(line);

	/* note: this adds to the head of the list, so the list
	 * will be in reverse order of lines received. the order is flipped
	 * again before passing on to the command issuer */
	p_new->p_next = p_response->p_intermediates;
	p_response->p_intermediates = p_new;
}

/**
 * returns 1 if line is a final response indicating error
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char *s_finalResponsesError[] = {
	"ERROR",
	"+CMS ERROR:",
	"+CME ERROR:",
	"NO CARRIER",   /* sometimes! */
	"NO ANSWER",
	"NO DIALTONE",
};

static int isFinalResponseError(const char *line)
{
	size_t i;
	for (i = 0; i < NUM_ELEMS(s_finalResponsesError); i++) {
		if (strStartsWith(line, s_finalResponsesError[i])) {
			return 1;
        }
	}
	return 0;
}

// customization for mediatek modem
static int isFinalResponseErrorEx(const char *line, RILChannelCtx *p_channel)
{
    size_t i;

#ifdef  MTK_GEMINI
    int j=0;
    for(j=0; j<MTK_GEMINI_SIM_NUM; j++){
        if( (p_channel->id == RIL_URC+j*RIL_CHANNEL_OFFSET) && (strStartsWith(line, "NO CARRIER")) ){
            // [ALPS01225455]NO CARRIER in URC channel is URC, not final response for mediatek modem
            return 0;
        }
    }
#else
    if( (p_channel->id == RIL_URC) && (strStartsWith(line, "NO CARRIER")) ){
        // [ALPS01225455]NO CARRIER in URC channel is URC, not final response for mediatek modem
        return 0;
    }
#endif

    for (i = 0; i < NUM_ELEMS(s_finalResponsesError); i++) {
        if (strStartsWith(line, s_finalResponsesError[i])) {
            return 1;
        }
    }
    return 0;
}

/**
 * returns 1 if line is a final response indicating success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static const char *s_finalResponsesSuccess[] = {
	"OK",
	"CONNECT"   /* some stacks start up data on another channel */
};

static int isFinalResponseSuccess(const char *line)
{
	size_t i;
	for (i = 0; i < NUM_ELEMS(s_finalResponsesSuccess); i++) {
		if (strStartsWith(line, s_finalResponsesSuccess[i])) {
			return 1;
        }
	}
	return 0;
}

/**
 * returns 1 if line is a final response, either  error or success
 * See 27.007 annex B
 * WARNING: NO CARRIER and others are sometimes unsolicited
 */
static int isFinalResponse(const char *line) {
	return isFinalResponseSuccess(line) || isFinalResponseError(line);
}

/**
 * returns 1 if line is the first line in (what will be) a two-line
 * SMS unsolicited response
 */
static const char *s_smsUnsoliciteds[] = {
	"+CMT:",
	"+CDS:",
	"+CBM:"
};

static int isSMSUnsolicited(const char *line)
{
	size_t i;
	for (i = 0; i < NUM_ELEMS(s_smsUnsoliciteds); i++) {
		if (strStartsWith(line, s_smsUnsoliciteds[i])) {
			return 1;
        }
	}
	return 0;
}

/** assumes s_commandmutex is held */
static void handleFinalResponse(const char *line, RILChannelCtx *p_channel)
{
	ATResponse *p_response = p_channel->p_response;
	p_response->finalResponse = strdup(line);

	pthread_cond_signal(&p_channel->commandcond);
}

static void handleUnsolicited(const char *line, RILChannelCtx *p_channel)
{
	if (p_channel->unsolHandler != NULL)
		p_channel->unsolHandler(line, NULL, p_channel);
}

static void processLine(const char *line, RILChannelCtx *p_channel)
{
	ATResponse *p_response = p_channel->p_response;
	const char *smsPDU = p_channel->smsPDU;
	//Move out to the function
	//pthread_mutex_lock(&p_channel->commandmutex);
	int isIntermediateResult = 0;

	if (p_response == NULL) {
		/* no command pending */
		handleUnsolicited(line, p_channel);
		return;
	} else {
		switch (p_channel->type) {
		case NO_RESULT:
			//handleUnsolicited(line,p_channel);
			break;
		case NUMERIC:
			if (p_response->p_intermediates == NULL
			    && isdigit(line[0])
			    ) {
				addIntermediate(line, p_channel);
				isIntermediateResult = 1;
			} else {
				/* either we already have an intermediate response or
				 *     the line doesn't begin with a digit */
				//handleUnsolicited(line,p_channel);
			}
			break;
		case SINGLELINE:
			if (p_response->p_intermediates == NULL
			    && strStartsWith(line, p_channel->responsePrefix)
			    ) {
				addIntermediate(line, p_channel);
				isIntermediateResult = 1;
			} else {
				/* we already have an intermediate response */
				//handleUnsolicited(line,p_channel);
			}
			break;
		case MULTILINE:
			if (strStartsWith(line, p_channel->responsePrefix)) {
				addIntermediate(line, p_channel);
				isIntermediateResult = 1;
			} else {
				//handleUnsolicited(line,p_channel);
			}
			break;
		/* atci start */
		case RAW:
			if (!isFinalResponseSuccess(line) && !isFinalResponseErrorEx(line, p_channel)) {
				addIntermediate(line, p_channel);
				isIntermediateResult = 1;
			}
			break;
		/* atci end */
		default: /* this should never be reached */
			LOGE("Unsupported AT command type %d\n", p_channel->type);
			//handleUnsolicited(line,p_channel);
			break;
		}
	}

	if (isIntermediateResult) {
		/* No need to run the following code*/
	} else if (isFinalResponseSuccess(line)) {
		p_response->success = 1;
		handleFinalResponse(line, p_channel);
	} else if (isFinalResponseErrorEx(line, p_channel)) {
		p_response->success = 0;
		handleFinalResponse(line, p_channel);
	} else if (smsPDU != NULL && 0 == strcmp(line, "> ")) {
		// See eg. TS 27.005 4.3
		// Commands like AT+CMGS have a "> " prompt
		writeCtrlZ(smsPDU, p_channel);
		smsPDU = NULL;
	} else {
		handleUnsolicited(line, p_channel);
	}
	// Move out to the function
	//pthread_mutex_unlock(&p_channel->commandmutex);
}

/**
 * Returns a pointer to the end of the next line
 * special-cases the "> " SMS prompt
 *
 * returns NULL if there is no complete line
 */
static char *findNextEOL(char *cur)
{
	if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0')
		/* SMS prompt character...not \r terminated */
		return cur + 2;

	// Find next newline
	while (*cur != '\0' && *cur != '\r' && *cur != '\n') {
        cur++;
    }

	return *cur == '\0' ? NULL : cur;
}

/**
 * Reads a line from the AT channel, returns NULL on timeout.
 * Assumes it has exclusive read access to the FD
 *
 * This line is valid only until the next call to readline
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */
static const char *readline(RILChannelCtx *p_channel)
{
	ssize_t count;

	char *p_read = NULL;
	char *p_eol = NULL;
	char *ret;

	/* this is a little odd. I use *s_ATBufferCur == 0 to
	 * mean "buffer consumed completely". If it points to a character, than
	 * the buffer continues until a \0
	 */
	if (*p_channel->ATBufferCur == '\0') {
		/* empty buffer */
		p_channel->ATBufferCur = p_channel->ATBuffer;
		*p_channel->ATBufferCur = '\0';
		p_read = p_channel->ATBuffer;
	} else { /* *s_ATBufferCur != '\0' */
		/* there's data in the buffer from the last read */

		// skip over leading newlines
		while (*p_channel->ATBufferCur == '\r' || *p_channel->ATBufferCur == '\n')
			p_channel->ATBufferCur++;

		p_eol = findNextEOL(p_channel->ATBufferCur);

		if (p_eol == NULL) {
			/* a partial line. move it up and prepare to read more */
			size_t len;

			len = strlen(p_channel->ATBufferCur);

			memmove(p_channel->ATBuffer, p_channel->ATBufferCur, len + 1);
			p_read = p_channel->ATBuffer + len;
			p_channel->ATBufferCur = p_channel->ATBuffer;
		}
		/* Otherwise, (p_eol !- NULL) there is a complete line  */
		/* that will be returned the while () loop below        */
	}

	while (p_eol == NULL) {
		if (0 == MAX_AT_RESPONSE - (p_read - p_channel->ATBuffer)) {
			LOGE("ERROR: Input line exceeded buffer\n");
			/* ditch buffer and start over again */
			p_channel->ATBufferCur = p_channel->ATBuffer;
			*p_channel->ATBufferCur = '\0';
			p_read = p_channel->ATBuffer;
		}

		do{
                LOGI("AT read start\n");
			count = read(p_channel->fd, p_read,
				     MAX_AT_RESPONSE - (p_read - p_channel->ATBuffer));
            if (mtk_ril_log_level)
                LOGI("AT read end: %d:%s\n", count, strerror(errno));
          }while (count < 0 && errno == EINTR);

		if (count > 0) {
			AT_DUMP("<< ", p_read, count);
			p_channel->readCount += count;

			p_read[count] = '\0';


			// skip over leading newlines
			while (*p_channel->ATBufferCur == '\r' || *p_channel->ATBufferCur == '\n')
				p_channel->ATBufferCur++;

			p_eol = findNextEOL(p_channel->ATBufferCur);
			p_read += count;
		} else if (count <= 0) {
			/* read error encountered or EOF reached */
			if (count == 0)
				LOGD("atchannel: EOF reached");
			else
				LOGD("atchannel: read error %s", strerror(errno));
			return NULL;
		}
	}

	/* a full line in the buffer. Place a \0 over the \r and return */

	ret = p_channel->ATBufferCur;
	*p_eol = '\0';
	p_channel->ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
	/* and there will be a \0 at *p_read */

	LOGD("AT< %s\n", ret);
	return ret;
}

static void onReaderClosed(RILChannelCtx *p_channel)
{
	if (s_onReaderClosed != NULL && p_channel->readerClosed == 0) {
		pthread_mutex_lock(&p_channel->commandmutex);

		p_channel->readerClosed = 1;

		pthread_cond_signal(&p_channel->commandcond);

		pthread_mutex_unlock(&p_channel->commandmutex);

		s_onReaderClosed(p_channel);
	}
}


static void *readerLoop(void *arg)
{
	RILChannelCtx *p_channel = (RILChannelCtx *)arg;
	const char *readerName = p_channel->myName;

	LOGI("%s is up", readerName);


	for (;; ) {
		const char *line;

		line = readline(p_channel);

		LOGD("%s:%s", readerName, line);

		if (line == NULL)
			break;

		if (isSMSUnsolicited(line)) {
			char *line1;
			const char *line2;
			LOGD("SMS Urc Received!");
			// The scope of string returned by 'readline()' is valid only
			// till next call to 'readline()' hence making a copy of line
			// before calling readline again.
			line1 = strdup(line);
			line2 = readline(p_channel);

			if (line2 == NULL) {
				LOGE("NULL line found in %s", readerName);
				break;
			}

			if (p_channel->unsolHandler != NULL) {
				LOGD("%s: line1:%s,line2:%s", readerName, line1, line2);
				p_channel->unsolHandler(line1, line2, p_channel);
			}
			free(line1);
		} else {
			pthread_mutex_lock(&p_channel->commandmutex);
			LOGD("%s Enter processLine", readerName);
			processLine(line, p_channel);
			pthread_mutex_unlock(&p_channel->commandmutex);
		}
	}
	LOGE("%s Closed", readerName);
	onReaderClosed(p_channel);

	return NULL;
}

/**
 * Sends string s to the radio with a \r appended.
 * Returns AT_ERROR_* on error, 0 on success
 *
 * This function exists because as of writing, android libc does not
 * have buffered stdio.
 */
static int writeline(const char *s, RILChannelCtx *p_channel)
{
	size_t cur = 0;
	size_t len = strlen(s);
	ssize_t written;

	if (p_channel->fd < 0 || p_channel->readerClosed > 0)
		return AT_ERROR_CHANNEL_CLOSED;

	LOGD("AT> %s\n", s);

	AT_DUMP(">> ", s, strlen(s));

	/* the main string */
	while (cur < len) {
		do{
			LOGI("AT write start\n");
			written = write(p_channel->fd, s + cur, len - cur);
		}while (written < 0 && errno == EINTR);

		if (written < 0)
			return AT_ERROR_GENERIC;

		cur += written;
	}

	/* the \r  */
	do {
		written = write(p_channel->fd, "\r", 1);
	} while ((written < 0 && errno == EINTR) || (written == 0));

	if (written < 0)
		return AT_ERROR_GENERIC;

	return 0;
}
static int writeCtrlZ(const char *s, RILChannelCtx *p_channel)
{
	size_t cur = 0;
	size_t len = strlen(s);
	ssize_t written;

	if (p_channel->fd < 0 || p_channel->readerClosed > 0)
		return AT_ERROR_CHANNEL_CLOSED;

	LOGD("AT> %s^Z\n", s);

	AT_DUMP(">* ", s, strlen(s));

	/* the main string */
	while (cur < len) {
		do
			written = write(p_channel->fd, s + cur, len - cur);
		while (written < 0 && errno == EINTR);

		if (written < 0)
			return AT_ERROR_GENERIC;

		cur += written;
	}

	/* the ^Z  */
	do {
		written = write(p_channel->fd, "\032", 1);
	} while ((written < 0 && errno == EINTR) || (written == 0));

	if (written < 0)
		return AT_ERROR_GENERIC;

	return 0;
}

static void clearPendingCommand(RILChannelCtx *p_channel)
{
	if (p_channel->p_response != NULL)
		at_response_free(p_channel->p_response);

	p_channel->p_response = NULL;
	p_channel->responsePrefix = NULL;
	p_channel->smsPDU = NULL;
}

/**
 * Starts AT handler on stream "fd'
 * returns 0 on success, -1 on error
 */
int at_open(int fd, ATUnsolHandler h, RILChannelCtx *p_channel)
{
	int ret;
	pthread_t tid;
	pthread_attr_t attr;

	assert(p_channel->fd == fd);
	p_channel->unsolHandler = h;
	p_channel->readerClosed = 0;

	p_channel->responsePrefix = NULL;
	p_channel->smsPDU = NULL;
	p_channel->p_response = NULL;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&p_channel->tid_reader, &attr, readerLoop, (void *)p_channel);

	if (ret < 0) {
		perror("pthread_create");
		return -1;
	}


	return 0;
}

/* FIXME is it ok to call this from the reader and the command thread? */
void at_close(RILChannelCtx *p_channel)
{
	if (p_channel->fd >= 0)
		close(p_channel->fd);
	p_channel->fd = -1;

	pthread_mutex_lock(&p_channel->commandmutex);

	p_channel->readerClosed = 1;

	pthread_cond_signal(&p_channel->commandcond);

	pthread_mutex_unlock(&p_channel->commandmutex);

	/* the reader thread should eventually die */
}

static ATResponse *at_response_new()
{
	return (ATResponse *)calloc(1, sizeof(ATResponse));
}

void at_response_free(ATResponse *p_response)
{
	ATLine *p_line;

	if (p_response == NULL) return;

	p_line = p_response->p_intermediates;

	while (p_line != NULL) {
		ATLine *p_toFree;

		p_toFree = p_line;
		p_line = p_line->p_next;

		free(p_toFree->line);
		free(p_toFree);
	}

	if (p_response->finalResponse != NULL) 
		free(p_response->finalResponse);
	free(p_response);
}

/**
 * The line reader places the intermediate responses in reverse order
 * here we flip them back
 */
static void reverseIntermediates(ATResponse *p_response)
{
	ATLine *pcur, *pnext;

	pcur = p_response->p_intermediates;
	p_response->p_intermediates = NULL;

	while (pcur != NULL) {
		pnext = pcur->p_next;
		pcur->p_next = p_response->p_intermediates;
		p_response->p_intermediates = pcur;
		pcur = pnext;
	}
}

/**
 * Internal send_command implementation
 * Doesn't lock or call the timeout callback
 *
 * timeoutMsec == 0 means infinite timeout
 */
static int at_send_command_full_nolock(const char *command, ATCommandType type,
				       const char *responsePrefix, const char *smspdu,
				       long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
	int err = 0;

#ifndef USE_NP
	struct timespec ts;
#endif  /*USE_NP*/

	if (p_channel->p_response != NULL) {
		err = AT_ERROR_COMMAND_PENDING;
		LOGE("AT_ERROR_COMMAND_PENDING: %s", command);
		goto error;
	}

	err = writeline(command, p_channel);

	if (err < 0)
		goto error;

	p_channel->type = type;
	p_channel->responsePrefix = responsePrefix;
	p_channel->smsPDU = smspdu;
	p_channel->p_response = at_response_new();

#ifndef USE_NP
	if (timeoutMsec != 0)
		setTimespecRelative(&ts, timeoutMsec);

#endif  /*USE_NP*/

	while (p_channel->p_response->finalResponse == NULL && p_channel->readerClosed == 0) {
		if (timeoutMsec != 0) {
#ifdef  USE_NP
			err = pthread_cond_timeout_np(&p_channel->commandcond, &p_channel->commandmutex, timeoutMsec);
#else
			err = pthread_cond_timedwait(&p_channel->commandcond, &p_channel->commandmutex, &ts);
#endif  /*USE_NP*/
		} else {
			err = pthread_cond_wait(&p_channel->commandcond, &p_channel->commandmutex);
		}

		if (err == ETIMEDOUT) {
			err = AT_ERROR_TIMEOUT;
			goto error;
		}
	}

	if (p_channel->readerClosed > 0) {
		err = AT_ERROR_CHANNEL_CLOSED;
		goto error;
	}

	if (pp_outResponse == NULL) {
		at_response_free(p_channel->p_response);
	} else {
		/* line reader stores intermediate responses in reverse order */
		reverseIntermediates(p_channel->p_response);
		*pp_outResponse = p_channel->p_response;
	}

	p_channel->p_response = NULL;

	err = 0;
    
error:
	clearPendingCommand(p_channel);

	return err;
}

/**
 * Internal send_command implementation
 *
 * timeoutMsec == 0 means infinite timeout
 */
static int at_send_command_full(const char *command, ATCommandType type,
				const char *responsePrefix, const char *smspdu,
				long long timeoutMsec, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
	int err;

	if (0 != pthread_equal(p_channel->tid_reader, pthread_self())) {
		/* cannot be called from reader thread */
		LOGD("Invalid Thread: send on %s, reader:%lu, self: %lu", p_channel->myName, p_channel->tid_reader, pthread_self());
		return AT_ERROR_INVALID_THREAD;
	}

	if (0 != p_channel->tid_myProxy) {
		/* This channel is occupied by some proxy */
		LOGD("Occupied Thread: %s send on %s, pthread_self(): %lu, tid: %lu", command, p_channel->myName, pthread_self(), p_channel->tid_myProxy);
		assert(0);
		return AT_ERROR_INVALID_THREAD;
	}

	pthread_mutex_lock(&p_channel->commandmutex);

	/* Assign owner proxy */
	p_channel->tid_myProxy = pthread_self();

	LOGD("AT send on %s, tid:%lu", p_channel->myName, p_channel->tid_myProxy);

	err = at_send_command_full_nolock(command, type, responsePrefix, smspdu, timeoutMsec, pp_outResponse, p_channel);

	LOGD("response received on %s, tid:%lu", p_channel->myName, p_channel->tid_myProxy);

	/* Release the proxy */
	p_channel->tid_myProxy = 0;

	pthread_mutex_unlock(&p_channel->commandmutex);

	if (err == AT_ERROR_TIMEOUT && s_onTimeout != NULL)
		s_onTimeout(p_channel);

	return err;
}

RILChannelCtx *getChannelCtxbyId(RILChannelId id)
{
	assert(id < RIL_SUPPORT_CHANNELS);
	return &s_RILChannel[id];
}

RILId getRILIdByChannelCtx(RILChannelCtx *p_channel)
{
#ifndef MTK_GEMINI
	return MTK_RIL_SOCKET_1;
#else   /* MTK_GEMINI */
	if (p_channel->id < RIL_CHANNEL_OFFSET)
		return MTK_RIL_SOCKET_1;
	else
#if (MTK_GEMINI_SIM_NUM >= 4)/* Gemini plus 4 SIM*/
         if (p_channel->id >= RIL_CHANNEL_SET4_OFFSET)
             return MTK_RIL_SOCKET_4;
#endif
#if (MTK_GEMINI_SIM_NUM >= 3)/* Gemini plus 3 SIM*/
         if (p_channel->id >= RIL_CHANNEL_SET3_OFFSET)
             return MTK_RIL_SOCKET_3;
#endif
		return MTK_RIL_SOCKET_2;
#endif  /* MTK_GEMINI */
}

RILChannelCtx *getDefaultChannelCtx(RILId rid)
{
	RILChannelCtx *channel = &s_RILChannel[RIL_URC];

#ifdef  MTK_GEMINI
	if (MTK_RIL_SOCKET_2 == rid)
		channel = &s_RILChannel[RIL_URC2];
#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    if (MTK_RIL_SOCKET_3 == rid)
        channel = &s_RILChannel[RIL_URC3];
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    if (MTK_RIL_SOCKET_4 == rid)
        channel = &s_RILChannel[RIL_URC4];
#endif
#endif  /* MTK_GEMINI */

	return channel;
}

RILChannelCtx *getRILChannelCtx(RILSubSystemId subsystem, RILId rid)
{
    RILChannelCtx *channel;
#ifdef  MTK_GEMINI
    int channelOffset = rid*RIL_CHANNEL_OFFSET;
#else
    int channelOffset = 0;
#endif
    LOGD("getRILChannelCtx subsystem:%d, rid:%d channelOffset:%d", subsystem, rid, channelOffset);

    switch (subsystem) {
        case RIL_CC:
        case RIL_SS:
            channel = &s_RILChannel[channelOffset+RIL_CMD_2];
            break;
        case RIL_DATA:
            channel = &s_RILChannel[channelOffset+RIL_CMD_4];
            break;          
        case RIL_NW:
        case RIL_OEM:
            channel = &s_RILChannel[channelOffset+RIL_CMD_1];
            break;
        case RIL_SIM:
        case RIL_STK:
        case RIL_SMS:
            channel = &s_RILChannel[channelOffset+RIL_CMD_3];
            break;
        case RIL_DEFAULT:
        default:
            /* RIL_URC as default */
            channel = &s_RILChannel[channelOffset+RIL_URC];
            break;
    }
    
    return channel;
}

RILChannelCtx *getRILChannelCtxFromToken(RIL_Token t)
{
	assert(RIL_queryMyChannelId(t) < RIL_SUPPORT_CHANNELS);
	return &(s_RILChannel[RIL_queryMyChannelId(t)]);
}

RILChannelCtx *getChannelCtxbyProxy(RILId rid)
{
	int proxyId = RIL_queryMyProxyIdByThread();
	assert(proxyId < RIL_SUPPORT_CHANNELS);

	LOGD("getChannelCtxbyProxy proxyId:%d, tid:%lu", proxyId, pthread_self());
	if (proxyId > -1 && proxyId < RIL_SUPPORT_CHANNELS)
#ifdef MTK_GEMINI
            if ((rid == MTK_RIL_SOCKET_1 && proxyId >= RIL_CHANNEL_OFFSET) ||
	        (rid == MTK_RIL_SOCKET_2 && proxyId < RIL_CHANNEL_OFFSET))
                return getDefaultChannelCtx(rid);
#if (MTK_GEMINI_SIM_NUM >= 4)/* Gemini plus 4 SIM*/
            else if (rid == MTK_RIL_SOCKET_4 && proxyId < RIL_CHANNEL_SET4_OFFSET)
                return getDefaultChannelCtx(rid);
#endif
#if (MTK_GEMINI_SIM_NUM >= 3)/* Gemini plus 3 SIM*/
            else if (rid == MTK_RIL_SOCKET_3 && proxyId < RIL_CHANNEL_SET3_OFFSET) 
                return getDefaultChannelCtx(rid);
#endif
	    else
#endif /* MTK_GEMINI */
            return &s_RILChannel[proxyId];
	else
	    // This is not a proxy thread (URC thread)
	    return getDefaultChannelCtx(rid);
}

/**
 * Issue a single normal AT command with no intermediate response expected
 *
 * "command" should not include \r
 * pp_outResponse can be NULL
 *
 * if non-NULL, the resulting ATResponse * must be eventually freed with at_response_free
 */
int at_send_command(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
	int err;
	err = at_send_command_full(command, NO_RESULT, NULL, NULL, 0, pp_outResponse, p_channel);
	return err;
}


int at_send_command_singleline(const char *	command,
			       const char *	responsePrefix,
			       ATResponse **	pp_outResponse,
			       RILChannelCtx *	p_channel)
{
	int err;
	err = at_send_command_full(command, SINGLELINE, responsePrefix, NULL, 0, pp_outResponse, p_channel);

	if (err == 0 && pp_outResponse != NULL
	    && (*pp_outResponse)->success > 0
	    && (*pp_outResponse)->p_intermediates == NULL
	    ) {
		/* successful command must have an intermediate response */
		at_response_free(*pp_outResponse);
		*pp_outResponse = NULL;
		return AT_ERROR_INVALID_RESPONSE;
	}

	return err;
}


int at_send_command_numeric(const char *	command,
			    ATResponse **	pp_outResponse,
			    RILChannelCtx *	p_channel)
{
	int err;
	err = at_send_command_full(command, NUMERIC, NULL, NULL, 0, pp_outResponse, p_channel);

	if (err == 0 && pp_outResponse != NULL && 
        (*pp_outResponse)->success > 0 && 
        (*pp_outResponse)->p_intermediates == NULL) 
    {
		/* successful command must have an intermediate response */
		at_response_free(*pp_outResponse);
		*pp_outResponse = NULL;
		return AT_ERROR_INVALID_RESPONSE;
	}

	return err;
}


int at_send_command_sms(const char *	command,
                        const char *	pdu,
                        const char *	responsePrefix,
                        ATResponse **	pp_outResponse,
                        RILChannelCtx * p_channel)
{
	int err;
	err = at_send_command_full(command, SINGLELINE, responsePrefix, pdu, 0, pp_outResponse, p_channel);

	if (err == 0 && pp_outResponse != NULL && 
        (*pp_outResponse)->success > 0 && 
        (*pp_outResponse)->p_intermediates == NULL) {
		/* successful command must have an intermediate response */
		at_response_free(*pp_outResponse);
		*pp_outResponse = NULL;
		return AT_ERROR_INVALID_RESPONSE;
	}

	return err;
}


int at_send_command_multiline(  const char * command,
                                const char * responsePrefix,
                                ATResponse ** pp_outResponse,
                                RILChannelCtx * p_channel)
{
	int err;
	err = at_send_command_full(command, MULTILINE, responsePrefix, NULL, 0, pp_outResponse, p_channel);
	return err;
}

/* atci start */
int at_send_command_raw(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel)
{
	int err;
	err = at_send_command_full(command, RAW, NULL, NULL, 0, pp_outResponse, p_channel);
	return err;
}
/* atci end */

/** This callback is invoked on the command thread */
void at_set_on_timeout(void (*onTimeout)(RILChannelCtx *p_channel))
{
	s_onTimeout = onTimeout;
}

/**
 *  This callback is invoked on the reader thread (like ATUnsolHandler)
 *  when the input stream closes before you call at_close
 *  (not when you call at_close())
 *  You should still call at_close()
 */
void at_set_on_reader_closed(void (*onClose)(RILChannelCtx *p_channel))
{
	s_onReaderClosed = onClose;
}

/**
 * Periodically issue an AT command and wait for a response.
 * Used to ensure channel has start up and is active
 */
int at_handshake(RILChannelCtx *p_channel)
{
	int i;
	int err = 0;

	if (0 != pthread_equal(p_channel->tid_reader, pthread_self()))
		/* cannot be called from reader thread */
		return AT_ERROR_INVALID_THREAD;

	pthread_mutex_lock(&p_channel->commandmutex);

	for (i = 0; i < HANDSHAKE_RETRY_COUNT; i++) {
		/* some stacks start with verbose off */
		err = at_send_command_full_nolock("ATE0Q0V1", NO_RESULT, NULL, NULL, 0, NULL, p_channel);
		if (err == 0)
			break;
	}

	if (err == 0) {
		/* pause for a bit to let the input buffer drain any unmatched OK's
		 * (they will appear as extraneous unsolicited responses) */
		sleepMsec(HANDSHAKE_TIMEOUT_MSEC);
	}

	pthread_mutex_unlock(&p_channel->commandmutex);

	return err;
}

/**
 * Returns error code from response
 * Assumes AT+CMEE=1 (numeric) mode
 */
AT_CME_Error at_get_cme_error(const ATResponse *p_response)
{
	int ret;
	int err;
	char *p_cur;

	if (p_response->success > 0)
		return CME_SUCCESS;

	if (p_response->finalResponse == NULL)
		return CME_ERROR_NON_CME;

	if (strStartsWith(p_response->finalResponse, "ERROR"))
		return CME_UNKNOWN;

	if (!strStartsWith(p_response->finalResponse, "+CME ERROR:"))
		return CME_ERROR_NON_CME;

	p_cur = p_response->finalResponse;
	err = at_tok_start(&p_cur);

	if (err < 0)
		return CME_ERROR_NON_CME;

	err = at_tok_nextint(&p_cur, &ret);

	if (err < 0)
		return CME_ERROR_NON_CME;

	return (AT_CME_Error)ret;
}

// M: For PPP channel
static RILChannelCtx g_pppDataChannel = {0};
static int g_channelIndex = 0;

RILChannelCtx *openPPPDataChannel(int isBlocking)
{
    LOGI("openDataChannel");
    RILChannelCtx* p_channel = &g_pppDataChannel;
    if (p_channel->fd > 0)
        closePPPDataChannel();

    memset(p_channel, 0, sizeof(RILChannelCtx));
    p_channel->fd = -1;    /* fd of the AT channel */

    int retryCounter = 0;
    int err = 0;
    while (p_channel->fd < 0 && retryCounter < 5) {
        do {
            LOGI("set property for usb permission");
            /* set this property than the permission of /dev/ttyACM0 will be set to 777 */
            property_set("gsm.usb.ttyusb", "1");
            if (isBlocking)
                p_channel->fd = open("/dev/ttyUSB0", O_RDWR);
            else
                p_channel->fd = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);
        } while (p_channel->fd < 0 && errno == EINTR);

        if (p_channel->fd < 0) {
            perror ("opening AT interface. retrying...");
            LOGE("could not connect to %s: %s", "/dev/ttyUSB0", strerror(errno));
            /* reduce polling time for usb connected */
            sleep(1);
            /* never returns */
        } else {
            struct termios ios;
            tcgetattr(p_channel->fd, &ios );
            ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
            ios.c_iflag = 0;
            tcsetattr(p_channel->fd, TCSANOW, &ios );
        }
        ++retryCounter;
    }

    if (p_channel->fd < 0) {
        LOGE("/dev/ttyUSB0 open failed");
        return NULL;
    } else {
        LOGI("/dev/ttyUSB0 open success");
        p_channel->ATBufferCur = p_channel->ATBuffer;
        p_channel->myName = "PPP_CHANNEL";
        p_channel->id = ++g_channelIndex;
        p_channel->unsolHandler = 0;
        p_channel->readerClosed = 0;
        p_channel->responsePrefix = NULL;
        p_channel->smsPDU = NULL;
        p_channel->p_response = NULL;
    }
    return &g_pppDataChannel;
}

void closePPPDataChannel()
{
    RILChannelCtx* p_channel = &g_pppDataChannel;
    LOGI("closeDataChannel [%d, %d]", g_channelIndex, p_channel->fd);
    if (p_channel->fd >= 0) {
        close(p_channel->fd);
    }
    p_channel->fd = -1;
}

int at_send_command_to_ppp_data_channel(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel) {
    const char* line = NULL;
    int ret = writeline(command, p_channel);
    if (ret == 0) {
        p_channel->p_response = at_response_new();
        do {
            line = readline(p_channel);
            if (line != NULL)
                LOGI("readline: %s", line);
            else
                LOGI("readline: EMPTY");
        } while (line != NULL && !(strcmp(line, "OK") == 0 || strcmp(line, "NO CARRIER") == 0 || strStartsWith(line, "CONNECT") == 1 || strstr(line, "ERROR")));

        if (line != NULL) {
            LOGI("process line: %s", line);
            processLine(line, p_channel);
            if (pp_outResponse == NULL) {
                at_response_free(p_channel->p_response);
            } else {
                reverseIntermediates(p_channel->p_response);
                *pp_outResponse = p_channel->p_response;
            }
            return 0;
        }
    }
    return AT_ERROR_GENERIC;
}

void waitForTargetPPPStopped(RILChannelCtx *p_channel) {
    const char* line = NULL;
    int count = 0;
    p_channel->p_response = at_response_new();
    while (count < 60) {
        line = readline(p_channel);
        if (line != NULL && strcmp(line, "NO CARRIER") == 0) {
            LOGI("readline: %s [%d]", line, count);
            break;
        } else {
            LOGI("Still wait for NO CARRIER [%d]", count);
            ++count;
            sleep(1);
        }
    }
    at_response_free(p_channel->p_response);
}

void purge_data_channel(RILChannelCtx *p_channel) {
    if (p_channel > 0 && p_channel->fd > 0) {
        const char* line = NULL;
        char buffer[64] = {0};
        int count = 0;
        int readbyte = 0;
        while ((readbyte = read(p_channel->fd, buffer, 64)) > 0) {
            count += readbyte;
        }
        LOGI("Total %d byte purged", count);
    } else {
        LOGI("Channel not opened, not to purge");
    }
}
// End of PPP channel

const char *channelIdToString(RILChannelId id)
{
	switch (id) {
    	case RIL_URC:       return "RIL_URC_READER";
    	case RIL_CMD_1:     return "RIL_CMD_READER_1";
    	case RIL_CMD_2:     return "RIL_CMD_READER_2";
    	case RIL_CMD_3:     return "RIL_CMD_READER_3";
        case RIL_CMD_4:     return "RIL_CMD_READER_4";		
        case RIL_ATCI:      return "RIL_ATCI_READER";	
#ifdef  MTK_GEMINI
    	case RIL_URC2:      return "RIL_URC2_READER";
    	case RIL_CMD2_1:    return "RIL_CMD2_READER_1";
    	case RIL_CMD2_2:    return "RIL_CMD2_READER_2";
    	case RIL_CMD2_3:    return "RIL_CMD2_READER_3";
    	case RIL_CMD2_4:    return "RIL_CMD2_READER_4";		
        case RIL_ATCI2:     return "RIL_ATCI2_READER";	

#if (MTK_GEMINI_SIM_NUM >= 3) /* Gemini plus 3 SIM*/
    	case RIL_URC3:		return "RIL_URC3_READER";
    	case RIL_CMD3_1:	return "RIL_CMD3_READER_1";
    	case RIL_CMD3_2:	return "RIL_CMD3_READER_2";
    	case RIL_CMD3_3:	return "RIL_CMD3_READER_3";
    	case RIL_CMD3_4:	return "RIL_CMD3_READER_4"; 	
        case RIL_ATCI3:         return "RIL_ATCI3_READER";	
#endif
#if (MTK_GEMINI_SIM_NUM >= 4) /* Gemini plus 4 SIM*/
    	case RIL_URC4:		return "RIL_URC4_READER";
    	case RIL_CMD4_1:	return "RIL_CMD4_READER_1";
    	case RIL_CMD4_2:	return "RIL_CMD4_READER_2";
    	case RIL_CMD4_3:	return "RIL_CMD4_READER_3";
    	case RIL_CMD4_4:	return "RIL_CMD4_READER_4"; 
        case RIL_ATCI4:         return "RIL_ATCI4_READER";		
#endif	
#endif  /* MTK_GEMINI */
	    default:            return "<unknown proxy>";
	}
}

inline int isATCmdRspErr(int err, const ATResponse *p_response)
{
    //assert(p_response); //checking null here ???
    return (err < 0 || 0 == p_response->success) ? 1: 0;
}


int openDataChannel(char* deviceNote, int isBlocking, RILChannelCtx* p_channel, int channelId)
{
    LOGI("openDataChannel");
    if (p_channel->fd > 0)
        closeDataChannel(p_channel);

    memset(p_channel, 0, sizeof(RILChannelCtx));
    p_channel->fd = -1;    /* fd of the AT channel */

    int retryCounter = 0;
    int err = 0;
    while (p_channel->fd < 0 && retryCounter < 5) {
        do {
            LOGI("set property for usb permission");
            p_channel->fd = open(deviceNote, isBlocking? O_RDWR : (O_RDWR | O_NONBLOCK));
        } while (p_channel->fd < 0 && errno == EINTR);

        if (p_channel->fd < 0) {
            perror ("opening AT interface. retrying...");
            LOGE("could not connect to %s: %s", "/dev/ttyUSB0", strerror(errno));
            /* reduce polling time for usb connected */
            sleep(1);
            /* never returns */
        } else {
            struct termios ios;
            tcgetattr(p_channel->fd, &ios );
            ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
            ios.c_iflag = 0;
            tcsetattr(p_channel->fd, TCSANOW, &ios );
        }
        ++retryCounter;
    }

    if (p_channel->fd < 0) {
        LOGE("%s open failed", deviceNote);
        return -1;
    } else {
        LOGI("%s open success", deviceNote);
        p_channel->ATBufferCur = p_channel->ATBuffer;
        p_channel->myName = deviceNote;
        p_channel->id = ++channelId;
        p_channel->unsolHandler = 0;
        p_channel->readerClosed = 0;
        p_channel->responsePrefix = NULL;
        p_channel->smsPDU = NULL;
        p_channel->p_response = NULL;
    }
    return 0;
}

void closeDataChannel(RILChannelCtx* p_channel)
{
    if (p_channel == NULL) {
        LOGE("closeDataChannel invalid input argument");
        return;
    }

    LOGI("closeDataChannel [%d, %d]", p_channel->id, p_channel->fd);
    if (p_channel->fd >= 0) {
        close(p_channel->fd);
    }
    p_channel->fd = -1;
}

int at_send_command_to_data_channel(const char *command, ATResponse **pp_outResponse, RILChannelCtx *p_channel) {
    const char* line = NULL;
    int ret = writeline(command, p_channel);
    if (ret == 0) {
        p_channel->p_response = at_response_new();
        do {
            line = readline(p_channel);
            if (line != NULL)
                LOGI("readline: %s", line);
            else
                LOGI("readline: EMPTY");
        } while (line != NULL && !(strcmp(line, "OK") == 0 || strcmp(line, "NO CARRIER") == 0 || strStartsWith(line, "CONNECT") == 1 || strstr(line, "ERROR")));

        if (line != NULL) {
            LOGI("process line: %s", line);
            processLine(line, p_channel);
            if (pp_outResponse == NULL) {
                at_response_free(p_channel->p_response);
            } else {
                reverseIntermediates(p_channel->p_response);
                *pp_outResponse = p_channel->p_response;
            }
            return 0;
        }
    }
    return AT_ERROR_GENERIC;
}
#endif  /* MTK_RIL */
