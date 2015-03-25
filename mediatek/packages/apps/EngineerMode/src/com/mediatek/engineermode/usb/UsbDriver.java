package com.mediatek.engineermode.usb;

public class UsbDriver {

    static {
        System.loadLibrary("em_usb_jni");

    }

    public static final String[] MSG = { "Driver return 0.",
            "Attached device not support.", "Device not connected/responding.",
            "Unsupported HUB topology." };
    public static final int MSG_LEN = 4;

    public static native boolean nativeInit();

    public static native void nativeDeInit();

    public static native boolean nativeCleanMsg();

    public static native int nativeGetMsg();

    public static native boolean nativeStartTest(int n);

    public static native boolean nativeStopTest(int n);
}
