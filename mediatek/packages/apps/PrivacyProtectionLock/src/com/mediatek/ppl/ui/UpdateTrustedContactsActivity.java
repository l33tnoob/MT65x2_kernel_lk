package com.mediatek.ppl.ui;

import java.util.LinkedList;
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
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.provider.ContactsContract;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.PplManager;
import com.mediatek.ppl.PplManager.ContactQueryResult;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.PplService.InternalControllerBinder;
import com.mediatek.ppl.R;
import com.mediatek.ppl.ui.ChoosePhoneNumberDialogFragment.IUpdateNumber;
import com.mediatek.ppl.ui.SlidingPanelLayout.ILayoutPolicyChanged;

/**
 * This activity implements the edit interface of trusted contacts.
 */
public class UpdateTrustedContactsActivity extends Activity implements IUpdateNumber {
    private static final String TAG = "PPL/UpdateTrustedContactsActivity";
    private static final String KEY_FOCUS = "focus";
    private static final String KEY_NUMBERS = "numbers";
    private static final String KEY_NAMES = "names";
    private static final String KEY_EDIT_BUFFERS = "edit_buffers";
    private static final String KEY_SHOW_DELS = "show_dels";
    private static final String KEY_SHOW_LINES = "show_lines";
    private static final String KEY_SHOW_ADD_LINE = "show_add_line";

    private ProgressBar mProgressBar;
    private View mContentView;
    private SlidingPanelLayout mSlidingPanel;
    private Button mConfirmButton;
    private Button mAddContactLineButton;
    private View mUpperPanel;
    private LinearLayout mLowerPanel;
    private TextView mTitleText;
    private LinearLayout mParentLayout;
    private LinkedList<ContactLine> mContactLines = new LinkedList<ContactLine>();
    private boolean mSuppressTextChangeCallback = false;

    private InternalControllerBinder mBinder;
    ControlData mEditBuffer;
    private ServiceConnection mServiceConnection;
    private EventReceiver mEventReceiver;
    private PendingActivityResult mPendingActivityResult;

    private void savePendingActivityResult(int requestCode, int resultCode, Intent data) {
        if (mPendingActivityResult != null) {
            throw new Error("mPendingActivityResult is not null, check for bug");
        } else {
            mPendingActivityResult = new PendingActivityResult(requestCode, resultCode, data);
        }
    }

    private void processPendingActivityResult() {
        if (mPendingActivityResult != null) {
            if (mPendingActivityResult.resultCode == RESULT_OK) {
                Uri contactURI = mPendingActivityResult.data.getData();
                ContactQueryResult info = PplManager.getContactInfo(this, contactURI);
                int i = 0;
                for (ContactLine c : mContactLines) {
                    if (c.line.getId() == mPendingActivityResult.requestCode) {
                        break;
                    }
                    i += 1;
                }
                if (info.phones.size() == 0) {
                    Toast toast = Toast.makeText(UpdateTrustedContactsActivity.this, R.string.toast_no_phone_number, Toast.LENGTH_SHORT);
                    toast.show();
                } else if (info.phones.size() == 1) {
                    changeNumber(info.phones.get(0), info.name, i);
                } else {
                    Log.d(TAG, "Multiple Phone Numbers: " + info.phones);
                    FragmentTransaction ft = getFragmentManager().beginTransaction();
                    DialogFragment newFragment = new ChoosePhoneNumberDialogFragment();
                    Bundle args = new Bundle();
                    args.putStringArray(ChoosePhoneNumberDialogFragment.ARG_KEY_ITEMS, info.phones.toArray(new String[0]));
                    args.putInt(ChoosePhoneNumberDialogFragment.ARG_KEY_LINE_INDEX, i);
                    args.putString(ChoosePhoneNumberDialogFragment.ARG_KEY_NAME, info.name);
                    newFragment.setArguments(args);
                    newFragment.show(ft, "choose_number_dialog");
                }
            }
            mPendingActivityResult = null;
        } else {
            Log.d(TAG, "No pending activity result");
        }
    }

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

    private class ContactLine {
        public LinearLayout line;
        public EditText edit;
        public ImageButton add;
        public ImageButton del;
        public String number;
        public String name;

        public ContactLine(LayoutInflater inflater) {
            number = null;
            name = null;
            line = (LinearLayout) inflater.inflate(R.layout.contact_line, null);
            edit = (EditText) line.findViewById(R.id.trusted_contact_line_edit);
            add = (ImageButton) line.findViewById(R.id.trusted_contact_line_add);
            add.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent intent = new Intent(Intent.ACTION_PICK, ContactsContract.Contacts.CONTENT_URI);
                    startActivityForResult(intent, line.getId());
                }
            });
            del = (ImageButton) line.findViewById(R.id.trusted_contact_line_delete);
            del.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    clearContent();
                    int visibleCount = getVisibleContactLineCount();
                    if (visibleCount == 1) {
                        hideDel();
                        edit.setFocusable(true);
                        edit.setFocusableInTouchMode(true);
                        mAddContactLineButton.setVisibility(View.GONE);
                    } else {
                        line.setVisibility(View.GONE);
                        mLowerPanel.removeView(line);
                        mLowerPanel.addView(line, mLowerPanel.getChildCount() - 1);
                        mContactLines.remove(ContactLine.this);
                        mContactLines.addLast(ContactLine.this);
                        if (visibleCount == PplService.MAX_CONTACTS) {
                            mAddContactLineButton.setVisibility(View.VISIBLE);
                        }
                    }
                }
            });
            edit.addTextChangedListener(new TextWatcher() {

                @Override
                public void afterTextChanged(Editable s) {
                    UpdateTrustedContactsActivity.this.afterTextChanged(ContactLine.this, s);
                }

                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }

                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }

            });
            edit.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d(TAG, "Edit Text " + edit.hashCode() + " clicked.");
                    if (!edit.isFocusable()) {
                        if (name != null) {
                            suppressTextChangeCallbackRun(new Runnable() {
                                @Override
                                public void run() {
                                    edit.setText(number);
                                }
                            });
                        }
                        edit.setFocusable(true);
                        edit.setFocusableInTouchMode(true);
                        edit.requestFocus();
                        edit.requestFocusFromTouch();
                        edit.setSelection(edit.getText().length());
                    }
                }
            });
            edit.setOnFocusChangeListener(new OnFocusChangeListener() {
                @Override
                public void onFocusChange(View v, boolean hasFocus) {
                    if (!hasFocus) {
                        if (number != null && number.length() > 0 && name == null) {
                            name = PplManager.getContactNameByPhoneNumber(UpdateTrustedContactsActivity.this, number);
                        }
                        suppressTextChangeCallbackRun(new Runnable() {
                            @Override
                            public void run() {
                                edit.setText(buildDisplayText(number, name));
                            }
                        });
                        if (name != null) {
                            edit.setFocusable(false);
                            edit.setFocusableInTouchMode(false);
                        }
                    } else {
                        if (name != null) {
                            suppressTextChangeCallbackRun(new Runnable() {
                                @Override
                                public void run() {
                                    edit.setText(number);
                                }
                            });
                        }
                    }
                }
            });
        }

        public ContactLine show() {
            line.setVisibility(View.VISIBLE);
            return this;
        }

        public ContactLine hide() {
            line.setVisibility(View.GONE);
            return this;
        }

        public ContactLine showDel() {
            del.setVisibility(View.VISIBLE);
            return this;
        }

        public ContactLine hideDel() {
            del.setVisibility(View.INVISIBLE);
            return this;
        }

        public boolean isVisible() {
            return line.getVisibility() == View.VISIBLE;
        }

        public boolean isDelVisible() {
            return del.getVisibility() == View.VISIBLE;
        }

        public void clearContent() {
            number = null;
            name = null;
            edit.setText("");
        }
    }

    private static String buildDisplayText(String number, String name) {
        if (name != null) {
            return number + " (" + name + ")";
        } else {
            return number;
        }
    }

    private int getVisibleContactLineCount() {
        int result = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.isVisible()) {
                result += 1;
            }
        }
        return result;
    }

    private void clearFocus() {
        for (ContactLine cl : mContactLines) {
            if (cl.isVisible()) {
                cl.edit.clearFocus();
            }
        }
    }

    private void afterTextChanged(ContactLine cl, Editable s) {
        Log.d(TAG, "afterTextChanged: " + cl.line.getId() + ", " + s.toString() + ", " + mSuppressTextChangeCallback);
        if (!mSuppressTextChangeCallback) {
            cl.number = s.toString();
            cl.name = null;
        }

        if (getVisibleContactLineCount() == 1 && s.length() > 0) {
            cl.showDel();
            mAddContactLineButton.setVisibility(View.VISIBLE);
        }
        boolean enableNextButton = false;
        for (ContactLine c : mContactLines) {
            if (c.isVisible() && c.number != null && c.number.length() > 0) {
                enableNextButton = true;
                break;
            }
        }
        mConfirmButton.setEnabled(enableNextButton);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult: " + requestCode + ", " + resultCode);
        savePendingActivityResult(requestCode, resultCode, data);
        if (mBinder != null) {
            processPendingActivityResult();
        }
    }

    @Override
    public void changeNumber(final String number, final String name, int index) {
        final ContactLine cl = mContactLines.get(index);
        suppressTextChangeCallbackRun(new Runnable() {
            @Override
            public void run() {
                cl.number = number;
                cl.name = name;
                cl.edit.setText(buildDisplayText(number, name));
            }
            
        });
        cl.edit.setFocusable(false);
        cl.edit.setFocusableInTouchMode(false);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate(" + savedInstanceState + ")");
        final Bundle pendingSavedInstanceState = savedInstanceState;
        getActionBar().setDisplayHomeAsUpEnabled(true);
        setContentView(R.layout.update_trusted_contact);
        mParentLayout = (LinearLayout) findViewById(R.id.update_trusted_contact_parent);
        mTitleText = (TextView) findViewById(R.id.update_trusted_contact_title);
        mProgressBar = (ProgressBar) findViewById(R.id.loading_indicator);
        mProgressBar.setVisibility(View.VISIBLE);
        mContentView = (View) findViewById(R.id.update_trusted_contact_content);
        mContentView.setVisibility(View.GONE);
        mConfirmButton = (Button) findViewById(R.id.update_trusted_contact_confirm_button);
        mConfirmButton.setEnabled(false);
        mConfirmButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                confirm();
            }
        });

        mUpperPanel = findViewById(R.id.update_trusted_contact_upper_panel);
        mLowerPanel = (LinearLayout) findViewById(R.id.update_trusted_contact_lower_panel);
        mSlidingPanel = (SlidingPanelLayout) findViewById(R.id.update_trusted_contact_panels);
        mSlidingPanel.registerPanels(mUpperPanel, mLowerPanel, new ILayoutPolicyChanged() {
            @Override
            public void onLayoutPolicyChanged(int policy, boolean lowerClipped) {
                Log.d(TAG, "onLayoutPolicyChanged(" + policy + ")");
                if (policy == ILayoutPolicyChanged.LAYOUT_POLICY_FLOATING) {
                    mLowerPanel.setBackgroundColor(getResources().getColor(R.color.floating_panel_background));
                    mConfirmButton.setBackgroundColor(getResources().getColor(R.color.floating_panel_background));
                    if (lowerClipped) {
                        if (mTitleText.getVisibility() != View.GONE) {
                            mTitleText.setVisibility(View.GONE);
                            mParentLayout.post(new Runnable() {
                                @Override
                                public void run() {
                                    mParentLayout.requestLayout();
                                }
                            });
                        }
                    }
                } else {
                    mLowerPanel.setBackgroundColor(getResources().getColor(R.color.follow_panel_background));
                    mConfirmButton.setBackgroundColor(getResources().getColor(R.color.follow_panel_background));
                    if (mTitleText.getVisibility() != View.VISIBLE) {
                        mTitleText.setVisibility(View.VISIBLE);
                        mParentLayout.post(new Runnable() {
                            @Override
                            public void run() {
                                mParentLayout.requestLayout();
                            }
                        });
                    }
                }
            }
        });
        mSlidingPanel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                clearFocus();
            }
            
        });
        mAddContactLineButton = (Button) findViewById(R.id.update_trusted_contact_add_contact_button);
        mAddContactLineButton.setVisibility(View.GONE);
        mAddContactLineButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                int i = 0;
                for (ContactLine cl : mContactLines) {
                    if (!cl.isVisible()) {
                        cl.clearContent();
                        cl.edit.setFocusable(true);
                        cl.edit.setFocusableInTouchMode(true);
                        cl.show();
                        cl.showDel();
                        cl.edit.requestFocus();
                        cl.edit.requestFocusFromTouch();
                        break;
                    }
                    i += 1;
                }
                if (i >= PplService.MAX_CONTACTS - 1) {
                    mAddContactLineButton.setVisibility(View.GONE);
                }
            }
        });

        LayoutInflater inflater = getLayoutInflater();
        for (int i = 0; i < PplService.MAX_CONTACTS; ++i) {
            ContactLine cl = new ContactLine(inflater);
            mContactLines.addLast(cl);
            cl.line.setId(i);
            mLowerPanel.addView(cl.line, mLowerPanel.getChildCount() - 1);
        }

        Intent intent = new Intent(PplService.Intents.PPL_MANAGER_SERVICE);
        intent.setClass(this, PplService.class);
        intent.setPackage("com.mediatek.ppl");
        mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName cn, IBinder binder) {
                mBinder = (InternalControllerBinder) binder;
                mBinder.registerSensitiveActivity(UpdateTrustedContactsActivity.this);
                mBinder.loadCurrentData();
                mEditBuffer = mBinder.startEdit();
                final List<String> numbers = mEditBuffer.TrustedNumberList;
                if (pendingSavedInstanceState != null) {
                    restoreStates(pendingSavedInstanceState);
                } else if (numbers != null) {
                    // check service data
                    suppressTextChangeCallbackRun(new Runnable () {
                        @Override
                        public void run() {
                            int i = 0;
                            for (ContactLine cl : mContactLines) {
                                if (i < numbers.size()) {
                                    String number = numbers.get(i);
                                    String name = PplManager.getContactNameByPhoneNumber(UpdateTrustedContactsActivity.this, numbers.get(i));
                                    String text = buildDisplayText(number, name);
                                    cl.edit.setText(text);
                                    cl.name = name;
                                    cl.number = number;
                                    cl.showDel();
                                    cl.show();
                                    cl.edit.setFocusable(false);
                                    cl.edit.setFocusableInTouchMode(false);
                                } else {
                                    cl.hide();
                                }
                                i += 1;
                            }
                        }
                    });
                    if (numbers.size() < PplService.MAX_CONTACTS) {
                        mAddContactLineButton.setVisibility(View.VISIBLE);
                    }
                } else {
                    mAddContactLineButton.setVisibility(View.GONE);
                    int i = 0;
                    for (ContactLine cl : mContactLines) {
                        if (i == 0) {
                            cl.hideDel().show();
                            cl.edit.requestFocus();
                            cl.edit.requestFocusFromTouch();
                        } else {
                            cl.hide();
                        }
                        i += 1;
                    }
                }
                mProgressBar.setVisibility(View.GONE);
                mContentView.setVisibility(View.VISIBLE);
                processPendingActivityResult();
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

    private String[] buildNumberList() {
        String[] result = new String[mContactLines.size()];
        int i = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.isVisible()) {
                result[i] = cl.number;
            } else {
                result[i] = null;
            }
            i += 1;
        }
        return result;
    }

    private String[] buildEditBufferList() {
        String[] result = new String[mContactLines.size()];
        int i = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.isVisible()) {
                result[i] = cl.edit.getText().toString();
            } else {
                result[i] = null;
            }
            i += 1;
        }
        return result;
    }

    private String[] buildNameList() {
        String[] result = new String[mContactLines.size()];
        int i = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.isVisible()) {
                result[i] = cl.name;
            } else {
                result[i] = null;
            }
            i += 1;
        }
        return result;
    }

    private boolean[] buildShowDelList() {
        boolean[] result = new boolean[mContactLines.size()];
        int i = 0;
        for (ContactLine cl : mContactLines) {
            result[i] = cl.isDelVisible();
            i += 1;
        }
        return result;
    }

    private boolean[] buildShowLineList() {
        boolean[] result = new boolean[mContactLines.size()];
        int i = 0;
        for (ContactLine cl : mContactLines) {
            result[i] = cl.isVisible();
            i += 1;
        }
        return result;
    }
    
    private int getLineFocus() {
        int i = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.edit.hasFocus()) {
                return i;
            }
            i += 1;
        }
        return -1;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.d(TAG, "onSaveInstanceState(" + outState + ")");
        outState.putInt(KEY_FOCUS, getLineFocus());
        outState.putStringArray(KEY_EDIT_BUFFERS, buildEditBufferList());
        outState.putStringArray(KEY_NUMBERS, buildNumberList());
        outState.putStringArray(KEY_NAMES, buildNameList());
        outState.putBooleanArray(KEY_SHOW_DELS, buildShowDelList());
        outState.putBooleanArray(KEY_SHOW_LINES, buildShowLineList());
        outState.putBoolean(KEY_SHOW_ADD_LINE, mAddContactLineButton.getVisibility() == View.VISIBLE);
    }

    public void restoreStates(Bundle savedInstance) {
        int lineFocus = savedInstance.getInt(KEY_FOCUS, -1);
        String[] editBuffers = savedInstance.getStringArray(KEY_EDIT_BUFFERS);
        String[] numbers = savedInstance.getStringArray(KEY_NUMBERS);
        String[] names = savedInstance.getStringArray(KEY_NAMES);
        boolean[] showDels = savedInstance.getBooleanArray(KEY_SHOW_DELS);
        boolean[] showLines = savedInstance.getBooleanArray(KEY_SHOW_LINES);
        boolean showAdd = savedInstance.getBoolean(KEY_SHOW_ADD_LINE, false);
        restoreStates(lineFocus, editBuffers, numbers, names, showDels, showLines, showAdd);
    }

    private void restoreStates(
            final int lineFocus,
            final String[] editBuffers,
            final String[] numbers,
            final String[] names,
            final boolean[] showDels,
            final boolean[] showLines,
            boolean showAdd) {
        mAddContactLineButton.setVisibility(showAdd ? View.VISIBLE : View.GONE);
        suppressTextChangeCallbackRun(new Runnable() {
            @Override
            public void run() {
                for (int i = 0; i < mContactLines.size(); ++i) {
                    ContactLine cl = mContactLines.get(i);
                    if (showDels[i]) {
                        cl.showDel();
                    } else {
                        cl.hideDel();
                    }
                    cl.edit.setText(editBuffers[i]);
                    cl.number = numbers[i];
                    cl.name = names[i];
                    cl.show();
                    if (showLines[i]) {
                        cl.show();
                    } else {
                        cl.hide();
                    }
                    if (i == lineFocus) {
                        cl.edit.setFocusable(true);
                        cl.edit.setFocusableInTouchMode(true);
                    } else {
                        if (cl.name == null) {
                            cl.edit.setFocusable(false);
                            cl.edit.setFocusableInTouchMode(false);
                        } else {
                            cl.edit.setFocusable(true);
                            cl.edit.setFocusableInTouchMode(true);
                        }
                        cl.edit.clearFocus();
                    }
                }
                if (lineFocus != -1) {
                    mContactLines.get(lineFocus).edit.requestFocus();
                    mContactLines.get(lineFocus).edit.requestFocusFromTouch();
                }
            }
        });
    }
    
    private void suppressTextChangeCallbackRun(Runnable r) {
        mSuppressTextChangeCallback = true;
        r.run();
        mSuppressTextChangeCallback = false;
    }

    private void confirm() {
        if (mEditBuffer.TrustedNumberList == null) {
            mEditBuffer.TrustedNumberList = new LinkedList<String>();
        } else {
            mEditBuffer.TrustedNumberList.clear();
        }

        for (ContactLine cl : mContactLines) {
            if (cl.isVisible() && cl.number != null && cl.number.length() > 0) {
                mEditBuffer.TrustedNumberList.add(cl.number);
            }
        }
        mBinder.finishEdit(PplManager.ACTION_COMMIT);
        goToControlPanel();
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

}
