package com.mediatek.ppl.ui;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.Settings;
import android.text.Html;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;
import com.mediatek.ppl.ui.ChooseSimDialogFragment.ISendMessage;
import com.mediatek.ppl.ui.ConfirmTurnoffAirplaneModeDialogFragment.ITurnOffAirplaneMode;

/**
 * The third step of setup wizard. It shows a screen for user to send instruction descriptions to emergency contacts.
 */
public class SetupSendManualActivity extends Activity implements ISendMessage, ITurnOffAirplaneMode {
    private static final String TAG = "PPL/SetupSendManualActivity";

    private ProgressBar mProgressBar;
    private View mContentView;
    private CheckBox mSendMessage;
    private Button mNextButton;
    private TextView mManualContent;
    private InternalControllerBinder mBinder;
    ControlData mEditBuffer;
    private ServiceConnection mServiceConnection;
    private EventReceiver mEventReceiver;

    private class EventReceiver extends BroadcastReceiver {
        public EventReceiver() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
            intentFilter.addAction(PplService.Intents.UI_NO_SIM);
            registerReceiver(this, intentFilter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (PplService.Intents.UI_QUIT_SETUP_WIZARD.equals(intent.getAction())) {
                finish();
            } else if (PplService.Intents.UI_NO_SIM.equals(intent.getAction())) {
                finish();
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.setup_send_manual);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.setup_send_manual_content);
        mContentView.setVisibility(View.GONE);
        mManualContent = (TextView) findViewById(R.id.setup_send_manual_instructions);
        mManualContent.setText(Html.fromHtml(getString(R.string.content_setup_send_manual)));
        mNextButton = (Button) findViewById(R.id.setup_send_manual_next_button);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                goToNext();
            }
        });
        mSendMessage = (CheckBox) findViewById(R.id.setup_send_manual_send_sms_checkbox);
        mSendMessage.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mNextButton.setText(R.string.button_send_sms);
                } else {
                    mNextButton.setText(R.string.button_finish);
                }
            }
        });
        mSendMessage.setChecked(false);

        Intent intent = new Intent(PplService.Intents.PPL_MANAGER_SERVICE);
        intent.setClass(this, PplService.class);
        intent.setPackage("com.mediatek.ppl");
        mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                mBinder = (InternalControllerBinder) binder;
                mEditBuffer = mBinder.startEdit();
                mProgressBar.setVisibility(View.GONE);
                mContentView.setVisibility(View.VISIBLE);
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                finish();
            }
        };
        bindService(intent, mServiceConnection, BIND_AUTO_CREATE);

        mEventReceiver = new EventReceiver();
    }

    @Override
    public void onDestroy() {
        mEventReceiver.destroy();

        if (mBinder != null) {
            mBinder.finishEdit(PplManager.ACTION_KEEP);
        }
        if (mServiceConnection != null) {
            unbindService(mServiceConnection);
        }
        super.onDestroy();
    }

    private void goToNext() {
        if (mSendMessage.isChecked()) {
            if (isAirplaneModeEnabled()) {
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                // Create and show the dialog.
                DialogFragment newFragment = new ConfirmTurnoffAirplaneModeDialogFragment();
                newFragment.show(ft, "turnoff_airplane_mode_dialog");
            } else {
                int[] insertedSim = mBinder.getInsertedSim();
                if (insertedSim.length > 1) {
                    String[] itemList = new String[insertedSim.length];
                    String itemTemplate = getResources().getString(R.string.item_sim_n);
                    for (int i = 0; i < insertedSim.length; ++i) {
                        itemList[i] = itemTemplate + i;
                    }
                    FragmentTransaction ft = getFragmentManager().beginTransaction();
                    // Create and show the dialog.
                    DialogFragment newFragment = new ChooseSimDialogFragment();
                    Bundle args = new Bundle();
                    args.putStringArray(ChooseSimDialogFragment.ARG_KEY_ITEMS, itemList);
                    args.putIntArray(ChooseSimDialogFragment.ARG_KEY_VALUES, insertedSim);
                    newFragment.setArguments(args);
                    newFragment.show(ft, "choose_sim_dialog");
                } else if (insertedSim.length == 1) {
                    mEditBuffer.setProvision(true);
                    mBinder.finishEdit(PplManager.ACTION_COMMIT);
                    mBinder.enable(true);
                    mBinder.sendInstructionDescriptionMessage(insertedSim[0]);
                    goToControlPanel();
                } else {
                    Toast toast = Toast.makeText(this, R.string.toast_no_sim, Toast.LENGTH_SHORT);
                    toast.show();
                }
            }
        } else {
            mEditBuffer.setProvision(true);
            mBinder.finishEdit(PplManager.ACTION_COMMIT);
            mBinder.enable(true);
            goToControlPanel();
        }
    }

    private boolean isAirplaneModeEnabled() {
        String state = Settings.Global.getString(getContentResolver(), Settings.Global.AIRPLANE_MODE_ON);
        return "1".equals(state);
    }

    @Override
    public void turnoffAirplaneMode() {
        Settings.Global.putString(getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, "0");
        sendBroadcast(new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED));
    }

    @Override
    public void sendMessage(int simId) {
        mEditBuffer.setProvision(true);
        mBinder.finishEdit(PplManager.ACTION_COMMIT);
        mBinder.enable(true);
        mBinder.sendInstructionDescriptionMessage(simId);
        goToControlPanel();
    }

    private void goToControlPanel() {
        Intent intent = new Intent();
        intent.setClass(this, ControlPanelActivity.class);
        startActivity(intent);
        // ask others to quit
        intent = new Intent(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        sendBroadcast(intent);
        finish();
    }
}
