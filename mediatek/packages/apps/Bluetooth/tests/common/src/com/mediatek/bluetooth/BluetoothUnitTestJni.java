
package com.mediatek.bluetooth;

import android.util.Log;

import java.util.ArrayList;

public class BluetoothUnitTestJni {

    private static final boolean DEBUG = true;

    private static final String TAG = "[BT][UT][BluetoothUnitTestJni]";

    public static final int UT_PROFILE_AVRCP = 0;
    
    public static final int UT_PROFILE_HID = 1;

    public static final int UT_PROFILE_OPP = 2;

    public static final int UT_PROFILE_PAN = 3;

    public static final int UT_PROFILE_PBAP = 4;

    public static final int UT_PROFILE_A2DP = 5;

    public static final int UT_PROFILE_HDP = 6;

    public static final int UT_PROFILE_NOUSE = 255;

//    private IServiceCheckInterface mCheckInterface;

    private ArrayList<IServiceCheckInterface> mInterfaceList;
    
    private ICleanUpInterface mCleanUpInterface;

    private static final String TRUE = "true";

    //check log num
    private int mCbLogNum;

    private ArrayList<String> mCheckLogs;

    //current call back check num
    private int mReturnCheckNum;

    static {
        System.loadLibrary("extut_simulator");
    }

    /**
     * Constructor
     */
    public BluetoothUnitTestJni() {
        mCbLogNum = 0;
        mReturnCheckNum = 0;
        mCheckLogs = new ArrayList<String>();
        mInterfaceList = new ArrayList<IServiceCheckInterface>();
//        mCleanUpInterface = new ICleanUpInterface();
    }

    /**
     * Set interface to make a call which in test case
     * @param inter IServiceCheckInterface<br>
     *              which should implements IServiceCheckInterface
     */
    public void addServiceCheckInterface(IServiceCheckInterface inter) {
        if (inter != null) {
            Log.d(TAG, "[API:addInterface] Interface is " + inter);
            //mCheckInterface = inter;
            if (mInterfaceList != null) {
                mInterfaceList.add(inter);
            }
        } else {
            Log.d(TAG, "[API:addServiceCheckInterface] the input interface is null");
        }
    }

    public void setCleanUpInterface(ICleanUpInterface inter) {
        if (inter != null) {
            Log.d(TAG, "[API:setCleanUpInterface] Interface is " + inter);
            mCleanUpInterface = inter;
        } else {
            printErrorLog("[API:setCleanUpInterface] Interface is null");
        }
    }

    /**
     * set case log to native.<br>
     * First initUnitTestNative.<br>
     * Check need callback log num.<br>
     * The method should be called before setCaseCheckLog method,if you need callback
     * 
     * @param logs String[] <br>
     *            string table for log which will be sent to jni
     * @param separator String <br>
     *            separator which contains in the log
     */
    public void setCaseLog(String[] logs, String separator) {
        initUnitTestNative();
        if (logs.length == 0 || separator == null) {
            printErrorLog("[API:setCaseLog] the param is wrong");
            return;
        }
        for (String eachLog : logs) {
            String[] ss = eachLog.split(separator);
            if (TRUE.equals(ss[2])) {
                mCbLogNum ++;
            }
        }
        printDebugLog("[API:setCaseJniLog] need call back num is : " + mCbLogNum);
        setCaseLogsNative(logs, separator);
    }

     /**
     * set case log profile ID to native.<br>
     * for separate different profiles
     * @param int
     *            profile ID
     */
    public void setCurLogProfileID(int profileID) {
        setLogProfileNative(profileID);
    }

    /**
     * set test case check log.
     * @param checkLogs ArrayList<String> <br>
     *        the expected result list
     * @return
     *      set succeed return true, else return false
     */
    public boolean setCaseExpectedResults(ArrayList<String> checkLogs) {
        if (mCbLogNum != 0) {
            if (checkLogs == null || checkLogs.size() == 0) {
                printErrorLog("[API:setCaseExpectedResults] check log cann't be null");
                return false;
            }
            if (checkLogs.size() != mCbLogNum) {
                printErrorLog("[API:setCaseExpectedResults] check log num is not equal need callback log num");
                return false;
            }
            mCheckLogs = checkLogs;
            for (String s : checkLogs) {
                printDebugLog("[API:setCaseExpectedResults] the check value is : " + s);
            }
            return true;
        }
        printErrorLog("[API:setCaseExpectedResults] there's no log need call back");
        return false;
    }

    public void setCaseLog(String[] logs, String separator, String remote) {
        setCaseLogs2RemoteNative(logs, separator, remote);
    }

    /**
     * This method used to clean jni logs ,and clean jni global object and environment<br>
     * This method should be called in last jni callback.
     */
    private void cleanUp() {
        printDebugLog("clean up enter");
        if (mCheckLogs != null) {
            mCheckLogs.removeAll(mCheckLogs);
            mCheckLogs = null;
        }
        if (mInterfaceList != null) {
            mInterfaceList.removeAll(mInterfaceList);
            mInterfaceList = null;
        }
        mCbLogNum = 0;
        mReturnCheckNum = 0;
        if (mCleanUpInterface != null) {
            mCleanUpInterface.cleanUp();
            mCleanUpInterface = null;
        }
        printDebugLog("clean up enter --------");
    }

    /**
     * This method is callback method which accept the callback from bt_simulator
     * 
     * @param logId call back log id,which should be checked in test case
     */
    private void notifyStateChange(int logId, String retString) {
        if (mInterfaceList != null) {
            if (!mInterfaceList.isEmpty()) {
                printDebugLog("[API:notifyStateChange] return log id is : " + logId);
                for (IServiceCheckInterface singleInterface : mInterfaceList) {
                    singleInterface.checkState(logId, retString, mCheckLogs.get(mReturnCheckNum));
                }
                mReturnCheckNum ++;
            } else {
                printErrorLog("[API:notifyStateChange] mInterfaceList is empty");
            }
        } else {
            printErrorLog("[API:notifyStateChange] mInterfaceList is null");
        }
    }

    public void dumpEmmaReport(String remote) {
        dumpEmmaReportNative(remote);
    }

    public void callPrivateMethod(int classId, int id, String remote) {
        callPrivateMethodNative(classId, id, remote);
    }

    /**
     * print debug log
     * @param msg debug log message
     */
    private void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * print error log
     * @param msg error log message
     */
    private void printErrorLog(String msg) {
        Log.e(TAG, msg);
    }

    // native method
    private native void initUnitTestNative();

    private native void dumpEmmaReportNative(String remote);

    private native void callPrivateMethodNative(int classId, int id, String remote);

    private native void setCaseLogsNative(String[] logs, String separator);

    private native void setLogProfileNative(int profileID);

    private native void setCaseLogs2RemoteNative(String[] logs, String separator, String remote);
}
