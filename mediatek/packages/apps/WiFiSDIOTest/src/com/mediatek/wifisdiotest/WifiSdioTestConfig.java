package com.mediatek.wifisdiotest;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.provider.Settings;
import android.location.LocationManager;

import com.mediatek.wifisdiotest.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class WifiSdioTestConfig extends Activity implements OnClickListener,
        OnCheckedChangeListener {

    private static final String TAG = "EM/WifiSdioConfig";

    private static final String KEY_PROGRESSBAR_VISIBLE = "progress_bar";
    private static final String KEY_LISTVIEW_VISIBLE = "ap_listview";
    private static final String REGEX_PATTER_IP = 
        "(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)){3}";
    private static final Pattern IP_PATTERN = Pattern.compile(REGEX_PATTER_IP);
    private static final int HANDLER_ID_UPDATE_AP_LIST = 1000;
    private static final int HANDLER_ID_UPDATE_CONFIG = 1001;
    private static final int HANDLER_ID_UPDATE_TESTUI = 1002;

    private static final int HANDLER_ID_INIT_DEVICE = 1003; 
    
    private static final String WIFI_CAPAB_WAPI_PSK = "WAPI-PSK";
    private static final String WIFI_CAPAB_WAPI_CERT = "WAPI-CERT";
    private static final String WIFI_CAPAB_WEP = "WEP";
    private static final String WIFI_CAPAB_PSK = "PSK";
    private static final String WIFI_CAPAB_EAP = "EAP";
    private static final String ACTION_TOFMSERVICE_POWERDOWN
                               = "com.mediatek.FMRadio.FMRadioService.ACTION_TOFMSERVICE_POWERDOWN";
                               
    private EditText mEtServerIp = null;
    private TextView mTvApSelect = null;
    private ProgressBar mPbWaitBar = null;
    private ListView mLvAps = null;
    private EditText mEtUsername = null;
    private EditText mEtPassword = null;
    private EditText mEtIperfTime = null;
    private EditText mEtIperfStream = null;
    private EditText mEtIperfWindow = null;
    private EditText mEtIperfMss = null;
    private EditText mEtCmdEdge = null;
    private EditText mEtCmdDelay = null;
    private EditText mEtLastRunTime = null;
    private EditText mEtLastRunCount = null;
    private CheckBox mCbCmdEnable = null;
    private Button mBtStart = null;
    private Button mBtStop = null;

    private int mOriBackup = 0;

    private WifiSdioTestService mService = null;
    private ArrayAdapter<String> mListAdapter = null;
    private ArrayList<String> mWifiApList = null;
    private List<ScanResult> mScanResultList = null;
    private WifiManager mWifiManager = null;
    // [JUJU_20130617] Add for Wifi Scan AP Check    
    //private boolean isScanAPDelayDone = false;
    private static final int HANDLER_ID_CONNECT_ERROR = 1004;
    private static final int HANDLER_ID_SCAN_NO_AP = 1005;
    private static final int HANDLER_ID_CHECK_AP = 1006; 
    private static final int HANDLER_ID_AP_FOUND = 1007;
    private static final int HANDLER_ID_INIT = 1008;
    private int mCurrentCount = 0;
    private static final int DIALOG_AP_SEARCH = 0;
    private boolean mScanResultDone = false;
     
    private final BroadcastReceiver mWifiReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "mWifiReceiver onReceive");
            String action = intent.getAction();
            if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(action)) {
                Xlog.v(TAG, "Wifi scan result available");
                List<WifiConfiguration> wcList = mWifiManager
                        .getConfiguredNetworks();
                mScanResultList = mWifiManager.getScanResults();
                if (null == mScanResultList) {
                    Xlog.w(TAG, "Wifi scan remote exception");
                    mFindDelayHandler.sendEmptyMessage(HANDLER_ID_CONNECT_ERROR);
                } else if (0 == mScanResultList.size()) {
                    Xlog.w(TAG, "Scan result size: 0");
                    mFindDelayHandler.sendEmptyMessage(HANDLER_ID_SCAN_NO_AP);
                } else {
                    mFindDelayHandler.sendEmptyMessage(HANDLER_ID_AP_FOUND);
                }
            }
        }
    };

    private final BroadcastReceiver mTestUpdateReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "mTestUpdateReceiver onReceive");
            String action = intent.getAction();
            if (WifiSdioTestService.ACTION_CONFIG_UPDATE_REQUEST.equals(action)) {
                mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_CONFIG);
            } else if (WifiSdioTestService.ACTION_TEST_STATUS_CHANGED
                    .equals(action)) {
                mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_TESTUI);
            }
        }
    };

    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case HANDLER_ID_UPDATE_AP_LIST:
                if (View.VISIBLE == mPbWaitBar.getVisibility()) {
                    mPbWaitBar.setVisibility(View.GONE);
                    mLvAps.setVisibility(View.VISIBLE);
                    Iterator<ScanResult> i = mScanResultList.iterator();
                    int index = 0;
                    mWifiApList.clear();
                    while (i.hasNext()) {
                        ScanResult scanResult = i.next();
                        Xlog.d(TAG, "index: " + index + scanResult.toString());
                        index++;
                        if (!mWifiApList.contains(scanResult.SSID)) {
                            mWifiApList.add(scanResult.SSID);
                        }
                    }
                    mListAdapter.notifyDataSetChanged();
                }
                break;
            case HANDLER_ID_UPDATE_CONFIG:
                updateTestConfig();
                break;
            case HANDLER_ID_UPDATE_TESTUI:
                if (null != mService) {
                    updateTestUi(WifiSdioTestService.STATUS_STOPPED != mService
                            .getTestStatus());
                }
                break;
            default:
                break;
            }
        }
    };
 
    private final Handler mInitHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case HANDLER_ID_INIT_DEVICE:
                BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
                if (btAdapter.getState() != BluetoothAdapter.STATE_OFF) {
                    // Close bluetooth
                    Xlog.v(TAG, "Close BT");
                    btAdapter.disable();
                }
                if(isGPSEnable()) {
                    // close GPS
                    Xlog.v(TAG, "Close GPS");
                    closeGPS();
                }
                break; 
            default:
                break;
            }
        }
    };
    
    private boolean isGPSEnable() {   
        String str = Settings.Secure.getString(getContentResolver(),  
                Settings.Secure.LOCATION_PROVIDERS_ALLOWED);  
        Xlog.v(TAG, "GPS-"+str);  
        if (str != null) {  
            return str.contains("gps");  
        }  
        else{  
            return false;  
        }  
    }
    
    private void closeGPS() {
        Settings.Secure.setLocationProviderEnabled(getContentResolver(), LocationManager.GPS_PROVIDER, false);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Configuration.ORIENTATION_LANDSCAPE == getResources()
                .getConfiguration().orientation) {
            setContentView(R.layout.wifi_sdio_config_land);
        } else {
            setContentView(R.layout.wifi_sdio_config_port);
        }
        mOriBackup = getResources().getConfiguration().orientation;
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        if (null == mWifiManager) {
            Xlog.e(TAG, "Wifi Manager is null");
            Toast.makeText(this, R.string.wifi_sdio_toast_not_support_wifi,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        mWifiApList = new ArrayList<String>();
        mListAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_single_choice, mWifiApList);
        initComponents();
        mPbWaitBar.setVisibility(View.GONE);
    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter wifiFilter = new IntentFilter();
        wifiFilter.addAction(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
        registerReceiver(mWifiReceiver, wifiFilter);
        IntentFilter testFilter = new IntentFilter();
        testFilter.addAction(WifiSdioTestService.ACTION_CONFIG_UPDATE_REQUEST);
        testFilter.addAction(WifiSdioTestService.ACTION_TEST_STATUS_CHANGED);
        registerReceiver(mTestUpdateReceiver, testFilter);
        getApplicationContext().bindService(
                new Intent(this, WifiSdioTestService.class), mServiceCon,
                Context.BIND_AUTO_CREATE);

        mInitHandler.sendEmptyMessage(HANDLER_ID_INIT_DEVICE);
        Intent requestIntent = new Intent(ACTION_TOFMSERVICE_POWERDOWN); // close FM
        Xlog.v(TAG, "send broadcast ACTION_TOFMSERVICE_POWERDOWN");
        sendBroadcast(requestIntent);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mOriBackup != getResources().getConfiguration().orientation) {
            onConfigurationChanged(getResources().getConfiguration());
        }
    }

    @Override
    protected void onPause() {
        mOriBackup = getResources().getConfiguration().orientation;
        super.onPause();
    }

    @Override
    protected void onStop() {
        mPbWaitBar.setVisibility(View.GONE);
        mLvAps.clearChoices();
        unregisterReceiver(mWifiReceiver);
        unregisterReceiver(mTestUpdateReceiver);
        getApplicationContext().unbindService(mServiceCon);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
    
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        switch (id) {
        case DIALOG_AP_SEARCH:
            ProgressDialog innerDialog = new ProgressDialog(this);
            innerDialog.setTitle(R.string.wifi_dialog_init);
            innerDialog
                    .setMessage(getString(R.string.wifi_dialog_init_message));
            innerDialog.setCancelable(false);
            innerDialog.setIndeterminate(true);
            dialog = innerDialog;
            break;
        default:
            Xlog.d(TAG, "error dialog ID");
            break;
        }
        return dialog;
    }
    
    private final Handler mFindDelayHandler = new Handler() {
    	public int cmdEdge = 0;
    	public int readEdge = 0;
    	public int writeEdge = 0;
    	public int cmdDelay = 0;
    	public int readDelay = 0;
    	public int writeDelay = 0;
    	public int cmdIntDelay = 0;
    	public List<Boolean> cmdDelayOKList = new ArrayList<Boolean>();
    	public List<Boolean> readDelayOKList = new ArrayList<Boolean>();
    	public List<Boolean> writeDelayOKList = new ArrayList<Boolean>();
    	public List<Boolean> cmdIntDelayOKList = new ArrayList<Boolean>();
        private int maxCount = 64+64+64+32;
        private static final int EDGE_MAX = 2;
        private static final int DELAY_MAX = 32;
        private static final long DELAY_TIME = 10000;
        private static final int GROUP_GAP = DELAY_MAX*EDGE_MAX; 
        private boolean isFirst = true;
        List<SDIOParameter> paramList = new ArrayList<SDIOParameter>();  
        class SDIOParameter
        {
        	int cmdEdge;
        	int readEdge;
        	int writeEdge;
        	int cmdDelay;
        	int readDelay;
        	int writeDelay;
        	int cmdIntDelay;
        	SDIOParameter(int cmdEdge, int readEdge, int writeEdge, int cmdDelay, int readDelay, int writeDelay, int cmdIntDelay)
        	{
        		this.cmdEdge = cmdEdge;
        		this.readEdge = readEdge;
        		this.writeEdge = writeEdge;
        		this.cmdDelay = cmdDelay;
        		this.readDelay = readDelay;
        		this.writeDelay = writeDelay;
        		this.cmdIntDelay = cmdIntDelay;
        	}
        }     
        
        private int getBestValue(List<Boolean> okList)
        {
        	int i;
    		Map<Integer, Integer> lenMinMap = new TreeMap<Integer, Integer>();	// key: Length, value: MinValue
			int size = okList.size();
			int minValue=-1, maxValue=-1, preValue=-1;
			if(okList.get(0)){
				minValue = 0;
				preValue = 0;
				maxValue = 0;
			}
			for(i=1; i<size; i++){
				if(okList.get(i)){
					if(minValue == -1)
						minValue = i;
					if(preValue == i-1)
						maxValue = i;
					preValue = i;
				} else {
					lenMinMap.put(maxValue-minValue, minValue);
					minValue = -1;
					preValue = -1;
				}
			}
			if(lenMinMap.size()>0){
				size = lenMinMap.values().size();
				Integer lenSet[] = ((Integer[])lenMinMap.keySet().toArray());
				Integer minSet[] = ((Integer[])lenMinMap.values().toArray());
				minValue = minSet[size-1];
				maxValue = minValue+lenSet[size-1];
				minValue = (minValue+maxValue)/2;
			} else if(minValue!=-1){
				minValue = (minValue + maxValue)/2;
			}
			return minValue;
        }
        
        private boolean selectResult()
        {        	        	       	
        	int curIndex = mCurrentCount%GROUP_GAP;
            boolean isScanAPDelayDone = false;
        	if(curIndex == 0){
        		int curGroup = mCurrentCount/GROUP_GAP;
        		int bestValue;
        		SDIOParameter curRes;
        		switch(curGroup){
        		case 1:
        			bestValue = getBestValue(cmdDelayOKList);
        			if(bestValue != -1){
        				curRes = paramList.get(bestValue);
        				this.cmdEdge = curRes.cmdEdge;
        				this.cmdDelay = curRes.cmdDelay;
        				isScanAPDelayDone = true;
        				mService.fillSDIOParameter(this.cmdEdge, this.readEdge, this.writeEdge, this.cmdDelay, this.readDelay, this.writeDelay, this.cmdIntDelay);	
        			}
        			break;
        		case 2:
        			bestValue = getBestValue(readDelayOKList);
        			if(bestValue != -1){
        				curRes = paramList.get(2*GROUP_GAP+bestValue);
        				this.readEdge = curRes.readEdge/DELAY_MAX;
        				this.readDelay = curRes.cmdDelay%DELAY_MAX;
        				isScanAPDelayDone = true;
        				mService.fillSDIOParameter(this.cmdEdge, this.readEdge, this.writeEdge, this.cmdDelay, this.readDelay, this.writeDelay, this.cmdIntDelay);	
        			}
        			break;
        		case 3:
        			bestValue = getBestValue(writeDelayOKList);
        			if(bestValue != -1){
        				curRes = paramList.get(3*GROUP_GAP+bestValue);
        				this.writeEdge = curRes.writeEdge/DELAY_MAX;
        				this.writeDelay = curRes.writeDelay%DELAY_MAX;
        				isScanAPDelayDone = true;
        				mService.fillSDIOParameter(this.cmdEdge, this.readEdge, this.writeEdge, this.cmdDelay, this.readDelay, this.writeDelay, this.cmdIntDelay);	
        			}
        			break;
        		case 4:
        			bestValue = getBestValue(cmdIntDelayOKList);
        			if(bestValue != -1){
        				curRes = paramList.get(3*GROUP_GAP+bestValue);
        				this.cmdIntDelay = curRes.cmdIntDelay/DELAY_MAX;
        				isScanAPDelayDone = true;
        				mService.fillSDIOParameter(this.cmdEdge, this.readEdge, this.writeEdge, this.cmdDelay, this.readDelay, this.writeDelay, this.cmdIntDelay);	
        			}
        			break;
        		}
        	}
            return isScanAPDelayDone;
        }
        // 0-63: cmdEdge+cmdDelay
        // 64-128: rDataEdge + rDataDelay
        // 129-192: wDataEdge wDataDelay
        // 192-224: intReadDelay
        private void configParameter(){
        	int ce=0, re=0, we=0, cd=0, rd=0, wd=0, cid=0;        	
        	SDIOParameter meter = null;
        	if(mCurrentCount<64){
        		ce = mCurrentCount/DELAY_MAX;
        		cd = mCurrentCount%DELAY_MAX;
        		meter = new SDIOParameter(ce, re, we, cd, rd, wd, cid);
        		paramList.add(meter);
        	} else if(mCurrentCount<128){
        		re = (mCurrentCount-64)/DELAY_MAX;
        		rd = mCurrentCount%DELAY_MAX;
        		meter = new SDIOParameter(this.cmdEdge, re, we, this.cmdDelay, rd, wd, cid);
        		paramList.add(meter);
        	} else if(mCurrentCount<192){
        		we = (mCurrentCount-128)/DELAY_MAX;
        		wd = mCurrentCount%DELAY_MAX;
        		meter = new SDIOParameter(this.cmdEdge, this.readEdge, we, this.cmdDelay, this.readDelay, wd, cid);
        		paramList.add(meter); 
        	} else if(mCurrentCount<224){
        		cid = (mCurrentCount-192)/DELAY_MAX;
        		meter = new SDIOParameter(this.cmdEdge, this.readEdge, this.writeEdge, this.cmdDelay, this.readDelay, this.writeDelay, cid);
        		paramList.add(meter);
        	} else {
        		Xlog.w(TAG, "Warning~~~~Warning~~~~Warning~~~~Warning~~~~"+mCurrentCount);
        	}
        	if(mCurrentCount != 0){
        		if(meter!=null)
        			mService.fillSDIOParameter(meter.cmdEdge, meter.readEdge, meter.writeEdge, meter.cmdDelay, meter.readDelay, meter.writeDelay, meter.cmdIntDelay);		
    		}
        	//selectResult();
        }
        private void fillResult(int curCount, Boolean isOK)
        {
        	if(curCount<GROUP_GAP){
        		cmdDelayOKList.add(isOK);
        	} else if(curCount<GROUP_GAP*2){
        		readDelayOKList.add(isOK);
        	} else if(curCount<GROUP_GAP*3){
        		writeDelayOKList.add(isOK);
        	} else if(curCount<GROUP_GAP*3+DELAY_MAX){
        		cmdIntDelayOKList.add(isOK);
        	} else {
        		Xlog.w(TAG, "Warning~~~~Warning~~~~Warning~~~~Warning~~~~"+curCount);
        	}
        }
        
    	@Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if(mCurrentCount >= 224 ) {
        		Xlog.w(TAG, "Found Done");
        		return;
            }        	
            switch (msg.what) {
            case HANDLER_ID_INIT:
            	mService.doWifiTestPrepare();
            	this.sendEmptyMessageDelayed(HANDLER_ID_CHECK_AP, 1000);
            	break;
            case HANDLER_ID_CHECK_AP:
            	if(selectResult()){
            		mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_AP_LIST);
                    removeDialog(DIALOG_AP_SEARCH);
                    mScanResultDone = true;
                    Xlog.w(TAG, "remove dialog");
            		break;
            	}
                if(mCurrentCount == 1){
                    showDialog(DIALOG_AP_SEARCH);
                }
                 Xlog.w(TAG, "config parmeter");
                configParameter();
            	if(mScanResultList != null){
            		mScanResultList.clear();
            		mScanResultList = null;
            	}        
            	mService.disableWifi();
            	/*Intent i = new Intent(WifiAdmService.WIFI_ADM_ACTION);
                i.putExtra(WifiAdmService.KEY_OPT, WifiAdmService.WIFI_OPT_ENABLE);
                startService(i);
                */
            	mService.enableWifi();
            	Intent i = new Intent(WifiAdmService.WIFI_ADM_ACTION);
                i.putExtra(WifiAdmService.KEY_OPT, WifiAdmService.WIFI_OPT_SCAN);
                startService(i);  
                break; 
            case HANDLER_ID_AP_FOUND:
            	fillResult(mCurrentCount, true);
                Xlog.d(TAG, "[GETAP_HANDLER] find ap, count =" + mCurrentCount);
            	if(mCurrentCount== 0 || mScanResultDone){
            		mScanResultDone = true;
            		mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_AP_LIST);
            	}else {
            		this.sendEmptyMessage(HANDLER_ID_CHECK_AP);
            		mCurrentCount++;
            	}
            	break;
            case HANDLER_ID_SCAN_NO_AP:
            case HANDLER_ID_CONNECT_ERROR:
                if(mScanResultDone) {
                    break;
                }
            	fillResult(mCurrentCount, false);
            	this.sendEmptyMessage(HANDLER_ID_CHECK_AP);
            	mCurrentCount++;
            	Xlog.d(TAG, "[GETAP_HANDLER] Can't get Handler Error Issue");
            	break;
            default:
                break;
            }
        }
    };
    
    
    @Override
    public void onClick(View v) {
        if (mTvApSelect.getId() == v.getId()) {
            Xlog.v(TAG, "onClick: refresh clicked");
            mPbWaitBar.setVisibility(View.VISIBLE);
            mLvAps.setVisibility(View.GONE);
            mLvAps.clearChoices();
            mCurrentCount = 0;
            mScanResultDone = false;
            mFindDelayHandler.sendEmptyMessage(HANDLER_ID_INIT);
        } else if (mBtStart.getId() == v.getId()) {
            Xlog.v(TAG, "onClick: start clicked");
            if (validateInput()) {
                Intent intent = new Intent();
                intent.setClass(this, WifiSdioTestService.class);
                intent.putExtras(getConfigs());
                intent.putExtra(WifiSdioTestService.KEY_AP_SECURITY,
                        getSecurity(mScanResultList.get(mLvAps
                                .getCheckedItemPosition())));
                startService(intent);
            }
        } else if (mBtStop.getId() == v.getId()) {
            Xlog.v(TAG, "onClick: stop clicked");
            if (null != mService) {
                mService.stopTestRequest();
            }
            Intent intent = new Intent();
            intent.setClass(this, WifiSdioTestService.class);
            stopService(intent);
            mBtStop.setEnabled(false);
        } else {
            Xlog.w(TAG, "unknown button");
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.getId() == mCbCmdEnable.getId()) {
            updateDefaultCmdStatus(isChecked);
        }
    }

    private void updateDefaultCmdStatus(boolean isChecked) {
        mEtCmdEdge.setEnabled(isChecked);
        mEtCmdDelay.setEnabled(isChecked);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Bundle bundle = saveState();
        if (Configuration.ORIENTATION_LANDSCAPE == newConfig.orientation) {
            setContentView(R.layout.wifi_sdio_config_land);
        } else if (Configuration.ORIENTATION_PORTRAIT == newConfig.orientation) {
            setContentView(R.layout.wifi_sdio_config_port);
        }
        initComponents();
        restoreState(bundle);
        mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_TESTUI);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        restoreState(savedInstanceState);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putAll(saveState());
    }

    private void initComponents() {
        mEtServerIp = (EditText) findViewById(R.id.wifi_sdio_server_ip);
        mTvApSelect = (TextView) findViewById(R.id.wifi_sdio_ap_refresh);
        mTvApSelect.setOnClickListener(this);
        mPbWaitBar = (ProgressBar) findViewById(R.id.wifi_sdio_ap_progress);
        mLvAps = (ListView) findViewById(R.id.wifi_sdio_ap_list);
        mEtUsername = (EditText) findViewById(R.id.wifi_sdio_ap_username);
        mEtPassword = (EditText) findViewById(R.id.wifi_sdio_ap_password);
        mEtIperfTime = (EditText) findViewById(R.id.wifi_sdio_iperf_time);
        mEtIperfStream = (EditText) findViewById(R.id.wifi_sdio_iperf_stream);
        mEtIperfWindow = (EditText) findViewById(R.id.wifi_sdio_iperf_window);
        mEtIperfMss = (EditText) findViewById(R.id.wifi_sdio_iperf_mss);
        mEtCmdEdge = (EditText) findViewById(R.id.wifi_sdio_default_cmd_edge);
        mEtCmdDelay = (EditText) findViewById(R.id.wifi_sdio_default_cmd_delay);
        mEtLastRunCount= (EditText) findViewById(R.id.wifi_sdio_lrun_count);
        mEtLastRunTime= (EditText) findViewById(R.id.wifi_sdio_lrun_time);

        mCbCmdEnable = (CheckBox) findViewById(R.id.wifi_sdio_default_enable_cmd);
        mCbCmdEnable.setOnCheckedChangeListener(this);
        mBtStart = (Button) findViewById(R.id.wifi_sdio_btn_start);
        mBtStart.setOnClickListener(this);
        mBtStop = (Button) findViewById(R.id.wifi_sdio_btn_stop);
        mBtStop.setOnClickListener(this);
        mLvAps.setAdapter(mListAdapter);
        mLvAps.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
    }

    private Bundle saveState() {
        Bundle bundle = new Bundle();
        bundle.putInt(KEY_PROGRESSBAR_VISIBLE, mPbWaitBar.getVisibility());
        bundle.putInt(KEY_LISTVIEW_VISIBLE, mLvAps.getVisibility());
        bundle.putAll(getConfigs());
        return bundle;
    }

    private void restoreState(Bundle bundle) {
        mPbWaitBar.setVisibility(bundle.getInt(KEY_PROGRESSBAR_VISIBLE));
        mLvAps.setVisibility(bundle.getInt(KEY_LISTVIEW_VISIBLE));
        setConfigs(bundle);
    }

    private Bundle getConfigs() {
        Bundle bundle = new Bundle();
        bundle.putString(WifiSdioTestService.KEY_SERVER_IP, mEtServerIp
                .getText().toString());
        String ssid = null;
        try {
            ssid = mListAdapter.getItem(mLvAps.getCheckedItemPosition());
        } catch (ArrayIndexOutOfBoundsException e) {
            Xlog.v(TAG, "Not select ap");
        }
        bundle.putString(WifiSdioTestService.KEY_AP_SSID, ssid);
        bundle.putString(WifiSdioTestService.KEY_AP_USERNAME, mEtUsername
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_AP_PASSWORD, mEtPassword
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_IPERF_TIME, mEtIperfTime
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_IPERF_STREAM, mEtIperfStream
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_IPERF_WINDOW, mEtIperfWindow
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_IPERF_MSS, mEtIperfMss
                .getText().toString());
        bundle.putBoolean(WifiSdioTestService.KEY_DEFAULT_CMD_ENABLE,
                mCbCmdEnable.isChecked());
        bundle.putString(WifiSdioTestService.KEY_DEFAULT_CMD_EDGE, mEtCmdEdge
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_DEFAULT_CMD_DELAY, mEtCmdDelay
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_LASTRUN_COUNT, mEtLastRunCount
                .getText().toString());
        bundle.putString(WifiSdioTestService.KEY_LASTRUN_TIME, mEtLastRunTime
                .getText().toString());

        return bundle;
    }

    private void setConfigs(Bundle bundle) {
        mEtServerIp
                .setText(bundle.getString(WifiSdioTestService.KEY_SERVER_IP));
        String ssid = bundle.getString(WifiSdioTestService.KEY_AP_SSID);
        if (ssid == null || ssid.isEmpty()) {
            mLvAps.clearChoices();
        } else {
            int index = mListAdapter.getPosition(ssid);
            if (-1 == index) {
                mWifiApList.clear();
                mWifiApList.add(bundle
                        .getString(WifiSdioTestService.KEY_AP_SSID));
                mListAdapter.notifyDataSetChanged();
                index = 0;
            }
            mLvAps.setItemChecked(index, true);
        }
        mEtUsername.setText(bundle
                .getString(WifiSdioTestService.KEY_AP_USERNAME));
        mEtPassword.setText(bundle
                .getString(WifiSdioTestService.KEY_AP_PASSWORD));
        mEtIperfTime.setText(bundle
                .getString(WifiSdioTestService.KEY_IPERF_TIME));
        mEtIperfStream.setText(bundle
                .getString(WifiSdioTestService.KEY_IPERF_STREAM));
        mEtIperfWindow.setText(bundle
                .getString(WifiSdioTestService.KEY_IPERF_WINDOW));
        mEtIperfMss.setText(bundle
                .getString(WifiSdioTestService.KEY_IPERF_MSS));
        mEtLastRunCount.setText(bundle
                .getString(WifiSdioTestService.KEY_LASTRUN_COUNT));
        mEtLastRunTime.setText(bundle
                .getString(WifiSdioTestService.KEY_LASTRUN_TIME));
        mCbCmdEnable.setChecked(bundle.getBoolean(
                WifiSdioTestService.KEY_DEFAULT_CMD_ENABLE, false));
        mEtCmdEdge.setText(bundle
                .getString(WifiSdioTestService.KEY_DEFAULT_CMD_EDGE));
        mEtCmdDelay.setText(bundle
                .getString(WifiSdioTestService.KEY_DEFAULT_CMD_DELAY));
        updateDefaultCmdStatus(mCbCmdEnable.isChecked());
    }

    private void updateTestConfig() {
        Xlog.v(TAG, "update test config");
        if (null != mService) {
            setConfigs(mService.getConfig());
        }
    }

    private void updateTestUi(boolean bRunning) {
        Xlog.v(TAG, "updateTestUi: " + bRunning);
        mEtServerIp.setEnabled(!bRunning);
        mTvApSelect.setEnabled(!bRunning);
        mLvAps.setEnabled(!bRunning);
        mEtUsername.setEnabled(!bRunning);
        mEtPassword.setEnabled(!bRunning);
        mEtIperfTime.setEnabled(!bRunning);
        mEtIperfStream.setEnabled(!bRunning);
        mEtIperfWindow.setEnabled(!bRunning);
        mEtIperfMss.setEnabled(!bRunning);
        mCbCmdEnable.setEnabled(!bRunning);
        mEtCmdEdge.setEnabled(!bRunning & mCbCmdEnable.isChecked());
        mEtCmdDelay.setEnabled(!bRunning & mCbCmdEnable.isChecked());
        mBtStart.setEnabled(!bRunning);
        mBtStop.setEnabled(bRunning);
        mEtLastRunCount.setEnabled(!bRunning);
        mEtLastRunTime.setEnabled(!bRunning);
    }

    private boolean validateInput() {
        String ip = mEtServerIp.getText().toString();
        Matcher mat = IP_PATTERN.matcher(mEtServerIp.getText().toString());
        if (!mat.matches()) {
            Toast.makeText(this, R.string.wifi_sdio_toast_ip_error,
                    Toast.LENGTH_SHORT).show();
            return false;
        }
        int pos = mLvAps.getCheckedItemPosition();
        if (pos < 0 || pos >= mWifiApList.size()) {
            Toast.makeText(this, R.string.wifi_sdio_toast_no_select_ap,
                    Toast.LENGTH_SHORT).show();
            return false;
        }
        try {
            if (Long.parseLong(mEtIperfTime.getText().toString()) <= 0
                    || Integer.parseInt(mEtIperfStream.getText().toString()) <= 0
                    || mEtIperfWindow.getText().toString().isEmpty()
                    || Integer.parseInt(mEtIperfMss.getText().toString()) <= 0) {
                throw new NumberFormatException();
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, R.string.wifi_sdio_toast_iperf_config_error,
                    Toast.LENGTH_SHORT).show();
            return false;
        }
        int value = 0;
        try {
            value = Integer.parseInt(mEtCmdEdge.getText().toString());
            if (value < 0 || value > 1) {
                throw new NumberFormatException();
            }
            value = Integer.parseInt(mEtCmdDelay.getText().toString());
            if (value < 0 || value > 32) {
                throw new NumberFormatException();
            }
        } catch (NumberFormatException e) {
            if (mCbCmdEnable.isChecked()) {
                Toast.makeText(this,
                        R.string.wifi_sdio_toast_default_cmd_value_error,
                        Toast.LENGTH_SHORT).show();
                return false;
            }
        }
        if (null == mScanResultList) {
            Toast.makeText(this, R.string.wifi_sdio_toast_scan_error,
                    Toast.LENGTH_SHORT).show();
            return false;
        }
        try {
            if (null == mScanResultList.get(mLvAps.getCheckedItemPosition())) {
                Toast.makeText(this, R.string.wifi_sdio_toast_scan_get_null,
                        Toast.LENGTH_SHORT).show();
                return false;
            }
        } catch (IndexOutOfBoundsException e) {
            return false;
        }
        return true;
    }

    private static int getSecurity(WifiConfiguration config) {
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_PSK)) {
            return WifiAdmService.SECURITY_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_EAP)
                || config.allowedKeyManagement.get(KeyMgmt.IEEE8021X)) {
            return WifiAdmService.SECURITY_EAP;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_PSK)) {
            return WifiAdmService.SECURITY_WAPI_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WAPI_CERT)) {
            return WifiAdmService.SECURITY_WAPI_CERT;
        }
        if (config.wepTxKeyIndex >= 0
                && config.wepTxKeyIndex < config.wepKeys.length
                && config.wepKeys[config.wepTxKeyIndex] != null) {
            return WifiAdmService.SECURITY_WEP;
        }
        return (config.wepKeys[0] != null) ? WifiAdmService.SECURITY_WEP
                : WifiAdmService.SECURITY_NONE;
    }

    private static int getSecurity(ScanResult result) {
        int security = WifiAdmService.SECURITY_NONE;
        if (result.capabilities.contains(WIFI_CAPAB_WAPI_PSK)) {
            security = WifiAdmService.SECURITY_WAPI_PSK;
        } else if (result.capabilities.contains(WIFI_CAPAB_WAPI_CERT)) {
            security = WifiAdmService.SECURITY_WAPI_CERT;
        } else if (result.capabilities.contains(WIFI_CAPAB_WEP)) {
            security = WifiAdmService.SECURITY_WEP;
        } else if (result.capabilities.contains(WIFI_CAPAB_PSK)) {
            security = WifiAdmService.SECURITY_PSK;
        } else if (result.capabilities.contains(WIFI_CAPAB_EAP)) {
            security = WifiAdmService.SECURITY_EAP;
        }
        return security;
    }

    private final ServiceConnection mServiceCon = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            Xlog.v(TAG, "onServiceConnected");
            mService = ((WifiSdioTestService.WifiSdioBinder) service)
                    .getService();
            mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_CONFIG);
            mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_TESTUI);
            if (WifiSdioTestService.STATUS_STOPPED == mService.getTestStatus()) {
                mTvApSelect.performClick();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Xlog.v(TAG, "onServiceDisconnected");
            mService = null;
        }
    };
}
