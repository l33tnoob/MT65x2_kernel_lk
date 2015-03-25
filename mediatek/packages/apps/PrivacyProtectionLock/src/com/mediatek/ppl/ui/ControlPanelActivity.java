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
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;

/**
 * Control panel screen of PPL. It will quit if SIM is plugged out or user lock the phone via power key.
 */
public class ControlPanelActivity extends Activity {
    private static final String TAG = "PPL/ControlPanelActivity";
    private ProgressBar mProgressBar;
    private View mContentView;
    private Button mChangePasswordButton;
    private Button mUpdateContactsButton;
    private Button mViewInstructionButton;
    private Button mDisableButton;
    private TextView mStatusLabel;
    private TextView mContactLabels[] = new TextView[3];
    private InternalControllerBinder mBinder;
    private ServiceConnection mServiceConnection = null;
    private ControlData mEditBuffer = null;
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
        Log.d(TAG, "onCreate(" + savedInstanceState + ")");
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.control_panel);

        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.control_panel_content);
        mContentView.setVisibility(View.GONE);

        mStatusLabel = (TextView) findViewById(R.id.control_panel_enable_status);

        mContactLabels[0] = (TextView) findViewById(R.id.control_panel_contact_1);
        mContactLabels[1] = (TextView) findViewById(R.id.control_panel_contact_2);
        mContactLabels[2] = (TextView) findViewById(R.id.control_panel_contact_3);

        mChangePasswordButton = (Button) findViewById(R.id.control_panel_change_password_button);
        mChangePasswordButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                gotoChangePassword();
            }
        });

        mUpdateContactsButton = (Button) findViewById(R.id.control_panel_update_emergency_contacts_button);
        mUpdateContactsButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                gotoUpdateContacts();
            }
        });

        mViewInstructionButton = (Button) findViewById(R.id.control_panel_view_instructions_button);
        mViewInstructionButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                gotoViewInstructions();
            }
        });

        mDisableButton = (Button) findViewById(R.id.control_panel_disable_button);
        mDisableButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                gotoDisable();
            }
        });

        Intent intent = new Intent(PplService.Intents.PPL_MANAGER_SERVICE);
        intent.setClass(this, PplService.class);
        intent.setPackage("com.mediatek.ppl");
        mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                mBinder = (InternalControllerBinder) binder;
                mBinder.registerSensitiveActivity(ControlPanelActivity.this);
                updateStatus();
                // Show Views
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
    public void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
        if (mBinder != null) {
            updateStatus();
        }
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop()");
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

    private void updateStatus() {
        mBinder.loadCurrentData();
        mEditBuffer = mBinder.startEdit();
        // Setup Views
        mStatusLabel.setText(mEditBuffer.isEnabled() ? R.string.status_control_panel_enabled : R.string.status_control_panel_disabled);
        List<String> contacts = mEditBuffer.TrustedNumberList;
        for (int i = 0; i < mContactLabels.length; ++i) {
            if (contacts != null && i < contacts.size()) {
                mContactLabels[i].setText(buildContactLabel(contacts.get(i)));
                mContactLabels[i].setVisibility(View.VISIBLE);
            } else {
                mContactLabels[i].setVisibility(View.GONE);
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()");
        mEventReceiver.destroy();

        if (mBinder != null) {
            mBinder.finishEdit(PplManager.ACTION_CLEAR);
            mBinder.unregisterSensitiveActivity(this);
        }
        if (mServiceConnection != null) {
            unbindService(mServiceConnection);
        }
        super.onDestroy();
    }

    private void gotoChangePassword() {
        Intent intent = new Intent();
        intent.setClass(this, ChangePasswordActivity.class);
        startActivity(intent);
    }

    private void gotoUpdateContacts() {
        Intent intent = new Intent();
        intent.setClass(this, UpdateTrustedContactsActivity.class);
        startActivity(intent);
    }

    private void gotoViewInstructions() {
        Intent intent = new Intent();
        intent.setClass(this, ViewInstructionActivity.class);
        startActivity(intent);
    }

    private void gotoDisable() {
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        // Create and show the dialog.
        DialogFragment newFragment = new ConfirmDisableDialogFragment();
        Bundle args = new Bundle();
        newFragment.setArguments(args);
        newFragment.show(ft, "dialog");
    }

    void executeDisable() {
        mBinder.disable();
        finish();
    }

    private String buildContactLabel(String number) {
        String name = PplManager.getContactNameByPhoneNumber(this, number);
        if (name != null) {
            return name + " " + number;
        } else {
            return number;
        }
    }
}
