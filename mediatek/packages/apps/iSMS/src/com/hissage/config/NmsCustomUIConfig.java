package com.hissage.config;

public class NmsCustomUIConfig {

    public static final boolean PRIVATEMESSAGE = true;// true:display the
                                                      // private message.
                                                      // false:disappear the
                                                      // private message item in
                                                      // sns setting page.
    public static final boolean AD_SHOW_ALWAYS = false;// true:display AD in
                                                       // mail viewer.
                                                       // false:disappear AD in
                                                       // page.
    public static final boolean NEED_GET_PAY_INFO = false; // true:get pay info
                                                           // from server,
                                                           // false:don't get
    public static final String AD_ADMOB_ID = "-1"; // The ad tag that is used
                                                   // for server, and account
                                                   // all click count.
    public static final boolean NEED_SHOW_WEBPASSWD = true;
    public static final boolean NEED_SHOW_PAY_INFO = true;
    public static final String AD_UNIT_ID = "a14ed30fed9daf2";
    public static final String ROOTDIRECTORY = "iSMS";
    public static final int COMPOSE_MAX_CONTACT_CNT = 50;
    public static final int COMPOSE_OVER_CONTACT_CNT = 30;
    public static final int COMPOSE_MAX_ADDR = 25;
    public static final int MESSAGE_MAX_LENGTH = 2048;
    public static final int CAPTION_MAX_LENGTH = 100;
    public static final int STATUE_MAX_LENGTH = 100;
    public static final int GROUPNAME_MAX_LENGTH = 64;
    public static final int GROUPMEM_MAX_COUNT = 99;
    public static final int PHONENUM_MAX_LENGTH = 30;
    public static final int LOCATION_ADDR_MAX_LENGTH = 100;
    public static final long MAX_FILE_SIZE = 1 * 1024 * 1024;
    public static final int MAX_MSG_NUM = 2000;
    public static final long VIDEO_MAX_SIZE = 1024 * 1024 * 2;// 2M
    public static final int VIDEO_MAX_DURATION = 30; // 30s
    public static final long AUDIO_MAX_DURATION = 180;
    public static final long MAX_FILE_SIZE_KEY = 300*1024;
    public static final int MAX_ATTACH_SIZE = 2 * 1024 * 1024;
    public static final int EMOTICONS_MAX_COUNT = 24;
    
    public static final long MIN_SEND_ATTACH_SD_CARD_SIZE = 2 * 1024 * 1024 ;
}
