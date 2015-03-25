package com.mediatek.voicecommand.tests;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.common.voicecommand.IVoiceCommandListener;

public class VoiceCommandTest extends InstrumentationTestCase {
    private static final String TAG = "VoiceCommandTest";
    private Context mContext = null;
    private IVoiceCommandManagerService mVCmdMgrService;
    private boolean mVCmdIsRegistered = false;
    private String mPkgName;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        Log.i(TAG, "setUp");
        mContext = getInstrumentation().getTargetContext();
        mPkgName = mContext.getPackageName();
        Log.d(TAG, "setUp mPkgName : " + mPkgName);
        bindVoiceService();
    }

    @Override
    protected void tearDown() throws Exception {
        mContext.unbindService(mVoiceSerConnection);
        mVCmdIsRegistered = false;
        mVCmdMgrService = null;
        super.tearDown();
    }

    private void bindVoiceService() {
        Log.v(TAG, "Bind voice service.");
        Intent mVoiceServiceIntent = new Intent();
        mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
        mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
        mContext.bindService(mVoiceServiceIntent, mVoiceSerConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mVoiceSerConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            // TODO Auto-generated method stub
            mVCmdMgrService = IVoiceCommandManagerService.Stub.asInterface(service);
            Log.i(TAG, "ServiceConnection onServiceConnected.");
            registerVoiceCommand(mPkgName);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            // TODO Auto-generated method stub
            Log.v(TAG, "Service disconnected");
            mVCmdIsRegistered = false;
            mVCmdMgrService = null;
        }
    };

    private void sendVoiceCommand(String pkgName, int mainAction, int subAction, Bundle extraData) {
        if (mVCmdIsRegistered) {
            try {
                Log.v(TAG, "Send Command " + "pkgName" + pkgName + "mainAction=" + mainAction
                        + " subAction=" + subAction + " extraData=" + extraData);
                int errorid = mVCmdMgrService
                        .sendCommand(pkgName, mainAction, subAction, extraData);
                if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                    Log.v(TAG, "Send Command failure");
                } else {
                    Log.v(TAG, "Send Command success");
                }
            } catch (RemoteException e) {
                mVCmdIsRegistered = false;
                mVCmdMgrService = null;
                Log.v(TAG, "sendCommand RemoteException");
            }
        } else {
            Log.v(TAG, "App has not register listener can not send command");
        }
    }

    private void registerVoiceCommand(String pkgName) {
        if (!mVCmdIsRegistered) {
            try {
                int errorid = mVCmdMgrService.registerListener(pkgName, mCallback);
                Log.v(TAG, "Register voice Listener pkgName = " + pkgName + ",errorid = " + errorid);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    mVCmdIsRegistered = true;
                } else {
                    Log.v(TAG, "Register voice Listener failure ");
                }
            } catch (RemoteException e) {
                mVCmdMgrService = null;
                Log.v(TAG, "Register voice Listener RemoteException = " + e.getMessage());
            }
        } else {
            Log.v(TAG, "App has register voice listener success");
        }
        Log.v(TAG, "Register voice listener end! mVCmdIsRegistered = " + mVCmdIsRegistered);
    }

    private void unRegisterVoiceCommand(String pkgName) {
        try {
            int errorid = mVCmdMgrService.unregisterListener(pkgName, mCallback);
            Log.v(TAG, "Unregister voice listener, errorid = " + errorid);
            if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                mVCmdIsRegistered = false;
            }
        } catch (RemoteException e) {
            Log.v(TAG, "Unregister error in handler RemoteException = " + e.getMessage());
            mVCmdIsRegistered = false;
            mVCmdMgrService = null;
        }
        Log.v(TAG, "UnRegister voice listener end! mVCmdIsRegistered = " + mVCmdIsRegistered);
    }

    // Callback used to notify apps
    private IVoiceCommandListener mCallback = new IVoiceCommandListener.Stub() {

        public void onVoiceCommandNotified(int mainAction, int subAction, Bundle extraData)
                throws RemoteException {
            Log.v(TAG, "onVoiceCommandNotified --> handleVoiceCommandNotified");
            Message.obtain(mHandler, mainAction, subAction, 0, extraData).sendToTarget();
        }
    };

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            handleVoiceCommandNotified(msg.what, msg.arg1, (Bundle) msg.obj);
        };
    };

    private void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData) {
        Log.v(TAG, "handleVoiceCommandNotified mainAction = " + mainAction
                + ",subAction" + subAction + "extraData" + extraData);
        int result = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT);
        switch (mainAction) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_COMMON:
            assertEquals(VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, result);
            String[] keyword = extraData
                    .getStringArray(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
            Log.i(TAG, "keyword = " + keyword);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            if (subAction == VoiceCommandListener.ACTION_VOICE_UI_NOTIFY) {
                int uiResult = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                int uiConfidence = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                Log.i(TAG, "uiResult = " + uiResult + "uiConfidence = " + uiConfidence);
            }
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            if (subAction == VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY) {
                int trainingResult = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                int trainingConfidence = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                Log.i(TAG, "trainingResult = " + trainingResult + "trainingConfidence = "
                        + trainingConfidence);
            } else if (subAction == VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY) {
                int intensity = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                Log.i(TAG, "intensity = " + intensity);
            } else if (subAction == VoiceCommandListener.ACTION_VOICE_TRAINING_PSWDFILE) {
                int pswdfile = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                Log.i(TAG, "pswdfile = " + pswdfile);
            }
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
             assertEquals(VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, result);
            if (subAction == VoiceCommandListener.ACTION_VOICE_RECOGNIZE_NOTIFY) {
                int recognizeResult = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                int recognizeConfidence = extraData
                        .getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1);
                Log.i(TAG, "recognizeResult = " + recognizeResult + "recognizeConfidence = "
                        + recognizeConfidence);
            } else if (subAction == VoiceCommandListener.ACTION_VOICE_RECOGNIZE_INTENSITY) {
                int intensity = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
                Log.i(TAG, "intensity = " + intensity);
            }
            break;
        default:
            break;
        }
    }

     public void test01_SendVoiceUI() {
        Log.i(TAG, "test01_VoiceUIRegister");
        if (mVCmdMgrService == null) {
            bindVoiceService();
        } else {
            registerVoiceCommand(mPkgName);
        }
        TestSleep.sleep(1000);
        // Normal case
        Log.i(TAG, "test01_voiceCommonCommand, " + "mVoiceUIPkgName=" + mPkgName);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_COMMON,
                VoiceCommandListener.ACTION_VOICE_COMMON_KEYWORD, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_COMMON,
                VoiceCommandListener.ACTION_VOICE_COMMON_COMMANDPATH, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_COMMON,
                VoiceCommandListener.ACTION_VOICE_COMMON_PROCSTATE, null);
        TestSleep.sleep(1000);
        Log.i(TAG, "test01_VoiceUiCommand");
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_START, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_STOP, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_ENABLE, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_DISALBE, null);
        TestSleep.sleep(1000);

        // First Training, then start UI
        Log.i(TAG, "test01_VoiceTrainingCommand");
        Bundle bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        int[] commandMask = { 5, 4 };
        bundle.putIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, commandMask);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_START, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_START, null);
        TestSleep.sleep(1000);
        // First Training, then stop UI
        Log.i(TAG, "test01_VoiceTrainingCommand stop UI");
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_START, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_STOP, null);
        TestSleep.sleep(1000);

        Log.i(TAG, "test01_VoiceUIUnregister");
        if (mVCmdMgrService != null) {
            unRegisterVoiceCommand(mPkgName);
        }
    }

    public void test02_SendVoiceTraining() {
        Log.i(TAG, "test02_VoiceTrainingRegister");
        if (mVCmdMgrService == null) {
            bindVoiceService();
        } else {
            registerVoiceCommand(mPkgName);
        }
        TestSleep.sleep(1000);

        //Normal case
        Log.i(TAG, "test02_VoiceTrainingCommand");
        Bundle bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        int[] commandMask = { 5, 4 };
        bundle.putIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, commandMask);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_START, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_PSWDFILE, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY, null);
        TestSleep.sleep(1000);
        bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_RESET, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);
        TestSleep.sleep(1000);

        //First start UI, then start training.
        Log.i(TAG, "test02_VoiceUiCommand start training");
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_START, null);
        TestSleep.sleep(10000);
        bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        bundle.putIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, commandMask);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_START, bundle);
        TestSleep.sleep(1000);

         //First start UI, then reset training.
        Log.i(TAG, "test02_VoiceUiCommand reset training");
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                VoiceCommandListener.ACTION_VOICE_UI_START, null);
        TestSleep.sleep(10000);
        bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_RESET, bundle);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_PSWDFILE, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_RESET, null);

        //Stop twice training
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);
        TestSleep.sleep(1000);

        Log.i(TAG, "test02_VoiceTrainingUnregister");
        if (mVCmdMgrService != null) {
            unRegisterVoiceCommand(mPkgName);
        }
    }

    public void test03_SendVoiceRecognize() {
        Log.i(TAG, "test03_voiceRecognizeRegister");
        if (mVCmdMgrService == null) {
            bindVoiceService();
        } else {
            registerVoiceCommand(mPkgName);
        }
        TestSleep.sleep(1000);
        //Normal case
        Log.i(TAG, "test03_voiceRecognizeCommand");
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
                VoiceCommandListener.ACTION_VOICE_RECOGNIZE_START, null);
        TestSleep.sleep(1000);
        sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
                VoiceCommandListener.ACTION_VOICE_RECOGNIZE_INTENSITY, null);
        TestSleep.sleep(1000);

        //First start UI, then start recognize.
       Log.i(TAG, "test03_VoiceUiCommand start recognize");
       sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_UI,
               VoiceCommandListener.ACTION_VOICE_UI_START, null);
       TestSleep.sleep(10000);
       sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
               VoiceCommandListener.ACTION_VOICE_RECOGNIZE_START, null);
       TestSleep.sleep(1000);
       //Stop twice recognize
       Log.i(TAG, "test03_VoiceUiCommand Stop twice recognize");
       sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
               VoiceCommandListener.ACTION_VOICE_RECOGNIZE_INTENSITY, null);
       TestSleep.sleep(1000);
       sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE,
               VoiceCommandListener.ACTION_VOICE_RECOGNIZE_INTENSITY, null);
       TestSleep.sleep(1000);

        Log.i(TAG, "test03_voiceRecognizeUnregister");
        if (mVCmdMgrService != null) {
            unRegisterVoiceCommand(mPkgName);
        }
    }
}