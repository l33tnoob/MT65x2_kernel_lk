package com.mediatek.telephony;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.app.ActivityThread;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.PhoneNumberUtils;
import android.telephony.SimSmsInsertStatus;
import android.telephony.SmsManager;
import android.telephony.SmsMemoryStatus;
import android.telephony.SmsMessage;
import android.telephony.SmsParameters;
import android.telephony.TelephonyManager;

import android.text.TextUtils;
import android.telephony.Rlog;
import android.content.Context;

import com.android.internal.telephony.ISms;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsRawData;
import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.GeminiSmsMessage;

import com.mediatek.common.telephony.ISmsManagerExt;
import com.mediatek.common.telephony.IccSmsStorageStatus;
import com.mediatek.common.telephony.ITelephonyEx;
import com.android.internal.telephony.ITelephony;
/// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.IOnlyOwnerSimSupport;
/// @}
// Cell broadcast config information
import com.android.internal.telephony.SmsCbConfigInfo;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;

/**
 * Manages SMS operations such as sending data, text, and PDU SMS messages.
 */
public class SmsManagerEx implements ISmsManagerExt {

    private static final String TAG = "SMSEx";

    private static final SmsManagerEx sInstance = new SmsManagerEx();
    private static int lastReceivedSmsSimId = PhoneConstants.GEMINI_SIM_1;

    private static final int TEST_MODE_CTA = 1;
    private static final int TEST_MODE_FTA = 2;
    private static final int TEST_MODE_IOT = 3;
    private static final int TEST_MODE_UNKNOWN = -1;
    private static final String TEST_MODE_PROPERTY_KEY = "gsm.gcf.testmode";
    private static final String TEST_MODE_PROPERTY_KEY2 = "gsm.gcf.testmode2";
    private int testMode = 0;

    /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
    private static IOnlyOwnerSimSupport mOnlyOwnerSimSupport = MediatekClassFactory.createInstance(IOnlyOwnerSimSupport.class);
    /// @}

    /**
     * Send a text based SMS.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK</code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param slotId SIM card the user would like to access
     *
     * @throws IllegalArgumentException if destinationAddress or text are empty
     */
    public void sendTextMessage(
            String destinationAddress, String scAddress, String text,
            PendingIntent sentIntent, PendingIntent deliveryIntent,
            int slotId) {
        Rlog.d(TAG, "sendTextMessage, text="+text+", destinationAddress="+destinationAddress);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);
                     
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntent,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendTextMessage return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destinationAddress, text, sentIntent)) {
            return;
        }

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                iccISms.sendText(ActivityThread.currentPackageName(), destinationAddress, scAddress,
                        text, sentIntent, deliveryIntent);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "sendTextMessage, RemoteException!");
        }
    }

   /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message size.
     *
     * @param text the original message.  Must not be null.
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     *
     * @throws IllegalArgumentException if text is null
     */
    public ArrayList<String> divideMessage(String text) {
        if (null == text) {
            throw new IllegalArgumentException("text is null");
        }
        return SmsMessage.fragmentText(text);
    }

    /**
     * Send a multi-part text based SMS.  The callee should have already
     * divided the message into correctly sized parts by calling
     * <code>divideMessage</code>.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK</code> for success,
     *   or one of these errors:<br>
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *   <code>RESULT_ERROR_RADIO_OFF</code><br>
     *   <code>RESULT_ERROR_NULL_PDU</code><br>
     *   For <code>RESULT_ERROR_GENERIC_FAILURE</code> each sentIntent may include
     *   the extra "errorCode" containing a radio technology specific value,
     *   generally only useful for troubleshooting.<br>
     *   The per-application based SMS control checks sentIntent. If sentIntent
     *   is NULL the caller will be checked against all unknown applications,
     *   which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param slotId SIM card the user would like to access
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     */
    public void sendMultipartTextMessage(
            String destinationAddress, String scAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents, ArrayList<PendingIntent> deliveryIntents,
            int slotId) {
        Rlog.d(TAG, "sendMultipartTextMessage, destinationAddress="+destinationAddress);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntents,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendMultipartTextMessage return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destinationAddress, parts, sentIntents)) {
            return;
        }

        if (parts.size() > 1) {
            try {
                ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
                if (iccISms != null) {
                    iccISms.sendMultipartText(ActivityThread.currentPackageName(),
                            destinationAddress, scAddress, parts, sentIntents, deliveryIntents);
                }
            } catch (RemoteException ex) {
                Rlog.d(TAG, "sendMultipartTextMessage, RemoteException!");
            }
        } else {
            PendingIntent sentIntent = null;
            PendingIntent deliveryIntent = null;
            if (sentIntents != null && sentIntents.size() > 0) {
                sentIntent = sentIntents.get(0);
            }
            if (deliveryIntents != null && deliveryIntents.size() > 0) {
                deliveryIntent = deliveryIntents.get(0);
            }
            String text = (parts == null || parts.size() == 0) ? "" : parts.get(0);
            sendTextMessage(destinationAddress, scAddress, text, 
                sentIntent, deliveryIntent, slotId);
        }
    }

    /**
     * Send a data based SMS to a specific application port.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param destinationPort the port to deliver the message to
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is successfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK</code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param slotId SIM card the user would like to access        
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     */
    public void sendDataMessage(
            String destinationAddress, String scAddress, short destinationPort,
            byte[] data, PendingIntent sentIntent, PendingIntent deliveryIntent,
            int slotId) {
        Rlog.d(TAG, "sendDataMessage, destinationAddress="+destinationAddress);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntent,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendDataMessage return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destinationAddress, "send_data", sentIntent)) {
            return;
        }

        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid message data");
        }

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                iccISms.sendData(ActivityThread.currentPackageName(),
                    destinationAddress, scAddress, destinationPort & 0xFFFF,
                    data, sentIntent, deliveryIntent);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "sendDataMessage, RemoteException!");
        }
    }

    /**
     * Get the default instance of the SmsManagerEx
     *
     * @return the default instance of the SmsManagerEx
     */
    public static SmsManagerEx getDefault() {
        return sInstance;
    }

    private SmsManagerEx() {
        // get test mode from SystemProperties
        try {
            if(getDefaultSim() == PhoneConstants.GEMINI_SIM_1) {
                Rlog.d(TAG, "SM-constructor: get test mode from SIM 1");
                testMode = Integer.valueOf(SystemProperties.get(TEST_MODE_PROPERTY_KEY)).intValue();
            } else {
                Rlog.d(TAG, "SM-constructor: get test mode from SIM 2");
                // testMode = Integer.valueOf(SystemProperties.get(TEST_MODE_PROPERTY_KEY2)).intValue();
                testMode = Integer.valueOf(SystemProperties.get(TEST_MODE_PROPERTY_KEY)).intValue();
            }
        } catch(NumberFormatException e) {
            Rlog.d(TAG, "SM-constructor: invalid property value");
            testMode = TEST_MODE_UNKNOWN;
        }
        Rlog.d(TAG, "SM-constructor: test mode is " + testMode);
    }

    /**
     * Copy a raw SMS PDU to the ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     *
     * @param smsc the SMSC for this message, or NULL for the default SMSC
     * @param pdu the raw PDU to store
     * @param status message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *               STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param slotId SIM card the user would like to access
     * @return true for success
     *
     * {@hide}
     */
    public int copyMessageToIcc(byte[] smsc, byte[] pdu, int status,
        int slotId) {
        Rlog.d(TAG, "copyMessageToIcc");
        String isms = getSmsServiceName(slotId);
        SimSmsInsertStatus smsStatus = null;

        if (null == pdu) {
            throw new IllegalArgumentException("pdu is NULL");
        }

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "copyMessageToIcc return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return -1;
        }
        /// @}

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                smsStatus = iccISms.insertRawMessageToIccCard(ActivityThread.currentPackageName(), status, pdu, smsc);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        Rlog.d(TAG, (smsStatus != null) ? "insert Raw " + smsStatus.indexInIcc : "insert Raw null");

        if(smsStatus != null) {
            int[] index = smsStatus.getIndex();

            if (index != null && index.length > 0) {
                return index[0];
            }
        }

        return -1;
    }

    /**
     * Delete the specified message from the ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     *
     * @param messageIndex is the record index of the message on ICC
     * @param slotId SIM card the user would like to access
     * @return true for success
     *
     * {@hide}
     */
    public boolean deleteMessageFromIcc(int messageIndex, int slotId) {
        Rlog.d(TAG, "deleteMessageFromIcc, messageIndex="+messageIndex);
        Rlog.d(TAG, "slotId="+slotId);
        boolean success = false;
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "deleteMessageFromIcc return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return false ;
        }
        /// @}

        byte[] pdu = new byte[IccConstants.SMS_RECORD_LENGTH - 1];
        Arrays.fill(pdu, (byte) 0xff);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                success = iccISms.updateMessageOnIccEf(ActivityThread.currentPackageName(),
                        messageIndex,  SmsManager.STATUS_ON_ICC_FREE, pdu);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "deleteMessageFromIcc, RemoteException!");
        }

        return success;
    }

    /**
     * Update the specified message on the ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     *
     * @param messageIndex record index of message to update
     * @param newStatus new message status (STATUS_ON_ICC_READ,
     *                  STATUS_ON_ICC_UNREAD, STATUS_ON_ICC_SENT,
     *                  STATUS_ON_ICC_UNSENT, STATUS_ON_ICC_FREE)
     * @param pdu the raw PDU to store
     * @param slotId SIM card the user would like to access
     * @return true for success
     *
     * {@hide}
     */
    public boolean updateMessageOnIcc(int messageIndex, int newStatus, byte[] pdu,
        int slotId) {
        Rlog.d(TAG, "updateMessageOnIcc, messageIndex="+messageIndex);
        Rlog.d(TAG, "slotId="+slotId);
        boolean success = false;
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "updateMessageOnIcc return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return false ;
        }
        /// @}

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                success = iccISms.updateMessageOnIccEf(ActivityThread.currentPackageName(),
                    messageIndex, newStatus, pdu);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "updateMessageOnIcc, RemoteException!");
        }

        return success;
    }

    /**
     * Retrieves all messages currently stored on ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     * @param slotId SIM card the user would like to access
     *
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects
     *
     * {@hide}
     */
    public ArrayList<SmsMessage> getAllMessagesFromIcc(int slotId) {
        Rlog.d(TAG, "getAllMessagesFromIcc");
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);
        List<SmsRawData> records = null;

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                records = iccISms.getAllMessagesFromIccEf(ActivityThread.currentPackageName());
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "getAllMessagesFromIcc, RemoteException!");
        }

        int sz = 0;
        if (records != null) {
            sz = records.size();
        }
        for (int i = 0; i < sz; ++i) {
            byte[] data = null;
            SmsRawData record = records.get(i);
            if (record == null) {
                continue;
            } else {
                data = record.getBytes();
            }
            int index = i + 1;
            if ((data[0] & 0xff) == SmsManager.STATUS_ON_ICC_UNREAD) {
                Rlog.d(TAG, "index[" + index + "] is STATUS_ON_ICC_READ");
                boolean ret;
                ret = updateMessageOnIcc(index, SmsManager.STATUS_ON_ICC_READ, data, slotId);
                if (ret) {
                    Rlog.d(TAG, "update index[" + index + "] to STATUS_ON_ICC_READ");
                } else {
                    Rlog.d(TAG, "fail to update message status");
                }
            }
        }

        return createMessageListFromRawRecords(records, slotId);

    }

    /**
     * Enable reception of cell broadcast (SMS-CB) messages with the given
     * message identifier. Note that if two different clients enable the same
     * message identifier, they must both disable it for the device to stop
     * receiving those messages. All received messages will be broadcast in an
     * intent with the action "android.provider.Telephony.SMS_CB_RECEIVED".
     * Note: This call is blocking, callers may want to avoid calling it from
     * the main thread of an application.
     *
     * @param messageIdentifier Message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param slotId SIM card the user would like to access
     * @return true if successful, false otherwise
     * @see #disableCellBroadcast(int)
     *
     * {@hide}
     */
    public static boolean enableCellBroadcast(int messageIdentifier, int slotId) {
        return enableCellBroadcastRange(messageIdentifier, messageIdentifier, slotId);
    }

    /**
     * Disable reception of cell broadcast (SMS-CB) messages with the given
     * message identifier. Note that if two different clients enable the same
     * message identifier, they must both disable it for the device to stop
     * receiving those messages.
     * Note: This call is blocking, callers may want to avoid calling it from
     * the main thread of an application.
     *
     * @param messageIdentifier Message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param slotId SIM card the user would like to access
     * @return true if successful, false otherwise
     *
     * @see #enableCellBroadcast(int)
     *
     * {@hide}
     */
    public static boolean disableCellBroadcast(int messageIdentifier, int slotId) {
        return disableCellBroadcastRange(messageIdentifier, messageIdentifier, slotId);
    }

    /**
     * Enable reception of cell broadcast (SMS-CB) messages with the given
     * message identifier range. Note that if two different clients enable the same
     * message identifier, they must both disable it for the device to stop
     * receiving those messages. All received messages will be broadcast in an
     * intent with the action "android.provider.Telephony.SMS_CB_RECEIVED".
     * Note: This call is blocking, callers may want to avoid calling it from
     * the main thread of an application.
     *
     * @param startMessageId first message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param endMessageId last message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param slotId SIM card the user would like to access
     * @return true if successful, false otherwise
     * @see #disableCellBroadcastRange(int, int)
     *
     * @throws IllegalArgumentException if endMessageId < startMessageId
     * {@hide}
     */
    public static boolean enableCellBroadcastRange(int startMessageId, int endMessageId, int slotId) {
        Rlog.d(TAG, "enableCellBroadcastRange, " + startMessageId + "-" + endMessageId);
        Rlog.d(TAG, "slotId="+slotId);
        boolean success = false;
        String isms = getSmsServiceName(slotId);

        if (endMessageId < startMessageId) {
            throw new IllegalArgumentException("endMessageId < startMessageId");
        }
        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                success = iccISms.enableCellBroadcastRange(startMessageId, endMessageId);
                Rlog.d(TAG, "enableCellBroadcastRange result: " + success);
            } else {
                Rlog.d(TAG, "fail to get sms service");
                success = false;
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "fail to enable CB range");
            success = false;
        }

        return success;
    }

    /**
     * Disable reception of cell broadcast (SMS-CB) messages with the given
     * message identifier range. Note that if two different clients enable the same
     * message identifier, they must both disable it for the device to stop
     * receiving those messages.
     * Note: This call is blocking, callers may want to avoid calling it from
     * the main thread of an application.
     *
     * @param startMessageId first message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param endMessageId last message identifier as specified in TS 23.041 (3GPP)
     * or C.R1001-G (3GPP2)
     * @param slotId SIM card the user would like to access
     * @return true if successful, false otherwise
     *
     * @see #enableCellBroadcastRange(int, int)
     *
     * @throws IllegalArgumentException if endMessageId < startMessageId
     * {@hide}
     */
    public static boolean disableCellBroadcastRange(int startMessageId, int endMessageId, int slotId) {
        Rlog.d(TAG, "disableCellBroadcastRange, " + startMessageId + "-" + endMessageId);
        Rlog.d(TAG, "slotId="+slotId);
        boolean success = false;
        String isms = getSmsServiceName(slotId);

        if (endMessageId < startMessageId) {
            throw new IllegalArgumentException("endMessageId < startMessageId");
        }
        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                success = iccISms.disableCellBroadcastRange(startMessageId, endMessageId);
                Rlog.d(TAG, "disableCellBroadcastRange result: " + success);
            } else {
                Rlog.d(TAG, "fail to get sms service");
                success = false;
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "fail to enable CB range");
            success = false;
        }

        return success;
    }

    /**
     * Create a list of <code>SmsMessage</code>s from a list of RawSmsData
     * records returned by <code>getAllMessagesFromIcc()</code>
     *
     * @param records SMS EF records, returned by
     *   <code>getAllMessagesFromIcc</code>
     * @param slotId SIM card the user would like to access
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects.
     */
    private static ArrayList<SmsMessage> createMessageListFromRawRecords(List<SmsRawData> records,
            int slotId) {
        ArrayList<SmsMessage> geminiMessages = null;
        Rlog.d(TAG, "createMessageListFromRawRecords");
        Rlog.d(TAG, "slotId="+slotId);

        if (records != null) {
            int count = records.size();
            geminiMessages = new ArrayList<SmsMessage>();

            for (int i = 0; i < count; i++) {
                SmsRawData data = records.get(i);

                if (data != null) {
                    GeminiSmsMessage geminiSms =
                            GeminiSmsMessage.createFromEfRecord(i + 1, data.getBytes(), slotId);
                    if (geminiSms != null) {
                        geminiMessages.add(geminiSms);
                    }
                }
            }
            Rlog.d(TAG, "actual SIM sms count is " + geminiMessages.size());
        } else {
            Rlog.d(TAG, "fail to parse SIM sms, records is null");
        }

        return geminiMessages;
    }

    /**
     * Send a data based SMS to a specific application port.
     *
     * @param destinationAddress the address to send the message to
     * @param scAddress is the service center address or null to use
     *  the current default SMSC
     * @param destinationPort the port to deliver the message to
     * @param originalPort the port to deliver the message from
     * @param data the body of the message to send
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applicaitons,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param slotId SIM card the user would like to access
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     *
     */
    public void sendDataMessage(
            String destinationAddress, String scAddress, short destinationPort, 
            short originalPort, byte[] data, PendingIntent sentIntent,
            PendingIntent deliveryIntent, int slotId) {
        Rlog.d(TAG, "sendDataMessage, destinationAddress="+destinationAddress);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntent,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendDataMessage return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destinationAddress, "send_data", sentIntent)) {
            return;
        }

        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Invalid message data");
        }

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                iccISms.sendDataWithOriginalPort(ActivityThread.currentPackageName(),
                        destinationAddress, scAddress, destinationPort & 0xFFFF,
                        originalPort & 0xFFFF, data, sentIntent, deliveryIntent);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "sendDataMessage, RemoteException!");
        }

    }

    /**
     * Copy a text SMS to the ICC.
     *
     * @param scAddress Service center address
     * @param address   Destination address or original address
     * @param text      List of message text
     * @param status    message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *                  STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param timestamp Timestamp when service center receive the message
     * @param slotId SIM card the user would like to access
     * @return success or not
     *
     * @hide
     */
    public int copyTextMessageToIccCard(String scAddress, String address, List<String> text,
            int status, long timestamp, int slotId) {
        Rlog.d(TAG, "copyTextMessageToIccCard");
        Rlog.d(TAG, "slotId="+slotId);
        int result = SmsManager.RESULT_ERROR_GENERIC_FAILURE;
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "sendDataMessage return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return result;
        }
        /// @}

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                result = iccISms.copyTextMessageToIccCard(
                    ActivityThread.currentPackageName(), scAddress, address, text, status, timestamp);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return result;

    }

    /**
     * Get the default SIM id
     */
    private int getDefaultSim() {
        return TelephonyManager.getDefault().getSmsDefaultSim();
    }

    /**
     * Send a text based SMS.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *  the current default SMSC
     * @param text the body of the message to send
     * @param encodingType the encoding type of message(gsm 7-bit, unicode or automatic)
     * @param slotId the sim card that user wants to access
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     *  The result code will be <code>Activity.RESULT_OK<code> for success,
     *  or one of these errors:<br>
     *  <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *  <code>RESULT_ERROR_RADIO_OFF</code><br>
     *  <code>RESULT_ERROR_NULL_PDU</code><br>
     *  For <code>RESULT_ERROR_GENERIC_FAILURE</code> the sentIntent may include
     *  the extra "errorCode" containing a radio technology specific value,
     *  generally only useful for troubleshooting.<br>
     *  The per-application based SMS control checks sentIntent. If sentIntent
     *  is NULL the caller will be checked against all unknown applications,
     *  which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     *
     * @throws IllegalArgumentException if destinationAddress or text are empty
     * @hide
     */
    public void sendTextMessageWithEncodingType(
        String destAddr,
        String scAddr,
        String text,
        int encodingType,
        PendingIntent sentIntent,
        PendingIntent deliveryIntent,
        int slotId) {
        Rlog.d(TAG, "sendTextMessageWithEncodingType, text="+text+", encoding="+encodingType);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntent,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendTextMessageWithEncodingType return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destAddr, text, sentIntent)) {
            Rlog.d(TAG, "the parameters are invalid");
            return;
        }

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                Rlog.d(TAG, "call ISms interface to send text message");
                iccISms.sendTextWithEncodingType(ActivityThread.currentPackageName(),
                    destAddr, scAddr, text, encodingType, sentIntent, deliveryIntent);
            } else {
                Rlog.d(TAG, "iccISms is null");
            }
        } catch (RemoteException ex) {
            // ignore it
            Rlog.d(TAG, "fail to get ISms");
        }

    }

    /**
     * Send a multi-part text based SMS.  The callee should have already
     * divided the message into correctly sized parts by calling
     * <code>divideMessage</code>.
     *
     * @param destAddr the address to send the message to
     * @param scAddr is the service center address or null to use
     *   the current default SMSC
     * @param parts an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @param encodingType the encoding type of message(gsm 7-bit, unicode or automatic)
     * @param sentIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been sent.
     *   The result code will be <code>Activity.RESULT_OK<code> for success,
     *   or one of these errors:<br>
     *   <code>RESULT_ERROR_GENERIC_FAILURE</code><br>
     *   <code>RESULT_ERROR_RADIO_OFF</code><br>
     *   <code>RESULT_ERROR_NULL_PDU</code><br>
     *   For <code>RESULT_ERROR_GENERIC_FAILURE</code> each sentIntent may include
     *   the extra "errorCode" containing a radio technology specific value,
     *   generally only useful for troubleshooting.<br>
     *   The per-application based SMS control checks sentIntent. If sentIntent
     *   is NULL the caller will be checked against all unknown applicaitons,
     *   which cause smaller number of SMS to be sent in checking period.
     * @param deliveryIntents if not null, an <code>ArrayList</code> of
     *   <code>PendingIntent</code>s (one for each message part) that is
     *   broadcast when the corresponding message part has been delivered
     *   to the recipient.  The raw pdu of the status report is in the
     *   extended data ("pdu").
     * @param slotId the sim card that user wants to access
     *
     * @throws IllegalArgumentException if destinationAddress or data are empty
     * @hide
     */
    public void sendMultipartTextMessageWithEncodingType(
            String destAddr,
            String scAddr,
            ArrayList<String> parts,
            int encodingType,
            ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents,
            int slotId) {
        Rlog.d(TAG, "sendMultipartTextMessageWithEncodingType, encoding = " + encodingType);
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntents,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendMultipartTextMessageWithEncodingType return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destAddr, parts, sentIntents)) {
            Rlog.d(TAG, "invalid parameters for multipart message");
            return;
        }

        if (parts.size() > 1) {
            try {
                ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
                if (iccISms != null) {
                    Rlog.d(TAG, "call ISms.sendMultipartText");
                    iccISms.sendMultipartTextWithEncodingType(ActivityThread.currentPackageName(),
                            destAddr, scAddr, parts, encodingType, sentIntents, deliveryIntents);
                }
            } catch (RemoteException ex) {
                // ignore it
            }
        } else {
            PendingIntent sentIntent = null;
            PendingIntent deliveryIntent = null;
            if (sentIntents != null && sentIntents.size() > 0) {
                sentIntent = sentIntents.get(0);
            }
            Rlog.d(TAG, "get sentIntent: " + sentIntent);
            if (deliveryIntents != null && deliveryIntents.size() > 0) {
                deliveryIntent = deliveryIntents.get(0);
            }
            Rlog.d(TAG, "send single message");
            if (parts != null) {
                Rlog.d(TAG, "parts.size = " + parts.size());
            }
            String text = (parts == null || parts.size() == 0) ? "" : parts.get(0);
            Rlog.d(TAG, "pass encoding type " + encodingType);
            sendTextMessageWithEncodingType(destAddr, scAddr, text, encodingType, 
                sentIntent, deliveryIntent, slotId);
        }

    }

    /**
     * Divide a message text into several fragments, none bigger than
     * the maximum SMS message size.
     *
     * @param text the original message.  Must not be null.
     * @param encodingType text encoding type(7-bit, 16-bit or automatic)
     * @return an <code>ArrayList</code> of strings that, in order,
     *   comprise the original message
     * @hide
     */
    public ArrayList<String> divideMessage(String text, int encodingType) {
        Rlog.d(TAG, "divideMessage, encoding = " + encodingType);
        ArrayList<String> ret = SmsMessage.fragmentText(text, encodingType);
        Rlog.d(TAG, "divideMessage: size = " + ret.size());
        return ret;
    }

    /**
     * Deletes all the messages from the ICC.
     * ICC (Integrated Circuit Card) is the card of the device.
     * For example, this can be the SIM or USIM for GSM.
     *
     * @param slotId the sim card that user wants to access
     * @return True for success
     * @hide
     */
    public boolean deleteAllMessagesFromIcc(int slotId) {
        return deleteMessageFromIcc(-1, slotId);
    }

    /**
     * Judges if SMS subsystem in SIM card is ready or not.
     *
     * @param slotId the sim card that user wants to access
     * @return True for success
     * @hide
     */
    public boolean isSmsReady(int slotId) {
        Rlog.d(TAG, "isSmsReady");
        boolean isReady = false;
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                isReady = iccISms.isSmsReady();
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return isReady;
    }

    /**
     * Copy a raw SMS PDU to the ICC.
     *
     * @param status message status (STATUS_ON_ICC_READ, STATUS_ON_ICC_UNREAD,
     *               STATUS_ON_ICC_SENT, STATUS_ON_ICC_UNSENT)
     * @param pdu the raw PDU to store
     * @param smsc encoded smsc service center
     * @param slotId the sim card that user wants to access
     * @return SimSmsInsertStatus
     * @hide
     */
    private static SimSmsInsertStatus insertRawMessageToIccCard(int status, byte[] pdu, byte[] smsc,
        int slotId) {
        Rlog.d(TAG, "insertRawMessageToIccCard");
        Rlog.d(TAG, "slotId="+slotId);
        SimSmsInsertStatus ret = null;
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "insertRawMessageToIccCard return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return null ;
        }
        /// @}

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                ret = iccISms.insertRawMessageToIccCard(ActivityThread.currentPackageName(), status, pdu, smsc);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        Rlog.d(TAG, (ret != null) ? "insert Raw " + ret.indexInIcc : "insert Raw null");
        return ret;
    }

    /**
     * Send an SMS with specified encoding type.
     *
     * @param destAddr the address to send the message to
     * @param scAddr the SMSC to send the message through, or NULL for the
     *  default SMSC
     * @param text the body of the message to send
     * @param extraParams extra parameters, such as validity period, encoding type
     * @param sentIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is sucessfully sent, or failed.
     * @param deliveryIntent if not NULL this <code>PendingIntent</code> is
     *  broadcast when the message is delivered to the recipient.  The
     *  raw pdu of the status report is in the extended data ("pdu").
     * @param slotId the sim card that user wants to access
     */
    public void sendTextMessageWithExtraParams(
            String destAddr, String scAddr, String text, Bundle extraParams,
            PendingIntent sentIntent, PendingIntent deliveryIntent,
            int slotId) {
        Rlog.d(TAG, "sendTextMessageWithExtraParams, text="+text);
        Rlog.d(TAG, "slotId="+slotId);
        String serviceName = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntent,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendTextMessageWithExtraParams return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destAddr, text, sentIntent)) {
            return;
        }

        if (extraParams == null) {
            Rlog.d(TAG, "bundle is null");
            return;
        }

        try {
            ISms service = ISms.Stub.asInterface(ServiceManager.getService(serviceName));
            if (service != null) {
                service.sendTextWithExtraParams(ActivityThread.currentPackageName(), 
                    destAddr, scAddr, text, extraParams, sentIntent, deliveryIntent);
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "fail to call sendTextWithExtraParams: " + e);
        }

    }

    /**
     * Sends a multi-part text based SMS with specified encoding type.
     * 
     * @param destAddr Address to send the message to
     * @param scAddr Service center address or null to use the current
     *            default SMSC
     * @param parts <code>ArrayList</code> of strings that, in order,
     *            comprise the original message
     * @param extraParams Extra parameters, such as validity period, encoding
     *            type
     * @param sentIntents If not null, an <code>ArrayList</code> of
     *            <code>PendingIntent</code>s (one for each message part) that
     *            will be broadcasted when the corresponding message part has been
     *            sent.
     * @param deliveryIntents If not null, an <code>ArrayList</code> of
     *            <code>PendingIntent</code>s (one for each message part) that
     *            will be broadcast when the corresponding message part has been
     *            delivered to the recipient. The raw PDU of the status report
     *            is in the extended data ("pdu").
     * @param slotId Identifier for SIM card slot
     */
    public void sendMultipartTextMessageWithExtraParams(String destAddr,
            String scAddr, ArrayList<String> parts, Bundle extraParams,
            ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents, int slotId) {
        Rlog.d(TAG, "sendMultipartTextMessageWithExtraParams");
        Rlog.d(TAG, "slotId="+slotId);
        String serviceName = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            mOnlyOwnerSimSupport.intercept(sentIntents,RESULT_ERROR_GENERIC_FAILURE);
            Rlog.d(TAG, "sendMultipartTextMessageWithExtraParams return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return ;
        }
        /// @}

        if (!isValidParameters(destAddr, parts, sentIntents)) {
            return;
        }

        if (extraParams == null) {
            Rlog.d(TAG, "bundle is null");
            return;
        }

        if (parts.size() > 1) {
            try {
                ISms service = ISms.Stub.asInterface(ServiceManager.getService(serviceName));
                if (service != null) {
                    service.sendMultipartTextWithExtraParams(ActivityThread.currentPackageName(),
                            destAddr, scAddr, parts, extraParams, sentIntents, deliveryIntents);
                }
            } catch (RemoteException e) {
                Rlog.d(TAG, "fail to call sendMultipartTextWithExtraParams: " + e);
            }
        } else {
            PendingIntent sentIntent = null;
            PendingIntent deliveryIntent = null;
            if (sentIntents != null && sentIntents.size() > 0) {
                sentIntent = sentIntents.get(0);
            }
            if (deliveryIntents != null && deliveryIntents.size() > 0) {
                deliveryIntent = deliveryIntents.get(0);
            }

            sendTextMessageWithExtraParams(destAddr, scAddr, parts.get(0), 
                extraParams, sentIntent, deliveryIntent, slotId);
        }
        
    }

    private static SmsParameters getSmsParameters(int slotId) {
        Rlog.d(TAG, "getSmsParameters");
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);

        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSmsParameters return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return null ;
        }
        /// @}

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                return iccISms.getSmsParameters(ActivityThread.currentPackageName());
            } else {
                return null;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "fail because of RemoteException");
        }

        Rlog.d(TAG, "fail to get SmsParameters");
        return null;

    }

    private static boolean setSmsParameters(SmsParameters params, int slotId) {
        Rlog.d(TAG, "setSmsParameters");
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                return iccISms.setSmsParameters(ActivityThread.currentPackageName(), params);
            } else {
                return false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "[EFsmsp fail because of RemoteException");
        }

        return false;

    }

    /**
     * @hide
     */
    public int readValidityPeriod(int subscription){
        return 0;
    }

    /**
     * @hide
     */
    public boolean updateValidityPeriod(int validityperiod, int subscription){
        return true;
    }

    /**
     * Gets SMS SIM card memory's total and used number.
     *
     * @param slotId SIM card the user would like to access
     * @return <code>IccSmsStorageStatus</code> object
     * @hide
     */
    public IccSmsStorageStatus getIccSmsStorageStatus(int slotId) {
        // impl
        Rlog.d(TAG, "call getSmsSimMemoryStatus");
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));

            if (iccISms != null) {
                return iccISms.getSmsSimMemoryStatus(ActivityThread.currentPackageName());
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return null;
    }

    /**
     * Gets the validity period of the sms in the sim card.
     * @param slotId SIM card the user would like to access
     * @return The validity period
     * @hide
     */
    public int getValidityPeriodFromIccCard(int slotId) {
        SmsParameters smsParam = getSmsParameters(slotId);
        if (smsParam == null) {
            return 0;
        }
        return smsParam.vp;
    }

    /**
     * @hide
     */
    public boolean updateSmsOnSimReadStatus(int index, boolean read,
       int slotId)
    {
        return true;
    }

    /**
     * @hide
     */
    public static boolean setEtwsConfig(int mode, int slotId) {
        Rlog.d(TAG, "setEtwsConfig, mode="+mode);
        Rlog.d(TAG, "slotId="+slotId);
        boolean ret = false;
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                ret = iccISms.setEtwsConfig(mode);
            }
        } catch (RemoteException ex) {
            // ignore it
        }

        return ret;

    }

    /**
     * judge if the input destination address is a valid SMS address or not
     *
     * @param da the input destination address
     * @return true for success
     *
     */
    private static boolean isValidSmsDestinationAddress(String da) {
        String encodeAddress = PhoneNumberUtils.extractNetworkPortion(da);
        if (encodeAddress == null)
            return true;

        int spaceCount = 0;
        for (int i = 0; i < da.length(); ++i) {
            if (da.charAt(i) == ' ' || da.charAt(i) == '-') {
                spaceCount++;
            }
        }

        return encodeAddress.length() == (da.length() - spaceCount);
    }

    /**
     * Sets the validity period of the sms in the sim card.
     * @param slotId, Identifier for SIM card slot
     * @return True if success
     * @hide
     */
    public boolean setValidityPeroidToIccCard(int validityPeriod, int slotId) {
        // impl
        SmsParameters smsParams = getSmsParameters(slotId);
        if (smsParams == null) {
            return false;
        }
        smsParams.vp = validityPeriod;
        return setSmsParameters(smsParams, slotId);
    }

     /**
     * Judge if the destination address is a valid SMS address or not, and if
     * the text is null or not
     *
     * @destinationAddress the destination address to which the message be sent
     * @text the content of shorm message
     * @sentIntent will be broadcast if the address or the text is invalid
     * @return true for valid parameters
     */
    private static boolean isValidParameters(
            String destinationAddress, String text, PendingIntent sentIntent) {
        // impl
        ArrayList<PendingIntent> sentIntents =
                new ArrayList<PendingIntent>();
        ArrayList<String> parts =
                new ArrayList<String>();

        sentIntents.add(sentIntent);
        parts.add(text);

        // if (TextUtils.isEmpty(text)) {
        // throw new IllegalArgumentException("Invalid message body");
        // }

        return isValidParameters(destinationAddress, parts, sentIntents);
    }

    /**
     * Judges if the destination address is a valid SMS address or not, and if
     * the text is null or not.
     *
     * @param destinationAddress The destination address to which the message be sent
     * @param parts The content of shorm message
     * @param sentIntent will be broadcast if the address or the text is invalid
     * @return True for valid parameters
     */
    private static boolean isValidParameters(
            String destinationAddress, ArrayList<String> parts,
            ArrayList<PendingIntent> sentIntents) {
        if (parts == null || parts.size() == 0) {
            return true;
        }

        if (!isValidSmsDestinationAddress(destinationAddress)) {
            for (int i = 0; i < sentIntents.size(); i++) {
                PendingIntent sentIntent = sentIntents.get(i);
                if (sentIntent != null) {
                    try {
                        sentIntent.send(SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                    } catch (CanceledException ex) {}
                }
            }

            Rlog.d(TAG, "Invalid destinationAddress: " + destinationAddress);
            return false;
        }

        if (TextUtils.isEmpty(destinationAddress)) {
            throw new IllegalArgumentException("Invalid destinationAddress");
        }
        if (parts == null || parts.size() < 1) {
            throw new IllegalArgumentException("Invalid message body");
        }

        return true;
    }

    /**
     * Gets the SMS service name by specific SIM ID.
     *
     * @param slotId SIM card the user would like to access
     * @return The SMS service name
     */
    private static String getSmsServiceName(int slotId) {
        if (slotId == PhoneConstants.GEMINI_SIM_1) {
            return "isms";
        } else if (slotId == PhoneConstants.GEMINI_SIM_2) {
            return "isms2";
        } else if (slotId == PhoneConstants.GEMINI_SIM_3) {
            return "isms3";
        } else if (slotId == PhoneConstants.GEMINI_SIM_4) {
            return "isms4";
        } else {
            return null;
        }
    }

    /**
     * Create an SmsMessage from an SMS EF record.
     *
     * @param index Index of SMS record. This should be index in ArrayList
     *              returned by SmsManager.getAllMessagesFromSim + 1.
     * @param data Record data.
     * @param slotId SIM card the user would like to access
     * @return An SmsMessage representing the record.
     *
     * @hide
     */
    private static GeminiSmsMessage createFromEfRecordByMode(int index, byte[] data, int slotId, int mode) {

        SmsMessage sms = null;

        if (mode == PhoneConstants.PHONE_TYPE_CDMA) {
            sms = SmsMessage.createFromEfRecord(index, data, SmsConstants.FORMAT_3GPP2);
        } else {
            sms = SmsMessage.createFromEfRecord(index, data, SmsConstants.FORMAT_3GPP);
        }

        return sms == null ? null : new GeminiSmsMessage(sms, slotId);
    }

    /**
     * Create a list of <code>SmsMessage</code>s from a list of RawSmsData
     * records returned by <code>getAllMessagesFromIcc()</code>.
     *
     * @param records SMS EF records, returned by
     *            <code>getAllMessagesFromIcc</code>
     * @param slotId SIM card the user would like to access
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects.
     */
    private static ArrayList<SmsMessage> createMessageListFromRawRecordsByMode(List<SmsRawData> records,
            int slotId, int mode) {
        Rlog.d(TAG, "createMessageListFromRawRecordsByMode");
        ArrayList<SmsMessage> geminiMessages = null;
        if (records != null) {
            int count = records.size();
            geminiMessages = new ArrayList<SmsMessage>();

            for (int i = 0; i < count; i++) {
                SmsRawData data = records.get(i);

                if (data != null) {
                    GeminiSmsMessage geminiSms =
                            createFromEfRecordByMode(i + 1, data.getBytes(), slotId, mode);
                    if (geminiSms != null) {
                        geminiMessages.add(geminiSms);
                    }
                }
            }
            Rlog.d(TAG, "actual SIM sms count is " + geminiMessages.size());
        } else {
            Rlog.d(TAG, "fail to parse SIM sms, records is null");
        }

        return geminiMessages;
    }

    /**
     * Retrieves all messages currently stored on ICC. ICC (Integrated Circuit
     * Card) is the card of the device. For example, this can be the SIM or USIM
     * for GSM.
     *
     * @param slotId SIM card the user would like to access
     * @param mode SIM mode the user would like to access
     * @return <code>ArrayList</code> of <code>SmsMessage</code> objects
     * @hide
     */
    public ArrayList<SmsMessage> getAllMessagesFromIcc(int slotId, int mode) {
        Rlog.d(TAG, "getAllMessagesFromIcc, mode="+mode);
        String isms = getSmsServiceName(slotId);
        List<SmsRawData> records = null;

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                records = iccISms.getAllMessagesFromIccEfByMode(
                    ActivityThread.currentPackageName(), mode);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "getAllMessagesFromIcc, RemoteException!");
        }

        int sz = 0;
        if (records != null) {
            sz = records.size();
        }
        for (int i = 0; i < sz; ++i) {
            byte[] data = null;
            SmsRawData record = records.get(i);
            if (record == null) {
                continue;
            } else {
                data = record.getBytes();
            }
            int index = i + 1;
            if ((data[0] & 0xff) == SmsManager.STATUS_ON_ICC_UNREAD) {
                Rlog.d(TAG, "index[" + index + "] is STATUS_ON_ICC_READ");
                boolean ret;
                ret = updateMessageOnIcc(index, SmsManager.STATUS_ON_ICC_READ, data, slotId);
                if (ret) {
                    Rlog.d(TAG, "update index[" + index + "] to STATUS_ON_ICC_READ");
                } else {
                    Rlog.d(TAG, "fail to update message status");
                }
            }
        }

        return createMessageListFromRawRecordsByMode(records, slotId, mode);
    }

    /**
     * Set the memory storage status of the SMS
     * This function is used for FTA test only
     *
     * @param status false for storage full, true for storage available
     * @param simId the sim card that user wants to access
     *
     */
    private void setSmsMemoryStatus(boolean status, int simId) {
        Rlog.d(TAG, "setSmsMemoryStatus");
        String isms = getSmsServiceName(simId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                iccISms.setSmsMemoryStatus(status);
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "setSmsMemoryStatus, RemoteException ");
        }
    }

    /**
     * Set the memory storage status of the SMS
     * This function is used for FTA test only
     *
     * @param status false for storage full, true for storage available
     *
     * @hide
     */
    public void setSmsMemoryStatus(boolean status) {
        boolean isTestIccCard = false;

        try{
            ITelephonyEx telephony = ITelephonyEx.Stub.asInterface(
                        ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
            if (telephony != null && lastReceivedSmsSimId >= 0) {
                isTestIccCard = telephony.isTestIccCard(getDefaultSim());
            }
        } catch(RemoteException ex) {
            // This shouldn't happen in the normal case
            Rlog.d(TAG, "setSmsMemoryStatus, remoteException: " + ex.getMessage());
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            Rlog.d(TAG, "setSmsMemoryStatus, NullPointerException: " + ex.getMessage());
        }

        if (isTestIccCard) {
            /* for FTA test, we need to send the status to the
            * SIM card which receiving the last incoming SMS
            */
            setSmsMemoryStatus(status, lastReceivedSmsSimId);
        } else if(testMode == TEST_MODE_FTA) {
            setSmsMemoryStatus(status, lastReceivedSmsSimId);
        } else {
            //getDefault SIM,
            setSmsMemoryStatus(status, getDefaultSim());
        }
    }

    /**
     * Set the last Incoming Sms SIM Id
     * This function is used for FTA test only
     *
     * @param simId the sim ID where the last incoming SMS comes from
     *
     * @hide
     */
    public void setLastIncomingSmsSimId(int simId) {
        if (simId == PhoneConstants.GEMINI_SIM_1 || simId == PhoneConstants.GEMINI_SIM_2) {
            lastReceivedSmsSimId = simId;
        }
    }

    /**
     * @hide
     */
    public boolean activateCellBroadcastSms(boolean activate, int slotId) {
        Rlog.d(TAG, "activateCellBroadcastSms activate : " + activate + ", slot = " + slotId);
        boolean result = false;

        String serviceName = getSmsServiceName(slotId);
        try {
            ISms service = ISms.Stub.asInterface(ServiceManager.getService(serviceName));
            if (service != null) {
                result = service.activateCellBroadcastSms(activate);
            } else {
                Rlog.d(TAG, "fail to get sms service");
                result = false;
            }
        } catch (RemoteException e) {
            Rlog.d(TAG, "fail to activate CB");
            result = false;
        }

        return result;
    }

    /**
     * @hide
     */
    public IccSmsStorageStatus getSmsSimMemoryStatus(int simId){
        return null;
    }

    /**
     * @hide
     */
    private SmsBroadcastConfigInfo Convert2SmsBroadcastConfigInfo(SmsCbConfigInfo info) {
        return new SmsBroadcastConfigInfo(
                        info.mFromServiceId, 
                        info.mToServiceId, 
                        info.mFromCodeScheme,
                        info.mToCodeScheme, 
                        info.mSelected);
    }

    /**
     * @hide
     */
    private SmsCbConfigInfo Convert2SmsCbConfigInfo(SmsBroadcastConfigInfo info) {
        return new SmsCbConfigInfo(
                        info.getFromServiceId(), 
                        info.getToServiceId(), 
                        info.getFromCodeScheme(), 
                        info.getToCodeScheme(), 
                        info.isSelected());
    }

    /**
     * @hide
     */
    public SmsBroadcastConfigInfo[] getCellBroadcastSmsConfig(int slotId) {
        Rlog.d(TAG, "getCellBroadcastSmsConfig");
        Rlog.d(TAG, "slotId="+slotId);
        String isms = getSmsServiceName(slotId);
        SmsCbConfigInfo[] configs = null;
                     
        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                configs = iccISms.getCellBroadcastSmsConfig();
            } else {
                Rlog.d(TAG, "fail to get sms service");
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "getCellBroadcastSmsConfig, RemoteException!");
        }

        if (configs != null)
        {
            Rlog.d(TAG, "config length = "+configs.length);        
            int i=0;
            if (configs.length != 0) {
                SmsBroadcastConfigInfo[] result = new SmsBroadcastConfigInfo[configs.length];
                for (i=0; i < configs.length; i++)
                    result[i] = Convert2SmsBroadcastConfigInfo(configs[i]);
                return result;
            }
        }

        /* Exception to return null case, Even if there is no channesl, it still have one config with -1 */
        return null;
    }

    /**
     * @hide
     */
    public boolean setCellBroadcastSmsConfig(SmsBroadcastConfigInfo[] channels,
            SmsBroadcastConfigInfo[] languages, int slotId) {
        Rlog.d(TAG, "setCellBroadcastSmsConfig");
        Rlog.d(TAG, "slotId="+slotId);
        Rlog.d(TAG, "channel size="+channels.length);
        Rlog.d(TAG, "language size="+languages.length);
        boolean result = false;
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                int i = 0;
                SmsCbConfigInfo[] channelInfos = null, languageInfos = null;
                if (channels != null && channels.length != 0) {
                    channelInfos = new SmsCbConfigInfo[channels.length];
                    for (i = 0 ; i < channels.length ; i++)
                        channelInfos[i] = Convert2SmsCbConfigInfo(channels[i]);
                }
                if (languages != null && languages.length != 0) {
                    languageInfos = new SmsCbConfigInfo[languages.length];
                    for (i = 0 ; i < languages.length ; i++)
                        languageInfos[i] = Convert2SmsCbConfigInfo(languages[i]);
                }

                result = iccISms.setCellBroadcastSmsConfig(channelInfos, languageInfos);
            } else {
                Rlog.d(TAG, "fail to get sms service");
                result = false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "setCellBroadcastSmsConfig, RemoteException!");
        }

        return result;
    }

    /**
     * @hide
     */
    public boolean queryCellBroadcastSmsActivation(int slotId) {
        Rlog.d(TAG, "queryCellBroadcastSmsActivation");
        Rlog.d(TAG, "slotId="+slotId);
        boolean result = false;
        String isms = getSmsServiceName(slotId);

        try {
            ISms iccISms = ISms.Stub.asInterface(ServiceManager.getService(isms));
            if (iccISms != null) {
                result = iccISms.queryCellBroadcastSmsActivation();
            } else {
                Rlog.d(TAG, "fail to get sms service");
                result = false;
            }
        } catch (RemoteException ex) {
            Rlog.d(TAG, "queryCellBroadcastSmsActivation, RemoteException!");
        }

        return result;
    }
}
