package android.os;

import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.util.Printer;

import java.util.HashMap;
import java.util.Iterator;

import com.mediatek.common.msgmonitorservice.IMessageLoggerWrapper;
import com.mediatek.common.msgmonitorservice.IMessageLogger;
import com.mediatek.common.MediatekClassFactory;
import android.content.Context;
import java.util.concurrent.ConcurrentHashMap;

import android.os.HandlerThread;
import android.os.Process;
import android.os.ServiceManager;
import android.os.IBinder;
import android.os.MessageLogger;


public class MessageMonitorLogger extends MessageLogger {
    private final static String TAG = "MessageMonitorLogger";
    protected static boolean sEnableLooperLog;
    protected static HashMap<MSGLoggerInfo, MessageMonitorLogger> sMap = new HashMap<MSGLoggerInfo, MessageMonitorLogger>();
    protected String mName;

    public static ConcurrentHashMap<Message, MonitorMSGInfo> monitorMsg = new ConcurrentHashMap<Message, MonitorMSGInfo>();
    private MessageMonitorLogger mInstance;
    private IMessageLoggerWrapper mMessageLoggerWrapper = null;
    public static HandlerThread mHandleThread;
    public static final int START_MONITOR_EXECUTION_TIMEOUT_MSG = 3001;
    public static final int START_MONITOR_PENDING_TIMEOUT_MSG = 3002;
    public static MsgLoggerHandler mMsgLoggerHandler = null;
    public static final int DISABLE_MONITOR_EXECUTION_TIMEOUT_MSG = -1;
    public static final int DISABLE_MONITOR_PENDING_TIMEOUT_MSG = -1;
    private static MessageLoggerCallbacks mMessageLoggerCallbacks;
    private static final boolean IS_USER_BUILD = "user".equals(Build.TYPE) || "userdebug".equals(Build.TYPE);


    public static class MSGLoggerInfo
    {
        public MSGLoggerInfo(){}
        public String msgLoggerName;
        public int msgLoggerPid;
        public int msgLoggerTid;
    }

    public static class MonitorMSGInfo
    {
        MonitorMSGInfo(){}
        String msgLoggerName;
        Message msg;
        long executionTimeout;
    }

    class MsgLoggerHandler extends Handler {
        public MsgLoggerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            IBinder b = ServiceManager.getService(Context.MESSAGE_MONITOR_SERVICE);
            IMessageLogger msgLoggerManager = IMessageLogger.Stub.asInterface(b);
            Iterator keyiter = sMap.keySet().iterator();
            Iterator<MonitorMSGInfo> valueiter = monitorMsg.values().iterator();
            MonitorMSGInfo msgMonitorInfo;
            switch (msg.what)
            {
                case START_MONITOR_EXECUTION_TIMEOUT_MSG:
                case START_MONITOR_PENDING_TIMEOUT_MSG:
                    if( null==mMessageLoggerCallbacks ) {
                        msgMonitorInfo = (MonitorMSGInfo)msg.obj;
                        if( null!=msgMonitorInfo ) {
                            Log.d(TAG,"Monitor message timeout begin.");
                            while(keyiter.hasNext()) {
                                MSGLoggerInfo key=(MSGLoggerInfo)keyiter.next();
                                if( msgMonitorInfo.msgLoggerName == key.msgLoggerName) {
                                    dumpMessageHistory(key.msgLoggerName);
                                    try {
                                        msgLoggerManager.dumpCallStack(Process.myPid());
                                    } catch(Exception e) {
                                        Log.d(TAG,"DumpCallStack fail"+e);
                                    }
                                }
                            }
                            mMsgLoggerHandler.removeMessages(START_MONITOR_PENDING_TIMEOUT_MSG);
                            mMsgLoggerHandler.removeMessages(START_MONITOR_EXECUTION_TIMEOUT_MSG);
                            while (valueiter.hasNext()) {
                                if(msgMonitorInfo == valueiter.next()) {
                                    valueiter.remove();
                                    break;
                                }
                            }
                            Log.d(TAG,"Monitor message timeout end.");
                        }
                    } else {
                        mMessageLoggerCallbacks.onMessageTimeout(msg);
                    }
                    break;
                    default:
                        break;

            }
        }
    };

    /// MSG Logger Manager @}
    public MessageMonitorLogger(String name) {
        mName = name;
        /// M: MSG Logger Manager @{
        mInstance = this;
        IBinder b = ServiceManager.getService(Context.MESSAGE_MONITOR_SERVICE);
        IMessageLogger msgLoggerManager = IMessageLogger.Stub.asInterface(b);

        try {
            // Create service instance by MediatekClassFactory.
            mMessageLoggerWrapper = MediatekClassFactory.createInstance(IMessageLoggerWrapper.class, mInstance);
            msgLoggerManager.registerMsgLogger(name, Process.myPid(), Process.myTid(), mMessageLoggerWrapper);
            Log.d(TAG, "Register message logger successfully name= "+name);
        } catch (Exception e) {
            Log.d(TAG, "Register message logger fail " + e);
        }

        if(null==mHandleThread) {
            mHandleThread = new HandlerThread("MSGLoggerMonitorThread");
            mHandleThread.start();
            mMsgLoggerHandler= new MsgLoggerHandler(mHandleThread.getLooper());
        } else {
            Log.d(TAG,"Message Monitor HandlerThread has exist "+mHandleThread); 
        }
        /// MSG Logger Manager @}
    }

    /// M: MSG Logger Manager @{
    public static MessageMonitorLogger newMessageLogger(boolean mValue, String name, MessageLoggerCallbacks callback) {
        if(!IS_USER_BUILD) {
            sEnableLooperLog = mValue;
            mMessageLoggerCallbacks = callback;

            Iterator iter = sMap.keySet().iterator();
            while(iter.hasNext()) {
                MSGLoggerInfo key=(MSGLoggerInfo)iter.next();
                if( name.equals(key.msgLoggerName) ) {
                    sMap.remove(key);
                    break;
                }
            }

            MessageMonitorLogger logger = new MessageMonitorLogger(name);

            MSGLoggerInfo msgLoggerInfo = new MSGLoggerInfo();
            msgLoggerInfo.msgLoggerName = name;
            msgLoggerInfo.msgLoggerPid = Process.myPid();
            msgLoggerInfo.msgLoggerTid = Process.myTid();
            sMap.put(msgLoggerInfo, logger);
            /// MSG Logger Manager @}
            return logger;
        }
        else
            return null;
    }

    /// M: MSG Logger Manager @{
    public void dumpMessageHistory(String name) {
        if (sMap == null)
            return;

        Iterator iter = sMap.keySet().iterator();
        while(iter.hasNext()) {
            MSGLoggerInfo key=(MSGLoggerInfo)iter.next();
            if( name.equals(key.msgLoggerName) ) {
                MessageMonitorLogger logger = sMap.get(key);
                if (logger != null)
                    logger.dump();
                break;
            }
        }
    }

    public void unregisterMsgLogger(String msgLoggerName) {
        Iterator iter = sMap.keySet().iterator();
        while(iter.hasNext()) {
            MSGLoggerInfo key=(MSGLoggerInfo)iter.next();
            if( msgLoggerName.equals(key.msgLoggerName) ) {
                sMap.remove(key);
                break;
            }
        }
    }
    /// MSG Logger Manager @}

    /// M: MSG Logger Manager
    public void dumpAllMessageHistory() {
        if (sMap == null)
            return;
        Iterator<MessageMonitorLogger> it = sMap.values().iterator();
        while (it.hasNext())
            it.next().dump();
    }

    /// M: MSG Logger Manager @{
    public interface MessageLoggerCallbacks {
        public void onMessageTimeout(Message msg);
    }

    public static Handler getMsgLoggerHandler() {
        if(null!=mMsgLoggerHandler)
            return mMsgLoggerHandler;
        else {
            Log.d(TAG,"Monitor message handler is null");
            return null;
        }
    }
    /// MSG Logger Manager @}
}

