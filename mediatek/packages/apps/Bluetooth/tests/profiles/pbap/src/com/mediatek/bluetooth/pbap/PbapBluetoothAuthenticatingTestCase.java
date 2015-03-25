
package com.mediatek.bluetooth.pbap;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.test.ActivityInstrumentationTestCase2;
import android.test.TouchUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

import java.lang.reflect.Field;

public class PbapBluetoothAuthenticatingTestCase extends ActivityInstrumentationTestCase2<BluetoothAuthenticating> {

    private static final String TAG = "[BT][PBAPUT][PbapBluetoothAuthenticatingTestCase]";

    private EditText mPassCodeText;

    private Button mAcceptButton;

    private Button mRejectButton;

    private AlertActivity mActivity;

    private AssertBroadcastReceiver mAssertRecevier = new AssertBroadcastReceiver();

    private AlertController mController = null;

    private static int sNum = 0;

    public PbapBluetoothAuthenticatingTestCase() {
        super(BluetoothAuthenticating.class);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        printLog("setUp +++++");
        
        printLog("[API:setUp] num is :" + sNum);
        setIntent(sNum);
        sNum++;

        mActivity = this.getActivity();
        mController = getController(mActivity);
        
        mAcceptButton = mController.getButton(DialogInterface.BUTTON_POSITIVE);
        mRejectButton = mController.getButton(DialogInterface.BUTTON_NEGATIVE);
        mPassCodeText = (EditText)(getLayoutView(mActivity).findViewById(com.mediatek.bluetooth.R.id.pass_code_edit));
        
        IntentFilter returnFilter = new IntentFilter(BluetoothPbapService.PBAP_AUTHENTICATE_RETURN_ACTION);
        this.getInstrumentation().getTargetContext().registerReceiver(mAssertRecevier, returnFilter);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        printLog("tearDown ------");
        mAssertRecevier.setExpectStatus(-1, "");
        this.getInstrumentation().getTargetContext().unregisterReceiver(mAssertRecevier);
        mActivity = null;
        mController = null;
        mAcceptButton = null;
        mRejectButton = null;
        mPassCodeText = null;
        super.tearDown();
    }

    private void setIntent(int n) {
        Intent intent = new Intent(BluetoothAuthenticating.class.getName());
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                .putExtra(BluetoothAuthenticating.DEVICE_NAME, "Test equipment")
                .putExtra(BluetoothAuthenticating.ACTION_RETURN, BluetoothPbapService.PBAP_AUTHENTICATE_RETURN_ACTION);
        switch (n) {
            case 0:
            case 1:
            case 2:
                intent.putExtra(BluetoothAuthenticating.ACTION_CANCEL, BluetoothPbapService.PBAP_AUTHENTICATE_CANCEL_ACTION);
                break;
            case 3:                
                //no extra
                break;
            case 4:
                intent.putExtra(BluetoothAuthenticating.ACTION_CANCEL, BluetoothPbapService.PBAP_AUTHENTICATE_CANCEL_ACTION)
                    .putExtra(BluetoothAuthenticating.TIMEOUT_VALUE, 1000);
                break;
            default:
                break;
        }
        this.setActivityIntent(intent);
    }
    
    private class AssertBroadcastReceiver extends BroadcastReceiver {
        
        private int mExpectResult = -1;
        private String mExpectPasscode;
        
        public void setExpectStatus(int result, String passcode) {
            mExpectResult = result;
            mExpectPasscode = passcode;
        }
        
        public void onReceive(Context context, Intent intent) {
            printLog("the check reciver is invoiked");
            
            int res = intent.getIntExtra(BluetoothAuthenticating.AUTHENTICATE_RESULT, BluetoothAuthenticating.RESULT_OTHERS);
            String passcode = intent.getStringExtra(BluetoothAuthenticating.AUTHETICATE_CODE);
            
            if (mExpectResult > 0) {
                assertTrue(res == mExpectResult);
            }
            if ((mExpectResult == BluetoothAuthenticating.RESULT_USER_ACCEPT) && (res == mExpectResult)) {
                assertTrue(mExpectPasscode.equals(passcode));
            }
        }
    }
    
    private AlertController getController(AlertActivity obj) {
        Class<?> activityClass;
        try {
            activityClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothAuthenticating");
            printLog("[API:getController] BluetoothAuthenticating: " + activityClass.toString());
            Class<?> parentClass = activityClass.getSuperclass();
            printLog("[API:getController] BluetoothAuthenticating super class: " + parentClass.toString());
            Field field = parentClass.getDeclaredField("mAlert");
            printLog("[API:getController] field : " + field.toString());
            field.setAccessible(true);
            AlertController returnController = (AlertController) field.get(obj);
            printLog("[API:getController] returnController : " + returnController.toString());
            return returnController;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (SecurityException e) {
            e.printStackTrace();
            return null;
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
            return null;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            return null;
        } catch (IllegalAccessException e) {
            e.printStackTrace();
            return null;
        } 
    }

    private View getLayoutView(AlertActivity obj) {
        
        Class<?> activityClass;
        try {
            activityClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothAuthenticating");
            printLog("[API:getContexView] BluetoothAuthenticating: " + activityClass.toString());
            Field field = activityClass.getDeclaredField("mView");
            printLog("[API:getContexView] field : " + field.toString());
            field.setAccessible(true);
            View returnView = (View) field.get(obj);
            printLog("[API:getContexView] returnView : " + returnView.toString());
            return returnView;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        } catch (SecurityException e) {
            e.printStackTrace();
            return null;
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
            return null;
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            return null;
        } catch (IllegalAccessException e) {
            e.printStackTrace();
            return null;
        }
        
    }

    public void test00AuthenticatingAccept() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothAuthenticating.RESULT_USER_ACCEPT, "123");
        TouchUtils.clickView(this, mPassCodeText);
        this.sendKeys("1 2 3");
        TouchUtils.clickView(this, mAcceptButton);
        sleep(2000);
    }

    public void test01AuthenticatingAcceptWithoutPassWord() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothAuthenticating.RESULT_USER_REJECT, "");
        TouchUtils.clickView(this, mAcceptButton);
        sleep(2000);
    }

    public void test02AuthenticatingCancel() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothAuthenticating.RESULT_CANCEL, "");
        Intent intent = new Intent(BluetoothPbapService.PBAP_AUTHENTICATE_CANCEL_ACTION);
        mActivity.sendBroadcast(intent);
        sleep(2000);
    }
    
    public void test03AuthenticatingReject() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothAuthenticating.RESULT_USER_REJECT, "");
        TouchUtils.clickView(this, mRejectButton);
        sleep(2000);
    }

    public void test04AuthenticatingTimeout() {
        mAssertRecevier.setExpectStatus(BluetoothAuthenticating.RESULT_TIMEOUT, "");
        sleep(3000);
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
