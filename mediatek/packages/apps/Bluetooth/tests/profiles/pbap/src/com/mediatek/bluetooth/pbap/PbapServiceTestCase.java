
package com.mediatek.bluetooth.pbap;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.bluetooth.IBluetoothPbap;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.CallLog;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Event;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.android.bluetooth.pbap.BluetoothPbapVCardListing;
import com.mediatek.bluetooth.BluetoothUnitTestJni;
import com.mediatek.bluetooth.IServiceCheckInterface;

import junit.framework.Assert;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


//public class PbapServiceTestCase extends ServiceTestCase<BluetoothPbapService> {
public class PbapServiceTestCase extends InstrumentationTestCase {


    private static final String TAG = "[BT][PBAPUT][PbapServiceTestCase]";

    private static final boolean DEBUG = true;

    private boolean mIsBluetoothOn = false; // check the bluetooth weather is on

    private BluetoothUnitTestJni mJniManager = null;

    private static IBluetoothPbap sTestServicePbap; // IBluetoothPbap
    
    private BluetoothPbapService mPbapService;
    
    private BluetoothPbapServer mPbapServer;
    
    private BluetoothPbapPath mPbapPath;
    
    private BluetoothPbapVCardListing mVCardListing;

    private Context mTestContext; // the test Context
    
    private HashMap<String, List<ContentValues>> mTestDataMap = new HashMap<String, List<ContentValues>>();

    private PbapOperateContacts mOperateContacts;// = new PbapOperateContacts();

    private ExepctVCardFilesString mVCardStrings = new ExepctVCardFilesString();

    private AssertBroadcastReceiver mAssertBroadcastReciver = new AssertBroadcastReceiver(); 
    
    /*
     * This is the IBluetoothPbap service connection
     */
    private ServiceConnection mTestConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            sTestServicePbap = IBluetoothPbap.Stub.asInterface(service);
            printLog("mTestConnection is :" + mTestConnection.toString());
            if (sTestServicePbap == null) {
                errorLog("Service connect failed!");
            }
        //BluetoothUtCommonClass.sConnected = true;
        }

        public void onServiceDisconnected(ComponentName className) {
            sTestServicePbap = null;
            printLog("onServiceDisconnected");
        }
    };

    /*
     * Override this method to get the test Context. And check weather the bluetooth is on Init the jni class
     * when you run each test method,setUp will be called
     */
    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();     //shoule be the first statement
        printLog("setUp +++++++");
        //get the test context
        BluetoothPbapService.sUtState = true;
        mTestContext = this.getInstrumentation().getTargetContext();//this.getInstrumentation().getContext();
        //services.setSharedPreferences(new MockSharedPreferences());
        
        // check the bluetooth is on
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter != null && bluetoothAdapter.isEnabled()) {
            mIsBluetoothOn = true;
        }
        mJniManager = new BluetoothUnitTestJni();
        mJniManager.setCurLogProfileID(BluetoothUnitTestJni.UT_PROFILE_PBAP);
        Assert.assertNotNull(mTestContext);
        
        IntentFilter intentFilter = new IntentFilter(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
        mTestContext.registerReceiver(mAssertBroadcastReciver, intentFilter);
    }

    /*
     * Do some clear work
     * when you finished each test method,tearDown will be called
     */
    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        
        printLog("tearDown---------");
        mTestContext.unregisterReceiver(mAssertBroadcastReciver);
        mIsBluetoothOn = false;
        mTestContext = null;
        mJniManager = null;
        BluetoothPbapService.sUtState = false;
        super.tearDown();         //shouled be the last statement
    }

    public PbapServiceTestCase() {
        super();
        // TODO Auto-generated constructor stub
        //super(BluetoothPbapService.class);
    }

    /*
     * Enable and disable normally 
     * case description: 
     *      1 start PanService, 
     *      2 stop PanService , 
     *      3 callback_pan_handle_activate_cnf and callback_pan_handle_deactivate_cnf parameter are both 1
     *      4 close socket in normal method.
     */

    public void test01EnableDisable1() {
        int waitingTime = 3000;

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;true;enableNative;1;0",
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;true;connectRspNative;1;0",
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;true;disableNative;1;0"
        };

        printLog("[API: test01EnableDisable1] : Starting");
        
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        checkLog.add("true");
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());

        if (mIsBluetoothOn) {
            startTestService();
            sleep(1500);
            bindTestService(true);
            try {
                sTestServicePbap.getState();
                sTestServicePbap.connect(null);
                sTestServicePbap.disconnect();
                sTestServicePbap.isConnected(null);
                sTestServicePbap.getClient(); 
            } catch (RemoteException e) {
                Log.e(TAG, "", e);
            }
            unbindTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test01EnableDisable1] isBluetoothOn is false");
        }
    }
    
    public void test01EnableFail() {
        int waitingTime = 1000;

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;true;enableNative;0;0",
                
                //"Req;0;disableNative;1;0"
        };

        printLog("[API: test01EnableDisable1] : Starting");
        
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ABNORMAL).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());

        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            //stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test01EnableDisable1] isBluetoothOn is false");
        }
    }

    public void test02SetPath1() {
        int waitingTime = 3000;

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;true;enableNative;1;0",
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;true;connectRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;on_use;2",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;on_use;1",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;/telecom/;0",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;pb;0",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;on_use;1",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;error_test_path;0",
                "Req;0;true;setPathRspNative;1;0",
                "Cb;0;false;method_onSetPathInd;void;2;no_use;3",
                "Req;0;true;setPathRspNative;1;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;true;disableNative;1;0"
        };

        printLog("[API: test02SetPath1] : Starting");
        
      //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check enableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //for check connectRspNative
        checkLog.add("true");
        //for check setPathRspNative
        checkLog.add("true;empty");
        checkLog.add("true;empty");
        checkLog.add("true;telecom");
        checkLog.add("true;telecom/pb");
        checkLog.add("true;telecom");
        checkLog.add("false;telecom");
        checkLog.add("true;telecom");
        //for check disableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());
        mJniManager.addServiceCheckInterface(new SetPathRspCheck());

        if (mIsBluetoothOn) {
            startTestService();            
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test02SetPath1] isBluetoothOn is false");
        }
    }

    public void test030SetPathAndAuthChall1() {
        int waitingTime = 3000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;true;enableNative;1;0",
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;true;connectRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;no_use;2",
                "Req;0;true;setPathRspNative;1;0",

                "Cb;0;false;method_onAuthorizeInd;void;1;00:15:83:52:83:17",
                "Req;0;true;authorizeRspNative;0;0",
                //for test AUTHORIZE_ALWAYS_ALLOWED is already true condition
                //"Cb;0;method_onAuthorizeInd;void;1;00:15:83:52:83:17",
                //"Req;0;authorizeRspNative;0;0",

                "Cb;0;false;method_onAuthChallInd;void;3;TEST_EQUIPMENT;1;1",
                "Req;0;true;authChallengeRspNative;0;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;true;disableNative;1;0"
        };

        printLog("[API: test030SetPathAndAuthChall1] : Starting");

        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();

        //for check enableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //for check connectRspNative
        checkLog.add("true");
        //for check setPathRspNative
        checkLog.add("true;empty");
        //for check authorizeRspNative
        checkLog.add("true");
        //for check authChallengeRspNative
        checkLog.add("false;12345;ANDROID BT");
        //for check disableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthorizeRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthChallengeRspCheck());

        if (mIsBluetoothOn) {
            startTestService();
            sleep(2000);
            Intent intent = new Intent(BluetoothPbapService.PBAP_AUTHORIZE_RETURN_ACTION);
            intent.putExtra(BluetoothServerAuthorize.AUTHORIZE_RESULT, BluetoothServerAuthorize.RESULT_USER_ACCEPT)
                    .putExtra(BluetoothServerAuthorize.AUTHORIZE_ALWAYS_ALLOWED, true);
            mTestContext.sendBroadcast(intent);
            intent = null;
            sleep(2000);
            intent = new Intent(BluetoothPbapService.PBAP_AUTHENTICATE_RETURN_ACTION);
            intent.putExtra(BluetoothAuthenticating.AUTHENTICATE_RESULT, BluetoothAuthenticating.RESULT_USER_ACCEPT)
                    .putExtra(BluetoothAuthenticating.AUTHETICATE_CODE, "12345");
            mTestContext.sendBroadcast(intent);
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test030SetPathAndAuthChall1] isBluetoothOn is false");
        }
    }

    public void test031SetPathAndAuthChall2() {
        int waitingTime = 3000;

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;no_use;2",
                "Req;0;false;setPathRspNative;1;0",

                "Cb;0;false;method_onAuthorizeInd;void;1;00:15:83:52:83:17",
                "Req;0;false;authorizeRspNative;0;0",

                "Cb;0;false;method_onAuthChallInd;void;3;TEST_EQUIPMENT;1;1",
                "Req;0;false;authChallengeRspNative;0;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test031SetPathAndAuthChall2] : Starting");

        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check enableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //for check connectRspNative
        checkLog.add("true");
        //for check setPathRspNative
        checkLog.add("true;empty");
        //for check authorizeRspNative
        checkLog.add("true");
        //for check authChallengeRspNative
        checkLog.add("true;null;null");
        //for check disableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthorizeRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthChallengeRspCheck());

        if (mIsBluetoothOn) {
            startTestService();
            sleep(2000);
            Intent intent = new Intent(BluetoothPbapService.PBAP_AUTHORIZE_RETURN_ACTION);
            intent.putExtra(BluetoothServerAuthorize.AUTHORIZE_RESULT, BluetoothServerAuthorize.RESULT_USER_ACCEPT)
                    .putExtra(BluetoothServerAuthorize.AUTHORIZE_ALWAYS_ALLOWED, true);
            mTestContext.sendBroadcast(intent);
            intent = null;
            sleep(2000);
            intent = new Intent(BluetoothPbapService.PBAP_AUTHENTICATE_RETURN_ACTION);
            intent.putExtra(BluetoothAuthenticating.AUTHENTICATE_RESULT, BluetoothAuthenticating.RESULT_USER_REJECT)
                    .putExtra(BluetoothAuthenticating.AUTHETICATE_CODE, "12345");
            mTestContext.sendBroadcast(intent);
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test031SetPathAndAuthChall2] isBluetoothOn is false");
        }
    }

    public void test032SetPathAndAuthChallOutTime1() {
        int waitingTime = 54000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;no_use;2",
                "Req;0;false;setPathRspNative;1;0",

                "Cb;0;false;method_onAuthorizeInd;void;1;00:15:83:52:83:17",
                "Req;0;false;authorizeRspNative;0;0",

                "Cb;0;false;method_onAuthChallInd;void;3;TEST_EQUIPMENT;1;1",
                "Req;0;false;authChallengeRspNative;0;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test032SetPathAndAuthChallOutTime1] : Starting");
        
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check enableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //for check connectRspNative
        checkLog.add("true");
        //for check setPathRspNative
        checkLog.add("true;empty");
        //for check authorizeRspNative
        checkLog.add("false");
        //for check authChallengeRspNative
        checkLog.add("true;null;null");
        //for check disableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthorizeRspCheck());
        mJniManager.addServiceCheckInterface(new SetAuthChallengeRspCheck());

        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API:test032SetPathAndAuthChallOutTime1] isBluetoothOn is false");
        }
    }

    public void test040PullPhoneBook() {
        int waitingTime = 3000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;true;enableNative;1;0",
                
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;true;connectRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",

                //Vcard type is v21
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;1;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/ich.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",

                //Vcard type is v21
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/ich.vcf;536870911;1;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/och.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/mch.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/cch.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;SIM1/telecom/pb.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;error_path.vcf;536870911;0;100;0",
                "Req;0;true;pullPhonebookRspNative;1;0",

                //pull phone book with negative parameter, maxListCount < 0, listStartOffset < 0
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;0;-1;-1",
                "Req;0;true;pullPhonebookRspNative;1;0",

                //pull phone book with negative parameter, listStartOffset > getCount()
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;0;100;100",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;true;disableNative;1;0"
        };

        printLog("[API: test040PullPhoneBook] : Starting");
        
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check enableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //for check connectRspNative
        checkLog.add("true");
        //for check pullPhonebookRspNative
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@0");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@1");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@2");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@3");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@4");
        checkLog.add("Success,65535,1;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@5");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@6");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@7");
        checkLog.add("Failed,65535,65535;null@8");
        checkLog.add("Failed,65535,65535;null@9");
        checkLog.add("Failed,65535,65535;null@10");
        //for check disableNative
        checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new ServiceConditionCheck());
        mJniManager.addServiceCheckInterface(new ConnectRspCheck());
        mJniManager.addServiceCheckInterface(new PullPhonebookRspCheck(0));
        
        mOperateContacts = new PbapOperateContacts();
        initContactInfoList();
        initContactSimInfoList();
        initCallLogInfoList();
        mOperateContacts = null;
        
        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[test040PullPhoneBook] isBluetoothOn is false");
        }
    }

    public void test050PullVcardEntryInd() {
        int waitingTime = 3000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",
                
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",

                "Cb;0;false;method_onSetPathInd;void;2;telecom/pb;0",
                "Req;0;false;setPathRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;2.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                //for case: if (name.endsWith(".vcf")) is false
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;2;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                //for case: index is larger than size
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;100.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                // for call log case
                "Cb;0;false;method_onSetPathInd;void;2;root;2",
                "Req;0;false;setPathRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;telecom/ich;0",
                "Req;0;false;setPathRspNative;1;0",

                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;2.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                //for wrong call log case
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;-2.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                "Cb;0;false;method_onSetPathInd;void;2;root;2",
                "Req;0;false;setPathRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;SIM1/telecom/pb;0",
                "Req;0;false;setPathRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;2.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                //for case: index is larger than size
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;100.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",

                //for case:if (type != BluetoothPbapPath.FOLDER_TYPE_UNKNOWN)
                "Cb;0;false;method_onSetPathInd;void;2;root;2",
                "Req;0;false;setPathRspNative;1;0",

                "Cb;0;false;method_onSetPathInd;void;2;telecom/test;0",
                "Req;0;false;setPathRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardEntryInd;void;4;0;2.vcf;536870911;0",
                "Req;0;true;pullVcardEntryRspNative;1;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test050PullVcardEntryInd] : Starting");
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check onPullVcardEntry result
        checkLog.add("Success;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@0");
        checkLog.add("Failed;null@1");
        checkLog.add("Failed;null@2");
        checkLog.add("Success;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@3");
        checkLog.add("Failed;null@4");
        checkLog.add("Success;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@5");
        checkLog.add("Failed;null@6");
        checkLog.add("Failed;null@7");
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new PullPhonebookRspCheck(1));

        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API: test050PullVcardEntryInd] isBluetoothOn is false");
        }
    }

    public void test060PullPhoneBookMaxListCount() {
        int waitingTime = 3000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",
                
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/ich.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/och.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/mch.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/cch.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;SIM1/telecom/pb.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;0;false;method_onPullPhonebookInd;void;6;0;telecom/pb.vcf;536870911;0;0;0",
                "Req;0;true;pullPhonebookRspNative;1;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test060PullPhoneBookMaxListCount] : Starting");
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check pullPhonebookRspNative
        checkLog.add("Success,6,65535;null@0");
        checkLog.add("Success,4,65535;null@1");
        checkLog.add("Success,1,65535;null@2");
        checkLog.add("Success,1,1;null@3");
        checkLog.add("Success,7,65535;null@4");
        checkLog.add("Success,3,65535;null@5");
        checkLog.add("Success,6,65535;null@6");
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new PullPhonebookRspCheck(2));

        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API: test060PullPhoneBookMaxListCount] isBluetoothOn is false");
        }
    }

    public void test07PullVcardListingInd() {
        int waitingTime = 3000;//this test case need long time to wait time out, 15seconds+

        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",

                //wrong case pull phonebook before connect, mPath == null
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;123;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",

                "Cb;0;false;method_onSetPathInd;void;2;telecom;0",
                "Req;0;false;setPathRspNative;1;0",

                //search name
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;TEST 1;0;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;123;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;1;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //second time can run not qurey path
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;1;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //second time can run not qurey path, but offset is 100, if (mIDList.length <= listOffset)
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;1;1;100;100",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //search value is empty
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;empty;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;mch.vcf;0;4GIVEN_NAME;0;0;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;ich.vcf;0;123;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                "Cb;0;false;method_onPullVcardListingInd;void;7;0;mch.vcf;0;4GIVEN_NAME;0;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                "Cb;0;false;method_onPullVcardListingInd;void;7;0;och.vcf;0;123;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                "Cb;0;false;method_onPullVcardListingInd;void;7;0;cch.vcf;0;123;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //wrong case searchAttr == BluetoothPbapVCardListing.VCARD_SEARCH_SOUND 
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;pb.vcf;0;123;2;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onSetPathInd;void;2;root;2",
                "Req;0;false;setPathRspNative;1;0",
                 
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;SIM1/telecom/pb.vcf;0;134;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //second time can run not qurey path
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;SIM1/telecom/pb.vcf;0;134;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //second time can run not qurey path, but offset is 100, if (mSimIDList.length <= listOffset)
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;SIM1/telecom/pb.vcf;0;134;1;100;100",
                "Req;0;true;pullVcardListingRspNative;1;0",

                //search name mode
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;SIM1/telecom/pb.vcf;0;Sim_given_name;0;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",

                "Cb;0;false;method_onPullVcardListingInd;void;7;0;SIM1/telecom/pb.vcf;0;empty;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;0;false;method_onPullVcardListingInd;void;7;0;errorpath.vcf;0;134;1;100;0",
                "Req;0;true;pullVcardListingRspNative;1;0",
                
                "Cb;1;false;method_onDisconnectInd;void;0",
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test07PullVcardListingInd] : Starting");
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        
        //for check onPullVcardEntry result
        checkLog.add("Failed,65535,65535;null@0");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@1");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@2");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@3");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@4");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@5");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@6");
        checkLog.add("Success,1,1;null@7");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@8");
        checkLog.add("Failed,65535,1;null@9");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@10");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@11");
        checkLog.add("Failed,65535,65535;null@12");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@13");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@14");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@15");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@16");
        checkLog.add("Success,65535,65535;/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf@17");
        checkLog.add("Failed,65535,65535;null@18");
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        mJniManager.addServiceCheckInterface(new PullPhonebookRspCheck(3));
        
        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API: test07PullVcardListingInd] isBluetoothOn is false");
        }
    }
    
    public void test099Abort() {

        int waitingTime = 3000;
        
        String jniLogs[] = {
                "Req;0;false;initializeNativeDataNative;void;0",
                "Req;0;false;enableNative;1;0",
                
                "Cb;1;false;method_onConnectInd;void;3;00:15:83:52:83:17;TEST_Equipment;123",
                "Req;0;false;connectRspNative;1;0",
                
                "Cb;1;false;method_onAbortInd;void;0",
                
                "Cb;0;false;method_PbaplistenIndicationNativeReturnFalse;void;0",
                
                "Req;0;false;wakeupListenerNative;1;0",
                "Req;0;false;disableNative;1;0"
        };

        printLog("[API: test099Abort] : Starting");
        //add assert log
        ArrayList<String> checkLog = new ArrayList<String>();
        //checkLog.add(new Integer(BluetoothProfileManager.STATE_ENABLED).toString());
        //checkLog.add("true");
        //checkLog.add(new Integer(BluetoothProfileManager.STATE_DISABLED).toString());
        
        mJniManager.setCaseLog(jniLogs, ";");
        mJniManager.setCaseExpectedResults(checkLog);
        
        mOperateContacts = new PbapOperateContacts();
        deleteContactInfoList();
        deleteCallLogInfoList();
        deleteContactSimInfoList();
        mOperateContacts = null;
        
        if (mIsBluetoothOn) {
            startTestService();
            sleep(waitingTime); // wait for the jni method and jni callback finished
            stopTestService();
        } else {
            Assert.assertTrue(mIsBluetoothOn);
            printLog("[API: test099Abort] isBluetoothOn is false");
        }
    }
    
    public void test900PrivateMethodAppendPhotos() {
        ArrayList<ContentValues> contentValuesList = new ArrayList<ContentValues>();
        ContentValues contentValues;
        
        String expectString[] = {
                "PHOTO;ENCODING=BASE64;GIF:R0lGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n" +
                " AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n" +
                "PHOTO;ENCODING=BASE64;PNG:iVBORwAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n" +
                "PHOTO;ENCODING=BASE64;JPEG:/9gAAAAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n",
                
                "PHOTO;ENCODING=b;TYPE=GIF:R0lGAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n" +
                " AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n" +
                "PHOTO;ENCODING=b;TYPE=PNG:iVBORwAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n" +
                "PHOTO;ENCODING=b;TYPE=JPEG:/9gAAAAAAAAAAAAAAAAAAAAAAAA=\r\n" +
                "\r\n"
        };
        
        printLog("[API: test900PrivateMethodAppendPhotos]");
        
        PbapSpecialTest specialtest = new PbapSpecialTest(this.getInstrumentation().getContext());
        specialtest.setTestData(mTestDataMap);
        
        //for test if (data == null) 
        contentValues = new ContentValues();
        byte[] phtoDataNull = null;
        contentValues.put(Photo.PHOTO, phtoDataNull); 
        contentValuesList.add(contentValues);
        
        contentValues = new ContentValues();
        byte[] photoDataGif = new byte[80];//lineCount > 72
        photoDataGif[0] = 'G';
        photoDataGif[1] = 'I';
        photoDataGif[2] = 'F';
        contentValues.put(Photo.PHOTO, photoDataGif); 
        contentValuesList.add(contentValues);
        
        contentValues = new ContentValues();
        byte[] photoDataPng = new byte[20];
        photoDataPng[0] = (byte)0x89;
        photoDataPng[1] = 'P';
        photoDataPng[2] = 'N';
        photoDataPng[3] = 'G';
        contentValues.put(Photo.PHOTO, photoDataPng);
        contentValuesList.add(contentValues);
        
        contentValues = new ContentValues();
        byte[] photoDataJpeg = new byte[20];
        photoDataJpeg[0] = (byte)0xff;
        photoDataJpeg[1] = (byte)0xd8;
        contentValues.put(Photo.PHOTO, photoDataJpeg);
        contentValuesList.add(contentValues);
        
        //for Unknown photo type. Ignore.
        contentValues = new ContentValues();
        byte[] photoDataUnknown = new byte[20];
        contentValues.put(Photo.PHOTO, photoDataUnknown);
        contentValuesList.add(contentValues);
        
        mTestDataMap.put(Photo.CONTENT_ITEM_TYPE, contentValuesList);
        
        String testResult = specialtest.testVCardPhoto(true, BluetoothPbapPath.FOLDER_TYPE_PB);
        assertTrue(testResult.equals(expectString[0]));
        
        //VCard v30
        testResult = specialtest.testVCardPhoto(false, BluetoothPbapPath.FOLDER_TYPE_PB);
        assertTrue(testResult.equals(expectString[1]));
        printLog("[API: test900PrivateMethodAppendPhotos] isBluetoothOn is false");
        
    }

    private void initContactInfoList() {
        ArrayList<ContactInfo> testCIList = new ArrayList<ContactInfo>();
        
        // new contacts
        ContactInfo contactInfo = null;
        
        contactInfo = new ContactInfo();
        contactInfo.setCiName(StructuredName.DISPLAY_NAME, "TEST 1");
        contactInfo.setCiName(StructuredName.GIVEN_NAME, "4GIVEN_NAME");
        contactInfo.setCiName(StructuredName.FAMILY_NAME, "1FAMILY_NAME");
        contactInfo.setCiName(StructuredName.PREFIX, "1PREFIX");
        contactInfo.setCiName(StructuredName.MIDDLE_NAME, "1MIDDLE_NAME");
        contactInfo.setCiName(StructuredName.SUFFIX, "1SUFFIX");
        contactInfo.setCiName(StructuredName.PHONETIC_GIVEN_NAME, "1PHONETIC_GIVEN_NAME");
        contactInfo.setCiName(StructuredName.PHONETIC_MIDDLE_NAME, "1PHONETIC_MIDDLE_NAME");
        contactInfo.setCiName(StructuredName.PHONETIC_FAMILY_NAME, "HONETIC_FAMILY_NAME");
        
        contactInfo.setCiNickName("SB");
        
        contactInfo.setCiNote("XXOO记录dd\raaa\nadat\\aaad\r\naet<445a>khhdgg");
        contactInfo.setCiNote("V587");
        
        contactInfo.setCiWebsite("www.11.com");
        contactInfo.setCiWebsite("www.22.com");
        
        contactInfo.setCiPhoneNums("9987", Phone.TYPE_CUSTOM, "new type");
        contactInfo.setCiPhoneNums("443896", Phone.TYPE_CUSTOM, "newtype");
        contactInfo.setCiPhoneNums("12345", Phone.TYPE_HOME, "NO_use");
        contactInfo.setCiPhoneNums("76878", Phone.TYPE_WORK, "NO_use");
        contactInfo.setCiPhoneNums("+46;5", Phone.TYPE_FAX_HOME, "NO_use");//test + and ;
        //phoneNumberList.isEmpty())  is true, because of no number character
        contactInfo.setCiPhoneNums("aaabb", Phone.TYPE_FAX_WORK, "NO_use");
        contactInfo.setCiPhoneNums("44651", Phone.TYPE_FAX_WORK, "NO_use");
        contactInfo.setCiPhoneNums("9078", Phone.TYPE_MOBILE, "NO_use");
        contactInfo.setCiPhoneNums("4544", Phone.TYPE_PAGER, "NO_use");
        contactInfo.setCiPhoneNums("3245", Phone.TYPE_OTHER, "NO_use");
        contactInfo.setCiPhoneNums("78934", Phone.TYPE_CAR, "NO_use");
        contactInfo.setCiPhoneNums("445280", Phone.TYPE_RADIO, "NO_use");
        
        contactInfo.setCiEmail("adfaf@111.com", Email.TYPE_CUSTOM, "newEMail");
        contactInfo.setCiEmail("aadff@111.com", Email.TYPE_CUSTOM, 
                android.provider.Contacts.ContactMethodsColumns.MOBILE_EMAIL_TYPE_NAME);
        contactInfo.setCiEmail("adf634faf@111.com", Email.TYPE_CUSTOM, "");
        contactInfo.setCiEmail("bdfaf@111.com", Email.TYPE_HOME, "NO_use");
        contactInfo.setCiEmail("cdfaf@111.com", Email.TYPE_WORK, "NO_use");
        contactInfo.setCiEmail("ddfaf@111.com", Email.TYPE_OTHER, "NO_use");
        contactInfo.setCiEmail("edfaf@111.com", Email.TYPE_MOBILE, "NO_use");
        
        contactInfo.setCiPostal("street-poBox-neighborhood-city-state-postalCode-country", 
                StructuredPostal.TYPE_HOME, "NO_use");
        contactInfo.setCiPostal("street-poBox-记录-city-state-postalCode-country", 
                StructuredPostal.TYPE_WORK, "NO_use");
        contactInfo.setCiPostal("streetdd-poBox-neighborhood-city-state-postalCode-country", 
                StructuredPostal.TYPE_OTHER, "NO_use");
        contactInfo.setCiPostal("null-poBox-neighborhood-city-state-postalCode-country", 
                StructuredPostal.TYPE_CUSTOM, "user_post");
        contactInfo.setCiPostal("streaaaet-poBox-neighbo44rhood-city-state-postdalCode-country", 0xFF, "NO_use");

        contactInfo.setCiIm("34tsgsm", Im.PROTOCOL_AIM, "NO_use");
        contactInfo.setCiIm("rfg5", Im.PROTOCOL_GOOGLE_TALK, "NO_use");
        
        contactInfo.setCiEvent("w.22.com", Event.TYPE_BIRTHDAY, "no_use");
        contactInfo.setCiEvent("dfa22434", Event.TYPE_ANNIVERSARY, "no_use");
        
        contactInfo.setCiOrganization("HugeSoft", Organization.TYPE_WORK, "no_use", "test_title");
        
        testCIList.add(contactInfo);
        
        // new contacts
        contactInfo = new ContactInfo();
        contactInfo.setCiName(StructuredName.MIDDLE_NAME, "DISPLAY_NAME");//cannot directly insert display name
        
        contactInfo.setCiFlagInfo(StructuredName.IS_PRIMARY, 1);
        contactInfo.setCiFlagInfo(StructuredName.IS_SUPER_PRIMARY, 1);

        contactInfo.setCiPhoneNums("123456记录", Phone.TYPE_HOME, "NO_use");
        
        testCIList.add(contactInfo);

        // new contacts
        contactInfo = new ContactInfo();
        contactInfo.setCiName(StructuredName.MIDDLE_NAME, "记录");//cannot directly insert display name
        contactInfo.setCiName(StructuredName.PHONETIC_GIVEN_NAME, "记录");
        contactInfo.setCiName(StructuredName.PHONETIC_MIDDLE_NAME, "记录");
        contactInfo.setCiName(StructuredName.PHONETIC_FAMILY_NAME, "记录");

        contactInfo.setCiNickName("世界");
        
        testCIList.add(contactInfo);

        // new contacts
        contactInfo = new ContactInfo();
        contactInfo.setCiName(StructuredName.FAMILY_NAME, "family_NAME记录");
        
        contactInfo.setCiNickName("");
        
        contactInfo.setCiPhoneNums("", Phone.TYPE_WORK, "NO_use");

        contactInfo.setCiEmail("", Email.TYPE_HOME, "NO_use");

        contactInfo.setCiPostal("", StructuredPostal.TYPE_HOME, "NO_use");

        testCIList.add(contactInfo);
        // new contacts
        contactInfo = new ContactInfo();
        contactInfo.setCiIm("234g", Im.PROTOCOL_AIM, "NO_use");

        testCIList.add(contactInfo);

        mOperateContacts.insertContactInfo(testCIList, this.getInstrumentation().getContext());
    }
    
    private void initContactSimInfoList() {
        ArrayList<ContactSimInfo> testCISimList = new ArrayList<ContactSimInfo>();
        
        ContactSimInfo contactSimInfo = null;
        
        // new contacts
        contactSimInfo = new ContactSimInfo();
        contactSimInfo.setCiName(StructuredName.GIVEN_NAME, "Sim_given_name");
        
        contactSimInfo.setCiPhoneNums("123597", Phone.TYPE_HOME, "NO_use");
        
        testCISimList.add(contactSimInfo);

        // new contacts
        contactSimInfo = new ContactSimInfo();
        contactSimInfo.setCiName(StructuredName.GIVEN_NAME, "SimName1");
        
        contactSimInfo.setCiPhoneNums("134998", Phone.TYPE_HOME, "NO_use");
        
        testCISimList.add(contactSimInfo);
        mOperateContacts.insertContactSimInfo(testCISimList, this.getInstrumentation().getContext());
        
    }

    private void initCallLogInfoList() {
        ArrayList<CallHistory> testChList = new ArrayList<CallHistory>();
        
        CallHistory callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.INCOMING_TYPE);
        callHistory.setChCalledNumber("12345");
        callHistory.setChCalledNew(false);
        callHistory.setChCalledDate(1340000000000l);
        callHistory.setChNumberName("Test one");
        callHistory.setChNumberType(Phone.TYPE_HOME);
        callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);
        
        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.INCOMING_TYPE);
        callHistory.setChCalledNumber("76878");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000001000l);
        callHistory.setChNumberName("4GIVEN_NAME");
        callHistory.setChNumberType(Phone.TYPE_WORK);
        //callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);

        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.INCOMING_TYPE);
        callHistory.setChCalledNumber("443896");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000002000l);
        //callHistory.setChNumberName("Test four");
        callHistory.setChNumberType(Phone.TYPE_WORK);
        //callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);

        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.INCOMING_TYPE);
        callHistory.setChCalledNumber("");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000003000l);
        callHistory.setChNumberName("4GIVEN_NAME");
        testChList.add(callHistory);
        
        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.OUTGOING_TYPE);
        callHistory.setChCalledNumber("12345");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000004000l);
        callHistory.setChNumberName("Test two");
        callHistory.setChNumberType(Phone.TYPE_HOME);
        callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);
        
        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.MISSED_TYPE);
        callHistory.setChCalledNumber("12345");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000005000l);
        callHistory.setChNumberName("Test three");
        callHistory.setChNumberType(Phone.TYPE_HOME);
        callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);

        callHistory = new CallHistory();
        callHistory.setChCallType(CallLog.Calls.AUTOREJECTED_TYPE);
        callHistory.setChCalledNumber("12345");
        callHistory.setChCalledNew(true);
        callHistory.setChCalledDate(1340000006000l);
        callHistory.setChNumberName("Test three");
        callHistory.setChNumberType(Phone.TYPE_HOME);
        callHistory.setChNumberLable("CanUse");
        testChList.add(callHistory);
        
        mOperateContacts.addCallHistory(testChList, this.getInstrumentation().getContext());
    }
    
    private void deleteContactInfoList() {
        mOperateContacts.deleteAllContacts(this.getInstrumentation().getContext());
    }
    
    private void deleteContactSimInfoList() {
        mOperateContacts.deleteAllSimContacts(this.getInstrumentation().getContext());
    }
    
    private void deleteCallLogInfoList() {
        mOperateContacts.deleteAllCallLogs(this.getInstrumentation().getContext());
    }
    
    /**
     * This Class is to check ServiceStatus result<br>
     * result
     * @author mtk54496
     *
    */
    private class ServiceConditionCheck implements IServiceCheckInterface {
        
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 0:
                case 9:
                    int expectStatus = Integer.parseInt(expectedResult);
                    mAssertBroadcastReciver.setExpectStatus(expectStatus);
                    printLog("Enabled reciver");
                    break;

                default:
                    printLog("[API: ServiceConditionCheck-checkState] unknown id");
                    break;
            }
        }
    }
    /**
     * This Class is to check connectRspNative result<br>
     * result
     * @author mtk54496
     *
    */
    private class ConnectRspCheck implements IServiceCheckInterface {

        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 1:
                    printLog("ConnectRspCheck" + retStringNative + "?=" + expectedResult);
                    assertTrue(retStringNative.equals(expectedResult));
                    initTestTargetItem();
                    assertTrue(mPbapServer != null);
                    assertTrue(mPbapPath == null);
                    assertTrue(mVCardListing == null);
                    printLog("the case 1 assert passed");
                    break;

                default:
                    printLog("[API: ConnectRspCheck-checkState] unknown id");
                    break;
            }
        }
    }
    
    private class SetPathRspCheck implements IServiceCheckInterface {
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 2:
                    initTestTargetItem();
                    printLog("SetPathRspCheck" + retStringNative + mPbapPath.getCurrentPath() + "?=" + expectedResult);
                    String[] expectStrings = expectedResult.split(";");
                    if ("empty".equals(expectStrings[1])) {
                        expectStrings[1] = "";
                    }
                    assertTrue(retStringNative.equals(expectStrings[0]));
                    assertTrue(mPbapPath.getCurrentPath().equals(expectStrings[1]));
                    printLog("the case 2 assert passed");
                    break;

                default:
                    printLog("[API: SetPathRspCheck-checkState] unknown id");
                    break;
            }
        }
    }

    private class SetAuthorizeRspCheck implements IServiceCheckInterface {
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 3:
                    printLog("SetAuthorizeRspCheck" + retStringNative + "?=" + expectedResult);
                    assertTrue(retStringNative.equals(expectedResult));
                    printLog("the case 3 assert passed");
                    break;

                default:
                    printLog("[API: SetAuthorizeRspCheck-checkState] unknown id");
                    break;
            }
        }
    }

    private class SetAuthChallengeRspCheck implements IServiceCheckInterface {
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 4:
                    printLog("SetAuthChallengeRspCheck" + retStringNative + "?=" + expectedResult);
                    assertTrue(retStringNative.equals(expectedResult));
                    printLog("the case 4 assert passed");
                    break;

                default:
                    printLog("[API: SetAuthChallengeRspCheck-checkState] unknown id");
                    break;
            }
        }
    }

    private class PullPhonebookRspCheck implements IServiceCheckInterface {
        private int mType ;
        
        PullPhonebookRspCheck(int type) {
            mType = type;
        }
        
        public void checkState(int id, String retStringNative, String expectedResult) {
            switch (id) {
                case 5:
                    printLog("pullPhonebookRspNative" + retStringNative + "?=" + expectedResult);
                    
                    String expectStrings[] = expectedResult.split("@"); 
                    int expectStringID = Integer.valueOf(expectStrings[1]);
                    
                    assertTrue(retStringNative.equals(expectStrings[0]));
                    if (!retStringNative.equals(expectStrings[0])) {
                        errorLog("retStringNative.equals(expectStrings[0]) not true");
                    }
                    // if return failed, no need check detail info
                    if ((expectStrings[0].startsWith("Failed")) || (expectStrings[0].endsWith(";null"))) {
                        break;
                    }
                    try {
                        FileInputStream fileStream = 
                            new FileInputStream(new File("/data/data/com.mediatek.bluetooth/files/btpbaptmp.vcf"));
                        InputStreamReader streamReader = new InputStreamReader(fileStream, "UTF-8");
                        BufferedReader bufferReader = new BufferedReader(streamReader);
                        char[] charBuffer = new char[10000];
                        bufferReader.read(charBuffer);
                        String phonebook = (String.valueOf(charBuffer)).trim();
                        streamReader.close();
                        String expectString = mVCardStrings.getPhonebook(mType, expectStringID);
                        assertTrue(expectString.equals(phonebook));
                        /*
                        if (expectString.equals(phonebook)) break;
                        Log.v(TAG, phonebook);
                        char result[] = phonebook.toCharArray();
                        char expect[] = expectString.toCharArray();
                        for (int i = 0; i <= 10000; i++) {
                            if ((i >= result.length) || (i >= expect.length)) {
                                break;
                            }
                            if (result[i] != expect[i]) {
                                printLog(result[i] + " != " + expect[i] + " at " + i);
                            } else {
                                //printLog(result[i] + " == " + expect[i] + " at " + i);
                            }
                            
                        }
                        */
                        
                    } catch (FileNotFoundException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (UnsupportedEncodingException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                    
                    printLog("the case 5 assert passed");
                    break;

                default:
                    printLog("[API: pullPhonebookRspNative-checkState] unknown id");
                    break;
            }
        }
    }

    private void initTestTargetItem() {
        mPbapService = BluetoothPbapService.getPbapServiceInstence();
        if (mPbapService != null) {
            mPbapServer = getPbapServerInstance(mPbapService);
            if (mPbapServer != null) {
                mPbapPath = getPbapPathInstance(mPbapServer);
                mVCardListing = getPbapVcardListingInstance(mPbapServer);
            }
        }
    }
    
    private BluetoothPbapServer getPbapServerInstance(BluetoothPbapService service) {
        Class<?> serviceClass;
        try {
            serviceClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothPbapService");
            printLog("[API: getPbapPathInstance] BluetoothPbapService: " + serviceClass.toString());
            Field mField = serviceClass.getDeclaredField("mPbapServer");
            printLog("[API: getPbapPathInstance] mPbapServer : " + mField.toString());
            mField.setAccessible(true);
            BluetoothPbapServer returnServer = (BluetoothPbapServer) mField.get(service);
            if (returnServer == null) {
                printLog("[API: getPbapPathInstance] returnAlertDialog : null");
            } else {
                printLog("[API: getPbapPathInstance] returnAlertDialog : " + returnServer.toString());
            }

            return returnServer;
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
    
    private BluetoothPbapPath getPbapPathInstance(BluetoothPbapServer server) {
        Class<?> serverClass;
        try {
            serverClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothPbapServer");
            printLog("[API: getPbapPathInstance] BluetoothPbapServer: " + serverClass.toString());
            Field mField = serverClass.getDeclaredField("mPath");
            printLog("[API: getPbapPathInstance] mPath : " + mField.toString());
            mField.setAccessible(true);
            BluetoothPbapPath returnPath = (BluetoothPbapPath) mField.get(server);
            if (returnPath == null) {
                printLog("[API: getPbapPathInstance] returnPath : null");
            } else {
                printLog("[API: getPbapPathInstance] returnPath : " + returnPath.toString());
            }
            return returnPath;
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
    
    private BluetoothPbapVCardListing getPbapVcardListingInstance(BluetoothPbapServer server) {
        Class<?> serverClass;
        try {
            serverClass = Class.forName("com.mediatek.bluetooth.pbap.BluetoothPbapServer");
            printLog("[API: getPbapPathInstance] BluetoothPbapServer: " + serverClass.toString());
            Field mField = serverClass.getDeclaredField("mVcardListing");
            printLog("[API: getPbapPathInstance] mVcardListing : " + mField.toString());
            mField.setAccessible(true);
            BluetoothPbapVCardListing returnListing = (BluetoothPbapVCardListing) mField.get(server);
            if (returnListing == null) {
                printLog("[API: getPbapPathInstance] BluetoothPbapVCardListing : null");
            } else {
                printLog("[API: getPbapPathInstance] BluetoothPbapVCardListing : " + returnListing.toString());
            }
            return returnListing;
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
    
    private class AssertBroadcastReceiver extends BroadcastReceiver {
        private int mExpectStatus = -1;
        
        public void setExpectStatus(int status) {
            mExpectStatus = status;
        }
        
        
        public void onReceive(Context context, Intent intent) {
            printLog("the check reciver is invoiked");
            BluetoothProfileManager.Profile profile = 
                (Profile) intent.getSerializableExtra(BluetoothProfileManager.EXTRA_PROFILE);
            if (profile == BluetoothProfileManager.Profile.Bluetooth_PBAP) {
                if (mExpectStatus >= 0) {
                    printLog("Checking pbap status condition");
                    int currentStatus = intent.getIntExtra(BluetoothProfileManager.EXTRA_NEW_STATE, 0);
                    printLog("get the status is " + currentStatus);
                    if (currentStatus == BluetoothProfileManager.STATE_ENABLING) {
                        //because broadcast always have some delay, our receiver sometimes receive last STATE_ENABLING
                        return;
                    }
                    printLog("get the broadcast is " + currentStatus + "?=" + mExpectStatus);
                    assertTrue(currentStatus == mExpectStatus);
                    mExpectStatus = -1;
                } else {
                    printLog("Uncared broadcast");
                }
                
            }
        }
    }
    
    /*
     * start service by mTestContext
     */
    private void startTestService() {
        //Intent intent = new Intent(IBluetoothPbap.class.getName());
        Intent intent = new Intent(mTestContext, BluetoothPbapService.class);
        mTestContext.startService(intent);
        printLog("[API:startTestService] start service");
    }

    /*
     * stop service by mTestContext the sleep method is to make sure the BluetoothPbapSerice can call onDestroy and do the
     * clean work(including java and jni)
     */
    private void stopTestService() {
        Intent intent = new Intent(mTestContext, BluetoothPbapService.class);
        mTestContext.stopService(intent);
        printLog("[API:stopTestService] stopService completed");
        sleep(3000); // sleep to make sure
    }

    /*
     * bind service by mTestContext bind service until bind succeed
     */
    private boolean bindTestService(boolean success) {
        boolean bondRes = false;
        int timeout = 0;
        if (mTestConnection != null) {
            while (sTestServicePbap == null) {
                printLog("[API:bindTestService] Service in while is null and start it");
                //Intent intent = new Intent(IBluetoothPbap.class.getName());
                Intent intent = new Intent(mTestContext, BluetoothPbapService.class);
                if (success) {
                    intent.setAction(IBluetoothPbap.class.getName());
                } else {
                    intent.setAction("NO_USE_TEST, for unsuccess test");
                }
                if (!bondRes) {
                    if (!mTestContext.bindService(intent, mTestConnection, Context.BIND_AUTO_CREATE)) {
                        errorLog("Bind Service failed in while");
                    } else {
                        bondRes = true;
                    }
                }
                timeout++;
                if (timeout > 20) {
                    printLog("Bind service out of time");
                    return false;
                }
                sleep(100);
            }
        } else {
            printLog("[API:bindTestService] mTestConnection is null");
            return false;
        }
        return true;
    }

    /*
     * unbind service by mTestContext
     */
    private void unbindTestService() {
        if (sTestServicePbap != null) {
            if (mTestConnection != null) {
                mTestContext.unbindService(mTestConnection);
                printLog("[API:unbindTestService] unbindService completed!");
                sleep(3000);
            }
        } else {
            printLog("[API:unbindTestService] mTestServicePbap is null");
        }
    }

    /*
     * sleep for a while
     */
    private void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /*
     * print the debug log
     */
    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
    
    private void errorLog(String msg) {
        Log.e(TAG, msg);
    }
}
