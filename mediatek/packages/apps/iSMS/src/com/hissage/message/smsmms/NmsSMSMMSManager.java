package com.hissage.message.smsmms;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import com.hissage.R;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsFeatureSupport;
import com.hissage.platfrom.NmsMtkBinderApi;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.HpnsApplication;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

/**
 * The Class NmsSMSMMSManager, all of sms/mms process in this class, retrieve
 * sms/mms data or update/insert/delte etc.
 */
public class NmsSMSMMSManager {
    
    public static final String PRIVATE_BODY_PREFIX= "2$e+";

    private static final String TAG = "NmsSMSMMSManager";

    private static NmsSMSMMSManager mInstance = null;

    final static int SEEN_TYPE_UNCHECK = 0x00;

    final static int SEEN_TYPE_EXIST = 0x01;

    final static int SEEN_TYPE_UNEXIST = 0x02;

    static int mSeenType = SEEN_TYPE_UNCHECK;

    /**
     * The Class UpdateFlagAction, for the setFlags operation flag.
     */
    public static class UpdateFlagAction {

        public static final int NMS_UPDATE_SET_READED = 0;

        public static final int NMS_UPDATE_SET_BLOCKED = 1;

        public static final int NMS_UPDATE_SET_UN_BLOCKED = 2;

        public static final int NMS_UPDATE_SET_SAVED = 3;

        public static final int NMS_UPDATE_SET_UN_SAVED = 4;
    };

    /**
     * The extension field exsit flag: important\spam\sim_id, exsit: 1, not
     * exsit: 0, init: -1
     */
    static int mExtentionExsit = -1;

    /**
     * The MAX_COUNT. when set read\important\spam flag with transaction one
     * time, this is sms max number
     */
    private static final int MAX_COUNT = 500;

    private ContentResolver mCrMsg = null;

    Context mContext = null;

    private NmsSMSMMSManager(Context context) {
        if (context != null)
            mContext = context.getApplicationContext();
        getCT();
    }

    /**
     * sync sms and isms db, when the isms service init done, we must save all
     * of sms which after setMaxNormalizedDate recved.
     */
    public void nmsSyncSMS() {

        synchronized (mInstance) {
            NmsConfig.mIsDBInitDone = true;
            mInstance.notifyAll();
        }

        new Thread() {
            @Override
            public void run() {
                long lDate = System.currentTimeMillis();
                NmsLog.trace(TAG, "..................backup sms start.........");
                nmsRefreshMsg();
                nmsRefreshDraftMsg();
                NmsLog.trace(TAG, "..................backup sms end...  ......");
                NmsConfig.setMaxNormalizedDate(lDate);

                Intent intent = new Intent();
                intent.setAction(NmsIpMessageConsts.NMS_INTENT_SERVICE_READY);
                NmsService.getInstance().sendBroadcast(intent);

                Intent regAllIntent = new Intent(NmsService.getInstance(), NmsService.class);
                regAllIntent.putExtra(NmsService.regAllReciverNow, true);
                NmsService.getInstance().startService(regAllIntent);

            }
        }.start();
    }

    private void nmsPreloadIpMsgForIntegration() {
        if (NmsConfig.getFirstTimeRunForIntegration() < 1) {
            NmsConfig.setFirstTimeRunForIntegration();
            if (isExtentionFieldExsit() == 1) {
                for (int i = 0; i < 3; ++i) {
                    long id = saveSmsToDb(
                            NmsService.getInstance().getString(
                                    R.string.STR_NMS_PRELOAD_IP_MSG_CONTENT),
                            NmsSMSMMS.PRELOAD_IP_MSG_NUMBER, NmsSMSMMS.PRELOAD_IP_MSG_NUMBER,
                            SNmsMsgKey.NMS_MSG_STATUS_INBOX, 0,
                            (int) (System.currentTimeMillis() / 1000), 0, 0, 1);
                    if (id > 0) {
                        ContentValues cv = new ContentValues();
                        cv.put(NmsSMSMMS.SERVICE_CENTER, NmsSMSMMS.PRELOAD_IP_MSG_NUMBER);
                        cv.put("seen", 1);

                        Uri uri = ContentUris.withAppendedId(NmsSMSMMS.SMS_CONTENT_URI, id);
                        
                        NmsMtkBinderApi.getInstance().update(uri, cv, null, null);
                    }
                }
            }
        }
    }

    private void nmsRefreshMsg() {
        final String[] PROJECTION = new String[] { NmsSMSMMS._ID, NmsSMSMMS.MSG_BOX,
                NmsSMSMMS.TYPE, NmsSMSMMS.STATUS, NmsSMSMMS.SUB, NmsSMSMMS.DATE, NmsSMSMMS.READ,
                NmsSMSMMS.THREAD_ID, NmsSMSMMS.CT_T, NmsSMSMMS.ADDRESS, NmsSMSMMS.BODY,
                NmsSMSMMS.LOCKED };
        final String[] PROJECTION_EXT = new String[] { NmsSMSMMS._ID, NmsSMSMMS.MSG_BOX,
                NmsSMSMMS.TYPE, NmsSMSMMS.STATUS, NmsSMSMMS.SUB, NmsSMSMMS.DATE, NmsSMSMMS.READ,
                NmsSMSMMS.THREAD_ID, NmsSMSMMS.CT_T, NmsSMSMMS.ADDRESS, NmsSMSMMS.BODY,
                NmsSMSMMS.LOCKED, NmsSMSMMS.IP_RECORDID_EXTENTION, NmsSMSMMS.SIM_ID };

        final String SELECTION = String.format(NmsSMSMMS.NORM_DATE + " > %d",
                NmsConfig.getMaxNormalizedDate());
        Cursor csrAllMsg = null;
        try {
            csrAllMsg = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_MMS_CONTENT_URI,
                    1 == isExtentionFieldExsit() ? PROJECTION_EXT : PROJECTION, SELECTION, null,
                    NmsSMSMMS.NORM_DATE + " DESC LIMIT "
                            + (int) ((double) NmsCustomUIConfig.MAX_MSG_NUM * 0.8));

            if ((null == csrAllMsg)) {
                NmsLog.trace(TAG, "fail to get refresh msg list.");
                return;
            }

            if (false == csrAllMsg.moveToLast()) {
                NmsLog.trace(TAG, "fail to get refresh msg list.");
                csrAllMsg.close();
                return;
            }

            do {
                if (csrAllMsg.getString(csrAllMsg.getColumnIndex(NmsSMSMMS.MSG_BOX)) == null) {
                    nmsStoreSMS(csrAllMsg);
                } else {
                    nmsStoreMMS(csrAllMsg);
                }
            } while (csrAllMsg.moveToPrevious());

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != csrAllMsg) {
                csrAllMsg.close();
            }
        }
    }

    private void nmsRefreshDraftMsg() // only get sms draft, ignore mms draft;
    {
        final String[] PROJECTION = new String[] { NmsSMSMMS._ID, NmsSMSMMS.TYPE, NmsSMSMMS.STATUS,
                NmsSMSMMS.DATE, NmsSMSMMS.READ, NmsSMSMMS.THREAD_ID, NmsSMSMMS.BODY,
                NmsSMSMMS.LOCKED }; // added status value, in 12.04.05;

        final String SELECTION = String.format(NmsSMSMMS.DATE + " > %d" + " and " + NmsSMSMMS.TYPE
                + " = %d", NmsConfig.getMaxNormalizedDate(), NmsSMSMMS.SMS_TYPE_DRAFT);
        Cursor csrDraftMsg = null;
        try {
            csrDraftMsg = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI, PROJECTION,
                    SELECTION, null, null);

            if (null == csrDraftMsg) {
                NmsLog.trace(TAG, "fail to get draft refresh msg list.");
                return;
            }

            if (!csrDraftMsg.moveToFirst()) {
                NmsLog.trace(TAG, "get draft refresh msg list, but there are not any draft.");
                csrDraftMsg.close();
                return;
            }

            do {
                nmsStoreSMS(csrDraftMsg);
            } while (csrDraftMsg.moveToNext());
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != csrDraftMsg) {
                csrDraftMsg.close();
            }
        }
    }

    /**
     * save mms to isms db when nmsSyncSMS() , but just subject\_id\addr only,
     * except part.
     * 
     * @param csrIn
     *            the cursor of mms which need save to isms db.
     */
    private void nmsStoreMMS(Cursor csrIn) {
        SNmsMsgCont msgCont = getMmsMsgContViaCsr(csrIn);
        NmsLog.trace(TAG, String.format(
                "store mms. from: %s, to: %s, source: %d, read: %d, isLocked: %d.", msgCont.pFrom,
                msgCont.pTo, msgCont.msgType, msgCont.readed, msgCont.isLocked));
        engineadapter.get().nmsProcessInterceptedSms(msgCont.pFrom, msgCont.pTo, msgCont.pBody,
                NmsSMSMMS.PROTOCOL_MMS, (int) msgCont.msgId, msgCont.msgType, 0, msgCont.readed,
                msgCont.deleteTime, msgCont.isLocked, msgCont.pThreadNumber, (int) msgCont.simId);// just
        // store
        // as
        // sms;
    }

    /**
     * save sms to isms db when nmsSyncSMS() .
     * 
     * @param csrIn
     *            the cursor of sms which need save to isms db.
     */
    private void nmsStoreSMS(Cursor csrIn) {
        SNmsMsgCont msgCont = getSmsMsgContViaCsr(csrIn);
        NmsLog.trace(
                TAG,
                String.format(
                        "store sms. from: %s, to: %s, source: %d, read: %d, body: %s, isLocked: %d, is IpMsg: %s",
                        msgCont.pFrom, msgCont.pTo, msgCont.msgType, msgCont.readed, msgCont.pBody,
                        msgCont.isLocked, SNmsMsgCont.NMS_IS_HESINE_MSG(msgCont.source) ? "yes"
                                : "no" + ", current sim Id: " + msgCont.simId));
        if (SNmsMsgCont.NMS_IS_HESINE_MSG(msgCont.source)) {
            return;
        }
        engineadapter.get().nmsProcessInterceptedSms(msgCont.pFrom, msgCont.pTo, msgCont.pBody,
                NmsSMSMMS.PROTOCOL_SMS, (int) msgCont.msgId, msgCont.msgType, 0, msgCont.readed,
                msgCont.deleteTime, msgCont.isLocked, msgCont.pThreadNumber, (int) msgCont.simId);
    }

    /**
     * Gets the ipMsg id in isms db via system sms id.
     * 
     * @param sysId
     *            the id of system sms id
     * @return the record id in isms db id.
     */
    public short getNmsRecordIDViaSysId(long sysId) {
        short ret = -1;
        Cursor cursor = null;
        try {
            cursor = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                    new String[] { NmsSMSMMS.IP_RECORDID_EXTENTION }, NmsSMSMMS._ID + "=" + sysId,
                    null, null);
            if (cursor != null && cursor.moveToFirst()) {
                ret = cursor.getShort(cursor.getColumnIndex(NmsSMSMMS.IP_RECORDID_EXTENTION));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return ret;
    }

    /**
     * Gets the single instance of NmsSMSMMSManager.
     * 
     * @return single instance of NmsSMSMMSManager
     */
    public static NmsSMSMMSManager getInstance(Context context) {
        if (null == mInstance) {

            mInstance = new NmsSMSMMSManager(context);
        }
        return mInstance;
    }
    
    private ContentResolver getCR() {
        if (null == mCrMsg) {
            mCrMsg = getCT().getContentResolver();
        }
        return mCrMsg;
    } 

    private Context getCT() {
        if (null == mContext) 
            mContext = HpnsApplication.mGlobalContext ;
        return mContext;
    }

    /**
     * Gets the sms type via ip msg status.
     * 
     * @param nSmsStatus
     *            the ip msg status of isms defined.
     * @return the sms type of android system
     */
    public int getSmsTypeViaNmsMsgStatus(int nSmsStatus) {
        switch (nSmsStatus) {
        case SNmsMsgKey.NMS_MSG_STATUS_INBOX:
            return NmsSMSMMS.SMS_TYPE_INBOX;

        case SNmsMsgKey.NMS_MSG_STATUS_SENT:
        case SNmsMsgKey.NMS_MSG_STATUS_DELIVERED:
            return NmsSMSMMS.SMS_TYPE_SENT;

        case SNmsMsgKey.NMS_MSG_STATUS_FAILED:
            return NmsSMSMMS.SMS_TYPE_FAILED;

        case SNmsMsgKey.NMS_MSG_STATUS_DRAFT:
            return NmsSMSMMS.SMS_TYPE_DRAFT;

        case SNmsMsgKey.NMS_MSG_STATUS_OUTBOX_PENDING:
        case SNmsMsgKey.NMS_MSG_STATUS_OUTBOX:
            return NmsSMSMMS.SMS_TYPE_OUTBOX;

        default:
            NmsLog.trace(TAG, "unknown NmsMsgStatus:" + nSmsStatus);
            return NmsSMSMMS.SMS_TYPE_ALL;
        }
    }

    /**
     * Gets the ip msg source via sms type.
     * 
     * @param nSmsType
     *            the sms type of android system defined.
     * @return the ip msg source
     */
    public int getNmsMsgSourceViaSmsType(int nSmsType) {
        int nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_OUTBOX;
        switch (nSmsType) {
        case NmsSMSMMS.SMS_TYPE_INBOX:
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_INBOX;
            break;

        case NmsSMSMMS.SMS_TYPE_SENT:
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_SENT;
            break;

        case NmsSMSMMS.SMS_TYPE_FAILED:
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_FAILED;
            break;

        case NmsSMSMMS.SMS_TYPE_DRAFT:
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_DRAFT;
            break;

        case NmsSMSMMS.SMS_TYPE_OUTBOX:
        case NmsSMSMMS.SMS_TYPE_QUEUED:
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_OUTBOX;
            break;

        default:
            NmsLog.trace(TAG, "unknown sms type:" + nSmsType);
            nNmsMsgSource = SNmsMsgKey.NMS_MSG_STATUS_OUTBOX;
        }
        return nNmsMsgSource;
    }

    /**
     * Gets the msg address via thread id.
     * 
     * @param nThreadId
     *            thread id in threads table of mmssms.db.
     * @return the address of to/from.
     */
    public String getMsgAddressViaThreadId(int nThreadId) {
        String strAddr = "";
        String strRecId = null;
        String[] PROJECTION = new String[] { NmsSMSMMS.RECIPIENT_IDS + " from threads where "
                + NmsSMSMMS._ID + " = " + nThreadId + " -- " };
        NmsLog.trace(TAG, "start to get address via threadId, cmd:" + PROJECTION[0]);
        Cursor csrThread = null;
        try {
            csrThread = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI, PROJECTION,
                    null, null, null);
            if ((csrThread != null) && (csrThread.moveToFirst())) {
                strRecId = csrThread.getString(csrThread.getColumnIndex(NmsSMSMMS.RECIPIENT_IDS));
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != csrThread) {
                csrThread.close();
                csrThread = null;
            }
            if (strRecId == null) {
                NmsLog.trace(TAG, "fail to get address via nThreadId:" + nThreadId);
                return null;
            }
        }

        String[] strArrayRecId = strRecId.split(" ");
        for (String strSingleRecId : strArrayRecId) {
            if (TextUtils.isEmpty(strSingleRecId)) {
                NmsLog.trace(TAG, "fail to get address via nThreadId:" + nThreadId + ", unkown "
                        + NmsSMSMMS.RECIPIENT_IDS);
                continue;
            }
            String[] PROJECTION_ADDRESS = new String[] { NmsSMSMMS.ADDRESS
                    + " from canonical_addresses where _ID = " + strSingleRecId + " --" };
            try {
                csrThread = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                        PROJECTION_ADDRESS, null, null, null);
                if ((csrThread != null) && (csrThread.moveToFirst())) {
                    if (!TextUtils.isEmpty(strAddr)) {
                        strAddr = strAddr + ",";
                    }
                    strAddr = strAddr
                            + NmsCommonUtils.nmsGetStandardPhoneNum(csrThread.getString(csrThread
                                    .getColumnIndex(NmsSMSMMS.ADDRESS)));
                }
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
            } finally {
                if (null != csrThread) {
                    csrThread.close();
                    csrThread = null;
                }
            }
        }
        NmsLog.trace(TAG, "getMsgAddressViaThreadId return:" + strAddr);
        return strAddr;
    }

    /**
     * Gets the isms contact id via sms thread id.
     * 
     * @param threadId
     *            the thread id
     * @return the isms contact id.
     */
    public short getEngineContactIdViaThreadId(long threadId) {

        if (threadId < 0) {
            NmsLog.error(TAG, "threadId <=0,  return -1");
            return -1;
        } else if (threadId == 0) {
            return 0;
        }

        short engineContactId = -1;
        engineContactId = NmsSMSMMSThreadsCache.getInstance().get(threadId);
        if (engineContactId >= 0) {
            return engineContactId;
        }

        String strAddr = getMsgAddressViaThreadId((int) threadId);
        if (TextUtils.isEmpty(strAddr)) {
            NmsLog.error(TAG, "Number is null/empty! ThreadId: " + threadId);
            return -1;
        }

        engineContactId = (short) engineadapter.get().nmsUIGetContactId(strAddr);

        NmsSMSMMSThreadsCache.getInstance().add(threadId, engineContactId);
        return engineContactId;
    }

    private Bitmap getMmsImage(String _id, boolean resize, int resizeWidth, int resizeHeight) {
        Uri partURI = Uri.parse("content://mms/part/" + _id);
        InputStream is = null;
        Bitmap bitmap = null;
        try {
            if (resize) {
                Options options = NmsBitmapUtils.getOptions(getCR().openInputStream(partURI),
                        resizeWidth, resizeHeight);
                if (options != null) {
                    is = getCR().openInputStream(partURI);
                    bitmap = NmsBitmapUtils.getBitmapByInputStream(is, options);
                }
            } else {
                is = getCR().openInputStream(partURI);
                bitmap = BitmapFactory.decodeStream(is);
            }
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
        } catch (java.lang.OutOfMemoryError e) {
            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    NmsLog.nmsPrintStackTrace(e);
                }
            }
        }
        return bitmap;
    }

    /**
     * Gets the mms first image(first slide show)
     * 
     * @param _id
     *            the mms id in pdu table
     * @return the Bitmap
     */
    public Bitmap getFirstImgFromMMS(int mmsId, boolean resize, int resizeWidth, int resizeHeight) {
        String[] PROJECTION = new String[] { "_id", "ct" };
        String selectionPart = new String("mid=" + mmsId);
        Bitmap ret = null;

        Cursor cPart = null;
        try {
            cPart = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI_PART, PROJECTION,
                    selectionPart, null, null);
            while (cPart != null && cPart.moveToNext()) {
                String attachType = cPart.getString(cPart.getColumnIndex("ct"));
                String id = cPart.getString(cPart.getColumnIndex("_id"));
                if (attachType.equals("image/jpeg") || attachType.equals("image/bmp")
                        || attachType.equals("image/gif") || attachType.equals("image/jpg")
                        || attachType.equals("image/png")) {
                    ret = getMmsImage(id, resize, resizeWidth, resizeHeight);
                } else if (attachType.equals("text/x-vcard")) {
                    ret = NmsBitmapUtils.decodeSampledBitmapFromResource(mContext.getResources(),
                            R.drawable.all_media_contact, resizeWidth, resizeHeight);
                } else if (attachType.equals("text/x-vcalendar")) {
                    ret = NmsBitmapUtils.decodeSampledBitmapFromResource(mContext.getResources(),
                            R.drawable.all_media_calendar, resizeWidth, resizeHeight);
                } else {
                    ret = null;
                }

                if (ret != null) {
                    break;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {

            if (cPart != null) {
                cPart.close();
            }
        }

        return ret;
    }

    /**
     * Get mms special content type. ("text/x-vcard" and "text/x-vcalendar")
     * 
     * @param mmsId
     *            the mms id
     * @return if content type is vcard or calendar, return String of the
     *         contentType, other content type will return null.
     */
    public String getMmsSpecialContentType(int mmsId) {
        String[] PROJECTION = new String[] { "ct" };
        String selectionPart = new String("mid=" + mmsId);
        String ret = null;

        Cursor cPart = null;
        try {
            cPart = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI_PART, PROJECTION,
                    selectionPart, null, null);
            while (cPart != null && cPart.moveToNext()) {
                String attachType = cPart.getString(cPart.getColumnIndex("ct"));
                if (attachType.equals("text/x-vcard")) {
                    ret = "text/x-vcard";
                } else if (attachType.equals("text/x-vcalendar")) {
                    ret = "text/x-vcalendar";
                } else {
                    ret = null;
                }

                if (ret != null) {
                    break;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cPart != null) {
                cPart.close();
                cPart = null;
            }
        }

        return ret;
    }

    /**
     * Get mms special attach path. ("text/x-vcard" and "text/x-vcalendar")
     * 
     * @param mmsId
     *            the mms id
     * @return if content type is vcard or calendar, return String of the attach
     *         path, other content type will return null.
     */
    public String getMmsSpecialAttachPath(int mmsId) {
        Uri partURI = null;
        String attachPath = NmsCommonUtils.getSDCardPath(mContext) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "mmsAttach" + File.separator;
        String[] PROJECTION = new String[] { "_id", "ct", "name" };
        String selectionPart = new String("mid=" + mmsId);
        InputStream is = null;
        OutputStream os = null;

        if (!NmsCommonUtils.getSDCardStatus()) {
            MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SAVE);
            NmsLog.error(TAG, "parse mms attach got error, SD card is not available");
            return null;
        }

        if (NmsCommonUtils.isExistsFile(attachPath)) {
            NmsCommonUtils.delAllFile(attachPath);
        }
        Cursor cPart = null;
        try {
            cPart = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI_PART, PROJECTION,
                    selectionPart, null, null);
            while (cPart != null && cPart.moveToNext()) {
                String attachType = cPart.getString(cPart.getColumnIndex("ct"));
                if (attachType.equals("text/x-vcard") || attachType.equals("text/x-vcalendar")) {
                    String partId = cPart.getString(cPart.getColumnIndex("_id"));
                    String attachName = cPart.getString(cPart.getColumnIndex("name"));
                    attachPath = attachPath.concat(attachName);
                    partURI = Uri.parse("content://mms/part/" + partId);
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {

            if (cPart != null) {
                cPart.close();
                cPart = null;
            }
        }
        if (partURI == null) {
            NmsLog.error(TAG, "parse mms attach got error, invalid URI");
            return null;
        }

        File file = new File(attachPath);
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdirs();
        }
        file.delete();

        try {
            if (!file.createNewFile()) {
                return null;
            }
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            return null;
        }
        try {
            is = mContext.getContentResolver().openInputStream(partURI);
            os = new BufferedOutputStream(new FileOutputStream(file));
        } catch (FileNotFoundException e) {
            NmsLog.nmsPrintStackTrace(e);
            return null;
        }

        byte[] buffer = new byte[256];
        try {
            for (int len = 0; (len = is.read(buffer)) != -1;) {
                os.write(buffer, 0, len);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            return null;
        }

        return attachPath;
    }

    /**
     * Checks if is mms downloaded.
     * 
     * @param mmsId
     *            the mms id
     * @return true, if is mms downloaded
     */
    public boolean isMmsDownloaded(long mmsId) {
        String[] PROJECTION = new String[] { "m_id" };
        String selectionPart = new String("_id=" + mmsId);
        boolean ret = false;
        Cursor cPart = null;
        try {
            cPart = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI, PROJECTION,
                    selectionPart, null, null);
            if (cPart != null && cPart.moveToFirst()) {
                String m_id = cPart.getString(cPart.getColumnIndex("m_id"));
                if (TextUtils.isEmpty(m_id)) {
                    ret = false;
                } else {
                    ret = true;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cPart != null) {
                cPart.close();
            }
        }
        return ret;
    }

    /**
     * Gets the sms content via cursor.
     * 
     * @param csrIn
     *            the cursor
     * @return the sms content
     */
    public SNmsMsgCont getSmsMsgContViaCsr(Cursor csrIn) {
        if (null == csrIn) {
            NmsLog.error(TAG, "fail to get SMSMsgCont via cursor, cursor is null.");
            return null;
        }
        int nThreadId = -1;
        SNmsMsgCont msgCont = new SNmsMsgCont();
        msgCont.msgId = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS._ID));
        int type = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.TYPE));
        int status = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.STATUS));
        msgCont.pBody = csrIn.getString(csrIn.getColumnIndex(NmsSMSMMS.BODY));
        msgCont.readed = (byte) csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.READ));
        msgCont.deleteTime = (int) (csrIn.getLong(csrIn.getColumnIndex(NmsSMSMMS.DATE)) / 1000);
        msgCont.isLocked = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.LOCKED));
        msgCont.msgType = getNmsMsgSourceViaSmsType(type);

        if (1 == isExtentionFieldExsit()) {
            int index = csrIn.getColumnIndex(NmsSMSMMS.IP_RECORDID_EXTENTION);
            if (index >= 0) {
                int recdId = csrIn.getInt(index);
                if (recdId != 0) {
                    msgCont.source = SNmsMsgCont.NMS_MSG_SOURCE_HESINE;
                }
            }
        }

        int simIdIndex = csrIn.getColumnIndex(NmsSMSMMS.SIM_ID);
        if (simIdIndex < 0 || (msgCont.simId = csrIn.getInt(simIdIndex)) <= 0) {
            msgCont.simId = NmsPlatformAdapter.getInstance(getCT()).getCurrentSimId();
            NmsLog.trace(TAG, "no sim_id column in sms, so get current sim id: " + msgCont.simId);
        }

        switch (msgCont.msgType) {
        case SNmsMsgKey.NMS_MSG_STATUS_INBOX:
            msgCont.pFrom = NmsCommonUtils.nmsGetStandardPhoneNum(csrIn.getString(csrIn
                    .getColumnIndex(NmsSMSMMS.ADDRESS)));
            msgCont.pThreadNumber = msgCont.pFrom;
            break;

        case SNmsMsgKey.NMS_MSG_STATUS_DRAFT:
            nThreadId = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.THREAD_ID));
            msgCont.pTo = getMsgAddressViaThreadId(nThreadId);
            msgCont.pThreadNumber = msgCont.pTo;
            break;

        default:
            nThreadId = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.THREAD_ID));
            msgCont.pTo = NmsCommonUtils.nmsGetStandardPhoneNum(csrIn.getString(csrIn
                    .getColumnIndex(NmsSMSMMS.ADDRESS)));
            msgCont.pThreadNumber = getMsgAddressViaThreadId(nThreadId);
        }
        return msgCont;
    }

    /**
     * Gets the sms thread id in threads table via sms id in sms table.
     * 
     * @param sysMsgId
     *            the sms id
     * @return the thread id
     */
    public long getThreadViaSysMsgId(long sysMsgId) {

        final String[] PROJECTION = new String[] { NmsSMSMMS._ID, NmsSMSMMS.THREAD_ID };

        final String SELECTION = NmsSMSMMS._ID + " = " + sysMsgId;
        long threadId = 0;
        Cursor cursor = null;
        try {
            cursor = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI, PROJECTION,
                    SELECTION, null, null);
            if ((cursor != null) && (cursor.moveToFirst())) {
                threadId = cursor.getLong(cursor.getColumnIndex(NmsSMSMMS.THREAD_ID));
            } else {
                NmsLog.trace(TAG, "fail to get thread id via id:" + sysMsgId);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return threadId;

    }

    /**
     * Gets the mms content via cursor
     * 
     * @param csrIn
     *            the cursor
     * @return the mms content, except part.
     */
    public SNmsMsgCont getMmsMsgContViaCsr(Cursor csrIn) {
        if (null == csrIn) {
            NmsLog.error(TAG, "fail to get  MMS MsgCont via cursor, cursor is null.");
            return null;
        }
        SNmsMsgCont msgCont = new SNmsMsgCont();
        msgCont.msgId = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS._ID));
        int type = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.MSG_BOX));
        int nThreadId = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.THREAD_ID));
        msgCont.pBody = csrIn.getString(csrIn.getColumnIndex(NmsSMSMMS.SUB));
        msgCont.deleteTime = (int) csrIn.getLong(csrIn.getColumnIndex(NmsSMSMMS.DATE));
        msgCont.isLocked = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.LOCKED));
        if (!TextUtils.isEmpty(msgCont.pBody)) {
            try {
                msgCont.pBody = (new String(msgCont.pBody.getBytes("iso-8859-1"), "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                NmsLog.nmsPrintStackTrace(e);
            }
        }
        msgCont.deleteTime = csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.DATE));
        msgCont.readed = (byte) csrIn.getInt(csrIn.getColumnIndex(NmsSMSMMS.READ));

        getMmsAddr(msgCont, nThreadId);
        getMmsAttach(msgCont);
        msgCont.msgType = getNmsMsgSourceViaSmsType(type);

        int simIdIndex = csrIn.getColumnIndex(NmsSMSMMS.SIM_ID);
        if (simIdIndex < 0 || (msgCont.simId = csrIn.getInt(simIdIndex)) <= 0) {
            msgCont.simId = NmsPlatformAdapter.getInstance(getCT()).getCurrentSimId();
            NmsLog.trace(TAG, "no sim_id column in mms, so get current sim id: " + msgCont.simId);
        }

        return msgCont;
    }

    /**
     * Gets the mms content via mms id.
     * 
     * @param id
     *            the mms id
     * @return the mms content
     */
    public SNmsMsgCont getMmsMsgContViaId(int id) {
        final String[] PROJECTION = new String[] { NmsSMSMMS._ID, NmsSMSMMS.MSG_BOX, NmsSMSMMS.SUB,
                NmsSMSMMS.DATE, NmsSMSMMS.READ, NmsSMSMMS.THREAD_ID, NmsSMSMMS.LOCKED };
        final String[] PROJECTION_EXT = new String[] { NmsSMSMMS._ID, NmsSMSMMS.MSG_BOX,
                NmsSMSMMS.SUB, NmsSMSMMS.DATE, NmsSMSMMS.READ, NmsSMSMMS.THREAD_ID,
                NmsSMSMMS.LOCKED, NmsSMSMMS.SIM_ID };

        final String SELECTION = NmsSMSMMS._ID + " = " + id;
        Cursor cursor = null;
        SNmsMsgCont msgCont = null;
        try {
            cursor = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI,

            isExtentionFieldExsit() == 1 ? PROJECTION_EXT : PROJECTION, SELECTION, null, null);

            if ((cursor != null) && (cursor.moveToFirst())) {
                msgCont = getMmsMsgContViaCsr(cursor);
            } else {
                NmsLog.trace(TAG, "fail to get mms via id:" + id);
            }
        } catch (Exception e) {
            NmsLog.nmsAssertException(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return msgCont;
    }

    public SNmsMsgCont getSmsMsgContViaId(int id) {
        final String[] PROJECTION = new String[] { NmsSMSMMS._ID, NmsSMSMMS.TYPE,
                NmsSMSMMS.ADDRESS, NmsSMSMMS.BODY, NmsSMSMMS.STATUS, NmsSMSMMS.DATE,
                NmsSMSMMS.READ, NmsSMSMMS.THREAD_ID, NmsSMSMMS.LOCKED };

        final String[] PROJECTION_EXT = new String[] { NmsSMSMMS._ID, NmsSMSMMS.TYPE,
                NmsSMSMMS.ADDRESS, NmsSMSMMS.BODY, NmsSMSMMS.STATUS, NmsSMSMMS.DATE,
                NmsSMSMMS.READ, NmsSMSMMS.THREAD_ID, NmsSMSMMS.LOCKED, NmsSMSMMS.SIM_ID };

        final String SELECTION = NmsSMSMMS._ID + " = " + id;
        SNmsMsgCont msgCont = null;
        Cursor cursor = null;
        try {
            cursor = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                    isExtentionFieldExsit() == 1 ? PROJECTION_EXT : PROJECTION, SELECTION, null,
                    null);
            if ((cursor != null) && (cursor.moveToFirst())) {
                msgCont = getSmsMsgContViaCsr(cursor);

            } else {
                NmsLog.trace(TAG, "fail to get sms via id:" + id);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }

        return msgCont;
    }

    /**
     * Gets the mms attach.
     * 
     * @param msgCont
     *            the msg cont
     * @return the mms attach
     */
    public void getMmsAttach(SNmsMsgCont msgCont) {

        String[] PROJECTION = new String[] { "ct", "name", "text" };
        String selectionPart = new String("mid=" + msgCont.msgId);

        Cursor cPart = null;
        try {
            cPart = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.MMS_CONTENT_URI_PART, PROJECTION,
                    selectionPart, null, null);
            while (cPart.moveToNext()) {
                String attachType = cPart.getString(cPart.getColumnIndex("ct"));
                String attachName = cPart.getString(cPart.getColumnIndex("name"));
                if (TextUtils.isEmpty(msgCont.pBody) && !attachType.equals("application/smil")) {
                    msgCont.pBody = cPart.getString(cPart.getColumnIndex("text"));
                }

                if (!TextUtils.isEmpty(attachType) && !attachType.equals("application/smil")
                        && !TextUtils.isEmpty(attachName) && msgCont.numOfAttach < 5) {
                    msgCont.pAttachName[msgCont.numOfAttach] = attachName;
                    msgCont.numOfAttach++;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cPart != null) {
                cPart.close();
            }
        }
    }

    public void getMmsAddr(SNmsMsgCont msgCont, int nThreadIdIn) {
        String selectionAdd = new String("msg_id=" + msgCont.msgId + "");
        Uri uriAddr = Uri.parse("content://mms/" + msgCont.msgId + "/addr");
        Cursor cAdd = null;
        try {
            cAdd = NmsMtkBinderApi.getInstance().query(uriAddr, null, selectionAdd, null, null);
            if (null == cAdd) {
                NmsLog.error(TAG, "get Mms Addr cursor is null:" + nThreadIdIn + ", msgID"
                        + msgCont.msgId);
                return;
            }
            if (cAdd.moveToFirst()) { // first index is from addr;
                msgCont.pFrom = NmsCommonUtils.nmsGetStandardPhoneNum(cAdd.getString(cAdd
                        .getColumnIndex(NmsSMSMMS.ADDRESS)));
            }
            if (cAdd.moveToNext()) {// second index is to addr;
                msgCont.pTo = NmsCommonUtils.nmsGetStandardPhoneNum(cAdd.getString(cAdd
                        .getColumnIndex(NmsSMSMMS.ADDRESS)));
            }
            msgCont.pThreadNumber = getMsgAddressViaThreadId(nThreadIdIn);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (cAdd != null) {
                cAdd.close();
            }
        }
    }

    public long saveSmsToOutBox(String msg, String number, long threadId, long simId) {
        ContentValues values = new ContentValues();
        values.put(NmsSMSMMS.THREAD_ID, threadId);
        values.put(NmsSMSMMS.ADDRESS, number);
        values.put(NmsSMSMMS.TYPE, NmsSMSMMS.SMS_TYPE_OUTBOX);
        values.put(NmsSMSMMS.BODY, msg);
        values.put(NmsSMSMMS.DATE, System.currentTimeMillis());
        if (1 == isExtentionFieldExsit()) {
            values.put(NmsSMSMMS.SIM_ID, simId);
        }
        
        Uri uRet = NmsMtkBinderApi.getInstance().insert(NmsSMSMMS.SMS_CONTENT_URI, values);
        return ContentUris.parseId(uRet);
    }

    public long saveSmsToDb(String msg, String fromOrTo, String threadNumber, int status, int flag,
            int date, int dateSent, int nmsRecId, long simId) {
        int type = NmsSMSMMS.SMS_TYPE_ALL;
        int read = 1;
        // int isLocked = SNmsMsgCont.NMS_MSG_UNLOCKED;
        long lThreadId = -1;
        long lDate = (long) date * 1000;
        long lDateSent = (long) dateSent * 1000;
        Set<String> setThreadAddr = new HashSet<String>();

        NmsLog.trace(TAG, "start to insert sms msg:" + msg + ", time:" + System.currentTimeMillis()
                + ", iSMS DB recdId: " + nmsRecId + ", fromOrTo: " + fromOrTo + ", threadNumber: "
                + threadNumber + ", simId: " + simId);

        if (!NmsSendMessage.getInstance().isAddressLegal(threadNumber, setThreadAddr)) {
            return -1;
        }

        lThreadId = NmsCreateSmsThread.getOrCreateThreadId(getCT(), setThreadAddr);

        type = getSmsTypeViaNmsMsgStatus(status);
        if (type == NmsSMSMMS.SMS_TYPE_DRAFT) {
            return saveSmsToDraft(msg, lThreadId, nmsRecId, simId);
        }

        ContentValues values = new ContentValues();
        values.put(NmsSMSMMS.THREAD_ID, lThreadId);

        if (type != NmsSMSMMS.SMS_TYPE_DRAFT) {
            values.put(NmsSMSMMS.ADDRESS, fromOrTo);
        }

        if (NmsSMSMMS.SMS_TYPE_FAILED == type) {
            values.put(NmsSMSMMS.TYPE, NmsSMSMMS.SMS_TYPE_ALL);
        } else {
            values.put(NmsSMSMMS.TYPE, type);
        }
        values.put(NmsSMSMMS.BODY, msg);
        values.put(NmsSMSMMS.DATE, lDate);
        values.put(NmsSMSMMS.DATE_SENT, lDateSent);
        if ((flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_READ) == NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_READ) {
            values.put(NmsSMSMMS.READ, 1);
        }

        values.put(
                NmsSMSMMS.LOCKED,
                ((flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) == NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) ? 1
                        : 0);

        if (isExtentionFieldExsit() == 1) {
            values.put(NmsSMSMMS.IP_RECORDID_EXTENTION, nmsRecId);
            values.put(NmsSMSMMS.SIM_ID, simId);
        }
        Uri uRet = null;
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsFeatureSupport.NMS_MSG_FLAG_PRIVATE_MESSAGE) != 0) {
            boolean isFirstSaveToDb = isFirstSave(lThreadId);
            if (!isChatMessage(threadNumber) && !isMassMessage(threadNumber)
                    && isPrivateContact(fromOrTo)) {
                values.put(NmsSMSMMS.BODY, encodePrivateMessageContent(msg));
                uRet = NmsMtkBinderApi.getInstance().insert(NmsSMSMMS.SMS_CONTENT_URI, values);
                if (isFirstSaveToDb) {
                    ContentValues priValues = new ContentValues();
                    priValues.put(NmsSMSMMS.IP_SPAM_EXTENTION, 2);
                    Uri uri = ContentUris.withAppendedId(NmsSMSMMS.THREAD_SETTINGS, lThreadId);
                    NmsMtkBinderApi.getInstance().update(uri, priValues, null, null);
                }
            } else {
                uRet = NmsMtkBinderApi.getInstance().insert(NmsSMSMMS.SMS_CONTENT_URI, values);
            }
        } else {
            uRet = NmsMtkBinderApi.getInstance().insert(NmsSMSMMS.SMS_CONTENT_URI, values);
        }
        if (uRet != null) {
            if (NmsSMSMMS.SMS_TYPE_FAILED == type) {
                ContentValues cv = new ContentValues();
                cv.put(NmsSMSMMS.TYPE, type);
                NmsMtkBinderApi.getInstance().update(uRet, cv, null, null);
            }
            return ContentUris.parseId(uRet);
        } else {
            return -1;
        }
    }

    public int isExtentionFieldExsit() {
        if (mExtentionExsit >= 0) {
            return mExtentionExsit;
        }
        mExtentionExsit = 0;
        try {
            Cursor csrTemp = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                    new String[] { NmsSMSMMS.IP_RECORDID_EXTENTION, NmsSMSMMS.SIM_ID }, null, null,
                    NmsSMSMMS._ID + " limit 1");

            if (csrTemp != null) {
                mExtentionExsit = 1;
                csrTemp.close();
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "this device not mtk dual card platform");
            // NmsLog.nmsPrintStackTrace(e);
        }
        return mExtentionExsit;
    }

    private boolean isSeenExist() {
        if (SEEN_TYPE_UNCHECK == mSeenType) {
            Cursor csrTemp = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI, null,
                    null, null, NmsSMSMMS._ID + " limit 1");
            if (null != csrTemp && csrTemp.moveToFirst()) {
                if (-1 != csrTemp.getColumnIndex(NmsSMSMMS.SEEN)) {
                    mSeenType = SEEN_TYPE_EXIST;
                } else {
                    mSeenType = SEEN_TYPE_UNEXIST;
                }
            } else {
                mSeenType = SEEN_TYPE_UNEXIST;
            }
            if (null != csrTemp) {
                csrTemp.close();
            }
        }
        switch (mSeenType) {
        case SEEN_TYPE_EXIST:
            return true;

        case SEEN_TYPE_UNEXIST:
        default:
            return false;
        }
    }

    private long saveSmsToDraft(String msg, long threadId, int nmsRecId, long simId) {
        ContentValues values = new ContentValues();
        values.put(NmsSMSMMS.THREAD_ID, threadId);
        values.put(NmsSMSMMS.TYPE, NmsSMSMMS.SMS_TYPE_DRAFT);
        values.put(NmsSMSMMS.BODY, msg);
        values.put(NmsSMSMMS.DATE, System.currentTimeMillis());
        long msgId = -1;
        if (isSeenExist()) {
            values.put(NmsSMSMMS.SEEN, 1);
        }

        if (isExtentionFieldExsit() == 1) {
            values.put(NmsSMSMMS.IP_RECORDID_EXTENTION, nmsRecId);
            values.put(NmsSMSMMS.SIM_ID, simId);
        }

        if (NmsMtkBinderApi.getInstance().update(NmsSMSMMS.SMS_CONTENT_DRAFT_URI, values,
                "thread_id = ? and type = ?", new String[] { "" + threadId,
                        "" + NmsSMSMMS.SMS_TYPE_DRAFT }) <= 0) {
            Uri uri = NmsMtkBinderApi.getInstance().insert(NmsSMSMMS.SMS_CONTENT_DRAFT_URI, values);
            if (uri != null) {
                return ContentUris.parseId(uri);
            } else {
                return -1;
            }
        } else {
            final String[] PROJECTION = new String[] { NmsSMSMMS._ID, };

            final String SELECTION = String.format(NmsSMSMMS.TYPE + " = %d",
                    NmsSMSMMS.SMS_TYPE_DRAFT);
            Cursor csrDraftMsg = null;
            try {
                csrDraftMsg = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                        PROJECTION, SELECTION, null, null);

                if (null == csrDraftMsg) {
                    NmsLog.trace(TAG, "fail to insert draft msg, cursor is null.");
                    return -1;
                }

                if (!csrDraftMsg.moveToFirst()) {
                    NmsLog.trace(TAG, "fail to insert draft msg .");
                    csrDraftMsg.close();
                    return -1;
                }

                int index = csrDraftMsg.getColumnIndex(NmsSMSMMS._ID);
                if (index < 0) {
                    return -1;
                }
                msgId = csrDraftMsg.getLong(index);
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
            } finally {
                if (null != csrDraftMsg) {
                    csrDraftMsg.close();
                }
            }
            return msgId;
        }
    }

    public void updateSmsStatusViaId(int msgTypeIn, int msgIdIn, int statusIn, int deliverIn) {
        ContentValues values = new ContentValues();
        switch (msgTypeIn) {
        case NmsSMSMMS.PROTOCOL_MMS:
            values.put(NmsSMSMMS.MSG_BOX, statusIn);
            break;

        case NmsSMSMMS.PROTOCOL_SMS:
            values.put(NmsSMSMMS.TYPE, statusIn);
            values.put(NmsSMSMMS.STATUS, deliverIn);
            break;

        default:
            NmsLog.trace(TAG, "unkonw msgType:" + msgTypeIn + " in update sms status.");
            return;
        }
        NmsMtkBinderApi.getInstance().update(NmsSMSMMS.SMS_CONTENT_URI, values, NmsSMSMMS._ID
                + " = ?", new String[] { "" + msgIdIn });
        return;
    }

    private void updateFlag(Uri uri, final int[] msgSmsId, ContentValues flag) {

        int count = msgSmsId.length / MAX_COUNT;
        count += (msgSmsId.length % MAX_COUNT > 0 ? 1 : 0);

        for (int i = 0; i < count; ++i) {
            String where = "_id = ?";
            String selectionArgs[] = null;
            ArrayList<String> idList = new ArrayList<String>();
            idList.add("" + msgSmsId[i * MAX_COUNT]);
            for (int j = 1; j < MAX_COUNT && i * MAX_COUNT + j < msgSmsId.length; j++) {
                where += " OR _id = ?";
                idList.add("" + msgSmsId[i * MAX_COUNT + j]);
            }

            selectionArgs = new String[idList.size()];
            idList.toArray(selectionArgs);

            NmsMtkBinderApi.getInstance().update(uri, flag, where, selectionArgs);
        }
    }

    public void nmsUpdateIpMsgFlagInSysDb(final int[] mmsIDs, final int[] smsIDs, int operation) {

        if (operation >= UpdateFlagAction.NMS_UPDATE_SET_BLOCKED && isExtentionFieldExsit() != 1) {
            NmsLog.error(TAG, "this device not supprot extension.");
            return;
        }

        ContentValues values = new ContentValues();
        switch (operation) {
        case UpdateFlagAction.NMS_UPDATE_SET_READED:
            values.put(NmsSMSMMS.READ, 1);
            break;

        case UpdateFlagAction.NMS_UPDATE_SET_SAVED:
            values.put(NmsSMSMMS.LOCKED, 1);
            break;

        case UpdateFlagAction.NMS_UPDATE_SET_UN_SAVED:
            values.put(NmsSMSMMS.LOCKED, 0);
            break;
        }

        if (mmsIDs != null && UpdateFlagAction.NMS_UPDATE_SET_READED == operation) {
            updateFlag(NmsSMSMMS.MMS_CONTENT_URI, mmsIDs, values);
        }
        if (smsIDs != null) {
            updateFlag(NmsSMSMMS.SMS_CONTENT_URI, smsIDs, values);
        }

    }

    public void nmsUpdateIpMsgInSysDb(int msgSmsId, int status, int flag) {

        int type = -1;

        switch (status) {
        case NmsIpMessageConsts.NmsIpMessageStatus.FAILED:
            type = NmsSMSMMS.SMS_TYPE_FAILED;
            break;

        case NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX_PENDING:
        case NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX:
            type = NmsSMSMMS.SMS_TYPE_OUTBOX;
            break;

        case NmsIpMessageConsts.NmsIpMessageStatus.SENT:
            /*
             * status of server drive to send sms, and waiting client confirm to
             * send sms
             */
        case NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED:
        case NmsIpMessageConsts.NmsIpMessageStatus.DELIVERED:
        case NmsIpMessageConsts.NmsIpMessageStatus.VIEWED:
            type = NmsSMSMMS.SMS_TYPE_SENT;
            break;

        case NmsIpMessageConsts.NmsIpMessageStatus.DRAFT:
            type = NmsSMSMMS.SMS_TYPE_DRAFT;
            break;

        case NmsIpMessageConsts.NmsIpMessageStatus.INBOX:
            type = NmsSMSMMS.SMS_TYPE_INBOX;
            break;

        default:
            NmsLog.error(TAG, "not handle msg status: " + status + ", id: " + msgSmsId);
            return;
        }

        ContentValues values = new ContentValues();

        values.put(NmsSMSMMS.TYPE, type);
        values.put(NmsSMSMMS.STATUS, NmsSMSMMS.SMS_DELIVER_UNKNOWN);
        if ((flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_READ) == NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_READ) {
            values.put(NmsSMSMMS.READ, 1);
        }
        values.put(
                NmsSMSMMS.LOCKED,
                ((flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) == NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) ? 1
                        : 0);

        Uri uri = ContentUris.withAppendedId(NmsSMSMMS.SMS_CONTENT_URI, msgSmsId);

        NmsMtkBinderApi.getInstance().update(uri, values, null, null);
    }

    public void nmsUpdateIpMsgRecdIdInSysDb(int msgId, int recdId) {
        if (isExtentionFieldExsit() != 1) {
            return;
        }
        ContentValues values = new ContentValues();
        values.put(NmsSMSMMS.IP_RECORDID_EXTENTION, recdId);

        NmsMtkBinderApi.getInstance().update(NmsSMSMMS.SMS_CONTENT_URI, values, NmsSMSMMS._ID
                + " = ?", new String[] { "" + msgId });
        return;
    }

    public int nmsUpdateSpamFlag(String phoneNumber, int flag) {
        if (isExtentionFieldExsit() != 1) {
            return 0;
        }

        if (TextUtils.isEmpty(phoneNumber)) {
            NmsLog.error(TAG, "update spam param error:" + phoneNumber);
            return 0;
        }

        long threadId = NmsCreateSmsThread.getOrCreateThreadId(getCT(), phoneNumber);
        if (threadId <= 0) {
            NmsLog.error(TAG, "update spam flag error, can't get thread id with phonenumber: "
                    + phoneNumber);
            return 0;
        }

        ContentValues values = new ContentValues();
        values.put(NmsSMSMMS.IP_SPAM_EXTENTION, flag);
        values.put(NmsSMSMMS.IP_SLIENCE_FLAG, flag > 0 ? 0 : 1);
        values.put("mute", flag > 0 ? 1 : 0);

        Uri uri = ContentUris.withAppendedId(NmsSMSMMS.THREAD_SETTINGS, threadId);

        return NmsMtkBinderApi.getInstance().update(uri, values, null, null);
    }

    public int updateLockedFlag(int msgTypeIn, long msgIdIn, int locked) {
        ContentValues values = new ContentValues();
        values.put("locked", locked);
        Uri uri = SNmsMsgKey.NMS_IS_MMS_MSG(msgTypeIn) ? NmsSMSMMS.MMS_CONTENT_URI
                : NmsSMSMMS.SMS_CONTENT_URI;
        NmsMtkBinderApi.getInstance().update(uri, values, NmsSMSMMS._ID + " = ?", new String[] { ""
                + msgIdIn });

        return 0;
    }

    public int deleteSMS(int smsType, int[] msgSmsId, boolean lockFlag) {
        if (null == msgSmsId || msgSmsId.length <= 0) {
            NmsLog.error(TAG, "deleteSMS param error, msgSmsId is null");
            return 0;
        }

        int count = msgSmsId.length / MAX_COUNT;
        count += (msgSmsId.length % MAX_COUNT > 0 ? 1 : 0);

        for (int i = 0; i < count; ++i) {
            String where = "_id = ?";
            String whereMid = "mid = ?";
            String selectionArgs[] = null;
            ArrayList<String> idList = new ArrayList<String>();
            idList.add("" + msgSmsId[i * MAX_COUNT]);
            for (int j = 1; j < MAX_COUNT && i * MAX_COUNT + j < msgSmsId.length; j++) {
                where += " OR _id = ?";
                idList.add("" + msgSmsId[i * MAX_COUNT + j]);
                whereMid += " OR mid = ?";
            }

            selectionArgs = new String[idList.size()];
            idList.toArray(selectionArgs);

            if (NmsSMSMMS.PROTOCOL_SMS == smsType) {
                if (lockFlag) {
                    NmsMtkBinderApi.getInstance().delete(NmsSMSMMS.SMS_CONTENT_URI, where,
                            selectionArgs);
                } else {
                    NmsMtkBinderApi.getInstance().delete(NmsSMSMMS.SMS_CONTENT_URI, "( " + where
                            + " ) " + " and locked = 0", selectionArgs);
                }
            } else {
                if (lockFlag) {
                    NmsMtkBinderApi.getInstance().delete(NmsSMSMMS.MMS_CONTENT_URI, where,
                            selectionArgs);
                } else {
                    NmsMtkBinderApi.getInstance().delete(NmsSMSMMS.MMS_CONTENT_URI, "( " + where
                            + " ) " + " and locked = 0", selectionArgs);
                }
                NmsMtkBinderApi.getInstance().delete(NmsSMSMMS.MMS_CONTENT_URI_PART, whereMid,
                        selectionArgs);
            }

            NmsLog.trace(TAG, "delete sms or mms by hesine and the type:" + smsType
                    + " the msgId length:" + msgSmsId.length);
        }
        return msgSmsId.length;
    }

    private boolean isChatMessage(String threadNumber) {
        return threadNumber.startsWith("7---");
    }

    private boolean isMassMessage(String threadNumber) {
        String[] numbers = threadNumber.split(",");
        if (numbers.length > 1) {
            return true;
        } else {
            return false;
        }
    }

    private boolean isFirstSave(long thread) {
        Cursor addrCur = NmsMtkBinderApi.getInstance().query(NmsSMSMMS.SMS_CONTENT_URI,
                new String[] { NmsSMSMMS.ADDRESS }, "thread_id=?",
                new String[] { String.valueOf(thread) }, null);
        if (addrCur != null && addrCur.moveToFirst()) {
            addrCur.close();
            return false;
        }
        return true;
    }

    private boolean isPrivateContact(String number) {
        Uri privateUri = Uri.parse("content://com.android.mms.data.provider/privatecontact");
        Cursor cur = NmsMtkBinderApi.getInstance().query(privateUri, null, "number=? or unify_number=?",
                new String[] { number, number }, null);
        if (cur != null && cur.moveToNext()) {
            cur.close();
            return true;
        }
        return false;
    }

    private String encodePrivateMessageContent(String body) {
        if (body != null && body.startsWith(PRIVATE_BODY_PREFIX)) {
            Pattern pattern = Pattern
                    .compile("^([A-Za-z0-9+/]{4})*([A-Za-z0-9+/]{4}|[A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{2}==)$");
            Matcher matcher = pattern.matcher(body.substring(4));
            if (matcher.matches()) {
                return body;
            }
        }
        String temp = new String(Base64.encode(body.getBytes(), Base64.NO_WRAP));
        return (PRIVATE_BODY_PREFIX + temp);
    }
    
    public static  boolean isDefaultSmsApp() {
       
        if (!NmsConfig.isAndroidKitKatOnward)
            return true ;
        
        try {
            Class clazz = HpnsApplication.mGlobalContext.getClassLoader().loadClass("android.provider.Telephony$Sms");
            String packageName = (String)clazz.getMethod("getDefaultSmsPackage",Context.class).invoke(null, HpnsApplication.mGlobalContext);  
            if(packageName.equalsIgnoreCase("com.android.mms")) {
                return true; 
            } 
            
            return false; 
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
            return false ;
        }
    }
}
