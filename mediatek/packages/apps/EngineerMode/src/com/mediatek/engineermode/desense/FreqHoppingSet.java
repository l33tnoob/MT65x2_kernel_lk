package com.mediatek.engineermode.desense;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class FreqHoppingSet extends Activity {

    private static final String TAG = "EM/FreqHoppingSet";
    private static final int RADIX_16 = 16;
    private static final int FREQ_HOPPING_SIZE = 6;
    private static final int EDITTEXT_SIZE = 5;
    private static final String FILE_FREQ_HOPPING_DEBUG = "/proc/freqhopping/freqhopping_debug";
    private static final String FILE_FREQ_DUMPREGS = "/proc/freqhopping/dumpregs";
    private static final String FILE_FREQ_STATUS = "/proc/freqhopping/status";
    private static final String PATSTR = "\\=[\\s]*?\\d+\\=";
    private static final String PLL_NAME_PATTERN = "\\=\\w*PLL\\w*";
    private static final int DIALOG_ID_SHOWALL = 0;
    private static final int DIALOG_ID_SHOWPLL = 1;
    private static final String SUCCESS = " success";
    private static final String FAIL = " fail";
    private Spinner mSpPlls;
    private EditText mEtDds;
    private EditText mEtDeltaFreq;
    private EditText mEtDeltaTime;
    private EditText mEtLimitUpper;
    private EditText mEtLimitDown;
    private Button mBtnEnable;
    private Button mBtnDisable;
    private Button mBtnReadAll;
    private EditText[] mEtArray = new EditText[EDITTEXT_SIZE];
    private int[] mFreqHopping;
    private int mPllCount;

    private final OnItemSelectedListener mSpinnerSelectListener = new OnItemSelectedListener() {

        public void onItemSelected(AdapterView<?> parent, View view,
                int position, long id) {
            selectItem(position);
            showDialog(DIALOG_ID_SHOWPLL);
        }

        public void onNothingSelected(AdapterView<?> parent) {
        }

    };

    private final View.OnClickListener mBtnClickListener = new View.OnClickListener() {

        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.desense_freqhopping_btn_enable:
                if (validateInput(true)) {
                    enableFreqHopping();
                }
                break;
            case R.id.desense_freqhopping_btn_disable:
                if (validateInput(false)) {
                    disableFreqHopping();
                }
                break;
            case R.id.desense_freqhopping_btn_readall:
                if (validateInput(false)) {
                    readAllFreqHopping();
                }
                break;
            default:
                break;
            }
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!new File(FILE_FREQ_HOPPING_DEBUG).exists()) {
            Toast.makeText(this, R.string.desense_freqhopping_notsupport,
                    Toast.LENGTH_SHORT).show();
            Xlog.w(TAG, "FreqHoppingSet file not exists");
            finish();
            return;
        }
        setContentView(R.layout.desense_freqhopping_set);
        mSpPlls = (Spinner) findViewById(R.id.desense_freqhopping_sp_plls);
        mSpPlls.setOnItemSelectedListener(mSpinnerSelectListener);
        if (0 != initPllSpinnerItems(mSpPlls)) {
            Toast.makeText(this, "init pll spinner fail", Toast.LENGTH_SHORT).show();
            finish();
            return;
        } 
            
        mFreqHopping = new int[mPllCount];
        mEtDds = (EditText) findViewById(R.id.desense_freqhopping_et_dds);
        mEtDeltaFreq = (EditText) findViewById(R.id.desense_freqhopping_et_deltafreq);
        mEtDeltaTime = (EditText) findViewById(R.id.desense_freqhopping_et_deltatime);
        mEtLimitUpper = (EditText) findViewById(R.id.desense_freqhopping_et_upperlimit);
        mEtLimitDown = (EditText) findViewById(R.id.desense_freqhopping_et_downlimit);
        mEtArray[0] = mEtDds;
        mEtArray[1] = mEtDeltaFreq;
        mEtArray[2] = mEtDeltaTime;
        mEtArray[EDITTEXT_SIZE - 2] = mEtLimitUpper;
        mEtArray[EDITTEXT_SIZE - 1] = mEtLimitDown;
        mBtnEnable = (Button) findViewById(R.id.desense_freqhopping_btn_enable);
        mBtnEnable.setOnClickListener(mBtnClickListener);
        mBtnDisable = (Button) findViewById(R.id.desense_freqhopping_btn_disable);
        mBtnDisable.setOnClickListener(mBtnClickListener);
        mBtnReadAll = (Button) findViewById(R.id.desense_freqhopping_btn_readall);
        mBtnReadAll.setOnClickListener(mBtnClickListener);
    }
    
    private int initPllSpinnerItems(Spinner pllSpn) {
        String debugMsg = getFreqHopDebugMsg();
        if (debugMsg != null) {
            Pattern pattern = Pattern.compile(PLL_NAME_PATTERN, Pattern.CASE_INSENSITIVE);
            Matcher matcher = pattern.matcher(debugMsg);
            List<String> pllList = new ArrayList<String>();
            while (matcher.find()) {
                String pllName = matcher.group().substring(1);
                pllList.add(pllName);
            }
            mPllCount = pllList.size();
            if (mPllCount > 0) {
                
                ArrayAdapter<String> pllAdatper = new ArrayAdapter<String>(this,
                        android.R.layout.simple_spinner_item, pllList);
                pllAdatper
                        .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                pllSpn.setAdapter(pllAdatper);
                return 0;
            } else {
                Xlog.d(TAG, "init pll spinner fail; mPllCount:" + mPllCount);
                return -1;
            }
            
        } else {
            Xlog.d(TAG, "init pll spinner fail; debugMsg = null");
            return -1;
        }
    }

    private void updateCurrentStatus() {
        selectItem(mSpPlls.getSelectedItemPosition());
    }

    @Override
    protected Dialog onCreateDialog(final int id) {
        Dialog dialog = null;
        if (DIALOG_ID_SHOWALL == id) {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                    FreqHoppingSet.this);
            builder.setTitle(R.string.desense_freqhopping_dialog_title);
            builder.setCancelable(false);
            builder.setMessage(getFreqHopMsg());
            builder.setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            removeDialog(id);
                        }

                    });
            dialog = builder.create();
        } else if (DIALOG_ID_SHOWPLL == id) {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                    FreqHoppingSet.this);
            builder.setTitle(R.string.desense_freqhopping_dialog_title);
            builder.setCancelable(false);
            builder.setMessage(getFreqHopDebugMsg());
            builder.setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            removeDialog(id);
                        }

                    });
            dialog = builder.create();
        }
        return dialog;
    }

    private void enableFreqHopping() {
        boolean bSuccess = false;
        boolean bEmpty = mEtArray[0].getText().toString().trim().isEmpty();
        StringBuilder strBuilder = new StringBuilder();
        if (bEmpty) {
            strBuilder.append("echo 1 ");
            strBuilder.append(mSpPlls.getSelectedItemPosition());
            strBuilder.append(" > ");
            strBuilder.append(FILE_FREQ_STATUS);
            Xlog.v(TAG, "enable command: " + strBuilder.toString());
        } else {
            strBuilder.append("echo 3 0 ");
            strBuilder.append(mSpPlls.getSelectedItemPosition());
            for (EditText edt : mEtArray) {
                strBuilder.append(" ");
                strBuilder.append(edt.getText().toString().trim());
            }
            strBuilder.append(" > ");
            strBuilder.append(FILE_FREQ_HOPPING_DEBUG);
            Xlog.v(TAG, "enable command 1: " + strBuilder.toString());
        }
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString())) {
                if (bEmpty) {
                    bSuccess = true;
                    updateCurrentStatus();
                } else {
                    strBuilder.delete(0, strBuilder.length());
                    strBuilder.append("echo 1 1 ");
                    strBuilder.append(mSpPlls.getSelectedItemPosition());
                    strBuilder.append(" 1 0 0 0 0 > ");
                    strBuilder.append(FILE_FREQ_HOPPING_DEBUG);
                    Xlog.v(TAG, "enable command 2: " + strBuilder.toString());
                    if (ShellExe.RESULT_SUCCESS == ShellExe
                            .execCommand(strBuilder.toString())) {
                        bSuccess = true;
                        updateCurrentStatus();
                    }
                }
            }
        } catch (IOException e) {
            Xlog.w(TAG, "enable freqhopping IOException: " + e.getMessage());
        }
        Toast.makeText(
                this,
                getString(R.string.desense_freqhopping_set)
                        + (bSuccess ? SUCCESS : FAIL), Toast.LENGTH_SHORT)
                .show();
    }

    private void disableFreqHopping() {
        boolean bSuccess = false;
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("echo 2 3 ");
        strBuilder.append(mSpPlls.getSelectedItemPosition());
        strBuilder.append(" 0 0 0 0 0 > ");
        strBuilder.append(FILE_FREQ_HOPPING_DEBUG);
        Xlog.v(TAG, "disable command: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString())) {
                bSuccess = true;
                updateCurrentStatus();
            }
        } catch (IOException e) {
            Xlog.w(TAG, "disable freqhopping IOException: " + e.getMessage());
        }
        Toast.makeText(
                this,
                getString(R.string.desense_freqhopping_set)
                        + (bSuccess ? SUCCESS : FAIL), Toast.LENGTH_SHORT)
                .show();
    }

    private void readAllFreqHopping() {
        showDialog(DIALOG_ID_SHOWALL);
    }

    private boolean validateInput(boolean bEtInput) {
        boolean result = true;
        if (bEtInput) {
            try {
                for (int index = 0; index < EDITTEXT_SIZE; index++) {
                    String input = mEtArray[index].getText().toString().trim();
                    if (input.isEmpty()
                            || Integer.parseInt(input, RADIX_16) < 0) {
                        result = false;
                        break;
                    }
                }
                if (!result) {
                    result = true;
                    for (int index = 0; index < EDITTEXT_SIZE; index++) {
                        String input = mEtArray[index].getText().toString()
                                .trim();
                        if (!input.isEmpty()) {
                            result = false;
                            break;
                        }
                    }
                }
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "validate input NumberFormatException: "
                        + e.getMessage());
                result = false;
            }
        }
        if (result) {
            int select = mSpPlls.getSelectedItemPosition();
            if (select < 0 || select >= mPllCount) {
                result = false;
            }
        }
        if (!result) {
            Toast.makeText(this, R.string.desense_freqhopping_inputerror,
                    Toast.LENGTH_SHORT).show();
        }
        return result;
    }

    private void selectItem(int position) {
        if (updateHoppingStatus()) {
            updateBtnStatus(position);
        } else {
            disableAllBtn();
            Toast.makeText(this, R.string.desense_freqhopping_geterror,
                    Toast.LENGTH_SHORT).show();
        }
    }

    private boolean updateHoppingStatus() {
        boolean result = false;
        String debugMsg = getFreqHopDebugMsg();
        if (null != debugMsg) {
            Pattern pattern = Pattern.compile(PATSTR);
            Matcher m = pattern.matcher(debugMsg);
            int index = 0;
            try {
                while (m.find() && index < mPllCount) {
                    String msg = m.group();
                    if (null != msg) {
                        int value = Integer.parseInt(msg.substring(1,
                                msg.length() - 1).trim());
                        Xlog.v(TAG, "index: " + index + " value: " + value);
                        mFreqHopping[index] = value;
                        index++;
                    }
                }
                result = index == mPllCount;
            } catch (IllegalStateException e) {
                Xlog.w(TAG, "updateHoppingStatus IllegalStateException: "
                        + e.getMessage());
            } catch (IndexOutOfBoundsException e) {
                Xlog.w(TAG, "updateHoppingStatus IndexOutOfBoundsException: "
                        + e.getMessage());
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "updateHoppingStatus NumberFormatException: "
                        + e.getMessage());
            }
        }
        return result;
    }

    private String getFreqHopDebugMsg() {
        String result = null;
        try {
            int ret = ShellExe.execCommand("cat " + FILE_FREQ_HOPPING_DEBUG);
            if (ShellExe.RESULT_SUCCESS == ret) {
                result = ShellExe.getOutput();
            }
        } catch (IOException e) {
            Xlog.w(TAG, "getFreqHopDebugMsg IOException: " + e.getMessage());
        }
        return result;
    }

    private String getFreqHopMsg() {
        String result = null;
        try {
            int ret = ShellExe.execCommand("cat " + FILE_FREQ_DUMPREGS);
            if (ShellExe.RESULT_SUCCESS == ret) {
                result = ShellExe.getOutput();
            }
            ret = ShellExe.execCommand("cat " + FILE_FREQ_STATUS);
            if (ShellExe.RESULT_SUCCESS == ret) {
                result += ShellExe.getOutput();
            }
        } catch (IOException e) {
            Xlog.w(TAG, "getFreqHopMsg IOException: " + e.getMessage());
        }
        return result;
    }

    private void updateBtnStatus(int position) {
        if (position < 0 || position >= mPllCount
                || mFreqHopping[position] < 0) {
            disableAllBtn();
        } else {
            updateBtns(0 == mFreqHopping[position]);
        }
    }

    private void updateBtns(boolean bEnable) {
        mBtnEnable.setEnabled(bEnable);
        mBtnDisable.setEnabled(!bEnable);
        mBtnReadAll.setEnabled(true);
    }

    private void disableAllBtn() {
        mBtnEnable.setEnabled(false);
        mBtnDisable.setEnabled(false);
        mBtnReadAll.setEnabled(false);
    }
}
