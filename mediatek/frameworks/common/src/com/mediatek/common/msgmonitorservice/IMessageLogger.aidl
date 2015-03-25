package com.mediatek.common.msgmonitorservice;

import android.os.IBinder;
import com.mediatek.common.msgmonitorservice.IMessageLoggerWrapper;


oneway interface IMessageLogger {
    void registerMsgLogger(String msgLoggerName, int pid, int tid, IMessageLoggerWrapper callback);
    void unregisterMsgLogger(String msgLoggerName);
    void dumpAllMessageHistory(int pid);
    void dumpMSGHistorybyName(String msgLoggerName);
    void dumpCallStack(int Pid);

}
