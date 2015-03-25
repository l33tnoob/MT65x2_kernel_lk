package com.mediatek.common.ppl;

import android.os.Bundle;

public interface IPplSmsFilter {
	String KEY_PDUS = "pdus";
	String KEY_FORMAT = "format";
	String KEY_SIM_ID = "simId";
	String KEY_SMS_TYPE = "smsType"; // 0 - MT, 1 - MO

	/**
	 * Whether need to filter the message out from saving & broadcasting.
	 * 
	 * @param params
	 * @return
	 */
	boolean pplFilter(Bundle params);
}
