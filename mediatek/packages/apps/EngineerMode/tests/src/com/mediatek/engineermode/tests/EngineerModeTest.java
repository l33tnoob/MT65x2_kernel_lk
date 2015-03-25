/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.EngineerMode;

public class EngineerModeTest extends
        ActivityInstrumentationTestCase2<EngineerMode> {

    private static final String TAG = "EMTest/engineermode";
    private Solo mSolo = null;
    private Activity mActivity = null;
    private Context mContext = null;
    private Instrumentation mInst = null;

    public EngineerModeTest() {
        super(EngineerMode.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
    }

    @Override
    protected void tearDown() throws Exception {
        mSolo.finishOpenedActivities();
        super.tearDown();
    }

    public void test01_Precondition() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
        mSolo.sleep(EmOperate.TIME_SHORT);
    }

    public void test02_ItemCount() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_LEFT);
        mSolo.sleep(EmOperate.TIME_SHORT);
    }
/*
    public void test03_ItemMemory() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.memory), Memory.class
                .getSimpleName());
    }

    public void test04_ItemWifi() {
        WifiManager wifiManager = (WifiManager) mActivity
                .getSystemService(Context.WIFI_SERVICE);
        assertNotNull(wifiManager);
        if (WifiManager.WIFI_STATE_DISABLED != wifiManager.getWifiState()) {
            return;
        }
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.clickOnText(mActivity.getString(R.string.wifi));
        mSolo.waitForActivity(WiFi.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
    }

    public void test05_ItemTouchScreen() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.scrollDownList(1);
        enterItem(mActivity.getString(R.string.touchscreen), TouchScreenList.class
                .getSimpleName());
    }

    public void test06_ItemAudio() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.audio), Audio.class
                .getSimpleName());
    }

    public void test7_ItemCamera() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.camera), Camera.class
                .getSimpleName());
    }

    public void test8_ItemNetworkInfo() {
        enterItem(mActivity.getString(R.string.network_info), NetworkInfo.class
                .getSimpleName());
    }

    public void test9_ItemBluetooth() {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (null == adapter || BluetoothAdapter.STATE_OFF != adapter.getState()) {
            return;
        }
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.clickOnText(mActivity.getString(R.string.Bluetooth));
        mSolo.waitForActivity(BtList.class.getSimpleName(),
                EmOperate.TIME_SUPER_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);
        EmOperate.backKey(mInst);
    }

    public void test10_ItemDisplay() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.display), Display.class
                .getSimpleName());
        EmOperate.backKey(mInst);
    }

    public void test11_ItemIo() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.io), IoList.class.getSimpleName());
    }

    public void test12_ItemPower() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.scrollDownList(1);
        enterItem(mActivity.getString(R.string.power), Power.class
                .getSimpleName());
    }

    public void test13_ItemSyslogger() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.system_logger), SysLogger.class
                .getSimpleName());
    }

    public void test14_ItemCmmb() {
        enterItem(mActivity.getString(R.string.cmmb), CmmbActivity.class
                .getSimpleName());
    }

    public void test15_ItemCpuStressTest() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.cpu_stress_test),
                CpuStressTest.class.getSimpleName());
    }

    public void test16_ItemUsb() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.scrollDownList(1);
        enterItem(mActivity.getString(R.string.usb), UsbList.class
                .getSimpleName());
    }

    public void test17_ItemDevMgr() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.device_manager), DeviceMgr.class
                .getSimpleName());
    }

    public void test18_ItemLog2Server() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.log2server), DialogSwitch.class
                .getSimpleName());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        EmOperate.backKey(mInst);
        mSolo.sleep(EmOperate.TIME_MID);
        enterItem(mActivity.getString(R.string.tag_log), TagLogSwitch.class
                .getSimpleName());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test19_ItemTagLog() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.tag_log), TagLogSwitch.class
                .getSimpleName());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test20_ItemSdcardTest() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.scrollDownList(1);
        enterItem(mActivity.getString(R.string.sd_card_test),
                SDLogActivity.class.getSimpleName());
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
        mSolo.clickOnButton(0);
        mSolo.sleep(EmOperate.TIME_MID);
    }

    public void test21_ItemSettingsFont() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.settings_font),
                SettingsFontSize.class.getSimpleName());
    }

    public void test22_ItemDebugUtils() {
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        mSolo.sendKey(KeyEvent.KEYCODE_DPAD_RIGHT);
        enterItem(mActivity.getString(R.string.debug_utils), NetworkInfo.class
                .getSimpleName());
    }
*/
    private void enterItem(String item, String activityName) {
        mSolo.clickOnText(item);
        mSolo.waitForActivity(activityName, EmOperate.TIME_LONG);
        EmOperate.waitSomeTime(EmOperate.TIME_LONG);
    }

}
