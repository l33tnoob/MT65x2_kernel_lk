package com.mediatek.systemupdate;

import android.app.AlarmManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.widget.Toast;

import com.mediatek.systemupdate.Util.DeviceInfo;
import com.mediatek.xlog.Xlog;

import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.StatusLine;
import org.apache.http.client.CookieStore;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.protocol.ClientContext;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.ConnectTimeoutException;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.cookie.Cookie;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.impl.cookie.BasicClientCookie;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.apache.http.protocol.BasicHttpContext;
import org.apache.http.protocol.HttpContext;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.io.UnsupportedEncodingException;
import java.net.SocketTimeoutException;
import java.net.URLEncoder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Map;

final class HttpManager {
    private static final String TAG = "SystemUpdate/HttpManager";
    private static final String FINGER_PRINT_NAME = "ro.build.fingerprint";
    private static final String BUILD_TYPE_NAME = "ro.build.type";
    private static final String ENG_LOAD_TAG = "eng";
    private static final int SERVER_VERSION_REQUIRED = 1;
    private static HttpManager sHttpManager;
    private static SimpleDateFormat sDateFormat = new SimpleDateFormat(Util.DATE_FORMAT);

    private static final String NETTYPE_WIFI = "WIFI";
    private static final String COOKIE_TAG = "PHPRAND";

    private static final String COMMAND_FILE = "/cache/recovery/command";
    private static final String COMMAND_PART2 = "COMMANDPART2";
    private static final String OTA_PATH_IN_RECOVERY = "/sdcard/system_update/update.zip";
    private static final String SYS_OPER_INTENT = "com.mediatek.intent.systemupdate.SysOperService";
    private static final String WRITE_COMMAND_INTENT = "com.mediatek.intent.systemupdate.WriteCommandService";

    private CookieStore mCookies = null;
    // to-do: handle Intent.ACTION_MEDIA_EJECT
    // private static boolean mEjectFlag = false;

    private static final long AUTO_DL_TIME = 3 * 60 * 60 * 1000;

    private static final int HTTP_RESPONSE_SUCCESS = 1000;

    private static final int HTTP_RESPONSE_AUTHEN_ERROR = 1002;
    private static final int HTTP_RESPONSE_ILLEGAL_ACCESS = 1004;
    private static final int HTTP_RESPONSE_TOKEN_REQUIRE = 1005;
    private static final int HTTP_RESPONSE_TOKEN_INVALID = 1006;
    private static final int HTTP_RESPONSE_SN_LOST = 1008;
    private static final int HTTP_RESPONSE_VERSION_REQUIRE = 1009;
    private static final int HTTP_RESPONSE_NO_NEW_VERSION = 1010;
    private static final int HTTP_RESPONSE_DATABASE_ERROR = 1103;
    private static final int HTTP_RESPONSE_PARAM_ERROR = 1104;
    private static final int HTTP_RESPONSE_VERSION_ILLEGAL = 1105;
    private static final int HTTP_RESPONSE_VERSION_DELETE = 1106;

    private static final int HTTP_RESPONSE_NETWORK_ERROR = 1201;
    private static final int HTTP_RESPONSE_REQUEST_TOO_LONG = 1202;
    private static final int HTTP_RESPONSE_DELTA_DELETE = 1900;
    private static final int HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT = 1901;
    private static final int HTTP_DETECTED_SDCARD_ERROR = 1902;
    private static final int HTTP_DETECTED_SDCARD_INSUFFICENT = 1903;
    private static final int HTTP_FILE_NOT_EXIST = 1904;
    private static final int HTTP_UNKNOWN_ERROR = 2000;
    private static final int HTTP_SERVER_VERSION_ERROR = 2001;
    private static final int HTTP_RESPONSE_UNZIP_ERROR = 2002;
    private static final int HTTP_RESPONSE_UNZIP_CKSUM = 2003;

    private static final int PORT_NUMBER = 443;
    private static final int TIME_OUT = 30000;
    private static final int FAULT_TOLERANT_BUFFER = 1024;
    private static final int BUFFER_SIZE = 1024;

    private static final int NETWORK_ERROR_TOAST = 0;
    private static final int SERVER_VERSION_ERROR_TOAST = 1;

    private boolean mIsDownloading = false;

    private Handler mHandler;
    private Handler mToastHandler;
    private Context mContext = null;
    private DownloadInfo mDownloadInfo = null;
    private int mErrorCode = HTTP_RESPONSE_SUCCESS;
    private NotifyManager mNotification;
    private HttpParams mHttpParam;
    private ClientConnectionManager mHttpConnMgr;

    static HttpManager getInstance(Context context) {
        Xlog.i(TAG, "sHttpManager = " + sHttpManager);
        if (sHttpManager == null) {
            sHttpManager = new HttpManager(context);
        }
        return sHttpManager;

    }

    private HttpManager(Context context) {
        mContext = context;
        mDownloadInfo = DownloadInfo.getInstance(mContext);
        mNotification = new NotifyManager(mContext);
        initHttpParam();
        initHttpClientMgr();
        new Thread() {
            public void run() {
                Xlog.v(TAG, "thread run " + Thread.currentThread().getName());
                Looper.prepare();
                mToastHandler = new Handler(Looper.myLooper()) {

                    @Override
                    public void handleMessage(Message msg) {
                        switch (msg.what) {
                        case NETWORK_ERROR_TOAST:
                            Toast.makeText(mContext, R.string.network_error, Toast.LENGTH_SHORT).show();
                            break;
                        case SERVER_VERSION_ERROR_TOAST:
                            Toast.makeText(mContext, R.string.server_version_error, Toast.LENGTH_LONG).show();
                            break;
                        default:
                            break;
                        }
                    }
                };
                Looper.loop();
            }
        }.start();
    }

    void setMessageHandler(Handler handler) {

        mHandler = handler;

        Xlog.i(TAG, "setMessageHandler, mHandler = " + mHandler);
    }

    void resetMessageHandler(Handler handler) {
        Xlog.i(TAG, "resetMessageHandler");
        if (mHandler == handler) {
            mHandler = null;
            Xlog.i(TAG, "resetMessageHandler");

        }

    }

    void queryNewVersion() {
        mNotification.clearNotification(NotifyManager.NOTIFY_NEW_VERSION);

        if (!Util.isNetWorkAvailable(mContext, "")) {
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
            mDownloadInfo.setIfNeedRefresh(true);
            sendErrorMessage();
            return;
        }

        resetDescriptionInfo();

        mDownloadInfo.setQueryDate(sDateFormat.format(new Date()));

        if (!handsakeAuthentication()) {
            mDownloadInfo.setIfNeedRefresh(true);
            sendErrorMessage();
            return;

        }

        if (!checkNewVersion()) {
            mDownloadInfo.setIfNeedRefresh(true);
            sendErrorMessage();
            return;
        }

        Xlog.i(TAG, "onQueryNewVersion, hasNewVersion");
        notifyNewVersion();
    }

    void notifyNewVersion() {

        if (Util.getUpdateType() == Util.UPDATE_TYPES.OTA_UPDATE_ONLY) {
            mDownloadInfo.setIfNeedRefresh(false);
            mDownloadInfo.setIfNeedRefreshMenu(true);

            Util.setAlarm(mContext, AlarmManager.RTC_WAKEUP, Calendar.getInstance().getTimeInMillis() + Util.REFRESHTIME,
                    Util.Action.ACTION_REFRESH_TIME_OUT);

            if (mHandler != null) {
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_NOTIFY_QUERY_DONE));
                Xlog.i(TAG, "onQueryNewVersion, Send new version founded message.");
            } else {
                mNotification.showNewVersionNotification();
            }

        }

    }

    void resetDescriptionInfo() {
        Xlog.i(TAG, "resetDescriptionInfo");

        resetDownloadFile();

        mDownloadInfo.resetDownloadInfo();

    }

    void resetDownloadFile() {
        if (doBindService(mContext)) {

            notifySysoper(MSG_DELETE_COMMANDFILE);
        }

        Util.deleteFile(Util.getPackagePathName(mContext));
    }

    private boolean handsakeAuthentication() {

        String url = ServerAddrReader.getInstance().getLoginAddress();
        if (url == null) {
            url = mContext.getResources().getString(R.string.address_login);
        }
        mCookies = null;
        DeviceInfo deviceInfo = Util.getDeviceInfo(mContext);
        Xlog.i(TAG, "onHandsakeAuthentication, imei = " + deviceInfo.mImei + ", sn = " + deviceInfo.mSnNumber + ", sim = "
                + deviceInfo.mSim + ", operator = " + deviceInfo.mOperator);
        BasicNameValuePair imei = new BasicNameValuePair("imei", deviceInfo.mImei);
        // BasicNameValuePair sn = new BasicNameValuePair("sn", deviceInfo.sn);
        BasicNameValuePair sim = new BasicNameValuePair("sim", deviceInfo.mSim);
        BasicNameValuePair operator = new BasicNameValuePair("operator", deviceInfo.mOperator);
        BasicNameValuePair sn = new BasicNameValuePair("sn", "15811375356");
        ArrayList<BasicNameValuePair> bnvpa = new ArrayList<BasicNameValuePair>();
        bnvpa.add(imei);
        bnvpa.add(sn);
        bnvpa.add(sim);
        bnvpa.add(operator);

        HttpResponse response = doPost(url, null, bnvpa);
        if (response == null) {
            mErrorCode = HTTP_UNKNOWN_ERROR;
            Xlog.i(TAG, "onHandsakeAuthentication, response = null");
            return false;
        }
        StatusLine status = response.getStatusLine();
        if (status.getStatusCode() != HttpStatus.SC_OK) {
            Xlog.i(TAG, "onHandsakeAuthentication, ReasonPhrase = " + status.getReasonPhrase());
            mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
            return false;
        }

        try {
            String content = getChunkedContent(response.getEntity());
            Xlog.i(TAG, "onHandsakeAuthentication, response content = " + content);
            HttpResponseContent res = parseAuthenInfo(content);
            if (res == null) {

                return false;
            }
            BasicClientCookie cookie = new BasicClientCookie("PHPRAND", String.valueOf(res.mRand));
            if (mCookies == null) {
                Xlog.e(TAG, "onHandsakeAuthentication: mCookies = null");
                mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
                return false;
            }
            mCookies.addCookie(cookie);
            Xlog.i(TAG, "mCookies size = " + mCookies.getCookies().size());
            Xlog.i(TAG, "onHandsakeAuthentication, rand = " + res.mRand + ", sessionId = " + res.mSessionId);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    private boolean checkNewVersion() {
        Xlog.i(TAG, "checkNewVersion");
        String url = ServerAddrReader.getInstance().getCheckVersionAddress();
        if (url == null) {
            url = mContext.getResources().getString(R.string.address_check_version);
        }
        try {
            String tokenCode = getToken();
            if (tokenCode == null) {
                mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
                return false;
            }
            BasicNameValuePair token = new BasicNameValuePair("token", URLEncoder.encode(tokenCode));
            BasicNameValuePair version = new BasicNameValuePair("version", Util.getDeviceVersionInfo(mContext));
            ArrayList<BasicNameValuePair> bnvpa = new ArrayList<BasicNameValuePair>();
            bnvpa.add(version);
            bnvpa.add(token);
            HttpResponse response = doPost(url, null, bnvpa);
            if (response == null) {
                Xlog.i(TAG, "onCheckNewVersion: response = null");
                mErrorCode = HTTP_UNKNOWN_ERROR;
                return false;
            }
            StatusLine status = response.getStatusLine();
            if (status.getStatusCode() != HttpStatus.SC_OK) {
                Xlog.i(TAG, "onCheckNewVersion, ReasonPhrase = " + status.getReasonPhrase());
                mCookies = null;
                mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
                return false;
            }
            String content = getChunkedContent(response.getEntity());
            Xlog.i(TAG, "onCheckNewVersion, response content = " + content);
            HttpResponseContent res = parseCheckVersionInfo(content);

            if (res == null) {
                return false;
            }
            if (res.mFileSize <= 0 || res.mPkgId < 0) {
                mErrorCode = HTTP_RESPONSE_NO_NEW_VERSION;
                Xlog.i(TAG, "onCheckNewVersion, fileSize = " + res.mFileSize + ", deltaId = " + res.mPkgId);
                return false;
            }
            
            if ((!res.mIsFullPkg) && (!checkFingerPrint(res.mFingerprint))) {
                mErrorCode = HTTP_RESPONSE_NO_NEW_VERSION;
                return false;
            }

            mDownloadInfo.setDLSessionDeltaId(res.mPkgId);
            mDownloadInfo.setFullPkgFlag(res.mIsFullPkg);
            mDownloadInfo.setUpdateImageSize(res.mFileSize);
            mDownloadInfo.setVersionNote(res.mReleaseNote);
            mDownloadInfo.setVerNum(res.mVersionName);
            mDownloadInfo.setAndroidNum(res.mAndroidNum);
            mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_NEWVERSION_READY);

            return true;
        } catch (IOException e) {
            e.printStackTrace();
            mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
            return false;
        }
    }

    private boolean checkFingerPrint(String fingerPrintPkg) {

        if (fingerPrintPkg != null) {
            fingerPrintPkg = fingerPrintPkg.trim();
        }
        Xlog.i(TAG, "fingerPrintPkg = " + fingerPrintPkg);

        String fingerPrintLocal = SystemProperties.get(FINGER_PRINT_NAME);
        if (fingerPrintLocal != null) {
            fingerPrintLocal = fingerPrintLocal.trim();
            Xlog.i(TAG, "fingerPrintLocal = " + fingerPrintLocal);
            return fingerPrintLocal.equals(fingerPrintPkg);
        }

        return false;
    }

    private void initHttpParam() {
        mHttpParam = new BasicHttpParams();
        HttpConnectionParams.setConnectionTimeout(mHttpParam, TIME_OUT);
        HttpConnectionParams.setSoTimeout(mHttpParam, TIME_OUT);

    }

    private void initHttpClientMgr() {

        SchemeRegistry schemeRegistry = new SchemeRegistry();
        schemeRegistry.register(new Scheme("http", PlainSocketFactory.getSocketFactory(), 80));

        schemeRegistry.register(new Scheme("https", SSLSocketFactory.getSocketFactory(), 443));

        mHttpConnMgr = new ThreadSafeClientConnManager(mHttpParam, schemeRegistry);
    }

    private HttpResponse doPost(String url, Map<String, String> headers, ArrayList<BasicNameValuePair> bnvpa) {

        Xlog.i(TAG, "doPost, url = " + url + ", mCookies = " + mCookies);
        HttpContext localcontext = new BasicHttpContext();
        if (mCookies != null) {
            localcontext.setAttribute(ClientContext.COOKIE_STORE, mCookies);
        }
        HttpResponse response = null;
        try {
            HttpHost host = null;
            HttpPost httpPost = null;

            if (url.contains("https")) {
                Uri uri = Uri.parse(url);
                host = new HttpHost(uri.getHost(), PORT_NUMBER, uri.getScheme());
                httpPost = new HttpPost(uri.getPath());
            } else {
                httpPost = new HttpPost(url);
            }

            if (headers != null) {
                for (Map.Entry<String, String> entry : headers.entrySet()) {
                    httpPost.addHeader(entry.getKey(), entry.getValue());
                }
            }

            if (bnvpa != null) {
                httpPost.setEntity(new UrlEncodedFormEntity(bnvpa));
            }
            DefaultHttpClient httpClient = new DefaultHttpClient(mHttpConnMgr, mHttpParam);

            try {
                if (url.contains("https")) {
                    Xlog.i(TAG, "doPost, https");
                    response = httpClient.execute(host, httpPost);
                } else {
                    Xlog.i(TAG, "doPost, http");
                    Xlog.i(TAG, "mHttpClient =" + httpClient + "httpPost = " + httpPost + "localcontext = " + localcontext);
                    response = httpClient.execute(httpPost, localcontext);
                }
                if (mCookies == null) {
                    mCookies = httpClient.getCookieStore();
                    Xlog.i(TAG, "mCookies size = " + mCookies.getCookies().size());
                }
                return response;
            } catch (ConnectTimeoutException e) {
                e.printStackTrace();
                mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
            }

        } catch (UnsupportedEncodingException e) {

            e.printStackTrace();
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
        } catch (IOException e) {
            e.printStackTrace();
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
        }
        return response;
    }

    private String getChunkedContent(HttpEntity entity) throws IOException {

        Xlog.i(TAG, "getChunkedContent, isChunked = " + Boolean.valueOf(entity.isChunked()).toString());
        InputStream in = entity.getContent();
        ByteArrayOutputStream swapStream = new ByteArrayOutputStream();
        int rCount = 0;
        byte[] buff = new byte[BUFFER_SIZE];

        try {
            while ((rCount = in.read(buff, 0, BUFFER_SIZE)) > 0) {
                swapStream.write(buff, 0, rCount);
            }

            return new String(swapStream.toByteArray());

        } catch (IOException e) {
            e.printStackTrace();
            onShutdownConn();
            if (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_DOWNLOADING) {
                mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);
                Xlog.e(TAG, "getChunkedContent, exception to set pause state");
            }
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
            return null;
        }
    }

    private HttpResponseContent parseAuthenInfo(String result) {

        if (result == null) {
            mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
            Xlog.e(TAG, "parseAuthenInfo error: param is null");
            return null;
        }
        try {
            JSONObject jo = new JSONObject(result);
            HttpResponseContent res = new HttpResponseContent();
            if (jo.getInt("status") == HTTP_RESPONSE_SUCCESS) {

                if (jo.has("server_version") && (jo.getInt("server_version") >= SERVER_VERSION_REQUIRED)) {
                    res.mRand = jo.getInt("rand");
                    res.mSessionId = jo.getString("sessionId");
                    mErrorCode = HTTP_RESPONSE_SUCCESS;
                    return res;

                } else {

                    mErrorCode = HTTP_SERVER_VERSION_ERROR;

                    return null;
                }

            } else {
                String info = jo.getString("info");
                mErrorCode = jo.getInt("status");
                Xlog.e(TAG, "parseAuthenInfo, error info = " + info);
                return null;
            }
        } catch (JSONException ex) {
            ex.printStackTrace();
            onShutdownConn();
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
            return null;
        }
    }

    private HttpResponseContent parseCheckVersionInfo(String result) {
        Xlog.i(TAG, "parseCheckVersionInfo");
        try {
            JSONObject jo = new JSONObject(result);
            HttpResponseContent res = new HttpResponseContent();
            if (jo.getInt("status") == HTTP_RESPONSE_SUCCESS) {
                Xlog.i(TAG, "HTTP_RESPONSE_SUCCESS");
                res.mVersionName = jo.getString("name");
                Xlog.i(TAG, "res.mVersionName = " + res.mVersionName);
                res.mFileSize = jo.getLong("size");
                Xlog.i(TAG, "res.fileSize = " + res.mFileSize);
                res.mReleaseNote = jo.getString("release_notes");
                Xlog.i(TAG, "res.releaseNote = " + res.mReleaseNote);
                if (jo.has("versionId")) {
                    res.mPkgId = jo.getInt("versionId");
                    res.mIsFullPkg = true;
                    Xlog.i(TAG, "full package: res.packageId = " + res.mPkgId);
                } else if (jo.has("deltaId")) {
                    res.mPkgId = jo.getInt("deltaId");
                    res.mIsFullPkg = false;
                    Xlog.i(TAG, "delta package: res.packageId = " + res.mPkgId);
                    res.mFingerprint = jo.getString("fingerprint");
                }
                Xlog.i(TAG, "res.packageId = " + res.mPkgId + "res.isFullPkg = " + res.mIsFullPkg);
                res.mAndroidNum = "Android " + jo.getString("android_version");
                Xlog.i(TAG, "res.mAndroidNum = " + res.mAndroidNum);

                mErrorCode = HTTP_RESPONSE_SUCCESS;

                return res;
            } else {

                mErrorCode = jo.getInt("status");
                return null;
            }
        } catch (JSONException e) {
            e.printStackTrace();
            onShutdownConn();
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;
            return null;
        }
    }

    private String getRandCookieStr() {

        String strCookie = null;

        if (mCookies != null) {

            int nSize = mCookies.getCookies().size();
            Cookie cookieRand = null;

            for (int n = 0; n < nSize; n++) {
                Cookie cookie = mCookies.getCookies().get(n);
                Xlog.i(TAG, "index:" + n + "cookieRand.getName = " + cookie.getName());
                if ((cookie != null) && (COOKIE_TAG.equals(cookie.getName()))) {
                    cookieRand = cookie;
                    Xlog.i(TAG, "cookieRand = " + cookieRand);
                }
            }

            if (cookieRand != null) {
                Xlog.i(TAG, "cookieRand getName = " + cookieRand.getName());

                strCookie = "15811375356" + cookieRand.getValue();
                Xlog.i(TAG, "getToken, strCookie = " + strCookie);

            }
        }
        Xlog.i(TAG, "strCookie = " + strCookie);
        return strCookie;

    }

    private String getToken() {
        Xlog.i(TAG, "getToken");

        StringBuffer buf = new StringBuffer();

        try {

            if (mCookies == null) {
                handsakeAuthentication();
            }

            String strCookie = getRandCookieStr();

            if (strCookie == null) {
                handsakeAuthentication();
            }
            strCookie = getRandCookieStr();
            if (strCookie == null) {

                Xlog.i(TAG, "could not get cookie value");
                return null;

            }
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            md5.update(strCookie.getBytes());
            byte[] bytes = md5.digest();
            for (int i = 0; i < bytes.length; i++) {
                String s = Integer.toHexString(bytes[i] & UpgradePkgManager.MD5_MASK);
                if (s.length() == 1) {
                    buf.append("0");
                }
                buf.append(s);
            }
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
        return buf.toString();
    }

    /*
     * void setEjectFlag(Boolean flag) { synchronized (this) { mEjectFlag =
     * flag; }
     * 
     * 
     * }
     */
    int writeFile(HttpResponse response, long currSize) {
        Xlog.i(TAG, "writeFile");

        if (mDownloadInfo.getDLSessionStatus() != DownloadInfo.STATE_QUERYNEWVERSION) {
            mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);
            mNotification.showDownloadingNotificaton(mDownloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                    .getPackageFileName(mContext)) / (double) mDownloadInfo.getUpdateImageSize()) * 100), true);
        }

        Util.cancelAlarm(mContext, Util.Action.ACTION_AUTO_DL_TIME_OUT);
        mDownloadInfo.setOtaAutoDlStatus(false);
        mDownloadInfo.setIfPauseWithinTime(false);

        try {
            // response.getParams().setIntParameter(CoreConnectionPNames.CONNECTION_TIMEOUT,
            // 10000);
            InputStream in = response.getEntity().getContent();
            File ifolder = new File(Util.getPackagePathName(mContext));
            if (!ifolder.exists()) {
                ifolder.mkdirs();
            }
            RandomAccessFile out = null;

            String pkgFile = Util.getPackageFileName(mContext);
            try {
                out = new RandomAccessFile(pkgFile, "rws");
                out.seek(currSize);
            } catch (IOException e) {
                e.printStackTrace();
                onShutdownConn();
                mErrorCode = HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT;
                return mErrorCode;
            }
            byte[] buff = new byte[4096];
            int rc = 0;
            int i = 0;
            int j = 0;
            boolean rightnow = false;
            boolean finish = false;
            File fPkg = new File(pkgFile);

            if (fPkg == null) {
                out.close();
                mErrorCode = HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT;
                return mErrorCode;
            }
            while ((rc = in.read(buff, 0, 4096)) > 0) {
                // to-do: handle Intent.ACTION_MEDIA_EJECT
                /*
                 * synchronized (this) { if (mEjectFlag) { try { out.close(); }
                 * catch (IOException e) { e.printStackTrace(); }
                 * onShutdownConn(); return mErrorCode =
                 * HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT; } }
                 */

                try {
                    if (fPkg.exists()) {
                        out.write(buff, 0, rc);
                    } else {
                        Xlog.e(TAG, "file not exist during downloading ");
                        setPauseState();
                        out.close();
                        onShutdownConn();
                        mErrorCode = HTTP_FILE_NOT_EXIST;
                        sendErrorMessage();
                        return mErrorCode;
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                    out.close();
                    onShutdownConn();
                    mErrorCode = HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT;
                    return mErrorCode;
                }
                i++;
                int status = mDownloadInfo.getDLSessionStatus();
                if (status == DownloadInfo.STATE_PAUSEDOWNLOAD || status == DownloadInfo.STATE_QUERYNEWVERSION) {
                    Xlog.i(TAG, "writeFile, DownloadInfo = " + status);
                    mCookies = null;
                    finish = false;
                    out.close();
                    onShutdownConn();
                    return 0;

                }
                if (mHandler == null) {
                    if (rightnow) {
                        i = 200;
                        rightnow = false;
                    }
                    if (i == 200) {
                        onDownloadProcessUpdate();
                        i = 0;
                    }
                } else {
                    if (!rightnow) {
                        i = 18;
                        rightnow = true;
                    }
                    if (i == 20) {
                        i = 0;
                        onDownloadProcessUpdate();
                    }
                }
                j++;
                if (j == 20) {
                    onTransferRatio();
                    j = 0;
                }
                finish = true;
            }
            Xlog.i(TAG, "writeFile, finish, rc = " + rc + "bytes" + ". finish = " + finish);
            if (finish) {
                onTransferRatio();
                onDownloadProcessUpdate();
            }

            long curSize = Util.getFileSize(Util.getPackageFileName(mContext));
            Xlog.i(TAG, "curSize = " + curSize + " mNewVersionInfo.mSize = " + mDownloadInfo.getUpdateImageSize());

            out.close();

            if (curSize >= mDownloadInfo.getUpdateImageSize()) {

                onShutdownConn();
                return 0;
            }

        } catch (SocketTimeoutException e) {

            e.printStackTrace();
        } catch (IOException e) {

            e.printStackTrace();
        }

        showNoNetworkToast();

        if (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_DOWNLOADING) {
            setPauseState();
            Xlog.e(TAG, "writeFile, exception to set pause state");
            mDownloadInfo.setOtaAutoDlStatus(true);
            mDownloadInfo.setIfPauseWithinTime(true);
            Util.setAlarm(mContext, AlarmManager.RTC, Calendar.getInstance().getTimeInMillis() + AUTO_DL_TIME,
                    Util.Action.ACTION_AUTO_DL_TIME_OUT);

        }

        onShutdownConn();

        mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;

        sendErrorMessage();
        return mErrorCode;
    }

    private void onTransferRatio() {

        long totalSize = mDownloadInfo.getUpdateImageSize();
        long currSize = Util.getFileSize(Util.getPackageFileName(mContext));
        if (currSize < 0) {
            currSize = 0;
        }
        if (totalSize == 0) {
            totalSize = -1;
        }
        if (totalSize < 0) {
            return;
        }
        int ratio = (int) (((double) currSize / (double) totalSize) * 100);
        if (ratio > Util.MAX_PERCENT) {
            ratio = Util.MAX_PERCENT;
            currSize = totalSize;
        }

        mDownloadInfo.setDownLoadPercent(ratio);
    }

    void onDownloadProcessUpdate() {

        mNotification.showDownloadingNotificaton(mDownloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                .getPackageFileName(mContext)) / (double) mDownloadInfo.getUpdateImageSize()) * 100), true);

        if (mHandler != null) {

            mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_DLPKGUPGRADE));

        }
    }

    private class HttpResponseContent {
        int mRand = -1;
        String mSessionId = null;
        String mAndroidNum = null;
        String mVersionName = null;
        long mFileSize = -1;
        String mReleaseNote = null;
        int mPkgId = -1;
        boolean mIsFullPkg = false;
        String mFingerprint = null;
    }

    private void sendErrorMessage() {
        Xlog.i(TAG, "sendErrorMessage, mErrorCode = " + mErrorCode);
        if ((mErrorCode == HTTP_RESPONSE_NETWORK_ERROR) || (mErrorCode == HTTP_UNKNOWN_ERROR)
                || (mErrorCode == HTTP_RESPONSE_AUTHEN_ERROR)) {
            showNoNetworkToast();

        } else if ((mErrorCode == HTTP_SERVER_VERSION_ERROR)
                && (ENG_LOAD_TAG.equalsIgnoreCase(SystemProperties.get(BUILD_TYPE_NAME)))) {
            showServerVersionErrorToast();
        }

        if (mHandler != null) {

            switch (mErrorCode) {

            case HTTP_SERVER_VERSION_ERROR:
            case HTTP_RESPONSE_NO_NEW_VERSION:
            case HTTP_RESPONSE_VERSION_REQUIRE:
            case HTTP_UNKNOWN_ERROR:
            case HTTP_RESPONSE_AUTHEN_ERROR:
            case HTTP_RESPONSE_VERSION_ILLEGAL:
            case HTTP_RESPONSE_VERSION_DELETE:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_NOTIFY_QUERY_DONE));
                break;

            case HTTP_RESPONSE_NETWORK_ERROR:
                if (mDownloadInfo.getDLSessionStatus() != DownloadInfo.STATE_QUERYNEWVERSION) {
                    mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_NETWORKERROR));
                } else {
                    mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_NOTIFY_QUERY_DONE));
                }
                break;

            case HTTP_RESPONSE_REQUEST_TOO_LONG:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_LARGEPKG));

                break;

            case HTTP_RESPONSE_DELTA_DELETE:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_DELTADELETED));
                break;

            case HTTP_DETECTED_SDCARD_ERROR:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_SDCARDUNKNOWNERROR));
                break;

            case HTTP_DETECTED_SDCARD_INSUFFICENT:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_SDCARDINSUFFICENT));
                break;

            case HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT:

                mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);

                // mHandler.sendMessage(mHandler
                // .obtainMessage(SystemUpdateService.MSG_SDCARDCRASHORUNMOUNT));
                break;

            case HTTP_RESPONSE_UNZIP_ERROR:
            case HTTP_RESPONSE_UNZIP_CKSUM:
                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_UNZIP_ERROR));
                break;

            case HTTP_FILE_NOT_EXIST:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_FILE_NOT_EXIST));
                break;
            default:

                mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_UNKNOWERROR));

            }
        }
    }

    void clearNotification(int type) {
        mNotification.clearNotification(type);
    }

    void onDownloadImage() {
        Xlog.i(TAG, "onDownloadImage");

        mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);
        mNotification.showDownloadingNotificaton(mDownloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                .getPackageFileName(mContext)) / (double) mDownloadInfo.getUpdateImageSize()) * 100), true);

        if (mIsDownloading) {
            return;
        }
        mIsDownloading = true;
        notifyDlStarted();

        boolean isunzip = mDownloadInfo.getDLSessionUnzipState();
        boolean isren = mDownloadInfo.getDLSessionRenameState();
        if (isren && isunzip) {

            setNotDownload();
            UpgradePkgManager.deleteCrashPkgFile(Util.getPackagePathName(mContext));
            onDownloadPackageUnzipAndCheck();

            return;
        }
        mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_DOWNLOADING);
        String strNetWorkType = mDownloadInfo.getIfWifiDLOnly() ? NETTYPE_WIFI : "";

        if (!Util.isNetWorkAvailable(mContext, strNetWorkType)) {
            mErrorCode = HTTP_RESPONSE_NETWORK_ERROR;

            sendErrorMessage();
            setPauseState();
            setNotDownload();

            return;
        }

        // to-do: handle Intent.ACTION_MEDIA_EJECT
        // setEjectFlag(false);

        String url = ServerAddrReader.getInstance().getDownloadDeltaAddress();
        if (url == null) {
            url = mContext.getResources().getString(R.string.address_download_delta);
        }
        String tokenCode = getToken();
        if (tokenCode == null) {
            mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
            sendErrorMessage();
            setPauseState();
            setNotDownload();

            return;
        }

        BasicNameValuePair token = new BasicNameValuePair("token", tokenCode);

        int packageid = mDownloadInfo.getDLSessionDeltaId();
        BasicNameValuePair deltaId = new BasicNameValuePair(mDownloadInfo.getFullPkgFlag() ? "versionId" : "deltaId",
                String.valueOf(packageid));
        Xlog.i(TAG, "onDownloadImage pkgid = " + packageid);

        ArrayList<BasicNameValuePair> bnvpa = new ArrayList<BasicNameValuePair>();
        bnvpa.add(token);
        bnvpa.add(deltaId);
        long currentSize = Util.getFileSize(Util.getPackageFileName(mContext));
        currentSize -= FAULT_TOLERANT_BUFFER;
        if (currentSize < 0) {
            currentSize = 0;
        }
        BasicNameValuePair sizePar = new BasicNameValuePair("HTTP_RANGE", String.valueOf(currentSize));
        bnvpa.add(sizePar);

        if (mDownloadInfo.getFullPkgFlag()) {
            BasicNameValuePair version = new BasicNameValuePair("version", mDownloadInfo.getVerNum());
            bnvpa.add(version);
            url = ServerAddrReader.getInstance().getDownloadFullAddress();
            if (url == null) {
                url = mContext.getResources().getString(R.string.address_download_full);
            }
            Xlog.v(TAG, "download full url = " + url);
        }
        HttpResponse response = doPost(url, null, bnvpa);

        if (mDownloadInfo.getDLSessionStatus() != DownloadInfo.STATE_DOWNLOADING) {
            Xlog.i(TAG, "onDownloadImage: status not right");
            setNotDownload();
            return;
        }
        if (response == null) {
            Xlog.i(TAG, "onDownloadImage: response = null");
            mErrorCode = HTTP_UNKNOWN_ERROR;

            sendErrorMessage();
            setPauseState();
            setNotDownload();

            return;
        }
        StatusLine status = response.getStatusLine();
        if (status.getStatusCode() != HttpStatus.SC_OK && status.getStatusCode() != HttpStatus.SC_PARTIAL_CONTENT) {
            Xlog.i(TAG, "onDownloadImage, ReasonPhrase = " + status.getReasonPhrase() + ", status.getStatusCode() = "
                    + status.getStatusCode());
            mCookies = null;

            if (status.getStatusCode() == HttpStatus.SC_NOT_FOUND) {
                resetDescriptionInfo();
                mErrorCode = HTTP_RESPONSE_DELTA_DELETE;
            } else if (status.getStatusCode() == HttpStatus.SC_REQUEST_TOO_LONG) {
                mErrorCode = HTTP_RESPONSE_REQUEST_TOO_LONG;
            } else {
                mErrorCode = HTTP_RESPONSE_AUTHEN_ERROR;
            }
            sendErrorMessage();
            setPauseState();
            setNotDownload();

            return;
        }
        // Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
        Intent service = new Intent(mContext, SystemUpdateService.class);
        service.setAction(Util.Action.ACTION_LCA_PROTECT);
        mContext.startService(service);
        int ret = writeFile(response, currentSize);
        mContext.stopService(service);
        // Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
        Xlog.i(TAG, "onDownloadImage, download result = " + ret);

        if (ret == 0) {
            int downloadStatus = mDownloadInfo.getDLSessionStatus();

            if (downloadStatus == DownloadInfo.STATE_PAUSEDOWNLOAD || downloadStatus == DownloadInfo.STATE_QUERYNEWVERSION) {
                setNotDownload();
                return;
            }

        }
        if (ret == HTTP_DETECTED_SDCARD_CRASH_OR_UNMOUNT) {
            // resetDescriptionInfo();
            resetDownloadFile();
            sendErrorMessage();
            setNotDownload();
            return;
        }
        if (ret == HTTP_RESPONSE_NETWORK_ERROR) {

            setNotDownload();

            checkIfAutoDl();

            return;
        }

        if (ret == HTTP_FILE_NOT_EXIST) {
            setNotDownload();
            return;
        }
        onDownloadPackageUnzipAndCheck();
        mIsDownloading = false;
    }

    private void checkIfAutoDl() {
        Xlog.i(TAG, "checkIfAutoDl again ");
        Util.NETWORK_STATUS networkStatus = Util.getNetworkType(mContext);

        Xlog.i(TAG, "networkStatus = " + networkStatus);
        if (networkStatus == Util.NETWORK_STATUS.STATE_NONE_NETWORK) {
            return;
        }

        int status = mDownloadInfo.getDLSessionStatus();
        Xlog.i(TAG, "status = " + status);

        if ((status == DownloadInfo.STATE_PAUSEDOWNLOAD) && (mDownloadInfo.getOtaAutoDlStatus())) {

            if ((networkStatus == Util.NETWORK_STATUS.STATE_WIFI) && mDownloadInfo.getIfPauseWithinTime()) {
                onDownloadImage();
                return;

            } else {
                Xlog.i(TAG, "showDlReminderNotification");
                mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);
                mNotification.showDownloadingNotificaton(mDownloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                        .getPackageFileName(mContext)) / (double) mDownloadInfo.getUpdateImageSize()) * 100), false);

                return;

            }
        }

        return;
    }

    private void onShutdownConn() {
        Xlog.i(TAG, "onShutdownConn");
        /*
         * if (mHttpClient != null) {
         * mHttpClient.getConnectionManager().shutdown(); } mHttpClient = null;
         */
        mCookies = null;
    }

    private void onDownloadPackageUnzipAndCheck() {

        onPackageUnzipping();
        // onDownloadPause();
        mDownloadInfo.setDLSessionUnzipState(true);
        if (!mDownloadInfo.getDLSessionRenameState()) {

            if (!UpgradePkgManager.renameOtaPkg(mContext)) {
                mErrorCode = HTTP_RESPONSE_UNZIP_ERROR;
                sendErrorMessage();
                return;
            }
            mDownloadInfo.setDLSessionRenameState(true);
        }

        long unzipSize = UpgradePkgManager.getSpaceForUnzipOtaPkg(mContext);

        Util.SDCARD_STATUS sdstat = Util.checkSdcardState(mContext, unzipSize);
        switch (sdstat) {
        case STATE_INSUFFICIENT:
            mErrorCode = HTTP_DETECTED_SDCARD_INSUFFICENT;
            sendErrorMessage();
            return;
        case STATE_LOST:
        case STATE_UNMOUNT:
            mErrorCode = HTTP_DETECTED_SDCARD_ERROR;
            sendErrorMessage();
            return;
        default:
            break;
        }

        int result = UpgradePkgManager.unzipUpgradePkg(UpgradePkgManager.getTempOtaPackage(mContext), null);
        if (result == UpgradePkgManager.UNZIP_SUCCESS) {
            onDownloadComplete();
            mDownloadInfo.setDLSessionUnzipState(false);
            mDownloadInfo.setDLSessionRenameState(false);
            UpgradePkgManager.deleteUnusedOtaFile(Util.getPackagePathName(mContext));
            return;
        } else {
            mErrorCode = (result == UpgradePkgManager.CKSUM_ERROR) ? HTTP_RESPONSE_UNZIP_CKSUM : HTTP_RESPONSE_UNZIP_ERROR;
            sendErrorMessage();
            return;
        }
    }

    void onPackageUnzipping() {

        mDownloadInfo.setDLSessionStatus(mDownloadInfo.STATE_PACKAGEUNZIPPING);
        if (mHandler != null) {
            mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_UNZIP_LODING));

        }
    }

    void onDownloadComplete() {
        Xlog.i(TAG, "onDownloadComplete");

        mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_DLPKGCOMPLETE);

        mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);
        mNotification.showDownloadCompletedNotification();

        if (!Util.checkIfTopActivity(mContext, OtaPkgManagerActivity.CLASS_NAME)) {

            Intent reminder = new Intent(mContext, ForegroundDialogService.class);
            reminder.putExtra(ForegroundDialogService.DLG_ID, ForegroundDialogService.DIALOG_INSTALL_REMINDER);

            mContext.startService(reminder);
        }

        if (mHandler != null) {

            mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_DLPKGCOMPLETE));
        }

        // mNewVersionInfo.version = null;
        mDownloadInfo.setDLSessionDeltaId(-1);
        // mDownloadInfo.setVersionNote(null);
        mDownloadInfo.setUpdateImageSize(-1);

        mDownloadInfo.setDownLoadPercent(Util.MAX_PERCENT);
    }

    void setPauseState() {
        if (mDownloadInfo.getDLSessionStatus() == DownloadInfo.STATE_DOWNLOADING) {
            mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_PAUSEDOWNLOAD);
        }

    }

    void setDownloadState() {
        mDownloadInfo.setDLSessionStatus(DownloadInfo.STATE_DOWNLOADING);

    }

    void setNotDownload() {
        mIsDownloading = false;
        if (mNotification != null) {

            int dlStatus = mDownloadInfo.getDLSessionStatus();

            if ((dlStatus == DownloadInfo.STATE_DOWNLOADING) || (dlStatus == DownloadInfo.STATE_PAUSEDOWNLOAD)) {
                mNotification.clearNotification(NotifyManager.NOTIFY_DOWNLOADING);
                mNotification.showDownloadingNotificaton(mDownloadInfo.getVerNum(), (int) (((double) Util.getFileSize(Util
                        .getPackageFileName(mContext)) / (double) mDownloadInfo.getUpdateImageSize()) * 100), false);
            }

        }
    }

    /**
     * ******************system operator service start******************
     **/

    /**
     * These constant flag must be the same as that defined in
     * SysOperService.java, please follow them.
     **/
    private static final int MSG_NONE = 0;
    private static final int MSG_DELETE_COMMANDFILE = 1;
    private static final String CMD_FILE_KEY = "COMMANDFILE";

    private final Messenger mMessenger = new Messenger(new IncomingHandler());

    private Messenger mService = null;
    private boolean mIsBound = false;
    private int mNeedServiceDo = MSG_NONE;

    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Xlog.i(TAG, "handleMessage, msg.what=" + msg.what);
            switch (msg.what) {

            case MSG_DELETE_COMMANDFILE:
                Xlog.i(TAG, "MSG_DELETE_COMMANDFILE: arg1=" + msg.arg1);
                break;
            default:
                super.handleMessage(msg);
            }
        }
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            Xlog.i(TAG, "onServiceConnected");
            mService = new Messenger(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            mService = null;
        }
    };

    private void notifySysoper(int actionType) {
        try {
            switch (actionType) {

            case MSG_DELETE_COMMANDFILE:
                Message msg = Message.obtain(null, MSG_DELETE_COMMANDFILE);
                msg.replyTo = mMessenger;
                Bundle data = new Bundle();
                if (data == null) {
                    return;
                }
                data.putString(CMD_FILE_KEY, COMMAND_FILE);
                msg.setData(data);
                if (mService != null) {
                    mService.send(msg);
                }
                break;
            default:
                break;
            }
        } catch (RemoteException e) {
            e.printStackTrace();

        }
    }

    private boolean doBindService(Context context) {

        if (context != null) {
            mIsBound = context.bindService(new Intent(SYS_OPER_INTENT), mConnection, Context.BIND_AUTO_CREATE);
        }
        Xlog.i(TAG, "dobindService, isbound=" + mIsBound);
        return mIsBound;
    }

    private void doUnbindService(Context context) {
        Xlog.i(TAG, "doUnbindService");
        if (mIsBound) {
            mService = null;
            if (context != null) {
                context.unbindService(mConnection);
            }
            mIsBound = false;
        }
    }

    /**
     * ******************system operator service stop******************
     **/

    private void showNoNetworkToast() {

        if (mToastHandler != null) {
            mToastHandler.sendEmptyMessage(NETWORK_ERROR_TOAST);
        }

    }

    private void showServerVersionErrorToast() {

        if (mToastHandler != null) {
            mToastHandler.sendEmptyMessage(SERVER_VERSION_ERROR_TOAST);
        }

    }

    void notifyDlStarted() {
        if (mHandler != null) {
            mHandler.sendMessage(mHandler.obtainMessage(SystemUpdateService.MSG_DL_STARTED));

        }
    }
}
