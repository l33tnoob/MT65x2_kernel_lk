package com.mediatek.wifisdiotest;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.widget.TextView;
import android.os.PowerManager;


import com.mediatek.wifisdiotest.R;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class WifiSdioTestResult extends Activity {

    private static final String TAG = "EM/WifiSdioResult";
    private static final int HANDLER_ID_UPDATE_RESULT = 1002;
    private static final String LINE_BREAK = "\n";

    private TextView mTvCmdEdge = null;
    private TextView mTvCmdDelay = null;
    private TextView mTvReadEdge = null;
    private TextView mTvReadDelay = null;
    private TextView mTvWriteEdge = null;
    private TextView mTvWriteDelay = null;
    private TextView mTvResult = null;
    private TextView mTvResultPath = null;

    private int mOriBackup = 0;
    private WifiSdioTestService mService = null;
    private WifiSdioWakeLock mWakeLock = null;
    
    private final BroadcastReceiver mTestUpdateReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (WifiSdioTestService.ACTION_RESULT_UPDATE_REQUEST.equals(action)) {
                mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
            }
        }
    };
    class WifiSdioWakeLock {
            private PowerManager.WakeLock mScreenWakeLock = null;
 
            /**
             * Acquire screen wake lock
             * 
             * @param context
             *            Getting lock context
             */
            void acquireScreenWakeLock(Context context) {
                Xlog.v(TAG, "Acquiring screen wake lock");
                if (mScreenWakeLock != null) {
                    return;
                }
    
                PowerManager pm = (PowerManager) context
                        .getSystemService(Context.POWER_SERVICE);
    
                mScreenWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                        | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
                // | PowerManager.ON_AFTER_RELEASE, TAG);
                mScreenWakeLock.acquire();
            }
    
            /**
             * Release wake locks
             */
            void release() {
                Xlog.v(TAG, "Releasing wake lock");
                if (mScreenWakeLock != null) {
                    mScreenWakeLock.release();
                    mScreenWakeLock = null;
                }
            }
        }

    private final Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case HANDLER_ID_UPDATE_RESULT:
                updateTestResult();
                break;
            default:
                break;
            }
        }
    };

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Configuration.ORIENTATION_LANDSCAPE == getResources()
                .getConfiguration().orientation) {
            setContentView(R.layout.wifi_sdio_result_land);
        } else {
            setContentView(R.layout.wifi_sdio_result_port);
        }
        mOriBackup = getResources().getConfiguration().orientation;
        getViews();
        mWakeLock = new WifiSdioWakeLock();
        mWakeLock.acquireScreenWakeLock(this);
    }

    private void getViews() {
        mTvCmdEdge = (TextView) findViewById(R.id.wifi_sdio_cmd_edge);
        mTvCmdDelay = (TextView) findViewById(R.id.wifi_sdio_cmd_delay);
        mTvReadEdge = (TextView) findViewById(R.id.wifi_sdio_read_edge);
        mTvReadDelay = (TextView) findViewById(R.id.wifi_sdio_read_delay);
        mTvWriteEdge = (TextView) findViewById(R.id.wifi_sdio_write_edge);
        mTvWriteDelay = (TextView) findViewById(R.id.wifi_sdio_write_delay);
        mTvResult = (TextView) findViewById(R.id.wifi_sdio_result);
        mTvResultPath = (TextView) findViewById(R.id.wifi_sdio_result_path);
    }

    protected void onStart() {
        super.onStart();
        IntentFilter testFilter = new IntentFilter();
        testFilter.addAction(WifiSdioTestService.ACTION_RESULT_UPDATE_REQUEST);
        registerReceiver(mTestUpdateReceiver, testFilter);
        getApplication().bindService(
                new Intent(this, WifiSdioTestService.class), mServiceCon,
                Context.BIND_AUTO_CREATE);
    }

    protected void onResume() {
        Xlog.v(TAG, "onResume");
        super.onResume();
        if (mOriBackup != getResources().getConfiguration().orientation) {
            onConfigurationChanged(getResources().getConfiguration());
        }
    }

    protected void onRestoreInstanceState(Bundle state) {
        super.onRestoreInstanceState(state);
    }

    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    protected void onPause() {
        mOriBackup = getResources().getConfiguration().orientation;
        super.onPause();
    }

    protected void onStop() {
        unregisterReceiver(mTestUpdateReceiver);
        getApplication().unbindService(mServiceCon);
        super.onStop();
    }

    protected void onDestroy() {
        mWakeLock.release();
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (Configuration.ORIENTATION_LANDSCAPE == newConfig.orientation) {
            setContentView(R.layout.wifi_sdio_result_land);
        } else if (Configuration.ORIENTATION_PORTRAIT == newConfig.orientation) {
            setContentView(R.layout.wifi_sdio_result_port);
        }
        getViews();
        mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
    }

    private void updateTestResult() {
        if (null != mService) {
            Bundle bundle = mService.getResult();
            int value = bundle.getInt(WifiSdioTestService.KEY_RESULT_CMD_EDGE);
            mTvCmdEdge.setText(value < 0 ? null : String.valueOf(value));
            value = bundle.getInt(WifiSdioTestService.KEY_RESULT_CMD_DELAY);
            mTvCmdDelay.setText(value < 0 ? null : String.valueOf(value));
            value = bundle.getInt(WifiSdioTestService.KEY_RESULT_READ_EDGE);
            mTvReadEdge.setText(value < 0 ? null : String.valueOf(value));
            value = bundle.getInt(WifiSdioTestService.KEY_RESULT_READ_DELAY);
            mTvReadDelay.setText(value < 0 ? null : String.valueOf(value));
            value = bundle.getInt(WifiSdioTestService.KEY_RESULT_WRITE_EDGE);
            mTvWriteEdge.setText(value < 0 ? null : String.valueOf(value));
            value = bundle.getInt(WifiSdioTestService.KEY_RESULT_WRITE_DELAY);
            mTvWriteDelay.setText(value < 0 ? null : String.valueOf(value));
            mTvResultPath.setText(bundle
                    .getString(WifiSdioTestService.KEY_RESULT_PATH));
            updateResultFile();
        }
    }

    private void updateResultFile() {
    	byte tmpByte;
    	int count = 0;
    	mTvResult.setText(null);
    	while(WifiSdioTestService.sCurrentTestLoop > count) {
    		tmpByte = WifiSdioTestService.sResultBuffer.get(count);
    		mTvResult.append(String.format("%c ",tmpByte));
    		if(count % WifiSdioTestService.TEST_HALF_ROUND == WifiSdioTestService.TEST_HALF_ROUND - 1) {
    			mTvResult.append(LINE_BREAK);
    		}
    		if(count % WifiSdioTestService.TEST_ONE_ROUND == WifiSdioTestService.TEST_ONE_ROUND - 1) {
    			switch((count+1)/WifiSdioTestService.TEST_ONE_ROUND) {
    			case 1:
    				mTvResult.append(String.format("final cmd edge : %d , ",WifiSdioTestService.sCmdEdge));
    				mTvResult.append(String.format("final cmd delay : %d",WifiSdioTestService.sCmdDelay));    				
    				break;
    			case 2:
    				mTvResult.append(String.format("final data read edge: %d , ",WifiSdioTestService.sReadEdge));
                    mTvResult.append(String.format("final read delay: %d",WifiSdioTestService.sReadDelay));		
    				break;
    			case 3:
    				mTvResult.append(String.format("final data write edge:  %d , ",WifiSdioTestService.sWriteEdge));
    				mTvResult.append(String.format("final write delay:  %d",WifiSdioTestService.sWriteDelay));
    				break;    			
    			}    			
    			mTvResult.append(LINE_BREAK);
    		}
    		count++;
    	}
    	/*
        File file = new File(mTvResultPath.getText().toString());
        mTvResult.setText(null);
        if (file.exists()) {
            String line = null;
            BufferedReader br = null;
            try {
                br = new BufferedReader(new FileReader(file));
                while (null != (line = br.readLine())) {
                    mTvResult.append(line);
                    mTvResult.append(LINE_BREAK);
                }
            } catch (FileNotFoundException e) {
                Xlog.d(TAG, "Open file not found: " + file.getAbsolutePath());
            } catch (IOException e) {
                Xlog.d(TAG, "Read file IOException: " + file.getAbsolutePath());
            } finally {
                if (null != br) {
                    try {
                        br.close();
                    } catch (IOException e) {
                        Xlog.d(TAG, "Close file IOException: "
                                + file.getAbsolutePath());
                    }
                    br = null;
                }
            }
        }
        */
    }

    private final ServiceConnection mServiceCon = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            Xlog.v(TAG, "onServiceConnected");
            mService = ((WifiSdioTestService.WifiSdioBinder) service)
                    .getService();
            mHandler.sendEmptyMessage(HANDLER_ID_UPDATE_RESULT);
        }

        public void onServiceDisconnected(ComponentName name) {
            Xlog.v(TAG, "onServiceDisconnected");
            mService = null;
        }
    };

}
