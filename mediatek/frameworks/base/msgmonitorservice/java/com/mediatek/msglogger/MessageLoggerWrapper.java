package com.mediatek.msglogger;

import com.mediatek.common.msgmonitorservice.IMessageLoggerWrapper;
import android.os.MessageMonitorLogger;
import android.util.Log;

/**
 * @hide
 */
public class MessageLoggerWrapper extends IMessageLoggerWrapper.Stub {
    protected static MessageMonitorLogger mMsgLogger;
    private final static String TAG = "MessageLoggerWrapper";
    public MessageLoggerWrapper( MessageMonitorLogger msgLogger) {
         mMsgLogger = msgLogger;
    }

    public void unregisterMsgLogger(String msgLoggerName) {
        mMsgLogger.unregisterMsgLogger(msgLoggerName);
    }

    public void dumpAllMessageHistory() {
        mMsgLogger.dumpAllMessageHistory();
    }

    public void dumpMSGHistorybyName(String msgLoggerName) {
        mMsgLogger.dumpMessageHistory(msgLoggerName);
    }
}

