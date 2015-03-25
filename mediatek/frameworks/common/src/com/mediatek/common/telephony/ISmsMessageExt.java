
package com.mediatek.common.telephony;

import android.telephony.SmsMessage;

public interface ISmsMessageExt {
    public byte[] getTpdu(SmsMessage msg, int slotId);
    public byte[] getSmsc(SmsMessage msg, int slotId);
}
