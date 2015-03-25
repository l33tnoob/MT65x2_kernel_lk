package com.mediatek.common.widget.tests;

import android.app.ActionBar;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.ArrayAdapter;
import android.widget.SpinnerAdapter;
import android.widget.Toast;

public class VolumePanelActivity extends Activity  {
	private AudioManager mAudioManager;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.volume_panel_main);
		
		mAudioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);

		mAudioManager.registerMediaButtonEventReceiver(new ComponentName(this, RemoteControlReceiver.class));
		/*
		final IntentFilter filter = new IntentFilter();
        filter.addAction(AudioManager.VOLUME_CHANGED_ACTION);
        registerReceiver(new BroadcastReceiver() {

            public void onReceive(Context context, Intent intent) {
                final String action = intent.getAction();

                if (AudioManager.VOLUME_CHANGED_ACTION.equals(action)) {
                }
            }
        }, filter);
        */
	}
	
	public class RemoteControlReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {

            Toast.makeText(VolumePanelActivity.this, "1", 10).show();
            if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction())) {
            	
            }
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) 
    {
        if (event.getAction() == KeyEvent.ACTION_DOWN)
        {
            switch (event.getKeyCode()) 
            {
                case KeyEvent.KEYCODE_VOLUME_UP:
                    return false;
                case KeyEvent.KEYCODE_VOLUME_DOWN:
                    return false;
            }
        }

        return super.dispatchKeyEvent(event);
    }
}
