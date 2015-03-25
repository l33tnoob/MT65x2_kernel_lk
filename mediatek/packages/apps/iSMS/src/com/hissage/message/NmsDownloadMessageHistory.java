package com.hissage.message;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import android.content.Intent;
import android.text.TextUtils;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.contact.NmsBroadCastContact;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsHesineApiConsts;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpCalendarMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsSaveHistory;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

/* message dumper */
public class NmsDownloadMessageHistory {

    protected final static String logTag = "DownloadHistory";
    
    protected final static String zipFolder = "Group";

    protected NmsIpMessage dumpMsg;
    protected ZipOutputStream zipStream;
    protected String zipEntryFolder;
    protected BufferedOutputStream dumpStreamFile;
    protected int dumpIndex;

    protected NmsDownloadMessageHistory(NmsIpMessage msg, ZipOutputStream zip, BufferedOutputStream file,
            String entryFolder, int index) {
        dumpMsg = msg;
        zipStream = zip;
        zipEntryFolder = entryFolder;
        dumpStreamFile = file;
        dumpIndex = index;
    }

    private static NmsDownloadMessageHistory createMsgDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {

        switch (msg.type) {

        case NmsIpMessageConsts.NmsIpMessageType.TEXT:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG:
        case NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG:
            return new NmsTextMessageDumper(msg, zip, file, entryFolder, index);

        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
        case NmsIpMessageConsts.NmsIpMessageType.SKETCH:
            return new NmsImageMessageDumper(msg, zip, file, entryFolder, index);

        case NmsIpMessageConsts.NmsIpMessageType.LOCATION:
            return new NmsLocationMessageDumper(msg, zip, file, entryFolder, index);

        case NmsIpMessageConsts.NmsIpMessageType.VCARD:
            return new NmsVcardMessageDumper(msg, zip, file, entryFolder, index);

        case NmsIpMessageConsts.NmsIpMessageType.VOICE:
            return new NmsVoiceMessageDumper(msg, zip, file, entryFolder, index);

        case NmsIpMessageConsts.NmsIpMessageType.VIDEO:
            return new NmsVideoMessageDumper(msg, zip, file, entryFolder, index);
        
        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR:
            return new NmsCalendarMessageDumper(msg, zip, file, entryFolder, index) ;
            
        default:
            NmsLog.error(NmsDownloadMessageHistory.logTag, "error type for createMsgDumper" + msg.type);
            return new NmsDownloadMessageHistory(msg, zip, file, entryFolder, index);
        }
    }

    protected static String getString(int resId) {
        return NmsService.getInstance().getString(resId);
    }

    protected static String getTimeString(int time) {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US);
        return sdf.format((long) time * 1000);
    }

    protected static String getDurationString(int time) {
        int hour = 0;
        int min = 0;
        int sec = 0;

        hour = time / 3600;

        if (hour > 0)
            time = time - 3600 * hour;

        min = time / 60;

        if (min > 0)
            time = time - 60 * min;

        sec = time;

        String ret = "";

        if (hour > 0)
            ret += hour + ":";

        if (min > 0) {
            if (min < 10)
                ret += "0" + min + ":";
            else
                ret += min + ":";
        } else if (ret.length() > 0) {
            ret += "00:";
        }

        if (sec == 0) {
            if (ret.length() > 0)
                ret += "00";
            else
                ret = "0";
        } else {
            if (sec < 10 && ret.length() > 0)
                ret += "0" + sec;
            else
                ret += sec;
        }

        return ret;
    }

    protected static void zipFile(ZipOutputStream zipStream, String inFile, String inFileEntry)
            throws IOException {

        int readLen = 1024 * 10;
        byte[] bytes = new byte[readLen];

        FileInputStream inputFile = new FileInputStream(inFile);
        ZipEntry entry = new ZipEntry(inFileEntry);

        zipStream.putNextEntry(entry);

        int len = inputFile.read(bytes, 0, readLen);
        while (len != -1) {
            zipStream.write(bytes, 0, len);
            len = inputFile.read(bytes, 0, readLen);
        }

        // zos.closeEntry();
        inputFile.close();
    }

    protected void dump() throws IOException {

        String protocolStr = "";
        String sendReceStr = "";
        if (dumpMsg.protocol == NmsIpMessageConsts.NmsMessageProtocol.SMS)
            protocolStr = getString(R.string.STR_NMS_MSG_DUMPER_MSG_SMS);
        else if (dumpMsg.protocol == NmsIpMessageConsts.NmsMessageProtocol.MMS)
            protocolStr = getString(R.string.STR_NMS_MSG_DUMPER_MSG_MMS);
        else
            protocolStr = getString(R.string.STR_NMS_MSG_DUMPER_MSG_IP);

        if (dumpMsg.status == NmsIpMessageConsts.NmsIpMessageStatus.INBOX)
            sendReceStr = getString(R.string.STR_NMS_MSG_DUMPER_RECEIVE_MSG);
        else
            sendReceStr = getString(R.string.STR_NMS_MSG_DUMPER_SEND_MSG);

        dumpStreamFile.write(String.format(sendReceStr, protocolStr, dumpIndex).getBytes());
        dumpStreamFile.write("\r\n".getBytes());

        dumpStreamFile
                .write((getString(R.string.STR_NMS_MSG_DUMPER_MSG_TIME) + " " + getTimeString(dumpMsg.time))
                        .getBytes());
        dumpStreamFile.write("\r\n".getBytes());

        /* TODO: dump group-chat sender */
    }

    private static String getNormalContactString(short contactId) {
        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);

        if (contact == null)
            return "";

        if (contact.getName() == contact.getNumber())
            return contact.getNumber();

        return String.format("%s<%s>", contact.getName(), contact.getNumber());
    }

    private static String getMembersContactString(short members[]) {

        String ret = "";
        
        if (members == null)
            return ret ;

        for (int i = 0; i < members.length; i++) {
            if (ret.length() > 0)
                ret += ",";
            ret += getNormalContactString(members[i]);
        }

        return ret;
    }

    private static void dumpContactInfo(NmsContact contact, BufferedOutputStream stream)
            throws IOException {

        if (contact.getType() == NmsContactType.HISSAGE_BROADCAST) {
            NmsBroadCastContact msContact = (NmsBroadCastContact) contact;
            stream.write(getString(R.string.STR_NMS_MSG_DUMPER_MULTI_SEND_DETAIL).getBytes());
            stream.write("\r\n".getBytes());
            stream.write((getString(R.string.STR_NMS_MSG_DUMPER_MEMBERS) + " " + getMembersContactString(msContact
                    .getMemberIds())).getBytes());
            stream.write("\r\n\r\n".getBytes());
            return;
        }

        if (contact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsGroupChatContact gcContact = (NmsGroupChatContact) contact;
            stream.write(getString(R.string.STR_NMS_MSG_DUMPER_GROUP_CHAT_DETAIL).getBytes());
            stream.write("\r\n".getBytes());
            stream.write((getString(R.string.STR_NMS_MSG_DUMPER_NAME) + " " + gcContact.getName())
                    .getBytes());
            stream.write("\r\n".getBytes());
            stream.write((getString(R.string.STR_NMS_MSG_DUMPER_GROUP_CHAT_CREATER) + " " +getNormalContactString(gcContact
                    .getCreaterId())).getBytes());
            stream.write("\r\n".getBytes());
            stream.write((getString(R.string.STR_NMS_MSG_DUMPER_MEMBERS) + " " + getMembersContactString(gcContact
                    .getMemberIds())).getBytes());
            stream.write("\r\n\r\n".getBytes());
            return;
        }

        /* normal */
        stream.write(getString(R.string.STR_NMS_MSG_DUMPER_NORMAL_DETAIL).getBytes());
        stream.write("\r\n".getBytes());
        stream.write((getString(R.string.STR_NMS_MSG_DUMPER_NAME) + " " + contact.getName()).getBytes());
        stream.write("\r\n".getBytes());
        stream.write((getString(R.string.STR_NMS_MSG_DUMPER_NUMBER) + " " + contact.getNumber())
                .getBytes());
        stream.write("\r\n\r\n".getBytes());
    }

    private static void dumpSingleContactMessages(short contactId, ZipOutputStream zipStream,
            String parentFolder) throws IOException {

        SNmsImMsgCountInfo info = engineadapter.get().nmsUISetImMode(contactId, -1,
                NmsHesineApiConsts.NmsImFlag.NMS_IM_FLAG_ALL,
                NmsHesineApiConsts.NmsImReadMode.NMS_IM_READ_MODE_ALL);

        if (info.allMsgCount < 0) {
            NmsLog.warn(NmsDownloadMessageHistory.logTag, "warnning that set im mode error: " + contactId);
            // return;
        }

        if (info.allMsgCount == 0) {
            NmsLog.warn(NmsDownloadMessageHistory.logTag, "warnning that message count is 0 in id: "
                    + contactId);
            // return;
        }

        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);

        if (contact == null) {
            NmsLog.error(NmsDownloadMessageHistory.logTag,
                    "error that can not get contact info in db, id: " + contactId);
            return;
        }

        String zipEntryFolder = zipFolder + "_" + contactId + "/";

        File msgFile = null ;
        BufferedOutputStream msgFileStream = null ;
        
        try {
            try {
                msgFile = new File(parentFolder + contact.getId() + ".txt");
                msgFileStream = new BufferedOutputStream(new FileOutputStream(msgFile)) ;
            } catch (Exception e) {
                NmsLog.trace(logTag, "can not create the dumper txt file: " + msgFile.getPath() + ", try simple name: " + parentFolder + contact.getId() + ".txt") ;
                msgFile = new File(parentFolder + contact.getId() + ".txt");
                msgFileStream = new BufferedOutputStream(new FileOutputStream(msgFile)) ;
            }
            
            dumpContactInfo(contact, msgFileStream);

            for (int i = 0; i < info.allMsgCount; i++) {
                createMsgDumper(engineadapter.get().nmsUIGetMsgSummary(i).ipMsg, zipStream,
                        msgFileStream, zipEntryFolder, i + 1).dump();
            }
            
            msgFileStream.close();
            msgFileStream = null ;

            zipFile(zipStream, msgFile.getPath(), zipEntryFolder + msgFile.getName());
            msgFile.delete();
            
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
            if (msgFileStream != null)
                msgFileStream.close();
            
            if (msgFile != null && msgFile.exists())
                msgFile.delete();
        }
    }

    private static void dumpContactMessageInThread(short contactIds[], String destFile)
            throws IOException {

        NmsLog.trace(logTag, "zip contacts messages start " + destFile);

        File zipFile = new File(destFile);

        ZipOutputStream zipStream = null ;
        
        try {
            zipStream = new ZipOutputStream(new BufferedOutputStream(
                    new FileOutputStream(zipFile)));
            String parentFolder = zipFile.getParent() + "/";

            for (int i = 0; i < contactIds.length; i++) {
                dumpSingleContactMessages(contactIds[i], zipStream, parentFolder);
                //sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_PROGRESS, i * 100 / contactIds.length,
                //        null);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
            zipStream.close();
            return ;
        }

        zipStream.close();

        NmsLog.trace(logTag, "zip end: file length: " + zipFile.length());
    }

    public static void sendStatus(String key, int parm, String file) {
        Intent intent = new Intent();
        intent.putExtra(key, parm);
        if (!TextUtils.isEmpty(file)) {
            intent.putExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_FILE, file);
        }
        intent.setAction(NmsSaveHistory.NMS_ACTION_DOWNLOAD_HISTORY);
        NmsService.getInstance().sendBroadcast(intent);
    }

    public static void dumpContactMessages(short contactIds[], final String destFile) {

        final short fContactIds[] = contactIds;
        final String fDestFile = destFile;

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    File folder = new File(fDestFile).getParentFile();
                    if (!folder.exists() && !folder.mkdirs()) {
                        NmsLog.error(NmsDownloadMessageHistory.logTag,
                                "error to create folder: " + folder.getPath());
                        sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE,
                                NmsSaveHistory.NMS_ERROR, destFile);
                    }
                    dumpContactMessageInThread(fContactIds, fDestFile);
                    sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE,
                            new File(fDestFile).exists() ? NmsSaveHistory.NMS_OK
                                    : NmsSaveHistory.NMS_EMPTY, fDestFile);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTrace(e);
                    sendStatus(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE, NmsSaveHistory.NMS_ERROR,
                            destFile);
                }
            }
        }).start();
    }
}

class NmsTextMessageDumper extends NmsDownloadMessageHistory {

    protected NmsTextMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();

        NmsIpTextMessage textMsg = (NmsIpTextMessage) dumpMsg;
        dumpStreamFile
                .write((getString(R.string.STR_NMS_MSG_DUMPER_MSG_TEXT) + ": " + textMsg.body)
                        .getBytes());
        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsAttachMessageDumper extends NmsDownloadMessageHistory {

    protected NmsAttachMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();

        NmsIpAttachMessage attachMsg = (NmsIpAttachMessage) dumpMsg;
        int typeResId = 0;
        String searchStr = "";

        switch (attachMsg.type) {

        case NmsIpMessageConsts.NmsIpMessageType.PICTURE:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_PIC;
            searchStr = ".";
            break;

        case NmsIpMessageConsts.NmsIpMessageType.SKETCH:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_SKETCH;
            searchStr = ".ske";
            break;

        case NmsIpMessageConsts.NmsIpMessageType.LOCATION:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_LOCATION_IMG;
            searchStr = ".map";
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VCARD:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_VCARD;
            searchStr = ".vcf";
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VOICE:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_VOICE;
            searchStr = ".";
            break;

        case NmsIpMessageConsts.NmsIpMessageType.VIDEO:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_VIDEO;
            searchStr = ".";
            break;
            
        case NmsIpMessageConsts.NmsIpMessageType.CALENDAR:
            typeResId = R.string.STR_NMS_MSG_DUMPER_MSG_CALENDAR;
            searchStr = ".";
            break;

        default:
            NmsLog.error(NmsDownloadMessageHistory.logTag, "error type for attach message, type: "
                    + attachMsg.type);
            return;
        }

        if (attachMsg.isInboxMsgDownloalable()) {
            dumpStreamFile.write((getString(typeResId) + " "
                    + getString(R.string.STR_NMS_MSG_DUMPER_MSG_URL) + attachMsg.url).getBytes());
        } else {
            if (attachMsg.path == null || attachMsg.path == "") {
                if (attachMsg.type != NmsIpMessageConsts.NmsIpMessageType.LOCATION) /*
                                                                                     * for
                                                                                     * location
                                                                                     * ,
                                                                                     * there
                                                                                     * may
                                                                                     * be
                                                                                     * not
                                                                                     * attach
                                                                                     */
                    NmsLog.error(NmsDownloadMessageHistory.logTag,
                            "error for attach message path is empty, type: " + attachMsg.type);
                return;
            }

            String attachName = new File(attachMsg.path).getName();
            int index = attachName.lastIndexOf(searchStr);

            if (index == -1) {
                NmsLog.error(NmsDownloadMessageHistory.logTag, "error to find " + searchStr + " "
                        + attachMsg.type + " in " + attachName);
                return;
            }

            String newAttachName = attachName.substring(0, index) + "_" + dumpIndex
                    + attachName.substring(index);

            zipFile(zipStream, attachMsg.path, zipEntryFolder + newAttachName);

            dumpStreamFile.write((getString(typeResId) + ": "+ newAttachName).getBytes());
        }

        dumpStreamFile.write("\r\n".getBytes());
    }
}

class NmsImageMessageDumper extends NmsAttachMessageDumper {

    protected NmsImageMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpImageMessage imgMsg = (NmsIpImageMessage) dumpMsg;
        if (imgMsg.caption.length() > 0)
            dumpStreamFile
                    .write((getString(R.string.STR_NMS_MSG_DUMPER_MSG_CAPTION) + " " + imgMsg.caption)
                            .getBytes());

        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsLocationMessageDumper extends NmsAttachMessageDumper {

    protected NmsLocationMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
        dumpMsg = NmsIpLocationMessage.formatLocationMsg((NmsIpLocationMessage)dumpMsg) ;
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpLocationMessage locaMsg = (NmsIpLocationMessage) dumpMsg;
        dumpStreamFile.write((getString(R.string.STR_NMS_MSG_DUMPER_COORDINATE) + " " + locaMsg.latitude
                + ", " + locaMsg.longitude).getBytes());
        dumpStreamFile.write("\r\n".getBytes());
        dumpStreamFile.write((getString(R.string.STR_NMS_MSG_DUMPER_ADDRESS) + " " + locaMsg.address)
                .getBytes());
        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsVoiceMessageDumper extends NmsAttachMessageDumper {

    protected NmsVoiceMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpVoiceMessage voiceMsg = (NmsIpVoiceMessage) dumpMsg;
        if (voiceMsg.caption.length() > 0) {
            dumpStreamFile
                    .write((getString(R.string.STR_NMS_MSG_DUMPER_MSG_CAPTION) + " " + voiceMsg.caption)
                            .getBytes());
            dumpStreamFile.write("\r\n".getBytes());
        }
        
        dumpStreamFile
                .write((getString(R.string.STR_NMS_MSG_DUMPER_DURATION) + " " + getDurationString(MessageUtils.decodeDurationFilename(voiceMsg.path)))
                        .getBytes());
        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsVideoMessageDumper extends NmsAttachMessageDumper {

    protected NmsVideoMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpVideoMessage videoMsg = (NmsIpVideoMessage) dumpMsg;
        if (videoMsg.caption.length() > 0) {
            dumpStreamFile
                    .write((getString(R.string.STR_NMS_MSG_DUMPER_MSG_CAPTION) + " " + videoMsg.caption)
                            .getBytes());
            dumpStreamFile.write("\r\n".getBytes());
        }
        
        dumpStreamFile
                .write((getString(R.string.STR_NMS_MSG_DUMPER_DURATION) + " " + getDurationString(MessageUtils.decodeDurationFilename(videoMsg.path)))
                        .getBytes());
        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsVcardMessageDumper extends NmsAttachMessageDumper {

    protected NmsVcardMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpVCardMessage vcardMsg = (NmsIpVCardMessage)dumpMsg;
        if (vcardMsg.name.length() > 0)
            dumpStreamFile
                    .write((getString(R.string.STR_NMS_MSG_DUMPER_VCARD_NAME) + " " + vcardMsg.name)
                            .getBytes());

        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

class NmsCalendarMessageDumper extends NmsAttachMessageDumper {

    protected NmsCalendarMessageDumper(NmsIpMessage msg, ZipOutputStream zip,
            BufferedOutputStream file, String entryFolder, int index) {
        super(msg, zip, file, entryFolder, index);
    }

    @Override
    protected void dump() throws IOException {
        super.dump();
        NmsIpCalendarMessage caleMsg = (NmsIpCalendarMessage)dumpMsg;
        if (caleMsg.summary.length() > 0)
            dumpStreamFile
                    .write((getString(R.string.STR_NMS_MSG_DUMPER_CALENDAR_SUMMARY) + " " + caleMsg.summary)
                            .getBytes());

        dumpStreamFile.write("\r\n\r\n".getBytes());
    }
}

