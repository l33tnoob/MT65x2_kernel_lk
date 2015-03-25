package com.hissage.struct;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import com.hissage.util.log.NmsLog;

public class SNmsSystemStatus {
    public static final short NMS_STATUS_NOT_ACTIVATED = -1;
    public static final short NMS_STATUS_INIT = 0;
    public static final short NMS_STATUS_UNCONNECTED = 1;
    public static final short NMS_STATUS_BLOCKING = 2;
    public static final short NMS_STATUS_CONNECTING = 3;
    public static final short NMS_STATUS_CONNECTED = 4;

    public short regStatus;
    public int recvBytes;
    public int sendBytes;
    public int recvBytesWifi;
    public int sendBytesWifi;
	public int recvBytesSim1;
    public int sendBytesSim1;
    public int recvBytesSim2;
    public int sendBytesSim2;	
    public String number;
    public String domain;
}