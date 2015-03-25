package com.mediatek.cellbroadcastreceiver;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuffXfermode;
import android.graphics.PorterDuff.Mode;
import android.util.AttributeSet;
import android.widget.ScrollView;

public class WindowScrollView extends ScrollView {

    public WindowScrollView(Context context) {
        super(context);
    }

    public WindowScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public WindowScrollView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void onDraw(Canvas canvas) {
        //Clear the canvas before draw;
        Paint paint = new Paint();
        paint.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
        canvas.drawPaint(paint);
        paint.setXfermode(new PorterDuffXfermode(Mode.SRC));

        super.onDraw(canvas);
    }

}
