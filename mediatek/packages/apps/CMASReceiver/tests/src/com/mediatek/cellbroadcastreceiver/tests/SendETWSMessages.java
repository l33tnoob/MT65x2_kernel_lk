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
public class SendETWSMessages {

    private static String sTAG = "SendETWSMessages";

    private static final int DCS_7BIT_ENGLISH = 0x01;

    private static final int DCS_16BIT_UCS2 = 0x48;

    // add for gemini
    public static final String SLOT_EXT = "simId";

    public static int sSlotId = 0;

    /* GS for dulication */
    public static final byte[] CMASMSG_PDU_001 = IccUtils.hexStringToBytes("000011120111"
            + "54741914AFA7C76B9058FEBEBB41E6371EA4AEB7E173D0DB5E9683E8E832881DD6E741E4F7B9");

	private static final byte[] etwsMessageNormal = IccUtils.hexStringToBytes("000011001101" +
            "0D0A5BAE57CE770C531790E85C716CBF3044573065B930675730" +
            "9707767A751F30025F37304463FA308C306B5099304830664E0B30553044FF086C178C615E81FF09" +
            "0000000000000000000000000000");


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
	public static void testSendEtwsMessageNormal(Context ctxt) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", createFromPdu(etwsMessageNormal));
        intent.putExtra(SLOT_EXT, 0);
        ctxt.sendOrderedBroadcast(intent,
                "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }
    public static void sendETWSMessage(Context ctxt, byte[] pdu) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", createFromPdu(pdu));
        intent.putExtra(SLOT_EXT, sSlotId);
        ctxt.sendOrderedBroadcast(intent, "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }

    public static void sendETWSMessageWithSpecLoc(Context ctxt, byte[] pdu, String plmn, int lac, int cid) {
        Intent intent = new Intent(Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION);
        intent.putExtra("message", createFromePdusWithSpcLoc(pdu, plmn, lac, cid));
        intent.putExtra(SLOT_EXT, sSlotId);
        ctxt.sendOrderedBroadcast(intent, "android.permission.RECEIVE_EMERGENCY_BROADCAST");
    }
}
