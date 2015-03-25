package com.mediatek.telephony;

import java.util.Arrays;
import java.util.regex.Pattern;

import android.content.Context;
import android.content.res.Resources;

/**
 *@hide
 */
public class PplMessageManager {
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
		public static final byte INSTRUCTION_DESCRIPTION2 = 11;
	}
	
	private static final String[] SMS_TEMPLATES = {
		"我的手机可能被盗，请保留发送此短信的号码。",
		"#suoding#",
		"已接受到您的锁屏指令，锁屏成功。",
		"#jiesuo#",
		"已接受到您的解锁指令，解锁成功。",
		"#mima#",
		"您的手机防盗密码为%s。",
		"#xiaohui#",
		"远程删除数据已开始。",
		"远程数据删除已完成，您的隐私得到保护，请放心。",
		"我开启了手机防盗功能，已将你的手机号码设置为紧急联系人号码，这样手机丢失也能够远程控制啦。\n以下是相关指令：\n远程锁定： #suoding#\n远程销毁数据： #xiaohui#\n找回密码： #mima#",
		"",
	};
	
	private static final String[] SMS_PATTERNS = {
		"我的手机可能被盗，请保留发送此短信的号码。",
		" *#suoding# *",
		"已接受到您的锁屏指令，锁屏成功。",
		" *#jiesuo# *",
		"已接受到您的解锁指令，解锁成功。",
		" *#mima# *",
		"您的手机防盗密码为[0-9]*。",
		" *#xiaohui# *",
		"远程删除数据已开始。",
		"远程数据删除已完成，您的隐私得到保护，请放心。",
		"我开启了手机防盗功能，已将你的手机号码设置为紧急联系人号码，这样手机丢失也能够远程控制啦。\n以下是相关指令：\n远程锁定： #suod",
		"ing#\n远程销毁数据： #xiaohui#\n找回密码： #mima#",
	};

	/**
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
		
		public static final String KEY_ID = "id";
		public static final String KEY_TYPE = "type";
		public static final String KEY_NUMBER = "number";
		public static final String KEY_SIM_ID = "simId";
		public static final String KEY_FIRST_TRIAL = "firstTrial";
		public static final String KEY_SEGMENT_INDEX = "segmentIndex";
		
		// the following properties are persistent
		public long id;
		public byte type;
		public String number;
		// the following properties are transient
		public int simId;
		public String content;

		public static final int PENDING_MESSAGE_LENGTH = PplControlData.TRUSTED_NUMBER_LENGTH + Long.SIZE/Byte.SIZE + Byte.SIZE/Byte.SIZE;
		
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
		 * @param buffer should be zeroed after the offset
		 * @param offset
		 */
		public void encode(byte[] buffer, int offset) {
			buffer[offset++] = type;
			byte[] idBytes = long2bytes(id);
			System.arraycopy(idBytes, 0, buffer, offset, idBytes.length);
			offset += Long.SIZE/Byte.SIZE;
			byte[] numberBytes = number.getBytes();
			if (numberBytes.length > PplControlData.TRUSTED_NUMBER_LENGTH) {
				throw new Error("Destination number is too long");
			} else {
				numberBytes = Arrays.copyOf(numberBytes, PplControlData.TRUSTED_NUMBER_LENGTH);
			}
			System.arraycopy(numberBytes, 0, buffer, offset, numberBytes.length);
		}
		
		public void decode(byte[] buffer, int offset) {
			type = buffer[offset++];
			id = bytes2long(buffer, offset);
			offset += Long.SIZE/Byte.SIZE;
			int j = offset;
			for (; j < offset + PplControlData.TRUSTED_NUMBER_LENGTH; ++j) {
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

	public PplMessageManager(Context context) {
		mContext = context;
		Resources resources = mContext.getResources();
		mMessageTemplates = SMS_TEMPLATES;//resources.getStringArray(R.array.sms_template_list);
		String patternStrings[] = SMS_PATTERNS;//resources.getStringArray(R.array.sms_pattern_list);
		mMessagePatterns = new Pattern[patternStrings.length];
		for (int i = 0; i < patternStrings.length; ++i) {
			mMessagePatterns[i] = Pattern.compile(patternStrings[i], Pattern.CASE_INSENSITIVE);
		}
	}

	/**
	 * Use the index into message patterns as the type. Therefore the definition
	 * of MessageManager.Type has to sync with R.array.sms_template_list and
	 * R.array.sms_pattern_list.
	 * 
	 * @param message
	 *            message body
	 * @return message type of the content if matched, Type.INVALID otherwise.
	 */
	public byte getMessageType(String message) {
		byte result = Type.INVALID;
		for (byte i = 0; i < mMessagePatterns.length; ++i) {
			if (mMessagePatterns[i].matcher(message).matches()) {
				result = i;
			}
		}
		if (result == Type.INSTRUCTION_DESCRIPTION2) {
			result = Type.INSTRUCTION_DESCRIPTION;
		}
		return result;
	}

	public String getMessageTemplate(byte type) {
		return mMessageTemplates[type];
	}

	public String buildMessage(byte type, Object... args) {
		return String.format(getMessageTemplate(type), args);
	}
}
