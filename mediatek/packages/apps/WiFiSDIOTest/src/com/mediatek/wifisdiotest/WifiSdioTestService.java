package com.mediatek.wifisdiotest;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiManager;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.widget.Toast;

import com.mediatek.wifisdiotest.R;
import com.mediatek.xlog.Xlog;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;

/**
 * Service to do Wifi SDIO test, could run test in background
 * 
 * @author mtk54046
 * @version 1.0
 */
public class WifiSdioTestService extends Service {

    private static final String TAG = "EM/WifiSdioTestService";
    private static final int NOTIFICATION_ONGOING = 1500;

    public static final String ACTION_CONFIG_UPDATE_REQUEST = "com.mediatk.wifisdiotest.config_update_request";
    public static final String ACTION_RESULT_UPDATE_REQUEST = "com.mediatek.wifisdiotest.result_update_request";
    public static final String ACTION_TEST_STATUS_CHANGED = "com.mediatek.wifisdiotest.test_status_changed";

    private static final int WIFI_WAIT_MAX_SLEEP = 1000;
    private static final int WIFI_WAIT_TIME = 300000;
    private static final int WIFI_PING_TIME = 10000;

    public static final int STATUS_STOPPED = 0;
    public static final int STATUS_STARTED = 1;
    public static final int STATUS_SDIO_SETTING = 2;
    public static final int STATUS_WIFI_NOT_CONNECT = 3;
    public static final int STATUS_WIFI_CONNECTIONG = 4;
    public static final int STATUS_WIFI_CONNECTED = 5;
    public static final int STATUS_WIFI_PING_PASS = 6;
    public static final int STATUS_IPERF_BEFORE_TEST = 7;
    public static final int STATUS_IPERF_TESTING = 8;
    public static final int STATUS_IPERF_AFTER_TEST = 9;
    public static final int STATUS_SDIO_CHECKING = 10;

    public static final String KEY_TEST_STATUS = "test_status";
    public static final String KEY_SERVER_IP = "server_ip";
    public static final String KEY_AP_SSID = "ap_ssid";
    public static final String KEY_AP_USERNAME = "ap_user_name";
    public static final String KEY_AP_PASSWORD = "ap_password";
    public static final String KEY_AP_SECURITY = "ap_security";
    public static final String KEY_IPERF_TIME = "iperf_time";
    public static final String KEY_IPERF_STREAM = "iperf_stream";
    public static final String KEY_IPERF_WINDOW = "iperf_window";
    public static final String KEY_IPERF_MSS = "iperf_mss";
    public static final String KEY_DEFAULT_CMD_ENABLE = "default_cmd_enable";
    public static final String KEY_DEFAULT_CMD_EDGE = "default_cmd_edge";
    public static final String KEY_DEFAULT_CMD_DELAY = "default_cmd_delay";
    public static final String KEY_RESULT_CMD_EDGE = "cmd_edge";
    public static final String KEY_RESULT_CMD_DELAY = "cmd_delay";
    public static final String KEY_RESULT_READ_EDGE = "read_edge";
    public static final String KEY_RESULT_READ_DELAY = "read_delay";
    public static final String KEY_RESULT_WRITE_EDGE = "write_edge";
    public static final String KEY_RESULT_WRITE_DELAY = "write_delay";
    public static final String KEY_RESULT_PATH = "file_path";
    public static final String KEY_LASTRUN_COUNT = "lrun_count";
    public static final String KEY_LASTRUN_TIME = "lrun_time";

    private static final String ECHO = "echo ";
    private static final String REDIRECT = " > ";

    private static final long DEFAULT_IPERF_TIME = 10;
    private static final int DEFAULT_IPERF_STREAM = 1;
    private static final String DEFAULT_IPERF_WINDOW = "1M";
    private static final int DEFAULT_IPERF_MSS = 1460;
    private static final int DEFAULT_INT_VALUE = 0;
    // [FIXME] JUJU to control the last run time and count
    private static final int DEFAULT_LASTRUN_COUNT = 3;
    private static final int DEFAULT_LASTRUN_TIME = 600;

    private static final String HANDLER_THREAD_NAME = "Wifi SDIO Test";
    private static final String LOG_FILE_NAME_PATTERN = "'WifiSDIOTest_'MM_dd_HH_mm_ss'.txt'";
    private static final String TEST_ERROR = "TEST ERROR ";

    private static final int HANDLER_ID_STOP = 101;
    private static final int HANDLER_ID_UPDATE_CONFIG = 102;
    private static final int HANDLER_ID_UPDATE_RESULT = 103;
    private static final int HANDLER_ID_UPDATE_STATUS = 104;
    private static final int HANDLER_ID_TOAST = 105;

    private static final String FILE_MSDC_TUNE = "/proc/msdc_tune";
    private static final String FILE_MSDC_FLAG = "/proc/msdc_tune_flag";
    private static final String MSDC_COMMAND_FORMAT = "%x %x %x";

    public static final int TEST_LOOP_COUNT = 224;
    public static final int TEST_ONE_ROUND = 64;
    public static final int TEST_COMMAND_ROUND = TEST_ONE_ROUND;
    public static final int TEST_DATA_READ_ROUND = TEST_ONE_ROUND*2;
    public static final int TEST_DATA_WRITE_ROUND = TEST_ONE_ROUND*3;
    public static final int TEST_4CMD_ROUND = TEST_DATA_WRITE_ROUND;    
    public static int TEST_5CMD_INTER_ROUND = TEST_LOOP_COUNT - DEFAULT_LASTRUN_COUNT;
    public static final int TEST_HALF_ROUND = 32;
    
    private static final String PING_PRE = "ping -c 1 ";
    private static final String IPERF_PRE = "iperf -i 1 -c ";
    private static final String CAT = "cat ";
    private static final String RESULT_E = "E ";
    private static final String RESULT_1 = "1 ";
    private static final String RESULT_0 = "0 ";
    private static final String RESULT_0C = "C ";
    private static final String RESULT_0R = "R ";
    private static final String RESULT_0W = "W ";
    private static final String RESULT_N = "N ";
    private static final String RESULT_I = "I ";
    private static final byte BYTE_0 = '0';
    private static final byte BYTE_1 = '1';
    private static final byte BYTE_C = 'C';
    private static final byte BYTE_R = 'R';
    private static final byte BYTE_W = 'W';
    private static final String RESULT_FILE_TITLE = "WiFi SDIO Test Result:";
    private static final String RESULT_FILE_LEGEND = "1: CRC correct, 0: CRC error, E: Other error occurs";
    private static final String TEST_FAIL_HEADER = "Test fail: ";
    private static final String IPERF_PARAM_TIME = " -t ";
    private static final String IPERF_PARAM_STREAM = " -P ";
    private static final String IPERF_PARAM_WINDOW = " -w ";
    private static final String IPERF_PARAM_MSS = " -M ";
    private static final int RADIX_16 = 16;
    private static final int WIFI_AUTO_CONNECT_TIMEOUT = 2000;
    private static final int BYTE_BUFFER_SIZE = 1024;
    private static final String SHELL_SH = "sh";
    private static final String SHELL_PARAM = "-c";
    private static final String ERROR_OCCURS_WHEN = " when ";
    private static final int MSDC_CMD_INDEX_1 = 1;
    private static final int MSDC_CMD_INDEX_2 = 2;
    private static final int MSDC_CMD_INDEX_3 = 3;
    private static final int MSDC_CMD_INDEX_4 = 4;
    private static final int MSDC_CMD_INDEX_5 = 5;

    public static int sCurrentTestLoop = 0;
    private static int sLocalRetry = 0;
    private static boolean s4CMDRoundFlag = false;
    private static boolean s5CMDInterRoundFlag = false;
    private static boolean sCMDErrorOccur = false;
    private static volatile int sWifiState = WifiManager.WIFI_STATE_UNKNOWN;
    private static volatile DetailedState sWifiDetailedState = DetailedState.DISCONNECTED;
    private static volatile int sStatus = STATUS_STOPPED;
    private static String sServerIp = null;
    private static String sApSsid = null;
    private static String sApUsername = null;
    private static String sApPassword = null;
    private static int sApSecurity = WifiAdmService.SECURITY_NONE;
    private static long sIperfTime = DEFAULT_IPERF_TIME;
    private static int sIperfStream = DEFAULT_IPERF_STREAM;
    private static String sIperfWindow = DEFAULT_IPERF_WINDOW;
    private static int sIperfMss = DEFAULT_IPERF_MSS;
    private static boolean sCmdDefaultEnable = false;
    public static volatile int sCmdEdge = DEFAULT_INT_VALUE;
    public static volatile int sCmdDelay = DEFAULT_INT_VALUE;
    public static volatile int sCmdIntDelay = DEFAULT_INT_VALUE;
    public static volatile int sReadEdge = DEFAULT_INT_VALUE;
    public static volatile int sReadDelay = DEFAULT_INT_VALUE;
    public static volatile int sWriteEdge = DEFAULT_INT_VALUE;
    public static volatile int sWriteDelay = DEFAULT_INT_VALUE;
    private static String sFilePath = null;
    private final WifiSdioBinder mBinder = new WifiSdioBinder();
    private HandlerThread mTestThread = null;
    private TestHandler mTestHandler = null;
    private WifiManager mWifiManager = null;
    public static ByteBuffer sResultBuffer = ByteBuffer
            .allocate(TEST_LOOP_COUNT);
    private static ByteBuffer sResultTFBuffer = ByteBuffer
            .allocate(TEST_LOOP_COUNT);
    private static int INDEX_COMMON_RESULT_BUFFER = 0;
    private static int INDEX_BINARY_RESULT_BUFFER = 1;
    // 	[FIXME] JUJU to control the last run time and count
    private static int sLastRunCount = DEFAULT_LASTRUN_COUNT;
    private static int sLastRunTime = DEFAULT_LASTRUN_TIME;
    private final BroadcastReceiver mWifiReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                sWifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                        WifiManager.WIFI_STATE_UNKNOWN);
                Xlog.v(TAG, "Wifi state changed: " + sWifiState);
            } else if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
                NetworkInfo info = (NetworkInfo) intent
                        .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
                sWifiDetailedState = info.getDetailedState();
                Xlog.v(TAG, "NETWORK_STATE_CHANGED_ACTION: "
                        + sWifiDetailedState);
            }
        }
    };

    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case HANDLER_ID_STOP:
                stopTestRequest();
                break;
            case HANDLER_ID_UPDATE_CONFIG:
                sendUpdateConfigRequest();
                break;
            case HANDLER_ID_UPDATE_RESULT:
                sendUpdateResultRequest();
                break;
            case HANDLER_ID_UPDATE_STATUS:
                sendUpdateStatusRequest();
                break;
            case HANDLER_ID_TOAST:
                if (STATUS_STOPPED != sStatus) {
                    showToast(msg.obj.toString());
                }
                break;
            default:
                break;
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (STATUS_STOPPED == sStatus) {
            Intent notificationIntent = new Intent(this, WifiSdioTestTab.class);
            PendingIntent pendingIntent = PendingIntent.getActivity(this, 0,
                    notificationIntent, 0);
            Notification.Builder builder = new Notification.Builder(this);
            builder.setContentTitle(getText(R.string.wifi_sdio_noti_title));
            builder.setContentText(getText(R.string.wifi_sdio_noti_message));
            builder.setContentIntent(pendingIntent);
            builder.setSmallIcon(R.drawable.ic_launcher);
            Notification noti = builder.getNotification();
            startForeground(NOTIFICATION_ONGOING, noti);
            resetValues();
            parseConfigIntent(intent);
            startTest();
        } else {
            Toast.makeText(this, R.string.wifi_sdio_toast_test_running,
                    Toast.LENGTH_SHORT).show();
        }
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        resetValues();
        super.onDestroy();
    }

    @Override
    public void onRebind(Intent intent) {
        super.onRebind(intent);
    }

    private void sendUpdateResultRequest() {
        Intent requestIntent = new Intent(ACTION_RESULT_UPDATE_REQUEST);
        sendBroadcast(requestIntent);
    }

    private void sendUpdateConfigRequest() {
        Intent requestIntent = new Intent(ACTION_CONFIG_UPDATE_REQUEST);
        sendBroadcast(requestIntent);
    }

    private void sendUpdateStatusRequest() {
        Intent changedIntent = new Intent(ACTION_TEST_STATUS_CHANGED);
        changedIntent.putExtra(KEY_TEST_STATUS, sStatus);
        sendBroadcast(changedIntent);
    }

    private void parseConfigIntent(Intent intent) {
        if (null != intent) {
            Bundle bundle = intent.getExtras();
            sServerIp = bundle.getString(KEY_SERVER_IP);
            sApSsid = bundle.getString(KEY_AP_SSID);
            sApUsername = bundle.getString(KEY_AP_USERNAME);
            sApPassword = bundle.getString(KEY_AP_PASSWORD);
            sApSecurity = bundle.getInt(KEY_AP_SECURITY);
            sIperfTime = Long.parseLong(bundle.getString(KEY_IPERF_TIME));
            sIperfStream = Integer.parseInt(bundle.getString(KEY_IPERF_STREAM));
            sIperfWindow = bundle.getString(KEY_IPERF_WINDOW);
            sIperfMss = Integer.parseInt(bundle.getString(KEY_IPERF_MSS));
            sLastRunCount = Integer.parseInt(bundle.getString(KEY_LASTRUN_COUNT));
            sLastRunTime = Integer.parseInt(bundle.getString(KEY_LASTRUN_TIME));
            sCmdDefaultEnable = bundle
                    .getBoolean(KEY_DEFAULT_CMD_ENABLE, false);
            if (sCmdDefaultEnable) {
                sCmdEdge = Integer.parseInt(bundle
                        .getString(KEY_DEFAULT_CMD_EDGE));
                sCmdDelay = Integer.parseInt(bundle
                        .getString(KEY_DEFAULT_CMD_DELAY));
                sCurrentTestLoop = TEST_ONE_ROUND;
            }
        }
    }

    private void showToast(String text) {
        Toast.makeText(WifiSdioTestService.this, text, Toast.LENGTH_SHORT)
                .show();
    }

    private void resetValues() {
        sStatus = STATUS_STOPPED;
        sServerIp = null;
        sApSsid = null;
        sApUsername = null;
        sApPassword = null;
        sApSecurity = WifiAdmService.SECURITY_NONE;
        sIperfTime = DEFAULT_IPERF_TIME;
        sIperfStream = DEFAULT_IPERF_STREAM;
        sIperfWindow = DEFAULT_IPERF_WINDOW;
        sIperfMss = DEFAULT_IPERF_MSS;
        sCmdEdge = DEFAULT_INT_VALUE;
        sCmdDelay = DEFAULT_INT_VALUE;
        sCmdIntDelay = DEFAULT_INT_VALUE;
        sReadEdge = DEFAULT_INT_VALUE;
        sReadDelay = DEFAULT_INT_VALUE;
        sWriteEdge = DEFAULT_INT_VALUE;
        sWriteDelay = DEFAULT_INT_VALUE;
        sCmdDefaultEnable = false;
        sFilePath = null;
        sCurrentTestLoop = 0;
        sCMDErrorOccur = false;
        s4CMDRoundFlag = false;
        s5CMDInterRoundFlag = false;
        sResultBuffer.clear();
        sResultTFBuffer.clear();
    }

    private void startTest() {
        if (null == mTestThread) {
            IntentFilter wifiFilter = new IntentFilter(
                    WifiManager.WIFI_STATE_CHANGED_ACTION);
            wifiFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
            registerReceiver(mWifiReceiver, wifiFilter);
            mTestThread = new HandlerThread(HANDLER_THREAD_NAME);
            mTestThread.start();
            sStatus = STATUS_STARTED;
            sendUpdateStatusRequest();
            mTestHandler = new TestHandler(mTestThread.getLooper());
            mTestHandler.sendEmptyMessage(TestHandler.TEST_PREPARE);
        }
    }

    private void stopTest() {
        stopForeground(true);
        mTestHandler.removeMessages(TestHandler.TEST_RUNNING);
        mTestThread.quit();
        mTestThread.interrupt();
        mTestThread = null;
        unregisterReceiver(mWifiReceiver);
        stopSelf();
    }

    private class TestHandler extends Handler {

        public static final int TEST_PREPARE = 0;
        public static final int TEST_RUNNING = 1;
        public static final int TEST_STOP = 2;
        private static final long DELAY_TIME = 500;

        public TestHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
        	int retWifitest = 0;
        	byte CurResult;
        	StringBuilder strBuilder = new StringBuilder();            
            switch (msg.what) {
            case TEST_PREPARE:
                if (doWifiTestPrepare()) {
                    createLogFile();
                    sendEmptyMessageDelayed(TEST_RUNNING, DELAY_TIME);
                } else {
                    mHandler
                            .sendMessage(mHandler
                                    .obtainMessage(
                                            HANDLER_ID_TOAST,
                                            getString(R.string.wifi_sdio_toast_enable_tune_fail)));
                    mHandler.sendEmptyMessage(HANDLER_ID_STOP);
                }
                break;
            case TEST_RUNNING:
                if (STATUS_STOPPED == sStatus) {
                    Xlog.v(TAG, "status is stop");
                    break;
                }
                //If any command CRC error occurs, it will go back to 1st CMD round. 
                if(TEST_DATA_READ_ROUND <= sCurrentTestLoop) {
                	if(sCMDErrorOccur) {
                		Xlog.w(TAG, "There is a CMD CRC error in the DATA read round!!!");
                		Xlog.w(TAG, "Back to CMD round!!!");
                		sCurrentTestLoop = 0;
                		sCMDErrorOccur = false;
                	}
                }
                calcSdioConfigParam();
                retWifitest = doWifiTest();
                CurResult = sResultBuffer.get(sCurrentTestLoop);
                strBuilder.append(String.format("Holmes [%d] ret: %d retry: %d Res: %c",sCurrentTestLoop,retWifitest,sLocalRetry,CurResult));
                Xlog.v(TAG,strBuilder.toString());
                if((sLocalRetry < 2) && (retWifitest == 0)) { //Local retry < 2 and Return CRC error happened
                	if(((sCurrentTestLoop / TEST_ONE_ROUND) == 0) && (CurResult == BYTE_R ||CurResult == BYTE_W)){
                		//There is a DATA CRC error in the CMD round.
                		//Modify data edge or delay setting
                		if(sLocalRetry == 0) {
                			sReadEdge = (sReadEdge == 0)? 1 : 0;
                		}else {
                			sReadDelay += 1;
                		}                		
                		sLocalRetry++;
                		
                		if(true == s4CMDRoundFlag)
                			s4CMDRoundFlag = false;
                		
                		Xlog.v(TAG,"Holmes There is a DATA CRC error in the CMD round");
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 0) && (CurResult == BYTE_C)){
                		//Next gear, There is a CMD CRC error in the CMD round.
                		sLocalRetry = 0;
                		sCurrentTestLoop++;
                		Xlog.v(TAG,"Holmes There is a CMD CRC error in the CMD round");
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 1) && (CurResult == BYTE_W)){
                		//There is a DATA write CRC error in the DATA read round.
                		//Modify data write delay setting
                		sWriteDelay += 4;
                		
                		if(sWriteDelay>=TEST_HALF_ROUND)
                			sWriteDelay = 0;
                		
                		Xlog.v(TAG,"Holmes There is a DATA write CRC error in the DATA read round");
                		sLocalRetry++;
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 1) && (CurResult == BYTE_C)){
                		//There is a CMD CRC error in the DATA round.
                    	sCMDErrorOccur = true;
                    	
                		sLocalRetry = 0;	
                    	sCurrentTestLoop++;
                    	//
                    	int tmpCMDindex;
                    	tmpCMDindex = sCmdEdge * TEST_HALF_ROUND + sCmdDelay;
                   		sResultBuffer.put(tmpCMDindex, BYTE_0);
                		sResultTFBuffer.put(tmpCMDindex, BYTE_0);
                    	Xlog.v(TAG,"Holmes There is a CMD CRC error in the DATA round "+tmpCMDindex);
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 1) && (CurResult == BYTE_R)){
                		//There is a DATA read CRC error in the DATA read round.
                		//TBD set a flag to tracking
                    	sLocalRetry = 0;	
                sCurrentTestLoop++;
                    	Xlog.v(TAG,"Holmes There is a DATA read CRC error in the DATA read round");
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 2) && (CurResult == BYTE_W)){
                		//There is a DATA write CRC error in the DATA write round.
                    	sLocalRetry = 0;	
                    	sCurrentTestLoop++;
                    	Xlog.v(TAG,"Holmes There is a DATA read CRC error in the DATA read round");
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 2) && (CurResult == BYTE_C)){
                		//There is a DATA write CRC error in the DATA write round.
                		//TBD set a flag to tracking
                    	sLocalRetry = 0;	
                    	sCurrentTestLoop++;
                    	Xlog.v(TAG,"Holmes There is a DATA read CRC error in the DATA read round");
                	}else if(((sCurrentTestLoop / TEST_ONE_ROUND) == 2) && (CurResult == BYTE_R)){
                		//There is a DATA write CRC error in the DATA write round.
                		//TBD Return to DATA Read round
                    	sLocalRetry = 0;	
                    	sCurrentTestLoop++;
                    	Xlog.v(TAG,"Holmes There is a DATA read CRC error in the DATA read round");
                	}else {
                    	sLocalRetry = 0;	
                    	sCurrentTestLoop++;
                    	Xlog.v(TAG,"Holmes Warning~~~Warning~~~Warning~~~Warning~~~Warning~~~Warning~~~");                	
                	}
                }else { //Next gear for no any CRC error appearing
                	sLocalRetry = 0;	
                	sCurrentTestLoop++;
                	Xlog.v(TAG,"Holmes There is a NO any CRC error in this round or too more retry");
                }
                if (sCurrentTestLoop == TEST_LOOP_COUNT) {
                    mHandler.sendEmptyMessage(HANDLER_ID_STOP);
                } else {
                	if((TEST_COMMAND_ROUND <= sCurrentTestLoop) && s4CMDRoundFlag && (!s5CMDInterRoundFlag)) {
                		//sCurrentTestLoop = TEST_5CMD_INTER_ROUND;
                		s5CMDInterRoundFlag = true;
                		sCurrentTestLoop = TEST_LOOP_COUNT -sLastRunCount;
                		sIperfTime = sLastRunTime;
                		Xlog.v(TAG,"Holmes Enter the 5th CMD Internal round.");                		
                	}else if( (TEST_4CMD_ROUND == sCurrentTestLoop) && !s4CMDRoundFlag) {
                		//Set the 4th CMD round flag here!
                		s4CMDRoundFlag = true;
                		sCurrentTestLoop = 0;
                		Xlog.v(TAG,"Holmes Enter the 4th CMD round.");
                	}
                	
                    sendEmptyMessageDelayed(TEST_RUNNING, DELAY_TIME);
                }
                System.gc();
                break;
            default:
                super.handleMessage(msg);
                break;
            }
        }
    }

    /**
     * Wifi SDIO server binder
     * 
     * @author mtk54046
     * @version 1.0
     */
    public class WifiSdioBinder extends Binder {

        /**
         * Binder
         * 
         * @return #WifiSdioTestService
         */
        WifiSdioTestService getService() {
            return WifiSdioTestService.this;
        }
    }

    /**
     * Get test status
     * 
     * @return Current test status
     */
    public int getTestStatus() {
        return sStatus;
    }

    /**
     * Request to stop test, invoked when "stop" button clicked
     */
    public void stopTestRequest() {
        Xlog.v(TAG, "stopTestRequest: " + sStatus);
        if (STATUS_STOPPED != sStatus) {
            sStatus = STATUS_STOPPED;
            mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_STATUS);
            stopTest();
        }
    }

    private void createLogFile() {
        Date date = Calendar.getInstance().getTime();
        SimpleDateFormat dateFormat = new SimpleDateFormat(
                LOG_FILE_NAME_PATTERN);
        String name = dateFormat.format(date);
        File logFile = new File(getFilesDir(), name);
        sFilePath = logFile.getAbsolutePath();
        PrintWriter writer = null;
        try {
            writer = new PrintWriter(new FileWriter(logFile));
            writer.println(date.toGMTString());
            writer.println(RESULT_FILE_TITLE);
            writer.println(RESULT_FILE_LEGEND);
            writer.println();
            writer.flush();
            mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
        } catch (FileNotFoundException e) {
            Xlog.w(TAG, "createLogFile FileNotFoundException: "
                    + e.getMessage());
        } catch (IOException e) {
            Xlog.w(TAG, "createLogFile IOException: " + e.getMessage());
        } finally {
            if (null != writer) {
                writer.close();
                writer = null;
            }
        }
    }

    private void appendLog(String log, boolean newLine) {
        PrintWriter writer = null;
        if (null == sFilePath) {
            return;
        }
        try {
            writer = new PrintWriter(new FileWriter(sFilePath, true));
            writer.print(log);
            if (newLine) {
                writer.println();
            }
            writer.flush();
        } catch (FileNotFoundException e) {
            Xlog.w(TAG, "appendLog FileNotFoundException: " + e.getMessage());
        } catch (IOException e) {
            Xlog.w(TAG, "appendLog IOException: " + e.getMessage());
        } finally {
            if (null != writer) {
                writer.close();
                writer = null;
            }
        }
    }

    private void fillResult(int index,byte result) {
    	if(INDEX_COMMON_RESULT_BUFFER == index) {
        sResultBuffer.put(sCurrentTestLoop, result);
    	}else if(INDEX_BINARY_RESULT_BUFFER == index) {
    		sResultTFBuffer.put(sCurrentTestLoop, result);
    	}
        
    }

    boolean doWifiTestPrepare() {
    	int index;
        if (!disableWifi()) {
            Xlog.w(TAG, "doWifiTestPrepare: init disableWifi fail");
        }
        Xlog.w(TAG, "doWifiTestPrepare: init fill the pass in the TFBuffer");
        for(index=0;index<TEST_LOOP_COUNT;index++){
        	sResultTFBuffer.put(index, BYTE_1);
        }
        StringBuilder commandBuilder = new StringBuilder();
        commandBuilder.append(ECHO);
        commandBuilder.append(String.format(MSDC_COMMAND_FORMAT, 0, 1, 1));
        commandBuilder.append(REDIRECT);
        commandBuilder.append(FILE_MSDC_TUNE);
        Xlog.v(TAG, "doWifiTestPrepare command: " + commandBuilder.toString());
        ShellResult result = runShellCommand(commandBuilder.toString(), false);
        if (0 != result.mExitValue) {
            Xlog.w(TAG, "doWifiTestPrepare: enable msdc tune fail");
        }
        commandBuilder = null;
        return 0 == result.mExitValue;
    }

    private int doWifiTest() {
    	int ret=0;	
        String errorMsg = null;
        do {
            if (!disableWifi()) {
                errorMsg = "init disableWifi fail";
                break;
            }
            if (!applySdioSetting()) {
                errorMsg = "applySdioSetting fail";
                break;
            }
            if (!enableWifi()) {
                errorMsg = "enableWifi fail";
                break;
            }
            if (!connectAp()) {
                errorMsg = "connectAp fail";
                break;
            }
            if (!checkConnection()) {
                errorMsg = "checkConnection fail";
                break;
            }
            if (!runIperfTest()) {
                errorMsg = "runIperfTest fail";
                break;
            }
            if (!disconnectAp()) {
                errorMsg = "disconnectAp fail";
                break;
            }
            if (!disableWifi()) {
                errorMsg = "disableWifi end fail";
                break;
            }
        } while (false);
        if (null == errorMsg) {
            ret = checkErrorFlag(0);
        } else {
            ret = checkErrorFlag(1);
        	if(1 == ret)
        	{	
                mHandler.sendMessage(mHandler.obtainMessage(HANDLER_ID_TOAST,
                        TEST_FAIL_HEADER + errorMsg));
                Xlog.w(TAG, TEST_ERROR + errorMsg + ERROR_OCCURS_WHEN
                        + sCurrentTestLoop);
	            appendLog(RESULT_E,
                        sCurrentTestLoop % TEST_HALF_ROUND == TEST_HALF_ROUND - 1);
	            fillResult(INDEX_COMMON_RESULT_BUFFER,RESULT_E.getBytes()[0]);
	            fillResult(INDEX_BINARY_RESULT_BUFFER,RESULT_E.getBytes()[0]);
            }
        }
        mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
        return ret;//return 1 means no CRC error!!!
    }
    
    public boolean fillSDIOParameter(int cmdEdge, int readEdge, int writeEdge, int cmdDelay, int readDelay, int writeDelay, int cmdIntDelay) {
    	StringBuilder strBuilder = new StringBuilder();
        ShellResult result = null;
        do {
        	strBuilder.append(ECHO);
        	strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_1, cmdEdge, readEdge));
        	strBuilder.append(REDIRECT);
        	strBuilder.append(FILE_MSDC_TUNE);
        	result = runShellCommand(strBuilder.toString(), false);
        	if (0 != result.mExitValue) {
        		break;
        	}
        	strBuilder.delete(0, strBuilder.length());
        	strBuilder.append(ECHO);
        	strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_2, cmdIntDelay, cmdDelay));
        	strBuilder.append(REDIRECT);
        	strBuilder.append(FILE_MSDC_TUNE);
        	result = runShellCommand(strBuilder.toString(), false);
        	if (0 != result.mExitValue) {
        		break;
        	}
        	strBuilder.delete(0, strBuilder.length());
        	strBuilder.append(ECHO);
        	strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_3, readDelay, readDelay));
        	strBuilder.append(REDIRECT);
        	strBuilder.append(FILE_MSDC_TUNE);
        	result = runShellCommand(strBuilder.toString(), false);
        	if (0 != result.mExitValue) {
        		break;
        	}
        	strBuilder.delete(0, strBuilder.length());
        	strBuilder.append(ECHO);
        	strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_4, readDelay, readDelay));
        	strBuilder.append(REDIRECT);
        	strBuilder.append(FILE_MSDC_TUNE);
        	result = runShellCommand(strBuilder.toString(), false);              
        	if (0 != result.mExitValue) {
        		break;
        	}
        	strBuilder.delete(0, strBuilder.length());
        	strBuilder.append(ECHO);
        	strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_5, writeEdge, writeDelay));
        	strBuilder.append(REDIRECT);
        	strBuilder.append(FILE_MSDC_TUNE);
        	result = runShellCommand(strBuilder.toString(), false);
        } while (false);
        strBuilder = null;
        return 0 == result.mExitValue;
    }
    
    private boolean applySdioSetting() {
        StringBuilder strBuilder = new StringBuilder();
        ShellResult result = null;
        do {
            if (sCurrentTestLoop < TEST_COMMAND_ROUND) {//1st CMD round
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_1, sCmdEdge, sReadEdge));//Holmes 2012/11/11
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_2, 0, sCmdDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {	//Holmes 2012/11/11
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_3, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {	//Holmes 2012/11/11
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_4, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);              
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_5, sWriteEdge, sWriteDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);       
            } else if (sCurrentTestLoop < TEST_DATA_READ_ROUND) {//2nd DATA round
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_1, sCmdEdge, sReadEdge));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_2, 0, sCmdDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_3, sReadDelay, sReadDelay)); 
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_4, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_5, sWriteEdge, sWriteDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);

            } else if (sCurrentTestLoop < TEST_DATA_WRITE_ROUND) {
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_1, sCmdEdge, sReadEdge));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_2, 0, sCmdDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_3, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_4, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_5, sWriteEdge, sWriteDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
            }else if(sCurrentTestLoop < TEST_LOOP_COUNT) {
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_1, sCmdEdge, sReadEdge));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_2, sCmdIntDelay, sCmdDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_3, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_4, sReadDelay, sReadDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
                if (0 != result.mExitValue) {
                    break;
                }
                strBuilder.delete(0, strBuilder.length());
                strBuilder.append(ECHO);
                strBuilder.append(String.format(MSDC_COMMAND_FORMAT, MSDC_CMD_INDEX_5, sWriteEdge, sWriteDelay));
                strBuilder.append(REDIRECT);
                strBuilder.append(FILE_MSDC_TUNE);
                result = runShellCommand(strBuilder.toString(), false);
            }else {
            	 Xlog.w(TAG, "Warning~~~~Warning~~~~Warning~~~~Warning~~~~"+sCurrentTestLoop);
            }
        } while (false);
        strBuilder = null;
        return 0 == result.mExitValue;
    }

    protected boolean disableWifi() {
        Intent intent = new Intent(WifiAdmService.WIFI_ADM_ACTION);
        intent
                .putExtra(WifiAdmService.KEY_OPT,
                        WifiAdmService.WIFI_OPT_DISABLE);
        startService(intent);
        return waitWifiReady(WifiManager.WIFI_STATE_DISABLED);
    }

    boolean enableWifi() {
        Intent intent = new Intent(WifiAdmService.WIFI_ADM_ACTION);
        intent.putExtra(WifiAdmService.KEY_OPT, WifiAdmService.WIFI_OPT_ENABLE);
        startService(intent);
        return waitWifiReady(WifiManager.WIFI_STATE_ENABLED);
    }

    private boolean isConnecting() {
        SupplicantState state = mWifiManager.getConnectionInfo()
                .getSupplicantState();
        Xlog.v(TAG, "SupplicantState: " + state);
        switch (state) {
        case AUTHENTICATING:
        case ASSOCIATING:
        case ASSOCIATED:
        case FOUR_WAY_HANDSHAKE:
        case GROUP_HANDSHAKE:
        case COMPLETED:
            return true;
        case DISCONNECTED:
        case INTERFACE_DISABLED:
        case INACTIVE:
        case SCANNING:
        case DORMANT:
        case UNINITIALIZED:
        case INVALID:
        default:
            return false;
        }
    }

    private boolean connectAp() {
        boolean bAutoConn = false;
        for (int i = 0; i < WIFI_AUTO_CONNECT_TIMEOUT / WIFI_WAIT_MAX_SLEEP
                && STATUS_STOPPED != sStatus; i++) {
            SystemClock.sleep(WIFI_WAIT_MAX_SLEEP);
            if (isConnecting()) {
                Xlog.v(TAG, "Wifi auto connect");
                bAutoConn = true;
                break;
            }
        }
        if (bAutoConn) {
            if (sCmdDefaultEnable) {
                if (sCurrentTestLoop == TEST_ONE_ROUND) {
                    bAutoConn = false;
                }
            } else {
                if (sCurrentTestLoop == 0) {
                    bAutoConn = false;
                }
            }
        }
        if (!bAutoConn) {
            Xlog.v(TAG, "Wifi manual connect");
            Intent intent = new Intent(WifiAdmService.WIFI_ADM_ACTION);
            intent.putExtra(WifiAdmService.KEY_OPT,
                    WifiAdmService.WIFI_OPT_CONNECT);
            intent.putExtra(WifiAdmService.KEY_WIFI_CONFIG_SSID, sApSsid);
            intent
                    .putExtra(WifiAdmService.KEY_WIFI_CONFIG_SECTYPE,
                            sApSecurity);
            intent.putExtra(WifiAdmService.KEY_WIFI_CONFIG_IDENTITY,
                    sApUsername);
            intent.putExtra(WifiAdmService.KEY_WIFI_CONFIG_PASSWORD,
                    sApPassword);
            startService(intent);
        }
        return waitWifiConnect(DetailedState.CONNECTED);
    }

    private boolean disconnectAp() {
        Intent intent = new Intent(WifiAdmService.WIFI_ADM_ACTION);
        intent.putExtra(WifiAdmService.KEY_OPT,
                WifiAdmService.WIFI_OPT_DISCONNECT);
        startService(intent);
        return true;
        //return waitWifiConnect(DetailedState.DISCONNECTED);
    }

    private boolean checkConnection() {
        boolean result = false;
        /*
        for (int i = 0; i < WIFI_PING_TIME / WIFI_WAIT_MAX_SLEEP
                && STATUS_STOPPED != sStatus; i++) {
            SystemClock.sleep(WIFI_WAIT_MAX_SLEEP);
            ShellResult shellResult = runShellCommand(PING_PRE + sServerIp, false);
            result = 0 == shellResult.mExitValue;
            shellResult = null;
            if (result) {
                break;
            }
        }
        */
        try{
            if(InetAddress.getByName(sServerIp).isReachable(WIFI_WAIT_MAX_SLEEP*WIFI_PING_TIME)){
                Xlog.v(TAG, " ip connect"); 
                result = true;
            }
        } catch(UnknownHostException e) {
             Xlog.v(TAG, "ip UnknownHostException: " + e.getMessage());
        } catch(IOException e) {
            Xlog.v(TAG, "ip IOException :" + e.getMessage());
        }
        return result;
    }

    private boolean runIperfTest() {
        StringBuilder shellParam = new StringBuilder();
        shellParam.append(IPERF_PRE);
        shellParam.append(sServerIp);
        shellParam.append(IPERF_PARAM_TIME);
        shellParam.append(sIperfTime);
        shellParam.append(IPERF_PARAM_STREAM);
        shellParam.append(sIperfStream);
        shellParam.append(IPERF_PARAM_WINDOW);
        shellParam.append(sIperfWindow);
        
        if(sCurrentTestLoop < TEST_ONE_ROUND || sCurrentTestLoop > TEST_4CMD_ROUND) {
        	  shellParam.append(IPERF_PARAM_MSS);
            shellParam.append(sIperfMss);  // only for cmd , use input mss value, to enlarge cmd data.
        }
        ShellResult result = runShellCommand(shellParam.toString(), false);
        shellParam = null;
        return 0 == result.mExitValue;
    }

    private int checkErrorFlag(int mode) {
    	int ret=0;	
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append(CAT);
        strBuilder.append(FILE_MSDC_FLAG);
        Xlog.v(TAG, "checkErrorFlag command: " + strBuilder.toString());
        ShellResult shellResult = runShellCommand(strBuilder.toString(), true);
        Xlog.v(TAG, "checkErrorFlag result: " + shellResult.mOutput);
        strBuilder = null;
        String result = RESULT_E;
        String TFresult = RESULT_0;
        try {
        	if(null == shellResult.mOutput){
        		TFresult = RESULT_1;
        		result = RESULT_N;
        		ret = 0;
        		Xlog.w(TAG, "checkErrorFlag NULL output!!!");
            	if(mode == 0)
            	{
            		appendLog(result,sCurrentTestLoop % TEST_HALF_ROUND == TEST_HALF_ROUND - 1);
            		fillResult(INDEX_COMMON_RESULT_BUFFER,result.getBytes()[0]);
            		fillResult(INDEX_BINARY_RESULT_BUFFER,TFresult.getBytes()[0]);
            	}else if((mode == 1) && (ret == 0))
            	{
            		appendLog(result,sCurrentTestLoop % TEST_HALF_ROUND == TEST_HALF_ROUND - 1);
            		fillResult(INDEX_COMMON_RESULT_BUFFER,result.getBytes()[0]);
            		fillResult(INDEX_BINARY_RESULT_BUFFER,TFresult.getBytes()[0]);
                }
                strBuilder = null;
                result = null;
                mode = 3; // avoid null pointer in finally.
                return ret;
        	}
            int value = Integer.parseInt(shellResult.mOutput.substring(2)
                    .trim(), RADIX_16);
            if (0 == value) {
                result = RESULT_1;
                TFresult = RESULT_1;
                ret = 1;//Means NO CRC error
            } else if ((0x1 & value) == 0x1) {
                result = RESULT_0C;
                
                if(sCurrentTestLoop < TEST_COMMAND_ROUND) //In the CMD round, it means FAIL!!!
                	TFresult = RESULT_0;
                else
                	TFresult = RESULT_1;
                
                ret = 0;//Means having CRC error
            }else if ((0x50 & value) == 0x50) {
                result = RESULT_0W;

                if(sCurrentTestLoop < TEST_COMMAND_ROUND) //In the CMD round, it means PASS!!!
                	TFresult = RESULT_1;
                else
                	TFresult = RESULT_0;
                
                ret = 0;//Means having CRC error
            }else if ((0x90 & value) == 0x90) {
                result = RESULT_0R;
                
                if(sCurrentTestLoop < TEST_COMMAND_ROUND) //In the CMD round, it means PASS!!!
                	TFresult = RESULT_1;
                else
                	TFresult = RESULT_0;
                
                ret = 0;//Means having CRC error
            }else {
                result = RESULT_0;
                TFresult = RESULT_0;
                ret = 0;//Means having CRC error
            }
        } catch (NumberFormatException e) {
            Xlog.w(TAG, "checkErrorFlag NumberFormatException: "
                    + e.getMessage());
            result = RESULT_N;
        } catch (IndexOutOfBoundsException e) {
            Xlog.w(TAG, "checkErrorFlag IndexOutOfBoundsException: "
                    + e.getMessage());
            result = RESULT_I;
        } finally {
        	if(mode == 0)
        	{
        		appendLog(result,sCurrentTestLoop % TEST_HALF_ROUND == TEST_HALF_ROUND - 1);
        		fillResult(INDEX_COMMON_RESULT_BUFFER,result.getBytes()[0]);
        		fillResult(INDEX_BINARY_RESULT_BUFFER,TFresult.getBytes()[0]);
        	}else if((mode == 1) && (ret == 0))
        	{
        		appendLog(result,sCurrentTestLoop % TEST_HALF_ROUND == TEST_HALF_ROUND - 1);
        		fillResult(INDEX_COMMON_RESULT_BUFFER,result.getBytes()[0]);
        		fillResult(INDEX_BINARY_RESULT_BUFFER,TFresult.getBytes()[0]);
            }
            strBuilder = null;
            result = null;
        }
        return ret;
    }

    private boolean waitWifiReady(int wifiState) {
        boolean bSuccess = false;
        boolean bHandleUnknown = wifiState == WifiManager.WIFI_STATE_DISABLED;
        int i = 0;
        for (; i < WIFI_WAIT_TIME / WIFI_WAIT_MAX_SLEEP
                && STATUS_STOPPED != sStatus; i++) {
            SystemClock.sleep(WIFI_WAIT_MAX_SLEEP);
            if (sWifiState == wifiState) {
                bSuccess = true;
            } else if (sWifiState == WifiManager.WIFI_STATE_UNKNOWN) {
                bSuccess = bHandleUnknown;
            } else {
                continue;
            }
            Xlog.v(TAG, "waitWifiReady " + sWifiState);
            break;
        }
        if (i == WIFI_WAIT_TIME / WIFI_WAIT_MAX_SLEEP) {
            Xlog.w(TAG, "waitWifiReady time out");
        } else if (STATUS_STOPPED == sStatus) {
            Xlog.w(TAG, "waitWifiReady stopped");
        }
        return bSuccess;
    }

    private boolean waitWifiConnect(DetailedState connected) {
        boolean bSuccess = false;
        int i = 0;
        for (; i < WIFI_WAIT_TIME / WIFI_WAIT_MAX_SLEEP
                && STATUS_STOPPED != sStatus; i++) {
            SystemClock.sleep(WIFI_WAIT_MAX_SLEEP);
            if (sWifiDetailedState == connected) {
                bSuccess = true;
                break;
            }
        }
        if (i == WIFI_WAIT_TIME / WIFI_WAIT_MAX_SLEEP) {
            Xlog.w(TAG, "waitWifiConnect time out");
        } else if (STATUS_STOPPED == sStatus) {
            Xlog.w(TAG, "waitWifiConnect stopped");
        }
        return bSuccess;
    }

    private void calcSdioConfigParam() {
    	int excep = 0;
        int group, group_mod, current, c;

    	while((BYTE_0 == sResultTFBuffer.get(sCurrentTestLoop)) && (0 == sLocalRetry) ) {
    		sCurrentTestLoop++;
    		excep++;
    		if(excep > TEST_LOOP_COUNT) {
    			Xlog.w(TAG, "Error!!! Error!!! Error!!! Error!!! Error!!!");
    			break;
    		}    			
    	}
    	
        group = sCurrentTestLoop / TEST_ONE_ROUND;
        group_mod = sCurrentTestLoop % TEST_ONE_ROUND;
        current = sCurrentTestLoop % TEST_HALF_ROUND;
        c = sCurrentTestLoop / TEST_HALF_ROUND % 2;
        /*
         * if (0 == current) { switch (group) { case 0: sCmdEdge = 0 + c; break;
         * case 1: if (!sCmdDefaultEnable) { sCmdEdge = calcEdge(0); } sReadEdge
         * = 0 + c; break; case 2: // sCmdEdge = 0; sReadEdge = calcEdge(1);
         * sWriteEdge = 0 + c; break; default: break; } } switch (group) { case
         * 0: sCmdDelay = current; break; case 1: if (!sCmdDefaultEnable) { int
         * cmdBackup = sCmdDelay; sCmdDelay = calcDelay(0); if (cmdBackup !=
         * sCmdDelay) { appendLog("final cmd edge : " + sCmdEdge, true);
         * appendLog("final cmd delay : " + sCmdDelay, true); } } sReadDelay =
         * current; break; case 2: // sCmdDelay = calcDelay(0); int readBackup =
         * sReadDelay; sReadDelay = calcDelay(1); if (readBackup != sReadDelay)
         * { appendLog("final data edge: " + sReadEdge, true);
         * appendLog("final read delay: " + sReadDelay, true); } sWriteDelay =
         * current; break; default: break; }
         */
        switch (group) {
        case 0:
            if (0 == current) {
                sCmdEdge = 0 + c;
                if(0 == group_mod) {//Add this code for back to CMD round, use the better DATA window
                	int[] result = calcFinalValue(1);
                    sReadEdge = result[0];
                    sReadDelay = result[1];
                    result = calcFinalValue(2);
                    sWriteEdge = result[0];
                    sWriteDelay = result[1];                    
                    Xlog.w(TAG, "Holmes Better Read DATA :"+sReadEdge+" "+sReadDelay);
                    Xlog.w(TAG, "Holmes Better Write DATA :"+sWriteEdge+" "+sWriteDelay);
                }
            }
            sCmdDelay = current;
            break;
        case 1:
            if (0 == current) {
                sReadEdge = 0 + c;
                if ((0 == group_mod)&& (!sCmdDefaultEnable)) {
                    int[] result = calcFinalValue(0);
                    sCmdEdge = result[0];
                    sCmdDelay = result[1];
                    appendLog("final cmd edge : " + sCmdEdge, true);
                    appendLog("final cmd delay : " + sCmdDelay, true);
                    Xlog.w(TAG, "Holmes Better CMD :"+sCmdEdge+" "+sCmdDelay);
                    sCMDErrorOccur = false;                    
                }
            }
            sReadDelay = current;
            break;
        case 2:
            if ((0 == current) && (0 == group_mod))  {
                int[] result = calcFinalValue(1);
                sReadEdge = result[0];
                sReadDelay = result[1];
                sWriteEdge = 0 + c;
                appendLog("final data read edge: " + sReadEdge, true);
                appendLog("final read delay: " + sReadDelay, true);
                Xlog.w(TAG, "Holmes Better Read DATA :"+sReadEdge+" "+sReadDelay);
            } else if (0 == current) {
                sWriteEdge = 0 + c;
            }
            sWriteDelay = current;
            break;
        case 3://CMD Internal delay Round
            if ((0 == current) && (0 == group_mod))  {
                int[] result = calcFinalValue(2);
                sWriteEdge = result[0];
                sWriteDelay = result[1];                
                appendLog("final data write edge: " + sWriteEdge, true);
                appendLog("final write delay: " + sWriteDelay, true);
                Xlog.w(TAG, "Holmes Better Write DATA :"+sWriteEdge+" "+sWriteDelay);
                if(s5CMDInterRoundFlag) { //From the 4th CMD round, it need to rescan the better CMD setting
                    int[] result2 = calcFinalValue(0);
                    sCmdEdge = result2[0];
                    sCmdDelay = result2[1];
                    Xlog.w(TAG, "Holmes Better CMD :"+sCmdEdge+" "+sCmdDelay);                    
                }                
            }

            sCmdIntDelay = current;
            break;
        default:
            break;
        }
        mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
    }

    private int calcEdge(int index) {
        int delta = index * TEST_ONE_ROUND;
        int failPos1 = -1;
        int failPos2 = -1;
        int result = 0;
        for (int i = 0 + delta; i < TEST_HALF_ROUND + delta; i++) {
            if (sResultTFBuffer.get(i) == BYTE_0) {
                failPos1 = i - delta;
                break;
            }
        }
        for (int i = TEST_HALF_ROUND + delta; i < TEST_ONE_ROUND + delta; i++) {
            if (sResultTFBuffer.get(i) == BYTE_0) {
                failPos2 = i - TEST_HALF_ROUND - delta;
                break;
            }
        }
        if (failPos1 > 0 && failPos2 == -1) {
            result = 1;
        } else {
            result = 0;
        }
        return result;
    }

    private int calcDelay(int index) {
        int delta = index * TEST_ONE_ROUND;
        int failPos1 = -1;
        int failPos2 = -1;
        int result = 0;
        for (int i = 0 + delta; i < TEST_HALF_ROUND + delta; i++) {
            if (sResultTFBuffer.get(i) == BYTE_0) {
                failPos1 = i - delta;
                break;
            }
        }
        for (int i = TEST_HALF_ROUND + delta; i < TEST_ONE_ROUND + delta; i++) {
            if (sResultTFBuffer.get(i) == BYTE_0) {
                failPos2 = i - TEST_HALF_ROUND - delta;
                break;
            }
        }
        if (-1 == failPos1 && -1 == failPos2) {
            result = (TEST_HALF_ROUND - 1) >> 1;
        } else if (-1 != failPos1 && -1 != failPos2) {
            int min = Math.min(failPos1, failPos2);
            result = min >> 1;
        } else {
            int max = Math.max(failPos1, failPos2);
            result = max >> 1;
        }
        return result;
    }

    private int[] calcFinalValue(int index) {
        int[] result = { 1, 15 };
        int delta = index * TEST_ONE_ROUND;
        int[] findEdge0 = findContiSuccess(delta);
        int[] findEdge1 = findContiSuccess(TEST_HALF_ROUND + delta);
        int length0 = findEdge0[1] - findEdge0[0] + 1;
        int length1 = findEdge1[1] - findEdge1[0] + 1;
        if (length0 <= length1) {
            result[0] = 1;
            result[1] = (findEdge1[1] + findEdge1[0]) >> 1;
        	result[1] = result[1] - delta - TEST_HALF_ROUND; 
        } else {
            result[0] = 0;
            result[1] = (findEdge0[1] + findEdge0[0]) >> 1;
        	result[1] = result[1] - delta;
        }
        return result;
    }

    private int[] findContiSuccess(int indexStart) {
        int[] result = { -1, 0 };
        int index = indexStart;
        int indexEnd = indexStart + TEST_HALF_ROUND;
        ArrayList<int[]> list = new ArrayList<int[]>();
        while (index < indexEnd) {
            if (BYTE_1 == sResultTFBuffer.get(index)) {
                int[] findResult = { -1, 0 };
                list.add(findResult);
                findResult[0] = index;
                findResult[1] = index;
                index++;
                while (index < indexEnd) {
                    if (BYTE_1 == sResultTFBuffer.get(index)) {
                        findResult[1] = index;
                        index++;
                    } else {
                        break;
                    }
                }
            } else {
                index++;
            }
        }
        Iterator<int[]> it = list.iterator();
        int[] temp;
        int length = -1;
        while (it.hasNext()) {
            temp = it.next();
            Xlog.v(TAG, Arrays.toString(temp));
            int lengthTemp = temp[1] + 1 - temp[0];
            if (lengthTemp > length) {
                length = lengthTemp;
                result[0] = temp[0];
                result[1] = temp[1];
            }
        }
        return result;
    }

    class ShellResult {
        int mExitValue = -1;
        String mOutput = null;
    }

    private ShellResult runShellCommand(String command, boolean recordOutput) {
        Xlog.v(TAG, "runShellCommand: " + command);
        ShellResult result = new ShellResult();
        Process process = null;
        DataInputStream reader = null;
        String line = null;
        StringBuilder strBuilder = new StringBuilder();
        byte[] buffer = new byte[BYTE_BUFFER_SIZE];
        int count = 0;
        try {
            process = Runtime.getRuntime().exec(
                    new String[] { SHELL_SH, SHELL_PARAM, command });
            reader = new DataInputStream(process.getInputStream());
            do {
                count = reader.read(buffer);
                if (-1 != count) {
                    line = new String(buffer, 0, count);
                    Xlog.d(TAG, command + ": " + line);
                    if (recordOutput) {
                        strBuilder.append(line);
                        result.mOutput = strBuilder.toString();
                    }
                }
            } while (-1 != count && STATUS_STOPPED != sStatus);
            int exitValue = process.waitFor();
            Xlog.d(TAG, "exit value: " + exitValue);
            result.mExitValue = exitValue;
            result.mOutput = strBuilder.toString();
            buffer = null;
            strBuilder = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
            Xlog.w(TAG, "runShellCommand InterruptedException: "
                    + e.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
            Xlog.w(TAG, "runShellCommand IOException: " + e.getMessage());
        } finally {
            if (null != reader) {
                try {
                    reader.close();
                    reader = null;
                } catch (IOException e) {
                    e.printStackTrace();
                    Xlog.w(TAG, "reader close IOException: " + e.getMessage());
                }
            }
            if (null != process) {
                process.destroy();
                process = null;
            }
        }
        return result;
    }

    public Bundle getConfig() {
        Bundle bundle = new Bundle();
        bundle.putString(KEY_SERVER_IP, sServerIp);
        bundle.putString(KEY_AP_SSID, sApSsid);
        bundle.putString(KEY_AP_USERNAME, sApUsername);
        bundle.putString(KEY_AP_PASSWORD, sApPassword);
        bundle.putInt(KEY_AP_SECURITY, sApSecurity);
        bundle.putString(KEY_IPERF_TIME, String.valueOf(sIperfTime));
        bundle.putString(KEY_IPERF_STREAM, String.valueOf(sIperfStream));
        bundle.putString(KEY_IPERF_WINDOW, sIperfWindow);
        bundle.putString(KEY_IPERF_MSS, String.valueOf(sIperfMss));
        bundle.putString(KEY_LASTRUN_COUNT, String.valueOf(sLastRunCount));
        bundle.putString(KEY_LASTRUN_TIME, String.valueOf(sLastRunTime));
        bundle.putBoolean(KEY_DEFAULT_CMD_ENABLE, sCmdDefaultEnable);
        bundle.putString(KEY_DEFAULT_CMD_EDGE, sCmdDefaultEnable ? String
                .valueOf(sCmdEdge) : "");
        bundle.putString(KEY_DEFAULT_CMD_DELAY, sCmdDefaultEnable ? String
                .valueOf(sCmdDelay) : "");
        Xlog.v(TAG, bundle.toString());
        return bundle;
    }

    public Bundle getResult() {
        Bundle bundle = new Bundle();
        bundle.putInt(KEY_RESULT_CMD_EDGE, sCmdEdge);
        bundle.putInt(KEY_RESULT_CMD_DELAY, sCmdDelay);
        bundle.putInt(KEY_RESULT_READ_EDGE, sReadEdge);
        bundle.putInt(KEY_RESULT_READ_DELAY, sReadDelay);
        bundle.putInt(KEY_RESULT_WRITE_EDGE, sWriteEdge);
        bundle.putInt(KEY_RESULT_WRITE_DELAY, sWriteDelay);
        bundle.putString(KEY_RESULT_PATH, sFilePath);
        Xlog.v(TAG, bundle.toString());
        return bundle;
    }
}
