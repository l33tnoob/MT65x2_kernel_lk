package com.hissage.ui.view;

import android.content.Context;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.ui.view.NmsLevelControlLayout.OnScrollToScreenListener;
import com.hissage.util.data.NmsDateUtils;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageConsts;
import com.hissage.util.message.MessageUtils;

public class NmsMultimediaBaseView extends LinearLayout {

    private Context mContext;
    private View mConvertView;
    private final static String TAG = "NmsMultimediaBaseView";

    public boolean isSetTime = false;

    public NmsMultimediaBaseView(Context context) {
        super(context);
        mContext = context;
    }

    public NmsMultimediaBaseView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
    }

    protected void setTimeStamp(TextView tv, NmsIpMessage session) {
        if(isSetTime) 
        {
            return;
        }
        
        NmsContact child = NmsIpMessageApiNative.nmsGetContactInfoViaNumber(session.from);
        String contactName = "";
        if (child != null) {
            contactName = child.getName();
        }

        String shareTime = NmsDateUtils.formatCurrentTime(mContext, (long) session.time * 1000);
        String s = String.format(mContext.getText(R.string.STR_NMS_MEDIA_SHARE_MESSAGE).toString(),
                contactName, shareTime);
        tv.setText(s);
    }

    public void setMediaImage() {

    }

    public void destoryMediaImage() {

    }

    public void play() {

    }

    public void pause() {

    }

    public void download() {

    }

    public void stop() {

    }

}
