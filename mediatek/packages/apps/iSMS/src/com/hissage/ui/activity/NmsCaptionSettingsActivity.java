package com.hissage.ui.activity;

import android.app.ActionBar;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.view.Gravity;
import android.view.MenuItem;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;

import com.hissage.R;
import com.hissage.config.NmsConfig;

public class NmsCaptionSettingsActivity extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private ActionBar mActionBar = null;
    private Switch mSwitch = null;

    private static final String KEY_AUDIO = "audio";
    private static final String KEY_VIDEO = "video";
    private static final String KEY_PHOTO = "photo";

    private CheckBoxPreference mAudio = null;
    private CheckBoxPreference mVideo = null;
    private CheckBoxPreference mPhoto = null;

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        switch (id) {
        case android.R.id.home: {
            finish();
            break;
        }
        }
        return super.onMenuItemSelected(featureId, item);
    }

    @Override
    public boolean onPreferenceChange(Preference arg0, Object arg1) {
        // TODO Auto-generated method stub
        return false;
    }

    private void init() {
        mAudio = (CheckBoxPreference) findPreference(KEY_AUDIO);
        mVideo = (CheckBoxPreference) findPreference(KEY_VIDEO);
        mPhoto = (CheckBoxPreference) findPreference(KEY_PHOTO);

        mAudio.setChecked(NmsConfig.getAudioCaptionFlag());
        mVideo.setChecked(NmsConfig.getVideoCaptionFlag());
        mPhoto.setChecked(NmsConfig.getPhotoCaptionFlag());
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mActionBar = getActionBar();
        mActionBar.setDisplayHomeAsUpEnabled(true);
        mActionBar.setDisplayShowHomeEnabled(true);
        mActionBar.setLogo(R.drawable.isms);
        
        CharSequence texton = getString(R.string.STR_NMS_TEXT_ON) ;
        CharSequence textoff = getString(R.string.STR_NMS_TEXT_OFF) ;
        final int height = this.getResources().getDimensionPixelSize(R.dimen.thumb_text_padding) ;
        mSwitch = new Switch(this);
        mSwitch.setTextOn(texton) ;
        mSwitch.setTextOff(textoff) ;
        mSwitch.setThumbTextPadding(height) ;
        if (this instanceof PreferenceActivity) {
            PreferenceActivity preferenceActivity = (PreferenceActivity) this;
            if (preferenceActivity.onIsHidingHeaders() || !preferenceActivity.onIsMultiPane()) {
                final int padding = this.getResources().getDimensionPixelSize(
                        R.dimen.action_bar_switch_padding);
                mSwitch.setPadding(0, 0, padding, 0);
                this.getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                        ActionBar.DISPLAY_SHOW_CUSTOM);
                this.getActionBar().setCustomView(
                        mSwitch,
                        new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                                        | Gravity.RIGHT));

                mSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {

                    @Override
                    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                        NmsConfig.setCaptionFlag(isChecked ? 0 : 1);
                    }
                });
            }
        }

        addPreferencesFromResource(R.xml.caption_settings);
        init();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mSwitch != null) {
            mSwitch.setChecked(NmsConfig.getCaptionFlag());
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {

        if (preference == mPhoto) {
            NmsConfig.setPhotoCaptionFlag(mPhoto.isChecked() ? 0 : 1);
        } else if (preference == mVideo) {
            NmsConfig.setVideoCaptionFlag(mVideo.isChecked() ? 0 : 1);
        } else if (preference == mAudio) {
            NmsConfig.setAudioCaptionFlag(mAudio.isChecked() ? 0 : 1);
        }
        return true;
    }

}
