package com.mediatek.deviceregister;

import com.android.internal.telephony.PhoneConstants;
public class Const {

    public static final String TAG = "DeviceRegister/";

    public static final String ACTION_BOOTCOMPLETED = "android.intent.action.BOOT_COMPLETED";

    /**
     * Broadcast when network is OK, Device can begin to register now
     */
    public static final String ACTION_CDMA_AUTO_SMS_REGISTER_FEASIBLE = "android.provider.Telephony.CDMA_AUTO_SMS_REGISTER_FEASIBLE";

    /**
     * Broadcast when Server returns register result
     */
    // intent action from SMS framework
    public static final String ACTION_CT_CONFIRMED_MESSAGE = "android.telephony.sms.CDMA_REG_SMS_ACTION";

    /**
     * Broadcast when register SMS has been sent successfully
     */
    public static final String ACTION_REGISTER_MESSAGE_SEND = "android.intent.action.REGISTER_SMS_SEND";

    // protocol version specified by CT
    public static final byte PROTOCOL_VERSION_ESN = 0x01;
    public static final byte PROTOCOL_VERSION_MEID = 0x02;

    // command type specified by CT
    public static final byte COMMAND_TYPE_SEND = 0x03;
    public static final byte COMMAND_TYPE_RECEIVED = 0x04;

    public static final int MANUFACTURE_MAX_LENGTH = 3;
    public static final int MODEL_MAX_LENGTH = 20;

    public static final String IMSI_DEFAULT_VALUE = "000000000000000";

    public static final String SERVER_ADDRESS = "10659401";
    public static final String OPERATOR_CT = "46003";
    public static final String OPERATOR_CT_MAC = "45502";

    public static final int SEND_MESSAGE_TIME_UP = 103;
    public static final int ONE_MINUTE = 60 * 1000;

    /**
     * Shared preferences to store flag whether need to listen feasible broadcast
     */
    public static final String KEY_RECEIVED_FEASIBLE_BROADCAST = "received_feasible_broadcast";
    public static final boolean DEFALT_RECEIVED_FEASIBLE_BROADCAST = false;

    public static final int SEND_MESSAGE_RETRY_TIMES_MAX = 3;

    public static final short PORT = 0;

    public static final String PESN_PREFIX = "80";
    public static final String MEID_TO_PESN_HASH_NAME = "SHA-1";

    public static final int CHECKSUM_LENGTH = 8;

    public static final int UIM_NONE = -1;

    public static final int[] UIM_ID_LIST = { PhoneConstants.GEMINI_SIM_1, PhoneConstants.GEMINI_SIM_2 };
}
