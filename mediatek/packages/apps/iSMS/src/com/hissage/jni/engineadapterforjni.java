package com.hissage.jni;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Locale;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.os.Environment;
import android.os.StatFs;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.db.NmsContentResolver;
import com.hissage.db.NmsDBUtils;
import com.hissage.location.NmsLocationFormat;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMS;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.message.smsmms.NmsSendMessage;
import com.hissage.message.smsmms.SNmsMsgCont;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.hpnsReceiver;
import com.hissage.protocol.NmsExtProtocol;
import com.hissage.receiver.system.NmsSMSReceiver;
import com.hissage.service.NmsService;
import com.hissage.timer.NmsTimer;
import com.hissage.timer.NmsWakeLock;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
import com.hissage.util.queue.NmsMessage;
import com.hissage.util.statistics.NmsNetTraffic;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;
import com.hissage.vcard.NmsVcardUtils;

public final class engineadapterforjni {
    private static final String TAG = "engineadapterforjni";
    private static NmsQueue UIMsgQ = null;

    private static ContentResolver crMsg = null;
    
    private static final long AVALIABLE_SPACE = 5 * 1024 * 1024 ;

    public enum boxtype {
        NMS_CATEGORY_NAME_OUTBOX, NMS_CATEGORY_NAME_INBOX, NMS_CATEGORY_NAME_DRAFTBOX, NMS_CATEGORY_NAME_SENDBOX, NMS_CATEGORY_NAME_ALL_MSG, NMS_CATEGORY_NAME_FAVOURITE, NMS_CATEGORY_NAME_SAVED, NMS_CATEGORY_NAME_UNREAD, NMS_CATEGORY_NAME_HESINE, NMS_CATEGORY_NAME_SMS, NMS_CATEGORY_NAME_PHONE_FRIENDS, NMS_CATEGORY_NAME_MAX
    };

    private engineadapterforjni() {
    }

    private static ContentResolver getCRMsg() {
        if (null == crMsg) {
            crMsg = NmsService.getInstance().getContentResolver();
        }
        return crMsg;
    }

    public static int nmsGetNetType() {
        return NmsNetTraffic.getInstance().getNetType();
    }

    public static int nmsGetNetConnSimId() {
        return NmsNetTraffic.getInstance().getNetConnSimId();
    }

    public static void nmsSendMsgToUI(int mid, byte[] pMsg, int msgLen) {
        if (null == UIMsgQ) {
            UIMsgQ = new NmsQueue();
            UIMsgQ.create();
        }
        NmsLog.trace(TAG, "UI recv Engine msg: " + mid);
        NmsQueue.postMessage(new NmsMessage(mid, pMsg, msgLen));

    }

    public static void nmsSendMsgToUI(int mid, Object obj) {
        if (null == UIMsgQ) {
            UIMsgQ = new NmsQueue();
            UIMsgQ.create();
        }
        NmsMessage msg = new NmsMessage(mid, obj);
        NmsQueue.postMessage(msg);
    }

    public static String getUserLogPath() {
        return NmsCommonUtils.getSDCardPath(NmsService.getInstance()) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY;
    }

    public static String getUserDataPath() {
        return NmsCommonUtils.getMemPath(NmsService.getInstance()) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY;
    }

    public static String nmsGetCountryCodeViaImsi(String imsi) {
        String[] imsiPreArray = NmsService.getInstance().getResources()
                .getStringArray(R.array.imsi_pre_list);
        String[] phoneArray = NmsService.getInstance().getResources()
                .getStringArray(R.array.phone_number_pre_list);
        for (int i = 0; i < imsiPreArray.length; ++i) {
            if (imsi.startsWith(imsiPreArray[i])) {
                return phoneArray[i];
            }
        }
        return "86";
    }

    public static String getClientVersion() {
        String version = NmsService.getInstance().getString(R.string.STR_NMS_VERSION_ID);
        version = version.replaceAll(": v", "");
        return version;
    }

    public static String nmsGetImsiPre() {
        String OpID = "460";
        try {
            String line;
            Process process = Runtime.getRuntime().exec("getprop");
            BufferedReader in = new BufferedReader(new InputStreamReader(process.getInputStream()));

            while ((line = in.readLine()) != null) {
                if (line.indexOf("gsm.operator.numeric") > 0) {
                    OpID = line;
                    NmsLog.error(TAG, "gsm.operator.numeric" + line);
                }
            }
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        int i = OpID.lastIndexOf("[");
        if (i > 0) {
            OpID = OpID.substring(i + 1);
            if (OpID.length() < 5) {
                OpID = "460";
            } else {
                if (Character.isDigit(OpID.charAt(0)) && Character.isDigit(OpID.charAt(1))
                        && Character.isDigit(OpID.charAt(2))) {
                    OpID = OpID.substring(0, 3);
                } else {
                    OpID = "460";
                }
            }
        }
        return OpID;
    }

    public static String nmsGetCategoryName(byte type) {
        switch ((int) type) {
        case 0:// NMS_CATEGORY_NAME_ALL_MSG,
            return new String(NmsService.getInstance().getString(R.string.STR_NMS_ALL_MSG));

        case 1: // NMS_CATEGORY_NAME_SAVED,
            return new String(NmsService.getInstance().getString(R.string.STR_NMS_SAVED));

        case 2: // NMS_CATEGORY_NAME_UNREAD,
            return new String(NmsService.getInstance().getString(R.string.STR_NMS_BLOCKMSG));

        default: // NMS_CATEGORY_NAME_MAX
            return new String(NmsService.getInstance().getString(R.string.STR_NMS_SMS_TYPE));
        }
    }

    public static String getIMSI() {

        String imsi = NmsConfig.getSim1IMSI(NmsService.getInstance());
        if (TextUtils.isEmpty(imsi)) {
            Log.e(TAG, "getIMSI from device, but IMSI is null");
            return "";
        }

        Log.e(TAG, "getIMSI from device, IMSI:" + imsi);
        return new String(imsi);
    }

    public static String[] getAllImsi() {
        return NmsPlatformAdapter.getInstance(NmsService.getInstance()).getAllImsi();
    }

    public static String getIMEI() {
        TelephonyManager telManager = (TelephonyManager) NmsService.getInstance().getSystemService(
                Context.TELEPHONY_SERVICE);
        String imei = "no IMEI DROID";
        if (null != telManager && !TextUtils.isEmpty(telManager.getDeviceId())) {
            imei = telManager.getDeviceId();
        }

        return new String(imei);

    }

    public static int getMobileLanguage() {
        String languageCode = Locale.getDefault().getLanguage();
        int language = 0;
        if (languageCode.equals("en")) {
            String countryCode = Locale.getDefault().getCountry();
            if (countryCode.equals("UK")) {
                language = NmsConfig.NMS_LANG_UK_ENGLISH;
            } else {
                language = NmsConfig.NMS_LANG_ENGLISH;
            }
        } else if (languageCode.equals("zh")) {
            String countryCode = Locale.getDefault().getCountry();
            if (countryCode.equals("TW")) {
                language = NmsConfig.NMS_LANG_TR_CHINESE;
            } else if (countryCode.equals("HK")) {
                language = NmsConfig.NMS_LANG_HK_CHINESE;
            } else {
                language = NmsConfig.NMS_LANG_SM_CHINESE;
            }
        } else if (languageCode.equals("th"))
            language = NmsConfig.NMS_LANG_THAI;
        else if (languageCode.equals("es")) {
            String countryCode = Locale.getDefault().getCountry();
            if (countryCode.equals("SA")) {
                language = NmsConfig.NMS_LANG_SA_SPANISH;
            } else {
                language = NmsConfig.NMS_LANG_SPANISH;
            }
        } else if (languageCode.equals("fr")) {
            String countryCode = Locale.getDefault().getCountry();
            if (countryCode.equals("CA")) {
                language = NmsConfig.NMS_LANG_CA_FRENCH;
            } else {
                language = NmsConfig.NMS_LANG_FRENCH;
            }
        } else if (languageCode.equals("de"))
            language = NmsConfig.NMS_LANG_GERMAN;
        else if (languageCode.equals("ru"))
            language = NmsConfig.NMS_LANG_RUSSIAN;
        else if (languageCode.equals("it"))
            language = NmsConfig.NMS_LANG_ITALIAN;
        else if (languageCode.equals("ar"))
            language = NmsConfig.NMS_LANG_ARABIC;
        else if (languageCode.equals("pt")) {
            String countryCode = Locale.getDefault().getCountry();
            if (countryCode.equals("BZ")) {
                language = NmsConfig.NMS_LANG_SA_PORTUGUESE;
            } else {
                language = NmsConfig.NMS_LANG_PORTUGUESE;
            }
        } else if (languageCode.equals("tr"))
            language = NmsConfig.NMS_LANG_TURKISH;
        else if (languageCode.equals("vi"))
            language = NmsConfig.NMS_LANG_VIETNAMESE;
        else if (languageCode.equals("id"))
            language = NmsConfig.NMS_LANG_INDONESIAN;
        else if (languageCode.equals("ms"))
            language = NmsConfig.NMS_LANG_MALAY;
        else if (languageCode.equals("hi"))
            language = NmsConfig.NMS_LANG_HINDI;
        else if (languageCode.equals("da"))
            language = NmsConfig.NMS_LANG_DANISH;
        else if (languageCode.equals("cs"))
            language = NmsConfig.NMS_LANG_CZECH;
        else if (languageCode.equals("pl"))
            language = NmsConfig.NMS_LANG_POLISH;
        else if (languageCode.equals("hu"))
            language = NmsConfig.NMS_LANG_HUNGARIAN;
        else if (languageCode.equals("fi"))
            language = NmsConfig.NMS_LANG_FINNISH;
        else if (languageCode.equals("nb"))
            language = NmsConfig.NMS_LANG_NORWEGIAN;
        else if (languageCode.equals("sk"))
            language = NmsConfig.NMS_LANG_SLOVAK;
        else if (languageCode.equals("nl"))
            language = NmsConfig.NMS_LANG_DUTCH;
        else if (languageCode.equals("sv"))
            language = NmsConfig.NMS_LANG_SWEDISH;
        else if (languageCode.equals("hr"))
            language = NmsConfig.NMS_LANG_CROATIAN;
        else if (languageCode.equals("ro"))
            language = NmsConfig.NMS_LANG_ROMANIAN;
        else if (languageCode.equals("sl"))
            language = NmsConfig.NMS_LANG_SLOVENIAN;
        else if (languageCode.equals("el"))
            language = NmsConfig.NMS_LANG_GREEK;
        else if (languageCode.equals("he"))
            language = NmsConfig.NMS_LANG_HEBREW;
        else if (languageCode.equals("bg"))
            language = NmsConfig.NMS_LANG_BULGARIAN;
        else if (languageCode.equals("mr"))
            language = NmsConfig.NMS_LANG_MARATHI;
        else if (languageCode.equals("ps"))
            language = NmsConfig.NMS_LANG_PERSIAN;
        else if (languageCode.equals("ur"))
            language = NmsConfig.NMS_LANG_URDU;
        else if (languageCode.equals("ta"))
            language = NmsConfig.NMS_LANG_TAMIL;
        else if (languageCode.equals("be"))
            language = NmsConfig.NMS_LANG_BENGALI;
        else if (languageCode.equals("pa"))
            language = NmsConfig.NMS_LANG_PUNJABI;
        else if (languageCode.equals("te"))
            language = NmsConfig.NMS_LANG_TELUGU;
        else if (languageCode.equals("uk"))
            language = NmsConfig.NMS_LANG_UKRAINIAN;
        else if (languageCode.equals("gu"))
            language = NmsConfig.NMS_LANG_GUJARATI;
        else if (languageCode.equals("kn"))
            language = NmsConfig.NMS_LANG_KANNADA;
        else if (languageCode.equals("zu"))
            language = NmsConfig.NMS_LANG_ZULU;
        else if (languageCode.equals("xh"))
            language = NmsConfig.NMS_LANG_XHOSA;
        else if (languageCode.equals("sw"))
            language = NmsConfig.NMS_LANG_SWAHILI;
        else if (languageCode.equals("af"))
            language = NmsConfig.NMS_LANG_AFRIKAANS;
        else if (languageCode.equals("lt"))
            language = NmsConfig.NMS_LANG_LITHUANIAN;
        else if (languageCode.equals("lv"))
            language = NmsConfig.NMS_LANG_LATVIAN;
        else if (languageCode.equals("et"))
            language = NmsConfig.NMS_LANG_ESTONIAN;
        else if (languageCode.equals("hy"))
            language = NmsConfig.NMS_LANG_ARMENIAN;
        else if (languageCode.equals("ka"))
            language = NmsConfig.NMS_LANG_GEORGIAN;
        else if (languageCode.equals("mo"))
            language = NmsConfig.NMS_LANG_MOLDOVAN;
        else if (languageCode.equals("ml"))
            language = NmsConfig.NMS_LANG_MALAYALAM;
        else if (languageCode.equals("or"))
            language = NmsConfig.NMS_LANG_ORIYA;
        else if (languageCode.equals("sq"))
            language = NmsConfig.NMS_LANG_ALBANIAN;
        else if (languageCode.equals("as"))
            language = NmsConfig.NMS_LANG_ASSAMESE;
        else if (languageCode.equals("az"))
            language = NmsConfig.NMS_LANG_AZERBAIJANI;
        else if (languageCode.equals("ca"))
            language = NmsConfig.NMS_LANG_CATALAN;
        else if (languageCode.equals("is"))
            language = NmsConfig.NMS_LANG_ICELANDIC;
        else if (languageCode.equals("mk"))
            language = NmsConfig.NMS_LANG_MACEDONIAN;
        else if (languageCode.equals("sr"))
            language = NmsConfig.NMS_LANG_SERBIAN;
        else if (languageCode.equals("st"))
            language = NmsConfig.NMS_LANG_SESOTHO;
        else if (languageCode.equals("tl"))
            language = NmsConfig.NMS_LANG_TAGALOG;
        else if (languageCode.equals("ha"))
            language = NmsConfig.NMS_LANG_HAUSA;
        else if (languageCode.equals("yo"))
            language = NmsConfig.NMS_LANG_YORUBA;
        else if (languageCode.equals("eu"))
            language = NmsConfig.NMS_LANG_BASQUE;
        else if (languageCode.equals("fil"))
            language = NmsConfig.NMS_LANG_FILIPINO;
        else if (languageCode.equals("gl"))
            language = NmsConfig.NMS_LANG_GALICIAN;
        else if (languageCode.equals("ig"))
            language = NmsConfig.NMS_LANG_IGBO;
        else if (languageCode.equals("ga"))
            language = NmsConfig.NMS_LANG_IRISH;
        else if (languageCode.equals("my"))
            language = NmsConfig.NMS_LANG_MYANMAR;
        else if (languageCode.equals("lo"))
            language = NmsConfig.NMS_LANG_LAO;
        else if (languageCode.equals("km"))
            language = NmsConfig.NMS_LANG_KHMER;
        else if (languageCode.equals("ko"))
            language = NmsConfig.NMS_LANG_KOREAN;
        else
            language = NmsConfig.NMS_LANG_DEFAULT;

        return language;
    }

    public static int getStorageSize() {
        String sdStatus = Environment.getExternalStorageState();

        if (TextUtils.isEmpty(sdStatus) || (!sdStatus.equals(android.os.Environment.MEDIA_MOUNTED))) {// added
                                                                                                      // by
                                                                                                      // luozheng
                                                                                                      // for
                                                                                                      // sdcard
                                                                                                      // not
                                                                                                      // exist;
            return 10 * 1024 * 1024;
        }

        String sdcard = Environment.getExternalStorageDirectory().getPath();
        File file = new File(sdcard);
        StatFs statFs = new StatFs(file.getPath());
        int blockSize = statFs.getBlockSize();
        int available = statFs.getAvailableBlocks();
        long size = (long) (blockSize * (available / 1024));
        if (size > 5 * 1024) {
            size = 10 * 1024 * 1024;
        } else {
            size = size * 1024;
        }
        return (int) size;
    }


    // add for jira-338-a
    public static int getSDCardFullState() {
        // no sd card! or System storage space less than 5M
        if (NmsCommonUtils.getSDCardFullStatus()) {
            NmsLog.trace(TAG, "getSDCardFullState :1");
            return 1;
        }
        return 0;
    }

    public static int getSDCardFullstateEx() {
        if (NmsCommonUtils.getSDCardFullStatusEx()) {
            NmsLog.trace(TAG, "getSDCardFullStateEx :1");
            return 1;
        }
        return 0;
    }
            

    // add end

    public static void sendMMS(String threadNumber, String sub, String body, String attachPath,
            String to) {

    }

    public static int sendSMS(String threadNumber, String number, String msg, int saveToSentFlag,
            int simId, int isRegSms) {
        number = number.replace(" ", ",");
        NmsLog.trace(TAG, "send SMS, threadNumber: " + threadNumber + ", number: " + number
                + ", simId: " + simId + ", body: " + msg+", isRegSms: "+isRegSms);
        if(1 == isRegSms){
            NmsSMSReceiver.getInstance().setRegPhone(number);
        }
        return NmsSendMessage.getInstance().inserSmsToVector(threadNumber, number, msg,
                saveToSentFlag, simId);
    }

    public static int resendSMS(int nSource, int nSmsIdIn) {
        return 0;
    }

    public static int setSpamFlag(String phoneNumber, int flag) {
        int ret = 0;
        try {
            NmsSMSMMSManager.getInstance(null).nmsUpdateSpamFlag(phoneNumber, flag);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return ret;
    }

    public static int setMsgLockFlag(int nSource, int nSmsIdIn, int savedFlag) {
        return NmsSMSMMSManager.getInstance(null).updateLockedFlag(nSource, (long) nSmsIdIn, savedFlag);
    }

    public static String nmsMatchNumberToPhoneBookName(String addr) {
        if (TextUtils.isEmpty(addr)) {
            NmsLog.error(TAG, "nmsMatchNumberToPhoneBookName: addr is empty");
            return "";
        }
        if (addr.contains(",")) {
            NmsLog.error(TAG, "nmsMatchNumberToPhoneBookName: addr contains ,");
            return "";
        }
        if (NmsGroupChatContact.isGroupChatContactNumber(addr)) {
            NmsLog.error(TAG, "nmsMatchNumberToPhoneBookName: addr is groupChatContact");
            return "";
        }

        String countryCode = engineadapter.get().nmsUIGetCountryCode();
        ArrayList<String> numberList = new ArrayList<String>();
        numberList.add(addr);
        numberList.add(countryCode + addr);
        numberList.add("+" + countryCode + addr);
        numberList.add("0" + addr);
        numberList.add("0" + countryCode + addr);

        String name = "";

        for (int i = 0; i < numberList.size(); ++i) {
            name = NmsContactApi.getInstance(null).getSystemNameViaNumber(numberList.get(i));
            if (!TextUtils.isEmpty(name)) {
                break;
            }
            name = "";
        }

        return name;
    }

    public static String nmsMatchNumberToPhoneBookNumber(String addr) {
        if (TextUtils.isEmpty(addr)) {
            return "";
        }
        if (addr.contains(",")) {
            return "";
        }

        String countryCode = engineadapter.get().nmsUIGetCountryCode();
        ArrayList<String> numberList = new ArrayList<String>();
        numberList.add(addr);
        numberList.add(countryCode + addr);
        numberList.add("+" + countryCode + addr);
        numberList.add("0" + addr);
        numberList.add("0" + countryCode + addr);

        String number = "";

        for (int i = 0; i < numberList.size(); ++i) {
            number = NmsContactApi.getInstance(null).getSystemNumberViaNumber(numberList.get(i));
            if (!TextUtils.isEmpty(number)) {
                break;
            }
            number = "";
        }

        return number;
    }

    public static void NmsAddVCard2Native(String file, int doWhatWhenFinish) {
        try {
            NmsVcardUtils.nmsImportContactFromFile2Native(NmsService.getInstance(),
                    doWhatWhenFinish, file);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public static void NmsExportVCard2File(final String file) {
        new Thread(new Runnable() {
            public void run() {
                NmsExportVCard2FileThread(file);
            }
        }).start();
    }

    public static void nmsJhnieSysLogFlag() {
        // new Thread(new Runnable() {
        // public void run() {
        // nmsTerminator.MLog.getLog();
        // }
        // }).start();
    }

    private static void NmsExportVCard2FileThread(String file) {
        try {
            NmsVcardUtils.nmsExportContactFromNative2File(NmsService.getInstance(), file);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        engineadapter.get().nmsSaveAllVCard2JNI();
    }

    public static String nmsGetDisplayMetrics() {
        DisplayMetrics dm = new DisplayMetrics();
        WindowManager wmg = (WindowManager) NmsService.getInstance().getSystemService(
                Context.WINDOW_SERVICE);
        wmg.getDefaultDisplay().getMetrics(dm);
        int width = dm.widthPixels;
        int height = dm.heightPixels;
        width = width <= 320 ? 321 : width;
        height = height <= 320 ? 321 : height;
        String ret = "" + height + "~" + width;
        return new String(ret);
    }

    /*
     * 设置 指定id的sms/mms 已读 smsType: 类型 NMS_TYPE_SMS / NMS_TYPE_MMS smsId: id
     */
    public static int nmsSmsSetSmsReaded(int smsType, int smsId) {
        ContentValues contentValues = new ContentValues();
        contentValues.put("read", 1);
        if (NmsSMSMMS.PROTOCOL_SMS == smsType) {
            NmsContentResolver.update(getCRMsg(), NmsSMSMMS.SMS_CONTENT_URI, contentValues,
                    "_id=?", new String[] { "" + smsId });
        } else {
            NmsContentResolver.update(getCRMsg(), NmsSMSMMS.MMS_CONTENT_URI, contentValues,
                    "_id=?", new String[] { "" + smsId });
        }
        return 1;
    }

    /*
     * 删除 指定id的sms/mms 已读 smsType: 类型 NMS_TYPE_SMS / NMS_TYPE_MMS smsId: id
     */
    public static int nmsDeleteSMS(int smsType, int smsId) {
        return NmsSMSMMSManager.getInstance(null).deleteSMS(smsType, new int[] { smsId }, true);
    }

    public static int nmsDeleteSMSTransaction(final int[] mmsIDs, final int[] smsIDs) {
        // new Thread() {
        // public void run() {
        int number = 0;
        if (mmsIDs != null) {
            NmsSMSMMSManager.getInstance(null).deleteSMS(NmsSMSMMS.PROTOCOL_MMS, mmsIDs, true);
            number += mmsIDs.length;
        }
        if (smsIDs != null) {
            NmsSMSMMSManager.getInstance(null).deleteSMS(NmsSMSMMS.PROTOCOL_SMS, smsIDs, true);
            number += smsIDs.length;
        }
        // notifyDeleteOK(number);
        // }
        // }.start();
        return 0;
    }

    /*
     * 获取指定id的sms/mms 信息 smsType: 类型 NMS_TYPE_SMS / NMS_TYPE_MMS smsId: id
     */
    public static SNmsMsgCont nmsUIGetMsgFromNative(int smsType, int smsId) {
        switch (smsType) {
        case NmsSMSMMS.PROTOCOL_MMS:
            return NmsSMSMMSManager.getInstance(null).getMmsMsgContViaId(smsId);

        case NmsSMSMMS.PROTOCOL_SMS:
            return NmsSMSMMSManager.getInstance(null).getSmsMsgContViaId(smsId);

        default:
            NmsLog.error(TAG, "fail to get MsgCont via type&id, unknow Type:" + smsType);
            return null;
        }
    }

    public static void nmsSetTimer(int timerId, int seconds) {
        NmsTimer.NmsSetTimer(timerId, seconds);
    }

    public static void nmsKillTimer(int timerId) {
        NmsTimer.NmsKillTimer(timerId);
    }

    public static void nmsGetWeakupLock(int id) {
        NmsWakeLock.NmsSetWakeupLock(NmsService.getInstance(), "" + id);

    }

    public static void nmsReleaseWeakupLock(int id) {
        NmsWakeLock.NmsReleaseWakeupLock("" + id);
    }

    public static int nmsResetSMSStamp() {
        NmsConfig.clearAllCache();
        return 0;
    }

    public static void nmsResetALlAndRestart() {
        NmsService.unRegAllReciver();

        int disconnectNetwork = engineadapter.msgtype.NMS_ENG_MSG_DISCONNECT_REQ.ordinal();
        engineadapter.get().nmsSendMsgToEngine(disconnectNetwork, null, 0);

        Intent i = new Intent(NmsService.getInstance(), NmsService.class);
        NmsService.getInstance().stopService(i);

        String path = engineadapterforjni.getUserDataPath();
        NmsCommonUtils.delAllFile(path);
        NmsConfig.clearAllCache();
        System.exit(0);
    }

    public static String nmsGetReadModeString(int readMode,int flag) {
        String strRM = "";
        switch (readMode) {
        case NmsIpMessageConsts.NmsIpMessageType.TEXT:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG:
            NmsLog.trace(TAG, "ReadModeString for text is null.");
            break;

        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
            if ((flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_READEDBURN_PIC);
            }else{
                strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_PIC);
            }
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VOICE:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_TAPE);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VCARD:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_VCARD);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.LOCATION:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_LOCATION);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.SKETCH:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_SKETCH);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VIDEO:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_VIDEO);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_CALENDAR);
            break;

        case NmsIpMessageConsts.NmsIpMessageType.UNKNOWN_FILE:
        default:
            strRM = NmsService.getInstance().getString(R.string.STR_NMS_READ_MODE_UNKNOWN);
        }
        return strRM;
    }

    public static int nmsMsgGetTextReadMode(String data) {
        int ret = NmsIpMessageConsts.NmsIpMessageType.TEXT;

        if (NmsLocationFormat.checkIsLocationData(data) == 0) {
            ret = NmsIpMessageConsts.NmsIpMessageType.LOCATION;
        }

        return ret;
    }

    /*
     * public static String nmsGetMmsEmptyLineTwo() { String mmsEmptyLineTwo =
     * ""; mmsEmptyLineTwo = NmsService.context
     * .getString(R.string.STR_NMS_MMS_EMPTY_LINETWO); return mmsEmptyLineTwo; }
     */

    public static String nmsGetImsiViaSimId(int id) {
        if (id < 0) {
            return "";
        }

        String ret = "";
        ret = NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsGetImsi(id);
        NmsLog.trace(TAG, "jni get Imsi via sim Id: " + id + ", and return: " + ret);
        if (TextUtils.isEmpty(ret)) {
            return "";
        }

        return ret;
    }

    public static int nmsGetSimIdViaImsi(String imsi) {
        if (TextUtils.isEmpty(imsi)) {
            return -1;
        }

        int ret = -1;
        ret = NmsDBUtils.getDataBaseInstance(NmsService.getInstance()).nmsGetSimId(imsi);
        NmsLog.trace(TAG, "jni get sim id va imsi: " + ret);
        return ret;
    }

    public static String nmsGetServerAutoEchoMsg(){
        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_SERVER_AUTO_ECHO_MSG));
    }

    public static String nmsGetGroupCreateMsgBody(String self, String members) {
        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_GROUP_CREATE_MSG),
                self, members);
    }

    public static String nmsGetGroupAddMsgBody(String self, String members) {
        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_GROUP_ADD_MSG), members);
    }

    public static String nmsGetGroupDeleteMsgBody(String self, String members) {
        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_GROUP_DELETE_MSG),
                self, members);
    }

    public static String nmsGetQuitMsgBody(String self) {
        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_GROUP_QUITE_MSG),
                self);
    }

    public static int nmsIsNetworkOK() {
        return NmsCommonUtils.isNetworkReady(NmsService.getInstance())? 1:0;
    }

    public static String nmsGetIString(int isI) {
        String iStr = "";
        iStr = NmsService.getInstance().getString(
                (isI != 0) ? R.string.STR_NMS_GROUP_CHAT_I : R.string.STR_NMS_GROUP_CHAT_ME);
        return iStr;
    }

    public static String nmsGetYouString() {
        return NmsService.getInstance().getString(R.string.STR_NMS_GROUP_CHAT_YOU);
    }

    public static String nmsGetGroupChatPlaceHolderMsgBody() {
        return NmsService.getInstance()
                .getString(R.string.STR_NMS_GROUP_CHAT_PLACE_HOLDER_MSG_BODY);
    }

    public static void nmsBackupMsgProgress(int nProgress) {
        // NmsBackupController.getInstance().nmsNotify(NmsBackupController.NMS_BAKCUP_BACKUP_MSG_PROGRESS,
        // nProgress, 0);
    }

    public static int nmsCheckIsWCP2DbExist() {
        return NmsSMSMMSManager.getInstance(null).isExtentionFieldExsit();
    }

    public static void nmsUpdateIpMsgInSysDb(int msgSmsId, int status, int flag) {

        if (msgSmsId == -1) {
            NmsLog.error(TAG, "fatal error in nmsUpdateIpMsgInSysDb for msgSmsId is invalid");
            return;
        }

        NmsSMSMMSManager.getInstance(null).nmsUpdateIpMsgInSysDb(msgSmsId, status, flag);
    }

    public static int nmsUpdateIpMsgFlagsInSysDb(final int[] mmsIDs, final int[] smsIDs,
            final int operation) {
        // new Thread(){
        // public void run(){
        NmsSMSMMSManager.getInstance(null).nmsUpdateIpMsgFlagInSysDb(mmsIDs, smsIDs, operation);
        NmsLog.trace(TAG, "nmsUpdateIpMsgFlagsInSysDb done");
        // Intent intent = new Intent();
        // intent.setAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
        // NmsService.getInstance().sendBroadcast(intent);
        // }
        // }.start();

        return 0;
    }


    public static void nmsUpdateIpMsgRecdIdInSysDb(int status, int id) {
        NmsSMSMMSManager.getInstance(null).nmsUpdateIpMsgRecdIdInSysDb(status, id);

    }

    public static int nmsInsertSms(String threadNumber, String fromOrTo, String msg, int status,
            int flag, int date, int dateSent, int nmsRecId, int simId) {

        return (int) NmsSMSMMSManager.getInstance(null).saveSmsToDb(msg, fromOrTo, threadNumber,
                status, flag, date, dateSent, nmsRecId, (long) simId);
    }

    public static int nmsIsMmsDownloadFinished(int mmsId) {
        return NmsSMSMMSManager.getInstance(null).isMmsDownloaded((long) mmsId) ? 1 : 0;
    }

    public static String nmsGetRegId() {
        return hpnsReceiver.readRegId(NmsService.getInstance());
    }

    public static String nmsGetPNType() {
        return hpnsReceiver.getPNType(NmsService.getInstance());
    }

    public static String getPhoneManufacturer() { //add for jira-569 v3_A
        return android.os.Build.MANUFACTURER;
    }

    public static String getPhoneModelType() {  //add for jira-569 v3_A
        return android.os.Build.MODEL;
    }

    public static void nmsSetAdvicePNType(String type) {
        hpnsReceiver.setAdviceType(type);
    }

    public static void nmsSetSystemLogPriority(int priority) {
        NmsLog.nmsSetSystemLogPriority(priority);
    }

    public static int nmsGetSystemLogPriority() {
        return NmsLog.nmsGetSystemLogPriority();
    }

    public static String nmsGetAttachShareString(String name, int readMode,int flag) {

        if (name == null) {
            NmsLog.error(TAG, "error name for getting attach share string, readMode: + " + readMode);
            return "";
        }

        if (readMode < NmsIpMessageConsts.NmsIpMessageType.PICTURE
                || readMode >= NmsIpMessageConsts.NmsIpMessageType.COUNT) {
            NmsLog.error(TAG, "error readMode for getting attach share string, readMode: + "
                    + readMode);
            return "";
        }

        String readModeString = nmsGetReadModeString(readMode,flag);

        return String.format(NmsService.getInstance().getString(R.string.STR_NMS_ATTACH_SHARE),
                name, readModeString);
    }
    
    static public void sendOfflineStatusToServer(String imsi, String number, int clientSessionId, int serverSessionId) {
        NmsExtProtocol.sendOfflineStatusToServer(imsi, number, clientSessionId, serverSessionId) ;
    }
    
    private static long ipToLong(String strIP) {
        
        if (TextUtils.isEmpty(strIP)) 
            return 0 ;
        
        try {
            long []ip = new long[4];
            int position1=strIP.indexOf(".");
            int position2=strIP.indexOf(".",position1+1);
            int position3=strIP.indexOf(".",position2+1);
            ip[0]=Long.parseLong(strIP.substring(0,position1));
            ip[1]=Long.parseLong(strIP.substring(position1+1,position2));
            ip[2]=Long.parseLong(strIP.substring(position2+1,position3));
            ip[3]=Long.parseLong(strIP.substring(position3+1));
            return (ip[0]) + (ip[1]<<8) + (ip[2]<<16) + (ip[3] << 24); 
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
        
        return 0 ;
  }
    
    public static void getIpViaDNS(String domain) {
        final String host = domain ;
        
        if (TextUtils.isEmpty(domain)) {
            NmsLog.error(TAG, "empty domain in nmsGetIpViaDNS") ;
            return ;
        }
        
        try {
            new Thread(new Runnable() {
                
                @Override
                public void run() {
                    try {
                        
                        if (engineadapter.get() == null) {
                            NmsLog.error(TAG, "engineadapter is null in nmsGetIpViaDNS") ;
                            return ;
                        }
                        
                        InetAddress address = InetAddress.getByName(host) ;
                        if (address != null) {
                            long ip = ipToLong(address.getHostAddress()) ;
                            if (ip != 0) 
                                engineadapter.get().nmsProcessDnsResult((int)ip) ;
                        }
                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e) ;
                    }
                }
            }).start() ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
    
    public static String getSdcardAttachPath() {
        try {
            long phoneSpace = NmsCommonUtils.getDataStorageAvailableSpace();
            long sdcardSpace = 0;

            try {
                sdcardSpace = NmsCommonUtils.getSDcardAvailableSpace();
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
            }

            NmsLog.trace(TAG, String.format("get storage space, phone: %d, sdcard: %d",
                    (int) phoneSpace, (int) sdcardSpace));

            if ((sdcardSpace >= AVALIABLE_SPACE) || (sdcardSpace > phoneSpace)) {
                return NmsCommonUtils.getSDCardPath(NmsService.getInstance()) + File.separator
                        + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "Attach";
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return getUserDataPath() + File.separator + "Attach";
    }
    
	//M: Activation Statistics
    public static void tellActivateOK(int activateType) {
        
        if (activateType <= NmsIpMessageConsts.NmsUIActivateType.AUTO 
                || activateType >= NmsIpMessageConsts.NmsUIActivateType.TYPE_COUNT) {
            return ;
        }
        
        int keyIndex = NmsStatistics.KEY_OTHER_ACTIVATE_OK ;
        if (activateType == NmsIpMessageConsts.NmsUIActivateType.EMOTION) 
            keyIndex = NmsStatistics.KEY_EMO_ACTIVATE_OK ;
        else if (activateType == NmsIpMessageConsts.NmsUIActivateType.MULTI_MEDIA) 
            keyIndex = NmsStatistics.KEY_MEDIA_ACTIVATE_OK ;
        else if (activateType == NmsIpMessageConsts.NmsUIActivateType.SETTING)
            keyIndex = NmsStatistics.KEY_SETTING_ACTIVATE_OK ;
        else if (activateType == NmsIpMessageConsts.NmsUIActivateType.DIALOG) 
            keyIndex = NmsStatistics.KEY_DLG_ACTIVATE_OK ;
        else if (activateType == NmsIpMessageConsts.NmsUIActivateType.PROMPT) 
            keyIndex = NmsStatistics.tips_activate_ok ;
        else if (activateType == NmsIpMessageConsts.NmsUIActivateType.MESSAGE) 
            keyIndex = NmsStatistics.sms_activate_ok ;
        
        Log.i("test1", "engineadapter.......tellActivateOK.......keyIndex:"+keyIndex);
        NmsStatistics.incKeyVal(keyIndex) ;
    }

    public static int getSwitchVariable() {
        return NmsIpMessageConsts.SWITCHVARIABLE;
    }
    
    public static int isDefaultSmsApp() {
        return NmsSMSMMSManager.isDefaultSmsApp() ? 1 : 0 ;
    }
}
