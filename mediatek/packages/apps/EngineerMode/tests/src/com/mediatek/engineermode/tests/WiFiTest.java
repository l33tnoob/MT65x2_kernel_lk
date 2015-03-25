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
import android.net.wifi.WifiManager;
import android.os.SystemProperties;
import android.test.SingleLaunchActivityTestCase;
import android.test.UiThreadTest;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.wifi.ChannelInfo;
import com.mediatek.engineermode.wifi.WiFi;
import com.mediatek.engineermode.wifi.WiFiEeprom;
import com.mediatek.engineermode.wifi.WiFiMcr;
import com.mediatek.engineermode.wifi.WiFiRx6620;
import com.mediatek.engineermode.wifi.WiFiTx6620;

public class WiFiTest extends SingleLaunchActivityTestCase<WiFi> {

    private static final String TAG = "EMTest/wifi";
    private static final int WIFI_ITEM_COUNT = 5;
    private static final int SLEEP_TIME = 1000;
    private static final String KEY_PROP_WPAWPA2 = "persist.radio.wifi.wpa2wpaalone";
    private static final String KEY_PROP_CTIA = "mediatek.wlan.ctia";
//    private static final String KEY_PROP_OPEN_AP_WPS = "mediatek.wlan.openap.wps";
    private static final String KEY_PROP_WPS2_SUPPORT = "persist.radio.wifi.wps2support";
    private static final String VALUE_TRUE = "true";
    private static final String VALUE_FALSE = "false";
    private static final String VALUE_0 = "0";
    private static final String VALUE_1 = "1";
    private static final int CHANNEL_NUMBER = 13;
    private static final int MODE_NUMBER = 4;
    private static final int CCK_RATE_NUMBER = 4;
    private static boolean sInit = false;
    private static boolean sSuccess = false;
    private static Solo sSolo = null;
    private static Activity sActivity = null;
    private static Context sContext = null;
    private static Instrumentation sInst = null;
    private static ListView sListView = null;
    private static WifiManager sWifiManager = null;
    private static boolean sFinished = false;

    public WiFiTest() {
        super("com.mediatek.engineermode", WiFi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (null == sInst) {
            sInst = getInstrumentation();
        }
        if (null == sContext) {
            sContext = sInst.getTargetContext();
        }
        if (null == sActivity) {
            sActivity = getActivity();
            if (sActivity.getClass() != WiFi.class) {
                sActivity.finish();
                sActivity = launchActivity("com.mediatek.engineermode",
                        WiFi.class, null);
            }
        }
        if (null == sSolo) {
            sSolo = new Solo(sInst, sActivity);
        }
        if (!sInit) {
            sWifiManager = (WifiManager) sContext
                    .getSystemService(Context.WIFI_SERVICE);
            assertNotNull(sWifiManager);
            int i = 0;
            int state = sWifiManager.getWifiState();
            if (WifiManager.WIFI_STATE_DISABLED != state
                    && WifiManager.WIFI_STATE_ENABLING != state) {
                sSuccess = false;
            } else {
                i = 0;
                while (WifiManager.WIFI_STATE_ENABLED != sWifiManager
                        .getWifiState()) {
                    if (i > 10) {
                        break;
                    }
                    EmOperate.waitSomeTime(EmOperate.TIME_LONG);
                    i++;
                }
                if (i > 10) {
                    sSuccess = false;
                } else {
                    sSuccess = true;
                }
            }
            sInit = true;
        }
    }

    @Override
    protected void tearDown() throws Exception {
        if (sFinished) {
            sSolo.finishOpenedActivities();
        }
        super.tearDown();
    }

    public void test01_Precondition() {
        assertNotNull(sInst);
        assertNotNull(sContext);
        assertNotNull(sActivity);
        assertNotNull(sSolo);
        if (null == sListView) {
            sListView = (ListView) sActivity.findViewById(R.id.ListView_WiFi);
        }
        assertNotNull(sListView);
        sSolo.sleep(EmOperate.TIME_MID);
    }

    public void test02_ErrorCheck() {
        if (!sSuccess) {
            sSolo.clickOnText(sActivity.getString(R.string.wifi_ok));
        }
    }

    public void test03_ItemCount() {
        if (sSuccess) {
            int expected = WIFI_ITEM_COUNT;
            int actual = sListView.getAdapter().getCount();
            sSolo.sleep(EmOperate.TIME_LONG);
            assertEquals(expected, actual);
        }
    }

    public void test04_CbWfa() {
        if (sSuccess) {
            CheckBox cbWfa = (CheckBox) sActivity
                    .findViewById(R.id.wifi_wfa_switcher);
            boolean actual = cbWfa.isChecked();
            boolean expect = VALUE_TRUE.equals(SystemProperties.get(
                    KEY_PROP_WPAWPA2, VALUE_FALSE));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(0);
            EmOperate.waitSomeTime(EmOperate.TIME_MID);
            actual = cbWfa.isChecked();
            expect = VALUE_TRUE.equals(SystemProperties.get(KEY_PROP_WPAWPA2,
                    VALUE_FALSE));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(0);
            EmOperate.waitSomeTime(EmOperate.TIME_SHORT);
        }
    }

    public void test05_CbCtia() {
        if (sSuccess) {
            CheckBox cbCtia = (CheckBox) sActivity
                    .findViewById(R.id.wifi_ctia_switcher);
            boolean actual = cbCtia.isChecked();
            boolean expect = VALUE_1.equals(SystemProperties.get(KEY_PROP_CTIA,
                    VALUE_0));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(1);
            EmOperate.waitSomeTime(EmOperate.TIME_MID);
            actual = cbCtia.isChecked();
            expect = VALUE_1.equals(SystemProperties
                    .get(KEY_PROP_CTIA, VALUE_0));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(1);
            EmOperate.waitSomeTime(EmOperate.TIME_SHORT);
        }
    }

    public void test06_CbWps() {
        if (sSuccess) {
            CheckBox cbWps = (CheckBox) sActivity
                    .findViewById(R.id.wifi_ap_wps2_support);
            boolean actual = cbWps.isChecked();
            boolean expect = VALUE_TRUE.equals(SystemProperties.get(
                    KEY_PROP_WPS2_SUPPORT, VALUE_TRUE));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(2);
            EmOperate.waitSomeTime(EmOperate.TIME_MID);
            actual = cbWps.isChecked();
            expect = VALUE_TRUE.equals(SystemProperties.get(
                    KEY_PROP_WPS2_SUPPORT, VALUE_TRUE));
            assertEquals(expect, actual);
            sSolo.clickOnCheckBox(2);
            EmOperate.waitSomeTime(EmOperate.TIME_SHORT);
        }
    }

    public void test07_ChipVersion() {
        if (sSuccess) {
            TextView version = (TextView) sActivity
                    .findViewById(R.id.wifi_version);
            boolean actual = version.getText().toString().contains("VERSION");
            assertEquals(true, actual);
        }
    }

    public void test08_TxChannel() {
        if (sSuccess) {
            int channelDefaultNum = 0;
            sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
            sSolo.waitForActivity(WiFiTx6620.class.getSimpleName());
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Tx activity", WiFiTx6620.class);
            sSolo.goBack();
            Spinner mWifiChannel = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Channel_Spinner);
            Spinner mWifiBandwidth = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Bandwidth_Spinner);
            assertNotNull(mWifiChannel);
            assertNotNull(mWifiBandwidth);
            //assertEquals(CHANNEL_NUMBER, mWifiChannel.getAdapter().getCount());
            channelDefaultNum = mWifiChannel.getAdapter().getCount();
            assertEquals("Channel 1 [2412MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.pressSpinnerItem(4, 1);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(channelDefaultNum - 4, mWifiChannel.getAdapter()
                    .getCount());
            assertEquals("Channel 3 [2422MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.pressSpinnerItem(4, -1);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(channelDefaultNum, mWifiChannel.getAdapter().getCount());
            assertEquals("Channel 1 [2412MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.goBack();
        }
    }

    public void test09_TxMode() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
            sSolo.waitForActivity(WiFiTx6620.class.getSimpleName());
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Tx activity", WiFiTx6620.class);
            sSolo.goBack();
            Spinner mWifiMode = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Mode_Spinner);
            Spinner mWifiRate = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Rate_Spinner);
            assertNotNull(mWifiMode);
            assertNotNull(mWifiRate);
            assertEquals(MODE_NUMBER, mWifiMode.getAdapter().getCount());
            sSolo.pressSpinnerItem(1, -CCK_RATE_NUMBER);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(MODE_NUMBER + 1, mWifiMode.getAdapter().getCount());
            sSolo.pressSpinnerItem(1, CCK_RATE_NUMBER);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(MODE_NUMBER, mWifiMode.getAdapter().getCount());
            sSolo.goBack();
        }
    }

    public void test10_TxPreamble() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
            sSolo.waitForActivity(WiFiTx6620.class.getSimpleName());
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Tx activity", WiFiTx6620.class);
            sSolo.goBack();
            Spinner mWifiPreamble = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Preamble_Spinner);
            assertNotNull(mWifiPreamble);
            assertEquals("Normal", mWifiPreamble.getAdapter().getItem(0));
            sSolo.pressSpinnerItem(1, 12);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals("802.11n mixed mode", mWifiPreamble.getAdapter()
                    .getItem(0));
            sSolo.pressSpinnerItem(1, -12);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals("Normal", mWifiPreamble.getAdapter().getItem(0));
            sSolo.goBack();
        }
    }

    public void test11_TxGo() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
            sSolo.waitForActivity(WiFiTx6620.class.getSimpleName());
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Tx activity", WiFiTx6620.class);
            sSolo.goBack();
            Spinner mWifiRate = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Rate_Spinner);
            assertNotNull(mWifiRate);
            sSolo.pressSpinnerItem(1, 11);
            //assertEquals("MCS3", mWifiRate.getSelectedItem().toString());
            Button btnGo = (Button) testActivity.findViewById(R.id.WiFi_Go);
            Button btnStop = (Button) testActivity.findViewById(R.id.WiFi_Stop);
            assertNotNull(btnGo);
            assertNotNull(btnStop);
            assertTrue(btnGo.isEnabled());
            assertFalse(btnStop.isEnabled());
            sSolo.clickOnButton(0);
            sSolo.sleep(EmOperate.TIME_SUPER_LONG);
            assertTrue(btnGo.isEnabled());
            assertFalse(btnStop.isEnabled());
            sSolo.pressSpinnerItem(1, -11);
            sSolo.clickOnButton(0);
            sSolo.sleep(EmOperate.TIME_MID);
            assertFalse(btnGo.isEnabled());
            assertTrue(btnStop.isEnabled());
            sSolo.clickOnButton(1);
            sSolo.sleep(EmOperate.TIME_MID);
            assertTrue(btnGo.isEnabled());
            assertFalse(btnStop.isEnabled());
            sSolo.goBack();
        }
    }

    public void test12_RxChannel() {
        if (sSuccess) {
            int channelDefaultNum = 0;
            sSolo.clickOnText(sListView.getAdapter().getItem(1).toString());
            sSolo.assertCurrentActivity("Enter WiFi Rx test fail",
                    WiFiRx6620.class);
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Rx activity", WiFiRx6620.class);
            Spinner mWifiChannel = (Spinner) testActivity
                    .findViewById(R.id.WiFi_RX_Channel_Spinner);
            Spinner mWifiBandwidth = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Bandwidth_Spinner);
            assertNotNull(mWifiChannel);
            assertNotNull(mWifiBandwidth);
            //assertEquals(CHANNEL_NUMBER, mWifiChannel.getAdapter().getCount());
            channelDefaultNum = mWifiChannel.getAdapter().getCount();
            assertEquals("Channel 1 [2412MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.pressSpinnerItem(1, 1);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(channelDefaultNum - 4, mWifiChannel.getAdapter()
                    .getCount());
            assertEquals("Channel 3 [2422MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.pressSpinnerItem(1, -1);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertEquals(channelDefaultNum, mWifiChannel.getAdapter().getCount());
            assertEquals("Channel 1 [2412MHz]", mWifiChannel.getSelectedItem()
                    .toString());
            sSolo.goBack();
        }
    }

    public void test13_RxTest() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(1).toString());
            sSolo.assertCurrentActivity("Enter WiFi Rx test fail",
                    WiFiRx6620.class);
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not Rx activity", WiFiRx6620.class);
            Spinner mWifiChannel = (Spinner) testActivity
                    .findViewById(R.id.WiFi_RX_Channel_Spinner);
            Spinner mWifiBandwidth = (Spinner) testActivity
                    .findViewById(R.id.WiFi_Bandwidth_Spinner);
            assertNotNull(mWifiChannel);
            assertNotNull(mWifiBandwidth);
            Button btnGo = (Button) testActivity.findViewById(R.id.WiFi_Go_Rx);
            Button btnStop = (Button) testActivity
                    .findViewById(R.id.WiFi_Stop_Rx);
            assertNotNull(btnGo);
            assertNotNull(btnStop);
            assertTrue(btnGo.isEnabled());
            assertFalse(btnStop.isEnabled());
            sSolo.clickOnButton(0);
            sSolo.sleep(EmOperate.TIME_LONG);
            assertFalse(btnGo.isEnabled());
            assertTrue(btnStop.isEnabled());
            TextView textView = (TextView) testActivity
                    .findViewById(R.id.WiFi_PER_Content);
            assertNotNull(textView);
            float percent = Float.valueOf(textView.getText().toString())
                    .floatValue();
            assertTrue(percent >= 0);
            assertTrue(percent <= 100);
            sSolo.sleep(EmOperate.TIME_LONG);
            sSolo.clickOnButton(1);
            sSolo.sleep(EmOperate.TIME_SHORT);
            assertTrue(btnGo.isEnabled());
            assertFalse(btnStop.isEnabled());
            sSolo.goBack();
        }
    }

    public void test14_McrTest() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(2).toString());
            sSolo.assertCurrentActivity("Enter WiFi MCR test fail",
                    WiFiMcr.class);
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo.assertCurrentActivity("Not MCR activity", WiFiMcr.class);
            Button btnRead = (Button) testActivity
                    .findViewById(R.id.WiFi_MCR_ReadBtn);
            Button btnWrite = (Button) testActivity
                    .findViewById(R.id.WiFi_MCR_WriteBtn);
            assertNotNull(btnRead);
            assertNotNull(btnWrite);
            assertEquals(0, sSolo.getEditText(1).length());
            sSolo.enterText(0, "0");
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.clickOnButton(0);
            sSolo.sleep(EmOperate.TIME_MID);
            assertEquals(8, sSolo.getEditText(1).length());
            sSolo.clickOnButton(1);
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.goBack();
        }
    }

    public void test15_NvramAccessWord() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(3).toString());
            sSolo.assertCurrentActivity("Enter WiFi NVRAM test fail",
                    WiFiEeprom.class);
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo
                    .assertCurrentActivity("Not EEPROM activity",
                            WiFiEeprom.class);
            sSolo.goBack();
            Button btnRead = (Button) testActivity
                    .findViewById(R.id.WiFi_Read_Word);
            Button btnWrite = (Button) testActivity
                    .findViewById(R.id.WiFi_Write_Word);
            assertNotNull(btnRead);
            assertNotNull(btnWrite);
            assertEquals(0, sSolo.getEditText(1).length());
            sSolo.enterText(0, "0");
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.clickOnButton(0);
            sSolo.sleep(EmOperate.TIME_MID);
            assertTrue(sSolo.getEditText(1).length() > 0);
            sSolo.clickOnButton(1);
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.goBack();
        }
    }

    public void test16_NvramAccessString() {
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(3).toString());
            sSolo.assertCurrentActivity("Enter WiFi NVRAM test fail",
                    WiFiEeprom.class);
            Activity testActivity = sSolo.getCurrentActivity();
            sSolo
                    .assertCurrentActivity("Not EEPROM activity",
                            WiFiEeprom.class);
            sSolo.goBack();
            Button btnRead = (Button) testActivity
                    .findViewById(R.id.WiFi_Read_String);
            Button btnWrite = (Button) testActivity
                    .findViewById(R.id.WiFi_Write_String);
            assertNotNull(btnRead);
            assertNotNull(btnWrite);
            assertEquals(0, sSolo.getEditText(4).length());
            sSolo.enterText(2, "0");
            sSolo.enterText(3, "1");
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.clickOnButton(2);
            sSolo.sleep(EmOperate.TIME_MID);
            assertTrue(sSolo.getEditText(4).length() > 0);
            sSolo.clickOnButton(3);
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.goBack();
        }
    }
    
    public void test17_Help() {
        sFinished = true;
        if (sSuccess) {
            sSolo.clickOnText(sListView.getAdapter().getItem(4).toString());
            sSolo.sleep(EmOperate.TIME_MID);
            sSolo.goBack();
        }
    }
}
