package com.orangelabs.rcs.wizard;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.R.layout;
import com.orangelabs.rcs.R.menu;

import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.util.DisplayMetrics;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.VideoView;

public class JoynIntroductionVideo extends Activity implements OnClickListener{

	private Button mNextbtn;
	VideoView video_player_view;
    DisplayMetrics dm; 

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_joyn_introduction_video);
		
		 mNextbtn = (Button) findViewById(R.id.next_btn);
	     mNextbtn.setOnClickListener(this);
	       
		getVideoPlayerInit();
	}

	public void getVideoPlayerInit() {
		
        video_player_view = (VideoView) findViewById(R.id.video_player_joyn_introduction);
        dm = new DisplayMetrics();
        this.getWindowManager().getDefaultDisplay().getMetrics(dm);
        int height = dm.heightPixels;
        int width = dm.widthPixels;
        video_player_view.setMinimumWidth(width);
        video_player_view.setMinimumHeight(height);
        
        Uri uri = Uri.parse("android.resource://"+getPackageName()+"/"+R.raw.joyn_introduction);

        video_player_view.setVideoURI(uri);
        
       // video_player_view.setVideoPath(R.raw.);
        video_player_view.start();
        
        
        video_player_view.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			
			@Override
			public void onCompletion(MediaPlayer mp) {
				// TODO Auto-generated method stub
				nextStep(true);
				
			}
		}) ;  
      }

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		nextStep(true);
		super.onDestroy();		
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		video_player_view.start();
	}
	
	
	private void nextStep(boolean flag){	
		int result = Utils.RESULT_CODE_FINISH_VIDEO;
        finishActivityByResult(result);	
	}
	
	  /**
     * Set result code and finish
     * @param resultCode true to start next step, false to start last step
     */
    private void finishActivityByResult(int resultCode) {
       // Xlog.d(TAG, "finishActivityByResult, resultCode: " + resultCode);
        Intent intent = new Intent();
        setResult(resultCode, intent);
        finish();
    }

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		 if (v == mNextbtn) {
			  nextStep(true);
	     }
	}
	

}
