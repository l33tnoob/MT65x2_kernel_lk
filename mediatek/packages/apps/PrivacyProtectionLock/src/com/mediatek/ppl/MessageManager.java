package com.mediatek.ppl;

import java.util.Arrays;
import java.util.regex.Pattern;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.ppl.R;

/**
 * Utility manager for message related operations.
 */
public class MessageManager {
    // sync this with R.array.sms_template_list and R.array.sms_pattern_list
    public static class Type {
        public static final byte INVALID = -1;
        public static final byte SIM_CHANGED = 0;
        public static final byte LOCK_REQUEST = 1;
        public static final byte LOCK_RESPONSE = 2;
        public static final byte UNLOCK_REQUEST = 3;
        public static final byte UNLOCK_RESPONSE = 4;
        public static final byte RESET_PW_REQUEST = 5;
        public static final byte RESET_PW_RESPONSE = 6;
        public static final byte WIPE_REQUEST = 7;
        public static final byte WIPE_STARTED = 8;
        public static final byte WIPE_COMPLETED = 9;
        public static final byte INSTRUCTION_DESCRIPTION = 10;
    }
    
    public static final String SMS_PENDING_INTENT_DATA_AUTH = "ppl";
    public static final String SMS_PENDING_INTENT_DATA_SCHEME = "sms";

    /**
     * Pending message is a container which can be used as message sending request or pending message record.
     */
	/*
	 * struct PendingMessage {
	 * 		unsigned char type;
	 * 		long id;
	 * 		unsigned char dest[CD_TRUSTED_NUMBER_LENGTH];
	 * } __attribute__ ((packed));
	 */
    public static class PendingMessage {
        public static final long INVALID_ID = -1;
        public static final int ANY_SIM_ID = -1;
        public static final int ALL_SIM_ID = -2;
        public static final int INVALID_SIM_ID = -3;

        public static final String KEY_TYPE = "type";
        public static final String KEY_NUMBER = "number";
        public static final String KEY_SIM_ID = "simId";
        public static final String KEY_FIRST_TRIAL = "firstTrial";

        // the following properties are persistent
        public long id;
        public byte type;
        public String number;
        // the following properties are transient
        public int simId;
        public String content;

        public static final int PENDING_MESSAGE_LENGTH =
                ControlData.TRUSTED_NUMBER_LENGTH + Long.SIZE/Byte.SIZE + Byte.SIZE/Byte.SIZE;

        // Let's hope the processor is NOT too powerful
        public static long getNextId() {
            return System.currentTimeMillis();
        }

        public PendingMessage(long id, byte type, String number, int simId, String content) {
            this.id = id;
            this.type = type;
            this.number = number;
            this.simId = simId;
            this.content = content;
        }

        public PendingMessage() {
            id = INVALID_ID;
            type = Type.INVALID;
            number = null;
            simId = -1;
            content = null;
        }

        /**
         * Create a pending message object by decoding the bytes.
         * 
         * @param buffer
         * @param offset
         */
        public PendingMessage(byte[] buffer, int offset) {
            decode(buffer, offset);
        }

        public PendingMessage clone() {
            return new PendingMessage(id, type, number, simId, content);
        }

        @Override
        public String toString() {
            return "PendingMessage " + hashCode() + " {"
                    + id + ", "
                    + type + ", "
                    + number + ", "
                    + simId + ", "
                    + content + "}";
        }

        /**
         * Convert the pending message data to bytes.
         * 
         * @param buffer    Should be zeroed after the offset.
         * @param offset
         */
        public void encode(byte[] buffer, int offset) {
            buffer[offset++] = type;
            byte[] idBytes = long2bytes(id);
            System.arraycopy(idBytes, 0, buffer, offset, idBytes.length);
            offset += Long.SIZE/Byte.SIZE;
            byte[] numberBytes = number.getBytes();
            if (numberBytes.length > ControlData.TRUSTED_NUMBER_LENGTH) {
                throw new Error("Destination number is too long");
            } else {
                numberBytes = Arrays.copyOf(numberBytes, ControlData.TRUSTED_NUMBER_LENGTH);
            }
            System.arraycopy(numberBytes, 0, buffer, offset, numberBytes.length);
        }

        /**
         * Re-construct the pending message from bytes.
         * 
         * @param buffer
         * @param offset
         */
        public void decode(byte[] buffer, int offset) {
            type = buffer[offset++];
            id = bytes2long(buffer, offset);
            offset += Long.SIZE/Byte.SIZE;
            int j = offset;
            for (; j < offset + ControlData.TRUSTED_NUMBER_LENGTH; ++j) {
                if (buffer[j] == 0) {
                    break;
                }
            }
            number = new String(buffer, offset, j - offset);
        }

        private static long bytes2long(byte[] b, int offset) {
            long temp = 0;
            long res = 0;
            for (int i = 0; i < 8; i++) {
                res <<= 8;
                temp = b[i + offset] & 0xff;
                res |= temp;
            }
            return res;
        }

        private static byte[] long2bytes(long num) {
            byte[] b = new byte[8];
            for (int i = 0; i < 8; i++) {
                b[i] = (byte) (num >>> (56 - (i * 8)));
            }
            return b;
        }
    }

    public static final String SMS_SENT_ACTION = "com.mediatek.ppl.SMS_SENT";

    private final Context mContext;
    private final String[] mMessageTemplates;
    private final Pattern[] mMessagePatterns;

    public MessageManager(Context context) {
        mContext = context;
        Resources resources = mContext.getResources();
        mMessageTemplates = resources.getStringArray(R.array.ppl_sms_template_list);
        String patternStrings[] = resources.getStringArray(R.array.ppl_sms_pattern_list);
        mMessagePatterns = new Pattern[patternStrings.length];
        for (int i = 0; i < patternStrings.length; ++i) {
            mMessagePatterns[i] = Pattern.compile(patternStrings[i], Pattern.CASE_INSENSITIVE);
        }
    }

    /**
     * Use the index into message patterns as the type. Therefore the definition of MessageManager.Type has to sync
     * with R.array.sms_template_list and R.array.sms_pattern_list.
     * 
     * @param message   Message body.
     * @return          Message type of the content if matched, Type.INVALID otherwise.
     */
    public byte getMessageType(String message) {
        for (byte i = 0; i < mMessagePatterns.length; ++i) {
            if (mMessagePatterns[i].matcher(message).matches()) {
                return i;
            }
        }
        return Type.INVALID;
    }

    public String getMessageTemplate(byte type) {
        return mMessageTemplates[type];
    }

    /**
     * Build message according to the type specified.
     * 
     * @param type  Type of the message.
     * @param args  Arguments used to construct the message. The number and types of the arguments depends on the type.
     * @return      Message content.
     */
    public String buildMessage(byte type, Object... args) {
        return String.format(getMessageTemplate(type), args);
    }
}
