/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package mediatek.alarmManager.test;


import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemClock;
import android.test.AndroidTestCase;
import java.io.IOException;
import android.util.Slog;

public class AlarmManagerTest extends AndroidTestCase {
    private AlarmManager mAlarmManager;
    private Intent mIntent;
    private PendingIntent mSender;
    private Intent mServiceIntent;
    private static final String TAG = "AlarmManagerServiceTest";
    /*
     *  The default snooze delay: 5 seconds
     */
    private final long SNOOZE_DELAY = 5 * 1000L;
    private long mWakeupTime;
    private MockAlarmReceiver mMockAlarmReceiver;

    private final int TIME_DELTA = 200;
    private final int TIME_DELAY = 2000;

    class Sync {
        public boolean mIsConnected;
        public boolean mIsDisConnected;
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mAlarmManager = (AlarmManager) mContext.getSystemService(Context.ALARM_SERVICE);
        mIntent = new Intent(MockAlarmReceiver.MOCKACTION);
        mSender = PendingIntent.getBroadcast(mContext, 0, mIntent, 0);
        mMockAlarmReceiver = new MockAlarmReceiver();
        IntentFilter filter = new IntentFilter(MockAlarmReceiver.MOCKACTION);
        mContext.registerReceiver(mMockAlarmReceiver, filter);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        if (mServiceIntent != null) {
            mContext.stopService(mServiceIntent);
        }
    }

    public void testSetTypes() throws Exception {
        // TODO: try to find a way to make device sleep then test whether
        // AlarmManager perform the expected way

        // test parameter type is RTC_WAKEUP
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = System.currentTimeMillis() + SNOOZE_DELAY;
        mAlarmManager.set(AlarmManager.RTC_WAKEUP, mWakeupTime, mSender);
        // test parameter type is PowerOffAlarm
        mAlarmManager.set(7, mWakeupTime, mSender);
        mAlarmManager.cancelPoweroffAlarm("com.android.deskclock");
        // test parameter type is RTC        
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        assertEquals(mMockAlarmReceiver.rtcTime, mWakeupTime, TIME_DELTA);

        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = System.currentTimeMillis() + SNOOZE_DELAY;
        mAlarmManager.set(AlarmManager.RTC, mWakeupTime, mSender);
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        assertEquals(mMockAlarmReceiver.rtcTime, mWakeupTime, TIME_DELTA);

        // test parameter type is ELAPSED_REALTIME
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = SystemClock.elapsedRealtime() + SNOOZE_DELAY;
        mAlarmManager.set(AlarmManager.ELAPSED_REALTIME, mWakeupTime, mSender);
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        assertEquals(mMockAlarmReceiver.elapsedTime, mWakeupTime, TIME_DELTA);

        // test parameter type is ELAPSED_REALTIME_WAKEUP
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = SystemClock.elapsedRealtime() + SNOOZE_DELAY;
        mAlarmManager.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, mWakeupTime, mSender);
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        assertEquals(mMockAlarmReceiver.elapsedTime, mWakeupTime, TIME_DELTA);
    }

    public void testAlarmTriggersImmediatelyIfSetTimeIsNegative() throws Exception {
        // An alarm with a negative wakeup time should be triggered immediately.
        // This exercises a workaround for a limitation of the /dev/alarm driver
        // that would instead cause such alarms to never be triggered.
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = -1000;
        mAlarmManager.set(AlarmManager.RTC, mWakeupTime, mSender);
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
    }

    public void testSetRepeating() throws Exception {
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = System.currentTimeMillis() + SNOOZE_DELAY;
        mAlarmManager.setRepeating(AlarmManager.RTC_WAKEUP, mWakeupTime, TIME_DELAY / 2, mSender);
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        mMockAlarmReceiver.setAlarmedFalse();
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        mAlarmManager.cancel(mSender);
    }

    public void testCancel() throws Exception {
        mMockAlarmReceiver.setAlarmedFalse();
        mWakeupTime = System.currentTimeMillis() + SNOOZE_DELAY;
        mAlarmManager.setRepeating(AlarmManager.RTC_WAKEUP, mWakeupTime, 1000, mSender);
        Thread.sleep(SNOOZE_DELAY + TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        mMockAlarmReceiver.setAlarmedFalse();
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockAlarmReceiver.alarmed);
        mAlarmManager.cancel(mSender);
        Thread.sleep(TIME_DELAY);
        mMockAlarmReceiver.setAlarmedFalse();
        Thread.sleep(TIME_DELAY * 5);
        assertFalse(mMockAlarmReceiver.alarmed);
    }

    public void testSetInexactRepeating() throws Exception {

        mAlarmManager.setInexactRepeating(AlarmManager.RTC_WAKEUP, System.currentTimeMillis(),
                AlarmManager.INTERVAL_FIFTEEN_MINUTES, mSender);
        SystemClock.setCurrentTimeMillis(System.currentTimeMillis()
                + AlarmManager.INTERVAL_FIFTEEN_MINUTES);
        // currently there is no way to write Android system clock. When try to
        // write the system time, there will be log as
        // " Unable to open alarm driver: Permission denied". But still fail
        // after tried many permission.
    }
    
    public void testErrorHandleAndDM() throws Exception {

       mAlarmManager.set(7, 0, mSender);
       try {
            Runtime.getRuntime().exec("dumpsys alarm");
        } catch (IOException e) {
	        Slog.e(TAG, "dumpsys "+ e);
        }
       try {
            Runtime.getRuntime().exec("am broadcast -a com.mediatek.dm.LAWMO_UNLOCK");
       } catch (IOException e) {
	        Slog.e(TAG, "am broadcast "+ e);
       }   
    }
    
}
