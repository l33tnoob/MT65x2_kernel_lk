package com.hissage.ui.view;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.ThumbnailUtils;
import android.os.Handler;
import android.provider.MediaStore.Video.Thumbnails;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.ui.activity.NmsVideoPlayActivity;

public class NmsMultimediaVideoView extends NmsMultimediaBaseView {

    private Handler mHandler;
    private Context mContext;
    private View mConvertView;

    private NmsIpVideoMessage session;
    private VideoView v;
    private TextView tvTime;
    private final static String TAG = "NmsMultimediaBaseView";

    public NmsMultimediaVideoView(Context context, NmsIpVideoMessage session) {
        super(context);
        mContext = context;
        this.session = session;

        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.all_media_video, this, true);

        String path = ((NmsIpVideoMessage) session).path;
        v = (VideoView) mConvertView.findViewById(R.id.vv_video);
        Bitmap bp = ThumbnailUtils.createVideoThumbnail(path, Thumbnails.MICRO_KIND);
        Drawable d = new BitmapDrawable(bp);
        v.setBackgroundDrawable(d);

        final ImageButton btnPlay = (ImageButton) mConvertView.findViewById(R.id.ib_play_video);
        btnPlay.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                // TODO Auto-generated method stub
                play();
            }
        });

        tvTime = (TextView) mConvertView.findViewById(R.id.tv_media_name);
    }

    public void setMediaImage() {
        setTimeStamp(tvTime, session);
        isSetTime = true;
    }

    public NmsMultimediaVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void play() {
        if (TextUtils.isEmpty(session.path)) {
            return;
        }
        
        if (!NmsCommonUtils.getSDCardStatus()) {
            Toast.makeText(mContext, R.string.STR_NMS_CANT_SHARE, Toast.LENGTH_SHORT)
            .show();
            return;
        }

        Intent i = new Intent(mContext, NmsVideoPlayActivity.class);
        i.putExtra("path", session.path);
        mContext.startActivity(i);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }
}
