package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.ResourceManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class ResourceManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private ResourceManagerExt mResourceManagerExt;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mResourceManagerExt = new ResourceManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01getStringById() throws Exception {
        mResourceManagerExt.getSingleString(ResourceId.STR_IPMESSAGE_SETTINGS);
        mResourceManagerExt.getSingleString(0);
    }
}
