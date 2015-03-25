package com.mediatek.voicecommand.adapter;

import java.util.Arrays;
import java.util.Random;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.cfg.VoiceProcessInfo;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class JNITestAdapter implements IVoiceAdapter {

    private int mCurMode;
    private final ConfigurationManager mCfgMgr;
    private final int MODE_VOICE_UNKNOW = -1;

    private final int mDelaytime = 5000;

    private final IMessageDispatcher mUpDispatcher;

    // private VoiceProcessInfo curProcessInfo ;

    private final Handler curHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                sendVoiceTrainingCommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                sendVoiceUICommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                sendVoiceContactsCommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            default:
                break;
            }
        }

    };

    /*
     * test adapter constructor
     *
     * @param dispatcher
     *
     * @param cfgmgr
     */
    public JNITestAdapter(IMessageDispatcher dispatcher,
            ConfigurationManager cfgmgr) {
        mCurMode = MODE_VOICE_UNKNOW;
        mCfgMgr = cfgmgr;
        mUpDispatcher = dispatcher;
    }

    private void sendVoiceUICommand(int curnumber, VoiceProcessInfo processinfo) {
        if (curnumber >= processinfo.mCommandIDList.size()) {
            return;
        }
        int commandid = processinfo.mCommandIDList.get(curnumber);

        VoiceMessage message = new VoiceMessage();
//        message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
        // msg.arg1 is the command id
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, commandid,
                null);
        mUpDispatcher.dispatchMessageUp(message);
        Message m = curHandler.obtainMessage(
                VoiceCommandListener.ACTION_MAIN_VOICE_UI, ++curnumber, 0,
                processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);

    }

    private void sendVoiceContactsCommand(int curnumber,
            VoiceProcessInfo processinfo) {
        String[] command = { "resultWang", "测试王",
                "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" };

        VoiceMessage message = new VoiceMessage();
//        message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;

        // if msg.arg1 is 0 notify speechDetected, if is 1 notify Contacts name.
        if (curnumber == 0) {
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_SPEECHDETECTED;
            message.mExtraData = DataPackage.packageSuccessResult();
        } else if (curnumber == 1) {
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, command, null);
        } else {
            return;
        }
        mUpDispatcher.dispatchMessageUp(message);

        Message m = curHandler.obtainMessage(
                VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS, ++curnumber,
                0, processinfo);
        curHandler.sendMessage(m);
    }

    private void sendVoiceTrainingCommand(int curnumber,
            VoiceProcessInfo processinfo) {
        VoiceMessage message = new VoiceMessage();
//        message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;

        int commandid = 0;
        if (curnumber < 5) {
            commandid = 1;
            curnumber++;
        }

        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, commandid,
                curnumber * 20);

        mUpDispatcher.dispatchMessageUp(message);
        if (commandid == 1) {
            Message m = curHandler.obtainMessage(
                    VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING, curnumber,
                    0, processinfo);
            curHandler.sendMessageDelayed(m, mDelaytime);
        }
    }

    @Override
    public int startVoiceUI(String modelpath, String patternpath,
            String processname, int pid, int languageid) {
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }

        int index = 0;

        Message m = curHandler.obtainMessage(
                VoiceCommandListener.ACTION_MAIN_VOICE_UI, index, 0,
                processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceUI(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int startVoiceContacts(String processname, int pid, int screenOrientation) {
        Log.i(VoiceCommandManagerStub.TAG, "startVoiceContacts screenOrientation " + screenOrientation);
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }
        Message m = curHandler.obtainMessage(
                VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS, 0, 0,
                processinfo);
//        curHandler.sendMessageDelayed(m, mDelaytime);
        curHandler.sendMessage(m);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceContacts(String processname, int pid) {
        Log.i(VoiceCommandManagerStub.TAG, "startVoiceContacts");
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int sendContactsName(String modelpath, String contactspath, String[] allContactsName) {
        Log.i(VoiceCommandManagerStub.TAG, "sendVoiceContacts : " + Arrays.toString(allContactsName));
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsSelected(String selectedName) {
        Log.i(VoiceCommandManagerStub.TAG, "sendContactsSelected : " + selectedName);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsOrientation(int screenOrientation) {
        Log.i(VoiceCommandManagerStub.TAG, "sendContactsOrientation : " + screenOrientation);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsSearchCnt(int searchCnt) {
        Log.i(VoiceCommandManagerStub.TAG, "sendContactsSearchCnt : " + searchCnt);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public void stopCurMode(String processname, int pid) {
        // TODO Auto-generated method stub
        // return 0;
    }

    @Override
    public int getNativeIntensity() {
        // TODO Auto-generated method stub
        return new Random(100).nextInt(100);
    }

    @Override
    public boolean isNativePrepared() {
        // TODO Auto-generated method stub
        return false;
    }

    private int mRecognitionCommandid = 0;

    @Override
    public int startVoicePWRecognition(String patternpath, String ubmpath,
            String processname, int pid) {
        // TODO Auto-generated method stub
        VoiceMessage message = new VoiceMessage();
        message.mPkgName = processname;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                mRecognitionCommandid++, null);
        if (mRecognitionCommandid > 3) {
            mRecognitionCommandid = 0;
        }
        mUpDispatcher.dispatchMessageUp(message);
        return 0;
    }

    @Override
    public int startVoiceTraining(String pwdpath, String patternpath,
            String featurepath, String umbpath, int commandid,
            int[] commandMask, String processname, int pid) {
        // TODO Auto-generated method stub
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }
        Message m = curHandler.obtainMessage(
                VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING, 0, 0,
                processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int resetVoiceTraining(String pwdpath, String patternpath,
            String featurepath, int commandid) {
        // TODO Auto-generated method stub
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoicePWRecognition(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceTraining(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;

    }

    @Override
    public void release() {
        // TODO Auto-generated method stub

    }

    @Override
    public void setCurHeadsetMode(boolean isPlugin) {
        // TODO Auto-generated method stub

    }

}
