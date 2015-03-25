package com.mediatek.batterywarning;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.text.Html;
import android.media.AudioManager;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;
import com.mediatek.batterywarning.R;
import com.mediatek.xlog.Xlog;

public class BatteryWarningActivity extends AlertActivity implements
    DialogInterface.OnClickListener{
    private static final String TAG = "BatteryWarningActivity";
    private static final Uri WARNING_SOUND_URI = Uri
            .parse("file:///system/media/audio/ui/VideoRecord.ogg");
    private static final String SHARED_PREFERENCES_NAME = "battery_warning_settings";

    private Ringtone mRingtone;
    private int mType;

    private static final int CHARGER_OVER_VOLTAGE_TYPE = 0;
    private static final int BATTERY_OVER_TEMPERATURE_TYPE = 1;
    private static final int CURRENT_OVER_PROTECTION_TYPE = 2;
    private static final int BATTERY_OVER_VOLTAGE_TYPE = 3;
    private static final int SAFETY_OVER_TIMEOUT_TYPE = 4;

    static final int[] sWarningTitle = new int[] {
            R.string.title_charger_over_voltage,
            R.string.title_battery_over_temperature,
            R.string.title_over_current_protection,
            R.string.title_battery_over_voltage,
            R.string.title_safety_timer_timeout };
    private static final int[] sWarningMsg = new int[] {
            R.string.msg_charger_over_voltage,
            R.string.msg_battery_over_temperature,
            R.string.msg_over_current_protection,
            R.string.msg_battery_over_voltage,
            R.string.msg_safety_timer_timeout };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_POWER_DISCONNECTED.equals(action)) {
                if (mType == CHARGER_OVER_VOLTAGE_TYPE
                        || mType == SAFETY_OVER_TIMEOUT_TYPE) {
                    Xlog.d(TAG, "receive ACTION_POWER_DISCONNECTED broadcast, finish");
                    finish();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        mType = intent.getIntExtra("type", -1);
        Xlog.d(TAG, "onCreate, mType is " + mType);
        if(mType >= CHARGER_OVER_VOLTAGE_TYPE  && mType <= SAFETY_OVER_TIMEOUT_TYPE) {
            showWarningDialog(mType);
            registerReceiver(mReceiver, new IntentFilter(
                    Intent.ACTION_POWER_DISCONNECTED));
        } else {
            finish();
        }
    }
    
    protected void onDestroy() {
        super.onDestroy();
        if(mType >= CHARGER_OVER_VOLTAGE_TYPE  && mType <= SAFETY_OVER_TIMEOUT_TYPE) {
            unregisterReceiver(mReceiver);
        }
    }
    
    private void showWarningDialog(int type) {
            warningMessageDialog(sWarningTitle[type], sWarningMsg[type]);
            playAlertSound(WARNING_SOUND_URI);
    }

    /**
     * 
     * @param context
     *            The Context that had been passed to
     *            {@link #warningMessageDialog(Context, int, int, int)}
     * @param titleResId
     *            Set the title using the given resource id.
     * @param messageResId
     *            Set the message using the given resource id.
     * @return Creates a {@link AlertDialog} with the arguments supplied to this
     *         builder.
     */
    private void warningMessageDialog(int titleResId, int messageResId) {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(titleResId);
        p.mView = createView(messageResId);
        p.mPositiveButtonText = getString(R.string.btn_ok_msg);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.btn_cancel_msg);
        p.mNegativeButtonListener = this;
        setupAlert();
    }

    private View createView(int messageResId) {
        View view = getLayoutInflater().inflate(R.layout.battery_warning, null);
        TextView mMessageView = (TextView) view.findViewById(R.id.subtitle);
        mMessageView.setText(messageResId);
        ImageView mImageView = (ImageView) view.findViewById(R.id.image);
        mImageView.setImageResource(R.drawable.battery_low_battery);
        return view;
    }
    
    /**
     * 
     * @param context
     *            The Context that had been passed to
     *            {@link #warningMessageDialog(Context, Uri)}
     * @param defaultUri
     */

    private void playAlertSound(Uri defaultUri) {

        if (defaultUri != null) {
            mRingtone = RingtoneManager.getRingtone(this, defaultUri);
            if (mRingtone != null) {
                mRingtone.setStreamType(AudioManager.STREAM_SYSTEM);
                mRingtone.play();
            }
        }
    }

    private void stopRingtone() {
        if (mRingtone != null) {
            mRingtone.stop();
        }
    }

    public void onClick(DialogInterface dialogInterface, int button) {
        Xlog.d(TAG, "onClick");
        if (button == DialogInterface.BUTTON_POSITIVE) {
            stopRingtone();
            return;
        } else {
            stopRingtone();
            SharedPreferences.Editor editor = getSharedPreferences().edit();
            editor.putBoolean(getString(sWarningTitle[mType]), false);
            editor.apply();
            Xlog.d(TAG, "set type " + mType + " false");
        }
    }
    
    private SharedPreferences getSharedPreferences() {
        return getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }
}
