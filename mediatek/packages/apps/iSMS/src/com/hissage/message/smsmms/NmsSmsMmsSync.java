package com.hissage.message.smsmms;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.Handler;
import android.os.Message;

import com.hissage.config.NmsConfig;
import com.hissage.db.NmsContentResolver;
import com.hissage.jni.engineadapter;
import com.hissage.service.NmsService;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;

public class NmsSmsMmsSync extends Handler {
    private static final String TAG = "NmsSmsMmsSync";
    public static final int CHANGE_THREAD = 0x01;

    private final int ONCHANG_ACTION_ILLEGAL = -1;
    private final int ONCHANG_ACTION_NEW = 0;
    private final int ONCHANG_ACTION_DELETE = 1;
    private final int ONCHANG_ACTION_UPDATE = 2;

    private final String sms = " sms ";
    private final String mms = " mms ";

    private String[] PROJECTION_SMS = new String[] { NmsSMSMMS._ID, NmsSMSMMS.DATE, NmsSMSMMS.READ,
            NmsSMSMMS.TYPE, NmsSMSMMS.STATUS, NmsSMSMMS.LOCKED };
    private String[] PROJECTION_SMS_EXT = new String[] { NmsSMSMMS._ID, NmsSMSMMS.DATE,
            NmsSMSMMS.READ, NmsSMSMMS.TYPE, NmsSMSMMS.STATUS, NmsSMSMMS.LOCKED,
            NmsSMSMMS.IP_RECORDID_EXTENTION };
    private String[] PROJECTION_MMS = new String[] { NmsSMSMMS._ID, NmsSMSMMS.DATE, NmsSMSMMS.READ,
            NmsSMSMMS.MSG_BOX, NmsSMSMMS.LOCKED };
    private String[] PROJECTION_THREADS = new String[] {NmsSMSMMS._ID+","+NmsSMSMMS.RECIPIENT_IDS+" from threads -- "};    

    private static NmsSmsMmsSync mInstance = null;

    private ArrayList<NmsDirtyMsg> mMMSList = new ArrayList<NmsDirtyMsg>();
    private ArrayList<NmsDirtyMsg> mSMSList = new ArrayList<NmsDirtyMsg>();
    private ArrayList<NmsDirtyThreads> mThreadsList = new ArrayList<NmsDirtyThreads>();

    private Timer mTimer = null;
    private ContentResolver mCrMsg = null;
    private static Context mContext = null;
    private int nEventCount = 0;
    private Cursor mCsrSms = null;
    private Cursor mCsrMms = null;
    private Cursor mCsrThreads = null;
    private boolean mFlag = true;
    private static boolean mExtetionFieldExsit = false;

    public class NmsDirtyMsg {
        int id = -1;
        int readed = -1;
        long date = 0;
        int status = 0;
        int deliver = NmsSMSMMS.SMS_DELIVER_UNKNOWN;
        int action = ONCHANG_ACTION_ILLEGAL;
        int isLocked = SNmsMsgCont.NMS_MSG_UNLOCKED;
        int nmsRecId = 0;

        @Override
        public String toString() {
            return "" + id;
        }
    }

    public class NmsDirtyThreads {
        long id = -1;
        long recipient_ids = -1;

        @Override
        public String toString() {
            return "" + id;
        }
    }
    
    class OwnTimerTask extends TimerTask {
        @Override
        public void run() {
            processThreadChange();
        }
    }

    public static NmsSmsMmsSync getInstance(Context c) {
        if (null == mInstance) {
            mContext = c;
            mInstance = new NmsSmsMmsSync();
            mExtetionFieldExsit = (NmsSMSMMSManager.getInstance(c).isExtentionFieldExsit() == 1);
        }
        return mInstance;
    }

    private NmsSmsMmsSync() {
        super();
        initSMSList();
        initMMSList();
        initThreadsList();
        mTimer = new Timer();
    }

    private ContentResolver getCR() {
        if (null == mCrMsg) {
            mCrMsg = NmsService.getInstance().getContentResolver();
        }
        return mCrMsg;
    }

    public void pauseOnchange() // added by luozheng in 12.05.02
    {
        mFlag = false;
        NmsService.getService().removeThreadObserver();
    }

    public void resumeOnchange() {
        mFlag = true;
        handleMessage(null);
        NmsService.getService().addThreadObserver();
    }

    public void handleMessage(Message message) {
        NmsLog.trace(TAG, "recv onChange Event.");
        if ((0 == nEventCount) && mFlag) {
            nEventCount++;
            mTimer.schedule(new OwnTimerTask(), 4000);
            NmsLog.trace(TAG, "start onChangeHandler Timer.");
        }
    }

    private void initSMSList() {
        Cursor smsCursor = null;
        try {
            smsCursor = NmsContentResolver.query(getCR(), NmsSMSMMS.SMS_CONTENT_URI,
                    mExtetionFieldExsit ? PROJECTION_SMS_EXT : PROJECTION_SMS, null, null,
                    "_id ASC");
            while (null != smsCursor && smsCursor.moveToNext()) {
                mSMSList.add(getDirtyMsg(smsCursor, NmsSMSMMS.PROTOCOL_SMS));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (smsCursor != null)
                smsCursor.close();
        }
    }

    private void initMMSList() {
        String SELECTION = NmsSMSMMS.MSG_BOX + " != " + NmsSMSMMS.SMS_TYPE_DRAFT
                + " and (m_type == 132 or m_type == 128 or m_type == 130 )";
        Cursor mmsCursor = null;
        try {
            mmsCursor = NmsContentResolver.query(getCR(), NmsSMSMMS.MMS_CONTENT_URI,

            PROJECTION_MMS, SELECTION, null, "_id ASC");

            while (null != mmsCursor && mmsCursor.moveToNext()) {
                mMMSList.add(getDirtyMsg(mmsCursor, NmsSMSMMS.PROTOCOL_MMS));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (mmsCursor != null)
                mmsCursor.close();
        }
    }

    private void initThreadsList() {       
        Cursor threadsCursor = null;
        try {
            threadsCursor = NmsContentResolver.query(getCR(), NmsSMSMMS.SMS_CONTENT_URI,
                    PROJECTION_THREADS, null, null, "_id ASC");
            while (null != threadsCursor && threadsCursor.moveToNext()) {
                mThreadsList.add(getDirtyThreads(threadsCursor));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (threadsCursor != null)
                threadsCursor.close();
        }
    }
    
    private void getAllMsgCursor() {
        releaseAllMsgCursor();
        String SELECTION_MMS = NmsSMSMMS.MSG_BOX + " != " + NmsSMSMMS.SMS_TYPE_DRAFT
                + " and (m_type == 132 or m_type == 128 or m_type == 130 )";
        mCsrSms = NmsContentResolver.query(getCR(), NmsSMSMMS.SMS_CONTENT_URI,
                mExtetionFieldExsit ? PROJECTION_SMS_EXT : PROJECTION_SMS, null, null, "_id ASC");
        mCsrMms = NmsContentResolver.query(getCR(), NmsSMSMMS.MMS_CONTENT_URI, PROJECTION_MMS,
                SELECTION_MMS, null, "_id ASC");
        mCsrThreads = NmsContentResolver.query(getCR(), NmsSMSMMS.SMS_CONTENT_URI, PROJECTION_THREADS,
                null, null, "_id ASC");
    }

    private void releaseAllMsgCursor() {
        if (null != mCsrSms) {
            mCsrSms.close();
            mCsrSms = null;
        }
        if (null != mCsrMms) {
            mCsrMms.close();
            mCsrMms = null;
        }
        if (null != mCsrThreads) {
            mCsrThreads.close();
            mCsrThreads = null;
        }        
    }

    private void processDelAction(NmsDirtyMsg dt, int protocol) {
        if (mExtetionFieldExsit && dt.nmsRecId != 0) {
            NmsLog.trace(TAG, "del msg, protocol: ip message" + " id:" + dt.id + ", nmsRecId: "
                    + dt.nmsRecId+", so ignore it.");
			/* TODO: remove it when release
            if (NmsConfig.isAndroidKitKatOnward && !NmsSMSMMSManager.isDefaultSmsApp())
                engineadapter.get().nmsUIDeleteMsg(new short[] { (short) dt.nmsRecId }, 1, 1, 0);*/
        } else {
            NmsLog.trace(TAG, "del msg, protocol:" + protocol + " id:" + dt.id);
            engineadapter.get().nmsSmsDeletedFromSystem(protocol, dt.id);
        }
    }

    private void processAddAction(NmsDirtyMsg dt, int protocol) {
        SNmsMsgCont msgCont = null;
        if (NmsSMSMMS.PROTOCOL_SMS == protocol) {
            if (mExtetionFieldExsit && dt.nmsRecId != 0) {
                NmsLog.error(TAG, "this is a ip message, so ignore and continue, _id: " + dt.id
                        + "nmsRecId: " + dt.nmsRecId);
                return;
            }
            msgCont = NmsSMSMMSManager.getInstance(mContext).getSmsMsgContViaId(dt.id);
        } else {
            msgCont = NmsSMSMMSManager.getInstance(mContext).getMmsMsgContViaId(dt.id);
        }
        if (null != msgCont) {
            NmsLog.trace(TAG, String.format(
                    "insert msg, protocol:%d, from: %s, to: %s, source: %d, body: %s, sim id: %d",
                    protocol, msgCont.pFrom, msgCont.pTo, msgCont.msgType, msgCont.pBody,
                    msgCont.simId));

            engineadapter.get().nmsProcessInterceptedSms(msgCont.pFrom, msgCont.pTo, msgCont.pBody,
                    protocol, (int) msgCont.msgId, msgCont.msgType, 0, msgCont.readed,
                    msgCont.deleteTime, msgCont.isLocked, msgCont.pThreadNumber,
                    (int) msgCont.simId);
        } else {
            NmsLog.error(TAG, "error, get message in system fail, id: " + dt.id + ", time: "
                    + dt.date);
        }
    }

    private void processUpdateAction(NmsDirtyMsg dt, int protocol) {
        SNmsMsgCont msgCont = null;
        if (NmsSMSMMS.PROTOCOL_SMS == protocol) {
            msgCont = NmsSMSMMSManager.getInstance(mContext).getSmsMsgContViaId(dt.id);
        } else {
            msgCont = NmsSMSMMSManager.getInstance(mContext).getMmsMsgContViaId(dt.id);
        }
        if (null != msgCont) {
            NmsLog.trace(TAG, String.format(
                    "update msg, protocol:%d, from: %s, to: %s, source: %d, body: %s", protocol,
                    msgCont.pFrom, msgCont.pTo, msgCont.msgType, msgCont.pBody));

            if (mExtetionFieldExsit && dt.nmsRecId != 0) {
                // engineadapter.get().nmsUpdateIpMsgSaved(dt.nmsRecId,
                // dt.isLocked) ;
            } else {
                engineadapter.get().nmsProcessUpdateSms(msgCont.pFrom, msgCont.pTo, msgCont.pBody,
                        protocol, (int) msgCont.msgId, msgCont.msgType, 0, msgCont.readed,
                        msgCont.deleteTime, msgCont.isLocked, msgCont.pThreadNumber,
                        (int) msgCont.simId);
            }
        } else {
            NmsLog.error(TAG, "error, get message in system fail, id: " + dt.id + ", time: "
                    + dt.date);
        }
    }

    private void processDirtyDataList(ArrayList<NmsDirtyMsg> dirtyDataList, int protocol) {
        for (NmsDirtyMsg dt : dirtyDataList) {
            if (ONCHANG_ACTION_NEW == dt.action) {
                processAddAction(dt, protocol);
            } else if (ONCHANG_ACTION_DELETE == dt.action) {
                processDelAction(dt, protocol);
            } else if (ONCHANG_ACTION_UPDATE == dt.action) {
                processUpdateAction(dt, protocol);
            } else {
                NmsLog.trace(TAG, "unknow ONCHANGE_ACTION type: " + dt.action + " protocol:"
                        + protocol + " id:" + dt.id);
            }

            if (!mExtetionFieldExsit
                    && (ONCHANG_ACTION_DELETE == dt.action || ONCHANG_ACTION_UPDATE == dt.action)) {
                Intent i = new Intent();
                i.setAction(NmsIntentStrId.NMS_INTENT_CANCEL_MMSSMS_NOTIFY);
                i.putExtra(NmsIntentStrId.NMS_INTENT_CANCEL_MMSSMS_NOTIFY, dt.id);
                mContext.sendBroadcast(i);
            }

        }
    }

    private void processDirtyThreadsList(ArrayList<Long> dirtyDataList) {
        for (Long dt : dirtyDataList) {
            NmsSMSMMSThreadsCache.getInstance().remove(dt);
        }
    }
    
    private void processMsgOnChange(Cursor csrIn, ArrayList<NmsDirtyMsg> sourceList, int protocol) {

        ArrayList<NmsDirtyMsg> dirtyDataList = new ArrayList<NmsDirtyMsg>();
        if ((null != csrIn) && (csrIn.getCount() > 0) && (csrIn.moveToFirst())) {
            int i = 0;
            for (i = 0; i < csrIn.getCount();) {
                csrIn.moveToPosition(i);
                NmsDirtyMsg currentDT = getDirtyMsg(csrIn, protocol);
                if (sourceList.size() > i) {
                    NmsDirtyMsg sourceDT = sourceList.get(i);

                    if (sourceDT.id == currentDT.id) {
                        if ((sourceDT.date != currentDT.date)
                                || (sourceDT.readed != currentDT.readed)
                                || (sourceDT.status != currentDT.status)
                                || (sourceDT.deliver != currentDT.deliver)
                                || (sourceDT.isLocked != currentDT.isLocked)) {
                            currentDT.action = ONCHANG_ACTION_UPDATE;
                            dirtyDataList.add(currentDT);
                            sourceList.get(i).date = currentDT.date;
                            sourceList.get(i).readed = currentDT.readed;
                            sourceList.get(i).status = currentDT.status;
                            sourceList.get(i).deliver = currentDT.deliver;
                            sourceList.get(i).isLocked = currentDT.isLocked;
                        }
                        i++;
                        continue;
                    } else {
                        sourceDT.action = ONCHANG_ACTION_DELETE;
                        dirtyDataList.add(sourceDT);
                        sourceList.remove(i);
                        continue;
                    }
                } else {
                    sourceList.add(currentDT);
                    currentDT.action = ONCHANG_ACTION_NEW;
                    dirtyDataList.add(currentDT);
                    i++;
                    continue;
                }
            }
            if (sourceList.size() > i) {
                for (int j = sourceList.size(); j > i; j--) {
                    NmsDirtyMsg sourceDT = sourceList.get(i);
                    sourceDT.action = ONCHANG_ACTION_DELETE;
                    dirtyDataList.add(sourceDT);
                    sourceList.remove(i);
                }
            }
        } else {
            NmsLog.trace(TAG, "no record in " + (protocol == NmsSMSMMS.PROTOCOL_MMS ? mms : sms)
                    + " db, may be is new phone or delete all by user.");

            dirtyDataList = (ArrayList<NmsDirtyMsg>) sourceList.clone();

            sourceList.clear();
            for (int i = 0; i < dirtyDataList.size(); ++i) {
                dirtyDataList.get(i).action = ONCHANG_ACTION_DELETE;
            }
        }

        NmsLog.trace(TAG, "this " + (protocol == NmsSMSMMS.PROTOCOL_MMS ? mms : sms)
                + " onChange dirty data size is: " + dirtyDataList.size()
                + ", ThreadListCache size is:" + sourceList.size());

        if (dirtyDataList.size() > 0) {
            processDirtyDataList(dirtyDataList, protocol);
        }

    }

    private void processThreadsOnChange(Cursor csrIn, ArrayList<NmsDirtyThreads> sourceList) {
        ArrayList<Long> removeList = new ArrayList<Long>();
        if ((null != csrIn) && (csrIn.getCount() > 0) && (csrIn.moveToFirst())) {
            int i = 0;
            for (i = 0; i < csrIn.getCount();) {
                csrIn.moveToPosition(i);
                NmsDirtyThreads currentDT = getDirtyThreads(csrIn);
                if (sourceList.size() > i) {
                    NmsDirtyThreads sourceDT = sourceList.get(i);

                    if (sourceDT.id == currentDT.id) {
                        if ((sourceDT.recipient_ids != currentDT.recipient_ids)) {
                            removeList.add( currentDT.id);
                            sourceList.get(i).recipient_ids = currentDT.recipient_ids;
                        }
                        i++;
                        continue;
                    } else {
                        removeList.add(sourceDT.id);
                        sourceList.remove(i);
                        continue;
                    }
                } else {
                    sourceList.add(currentDT);
                    removeList.add(currentDT.id);
                    i++;
                    continue;
                }
            }
            if (sourceList.size() > i) {
                for (int j = sourceList.size(); j > i; j--) {
                    NmsDirtyThreads sourceDT = sourceList.get(i);
                    removeList.add(sourceDT.id);
                    sourceList.remove(i);
                }
            }
        } else {
            NmsLog.trace(TAG, "no record in threads db, may be is new phone or delete all by user.");
            for (int i = 0; i < sourceList.size(); ++i) {
                removeList.add(sourceList.get(i).id);
            }
            sourceList.clear();
        }

        NmsLog.trace(TAG, "this threads onChange dirty data size is: " + removeList.size()
                + ", ThreadListCache size is:" + sourceList.size());

        if (removeList.size() > 0) {
            processDirtyThreadsList(removeList);
        }

    }
    
    private NmsDirtyMsg getDirtyMsg(Cursor cursor, int protocol) {
        NmsDirtyMsg dt = new NmsDirtyMsg();
        dt.id = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS._ID));
        dt.readed = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.READ));
        dt.date = cursor.getLong(cursor.getColumnIndex(NmsSMSMMS.DATE));
        dt.isLocked = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.LOCKED));

        if (protocol == NmsSMSMMS.PROTOCOL_MMS) {
            dt.status = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.MSG_BOX));
        } else {
            dt.status = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.TYPE));
            dt.deliver = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.STATUS));
            if (mExtetionFieldExsit) {
                dt.nmsRecId = cursor.getInt(cursor.getColumnIndex(NmsSMSMMS.IP_RECORDID_EXTENTION));
            }
        }
        return dt;
    }

    private NmsDirtyThreads getDirtyThreads(Cursor cursor){
        NmsDirtyThreads dt = new NmsDirtyThreads();
        dt.id = cursor.getLong(cursor.getColumnIndex(NmsSMSMMS._ID));
        dt.recipient_ids = cursor.getLong(cursor.getColumnIndex(NmsSMSMMS.RECIPIENT_IDS));
        return dt;
    }
    
    public synchronized void processThreadChange() {
        if (0 == nEventCount) {
            NmsLog.trace(TAG, "nEvntCount is 0, so ignor this sync.");
            return;
        }
        
        try {
        long lDate = System.currentTimeMillis();
        NmsConfig.setMaxNormalizedDate(lDate);
        getAllMsgCursor();
        processMsgOnChange(mCsrSms, mSMSList, NmsSMSMMS.PROTOCOL_SMS);
        processMsgOnChange(mCsrMms, mMMSList, NmsSMSMMS.PROTOCOL_MMS);
        processThreadsOnChange(mCsrThreads, mThreadsList);
        releaseAllMsgCursor();
        NmsLog.trace(TAG, "process thread change cost time:" + (System.currentTimeMillis() - lDate)
                + "ms");
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
        
        nEventCount = 0;
    }

}