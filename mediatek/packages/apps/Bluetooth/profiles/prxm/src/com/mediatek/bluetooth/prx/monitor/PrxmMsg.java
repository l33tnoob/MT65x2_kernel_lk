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

package com.mediatek.bluetooth.prx.monitor;

public interface PrxmMsg {

/************************************************************************************************
 * message id constants from native
 ************************************************************************************************/

	// Proximity - Monitor
	public static final int MSG_ID_BT_PRXM_GROUP_START = 2900;
	public static final int MSG_ID_BT_PRXM_CONNECT_REQ = 2900;
	public static final int MSG_ID_BT_PRXM_CONNECT_CNF = 2901;
	public static final int MSG_ID_BT_PRXM_DISCONNECT_REQ = 2902;
	public static final int MSG_ID_BT_PRXM_DISCONNECT_IND = 2903;
	public static final int MSG_ID_BT_PRXM_GET_CAPABILITY_REQ = 2904;
	public static final int MSG_ID_BT_PRXM_GET_CAPABILITY_CNF = 2905;
	public static final int MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_REQ = 2906;
	public static final int MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF = 2907;
	public static final int MSG_ID_BT_PRXM_SET_PATHLOSS_REQ = 2908;
	public static final int MSG_ID_BT_PRXM_SET_PATHLOSS_CNF = 2909;
	public static final int MSG_ID_BT_PRXM_SET_LINKLOSS_REQ = 2910;
	public static final int MSG_ID_BT_PRXM_SET_LINKLOSS_CNF = 2911;
	public static final int MSG_ID_BT_PRXM_GET_RSSI_REQ = 2912;
	public static final int MSG_ID_BT_PRXM_GET_RSSI_CNF = 2913;
	public static final int MSG_ID_BT_PRXM_GET_LINKLOSS_REQ = 2914;
	public static final int MSG_ID_BT_PRXM_GET_LINKLOSS_CNF = 2915;
	public static final int MSG_ID_BT_PRXM_GROUP_END = 2916;

/************************************************************************************************
 * message id constants from native
 ************************************************************************************************/

	public static final int[] PRXM_CONNECT_REQ={MSG_ID_BT_PRXM_CONNECT_REQ,12};
	public static final int PRXM_CONNECT_REQ_BA_ADDR=5;
	public static final int PRXM_CONNECT_REQ_BL_ADDR=6;

	public static final int[] PRXM_CONNECT_CNF={MSG_ID_BT_PRXM_CONNECT_CNF,6};
	public static final int PRXM_CONNECT_CNF_B_RSPCODE=5;

	public static final int[] PRXM_DISCONNECT_REQ={MSG_ID_BT_PRXM_DISCONNECT_REQ,6};

	public static final int[] PRXM_DISCONNECT_IND={MSG_ID_BT_PRXM_DISCONNECT_IND,6};
	public static final int PRXM_DISCONNECT_IND_B_RSPCODE=5;

	public static final int[] PRXM_GET_CAPABILITY_REQ={MSG_ID_BT_PRXM_GET_CAPABILITY_REQ,6};

	public static final int[] PRXM_GET_CAPABILITY_CNF={MSG_ID_BT_PRXM_GET_CAPABILITY_CNF,12};
	public static final int PRXM_GET_CAPABILITY_CNF_B_RSPCODE=5;
	public static final int PRXM_GET_CAPABILITY_CNF_I_CAPABILITY=2;

	public static final int[] PRXM_GET_REMOTE_TXPOWER_REQ={MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_REQ,6};

	public static final int[] PRXM_GET_REMOTE_TXPOWER_CNF={MSG_ID_BT_PRXM_GET_REMOTE_TXPOWER_CNF,8};
	public static final int PRXM_GET_REMOTE_TXPOWER_CNF_B_RSPCODE=5;
	public static final int PRXM_GET_REMOTE_TXPOWER_CNF_B_TXPOWER=6;

	public static final int[] PRXM_SET_PATHLOSS_REQ={MSG_ID_BT_PRXM_SET_PATHLOSS_REQ,6};
	public static final int PRXM_SET_PATHLOSS_REQ_B_LEVEL=5;

	public static final int[] PRXM_SET_PATHLOSS_CNF={MSG_ID_BT_PRXM_SET_PATHLOSS_CNF,6};
	public static final int PRXM_SET_PATHLOSS_CNF_B_RSPCODE=5;

	public static final int[] PRXM_SET_LINKLOSS_REQ={MSG_ID_BT_PRXM_SET_LINKLOSS_REQ,6};
	public static final int PRXM_SET_LINKLOSS_REQ_B_LEVEL=5;

	public static final int[] PRXM_SET_LINKLOSS_CNF={MSG_ID_BT_PRXM_SET_LINKLOSS_CNF,6};
	public static final int PRXM_SET_LINKLOSS_CNF_B_RSPCODE=5;

	public static final int[] PRXM_GET_RSSI_REQ={MSG_ID_BT_PRXM_GET_RSSI_REQ,6};

	public static final int[] PRXM_GET_RSSI_CNF={MSG_ID_BT_PRXM_GET_RSSI_CNF,8};
	public static final int PRXM_GET_RSSI_CNF_B_RSPCODE=5;
	public static final int PRXM_GET_RSSI_CNF_B_RSSI=6;

	public static final int[] PRXM_GET_LINKLOSS_REQ={MSG_ID_BT_PRXM_GET_LINKLOSS_REQ,6};

	public static final int[] PRXM_GET_LINKLOSS_CNF={MSG_ID_BT_PRXM_GET_LINKLOSS_CNF,8};
	public static final int PRXM_GET_LINKLOSS_CNF_B_RSPCODE=5;
	public static final int PRXM_GET_LINKLOSS_CNF_B_LEVEL=6;
}
