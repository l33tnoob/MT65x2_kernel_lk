package com.mediatek.common.widget.tests;

import com.mediatek.common.widget.tests.EditNameDialog.EditNameDialogListener;

import android.os.Bundle;
import android.app.Activity;
import android.app.FragmentManager;
import android.view.Menu;
import android.widget.TextView;
import android.widget.Toast;

public class DialogFragmentActivity extends Activity implements EditNameDialogListener {
	private TextView mTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dialog_fragment_main);
        mTextView = (TextView)findViewById(R.id.text);
        showEditDialog();
    }

    private void showEditDialog() {
        FragmentManager fm = getFragmentManager();
        EditNameDialog editNameDialog = new EditNameDialog();
        editNameDialog.show(fm, "fragment_edit_name");
    }

    public void onFinishEditDialog(String inputText) {
        Toast.makeText(this, "Hi, " + inputText, Toast.LENGTH_SHORT).show();
        mTextView.setText("Hi, " + inputText);
    }
}