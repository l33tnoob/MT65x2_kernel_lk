package com.mediatek.msglogger;

import com.mediatek.common.msgmonitorservice.IMessageLogger;
import android.util.Log;
import com.mediatek.common.msgmonitorservice.IMessageLoggerWrapper;
import android.os.MessageMonitorLogger;
import java.util.HashMap;
import java.util.Iterator;
import android.os.Process;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.HandlerThread;


/**
  * @hide
  */
public class MessageMonitorService extends IMessageLogger.Stub {
    static final String TAG = "MessageMonitorService";
    protected static HashMap<MessageMonitorLogger.MSGLoggerInfo, IMessageLoggerWrapper> sMSGLoggerMap = new HashMap<MessageMonitorLogger.MSGLoggerInfo, IMessageLoggerWrapper>();
    protected MessageMonitorHandler mMsgMonitorHandler;
    protected static final int REGISTER_MSG_LOGGER = 2000;
    protected static final int UNREGISTER_MSG_LOGGER = 2001;
    protected static final int DUMP_ALL_MSG_HISTORY = 2002;
    protected static final int DUMP_MSG_HISTORY_BY_NAME = 2003; 
    protected static final int DUMP_CALL_STACK = 2004;
    private static HandlerThread mHandleThread;
    private static final int SIGNAL_STKFLT = 16;

    public MessageMonitorService() {
        if(null==mHandleThread) {
            mHandleThread = new HandlerThread("MessageMonitorServiceThread");
            mHandleThread.start();
            mMsgMonitorHandler= new MessageMonitorHandler(mHandleThread.getLooper());
            Log.d(TAG,"Create MessageMonitorServiceThread");
        } else {
            Log.d(TAG,"MessageMonitorServiceThread has exist. "); 
        }
    }

    public void registerMsgLogger(String msgLoggerName, int pid, int tid, IMessageLoggerWrapper callback) {
        Log.d(TAG,"RegisterMsgLogger "+msgLoggerName);
        MessageMonitorLogger.MSGLoggerInfo msgLoggerInfo = new MessageMonitorLogger.MSGLoggerInfo();
        msgLoggerInfo.msgLoggerName = msgLoggerName;
        msgLoggerInfo.msgLoggerPid = pid;
        msgLoggerInfo.msgLoggerTid = tid;
        sMSGLoggerMap.put(msgLoggerInfo, callback); 
    }

    public void unregisterMsgLogger(String msgLoggerName) {
        Message msg = mMsgMonitorHandler.obtainMessage(UNREGISTER_MSG_LOGGER, msgLoggerName);
        mMsgMonitorHandler.sendMessage(msg);
    }

    public void dumpAllMessageHistory(int pid) {
        Message msg = mMsgMonitorHandler.obtainMessage(DUMP_ALL_MSG_HISTORY, pid, 0);
        mMsgMonitorHandler.sendMessage(msg);
    }

    public void dumpMSGHistorybyName(String msgLoggerName) {
        Message msg = mMsgMonitorHandler.obtainMessage(DUMP_MSG_HISTORY_BY_NAME, msgLoggerName);
        mMsgMonitorHandler.sendMessage(msg);
    }

    public void dumpCallStack(int pid) {
        Message msg = mMsgMonitorHandler.obtainMessage(DUMP_CALL_STACK, pid, 0);
        mMsgMonitorHandler.sendMessage(msg);
    }


    private class MessageMonitorHandler extends Handler {
        public MessageMonitorHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            
            switch (msg.what) 
            {
                case UNREGISTER_MSG_LOGGER:
                    String msgLoggerName = (String)msg.obj;
                    Iterator iter = sMSGLoggerMap.keySet().iterator();
                    try {
                        while(iter.hasNext()) 
                        {
                            MessageMonitorLogger.MSGLoggerInfo key=(MessageMonitorLogger.MSGLoggerInfo)iter.next();
                            if( msgLoggerName.equals(key.msgLoggerName) )
                            {
                                sMSGLoggerMap.remove(key);
                                sMSGLoggerMap.get(key).unregisterMsgLogger(msgLoggerName);
                                break;
                            }
                        }
                    } catch(Exception e) {
                        Log.d(TAG,"UnregisterMsgLogger fail "+e);
                    }
                    Log.d(TAG,"UnregisterMsgLogger "+msgLoggerName);
                    break;
                    
                case DUMP_ALL_MSG_HISTORY:
                    int pid = msg.arg1;
                    iter = sMSGLoggerMap.keySet().iterator();
                    try {
                        while(iter.hasNext()) 
                        {
                            MessageMonitorLogger.MSGLoggerInfo key=(MessageMonitorLogger.MSGLoggerInfo)iter.next();
                            if( pid==key.msgLoggerPid )
                            {
                                sMSGLoggerMap.get(key).dumpAllMessageHistory();
                                break;
                            }
                        }
                    } catch(Exception e) {
                        Log.d(TAG,"DumpAllMessageHistory fail "+e);
                    }
                    Log.d(TAG,"DumpAllMessageHistory  pid= "+pid);
                    break;

                case DUMP_MSG_HISTORY_BY_NAME:
                    msgLoggerName = (String)msg.obj;
                    iter = sMSGLoggerMap.keySet().iterator();
                    try {
                        while(iter.hasNext()) 
                        {
                            MessageMonitorLogger.MSGLoggerInfo key=(MessageMonitorLogger.MSGLoggerInfo)iter.next();
                            if( msgLoggerName.equals(key.msgLoggerName) )
                            {
                                sMSGLoggerMap.get(key).dumpMSGHistorybyName(msgLoggerName);
                                break;
                            }
                            else
                            {
                                Log.d(TAG,"Doesn't have msgLogger "+msgLoggerName);
                            }
                        }
                    } catch(Exception e) {
                        Log.d(TAG,"DumpMSGHistorybyName fail "+e);
                    }
                    Log.d(TAG,"DumpMSGHistorybyName "+msgLoggerName);
                    break;

                case DUMP_CALL_STACK: 
                    pid = msg.arg1;
                    Process.sendSignal(pid, SIGNAL_STKFLT);
                    Log.d(TAG,"DumpCallStack pid= "+pid);
                    break;

                default :
                    break;
            }
        }
    }
}



