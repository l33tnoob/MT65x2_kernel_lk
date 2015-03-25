package com.hissage.message.smsmms;

import android.database.ContentObserver;
import android.os.Handler;
import android.os.Message;

import com.hissage.util.log.NmsLog;

public class NmsThreadsObserver extends ContentObserver {
    private static final String TAG = "NmsThreadsObserver";
    private Handler mThreadChange = null;

    public NmsThreadsObserver(Handler handler) {
        super(handler);
        mThreadChange = handler;
    }

    @Override
    public void onChange(boolean selfChange) {
        super.onChange(selfChange);
        NmsLog.trace(TAG, "ThreadObserver recv onChange event, self flag is: " + selfChange);
        Message msgToSend = Message.obtain(mThreadChange, NmsSmsMmsSync.CHANGE_THREAD, 10, 0,
                null);
        mThreadChange.sendMessage(msgToSend);

    }
}
