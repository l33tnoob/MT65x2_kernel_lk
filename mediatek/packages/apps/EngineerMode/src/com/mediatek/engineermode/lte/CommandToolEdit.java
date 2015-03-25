package com.mediatek.engineermode.lte;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

public class CommandToolEdit extends Activity implements OnClickListener, ServiceConnection, CommandToolService.OnUpdateResultListener {
    private static final String TAG = "Tool";

    private EditText mEditorCommands;
    private EditText mEditorInterval;
    private TextView mTextView;
    private Button mButtonSend;
    private Button mButtonStop;
    private Toast mToast;
    private CommandToolService mService;

    ArrayList<String> mCommands = new ArrayList<String>();
    private int mDefaultInteval = 1;
    private int mInterval = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_tool_edit);

        mEditorCommands = (EditText) findViewById(R.id.commands);
        mEditorCommands.setEnabled(false);
        mEditorInterval = (EditText) findViewById(R.id.inteval);
        mEditorInterval.setEnabled(false);
        mTextView = (TextView) findViewById(R.id.output);
        mButtonSend = (Button) findViewById(R.id.send);
        mButtonSend.setOnClickListener(this);
        mButtonSend.setEnabled(false);
        mButtonStop = (Button) findViewById(R.id.stop);
        mButtonStop.setOnClickListener(this);
        mButtonStop.setEnabled(false);

        mEditorInterval.setText(String.valueOf(mDefaultInteval));
        mTextView.setText("");

        String text = "";
        ArrayList<String> commands = getIntent().getStringArrayListExtra("commands");
        if (commands != null) {
            for (String cmd : commands) {
                text += (commands.get(0) == cmd ? "" : "\n") + cmd;
            }
        }
        mEditorCommands.setText(text);
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSend) {
            if (parseEditors()) {
                mService.startTest(mCommands, mInterval);
            }
        } else if (v == mButtonStop) {
            mService.stopTest();
        }
    }

    private boolean parseEditors() {
        String text = mEditorCommands.getText().toString();
        if (text == null) {
            return false;
        }
        String[] commands = text.split("\n");
        if (commands == null || commands.length < 1) {
            return false;
        }

        mCommands.clear();
        for (int i = 0; i < commands.length; i++) {
            if (commands[i].trim().length() > 0) {
                mCommands.add(commands[i].trim());
            }
        }

        try {
            mInterval = Integer.parseInt(mEditorInterval.getText().toString().trim());
        } catch (NumberFormatException e) {
            mInterval = mDefaultInteval;
            mEditorInterval.setText(String.valueOf(mDefaultInteval));
        }
        return true;
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, CommandToolService.class), this, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        unbindService(this);
        super.onStop();
    }

    @Override
    public void onServiceConnected(ComponentName name, IBinder service) {
        Xlog.v(TAG, "Enter onServiceConnected");
        mService = ((CommandToolService.CommandToolServiceBinder) service).getService();
        mService.setOnUpdateResultListener(this);
        Xlog.v(TAG, "onServiceConnected " + mService.isRunning());
        onUpdateResult();
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        Xlog.v(TAG, "Enter onServiceDisconnected");
        mService = null;
    }

    @Override
    public void onUpdateResult() {
        mTextView.setText(mService.getOutput());
        mButtonSend.setEnabled(!mService.isRunning());
        mButtonStop.setEnabled(mService.isRunning());
        mEditorCommands.setEnabled(!mService.isRunning());
        mEditorInterval.setEnabled(!mService.isRunning());
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

