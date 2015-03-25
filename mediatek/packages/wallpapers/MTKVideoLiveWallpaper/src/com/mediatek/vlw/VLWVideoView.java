package com.mediatek.vlw;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.VideoView;

import com.mediatek.xlog.Xlog;

public class VLWVideoView extends VideoView {
    
    private static final String TAG = "VLWVideoView";

    public VLWVideoView(Context context) {
        super(context);
    }

    public VLWVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public VLWVideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    /**
     * 
     * VideoView.java has been changed in KK, only set widthMeasureSpec will not
     * adjust height. So re-implement onMeasure to make sure VideoView's size is
     * normal [ALPS01285144]
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
        if (mVideoWidth > 0 && mVideoHeight > 0) {
            if ( mVideoWidth * height  > width * mVideoHeight ) {
                Xlog.i(TAG, "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            } else if ( mVideoWidth * height  < width * mVideoHeight ) {
                Xlog.i(TAG, "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            } else {
                Xlog.i(TAG, "aspect ratio is correct: " +
                        width+"/"+height+"="+
                        mVideoWidth+"/"+mVideoHeight);
            }
        }
        Xlog.i(TAG, "setting size: " + width + 'x' + height);
        setMeasuredDimension(width, height);
    }  
}
