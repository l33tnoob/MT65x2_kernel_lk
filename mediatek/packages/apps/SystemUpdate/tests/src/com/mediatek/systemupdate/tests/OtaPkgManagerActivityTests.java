package com.mediatek.systemupdate.tests;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.systemupdate.OtaPkgManagerActivity;
import com.mediatek.systemupdate.R;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;

public class OtaPkgManagerActivityTests extends ActivityInstrumentationTestCase2<OtaPkgManagerActivity> {

    private final String TAG = "SystemUpdate/OtaPkgMgrTest";

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

    private static final String ANDROID_VER_TEST = "Android 4.3";
    private static final String LOAD_VER_TEST = "ALPS.JB.P1";
    private static final int PKG_ID_TEST = 11288;

    private static final int STATE_QUERYNEWVERSION = 0;
    private static final int STATE_NEWVERSION_READY = 1;
    private static final int STATE_DOWNLOADING = 2;
    private static final int STATE_CANCELDOWNLOAD = 3;
    private static final int STATE_PAUSEDOWNLOAD = 4;
    private static final int STATE_DLPKGCOMPLETE = 5;
    private static final int STATE_PACKAGEERROR = 6;

    private static final int MENU_ID_DOWNLOAD = Menu.FIRST;
    private static final int MENU_ID_CANCEL = Menu.FIRST + 1;
    private static final int MENU_ID_UPGRADE = Menu.FIRST + 2;

    /*
     * The testing activity
     */
    private OtaPkgManagerActivity mActivity;

    private SharedPreferences mPreference = null;

    private Context mContext;

    /*
     * The intsrumenation
     */
    Instrumentation mInst;

    /*
     * Constructor
     */
    public OtaPkgManagerActivityTests() {
        super("com.mediatek.systemupdate", OtaPkgManagerActivity.class);

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

        }
        super.tearDown();
    }

    private void resetDownloadDesctiptor() {

        mPreference.edit().putInt(OTA_PRE_DOWNLOAND_PERCENT, -1).commit();
        mPreference.edit().putBoolean(OTA_UNZ_STATUS, false).commit();
        mPreference.edit().putBoolean(OTA_REN_STATUS, false).commit();
        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_QUERYNEWVERSION).commit();

    }

    /*
     * Test the download function when new version detected
     */
    public void testcase01_NewVersionLayout() {
        Xlog.i(TAG, "start1 ");
        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_NEWVERSION_READY).commit();
        mPreference.edit().putString(OTA_ANDR_VER, ANDROID_VER_TEST).commit();
        mPreference.edit().putString(OTA_PRE_VER, LOAD_VER_TEST).commit();
        mActivity = getActivity();

        TextView textAndroidVer = (TextView) mActivity.findViewById(R.id.textAndroidNum);

        assertTrue(textAndroidVer != null);

        Xlog.i(TAG, textAndroidVer.getText().toString());

        assertTrue(ANDROID_VER_TEST.equals(textAndroidVer.getText()));

        TextView textLoadVer = (TextView) mActivity.findViewById(R.id.textVerNum);

        assertTrue(textLoadVer != null);

    }

    public void testcase02_NewVersionDl() {

        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_NEWVERSION_READY).commit();
        mPreference.edit().putInt(OTA_PRE_DELTA_ID, PKG_ID_TEST).commit();
        mActivity = getActivity();

        mInst.invokeMenuActionSync(mActivity, MENU_ID_DOWNLOAD, 0);
        mInst.waitForIdleSync();

        Xlog.i(TAG, "OTA_PRE_STATUS = " + mPreference.getInt(OTA_PRE_STATUS, -1));
        if (Util.isSdcardAvailable(mActivity) && Util.isNetWorkAvailable(mActivity, "WIFI")) {
            assertTrue(mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_DOWNLOADING
                    || mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_PAUSEDOWNLOAD);

        }

    }

    public void testcase03_DownloadingPaused() {

        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_PAUSEDOWNLOAD).commit();
        mPreference.edit().putInt(OTA_PRE_DELTA_ID, PKG_ID_TEST).commit();
        mActivity = getActivity();

        ProgressBar dlRatioProgressBar = (ProgressBar) mActivity.findViewById(R.id.downloaingProBar);

        assertTrue(dlRatioProgressBar != null);
        assertTrue(dlRatioProgressBar.isShown());

        Xlog.i(TAG, "OTA_PRE_STATUS = " + mPreference.getInt(OTA_PRE_STATUS, -1));
        if (Util.isSdcardAvailable(mActivity) && Util.isNetWorkAvailable(mActivity, "WIFI")) {
            assertTrue(mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_PAUSEDOWNLOAD);
        }

    }

    public void testcase04_DownloadingCancel() {

        mPreference.edit().putInt(OTA_PRE_STATUS, STATE_DOWNLOADING).commit();
        mPreference.edit().putInt(OTA_PRE_DELTA_ID, PKG_ID_TEST).commit();

        mActivity = getActivity();

        ProgressBar dlRatioProgressBar = (ProgressBar) mActivity.findViewById(R.id.downloaingProBar);

        assertTrue(dlRatioProgressBar != null);
        assertTrue(dlRatioProgressBar.isShown());

        if (Util.isSdcardAvailable(mActivity) && Util.isNetWorkAvailable(mActivity, "WIFI")) {
            if (mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_PAUSEDOWNLOAD) {
                return;
            }
            mInst.invokeMenuActionSync(mActivity, MENU_ID_CANCEL, 0);
            mInst.waitForIdleSync();

            assertTrue(mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_PAUSEDOWNLOAD);
            sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
            sendKeys(KeyEvent.KEYCODE_DPAD_RIGHT);

            sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);

            mInst.waitForIdleSync();
            assertTrue(mPreference.getInt(OTA_PRE_STATUS, -1) == STATE_QUERYNEWVERSION);

        }

    }

    /*
     * Test the wrong package
     */
    /*
     * public void testcase05_UpgradeWithWrongPkg() {
     * 
     * mPreference.edit().putInt(OTA_PRE_STATUS, STATE_DLPKGCOMPLETE).commit();
     * 
     * createPkgFile(); File fPkg = new File(Util.getPackageFileName(mContext));
     * 
     * assertTrue(fPkg.exists());
     * 
     * mActivity = getActivity();
     * 
     * mInst.invokeMenuActionSync(mActivity, MENU_ID_UPGRADE, 0);
     * 
     * mInst.waitForIdleSync(); sendKeys(KeyEvent.KEYCODE_DPAD_DOWN);
     * sendKeys(KeyEvent.KEYCODE_DPAD_CENTER);
     * 
     * mInst.waitForIdleSync(); try { Thread.sleep(3000); } catch
     * (InterruptedException e) { fail("Thread.sleep(3000)"); }
     * assertTrue(!fPkg.exists());
     * 
     * }
     */

    private void createPkgFile() {

        File ifolder = new File(Util.getPackagePathName(mContext));
        if (!ifolder.exists()) {
            ifolder.mkdirs();
        }

        File fPkg = new File(Util.getPackageFileName(mContext));

        if (!fPkg.exists()) {

            try {
                fPkg.createNewFile();
            } catch (IOException ex) {
                ex.printStackTrace();
            }

        }

    }

}
