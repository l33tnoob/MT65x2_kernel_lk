package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.GroupManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class GroupManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;

    protected Context mContext;

    private GroupManagerExt mGroupManagerExt;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mGroupManagerExt = new GroupManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01getGroupIdList() throws Exception {
        mGroupManagerExt.getGroupIdList();
    }

    public void test02getGroupIpMessageCount() throws Exception {
        mGroupManagerExt.getGroupIpMessageCount(16);
    }
}
