package com.hissage.ui.view;

import java.io.InputStream;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Movie;
import android.util.AttributeSet;
import android.view.View;

public class NmsGifView extends View {
    Movie movie;
    InputStream is = null;
    long moviestart;
    int mResId;
    Context mContext;

    public NmsGifView(Context context) {
        super(context);
        mContext = context;
    }

    public NmsGifView(Context context, AttributeSet attr) {
        super(context, attr);
        mContext = context;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        long now = android.os.SystemClock.uptimeMillis();
        if (moviestart == 0) {
            moviestart = now;

        }
        int duration = movie.duration();
        int relTime;
        if (duration == 0) {
            relTime = 0;
        } else {
            relTime = (int) ((now - moviestart) % duration);
        }
        movie.setTime(relTime);
        int bw = movie.width();
        int bh = movie.height();
        int w = getWidth();
        int h = getHeight();
        movie.draw(canvas, (w - bw) / 2, (h - bh) / 2);
        this.invalidate();
    }

    public void setSource(int id) {
        mResId = id;
        is = mContext.getResources().openRawResource(mResId);
        movie = Movie.decodeStream(is);
    }
}
