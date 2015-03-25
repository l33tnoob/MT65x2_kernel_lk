package com.mediatek.voicecommand.voicesettings;

import java.util.HashMap;

import com.mediatek.voicecommand.R;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.xlog.Xlog;

import android.content.Intent;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

public class VoiceUiCommandPlay extends PreferenceActivity {
    private static final String TAG = "VoiceUiCommandPlay";
    private static final String KEY_VOICE_UI_FOR_COMMAND_PLAY = "command_play";
    private static final String KEY_VOICE_UI_FOR_COMMAND_CATEGORY = "voice_ui_command";
    private String mProcessKey;
    String[] mCommands;
    private PreferenceCategory mVoiceUiCommandCategory;
    private ConfigurationManager mVoiceConfigMgr;
    private SoundPool mSoundPool;
    private HashMap<String, Integer> mSoundIdMap = new HashMap<String, Integer>();

    @SuppressWarnings("deprecation")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.voice_ui_available_preference);

        Intent intent = getIntent();
        if (intent == null) {
            Xlog.e(TAG, "Intent is null");
            finish();
            return;
        }
        mVoiceConfigMgr = ConfigurationManager.getInstance(this);
        mProcessKey = (String) intent
                .getCharSequenceExtra(VoiceUiSettings.KEY_VOICE_UI_FOR_PLAY_COMMAND);
        int processID = mVoiceConfigMgr.getProcessID(mProcessKey);
        int CommandTitleId = VoiceUiResUtil
                .getCommandTitleResourceId(processID);
        if (CommandTitleId != 0) {
            setTitle(VoiceUiResUtil.getCommandTitleResourceId(processID));
        } else {
            setTitle("Error");
        }

        if (mVoiceConfigMgr == null) {
            Xlog.e(TAG, "ConfigurationManager is null");
            finish();
            return;
        }
        mCommands = mVoiceConfigMgr.getKeyWordForSettings(mProcessKey);
        if (mCommands == null) {
            Xlog.e(TAG, "mCommands is null");
            finish();
            return;
        }

        CommandPlayPreference titlePref = (CommandPlayPreference) findPreference(KEY_VOICE_UI_FOR_COMMAND_PLAY);
        titlePref.setShowTitle(fetchSummary(processID));
        titlePref.setSelectable(false);
        titlePref.setOrder(0);

        mSoundPool = new SoundPool(1, AudioManager.STREAM_MUSIC, 0);

        mVoiceUiCommandCategory = (PreferenceCategory) findPreference(KEY_VOICE_UI_FOR_COMMAND_CATEGORY);
        mVoiceUiCommandCategory.setTitle(R.string.voice_ui_commands);
        for (int i = 0; i < mCommands.length; i++) {
            Preference pref = new Preference(this);
            pref.setLayoutResource(R.layout.voice_ui_preference_image);
            pref.setTitle(mCommands[i]);
            String path = mVoiceConfigMgr.getCommandPath(mProcessKey);
            path = path + i + ".ogg";
            mSoundIdMap.put(mCommands[i], mSoundPool.load(path, 1));
            pref.setKey(mCommands[i]);
            pref.setOrder(i + 1);
            mVoiceUiCommandCategory.addPreference(pref);
        }
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        if (mSoundPool != null) {
            mSoundPool.release();
        }
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        mSoundPool.autoPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        mSoundPool.autoResume();
        super.onResume();
    }

    /*
     * fetch summary according process ID
     *
     * @param process ID
     *
     * @return String
     */
    private String fetchSummary(int processID) {

        StringBuilder keywords = new StringBuilder();
        String lastWord = "\"" + mCommands[mCommands.length - 1] + "\"";

        for (int i = 0; i < mCommands.length - 1; i++) {
            keywords.append("\"").append(mCommands[i]).append("\"");
            if (i != mCommands.length - 2) {
                keywords.append(",");
            }
        }

        int resId = VoiceUiResUtil.getSummaryResourceId(processID);
        if (resId == 0) {
            return new String("Error");
        }
        String summary = getString(resId, keywords.toString(), lastWord);

        return summary;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        // TODO Auto-generated method stub
        String Key = preference.getKey();
        if ((Key != null) && (mSoundIdMap.containsKey(Key))) {
            mSoundPool.play(mSoundIdMap.get(Key), 1, 1, 0, 0, 1);
        } else {
            Xlog.e(TAG, "onPreferenceTreeClick path is null ");
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

}
