package com.mediatek.ppl.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

/**
 * Show a dialog and prompt user to select enable mode: start newly or enable previously configured settings.
 */
public class ChooseEnableModeActivity extends Activity {

    private EventReceiver mEventReceiver;

    private class EventReceiver extends BroadcastReceiver {
        public EventReceiver() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(PplService.Intents.UI_NO_SIM);
            registerReceiver(this, intentFilter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (PplService.Intents.UI_NO_SIM.equals(intent.getAction())) {
                finish();
            }
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mEventReceiver = new EventReceiver();

        new AlertDialog.Builder(this, AlertDialog.THEME_HOLO_DARK)
                .setCancelable(false)
                .setCancelable(false)
                .setTitle(R.string.title_choose_enable_mode)
                .setItems(R.array.enable_mode_list, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which) {
                        case 0:
                            usePreviousSettings();
                            break;
                        case 1:
                            useNewSettings();
                            break;
                        default:
                            break;
                        }
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                })
                .create().show();
    }

    @Override
    public void onDestroy() {
        mEventReceiver.destroy();
        super.onDestroy();
    }

    private void usePreviousSettings() {
        Intent intent = new Intent();
        intent.setClass(this, AccessLockActivity.class);
        intent.putExtra(AccessLockActivity.EXTRA_ENABLE_PREVIOUS, true);
        startActivity(intent);
        finish();
    }

    private void useNewSettings() {
        Intent intent = new Intent();
        intent.setClass(this, SetupPasswordActivity.class);
        startActivity(intent);
        finish();
    }

}
