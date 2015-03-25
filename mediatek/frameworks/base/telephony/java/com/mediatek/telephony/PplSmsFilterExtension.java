package com.mediatek.telephony;

import java.util.List;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.PhoneNumberUtils;
import android.util.Log;

import com.android.internal.telephony.SmsConstants;
import com.android.internal.telephony.gsm.SmsMessage;
import com.android.internal.telephony.GeminiSmsMessage;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.common.ppl.IPplSmsFilter;

/**
 *@hide
 */
public class PplSmsFilterExtension extends ContextWrapper implements IPplSmsFilter {
	private static final String TAG = "PPL/PplSmsFilterExtension";

	public static final String INTENT_REMOTE_INSTRUCTION_RECEIVED = "com.mediatek.ppl.REMOTE_INSTRUCTION_RECEIVED";
	public static final String INSTRUCTION_KEY_TYPE = "Type";
	public static final String INSTRUCTION_KEY_FROM = "From";
	public static final String INSTRUCTION_KEY_TO = "To";
	public static final String INSTRUCTION_KEY_SIM_ID = "SimId";

	private final IPplAgent mAgent;
	private final PplMessageManager mMessageManager;
	private final boolean mEnabled;

	public PplSmsFilterExtension(Context context) {
		super(context);
		Log.d(TAG, "PplSmsFilterExtension enter");
		if (!FeatureOption.MTK_PRIVACY_PROTECTION_LOCK) {
		    mAgent = null;
		    mMessageManager = null;
		    mEnabled = false;
		    return;
		}
		
		IBinder binder = ServiceManager.getService("PPLAgent");
		if (binder == null) {
			Log.e(TAG, "Failed to get PPLAgent");
			mAgent = null;
			mMessageManager = null;
			mEnabled = false;
			return;
		}

		mAgent = IPplAgent.Stub.asInterface(binder);
		if (mAgent == null) {
			Log.e(TAG, "mAgent is null!");
			mMessageManager = null;
			mEnabled = false;
			return;
		}

        mMessageManager = new PplMessageManager(context);
	    mEnabled = true;
		Log.d(TAG, "PplSmsFilterExtension exit");
	}

	@Override
	public boolean pplFilter(Bundle params) {
		Log.d(TAG, "pplFilter(" + params + ")");
		if (!mEnabled) {
			return false;
		}

		boolean isMO = (params.getInt(KEY_SMS_TYPE) == 1);
		String format = params.getString(KEY_FORMAT);
		// only GSM is supported
		if (!format.equals(SmsConstants.FORMAT_3GPP)) {
			return false;
		}

		int simId = params.getInt(KEY_SIM_ID);

		Object[] messages = (Object[]) params.getSerializable(KEY_PDUS);
		if (messages == null) {
			return false;
		}
		byte[][] pdus = new byte[messages.length][];
		for (int i = 0; i < messages.length; i++) {
			pdus[i] = (byte[]) messages[i];
		}
		int pduCount = pdus.length;
		GeminiSmsMessage[] msgs = new GeminiSmsMessage[pduCount];
		for (int i = 0; i < pduCount; i++) {
			msgs[i] = GeminiSmsMessage.createFromPdu(pdus[i], format, simId);
		}

		Log.d(TAG, "pplFilter: pdus is " + pdus + " with length " + pdus.length);
		Log.d(TAG, "pplFilter: pdus[0] is " + pdus[0]);
		String content = msgs[0].getMessageBody();
		Log.d(TAG, "pplFilter: message content is " + content);
		if (content == null) {
			return false;
		}
		PplControlData controlData = null;
		try {
			controlData = PplControlData.buildControlData(mAgent.readControlData());
		} catch (RemoteException e) {
			e.printStackTrace();
			return false;
		}
		
		if (controlData == null || !controlData.isEnabled()) {
		    return false;
		}
		
		String dst = null;
		String src = null;

        if (isMO) {
            dst = msgs[0].getDestinationAddress();
    		Log.d(TAG, "pplFilter: dst is " + dst);
            if (!matchNumber(dst, controlData.TrustedNumberList)) {
			    return false;
		    }
        } else {
            src = msgs[0].getOriginatingAddress();
    		Log.d(TAG, "pplFilter: src is " + src);
            if (!matchNumber(src, controlData.TrustedNumberList)) {
			    return false;
		    }
        }

		byte instruction = mMessageManager.getMessageType(content);
		if (instruction == PplMessageManager.Type.INVALID) {
			return false;
		}

		Intent intent = new Intent(INTENT_REMOTE_INSTRUCTION_RECEIVED);
		intent.putExtra(INSTRUCTION_KEY_TYPE, instruction);
		intent.putExtra(INSTRUCTION_KEY_SIM_ID, simId);

        if (isMO) {
    		intent.putExtra(INSTRUCTION_KEY_TO, dst);
        } else {
    		intent.putExtra(INSTRUCTION_KEY_FROM, src);
        }
		startService(intent);

		return true;
	}

	private boolean matchNumber(String number, List<String> numbers) {
	    if (number != null && numbers != null) {
		    for (String s : numbers) {
			    if (PhoneNumberUtils.compare(s, number)) {
				    return true;
			    }
		    }
	    }
		return false;
	}

	private static void hexdump(byte[] data) {
		final int ROW_BYTES = 16;
		final int ROW_QTR1 = 3;
		final int ROW_HALF = 7;
		final int ROW_QTR2 = 11;
		int			rows, residue, i, j;
		byte[]		save_buf= new byte[ ROW_BYTES+2 ];
		char[]		hex_buf = new char[ 4 ];
		char[]		idx_buf = new char[ 8 ];
		char[]		hex_chars = new char[20];

		hex_chars[0] = '0';
		hex_chars[1] = '1';
		hex_chars[2] = '2';
		hex_chars[3] = '3';
		hex_chars[4] = '4';
		hex_chars[5] = '5';
		hex_chars[6] = '6';
		hex_chars[7] = '7';
		hex_chars[8] = '8';
		hex_chars[9] = '9';
		hex_chars[10] = 'A';
		hex_chars[11] = 'B';
		hex_chars[12] = 'C';
		hex_chars[13] = 'D';
		hex_chars[14] = 'E';
		hex_chars[15] = 'F';

		rows = data.length >> 4;
		residue = data.length & 0x0000000F;
		for ( i = 0 ; i < rows ; i++ )
		{
			int hexVal = (i * ROW_BYTES);
			idx_buf[0] = hex_chars[ ((hexVal >> 12) & 15) ];
			idx_buf[1] = hex_chars[ ((hexVal >> 8) & 15) ];
			idx_buf[2] = hex_chars[ ((hexVal >> 4) & 15) ];
			idx_buf[3] = hex_chars[ (hexVal & 15) ];

			String idxStr = new String( idx_buf, 0, 4 );
			System.out.print( idxStr + ": " );

			for ( j = 0 ; j < ROW_BYTES ; j++ )
			{
				save_buf[j] = data[ (i * ROW_BYTES) + j ];

				hex_buf[0] = hex_chars[ (save_buf[j] >> 4) & 0x0F ];
				hex_buf[1] = hex_chars[ save_buf[j] & 0x0F ];

				System.out.print( hex_buf[0] );
				System.out.print( hex_buf[1] );
				System.out.print( ' ' );

				if ( j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2 )
					System.out.print( " " );

				if ( save_buf[j] < 0x20 || save_buf[j] > 0x7E )
					save_buf[j] = (byte) '.';
			}

			String saveStr = new String( save_buf, 0, j );
			System.out.println( " | " + saveStr + " |" );
		}

		if ( residue > 0 )
		{
			int hexVal = (i * ROW_BYTES);
			idx_buf[0] = hex_chars[ ((hexVal >> 12) & 15) ];
			idx_buf[1] = hex_chars[ ((hexVal >> 8) & 15) ];
			idx_buf[2] = hex_chars[ ((hexVal >> 4) & 15) ];
			idx_buf[3] = hex_chars[ (hexVal & 15) ];

			String idxStr = new String( idx_buf, 0, 4 );
			System.out.print( idxStr + ": " );

			for ( j = 0 ; j < residue ; j++ )
			{
				save_buf[j] = data[ (i * ROW_BYTES) + j ];

				hex_buf[0] = hex_chars[ (save_buf[j] >> 4) & 0x0F ];
				hex_buf[1] = hex_chars[ save_buf[j] & 0x0F ];

				System.out.print( (char)hex_buf[0] );
				System.out.print( (char)hex_buf[1] );
				System.out.print( ' ' );

				if ( j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2 )
					System.out.print( " " );

				if ( save_buf[j] < 0x20 || save_buf[j] > 0x7E )
					save_buf[j] = (byte) '.';
			}

			for ( /*j INHERITED*/ ; j < ROW_BYTES ; j++ )
			{
				save_buf[j] = (byte) ' ';
				System.out.print( "   " );
				if ( j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2 )
					System.out.print( " " );
			}

			String saveStr = new String( save_buf, 0, j );
			System.out.println( " | " + saveStr + " |" );
		}
	}

}
