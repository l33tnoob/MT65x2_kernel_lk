package com.mediatek.rcse.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.R;


/**
 * @author MTK33296
 * This Class creates a dialog alert to show when users first time boot the phone
 * and register is failed on wifi because configuration/registration only allowed with sim
 * on first time.
 *
 */
public class RegistrationStatusActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Activity to show alert dialog for RCSE register failure at bootup first time
        super.onCreate(savedInstanceState);
        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
        // set title
        alertDialogBuilder.setTitle(R.string.register_fail_wifi_title);
        // set dialog message
        alertDialogBuilder.setMessage(R.string.register_fail_wifi_description)
                .setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // if this button is clicked, close current activity
                        RegistrationStatusActivity.this.finish();
                    }
                });
        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();
        // show it
        alertDialog.show();
    }
}

