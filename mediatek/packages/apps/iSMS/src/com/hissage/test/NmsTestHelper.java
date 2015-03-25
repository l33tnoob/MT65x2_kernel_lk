package com.hissage.test;

import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapterforjni;
import com.hissage.util.log.NmsLog;

public class NmsTestHelper {

    public static int networkStatus = 0;
    public static int phoneStorageStatus = 0;
    public static int sdCardStatus = 0;
    public static int memoryStatus = 0;
    static int cLogPos = 0;
    static long javaLogPos = 0;

    public static int NMS_TEST_NETWORK_STATUS_OK = 0;
    public static int NMS_TEST_NETWORK_STATUS_NOT_OK = -1;
    public static int NMS_TEST_NETWORK_STATUS_SERVER_DONE = -2;

    public static int NMS_TEST_PHONE_STORAGE_STATUS_OK = 0;
    public static int NMS_TEST_PHONE_STORAGE_STATUS_CAN_NOT_WRITE = -1;

    public static int NMS_TEST_SD_CARD_STATUS_OK = 0;
    public static int NMS_TEST_SD_CARD_STATUS_CAN_NOT_WRITE = -1;
    public static int NMS_TEST_SD_CARD_STATUS_MISSING = -2;

    public static int NMS_TEST_MEMORY_STATUS_OK = 0;
    public static int NMS_TEST_MEMORY_STATUS_NOT_OK = -1;

    public static int NMS_TEST_ACTIVATED_STATUS_OK = 1;
    public static int NMS_TEST_ACTIVATED_STATUS_NOT_OK = 0;

    public static boolean isTestServiceStarted = false;
    public static boolean isKeyLockOn = true;

    public static void setNetworkStatus(int status) {
        networkStatus = status;
        engineadapter.get().nmsTestSetNetworkStatus(status);

    }

    public static void setPhoneStorageStatus(int status) {
        phoneStorageStatus = status;
        engineadapter.get().nmsTestSetPhoneStorageStatus(status);
    }

    public static void setSDCardStatus(int status) {
        sdCardStatus = status;
        engineadapter.get().nmsTestSetSDCardStatus(status);
    }

    public static void setMemoryStatus(int status) {
        memoryStatus = status;
        engineadapter.get().nmsTestSetMemoryStatus(status);
    }

    public static void setActivatedStatus(int status) {
        engineadapter.get().nmsTestSetActivatedStatus(status);
    }

    public static String getSelfNumber() {
        return engineadapter.get().nmsTestGetSelfNumber();
    }

    public static void clearLog() {
        engineadapter.get().nmsTestClearLog();
        recordLogPosition();
    }

    public static void recordLogPosition() {
        cLogPos = engineadapter.get().nmsTestGetCurLogPosition();
        javaLogPos = NmsLog.nmsTestGetJavaCurLogPos();
    }

    public static boolean waitAndSearchInLog(String logStr, int sleepTime) {

        boolean ret = false;

        if (sleepTime > 0) {
            try {
                Thread.sleep(sleepTime * 1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        ret = (engineadapter.get().nmsTestSearchInLog(cLogPos, logStr) >= 0);

        if (!ret)
            ret = NmsLog.nmsTestSearchInLog(javaLogPos, logStr);

        recordLogPosition();

        return ret;
    }
    
    public static String getiSmsTestPath() {
        return engineadapterforjni.getUserLogPath() + "/" + "test";
    }
}
