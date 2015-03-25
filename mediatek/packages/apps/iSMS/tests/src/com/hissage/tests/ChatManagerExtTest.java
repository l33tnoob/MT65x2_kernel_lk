package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.ChatManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ContactStatus;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class ChatManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;

    protected Context mContext;

    private ChatManagerExt mChatManagerExt;

    private static final String NUMBER = "18811059206";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mChatManagerExt = new ChatManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01needShowInviteDlg() throws Exception {
        mChatManagerExt.needShowInviteDlg(1);
    }

    public void test02handleInviteDlgLater() throws Exception {
        mChatManagerExt.handleInviteDlgLater(1);
    }

    public void test03handleInviteDlg() throws Exception {
        mChatManagerExt.handleInviteDlg(1);
    }

    public void test04needShowReminderDlg() throws Exception {
        mChatManagerExt.needShowReminderDlg(1);
    }

    public void test05enterChatMode() throws Exception {
        mChatManagerExt.enterChatMode(NUMBER);
    }

    public void test06sendChatMode() throws Exception {
        mChatManagerExt.sendChatMode(NUMBER, ContactStatus.TYPING);
    }

    public void test07exitFromChatMode() throws Exception {
        mChatManagerExt.exitFromChatMode(NUMBER);
    }

    public void test08saveChatHistory() throws Exception {
        mChatManagerExt.saveChatHistory(new long[] {1});
    }

    public void test09getIpMessageCountOfTypeInThread() throws Exception {
        mChatManagerExt.getIpMessageCountOfTypeInThread(1, IpMessageMediaTypeFlag.PICTURE);
    }

    public void test10deleteDraftMessageInThread() throws Exception {
        mChatManagerExt.deleteDraftMessageInThread(1);
    }
}
