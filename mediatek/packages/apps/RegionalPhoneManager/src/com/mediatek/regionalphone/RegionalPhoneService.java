package com.mediatek.regionalphone;

import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.rpm.ext.IRegionalPhoneAddMmsApn;
import com.mediatek.rpm.ext.ISettingsExt;

import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.IBinder;
import com.mediatek.xlog.Xlog;

public class RegionalPhoneService extends Service {

    public static final String ACTION = "com.mediatek.regionalphone.regionalphoneservice";

    private static final String TAG = Common.LOG_TAG;
    private ContentResolver mContentResolver;
    private Context mContext;
    private Thread mRPMThread = null;

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Xlog.d(TAG, "RegionalPhoneService::onCreate");
        mContentResolver = this.getContentResolver();
        mContentResolver.registerContentObserver(RegionalPhone.MMS_SMS_URI,
                true, mmsObserver);
        mContentResolver.registerContentObserver(RegionalPhone.SETTINGS_URI,
                true, settingsObserver);
        mContext = this.getApplicationContext();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Xlog.d(TAG, "RegionalPhoneService::onStartCommand");
        boolean isSIMLoaded = (intent != null && intent.getBooleanExtra(Common.SIM_LOADED, true));
        Xlog.d(TAG, "RegionalPhoneService::onStartCommand, is sim loaded: " + isSIMLoaded);
        if (mRPMThread == null || isSIMLoaded){
            mRPMThread = new Thread(new RegionalPhoneRunnable(mContext));
            mRPMThread.start();
        }

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mContentResolver.unregisterContentObserver(mmsObserver);
        mContentResolver.unregisterContentObserver(settingsObserver);
    }

    private ContentObserver mmsObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            try {
                PluginManager<IRegionalPhoneAddMmsApn> pluginManager;
                pluginManager = PluginManager.<IRegionalPhoneAddMmsApn> create(
                        mContext, IRegionalPhoneAddMmsApn.class.getName());
                Plugin<IRegionalPhoneAddMmsApn> plugin = pluginManager
                        .getPlugin(0);
                IRegionalPhoneAddMmsApn mIRegionalPhoneAddMmsApn = plugin
                        .createObject();
                mIRegionalPhoneAddMmsApn.addMmsApn(getApplicationContext());
            } catch (Plugin.ObjectCreationException e) {
                Xlog.d(TAG, "IRegionalPhoneAddMmsApn No plugin found");
            } catch (IndexOutOfBoundsException e) {
                Xlog.d(TAG, "IRegionalPhoneAddMmsApn " + e);
            } catch (NullPointerException e) {
                Xlog.d(TAG, "IRegionalPhoneAddMmsApn Null Pointer Exception");
            }
        }
    };

    private ContentObserver settingsObserver = new ContentObserver(
            new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            try {
                PluginManager<ISettingsExt> pluginManager;
                pluginManager = PluginManager.<ISettingsExt> create(mContext,
                        ISettingsExt.class.getName());
                Plugin<ISettingsExt> plugin = pluginManager.getPlugin(0);
                ISettingsExt mISettingsExt = plugin.createObject();
                mISettingsExt.updateConfiguration(getApplicationContext());
            } catch (Plugin.ObjectCreationException e) {
                Xlog.d(TAG, "ISettingsExt No plugin found");
            } catch (IndexOutOfBoundsException e) {
                Xlog.d(TAG, "ISettingsExt " + e);
            } catch (NullPointerException e) {
                Xlog.d(TAG, "ISettingsExt Null Pointer Exception");
            }
        }
    };

}
