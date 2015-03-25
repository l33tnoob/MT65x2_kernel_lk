package com.mediatek.engineermode.hqanfc;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.nfc.NfcAdapter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;

import java.io.IOException;

public class NfcMainPage extends PreferenceActivity {

    public static final String TAG = "EM/HQA/NFC";
    private static final String START_LIB_COMMAND = "./system/xbin/nfcstackp";
    private static final String KILL_LIB_COMMAND = "kill -9 nfcstackp";
    private static final int DIALOG_WARN = 1;
    private ConnectServerTask mTask;
    private boolean mShowDialog = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.hqa_nfc_main);
//        PreferenceScreen screen = getPreferenceScreen();
//        int count = screen.getPreferenceCount();
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp != null && adp.isEnabled()) {
            showDialog(DIALOG_WARN);
            mShowDialog = true;
            return;
        }
        // for (int index = 0; index < count; index++) {
        // screen.getPreference(index).setEnabled(false);
        // }
        // closeNFCServiceAtStart();
        executeXbinFile(START_LIB_COMMAND, 500);
        mTask = new ConnectServerTask();
        mTask.execute();
    }

    protected void onDestroy() {
        Elog.i(TAG, "[NfcMainPage]Nfc main page onDestroy().");
        if(mShowDialog == false) {
            NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_STOP_CMD, null);
            NfcClient.getInstance().closeConnection();
            // try {
            // Thread.sleep(3000);
            // } catch (InterruptedException e) {
            // Elog.e(TAG, "[NfcMainPage]onDestroy InterruptedException: " + e.getMessage());
            // }
            // executeXbinFile(KILL_LIB_COMMAND, 100);
            mTask.cancel(true);
        }
        super.onDestroy();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        switch (id) {
        case DIALOG_WARN:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.hqa_nfc_dialog_warn);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.hqa_nfc_dialog_warn_message));
            builder.setPositiveButton(android.R.string.ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
            break;
        default:
            Elog.d(TAG, "error dialog ID");
            break;
        }
        return dialog;
    }

    private void closeNFCServiceAtStart() {
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp.isEnabled()) {
            if (adp.disable()) {
                Elog.i(TAG, "[NfcMainPage]Nfc service set off.");
            } else {
                Elog.i(TAG, "[NfcMainPage]Nfc service set off Fail.");
            }
        } else {
            Elog.i(TAG, "[NfcMainPage]Nfc service is off");
        }
    }

    private void executeXbinFile(final String command, int sleepTime) {
        new Thread() {
            @Override
            public void run() {
                Elog.d(TAG, "[NfcMainPage]nfc command:" + command);
                try {
                    int err = ShellExe.execCommand(command);
                    Elog.d(TAG, "[NfcMainPage]nfc command:result: " + err);
                } catch (IOException e) {
                    Elog.e(TAG, "[NfcMainPage]executeXbinFile IOException: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }.start();
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Elog.e(TAG, "[NfcMainPage]executeXbinFile InterruptedException: " + e.getMessage());
        }
    }

    private class ConnectServerTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            return NfcClient.getInstance().createConnection(NfcMainPage.this);
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if (null != result && result.booleanValue()) {
                PreferenceScreen screen = getPreferenceScreen();
                int count = screen.getPreferenceCount();
                for (int index = 0; index < count; index++) {
                    screen.getPreference(index).setEnabled(true);
                }
                NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_START_CMD, null);
            } else {
                Toast.makeText(NfcMainPage.this, R.string.hqa_nfc_connect_fail, Toast.LENGTH_SHORT).show();
                // NfcMainPage.this.finish();
            }
        }
    }

    // TODO: remove "\\"
}
