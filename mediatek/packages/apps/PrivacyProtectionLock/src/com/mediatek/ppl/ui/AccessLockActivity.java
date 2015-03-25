package com.mediatek.ppl.ui;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Toast;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ProgressBar;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;

/**
 * Guarding screen of PPL settings functionality. User has to enter the correct password on this screen to change the
 * settings after PPL is enabled. It will quit if the SIM card is plugged out.
 */
public class AccessLockActivity extends Activity {
    private static final String TAG = "PPL/AccessLockActivity";

    public static final String EXTRA_ENABLE_PREVIOUS = "enable_previous";

    private ProgressBar mProgressBar;
    private View mContentView;
    private CheckBox mShowPassword;
    private EditText mPassword;
    private Button mConfirmButton;
    private InternalControllerBinder mBinder;
    private boolean mEnablePreviousConfiguration;
    private ServiceConnection mServiceConnection = null;
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
        mEnablePreviousConfiguration = getIntent().getBooleanExtra(EXTRA_ENABLE_PREVIOUS, false);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.access_lock);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.access_lock_content);
        mContentView.setVisibility(View.GONE);
        mPassword = (EditText) findViewById(R.id.access_lock_password);
        mPassword.addTextChangedListener(mTextWatcher);
        mPassword.requestFocus();
        mShowPassword = (CheckBox) findViewById(R.id.access_lock_checkbox_show_password);
        mShowPassword.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                int newLocation = mPassword.getSelectionStart() == mPassword.getSelectionEnd() ?
                        mPassword.getSelectionStart() :
                        mPassword.getText().length();
                if (isChecked) {
                    mPassword.setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_VARIATION_NORMAL);
                } else {
                    mPassword.setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_VARIATION_PASSWORD);
                }
                mPassword.setSelection(newLocation);
            }
        });
        mConfirmButton = (Button) findViewById(R.id.access_lock_confirm_button);
        mConfirmButton.setEnabled(false);
        mConfirmButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                goToNext();
            }
        });

        Intent intent = new Intent(PplService.Intents.PPL_MANAGER_SERVICE);
        intent.setClass(this, PplService.class);
        intent.setPackage("com.mediatek.ppl");
        mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                mProgressBar.setVisibility(View.GONE);
                mContentView.setVisibility(View.VISIBLE);
                mBinder = (InternalControllerBinder) binder;
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
            if (mPassword.getText().length() > 0) {
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
        mEventReceiver.destroy();
        if (mServiceConnection != null) {
            unbindService(mServiceConnection);
        }
        super.onDestroy();
    }

    private void goToNext() {
        if (mBinder.verifyPassword(mPassword.getText().toString())) {
            Log.w(TAG, "Password is correct. Launch control panel.");
            if (mEnablePreviousConfiguration) {
                mEnablePreviousConfiguration = false;
                mBinder.enable(false);
            }
            Intent intent = new Intent();
            intent.setClass(this, ControlPanelActivity.class);
            startActivity(intent);
            finish();
        } else {
            Log.w(TAG, "Password is incorrect.");
            Toast toast = Toast.makeText(this, R.string.toast_password_is_incorrect, Toast.LENGTH_SHORT);
            toast.show();
        }
    }

}
