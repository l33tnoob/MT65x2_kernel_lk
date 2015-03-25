package com.hissage.tests;

import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.Context;
import android.content.Intent;
import android.content.ComponentName;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.ServiceManagerExt;
import com.mediatek.imsp.IpMessagePluginExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.mms.ipmessage.ServiceManager;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class ServiceManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private IpMessagePluginExt mIpMessagePluginExt;
    private ServiceManager mServiceManager;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mIpMessagePluginExt = new IpMessagePluginExt(mContext);
        mServiceManager = mIpMessagePluginExt.getServiceManager(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01getIpMessageServiceId() throws Exception {
        mServiceManager.getIpMessageServiceId();
    }

    public void test02startIpService() throws Exception {
        mServiceManager.startIpService();
    }

    public void test03serviceIsReady() throws Exception {
        mServiceManager.serviceIsReady();
    }

    public void test04isActivated() throws Exception {
        mServiceManager.isActivated();
    }

    public void test05isActivatedSimId() throws Exception {
        mServiceManager.isActivated(1);
    }

    public void test06isEnabled() throws Exception {
        mServiceManager.isEnabled();
    }

    public void test07isEnabledSimId() throws Exception {
        mServiceManager.isEnabled(1);
    }

    public void test08getActivationStatus() throws Exception {
        mServiceManager.getActivationStatus();
    }

    public void test09getActivationStatus() throws Exception {
        mServiceManager.getActivationStatus(1);
    }

    public void test10enableIpService() throws Exception {
        mServiceManager.enableIpService();
    }

    public void test11enableIpService() throws Exception {
        mServiceManager.enableIpService(1);
    }

    public void test12disableIpService() throws Exception {
        mServiceManager.disableIpService();
    }

    public void test13disableIpService() throws Exception {
        mServiceManager.disableIpService(1);
    }

    public void test14isFeatureSupported() throws Exception {
        mServiceManager.isFeatureSupported(FeatureId.CHAT_SETTINGS);
        mServiceManager.isFeatureSupported(FeatureId.APP_SETTINGS);
        mServiceManager.isFeatureSupported(FeatureId.ACTIVITION);
        mServiceManager.isFeatureSupported(FeatureId.ACTIVITION_WIZARD);
        mServiceManager.isFeatureSupported(FeatureId.ALL_LOCATION);
        mServiceManager.isFeatureSupported(FeatureId.ALL_MEDIA);
        mServiceManager.isFeatureSupported(FeatureId.MEDIA_DETAIL);
        mServiceManager.isFeatureSupported(FeatureId.GROUP_MESSAGE);
        mServiceManager.isFeatureSupported(FeatureId.CONTACT_SELECTION);
        mServiceManager.isFeatureSupported(FeatureId.SKETCH);
        mServiceManager.isFeatureSupported(FeatureId.LOCATION);
        mServiceManager.isFeatureSupported(FeatureId.TERM);
        mServiceManager.isFeatureSupported(FeatureId.SAVE_CHAT_HISTORY);
        mServiceManager.isFeatureSupported(FeatureId.SAVE_ALL_HISTORY);
        mServiceManager.isFeatureSupported(FeatureId.SHARE_CHAT_HISTORY);
        mServiceManager.isFeatureSupported(FeatureId.SHARE_ALL_HISTORY);
        mServiceManager.isFeatureSupported(100);
    }
}
