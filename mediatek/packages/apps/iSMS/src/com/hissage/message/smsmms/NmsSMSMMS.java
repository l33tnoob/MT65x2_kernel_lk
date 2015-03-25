package com.hissage.message.smsmms;

import com.hissage.struct.SNmsMsgKey;
import android.provider.Telephony.Sms;
import android.provider.Telephony.ThreadSettings;
import android.net.Uri;

public class NmsSMSMMS {

    public static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");
    public static final Uri MMS_CONTENT_URI = Uri.parse("content://mms");
    public static final Uri SMS_CONTENT_SENT_URI = Uri.parse("content://sms/sent");
    public static final Uri SMS_CONTENT_DRAFT_URI = Uri.parse("content://sms/draft");
    public static final Uri SMS_MMS_CONTENT_URI = Uri
            .parse("content://mms-sms/complete-conversations"); // added by
                                                                // luozheng in
                                                                // 12.2.29;
    public static final Uri THREAD_SETTINGS = Uri.parse("content://mms-sms/thread_settings/"); // for integration only
    public static final Uri MMS_CONTENT_THREAD_URI = Uri.parse("content://mms-sms/conversations");
    public static final Uri DELETE_SMS_URI = Uri.parse("content://sms/conversations/");
    public static final Uri MMS_CONTENT_URI_PART = Uri.parse("content://mms/part"); // 彩信附件表
    public static final Uri SMS_CANONICAL_ADDRESSES_URI = Uri
            .parse("content://sms/canonical_addresses");

    public static final String SMS_RECEIVED = "android.provider.Telephony.SMS_RECEIVED";
    
    public static final String PRELOAD_IP_MSG_NUMBER = "35221601851";

    public static final String IP_RECORDID_EXTENTION = Sms.IPMSG_ID;
    public static final String IP_SPAM_EXTENTION = ThreadSettings.SPAM;
    public static final String IP_SLIENCE_FLAG = "notification_enable";
    public static final String SIM_ID = "sim_id";
    public static final String TYPE = "type";
    public static final String THREAD_ID = "thread_id";
    public static final String SERVICE_CENTER = "service_center";
    public static final String ADDRESS = "address";
    public static final String PERSON_ID = "person";
    public static final String DATE = "date";
    public static final String DATE_SENT = "date_sent";
    public static final String READ = "read";
    public static final String BODY = "body";
    public static final String PROTOCOL = "protocol";
    public static final String _ID = "_id";
    public static final String SUB = "sub";
    public static final String MSG_BOX = "msg_box";
    public static final String STATUS = "status";
    public static final String LOCKED = "locked";
    public static final String CT_T = "ct_t";
    public static final String MESSAGE_COUNT = "message_count";
    public static final String NORM_DATE = "normalized_date";
    public static final String RECIPIENT_IDS = "recipient_ids";
    public static final String SEEN = "seen";
    public static final String MSG_TYPE = "m_type";

    public static final int SMS_TYPE_ALL = 0;
    public static final int SMS_TYPE_INBOX = 1;
    public static final int SMS_TYPE_SENT = 2;
    public static final int SMS_TYPE_DRAFT = 3;
    public static final int SMS_TYPE_OUTBOX = 4;
    public static final int SMS_TYPE_FAILED = 5; // for failed outgoing messages
    public static final int SMS_TYPE_QUEUED = 6; // for messages to send later

    public static final int SMS_DELIVER_OK = 0;
    public static final int SMS_DELIVER_UNKNOWN = -1;

    public static final int PROTOCOL_SMS = SNmsMsgKey.NMS_MSG_SOURCE_SMS;// SMS_PROTO
    public static final int PROTOCOL_MMS = SNmsMsgKey.NMS_MSG_SOURCE_MMS;// MMS_PROTO

}