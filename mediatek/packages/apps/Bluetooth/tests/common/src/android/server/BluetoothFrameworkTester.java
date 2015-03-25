
/*******************************************************************************
*
*   The class is used to receive the bt_framework call back
*   Transfer the call back to different class.
*
*******************************************************************************/

package android.server;

import android.util.Log;

import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class BluetoothFrameworkTester {

    private static final String TAG = "BluetoothFrameworkTester";
    
    private static final boolean DEBUG = true;

    // "/data/data/a2dpcoverage.ec";
    private static final String DEFAULT_COVERAGE_FILENAME = "/data/data/a2dpcoverage.ec";

    private static BluetoothFrameworkTester sInstance;

//    static final String ACTION_RESET_COVERAGE = "action.resetcoverage";

//    static final String ACTION_DUMP_COVERAGE = "action.dumpcoverage";

    private static PrivateA2dpTestCase sPrivateA2dp;

    /**
    * Construnctor
    */
    BluetoothFrameworkTester() {
        printDebugLog(" new BluetoothFrameworkTester ");
    }

    /**
    * callback method which used to generate the class instance
    */
    private static void callbackCreateBTFrameworkTester(boolean result) {
        if (sInstance == null) {
            sInstance = new BluetoothFrameworkTester();
        }
    }

    /**
    * callback method which used to generate the coverage.ec by using reflector
    */
    private static void callbackDumpCoverage() {

        //new file which location is /data/data/a2dpcoverage.ec
        File coverageFile = new File(DEFAULT_COVERAGE_FILENAME);
        try {
            //get emma class
            Class<?> emmaRTClass = Class.forName("com.vladium.emma.rt.RT");
            //get generate coverage method
            Method dumpCoverageMethod = emmaRTClass.getMethod("dumpCoverageData", coverageFile
                    .getClass(), boolean.class, boolean.class);
            //generate coverage.ec
            dumpCoverageMethod.invoke(null, coverageFile, false, false);
        } catch (ClassNotFoundException e) {
            reportEmmaError(e);
        } catch (SecurityException e) {
            reportEmmaError(e);
        } catch (NoSuchMethodException e) {
            reportEmmaError(e);
        } catch (IllegalArgumentException e) {
            reportEmmaError(e);
        } catch (IllegalAccessException e) {
            reportEmmaError(e);
        } catch (InvocationTargetException e) {
            reportEmmaError(e);
        }
    }

    /**
    * callback method which used to call private method in PrivateTestCase
    * @param classId used to switch different class method(HDP, A2DP)
    * @param id which method to call
    */
    private static void callbackCallPrivateMethod(int classId, int id) {
        printDebugLog("[API:callbackPrivateMethod] classId is : " + classId + ", method id is : " + id);
        if (classId == 0) {
            if (sPrivateA2dp == null) {
                sPrivateA2dp = new PrivateA2dpTestCase();
            }
            sPrivateA2dp.callPrivateMethod(id);
        }
    }

    public static boolean onCreated() {
        printDebugLog("on Create");
        return true;
    }

    /**
     * print the debug log
     */
    private static void printDebugLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    /**
     * print the emma report while report failed
     */
    private static void reportEmmaError(Exception e) {
        Log.w(TAG, e.getLocalizedMessage());
    }

    private native boolean initServiceNative();

}
