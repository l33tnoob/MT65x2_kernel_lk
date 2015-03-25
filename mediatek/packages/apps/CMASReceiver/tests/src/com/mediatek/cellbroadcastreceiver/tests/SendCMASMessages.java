/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cellbroadcastreceiver.tests;

import android.content.Context;
import android.content.Intent;
import android.provider.Telephony.Sms.Intents;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.telephony.SmsCbEtwsInfo;
import android.util.Log;

import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.IccUtils;
import com.android.internal.telephony.gsm.GsmSmsCbMessage;

//import java.io.UnsupportedEncodingException;

/**
 * Send test messages.
 */
public class SendCMASMessages {

    private static String sTAG = "SendTestMessages";

    private static final int DCS_7BIT_ENGLISH = 0x01;

    private static final int DCS_16BIT_UCS2 = 0x48;

    // add for gemini
    public static final String SLOT_EXT = "simId";

    public static int sSlotId = 0;

    /* GS for dulication */
    public static final byte[] CMASMSG_PDU_001 = IccUtils.hexStringToBytes("000011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

    public static final byte[] CMASMSG_PDU_002 = IccUtils.hexStringToBytes("400011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

    public static final byte[] CMASMSG_PDU_003 = IccUtils.hexStringToBytes("800011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

    public static final byte[] CMASMSG_PDU_004 = IccUtils.hexStringToBytes("C00011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

    /* presidential */
    public static final byte[] CMASMSG_PDU_00 = IccUtils.hexStringToBytes("010011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

    /* imminent threat alert message */
    public static final byte[] CMASMSG_PDU_01 = IccUtils.hexStringToBytes("010011130111"
            + "C976BB4C4E87E96510BD3CA78362B1D80C");

    public static final byte[] CMASMSG_PDU_02 = IccUtils.hexStringToBytes("010011140111"
            + "C976BB4C4E87E96510BD3CA78362B1180D");

    public static final byte[] CMASMSG_PDU_03 = IccUtils.hexStringToBytes("010011150111"
            + "C976BB4C4E87E96510BD3CA78362B1580D");

    public static final byte[] CMASMSG_PDU_04 = IccUtils.hexStringToBytes("010011160111"
            + "C976BB4C4E87E96510BD3CA78362B1980D");

    public static final byte[] CMASMSG_PDU_05 = IccUtils.hexStringToBytes("010011170111"
            + "C976BB4C4E87E96510BD3CA78362B1D80D");

    public static final byte[] CMASMSG_PDU_06 = IccUtils.hexStringToBytes("010011180111"
            + "C976BB4C4E87E96510BD3CA78362B1180E");

    public static final byte[] CMASMSG_PDU_07 = IccUtils.hexStringToBytes("010011190111"
            + "C976BB4C4E87E96510BD3CA78362B1580E");

    public static final byte[] CMASMSG_PDU_08 = IccUtils.hexStringToBytes("0100111A0111"
            + "C976BB4C4E87E96510BD3CA78362B15810");

    /* Amber Alert */
    public static final byte[] CMASMSG_PDU_09 = IccUtils.hexStringToBytes("0100111B0111"
            + "C1B6B82C0705D965391D442FCFE9A0582C2604");

    /* RMT */
    public static final byte[] CMASMSG_PDU_10 = IccUtils.hexStringToBytes("0100111C0111" 
            + "D22615442FCFE9A0582C3604499B54");

    /* Exercise */
    public static final byte[] CMASMSG_PDU_11 = IccUtils.hexStringToBytes("0100111D0111"
            + "457C593E4ECFCB207A794E07C5623122A8882FCBC7E97919");

    public static final byte[] CMASMSG_PDU_12 = IccUtils.hexStringToBytes("0100111E0111"
            + "F57959DE2297CD6977990CA297E774502C162B02");

    /* content - tele */
    public static final byte[] CMASMSG_PDU_090 = IccUtils.hexStringToBytes("0200111B0111"
            + "F432BBDC1ABFC96510BD3CA78340B1190E07B3E16837990D");

    /* content - email */
    public static final byte[] CMASMSG_PDU_091 = IccUtils.hexStringToBytes("0300111B0111"
            + "E57638CD06D1CB733A0804028140F97C1E707EBFCFECB26BFC6E03");

    /* content -site */
    public static final byte[] CMASMSG_PDU_092 = IccUtils.hexStringToBytes("0400111B0111"
            + "F7B218344FD3CB207A794E078140F7FBDD15B3CD5CE3771B");

    /* content -long */
    public static final byte[] CMASMSG_PDU_093 = IccUtils.hexStringToBytes("0500111B0111"
            + "ECB7FB4C2DE3E93018FBED3E53CB783A0CC67EBBCFD4329E0E83B1DFEE33B58CA7C360ECB"
            + "7FB4C2DE3E93018FBED3E53CB783A0CC67EBBCFD4329E0E83B1DFEE33B58CA7C3602E970B");

    /* update msg 01 */
    public static final byte[] CMASMSG_PDU_101 = IccUtils.hexStringToBytes("050011130111"
            + "7538394C2FA362291DE8ED2E83CCF2F71904");

    /* update msg 02 */
    public static final byte[] CMASMSG_PDU_102 = IccUtils.hexStringToBytes("050111130111"
            + "7538394C2FA364291DE8ED2E83CCF2F719744FD3D120FAFD0D2AE7CB731608");

    /* update msg 03 */
    public static final byte[] CMASMSG_PDU_103 = IccUtils.hexStringToBytes("050211130111"
            + "7538394C2FA366291DE8ED2E83CCF2F719744FD3D120FAFD0D2AE7CB7316C8FCAECB41ECF279EE02");

    private static SmsCbLocation sEmptyLocation = new SmsCbLocation();

    private static SmsCbMessage createFromPdu(byte[] pdu) {
        try {
            byte[][] pdus = new byte[1][];
            pdus[0] = pdu;
            return GsmSmsCbMessage.createSmsCbMessage(sEmptyLocation, pdus);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }

    private static SmsCbMessage createFromPdus(byte[][] pdus) {
        try {
            return GsmSmsCbMessage.createSmsCbMessage(sEmptyLocation, pdus);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }

    private static SmsCbMessage createFromePdusWithSpcLoc(byte[] pdu, String plmn, int lac, int cid) {
        SmsCbLocation location = new SmsCbLocation(plmn, lac, cid);
        try {
            byte[][] pdus = new byte[1][];
            pdus[0] = pdu;
            return GsmSmsCbMessage.createSmsCbMessage(location, pdus);
        } catch (IllegalArgumentException e) {
            return null;
        }
    }

    /*
     * public static void testSendEtwsMessageNormal(Context ctxt) { Intent intent = new
     * Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION); intent.putExtra("message", createFromPdu(etwsMessageNormal));
     * intent.putExtra(SLOT_EXT, sSlotId); ctxt.sendOrderedBroadcast(intent,
     * "android.permission.RECEIVE_EMERGENCY_BROADCAST"); }
     */
/*
    public static void sendEtwsMessageTest(Context ctxt) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", new SmsCbMessage(1, 0, 1, new SmsCbLocation("00101", 128, 3670016), 4355, null,
                "ETWStestMessage", 3, new SmsCbEtwsInfo(3, true, true, etwsMessageTest), null));
        intent.putExtra(SLOT_EXT, sSlotId);
        ctxt.sendOrderedBroadcast(intent, "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }
*/
    public static void sendCMASMessage(Context ctxt, byte[] pdu) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", createFromPdu(pdu));
        intent.putExtra(SLOT_EXT, sSlotId);
        ctxt.sendOrderedBroadcast(intent, "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }

    public static void sendCMASMessageWithSpecLoc(Context ctxt, byte[] pdu, String plmn, int lac, int cid) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", createFromePdusWithSpcLoc(pdu, plmn, lac, cid));
        intent.putExtra(SLOT_EXT, sSlotId);
        ctxt.sendOrderedBroadcast(intent, "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }
}
