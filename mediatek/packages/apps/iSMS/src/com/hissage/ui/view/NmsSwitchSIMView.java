package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.ImageSpan;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.util.message.MessageUtils;

public class NmsSwitchSIMView extends LinearLayout {

    private Context mContext;
    private Button mSwitchSim;
    private Button mEnableSim;
    private TextView mText;
    private ActionListener mListener;

    public final static int ENABLE = 100;
    public final static int SWITCH = 101;

    public NmsSwitchSIMView(Context context) {
        super(context);
        mContext = context;
    }

    public NmsSwitchSIMView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mText = (TextView) findViewById(R.id.tv_switch_text);
        mEnableSim = (Button) findViewById(R.id.sw_enable_sim);
        mEnableSim.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mListener.doAction(ENABLE);
            }
        });
        mSwitchSim = (Button) findViewById(R.id.btn_switch_sim);
        mSwitchSim.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                mListener.doAction(SWITCH);
            }
        });

    }

    public void setSwitchMode(int simId) {
        String simName = MessageUtils.getServiceIndiactorName(mContext, simId,
                NmsIpMessageConsts.NmsMessageProtocol.IP);
        String text = String.format(getResources().getString(R.string.STR_NMS_SWITCH_TEXT_FORMAT),
                simName);
        setText(simId, text);
        mSwitchSim.setVisibility(View.VISIBLE);
        mEnableSim.setVisibility(View.GONE);
    }

    public void setEnableMode(int simId) {
        String simName = MessageUtils.getServiceIndiactorName(mContext, simId,
                NmsIpMessageConsts.NmsMessageProtocol.IP);
        String text = String.format(getResources().getString(R.string.STR_NMS_ENABLE_TEXT_FORMAT),
                simName);
        setText(simId, text);
        mSwitchSim.setVisibility(View.GONE);
        mEnableSim.setVisibility(View.VISIBLE);
    }

    public void setLeaveNotice() {
        mSwitchSim.setVisibility(View.GONE);
        mEnableSim.setVisibility(View.GONE);
        mText.setText(R.string.STR_NMS_LEAVE_GROUP_NOTICE);
    }
    
    public void setDbStoreFullNotice(){
        mSwitchSim.setVisibility(View.GONE);
        mEnableSim.setVisibility(View.GONE);
        mText.setText(R.string.STR_NMS_STORE_STATUS_FULL);
    }

    public void setLoseSimNotice() {
        mSwitchSim.setVisibility(View.GONE);
        mEnableSim.setVisibility(View.GONE);
        mText.setText(R.string.STR_NMS_LOSE_SIM_NOTICE);
    }

    private void setText(int simId, String text) {
        CharSequence simSeq = MessageUtils.getMTKServiceIndicator(mContext, simId);
        String simName = MessageUtils.getServiceIndiactorName(mContext, simId,
                NmsIpMessageConsts.NmsMessageProtocol.IP);
        int resId = MessageUtils.getServiceIndicatorColor(mContext, simId);
        int index = text.indexOf(simName);

        if (simSeq != null) {
            SpannableStringBuilder style = new SpannableStringBuilder();
            style.append(text.subSequence(0, index));
            style.append(simSeq);
            style.append(text.subSequence(index + simName.length(), text.length()));
            int textColor = mContext.getResources().getColor(R.color.switch_sim_text_color);
            style.setSpan(new ForegroundColorSpan(textColor), 0, index,
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            style.setSpan(new ForegroundColorSpan(textColor), index + simSeq.length(),
                    style.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            mText.setTextColor(Color.WHITE);
            mText.setText(style);
        } else {
            Spannable span = new SpannableString(text);
            if (resId == 0) {
                span.setSpan(new BackgroundColorSpan(Color.BLUE), index, index + simName.length(),
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            } else {
                Bitmap b = BitmapFactory.decodeResource(getResources(), resId);
                if (b == null) {
                    span.setSpan(new BackgroundColorSpan(Color.BLUE), index,
                            index + simName.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                } else {
                    for (int j = 0; j < simName.length(); j++) {
                        String t = simName.substring(j, j + 1);
                        Paint p = new Paint();
                        p.setAntiAlias(true);
                        p.setTextSize(mText.getTextSize());
                        p.setColor(Color.WHITE);
                        float width = p.measureText(t);
                        Bitmap b1 = BitmapFactory.decodeResource(getResources(), resId);
                        b1 = NmsCommonUtils.resizeImage(b1, (int) width,
                                (int) mText.getTextSize() + 5, false);
                        Canvas canvas = new Canvas(b1);
                        canvas.drawText(t, 0, mText.getTextSize(), p);
                        ImageSpan i = new ImageSpan(mContext, b1);
                        span.setSpan(i, index + j, index + j + 1,
                                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    }
                }
            }
            mText.setText(span);
        }
    }

    public void setActionListener(ActionListener l) {
        mListener = l;
    }

    public interface ActionListener {
        public void doAction(int type);
    }
}
