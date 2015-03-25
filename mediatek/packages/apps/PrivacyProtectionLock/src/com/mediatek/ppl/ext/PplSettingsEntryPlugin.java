package com.mediatek.ppl.ext;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceGroup;
import android.widget.Toast;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.R;
import com.mediatek.ppl.SimTracker;
import com.mediatek.ppl.ui.AccessLockActivity;
import com.mediatek.ppl.ui.ChooseEnableModeActivity;
import com.mediatek.ppl.ui.SetupPasswordActivity;
import com.mediatek.settings.ext.IPplSettingsEntryExt;

public class PplSettingsEntryPlugin extends ContextWrapper implements IPplSettingsEntryExt {

    private Preference mPreference;
    private final IPplAgent mAgent;
    private final Context mContext;
    private SimTracker mSimTracker;
    private EventReceiver mReceiver;
    private OnPreferenceClickListener mNoSimListener;
    private OnPreferenceClickListener mEnabledListener;
    private OnPreferenceClickListener mProvisionedListener;
    private OnPreferenceClickListener mNotProvisionedListener;

    private class EventReceiver extends BroadcastReceiver {
        // com.android.internal.telephony.TelephonyIntents.ACTION_SIM_STATE_CHANGED
        public static final String ACTION_SIM_STATE_CHANGED = TelephonyIntents.ACTION_SIM_STATE_CHANGED;

        public void initialize() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_SIM_STATE_CHANGED);
            registerReceiver(this, filter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                return;
            }

            if (action.equals(ACTION_SIM_STATE_CHANGED)) {
                mSimTracker.takeSnapshot();
                updateUI();
            }
        }
    }

    public PplSettingsEntryPlugin(Context context) {
        super(context);
        mContext = context;
        IBinder binder = ServiceManager.getService("PPLAgent");
        if (binder == null) {
            throw new Error("Failed to get PPLAgent");
        }
        mAgent = IPplAgent.Stub.asInterface(binder);
        if (mAgent == null) {
            throw new Error("mAgent is null!");
        }
        mPreference = new Preference(context);
        mPreference.setTitle(R.string.app_name);
        mPreference.setSummary(R.string.status_pending);
        mSimTracker = new SimTracker(PlatformManager.SIM_NUMBER, mContext);
        mNoSimListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Toast toast = Toast.makeText(mContext, R.string.toast_no_sim, Toast.LENGTH_SHORT);
                toast.show();
                return true;
            }
        };
        mEnabledListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                // enter access lock
                Intent intent = new Intent();
                intent.setClass(mContext, AccessLockActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                return true;
            }
        };
        mProvisionedListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent intent = new Intent();
                intent.setClass(mContext, ChooseEnableModeActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                return true;
            }
        };
        mNotProvisionedListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent intent = new Intent();
                intent.setClass(mContext, SetupPasswordActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                return true;
            }
        };
    }

    public void addPplPrf(PreferenceGroup prefGroup) {
        if (prefGroup instanceof PreferenceGroup) {
            prefGroup.addPreference(mPreference);
        }
    }

    @Override
    public void enablerResume() {
        mReceiver = new EventReceiver();
        mReceiver.initialize();
        mSimTracker.takeSnapshot();
        updateUI();
    }

    @Override
    public void enablerPause() {
        if (mReceiver != null) {
            mReceiver.destroy();
            mReceiver = null;
        }
    }

    private void updateUI() {
        if (mSimTracker.getInsertedSim().length == 0) {
            mPreference.setOnPreferenceClickListener(mNoSimListener);
            mPreference.setSummary(R.string.status_pending);
        } else {
            ControlData controlData = null;
            try {
                controlData = ControlData.buildControlData(mAgent.readControlData());
            } catch (RemoteException e) {
                e.printStackTrace();
            }

            if (controlData == null) {
                mPreference.setEnabled(false);
                return;
            }
            if (controlData.isEnabled()) {
                mPreference.setSummary(R.string.status_enabled);
                mPreference.setOnPreferenceClickListener(mEnabledListener);
            } else if (controlData.isProvisioned()) {
                mPreference.setSummary(R.string.status_provisioned);
                mPreference.setOnPreferenceClickListener(mProvisionedListener);
            } else {
                mPreference.setSummary(R.string.status_unprovisioned);
                mPreference.setOnPreferenceClickListener(mNotProvisionedListener);
            }
        }
    }
}
