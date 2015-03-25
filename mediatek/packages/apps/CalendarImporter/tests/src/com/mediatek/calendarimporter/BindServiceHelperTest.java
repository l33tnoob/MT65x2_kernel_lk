package com.mediatek.calendarimporter;

import android.test.AndroidTestCase;

import com.mediatek.calendarimporter.service.VCalService;
import com.mediatek.vcalendar.utils.LogUtil;

public class BindServiceHelperTest extends AndroidTestCase {
    private static final String TAG = "BindServiceHelperTest";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    //only to override the interface
    private class ServiceConnectedListener implements BindServiceHelper.ServiceConnectedOperation {
        @Override
        public void serviceConnected(VCalService service) {
            LogUtil.d(TAG, "serviceConnected.");
        }
        @Override
        public void serviceUnConnected() {
        }
    }

    public void test01_BindService() {
        BindServiceHelper helper = new BindServiceHelper(getContext(), new ServiceConnectedListener());
        helper.onBindService();
        TestUtils.sleepForTime(TestUtils.DEFAULT_SLEEP_TIME);
        helper.unBindService();
    }
}

