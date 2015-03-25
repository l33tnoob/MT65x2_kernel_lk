package com.hissage.ui.view;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.VideoView;

import com.hissage.R;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.util.log.NmsLog;

public class NmsMultimediaMMSView extends NmsMultimediaBaseView {

    private Context mContext;
    private View mConvertView;
    private NmsIpMessage session;
    private ImageButton btnPlay;

    private TextView tvTime;
    private final static String TAG = "NmsMultimediaMMSView";

    public NmsMultimediaMMSView(Context context, NmsIpMessage session) {
        super(context);

        this.session = session;

        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.all_media_mms, this, true);
        mContext = context;

        VideoView v = (VideoView) mConvertView.findViewById(R.id.vv_video);
        Bitmap bp = NmsSMSMMSManager.getInstance(mContext).getFirstImgFromMMS((int) session.id,
                false, 0, 0);
        if (bp == null) {
            v.setBackgroundResource(R.drawable.all_media_mms);
        } else {
            Drawable d = new BitmapDrawable(bp);
            v.setBackgroundDrawable(d);
        }

        btnPlay = (ImageButton) mConvertView.findViewById(R.id.ib_play_mms);
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

    public NmsMultimediaMMSView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void play() {
        NmsLog.trace(TAG, "The MMS messag id:" + session.id);
        Intent intent = new Intent("com.android.mms.ui.SlideshowActivity");
        intent.setClassName("com.android.mms", "com.android.mms.ui.SlideshowActivity");
        intent.setData(Uri.parse("content://mms/" + session.id));
        mContext.startActivity(intent);
    }
}
