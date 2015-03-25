package com.mediatek.voicecommand.voicesettings;

import android.content.Context;
import android.preference.Preference;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RadioButton;

import com.mediatek.voicecommand.R;

import com.mediatek.xlog.Xlog;

public class RadioButtonPreference extends Preference {
    
    private static final String TAG = "RadioButtonPreference";
    
    private String mTitle;
    private String mSummary;
    private RadioButton mRadioButton;
    private boolean mIsChecked;

    /**
     * constructor of radio button
     * @param context Context
     */
    public RadioButtonPreference(Context context) {
        this(context, "title", "summary");
    }

    /**
     * constructor of radio button
     * @param context Context
     * @param attrs AttributeSet
     */
    public RadioButtonPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.voice_ui_preference_radio_button);
    }

    /**
     * constructor of radio button
     * @param context Context
     * @param title String
     * @param summary String
     */
    public RadioButtonPreference(Context context, String title, String summary) {
        this(context, title, summary, false);
    }

    /**
     * constructor of radio button
     * @param context Context
     * @param title String
     * @param summary String
     * @param isChecked boolean
     */
    public RadioButtonPreference(Context context, String title, String summary, boolean isChecked) {
        super(context);
        mTitle = title;
        mSummary = summary;
        mIsChecked = isChecked;
        setLayoutResource(R.layout.voice_ui_preference_radio_button);

        if (!TextUtils.isEmpty(mTitle)) {
            setTitle(mTitle);
        }
        if (mSummary != null) {
            setSummary(mSummary);
        }
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mRadioButton = (RadioButton)view.findViewById(R.id.radiobutton);
        if (mRadioButton != null) {
            mRadioButton.setChecked(mIsChecked);
        } else {
            Xlog.d(TAG, "radio button can't be find");
        }
    }

    /**
     * set Checked
     * @param newCheckStatus boolean
     */
    public void setChecked(boolean newCheckStatus) {
        mIsChecked = newCheckStatus;
        notifyChanged();
    }
}
