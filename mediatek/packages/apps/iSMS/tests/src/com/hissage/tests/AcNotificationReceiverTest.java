package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.test.InstrumentationTestCase;

import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.imsp.ServiceManagerExt;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateGroupAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsSaveHistory;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.service.NmsService;
import com.hissage.contact.NmsContact;
import com.hissage.util.data.NmsConsts;

import java.util.List;

public class AcNotificationReceiverTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private ServiceManagerExt mServiceManagerExt;
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mServiceManagerExt = new ServiceManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test00EnableReceiver() throws Exception {
        mServiceManagerExt.startIpService();
    }

    public void test01RestartAction() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsConsts.NmsIntentStrId.NMS_MMS_RESTART_ACTION);
        mContext.sendBroadcast(intent);
    }

    public void test02ServiceReady() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NMS_INTENT_SERVICE_READY);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test03IMStatusAction() throws Exception {
        Intent intent = new Intent();
        NmsContact nc = new NmsContact((short)1, 1, "test", "12345", "case", 1, 0, false);
        intent.putExtra(NmsIpMessageConsts.NmsImStatus.NMS_CONTACT_CURRENT_STATUS, nc);
        intent.setAction(NmsIpMessageConsts.NmsImStatus.NMS_IM_STATUS_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test04StatusAction() throws Exception {
        Intent intent = new Intent();
        intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_RECD_ID, 1);
        intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_SYS_ID, 1);
        intent.setAction(NmsIpMessageConsts.NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test05DownloadStatusAction() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test06DownloadHistory() throws Exception {
        Intent intent = new Intent();
        intent.putExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE, -1);
        intent.setAction(NmsIpMessageConsts.NmsSaveHistory.NMS_ACTION_DOWNLOAD_HISTORY);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test07UpdateGroup() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NmsUpdateGroupAction.NMS_UPDATE_GROUP);
        intent.putExtra(NmsUpdateGroupAction.NMS_UPDATE_GROUP, 51);
        intent.putExtra(NmsUpdateGroupAction.NMS_GROUP_ID, 2);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test08UpdateContact() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public void test09NewMessageAction() throws Exception {
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
        NmsService.getInstance().sendBroadcast(intent);
    }
}
