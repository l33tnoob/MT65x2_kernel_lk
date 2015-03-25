package com.mediatek.systemupdate.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.storage.IMountService;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.systemupdate.SdPkgInstallActivity;
import com.mediatek.systemupdate.UpdatePackageInfo;

import com.mediatek.systemupdate.R;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

import junit.framework.Assert;

/***
 * 
 * @author MTK80800
 *
 * preconditions: Two upgrade packages in default available storage, and correct pkgInfos.xml in data/data/com.mediatek.systemupdate 
 */
public class SdPkgInstallActivityTests extends
        ActivityInstrumentationTestCase2<SdPkgInstallActivity> {

    private final String TAG = "SystemUpdate/SdPkgInstallTests";

    private static final String OTA_PREFERENCE = "system_update";
    private static final String OTA_PRE_STATUS = "downlaodStatus";
    private static final String OTA_PRE_DOWNLOAND_PERCENT = "downloadpercent";
    private static final String OTA_PRE_IMAGE_SIZE = "imageSize";
    private static final String OTA_PRE_VER = "version";
    private static final String OTA_PRE_VER_NOTE = "versionNote";
    private static final String OTA_PRE_DELTA_ID = "versionDeltaId";
    private static final String OTA_UNZ_STATUS = "isunzip";
    private static final String OTA_REN_STATUS = "isrename";
    private static final String OTA_QUERY_DATE = "query_date";
    private static final String OTA_UPGRADE_STATED = "upgrade_stated";
    private static final String OTA_ANDR_VER = "android_num";
    private static final String EXTERNAL_USB_STORAGE = "usbotg";

    private static final String NEED_REFRESH_PACKAGE = "need_refresh_package";
    private static final String NEED_REFRESH_MENU = "need_refresh_menu";
    private static final String ACTIVITY_ID = "activity_id";
    // private static final String SD_CARD_PATH = "/storage/sdcard1";

    // private static final int PKG_NUM = 2;
    private static final int PKG_ID_OK = 0;
    // private static final int PKG_ID_ZIP = 1;
    // private static final int PKG_ID_SCATTER = 2;

    static final int MSG_NETWORKERROR = 0;
    static final int MSG_NEWVERSIONDETECTED = 1;
    static final int MSG_RELOAD_ZIP_FILE = 2;
    static final int MSG_NONEWVERSIONDETECTED = 3;
    static final int MSG_DLPKGCOMPLETE = 4;
    static final int MSG_DLPKGUPGRADE = 5;
    static final int MSG_NOTSUPPORT = 6;
    static final int MSG_NOVERSIONINFO = 7;
    static final int MSG_DELTADELETED = 8;
    static final int MSG_SDCARDCRASHORUNMOUNT = 9;
    static final int MSG_SDCARDUNKNOWNERROR = 10;
    static final int MSG_SDCARDINSUFFICENT = 11;
    static final int MSG_SDCARDMOUNTED = 25;

    static final int MSG_SDCARDPACKAGESDETECTED = 12;

    static final int MSG_UNKNOWERROR = 13;
    static final int MSG_OTA_PACKAGEERROR = 14;

    static final int MSG_OTA_NEEDFULLPACKAGE = 15;
    static final int MSG_OTA_USERDATAERROR = 16;
    static final int MSG_OTA_USERDATAINSUFFICENT = 17;
    static final int MSG_OTA_CLOSECLIENTUI = 18;
    static final int MSG_OTA_SDCARDINFUFFICENT = 19;
    static final int MSG_OTA_SDCARDERROR = 20;
    static final int MSG_UNZIP_ERROR = 21;
    static final int MSG_CKSUM_ERROR = 22;
    static final int MSG_UNZIP_LODING = 23;
    static final int MSG_LARGEPKG = 24;

    private static final int MENU_ID_UPGRADE = Menu.FIRST;
    private static final int MENU_ID_REFRESH = Menu.FIRST + 1;
    /*
     * The testing activity
     */
    private SdPkgInstallActivity mActivity;
    private SharedPreferences mPreference = null;
    private Context mContext;

    /*
     * The intsrumenation
     */
    Instrumentation mInst;

    /*
     * Constructor
     */
    public SdPkgInstallActivityTests() {
        super("com.mediatek.systemupdate", SdPkgInstallActivity.class);

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
        mPreference = mContext.getSharedPreferences(OTA_PREFERENCE, Context.MODE_WORLD_READABLE);

    }

    protected void tearDown() throws Exception {

        if (mActivity != null && !mActivity.isFinishing()) {
            mActivity.finish();
            Xlog.i(TAG, "activity finish");
            mInst.waitForIdleSync();
            sleepForActivityFinish();
        }
        super.tearDown();
    }

    /**
     * Test the download function when new version detected
     */
    public void testcase01Layout() {
        Xlog.i(TAG, "testcase01Layout ");
        getActivityWithIntent(PKG_ID_OK);

        TextView textAndroidVer = (TextView) mActivity.findViewById(R.id.textAndroidNum);
        assertTrue(textAndroidVer != null);
        Xlog.i(TAG, textAndroidVer.getText().toString());

        TextView textLoadVer = (TextView) mActivity.findViewById(R.id.textVerNum);
        assertTrue(textLoadVer != null);
        Xlog.i(TAG, textLoadVer.getText().toString());

        mInst.waitForIdleSync();
    }

    /**
     * Available SD card is unmounted, popup a dialog, click OK button, Activity
     * finished
     */
    public void testcase02CheckPkgSdErr() {
        Xlog.i(TAG, "testcase02CheckPkgSdErr ");

        final String mountPoint = Util.getAvailablePath(mContext);
        Util.unmountSdCard(mInst, mountPoint);
        testPkg(PKG_ID_OK);

        Util.mountSdCard(mInst, mountPoint);
    }

    /**
     * SdCard Package missing, popup a dialog, click OK button, Activity
     * finished
     */
    public void testcase03CheckPkgMiss() {
        Xlog.i(TAG, "testcase03CheckPkgMiss ");

        Intent intent = new Intent();
        intent.setClass(mInst.getTargetContext(), SdPkgInstallActivity.class);
        intent.putExtra(SdPkgInstallActivity.KEY_VERSION, "version :ota_test");
        intent.putExtra(SdPkgInstallActivity.KEY_PATH, "data/ota_test");
        intent.putExtra(SdPkgInstallActivity.KEY_ANDROID_NUM, "Android test");
        intent.putExtra(SdPkgInstallActivity.KEY_NOTES, "It's only for test");
        this.setActivityIntent(intent);
        mActivity = getActivity();
        mInst.waitForIdleSync();

        invokeCheckUpgradePackage();
        mInst.waitForIdleSync();

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        sleepForActivityFinish();
        Assert.assertEquals("Activity is finishing", true, mActivity.isFinishing());
    }

    /**
     * SdCard insufficent, popup a dialog, click OK button, Activity finished
     */
    public void testcase04SdcardInsufficent() {
        Xlog.i(TAG, "testcase04SdcardInsufficent ");

        getActivityWithIntent(PKG_ID_OK);
        mPreference.edit().putInt(ACTIVITY_ID, PKG_ID_OK);

        sendEmptyMessage(MSG_SDCARDINSUFFICENT);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        Xlog.v(TAG, "[testcase04]Click Positive Button");
        mInst.waitForIdleSync();

        sleepForActivityFinish();
        Assert.assertEquals(true, mActivity.isFinishing());
        Assert.assertEquals(-1, mPreference.getInt(ACTIVITY_ID, -1));
    }

    /**
     * Unknown error happens, popup a dialog, click OK button, Activity finished
     */
    public void testcase05SdcardUnknownError() {
        Xlog.i(TAG, "testcase05SdcardUnknownError ");

        getActivityWithIntent(PKG_ID_OK);
        mPreference.edit().putInt(ACTIVITY_ID, PKG_ID_OK);

        sendEmptyMessage(MSG_SDCARDUNKNOWNERROR);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        Xlog.v(TAG, "[testcase05]unknown error, Click Positive Button");
        mInst.waitForIdleSync();
        sleepForActivityFinish();
        Assert.assertEquals("sdcard insufficent,finish activity", true, mActivity.isFinishing());
        Assert.assertEquals(-1, mPreference.getInt(ACTIVITY_ID, -1));
    }

    /**
     * Unzip package to default storage, error happens, popup a dialog, press
     * BACK key, Activity finished
     */
    public void testcase06UnzipError() {
        Xlog.i(TAG, "testcase06UnzipError ");

        getActivityWithIntent(PKG_ID_OK);
        mPreference.edit().putInt(ACTIVITY_ID, PKG_ID_OK);

        sendEmptyMessage(MSG_UNZIP_ERROR);
        mInst.waitForIdleSync();
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        Xlog.v(TAG, "[testcase06]mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK)");
        mInst.waitForIdleSync();
        sleepForActivityFinish();
        Assert.assertEquals(true, mActivity.isFinishing());
        Assert.assertEquals(-1, mPreference.getInt(ACTIVITY_ID, PKG_ID_OK));
    }

    /**
     * Unzip package to default storage, popup a progress dialog, press BACK
     * key, Activity finished
     */
    public void testcase07UnzipLoading() {
        Xlog.i(TAG, "testcase07UnzipLoading");

        getActivityWithIntent(PKG_ID_OK);

        sendEmptyMessage(MSG_UNZIP_LODING);
        mInst.waitForIdleSync();
//        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInst.sendKeySync(new KeyEvent( KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK));
        Xlog.v(TAG, "[testcase07]mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK)");
        mInst.waitForIdleSync();
        sleepForActivityFinish();
        Assert.assertEquals(true, mActivity.isFinishing());
        setFieldBoolean("sIsUnzip", false);
    }

    /**
     * Unzip package to default storage, popup a progress dialog, press BACK
     * key, Activity finished
     */
    public void testcase08NavigationMenu() {
        Xlog.i(TAG, "testcase08NavigationMenu");
        this.getActivityWithIntent(PKG_ID_OK);

        // mInst.invokeMenuActionSync(mActivity, android.R.id.home, 0);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_LEFT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        sleepForActivityFinish();
        Assert.assertTrue("Click Navigation buttion, activity should be finished", mActivity.isFinishing());

    }

    public void testcase09RefreshMenu() {
        Xlog.i(TAG, "testcase09RefreshMenu");
        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();
        mPreference.edit().putBoolean(NEED_REFRESH_MENU, true).commit();

        this.getActivityWithIntent(PKG_ID_OK);
        mInst.invokeMenuActionSync(mActivity, MENU_ID_REFRESH, 0);
        mInst.waitForIdleSync();

        // Assert.assertTrue("Press Refresh menu, activity should be finished",
        // mActivity.isFinishing());
        Assert.assertEquals(true, mPreference.getBoolean(NEED_REFRESH_PACKAGE, false));
        waitForNewVersion();
    }

    /**
     * Unmouont SDcard before install, finish activity and toast a message
     */
    public void testcase10SdcardUnmount() {
        Xlog.i(TAG, "testcase10SdcardUnmount");
        getActivityWithIntent(PKG_ID_OK);

        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();
        Util.unmountSdCard(mInst, Util.getAvailablePath(mContext));
        mInst.waitForIdleSync();

        sleepForActivityFinish();
        Assert.assertTrue("SDcard unmounted activity finished", mActivity.isFinishing());
    }

    /**
     * Mount SDcard before install, Popup a Dialog
     */
    public void testcase11SdcardMount() {
        Xlog.i(TAG, "testcase11SdcardMount");
        getActivityWithIntent(PKG_ID_OK);

        Util.mountSdCard(mInst, Util.getAvailablePath(mContext));
        mInst.waitForIdleSync();
        // Should popup a dialog
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        Assert.assertTrue("SDcard mounted,set need_refresh_package fail",
                mPreference.getBoolean(NEED_REFRESH_PACKAGE, false));

        waitForNewVersion();
    }

    /**
     * Unmount SDcard in installing process.Activity finish, and SystemUpdate
     * popup a Dialog
     */
    public void testcase12SdcardUnmount() {
        Xlog.i(TAG, "testcase12SdcardUnmount");
        getActivityWithIntent(PKG_ID_OK);

        mPreference.edit().putInt(ACTIVITY_ID, 0).commit();
        Util.unmountSdCard(mInst, Util.getAvailablePath(mContext));
        mInst.waitForIdleSync();

        sleepForActivityFinish();
        Assert.assertTrue("SDcard unmounted activity finished", mActivity.isFinishing());

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        Xlog.v(TAG, "[testcase12]mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK)");
        // mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        // mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        Assert.assertEquals(-1, mPreference.getInt(ACTIVITY_ID, 0));
    }

    /**
     * Mount SDcard in installing process, No response
     */
    public void testcase13SdcardMount() {
        Xlog.i(TAG, "testcase13SdcardMount");
        getActivityWithIntent(PKG_ID_OK);

        mPreference.edit().putInt(ACTIVITY_ID, PKG_ID_OK).commit();
        Util.mountSdCard(mInst, Util.getAvailablePath(mContext));
        mInst.waitForIdleSync();

        Assert.assertTrue(!mPreference.getBoolean(NEED_REFRESH_PACKAGE, true));
        Assert.assertEquals(PKG_ID_OK, mPreference.getInt(ACTIVITY_ID, 0));
        mPreference.edit().putInt(ACTIVITY_ID, -1).commit();
    }

    /*
     * Test upgrade success
     */
    public void atestcase14Upgrade() {
        Xlog.i(TAG, "testcase14Upgrade ");
        getActivityWithIntent(PKG_ID_OK);

        mInst.invokeMenuActionSync(mActivity, MENU_ID_UPGRADE, 0);
        mInst.waitForIdleSync();

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

    }

    private void getActivityWithIntent(int id) {
        mPreference.edit().putBoolean(NEED_REFRESH_PACKAGE, false).commit();
        UpdatePackageInfo info = getPackageInfo(id);
        Assert.assertNotNull("get package info fail", info);

        Intent intent = new Intent();
        intent.setClass(mInst.getTargetContext(), SdPkgInstallActivity.class);
        intent.putExtra(SdPkgInstallActivity.KEY_VERSION, info.version);
        intent.putExtra(SdPkgInstallActivity.KEY_PATH, info.path);
        intent.putExtra(SdPkgInstallActivity.KEY_ANDROID_NUM, info.androidNumber);
        intent.putExtra(SdPkgInstallActivity.KEY_NOTES, info.notes);
        this.setActivityIntent(intent);
        mActivity = getActivity();

        mInst.waitForIdleSync();
    }

    private void testPkg(int PkgId) {

        getActivityWithIntent(PkgId);

        mInst.invokeMenuActionSync(mActivity, MENU_ID_UPGRADE, 0);
        mInst.waitForIdleSync();

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_RIGHT);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();
        Assert.assertEquals("Start install", PkgId, mPreference.getInt(ACTIVITY_ID, -1));

        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Xlog.e(TAG, "sleep broken");
        }

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        sleepForActivityFinish();
        Assert.assertEquals("Finish install", -1, mPreference.getInt(ACTIVITY_ID, PkgId));
        Assert.assertEquals("Activity is finishing", true, mActivity.isFinishing());

    }

    private void waitForNewVersion() {
        int time = 0;
        while (mPreference.getBoolean(NEED_REFRESH_PACKAGE, true) && time < 200) {
            ++time;
            try {
                Thread.sleep(3000);
            } catch (InterruptedException e) {
                fail("Thread.sleep(3000)");
            }
        }
    }

    private UpdatePackageInfo getPackageInfo(int id) {
        UpdatePackageInfo info = null;
        try {
            Class cls = Class.forName("com.mediatek.systemupdate.PackageInfoReader");
            Constructor<?> con = cls.getConstructor(Context.class, String.class);
            Xlog.v(TAG, "[getPackageInfo],get class PackageInfoReader");
            con.setAccessible(true);
            Object reader = con.newInstance(mInst.getTargetContext(), Util.PathName.PKG_INFO_IN_DATA);
            Xlog.v(TAG, "[getPackageInfo],get PackageInfoReader's new instance");

            Method m = cls.getDeclaredMethod("getInfoList");
            Xlog.v(TAG, "[getPackageInfo],get PackageInfoReader's getInfoList method");
            m.setAccessible(true);
            List<UpdatePackageInfo> infoList = (List<UpdatePackageInfo>) m.invoke(reader, (Object[]) null);
            Xlog.v(TAG, "[getPackageInfo],getInfoList method invoked");
            // if (infoList.size() == PKG_NUM) {
            // Xlog.v(TAG, "[getPackageInfo], info list number correct");
            info = infoList.get(id);
            // }
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        return info;
    }

    private void invokeCheckUpgradePackage() {
        try {
            Class cls = SdPkgInstallActivity.class;
            Method m = cls.getDeclaredMethod("checkUpgradePackage");
            Xlog.v(TAG, "[invokeCheckUpgradePackage]get checkUpgradePackage method");
            m.setAccessible(true);
            m.invoke(mActivity, (Object[]) null);
            Xlog.v(TAG, "[invokeCheckUpgradePackage]checkUpgradePackage invoked");
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
            Class cls = SdPkgInstallActivity.class;
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

    private void sendOTAMessage(int result) {
        try {
            Class cls = SdPkgInstallActivity.class;
            Field f = cls.getDeclaredField("mOTAresult");
            Xlog.v(TAG, "[sendOTAmessage]get Field mOTAresult");
            f.setAccessible(true);
            f.setInt(mActivity, result);
            Xlog.v(TAG, "[sendOTAmessage]set OTA result : " + result);

            Method m = cls.getDeclaredMethod("sendCheckOTAMessage");
            Xlog.v(TAG, "[sendOTAmessage]get sendCheckOTAMessage method");
            m.setAccessible(true);
            m.invoke(mActivity, (Object[]) null);
            Xlog.v(TAG, "[sendOTAmessage]sendCheckOTAMessage invoked");
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    private void sendEmptyMessage(int what) {
        try {
            Class cls = SdPkgInstallActivity.class;
            Field f = cls.getDeclaredField("mHandler");
            Xlog.v(TAG, "get Field mHandler");
            f.setAccessible(true);
            Handler handler = (Handler) f.get(mActivity);
            handler.sendEmptyMessage(what);
            Xlog.v(TAG, "send sendEmptyMessage : " + what);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
    }

    private int getOtaMessageId() {
        int id = -1;
        try {
            Class cls = SdPkgInstallActivity.class;
            Field f = cls.getDeclaredField("mOTADialogMessageResId");
            Xlog.v(TAG, "[getOtaMessageId]get Field mOTADialogMessageResId");
            f.setAccessible(true);
            id = f.getInt(mActivity);
            Xlog.v(TAG, "[getOtaMessageId]" + mContext.getString(id));
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }
        return id;
    }

    private void sleepForActivityFinish() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Xlog.e(TAG, "sleep broken");
        }
    }
}
