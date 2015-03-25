package com.mediatek.systemupdate.otatest;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.systemupdate.R;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.systemupdate.UpdateOption;
import com.mediatek.xlog.Xlog;

import junit.framework.Assert;

public class OtaSetDataTests extends ActivityInstrumentationTestCase2<UpdateOption> {

    private final String TAG = "SystemUpdate/OtaSetDataTests";

    class KeySet {
        static final String OTA_PREFERENCE = "googleota";
        static final String OTA_PRE_STATUS = "downlaodStatus";
        static final String OTA_PRE_DOWNLOAND_PERCENT = "downloadpercent";
        static final String OTA_PRE_IMAGE_SIZE = "imageSize";
        static final String OTA_PRE_VER = "version";
        static final String OTA_PRE_VER_NOTE = "versionNote";
        static final String OTA_PRE_DELTA_ID = "versionDeltaId";
        static final String OTA_UNZ_STATUS = "isunzip";
        static final String OTA_REN_STATUS = "isrename";
        static final String OTA_QUERY_DATE = "query_date";
        static final String OTA_UPGRADE_STARTED = "upgrade_started";
        static final String OTA_FULL_PKG = "is_full_pkg";
        static final String OTA_ANDR_VER = "android_num";
        static final String WIFI_DOWNLOAD_ONLY = "wifi_download_only";
        static final String OTA_AUTO_DOWNLOADING = "ota_auto_downloading";
        static final String PAUSE_WITHIN_TIME = "pause_whthin_time";
        static final String NEED_REFRESH_PACKAGE = "need_refresh_package";
        static final String NEED_REFRESH_MENU = "need_refresh_menu";
        static final String IS_SHUTTING_DOWN = "is_shutting_down";
        static final String ACTIVITY_ID = "activity_id";
    }

    class DataValue {
        static final int TEST_STATUS = 0;
        static final int TEST_PERCENT = 0;
        static final long TEST_SIZE = 100L;
        static final String TEST_VERSION = "ALPS.JB2.Test.p0";
        static final String TEST_NOTE = "It's only a test";
        static final int TEST_DELTA_ID = 20;
        static final boolean TEST_UNZ = true;
        static final boolean TEST_REN = true;
        static final String TEST_QUERY_DATE = "2013-03-11";
        static final boolean TEST_UPGRADE_STARTED = true;
        static final boolean TEST_FULL_PKG = true;
        static final String TEST_ANDR_VER = "Android 4.2.1";
        static final boolean TEST_DOWNLOAD_ONLY = true;
        static final boolean TEST_AUTO_DOWNLOADING = false;
        static final boolean TEST_PAUSE_WITHIN_TIME = true;
        static final boolean TEST_REFRESH_STATUS = true;
        static final boolean TEST_MENU_STATUS = true;
        static final boolean TEST_SHUT_STATUS = true;
        static final int TEST_ACTIVITY_ID = 0;
    }

    private static final int STATE_QUERYNEWVERSION = 0;
    private static final int STATE_NEWVERSION_READY = 1;

    /*
     * The testing activity
     */
    private UpdateOption mActivity;
//    private Context mContext;

    /*
     * The intsrumenation
     */
    Instrumentation mInst;

    /*
     * Constructor
     */
    public OtaSetDataTests() {
        super(UpdateOption.class);

    }

    /*
     * Sets up the test environment before each test.
     */
    @Override
    protected void setUp() throws Exception {

        super.setUp();

        setActivityInitialTouchMode(false);

        mActivity = getActivity();
        mInst = getInstrumentation();
//        mContext = mInst.getTargetContext();

    }

    protected void tearDown() throws Exception {

        if (mActivity != null) {
            mActivity.finish();
            Xlog.i(TAG, "activity finish");
        }
        super.tearDown();
    }

    /**
     * Test files set in data/data/com.mediatek.systemupdate/
     */
    public void testcase01SetOptionOta() {

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_UP);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        mInst.waitForIdleSync();
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_TYPES.OTA_UPDATE_ONLY,
                com.mediatek.systemupdate.Util.getUpdateType());
    }

    /**
     * Test SharedPreferences in data/data/com.mediatek.systemupdate/
     */
    public void testcase02SetDownloadInfo() {
        Context context= mInst.getTargetContext();
        SharedPreferences preference = context.getSharedPreferences(KeySet.OTA_PREFERENCE,
                Context.MODE_WORLD_READABLE);

        Editor editor = preference.edit();
        editor.putInt(KeySet.OTA_PRE_STATUS, DataValue.TEST_STATUS);
        editor.putInt(KeySet.OTA_PRE_DOWNLOAND_PERCENT, DataValue.TEST_PERCENT);

        editor.putLong(KeySet.OTA_PRE_IMAGE_SIZE, DataValue.TEST_SIZE);
        editor.putString(KeySet.OTA_PRE_VER, DataValue.TEST_VERSION);
        editor.putString(KeySet.OTA_PRE_VER_NOTE, DataValue.TEST_NOTE);
        editor.putInt(KeySet.OTA_PRE_DELTA_ID, DataValue.TEST_DELTA_ID);
        editor.putBoolean(KeySet.OTA_UNZ_STATUS, DataValue.TEST_UNZ);
        editor.putBoolean(KeySet.OTA_REN_STATUS, DataValue.TEST_REN);
        editor.putString(KeySet.OTA_QUERY_DATE, DataValue.TEST_QUERY_DATE);
        ;
        editor.putBoolean(KeySet.OTA_UPGRADE_STARTED, DataValue.TEST_UPGRADE_STARTED);
        editor.putBoolean(KeySet.OTA_FULL_PKG, DataValue.TEST_FULL_PKG);
        editor.putString(KeySet.OTA_ANDR_VER, DataValue.TEST_ANDR_VER);
        editor.putBoolean(KeySet.WIFI_DOWNLOAD_ONLY, DataValue.TEST_DOWNLOAD_ONLY);
        editor.putBoolean(KeySet.OTA_AUTO_DOWNLOADING, DataValue.TEST_AUTO_DOWNLOADING);
        editor.putBoolean(KeySet.PAUSE_WITHIN_TIME, DataValue.TEST_PAUSE_WITHIN_TIME);
        editor.putBoolean(KeySet.NEED_REFRESH_PACKAGE, DataValue.TEST_REFRESH_STATUS);
        editor.putBoolean(KeySet.NEED_REFRESH_MENU, DataValue.TEST_MENU_STATUS);
        editor.putBoolean(KeySet.IS_SHUTTING_DOWN, DataValue.TEST_SHUT_STATUS);
        editor.putInt(KeySet.ACTIVITY_ID, DataValue.TEST_ACTIVITY_ID);
        editor.commit();
    }
}
