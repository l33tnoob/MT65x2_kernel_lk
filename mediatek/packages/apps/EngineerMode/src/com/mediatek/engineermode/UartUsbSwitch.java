package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.os.SystemProperties;
import android.hardware.usb.UsbManager;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;

public class UartUsbSwitch extends Activity {

    private static final String TAG = "EM/UartUsbSwitch";
    private static final String FILE_PORT_MODE = "/sys/devices/platform/mt_usb/portmode";
    private static final String SUCCESS = " success";
    private static final String FAIL = " fail";
    private static final String MODE_USB = "0";
    private static final String MODE_UART = "1";
    private static final String KEY_USB_PORT = "mediatek.usb.port.mode";
    private static final String VAL_USB = "usb";
    private static final String VAL_UART = "uart";
    private static final int MSG_CHECK_RESULT = 11; 
    private TextView mTvCurrent;
    private RadioGroup mRgMode;
    private WorkerHandler mWorkerHandler = null;
    private HandlerThread mWorkerThread = null;
    private String mModeVal;
    private RadioButton mRbUsb;
    private RadioButton mRbUart;
    private int mPlugType = 0;
    private final BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
            // mPlugType: 0: Disconnect, 1: Charger, 2: PC
            if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
                mPlugType = intent.getIntExtra("plugged", 0);
            }
            if (action.equals(UsbManager.ACTION_USB_STATE)) {             
                boolean isHwUsbConnected = !intent.getBooleanExtra(
                        "USB_HW_DISCONNECTED", false);
                boolean mSettingUsbCharging = intent.getBooleanExtra(
                        "SettingUsbCharging", false);
                if ((!isHwUsbConnected) || ((mPlugType == 0) && mSettingUsbCharging)) {
                    enableUsbUartSwitch(true);
                } else {
                    enableUsbUartSwitch(false);
                    Toast.makeText(UartUsbSwitch.this, R.string.uart_usb_switch_warning, Toast.LENGTH_LONG).show();
                    UartUsbSwitch.this.finish();
                } 
            }
        }
    };
    
    private void showDialog(String title, String msg) {
        AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                true).setTitle(title).setMessage(msg).
                setPositiveButton(android.R.string.ok, null).create();
        dialog.show();
    }
    
    private class WorkerHandler extends Handler {
        WorkerHandler(Looper looper) {
            super(looper);
        }
        
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_CHECK_RESULT:
                final boolean result = waitForState(mModeVal, 2000);
                if (result) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            updateStatus(VAL_USB.equals(mModeVal));
                            enableUsbUartSwitch(true);
                            showDialog(null, getString(R.string.uart_usb_switch_set)
                                  + (result ? SUCCESS : FAIL));
                            
                        }
                    });
                }

                break;
            default:
                Xlog.w(TAG, "mWorkerHandler Unknown msg: " + msg.what);
                break;
            }
            super.handleMessage(msg);
        }
    } 

    private final RadioGroup.OnCheckedChangeListener mCheckListener = new RadioGroup.OnCheckedChangeListener() {

        public void onCheckedChanged(RadioGroup group, int checkedId) {
            Boolean bModeUsb = null;
            switch (checkedId) {
            case R.id.uart_usb_switch_mode_usb:
                bModeUsb = true;
                break;
            case R.id.uart_usb_switch_mode_uart:
                bModeUsb = false;
                break;
            case -1:
            default:
                break;
            }
            doSwitch(bModeUsb);
            Xlog.d(TAG, "OnCheckedChangeListener.onCheckedChanged() checkId:" + checkedId + " bModeUsb:" + bModeUsb);
        }

    };
    
    private void enableUsbUartSwitch(boolean enabled) {
        mRbUsb.setEnabled(enabled);
        mRbUart.setEnabled(enabled);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!new File(FILE_PORT_MODE).exists()) {
            Toast.makeText(this, R.string.uart_usb_switch_notsupport,
                    Toast.LENGTH_SHORT).show();
            Xlog.w(TAG, "Port mode file not exist");
            finish();
            return;
        }
        setContentView(R.layout.uart_usb_switch);
        mTvCurrent = (TextView) findViewById(R.id.uart_usb_switch_current_mode);
        mRgMode = (RadioGroup) findViewById(R.id.uart_usb_switch_mode);
        mRbUsb = (RadioButton)findViewById(R.id.uart_usb_switch_mode_usb);
        mRbUart = (RadioButton)findViewById(R.id.uart_usb_switch_mode_uart);
        mWorkerThread = new HandlerThread(TAG);
        mWorkerThread.start();
        mWorkerHandler = new WorkerHandler(mWorkerThread.getLooper());
    }

    @Override
    protected void onResume() {
        super.onResume();
        String current = getUsbMode();
        Xlog.v(TAG, "Current: " + current);
        if (null == current) {
            Toast.makeText(this, R.string.uart_usb_switch_geterror,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        Boolean mode = null;
        if (current.contains(MODE_USB)) {
            mode = true;
        } else if (current.contains(MODE_UART)) {
            mode = false;
        }
        updateStatus(mode);
        mRgMode.setOnCheckedChangeListener(mCheckListener);
        // ACTION_USB_STATE is sticky 
        IntentFilter filter = new IntentFilter();
        filter.addAction(UsbManager.ACTION_USB_STATE);
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);

        registerReceiver(mStateReceiver, filter);
    }
    
    @Override
    protected void onPause() {
        unregisterReceiver(mStateReceiver);
        super.onPause();
    };
    
    @Override
    protected void onDestroy() {
        if (mWorkerHandler != null) {
            mWorkerHandler.removeMessages(MSG_CHECK_RESULT);
        }
        if (mWorkerThread != null) {
            mWorkerThread.quit();
        }
        super.onDestroy();
    };

    private void doSwitch(Boolean bModeUsb) {
        if (null != bModeUsb) {
            if (bModeUsb.booleanValue()) {
                mModeVal = VAL_USB;
            } else {
                mModeVal = VAL_UART;
            }
            enableUsbUartSwitch(false);
            setUsbMode(mModeVal);
            mWorkerHandler.sendEmptyMessage(MSG_CHECK_RESULT);
        }
    }

    private void updateStatus(Boolean bModeUsb) {
        if (null == bModeUsb) {
            mTvCurrent.setText(R.string.uart_usb_switch_unknown);
            mRgMode.check(-1);
        } else if (bModeUsb.booleanValue()) {
            mTvCurrent.setText(R.string.uart_usb_switch_usb);
            mRgMode.check(R.id.uart_usb_switch_mode_usb);
        } else {
            mTvCurrent.setText(R.string.uart_usb_switch_uart);
            mRgMode.check(R.id.uart_usb_switch_mode_uart);
        }
    }

    private String getUsbMode() {
        String result = null;
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("cat ");
        strBuilder.append(FILE_PORT_MODE);
        Xlog.v(TAG, "get current dramc cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString())) {
                result = ShellExe.getOutput();
            }
        } catch (IOException e) {
            Xlog.w(TAG, "get current dramc IOException: " + e.getMessage());
        }
        return result;
    }

    private void setUsbMode(String value) {
        Xlog.v(TAG, "setUsbMode(), value: " + value);
        SystemProperties.set(KEY_USB_PORT, value);
    }
    
    private boolean waitForState(String modeVal, int milliSec) {
        int count = milliSec / 50;
        for (int i = 0; i < count; i++) {
            String relValue = SystemProperties.get(KEY_USB_PORT);
            Xlog.d(TAG, "Check value of usb port mode:" + relValue);
            if (modeVal.equals(relValue)) {
                return true;
            }
            SystemClock.sleep(50);
        }
        return false;
    }
}
