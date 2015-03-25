package com.mediatek.datatransfer.utils;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.mediatek.datatransfer.R;
import com.mediatek.datatransfer.modules.SmsBackupComposer;

import android.content.Context;
import android.content.SharedPreferences;
import android.telephony.SmsMessage;
import android.text.format.DateFormat;

public class CosmosBackupHandler implements BackupsHandler {
    private String mContactPath = "";
    private String mMMSPath = "";
    private String mSMSPath = "";
    private static final String CLASS_TAG = "CosmosBackupHandler";
    Context mContext;

    public CosmosBackupHandler(Context context) {
        // TODO Auto-generated constructor stub
        mContext = context;
    }

    @Override
    public boolean init() {
        // TODO Auto-generated method stub
        String externalStoragePath = SDCardUtils.getExternalStoragePath(mContext);
        if (externalStoragePath == null) {
            return false;
        }
        mContactPath = externalStoragePath + File.separator
                + Constants.BackupScanType.COSMOS_CONTACT_PATH;
        mMMSPath = externalStoragePath + File.separator + Constants.BackupScanType.COSMOS_MMS_PATH;
        mSMSPath = externalStoragePath + File.separator + Constants.BackupScanType.COSMOS_SMS_PATH;
        MyLogger.logD(CLASS_TAG, "init()=====>>mContactPath is " + mContactPath);
        return true;
    }

    @Override
    public void onStart() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "onStart()");
        File contactParentFile = new File(mContactPath);
        if (contactParentFile.exists() && contactParentFile.isDirectory()
                && contactParentFile.canRead()) {
            List<File> files = filterFiles(contactParentFile.listFiles());
            File newestFile = FileUtils.getNewestFile(files);
            if (newestFile != null) {
                result.add(newestFile);
            } else {
                MyLogger.logE(CLASS_TAG, "contact file is null!!");
            }
        }

        File mmsParentFile = new File(mMMSPath);
        if (mmsParentFile.exists() && mmsParentFile.isDirectory() && mmsParentFile.canRead()) {
            List<File> files = filterFiles(mmsParentFile.listFiles());
            File newestFile = FileUtils.getNewestFile(files);
            if (newestFile != null) {
                result.add(newestFile);
            } else {
                MyLogger.logE(CLASS_TAG, "mms file is null!!");
            }
        }

        File smsParentFile = new File(mSMSPath);
        if (smsParentFile.exists() && smsParentFile.isFile() && smsParentFile.canRead()) {
            MyLogger.logD(CLASS_TAG, "smsParentFile is " + mSMSPath);
            File vmsg = generateSmsSmvg(smsParentFile);
            if (vmsg != null) {
                result.add(vmsg);
            } else {
                MyLogger.logE(CLASS_TAG, "vmsg file is null!!");
            }
        }
    }

    private static final String mStorageName = "sms.vmsg";
    BufferedWriter mWriter = null;

    private File generateSmsSmvg(File pduFile) {
        // TODO Auto-generated method stub
        List<MySmsMessage> smss = parsePDUFile(pduFile);
        File file = null;
        if (smss != null && !smss.isEmpty()) {
            file = new File(pduFile.getParentFile().getAbsolutePath() + File.separator
                    + mStorageName);
            if (!file.exists()) {
                try {
                    file.createNewFile();
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "generateSmsSmvg():create file failed!"
                            + file.getAbsolutePath());
                }
            }
            try {
                mWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            MyLogger.logE(CLASS_TAG, "smss count is " + smss.size());
            for (MySmsMessage message : smss) {
                MyLogger.logE(CLASS_TAG, "message DisplayMessageBody type is "
                        + message.getMessage().getDisplayMessageBody());
                SmsMessage smsMessage = message.getMessage();

                long mtime = message.getTimeStamp();

                String timeStamp = formatTimeStampString(mtime);

                String read = message.getType() == UNREAD ? "UNREAD" : "READ";

                String seen = "1";

                String boxType = (message.getType() == SEND ? "SENDBOX" : "INBOX");

                String mSlotid = "0";

                String smsAddress = (message.getType() == SEND ? smsMessage.getDestinationAddress()
                        : smsMessage.getOriginatingAddress());

                String sc = smsMessage.getServiceCenterAddress();

                String body = smsMessage.getDisplayMessageBody() == null ? "" : smsMessage
                        .getDisplayMessageBody();
                StringBuffer sbf = new StringBuffer(body);
                int num = 0;
                num = sbf.indexOf("END:VBODY");
                do {
                    if (num >= 0) {
                        sbf.insert(num, "/");
                    } else {
                        break;
                    }
                } while ((num = sbf.indexOf("END:VBODY", num + 1 + "END:VBODY".length())) >= 0);
                body = sbf.toString();
                MyLogger.logD(CLASS_TAG, "timeStamp =" +
                        timeStamp + "read = " + read + " boxType = " + boxType + "mSlotid = "
                        + mSlotid + " smsAddress = " + smsAddress + " body = " + body + " seen= "
                        + seen);
                String locked = "UNLOCKED";
                try {
                    if (mWriter != null) {
                        mWriter.write(SmsBackupComposer.combineVmsg(timeStamp, read, boxType,
                                mSlotid, locked, smsAddress, body, seen));
                        mWriter.flush();
                    }
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "mWriter.write() failed");
                }
            }
        }
        MyLogger.logE(CLASS_TAG, "VMSG FILE IS " + file.getAbsolutePath());
        return file;

    }

    private String formatTimeStampString(long when) {// , boolean fullFormat
        CharSequence formattor = DateFormat.format("yyyy/MM/dd kk:mm:ss", when);
        return formattor.toString();
    }

    private static final int READ = 0x01;
    private static final int UNREAD = 0x03;
    private static final int SEND = 0x05;
    private static final int STATUS_START_INDEX = 2;
    private static final int STATUS_END_INDEX = 3;
    private static final int PDU_START_INDEX = 3;
    private static final int PDU_END_INDEX = 178;
    private static final int TIME_END_INDEX = 182;

    private List<MySmsMessage> parsePDUFile(File pduFile) {
        // TODO Auto-generated method stub
        List<MySmsMessage> smsMessages = new ArrayList<MySmsMessage>();
        FileInputStream inputStream = null;
        byte[] buffer = new byte[190];
        int length = -1;
        try {
            inputStream = new FileInputStream(pduFile);
            while ((length = inputStream.read(buffer)) != -1) {
                byte[] status = Arrays.copyOfRange(buffer, STATUS_START_INDEX, STATUS_END_INDEX);
                byte statu = status[0];
                if (statu == READ || statu == UNREAD || statu == SEND) {
                    byte[] sum = Arrays.copyOfRange(buffer, PDU_START_INDEX, PDU_END_INDEX);
                    byte[] timedata = Arrays.copyOfRange(buffer, PDU_END_INDEX, TIME_END_INDEX);
                    ;
                    SmsMessage message = SmsMessage.createFromPdu(sum);
                    smsMessages.add(new MySmsMessage(message, statu, Utils
                            .getFPsendSMSTime(timedata)));
                    buffer = new byte[190];
                }
            }
        } catch (IOException e) {
            // TODO: handle exception
            MyLogger.logE(CLASS_TAG, "@Tcard/SMS/PDU.o file read failed!");
        }
        return smsMessages;
    }

    private class MySmsMessage {

        SmsMessage message = null;
        int type = -1;
        long timeStamp = 0;

        public MySmsMessage(SmsMessage message, int type, long timeStamp) {
            super();
            this.message = message;
            this.type = type;
            this.timeStamp = message.getTimestampMillis() == 0 ? timeStamp : message
                    .getTimestampMillis();
            MyLogger.logD(CLASS_TAG, toString());
        }

        public SmsMessage getMessage() {
            return message;
        }

        public int getType() {
            return type;
        }

        public long getTimeStamp() {
            return timeStamp;
        }

        @Override
        public String toString() {
            return "MySmsMessage [message=" + message.getDisplayMessageBody() + ", type=" + type
                    + ", timeStamp=" + timeStamp + "]";
        }

    }

    private List<File> filterFiles(File[] files) {
        if (files == null) {
            return null;
        }
        List<File> list = new ArrayList<File>();
        for (File file : files) {
            if (file != null && file.isFile()) {
                String fileName = file.getName().trim();
                if ((fileName.endsWith(".vcf") && fileName.startsWith("Contact_"))
                        || (fileName.endsWith(".s") || fileName.endsWith(".m"))) {
                    list.add(file);
                }
            }
        }
        return list;
    }

    @Override
    public void cancel() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "cancel()");

    }

    @Override
    public List<File> onEnd() {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "onEnd()");
        if (mWriter != null) {
            try {
                mWriter.close();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } finally {
                mWriter = null;
            }
        }
        return result;
    }

    @Override
    public String getBackupType() {
        // TODO Auto-generated method stub
        return Constants.BackupScanType.COSMOS;
    }

    @Override
    public void reset() {
        // TODO Auto-generated method stub
        result.clear();
    }
}
