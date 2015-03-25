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

package com.mediatek.bluetooth.prx.reporter;

public interface PrxrMsg {

/************************************************************************************************
 * message id constants from native
 ************************************************************************************************/

	// Proximity - Reporter
	public static final int MSG_ID_BT_PRXR_GROUP_START = 3000;
	public static final int MSG_ID_BT_PRXR_REGISTER_REQ = 3000;
	public static final int MSG_ID_BT_PRXR_REGISTER_CNF = 3001;
	public static final int MSG_ID_BT_PRXR_DEREGISTER_REQ = 3002;
	public static final int MSG_ID_BT_PRXR_DEREGISTER_CNF = 3003;
	public static final int MSG_ID_BT_PRXR_AUTHORIZE_IND = 3004;
	public static final int MSG_ID_BT_PRXR_AUTHORIZE_RSP = 3005;
	public static final int MSG_ID_BT_PRXR_CONNECT_IND = 3006;
	public static final int MSG_ID_BT_PRXR_DISCONNECT_REQ = 3007;
	public static final int MSG_ID_BT_PRXR_DISCONNECT_IND = 3008;
	public static final int MSG_ID_BT_PRXR_PATHLOSS_IND = 3009;
	public static final int MSG_ID_BT_PRXR_LINKLOSS_IND = 3010;
	public static final int MSG_ID_BT_PRXR_UPDATE_TXPOWER_REQ = 3011;
	public static final int MSG_ID_BT_PRXR_UPDATE_TXPOWER_CNF = 3012;
	public static final int MSG_ID_BT_PRXR_GROUP_END = 3013;

/************************************************************************************************
 * message id constants from native
 ************************************************************************************************/

	public static final int[] PRXR_REGISTER_REQ={MSG_ID_BT_PRXR_REGISTER_REQ,6};

	public static final int[] PRXR_REGISTER_CNF={MSG_ID_BT_PRXR_REGISTER_CNF,6};
	public static final int PRXR_REGISTER_CNF_B_RSPCODE=5;

	public static final int[] PRXR_DEREGISTER_REQ={MSG_ID_BT_PRXR_DEREGISTER_REQ,6};

	public static final int[] PRXR_DEREGISTER_CNF={MSG_ID_BT_PRXR_DEREGISTER_CNF,6};
	public static final int PRXR_DEREGISTER_CNF_B_RSPCODE=5;

	public static final int[] PRXR_AUTHORIZE_IND={MSG_ID_BT_PRXR_AUTHORIZE_IND,12};
	public static final int PRXR_AUTHORIZE_IND_BA_ADDR=5;
	public static final int PRXR_AUTHORIZE_IND_BL_ADDR=6;

	public static final int[] PRXR_AUTHORIZE_RSP={MSG_ID_BT_PRXR_AUTHORIZE_RSP,6};
	public static final int PRXR_AUTHORIZE_RSP_B_RSPCODE=5;

	public static final int[] PRXR_CONNECT_IND={MSG_ID_BT_PRXR_CONNECT_IND,12};
	public static final int PRXR_CONNECT_IND_BA_ADDR=5;
	public static final int PRXR_CONNECT_IND_BL_ADDR=6;

	public static final int[] PRXR_DISCONNECT_REQ={MSG_ID_BT_PRXR_DISCONNECT_REQ,6};

	public static final int[] PRXR_DISCONNECT_IND={MSG_ID_BT_PRXR_DISCONNECT_IND,6};
	public static final int PRXR_DISCONNECT_IND_B_RSPCODE=5;

	public static final int[] PRXR_PATHLOSS_IND={MSG_ID_BT_PRXR_PATHLOSS_IND,6};
	public static final int PRXR_PATHLOSS_IND_B_LEVEL=5;

	public static final int[] PRXR_LINKLOSS_IND={MSG_ID_BT_PRXR_LINKLOSS_IND,6};
	public static final int PRXR_LINKLOSS_IND_B_LEVEL=5;

	public static final int[] PRXR_UPDATE_TXPOWER_REQ={MSG_ID_BT_PRXR_UPDATE_TXPOWER_REQ,6};

	public static final int[] PRXR_UPDATE_TXPOWER_CNF={MSG_ID_BT_PRXR_UPDATE_TXPOWER_CNF,6};
	public static final int PRXR_UPDATE_TXPOWER_CNF_B_TXPOWER=5;
}
