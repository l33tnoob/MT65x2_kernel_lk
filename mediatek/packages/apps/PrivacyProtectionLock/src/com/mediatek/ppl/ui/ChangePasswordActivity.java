package com.mediatek.ppl.ui;

import java.util.List;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;

/**
 * Password editing screen for user to modify the password after PPL is configured. It will quit if SIM is plugged out
 * or user lock the phone via power key.
 */
public class ChangePasswordActivity extends Activity {
    private static final String TAG = "PPL/ChangePasswordActivity";
    private static final String STATE_KEY_FIRST_INPUT = "first_input";
    private static final String STATE_KEY_SECOND_INPUT = "second_input";

    private ProgressBar mProgressBar;
    private View mContentView;
    private Button mConfirmButton;
    private EditText mFirstPassword;
    private EditText mSecondPassword;
    private InternalControllerBinder mBinder;
    ControlData mEditBuffer;
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
        setContentView(R.layout.change_password);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.change_password_content);
        mContentView.setVisibility(View.GONE);
        mConfirmButton = (Button) findViewById(R.id.change_password_confirm_button);
        mConfirmButton.setEnabled(false);
        mConfirmButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                confirm();
            }
        });
        mFirstPassword = (EditText) findViewById(R.id.change_password_edit_password_1);
        mFirstPassword.addTextChangedListener(mTextWatcher);
        mFirstPassword.requestFocus();
        mSecondPassword = (EditText) findViewById(R.id.change_password_edit_password_2);
        mSecondPassword.addTextChangedListener(mTextWatcher);
        if (savedInstanceState != null) {
            String first_input = savedInstanceState.getString(STATE_KEY_FIRST_INPUT);
            String second_input = savedInstanceState.getString(STATE_KEY_SECOND_INPUT);
            mFirstPassword.setText(first_input);
            mSecondPassword.setText(second_input);
        }

        Intent intent = new Intent(PplService.Intents.PPL_MANAGER_SERVICE);
        intent.setClass(this, PplService.class);
        intent.setPackage("com.mediatek.ppl");
        mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                mBinder = (InternalControllerBinder) binder;
                mBinder.registerSensitiveActivity(ChangePasswordActivity.this);
                mBinder.loadCurrentData();
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

    private TextWatcher mTextWatcher = new TextWatcher() {
        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (mFirstPassword.getText().length() >= PplManager.MIN_PASSWORD_LENGTH
                    && mSecondPassword.getText().length() >= PplManager.MIN_PASSWORD_LENGTH) {
                mConfirmButton.setEnabled(true);
            } else {
                mConfirmButton.setEnabled(false);
            }
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void afterTextChanged(Editable s) {
        }
    };

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

    private void goToControlPanel() {
        Intent intent = new Intent();
        intent.setClass(this, ControlPanelActivity.class);
        startActivity(intent);
        finish();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putString(STATE_KEY_FIRST_INPUT, mFirstPassword.getText().toString());
        outState.putString(STATE_KEY_SECOND_INPUT, mSecondPassword.getText().toString());
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (savedInstanceState != null) {
            String first_input = savedInstanceState.getString(STATE_KEY_FIRST_INPUT);
            String second_input = savedInstanceState.getString(STATE_KEY_SECOND_INPUT);
            mFirstPassword.setText(first_input);
            mSecondPassword.setText(second_input);
        }
    }

    private void confirm() {
        if (mFirstPassword.getText().length() < PplManager.MIN_PASSWORD_LENGTH) {
            // TOAST: too short
        } else if (!mFirstPassword.getText().toString().equals(mSecondPassword.getText().toString())) {
            Toast toast = Toast.makeText(this, R.string.toast_passwords_do_not_match, Toast.LENGTH_SHORT);
            toast.show();
        } else {
            byte[] salt = new byte[ControlData.SALT_SIZE];
            byte[] secret = PplManager.generateSecrets(mFirstPassword.getText().toString().getBytes(), salt);
            System.arraycopy(salt, 0, mEditBuffer.salt, 0, salt.length);
            System.arraycopy(secret, 0, mEditBuffer.secret, 0, mEditBuffer.secret.length);
            mBinder.finishEdit(PplManager.ACTION_COMMIT);
            goToControlPanel();
        }
    }

}
