
package com.mediatek.DataUsageLockScreenClient;
import android.content.Context;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.util.Log;
import android.view.View;

import com.mediatek.keyguard.ext.IKeyguardLayer;
import com.mediatek.keyguard.ext.KeyguardLayerInfo;

public class DataUsageLauncher implements IKeyguardLayer  {
    private static final String TAG = "DataUsageLauncher";
    private static int mTotalRunInstance = 0;
    private static DataUsageStageView mStageView = null;
    private Context mContext;
    private int mID;

    private void log(String msg) {
        Log.d(TAG, "<" + mID + "> " + msg);
    }

    public DataUsageLauncher(Context context) {
        mContext = context;
        mID = mTotalRunInstance;
        mTotalRunInstance++;
        log("DataUsageLauncher() " + context);
    }

    @Override
    public View create() {
        synchronized (TAG) {
            View ret = null;
            boolean isShowing = getShowLockScreen();
            if (isShowing) {
                if (mStageView == null) {
                    mStageView = new DataUsageStageView(mContext);
                }
                log("create() " + mStageView);
                ret = mStageView.create();
            } else {
                log("create() " + mStageView);
            }
            return ret;
        }
    }

    @Override
    public void destroy() {
        synchronized (TAG) {
            log("destroy() " + mStageView);
            if (null != mStageView) {
                mStageView.destroy();
                mStageView = null;
            }
        }
    }

    @Override
    public KeyguardLayerInfo getKeyguardLayerInfo() {
        return null;
    }

    // setting -> data usage -> switch button " Show on LockScreen"
    private boolean getShowLockScreen() {
        int isShowingSim1 = 0;
        int isShowingSim2 = 0;

        try {
            isShowingSim1 = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.DATAUSAGE_ON_LOCKSCREEN_SIM1);
            isShowingSim2 = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.DATAUSAGE_ON_LOCKSCREEN_SIM2);
            log("On LockScreen (Sim1:" + isShowingSim1 + ", Sim2:" + isShowingSim2 + ")");
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
        }

        return (isShowingSim1 + isShowingSim2) > 0;
    }
}
