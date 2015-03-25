package com.mediatek.apst.target.tests;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;

public class SystemInfoProxyTest extends AndroidTestCase {
    private Context mContext;
    SystemInfoProxy mProxy;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mProxy = SystemInfoProxy.getInstance(mContext);

    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        super.tearDown();
    }

    public void test01_getDevice() {
        String device = SystemInfoProxy.getDevice();
        assertNotNull(device);
    }

    public void test02_getFirmwareVersion() {
        String firmwareVersion = SystemInfoProxy.getFirmwareVersion();
        assertNotNull(firmwareVersion);
    }

    public void test03_getManufacturer() {
        String manufacturer = SystemInfoProxy.getManufacturer();
        assertNotNull(manufacturer);
    }

    public void test04_getModel() {
        String model = SystemInfoProxy.getModel();
        assertNotNull(model);
    }

    public void test05_getSdPath() {
        String sdPath = SystemInfoProxy.getSdPath();
        assertNotNull(sdPath);
    }

    public void test06_getSdTotalSpace() {
        long sdTotalSpace = SystemInfoProxy.getSdTotalSpace();
        assertTrue(sdTotalSpace >= 0);
    }

    public void test07_getSdAvailableSpace() {
        long sdAvailableSpace = SystemInfoProxy.getSdAvailableSpace();
        assertTrue(sdAvailableSpace >= 0);
    }

    public void test08_getInternalStoragePath() {
        String internalStoragePath = SystemInfoProxy.getInternalStoragePath();
        assertNotNull(internalStoragePath);
    }

    public void test09_getInternalTotalSpace() {
        long InternalTotalSpace = SystemInfoProxy.getInternalTotalSpace();
        assertTrue(InternalTotalSpace >= 0);
    }

    public void test10_getInternalAvailableSpace() {
        long internalAvailableSpace = SystemInfoProxy
                .getInternalAvailableSpace();
        assertTrue(internalAvailableSpace >= 0);
    }

    public void test11_getSimState() {
        SystemInfoProxy.getSimState(0);
        SystemInfoProxy.getSimState(1);
    }

    public void test12_isSimAccessible() {
        boolean simAccessible;
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_READY);
        assertTrue(simAccessible);
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_ABSENT);
        assertTrue(!simAccessible);
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_PIN_REQUIRED);
        assertTrue(!simAccessible);
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_PUK_REQUIRED);
        assertTrue(!simAccessible);
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_NETWORK_LOCKED);
        assertTrue(!simAccessible);
        simAccessible = SystemInfoProxy
                .isSimAccessible(TelephonyManager.SIM_STATE_UNKNOWN);
        assertTrue(!simAccessible);
        simAccessible = SystemInfoProxy.isSimAccessible(255);
    }

    public void test13_isSimAccessible() {
        mProxy.isSimAccessible();
    }

    public void test14_isSim1Accessible() {
        SystemInfoProxy.isSim1Accessible();
    }

    public void test15_isSim2Accessible() {
        SystemInfoProxy.isSim2Accessible();
    }

    public void test16_isSdPresent() {
        SystemInfoProxy.isSdPresent();
    }

    public void test17_isSdMounted() {
        SystemInfoProxy.isSdMounted();
    }

    public void test18_isSdReadable() {
        SystemInfoProxy.isSdReadable();
    }

    public void test19_isSdWriteable() {
        SystemInfoProxy.isSdWriteable();
    }

    public void test20_checkSDCardState() {
        boolean[] sDstate = mProxy.checkSDCardState();
        assertNotNull(sDstate);
    }

    public void test21_GetInternalStoragePathSD() {
        String result = null;
        SystemInfoProxy.getInternalStoragePathSD();
        if (SystemInfoProxy.isSdSwap()) {
            result = SystemInfoProxy.getInternalStoragePathSD();
            assertNotNull(result);
        }
    }

    public void test22_GetExternalStoragePath() {
        String result = null;
        SystemInfoProxy.getExternalStoragePath();
        if (SystemInfoProxy.isSdSwap()) {
            result = SystemInfoProxy.getExternalStoragePath();
            assertNotNull(result);
        }
    }

    public void test23_IsExSdcardInserted() {
        SystemInfoProxy.isExSdcardInserted();
    }
}
