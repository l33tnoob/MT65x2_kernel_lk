package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.NotificationsManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.INotificationsListener;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class NotificationsManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private INotificationListenerImpl nl;
    private INotificationListenerImpl nl2;
    private NotificationsManagerExt mNotificationsManagerExt;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mNotificationsManagerExt = new NotificationsManagerExt(mContext);
        nl = new INotificationListenerImpl();
        nl2 = new INotificationListenerImpl();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01Register() throws Exception {
        mNotificationsManagerExt.registerNotificationsListener(nl);
        mNotificationsManagerExt.registerNotificationsListener(nl);
    }

    public void test02Unregister() throws Exception {
        mNotificationsManagerExt.unregisterNotificationsListener(nl);
        mNotificationsManagerExt.unregisterNotificationsListener(nl2);
    }

    public void test03Notify() throws Exception {
        mNotificationsManagerExt.notify(null);
    }
}
