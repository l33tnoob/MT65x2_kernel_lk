package com.mediatek.ppl.ui;

import java.util.List;

import android.app.Activity;
import android.app.ActivityManager;
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
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;
import com.mediatek.ppl.ui.ChooseSimDialogFragment.ISendMessage;
import com.mediatek.ppl.ui.ConfirmTurnoffAirplaneModeDialogFragment.ITurnOffAirplaneMode;

/**
 * Screen shows instruction descriptions and provides a "Send" button to user to send the descriptions to emergency
 * contacts. It will quit if SIM is plugged out or user lock the phone via power key.
 */
public class ViewInstructionActivity extends Activity implements ISendMessage, ITurnOffAirplaneMode {
    private static final String TAG = "PPL/ViewInstructionActivity";

    private ProgressBar mProgressBar;
    private View mContentView;
    private CheckBox mSendMessage;
    private Button mConfirmButton;
    private TextView mManualContent;
    private InternalControllerBinder mBinder;
    private ServiceConnection mServiceConnection;
    private EventReceiver mEventReceiver;

    private class EventReceiver extends BroadcastReceiver {
        public EventReceiver() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(PplService.Intents.UI_NO_SIM);
            intentFilter.addAction(Intent.ACTION_SCREEN_OFF);
            registerReceiver(this, intentFilter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            if (PplService.Intents.UI_NO_SIM.equals(intent.getAction())) {
                finish();
            } else if (Intent.ACTION_SCREEN_OFF.equals(intent.getAction())) {
                finish();
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.view_instruction);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.view_instruction_content);
        mContentView.setVisibility(View.GONE);
        mManualContent = (TextView) findViewById(R.id.view_instruction_instructions);
        mManualContent.setText(Html.fromHtml(getString(R.string.content_setup_send_manual)));
        mConfirmButton = (Button) findViewById(R.id.view_instruction_confirm_button);
        mConfirmButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                confirm();
            }
        });
        mSendMessage = (CheckBox) findViewById(R.id.view_instruction_send_sms_checkbox);
        mSendMessage.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mConfirmButton.setText(R.string.button_send_sms);
                } else {
                    mConfirmButton.setText(R.string.button_finish);
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
                mBinder.registerSensitiveActivity(ViewInstructionActivity.this);
                mProgressBar.setVisibility(View.GONE);
                mContentView.setVisibility(View.VISIBLE);
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                mBinder = null;
                finish();
            }
        };
        bindService(intent, mServiceConnection, BIND_AUTO_CREATE);

        mEventReceiver = new EventReceiver();
    }

    @Override
    public void onBackPressed() {
        goToControlPanel();
    }

    @Override
    public void onStop() {
        super.onStop();
        ActivityManager am = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> taskInfo = am.getRunningTasks(5);
        int taskId = getTaskId();
        Log.i(TAG, "Our Task Id: " + taskId);
        int currentTaskId = -1;
        if (taskInfo.size() > 0) {
            currentTaskId = taskInfo.get(0).id;
        }
        Log.i(TAG, "Current Task Id: " + taskId);
        if (currentTaskId != taskId) {
            finish();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            goToControlPanel();
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void onDestroy() {
        mEventReceiver.destroy();

        if (mBinder != null) {
            mBinder.finishEdit(PplManager.ACTION_KEEP);
            mBinder.unregisterSensitiveActivity(this);
        }
        if (mServiceConnection != null) {
            unbindService(mServiceConnection);
        }
        super.onDestroy();
    }

    private void confirm() {
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
                    mBinder.sendInstructionDescriptionMessage(insertedSim[0]);
                    goToControlPanel();
                } else {
                    Toast toast = Toast.makeText(this, R.string.toast_no_sim, Toast.LENGTH_SHORT);
                    toast.show();
                }
            }
        } else {
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
        mBinder.sendInstructionDescriptionMessage(simId);
        goToControlPanel();
    }

    private void goToControlPanel() {
        Intent intent = new Intent();
        intent.setClass(this, ControlPanelActivity.class);
        startActivity(intent);
        finish();
    }
}
