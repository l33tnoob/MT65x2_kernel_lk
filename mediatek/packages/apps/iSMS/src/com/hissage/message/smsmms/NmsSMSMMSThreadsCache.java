package com.hissage.message.smsmms;

import java.util.ArrayList;
import java.util.HashMap;

import com.hissage.service.NmsService;
import com.hissage.util.data.NmsImportantList;
import com.hissage.util.log.NmsLog;

public class NmsSMSMMSThreadsCache {
    private static final String TAG = "NmsSMSMMSThreadsCache";
    private static HashMap<Long, Short> threadsCache = null;
    static NmsSMSMMSThreadsCache mInstance;

    private NmsSMSMMSThreadsCache() {
        threadsCache = new HashMap<Long, Short>();
    }

    public static NmsSMSMMSThreadsCache getInstance() {
        if (null == mInstance) {
            mInstance = new NmsSMSMMSThreadsCache();
        }
        return mInstance;
    }

    public void add(long threadId, short engineContactId) {
        NmsLog.trace(TAG, "add threadId: " + threadId + "engineContactId: " + engineContactId);
        synchronized (threadsCache) {
            threadsCache.put(threadId, engineContactId);
        }
    }

    public short get(long threadId) {
        Short engineContactId = threadsCache.get(threadId);
        if (null == engineContactId){
            engineContactId = -1;
        }
        NmsLog.trace(TAG, "get threadId: " + threadId + "engineContactId: " + engineContactId);
        return engineContactId;
    }

    public void remove(long threadId) {
        NmsLog.trace(TAG, "remove threadId: " + threadId);        
        synchronized (threadsCache) {
            threadsCache.remove(threadId);
        }
    }
}
