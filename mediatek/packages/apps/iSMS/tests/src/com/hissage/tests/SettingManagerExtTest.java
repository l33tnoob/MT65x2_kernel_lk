package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.SettingsManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class SettingManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private SettingsManagerExt mSettingsManagerExt;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mSettingsManagerExt = new SettingsManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01isCaptionOn() throws Exception {
        mSettingsManagerExt.isCaptionOn();
    }

    public void test02isVideoCaptionOn() throws Exception {
        mSettingsManagerExt.isVideoCaptionOn();
    }

    public void test03isAudioCaptionOn() throws Exception {
        mSettingsManagerExt.isAudioCaptionOn();
    }

    public void test04isPhotoCaptionOn() throws Exception {
        mSettingsManagerExt.isPhotoCaptionOn();
    }

    public void test05isAutoDownloadOn() throws Exception {
        mSettingsManagerExt.isAutoDownloadOn();
    }
}
