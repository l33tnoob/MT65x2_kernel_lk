package com.mediatek.cts.window;

import com.mediatek.cts.window.stub.R;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.PopupMenu;

public class WindowTimeoutStubActivity extends Activity {
    public static boolean mStartTesting = false;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.windowtimeoutstub_layout);
    }
    @Override
    protected void onResume() {
        if (mStartTesting) {
            try {
                Thread.sleep(5000);
            }catch (Exception e) {}
        }
        super.onResume();
    }

    public void onPopupButtonClick(View button) {

        PopupMenu popup = new PopupMenu(this, button);
        popup.getMenuInflater().inflate(R.menu.popup, popup.getMenu());

        popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
            public boolean onMenuItemClick(MenuItem item) {
                ProgressDialog mProgressDialog;
                mProgressDialog = new ProgressDialog(WindowTimeoutStubActivity.this);
                mProgressDialog.setIconAttribute(android.R.attr.alertDialogIcon);
                mProgressDialog.setTitle(R.string.select_dialog);
                mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                mProgressDialog.setMax(100);
                mProgressDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        getText(R.string.alert_dialog_hide), new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {

                    }
                });
                mProgressDialog.show();
                return true;
            }
        });

        popup.show();
    }
}