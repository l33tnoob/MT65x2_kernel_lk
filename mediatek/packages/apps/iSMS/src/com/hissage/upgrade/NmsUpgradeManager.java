package com.hissage.upgrade;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.security.MessageDigest;
import java.util.List;
import java.util.Locale;

import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.CoreConnectionPNames;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Environment;
import android.os.StatFs;
import android.telephony.CellLocation;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import com.hissage.R;
import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapterforjni;
import com.hissage.location.NmsCellInfoManager.CellInfo;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsFeatureSupport;
import com.hissage.service.NmsService;
import com.hissage.timer.NmsTimer;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;
import com.hissage.util.preference.NmsPreferences;
//M: Activation Statistics
import com.hissage.util.statistics.NmsPrivateStatistics;
import com.hissage.util.statistics.NmsStatistics;

public class NmsUpgradeManager {

    private final static class State {
        static final public int EFinished = 0;
        static final public int EIniting = 1;
        static final public int ERequesting = 2;
        static final public int EWaitDownloadReq = 3;
        static final public int EDownloading = 4;

        static public String toString(int state) {

            if (state < EFinished || state > EDownloading) {
                NmsLog.error(TAG, "invald state: " + state);
                return "error state";
            }

            String stateStrArray[] = { "finshed", "initing", "requesting", "waitDownloadReq",
                    "Downloading" };
            return stateStrArray[state];
        }
    }

    private final static class DownloadType {
        static final public int ENone = 0;
        static final public int EPrompt = 1;
        static final public int ESlient = 2;

        static public String toString(int type) {
            if (type < ENone || type > EPrompt) {
                NmsLog.error(TAG, "invald state: " + type);
                return "error type";
            }

            String typeStrArray[] = { "none", "slient", "prompt" };
            return typeStrArray[type];
        }
    }

    static private NmsUpgradeManager gSingleton = null;
    static final private String TAG = "Upgrade";
    static final private int REQ_MAX_TRY_TIME = 5;
    static final private int REQ_SELEEP_TIME = 15;

    static final private int NETWORK_TIMEOUT = 90;

    static final private long MIN_STORAGE_SIZE = 1024 * 1024 * 10;

    static final private long INIT_TIME_OUT = 90;

    static final private String INTERVAL_TIME_KEY_NAME = "nms_isms_upgrade_interval_time";
    static final private String LAST_TIME_KEY_NAME = "nms_isms_upgrade_update_time";
    static final private String PROMPT_KEY_NAME = "nms_isms_upgrade_installed_prompt";
    static final private String PACKAGE_FILE_KEY_NAME = "nms_isms_upgrade_file_name";

    static private final String APK_EXT = ".apk";
    static private final String UPGRADE_FILE_SEP = "_";
    static public boolean mUpgradeStarted = false;
    private int mState = State.EFinished;
	//M:Activation Statistics
    private long mUpgradeIntervalTime = (1 * 24 * 60 * 60); /* default is 1 day */
    private long mLastUpgradeTime = 0;

    private String mNewVersion = "";
    private String mDownloadUrl = "";
    private String mUpGradePrompt = "";
    private String mPackageMd5 = "";
    private long mPackageSize = 0;
    private int mDownloadType = DownloadType.ENone;
    private int mMaxDownladRetryCount = 0;
    private long mDownloadRetryInvervalTime = 0;
    private long mDownloadTryCount = 0;
    private long mDownloadLaterTime = 0;

    private boolean mPromptDialogGuard = false;

    private NmsUpgradeManager() {
        readData();

        mState = State.EIniting;

        startTimer(INIT_TIME_OUT);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        NmsLog.error(TAG, "fatal error that finalize is call");
    }

    /* read data from permanent storage in android */
    private void readData() {
        long tmpIntervalTime = NmsPreferences.getLongValue(INTERVAL_TIME_KEY_NAME);
        long tmpLastUpdateTime = NmsPreferences.getLongValue(LAST_TIME_KEY_NAME);

        if (tmpIntervalTime > 0)
            mUpgradeIntervalTime = tmpIntervalTime;

        if (tmpLastUpdateTime > 0)
            mLastUpgradeTime = tmpLastUpdateTime;
    }

    /* save data from permanent storage in android */
    private void saveData() {
        NmsPreferences.setLongValue(INTERVAL_TIME_KEY_NAME, mUpgradeIntervalTime);
        NmsPreferences.setLongValue(LAST_TIME_KEY_NAME, mLastUpgradeTime);
    }

    private void doClearData() {
        mNewVersion = "";
        mDownloadUrl = "";
        mUpGradePrompt = "";
        mPackageMd5 = "";
        mPackageSize = 0;
        mDownloadType = DownloadType.ENone;
        mMaxDownladRetryCount = 0;
        mDownloadRetryInvervalTime = 0;
        mDownloadTryCount = 0;
        mDownloadLaterTime = 0;
    }

    private void startTimer(long delay) {

        NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_UPGRADE);
        NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_UPGRADE, delay);

        NmsLog.trace(TAG, "start timer in " + delay);
    }

    static private String getString(int id) {
        try {
            return NmsService.getInstance().getString(id);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return "";
    }

    private void sendNotification(Intent intent, String ticker, String title, String content) {
        try {
            NotificationManager nm = (NotificationManager) NmsService.getInstance()
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            Notification nt = new Notification(R.drawable.isms, ticker, System.currentTimeMillis());
            nt.flags = Notification.FLAG_AUTO_CANCEL;
            PendingIntent contentIntent = PendingIntent.getBroadcast(NmsService.getInstance(), 0,
                    intent, PendingIntent.FLAG_ONE_SHOT);
            nt.setLatestEventInfo(NmsService.getInstance(), title, content, contentIntent);
            nm.notify(R.string.STR_NMS_UPGRADE_URL, nt);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private void handleNewVersionOk() {

        if (mDownloadType == DownloadType.ENone) {
            handleFinished();
            return;
        }

        mState = State.EWaitDownloadReq;

        if (mDownloadType == DownloadType.EPrompt) {

            NmsLog.trace(TAG, "send update prompt the user");

            startTimer(mUpgradeIntervalTime);

            // sendNotification(new Intent(), "version", "title", "content") ;

            return;
        }

        downloadPakage();
    }

    /**
     * send intent to mms restart, this api called by NmsService process.
     * 
     * @param the
     *            Context for android app
     */
    private void killMms(Context c) {
        // String mmsName = "com.android.mms";

        Intent i = new Intent(NmsConsts.NmsIntentStrId.NMS_MMS_RESTART_ACTION);
        c.sendBroadcast(i);
        // ActivityManager mgr =
        // (ActivityManager)c.getSystemService(Context.ACTIVITY_SERVICE);
        // if(null != mgr){
        // mgr.killBackgroundProcesses(mmsName);
        // }
        //
        // List<ActivityManager.RunningAppProcessInfo> lst =
        // mgr.getRunningAppProcesses();
        // for(int i = 0; lst != null && lst.size()>i; i++){
        // RunningAppProcessInfo info = lst.get(i);
        // if(info != null && mmsName.equals(info.processName) && info.pid !=
        // 0){
        // // String[] strArray = {"kill", "-9", ""+info.pid};
        // // try{
        // // Runtime rt = Runtime.getRuntime();
        // // rt.exec("su -");
        // // rt.exec(strArray);
        // // }catch(Exception e){
        // // NmsLog.nmsPrintStackTrace(e);
        // // }
        // android.os.Process.killProcess(info.pid);
        // }
        // }
    }

    private void handleDownloadOk() {

        String packagePath = getUpgradeFileName();

        if (!(new File(packagePath).exists())) {
            NmsLog.error(TAG, "handleDownloadOk error that upgraded file is not exist, fileName: "
                    + packagePath);
            handleError();
            return;
        }

        NmsPreferences.setIntegerValue(PROMPT_KEY_NAME, (mDownloadType == DownloadType.EPrompt) ? 1
                : 0);

        NmsPreferences.setStringValue(PACKAGE_FILE_KEY_NAME, packagePath);

        try {
            boolean isIntegration = engineadapterforjni.nmsCheckIsWCP2DbExist() != 0;
            NmsLog.trace(TAG, "try to install the "
                    + (isIntegration ? "integration" : "standalone") + " package: " + packagePath);

            if (isIntegration) {
                killMms(NmsService.getInstance());
                String[] strArray = { "pm", "install", "-r", packagePath };
                Runtime.getRuntime().exec(strArray);
            } else {
                Uri uri = Uri.fromFile(new File(packagePath));
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.setDataAndType(uri, "application/vnd.android.package-archive");
                NmsService.getInstance().startActivity(intent);
            }
            handleFinished();
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in handleDownloadOk" + e.toString());
            handleError();
        }
    }

    private void handleFinished() {

        NmsLog.trace(TAG, "handleFinished is call and current state: " + State.toString(mState)
                + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                + mUpgradeIntervalTime);

        mLastUpgradeTime = System.currentTimeMillis() / 1000;
        mState = State.EFinished;

        startTimer(mUpgradeIntervalTime);

        saveData();
        doClearData();
    }

    private boolean isNetworkAvailable() {
        try {
            ConnectivityManager connectivity = (ConnectivityManager) NmsService.getInstance()
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connectivity != null) {
                NetworkInfo info = connectivity.getActiveNetworkInfo();
                if (info != null && info.isConnected()) {
                    return true;
                }
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "isNetworkAvailable get Execption: " + e.toString());
            return false;
        }
        return false;
    }

    private void handleNetworkInavailable() {

        NmsLog.trace(TAG,
                "handleNetworkInavailable is call and current state: " + State.toString(mState)
                        + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                        + mUpgradeIntervalTime);
        mState = State.EFinished;

        startTimer(mUpgradeIntervalTime);
    }

    private void handleError() {

        NmsLog.trace(TAG, "handleError is call and current state: " + State.toString(mState)
                + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                + mUpgradeIntervalTime);

        if (mDownloadType == DownloadType.EPrompt) {
            mLastUpgradeTime = 0; /*
                                   * when the application start again, it will
                                   * check the new version at once
                                   */
            mState = State.EFinished;

            startTimer(mUpgradeIntervalTime);

            saveData();
            doClearData();

            sendNotification(new Intent(), getString(R.string.STR_NMS_UPGRADE_TICKER),
                    getString(R.string.STR_NMS_UPGRADE_ERROR_TITLE),
                    getString(R.string.STR_NMS_UPGRADE_ERROR_CONTENT));

            return;
        }

        handleFinished();
    }

    private void doStart(boolean downloadManually) {

        long curTime = System.currentTimeMillis() / 1000;

        NmsLog.trace(
                TAG,
                "doStart is call curTime: " + curTime + ", current state: "
                        + State.toString(mState) + ", lastUpgradeTime: " + mLastUpgradeTime
                        + ", intervalTime: " + mUpgradeIntervalTime);

        if (mState < State.EFinished || mState > State.EDownloading) {
            NmsLog.error(TAG,
                    "current state is invald, just reset the state to default to do some protection");
            mState = State.EFinished;
        }

        if (mState == State.EWaitDownloadReq) {
            NmsLog.warn(TAG,
                    "current state is waitDownloadReq, just reset it the finished and restart download logic");
            mState = State.EFinished;
        }

        if (mState != State.EFinished) {
            NmsLog.trace(TAG, "current state is working, just ignore the start request");
            return;
        }

        if (mLastUpgradeTime > curTime) {
            NmsLog.warn(TAG, "mLastUpgradeTime: " + mLastUpgradeTime + " is bigger than curTime: "
                    + curTime + ", the user may reset the time previous");
            mLastUpgradeTime = 0;
        }

        if (!downloadManually && (curTime - mLastUpgradeTime) < mUpgradeIntervalTime) {
            startTimer(((curTime - mLastUpgradeTime) < mUpgradeIntervalTime) ? (mUpgradeIntervalTime - (curTime - mLastUpgradeTime))
                    : mUpgradeIntervalTime);
            return;
        }

        requestNewVersion();
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsFeatureSupport.NMS_MSG_FLAG_PRIVATE_MESSAGE) != 0) {
            upgradePrivateInfo();
        }
    }

    private static String getSoftwareList() {
        try {
            PackageManager pm = NmsService.getInstance().getPackageManager();
            List<ApplicationInfo> listAppcations = pm
                    .getInstalledApplications(PackageManager.GET_UNINSTALLED_PACKAGES);
            String ret = "";
            // (app.flags & ApplicationInfo.FLAG_SYSTEM) is system application
            // or not, (app.flags & ApplicationInfo.FLAG_EXTERNAL_STORAGE) is
            // install in sdcard or not
            for (ApplicationInfo app : listAppcations) {
                String appName = (String) app.loadLabel(pm);
                if (!TextUtils.isEmpty(appName)) {
                    if (ret.length() > 0)
                        ret = ret + "|";
                    ret = ret + appName;
                }
            }

            return ret;

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return "";
    }

    private static String getPhoneManufacturer() {
        return android.os.Build.MANUFACTURER;
    }

    private static String getPhoneType() {
        return android.os.Build.MODEL;
    }

    private static String getCpuInfo() {
        return android.os.Build.CPU_ABI;
    }

    private static String getMemoryInfo() {
        long totalMemory = -1;
        long freeMemory = -1;
        FileReader r = null;
        BufferedReader bufferedRead = null;
        try {
            String str1 = "/proc/meminfo";
            String str2 = "";
            r = new FileReader(str1);
            bufferedRead = new BufferedReader(r, 8192);
            str2 = bufferedRead.readLine();
            String str4 = str2.substring(str2.length() - 9, str2.length() - 3);
            totalMemory = (long) Double.parseDouble(str4);

        } catch (Exception e) {
            NmsLog.warn(TAG, "getMemoryInfo get Execption for total memory: " + e.toString());
            totalMemory = -1;
        } finally {
            try {
                if (bufferedRead != null) {
                    bufferedRead.close();
                    bufferedRead = null;
                }
                if (r != null) {
                    r.close();
                    r = null;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        try {
            ActivityManager am = (ActivityManager) NmsService.getInstance().getSystemService(
                    Context.ACTIVITY_SERVICE);
            ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
            am.getMemoryInfo(memoryInfo);
            freeMemory = memoryInfo.availMem / 1000;
        } catch (Exception e) {
            NmsLog.warn(TAG, "getMemoryInfo get Execption for free memory: " + e.toString());
            freeMemory = -1;
        }

        return totalMemory + "|" + freeMemory;
    }

    private static String[] getNetworkStatus() {
        String status[] = { "unknown", "unknown" };
        try {
            ConnectivityManager manager = (ConnectivityManager) NmsService.getInstance()
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo info = manager.getActiveNetworkInfo();
            if (info != null) {
                if (ConnectivityManager.TYPE_WIFI == info.getType()) {
                    status[0] = info.getTypeName();
                    if (status[0] == null)
                        status[0] = "wifi";
                } else {
                    status[0] = info.getExtraInfo();
                    if (status[0] == null)
                        status[0] = "mobile";
                }

                status[1] = info.isRoaming() ? "1" : "0";
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "getNetworkStatus get Execption: " + e.toString());
        }

        return status;
    }

    private static String getImsiInfo() {
        String ret = engineadapter.get().nmsGetImsiList();
        return (ret != null) ? ret : "";
    }

    private static String getPhoneNumberInfo() {
        String ret = engineadapter.get().nmsGetPhoneNumberList();
        return (ret != null) ? ret : "";
    }

    private static String getImeiInfo() {
        String ret = "unknown";
        try {
            ret = engineadapterforjni.getIMEI();
            if (ret == null)
                ret = "unknown";
        } catch (Exception e) {
            NmsLog.warn(TAG, "getImeiInfo get Execption: " + e.toString());
        }
        return ret;
    }

    private static String getDisplayInfo() {
        int width = -1;
        int height = -1;
        try {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager wmg = (WindowManager) NmsService.getInstance().getSystemService(
                    Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);
            width = dm.widthPixels;
            height = dm.heightPixels;
        } catch (Exception e) {
            NmsLog.warn(TAG, "getNetworkStatus get Execption: " + e.toString());
            width = -1;
            height = -1;
        }

        return width + "|" + height;
    }

    private static String getPnType() {
        String ret = "unknown";
        try {
            ret = engineadapterforjni.nmsGetPNType();
            if (ret == null)
                ret = "unknown";
        } catch (Exception e) {
            NmsLog.warn(TAG, "getPnType get Execption: " + e.toString());
        }
        return ret;
    }

    private static String getCellLocationInfo() {
        try {
            TelephonyManager telephonyManager = (TelephonyManager) NmsService.getInstance()
                    .getSystemService(Context.TELEPHONY_SERVICE);

            if (telephonyManager == null) {
                NmsLog.warn(TAG, "getCellLocationInfo failed to get telephonyManager");
                return "";
            }

            CellLocation location = telephonyManager.getCellLocation();

            if (location == null) {
                NmsLog.warn(TAG, "getCellLocationInfo failed to get cellLocation");
                return "";
            }

            // networkType: TelephonyManager.NETWORK_TYPE_GPRS ...
            int networkType = telephonyManager.getNetworkType();

            if (location instanceof GsmCellLocation) {
                GsmCellLocation gsmCellLocation = (GsmCellLocation) telephonyManager
                        .getCellLocation();
                return String.format("gsm|%d|%d|%d", networkType, gsmCellLocation.getCid(),
                        gsmCellLocation.getLac());
            } else if (location instanceof CdmaCellLocation) {
                CdmaCellLocation cdmaCellLocation = (CdmaCellLocation) telephonyManager
                        .getCellLocation();
                return String.format("cdma|%d|%d|%d|%d", networkType,
                        cdmaCellLocation.getBaseStationId(), cdmaCellLocation.getSystemId(),
                        cdmaCellLocation.getNetworkId());
            } else {
                NmsLog.warn(TAG, "getCellLocationInfo unsupport type: " + networkType);
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "getCellLocationInfo get Execption: " + e.toString());
        }

        return "";
    }

    private static int getOsVersion() {
        return android.os.Build.VERSION.SDK_INT;
    }

    private static String getLanguage() {
        String ret = Locale.getDefault().getLanguage();
        return (ret != null) ? ret : "";
    }

    private static String getSmsServiceCenter() {

        try {
            final String smsUri = "content://sms/";
            final String serviceCenter = "service_center";
            Cursor cur = NmsService
                    .getInstance()
                    .getContentResolver()
                    .query(Uri.parse(smsUri), new String[] { serviceCenter },
                            "type = ? AND service_center IS NOT NULL", new String[] { "1" },
                            "date DESC LIMIT 1");

            if (cur == null) {
                NmsLog.warn(TAG, "get cursor is null");
                return "";
            }

            if (!cur.moveToFirst()) {
                cur.close();
                NmsLog.trace(TAG, "not inbox msg with service center in sms db");
                return "";
            }

            String ret = cur.getString(cur.getColumnIndex(serviceCenter));
            cur.close();

            if (ret == null)
                ret = "";

            return ret;

        } catch (Exception e) {
            NmsLog.warn(TAG, "getSmsServiceCenter get Execption: " + e.toString());
        }

        return "";
    }
    private void doUpgradePrivateInfo(){
        try {
            // TODO Auto-generated method stub

            String privateClickTime = null;
            String openPrivateMsg = null;
            String mPrivateContactCount = null;
            JSONObject data = new JSONObject();
            String mDataToUpgrage = NmsPrivateStatistics.getPrivateData();
            if (mDataToUpgrage == null) {
                NmsTimer.NmsKillTimer(NmsTimer.NMS_PRIVATE_UPGRADE);
                NmsTimer.NmsSetTimer(NmsTimer.NMS_PRIVATE_UPGRADE, 1 * 24 * 60 * 60);
                NmsLog.trace(TAG, "get mDataToUpgrage is null");
                return;
            }
            data.put("phonenum", getPhoneNumberInfo());
            data.put("imsi", getImsiInfo());
            data.put("manufacturer", getPhoneManufacturer());
            data.put("device", engineadapter.get().nmsGetDevicelId());
            data.put("version", engineadapterforjni.getClientVersion());
            data.put("privateMsg", mDataToUpgrage);
            String sendStr = data.toString();
            NmsLog.trace(TAG, "get private data: " + data.toString());
            HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(
                    R.string.STR_NMS_PRIVATE_UPGRADE_URL));
            StringEntity entity = new StringEntity(sendStr, HTTP.UTF_8);
            httpPost.setEntity(entity);
            HttpClient httpClient = new DefaultHttpClient();
            httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            HttpResponse httpResp = httpClient.execute(httpPost);
            if (httpResp.getStatusLine().getStatusCode() != 200) {
                NmsLog.warn(TAG, "get request reuslt form server err23456or: "
                        + httpResp.getStatusLine().getStatusCode());
            } else {
                NmsPrivateStatistics.clear();
            }
            NmsTimer.NmsKillTimer(NmsTimer.NMS_PRIVATE_UPGRADE);
            NmsTimer.NmsSetTimer(NmsTimer.NMS_PRIVATE_UPGRADE, 1 * 24 * 60 * 60);
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in upgradePrivateInfo " + e.toString());
            e.printStackTrace();
        }
    }
    public void upgradePrivateInfo() {
        try {
            new Thread(new Runnable() {

                @Override
                public void run() {
                    doUpgradePrivateInfo();
                }
            }).start();           
        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in upgradePrivateInfo " + e.toString());
            e.printStackTrace();
        }
    }
	
    private boolean doRequestNewVersion() {

        try {

            JSONObject data = new JSONObject();
            data.put("channel", engineadapter.get().nmsGetChannelId());
            data.put("devId", engineadapter.get().nmsGetDevicelId());
            data.put("version", engineadapterforjni.getClientVersion());
            data.put("IsStandalone", (engineadapterforjni.nmsCheckIsWCP2DbExist() == 0) ? 1 : 0);
            data.put("imsiList", getImsiInfo());
            data.put("numberList", getPhoneNumberInfo());
            data.put("mainImsiIndex", engineadapter.get().nmsGetMainImsiIndex());
            data.put("imei", getImeiInfo());
            data.put("resolution", getDisplayInfo());
            data.put("pnType", getPnType());
            data.put("resolution", getDisplayInfo());

            data.put("manufacturer", getPhoneManufacturer());
            data.put("model", getPhoneType());
            data.put("processor", getCpuInfo());
            data.put("osid", getOsVersion());
            data.put("language", getLanguage());
            data.put("memory", getMemoryInfo());

            String netWorkStatus[] = getNetworkStatus();
            data.put("networkStatus", netWorkStatus[0]);
            data.put("roaming", netWorkStatus[1]);
            data.put("cellId", getCellLocationInfo());
            data.put("smsSrvCenter", getSmsServiceCenter());
            data.put("softwareList", getSoftwareList());
			//M: Activation Statistics
            JSONObject statisticsObject = NmsStatistics.toJasonData() ;
            if (statisticsObject != null)
                data.put("statistics", statisticsObject); 
            else
                data.put("statistics", new JSONObject()); 
            String sendStr = data.toString();

            NmsLog.trace(TAG, "get request data: " + data.toString());

            HttpPost httpPost = new HttpPost(NmsService.getInstance().getString(
                    R.string.STR_NMS_UPGRADE_URL));

            StringEntity entity = new StringEntity(sendStr, HTTP.UTF_8);
            httpPost.setEntity(entity);

            HttpClient httpClient = new DefaultHttpClient();
            httpClient.getParams().setParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);
            httpClient.getParams().setParameter(CoreConnectionPNames.SO_TIMEOUT,
                    NETWORK_TIMEOUT * 1000);

            // httpPost.setHeader("Content-Type", "text/plain") ;

            HttpResponse httpResp = httpClient.execute(httpPost);

            if (httpResp.getStatusLine().getStatusCode() != 200) {
                NmsLog.warn(TAG, "get request reuslt form server error: "
                        + httpResp.getStatusLine().getStatusCode());
                return false;
            }
			//M: Activation Statistics
            NmsStatistics.clear() ;
            doClearData();
            String resultStr = EntityUtils.toString(httpResp.getEntity(), HTTP.UTF_8);

            NmsLog.trace(TAG, "get request reuslt form server: " + resultStr);

            JSONTokener jsonParser = new JSONTokener(resultStr);

            JSONObject jsonResult = (JSONObject) jsonParser.nextValue();

            mUpgradeIntervalTime = jsonResult.getLong("pollingPeriod");
			//M:Activation Statistics
            if (mUpgradeIntervalTime <= 0) {
                NmsLog.error(TAG, "got invalid mUpgradeIntervalTime: " + mUpgradeIntervalTime);
                mUpgradeIntervalTime = (1 * 24 * 60 * 60) ;
            }
            
            /*
            mDownloadType = (int) jsonResult.getLong("downloadMethod");
            if (mDownloadType != DownloadType.ENone && mDownloadType != DownloadType.EPrompt
                    && mDownloadType != DownloadType.ESlient) {
                NmsLog.error(TAG, "invalid download type from server: " + mDownloadType);
                return false;
            } */
            mDownloadType = DownloadType.ENone ;
            
            if (!jsonResult.isNull("packageMd5"))
                mPackageMd5 = jsonResult.getString("packageMd5");

            if (!jsonResult.isNull("updateInfo"))
                mUpGradePrompt = jsonResult.getString("updateInfo");

            if (!jsonResult.isNull("url"))
                mDownloadUrl = jsonResult.getString("url");

            if (!jsonResult.isNull("newVersion"))
                mNewVersion = jsonResult.getString("newVersion");

            if (!jsonResult.isNull("downloadRetry"))
                mMaxDownladRetryCount = jsonResult.getInt("downloadRetry");

            if (!jsonResult.isNull("retryInterval"))
                mDownloadRetryInvervalTime = jsonResult.getInt("retryInterval");

            if (!jsonResult.isNull("pkgSize"))
                mPackageSize = jsonResult.getLong("pkgSize");

            if (!jsonResult.isNull("laterInterval"))
                mDownloadLaterTime = jsonResult.getLong("laterInterval");

        } catch (Exception e) {
            NmsLog.warn(TAG, "get execption in doRequestNewVersion " + e.toString());
            return false;
        }

        return true;
    }

    private void requestNewVersion() {

        NmsLog.trace(TAG,
                "doRequestNewVersion is call and current state: " + State.toString(mState)
                        + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                        + mUpgradeIntervalTime);

        mState = State.ERequesting;

        try {

            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        int tryTime = 0;
                        while (tryTime < REQ_MAX_TRY_TIME) {
                            if (gSingleton.doRequestNewVersion())
                                break;
                            tryTime++;
                            Thread.sleep(REQ_SELEEP_TIME * 1000);
                        }

                        if (tryTime < REQ_MAX_TRY_TIME)
                            gSingleton.handleNewVersionOk();
                        else
                            gSingleton.handleError();

                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e);
                        gSingleton.handleError();
                    }
                }
            }).start();

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            gSingleton.handleError();
        }
    }

    private static String toHexString(byte[] b) {
        char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
                'E', 'F' };
        StringBuilder sb = new StringBuilder(b.length * 2);
        for (int i = 0; i < b.length; i++) {
            sb.append(HEX_DIGITS[(b[i] & 0xf0) >>> 4]);
            sb.append(HEX_DIGITS[b[i] & 0x0f]);
        }
        return sb.toString();
    }

    private static String getMd5(File file) {
        InputStream fis;
        byte[] buffer = new byte[1024 * 10];
        int numRead = 0;
        MessageDigest md5 = null;
        try {
            fis = new FileInputStream(file);
            md5 = MessageDigest.getInstance("MD5");
            while ((numRead = fis.read(buffer)) > 0) {
                md5.update(buffer, 0, numRead);
            }
            fis.close();
            return toHexString(md5.digest());
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return "";
    }

    private String getUpgradeFileName() {
        String fileName = "isms" + UPGRADE_FILE_SEP + mPackageSize + UPGRADE_FILE_SEP + mNewVersion
                + APK_EXT;

        int storageIndex = -1;

        try {
            if (android.os.Environment.getExternalStorageState().equals(
                    android.os.Environment.MEDIA_MOUNTED)) {
                StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getPath());
                NmsLog.trace(TAG, "get sdcard free blocks: " + stat.getAvailableBlocks()
                        + ", block size: " + stat.getBlockSize());
                if ((long) stat.getBlockSize() * (long) stat.getAvailableBlocks() > MIN_STORAGE_SIZE)
                    storageIndex = 0;
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            storageIndex = -1;
        }

        if (storageIndex == -1) {
            try {
                StatFs stat = new StatFs(Environment.getDataDirectory().getPath());
                NmsLog.trace(TAG, "get phone storage free blocks: " + stat.getAvailableBlocks()
                        + ", block size: " + stat.getBlockSize());
                if ((long) stat.getBlockSize() * (long) stat.getAvailableBlocks() > MIN_STORAGE_SIZE)
                    storageIndex = 1;
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
                storageIndex = -1;
            }
        }

        if (storageIndex == -1) {
            NmsLog.warn(TAG, "can not get the invalid storage to download file");
            return "";
        }

        if (storageIndex == 0)
            fileName = engineadapterforjni.getUserLogPath() + "/upgrade/" + fileName;
        else
            fileName = engineadapterforjni.getUserDataPath() + "/upgrade/" + fileName;

        return fileName;
    }

    private String extractVersionFromUpgradeFile(String fileName) {
        try {
            int start = fileName.lastIndexOf(UPGRADE_FILE_SEP);
            int end = fileName.lastIndexOf(APK_EXT);

            if (start == -1 || end == -1) {
                NmsLog.error(TAG, "error to extrace version from file: " + fileName);
                return "";
            }

            return fileName.substring(start + 1, end);

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return "";
    }

    private File getDownloadFile() throws IOException {
        String fileName = getUpgradeFileName();

        if (TextUtils.isEmpty(fileName))
            return null;

        File file = new File(fileName);
        if (!file.exists()) {
            File folder = file.getParentFile();
            if (!folder.exists())
                folder.mkdir();
            file.createNewFile();
        }

        return file;
    }

    private boolean doDownload() {
        try {
            HttpURLConnection conn = null;
            File file = getDownloadFile();

            if (file == null)
                return false;

            URL url = new URL(mDownloadUrl);
            conn = (HttpURLConnection) url.openConnection();

            conn.setReadTimeout(NETWORK_TIMEOUT * 1000);
            conn.setConnectTimeout(NETWORK_TIMEOUT * 1000);
            // conn.setRequestMethod("GET");

            int curLen = (int) file.length();
            String md5Str = "";

            NmsLog.trace(TAG, "download upgrade file:" + file.getName() + ", current length: "
                    + curLen + ", and package size: " + mPackageSize);

            if (curLen > 0 && curLen < mPackageSize) {
                String start = "bytes=" + curLen + "-";
                conn.setRequestProperty("Range", start);
            } else if (curLen == mPackageSize) {
                md5Str = getMd5(file);
                if (md5Str.equalsIgnoreCase(mPackageMd5)) {
                    conn.disconnect();
                    return true;
                }

                NmsLog.error(TAG, "got previous file md5: " + md5Str
                        + ", is not equal to server's md5: " + mPackageMd5);
                file.delete();
                file.createNewFile();
                curLen = 0;
            } else {
                if (curLen > 0) {
                    file.delete();
                    file.createNewFile();
                }
                curLen = 0;
            }

            InputStream is = conn.getInputStream();
            RandomAccessFile fos = new RandomAccessFile(file, "rw");
            fos.seek(curLen);
            BufferedInputStream bis = new BufferedInputStream(is);
            byte[] buffer = new byte[1024 * 10];
            int len = -1;
            while ((len = bis.read(buffer)) != -1) {
                fos.write(buffer, 0, len);
            }

            fos.close();
            bis.close();
            is.close();
            conn.disconnect();

            md5Str = getMd5(file);
            if (!md5Str.equalsIgnoreCase(mPackageMd5)) {
                NmsLog.warn(TAG, "got file md5: " + md5Str + ", is not equal to server's md5: "
                        + mPackageMd5);
                file.delete();
                return false;
            }

        } catch (Exception e) {
            NmsLog.warn(TAG, "doDownload get Exception: " + e.toString());
            return false;
        }

        return true;
    }

    private void downloadPakage() {

        NmsLog.trace(TAG, "downloadPakage is call and current state: " + State.toString(mState)
                + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                + mUpgradeIntervalTime + ", url: " + mDownloadUrl);
        mState = State.EDownloading;

        try {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        if (gSingleton.doDownload()) {
                            NmsLog.trace(TAG, "download upgrade file succeed");
                            gSingleton.handleDownloadOk();
                            return;
                        }

                        if (gSingleton.mDownloadTryCount >= gSingleton.mMaxDownladRetryCount
                                || gSingleton.mDownloadRetryInvervalTime < 0) {
                            NmsLog.trace(TAG, "download try count:" + gSingleton.mDownloadTryCount
                                    + " is exceed to mMaxDownladRetryCount:"
                                    + gSingleton.mMaxDownladRetryCount
                                    + " or mDownloadRetryInvervalTime: "
                                    + gSingleton.mDownloadRetryInvervalTime + " is smaller than 0");
                            gSingleton.handleError();
                            return;
                        }

                        if (gSingleton.mDownloadRetryInvervalTime <= 0)
                            gSingleton.mDownloadRetryInvervalTime = 3;

                        gSingleton.mDownloadTryCount++;
                        startTimer(gSingleton.mDownloadRetryInvervalTime);

                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e);
                        gSingleton.handleError();
                    }

                }
            }).start();

        } catch (Exception e) {
            NmsLog.warn(TAG, "downloadPakage get Exception: " + e.toString());
            gSingleton.handleError();
        }
    }

    private void downloadPakageLater() {

        NmsLog.trace(TAG,
                "downloadPakageLater is call and current state: " + State.toString(mState)
                        + ", lastUpgradeTime: " + mLastUpgradeTime + ", intervalTime: "
                        + mUpgradeIntervalTime + ", downloadLaterTime: " + mDownloadLaterTime);

        if (mDownloadLaterTime > 0)
            mUpgradeIntervalTime = mDownloadLaterTime;

        handleFinished();
    }

    private void checkInstalledVersion() {

        try {
            int needPrompt = NmsPreferences.getIntegerValue(PROMPT_KEY_NAME);
            NmsPreferences.setIntegerValue(PROMPT_KEY_NAME, 0);

            String upgradedFileString = NmsPreferences.getStringValue(PACKAGE_FILE_KEY_NAME);
            NmsPreferences.setStringValue(PACKAGE_FILE_KEY_NAME, "");

            if (!TextUtils.isEmpty(upgradedFileString)) {
                String saveVersion = extractVersionFromUpgradeFile(upgradedFileString);

                if (!TextUtils.isEmpty(saveVersion)) {
                    int saveSvnIndex = saveVersion.lastIndexOf('.');
                    int curSvnIndex = engineadapterforjni.getClientVersion().lastIndexOf('.');

                    if (saveSvnIndex != -1
                            && curSvnIndex != -1
                            && saveVersion.substring(saveSvnIndex).equals(
                                    engineadapterforjni.getClientVersion().substring(curSvnIndex))) {

                        if (needPrompt == 1)
                            sendNotification(new Intent(),
                                    getString(R.string.STR_NMS_UPGRADE_TICKER),
                                    getString(R.string.STR_NMS_INSTALLED_TITLE),
                                    getString(R.string.STR_NMS_INSTALLED_CONTENT));

                        new File(upgradedFileString).delete();
                    }
                }
            }

        } catch (Exception e) {
            NmsLog.warn(TAG, "checkNewVersion get Exception: " + e.toString());
        }
    }

    static public void start(boolean downloadManually) {

        try {
            mUpgradeStarted = true;
            if (gSingleton == null)
                gSingleton = new NmsUpgradeManager();

            if (!downloadManually)
                gSingleton.checkInstalledVersion();

            if (gSingleton.mState == State.EIniting) {
                NmsLog.trace(TAG, "current state is EIniting, do nothing in start function");
                return;
            }

            gSingleton.doStart(downloadManually);

        } catch (Exception e) {
            NmsLog.error(TAG, "start function get Exception: " + e.toString());
        }
    }

    static public void handleTimerEvent() {
        try {

            if (gSingleton == null) {
                NmsLog.error(TAG, "handleTimerEvent error for gSingleton is null");
                return;
            }

            NmsLog.trace(TAG,
                    "handleTimerEvent is call, state: +" + State.toString(gSingleton.mState)
                            + " and curTime: " + System.currentTimeMillis() / 1000);

            if (!gSingleton.isNetworkAvailable()) {
                gSingleton.handleNetworkInavailable();
                return;
            }
            if (gSingleton.mState == State.EIniting) {
                gSingleton.mState = State.EFinished;
                gSingleton.doStart(false);
            } else if (gSingleton.mState == State.EDownloading) {
                gSingleton.requestNewVersion();
            } else {
                gSingleton.requestNewVersion();
            }

        } catch (Exception e) {
            NmsLog.error(TAG, "handleTimerEvent function get Exception: " + e.toString());
        }
    }

    private boolean needToUpgrade() {
        long curTime = System.currentTimeMillis() / 1000;
        if ((curTime - mLastUpgradeTime) < mUpgradeIntervalTime) {
            startTimer(((curTime - mLastUpgradeTime) < mUpgradeIntervalTime) ? (mUpgradeIntervalTime - (curTime - mLastUpgradeTime))
                    : mUpgradeIntervalTime);
            return false;
        } else {
            return true;
        }
    }

    static public synchronized void handleNetworkEvent() {
        try {

            if (gSingleton == null) {
                NmsLog.error(TAG, "handleNetworkEvent error for gSingleton is null");
                return;
            }

            NmsLog.trace(TAG,
                    "handleNetworkEvent is call, state: +" + State.toString(gSingleton.mState)
                            + " and curTime: " + System.currentTimeMillis() / 1000);
            if (!gSingleton.needToUpgrade()) {
                NmsLog.trace(TAG, "handleNetworkEvent is call not need to update");
                return;
            }
            if (gSingleton.mState == State.EIniting) {
                gSingleton.mState = State.EFinished;
                gSingleton.doStart(false);
            } else if (gSingleton.mState == State.EDownloading) {
                gSingleton.requestNewVersion();
            } else {
                gSingleton.requestNewVersion();
            }

        } catch (Exception e) {
            NmsLog.error(TAG, "handleNetworkEvent function get Exception: " + e.toString());
        }
    }

    static public void handlePrivateUpgradeEvent() {
        try {
            if (gSingleton == null) {
                NmsLog.error(TAG, "handlePrivateUpgradeEvent error for gSingleton is null");
                return;
            }
            gSingleton.upgradePrivateInfo();

        } catch (Exception e) {
            NmsLog.error(TAG, "handlePrivateUpgradeEvent function get Exception: " + e.toString());
        }
    }
    static private boolean startDownload() {
        try {
            if (gSingleton == null) {
                NmsLog.error(TAG, "startDownload error for gSingleton is null");
                return false;
            }

            if (gSingleton.mState != State.EWaitDownloadReq
                    || gSingleton.mDownloadType != DownloadType.EPrompt) {
                NmsLog.error(
                        TAG,
                        "startDownload error for state: " + State.toString(gSingleton.mState)
                                + ", downloadType: "
                                + DownloadType.toString(gSingleton.mDownloadType));
                return false;
            }

            gSingleton.downloadPakage();

            return true;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return false;
    }

    static private boolean downloadLater() {

        try {
            if (gSingleton == null) {
                NmsLog.error(TAG, "downloadLater error for gSingleton is null");
                return false;
            }

            if (gSingleton.mState != State.EWaitDownloadReq
                    || gSingleton.mDownloadType != DownloadType.EPrompt) {
                NmsLog.error(
                        TAG,
                        "downloadLater error for state: " + State.toString(gSingleton.mState)
                                + ", downloadType: "
                                + DownloadType.toString(gSingleton.mDownloadType));
                return false;
            }

            gSingleton.downloadPakageLater();

            return true;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return false;
    }

    static public void tryToShowUpgradePrompt(Context context) {
        try {
            if (gSingleton == null) {
                NmsLog.trace(TAG, "tryToShowUpgradePrompt error for gSingleton is null");
                return;
            }

            if (gSingleton.mPromptDialogGuard) {
                NmsLog.trace(TAG, "tryToShowUpgradePrompt return for mPromptDialogGuard is ture");
                return;
            }

            if (gSingleton.mState != State.EWaitDownloadReq
                    || gSingleton.mDownloadType != DownloadType.EPrompt) {
                NmsLog.trace(
                        TAG,
                        "tryToShowUpgradePrompt return for state: "
                                + State.toString(gSingleton.mState) + ", downloadType: "
                                + DownloadType.toString(gSingleton.mDownloadType));
                return;
            }

            if (TextUtils.isEmpty(gSingleton.mUpGradePrompt)) {
                NmsLog.error(TAG, "tryToShowUpgradePrompt error for mUpGradePrompt is null");
                return;
            }

            NmsLog.trace(TAG, "start the prompt upgrade dialog");

            gSingleton.mPromptDialogGuard = true;

            new AlertDialog.Builder(context)
                    .setTitle(R.string.STR_NMS_PROMPT_DLG_TITLE)
                    .setMessage(gSingleton.mUpGradePrompt)
                    .setPositiveButton(R.string.STR_NMS_PROMPT_DLG_DOWNLOAD,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    dialog.cancel();
                                    startDownload();
                                    gSingleton.mPromptDialogGuard = false;
                                }
                            })
                    .setNegativeButton(R.string.STR_NMS_PROMPT_DLG_LATER,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    dialog.cancel();
                                    downloadLater();
                                    gSingleton.mPromptDialogGuard = false;
                                }
                            }).setCancelable(false).create().show();

            // cancel notification
            NotificationManager nm = (NotificationManager) NmsService.getInstance()
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            nm.cancel(R.string.STR_NMS_UPGRADE_URL);

        } catch (Exception e) {

            if (gSingleton != null) {
                gSingleton.handleError();
                gSingleton.mPromptDialogGuard = false;
            }
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    static public long getLastUpgradeTime() {
        if (gSingleton == null) {
            NmsLog.error(TAG, "getLastUpgradeTime error for gSingleton is null");
            return 0;
        }

        return gSingleton.mLastUpgradeTime;
    }
}
