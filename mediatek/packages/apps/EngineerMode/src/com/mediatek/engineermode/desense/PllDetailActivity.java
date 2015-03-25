package com.mediatek.engineermode.desense;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class PllDetailActivity extends Activity {

    private static final String TAG = "EM/DesensePllDetail";
    public static final String PLL_DETAIL_ITEM_NAME = "name";
    public static final String PLL_DETAIL_ITEM_ID = "id";
    public static final String PLL_DETAIL_ITEM_VALUE = "value";
    private static final String PATTERN = "^[0-9a-fA-F]{1,16}$";
    private Button mBtnSet;
    private TextView mTvTitle;
    private EditText mEtValue;
    private String mName;
    private int mId;
    private String mValue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_pll_detail_activity);
        mBtnSet = (Button) findViewById(R.id.desense_pll_detail_set_btn);
        mTvTitle = (TextView) findViewById(R.id.desense_pll_detail_title_textview);
        mEtValue = (EditText) findViewById(R.id.desense_pll_detail_edit);
        Intent intent = getIntent();
        mName = intent.getStringExtra(PLL_DETAIL_ITEM_NAME);
        mId = intent.getIntExtra(PLL_DETAIL_ITEM_ID, -1);
        mValue = intent.getStringExtra(PLL_DETAIL_ITEM_VALUE);
        mTvTitle.setText(mName);
        mEtValue.setText(mValue);
        mEtValue.setSelection(mValue.length());
        mBtnSet.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                String editValue = mEtValue.getText().toString().trim();
                Xlog.v(TAG, "editValue = " + editValue);
                Pattern pattern = Pattern.compile(PATTERN);
                Matcher m = pattern.matcher(editValue);
                if (m.find()) {
                    if (pllSetClock(mId, editValue) < 0) {
                        Toast.makeText(PllDetailActivity.this, "Set PLL fail",
                                Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(PllDetailActivity.this,
                                "Set PLL success", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    Xlog.w(TAG, "set button NumberFormatException");
                    Toast.makeText(PllDetailActivity.this,
                            "The input number is wrong!", Toast.LENGTH_SHORT)
                            .show();
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        Toast.makeText(this, "Wrong set may cause system error!",
                Toast.LENGTH_SHORT).show();
    }
    
    private static int pllSetClock(int id, String hexVal) {
        int result = -1;
        String cmd = "echo enable " + id + " >/proc/clkmgr/pll_test";
        Xlog.i(TAG, cmd);
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(cmd)) {
                cmd = "echo " + id + " " + hexVal + " >/proc/clkmgr/pll_fsel";
                if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(cmd)) {
                    result = 0;
                }
            }
        } catch (IOException e) {
            Xlog.v(TAG, "pllSetClock IOException: " + e.getMessage());
        }
        return result;
    }
}
