/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright © 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.utils;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.SystemClock;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Periodic refresher
 *
 * @author JM. Auffret
 */
public abstract class PeriodicRefresher {
	/**
     * Keep alive manager
     */
    private KeepAlive alarmReceiver = new KeepAlive(); 

    /**
     * Alarm intent
     */
    private PendingIntent alarmIntent;

    /**
     * Action 
     */
    private String action;
    
    /**
     * M: Modified to resolve the java exception issue. @{
     */
    /**
     * Timer state
     */
    private volatile Boolean timerStarted = false;
    /**
     * @}
     */
    
    /**
     * Polling period
     */
    private int pollingPeriod;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * M: WakeLock @{
     */
    //The Wakelock acquired when alarm time is up
    private WakeLock mPeriodWakeLock = null;
    //The Wakelock acquired when receive data from network
    private WakeLock mNetworkWakeLock = null;
    //Tag
    protected static final String PERIOD_WAKELOCK = "PeriodWakeLock";
    protected static final String NETWORK_WAKELOCK = "NetworkWakeLock";
    /**
     * @}
     */
    
    /**
     * Constructor
     */
    public PeriodicRefresher() {
    	// Create a unique pending intent
    	this.action = this.toString(); // Unique action ID 
    	this.alarmIntent = PendingIntent.getBroadcast(
    			AndroidFactory.getApplicationContext(),
    			0,
    			new Intent(action),
    			0);
        /**
         * M: Initialize WakeLock @{
         */
        PowerManager pm = (PowerManager) AndroidFactory.getApplicationContext().getSystemService(
                Context.POWER_SERVICE);
        mPeriodWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,PERIOD_WAKELOCK);
        mNetworkWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,NETWORK_WAKELOCK);
        /**
         * @}
         */
    }
    
    /**
     * Periodic processing
     */
    public abstract void periodicProcessing();
    
    /**
     * Start the timer
     * 
     * @param expirePeriod Expiration period in seconds
     */
    public void startTimer(int expirePeriod) {
    	startTimer(expirePeriod, 1.0);
    }
    	
    /**
     * M: Modified to resolve the java exception issue. @{
     */
    /**
     * Start the timer
     * 
     * @param expirePeriod Expiration period in seconds
     * @param delta Delta to apply on the expire period in percentage
     */
    public void startTimer(int expirePeriod, double delta) {
    	// Check expire period
    	if (expirePeriod <= 0) {
    		// Expire period is null
        	if (logger.isActivated()) {
        		logger.debug("Timer is deactivated");
        	}
    		return;
    	}

    	// Calculate the effective refresh period
    	pollingPeriod = (int)(expirePeriod * delta);
    	if (logger.isActivated()) {
    		logger.debug("Start timer at period=" + pollingPeriod +  "s (expiration=" + expirePeriod + "s)");
    	}
        /**
         * M: Modified to resolve the RCS-e server can't be connected issue. @{
         */
        if (!timerStarted) {
            synchronized (timerStarted) {
                // Double check in synchronization
                if (!timerStarted) {
                    logger.debug("startTimer() begin to register, current PeriodicRefresher is "
                            + this);
        // Register the alarm receiver
                    AndroidFactory.getApplicationContext().registerReceiver(alarmReceiver,
                            new IntentFilter(action));
        // The timer is started
    	timerStarted = true;
                } else {
                    logger
                            .error("startTimer() timerStarted is true in double check, current PeriodicRefresher is "
                                    + this);
                }
            }
        } else {
            logger.debug("startTimer() timerStarted is true in first check," +
                " so doesn't register again,current PeriodicRefresher is " + this);
        }
        /**
         * @}
         */
        // Start alarm from now to the expire value
        AlarmManager am = (AlarmManager) AndroidFactory.getApplicationContext()
                .getSystemService(Context.ALARM_SERVICE);
        am.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + pollingPeriod
                * 1000, alarmIntent);
        logger.debug("startTimer() exit");
    }

    /**
     * Stop the timer
     */
    public void stopTimer() {
        logger.debug("stopTimer() entry, current PeriodicRefresher is " + this);
        if (timerStarted) {
            synchronized (timerStarted) {
                // Double check for timerStarted
                if (timerStarted) {
    	if (logger.isActivated()) {
    		logger.debug("Stop timer");
    	}
                    logger.debug("stopTimer() begin to stop, current PeriodicRefresher is " + this);
    	// The timer is stopped
		timerStarted = false;
		
		// Cancel alarm
                    AlarmManager am = (AlarmManager) AndroidFactory.getApplicationContext()
                            .getSystemService(Context.ALARM_SERVICE);
		am.cancel(alarmIntent);

		// Unregister the alarm receiver
			AndroidFactory.getApplicationContext().unregisterReceiver(alarmReceiver);
                } else {
                    logger
                            .error("stopTimer() timerStarted is false in double check, current PeriodicRefresher is "
                                    + this);
	    }
            }
        } else {
            logger.error("stopTimer() timerStarted is false, current PeriodicRefresher is " + this);
        }
    }
    /**
     * @}
     */

    /**
     * Keep alive manager
     */
    private class KeepAlive extends BroadcastReceiver {
    	public void onReceive(Context context, Intent intent) {
    		Thread t = new Thread() {
    			public void run() {
    				// Processing
    			    if (logger.isActivated()) {
    		            logger.info("Execute periodicProcessing, begin at " + SystemClock.elapsedRealtime());
    		        }
    				periodicProcessing();
    				if (logger.isActivated()) {
                        logger.info("Execute periodicProcessing, end at " + SystemClock.elapsedRealtime());
                    }
    			}
    		};
    		t.start();
    	}
    }    
    
    /**
     * M: Initialize WakeLock @{
     */
    protected void acquirePeriodWakeLock() {
        if (logger.isActivated()) {
            logger.debug("acquirePeriodWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (!mPeriodWakeLock.isHeld()) {
            mPeriodWakeLock.acquire();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is held, so do not acquire");
            }
        }
    }

    protected void acquireNetworkWakeLock() {
        if (logger.isActivated()) {
            logger.debug("acquireNetworkWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (!mNetworkWakeLock.isHeld()) {
            mNetworkWakeLock.acquire();
        } else {
            if (logger.isActivated()) {
                logger.debug("mNetworkWakeLock is held, so do not acquire");
            }
        }
    }

    protected void releasePeriodWakeLock() {
        if (logger.isActivated()) {
            logger.debug("releasePeriodWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (mPeriodWakeLock.isHeld()) {
            mPeriodWakeLock.release();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is not held, so do not release");
            }
        }
    }

    protected void releaseNetworkWakeLock() {
        if (logger.isActivated()) {
            logger.debug("releaseeNetworkWakeLock() at " + SystemClock.elapsedRealtime());
        }
        if (mNetworkWakeLock.isHeld()) {
            mNetworkWakeLock.release();
        } else {
            if (logger.isActivated()) {
                logger.debug("mPeriodWakeLock is not held, so do not release");
            }
        }
    }
    /**
     * @}
     */
}
