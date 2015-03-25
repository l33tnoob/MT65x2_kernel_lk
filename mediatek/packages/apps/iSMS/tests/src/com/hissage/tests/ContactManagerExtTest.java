package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.ContactManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class ContactManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;

    protected Context mContext;

    private ContactManagerExt mContactManagerExt;

    private static final String NUMBER = "18811059206";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        mContactManagerExt = new ContactManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01getNumberByEngineId() throws Exception {
        mContactManagerExt.getNumberByEngineId((short) 2);
    }

    public void test02getNumberByMessageId() throws Exception {
        mContactManagerExt.getNumberByMessageId(3);
    }

    public void test03getNumberByThreadId() throws Exception {
        mContactManagerExt.getNumberByThreadId(1);
    }

    public void test04getContactIdByNumber() throws Exception {
        mContactManagerExt.getContactIdByNumber(NUMBER);
    }

    public void test05getTypeByNumber() throws Exception {
        mContactManagerExt.getTypeByNumber(NUMBER);
    }

    public void test06getStatusByNumber() throws Exception {
        mContactManagerExt.getStatusByNumber(NUMBER);
    }

    public void test07getOnlineTimeByNumber() throws Exception {
        mContactManagerExt.getOnlineTimeByNumber(NUMBER);
    }

    public void test08getNameByNumber() throws Exception {
        mContactManagerExt.getNameByNumber(NUMBER);
    }

    public void test09getNameByThreadId() throws Exception {
        mContactManagerExt.getNameByThreadId(1);
    }

    public void test10getSignatureByNumber() throws Exception {
        mContactManagerExt.getSignatureByNumber(NUMBER);
    }

    public void test11getAvatarByNumber() throws Exception {
        mContactManagerExt.getAvatarByNumber(NUMBER);
    }

    public void test12getAvatarByThreadId() throws Exception {
        mContactManagerExt.getAvatarByThreadId(1);
    }

    public void test13isIpMessageNumber() throws Exception {
        mContactManagerExt.isIpMessageNumber(NUMBER);
    }

    public void test14addContactToSpamList() throws Exception {
        mContactManagerExt.addContactToSpamList(new int[] {1});
    }

    public void test15deleteContactFromSpamList() throws Exception {
        mContactManagerExt.deleteContactFromSpamList(new int[] {1});
    }
}
