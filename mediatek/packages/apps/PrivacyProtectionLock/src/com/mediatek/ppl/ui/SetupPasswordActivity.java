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
import android.text.TextWatcher;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Toast;

import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.ui.SlidingPanelLayout.ILayoutPolicyChanged;
import com.mediatek.ppl.R;

/**
 * The first step of setup wizard. It shows a screen for user to configure password of PPL.
 */
public class SetupPasswordActivity extends Activity {
    private static final String TAG = "PPL/SetupPasswordActivity";
    private static final String STATE_KEY_FIRST_INPUT = "first_input";
    private static final String STATE_KEY_SECOND_INPUT = "second_input";

    private ProgressBar mProgressBar;
    private View mContentView;
    private Button mNextButton;
    private EditText mFirstPassword;
    private EditText mSecondPassword;
    private SlidingPanelLayout mSlidingPanel;
    private View mUpperPanel;
    private LinearLayout mLowerPanel;
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
        final Intent launchIntent = getIntent();
        Log.d(TAG, "launchIntent is " + launchIntent);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.setup_password);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.setup_password_content);
        mContentView.setVisibility(View.GONE);
        mNextButton = (Button) findViewById(R.id.setup_password_next_button);
        mNextButton.setEnabled(false);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                goToNext();
            }
        });
        mUpperPanel = findViewById(R.id.setup_password_upper_panel);
        mLowerPanel = (LinearLayout) findViewById(R.id.setup_password_lower_panel);
        mSlidingPanel = (SlidingPanelLayout) findViewById(R.id.setup_password_panels);
        mSlidingPanel.registerPanels(mUpperPanel, mLowerPanel, new ILayoutPolicyChanged() {
            @Override
            public void onLayoutPolicyChanged(int policy, boolean lowerClipped) {
                Log.d(TAG, "onLayoutPolicyChanged(" + policy + ")");
                if (policy == ILayoutPolicyChanged.LAYOUT_POLICY_FLOATING) {
                    mLowerPanel.setBackgroundColor(getResources().getColor(R.color.floating_panel_background));
                    mNextButton.setBackgroundColor(getResources().getColor(R.color.floating_panel_background));
                } else {
                    mLowerPanel.setBackgroundColor(getResources().getColor(R.color.follow_panel_background));
                    mNextButton.setBackgroundColor(getResources().getColor(R.color.follow_panel_background));
                }
            }
        });
        mFirstPassword = (EditText) findViewById(R.id.setup_password_edit_password_1);
        mFirstPassword.addTextChangedListener(mTextWatcher);
        mFirstPassword.requestFocus();
        mSecondPassword = (EditText) findViewById(R.id.setup_password_edit_password_2);
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
                if (launchIntent != null &&
                    (launchIntent.getFlags() & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY) != 0 &&
                    mBinder.isEnabled()) {
                    Intent intent = new Intent();
                    intent.setClass(SetupPasswordActivity.this, AccessLockActivity.class);
                    startActivity(intent);
                    finish();
                } else {
                    mEditBuffer = mBinder.startEdit();
                    mProgressBar.setVisibility(View.GONE);
                    mContentView.setVisibility(View.VISIBLE);
                }
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
                mNextButton.setEnabled(true);
            } else {
                mNextButton.setEnabled(false);
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
        }
        if (mServiceConnection != null) {
            unbindService(mServiceConnection);
        }
        super.onDestroy();
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

    private void goToNext() {
        if (mFirstPassword.getText().length() < PplManager.MIN_PASSWORD_LENGTH) {
            // TOAST: too short
        } else if (!mFirstPassword.getText().toString().equals(mSecondPassword.getText().toString())) {
            Toast toast = Toast.makeText(this, R.string.toast_passwords_do_not_match, Toast.LENGTH_SHORT);
            toast.show();
        } else {
            mEditBuffer.clear();
            byte[] salt = new byte[ControlData.SALT_SIZE];
            byte[] secret = PplManager.generateSecrets(mFirstPassword.getText().toString().getBytes(), salt);
            System.arraycopy(salt, 0, mEditBuffer.salt, 0, salt.length);
            System.arraycopy(secret, 0, mEditBuffer.secret, 0, mEditBuffer.secret.length);
            mBinder.finishEdit(PplManager.ACTION_KEEP);
            Intent intent = new Intent();
            intent.setClass(this, SetupTrustedContactsActivity.class);
            startActivity(intent);
        }
    }
}
