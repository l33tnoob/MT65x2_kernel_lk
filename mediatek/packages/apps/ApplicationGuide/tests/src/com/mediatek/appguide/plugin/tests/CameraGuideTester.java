package com.mediatek.appguide.plugin.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.test.ActivityInstrumentationTestCase2;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.xlog.Xlog;

public class CameraGuideTester extends ApplicationGuideBase {
    public static final String TAG = "CameraGuideTester";
    
    private Activity mActivity;
    private Context mContext;
    private Instrumentation mInst;
    private Solo mSolo;
    private Resources mResource;
    private String mPackageName;
    private static boolean mFirstRun = false;

    private static Class<?> launcherActivityClass;
    private static final String PACKAGE_ID_STRING = "com.android.gallery3d";
    private static final String ACTIVITY_FULL_CLASSNAME = "com.android.camera.Camera";
    private static final int SLEEP_TIME = 5000;
    private static final int SUM_TIMES = 10;
    private static final String SHARED_PREFERENCE_NAME = "application_guide";
    private static final String KEY_CAMERA_GUIDE = "camera_guide";

    static {
        try {
            launcherActivityClass = Class.forName(ACTIVITY_FULL_CLASSNAME);
        } catch (Exception exception) {
            throw new RuntimeException(exception);
        } 
    }

    public CameraGuideTester() {
        super(PACKAGE_ID_STRING, launcherActivityClass);
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        Xlog.i(TAG, "setUp()");
        setActivityInitialTouchMode(false);
        mActivity = getActivity();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mSolo = new Solo(mInst, mActivity);

        mResource = mActivity.getResources();
        mPackageName = mActivity.getPackageName();
    }

    @Override
    public void tearDown() throws Exception {
        try {
            mSolo.finishOpenedActivities();
        } catch (Exception e) {
            Xlog.d(TAG, "tearDown exception");
        }
        super.tearDown();
    }

    /**
     * init CameraGuide
     */
    public void test01_Init() {
        Xlog.d(TAG, "test01_Init()");
        SharedPreferences sharedPrefs = mActivity.getSharedPreferences(SHARED_PREFERENCE_NAME,
                Context.MODE_WORLD_WRITEABLE);
        if (sharedPrefs.getBoolean(KEY_CAMERA_GUIDE, false)) {
            Xlog.d(TAG, "SharedPreferences is true");
            sharedPrefs.edit().putBoolean(KEY_CAMERA_GUIDE, false).commit();
            mSolo.sleep(1000);
            
            mFirstRun = !(sharedPrefs.getBoolean(KEY_CAMERA_GUIDE, false));
            Xlog.d(TAG, "mFirstRun = " + mFirstRun);
        }
        mSolo.goBack();
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    /**
     * Enter CameraGuide
     */
    public void test02_StartCameraGuide() {
        Xlog.d(TAG, "test02_StartCameraGuide()");
        mSolo.goBack();
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    /**
     * play the first video
     */
    public void test03_StepOne() {
        Xlog.d(TAG, "test03_StepOne()");
        if (mFirstRun) {
            boolean res = playVideo();
            assertTrue(res);           
        }
        mSolo.goBack();
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    /**
     * play the second video
     */
    public void test04_StepTwo() {
        Xlog.d(TAG, "test04_StepTwo()");
        if (mFirstRun) {
            boolean res = playVideo();
            if (res) {
                mSolo.clickOnButton(getButtonString());
                res = playVideo();
                assertTrue(res);           
            }           
        }
        mSolo.goBack();
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    /**
     * finish play video
     */
    public void test05_FinishCameraGuide() {
        Xlog.d(TAG, "test05_FinishCameraGuide()");
        if (mFirstRun) {
            boolean res = playVideo();
            if (res) {
                mSolo.clickOnButton(getButtonString());
                res = playVideo();
                if (res) {
                    mSolo.clickOnButton(getButtonString());
                    mSolo.sleep(500);
                }          
            }            
        }
        mSolo.goBack();
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    /**
     * 
     * @return true: search OK button, else return false
     */
    private boolean playVideo() {
        boolean res = false;
        mSolo.sleep(SLEEP_TIME);
        String buttonString = getButtonString();
        int i = 0;
        while (i++ < SUM_TIMES) {
            if (mSolo.searchButton(buttonString)) {
                res = true;
                break;
            } else {
                mSolo.sleep(1000);
            }
        }
        
        return res;
    }
    
    /**
     * get some String
     * @return the String of OK Button
     */
    private String getButtonString() {
        String buttonString = mSolo.getString(android.R.string.ok);
        Xlog.d(TAG, "buttonString = " + buttonString);
        return buttonString;
    }
}
