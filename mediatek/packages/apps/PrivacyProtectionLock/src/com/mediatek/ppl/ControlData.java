package com.mediatek.ppl;

import java.util.Arrays;
import java.util.Comparator;
import java.util.LinkedList;
import java.util.List;

import android.util.Log;

import com.mediatek.ppl.MessageManager.PendingMessage;


/**
 * Control data which will be saved to NVRAM via ppl_agent.
 */
/*
 * struct ControlDataHeader {
 *  unsigned char   version;
 *  unsigned char   status;
 *  unsigned char   sim_fingerprint_n;
 *  unsigned char   trusted_number_n;
 *  unsigned char   pending_sms_n;
 *  unsigned char   reserved[3];
 *  unsigned char   secret[CD_SECRET_LENGTH];
 *  unsigned char   salt[CD_SALT_LENGTH];
 * } __attribute__ ((packed));
 */
public class ControlData {
    private static final String TAG = "PPL/ControlData";

    public static final byte VERSION = 0x01;
    public static final int SECRET_SIZE = 20;
    public static final int SALT_SIZE = 20;
    public static final int SECRET_LIST_LENGTH = 40;
    public static final int SALT_LIST_LENGTH = 40;
    public static final int SIM_FINGERPRINT_LENGTH = 40;
    public static final int TRUSTED_NUMBER_LENGTH = 40;
    public static final byte STATUS_PROVISIONED = 0x1;
    public static final byte STATUS_ENABLED = 0x2;
    public static final byte STATUS_LOCKED = 0x4;
    public static final byte STATUS_SIM_LOCKED = 0x8;
    public static final byte STATUS_WIPE_REQUESTED = 0x10;

    private static final int HEADER_SIZE = 8 + SECRET_SIZE + SALT_SIZE;

    public byte version = VERSION;
    public byte status = 0;
    public byte[] secret = new byte[SECRET_SIZE];
    public byte[] salt = new byte[SALT_SIZE];
    public List<byte[]> SimFingerPrintList = null;
    public List<String> TrustedNumberList = null;
    public List<MessageManager.PendingMessage> PendingMessageList = null;

	/**
	 * Serialize this object for PPLAgent to write.
	 * Layout of Control Data is:
	 *
	 * +---------------------+
	 * |Header               |
	 * +---------------------+
	 * |Body                 |
	 * +---------------------+
	 *
	 * Layout of Control Data is:
	 *
	 * +---------------------+
	 * |SIM Finger Print List|
	 * +---------------------+
	 * |Trusted Number List  |
	 * +---------------------+
	 * |Pending Message List |
	 * +---------------------+
	 *
	 * 
	 * @return The serialized byte stream.
	 */
    public byte[] encode() {
        byte[] result = new byte[getDataSize()];
        result[0] = version;
        result[1] = status;
        result[2] = SimFingerPrintList == null ? 0 : (byte) SimFingerPrintList.size();
        result[3] = TrustedNumberList == null ? 0 : (byte) TrustedNumberList.size();
        result[4] = PendingMessageList == null ? 0 : (byte) PendingMessageList.size();
        result[5] = 0;
        result[6] = 0;
        result[7] = 0;
        int offset = 8;
        System.arraycopy(secret, 0, result, offset, secret.length);
        offset += secret.length;
        System.arraycopy(salt, 0, result, offset, salt.length);
        offset += salt.length;
        if (SimFingerPrintList != null) {
            for (int i = 0; i < SimFingerPrintList.size(); ++i) {
                System.arraycopy(SimFingerPrintList.get(i), 0, result, offset, SIM_FINGERPRINT_LENGTH);
                offset += SIM_FINGERPRINT_LENGTH;
            }
        }
        if (TrustedNumberList != null) {
            for (int i = 0; i < TrustedNumberList.size(); ++i) {
                byte[] buffer = TrustedNumberList.get(i).getBytes();
                if (buffer.length > TRUSTED_NUMBER_LENGTH) {
                    throw new Error("Trusted number is too long");
                } else {
                    buffer = Arrays.copyOf(buffer, TRUSTED_NUMBER_LENGTH);
                }
                System.arraycopy(buffer, 0, result, offset, TRUSTED_NUMBER_LENGTH);
                offset += TRUSTED_NUMBER_LENGTH;
            }
        }
        if (PendingMessageList != null) {
            for (int i = 0; i < PendingMessageList.size(); ++i) {
                PendingMessageList.get(i).encode(result, offset);
                offset += MessageManager.PendingMessage.PENDING_MESSAGE_LENGTH;
            }
        }
        return result;
    }

    /**
     * Re-construct the object from byte stream.
     * 
     * @param data  Previously serialized object data stream.
     */
    public void decode(byte[] data) {
        version = data[0];
        status = data[1];
        byte numberOfSimFingerPrints = data[2];
        byte numberOfTrustedNumber = data[3];
        byte numberOfPendingMessage = data[4];
        int offset = 8;
        System.arraycopy(data, offset, secret, 0, secret.length);
        offset += secret.length;
        System.arraycopy(data, offset, salt, 0, salt.length);
        offset += salt.length;
        if (numberOfSimFingerPrints != 0) {
            SimFingerPrintList = new LinkedList<byte[]>();
            for (int i = 0; i < numberOfSimFingerPrints; ++i) {
                byte[] fingerprint = new byte[SIM_FINGERPRINT_LENGTH];
                System.arraycopy(data, offset, fingerprint, 0, SIM_FINGERPRINT_LENGTH);
                SimFingerPrintList.add(fingerprint);
                offset += SIM_FINGERPRINT_LENGTH;
            }
        } else {
            SimFingerPrintList = null;
        }
        if (numberOfTrustedNumber != 0) {
            TrustedNumberList = new LinkedList<String>();
            for (int i = 0; i < numberOfTrustedNumber; ++i) {
                int j = offset;
                for (; j < offset + TRUSTED_NUMBER_LENGTH; ++j) {
                    if (data[j] == 0) {
                        break;
                    }
                }
                TrustedNumberList.add(new String(data, offset, j - offset));
                offset += TRUSTED_NUMBER_LENGTH;
            }
        } else {
            TrustedNumberList = null;
        }
        if (numberOfPendingMessage != 0) {
            PendingMessageList = new LinkedList<MessageManager.PendingMessage>();
            for (int i = 0; i < numberOfPendingMessage; ++i) {
                PendingMessageList.add(new MessageManager.PendingMessage(data, offset));
                offset += MessageManager.PendingMessage.PENDING_MESSAGE_LENGTH;
            }
        } else {
            PendingMessageList = null;
        }
    }

    /*
     * Calculate the length of the serialized data. Keep this sync with the data structure.
     */
    private int getDataSize() {
        int result = HEADER_SIZE;
        if (SimFingerPrintList != null) {
            result += SIM_FINGERPRINT_LENGTH * SimFingerPrintList.size();
        }
        if (TrustedNumberList != null) {
            result += TRUSTED_NUMBER_LENGTH * TrustedNumberList.size();
        }
        if (PendingMessageList != null) {
            result += PendingMessage.PENDING_MESSAGE_LENGTH * PendingMessageList.size();
        }
        return result;
    }

    /**
     * Utility method to build ControlData object from byte stream.
     * 
     * @param data  Previously serialized object data stream.
     * @return      A ControlData object initialize according to data.
     */
    public static ControlData buildControlData(byte[] data) {
        ControlData result = new ControlData();
        if (data != null && data.length != 0) {
            result.decode(data);
        } else {
            Log.w(TAG, "buildControlData: data is empty, return empty instance");
        }
        return result;
    }

    /**
     * Create a deep copy of this object.
     * 
     * @return  The deep copy of this object
     */
    public ControlData clone() {
        ControlData result = new ControlData();
        result.version = version;
        result.status = status;
        result.secret = secret.clone();
        result.salt = salt.clone();
        if (SimFingerPrintList != null) {
            result.SimFingerPrintList = new LinkedList<byte[]>();
            for (int i = 0; i < SimFingerPrintList.size(); ++i) {
                result.SimFingerPrintList.add(SimFingerPrintList.get(i).clone());
            }
        } else {
            result.SimFingerPrintList = null;
        }
        if (TrustedNumberList != null) {
            // String is immutable so we can simply add the reference to it
            result.TrustedNumberList = new LinkedList<String>();
            for (String s : TrustedNumberList) {
                result.TrustedNumberList.add(s);
            }
        } else {
            result.TrustedNumberList = null;
        }
        if (PendingMessageList != null) {
            result.PendingMessageList = new LinkedList<PendingMessage>();
            for (MessageManager.PendingMessage pm : PendingMessageList) {
                result.PendingMessageList.add(pm.clone());
            }
        } else {
            PendingMessageList = null;
        }
        return result;
    }

    /**
     * Clear the contents of ControlData.
     */
    public void clear() {
        version = VERSION;
        status = 0;
        secret = new byte[SECRET_SIZE];
        salt = new byte[SALT_SIZE];
        SimFingerPrintList = null;
        TrustedNumberList = null;
        PendingMessageList = null;
    }

    public boolean isEnabled() {
        return (status & STATUS_ENABLED) == STATUS_ENABLED;
    }

    public void setEnable(boolean flag) {
        if (flag) {
            status |= STATUS_ENABLED;
        } else {
            status &= ~STATUS_ENABLED;
        }
    }

    public boolean hasWipeFlag() {
        return (status & STATUS_WIPE_REQUESTED) == STATUS_WIPE_REQUESTED;
    }

    public void setWipeFlag(boolean flag) {
        if (flag) {
            status |= STATUS_WIPE_REQUESTED;
        } else {
            status &= ~STATUS_WIPE_REQUESTED;
        }
    }

    public boolean isProvisioned() {
        return (status & STATUS_PROVISIONED) == STATUS_PROVISIONED;
    }

    public void setProvision(boolean flag) {
        if (flag) {
            status |= STATUS_PROVISIONED;
        } else {
            status &= ~STATUS_PROVISIONED;
        }
    }

    public boolean isLocked() {
        return (status & STATUS_LOCKED) == STATUS_LOCKED;
    }

    public void setLock(boolean flag) {
        if (flag) {
            status |= STATUS_LOCKED;
        } else {
            status &= ~STATUS_LOCKED;
        }
    }

    public boolean isSimLocked() {
        return (status & STATUS_SIM_LOCKED) == STATUS_SIM_LOCKED;
    }

    public void setSimLock(boolean flag) {
        if (flag) {
            status |= STATUS_SIM_LOCKED;
        } else {
            status &= ~STATUS_SIM_LOCKED;
        }
    }

    public static byte[][] sortSimFingerPrints(byte[][] input) {
        byte[][] result = input.clone();
        for (int i = 0; i < result.length; ++i) {
            if (result[i] != null) {
                result[i] = result[i].clone();
            }
        }
        Arrays.sort(result, mSimComparator);
        return result;
    }

    /**
     * Check whether the two list is different.
     * NOTE: lhs and rhs must be sorted.
     * 
     * @param lhs
     * @param rhs
     * @return  0 means lhs and rhs are the same, otherwise lhs and rhs are different.
     */
    public static int compareSimFingerPrints(byte[] lhs, byte[] rhs) {
        if (lhs.length != rhs.length) {
            throw new Error("The two fingerprints must have the same length");
        }
        for (int i = 0; i < lhs.length; ++i) {
            int difference = lhs[i] - rhs[i];
            if (difference != 0) {
                return difference;
            }
        }
        return 0;
    }

    /*
     * Comparator for SIM finger prints.
     */
    private static Comparator<byte[]> mSimComparator = new Comparator<byte[]>() {
        @Override
        public int compare(byte[] lhs, byte[] rhs) {
            return compareSimFingerPrints(lhs, rhs);
        }
    };

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("ControlData ");
        sb.append(this.hashCode())
                .append(" {")
                .append(Integer.toHexString(version))
                .append(", ")
                .append(Integer.toHexString(status))
                .append(", ")
                .append(SimFingerPrintList)
                .append(", ")
                .append(TrustedNumberList)
                .append(", ")
                .append(PendingMessageList)
                .append("}");
        return sb.toString();
    }
}
