package com.hissage.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.test.InstrumentationTestCase;

import com.mediatek.imsp.MessageManagerExt;
import com.mediatek.mms.ipmessage.IIpMessagePlugin;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageMediaTypeFlag;
import com.mediatek.mms.ipmessage.IpMessageConsts.ResourceId;
import com.mediatek.mms.ipmessage.message.IpMessage;
import com.mediatek.mms.ipmessage.message.IpTextMessage;
import com.mediatek.mms.ipmessage.message.IpVCalendarMessage;
import com.mediatek.mms.ipmessage.message.IpVCardMessage;
import com.mediatek.mms.ipmessage.message.IpVideoMessage;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpImageMessage;
import com.mediatek.mms.ipmessage.message.IpVoiceMessage;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageSendMode;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageType;
import com.mediatek.pluginmanager.PluginManager;

import java.util.List;

public class MessageManagerExtTest extends InstrumentationTestCase {
    protected Instrumentation mInstrumentation;
    protected Context mContext;
    private MessageManagerExt mMessageManagerExt;
    private static final String ATTACHMENT_PATH = "/data/data/com.hissage.tests/";
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        mMessageManagerExt = new MessageManagerExt(mContext);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01getIpMsgInfo() throws Exception {
        mMessageManagerExt.getIpMsgInfo(1);
        mMessageManagerExt.getIpMsgInfo(3);
        mMessageManagerExt.getIpMsgInfo(5);
        mMessageManagerExt.getIpMsgInfo(7);
        mMessageManagerExt.getIpMsgInfo(8);
        mMessageManagerExt.getIpMsgInfo(9);
        mMessageManagerExt.getIpMsgInfo(13);
        mMessageManagerExt.getIpMsgInfo(100);
    }

//    public void test02getIpMessageFromIntent() throws Exception {
//        mMessageManagerExt.getIpMessageFromIntent(null);
//    }

    public void test03getType() throws Exception {
        mMessageManagerExt.getType(1);
        mMessageManagerExt.getType(3);
        mMessageManagerExt.getType(5);
        mMessageManagerExt.getType(7);
        mMessageManagerExt.getType(8);
        mMessageManagerExt.getType(9);
        mMessageManagerExt.getType(13);
        mMessageManagerExt.getType(100);
    }

    public void test04getIpDatabaseId() throws Exception {
        mMessageManagerExt.getIpDatabaseId(1);
        mMessageManagerExt.getIpDatabaseId(100);
    }

    public void test05getStatus() throws Exception {
        mMessageManagerExt.getStatus(1);
        mMessageManagerExt.getStatus(100);
    }

    public void test06getTime() throws Exception {
        mMessageManagerExt.getTime(1);
        mMessageManagerExt.getTime(100);
    }

    public void test07getSimId() throws Exception {
        mMessageManagerExt.getSimId(1);
        mMessageManagerExt.getSimId(100);
    }

    public void test08isReaded() throws Exception {
        mMessageManagerExt.isReaded(1);
        mMessageManagerExt.isReaded(100);
    }

    public void test09getTo() throws Exception {
        mMessageManagerExt.getTo(1);
        mMessageManagerExt.getTo(100);
    }

    public void test10setIpMsgAsViewed() throws Exception {
        mMessageManagerExt.setIpMsgAsViewed(1);
    }

    public void test11setThreadAsViewed() throws Exception {
        mMessageManagerExt.setThreadAsViewed(9);
    }

    public void test12downloadAttach() throws Exception {
        mMessageManagerExt.downloadAttach(3);
    }

    public void test13cancelDownloading() throws Exception {
        mMessageManagerExt.cancelDownloading(3);;
    }

    public void test14isDownloading() throws Exception {
        mMessageManagerExt.isDownloading(3);
    }

    public void test15getDownloadProcess() throws Exception {
        mMessageManagerExt.getDownloadProcess(3);
    }

    public void test16addMessageToImportantList() throws Exception {
        mMessageManagerExt.addMessageToImportantList(new long[]{1,3});
    }

    public void test17resendMessage() throws Exception {
        mMessageManagerExt.resendMessage(1);
    }

    public void test18resendMessage() throws Exception {
        mMessageManagerExt.resendMessage(1, 1);
    }

    public void test19getFrom() throws Exception {
        mMessageManagerExt.getFrom(1);
        mMessageManagerExt.getFrom(100);
    }

    public void test20saveIpMsg() throws Exception {
        IpTextMessage it = new IpTextMessage();
        it.setBody("test");
        mMessageManagerExt.saveIpMsg(it, IpMessageSendMode.AUTO);
        IpImageMessage imImage = new IpImageMessage();
        imImage.setType(IpMessageType.PICTURE);
        imImage.setPath(ATTACHMENT_PATH + "test_image.png");
        mMessageManagerExt.saveIpMsg(imImage, IpMessageSendMode.AUTO);
        IpVoiceMessage imAudio = new IpVoiceMessage();
        imAudio.setType(IpMessageType.VOICE);
        imAudio.setPath(ATTACHMENT_PATH + "test_audio.amr");
        mMessageManagerExt.saveIpMsg(imAudio, IpMessageSendMode.AUTO);
        IpVideoMessage imVideo = new IpVideoMessage();
        imVideo.setType(IpMessageType.VIDEO);
        imVideo.setPath(ATTACHMENT_PATH + "test_video.3gp");
        mMessageManagerExt.saveIpMsg(imVideo, IpMessageSendMode.AUTO);
    }

    public void test21deleteIpMsg() throws Exception {
        mMessageManagerExt.deleteIpMsg(new long[]{1},false,false);
    }

    public void test22deleteMessageFromImportantList() throws Exception {
        mMessageManagerExt.deleteMessageFromImportantList(new long[]{1,3});
    }
}
