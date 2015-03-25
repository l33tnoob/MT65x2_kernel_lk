package com.hissage.ui.view;

import com.hissage.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.SweepGradient;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class NmsColorPickerPanelView extends View {

    private Paint mPaint;
    private Paint mCenterPaint;
    private Paint mLinePaint;
    private Paint mRectPaint;
    private Shader rectShader;
    private float rectLeft;
    private float rectTop;
    private float rectRight;
    private float rectBottom;

    private int mInitialColor;
    private int[] mCircleColors;
    private int[] mRectColors;
    private int mHeight;
    private int mWidth;
    private float r;
    private float centerRadius;

    private float x;
    private float y;
    private float rectx = 0;
    private float recty = 0;
    private float circlex = 0;
    private float circley = 0;
    
    private Bitmap colorPosition;

    private boolean downInCircle = false;
    private boolean downInRect = false;
    boolean inCircle = false;
    boolean inIn = false;

    public NmsColorPickerPanelView(Context context) {
        super(context);
        // mContext = context;
    }

    public void setInitialColor(int c) {
        mInitialColor = c;
        mCenterPaint.setColor(mInitialColor);
        mRectColors[1] = c;
        invalidate();
    }

    public NmsColorPickerPanelView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mCircleColors = new int[] { 0xFFFF0000, 0xFFFF00FF, 0xFF0000FF, 0xFF00FFFF, 0xFF00FF00,
                0xFFFFFF00, 0xFFFF0000 };
        
        colorPosition = Bitmap.createBitmap(BitmapFactory.decodeResource(getResources(),
                R.drawable.ic_color_position));
        Shader s = new SweepGradient(0, 0, mCircleColors, null);
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setShader(s);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeWidth(50);

        mCenterPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mCenterPaint.setStrokeWidth(5);

        mLinePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mLinePaint.setColor(Color.GRAY);
        mLinePaint.setStrokeWidth(4);

        mRectColors = new int[] { 0xFF000000, mCenterPaint.getColor(), 0xFFFFFFFF };
        mRectPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mRectPaint.setStrokeWidth(5);

    }

    @Override
    protected void onDraw(Canvas canvas) {

        this.mHeight = getHeight();
        this.mWidth = getWidth() - 30;

        r = mWidth / 2 * 0.7f - mPaint.getStrokeWidth() * 0.5f;

        rectLeft = r + mPaint.getStrokeWidth() * 0.5f + r / 3;
        rectTop = -r - mPaint.getStrokeWidth() * 0.5f;
        rectRight = r + mPaint.getStrokeWidth() * 0.5f + 2 * r / 3;
        rectBottom = r + mPaint.getStrokeWidth() * 0.5f;

        centerRadius = (r - mPaint.getStrokeWidth() / 2) * 0.7f;

        canvas.translate(mWidth / 2 - 40, mHeight / 2);

        Paint centerPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        centerPaint.setAlpha(0x00);
        // centerPaint.setColor(Color.WHITE);

        centerPaint.setStrokeWidth(5);
        // canvas.drawCircle(0, 0, r, centerPaint);

        int c = mCenterPaint.getColor();

        if (downInCircle) {
            mRectColors[1] = mCenterPaint.getColor();
        }

        if (downInRect) {
            mCenterPaint.setStyle(Paint.Style.FILL);
            mCenterPaint.setColor(c);
        }

        canvas.drawOval(new RectF(-r, -r, r, r), mPaint);

        rectShader = new LinearGradient(rectLeft, rectTop, rectRight, rectBottom, mRectColors,
                null, Shader.TileMode.MIRROR);
        mRectPaint.setShader(rectShader);
        canvas.drawRect(rectLeft, rectTop, rectRight, rectBottom, mRectPaint);

        mCenterPaint.setStyle(Paint.Style.FILL);
        canvas.drawRect(-centerRadius * 0.7f, -centerRadius * 0.7f, centerRadius * 0.7f,
                centerRadius * 0.7f, mCenterPaint);

        float offset = mLinePaint.getStrokeWidth() / 2;

        if (rectx == 0 && recty == 0) {
            rectx = rectRight + offset;
            recty = (rectBottom + rectTop) / 2;
        }

        if (circlex == 0 && circley == 0) {
            circlex = r;
            circley = 0;
        }

//        canvas.drawLine(rectLeft - offset, rectTop - offset * 2, rectLeft - offset, rectBottom
//                + offset * 2, mLinePaint);
//
//        canvas.drawLine(rectLeft - offset * 2, rectTop - offset, rectRight + offset * 2, rectTop
//                - offset, mLinePaint);
//
//        canvas.drawLine(rectRight + offset, rectTop - offset * 2, rectRight + offset, rectBottom
//                + offset * 2, mLinePaint);
//
//        canvas.drawLine(rectLeft - offset * 2, rectBottom + offset, rectRight + offset * 2,
//                rectBottom + offset, mLinePaint);

        if (recty < rectTop - offset * 2) {
            canvas.drawBitmap(colorPosition, rectRight, rectTop - 12, null);
        } else if (recty > rectBottom + offset * 2) {
            canvas.drawBitmap(colorPosition, rectRight, rectBottom - 12, null);
        } else {
            canvas.drawBitmap(colorPosition, rectRight, recty - 12, null);
        }
        
        if (downInCircle) {
            validateCircle(r);
        }
        
        canvas.translate(circlex, circley);
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setColor(Color.WHITE);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(5);
        canvas.drawOval(new RectF(-10, -10, 10, 10), paint);
        
        //canvas.drawBitmap(colorCircle, circlex, circley, mPaint);

        super.onDraw(canvas);
    }

    private void validateCircle(float r){
        double rx = Math.sqrt(circlex*circlex+circley*circley);
        circlex = r*(float)(circlex/rx);
        circley = r*(float)(circley/rx);
    }
    
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {

        x = event.getX() - (mWidth / 2 - 40);
        y = event.getY() - mHeight / 2;
        boolean inCircle = inColorCircle(x, y, r + mPaint.getStrokeWidth() / 2,
                r - mPaint.getStrokeWidth() / 2);

        downInCircle = false;
        downInRect = false;
        boolean inRect = inRect(x, y);

        switch (event.getAction()) {

        case MotionEvent.ACTION_DOWN:
            invalidate();
            break;
        case MotionEvent.ACTION_MOVE:
            if (inCircle) {
                circlex = x;
                circley = y;
                downInCircle = inCircle;
                float angle = (float) Math.atan2(y, x);
                float unit = (float) (angle / (2 * Math.PI));
                if (unit < 0) {
                    unit += 1;
                }
                mCenterPaint.setColor(interpCircleColor(mCircleColors, unit));
            } else if (inRect) {
                rectx = x;
                recty = y;
                downInRect = inRect;
                mCenterPaint.setColor(interpRectColor(mRectColors, y));
            }

            invalidate();
            break;
        case MotionEvent.ACTION_UP:
            invalidate();
            // downInRect = false;
            // downInCircle = false;
            break;
        case MotionEvent.ACTION_CANCEL:
            downInRect = false;
            downInCircle = false;
            // invalidate();
            break;
        }
        return true;
    }

    private boolean inColorCircle(float x, float y, float outRadius, float inRadius) {
        double outCircle = Math.PI * outRadius * outRadius;
        double inCircle = Math.PI * inRadius * inRadius;
        double fingerCircle = Math.PI * (x * x + y * y);
        if (fingerCircle < outCircle && fingerCircle > inCircle) {
            return true;
        } else {
            return false;
        }
    }

    private boolean inCenter(float x, float y, float centerRadius) {
        double centerCircle = Math.PI * centerRadius * centerRadius;
        double fingerCircle = Math.PI * (x * x + y * y);
        if (fingerCircle < centerCircle) {
            return true;
        } else {
            return false;
        }
    }

    private boolean inRect(float x, float y) {
        if (x <= rectRight && x >= rectLeft && y <= rectBottom && y >= rectTop) {
            return true;
        } else {
            return false;
        }
    }

    private int interpCircleColor(int colors[], float unit) {
        if (unit <= 0) {
            return colors[0];
        }
        if (unit >= 1) {
            return colors[colors.length - 1];
        }

        float p = unit * (colors.length - 1);
        int i = (int) p;
        p -= i;

        // now p is just the fractional part [0...1) and i is the index
        int c0 = colors[i];
        int c1 = colors[i + 1];
        int a = ave(Color.alpha(c0), Color.alpha(c1), p);
        int r = ave(Color.red(c0), Color.red(c1), p);
        int g = ave(Color.green(c0), Color.green(c1), p);
        int b = ave(Color.blue(c0), Color.blue(c1), p);

        return Color.argb(a, r, g, b);
    }

    private int interpRectColor(int colors[], float y) {
        int a, r, g, b, c0, c1;
        float p;
        if (y < 0) {
            c0 = colors[0];
            c1 = colors[1];
            p = (y + rectBottom) / rectBottom;
        } else {
            c0 = colors[1];
            c1 = colors[2];
            p = y / rectBottom;
        }
        a = ave(Color.alpha(c0), Color.alpha(c1), p);
        r = ave(Color.red(c0), Color.red(c1), p);
        g = ave(Color.green(c0), Color.green(c1), p);
        b = ave(Color.blue(c0), Color.blue(c1), p);
        return Color.argb(a, r, g, b);
    }

    private int ave(int s, int d, float p) {
        return s + Math.round(p * (d - s));
    }

    public int getCurrentColor() {
        return mCenterPaint.getColor();
    }
}
