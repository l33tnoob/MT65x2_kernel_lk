package com.mediatek.common.msgmonitorservice;

import android.os.IBinder;

oneway interface IMessageLoggerWrapper {
    void unregisterMsgLogger(String msgLoggerName);
    void dumpAllMessageHistory();
    void dumpMSGHistorybyName(String msgLoggerName);
}

