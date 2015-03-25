package com.mediatek.datatransfer;

import com.mediatek.datatransfer.utils.MyLogger;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;

import java.io.File;

public class RestoreCheckBoxPreference extends Preference {
    
    private final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreCheckBoxPreference";

    private CheckBox mCheckBox;
    private TextView mTextView;
    private boolean mVisibility = false;
    private boolean mChecked = false;
    private boolean mRestored = false;
    private File mAccociateFile;

    public RestoreCheckBoxPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

    }

    public RestoreCheckBoxPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setWidgetLayoutResource(R.layout.prefrence_checkbox);
    }

    public RestoreCheckBoxPreference(Context context) {
        super(context);
        setWidgetLayoutResource(R.layout.prefrence_checkbox);

    }

    public File getAccociateFile() {
        return mAccociateFile;
    }

    public void setAccociateFile(File mAccociateFile) {
        this.mAccociateFile = mAccociateFile;
    }

    @Override
    protected void onBindView(View view) {
        mTextView = (TextView) view.findViewById(R.id.prefrence_textview_id);
        mCheckBox = (CheckBox) view.findViewById(R.id.prefrence_checkbox_id);
        showTextView(mRestored);
        showCheckbox(mVisibility);
        if (mCheckBox != null) {
            mCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                @Override
                public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
                    mChecked = arg1;
                }
            });
        }
        setChecked(mChecked);
        super.onBindView(view);
    }

    private void showTextView(boolean bRestored) {
        // TODO Auto-generated method stub
        MyLogger.logD(CLASS_TAG, "showTextView: " + bRestored);
        if (mTextView == null) {
            return;
        }
        if (bRestored) {
            mTextView.setVisibility(View.VISIBLE);
        } else {
            mTextView.setVisibility(View.INVISIBLE);
        }
    }

    public void showCheckbox(boolean bshow) {
        MyLogger.logD(CLASS_TAG, "showCheckbox: " + bshow);
        mVisibility = bshow;
        if (mCheckBox == null) {
            return;
        }
        if (bshow) {
            mCheckBox.setVisibility(View.VISIBLE);
        } else {
            mCheckBox.setVisibility(View.GONE);
        }
    }

    public boolean isChecked() {
        return mChecked;
    }

    public void setChecked(boolean bChecked) {
        mChecked = bChecked;
        if (mCheckBox != null) {
            mCheckBox.setChecked(bChecked);
        }
    }

    public void setRestored(boolean bRestored) {
        mRestored = bRestored;
    }

    public String getKey() {
        if (mAccociateFile != null) {
            return mAccociateFile.getAbsolutePath();
        }
        return "";
    }
}
