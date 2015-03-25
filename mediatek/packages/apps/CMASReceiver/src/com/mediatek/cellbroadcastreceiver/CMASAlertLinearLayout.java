package com.mediatek.cellbroadcastreceiver;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.widget.LinearLayout;

public class CMASAlertLinearLayout extends LinearLayout {

    Context mContext;
    CMASAlertLinearLayout(Context context) {
        super(context);
        mContext = context;
    }

    public CMASAlertLinearLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            switch (event.getKeyCode()) {
                case KeyEvent.KEYCODE_VOLUME_UP:
                case KeyEvent.KEYCODE_VOLUME_DOWN:
                    mContext.stopService(new Intent(mContext, CellBroadcastAlertAudio.class));
                    return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }
}