package com.mediatek.rcse.plugin.phone;

import android.content.Context;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;
import com.mediatek.phone.ext.CallNotifierExtension ;


public class RCSeCallNotifierExtension extends CallNotifierExtension{
	
	/**
     * callNotifier Phone state changed
     */
    public void onPhoneStateChanged(CallManager cm, Context context) {
    	RCSeInCallUIExtension.getInstance().onPhoneStateChanged(cm);
    }

    /**
     * 
     * @param cn 
     * @return 
     */
    public boolean onDisconnect(Connection cn) {
    	
        return false;
    }


}
