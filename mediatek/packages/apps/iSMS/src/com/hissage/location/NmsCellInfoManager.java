package com.hissage.location;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.json.JSONArray;
import org.json.JSONObject;

import android.content.Context;
import android.net.http.AndroidHttpClient;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;

import com.hissage.util.log.NmsLog;

public class NmsCellInfoManager {

    private final static String TAG = "NmsCellInfoManager";

    public final static int HANDLER_CELLINFO_CALLBACK = 10000;

    public class CellInfo {
        public int cellId;
        public int locationAreaCode;
        public int mobileCountryCode;
        public int mobileNetworkCode;
        public String radioType;

        public CellInfo() {
        }

        public CellInfo(int cellId, int locationAreaCode, int mobileCountryCode,
                int mobileNetworkCode, String radioType) {
            this.cellId = cellId;
            this.locationAreaCode = locationAreaCode;
            this.mobileCountryCode = mobileCountryCode;
            this.mobileNetworkCode = mobileNetworkCode;
            this.radioType = radioType;
        }
    }

    private Context mContext;
    private Handler mHandler;

    private CellInfo mCellInfo;

    public NmsCellInfoManager(Context context) {
        mContext = context;
    }

    public NmsCellInfoManager(Context context, Handler handler) {
        mContext = context;
        mHandler = handler;
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public void handlerCallback(int msgId, NmsLocation location) {
        if (null != mHandler) {
            Message msg = Message.obtain(mHandler, msgId, location);
            mHandler.sendMessage(msg);
        } else {
            NmsLog.warn(TAG, "mHandler is null");
        }
    }

    public int initMyLocationCell() {
        TelephonyManager telephonyManager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        int networkType = telephonyManager.getNetworkType();
        System.setProperty("java.net.preferIPv6Addresses", "false");
        NmsLog.trace(TAG, "Current the networkType(" + networkType + ").");

        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            NmsLog.warn(TAG, "NETWORK_TYPE_UNKNOWN");
            return -1;
        }

        if (telephonyManager.getCellLocation() instanceof CdmaCellLocation) {
            // TelephonyManager.NETWORK_TYPE_EVDO_A
            // TelephonyManager.NETWORK_TYPE_CDMA
            // TelephonyManager.NETWORK_TYPE_1xRTT
            try {
                CdmaCellLocation cdmaCellLocation = (CdmaCellLocation) telephonyManager
                        .getCellLocation();
                if (cdmaCellLocation == null) {
                    NmsLog.error(TAG, "cdmaCellLocation is null");
                    return -1;
                }

                mCellInfo = new CellInfo(cdmaCellLocation.getBaseStationId(),
                        cdmaCellLocation.getNetworkId(), Integer.valueOf(telephonyManager
                                .getNetworkOperator().substring(0, 3)),
                        cdmaCellLocation.getSystemId(), "cdma");
            } catch (Exception e) {
                NmsLog.error(TAG, "cdmaCellLocation is exception");
                return -1;
            }
        } else if (telephonyManager.getCellLocation() instanceof GsmCellLocation) {
            // TelephonyManager.NETWORK_TYPE_EDGE
            // TelephonyManager.NETWORK_TYPE_UMTS
            // TelephonyManager.NETWORK_TYPE_GPRS
            try {
                GsmCellLocation gsmCellLocation = (GsmCellLocation) telephonyManager
                        .getCellLocation();
                if (gsmCellLocation == null) {
                    NmsLog.error(TAG, "gsmCellLocation is null");
                    return -1;
                }

                mCellInfo = new CellInfo(gsmCellLocation.getCid(), gsmCellLocation.getLac(),
                        Integer.valueOf(telephonyManager.getNetworkOperator().substring(0, 3)),
                        Integer.valueOf(telephonyManager.getNetworkOperator().substring(3, 5)),
                        "gsm");
            } catch (Exception e) {
                NmsLog.error(TAG, "gsmCellLocation is exception");
                return -1;
            }
        } else {
            NmsLog.warn(TAG, "CellLocation conversion failed.");
            return -1;
        }

        threadGetLocation();

        return 0;
    }

    private void threadGetLocation() {

        new Thread() {
            @Override
            public void run() {
                super.run();
                handlerCallback(HANDLER_CELLINFO_CALLBACK, callGear(mCellInfo));
            }
        }.start();
    }

    private NmsLocation callGear(CellInfo cellInfo) {
        if (cellInfo == null) {
            NmsLog.error(TAG, "cellInfo is null");
            return null;
        }

        NmsLocation result = null;

        JSONObject holder = new JSONObject();
        AndroidHttpClient client = AndroidHttpClient.newInstance("Android-Mms/2.0");
        HttpParams params = client.getParams();
        HttpProtocolParams.setContentCharset(params, "UTF-8");
        try {
            JSONObject data;
            JSONArray jsonArray = new JSONArray();
            holder.put("version", "1.1.0");
            holder.put("host", "maps.google.com");
            holder.put("request_address", true);
            holder.put("radio_type", cellInfo.radioType);
            // holder.put("address_language",
            // getLanguageCode(cellInfo.mobileCountryCode));
            data = new JSONObject();
            data.put("cell_id", cellInfo.cellId);
            data.put("location_area_code", cellInfo.locationAreaCode);
            data.put("mobile_country_code", cellInfo.mobileCountryCode);
            data.put("mobile_network_code", cellInfo.mobileNetworkCode);
            // data.put("age", 0);
            jsonArray.put(data);
            holder.put("cell_towers", jsonArray);

            HttpPost post = new HttpPost("http://www.google.com/loc/json");
            StringEntity se = new StringEntity(holder.toString());
            NmsLog.error(TAG, "Location send: " + holder.toString());
            post.setEntity(se);
            HttpResponse resp = client.execute(post);
            HttpEntity entity = resp.getEntity();

            BufferedReader br = new BufferedReader(new InputStreamReader(entity.getContent()));
            StringBuffer sb = new StringBuffer();
            String res = null;
            while ((res = br.readLine()) != null) {
                NmsLog.error(TAG, "Locaiton receive: " + res);
                sb.append(res);
            }
            if (sb.length() <= 1)
                return null;
            data = new JSONObject(sb.toString());
            data = (JSONObject) data.get("location");

            result = new NmsLocation((Double) data.get("latitude"), (Double) data.get("longitude"));
        } catch (Exception e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        } finally {
            client.close();
        }

        return result;
    }
}
