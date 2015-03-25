package com.mediatek.common.mom;

/**
 * This class defined permissions to be monitored.
 * Each sub-permission has an parent permission defined by android,
 * and the operation can be executed only with all of the
 * corresponding permissions are granted.
 * To add a item here should sync to PermissionRecordHelper.java, too.
 */
public class SubPermissions {
    // READ_SMS
    public static final String QUERY_SMS = "sub-permission.QUERY_SMS";
    public static final String QUERY_MMS = "sub-permission.QUERY_MMS";
    // READ_CONTACTS
    public static final String QUERY_CONTACTS = "sub-permission.QUERY_CONTACTS";
    // READ_CALL_LOG
    public static final String QUERY_CALL_LOG = "sub-permission.QUERY_CALL_LOG";
    // SEND_SMS
    public static final String SEND_SMS = "sub-permission.SEND_SMS";
    // INTERNET
    public static final String SEND_MMS = "sub-permission.SEND_MMS";
    // ACCESS_FINE_LOCATION
    public static final String ACCESS_LOCATION = "sub-permission.ACCESS_LOCATION";
    // RECORD_AUDIO
    public static final String RECORD_MIC = "sub-permission.RECORD_MIC";
    // CAMERA
    public static final String OPEN_CAMERA = "sub-permission.OPEN_CAMERA";
    // CALL_PHONE
    public static final String MAKE_CALL = "sub-permission.MAKE_CALL";
    public static final String MAKE_CONFERENCE_CALL = "sub-permission.MAKE_CONFERENCE_CALL";
    // CHANGE_NETWORK_STATE
    public static final String CHANGE_NETWORK_STATE_ON = "sub-permission.CHANGE_NETWORK_STATE_ON";
    // CHANGE_WIFI_STATE
    public static final String CHANGE_WIFI_STATE_ON = "sub-permission.CHANGE_WIFI_STATE_ON";
    // BLUETOOTH_ADMIM
    public static final String CHANGE_BT_STATE_ON = "sub-permission.CHANGE_BT_STATE_ON";
    // READ_PHONE_STATE
    public static final String READ_PHONE_IMEI = "sub-permission.READ_PHONE_IMEI";
    // HOTKNOT
    public static final String HOTKNOT_BIND = "sub-permission.HOTKNOT_BIND";
}
