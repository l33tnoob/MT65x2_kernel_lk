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

/* //device/system/reference-ril/reference-ril.c
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
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <alloca.h>
#include "atchannels.h"
#include "at_tok.h"
#include "misc.h"
#include <getopt.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <termios.h>

#include <ril_callbacks.h>

#ifdef MTK_RIL_MD1
#define LOG_TAG "RIL"
#else
#define LOG_TAG "RILMD2"
#endif

#include <utils/Log.h>

#ifdef USE_TI_COMMANDS
// Enable a workaround
// 1) Make incoming call, do not answer
// 2) Hangup remote end
// Expected: call should disappear from CLCC line
// Actual: Call shows as "ACTIVE" before disappearing
#define WORKAROUND_ERRONEOUS_ANSWER 1
#endif

#ifdef WORKAROUND_ERRONEOUS_ANSWER
// Max number of times we'll try to repoll when we think
// we have a AT+CLCC race condition
#define REPOLL_CALLS_COUNT_MAX 4

// Line index that was incoming or waiting at last poll, or -1 for none
static int s_incomingOrWaitingLine = -1;
// Number of times we've asked for a repoll of AT+CLCC
static int s_repollCallsCount = 0;
// Should we expect a call to be answered in the next CLCC?
static int s_expectAnswer = 0;
#endif /* WORKAROUND_ERRONEOUS_ANSWER */

/* MTK proprietary start */
#define CC_CHANNEL_CTX getRILChannelCtxFromToken(t)

char *setupCpiData[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int callWaiting = 0;
int isAlertSet = 0;
int hasReceivedRing = 0;
int inCallNumber = 0;
int isReplaceRequest = 0;
//BEGIN mtk03923 [20120210][ALPS00114093]
int inDTMF = 0;
//END   mtk03923 [20120210][ALPS00114093]
bool isRecvECPI0 = false;
int ringCallID = 0;

//Solve [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
//mtk04070, 2012.02.24
int bUseLocalCallFailCause = 0;
int dialLastError = 0;
/* MTK proprietary end */

static int clccStateToRILState(int state, RIL_CallState *p_state)
{
	switch (state) {
    	case 0: *p_state = RIL_CALL_ACTIVE;   return 0;
    	case 1: *p_state = RIL_CALL_HOLDING;  return 0;
    	case 2: *p_state = RIL_CALL_DIALING;  return 0;
    	case 3: *p_state = RIL_CALL_ALERTING; return 0;
    	case 4: *p_state = RIL_CALL_INCOMING; return 0;
    	case 5: *p_state = RIL_CALL_WAITING;  return 0;
    	default: return -1;
	}
}


/**
 * Note: directly modified line and has *p_call point directly into
 * modified line
 */
static int callFromCLCCLine(char *line, RIL_Call *p_call)
{
	//+CLCC: 1,0,2,0,0,\"+18005551212\",145
	//     index,isMT,state,mode,isMpty(,number,TOA)?

	int err;
	int state;
	int mode;

	err = at_tok_start(&line);
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &(p_call->index));
	if (err < 0) goto error;

	err = at_tok_nextbool(&line, &(p_call->isMT));
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &state);
	if (err < 0) goto error;

	err = clccStateToRILState(state, &(p_call->state));
	if (err < 0) goto error;

	err = at_tok_nextint(&line, &mode);
	if (err < 0) goto error;

	p_call->isVoice = (mode == 0);

	err = at_tok_nextbool(&line, &(p_call->isMpty));
	if (err < 0) goto error;

	if (at_tok_hasmore(&line)) {
		err = at_tok_nextstr(&line, &(p_call->number));

		/* tolerate null here */
		if (err < 0) return 0;

		// Some lame implementations return strings
		// like "NOT AVAILABLE" in the CLCC line
		if (p_call->number != NULL && 0 == strspn(p_call->number, "+0123456789"))
			p_call->number = NULL;

		err = at_tok_nextint(&line, &p_call->toa);
		if (err < 0) goto error;
	}

	p_call->uusInfo = NULL;

	return 0;

error:
	LOGE("invalid CLCC line\n");
	return -1;
}


extern void requestGetCurrentCalls(void *data, size_t datalen, RIL_Token t)
{
	int err;
	ATResponse *p_response;
	ATLine *p_cur;
	int countCalls;
	int countValidCalls;
	RIL_Call *p_calls;
	RIL_Call **pp_calls;
	int i;
	int needRepoll = 0;

#ifdef WORKAROUND_ERRONEOUS_ANSWER
	int prevIncomingOrWaitingLine;

	prevIncomingOrWaitingLine = s_incomingOrWaitingLine;
	s_incomingOrWaitingLine = -1;
#endif  /*WORKAROUND_ERRONEOUS_ANSWER*/

	err = at_send_command_multiline("AT+CLCC", "+CLCC:", &p_response, CC_CHANNEL_CTX);

	if (err != 0 || p_response->success == 0) {
		RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
    at_response_free(p_response);		
		return;
	}

	/* count the calls */
	for (countCalls = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
		countCalls++;

	/* yes, there's an array of pointers and then an array of structures */

	pp_calls = (RIL_Call **)alloca(countCalls * sizeof(RIL_Call *));
	p_calls = (RIL_Call *)alloca(countCalls * sizeof(RIL_Call));
	memset(p_calls, 0, countCalls * sizeof(RIL_Call));

	/* init the pointer array */
	for (i = 0; i < countCalls; i++)
		pp_calls[i] = &(p_calls[i]);

	for (countValidCalls = 0, p_cur = p_response->p_intermediates
	     ; p_cur != NULL
	     ; p_cur = p_cur->p_next
	     ) {
		err = callFromCLCCLine(p_cur->line, p_calls + countValidCalls);

		if (err != 0)
			continue;

#ifdef  WORKAROUND_ERRONEOUS_ANSWER
		if (p_calls[countValidCalls].state == RIL_CALL_INCOMING || 
            p_calls[countValidCalls].state == RIL_CALL_WAITING) {
			s_incomingOrWaitingLine = p_calls[countValidCalls].index;
        }
#endif  /*WORKAROUND_ERRONEOUS_ANSWER*/

		if (p_calls[countValidCalls].state != RIL_CALL_ACTIVE && 
            p_calls[countValidCalls].state != RIL_CALL_HOLDING) {
			needRepoll = 1;
        }

		countValidCalls++;
	}

#ifdef WORKAROUND_ERRONEOUS_ANSWER
	// Basically:
	// A call was incoming or waiting
	// Now it's marked as active
	// But we never answered it
	//
	// This is probably a bug, and the call will probably
	// disappear from the call list in the next poll
	if (prevIncomingOrWaitingLine >= 0
	    && s_incomingOrWaitingLine < 0
	    && s_expectAnswer == 0
	    ) {
		for (i = 0; i < countValidCalls; i++) {
			if (p_calls[i].index == prevIncomingOrWaitingLine && 
                p_calls[i].state == RIL_CALL_ACTIVE && 
                s_repollCallsCount < REPOLL_CALLS_COUNT_MAX) {
				LOGI("Hit WORKAROUND_ERRONOUS_ANSWER case. Repoll count: %d\n", s_repollCallsCount);
				s_repollCallsCount++;
				goto error;
			}
		}
	}

	s_expectAnswer = 0;
	s_repollCallsCount = 0;
#endif  /*WORKAROUND_ERRONEOUS_ANSWER*/

	RIL_onRequestComplete(t, RIL_E_SUCCESS, pp_calls, countValidCalls * sizeof(RIL_Call *));

	at_response_free(p_response);
	return;
    
error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}


void requestDial(void *data, size_t datalen, RIL_Token t, int isEmergency)
{
	RIL_Dial *p_dial;
	char *cmd;
	const char *clir;
	int ret;
	ATResponse *p_response = NULL;

	/*mtk00924: ATDxxxI can not used for FDN check, therefore, change to #31# or *31#*/

	p_dial = (RIL_Dial *)data;

	switch (p_dial->clir) {
	case 1: /*invocation*/
		clir = "#31#";
		break;
	case 2: /*suppression*/
		clir = "*31#";
		break;
	case 0:
	default: /*subscription default*/
		clir = "";
		break;
	}

	if (isEmergency) {
		asprintf(&cmd, "ATDE%s%s;", clir, p_dial->address);
	} else {
		//BEGIN mtk03923 [20111004][ALPS00077405]
		// CC operation will fail when dialing number exceed 40 character due to modem capability limitation.
		if (strlen(p_dial->address) > 40) {
			LOGE("strlen(%s)=%d exceeds 40 character\n", p_dial->address, strlen(p_dial->address));

			RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);
			at_response_free(p_response);

			//Solve [ALPS00251057][Call]It didn't pop FDN dialog when dial an invalid number
			//But this is not related to FDN issue, it returned to AP since number is too long.			
			//mtk04070, 2012.03.12
			bUseLocalCallFailCause = 1;
			dialLastError = 28; /* Refer to CallFailCause.java - INVALID_NUMBER_FORMAT */
			
			return;
		}
		//END   mtk03923 [20111004][ALPS00077405]

		asprintf(&cmd, "ATD%s%s;", clir, p_dial->address);
	}
	ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

	//Solve [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
	//mtk04070, 2012.02.24
	bUseLocalCallFailCause = 0;
	dialLastError = 0;

	free(cmd);

	if (ret < 0 || p_response->success == 0)
		goto error;
	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}


void requestHangup(void *data, size_t datalen, RIL_Token t)
{
	int *p_line;
	char *cmd;

	p_line = (int *)data;

	// 3GPP 22.030 6.5.5
	// "Releases a specific active call X"
	asprintf(&cmd, "AT+CHLD=1%d", p_line[0]);
	at_send_command(cmd, NULL, CC_CHANNEL_CTX);

	free(cmd);

	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}


void requestHangupWaitingOrBackground(void *data, size_t datalen, RIL_Token t)
{
	// 3GPP 22.030 6.5.5
	// "Releases all held calls or sets User Determined User Busy
	//  (UDUB) for a waiting call."
	at_send_command("AT+CHLD=0", NULL, CC_CHANNEL_CTX);

	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}


void requestHangupForegroundResumeBackground(void *data, size_t datalen, RIL_Token t)
{
	// 3GPP 22.030 6.5.5
	// "Releases all active calls (if any exist) and accepts
	//  the other (held or waiting) call."
	//at_send_command("AT+CHLD=1", NULL, CC_CHANNEL_CTX);
	at_send_command_multiline("AT+CHLD=1", "NO CARRIER", NULL, CC_CHANNEL_CTX);

	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}


void requestSwitchWaitingOrHoldingAndActive(void *data, size_t datalen, RIL_Token t)
{
	int ret;
	ATResponse *p_response = NULL;

        //BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
	    RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        //END   mtk03923 [20120210][ALPS00114093]


	ret = at_send_command("AT+CHLD=2", &p_response, CC_CHANNEL_CTX);

	if (ret < 0 || p_response->success == 0)
		goto error;

#ifdef WORKAROUND_ERRONEOUS_ANSWER
	s_expectAnswer = 1;
#endif  /* WORKAROUND_ERRONEOUS_ANSWER */

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}


void requestAnswer(void *data, size_t datalen, RIL_Token t)
{
	at_send_command("ATA", NULL, CC_CHANNEL_CTX);

#ifdef WORKAROUND_ERRONEOUS_ANSWER
	s_expectAnswer = 1;
#endif  /* WORKAROUND_ERRONEOUS_ANSWER */

	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}


void requestConference(void *data, size_t datalen, RIL_Token t)
{
	int ret;
	ATResponse *p_response = NULL;

        //BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
	    RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        //END   mtk03923 [20120210][ALPS00114093]

	ret = at_send_command("AT+CHLD=3", &p_response, CC_CHANNEL_CTX);

	if (ret < 0 || p_response->success == 0)
		goto error;

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
}


void requestUdub(void *data, size_t datalen, RIL_Token t)
{
	/* user determined user busy */
	/* sometimes used: ATH */
	at_send_command("ATH", NULL, CC_CHANNEL_CTX);

	/* success or failure is ignored by the upper layer here.
	 * it will call GET_CURRENT_CALLS and determine success that way */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}


void requestSeparateConnection(void *data, size_t datalen, RIL_Token t)
{
	char cmd[12];
	int party = ((int *)data)[0];
	int ret;
	ATResponse *p_response = NULL;

        //BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
	    RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        //END   mtk03923 [20120210][ALPS00114093]


	// Make sure that party is in a valid range.
	// (Note: The Telephony middle layer imposes a range of 1 to 7.
	// It's sufficient for us to just make sure it's single digit.)
	if (party > 0 && party < 10) {
		sprintf(cmd, "AT+CHLD=2%d", party);
		ret = at_send_command(cmd, &p_response, CC_CHANNEL_CTX);

		if (ret < 0 || p_response->success == 0) {
			at_response_free(p_response);
			goto error;
		}

		RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
		at_response_free(p_response);
		return;
	}

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}


void requestExplicitCallTransfer(void *data, size_t datalen, RIL_Token t)
{
	/* MTK proprietary start */
	int ret;
	ATResponse *p_response = NULL;

        //BEGIN mtk03923 [20120210][ALPS00114093]
        if (inDTMF) {
	    RIL_onRequestComplete(t, RIL_E_CANCELLED, NULL, 0);     // RIL_E_GENERIC_FAILURE
            return;
        }
        //END   mtk03923 [20120210][ALPS00114093]


	ret = at_send_command("AT+CHLD=4", &p_response, CC_CHANNEL_CTX);

	if (ret < 0 || p_response->success == 0)
		goto error;

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
	at_response_free(p_response);
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	at_response_free(p_response);
	/* MTK proprietary end */
}


void requestLastCallFailCause(void *data, size_t datalen, RIL_Token t)
{
	/* MTK proprietary start */
	int callFailCause;
	char *line;
	int ret;
	ATResponse *p_response = NULL;

	//Solve [ALPS00242104]Invalid number show but cannot call drop when dial VT call in 2G network
	//mtk04070, 2012.02.24
	if (bUseLocalCallFailCause == 1) {
	   callFailCause = dialLastError;
	   LOGD("Use local call fail cause = %d", callFailCause);
	}
	else {
 	   ret = at_send_command_singleline("AT+CEER", "+CEER:", &p_response, CC_CHANNEL_CTX);

	   if (ret < 0 || p_response->success == 0)
		   goto error;

	   line = p_response->p_intermediates->line;

	   ret = at_tok_start(&line);

	   if (ret < 0)
		   goto error;

	   ret = at_tok_nextint(&line, &callFailCause);

	   if (ret < 0)
		   goto error;

	   LOGD("MD fail cause = %d", callFailCause);
	}

	/*if there are more causes need to be translated in the future,
	 * discussing with APP owner to implement this in upper layer.
	 * For the hard coded value, please refer to modem code.*/

	if (callFailCause == 10 || callFailCause == 8)
		callFailCause = CALL_FAIL_CALL_BARRED;
	else if (callFailCause == 2600)
		callFailCause = CALL_FAIL_FDN_BLOCKED;
	else if (callFailCause == 2052)
		callFailCause = CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
	else if (callFailCause == 2053)
		callFailCause = CALL_FAIL_IMEI_NOT_ACCEPTED;
	else if ((callFailCause > 127 && callFailCause != 2165) || callFailCause <= 0)
		callFailCause = CALL_FAIL_ERROR_UNSPECIFIED;

	LOGD("RIL fail cause = %d", callFailCause);
	RIL_onRequestComplete(t, RIL_E_SUCCESS, &callFailCause, sizeof(int));
	if (NULL != p_response) 
	{
		at_response_free(p_response);
	}
	return;

error:
	RIL_onRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	if (NULL != p_response) 
	{
	    at_response_free(p_response);
	}
	/* MTK proprietary end */
}


void requestDtmf(void *data, size_t datalen, RIL_Token t)
{
	char c = ((char *)data)[0];
	char *cmd;

	asprintf(&cmd, "AT+VTS=%c", (int)c);
	at_send_command(cmd, NULL, CC_CHANNEL_CTX);

	free(cmd);

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

extern int rilCcMain(int request, void *data, size_t datalen, RIL_Token t)
{
	switch (request) {
	case RIL_REQUEST_GET_CURRENT_CALLS:
		requestGetCurrentCalls(data, datalen, t);
		break;
	case RIL_REQUEST_DIAL:
		requestDial(data, datalen, t, 0);
		break;
	case RIL_REQUEST_HANGUP:
		requestHangup(data, datalen, t);
		break;
	case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
		requestHangupWaitingOrBackground(data, datalen, t);
		break;
	case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
		requestHangupForegroundResumeBackground(data, datalen, t);
		break;
	case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
		requestSwitchWaitingOrHoldingAndActive(data, datalen, t);
		break;
	case RIL_REQUEST_ANSWER:
		requestAnswer(data, datalen, t);
		break;
	case RIL_REQUEST_CONFERENCE:
		requestConference(data, datalen, t);
		break;
	case RIL_REQUEST_UDUB:
		requestUdub(data, datalen, t);
		break;
	case RIL_REQUEST_SEPARATE_CONNECTION:
		requestSeparateConnection(data, datalen, t);
		break;
	case RIL_REQUEST_EXPLICIT_CALL_TRANSFER:
		requestExplicitCallTransfer(data, datalen, t);
		break;
	case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
		requestLastCallFailCause(data, datalen, t);
		break;
	case RIL_REQUEST_DTMF:
		requestDtmf(data, datalen, t);
		break;
	case RIL_REQUEST_DTMF_START:
		requestDtmfStart(data, datalen, t);
		break;
	case RIL_REQUEST_DTMF_STOP:
		requestDtmfStop(data, datalen, t);
		break;
	case RIL_REQUEST_SET_TTY_MODE:
		requestSetTTYMode(data, datalen, t);
		break;
	/* MTK proprietary start */
	case RIL_REQUEST_EMERGENCY_DIAL:
		requestDial(data, datalen, t, 1);
		break;
	case RIL_REQUEST_HANGUP_ALL:
		requestHangupAll(data, datalen, t);
		break;
	case RIL_REQUEST_HANGUP_ALL_EX:
		requestHangupAllEx(data, datalen, t);
		break;
	case RIL_REQUEST_FORCE_RELEASE_CALL:
		requestForceReleaseCall(data, datalen, t);
		break;
	case RIL_REQUEST_SET_CALL_INDICATION:
		requestSetCallIndication(data, datalen, t);
		break;
	case RIL_REQUEST_GET_CCM:
		requestGetCcm(data, datalen, t);
		break;
	case RIL_REQUEST_GET_ACM:
		requestGetAcm(data, datalen, t);
		break;
	case RIL_REQUEST_GET_ACMMAX:
		requestGetAcmMax(data, datalen, t);
		break;
	case RIL_REQUEST_GET_PPU_AND_CURRENCY:
		requestGetPpuAndCurrency(data, datalen, t);
		break;
	case RIL_REQUEST_SET_ACMMAX:
		requestSetAcmMax(data, datalen, t);
		break;
	case RIL_REQUEST_RESET_ACM:
		requestResetAcm(data, datalen, t);
		break;
	case RIL_REQUEST_SET_PPU_AND_CURRENCY:
		requestSetPpuAndCurrency(data, datalen, t);
		break;
	case RIL_REQUEST_DISABLE_VT_CAPABILITY:
		requestDisableVTCapability(data, datalen, t);
		break;
		/* MTK proprietary end */
#ifdef  MTK_VT3G324M_SUPPORT
	case RIL_REQUEST_VT_DIAL:
		requestVtDial(data, datalen, t);
		break;
	case RIL_REQUEST_VOICE_ACCEPT:
		requestVoiceAccept(data, datalen, t);
		break;
	case RIL_REQUEST_REPLACE_VT_CALL:
		requestReplaceVtCall(data, datalen, t);
		break;
#endif  /* MTK_VT3G324M_SUPPORT */
	default:
		return 0; /* no matched request */
		break;
	}

	return 1; /* request found and handled */
}


extern int rilCcUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx *p_channel)
{
	RILId rid = getRILIdByChannelCtx(p_channel);

	/* MTK proprietary start */
	if (strStartsWith(s, "RING") || strStartsWith(s, "+CRING")) {
		LOGD("receiving RING!!!!!!");

        if(!isRecvECPI0) {
            LOGD("we havn't receive ECPI0, skip this RING!");
            return 1;
        }
		if (!hasReceivedRing) {
			LOGD("receiving first RING!!!!!!");
			hasReceivedRing = 1;
		}

		if (setupCpiData[0] != NULL) {
			LOGD("sending ECPI!!!!!!");
			RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_PROGRESS_INFO, setupCpiData, 9 * sizeof(char *), rid);

			int i;
			for (i = 0; i < 9; i++) {
				free(setupCpiData[i]);
				setupCpiData[i] = NULL;
			}
			sleep(1);
		}

#ifdef MTK_VT3G324M_SUPPORT
		if (strStartsWith(s, "+CRING: VIDEO")) {
			if (!isReplaceRequest) {
				RIL_onUnsolicitedResponse(RIL_UNSOL_VT_RING_INFO, NULL, 0, rid);
			}
		} else {
#endif
		if (!isReplaceRequest) {
			RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_RING, NULL, 0, rid);
		}
#ifdef MTK_VT3G324M_SUPPORT
	}
#endif

		return 1;
	} else if (strStartsWith(s, "+ECPI")) {
		onCallProgressInfo((char *)s, rid);
		return 1;
	} else if (strStartsWith(s, "+ESPEECH")) {
		onSpeechInfo((char *)s, rid);
		return 1;
	} else if (strStartsWith(s, "+EAIC")) {
		onIncomingCallIndication((char *)s, rid);
		return 1;
	} else if (strStartsWith(s, "+ECIPH")) {
		onCipherIndication((char *)s, rid);
		return 1;
	}
#ifdef MTK_VT3G324M_SUPPORT
	else if (strStartsWith(s, "+EVTSTATUS")) {
		onVtStatusInfo((char *)s, rid);
		return 1;
	}
#endif
	return 0;
	/* MTK proprietary end */
}

