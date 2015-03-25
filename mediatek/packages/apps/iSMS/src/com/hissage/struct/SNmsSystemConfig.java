package com.hissage.struct;

public class SNmsSystemConfig {
    public static final byte NMS_SEND_MODE_iSMS = 0;
    public static final byte NMS_SEND_MODE_SMS = 1;
    public static final byte NMS_SEND_MODE_AUTO = 2;

    public byte connMode;
    public byte heartbeat;
    public byte autoStart;
    public byte backupsms;
    public int  mainSimId;
}
