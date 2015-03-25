package com.hissage.ui.view;

import java.io.File;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.message.ip.NmsIpVoiceMessage;

public class NmsMultimediaAudioView extends NmsMultimediaBaseView {

    private Context mContext;
    private View mConvertView;
    private NmsIpVoiceMessage mSession;
    private ImageButton btnPlay;
    private  TextView tvTime;
    private final static String TAG = "NmsMultimediaBaseView";

    public NmsMultimediaAudioView(Context context, NmsIpVoiceMessage session) {
        super(context);

        this.mSession = session;

        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.all_media_audio, this, true);
        mContext = context;

        btnPlay = (ImageButton) mConvertView.findViewById(R.id.ib_play);
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
        setTimeStamp(tvTime, mSession);
        isSetTime = true;
    }

    public NmsMultimediaAudioView(Context context, AttributeSet attrs) {
        super(context, attrs);

    }

    public void play() {
        if (!NmsCommonUtils.getSDCardStatus()) {
            Toast.makeText(mContext, R.string.STR_NMS_CANT_SHARE, Toast.LENGTH_SHORT)
            .show();
            return;
        }
        
        int index = mSession.path.lastIndexOf("/");
        String name = mSession.path.substring(index);
        String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + name;
        NmsCommonUtils.copy(mSession.path, dest);

        Intent intent = new Intent();
        intent.setAction(android.content.Intent.ACTION_VIEW);
        Uri uri = Uri.fromFile(new File(dest));
        intent.setDataAndType(uri, "audio/*");
        mContext.startActivity(Intent.createChooser(intent, "Choose share method."));
    }
}
