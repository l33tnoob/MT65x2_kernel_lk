package com.mediatek.voicecommand.tests;

import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.voicecommand.adapter.JNICommandAdapter;
import com.mediatek.voicecommand.adapter.JNITestAdapter;
import com.mediatek.voicecommand.adapter.RecognitionResult;
import com.mediatek.voicecommand.mgr.AppDataManager;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.NativeDataManager;
import com.mediatek.voicecommand.mgr.ServiceDataManager;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;
import com.mediatek.common.voicecommand.VoiceCommandListener;

public class JNIAdapterTest extends InstrumentationTestCase {
    private static final String TAG = "JNITestAdapterTest";
    private Context mContext = null;
    private ConfigurationManager mCfgMgr;
    private VoiceCommandManagerStub mVoiceCommandManagerStub;
    private AppDataManager mAppDataManager;
    private ServiceDataManager mServiceDataManager;
    private NativeDataManager mNativeDataManager;
    private JNITestAdapter mJNITestAdapter;
    private JNICommandAdapter mJNICommandAdapter;
    private String mProcessName;
    private int mPid = 2000;
    private int mVoicecmdid = 1;
    private int mMsgid = 1;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mProcessName = mContext.getPackageName();
        mCfgMgr = ConfigurationManager.getInstance(mContext);
        mVoiceCommandManagerStub = new VoiceCommandManagerStub(mContext);
        mAppDataManager = new AppDataManager(mVoiceCommandManagerStub);
        mServiceDataManager = new ServiceDataManager(mVoiceCommandManagerStub);
        mNativeDataManager = new NativeDataManager(mVoiceCommandManagerStub);
        // set the order of dispatcher;
        mAppDataManager.setDownDispatcher(mServiceDataManager);
        mServiceDataManager.setUpDispatcher(mAppDataManager);
        mServiceDataManager.setDownDispatcher(mNativeDataManager);
        mNativeDataManager.setUpDispatcher(mServiceDataManager);

        mJNITestAdapter = new JNITestAdapter(mNativeDataManager, mCfgMgr);
        mJNICommandAdapter = new JNICommandAdapter(mNativeDataManager);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_voiceUI() {
        isNativePrepared();
        setCurHeadsetMode(true);
        String modelpath = mCfgMgr.getModelFile();
        String patternpath = mCfgMgr.getVoiceUIPatternPath();
        int languageid = mCfgMgr.getCurrentLanguageID();
        // Test JNITestAdapter
        int errorid = mJNITestAdapter.startVoiceUI(modelpath, patternpath, mProcessName,
                mPid, languageid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        errorid = mJNITestAdapter.stopVoiceUI(mProcessName, mPid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        stopCurMode();
        release();
    }

    public void test02_voiceTraining() {
        isNativePrepared();
        setCurHeadsetMode(true);
        String pwdpath = mCfgMgr.getPasswordFilePath();
        String patternpath = mCfgMgr.getVoiceRecognitionPatternFilePath();
        String featurepath = mCfgMgr.getFeatureFilePath();
        String umbpath = mCfgMgr.getUBMFilePath();
        int[] commandMask = { 5, 4 };
        // Test JNITestAdapter
        int errorid = mJNITestAdapter.startVoiceTraining(pwdpath, patternpath, featurepath,
                umbpath, 1, commandMask, mProcessName, mPid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        getNativeIntensity();
        errorid = mJNITestAdapter
                .resetVoiceTraining(pwdpath, patternpath, featurepath, 1);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        mJNITestAdapter.stopVoiceTraining(mProcessName, mPid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        stopCurMode();
        release();
    }

    public void test03_voicePWRecognition() {
        isNativePrepared();
        setCurHeadsetMode(true);
        RecognitionResult recognitionResult = new RecognitionResult(
                mVoicecmdid, mMsgid);
        recognitionResult.set(2, 2);
        RecognitionResult result = new RecognitionResult(recognitionResult);
        String resultString = result.toString();
        assertNotNull(resultString);

        // Test JNITestAdapter
        String patternpath = mCfgMgr.getVoiceRecognitionPatternFilePath();
        String ubmpath = mCfgMgr.getUBMFilePath();
        int errorid = mJNITestAdapter.startVoicePWRecognition(patternpath, ubmpath,
                mProcessName, mPid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        errorid = mJNITestAdapter.stopVoicePWRecognition(mProcessName, mPid);
        TestSleep.sleep(1000);
        assertEquals(errorid, VoiceCommandListener.VOICE_NO_ERROR);
        stopCurMode();
        release();
    }

    public void getNativeIntensity() {
        // Test JNITestAdapter
        int intensity = mJNITestAdapter.getNativeIntensity();
        TestSleep.sleep(1000);
        assertNotNull(intensity);

        // Test JNICommandAdapter
        intensity = mJNICommandAdapter.getNativeIntensity();
        TestSleep.sleep(1000);
        assertNotNull(intensity);
    }

    public void isNativePrepared() {
        // Test JNITestAdapter
        boolean isPrepared = mJNITestAdapter.isNativePrepared();
        TestSleep.sleep(1000);
        assertNotNull(isPrepared);

        // Test JNICommandAdapter
        isPrepared = mJNICommandAdapter.isNativePrepared();
        TestSleep.sleep(1000);
        assertNotNull(isPrepared);
    }

    public void setCurHeadsetMode(boolean mode) {
        // Test JNITestAdapter
        mJNITestAdapter.setCurHeadsetMode(mode);
        TestSleep.sleep(1000);

        // Test JNICommandAdapter
        mJNICommandAdapter.setCurHeadsetMode(mode);
        TestSleep.sleep(1000);
    }

    public void release() {
        // Test JNITestAdapter
        mJNITestAdapter.release();
        TestSleep.sleep(1000);

        // Test JNICommandAdapter
        mJNICommandAdapter.release();
        TestSleep.sleep(1000);
    }

    public void stopCurMode() {
        mJNICommandAdapter.stopCurMode(mProcessName, mPid);
        TestSleep.sleep(1000);
    }

}