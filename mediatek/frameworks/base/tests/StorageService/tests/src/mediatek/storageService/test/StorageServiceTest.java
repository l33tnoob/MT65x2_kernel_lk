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

package mediatek.StorageService.test;


import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemClock;
import android.util.Slog;
import java.io.IOException;
import android.test.AndroidTestCase;

public class StorageServiceTest extends AndroidTestCase {
    private static final String TAG = "DeviceStorageMonitorServiceTest";
    private MockStorageReceiver mMockStorageReceiver;

    private final int TIME_DELAY = 31000;


    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMockStorageReceiver = new MockStorageReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(MockStorageReceiver.LOW);
        filter.addAction(MockStorageReceiver.FULL);
        filter.addAction(MockStorageReceiver.NOTFULL);
        filter.addAction(MockStorageReceiver.OK);
        mContext.registerReceiver(mMockStorageReceiver, filter);         
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        //System.exit(0);
        
        /*try {
            Runtime.getRuntime().exit(0);
        } catch (Exception e) {
	        Slog.e(TAG, "exit error"+ e);
        } */   
    }

    public void testStorageLow() throws Exception {
       
        try {  
            Runtime.getRuntime().exec("setprop ctsrunning 2");
            Slog.e(TAG, "set ctsrunning !");   
        } catch (IOException e) {
	        Slog.e(TAG, "set ctsrunning error"+ e);
        }
        // test storage low case
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 9437184");
            Slog.e(TAG, "set debug.freemem !");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageLow);
        
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 3145728");            
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageFull);        

        try {
            Runtime.getRuntime().exec("input keyevent 4");
        } catch (IOException e) {
	        Slog.e(TAG, "input keyevent error"+ e);
        }
                
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 9437184");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageNotFull);        
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 94371840");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        } 
                    
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageOk);  
        try {
            Runtime.getRuntime().exec("am force-stop com.mediatek.storageService.test");
        } catch (IOException e) {
	        Slog.e(TAG, "am force-stop "+ e);
        }          
      //  android.os.Process.killProcess(android.os.Process.myPid());    
    }
/*
    public void testStorageFull() throws Exception {
        // test storage low case
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 3145728");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageFull);
    }
    
    public void testStorageNotFull() throws Exception {
        // test storage low case
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 9437184");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageNotFull);
    }
    
    
    public void testStorageOk() throws Exception {
        // test storage low case
        mMockStorageReceiver.setStorageFlagFalse();
        try {
            Runtime.getRuntime().exec("setprop debug.freemem 94371840");
        } catch (IOException e) {
	        Slog.e(TAG, "set debug.freemem error"+ e);
        }  
        Thread.sleep(TIME_DELAY);
        assertTrue(mMockStorageReceiver.mStorageOk);
    }
    */
}
