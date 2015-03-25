package com.mediatek.engineermode.desense;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.Scanner;

public class MemPllSet extends Activity {

    private static final String TAG = "EM/MemPllSet";
    private static final String FILE_DRAMC = "/proc/freqhopping/dramc";
    private static final String CURRENT_PRE = "Current DRAMC is ";
    private static final String SUCCESS = " success";
    private static final String FAIL = " fail";
    private static final String VALUE_200 = "200";
    private static final String VALUE_266 = "266";
    private static final String VALUE_333 = "333";
    private static final int VALUE = 266;
    private TextView mTvCurrent;
    private Button mBtn200to266;
    private Button mBtn266to200;
    private int value_0 = 0;
    private int value_1 = 0;
    private String text_0;
    private String text_1;

    private final View.OnClickListener mClickListener = new View.OnClickListener() {

        public void onClick(View v) {
            if (v.equals(mBtn200to266)) {
                doConvert(false);
            } else if (v.equals(mBtn266to200)) {
                doConvert(true);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!new File(FILE_DRAMC).exists() || !new File(FILE_DRAMC).exists()) {
            Toast.makeText(this, R.string.desense_mempll_notsupport,
                    Toast.LENGTH_SHORT).show();
            Xlog.w(TAG, "MemPllSet files not exist");
            finish();
            return;
        }
        setContentView(R.layout.desense_mempll_set);
        mTvCurrent = (TextView) findViewById(R.id.desense_mempll_tv_current);
        mBtn200to266 = (Button) findViewById(R.id.desense_mempll_btn_convert266);
        mBtn200to266.setOnClickListener(mClickListener);
        mBtn266to200 = (Button) findViewById(R.id.desense_mempll_btn_convert200);
        mBtn266to200.setOnClickListener(mClickListener);
        if(ChipSupport.MTK_6592_SUPPORT == ChipSupport.getChip()) {
            value_0 = 266;
            value_1 = 333;
            text_0 = VALUE_266;
            text_1 = VALUE_333;
            mBtn200to266.setText(getString(R.string.desense_mempll_266to333));
            mBtn266to200.setText(getString(R.string.desense_mempll_333to266));
        } else {
            value_0 = 200;
            value_1 = 266;
            text_0 = VALUE_200;
            text_1 = VALUE_266;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        int current = getCurrentDramc();
        Xlog.v(TAG, "Current: " + current);
        if (0 == current) {
            Toast.makeText(this, R.string.desense_mempll_geterror,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        mTvCurrent.setText(CURRENT_PRE
                + (current < value_1 ? text_0 : text_1));
        updateBtnStatus(mTvCurrent.getText().toString().contains(text_0));
    }

    private void doConvert(boolean bValue200) {
        boolean result = setCurrentDramc(bValue200 ? text_0 : text_1);
        if (result) {
            mTvCurrent.setText(CURRENT_PRE
                    + (bValue200 ? text_0 : text_1));
            updateBtnStatus(bValue200);
        }
        Toast.makeText(
                this,
                getString(R.string.desense_mempll_set)
                        + (result ? SUCCESS : FAIL), Toast.LENGTH_SHORT).show();
    }

    private void updateBtnStatus(boolean bValue200) {
        mBtn200to266.setEnabled(bValue200);
        mBtn266to200.setEnabled(!bValue200);
    }

    private int getCurrentDramc() {
        StringBuilder strBuilder = new StringBuilder();
        int value = 0;
        strBuilder.append("cat ");
        strBuilder.append(FILE_DRAMC);
        Xlog.v(TAG, "get current dramc cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString())) {
                String result = ShellExe.getOutput();
                Scanner scan = new Scanner(result);
                value = Integer.valueOf(scan.findInLine("\\d+"));
                scan.close();
            }
        } catch (IOException e) {
            Xlog.w(TAG, "get current dramc IOException: " + e.getMessage());
        }
        return value;
    }

    private boolean setCurrentDramc(String value) {
        StringBuilder strBuilder = new StringBuilder();
        boolean result = false;
        strBuilder.append("echo ");
        strBuilder.append(value);
        strBuilder.append(" > ");
        strBuilder.append(FILE_DRAMC);
        Xlog.v(TAG, "set current dramc cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString())) {
                result = true;
            }
        } catch (IOException e) {
            Xlog.w(TAG, "set current dramc IOException: " + e.getMessage());
        }
        return result;
    }
}
