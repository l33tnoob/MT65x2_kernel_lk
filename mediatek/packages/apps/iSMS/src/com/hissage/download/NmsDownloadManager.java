package com.hissage.download;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;

import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDownloadAttachStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.service.NmsService;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsDownloadManager extends Handler {

    private static ArrayList<Long> mDownloadMsgList = null;
    private static ArrayList<Integer> mDownloadProgressList = null;
    private static ArrayList<HttpURLConnection> mDownloadConnList = null;

    private static int ADD_NEW_DOWNALOD_MSG = 0;
    private static int DEL_DOWNALOD_MSG = 1;
    private static int UPDATE_DOWNALOD_MSG = 2;
    private static int CANCEL_DOWNLOAD = 3;

    private static final long AVALIABLE_SPACE = 5 * 1024 * 1024;

    public static class SNmsDownloadMsgInfo {
        long msgId = 0;
        HttpURLConnection conn = null;
        int progress = 0;
        Intent intent = null;
    }

    private static NmsDownloadManager mInstance = null;
    

    private NmsDownloadManager() {
    }
    
    public void init(){
        
    }

    public void handleMessage(Message message) {

        SNmsDownloadMsgInfo info = (SNmsDownloadMsgInfo) message.obj;
        NmsLog.trace(HissageTag.api, "downloadmanager recv msg :" + message);
        if (null == info) {
            return;
        }

        if (ADD_NEW_DOWNALOD_MSG == message.what) {
            mDownloadConnList.add(info.conn);
            mDownloadMsgList.add(info.msgId);
            mDownloadProgressList.add(info.progress);
        } else if (DEL_DOWNALOD_MSG == message.what) {
            int index = mDownloadMsgList.indexOf(info.msgId);

            if (index >= 0 && index < mDownloadMsgList.size()) {
                mDownloadMsgList.remove(index);
            }

            if (index >= 0 && index < mDownloadConnList.size()) {
                mDownloadConnList.remove(index);
            }

            if (index >= 0 && index < mDownloadProgressList.size()) {
                mDownloadProgressList.remove(index);
            }

        } else if (UPDATE_DOWNALOD_MSG == message.what) {
            int index = mDownloadMsgList.indexOf(info.msgId);

            if (index >= 0 && index < mDownloadProgressList.size()) {
                mDownloadProgressList.set(index, info.progress);
            }
        } else if (CANCEL_DOWNLOAD == message.what) {
            int index = mDownloadMsgList.indexOf(info.msgId);

            if (index >= 0 && index < mDownloadConnList.size()) {
                mDownloadConnList.get(index).disconnect();
                if (info.intent != null) {
                    NmsLog.error(HissageTag.api, "post download status :" + info.intent.getAction());
                    NmsService.getInstance().sendBroadcast(info.intent);
                }
            }
            return;
        }
        if (info.intent != null) {
            NmsLog.error(HissageTag.api, "post download status :" + info.intent.getAction());
            NmsService.getInstance().sendBroadcast(info.intent);
        }
    }

    public static NmsDownloadManager getInstance() {
        if (null == mInstance) {
            mInstance = new NmsDownloadManager();
            mDownloadMsgList = new ArrayList<Long>();
            mDownloadConnList = new ArrayList<HttpURLConnection>();
            mDownloadProgressList = new ArrayList<Integer>();
        }
        return mInstance;
    }

    private static void nmsNotifyDownloadAttachStatus(String action, String extraName,
            int extraValue, long msgId) {
        Intent intent = new Intent();
        intent.putExtra(extraName, extraValue);
        intent.putExtra(NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID, msgId);
        intent.setAction(action);
        NmsService.getInstance().sendBroadcast(intent);
    }

    private static Intent nmsGetIntent(String action, String extraName, int extraValue, long msgId) {
        Intent intent = new Intent();
        intent.putExtra(extraName, extraValue);
        intent.putExtra(NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID, msgId);
        intent.setAction(action);
        return intent;
    }

    private static void nmsAddMsg(long msgId, HttpURLConnection conn) {
        SNmsDownloadMsgInfo info = new SNmsDownloadMsgInfo();
        info.conn = conn;
        info.msgId = msgId;
        Message msg = Message.obtain(mInstance, ADD_NEW_DOWNALOD_MSG, info);
        mInstance.sendMessage(msg);
    }

    private static void nmsDelMsg(long msgId, Intent intent) {
        SNmsDownloadMsgInfo info = new SNmsDownloadMsgInfo();
        info.msgId = msgId;
        info.intent = intent;
        Message msg = Message.obtain(mInstance, DEL_DOWNALOD_MSG, info);
        mInstance.sendMessage(msg);
    }

    private static void nmsUpdateMsg(long msgId, int progress) {
        SNmsDownloadMsgInfo info = new SNmsDownloadMsgInfo();
        info.msgId = msgId;
        info.progress = progress;
        Message msg = Message.obtain(mInstance, UPDATE_DOWNALOD_MSG, info);
        mInstance.sendMessage(msg);
    }

    public void nmsDownload(final long msgId, final long smsId) {
        if (null != mDownloadMsgList && mDownloadMsgList.contains(msgId)) {
            NmsLog.error(HissageTag.api, "this msg is downloading, so ignore this req, msgId: "
                    + msgId+", smsId: "+smsId);
            return;
        }
        if (msgId <= 0) {
            NmsLog.error(HissageTag.api, "nmsDownloadAttach param error: " + msgId);
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
            return;
        }


        NmsIpMessage msg = engineadapter.get().nmsUIGetMsgKey((short) msgId);
        if (null == msg) {
            NmsLog.error(HissageTag.api, "nmsDownloadAttach can't get nms recordId.");
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
            return;
        }
        
        String downloadPathStore = "";
        if (msg instanceof NmsIpAttachMessage) {
            int fileSize = ((NmsIpAttachMessage) msg).size;
            long availableSpace = 0;
            if (NmsCommonUtils.getSDCardStatus()) {
                availableSpace = NmsCommonUtils.getSDcardAvailableSpace();
                if (availableSpace > AVALIABLE_SPACE) {
                    downloadPathStore = NmsCommonUtils.getSDCardPath(NmsService.getInstance());
                } else {
                    availableSpace = NmsCommonUtils.getDataStorageAvailableSpace();
                    downloadPathStore = NmsCommonUtils.getMemPath(NmsService.getInstance());
                }
            } else {
                availableSpace = NmsCommonUtils.getDataStorageAvailableSpace();
                downloadPathStore = NmsCommonUtils.getMemPath(NmsService.getInstance());
            }

            if (fileSize >= availableSpace) {
                NmsLog.error(HissageTag.api, "SD card don't have enough available space.");
                nmsNotifyDownloadAttachStatus(
                        NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                        NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                        NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
                return;
            }
        }

        long simId1 = NmsPlatformAdapter.getInstance(NmsService.getInstance()).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_1);
        long simId2 = NmsPlatformAdapter.getInstance(NmsService.getInstance()).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_2);
        if (msg.simId != simId1 && msg.simId != simId2) {
            NmsLog.error(HissageTag.api, "nmsDownloadAttach's simId not current sim id: "
                    + msg.simId + ", current simId: " + simId1 + ", " + simId2);
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
            return;
        }

        String strUrl = null;
        String fileName = null;
        if (msg.type == NmsIpMessageType.PICTURE && (msg instanceof NmsIpImageMessage)) {
            strUrl = ((NmsIpImageMessage) msg).url;
        } else if (msg.type == NmsIpMessageType.SKETCH && (msg instanceof NmsIpImageMessage)) {
            strUrl = ((NmsIpImageMessage) msg).url;
        } else if (msg.type == NmsIpMessageType.VIDEO && (msg instanceof NmsIpVideoMessage)) {
            strUrl = ((NmsIpVideoMessage) msg).url;
        } else if (msg.type == NmsIpMessageType.VOICE && (msg instanceof NmsIpVoiceMessage)) {
            strUrl = ((NmsIpVoiceMessage) msg).url;
        } else if (msg.type == NmsIpMessageType.LOCATION && (msg instanceof NmsIpLocationMessage)) {
            strUrl = ((NmsIpLocationMessage) msg).url;
        }

        fileName = engineadapter.get().nmsUIGetMsgDownloadAttachName(msg.ipDbId);
        if (TextUtils.isEmpty(fileName)) {
            fileName = "/unknow.nms";
        }

        final String finalUrl = strUrl;
        // final String finalUrl =
        // "http://auto.sohu.com/piclib/gm/vauxhall/vxlightning/big/Vauxhall_VX_Lightning001.jpg";
        if (TextUtils.isEmpty(strUrl)) {
            NmsLog.error(HissageTag.api, "nmsDownloadAttach can't get url:" + strUrl);
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
            return;
        }
        String downloadPath = downloadPathStore + "/" + NmsCustomUIConfig.ROOTDIRECTORY
                + "/Download/" + msgId + "/";

        File f = new File(downloadPath);
        if (!f.exists()) {
            f.mkdirs();
        }


        final String filePath = downloadPath + fileName;
        final File file = new File(filePath);
        if (file.exists()) {
            file.delete();
        }

        try {
            file.createNewFile();
        } catch (IOException e1) {
            NmsLog.nmsPrintStackTrace(e1);
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
        }

        nmsNotifyDownloadAttachStatus(NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                NmsDownloadAttachStatus.STARTING, smsId > 0 ? smsId : msgId);

        final int nmsMessageId = msg.ipDbId;

        new Thread() {
            @Override
            public void run() {
                HttpURLConnection conn = null;
                try {

                    URL url = new URL(finalUrl);
                    conn = (HttpURLConnection) url.openConnection();

                    conn.setReadTimeout(5000);
                    conn.setConnectTimeout(5000);

                    nmsAddMsg(msgId, conn);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTrace(e);
                    Intent intent = nmsGetIntent(
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
                    nmsDelMsg(msgId, intent);
                    return;
                }
                try {
                    int max = conn.getContentLength();
                    InputStream is = conn.getInputStream();
                    FileOutputStream fos = new FileOutputStream(file);
                    BufferedInputStream bis = new BufferedInputStream(is);
                    byte[] buffer = new byte[1024 * 10];
                    int len;
                    int total = 0;
                    nmsNotifyDownloadAttachStatus(
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.DOWNLOADING, smsId > 0 ? smsId : msgId);
                    while ((len = bis.read(buffer)) != -1) {
                        fos.write(buffer, 0, len);
                        total += len;
                        nmsUpdateMsg(msgId, total * 100 / max);
                        nmsNotifyDownloadAttachStatus(
                                NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                                NmsDownloadAttachStatus.NMS_DOWNLOAD_PERCENTAGE, total * 100 / max,
                                smsId > 0 ? smsId : msgId);
                    }
                    fos.close();
                    bis.close();
                    is.close();
                    conn.disconnect();
                    NmsLog.trace(HissageTag.api, "msg download success, msgId: " + nmsMessageId
                            + ", filePath: " + filePath);
                    engineadapter.get().nmsUISetMsgDownloadedAttachPath(nmsMessageId, filePath);
                    Intent intent = nmsGetIntent(
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.DONE, smsId > 0 ? smsId : msgId);
                    nmsDelMsg(msgId, intent);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTrace(e);
                    if(file.exists()){
                        file.delete();
                    }
                    Intent intent = nmsGetIntent(
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                            NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
                    nmsDelMsg(msgId, intent);
                }
            }
        }.start();
    }

    public void nmsCancelDownload(long msgId, long smsId) {
        if (msgId <= 0 || null == mDownloadMsgList || null == mDownloadConnList) {
            NmsLog.error(HissageTag.api, "cancel download error, msgId is error: " + msgId);
            return;
        }

        Intent intent = nmsGetIntent(NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);

        SNmsDownloadMsgInfo info = new SNmsDownloadMsgInfo();
        info.msgId = msgId;
        info.intent = intent;

        Message msg = Message.obtain(mInstance, CANCEL_DOWNLOAD, info);
        mInstance.sendMessage(msg);

    }

    public boolean nmsIsDownloading(long msgId, long smsId) {
        if (msgId <= 0) {
            NmsLog.error(HissageTag.api, "nmsIsDownloading param error: " + msgId);
            nmsNotifyDownloadAttachStatus(
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    NmsDownloadAttachStatus.FAILED, smsId > 0 ? smsId : msgId);
            return false;
        }

        if (null != mDownloadMsgList && mDownloadMsgList.contains(msgId)) {
            return true;
        } else {
            return false;
        }
    }

    public int nmsGetProgress(long msgId, long smsId) {
        if (msgId <= 0 || null == mDownloadMsgList || null == mDownloadProgressList) {
            NmsLog.error(HissageTag.api, "get download progress error, msgId is error: " + msgId);
            return 0;
        }

        int index = mDownloadMsgList.indexOf(msgId);

        if (index >= 0 && index < mDownloadProgressList.size()) {
            return mDownloadProgressList.get(index);
        }

        return 0;
    }

}
