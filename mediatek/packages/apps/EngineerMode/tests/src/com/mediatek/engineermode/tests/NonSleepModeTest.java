package com.mediatek.engineermode.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.EditText;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.nonsleep.NonSleepMode;
import com.mediatek.engineermode.touchscreen.TsMultiTouch;

public class NonSleepModeTest extends ActivityInstrumentationTestCase2<NonSleepMode> {

    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;

    public NonSleepModeTest() {
        super("com.mediatek.engineermode", NonSleepMode.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
    }

    public void testCase01_VerifyPreconditions() {
        verifyPreconditions();
    }

    public void testCase02_CheckBtn() {
        Activity testActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not NonSleepMode Class", NonSleepMode.class);
        
        // enable non-sleep mode
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_LONG);
        
        // release non-sleep mode
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_LONG);
        
        mSolo.goBack();
    }

}
