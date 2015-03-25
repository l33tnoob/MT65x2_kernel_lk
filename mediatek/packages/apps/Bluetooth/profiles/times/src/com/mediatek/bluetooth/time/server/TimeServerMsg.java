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

package com.mediatek.bluetooth.time.server;

public interface TimeServerMsg {

/************************************************************************************************
 * Message id constants from native
 ************************************************************************************************/
	public static final int MSG_ID_BT_TIMES_GROUP_START						= 3400,
							MSG_ID_BT_TIMES_REGISTER_REQ					= 3400,
							MSG_ID_BT_TIMES_REGISTER_CNF					= 3401,
							MSG_ID_BT_TIMES_DEREGISTER_REQ					= 3402,
							MSG_ID_BT_TIMES_DEREGISTER_CNF					= 3403,
							MSG_ID_BT_TIMES_AUTHORIZE_IND					= 3404,
							MSG_ID_BT_TIMES_AUTHORIZE_RSP					= 3405,
							MSG_ID_BT_TIMES_CONNECT_IND						= 3406,
							MSG_ID_BT_TIMES_DISCONNECT_REQ					= 3407,
							MSG_ID_BT_TIMES_DISCONNECT_IND					= 3408,
							MSG_ID_BT_TIMES_GET_CTTIME_IND					= 3409,
							MSG_ID_BT_TIMES_GET_CTTIME_RSP					= 3410,
							MSG_ID_BT_TIMES_GET_CTTIME_NOTIFY_IND			= 3411,
							MSG_ID_BT_TIMES_GET_CTTIME_NOTIFY_RSP			= 3412,
							MSG_ID_BT_TIMES_SET_CTTIME_NOTIFY_IND			= 3413,
							MSG_ID_BT_TIMES_SET_CTTIME_NOTIFY_RSP			= 3414,
							MSG_ID_BT_TIMES_UPDATE_CTTIME_REQ				= 3415,
							MSG_ID_BT_TIMES_UPDATE_CTTIME_CNF				= 3416,
							MSG_ID_BT_TIMES_SET_LOCAL_TIME_INFO_REQ			= 3417,
							MSG_ID_BT_TIMES_SET_LOCAL_TIME_INFO_CNF			= 3418,
							MSG_ID_BT_TIMES_SET_REF_TIME_INFO_REQ			= 3419,
							MSG_ID_BT_TIMES_SET_REF_TIME_INFO_CNF			= 3420,
							MSG_ID_BT_TIMES_SET_DST_REQ						= 3421,
							MSG_ID_BT_TIMES_SET_DST_CNF						= 3422,
							MSG_ID_BT_TIMES_REQUEST_SERVER_UPDATE_IND		= 3423,
							MSG_ID_BT_TIMES_REQUEST_SERVER_UPDATE_RSP		= 3424,
							MSG_ID_BT_TIMES_CANCEL_SERVER_UPDATE_IND		= 3425,
							MSG_ID_BT_TIMES_CANCEL_SERVER_UPDATE_RSP		= 3426,
							MSG_ID_BT_TIMES_GET_SERVER_UPDATE_STATUS_IND	= 3427,
							MSG_ID_BT_TIMES_GET_SERVER_UPDATE_STATUS_RSP	= 3428,
							MSG_ID_BT_TIMES_GROUP_END						= 3429;

/************************************************************************************************
 * Message int array flattened from native message structs
 ************************************************************************************************/
	public static final int[] TIMES_REGISTER_REQ={MSG_ID_BT_TIMES_REGISTER_REQ,6};
	public static final int TIMES_REGISTER_REQ_B_REF_COUNT=0;
	public static final int TIMES_REGISTER_REQ_S_MSG_LEN=1;
	public static final int TIMES_REGISTER_REQ_B_INDEX=4;

	public static final int[] TIMES_REGISTER_CNF={MSG_ID_BT_TIMES_REGISTER_CNF,6};
	public static final int TIMES_REGISTER_CNF_B_REF_COUNT=0;
	public static final int TIMES_REGISTER_CNF_S_MSG_LEN=1;
	public static final int TIMES_REGISTER_CNF_B_INDEX=4;
	public static final int TIMES_REGISTER_CNF_B_RSPCODE=5;

	public static final int[] TIMES_DEREGISTER_REQ={MSG_ID_BT_TIMES_DEREGISTER_REQ,6};
	public static final int TIMES_DEREGISTER_REQ_B_REF_COUNT=0;
	public static final int TIMES_DEREGISTER_REQ_S_MSG_LEN=1;
	public static final int TIMES_DEREGISTER_REQ_B_INDEX=4;

	public static final int[] TIMES_DEREGISTER_CNF={MSG_ID_BT_TIMES_DEREGISTER_CNF,6};
	public static final int TIMES_DEREGISTER_CNF_B_REF_COUNT=0;
	public static final int TIMES_DEREGISTER_CNF_S_MSG_LEN=1;
	public static final int TIMES_DEREGISTER_CNF_B_INDEX=4;
	public static final int TIMES_DEREGISTER_CNF_B_RSPCODE=5;

	public static final int[] TIMES_AUTHORIZE_IND={MSG_ID_BT_TIMES_AUTHORIZE_IND,12};
	public static final int TIMES_AUTHORIZE_IND_B_REF_COUNT=0;
	public static final int TIMES_AUTHORIZE_IND_S_MSG_LEN=1;
	public static final int TIMES_AUTHORIZE_IND_B_INDEX=4;
	public static final int TIMES_AUTHORIZE_IND_BA_ADDR=5;
	public static final int TIMES_AUTHORIZE_IND_BL_ADDR=6;

	public static final int[] TIMES_AUTHORIZE_RSP={MSG_ID_BT_TIMES_AUTHORIZE_RSP,6};
	public static final int TIMES_AUTHORIZE_RSP_B_REF_COUNT=0;
	public static final int TIMES_AUTHORIZE_RSP_S_MSG_LEN=1;
	public static final int TIMES_AUTHORIZE_RSP_B_INDEX=4;
	public static final int TIMES_AUTHORIZE_RSP_B_RSPCODE=5;

	public static final int[] TIMES_CONNECT_IND={MSG_ID_BT_TIMES_CONNECT_IND,12};
	public static final int TIMES_CONNECT_IND_B_REF_COUNT=0;
	public static final int TIMES_CONNECT_IND_S_MSG_LEN=1;
	public static final int TIMES_CONNECT_IND_B_INDEX=4;
	public static final int TIMES_CONNECT_IND_BA_ADDR=5;
	public static final int TIMES_CONNECT_IND_BL_ADDR=6;

	public static final int[] TIMES_DISCONNECT_REQ={MSG_ID_BT_TIMES_DISCONNECT_REQ,6};
	public static final int TIMES_DISCONNECT_REQ_B_REF_COUNT=0;
	public static final int TIMES_DISCONNECT_REQ_S_MSG_LEN=1;
	public static final int TIMES_DISCONNECT_REQ_B_INDEX=4;

	public static final int[] TIMES_DISCONNECT_IND={MSG_ID_BT_TIMES_DISCONNECT_IND,6};
	public static final int TIMES_DISCONNECT_IND_B_REF_COUNT=0;
	public static final int TIMES_DISCONNECT_IND_S_MSG_LEN=1;
	public static final int TIMES_DISCONNECT_IND_B_INDEX=4;
	public static final int TIMES_DISCONNECT_IND_B_RSPCODE=5;

	public static final int[] TIMES_GET_CTTIME_IND={MSG_ID_BT_TIMES_GET_CTTIME_IND,6};
	public static final int TIMES_GET_CTTIME_IND_B_REF_COUNT=0;
	public static final int TIMES_GET_CTTIME_IND_S_MSG_LEN=1;
	public static final int TIMES_GET_CTTIME_IND_B_INDEX=4;

	public static final int[] TIMES_GET_CTTIME_RSP={MSG_ID_BT_TIMES_GET_CTTIME_RSP,16};
	public static final int TIMES_GET_CTTIME_RSP_B_REF_COUNT=0;
	public static final int TIMES_GET_CTTIME_RSP_S_MSG_LEN=1;
	public static final int TIMES_GET_CTTIME_RSP_B_INDEX=4;
	public static final int TIMES_GET_CTTIME_RSP_B_RSPCODE=5;
	public static final int TIMES_GET_CTTIME_RSP_S_YEAR=3;
	public static final int TIMES_GET_CTTIME_RSP_B_MONTH=8;
	public static final int TIMES_GET_CTTIME_RSP_B_DAY=9;
	public static final int TIMES_GET_CTTIME_RSP_B_HOURS=10;
	public static final int TIMES_GET_CTTIME_RSP_B_MINUTES=11;
	public static final int TIMES_GET_CTTIME_RSP_B_SECONDS=12;
	public static final int TIMES_GET_CTTIME_RSP_B_DAY_OF_WEEK=13;
	public static final int TIMES_GET_CTTIME_RSP_B_FRAC256=14;
	public static final int TIMES_GET_CTTIME_RSP_B_ADJUST_REASON=15;

	public static final int[] TIMES_GET_CTTIME_NOTIFY_IND={MSG_ID_BT_TIMES_GET_CTTIME_NOTIFY_IND,6};
	public static final int TIMES_GET_CTTIME_NOTIFY_IND_B_REF_COUNT=0;
	public static final int TIMES_GET_CTTIME_NOTIFY_IND_S_MSG_LEN=1;
	public static final int TIMES_GET_CTTIME_NOTIFY_IND_B_INDEX=4;

	public static final int[] TIMES_GET_CTTIME_NOTIFY_RSP={MSG_ID_BT_TIMES_GET_CTTIME_NOTIFY_RSP,8};
	public static final int TIMES_GET_CTTIME_NOTIFY_RSP_B_REF_COUNT=0;
	public static final int TIMES_GET_CTTIME_NOTIFY_RSP_S_MSG_LEN=1;
	public static final int TIMES_GET_CTTIME_NOTIFY_RSP_B_INDEX=4;
	public static final int TIMES_GET_CTTIME_NOTIFY_RSP_B_RSPCODE=5;
	public static final int TIMES_GET_CTTIME_NOTIFY_RSP_B_NOTIFY_CONFIG=6;

	public static final int[] TIMES_SET_CTTIME_NOTIFY_IND={MSG_ID_BT_TIMES_SET_CTTIME_NOTIFY_IND,6};
	public static final int TIMES_SET_CTTIME_NOTIFY_IND_B_REF_COUNT=0;
	public static final int TIMES_SET_CTTIME_NOTIFY_IND_S_MSG_LEN=1;
	public static final int TIMES_SET_CTTIME_NOTIFY_IND_B_INDEX=4;
	public static final int TIMES_SET_CTTIME_NOTIFY_IND_B_NOTIFY_CONFIG=5;

	public static final int[] TIMES_SET_CTTIME_NOTIFY_RSP={MSG_ID_BT_TIMES_SET_CTTIME_NOTIFY_RSP,6};
	public static final int TIMES_SET_CTTIME_NOTIFY_RSP_B_REF_COUNT=0;
	public static final int TIMES_SET_CTTIME_NOTIFY_RSP_S_MSG_LEN=1;
	public static final int TIMES_SET_CTTIME_NOTIFY_RSP_B_INDEX=4;
	public static final int TIMES_SET_CTTIME_NOTIFY_RSP_B_RSPCODE=5;

	public static final int[] TIMES_UPDATE_CTTIME_REQ={MSG_ID_BT_TIMES_UPDATE_CTTIME_REQ,16};
	public static final int TIMES_UPDATE_CTTIME_REQ_B_REF_COUNT=0;
	public static final int TIMES_UPDATE_CTTIME_REQ_S_MSG_LEN=1;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_INDEX=4;
	public static final int TIMES_UPDATE_CTTIME_REQ_S_YEAR=3;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_MONTH=8;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_DAY=9;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_HOURS=10;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_MINUTES=11;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_SECONDS=12;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_DAY_OF_WEEK=13;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_FRAC256=14;
	public static final int TIMES_UPDATE_CTTIME_REQ_B_ADJUST_REASON=15;

	public static final int[] TIMES_UPDATE_CTTIME_CNF={MSG_ID_BT_TIMES_UPDATE_CTTIME_CNF,6};
	public static final int TIMES_UPDATE_CTTIME_CNF_B_REF_COUNT=0;
	public static final int TIMES_UPDATE_CTTIME_CNF_S_MSG_LEN=1;
	public static final int TIMES_UPDATE_CTTIME_CNF_B_INDEX=4;
	public static final int TIMES_UPDATE_CTTIME_CNF_B_RSPCODE=5;

	public static final int[] TIMES_SET_LOCAL_TIME_INFO_REQ={MSG_ID_BT_TIMES_SET_LOCAL_TIME_INFO_REQ,8};
	public static final int TIMES_SET_LOCAL_TIME_INFO_REQ_B_REF_COUNT=0;
	public static final int TIMES_SET_LOCAL_TIME_INFO_REQ_S_MSG_LEN=1;
	public static final int TIMES_SET_LOCAL_TIME_INFO_REQ_B_INDEX=4;
	public static final int TIMES_SET_LOCAL_TIME_INFO_REQ_B_TIME_ZONE=5;
	public static final int TIMES_SET_LOCAL_TIME_INFO_REQ_B_DST=6;

	public static final int[] TIMES_SET_LOCAL_TIME_INFO_CNF={MSG_ID_BT_TIMES_SET_LOCAL_TIME_INFO_CNF,6};
	public static final int TIMES_SET_LOCAL_TIME_INFO_CNF_B_REF_COUNT=0;
	public static final int TIMES_SET_LOCAL_TIME_INFO_CNF_S_MSG_LEN=1;
	public static final int TIMES_SET_LOCAL_TIME_INFO_CNF_B_INDEX=4;
	public static final int TIMES_SET_LOCAL_TIME_INFO_CNF_B_RSPCODE=5;

	public static final int[] TIMES_SET_REF_TIME_INFO_REQ={MSG_ID_BT_TIMES_SET_REF_TIME_INFO_REQ,10};
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_REF_COUNT=0;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_S_MSG_LEN=1;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_INDEX=4;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_TIME_SOURCE=5;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_ACCURACY=6;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_DAYS_SINCE_UPDATE=7;
	public static final int TIMES_SET_REF_TIME_INFO_REQ_B_HOURS_SINCE_UPDATE=8;

	public static final int[] TIMES_SET_REF_TIME_INFO_CNF={MSG_ID_BT_TIMES_SET_REF_TIME_INFO_CNF,6};
	public static final int TIMES_SET_REF_TIME_INFO_CNF_B_REF_COUNT=0;
	public static final int TIMES_SET_REF_TIME_INFO_CNF_S_MSG_LEN=1;
	public static final int TIMES_SET_REF_TIME_INFO_CNF_B_INDEX=4;
	public static final int TIMES_SET_REF_TIME_INFO_CNF_B_RSPCODE=5;

	public static final int[] TIMES_SET_DST_REQ={MSG_ID_BT_TIMES_SET_DST_REQ,14};
	public static final int TIMES_SET_DST_REQ_B_REF_COUNT=0;
	public static final int TIMES_SET_DST_REQ_S_MSG_LEN=1;
	public static final int TIMES_SET_DST_REQ_B_INDEX=4;
	public static final int TIMES_SET_DST_REQ_S_YEAR=3;
	public static final int TIMES_SET_DST_REQ_B_MONTH=8;
	public static final int TIMES_SET_DST_REQ_B_DAY=9;
	public static final int TIMES_SET_DST_REQ_B_HOURS=10;
	public static final int TIMES_SET_DST_REQ_B_MINUTES=11;
	public static final int TIMES_SET_DST_REQ_B_SECONDS=12;
	public static final int TIMES_SET_DST_REQ_B_DST=13;

	public static final int[] TIMES_SET_DST_CNF={MSG_ID_BT_TIMES_SET_DST_CNF,6};
	public static final int TIMES_SET_DST_CNF_B_REF_COUNT=0;
	public static final int TIMES_SET_DST_CNF_S_MSG_LEN=1;
	public static final int TIMES_SET_DST_CNF_B_INDEX=4;
	public static final int TIMES_SET_DST_CNF_B_REPCODE=5;

	public static final int[] TIMES_REQUEST_SERVER_UPDATE_IND={MSG_ID_BT_TIMES_REQUEST_SERVER_UPDATE_IND,6};
	public static final int TIMES_REQUEST_SERVER_UPDATE_IND_B_REF_COUNT=0;
	public static final int TIMES_REQUEST_SERVER_UPDATE_IND_S_MSG_LEN=1;
	public static final int TIMES_REQUEST_SERVER_UPDATE_IND_B_INDEX=4;

	public static final int[] TIMES_REQUEST_SERVER_UPDATE_RSP={MSG_ID_BT_TIMES_REQUEST_SERVER_UPDATE_RSP,6};
	public static final int TIMES_REQUEST_SERVER_UPDATE_RSP_B_REF_COUNT=0;
	public static final int TIMES_REQUEST_SERVER_UPDATE_RSP_S_MSG_LEN=1;
	public static final int TIMES_REQUEST_SERVER_UPDATE_RSP_B_INDEX=4;
	public static final int TIMES_REQUEST_SERVER_UPDATE_RSP_B_RSPCODE=5;

	public static final int[] TIMES_CANCEL_SERVER_UPDATE_IND={MSG_ID_BT_TIMES_CANCEL_SERVER_UPDATE_IND,6};
	public static final int TIMES_CANCEL_SERVER_UPDATE_IND_B_REF_COUNT=0;
	public static final int TIMES_CANCEL_SERVER_UPDATE_IND_S_MSG_LEN=1;
	public static final int TIMES_CANCEL_SERVER_UPDATE_IND_B_INDEX=4;

	public static final int[] TIMES_CANCEL_SERVER_UPDATE_RSP={MSG_ID_BT_TIMES_CANCEL_SERVER_UPDATE_RSP,6};
	public static final int TIMES_CANCEL_SERVER_UPDATE_RSP_B_REF_COUNT=0;
	public static final int TIMES_CANCEL_SERVER_UPDATE_RSP_S_MSG_LEN=1;
	public static final int TIMES_CANCEL_SERVER_UPDATE_RSP_B_INDEX=4;
	public static final int TIMES_CANCEL_SERVER_UPDATE_RSP_B_RSPCODE=5;

	public static final int[] TIMES_GET_SERVER_UPDATE_STATUS_IND={MSG_ID_BT_TIMES_GET_SERVER_UPDATE_STATUS_IND,6};
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_IND_B_REF_COUNT=0;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_IND_S_MSG_LEN=1;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_IND_B_INDEX=4;

	public static final int[] TIMES_GET_SERVER_UPDATE_STATUS_RSP={MSG_ID_BT_TIMES_GET_SERVER_UPDATE_STATUS_RSP,8};
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_B_REF_COUNT=0;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_S_MSG_LEN=1;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_B_INDEX=4;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_B_RSPCODE=5;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_B_CUR_STATE=6;
	public static final int TIMES_GET_SERVER_UPDATE_STATUS_RSP_B_RESULT=7;


}
