package com.mediatek.voicecommand.tests;

import java.util.Locale;

import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.VoiceMessage;

public class CFManagerTest extends InstrumentationTestCase {
    private static final String TAG = "JNITestAdapterTest";
    private Context mContext = null;
    private ConfigurationManager mCfgMgr;
    private int mVoicecmdid = 1;
    private int mMsgid = 1;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mCfgMgr = ConfigurationManager.getInstance(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_ConfigurationManager() {
        mCfgMgr.useSystemLanguage();
        TestSleep.sleep(1000);
        String systemLanguage = Locale.getDefault().getLanguage() + "-"
                + Locale.getDefault().getCountry();
        mCfgMgr.updateCurLanguageIndex(systemLanguage);
    }

    public void test02_VoiceMessage() {
        VoiceMessage voiceMessage = new VoiceMessage();
        VoiceMessage self = voiceMessage.copySelf(true);
        assertNotNull(self);
    }
}