package com.hissage.ui.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.hissage.R;

public class NmsLocationCustomView extends LinearLayout {

    public interface OnLocCustomViewClickListener {
        public void onRefreshLocation();

        public void onEditLocation(CharSequence text);

        public void onShareLoaction(CharSequence textOne, CharSequence textTwo);
    }

    private Context mContext;

    private View mConvertView;
    private LinearLayout mTextShow;
    private TextView mLineOne;
    private TextView mLineTwo;
    private ImageView mRefresh;
    private ImageView mEdit;
    private View mline;
    private TextView mLineThree;
    private ProgressBar mWait;
    private ListView mNearby;

    private OnLocCustomViewClickListener mListener;

    public void setOnLocCustomViewClickListener(OnLocCustomViewClickListener listener) {
        this.mListener = listener;
    }

    public NmsLocationCustomView(Context context) {
        super(context);
        mContext = context;

        init();
    }

    public NmsLocationCustomView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;

        init();
    }

    private void init() {
        mConvertView = LayoutInflater.from(mContext).inflate(R.layout.location_custom_view, this,
                true);

        mTextShow = (LinearLayout) mConvertView.findViewById(R.id.ll_show);
        mLineOne = (TextView) mConvertView.findViewById(R.id.tv_one);
        mLineTwo = (TextView) mConvertView.findViewById(R.id.tv_two);
        mRefresh = (ImageView) mConvertView.findViewById(R.id.ib_refresh);
        mEdit = (ImageView) mConvertView.findViewById(R.id.ib_edit);
        mline = (View) mConvertView.findViewById(R.id.v_line);
        mLineThree = (TextView) mConvertView.findViewById(R.id.tv_three);
        mWait = (ProgressBar) mConvertView.findViewById(R.id.pb_wait);
        mNearby = (ListView) mConvertView.findViewById(R.id.lv_nearby);

        mTextShow.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mListener.onShareLoaction(null, mLineTwo.getText());
            }
        });

        mRefresh.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mListener.onRefreshLocation();
            }
        });

        mEdit.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mListener.onEditLocation(mLineTwo.getText());
            }
        });

        mNearby.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                TextView lineOne = (TextView) arg1.findViewById(R.id.tv_one);
                TextView lineTwo = (TextView) arg1.findViewById(R.id.tv_two);
                mListener.onShareLoaction(lineOne.getText().toString(), lineTwo.getText()
                        .toString());
            }
        });
    }

    public ListView getNearbyListView() {
        return mNearby;
    }

    public void setTextShowClickable(boolean clickable) {
        mTextShow.setClickable(clickable);
    }

    public void setTextViewText(CharSequence textOne, CharSequence textTwo, CharSequence textThree) {
        if (textOne != null)
            mLineOne.setText(textOne);
        if (textTwo != null)
            mLineTwo.setText(textTwo);
        if (textThree != null)
            mLineThree.setText(textThree);
    }

    public void setTextViewVisibility(int visibilityOne, int visibilityTwo, int visibilityThree) {
        if (visibilityOne != -1)
            mLineOne.setVisibility(visibilityOne);
        if (visibilityTwo != -1)
            mLineTwo.setVisibility(visibilityTwo);
        if (visibilityThree != -1)
            mLineThree.setVisibility(visibilityThree);
    }

    public void setImageButtonVisibility(int visibilityRefresh, int visibilityEdit) {
        mRefresh.setVisibility(visibilityRefresh);
        mEdit.setVisibility(visibilityEdit);
    }

    public void setLineVisibility(int visibility) {
        mline.setVisibility(visibility);
    }

    public void setProgressBar(int visibility) {
        mWait.setVisibility(visibility);
    }

    public void setListViewVisibility(int visibility) {
        mNearby.setVisibility(visibility);
    }
}
