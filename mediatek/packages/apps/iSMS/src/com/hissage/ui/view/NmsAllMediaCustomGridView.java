package com.hissage.ui.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.GridView;

public class NmsAllMediaCustomGridView extends GridView {

    public NmsAllMediaCustomGridView(Context context) {
        super(context);
        // TODO Auto-generated constructor stub
    }

    public NmsAllMediaCustomGridView(Context context, AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int expandSpec = MeasureSpec.makeMeasureSpec(Integer.MAX_VALUE >> 2, MeasureSpec.AT_MOST);
        super.onMeasure(widthMeasureSpec, expandSpec);
    }

}
