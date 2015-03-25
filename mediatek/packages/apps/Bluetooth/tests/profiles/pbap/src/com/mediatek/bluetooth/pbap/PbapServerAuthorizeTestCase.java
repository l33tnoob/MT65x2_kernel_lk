
package com.mediatek.bluetooth.pbap;

import android.app.Activity;
import android.app.AlertDialog;
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
import android.widget.CheckBox;

import java.lang.reflect.Field;

public class PbapServerAuthorizeTestCase extends ActivityInstrumentationTestCase2<BluetoothServerAuthorize> {

    private static final String TAG = "[BT][PBAPUT][AlertActivityTestCase]";

    private CheckBox mAlwaysTrustBox;

    private Button mConfirmButton;

    private Button mDeclineButton;

    private Activity mActivity;

    private AlertDialog mAlertDialog = null;

    private AssertBroadcastReceiver mAssertRecevier = new AssertBroadcastReceiver();

    private static int sNum = 0;

    public PbapServerAuthorizeTestCase() {
        super(BluetoothServerAuthorize.class);
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
        mAlertDialog = getAlertDialog(mActivity);
        
        mConfirmButton = (Button)mAlertDialog.getButton(DialogInterface.BUTTON_POSITIVE);
        mDeclineButton = (Button)mAlertDialog.getButton(DialogInterface.BUTTON_NEGATIVE);
        mAlwaysTrustBox = (CheckBox)(getLayoutView(mActivity).findViewById(com.mediatek.bluetooth.R.id.alwaysallowed));
        
        IntentFilter returnFilter = new IntentFilter(BluetoothPbapService.PBAP_AUTHORIZE_RETURN_ACTION);
        this.getInstrumentation().getTargetContext().registerReceiver(mAssertRecevier, returnFilter);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        printLog("tearDown ------");
        mAssertRecevier.setExpectStatus(-1, false);
        this.getInstrumentation().getTargetContext().unregisterReceiver(mAssertRecevier);
        mActivity = null;
        mAlertDialog = null;
        mConfirmButton = null;
        mDeclineButton = null;
        mAlwaysTrustBox = null;
        super.tearDown();
    }

    private void setIntent(int n) {
        Intent intent = new Intent(BluetoothServerAuthorize.class.getName());
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                .putExtra(BluetoothServerAuthorize.DEVICE_NAME, "Test equipment")
                .putExtra(BluetoothServerAuthorize.ACTION_RETURN, BluetoothPbapService.PBAP_AUTHORIZE_RETURN_ACTION);
        switch (n) {
            case 0:
            case 1:
            case 2:
                intent.putExtra(BluetoothServerAuthorize.ACTION_CANCEL, BluetoothPbapService.PBAP_AUTHORIZE_CANCEL_ACTION);
                break;
            case 3:                
                
                break;
            case 4:
                intent.putExtra(BluetoothServerAuthorize.ACTION_CANCEL, BluetoothPbapService.PBAP_AUTHORIZE_CANCEL_ACTION)
                    .putExtra(BluetoothServerAuthorize.TIMEOUT_VALUE, 1000);
                break;
            default:
                break;
        }
        this.setActivityIntent(intent);
    }
    
    private class AssertBroadcastReceiver extends BroadcastReceiver {
        private int mExpectResult = -1;
        private boolean mAlwaysAllow = true;
        
        public void setExpectStatus(int result, boolean alwaysAllow) {
            mExpectResult = result;
            mAlwaysAllow = alwaysAllow;
        }
        
        public void onReceive(Context context, Intent intent) {
            printLog("the check reciver is invoiked");
            
            int res = intent.getIntExtra(BluetoothServerAuthorize.AUTHORIZE_RESULT, BluetoothServerAuthorize.RESULT_OTHERS);
            boolean alwaysaccept = intent.getBooleanExtra(BluetoothServerAuthorize.AUTHORIZE_ALWAYS_ALLOWED, false);
            
            if (mExpectResult > 0) {
                assertTrue(res == mExpectResult);
            }
            if ((mExpectResult == BluetoothServerAuthorize.RESULT_USER_ACCEPT) && (res == mExpectResult)) {
                assertTrue(alwaysaccept == mAlwaysAllow);
            }
        }
    }

    private AlertDialog getAlertDialog(Activity obj) {

        Class<?> activityClass;
        try {
            activityClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothServerAuthorize");
            printLog("[API:getAlertDialog] BluetoothServerAuthorize: " + activityClass.toString());
            //Class parentClass = activityClass.getSuperclass();
            //printLog("[API:getAlertDialog] BluetoothServerAuthorize super class: " + parentClass.toString());
            Field mField = activityClass.getDeclaredField("mInfoDialog");
            printLog("[API:getAlertDialog] mField : " + mField.toString());
            mField.setAccessible(true);
            AlertDialog returnDialog = (AlertDialog) mField.get(obj);
            printLog("[API:getAlertDialog] returnAlertDialog : " + returnDialog.toString());
            return returnDialog;
        } catch (ClassNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        } catch (SecurityException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        } catch (NoSuchFieldException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
        

    }

    private View getLayoutView(Activity obj) {

            Class<?> activityClass;
            try {
                activityClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothServerAuthorize");
                printLog("[API:getContexView] BluetoothServerAuthorize: " + activityClass.toString());
                Field mField = activityClass.getDeclaredField("mView");
                printLog("[API:getContexView] mField : " + mField.toString());
                mField.setAccessible(true);
                View returnView = (View) mField.get(obj);
                printLog("[API:getContexView] returnAlertDialog : " + returnView.toString());
                return returnView;
            } catch (ClassNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return null;
            } catch (SecurityException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return null;
            } catch (NoSuchFieldException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return null;
            } catch (IllegalArgumentException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return null;
            } catch (IllegalAccessException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return null;
            }
    }

    public void test00ServerAuthorizeAllow() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothServerAuthorize.RESULT_USER_ACCEPT, false);
        TouchUtils.clickView(this, mAlwaysTrustBox);
        TouchUtils.clickView(this, mAlwaysTrustBox);
        TouchUtils.clickView(this, mConfirmButton);
        sleep(2000);
    }

    public void test01ServerAuthorizeDecline() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothServerAuthorize.RESULT_USER_REJECT, false);
        TouchUtils.clickView(this, mDeclineButton);
        sleep(2000);
    }

    public void test02ServerAuthorizeCancel() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothServerAuthorize.RESULT_CANCEL, false);
        Intent intent = new Intent(BluetoothPbapService.PBAP_AUTHORIZE_CANCEL_ACTION);
        printLog(" [API: Cancel  " + BluetoothPbapService.PBAP_AUTHORIZE_CANCEL_ACTION);
        mActivity.sendBroadcast(intent);
        sleep(2000);
    }

    public void test03ServerAuthorizeAlwaysAllow() {
        sleep(3000);
        mAssertRecevier.setExpectStatus(BluetoothServerAuthorize.RESULT_USER_ACCEPT, true);
        TouchUtils.clickView(this, mAlwaysTrustBox);
        TouchUtils.clickView(this, mConfirmButton);
        sleep(2000);
    }

    public void test04ServerAuthorizeTimeOut() {
        mAssertRecevier.setExpectStatus(BluetoothServerAuthorize.RESULT_TIMEOUT, true);
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
