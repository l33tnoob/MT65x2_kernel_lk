package com.hissage.ui.activity;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Window;
import android.widget.MediaController;
import android.widget.VideoView;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;

public class NmsVideoPlayActivity extends NmsBaseActivity {

    private VideoView videoView = null ;
    private int videoPos = 0 ;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.video_play);
        videoView = (VideoView) findViewById(R.id.vv_media);

        Intent intent = getIntent();

        String path = intent.getStringExtra("path");
        Uri videoUri;
        if (TextUtils.isEmpty(path) || !NmsCommonUtils.isExistsFile(path)) {
            videoUri = intent.getData();
            if (null == videoUri) {
                finish();
                return;
            } else {
                videoView.setVideoURI(videoUri);
            }
        } else {
            videoView.setVideoPath(path);
        }

        videoView.setMediaController(new MediaController(NmsVideoPlayActivity.this));
        videoView.requestFocus();

        videoView.start();
       
    }
    
    @Override
    protected void onPause() {
        if (videoView != null)
            videoPos = videoView.getCurrentPosition() ;
        super.onPause();
    }
    

    @Override
    protected void onResume() {
        super.onResume();
        
        if (videoView != null && videoPos >= 0) 
            videoView.seekTo(videoPos) ;
    }
}
