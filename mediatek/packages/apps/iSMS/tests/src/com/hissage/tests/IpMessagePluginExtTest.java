package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.ipmessage.ActivitiesManager;
import com.mediatek.mms.ipmessage.ChatManager;
import com.mediatek.mms.ipmessage.ContactManager;
import com.mediatek.mms.ipmessage.GroupManager;
import com.mediatek.mms.ipmessage.IpMessagePluginImpl;
import com.mediatek.mms.ipmessage.MessageManager;
import com.mediatek.mms.ipmessage.NotificationsManager;
import com.mediatek.mms.ipmessage.ResourceManager;
import com.mediatek.mms.ipmessage.ServiceManager;
import com.mediatek.mms.ipmessage.SettingsManager;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;

import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class IpMessagePluginExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    public IIpMessagePlugin mIpMessagePlugin;
    public static final String PLUGIN_VERSION = "1.0.0";
    public static final String PLUGIN_METANAME = "class";
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mIpMessagePlugin = (IIpMessagePlugin)PluginManager.createPluginObject(mContext,
                IIpMessagePlugin.class.getName(), PLUGIN_VERSION, PLUGIN_METANAME);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test1GetAllManagers() throws Exception {
        ActivitiesManager mActivitiesManager = mIpMessagePlugin.getActivitiesManager(mContext);
        assertNotNull(mActivitiesManager);
        ChatManager mChatManager = mIpMessagePlugin.getChatManager(mContext);
        assertNotNull(mChatManager);
        ContactManager mContactManager = mIpMessagePlugin.getContactManager(mContext);
        assertNotNull(mContactManager);
        GroupManager mGroupManager = mIpMessagePlugin.getGroupManager(mContext);
        assertNotNull(mGroupManager);
        MessageManager mMessageManager = mIpMessagePlugin.getMessageManager(mContext);
        assertNotNull(mMessageManager);
        NotificationsManager mNotificationsManager = mIpMessagePlugin.getNotificationsManager(mContext);
        assertNotNull(mNotificationsManager);
        ServiceManager mServiceManager = mIpMessagePlugin.getServiceManager(mContext);
        assertNotNull(mServiceManager);
        SettingsManager mSettingsManager = mIpMessagePlugin.getSettingsManager(mContext);
        assertNotNull(mSettingsManager);
        ResourceManager mResourceManager = mIpMessagePlugin.getResourceManager(mContext);
        assertNotNull(mResourceManager);
    }
}
