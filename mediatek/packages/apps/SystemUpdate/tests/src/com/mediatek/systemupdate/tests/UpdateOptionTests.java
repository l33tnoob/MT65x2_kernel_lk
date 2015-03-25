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
import android.widget.TextView;

import com.mediatek.systemupdate.R;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.systemupdate.UpdateOption;
import com.mediatek.xlog.Xlog;

//import com.jayway.android.robotium.solo.Solo;

import junit.framework.Assert;

public class UpdateOptionTests extends ActivityInstrumentationTestCase2<UpdateOption> {

    private final String TAG = "SystemUpdate/EMTest";

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
    public UpdateOptionTests() {
        super("com.mediatek.systemupdate", UpdateOption.class);

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

    public void testcase01_Preconditions() {
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_OPTION, com.mediatek.systemupdate.Util.getUpdateType());
    }

    public void testcase02_setOptionOta() {

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        
//        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        mInst.waitForIdleSync();
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_TYPES.OTA_UPDATE_ONLY,
                com.mediatek.systemupdate.Util.getUpdateType());
    }

    public void testcase03_setOptionSd() {

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        
//        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        mInst.waitForIdleSync();
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_TYPES.SDCARD_UPDATE_ONLY,
                com.mediatek.systemupdate.Util.getUpdateType());
    }

    public void testcase04_setOptionOtaSd() {

        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInst.waitForIdleSync();

        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        mInst.sendCharacterSync( KeyEvent.KEYCODE_DPAD_UP );
        
//        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInst.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);

        mInst.waitForIdleSync();
        Assert.assertEquals(com.mediatek.systemupdate.Util.UPDATE_TYPES.OTA_SDCARD_UPDATE,
                com.mediatek.systemupdate.Util.getUpdateType());
    }

}
