package com.mediatek.systemupdate.otatest;

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
import android.widget.TextView;

import com.mediatek.systemupdate.R;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.systemupdate.UpdateOption;
import com.mediatek.xlog.Xlog;
import com.mediatek.systemupdate.otatest.OtaSetDataTests.DataValue;
import com.mediatek.systemupdate.otatest.OtaSetDataTests.KeySet;
import junit.framework.Assert;

public class OtaGetDataTests extends ActivityInstrumentationTestCase2<UpdateOption> {

    private final String TAG = "SystemUpdate/OtaGetDataTests";

    private static final String OTA_PREFERENCE = "googleota";
    private static final String OTA_PRE_STATUS = "downlaodStatus";
    private static final String OTA_UNZ_STATUS = "isunzip";
    private static final String OTA_REN_STATUS = "isrename";
    private static final String OTA_PRE_DOWNLOAND_PERCENT = "downloadpercent";
    private static final String EXTERNAL_USB_STORAGE = "usbotg";

    private static final int STATE_QUERYNEWVERSION = 0;
    private static final int STATE_NEWVERSION_READY = 1;

    /*
     * The testing activity
     */
    private UpdateOption mActivity;
    private Context mContext;

    /*
     * The intsrumenation
     */
    Instrumentation mInst;

    /*
     * Constructor
     */
    public OtaGetDataTests() {
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
        mContext = mInst.getTargetContext();

    }

    protected void tearDown() throws Exception {

        if (mActivity != null) {
            mActivity.finish();
            Xlog.i(TAG, "activity finish");

        }
        super.tearDown();
    }


    /**
     * Test files get from data/data/com.mediatek.systemupdate/, and compare the result
     */
    public void testcase01GetUpdateType() {
        // get Summay equals what we set
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_TYPES.OTA_UPDATE_ONLY,
                com.mediatek.systemupdate.Util.getUpdateType());
    }

    /**
     * Test SharedPreferences get from data/data/com.mediatek.systemupdate/, and compare the result
     */
    public void testcase02GetDownloadInfo() {
        Context context= mInst.getTargetContext();
        SharedPreferences preference = context.getSharedPreferences(KeySet.OTA_PREFERENCE,
                Context.MODE_WORLD_READABLE);

        Assert.assertEquals(DataValue.TEST_STATUS,
                preference.getInt(KeySet.OTA_PRE_STATUS, DataValue.TEST_STATUS - 1));
        Assert.assertEquals(DataValue.TEST_PERCENT,
                preference.getInt(KeySet.OTA_PRE_DOWNLOAND_PERCENT, DataValue.TEST_PERCENT - 1));

        Assert.assertEquals(DataValue.TEST_SIZE,
                preference.getLong(KeySet.OTA_PRE_IMAGE_SIZE, DataValue.TEST_SIZE - 1));
        Assert.assertEquals(DataValue.TEST_VERSION,
                preference.getString(KeySet.OTA_PRE_VER, DataValue.TEST_VERSION + "fail"));
        Assert.assertEquals(DataValue.TEST_NOTE,
                preference.getString(KeySet.OTA_PRE_VER_NOTE, DataValue.TEST_NOTE + "fail"));
        Assert.assertEquals(DataValue.TEST_DELTA_ID,
                preference.getInt(KeySet.OTA_PRE_DELTA_ID, DataValue.TEST_DELTA_ID - 1));
        Assert.assertEquals(DataValue.TEST_UNZ,
                preference.getBoolean(KeySet.OTA_UNZ_STATUS, !DataValue.TEST_UNZ));
        Assert.assertEquals(DataValue.TEST_REN,
                preference.getBoolean(KeySet.OTA_REN_STATUS, !DataValue.TEST_REN));
        Assert.assertEquals(DataValue.TEST_QUERY_DATE,
                preference.getString(KeySet.OTA_QUERY_DATE, DataValue.TEST_QUERY_DATE + "fail"));

        Assert.assertEquals(DataValue.TEST_UPGRADE_STARTED,
                preference.getBoolean(KeySet.OTA_UPGRADE_STARTED, !DataValue.TEST_UPGRADE_STARTED));
        Assert.assertEquals(DataValue.TEST_FULL_PKG,
                preference.getBoolean(KeySet.OTA_FULL_PKG, !DataValue.TEST_FULL_PKG));
        Assert.assertEquals(DataValue.TEST_ANDR_VER,
                preference.getString(KeySet.OTA_ANDR_VER, DataValue.TEST_ANDR_VER + "fail"));
        Assert.assertEquals(DataValue.TEST_DOWNLOAD_ONLY,
                preference.getBoolean(KeySet.WIFI_DOWNLOAD_ONLY, !DataValue.TEST_DOWNLOAD_ONLY));
        Assert.assertEquals(DataValue.TEST_AUTO_DOWNLOADING, preference.getBoolean(
                KeySet.OTA_AUTO_DOWNLOADING, !DataValue.TEST_AUTO_DOWNLOADING));
        Assert.assertEquals(DataValue.TEST_PAUSE_WITHIN_TIME,
                preference.getBoolean(KeySet.PAUSE_WITHIN_TIME, !DataValue.TEST_PAUSE_WITHIN_TIME));
        Assert.assertEquals(DataValue.TEST_REFRESH_STATUS,
                preference.getBoolean(KeySet.NEED_REFRESH_PACKAGE, !DataValue.TEST_REFRESH_STATUS));
        Assert.assertEquals(DataValue.TEST_MENU_STATUS,
                preference.getBoolean(KeySet.NEED_REFRESH_MENU, !DataValue.TEST_MENU_STATUS));
        Assert.assertEquals(DataValue.TEST_SHUT_STATUS,
                preference.getBoolean(KeySet.IS_SHUTTING_DOWN, !DataValue.TEST_SHUT_STATUS));
        Assert.assertEquals(DataValue.TEST_ACTIVITY_ID,
                preference.getInt(KeySet.ACTIVITY_ID, DataValue.TEST_ACTIVITY_ID - 1));
        // preference.edit().clear().commit();
    }
}
