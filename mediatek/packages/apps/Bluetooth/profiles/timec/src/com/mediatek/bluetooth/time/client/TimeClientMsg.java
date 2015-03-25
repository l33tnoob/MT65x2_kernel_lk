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

package com.mediatek.bluetooth.time.client;

public interface TimeClientMsg {

/************************************************************************************************
 * Message id constants from native
 ************************************************************************************************/
	public static final int MSG_ID_BT_TIMEC_GROUP_START						= 3300,
							MSG_ID_BT_TIMEC_CONNECT_REQ						= 3300,
							MSG_ID_BT_TIMEC_CONNECT_CNF						= 3301,
							MSG_ID_BT_TIMEC_DISCONNECT_REQ					= 3302,
							MSG_ID_BT_TIMEC_DISCONNECT_IND					= 3303,
							MSG_ID_BT_TIMEC_GET_CTTIME_REQ					= 3304,
							MSG_ID_BT_TIMEC_GET_CTTIME_CNF					= 3305,
							MSG_ID_BT_TIMEC_GET_CTTIME_NOTIFY_REQ			= 3306,
							MSG_ID_BT_TIMEC_GET_CTTIME_NOTIFY_CNF			= 3307,
							MSG_ID_BT_TIMEC_SET_CTTIME_NOTIFY_REQ			= 3308,
							MSG_ID_BT_TIMEC_SET_CTTIME_NOTIFY_CNF			= 3309,
							MSG_ID_BT_TIMEC_UPDATE_CTTIME_IND				= 3310,
							MSG_ID_BT_TIMEC_UPDATE_CTTIME_RSP				= 3311,
							MSG_ID_BT_TIMEC_GET_LOCAL_TIME_INFO_REQ			= 3312,
							MSG_ID_BT_TIMEC_GET_LOCAL_TIME_INFO_CNF			= 3313,
							MSG_ID_BT_TIMEC_GET_REF_TIME_INFO_REQ			= 3314,
							MSG_ID_BT_TIMEC_GET_REF_TIME_INFO_CNF			= 3315,
							MSG_ID_BT_TIMEC_GET_DST_REQ						= 3316,
							MSG_ID_BT_TIMEC_GET_DST_CNF						= 3317,
							MSG_ID_BT_TIMEC_REQUEST_SERVER_UPDATE_REQ		= 3318,
							MSG_ID_BT_TIMEC_REQUEST_SERVER_UPDATE_CNF		= 3319,
							MSG_ID_BT_TIMEC_CANCEL_SERVER_UPDATE_REQ		= 3320,
							MSG_ID_BT_TIMEC_CANCEL_SERVER_UPDATE_CNF		= 3321,
							MSG_ID_BT_TIMEC_GET_SERVER_UPDATE_STATUS_REQ	= 3322,
							MSG_ID_BT_TIMEC_GET_SERVER_UPDATE_STATUS_CNF	= 3323,
							MSG_ID_BT_TIMEC_GROUP_END						= 3324;

/************************************************************************************************
 * Message int array flattened from native message structs
 ************************************************************************************************/
	public static final int[] TIMEC_CONNECT_REQ={MSG_ID_BT_TIMEC_CONNECT_REQ,12};
	public static final int TIMEC_CONNECT_REQ_B_REF_COUNT=0;
	public static final int TIMEC_CONNECT_REQ_S_MSG_LEN=1;
	public static final int TIMEC_CONNECT_REQ_B_INDEX=4;
	public static final int TIMEC_CONNECT_REQ_BA_ADDR=5;
	public static final int TIMEC_CONNECT_REQ_BL_ADDR=6;

	public static final int[] TIMEC_CONNECT_CNF={MSG_ID_BT_TIMEC_CONNECT_CNF,6};
	public static final int TIMEC_CONNECT_CNF_B_REF_COUNT=0;
	public static final int TIMEC_CONNECT_CNF_S_MSG_LEN=1;
	public static final int TIMEC_CONNECT_CNF_B_INDEX=4;
	public static final int TIMEC_CONNECT_CNF_B_RSPCODE=5;

	public static final int[] TIMEC_DISCONNECT_REQ={MSG_ID_BT_TIMEC_DISCONNECT_REQ,6};
	public static final int TIMEC_DISCONNECT_REQ_B_REF_COUNT=0;
	public static final int TIMEC_DISCONNECT_REQ_S_MSG_LEN=1;
	public static final int TIMEC_DISCONNECT_REQ_B_INDEX=4;

	public static final int[] TIMEC_DISCONNECT_IND={MSG_ID_BT_TIMEC_DISCONNECT_IND,6};
	public static final int TIMEC_DISCONNECT_IND_B_REF_COUNT=0;
	public static final int TIMEC_DISCONNECT_IND_S_MSG_LEN=1;
	public static final int TIMEC_DISCONNECT_IND_B_INDEX=4;
	public static final int TIMEC_DISCONNECT_IND_B_RSPCODE=5;

	public static final int[] TIMEC_GET_CTTIME_REQ={MSG_ID_BT_TIMEC_GET_CTTIME_REQ,6};
	public static final int TIMEC_GET_CTTIME_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_CTTIME_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_CTTIME_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_CTTIME_CNF={MSG_ID_BT_TIMEC_GET_CTTIME_CNF,16};
	public static final int TIMEC_GET_CTTIME_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_CTTIME_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_CTTIME_CNF_B_INDEX=4;
	public static final int TIMEC_GET_CTTIME_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_CTTIME_CNF_S_YEAR=3;
	public static final int TIMEC_GET_CTTIME_CNF_B_MONTH=8;
	public static final int TIMEC_GET_CTTIME_CNF_B_DAY=9;
	public static final int TIMEC_GET_CTTIME_CNF_B_HOURS=10;
	public static final int TIMEC_GET_CTTIME_CNF_B_MINUTES=11;
	public static final int TIMEC_GET_CTTIME_CNF_B_SECONDS=12;
	public static final int TIMEC_GET_CTTIME_CNF_B_DAY_OF_WEEK=13;
	public static final int TIMEC_GET_CTTIME_CNF_B_FRAC256=14;
	public static final int TIMEC_GET_CTTIME_CNF_B_ADJUST_REASON=15;

	public static final int[] TIMEC_GET_CTTIME_NOTIFY_REQ={MSG_ID_BT_TIMEC_GET_CTTIME_NOTIFY_REQ,6};
	public static final int TIMEC_GET_CTTIME_NOTIFY_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_CTTIME_NOTIFY_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_CTTIME_NOTIFY_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_CTTIME_NOTIFY_CNF={MSG_ID_BT_TIMEC_GET_CTTIME_NOTIFY_CNF,8};
	public static final int TIMEC_GET_CTTIME_NOTIFY_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_CTTIME_NOTIFY_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_CTTIME_NOTIFY_CNF_B_INDEX=4;
	public static final int TIMEC_GET_CTTIME_NOTIFY_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_CTTIME_NOTIFY_CNF_S_NOTIFY_CONFIG=3;

	public static final int[] TIMEC_SET_CTTIME_NOTIFY_REQ={MSG_ID_BT_TIMEC_SET_CTTIME_NOTIFY_REQ,8};
	public static final int TIMEC_SET_CTTIME_NOTIFY_REQ_B_REF_COUNT=0;
	public static final int TIMEC_SET_CTTIME_NOTIFY_REQ_S_MSG_LEN=1;
	public static final int TIMEC_SET_CTTIME_NOTIFY_REQ_B_INDEX=4;
	public static final int TIMEC_SET_CTTIME_NOTIFY_REQ_S_NOTIFY_CONFIG=3;

	public static final int[] TIMEC_SET_CTTIME_NOTIFY_CNF={MSG_ID_BT_TIMEC_SET_CTTIME_NOTIFY_CNF,6};
	public static final int TIMEC_SET_CTTIME_NOTIFY_CNF_B_REF_COUNT=0;
	public static final int TIMEC_SET_CTTIME_NOTIFY_CNF_S_MSG_LEN=1;
	public static final int TIMEC_SET_CTTIME_NOTIFY_CNF_B_INDEX=4;
	public static final int TIMEC_SET_CTTIME_NOTIFY_CNF_B_RSPCODE=5;

	public static final int[] TIMEC_UPDATE_CTTIME_IND={MSG_ID_BT_TIMEC_UPDATE_CTTIME_IND,16};
	public static final int TIMEC_UPDATE_CTTIME_IND_B_REF_COUNT=0;
	public static final int TIMEC_UPDATE_CTTIME_IND_S_MSG_LEN=1;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_INDEX=4;
	public static final int TIMEC_UPDATE_CTTIME_IND_S_YEAR=3;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_MONTH=8;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_DAY=9;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_HOURS=10;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_MINUTES=11;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_SECONDS=12;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_DAY_OF_WEEK=13;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_FRAC256=14;
	public static final int TIMEC_UPDATE_CTTIME_IND_B_ADJUST_REASON=15;

	public static final int[] TIMEC_UPDATE_CTTIME_RSP={MSG_ID_BT_TIMEC_UPDATE_CTTIME_RSP,6};
	public static final int TIMEC_UPDATE_CTTIME_RSP_B_REF_COUNT=0;
	public static final int TIMEC_UPDATE_CTTIME_RSP_S_MSG_LEN=1;
	public static final int TIMEC_UPDATE_CTTIME_RSP_B_INDEX=4;
	public static final int TIMEC_UPDATE_CTTIME_RSP_B_RSPCODE=5;

	public static final int[] TIMEC_GET_LOCAL_TIME_INFO_REQ={MSG_ID_BT_TIMEC_GET_LOCAL_TIME_INFO_REQ,6};
	public static final int TIMEC_GET_LOCAL_TIME_INFO_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_LOCAL_TIME_INFO_CNF={MSG_ID_BT_TIMEC_GET_LOCAL_TIME_INFO_CNF,8};
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_B_INDEX=4;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_B_TIME_ZONE=6;
	public static final int TIMEC_GET_LOCAL_TIME_INFO_CNF_B_DST=7;

	public static final int[] TIMEC_GET_REF_TIME_INFO_REQ={MSG_ID_BT_TIMEC_GET_REF_TIME_INFO_REQ,6};
	public static final int TIMEC_GET_REF_TIME_INFO_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_REF_TIME_INFO_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_REF_TIME_INFO_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_REF_TIME_INFO_CNF={MSG_ID_BT_TIMEC_GET_REF_TIME_INFO_CNF,10};
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_INDEX=4;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_TIME_SOURCE=6;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_ACCURACY=7;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_DAYS_SINCE_UPDATE=8;
	public static final int TIMEC_GET_REF_TIME_INFO_CNF_B_HOURS_SINCE_UPDATE=9;

	public static final int[] TIMEC_GET_DST_REQ={MSG_ID_BT_TIMEC_GET_DST_REQ,6};
	public static final int TIMEC_GET_DST_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_DST_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_DST_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_DST_CNF={MSG_ID_BT_TIMEC_GET_DST_CNF,14};
	public static final int TIMEC_GET_DST_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_DST_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_DST_CNF_B_INDEX=4;
	public static final int TIMEC_GET_DST_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_DST_CNF_S_YEAR=3;
	public static final int TIMEC_GET_DST_CNF_B_MONTH=8;
	public static final int TIMEC_GET_DST_CNF_B_DAY=9;
	public static final int TIMEC_GET_DST_CNF_B_HOURS=10;
	public static final int TIMEC_GET_DST_CNF_B_MINUTES=11;
	public static final int TIMEC_GET_DST_CNF_B_SECONDS=12;
	public static final int TIMEC_GET_DST_CNF_B_DST=13;

	public static final int[] TIMEC_REQUEST_SERVER_UPDATE_REQ={MSG_ID_BT_TIMEC_REQUEST_SERVER_UPDATE_REQ,6};
	public static final int TIMEC_REQUEST_SERVER_UPDATE_REQ_B_REF_COUNT=0;
	public static final int TIMEC_REQUEST_SERVER_UPDATE_REQ_S_MSG_LEN=1;
	public static final int TIMEC_REQUEST_SERVER_UPDATE_REQ_B_INDEX=4;

	public static final int[] TIMEC_REQUEST_SERVER_UPDATE_CNF={MSG_ID_BT_TIMEC_REQUEST_SERVER_UPDATE_CNF,6};
	public static final int TIMEC_REQUEST_SERVER_UPDATE_CNF_B_REF_COUNT=0;
	public static final int TIMEC_REQUEST_SERVER_UPDATE_CNF_S_MSG_LEN=1;
	public static final int TIMEC_REQUEST_SERVER_UPDATE_CNF_B_INDEX=4;
	public static final int TIMEC_REQUEST_SERVER_UPDATE_CNF_B_RSPCODE=5;

	public static final int[] TIMEC_CANCEL_SERVER_UPDATE_REQ={MSG_ID_BT_TIMEC_CANCEL_SERVER_UPDATE_REQ,6};
	public static final int TIMEC_CANCEL_SERVER_UPDATE_REQ_B_REF_COUNT=0;
	public static final int TIMEC_CANCEL_SERVER_UPDATE_REQ_S_MSG_LEN=1;
	public static final int TIMEC_CANCEL_SERVER_UPDATE_REQ_B_INDEX=4;

	public static final int[] TIMEC_CANCEL_SERVER_UPDATE_CNF={MSG_ID_BT_TIMEC_CANCEL_SERVER_UPDATE_CNF,6};
	public static final int TIMEC_CANCEL_SERVER_UPDATE_CNF_B_REF_COUNT=0;
	public static final int TIMEC_CANCEL_SERVER_UPDATE_CNF_S_MSG_LEN=1;
	public static final int TIMEC_CANCEL_SERVER_UPDATE_CNF_B_INDEX=4;
	public static final int TIMEC_CANCEL_SERVER_UPDATE_CNF_B_RSPCODE=5;

	public static final int[] TIMEC_GET_SERVER_UPDATE_STATUS_REQ={MSG_ID_BT_TIMEC_GET_SERVER_UPDATE_STATUS_REQ,6};
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_REQ_B_REF_COUNT=0;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_REQ_S_MSG_LEN=1;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_REQ_B_INDEX=4;

	public static final int[] TIMEC_GET_SERVER_UPDATE_STATUS_CNF={MSG_ID_BT_TIMEC_GET_SERVER_UPDATE_STATUS_CNF,8};
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_B_REF_COUNT=0;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_S_MSG_LEN=1;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_B_INDEX=4;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_B_RSPCODE=5;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_B_CUR_STATE=6;
	public static final int TIMEC_GET_SERVER_UPDATE_STATUS_CNF_B_RESULT=7;

}
