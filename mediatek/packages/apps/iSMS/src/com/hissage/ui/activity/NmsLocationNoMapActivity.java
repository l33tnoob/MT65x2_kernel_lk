package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import org.json.JSONArray;
import org.json.JSONObject;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputFilter.LengthFilter;
import android.text.Selection;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.hissage.R;
import com.baidu.location.BDLocation;
import com.baidu.location.BDLocationListener;
import com.baidu.location.LocationClient;
import com.baidu.location.LocationClientOption;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.location.NmsCellInfoManager;
import com.hissage.location.NmsLocation;
import com.hissage.location.NmsLocationFormat;
import com.hissage.location.NmsLocationManager;
import com.hissage.message.ip.NmsIpMessageConsts.NmsShareLocationDone;
import com.hissage.ui.adapter.NmsLocationListAdapter;
import com.hissage.ui.view.NmsLocationCustomView;
import com.hissage.ui.view.NmsLocationCustomView.OnLocCustomViewClickListener;
import com.hissage.util.log.NmsLog;

public class NmsLocationNoMapActivity extends Activity implements OnLocCustomViewClickListener {

    private final static String TAG = "NmsLocationNoMapActivity";

    private final static int REQUSETCODE_SYSTEM_LOCATION_SERVICES = 1000;

    private final static int DEFAULT_TIMEROUT_CURR_LOC = 15000; // 15s
    private final static int DEFAULT_TIMEROUT_WAIT_GPS = 2000; // 2s
    private final static int DEFAULT_NEARBY_PIN_COUNT = 6;
    private final static int DEFAULT_SEARCH_RADIUS = 150;
    private final static double DEFAULT_LAT_OFFSET = 0; // 0.0015;
    private final static double DEFAULT_LNG_OFFSET = 0; // 0.0065;

    private final static int LOC_LISTENER_GPS = 1;
    private final static int LOC_LISTENER_NETWORK = 2;
    private final static int LOC_LISTENER_ALL = 3;

    private final static int CONTROL_CUSTOMVIEW = 1;

    private final static int ACTION_CUSTOMVIEW_CURR_LOADING = 0;
    private final static int ACTION_CUSTOMVIEW_CURR_FAILED = 1;
    private final static int ACTION_CUSTOMVIEW_CURR_SUCCEED = 2;
    private final static int ACTION_CUSTOMVIEW_NEARBY_LOADING = 6;
    private final static int ACTION_CUSTOMVIEW_NEARBY_FAILED = 7;
    private final static int ACTION_CUSTOMVIEW_NEARBY_SUCCEED = 8;

    private final static int HANDLER_TOAST_SHOW = 0;
    private final static int HANDLER_UPDATE_UI = 3;
    private final static int HANDLER_SHARE_DATA_READY = 4;

    private static enum nmsListenerStatus {
        UNKNOWN, INVALID, RUNNING, REMOVE
    }

    private ProgressDialog mPDWait;

    private NmsLocationCustomView mCustomView;
    private ListView mListView;
    private NmsLocationListAdapter mListAdapter;
    private List<NmsLocation> mListData;

    private NmsLocation mCurrLocation;
    private NmsLocation mPinLocation;

    private Context mContext;
    private Timer mTimerCurrLoc;
    private Timer mTimerWaitForGps;
    private nmsListenerStatus mGpsListenerStatus;
    private nmsListenerStatus mNetworkListenerStatus;
    private int mPinCount;
    private int mRadius; // search radius
    private int mLastFailed; // CUSTOMVIEW_ACTION_FAILED
	private LocationListenner baiduLocationListener;
	private LocationClient locationClient = null;
    private boolean useGMS = true;
    private boolean tryAgarin = true;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            NmsLog.trace(TAG, "handler received msg type: " + msg.what);
            switch (msg.what) {
            case HANDLER_TOAST_SHOW:
                showToast(msg.arg1, msg.arg2);
                break;

            case NmsCellInfoManager.HANDLER_CELLINFO_CALLBACK:
                doCellInfoCallback((NmsLocation) msg.obj);
                break;

            case HANDLER_UPDATE_UI:
                updateUI(msg.arg1, msg.arg2);
                break;

            case HANDLER_SHARE_DATA_READY:
                shareFinished();
                break;

            default:
                NmsLog.error(TAG, "handler received msg type is UNKNOWN!");
                break;
            }
        }

    };
    private LocationManager mLocationManager;
    private LocationListener mGpsLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            NmsLog.error(TAG, "gps onLocationChanged");
            doLocationChanged(location, LOC_LISTENER_GPS);
        }

        @Override
        public void onProviderDisabled(String provider) {
            NmsLog.trace(TAG, "gps onProviderDisabled");
        }

        @Override
        public void onProviderEnabled(String provider) {
            NmsLog.trace(TAG, "gps onProviderEnabled");
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            NmsLog.trace(TAG, "gps onStatusChanged");
        }

    };
    private LocationListener mNetworkLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            NmsLog.error(TAG, "network onLocationChanged");
            doLocationChanged(location, LOC_LISTENER_NETWORK);
        }

        @Override
        public void onProviderDisabled(String provider) {
            NmsLog.trace(TAG, "network onProviderDisabled");
        }

        @Override
        public void onProviderEnabled(String provider) {
            NmsLog.trace(TAG, "network onProviderEnabled");
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            NmsLog.trace(TAG, "network onStatusChanged");
        }

    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.location_no_map);
        
        try {
            Class.forName("com.google.android.maps.MapActivity");
        }catch(Exception e){
        	NmsLog.trace(TAG, "this phone is not google maps.jar");
        	useGMS = false;
        }        
        init();

        if (mGpsListenerStatus == nmsListenerStatus.INVALID
                && mNetworkListenerStatus == nmsListenerStatus.INVALID) {
            updateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
            showSetSysLocDialog();
        } else {
            updateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_LOADING);
            if(useGMS){
                startTimerCurrLoc();
            }else{
                startTimerCurrLocForBaidu();
            }
            
        }
    }

    private void init() {
        initFiled();
        initComponents();
        initAllProvider();
    }

    private void initFiled() {
        mContext = this;

        mRadius = DEFAULT_SEARCH_RADIUS;
        mPinCount = DEFAULT_NEARBY_PIN_COUNT;

        setListenerStatus(LOC_LISTENER_ALL, nmsListenerStatus.UNKNOWN);
    }

    private void initCustomView() {
        mCustomView = (NmsLocationCustomView) findViewById(R.id.lcv_view);
        mCustomView.setOnLocCustomViewClickListener(this);

        mListView = mCustomView.getNearbyListView();
        mListData = new ArrayList<NmsLocation>();
        mListAdapter = new NmsLocationListAdapter(mContext, mListData);
        mListView.setAdapter(mListAdapter);
    }

    private void initComponents() {
        initCustomView();
    }

    private void initGpsProvider() {
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

        try {
            boolean isEnabled = mLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
            if (isEnabled) {
                mLocationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0,
                        mGpsLocationListener);
                setListenerStatus(LOC_LISTENER_GPS, nmsListenerStatus.RUNNING);
            } else {
                NmsLog.warn(TAG, "gps provider is not open.");
                setListenerStatus(LOC_LISTENER_GPS, nmsListenerStatus.INVALID);
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "initGpsProvider Exception!");
            setListenerStatus(LOC_LISTENER_GPS, nmsListenerStatus.INVALID);
        }
    }

    private void initNetworkProvider() {
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);

        try {
            boolean isEnabled = mLocationManager
                    .isProviderEnabled(LocationManager.NETWORK_PROVIDER);
            if (isEnabled) {
                mLocationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, 0,
                        mNetworkLocationListener);
                setListenerStatus(LOC_LISTENER_NETWORK, nmsListenerStatus.RUNNING);
            } else {
                NmsLog.warn(TAG, "network provider is not open.");
                setListenerStatus(LOC_LISTENER_NETWORK, nmsListenerStatus.INVALID);
            }
        } catch (Exception e) {
            NmsLog.warn(TAG, "initNetworkProvider Exception!");
            setListenerStatus(LOC_LISTENER_NETWORK, nmsListenerStatus.INVALID);
        }
    }

    private class LocationListenner implements BDLocationListener {
        public void onReceiveLocation(BDLocation location) { 
            // TODO Auto-generated method stub 
            if (location == null){
                if(tryAgarin){
                    tryAgarin = false;
                    if(locationClient != null && locationClient.isStarted()){
                        locationClient.requestLocation();  
                    }
                }else{
                    NmsLog.error(TAG, "onReceiveLocation: location is null!");
                    reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                    return;
                }
            }
            endTimerCurrLoc();
            double lat = NmsLocationManager.formatLatLng(location.getLatitude());
            double lng = NmsLocationManager.formatLatLng(location.getLongitude());
            lat += DEFAULT_LAT_OFFSET;
            lng += DEFAULT_LNG_OFFSET;      
 
            mCurrLocation = new NmsLocation(lat, lng);                   
            NmsLog.trace(TAG, "onReceiveLocation: location type:" + location.getLocType());
            if (location.getLocType() == BDLocation.TypeGpsLocation){
                int lati = (int) ((location.getLatitude()) * 1E6);
                int lont = (int) ((location.getLongitude()) * 1E6);
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED); 
				return;
            } else if (location.getLocType() == BDLocation.TypeNetWorkLocation){
                mCurrLocation.setName(location.getStreet());
                mCurrLocation.setVicinity(location.getAddrStr());
            }else if(location.getLocType() == BDLocation.TypeOffLineLocationFail
                    ||location.getLocType() == BDLocation.TypeCriteriaException
                    ||location.getLocType() == BDLocation.TypeNetWorkException
                    ||location.getLocType() == BDLocation.TypeNone
                    ||location.getLocType() == BDLocation.TypeOffLineLocationNetworkFail
                    ||location.getLocType() == BDLocation.TypeServerError){
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                return;
            }            
            mPinLocation = mCurrLocation.clone();               
            if(locationClient != null && locationClient.isStarted()){
                locationClient.requestPoi();
            }
            reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_SUCCEED);
        }
        
        public void onReceivePoi(BDLocation poiLocation) {
            if (poiLocation == null){
                NmsLog.error(TAG, "onReceivePoi: poiLocation is null!");
                return ; 
            }
            if (poiLocation.getLocType() == BDLocation.TypeNetWorkLocation){
                mCurrLocation.setName(poiLocation.getStreet());
                mCurrLocation.setVicinity(poiLocation.getAddrStr());
            } 
            if (mListData == null) {
                NmsLog.error(TAG, "onReceivePoi: mListData is null!");
                return;
            }
            mListData.clear();
            if(poiLocation.hasPoi()){
                try{
                    JSONObject jsonObject = new JSONObject(poiLocation.getPoi());
                    JSONArray results = jsonObject.getJSONArray("p");
                    for(int i = 0;i < results.length(); ++i){
                        JSONObject object = results.getJSONObject(i);
                        double lng = object.getDouble("x");
                        double lat = object.getDouble("y");
                        String name = object.getString("name");
                        String vicinity = object.getString("addr");
                        NmsLocation loc = new NmsLocation(lat, lng, name, vicinity);
                        mListData.add(loc);
                    }
                }catch(Exception e){
                    NmsLog.error(TAG, "getLocation: " + NmsLog.nmsGetStactTrace(e));
                }
            }else{ 
                NmsLog.error(TAG, "onReceivePoi: noPoi information");
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_NEARBY_FAILED);
                return;
            }
            reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_NEARBY_SUCCEED);
        }
    }
    
    private void initBaiduProvider(){
        baiduLocationListener = new LocationListenner();
        locationClient = new LocationClient(this);
        LocationClientOption option = new LocationClientOption(); 
        option.setOpenGps(true);
        option.setCoorType("bd09ll");
        option.setPriority(LocationClientOption.NetWorkFirst);
        //option.setScanSpan(5000);
        option.disableCache(true);
        option.setPoiNumber(DEFAULT_NEARBY_PIN_COUNT);
        option.setPoiDistance(1000);
        option.setPoiExtraInfo(true);
        option.setAddrType("all");
        option.setPoiExtraInfo(true);
        locationClient.start();
        locationClient.setLocOption(option);
        locationClient.registerLocationListener(baiduLocationListener);            
        if(locationClient != null && locationClient.isStarted()){
            locationClient.requestLocation();  
        }
    }
    private void initAllProvider() {
        if(useGMS){
            initGpsProvider();
            initNetworkProvider();
        }else{
            initBaiduProvider();
        }
    }

    private void removeGpsProvider() {
        mLocationManager.removeUpdates(mGpsLocationListener);
        setListenerStatus(LOC_LISTENER_GPS, nmsListenerStatus.REMOVE);
    }

    private void removeNetworkProvider() {
        mLocationManager.removeUpdates(mNetworkLocationListener);
        setListenerStatus(LOC_LISTENER_NETWORK, nmsListenerStatus.REMOVE);
    }

    private void removeAllProvider() {
        mLocationManager.removeUpdates(mGpsLocationListener);
        mLocationManager.removeUpdates(mNetworkLocationListener);

        setListenerStatus(LOC_LISTENER_ALL, nmsListenerStatus.REMOVE);
    }

    private void setListenerStatus(int type, nmsListenerStatus status) {
        NmsLog.trace(TAG, "setListenerStatus. type: " + type + ". status: " + status);

        if (type == LOC_LISTENER_GPS) {
            mGpsListenerStatus = status;
        } else if (type == LOC_LISTENER_NETWORK) {
            mNetworkListenerStatus = status;
        } else if (type == LOC_LISTENER_ALL) {
            mGpsListenerStatus = status;
            mNetworkListenerStatus = status;
        } else {
            NmsLog.error(TAG, "setListenerStatus: listener type is UNKNOWN!");
        }
    }

    private void reqShowToast(int resId, int duration) {
        NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_TOAST_SHOW);

        Message msg = Message.obtain(mHandler, HANDLER_TOAST_SHOW, resId, duration, null);
        mHandler.sendMessage(msg);
    }

    private void showToast(int resId, int duration) {
        Toast.makeText(mContext, resId, duration).show();
    }

    private void startTimerCurrLoc() {
        NmsLog.trace(TAG, "startTimerCurrLoc. Timeout: " + DEFAULT_TIMEROUT_CURR_LOC / 1000 + "s");

        mTimerCurrLoc = new Timer();
        mTimerCurrLoc.schedule(new TimerTask() {
            public void run() {
                NmsLog.trace(TAG, "locate curr location timer has timed out");
                removeAllProvider();

                // NmsCellInfoManager cellInfoManager = new
                // NmsCellInfoManager(mContext, mHandler);
                // if (-1 == cellInfoManager.initMyLocationCell()) {
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                //reqShowToast(R.string.STR_NMS_MAP_ERROR_1, Toast.LENGTH_SHORT);
                // }
            }
        }, DEFAULT_TIMEROUT_CURR_LOC);
    }
    private void startTimerCurrLocForBaidu() {
        NmsLog.trace(TAG, "startTimerCurrLocForBaidu. Timeout: " + DEFAULT_TIMEROUT_CURR_LOC / 1000 + "s");

        mTimerCurrLoc = new Timer();
        mTimerCurrLoc.schedule(new TimerTask() {
            public void run() {
                NmsLog.trace(TAG, "locate curr location timer has timed out");
                if (locationClient != null && locationClient.isStarted()){ 
                    locationClient.unRegisterLocationListener(baiduLocationListener);
                    locationClient.stop();  
                    locationClient = null;  
                } 
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
            }
        }, DEFAULT_TIMEROUT_CURR_LOC);
    }
    private void endTimerCurrLoc() {
        NmsLog.trace(TAG, "endTimerCurrLoc");

        if (mTimerCurrLoc != null) {
            mTimerCurrLoc.cancel();
            mTimerCurrLoc.purge();
            mTimerCurrLoc = null;
        }
    }

    private void startTimerWaitForGps() {
        NmsLog.trace(TAG, "startTimerWaitForGps. Timeout: " + DEFAULT_TIMEROUT_WAIT_GPS / 1000
                + "s");

        mTimerWaitForGps = new Timer();
        mTimerWaitForGps.schedule(new TimerTask() {
            public void run() {
                NmsLog.trace(TAG, "wait for gps timer has timed out");

                removeGpsProvider();
                threadGetCurrLocation();
            }
        }, DEFAULT_TIMEROUT_WAIT_GPS);
    }

    private void endTimerWaitForGps() {
        NmsLog.trace(TAG, "endTimerWaitForGps");

        if (mTimerWaitForGps != null) {
            mTimerWaitForGps.cancel();
            mTimerWaitForGps.purge();
            mTimerWaitForGps = null;
        }
    }

    private void doCellInfoCallback(NmsLocation location) {
        if (location == null) {
            NmsLog.warn(TAG, "doCellInfoCallback. location is null");
            reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
            return;
        }

        location.setLatitude(NmsLocationManager.formatLatLng(location.getLatitude())
                + DEFAULT_LAT_OFFSET);
        location.setLongitude(NmsLocationManager.formatLatLng(location.getLongitude())
                + DEFAULT_LNG_OFFSET);

        mCurrLocation = location.clone();

        threadGetCurrLocation();
    }

    private void doLocationChanged(Location location, int type) {
        if (location != null) {
            endTimerCurrLoc();

            double lat = NmsLocationManager.formatLatLng(location.getLatitude());
            double lng = NmsLocationManager.formatLatLng(location.getLongitude());

            lat += DEFAULT_LAT_OFFSET;
            lng += DEFAULT_LNG_OFFSET;

            mCurrLocation = new NmsLocation(lat, lng);

            if (type == LOC_LISTENER_GPS) {
                endTimerWaitForGps();
                removeAllProvider();

                threadGetCurrLocation();
            } else if (type == LOC_LISTENER_NETWORK) {
                removeNetworkProvider();

                if (mGpsListenerStatus == nmsListenerStatus.RUNNING) {
                    startTimerWaitForGps();
                } else {
                    threadGetCurrLocation();
                }
            } else {
                NmsLog.error(TAG, "doLocationChanged: listener type is UNKNOWN!");
            }
        }
    }

    private void threadGetCurrLocation() {
        NmsLog.trace(TAG, "thread to load the current location");

        new Thread() {
            @Override
            public void run() {
                super.run();

                if (mCurrLocation == null) {
                    NmsLog.error(TAG, "thread curr: mCurrLocation is null");
                    return;
                }

                NmsLocation loc = NmsLocationManager.getAddress(mCurrLocation.getLatitude(),
                        mCurrLocation.getLongitude());

                if (loc == null) {
                    NmsLog.warn(TAG, "thread curr: loc is null");
                    reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                    return;
                }

                if (!loc.isOk()) {
                    NmsLog.warn(TAG, "thread curr: loc.isOk() is false.");
                    reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                    //reqShowToast(R.string.STR_NMS_MAP_ERROR_2, Toast.LENGTH_SHORT);
                    return;
                }

                if (TextUtils.isEmpty(loc.getVicinity())) {
                    NmsLog.warn(TAG, "thread curr: loc.getVicinity() is empty");
                    reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_FAILED);
                    return;
                }

                mCurrLocation.setName(loc.getName());
                mCurrLocation.setVicinity(loc.getVicinity());
                mPinLocation = mCurrLocation.clone();

                threadGetNearbyLocation();

                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_SUCCEED);
            }
        }.start();
    }

    private void threadGetNearbyLocation() {
        NmsLog.trace(TAG, "thread to load the nearby location");

        new Thread() {
            @Override
            public void run() {
                super.run();

                if (mPinLocation == null) {
                    NmsLog.error(TAG, "thread nearby: mPinLocation is null!");
                    return;
                }
                if (mListData == null) {
                    NmsLog.error(TAG, "thread nearby: mListData is null!");
                    return;
                }

                List<NmsLocation> locList = NmsLocationManager.getLocation(
                        mPinLocation.getLatitude(), mPinLocation.getLongitude(), mRadius);

                if (locList == null || locList.size() <= 0) {
                    NmsLog.warn(TAG, "thread nearby: locList is null, or locList.size() <= 0");
                    reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_NEARBY_FAILED);
                    return;
                }

                mListData.clear();
                for (int i = 0; i < locList.size() && i < mPinCount; ++i) {
                    NmsLocation loc = locList.get(i);
                    mListData.add(loc.clone());
                }

                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_NEARBY_SUCCEED);
            }
        }.start();
    }

    private void reqUpdateUI(int control, int action) {
        NmsLog.trace(TAG, "handler send msg, " + "msg type: " + HANDLER_UPDATE_UI + ", control: "
                + control + ", action: " + action);

        Message msg = Message.obtain(mHandler, HANDLER_UPDATE_UI, control, action);
        mHandler.sendMessage(msg);
    }

    private void updateUI(int control, int action) {
        NmsLog.trace(TAG, "updateUI. control: " + control + ", action: " + action);
        switch (control) {
        case CONTROL_CUSTOMVIEW:
            updateCustomView(action);
            break;
        default:
            NmsLog.error(TAG, "updateUI. control is UNKNOWN!");
            break;
        }
    }

    private void updateCustomView(int action) {
        NmsLog.trace(TAG, "updateCustomView. action: " + action);
        switch (action) {
        case ACTION_CUSTOMVIEW_CURR_LOADING:
        case ACTION_CUSTOMVIEW_NEARBY_LOADING:
            showLoadingCustomView(action);
            break;
        case ACTION_CUSTOMVIEW_CURR_FAILED:
        case ACTION_CUSTOMVIEW_NEARBY_FAILED:
            showFailedCustomView(action);
            break;
        case ACTION_CUSTOMVIEW_CURR_SUCCEED:
        case ACTION_CUSTOMVIEW_NEARBY_SUCCEED:
            showSucceedCustomView(action);
            break;
        default:
            NmsLog.error(TAG, "updateCustomView. action is UNKNOWN!");
            break;
        }
    }

    private void showLoadingCustomView(int action) {
        if (action == ACTION_CUSTOMVIEW_NEARBY_LOADING) {
            String lineThree = getString(R.string.STR_NMS_MAP_LINETHREE_NEARBY_LOCATING);

            mCustomView.setTextViewText(null, null, lineThree);
            mCustomView.setTextViewVisibility(-1, -1, View.VISIBLE);
            mCustomView.setLineVisibility(View.VISIBLE);
        } else {
            String lineOne = "";

            if (action == ACTION_CUSTOMVIEW_CURR_LOADING) {
                lineOne = getString(R.string.STR_NMS_MAP_LINEONE_CURR_LOCATING);
            } else {
                NmsLog.error(TAG, "showLoadingCustomView. action is UNKNOWN!");
                return;
            }

            mCustomView.setTextViewText(lineOne, null, null);
            mCustomView.setTextViewVisibility(View.VISIBLE, View.GONE, View.GONE);
            mCustomView.setTextShowClickable(false);
            mCustomView.setImageButtonVisibility(View.GONE, View.GONE);
            mCustomView.setLineVisibility(View.GONE);
        }
        mCustomView.setProgressBar(View.VISIBLE);
        mCustomView.setListViewVisibility(View.GONE);
    }

    private void showFailedCustomView(int action) {
        if (action == ACTION_CUSTOMVIEW_NEARBY_FAILED) {
            String lineThree = getString(R.string.STR_NMS_MAP_LINETHREE_FAILED);
            mCustomView.setTextViewText(null, null, lineThree);
            mCustomView.setTextViewVisibility(-1, -1, View.VISIBLE);
            mCustomView.setLineVisibility(View.VISIBLE);
        } else {
            if (action == ACTION_CUSTOMVIEW_CURR_FAILED)
                mLastFailed = ACTION_CUSTOMVIEW_CURR_FAILED;
            else
                mLastFailed = -1;

            String lineOne = getString(R.string.STR_NMS_MAP_LINEONE_FAILED);
            String lineTwo = getString(R.string.STR_NMS_MAP_LINETWO_FAILED);

            mCustomView.setTextViewText(lineOne, lineTwo, null);
            mCustomView.setTextViewVisibility(View.VISIBLE, View.VISIBLE, View.GONE);
            mCustomView.setTextShowClickable(false);
            mCustomView.setImageButtonVisibility(View.VISIBLE, View.GONE);
            mCustomView.setLineVisibility(View.GONE);
        }
        mCustomView.setProgressBar(View.GONE);
        mCustomView.setListViewVisibility(View.GONE);
    }

    private void showSucceedCustomView(int action) {
        if (action == ACTION_CUSTOMVIEW_NEARBY_SUCCEED) {
            if (mListAdapter == null) {
                NmsLog.error(TAG, "showSucceedCustomView: mListAdapter is null!");
                return;
            }

            mCustomView.setTextViewVisibility(-1, -1, View.GONE);
            mCustomView.setProgressBar(View.GONE);
            mCustomView.setListViewVisibility(View.VISIBLE);

            mListAdapter.notifyDataSetChanged();
        } else {
            if (mPinLocation == null) {
                NmsLog.error(TAG, "showSucceedCustomView: mCurrLocation is null");
                return;
            }

            String lineOne = "";
            String lineTwo = mPinLocation.getVicinity();
            String lineThree = getString(R.string.STR_NMS_MAP_LINETHREE_NEARBY_LOCATING);

            if (action == ACTION_CUSTOMVIEW_CURR_SUCCEED) {
                lineOne = getString(R.string.STR_NMS_MAP_LINEONE_CURR);
            } else {
                NmsLog.error(TAG, "showSucceedCustomView: action(" + action + ") is invalid!");
                return;
            }

            mCustomView.setTextViewText(lineOne, lineTwo, lineThree);
            mCustomView.setTextViewVisibility(View.VISIBLE, View.VISIBLE, View.VISIBLE);
            mCustomView.setTextShowClickable(true);
            mCustomView.setImageButtonVisibility(View.GONE, View.VISIBLE);
            mCustomView.setLineVisibility(View.VISIBLE);
            mCustomView.setProgressBar(View.VISIBLE);
            mCustomView.setListViewVisibility(View.GONE);
        }
    }

    private void startSysLocActivity() {
        NmsLog.trace(TAG, "intent to System location settings");

        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        try {
            this.startActivityForResult(intent, REQUSETCODE_SYSTEM_LOCATION_SERVICES);
        } catch (Exception e) {
            NmsLog.error(TAG, "can not open system location services");
            showToast(R.string.STR_NMS_MAP_NOTE_1, Toast.LENGTH_SHORT);
        }
    }

    private void showSetSysLocDialog() {
        new AlertDialog.Builder(mContext)
                .setTitle(R.string.STR_NMS_MAP_TITLE)
                .setMessage(R.string.STR_NMS_MAP_PROMPT)
                .setPositiveButton(R.string.STR_NMS_MAP_SETTINGS,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                startSysLocActivity();
                            }
                        }).setNegativeButton(R.string.STR_NMS_MAP_CANCEL, null).show();
    }

    private void refreshCurrLocation() {
        if (mGpsListenerStatus != nmsListenerStatus.RUNNING
                && mNetworkListenerStatus != nmsListenerStatus.RUNNING) {
            initAllProvider();
            if (mGpsListenerStatus == nmsListenerStatus.INVALID
                    && mNetworkListenerStatus == nmsListenerStatus.INVALID) {
                // reqUpdateUI(CONTROL_CUSTOMVIEW,
                // ACTION_CUSTOMVIEW_CURR_FAILED);
                showSetSysLocDialog();
            } else {
                if(useGMS){
                    startTimerCurrLoc();
                }else{
                    startTimerCurrLocForBaidu();
                }
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_LOADING);
            }
        }
    }

    private void shareFinished() {
        if (mPDWait != null) {
            mPDWait.dismiss();
        }

        String shareAddr = mPinLocation.getName() + NmsLocationFormat.FLAG_THIRD
                + mPinLocation.getVicinity();
        shareAddr = NmsLocationFormat.formatLocationStr(shareAddr);
        if (shareAddr != null
                && shareAddr.length() > NmsCustomUIConfig.LOCATION_ADDR_MAX_LENGTH + 20) {
            NmsLog.warn(TAG, "shareAddr is too long: " + shareAddr.length());
            shareAddr = NmsLocationFormat.FLAG_THIRD + mPinLocation.getVicinity();
            shareAddr = NmsLocationFormat.formatLocationStr(shareAddr);
        }

        double lat = mPinLocation.getLatitude();
        double lng = mPinLocation.getLongitude();

        lat = NmsLocationManager.formatLatLng(lat);
        lng = NmsLocationManager.formatLatLng(lng);

        // NmsIpLocationMessage msg = new NmsIpLocationMessage();
        // msg.latitude = lat;
        // msg.longitude = lng;
        // msg.address = shareAddr;

        Intent intent = new Intent();
        // intent.putExtra(NmsIpMessageConsts.NMS_SHARE_LOCATION_DONE, msg);

        intent.putExtra(NmsShareLocationDone.NMS_LOCATION_ADDRESS, shareAddr);
        intent.putExtra(NmsShareLocationDone.NMS_LOCATION_LATITUDE, lat);
        intent.putExtra(NmsShareLocationDone.NMS_LOCATION_LONGITUDE, lng);

        NmsLog.trace(TAG, "shareFinished");
        this.setResult(RESULT_OK, intent);
        finish();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        NmsLog.trace(TAG, "Get activtiy result requestCode: " + requestCode);
        switch (requestCode) {
        case REQUSETCODE_SYSTEM_LOCATION_SERVICES:
            initAllProvider();
            if (mGpsListenerStatus == nmsListenerStatus.RUNNING
                    || mNetworkListenerStatus == nmsListenerStatus.RUNNING) {
                NmsLog.trace(TAG, "User turn on the location service.");
                reqUpdateUI(CONTROL_CUSTOMVIEW, ACTION_CUSTOMVIEW_CURR_LOADING);
                startTimerCurrLoc();
            }
            break;
        default:
            NmsLog.trace(TAG, "onActivityResult requestCode is UNKNOWN!");
            break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(useGMS){
            removeAllProvider();
        }else{
            if (locationClient != null && locationClient.isStarted()){ 
                locationClient.unRegisterLocationListener(baiduLocationListener);
                locationClient.stop();  
                locationClient = null;  
            } 
        }
    }

    private void showEditLocationDialog(CharSequence text) {
        final EditText input = new EditText(mContext);

        InputFilter[] filters = { new LengthFilter(NmsCustomUIConfig.LOCATION_ADDR_MAX_LENGTH) };
        input.setFilters(filters);
        input.setText(text);

        Selection.setSelection(input.getEditableText(), input.getText().length());

        input.addTextChangedListener(new TextWatcher() {
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if (NmsCustomUIConfig.LOCATION_ADDR_MAX_LENGTH == input.getText().length()) {
                    showToast(R.string.STR_NMS_MAP_EDIT_LIMIT, Toast.LENGTH_SHORT);
                }
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void afterTextChanged(Editable s) {
            }
        });

        new AlertDialog.Builder(mContext)
                .setTitle(R.string.STR_NMS_MAP_EDIT)
                .setView(input)
                .setPositiveButton(R.string.STR_NMS_MAP_SHARE,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                String value = input.getText().toString();
                                onShareLoaction(null, value);
                                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                                if (imm.isActive()) {
                                    imm.hideSoftInputFromWindow(input.getWindowToken(), 0);
                                }
                            }
                        }).setNegativeButton(R.string.STR_NMS_MAP_CANCEL, null).show();

        new Timer().schedule(new TimerTask() {
            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) mContext
                        .getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT,
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        }, 300);
    }

    @Override
    public void onRefreshLocation() {
        NmsLog.trace(TAG, "listened to the onRefreshLocation event, mLastFailed: " + mLastFailed);

        if (mLastFailed == ACTION_CUSTOMVIEW_CURR_FAILED) {
            refreshCurrLocation();
        } else {
            NmsLog.error(TAG, "mLastFailed UNKNOWN");
        }
    }

    @Override
    public void onEditLocation(CharSequence text) {
        NmsLog.trace(TAG, "listened to the onEditLocation event");
        showEditLocationDialog(text);
    }

    @Override
    public void onShareLoaction(CharSequence textOne, CharSequence textTwo) {
        NmsLog.trace(TAG, "listened to the onShareLoaction event");

        int len = 0;
        if (textOne != null) {
            len += textOne.length();
        }
        if (textTwo != null) {
            len += textTwo.length();
        }
        if (len > NmsCustomUIConfig.LOCATION_ADDR_MAX_LENGTH) {
            showToast(R.string.STR_NMS_MAP_ADDR_LEN, Toast.LENGTH_SHORT);
            return;
        }

        mPDWait = new ProgressDialog(mContext);
        mPDWait.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mPDWait.setTitle(R.string.STR_NMS_MAP_TITLE);
        mPDWait.setMessage(getString(R.string.STR_NMS_MAP_WAIT));
        mPDWait.setIndeterminate(false);
        mPDWait.setCancelable(false);
        mPDWait.show();

        String name = null;
        if (textOne != null) {
            name = (String) textOne;
        } else {
            name = mPinLocation.getName();
        }
        name = NmsLocationFormat.formatLocationStr(name);
        if (!TextUtils.isEmpty(name)) {
            name = name.replace(NmsLocationFormat.FLAG_THIRD, " ");
            mPinLocation.setName(name);
        } else {
            mPinLocation.setName("");
        }
        String vicinity = NmsLocationFormat.formatLocationStr((String) textTwo);
        vicinity = vicinity.replace(NmsLocationFormat.FLAG_THIRD, " ");
        mPinLocation.setVicinity(vicinity);

        NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_SHARE_DATA_READY);
        mHandler.sendEmptyMessage(HANDLER_SHARE_DATA_READY);
    }

}
