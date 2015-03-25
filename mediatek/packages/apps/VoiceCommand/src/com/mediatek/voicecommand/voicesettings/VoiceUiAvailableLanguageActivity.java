package com.mediatek.voicecommand.voicesettings;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;

import com.mediatek.xlog.Xlog;

public class VoiceUiAvailableLanguageActivity extends PreferenceActivity {
    private static final String TAG = "VoiceUiAvailableLanguageFragment";

    private RadioButtonPreference mLastSelectedPref;
    private int mDefaultLanguage = 0;
    private String[] mAvailableLangs;
    private ConfigurationManager mVoiceConfigMgr;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Xlog.d(TAG, "OnCreate VoiceUiAvailableLanguageFragment");
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.voice_ui_available_langs);
        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        if (mVoiceConfigMgr == null) {
            Xlog.e(TAG, "ConfigurationManager is null");
        }
        mAvailableLangs = mVoiceConfigMgr.getLanguageList();
        mDefaultLanguage = mVoiceConfigMgr.getCurrentLanguage();

        Xlog.d(TAG, "voice ui deafult language: " + mAvailableLangs[mDefaultLanguage]);
        Xlog.d(TAG, mAvailableLangs.toString());

        for (int j = 0; j < mAvailableLangs.length; j++) {
            RadioButtonPreference pref = new RadioButtonPreference(this, mAvailableLangs[j], "");
            pref.setKey(Integer.toString(j));

                Xlog.v(TAG, "available[" + j + "]" + mAvailableLangs[j]);

            if (mDefaultLanguage == j) {
                pref.setChecked(true);
                mLastSelectedPref = pref;
            }
            getPreferenceScreen().addPreference(pref);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference instanceof RadioButtonPreference) {
            selectLanguage((RadioButtonPreference) preference);
            Xlog.d(TAG, "default language changed to " + mAvailableLangs[mDefaultLanguage]);
            finish();
        }
        return true;
    }

    private void selectLanguage(RadioButtonPreference preference) {

        if (mLastSelectedPref != null) {
            if (mLastSelectedPref == preference) {
                return;
            }
            mLastSelectedPref.setChecked(false);
        }
        mDefaultLanguage = Integer.parseInt(preference.getKey().toString());
        Xlog.d(TAG, "set default language to " + mAvailableLangs[mDefaultLanguage]);
        mVoiceConfigMgr.setCurrentLanguage(mDefaultLanguage);
        preference.setChecked(true);
        mLastSelectedPref = preference;
    }

}
