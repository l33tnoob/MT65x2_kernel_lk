package com.mediatek.engineermode.nonsleep;

import android.app.Activity;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.PowerManager;
import android.support.v4.app.NotificationCompat;

import com.mediatek.xlog.Xlog;

public class EMWakeLockService extends Service {
    
    private static final String TAG = "EMWakeLockService";
    private static final int ID_FORE_GROUND_NOTIF = 11;
    
    private final IBinder mBinder = new LocalBinder();
    private PowerManager.WakeLock mWakeLock = null;
    
    
    public class LocalBinder extends Binder {
        EMWakeLockService getService() {
            return EMWakeLockService.this;
        }
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        Xlog.d(TAG, "onCreate()");
        mWakeLock = ((PowerManager)getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.FULL_WAKE_LOCK | 
                PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
    }
    
    @Override
    public void onDestroy() {
        Xlog.d(TAG, "onDestroy()");
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
            mWakeLock = null;
        }
        super.onDestroy();
    }
    
    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    public boolean isHeld() {
        return mWakeLock.isHeld();
    }
    
    public void release() {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
            stopForeground(true);
        }
    }
    
    public void acquire(Class<? extends Activity> targetClass) {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
            startForeground(ID_FORE_GROUND_NOTIF, buildNotification(targetClass));
        }
    }
    
    public void acquire(Class<? extends Activity> targetClass, long milliSec) {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire(milliSec);
            startForeground(ID_FORE_GROUND_NOTIF, buildNotification(targetClass));
        }
    }
    
    private Notification buildNotification(Class<? extends Activity> clazz) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this);
        builder.setSmallIcon(android.R.drawable.ic_dialog_alert).
        setContentTitle("No Sleep Mode is Enabled").
        setContentText("Click Here to Switch No Sleep Mode");
        Intent intent = new Intent(this, clazz).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent pi = PendingIntent.getActivity(this, 0, intent, 0);
        builder.setContentIntent(pi);
        return builder.build();
    }

}
