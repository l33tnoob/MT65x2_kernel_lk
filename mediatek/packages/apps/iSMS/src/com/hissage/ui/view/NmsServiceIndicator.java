package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.widget.TextView;

import com.hissage.config.NmsCommonUtils;

public class NmsServiceIndicator extends TextView {

    private int mServiceColor;
    private Context mContext;
    private boolean mRedraw = false;
    public NmsServiceIndicator(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public NmsServiceIndicator(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public void setIndicator(CharSequence text, boolean redraw){
        mRedraw = redraw;
        super.setText(text);
    }
    
    @Override
    public void onDraw(Canvas canvas) {

        if (mRedraw) {
            final Paint paint = new Paint();
            Bitmap bitmap = null;
            paint.setColor(Color.BLUE);
            final Rect rect = new Rect(0, 0, getWidth(), getHeight());
            final RectF rectF = new RectF(rect);
            final float roundPx = 4;

            paint.setAntiAlias(true);
            canvas.drawRoundRect(rectF, roundPx, roundPx, paint);
            if (mServiceColor != 0) {
                bitmap = BitmapFactory.decodeResource(getResources(), mServiceColor);
                bitmap = NmsCommonUtils.resizeImage(bitmap, getWidth(), getHeight(), false);
                paint.setXfermode(new PorterDuffXfermode(Mode.SRC_IN));
                canvas.drawBitmap(bitmap, rect, rect, null);
                if (null != bitmap) {
                    bitmap.recycle();
                }
            }
        }
        super.onDraw(canvas);
    }

    public void setServiceColor(int color) {
        mServiceColor = color;
    }
}
