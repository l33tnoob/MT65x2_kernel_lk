package com.mediatek.engineermode.lte;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.storage.StorageManagerEx;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.ArrayList;

public class CommandTool extends PreferenceActivity {
    private static final String TAG = "Tool";
    private static final int DIALOG_CHOOSE_CONFIG_FILE = 1;
    private static final String PREF = "command_tool";
    private static final String PREF_KEY = "config_file";
    private static final String DEFAULT_CONFIG_FILE = "/config.txt";

    private String mConfigFilePath = "";
    private HashMap<Preference, ArrayList<String>> mTestItems = new HashMap<Preference, ArrayList<String>>();
    private Toast mToast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PreferenceScreen preferenceScreen = getPreferenceManager().createPreferenceScreen(this);
        setPreferenceScreen(preferenceScreen);
        startService(new Intent(this, CommandToolService.class));

        SharedPreferences pref = getSharedPreferences(PREF, MODE_PRIVATE);
        mConfigFilePath = pref.getString(PREF_KEY, StorageManagerEx.getDefaultPath() + DEFAULT_CONFIG_FILE);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        parse();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, Menu.FIRST, 0, "Choose config file");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
        case Menu.FIRST:
            showDialog(DIALOG_CHOOSE_CONFIG_FILE);
        default:
            break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }

    @Override
    public Dialog onCreateDialog(int id) {
        if (id == DIALOG_CHOOSE_CONFIG_FILE) {
            final EditText input = new EditText(this);
            input.setText(mConfigFilePath);
            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        mConfigFilePath = input.getText().toString().trim();
                        SharedPreferences pref = CommandTool.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        pref.edit().putString(PREF_KEY, mConfigFilePath).commit();
                        parse();
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Config file:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        }
        return super.onCreateDialog(id);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        Intent intent = new Intent(this, CommandToolEdit.class);
        intent.putExtra("commands", mTestItems.get(preference));
        startActivity(intent);
        return true;
    }

    private void parse() {
        FileInputStream inputStream = null;
        try {
            File f = new File(mConfigFilePath);
            inputStream = new FileInputStream(f);
            doParse(new BufferedReader(new InputStreamReader(inputStream)));
            inputStream.close();
        } catch (FileNotFoundException e) {
            showToast("Config file not found.");
            return;
        } catch (IOException e) {
            showToast("Read config file error.");
            return;
        }
    }

    public void doParse(BufferedReader in) throws IOException {
        getPreferenceScreen().removeAll();
        mTestItems.clear();

        PreferenceCategory category = new PreferenceCategory(this);
        Preference pref = new Preference(this);
        ArrayList<String> cmds = new ArrayList<String>();

        String line = in.readLine();
        while (line != null) {
            if (!line.startsWith(" ")) {
                category = new PreferenceCategory(this);
                category.setTitle(line);
                getPreferenceScreen().addPreference(category);
            } else {
                line = line.substring(1);
                if (!line.startsWith(" ")) {
                    cmds = new ArrayList<String>();
                    pref = new Preference(this);
                    pref.setTitle(line);
                    category.addPreference(pref);
                    mTestItems.put(pref, cmds);
                } else {
                    cmds.add(line.trim());
                }
            }
            line = in.readLine();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    };

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

