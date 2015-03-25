package com.mediatek.systemupdate.tests;

import android.app.AlarmManager;
import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.Handler;
import android.os.storage.IMountService;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.systemupdate.MainEntry;
import com.mediatek.systemupdate.R;
import com.mediatek.systemupdate.StorageReceiver;
import com.mediatek.systemupdate.UpdatePackageInfo;
import com.mediatek.xlog.Xlog;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import junit.framework.Assert;

public class MainEntryTests extends ActivityInstrumentationTestCase2<MainEntry> {

    private final String TAG = "SystemUpdate/MainEntryTest";

    private static final String OTA_PREFERENCE = "system_update";
    private static final String OTA_PRE_STATUS = "downlaodStatus";
    private static final String OTA_UNZ_STATUS = "isunzip";
    private static final String OTA_REN_STATUS = "isrename";
    private static final String OTA_PRE_DOWNLOAND_PERCENT = "downloadpercent";
    private static final String EXTERNAL_USB_STORAGE = "usbotg";
    private static final String NEED_REFRESH_PACKAGE = "need_refresh_package";
    private static final String NEED_REFRESH_MENU = "need_refresh_menu";
    private static final String ACTIVITY_ID = "activity_id";
    private static final String IS_SHUTTING_DOWN = "is_shutting_down";

    private static final String SD_CARD_PATH = "/storage/sdcard1";
    private static final int STATE_QUERYNEWVERSION = 0;
    private static final int STATE_NEWVERSION_READY = 1;

    private static final int MENU_ID_REFRESH = Menu.FIRST;
    /*
     * The testing activity
     */
    private MainEntry mActivity;

    private SharedPreferences mPreference = null;

    private Context mContext;

    /*
     * The intsrumenation
     */
    Instrumentation mInst;

    /*
     * Constructor
     */
    public MainEntryTests() {
        super("com.mediatek.systemupdate", MainEntry.class);

    }

    /*
     * Sets up the test environment before each test.
     */
    @Override
    protected void setUp() throws Exception {

        super.setUp();

        setActivityInitialTouchMode(false);

        mInst = getInstrumentation();

        mContext = mInst.getTargetContext();

        mPreference = mContext.getSharedPreferences(OTA_PREFERENCE, Context.MODE_PRIVATE);

    }

    protected void tearDown() throws Exception {

        if (mActivity != null) {
            mActivity.finish();
            Xlog.i(TAG, "activity finish");
            mInst.waitForIdleSync();

            sleepThread(2000);
        }
        super.tearDown();
    }

    private void resetDownloadDesctiptor() {

        mPreference.edit().putInt(OTA_PRE_DOWNLOAND_PERCENT, -1).commit();
        mPreference.edit().putBoolean(OTA_UNZ_STATUS, false).commit();
        mPreference.edit().putBoolean(OTA_REN_STATUS, false).commit();
        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_QUERYNEWVERSION).commit();
        mPreference.edit().putBoolean(NEED_REFRESH_PACKAGE, true).commit();
        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();

    }

    /*
     * private boolean isNetWorkAvailable() {
     * 
     * boolean ret = false;
     * 
     * try { ConnectivityManager connetManager = (ConnectivityManager) mContext
     * .getSystemService(Context.CONNECTIVITY_SERVICE); if (connetManager ==
     * null) { Xlog.e(TAG, "isNetWorkAvailable connetManager = null"); return
     * ret; } NetworkInfo[] infos = connetManager.getAllNetworkInfo(); if (infos
     * == null) { return ret; }
     * 
     * for (int i = 0; i < infos.length && infos[i] != null; i++) { if
     * (infos[i].isConnected() && infos[i].isAvailable()) { ret = true; break; }
     * }
     * 
     * } catch (Exception e) {
     * 
     * e.printStackTrace(); } Xlog.i(TAG, "isNetWorkAvailable result is : " +
     * ret); return ret; }
     */

    /*
     * Test the pre-condition
     */
    public void testcase01_Preconditions() {

        assertTrue(mPreference != null);
        resetDownloadDesctiptor();
        assertTrue(mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_QUERYNEWVERSION);

    }

    public void testcase02_QueryResult() {

        if (!Util.isNetWorkAvailable(mContext, null)) {
            resetDownloadDesctiptor();
            mActivity = getActivity();

            assert (mActivity != null);

            for (int i = 0; i <= 5; i++) {

                if (mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_QUERYNEWVERSION) {
                    sleepThread(3000);
                } else {
                    break;
                }
            }
            Xlog.i(TAG, "query finish");
            assertTrue((mPreference.getInt(OTA_PRE_STATUS, -1) == (STATE_QUERYNEWVERSION))
                    || (mPreference.getInt(OTA_PRE_STATUS, -1) == (STATE_NEWVERSION_READY)));
            Xlog.i(TAG, "assert finish");

            waitForUIready(300);
        }

    }

    public void testcase03_Requery() {
        Xlog.i(TAG, "testcase03_Requery");

        mActivity = getActivity();

        View loadingContainer = mActivity.findViewById(R.id.loading_container);
        View listContainer = mActivity.findViewById(R.id.list_container);
        waitForUIready(300);
        Assert.assertEquals(View.VISIBLE, listContainer.getVisibility());
        Assert.assertEquals(View.INVISIBLE, loadingContainer.getVisibility());

        mInst.invokeMenuActionSync(mActivity, MENU_ID_REFRESH, 0);
        Assert.assertTrue("Refresh Menu, reset need_refresh_package fail",
                mPreference.getBoolean(NEED_REFRESH_PACKAGE, false));
//        Assert.assertEquals(2, mActivity.getPreferenceScreen().getPreferenceCount());
        waitForNewVersion();
    }

    public void testcase04_LoadFromFile() {
        Xlog.i(TAG, "testcase04_LoadFromFile");
        mPreference.edit().putBoolean(NEED_REFRESH_PACKAGE, false).commit();
        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();

        mActivity = getActivity();
        waitForUIready(1);
        Assert.assertTrue(2 <= mActivity.getPreferenceScreen().getPreferenceCount());
    }

    public void testcase05_RefreshUIWithOTA() {
        Xlog.i(TAG, "testcase05_RefreshUIWithOTA");

        // OTA package & SD packages
        Xlog.w(TAG, "list OTA package & SD packages");
        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_NEWVERSION_READY).commit();
        mActivity = getActivity();
        waitForUIready(300);

        // One OTA package and one SD package
        Xlog.w(TAG, "One OTA package & One SD packages");
        List<UpdatePackageInfo> infoList = new ArrayList<UpdatePackageInfo>();
        UpdatePackageInfo info = new UpdatePackageInfo();
        infoList.add(info);
        this.setmUpdateInfoList(infoList);
        this.invokeRefreshUI();
        waitForUIready(300);

        // Only OTA package
        Xlog.w(TAG, "Only OTA package");
        this.setmUpdateInfoList(null);
        mPreference.edit().putBoolean(NEED_REFRESH_MENU, true).commit();
        this.invokeRefreshUI();
        sleepThread(2000);
        Assert.assertTrue("Only OTA package, MainEntry should be finished", mActivity.isFinishing());
        this.sendKeys(KeyEvent.KEYCODE_BACK);
    }

    public void testcase06_RefreshUINoOTA() {
        Xlog.i(TAG, "testcase06_RefreshUINoOTA");

        // All SD packages
        Xlog.w(TAG, "list All SD packages");
        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_QUERYNEWVERSION).commit();
        mActivity = getActivity();
        waitForUIready(300);

        // No packages
        Xlog.w(TAG, "No packages");
        this.setmUpdateInfoList(null);
        this.invokeRefreshUI();
        waitForUIready(300);

        // Activity is Stopping, pop notification
        Xlog.w(TAG, "Activity is Stopping, pop notification");
        this.setFieldBoolean("mIsStopping", true);
        this.setFieldBoolean("mIsQuerying", true);
        this.invokeRefreshUI();
        sleepThread(2000);
        this.setFieldBoolean("mIsStopping", false);

        // Only SD package
        Xlog.w(TAG, "Only SD package");
        List<UpdatePackageInfo> infoList = new ArrayList<UpdatePackageInfo>();
        UpdatePackageInfo info = new UpdatePackageInfo();
        infoList.add(info);
        this.setmUpdateInfoList(infoList);
        this.invokeRefreshUI();
        sleepThread(2000);
        Assert.assertTrue("Only SD package, MainEntry should be finished", mActivity.isFinishing());
        this.sendKeys(KeyEvent.KEYCODE_BACK);
    }

    public void testcase07_RefreshTimeout() {
        Xlog.i(TAG, "testcase06_RefreshTimeout");
        mActivity = getActivity();
        this.waitForUIready(300);
        Assert.assertTrue("Don't need refresh UI", !mPreference.getBoolean(NEED_REFRESH_PACKAGE, false));

        Util.setAlarm(mContext, AlarmManager.RTC_WAKEUP, Calendar.getInstance().getTimeInMillis() + 1000,
                Util.Action.ACTION_REFRESH_TIME_OUT);

        sleepThread(15000);

        Assert.assertTrue("Need refresh UI", mPreference.getBoolean(NEED_REFRESH_PACKAGE, true));
    }

    public void testcase08_InstallingProcess() {
        Xlog.i(TAG, "testcase07_InstallingProcess");
        mPreference.edit().putInt(ACTIVITY_ID, 0).commit();

        mActivity = getActivity();
        mInst.waitForIdleSync();

        Assert.assertTrue("Installing Process, MainEntry should be finish", mActivity.isFinishing());

        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();
        mInst.waitForIdleSync();
    }

    public void testcase09_Navigation() {
        Xlog.i(TAG, "testcase08_Navigation");
        mActivity = getActivity();
        mInst.waitForIdleSync();
        waitForUIready(300);

        // mInst.invokeMenuActionSync(mActivity, android.R.id.home, 0);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_LEFT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
        sleepThread(1000);

        Assert.assertTrue("Click Navigation buttion, activity should be finished",
                mActivity.isFinishing());
    }

    public void testcase10_BackKey() {
        Xlog.i(TAG, "testcase09_BackKey");
        mActivity = getActivity();
        mInst.waitForIdleSync();

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.waitForIdleSync();
        Assert.assertTrue("Press Back key, activity should be finished", mActivity.isFinishing());
    }

    /**
     * Query done,SD card unmount
     */
    public void testcase11_SdcardUnmount() {
        Xlog.i(TAG, "testcase10_SdcardUnmount");
        mActivity = getActivity();
        waitForNewVersion();
        Util.unmountSdCard(mInst, Util.getAvailablePath(mContext));
        Assert.assertTrue("SDcard unmounted, activity should be finished", mActivity.isFinishing());
    }

    /**
     * Query done, SD card mount
     */
    public void testcase12_SdcardMount() {
        Xlog.i(TAG, "testcase11_SdcardMount");
        mActivity = getActivity();
        waitForUIready(300);

        Util.mountSdCard(mInst, Util.getAvailablePath(mContext));

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        Assert.assertTrue("SDcard mounted,set need_refresh_package fail",
                mPreference.getBoolean(NEED_REFRESH_PACKAGE, false));
    }

    public void testcase13_Shutdown() {
        Xlog.i(TAG, "testcase12_ShutDown");
        mPreference.edit().putBoolean(IS_SHUTTING_DOWN, false).commit();
        StorageReceiver receiver = new StorageReceiver();
        receiver.onReceive(mContext, new Intent(Util.Action.ACTION_SHUTDOWN));
        mInst.waitForIdleSync();

        sleepThread(1000);
        boolean result = mPreference.getBoolean(IS_SHUTTING_DOWN, false);
        Assert.assertTrue("Shutdown, set is_shutting_down flag fail", result);
    }

    public void testcase14_BootComplete() {
        Xlog.i(TAG, "testcase13_BootComplete");
        mPreference.edit().putBoolean(IS_SHUTTING_DOWN, true).commit();
        StorageReceiver receiver = new StorageReceiver();
        receiver.onReceive(mContext, new Intent(Util.Action.ACTION_BOOT_COMPLETED));
        mInst.waitForIdleSync();

        sleepThread(1000);
        boolean result = mPreference.getBoolean(IS_SHUTTING_DOWN, true);
        Assert.assertTrue("bootcomplete, reset is_shutting_down flag fail", !result);
    }

    private void sleepThread (int time){
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            fail("Thread.sleep("+time+")");
        }
    }
    private void waitForNewVersion() {
        int time = 0;
        while (mPreference.getBoolean(NEED_REFRESH_PACKAGE, true) && time < 200) {
            ++time;
            sleepThread(1000);
        }
        Assert.assertTrue("no need refresh", !mPreference.getBoolean(NEED_REFRESH_PACKAGE, true));
    }

    private void waitForUIready(int maxTimes) {
        View listContainer = mActivity.findViewById(R.id.list_container);
        int time = 0;
        while (View.VISIBLE != listContainer.getVisibility() && time < maxTimes) {
            ++time;
            sleepThread(2000);
        }
    }

    private void invokeRefreshUI() {
        try {
            Class cls = MainEntry.class;
            Method m = cls.getDeclaredMethod("refreshUI");
            Xlog.v(TAG, "[invokeRefreshUI]get refreshUI method");
            m.setAccessible(true);
            m.invoke(mActivity, (Object[]) null);
            Xlog.v(TAG, "[invokeRefreshUI]refreshUI invoked");
            mInst.waitForIdleSync();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    private void setFieldBoolean(String fieldName, boolean value) {
        try {
            Class cls = MainEntry.class;
            Field f = cls.getDeclaredField(fieldName);
            Xlog.v(TAG, "get Field : " + fieldName);
            f.setAccessible(true);
            f.setBoolean(mActivity, value);
            Xlog.v(TAG, "set value : " + value);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }

    private void setmUpdateInfoList(List<UpdatePackageInfo> value) {
        try {
            Class cls = MainEntry.class;
            Field f = cls.getDeclaredField("mUpdateInfoList");
            Xlog.v(TAG, "get Field : mUpdateInfoList");
            f.setAccessible(true);
            f.set(mActivity, value);
            Xlog.v(TAG, "set value");
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }
}
