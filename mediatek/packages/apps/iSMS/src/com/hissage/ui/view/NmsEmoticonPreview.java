package com.hissage.ui.view;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.drawable.Drawable;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.PopupWindow;

import com.hissage.R;

public class NmsEmoticonPreview {

    private Context mContext;
    private PopupWindow mPopWindow;
    private LayoutInflater mInflater;
    private View mParent;
    private View mContentView;
    private NmsGifView mImage;
    private int mResId;
    private boolean isdismissed = true;

    public NmsEmoticonPreview(Context context, View parent) {
        mContext = context;
        mInflater = LayoutInflater.from(mContext);

        mParent = parent;
        constructRecordWinsow();
    }

    private void constructRecordWinsow() {

        mContentView = mInflater.inflate(R.layout.emoticon_preview, null);

        mImage = (NmsGifView) mContentView.findViewById(R.id.iv_emoticon_img);

        int width = mContext.getResources().getDimensionPixelOffset(R.dimen.emoticon_preview_width);
        int height = mContext.getResources().getDimensionPixelOffset(
                R.dimen.emoticon_preview_height);
        mPopWindow = new PopupWindow(mContentView, width, height);
    }

    public void showWindow() {

        isdismissed = false;
        if (mContext.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
            int offset = mContext.getResources().getDimensionPixelOffset(
                    R.dimen.emoticon_preview_port_offset);
            mPopWindow.showAtLocation(mParent, Gravity.CENTER_HORIZONTAL | Gravity.TOP, 0, offset);
        } else {
            int offset = mContext.getResources().getDimensionPixelOffset(
                    R.dimen.emoticon_preview_land_offset);
            mPopWindow.showAtLocation(mParent, Gravity.CENTER_HORIZONTAL | Gravity.TOP, 0, offset);
        }

        mPopWindow.setFocusable(false);
        mPopWindow.setTouchable(false);
    }

    public void dissWindow() {

        isdismissed = true;
        mPopWindow.dismiss();
    }

    public boolean isShow() {
        return !isdismissed;
    }

    public void setEmoticon(int id) {
        mResId = id;
        mImage.setSource(id);
    }

    public void setEmoticon(Drawable d) {

    }
}
