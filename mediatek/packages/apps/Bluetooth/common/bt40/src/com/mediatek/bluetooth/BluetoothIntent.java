/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth;

/**
 * TODO change parameters type (using java.nio to replace String[])
 */
public interface BluetoothIntent {

    // <--- handover begin --->
    /**
     * the intent that whitelists a remote bluetooth device for auto-receive
     * confirmation (NFC)
     */
    String ACTION_WHITELIST_DEVICE = "android.btopp.intent.action.WHITELIST_DEVICE";

    /**
     * the intent that can be sent by handover requesters to stop a BTOPP
     * transfer
     */
    String ACTION_STOP_HANDOVER = "android.btopp.intent.action.STOP_HANDOVER_TRANSFER";

    /** the intent that is used for initiating a handover transfer */
    String ACTION_HANDOVER_SEND = "android.btopp.intent.action.HANDOVER_SEND";

    /** the intent that is used for initiating a multi-uri handover transfer */
    String ACTION_HANDOVER_SEND_MULTIPLE = "android.btopp.intent.action.HANDOVER_SEND_MULTIPLE";

    /** intent action used to indicate the progress of a handover transfer */
    String ACTION_BT_OPP_TRANSFER_PROGRESS = "android.btopp.intent.action.BT_OPP_TRANSFER_PROGRESS";

    /** intent action used to indicate the completion of a handover transfer */
    String ACTION_BT_OPP_TRANSFER_DONE = "android.btopp.intent.action.BT_OPP_TRANSFER_DONE";

    /** intent extra used to indicate the success of a handover transfer */
   String EXTRA_BT_OPP_TRANSFER_STATUS = "android.btopp.intent.extra.BT_OPP_TRANSFER_STATUS";

    /** intent extra used to indicate the address associated with the transfer */
    String EXTRA_BT_OPP_ADDRESS = "android.btopp.intent.extra.BT_OPP_ADDRESS";

    int HANDOVER_TRANSFER_STATUS_SUCCESS = 0;

    int HANDOVER_TRANSFER_STATUS_FAILURE = 1;

    /** intent extra used to indicate the direction of a handover transfer */
    String EXTRA_BT_OPP_TRANSFER_DIRECTION = "android.btopp.intent.extra.BT_OPP_TRANSFER_DIRECTION";

    int DIRECTION_BLUETOOTH_INCOMING = 0;

   int DIRECTION_BLUETOOTH_OUTGOING = 1;

    /** intent extra used to provide a unique ID for the transfer */
    String EXTRA_BT_OPP_TRANSFER_ID = "android.btopp.intent.extra.BT_OPP_TRANSFER_ID";

    /** intent extra used to provide progress of the transfer */
    String EXTRA_BT_OPP_TRANSFER_PROGRESS = "android.btopp.intent.extra.BT_OPP_TRANSFER_PROGRESS";

    /**
     * intent extra used to provide the Uri where the data was stored by the
     * handover transfer
     */
    String EXTRA_BT_OPP_TRANSFER_URI = "android.btopp.intent.extra.BT_OPP_TRANSFER_URI";

    /**
     * intent extra used to provide the mime-type of the data in the handover
     * transfer
     */
    String EXTRA_BT_OPP_TRANSFER_MIMETYPE = "android.btopp.intent.extra.BT_OPP_TRANSFER_MIMETYPE";

    /** permission needed to be able to receive handover status requests */
    String HANDOVER_STATUS_PERMISSION = "com.android.permission.HANDOVER_STATUS";

    /**
     * intent extra that indicates this transfer is a handover from another
     * transport (NFC, WIFI)
     */
    String EXTRA_CONNECTION_HANDOVER = "com.android.intent.extra.CONNECTION_HANDOVER";

    String EXTRA_BT_OPP_TRANSFER_OBJECT_NAME = "com.mediatek.bluetooth.opp.extra.BT_OPP_TRANSFER_OBJECT_NAME";

    String EXTRA_BT_OPP_TRANSFER_FILE_SIZE = "com.mediatek.bluetooth.opp.extra.BT_OPP_TRANSFER_FILE_SIZE";

    String EXTRA_BT_OPP_TRANSFER_DONE_SIZE = "com.mediatek.bluetooth.opp.extra.BT_OPP_TRANSFER_DONE_SIZE";

    // <--- handover end --->

    public interface OppService {

        String ACTION_OPPC_START = "com.mediatek.bluetooth.opp.action.OPPC_START";

        String ACTION_OPPS_START = "com.mediatek.bluetooth.opp.action.OPPS_START";
    }

    public interface OppsAccessRequest {

        String ACTION_PUSH_REQUEST = "com.mediatek.bluetooth.opp.action.PUSH_REQUEST";

        String ACTION_PULL_REQUEST = "com.mediatek.bluetooth.opp.action.PULL_REQUEST";

        String EXTRA_PEER_NAME = "com.mediatek.bluetooth.opp.extra.PEER_NAME";

        String EXTRA_OBJECT_NAME = "com.mediatek.bluetooth.opp.extra.OBJECT_NAME";

        String EXTRA_TOTAL_BYTES = "com.mediatek.bluetooth.opp.extra.TOTAL_BYTES";

        String EXTRA_TASK_ID = "com.mediatek.bluetooth.opp.extra.TASK_ID";
    }
}
