package com.hissage.util.queue;

import com.hissage.struct.SNmsAssertAlertMsgData;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.struct.SNmsProgress;
import com.hissage.struct.SNmsRegistrationOverData;

public final class NmsMessage {

    // private final static int NMS_CH_PER_LINE=30;

    // data members.
    private int identify = 0;
    private byte pMsg[] = null;
    private int msgLen = 0;
    private Object param = null;

    // constructor.
    public NmsMessage(int id, byte[] pMsg, int msgLen) {
        identify = id;
        this.pMsg = pMsg;
        this.msgLen = msgLen;
    }

    public NmsMessage(int id, Object param) {
        identify = id;
        this.param = param;
    }

    public int getIdentity() {
        return identify;
    }

    public byte[] getMsgData() {
        return pMsg;
    }

    public int getMsgDataLen() {
        return msgLen;
    }

    public SNmsMsgKey getNmsMsgKey() {

        if (param != null)
            return (SNmsMsgKey) param;
        else
            return null;
    }

    public SNmsProgress getSNmsProgress() {
        if (param != null)
            return (SNmsProgress) param;
        else
            return null;
    }

    public SNmsAssertAlertMsgData getAssertAlertMsgData() {
        if (param != null)
            return (SNmsAssertAlertMsgData) param;

        return null;
    }
    
    public SNmsRegistrationOverData getRegOverData(){
        if (param != null)
            return (SNmsRegistrationOverData) param;

        return null;
    }
    
}
