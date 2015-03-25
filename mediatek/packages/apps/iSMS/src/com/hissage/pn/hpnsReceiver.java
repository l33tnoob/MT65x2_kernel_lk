package com.hissage.pn;

import java.io.IOException;
import java.util.List;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.text.TextUtils;

import com.google.android.gcm.GCMRegistrar;
import com.hissage.GCMIntentService;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.hpe.SDK;
import com.hissage.jni.engineadapter;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.service.NmsService;
import com.hissage.timer.NmsTimer;
import com.hissage.util.log.NmsLog;

public class hpnsReceiver extends BroadcastReceiver {
    public static final String action_registration = "com.hpns.android.intent.REGISTRATION";
    public static final String action_receive = "com.hpns.android.intent.RECEIVE";
    public static final String action_unregister = "com.hpns.android.intent.UNREGISTER";
    public static final String action_reconnect = "com.hpns.android.intent.RECONNECT";
    public static final String action_gcmError = "com.hpns.android.intent.GCMERROR";

    public static final String logTag = "HPESDK";

    public static int appId = 805832216;
    public static String senderId = "4bcd68c659956190";
    // public static String ACCOUNT_URI_HESINE =
    // "http://118.26.192.200/hpns/demo/asdemo/posttoken.php";
    public static final int HPNS_CODE_SUCCESS = 0;

    public static SharedPreferences sharedPrefs = null;
    public static final String HPNS_PREFS_NAME = "v1PNregid";

    public static final String NMS_PN_TYPE_HPNS = "HPNS";
    public static final String NMS_PN_TYPE_GCM = "GCM";

    private static String pnType = null;

    private static int trypHpns = 0;
    private static int trypGcm = 0;

    private static final int NMS_MAX_HPNS_TRY_TIME = 10;
    private static final int NMS_MAX_GCM_TRY_TIME = 5;

    private static final int ISMS_REGISTER_DEFAULT_TIME = 60; // 60 s
    private static long ISMS_REGISTER_TIME = ISMS_REGISTER_DEFAULT_TIME;

    public static boolean isPNReged = false;
    private static boolean isPNLoaded = false ;
    
    private static Object getAppHpnsValue(Context context, String key) {
        if (context == null) {
            NmsLog.trace(logTag, "getAppHpnsValue | context is null!");
            return null;
        }

        try {
            ApplicationInfo applicationInfo = context.getPackageManager().getApplicationInfo(
                    context.getPackageName(), PackageManager.GET_META_DATA);
            if (applicationInfo == null) {
                NmsLog.trace(logTag, "getAppHpnsValue | applicationInfo is null!");
                return null;
            }
            Bundle bundle = applicationInfo.metaData;
            if (bundle == null) {
                NmsLog.trace(logTag, "getAppHpnsValue | bundle is null!");
                return null;
            }

            return bundle.get(key);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static int getAppId(Context context) {
        if (context == null) {
            NmsLog.trace(logTag, "getAppId | context is null!");
            return -1;
        }

        int result = (Integer) getAppHpnsValue(context, "HPNS_APP_ID");
        if (result <= 0) {
            NmsLog.trace(logTag, "getAppId | result is empty!");
            return -1;
        }
        NmsLog.trace(logTag, "getAppId | result=" + result);
        return result;
    }

    public static String getAccountId(Context context) {
        if (context == null) {
            NmsLog.trace(logTag, "getAccountId | context is null!");
            return "";
        }

        String result = (String) getAppHpnsValue(context, "HPNS_ACCOUNT_ID");
        if (TextUtils.isEmpty(result)) {
            NmsLog.trace(logTag, "getAccountId | result is empty!");
            return "";
        }
        NmsLog.trace(logTag, "getAccountId | result=" + result);
        return result;
    }

    public void onReceive(Context context, Intent intent) {
        String recvaction = intent.getAction();
        NmsLog.trace(logTag, "receive action:" + recvaction);
        if (recvaction.equals(action_registration)) {
            handleRegistration(context, intent);
        } else if (recvaction.equals(action_unregister)) {
            handleUnRegistration(context, intent);
        } else if (recvaction.equals(action_receive)) {
            handleMessage(context, intent);
        } else if (recvaction.equals(action_reconnect)) {
            SDK.onRegister(context);
        } else if (recvaction.equals(action_gcmError)) {
            setAdviceType(NMS_PN_TYPE_HPNS);
        } else {
            NmsLog.trace(logTag, "receive unexpected action:" + recvaction);
        }

    }

    private void handleUnRegistration(Context context, Intent intent) {
        int code = intent.getIntExtra("code", 0);
        NmsLog.trace(logTag, "handleUnRegitration, code:" + code);

    }

    private boolean isNetworkActive(Context context) {

        ConnectivityManager connManager = (ConnectivityManager) context
                .getSystemService(context.CONNECTIVITY_SERVICE);

        if (null == connManager) {
            NmsLog.error(logTag, "connectivity manager is null when checking active network");
            return false;
        }

        NetworkInfo i = connManager.getActiveNetworkInfo();

        if (i == null) {
            NmsLog.error(logTag, "no active network when checking active network");
            return false;
        }

        if (!i.isConnected()) {
            NmsLog.error(logTag, "current network is not connected when checking active network");
            return false;
        }

        if (!i.isAvailable()) {
            NmsLog.error(logTag, "current network is not available when checking active network");
            return false;
        }
        return true;
    }

    private void handleRegistration(final Context context, Intent intent) {
        String registration = intent.getStringExtra("registration_id");
        int code = intent.getIntExtra("code", 0);
        NmsLog.trace(logTag, "handleRegitration, code:" + code);

        if (HPNS_CODE_SUCCESS != code) {
            NmsLog.error(logTag, "fatal error in HPNS for handleRegitration is failed, code:"
                    + code);
            if (isNetworkActive(context)) {
                int status = NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_PN_REGISTER,
                        ISMS_REGISTER_TIME);
                if (status == 0) {
                    ISMS_REGISTER_TIME = ISMS_REGISTER_TIME * 2;
                } else {
                    NmsLog.warn(logTag, "NmsSetTimer status:" + status);
                }
            } else {
                NmsLog.error(logTag, "Network Not Available");
            }
            isPNReged = false;
        } else {
            ISMS_REGISTER_TIME = ISMS_REGISTER_DEFAULT_TIME;
            if (registration != null) {
                isPNReged = true;
                String oldRegId = readRegId(context);

                // save
                saveRegId(context, registration);
                // Send the registration ID to the 3rd party site that is
                // sending
                // the messages.
                // This should be done in a separate thread.
                // When done, remember that all registration is done.
                /*
                 * List<NameValuePair> nameValuePairs = new
                 * ArrayList<NameValuePair>( 2); nameValuePairs.add(new
                 * BasicNameValuePair("registration_id", registration));
                 * nameValuePairs.add(new BasicNameValuePair("account_id",
                 * senderId)); postData(ACCOUNT_URI_HESINE, nameValuePairs);
                 * NmsLog.trace(logTag, "post token:" + registration +
                 * " to uri:" + ACCOUNT_URI_HESINE);
                 */

                if (!oldRegId.equalsIgnoreCase(registration)) {
                    NmsLog.error(logTag, String.format(
                            "receive new regid:%s from pn, old regid:%s", registration, oldRegId));
                    if (NmsConfig.mIsDBInitDone && engineadapter.get().nmsUIIsActivated())
                        engineadapter.get().nmsUIHandlePNNewRegId();
                }
            } else {
                registration = "";
            }
        }

    }

    private void handleMessage(Context context, Intent intent) {

        NmsLog.trace(logTag, "receive new message from pn");
        forceISmsReconnect();
    }

    private static void forceISmsReconnect() {

        if (NmsConfig.mIsDBInitDone && engineadapter.get().nmsUIIsActivated()) {
            engineadapter.get().nmsUIHandlePNNotification();
        } else {
            NmsLog.warn(logTag, "iSms is not activated yet, not deliver the pn message");
        }
    }

    public static void postData(String uri, List<NameValuePair> nameValuePairs) {
        // Create a new HttpClient and Post Header
        HttpClient httpclient = new DefaultHttpClient();
        HttpPost httppost = new HttpPost(uri);

        try {
            // Add your data
            httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
            // Execute HTTP Post Request
            HttpResponse response = httpclient.execute(httppost);
            NmsLog.trace(logTag, "post data to uri:" + uri);

        } catch (ClientProtocolException e) {
            // TODO Auto-generated catch block
        } catch (IOException e) {
            // TODO Auto-generated catch block
        }
    }

    public static void saveRegId(Context context, String regid) {
        try {
            if (null == sharedPrefs) {
                sharedPrefs = context.getSharedPreferences(HPNS_PREFS_NAME, Context.MODE_PRIVATE);
            }
            sharedPrefs.edit().putString(HPNS_PREFS_NAME, regid).commit();
            NmsLog.trace(logTag, "update regid:" + regid);
        } catch (Exception e) {
            NmsLog.trace(logTag, "save pendingintent exception, appid:" + appId + ", exception:"
                    + e.getMessage());
        }
    }

    public static String readRegId(Context context) {
        final int reglen = 24;
        String regid = "3132333435363738393A3B3C";
        try {
            if (null == sharedPrefs) {
                sharedPrefs = context.getSharedPreferences(HPNS_PREFS_NAME, Context.MODE_PRIVATE);
            }

            String temp = sharedPrefs.getString(HPNS_PREFS_NAME, null);
            if (null != temp/* && temp.length() == reglen */) {
                regid = temp;
            } else {
                NmsLog.error(logTag, "regid from file error:" + temp);
            }
            // return strpackagename;
        } catch (Exception e) {
            NmsLog.error(logTag,
                    "restore pendingintent, appid:" + appId + ", exception:" + e.getMessage());
        }
        return regid;
    }

    public static void setPNType(String type) {

        if (type == null) {
            NmsLog.error(logTag, "type is null in setPNType");
            return;
        }

        pnType = type;
        NmsConfig.setPnType(type);
    }

    public static String getPNType(Context context) {

        if (TextUtils.isEmpty(pnType)) {
            NmsLog.error(logTag, "pnType is " + ((pnType == null) ? "NULL" : "EMPTY")
                    + " in getPNType, may be the loadPn function is not call or got error before, but we try again");
			
			pnType = NmsConfig.getPnType();

			if (TextUtils.isEmpty(pnType)) {
				pnType = ((context == null) ? NMS_PN_TYPE_HPNS : getDefaultPnType(context)) ;
				NmsLog.error(logTag, "pnType is " + pnType
                    + " in getPNType, got error before or the isms is not activated yet, just return default pn type") ;
			}
        }

        return pnType;
    }

    public static boolean startHPNS(Context context) {

        NmsLog.trace(logTag, "try to start hpns current try time:" + trypHpns);

        if (trypHpns >= NMS_MAX_HPNS_TRY_TIME)
            return false;

        trypHpns++;

        boolean ret = false;

        if (Config.PN ){
        	SDK.startService(context);
            SDK.onRegister(context);
            ret = true;
            NmsLog.trace(logTag, "HPNS is started by NmsService") ;
        } else {
            NmsLog.warn(logTag, "HPNS is not open");
        }

        if (ret)
            setPNType(hpnsReceiver.NMS_PN_TYPE_HPNS);

        return ret;
    }

    public static void stopHPNS(Context context) {
        NmsLog.trace(logTag, "un register hpns");
        Intent unRegistrationIntent = new Intent(hpnsReceiver.action_unregister);
        PendingIntent intent = PendingIntent.getBroadcast(context, 0, new Intent(""), 0);

        unRegistrationIntent.putExtra("app", intent); // boilerplate
        unRegistrationIntent.putExtra("appId", appId);
        unRegistrationIntent.putExtra("accountId", senderId);
        context.startService(unRegistrationIntent);
    }

    private static boolean startGCM(Context context) {

        NmsLog.trace(logTag, "try to start gcm current try time:" + trypGcm);

        if (trypGcm >= NMS_MAX_GCM_TRY_TIME)
            return false;

        trypGcm++;

        boolean ret = false;

        try {
            GCMRegistrar.checkDevice(context);
            GCMRegistrar.checkManifest(context);
            final String regId = GCMRegistrar.getRegistrationId(context);
            if (regId.equals("")) {
                GCMRegistrar.register(context, GCMIntentService.SENDER_ID);
                ret = true;
            } else {
                NmsLog.warn(logTag, "GCM already registered, just save the reg id");
                Intent intent = new Intent();
                intent.putExtra("registration_id", regId);
                intent.putExtra("code", hpnsReceiver.HPNS_CODE_SUCCESS);
                intent.setAction(hpnsReceiver.action_registration);
                context.sendBroadcast(intent);
            }
        } catch (Exception e) {
            NmsLog.warn(logTag, "GCM is not open");
        }

        if (ret)
            hpnsReceiver.setPNType(hpnsReceiver.NMS_PN_TYPE_GCM);

        return ret;
    }

    private static void stopGCM(Context context) {
        try {
            if (GCMRegistrar.isRegistered(context))
                GCMRegistrar.unregister(context);
        } catch (Exception e) {
            NmsLog.warn(logTag, "GCM is not open");
        }
    }

    private static String getDefaultPnType(Context context) {
        String ret = "";
        String[] imsiList = NmsPlatformAdapter.getInstance(context).getAllImsi();
        if (imsiList != null) {
            for (int i = 0; i < imsiList.length; ++i) {
                String imsi = imsiList[i];
                if (TextUtils.isEmpty(imsi)) {
                    continue;
                }
                if (NmsCommonUtils.isChinaCard(imsi)) {
                    ret = NMS_PN_TYPE_HPNS;
                    break;
                } else {
                    ret = NMS_PN_TYPE_GCM;
                }
            }
        }

        return ret;
    }

    public static void loadPn(Context context) {
        if (context == null) {
            NmsLog.error(logTag, "loadPn: context is null");
            return;
        }
        
        if (isPNLoaded) {
            NmsLog.warn(logTag, "loadPn: pn is loaded already");
            return ;
        }
        
        isPNLoaded = true ;
        pnType = NmsConfig.getPnType();
        trypGcm = 0;
        trypHpns = 0;
        appId = getAppId(context);
        senderId = getAccountId(context);
        if (TextUtils.isEmpty(pnType)) {
            NmsLog.trace(logTag, "loadPn: pnType is " + ((pnType == null) ? "NULL" : "EMPTY")
                    + " and try to get default type");
            pnType = getDefaultPnType(context);
        }

        NmsLog.trace(logTag, "loadPn: get pn type:" + pnType);

        setPNType(pnType);

        if (TextUtils.isEmpty(pnType)) {
            isPNLoaded = false ;
            NmsLog.error(logTag, "loadPn: no sim card, pn is not start!");
            return;
        }

        boolean succeed = false;

        if (pnType.equalsIgnoreCase(NMS_PN_TYPE_HPNS)) {
            succeed = startHPNS(context);
            if (!succeed) {
                succeed = startGCM(context);
            }
        } else if (pnType.equalsIgnoreCase(NMS_PN_TYPE_GCM)) {
            succeed = startGCM(context);
            if (!succeed) {
                succeed = startHPNS(context);
            }
        }

        if (!succeed) {
            isPNLoaded = false ;
            NmsLog.error(logTag, "loadPn: " + pnType + " is not succeed");
            setPNType("");
        }
        
        NmsLog.error(logTag, "loadPn is succeed, pn: " + (pnType == null ? "null" : pnType)) ;
    }

    public static void setAdviceType(String type) {
        if (TextUtils.isEmpty(type)) {
            NmsLog.error(logTag, "setAdviceType: type is null or empty");
            return;
        }

        if (pnType == null) {
            NmsLog.error(logTag, "setAdviceType: pnType is null");
            pnType = "";
        }

        if (type.equalsIgnoreCase(pnType)) {
            NmsLog.warn(logTag, "setAdviceType: confuse for type: " + type
                    + " is match to pnType: " + pnType);
            return;
        }

        NmsLog.trace(logTag, "setAdviceType: get advice pn type:" + type + " and current pnType "
                + pnType);

        Context context = NmsService.getInstance();
        if (context == null) {
            NmsLog.error(logTag, "setAdviceType: context is null");
            return;
        }

        String prePnType = pnType;
        boolean succeed = false;

        if (type.equalsIgnoreCase(NMS_PN_TYPE_HPNS)) {
            succeed = startHPNS(context);
            if (succeed) {
                if (prePnType.equalsIgnoreCase(NMS_PN_TYPE_GCM))
                    stopGCM(context);
                forceISmsReconnect();
            }
        } else if (type.equalsIgnoreCase(NMS_PN_TYPE_GCM)) {
            succeed = startGCM(context);
            if (succeed) {
                if (prePnType.equalsIgnoreCase(NMS_PN_TYPE_HPNS))
                    stopHPNS(context);
                forceISmsReconnect();
            }
        }

        if (!succeed) {
            NmsLog.warn(logTag, "setAdviceType: try to start advice: " + pnType + " is not succeed");
            setPNType(prePnType);
        }
    }
}
