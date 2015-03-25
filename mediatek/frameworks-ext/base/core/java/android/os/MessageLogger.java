package android.os;

import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.util.Printer;

import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Date;
import java.text.SimpleDateFormat;
import android.os.MessageQueue;
import android.app.ANRManagerNative;
import java.lang.Thread;
import java.lang.StackTraceElement;

/**
 * @hide
 */
public class MessageLogger implements Printer {
    private static final String TAG = "MessageLogger";
    /// M: enable message history debugging log
    public static boolean mEnableLooperLog = false;
    private LinkedList mMessageHistoryRecord = new LinkedList();
    private LinkedList mLongTimeMessageHistoryRecord = new LinkedList();
    private LinkedList mMessageTimeRecord = new LinkedList();
    private LinkedList mNonSleepMessageTimeRecord = new LinkedList();
    private LinkedList mNonSleepLongTimeRecord = new LinkedList();
    private LinkedList mElapsedLongTimeRecord = new LinkedList();
    final static int MESSAGE_SIZE = 20 * 2;// One for dispatching, one for
    // finished
    final static int LONGER_TIME_MESSAGE_SIZE = 40 ;// One for dispatching, one for
    // finished
    final static int LONGER_TIME = 200; //ms
    final static int FLUSHOUT_SIZE = 1024*2; //ms
    private  String mLastRecord = null;
    private  long mLastRecordKernelTime;            //Unir: Milli
    private  long mNonSleepLastRecordKernelTime;    //Unit: Milli
    private  long mLastRecordDateTime;            //Unit: Micro
    private  int mState = 0;
    private  long mMsgCnt = 0;
    private String mName = null;
    /// M: Fix LongMsgHistory to __exp_main.txt
    private String MSL_Warn = "MSL Waraning:";
    private String sInstNotCreated = MSL_Warn + "!!! MessageLoggerInstance might not be created !!!\n";
    /// M: Add message history/queue to _exp_main.txt
    private  String messageInfo = "";

    public MessageLogger() {
    }

    public MessageLogger(boolean mValue) {
        mEnableLooperLog = mValue;
    }
    public MessageLogger(boolean mValue, String Name) {
        mName = Name;
        mEnableLooperLog = mValue;
    }

    public long wallStart;                 //Unit:Micro
    public long wallTime;                 //Unit:Micro
    public long nonSleepWallStart;        //Unit:Milli
    public long nonSleepWallTime;        //Unit:Milli
    public void addTimeToList(LinkedList mList, long startTime, long durationTime) {
        mList.add(startTime);
        mList.add(durationTime);
        return;
    }

    public void println(String s) {
        synchronized (mMessageHistoryRecord) {
            mState++;
            int size = mMessageHistoryRecord.size();
            if (size > MESSAGE_SIZE) {
                mMessageHistoryRecord.removeFirst();
                mMessageTimeRecord.removeFirst();
                mNonSleepMessageTimeRecord.removeFirst();
            }
            s = "Msg#:" + mMsgCnt + " " + s;
            mMsgCnt++;

            mMessageHistoryRecord.add(s);
            mLastRecordKernelTime = SystemClock.elapsedRealtime();
            mNonSleepLastRecordKernelTime = SystemClock.uptimeMillis();
            mLastRecordDateTime = SystemClock.currentTimeMicro();
            if( mState%2 == 0) {
                mState = 0;
                wallTime = SystemClock.currentTimeMicro() - wallStart;
                nonSleepWallTime = SystemClock.uptimeMillis() - nonSleepWallStart;
                addTimeToList(mMessageTimeRecord, wallStart, wallTime);
                addTimeToList(mNonSleepMessageTimeRecord, nonSleepWallStart, nonSleepWallTime);

                if(nonSleepWallTime >= LONGER_TIME) {
                    if(mLongTimeMessageHistoryRecord.size() >= LONGER_TIME_MESSAGE_SIZE)
                    {
                        mLongTimeMessageHistoryRecord.removeFirst();
                        for(int i = 0; i < 2 ;i++)
                        {
                            mNonSleepLongTimeRecord.removeFirst();
                            mElapsedLongTimeRecord.removeFirst();
                        }
                    }

                    mLongTimeMessageHistoryRecord.add(s);
                    addTimeToList(mNonSleepLongTimeRecord,wallStart,nonSleepWallTime);
                    addTimeToList(mElapsedLongTimeRecord,wallStart,wallTime);
                   
                }
            } else {
                wallStart = SystemClock.currentTimeMicro();
                nonSleepWallStart = SystemClock.uptimeMillis();

                    /*
                       Test Longer History Code.
                     */
                    /*
                       if(mMsgCnt%3 == 0 && nonSleepWallStart > 2*LONGER_TIME) {
                       nonSleepWallStart -= 2*LONGER_TIME;
                       }
                     */

                    /*
                       Test Longer History Code
                       ================================.
                     */

            }

            if (mEnableLooperLog) {
                if (s.contains(">")) {
                    Log.d(TAG,"Debugging_MessageLogger: " + s + " start");
                } else {
                    Log.d(TAG,"Debugging_MessageLogger: " + s + " spent " + wallTime / 1000 + "ms");
                }
            }
        }
    }

    private int sizeToIndex( int size) {
            return --size;
    }
    public void setInitStr(String str_tmp){
         messageInfo = str_tmp;
    }
    private boolean flushedOrNot(StringBuilder sb, boolean bl ) {
        if(sb.length() > FLUSHOUT_SIZE && !bl) {
            //Log.d(TAG, "After Flushed, Current Size Is:" + sb.length() + ",bool" + bl);
            sb.append("***Flushing, Current Size Is:" + sb.length() + ",bool" + bl +"***TAIL\n");
            bl = true;
            /// M: Add message history/queue to _exp_main.txt
            messageInfo = messageInfo + sb.toString();
            Log.d(TAG, sb.toString());
            //Why New the one, not Clear the one? -> http://stackoverflow.com/questions/5192512/how-to-clear-empty-java-stringbuilder
            //Performance  is better for new object allocation.
            //sb = new StringBuilder(1024);
            sb.delete(0,sb.length());
        }
        else if(bl) {
            bl = false;
        }
        return bl;
            /*
               Test Longer History Code.
             */
            /*
               sb.append("***Current Size Is:" + sb.length() + "***\n");
             */
            /*
               Test Longer History Code.
               ================
             */
    }
        /*
           Test Longer History Code.
         */
        /*
           private static int DumpRound = 0;
         */
        /*
           Test Longer History Code.
           ================
         */

        public String dumpMsgQueueFromCurrentThread()
        {
            Looper currLooper = null;
            MessageQueue currMsgQueue = null;
            do{
                if(null == (currLooper = Looper.getMainLooper())) {
                    Log.d(TAG,MSL_Warn + "!!! Current MainLooper is Null !!!\n");
                    messageInfo = messageInfo + MSL_Warn + "!!! Current MainLooper is Null !!!\n";
                    break;
                }
                else if(null == (currMsgQueue = currLooper.getQueue())) {
                    Log.d(TAG,MSL_Warn + "!!! Current MainLooper's MsgQueue is Null !!!\n");
                    messageInfo = messageInfo + MSL_Warn + MSL_Warn + "!!! Current MainLooper's MsgQueue is Null !!!\n";
                    break;
                }
                return currMsgQueue.dumpMessageQueue();
            }while(false);

            boolean flushed = false;
            StringBuilder history = new StringBuilder(1024);
//            history.append(String.format("PID:%d(%s),TID:%d(%s),Thread's type is %s",));
            history.append(String.format(MSL_Warn + "!!! Calling thread from PID:%d's TID:%d(%s),Thread's type is %s!!!\n",
                        android.os.Process.myPid(),
                        Thread.currentThread().getId(),
                        Thread.currentThread().getName(),
                        Thread.currentThread().getClass().getName()));

           StackTraceElement[] stkTrace = Thread.currentThread().getStackTrace();
           history.append(String.format(MSL_Warn + "!!! get StackTrace: !!!\n"));
           for(int index = 0;  index < stkTrace.length; index++)
           {
               history.append(String.format(MSL_Warn + "File:%s's Linenumber:%d, Class:%s, Method:%s\n",
                       stkTrace[index].getFileName(),
                       stkTrace[index].getLineNumber(),
                       stkTrace[index].getClassName(),
                       stkTrace[index].getMethodName()));
               flushed = flushedOrNot(history,flushed);
           }

           if(!flushed){
               messageInfo = messageInfo + history.toString();
               Log.d(TAG,history.toString());
           }

           return "";
        }
        public void dump() {
        synchronized (mMessageHistoryRecord) {

                Log.d(TAG, ">>> Entering MessageLogger.dump. to Dump MSG HISTORY <<<\n");
                /// M: Add message history/queue to _exp_main.txt
                messageInfo +=  ">>> Entering MessageLogger.dump. to Dump MSG HISTORY <<<\n";
                if(null == mMessageHistoryRecord || 0 == mMessageHistoryRecord.size())
                {
                    Log.d(TAG,sInstNotCreated);
                    messageInfo = messageInfo+ sInstNotCreated + dumpMsgQueueFromCurrentThread();
                    try {
                        ANRManagerNative.getDefault().informMessageDump(messageInfo, Process.myPid());
                    } catch (RemoteException ex) {
                        Log.d(TAG, "informMessageDump exception " + ex);
                    }
                    return ;
                }
                /*
                   Test Longer History Code.
                 */
                /*
                   history.append("=== DumpRound:" + DumpRound + " ===\n");
                   DumpRound++;
                 */
                /*
                   Test Longer History Code.
                   ================
                 */
                boolean flushed = false;
                StringBuilder history = new StringBuilder(1024);
            history.append("MSG HISTORY IN MAIN THREAD:\n");
            history.append("Current kernel time : " + SystemClock.uptimeMillis() + "ms\n");
            SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
            //State = 1 means the current dispatching message has not been finished
                int sizeForMsgRecord =  mMessageHistoryRecord.size();
            if (mState == 1) {
                Date date = new Date((long)mLastRecordDateTime/1000);
                long spent = SystemClock.elapsedRealtime() - mLastRecordKernelTime;
                long nonSleepSpent = SystemClock.uptimeMillis()- mNonSleepLastRecordKernelTime;

                history.append("Last record : " + mMessageHistoryRecord.getLast());
                history.append("\n");
                history.append("Last record dispatching elapsedTime:" + spent + " ms/upTime:"+ nonSleepSpent +" ms\n");
                history.append("Last record dispatching time : " + simpleDateFormat.format(date));
                history.append("\n");
                sizeForMsgRecord --;
            }

            String msg = null;
            Long time = null;
            Long nonSleepTime = null;
            flushed = false;
            for (;sizeForMsgRecord > 0; sizeForMsgRecord--) {
                msg = (String)mMessageHistoryRecord.get(sizeToIndex(sizeForMsgRecord));
                time = (Long)mMessageTimeRecord.get(sizeToIndex(sizeForMsgRecord));
                nonSleepTime = (Long)mNonSleepMessageTimeRecord.get(sizeToIndex(sizeForMsgRecord));
                if (msg.contains(">")) {
                    Date date = new Date((long)time.longValue()/1000);
                    history.append(msg + " from " + simpleDateFormat.format(date));
                    history.append("\n");
                } else {
                    history.append(msg + " elapsedTime:" + time/1000 + " ms/upTime:" + nonSleepTime +" ms");
                    history.append("\n");
                }

                flushed = flushedOrNot(history, flushed);
            }

            history.append("=== Finish Dumping MSG HISTORY===\n");

            /*Dump for LongerTimeMessageRecord*/
            flushed = false;
            history.append("=== LONGER MSG HISTORY IN MAIN THREAD ===\n");
            sizeForMsgRecord = mLongTimeMessageHistoryRecord.size();
            int indexForTimeRecord = mNonSleepLongTimeRecord.size() - 1;
            for ( ;sizeForMsgRecord > 0; sizeForMsgRecord--, indexForTimeRecord-=2) {
                msg = (String)mLongTimeMessageHistoryRecord.get(sizeToIndex(sizeForMsgRecord));
                nonSleepTime = (Long) mNonSleepLongTimeRecord.get(indexForTimeRecord);
                time = (Long) mNonSleepLongTimeRecord.get(indexForTimeRecord-1);
                Date date = new Date((long)time.longValue()/1000);
                history.append(msg + " from " + simpleDateFormat.format(date) + " elapsedTime:"+ (((Long)(mElapsedLongTimeRecord.get(indexForTimeRecord))).longValue()/1000)+" ms/upTime:" + nonSleepTime +"ms");
                history.append("\n");
                flushed = flushedOrNot(history, flushed);
            }
            history.append("=== Finish Dumping LONGER MSG HISTORY===\n");
            /// M: Add message history/queue to _exp_main.txt
            messageInfo = messageInfo + history.toString();
            Log.d(TAG, history.toString());
//                history.delete(0,history.length())
            // Dump message queue
            /// M: Add message history/queue to _exp_main.txt
            /// M: Message history v1.5 @{
            try {
                messageInfo = messageInfo + dumpMsgQueueFromCurrentThread();
                ANRManagerNative.getDefault().informMessageDump(new String(messageInfo), Process.myPid());
                messageInfo = "";
            } catch (RemoteException ex) {
                Log.d(TAG, "informMessageDump exception " + ex);
            }
        /// M: Message history v1.5 @{
        }
    }
    /// MSG Logger Manager @}
}
