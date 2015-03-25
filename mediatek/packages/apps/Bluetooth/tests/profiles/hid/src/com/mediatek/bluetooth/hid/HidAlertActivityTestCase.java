package com.mediatek.bluetooth.hid;

import android.app.Instrumentation;
import android.bluetooth.BluetoothHid;
import android.content.DialogInterface;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.Button;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import junit.framework.Assert;

import java.lang.reflect.Field;

public class HidAlertActivityTestCase extends ActivityInstrumentationTestCase2<BluetoothHidAlert> {

    private static final String TAG = "[BT][HIDUT][HidAlertActivityTestCase]";

    protected Instrumentation mInstrumentation;

    private Button mConfirmButton;

    private Button mCancelButton;

    private AlertActivity mAlertActivity;

    private AlertController mController;

    private static int sNum = 0;

    public HidAlertActivityTestCase() {
        super(BluetoothHidAlert.class);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        printLog("setUp ++++++++");
        sNum++;
        printLog("[API:setUp] num is :" + sNum);
        BluetoothHidService.sUtState = true;
        setIntent(sNum);
        mAlertActivity = this.getActivity();
        mInstrumentation = new Instrumentation();
        mController = getController(mAlertActivity);
        if (mController == null) {
            Assert.assertNotNull(mController);
            printLog("[API:setUp] AlertController mController is null");
        } else {
            mConfirmButton = mController.getButton(DialogInterface.BUTTON_POSITIVE);
            mCancelButton = mController.getButton(DialogInterface.BUTTON_NEGATIVE);
            Assert.assertNotNull(mConfirmButton);
            Assert.assertNotNull(mCancelButton);
        }
        printLog("setUp --------");
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        printLog("tearDown ++++");
        mAlertActivity = null;
        mController = null;
        mConfirmButton = null;
        mCancelButton = null;
        super.tearDown();
    }

    public void test01AlertDisconnectConfirm() {
        sleep(3000);
        TouchUtils.clickView(this, mConfirmButton);
        sleep(2000);
    }

    public void test02AlertDisconnectCancel() {
        sleep(3000);
        TouchUtils.clickView(this, mCancelButton);
        sleep(2000);
    }

    public void test03AlertUnplugConfirm() {
        sleep(3000);
        TouchUtils.clickView(this, mConfirmButton);
        sleep(2000);
    }

    public void test04AlertUnplugCancel() {
        sleep(3000);
        TouchUtils.clickView(this, mCancelButton);
        sleep(2000);
    }

    public void test05AlertAuthorizeConfirm() {
        sleep(3000);
        TouchUtils.clickView(this, mConfirmButton);
        sleep(2000);
    }

    public void test06AlertAuthorizeCancel() {
        sleep(3000);
        TouchUtils.clickView(this, mCancelButton);

    }

    public void test07onKeyDown() throws Exception {
        try {
            sleep(2000);
            mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        } finally {
         printLog("sendKeys");
        }
    }

    private void setIntent(int n) {
        Intent intent = new Intent(BluetoothHidAlert.class.getName());
        intent.putExtra(BluetoothHid.DEVICE_ADDR, "00:15:83:52:83:17");
        switch (n) {
            case 1:
            case 2:
                intent.putExtra(BluetoothHid.ACTION, BluetoothHid.BT_HID_DEVICE_DISCONNECT);
                break;

            case 3:
            case 4:
                intent.putExtra(BluetoothHid.ACTION, BluetoothHid.BT_HID_DEVICE_UNPLUG);
                break;

            case 5:
            case 6:
            case 7:
                intent.putExtra(BluetoothHid.ACTION, BluetoothHid.BT_HID_DEVICE_AUTHORIZE);
                break;

            default:
                break;
        }
        this.setActivityIntent(intent);
    }

    @SuppressWarnings("unchecked")
    private AlertController getController(AlertActivity obj) {
        try {
            Class activityClass = Class.forName("com.mediatek.bluetooth.hid.BluetoothHidAlert");
            printLog("[API:getController] BluetoothHidAlert: " + activityClass.toString());
            Class parentClass = activityClass.getSuperclass();
            printLog("[API:getController] BluetoothHidAlert super class: " + parentClass.toString());
            Field mField = parentClass.getDeclaredField("mAlert");
            printLog("[API:getController] mField : " + mField.toString());
            mField.setAccessible(true);
            AlertController returnController = (AlertController) mField.get(obj);
            printLog("[API:getController] returnController : " + returnController.toString());
            return returnController;
        } catch (ClassNotFoundException e) {
            printLog("[API:getController] Exception is " + e.toString());
            return null;
        } catch (NoSuchFieldException e) {
            printLog("[API:getController] Exception is " + e.toString());
            return null;
        } catch (IllegalAccessException e) {
            printLog("[API:getController] Exception is " + e.toString());
            return null;
        }
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
