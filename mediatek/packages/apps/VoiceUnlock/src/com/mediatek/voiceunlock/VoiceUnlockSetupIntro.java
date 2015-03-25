package com.mediatek.voiceunlock;

import android.app.Fragment;
import android.content.Intent;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceFragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.voiceunlock.R;

public class VoiceUnlockSetupIntro extends PreferenceActivity {

    // required constructor for fragments
    public VoiceUnlockSetupIntro() {

    }

    @Override
    public Intent getIntent() {
        Intent modIntent = new Intent(super.getIntent());
        modIntent.putExtra(EXTRA_SHOW_FRAGMENT, VoiceUnlockSetupIntroFragment.class.getName());
        modIntent.putExtra(EXTRA_NO_HEADERS, true);
        return modIntent;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        CharSequence msg = getText(R.string.voice_unlock_setup_intro_header);
        showBreadCrumbs(msg, msg);
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        if (VoiceUnlockSetupIntroFragment.class.getName().equals(fragmentName)) return true;
        return false;
    }

    public static class VoiceUnlockSetupIntroFragment extends SettingsPreferenceFragment
            implements View.OnClickListener {
        private View mContinueButton;
        private View mCancelButton;

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View view = inflater.inflate(R.layout.voice_unlock_setup_intro, null);
            mContinueButton = view.findViewById(R.id.continue_button);
            mContinueButton.setOnClickListener(this);
            mCancelButton = view.findViewById(R.id.cancel_button);
            mCancelButton.setOnClickListener(this);

            return view;
        }

        public void onClick(View v) {
            if (v == mCancelButton) {
                // Canceling, so finish
                getActivity().finish();
            } else if (v == mContinueButton) {
                Intent intent = getActivity().getIntent();
                intent.setClass(getActivity(), VoiceCommandRecord.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_FORWARD_RESULT);
                startActivity(intent);
                getActivity().overridePendingTransition(0, 0); // no animation
                getActivity().finish();
            }

        }
    }
}
