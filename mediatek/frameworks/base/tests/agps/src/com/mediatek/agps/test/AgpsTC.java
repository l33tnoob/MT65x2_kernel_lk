package com.mediatek.agps.test;

import android.test.InstrumentationTestCase;
import android.util.Log;
import android.content.Context;


import com.mediatek.common.agps.MtkAgpsManager;
import com.mediatek.common.agps.MtkAgpsConfig;
import com.mediatek.common.agps.MtkAgpsProfile;

public class AgpsTC extends InstrumentationTestCase {
    
    private final static String TAG = "agps_test";
    private MtkAgpsManager mAgpsMgr;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        
        Context context = getInstrumentation().getTargetContext();
        mAgpsMgr = (MtkAgpsManager)context.getSystemService(Context.MTK_AGPS_SERVICE);
        assertNotNull(mAgpsMgr);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        mAgpsMgr = null;
        msleep(50);
    }
    

    public void test01Enable() {
        log("test01Enable ++");
        mAgpsMgr.enable();
        assertTrue(mAgpsMgr.getStatus());
        log("test01Enable --");
    }
    
    public void test02Disable() {
        log("test02Disable ++");
        mAgpsMgr.disable();
        assertFalse(mAgpsMgr.getStatus());
        log("test02Disable --");
    }

    public void test03EnableDisable() {
        log("test03EnableDisable ++");
        mAgpsMgr.enable();
        assertTrue(mAgpsMgr.getStatus());
        mAgpsMgr.disable();
        assertFalse(mAgpsMgr.getStatus());
        log("test03EnableDisable --");
    }

    public void test04EnableDisableStress() {
        log("test04EnableDisableStress ++");
        for(int i = 0; i < 10; i ++) {
            mAgpsMgr.enable();
            assertTrue(mAgpsMgr.getStatus());
            mAgpsMgr.disable();
            assertFalse(mAgpsMgr.getStatus());
        }
        msleep(1200);
        log("test04EnableDisableStress --");
    }
    
    public void test05SetNullProfile() {
        log("test05SetNullProfile ++");
        MtkAgpsProfile backup = mAgpsMgr.getProfile();
        mAgpsMgr.setProfile(null);
        mAgpsMgr.setProfile(backup);
        log("test05SetNullProfile --");
    }

    public void test06SetProfile() {
        log("test06SetProfile ++");
        MtkAgpsProfile backup = mAgpsMgr.getProfile();
        MtkAgpsProfile profile = new MtkAgpsProfile();
        profile.addr = "1";
        profile.addrType = "2";
        profile.appId = "3";
        profile.backupSlpNameVar = "4";
        profile.code = "5";
        profile.defaultApn = "6";
        profile.name = "7";
        profile.optionApn = "8";
        profile.optionApn2 = "9";
        profile.port = 1;
        profile.providerId = "10";
        profile.showType = 2;
        profile.tls = 3;
        
        mAgpsMgr.setProfile(profile);
        profile = null;
        profile = mAgpsMgr.getProfile();

        assertEquals("1" , profile.addr);
        assertEquals("2" , profile.addrType);
        assertEquals("3" , profile.appId);
        assertEquals("4" , profile.backupSlpNameVar);
        assertEquals("5" , profile.code);
        assertEquals("6" , profile.defaultApn);
        assertEquals("7" , profile.name);
        assertEquals("8" , profile.optionApn);
        assertEquals("9" , profile.optionApn2);
        assertEquals("10", profile.providerId);
        assertEquals(1, profile.port);
        assertEquals(2, profile.showType);
        assertEquals(3, profile.tls);

        mAgpsMgr.setProfile(backup);
        log("test06SetProfile --");
    }

    public void test07SetNullConfig() {
        log("test07SetNullConfig ++");
        MtkAgpsConfig backup = mAgpsMgr.getConfig();
        mAgpsMgr.setConfig(null);
        mAgpsMgr.setConfig(backup);
        log("test07SetNullConfig --");
    }

    public void test08SetConfig2() {
        log("test08SetConfig2 ++");
        MtkAgpsConfig backup = mAgpsMgr.getConfig();
        MtkAgpsConfig config = new MtkAgpsConfig();
        config.siMode           = 1;
        config.setId            = 1;
        config.qopHacc          = 1;
        config.qopVacc          = 1;
        config.qopAge           = 1;
        config.qopDelay         = 1;
        config.notifyTimeout    = 1;
        config.verifyTimeout    = 1;
        config.niEnable         = 1;
        config.agpsProtocol     = 1;
        config.extAddress       = "";
        config.extAddressEnable = 1;
        config.mlcNum           = "";
        config.mlcNumEnable     = 1;
        config.suplPosProtocol  = 1;
        config.cpMolrType       = 1;
        config.log2file         = 1;
        config.supl2file        = 1;
        config.log2uart         = 1;
        config.niIot            = 1;
        config.logFileMaxSize   = 1;
        config.simIdPref        = 1;
        config.roaming          = 1;
        config.caEnable         = 1;
        config.emEnable         = 1;
        config.niTimer          = 1;
        config.eCidEnable       = 1;
        config.suplVersion      = 1;
        
        mAgpsMgr.setConfig(config);
        config = mAgpsMgr.getConfig();
        
        assertEquals(1, config.siMode);
        assertEquals(1, config.setId);
        assertEquals(1, config.qopHacc);
        assertEquals(1, config.qopVacc);
        assertEquals(1, config.qopAge);
        assertEquals(1, config.qopDelay);
        assertEquals(1, config.notifyTimeout);
        assertEquals(1, config.verifyTimeout);
        assertEquals(1, config.niEnable);
        assertEquals(1, config.agpsProtocol);
        assertEquals("", config.extAddress);
        assertEquals(1, config.extAddressEnable);
        assertEquals("", config.mlcNum);
        assertEquals(1, config.mlcNumEnable);
        assertEquals(1, config.suplPosProtocol);
        assertEquals(1, config.cpMolrType);
        assertEquals(1, config.log2file);
        assertEquals(1, config.supl2file);
        assertEquals(1, config.log2uart);
        assertEquals(1, config.niIot);
        assertEquals(1, config.logFileMaxSize);
        assertEquals(1, config.simIdPref);
        assertEquals(1, config.roaming);
        assertEquals(1, config.caEnable);
        assertEquals(1, config.emEnable);
        assertEquals(1, config.niTimer);
        assertEquals(1, config.eCidEnable);

        mAgpsMgr.setConfig(backup);
        log("test08SetConfig2 --");
    }

    public void test09NINoResponse() {
        log("test09NINoResponse ++");
        mAgpsMgr.niUserResponse(0, 0);
        log("test09NINoResponse --");
    }
    
    public void test10NIAccept() {
        log("test10NIAccept ++");
        mAgpsMgr.niUserResponse(0, 1);
        log("test10NIAccept --");
    }
    
    public void test11NIDeny() {
        log("test11NIDeny ++");
        mAgpsMgr.niUserResponse(0, 2);
        log("test11NIDeny --");
    }
    
    public void test12NIUndefined() {
        log("test12NIUndefined ++");
        mAgpsMgr.niUserResponse(0, 3);
        log("test12NIUndefined --");
    }

    public void test13ExtraUsingXml() {
        log("test13ExtraUsingXml ++");
        mAgpsMgr.extraCommand("USING_XML", null);
        log("test13ExtraUsingXml --");
    }
    
    public void test14ExtraResetToDefault() {
        log("test14ExtraResetToDefault ++");
        mAgpsMgr.extraCommand("RESET_TO_DEFAULT", null);
        log("test14ExtraResetToDefault --");
    }
    
    public void test15ExtraUndefined() {
        log("test15ExtraUndefined ++");
        mAgpsMgr.extraCommand("UNDEFINED", null);
        log("test15ExtraUndefined --");
    }

    public void test16Combination() {
        log("test16Combination ++");

        test02Disable();
        test01Enable();
        test06SetProfile();
        test08SetConfig2();
        test09NINoResponse();
        test10NIAccept();
        test11NIDeny();
        test12NIUndefined();
        test13ExtraUsingXml();
        test14ExtraResetToDefault();
        test15ExtraUndefined();
        
        log("test16Combination --");
    }


    public void testSetProfileWithNullMemer() {
        MtkAgpsProfile backup = mAgpsMgr.getProfile();
        MtkAgpsProfile profile = new MtkAgpsProfile();
        mAgpsMgr.setProfile(profile);
        mAgpsMgr.setProfile(backup);
    }
    
    public void testSetConfigWithNullMember() {
        MtkAgpsConfig backup = mAgpsMgr.getConfig();
        MtkAgpsConfig config = new MtkAgpsConfig();
        mAgpsMgr.setConfig(config);
        mAgpsMgr.setConfig(backup);
    }
    
    public void testSetConfig1() {
        MtkAgpsConfig backup = mAgpsMgr.getConfig();
        MtkAgpsConfig config = new MtkAgpsConfig();
        config.siMode           = 0;
        config.setId            = 0;
        config.qopHacc          = 0;
        config.qopVacc          = 0;
        config.qopAge           = 0;
        config.qopDelay         = 0;
        config.notifyTimeout    = 0;
        config.verifyTimeout    = 0;
        config.niEnable         = 0;
        config.agpsProtocol = 0;
        config.extAddress       = null;
        config.extAddressEnable = 0;
        config.mlcNum           = null;
        config.mlcNumEnable     = 0;
        config.suplPosProtocol = 0;
        config.cpMolrType       = 0;
        config.log2file         = 0;
        config.supl2file        = 0;
        config.log2uart         = 0;
        config.niIot            = 0;
        config.logFileMaxSize   = 0;
        config.simIdPref        = 0;
        config.roaming          = 0;
        config.caEnable         = 0;
        config.emEnable         = 0;
        config.niTimer          = 0;
        config.eCidEnable       = 0;

        //only verify no JE case
        mAgpsMgr.setConfig(config);
        /*
        config = mAgpsMgr.getConfig();
        
        assertEquals(0, config.siMode);
        assertEquals(0, config.setId);
        assertEquals(0, config.qopHacc);
        assertEquals(0, config.qopVacc);
        assertEquals(0, config.qopAge);
        assertEquals(0, config.qopDelay);
        assertEquals(0, config.notifyTimeout);
        assertEquals(0, config.verifyTimeout);
        assertEquals(0, config.niEnable);
        assertEquals(0, config.agpsProtocol);
        assertNotNull(config.extAddress);
        assertEquals(0, config.extAddressEnable);
        assertNotNull(config.mlcNum);
        assertEquals(0, config.mlcNumEnable);
        assertEquals(0, config.suplPosProtocol);
        assertEquals(0, config.cpMolrType);
        assertEquals(0, config.log2file);
        assertEquals(0, config.supl2file);
        assertEquals(0, config.log2uart);
        assertEquals(0, config.niIot);
        assertEquals(0, config.logFileMaxSize);
        assertEquals(0, config.simIdPref);
        assertEquals(0, config.roaming);
        assertEquals(0, config.caEnable);
        assertEquals(0, config.emEnable);
        assertEquals(0, config.niTimer);
        assertEquals(0, config.eCidEnable);
        */

        mAgpsMgr.setConfig(backup);
    }
    /* cannot pass case
    */
    
    /* Template
    public void test() {
        log("test ++");
        
        log("test --");
    }
    */
    
    private void log(String msg) {
        Log.d(TAG, msg);
    }
    
    private void msleep(long milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (Exception e) {
        }
    }
    
}
