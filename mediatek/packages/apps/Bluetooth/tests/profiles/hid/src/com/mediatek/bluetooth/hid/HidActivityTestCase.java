
package com.mediatek.bluetooth.hid;

import android.app.Activity;
import android.app.Instrumentation;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.IBluetoothHid;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.Reflector;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

public class HidActivityTestCase extends ActivityInstrumentationTestCase2<BluetoothHidActivity> {

    private static final String TAG = "[BT][HIDUT][HidActivityTestCase]";

    private static final int TEST_INTENT_FLAG = Intent.FLAG_ACTIVITY_NEW_TASK
            | Intent.FLAG_ACTIVITY_MULTIPLE_TASK;

    private BluetoothHidActivity mHidActivity;

    private Instrumentation mInstrumentation;

    private Context mContext;

    private Solo mSolo;

    private PreferenceScreen mPreferenceScreen;

    private PreferenceCategory mDeviceList;

    private Preference mTmpPre;

    private IBluetoothHid mService;

    private BluetoothUnitTestJni mJniManager = null;

    private boolean mIsBluetoothOn = false;

    private static int sNum = 0;

    private static final String AUTO_TEST_TITLE = "For Hid autoTest";

    public HidActivityTestCase() {
        super(BluetoothHidActivity.class);
        // TODO Auto-generated constructor stub
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mService = IBluetoothHid.Stub.asInterface(service);
            printLog("onServiceConnected is called");
        }

        public void onServiceDisconnected(ComponentName className) {
            mService = null;
            printLog("onServiceDisconnected is called");

        }
    };

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        sNum++;
        String test = "BLUETOOTH_HID_PTS";
        BluetoothHidService.sUtState = true;
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_HID);
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        Intent intent = new Intent();
        intent.setFlags(TEST_INTENT_FLAG);
        intent.setClass(mContext, BluetoothHidActivity.class);
        intent.putExtra(test, 0);
        printLog("setUp ++++++++");
        setActivityInitialTouchMode(true);
        setActivityIntent(intent);
        enableBt();
        sleep(2000);
        mHidActivity = this.getActivity();
        initPreference();
        mSolo = new Solo(mInstrumentation, mHidActivity);
        printLog("setUp --------");
    }

    public void test01testCondition() throws InterruptedException {
        printLog("test01");
        mockConnect();
        enableBt();
        assertEquals(true, BluetoothAdapter.getDefaultAdapter().isEnabled());

    }

    public void test02preferenceClick() throws Exception {
        printLog("test03");
        mockEnableBt(Activity.RESULT_OK);
        try {
            inputKeyEventSequence(KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_DOWN,
                    KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_DPAD_DOWN,
                    KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_DPAD_UP,
                    KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_DPAD_CENTER,
                    KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_CENTER);

        } finally {
            mDeviceList.removeAll();
        }
    }

    public void test03mockOnPreferenceClick1() throws Exception {

        printLog("test03_mockPreferenceClick1");

        sleep(2000);
        try {
            mHidActivity.onPreferenceTreeClick(mPreferenceScreen, mTmpPre);
        } finally {
            mDeviceList.removeAll();
        }
    }

    public void test04mockOnPreferenceClick2() throws Exception {

        printLog("test04_mockOnPreferenceClick2");
        mockEnableBt(Activity.RESULT_OK);
        sleep(5000);

        try {
            mHidActivity.onPreferenceTreeClick(mPreferenceScreen, mTmpPre);
            sleep(2000);
        } finally {
            mDeviceList.removeAll();

        }
    }

    protected void inputKeyEventSequence(int... keys) throws InterruptedException {
        for (int key : keys) {
            mInstrumentation.sendKeyDownUpSync(key);
            sleep(2000);
        }
    }

    private void initPreference() {

            printLog("initPreference");
            try {
            SharedPreferences settings = mHidActivity.getSharedPreferences(Reflector.getField(
                    mHidActivity, "BT_HID_SETTING_INFO").toString(), 0);
            mTmpPre = new Preference(mHidActivity);
            settings.edit().putInt("preferenceCount", 1).commit();
            Editor editor = settings.edit();
            editor.putString("deviceAddr" + Integer.toString(0), "7C:ED:8D:68:48:E8").putString(
                    "newAdd" + Integer.toString(0), "FALSE").commit();
            mTmpPre.setKey("7C:ED:66:68:8F:E8");
            mTmpPre.setTitle(AUTO_TEST_TITLE);
            mDeviceList = (PreferenceCategory) (Reflector.get(mHidActivity, "sDeviceList"));
            mDeviceList.addPreference(mTmpPre);
            } catch (NoSuchFieldException e) {
                e.printStackTrace();
        }

    }

    @SuppressWarnings("static-access")
    public void test05reflectReceiver() throws Exception {
        printLog("test05_reflectReceiver");
        Intent tmpIntent1 = new Intent(BluetoothAdapter.ACTION_STATE_CHANGED);
        Intent tmpIntent2 = new Intent(mHidActivity.ACTION_SUMMARY_CHANGED);
        Intent tmpIntent3 = new Intent(mHidActivity.ACTION_DEVICE_ADDED);
        tmpIntent1.putExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.STATE_TURNING_OFF);
        tmpIntent2.putExtra(mHidActivity.EXTRA_SUMMARY, R.string.bluetooth_hid_connect);
        BroadcastReceiver receiver = (BroadcastReceiver) Reflector.get(mHidActivity, "mReceiver");
        receiver.onReceive(mContext, tmpIntent1);
        receiver.onReceive(mContext, tmpIntent2);
        receiver.onReceive(mContext, tmpIntent3);

    }

    public void test06ContextItemSelect1() throws Exception {
        try {
            printLog("test06_ContextItemSelect1");
            //this.mInstrumentation.invokeContextMenuAction(mHidActivity, 0, 0);

            sleep(5000);
            if (mSolo == null) {
                printLog("test06_ContextItemSelect1 mSolo is null");
                //Assert.assertNotNull(mSolo);
            } else {
                mSolo.clickLongOnText(AUTO_TEST_TITLE);
            }
            inputKeyEventSequence(KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_CENTER,
                    KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_CENTER);
        } finally {
            mDeviceList.removeAll();
        }
    }

    public void test07ContextItemSelect2() throws Exception {
        try {
            printLog("test07_ContextItemSelect2");

            sleep(5000);
            if (mSolo == null) {
                printLog("test07_ContextItemSelect2 mSolo is null");
                //Assert.assertNotNull(mSolo);
            } else {
                mSolo.clickLongOnText(AUTO_TEST_TITLE);
            }
            inputKeyEventSequence(KeyEvent.KEYCODE_DPAD_DOWN, KeyEvent.KEYCODE_DPAD_DOWN,
                    KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_DPAD_DOWN,
                    KeyEvent.KEYCODE_DPAD_CENTER, KeyEvent.KEYCODE_DPAD_UP,
                    KeyEvent.KEYCODE_DPAD_CENTER);

        } finally {
            mDeviceList.removeAll();
        }

    }

    public void test08callOnRestart() throws Exception {
        try {
            printLog("test08_callOnRestart");

            disableBt();
            mockEnableBt(Activity.RESULT_CANCELED);
            this.mInstrumentation.callActivityOnRestart(mHidActivity);
            sleep(1000);
            inputKeyEventSequence(KeyEvent.KEYCODE_BACK);
        } catch (InterruptedException e) {
            e.printStackTrace();

        } finally {
            mockEnableBt(2);
        }
    }

    public void test09callOnCreate() throws Exception {
        printLog("test09_callOnCreate");
        disableBt();// mock !mBluetoothAdapter.isEnable()
        this.getActivity();
        sleep(1000);
        inputKeyEventSequence(KeyEvent.KEYCODE_BACK);
    }

    public void test10callOnResume() throws Exception {

        printLog("test10_callOnResume");
        disableBt();
        this.mInstrumentation.callActivityOnResume(mHidActivity);
        sleep(1000);
        inputKeyEventSequence(KeyEvent.KEYCODE_BACK);
    }

    public void test11callOnRestoreInstanceState() throws Exception {

        printLog("test11callOnRestoreInstanceState");
        Bundle bundle = new Bundle();
        bundle.putString("test", "forHidTest");
        sleep(2000);
        this.mInstrumentation.callActivityOnRestoreInstanceState(mHidActivity, bundle);
        sleep(1000);
        inputKeyEventSequence(KeyEvent.KEYCODE_BACK);

    }

    private void mockEnableBt(int resultCode) throws Exception {

        Reflector.invoke(mHidActivity, "onActivityResult", new Reflector.Parameter(int.class, 2),
                new Reflector.Parameter(int.class, resultCode), new Reflector.Parameter(
                        Intent.class, null));

    }

    private void enableBt() {
        if (!BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            BluetoothAdapter.getDefaultAdapter().enable();
        }
        mIsBluetoothOn = true;
        sleep(2000);
    }

    private void disableBt() {
        if (BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            BluetoothAdapter.getDefaultAdapter().disable();
        }
    }

    @SuppressWarnings("unchecked")
    private void mockConnect() {
        /*
         * case description(from settings): 1 turn on BT, 2 connect Hid device ,
         * 3 connect success,
         */
        String jniLogs[] = {
                "Req;0;false;initServiceNative;void;0",

                "Req;0;false;serverActivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;0;7C:ED:8D:68:48:E8",
                "Req;0;false;serverConnectReqNative;void;1;7C:ED:8D:68:48:E8",
                "Cb;0;false;sendServiceMsg;void;2;4;7C:ED:8D:68:48:E8",

                "Req;0;false;serverDeactivateReqNative;void;0",
                "Cb;0;false;sendServiceMsg;void;2;2;7C:ED:8D:68:48:E8",
                "Req;0;false;wakeupListenerNative;void;0",
                "Req;0;false;stopListentoSocketNative;void;0",
                "Req;0;false;cleanServiceNative;void;0"

        };
        mJniManager.setCaseLog(jniLogs, ";");
        if (mIsBluetoothOn && mService == null) {
            Intent tmpIntent = new Intent(IBluetoothHid.class.getName());

            if (mContext.bindService(tmpIntent, mConnection, Context.BIND_AUTO_CREATE)) {

                printLog("testConnect bindSerivce Success");
            }
            int timeout = 0;
            while (mService == null) {
                Log.d(TAG, "mService in while is null");
                if (timeout > 100) {
                    Log.d(TAG, "timeout is out of bound,then return");
                    return;
                }
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                timeout++;
            }
            try {
            Constructor con = BluetoothDevice.class.getDeclaredConstructor(String.class);
            Log.i(TAG, "Constructor:" + con);
                con.setAccessible(true);
                BluetoothDevice bd = (BluetoothDevice) con.newInstance("7C:ED:66:68:8F:E8");
                Log.i(TAG, "BluetoothDevice:" + bd);
                mService.connect(bd);
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            sleep(5000);

            Log.i(TAG, "connect HidService success");
        } else {
            assertTrue(mIsBluetoothOn);
            Log.d(TAG, "mIsBluetoothOn false");
        }
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        printLog("tearDown ++++");
        mHidActivity = null;
        if (sNum == 11) {
            disableBt();
            sleep(2000);
        }
        super.tearDown();
    }

    private void sleep(int mSeconds) {
        try {
            Thread.sleep(mSeconds);
        } catch (InterruptedException e) {
            printLog("[API:sleep] Exception :" + e.toString());
        }
    }

    private void printLog(String str) {
        if (true) {
            Log.d(TAG, str);
        }
    }
}
