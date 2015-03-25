
package com.mediatek.bluetooth.pan;

import android.bluetooth.BluetoothPan;
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

public class PanAlertActivityTestCase extends ActivityInstrumentationTestCase2<BluetoothPanAlert> {

    private static final boolean DEBUG = true;

    private static final String TAG = "[BT][PANUT][PanAlertActivityTestCase]";

    private Button mConfirmButton;      //confirm button

    private Button mCancelButton;       //cancel button

    private AlertActivity mAlertActivity;   //activity

    private AlertController mController;    //alert controller

    //which used to set different intent to start activity
    private static int sNum = 0;

    private static final String ACTIVITY_CLASS_NAME = "com.mediatek.bluetooth.pan.BluetoothPanAlert";

    private static final String DEVICE_ADDRESS_FIELD = "mDeviceAddr";

    private static final String BLUETOOTH_DEVICE_ADDRESS = "00:15:83:52:83:17";

    /**
     * constructor
     */
    public PanAlertActivityTestCase() {
        super(BluetoothPanAlert.class);
    }

    /**
    * Which will be called before each test case
    */
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        printLog("setUp +++++");
        sNum++;
        printLog("[API:setUp] num is :" + sNum);
        setIntent(sNum);
        mAlertActivity = this.getActivity();
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
    }

    /**
    * Which will be called after each test case
    */
    @Override
    protected void tearDown() throws Exception {
        printLog("tearDown ------");
        mAlertActivity = null;
        mController = null;
        mConfirmButton = null;
        mCancelButton = null;
        super.tearDown();
    }

    /**
     * Intent : NAP authorize<br>
     * Click confirm button
     */
    public void test01AlertNapAuthorizeConfirm() {
        assertForActivity();
        sleep(1000);
        
        TouchUtils.clickView(this, mConfirmButton);
    }

    /**
     * Intent : NAP authorize<br>
     * Click cancel button
     */
    public void test02AlertNapAuthorizeCancel() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mCancelButton);
    }

    /**
     * Intent : NAP authorize<br>
     * Click home key
     */
    public void test03AlertNapAuthorizeKeyhome() {
        assertForActivity();
        sleep(1000);
        this.sendKeys(KeyEvent.KEYCODE_HOME);
    }

    /**
     * Intent : GN authorize<br>
     * Click confirm button
     */
    public void test04AlertGnAuthorizeConfirm() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mConfirmButton);
    }

    /**
     * Intent : GN authorize<br>
     * Click cancel button
     */
    public void test05AlertGnAuthorizeCancel() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mCancelButton);
    }

    /**
     * Intent : GN authorize<br>
     * Click back key
     */
    public void test06AlertGnAuthorizeKeyback() {
        assertForActivity();
        sleep(1000);
        this.sendKeys(KeyEvent.KEYCODE_BACK);
    }

    /**
     * Intent : NAP connect<br>
     * Click confirm button
     */
    public void test07AlertNapConnectConfirm() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mConfirmButton);
    }

    /**
     * Intent : NAP connect<br>
     * Click cancel button
     */
    public void test08AlertNapConnectCancel() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mCancelButton);
    }

    /**
     * Intent : GN connect<br>
     * Click confirm button
     */
    public void test09AlertGnConnectConfirm() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mConfirmButton);
    }

    /**
     * Intent : GN connect<br>
     * Click cancel button
     */
    public void test10AlertGnConnectCancel() {
        assertForActivity();
        sleep(1000);
        TouchUtils.clickView(this, mCancelButton);
    }

    /**
     * Set intent for activity according to num n
     */
    private void setIntent(int n) {
        Intent intent = new Intent(BluetoothPanAlert.class.getName());
        intent.putExtra(BluetoothPan.DEVICE_ADDR, BLUETOOTH_DEVICE_ADDRESS);
        switch (n) {
            case 1:
            case 2:
            case 3:
                intent.putExtra(BluetoothPan.ACTION, BluetoothPan.BT_PAN_NAP_DEVICE_AUTHORIZE);
                break;

            case 4:
            case 5:
            case 6:
                intent.putExtra(BluetoothPan.ACTION, BluetoothPan.BT_PAN_GN_DEVICE_AUTHORIZE);
                break;

            case 7:
            case 8:
                intent.putExtra(BluetoothPan.ACTION, BluetoothPan.BT_PAN_NAP_DEVICE_CONNECTED);
                break;

            case 9:
            case 10:
                intent.putExtra(BluetoothPan.ACTION, BluetoothPan.BT_PAN_GN_DEVICE_CONNECTED);
                break;

            default:
                printLog("[API:setIntent] unknown id");
                break;
        }
        this.setActivityIntent(intent);
    }

    /**
    * Add assert for activity
    */
    private void assertForActivity() {
        String strAddr = this.getStringFieldValue(ACTIVITY_CLASS_NAME, DEVICE_ADDRESS_FIELD, mAlertActivity);

        if (strAddr != null) {
            Assert.assertEquals(strAddr, BLUETOOTH_DEVICE_ADDRESS);
        } else {
            printLog("[assertForActivity] bluetooth device address is null");
            Assert.assertNotNull(strAddr);
        }
    }

    /**
    * Get string field value which in object<br>
    * @param clsName class name which to get string field
    * @param fieldName field name which to get in class
    * @param obj object which is class object
    */
    @SuppressWarnings("unchecked")
    private String getStringFieldValue(String clsName, String fieldName, Object obj) {
        String retString = null;
        if (clsName == null || fieldName == null || obj == null) {
            printLog("[API:getStringFieldValue] the parameter is wrong");
            return null;
        }
        try {
            Class cls = Class.forName(clsName);
            printLog("[API:getStringFieldValue] class is " + cls.toString());
            Field field = cls.getDeclaredField(fieldName);
            printLog("[API:getStringFieldValue] field is " + field.toString());
            field.setAccessible(true);
            retString = (String) field.get(obj);
        } catch (NoSuchFieldException ex) {
            printLog("[API:getStringFieldValue] NoSuchFieldException happened");
        } catch (SecurityException ex) {
            printLog("[API:getStringFieldValue] SecurityException happened");
        } catch (ClassNotFoundException ex) {
            printLog("[API:getStringFieldValue] ClassNotFoundException happened");
        } catch (IllegalAccessException ex) {
            printLog("[API:getStringFieldValue] IllegalAccessException happened");
        }
        return retString;
    }

    /**
     * Get activity controller- AlertController
     */
    @SuppressWarnings("unchecked")
    private AlertController getController(AlertActivity obj) {
        try {
            Class activityClass = Class.forName("com.mediatek.bluetooth.pan.BluetoothPanAlert");
            printLog("[API:getController] BluetoothPanAlert: " + activityClass.toString());
            Class parentClass = activityClass.getSuperclass();
            printLog("[API:getController] BluetoothPanAlert super class: " + parentClass.toString());
            Field mField = parentClass.getDeclaredField("mAlert");
            printLog("[API:getController] mField : " + mField.toString());
            mField.setAccessible(true);
            AlertController returnController = (AlertController) mField.get(obj);
            printLog("[API:getController] returnController : " + returnController.toString());
            return returnController;
        } catch (ClassNotFoundException e) {
            printLog("[API:getController] Exception is " + e.toString());
            return null;
        } catch (NoSuchFieldException ex) {
            printLog("[API:getController] Exception is " + ex.toString());
            return null;
        } catch (IllegalAccessException ex) {
            printLog("[API:getController] Exception is " + ex.toString());
            return null;
        }
    }

    /**
     * Sleep some time
     */
    private void sleep(int mSeconds) {
        try {
            Thread.sleep(mSeconds);
        } catch (InterruptedException e) {
            printLog("[API:sleep] Exception :" + e.toString());
        }
    }

    /**
     * print log
     */
    private void printLog(String str) {
        if (DEBUG) {
            Log.d(TAG, str);
        }
    }
}
