package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

public class NmsToolSizeView extends View {
    private float mRadius = 6.0f;

    private Paint mPaint = null;
    private static float amplifier = 30.0f;
    private static float frequency = 1.0f;
    private static float phase = 0.0f;
    private int height = 0;
    private int width = 0;

    public NmsToolSizeView(Context context) {
        super(context);
    }

    public void setRadius(float radius) {
        this.mRadius = radius;
        invalidate();
    }

    public NmsToolSizeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mPaint = new Paint();
    }

    // @Override
    protected void onDraw(Canvas canvas) {
        // canvas.drawColor(Color.parseColor("#555555"));
        height = this.getHeight();
        width = this.getWidth();
        mPaint.setAntiAlias(true);
        mPaint.setColor(Color.parseColor("#555555"));
        amplifier = (amplifier * 2 > height) ? (height / 2) : amplifier;
        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setStrokeCap(Paint.Cap.ROUND);
        mPaint.setStrokeJoin(Paint.Join.ROUND);
        mPaint.setStrokeWidth(mRadius);
        float cy = height / 2;
        for (int i = 0; i < width - 1; i++) {
            canvas.drawLine(
                    (float) i,
                    cy
                            - amplifier
                            * (float) (Math.sin(phase * 2 * (float) Math.PI / 360.0f + 2 * Math.PI
                                    * frequency * i / width)),
                    (float) (i + 1),
                    cy
                            - amplifier
                            * (float) (Math.sin(phase * 2 * (float) Math.PI / 360.0f + 2 * Math.PI
                                    * frequency * (i + 1) / width)), mPaint);
        }
        
        canvas.drawCircle(
                0,
                cy
                        - amplifier
                        * (float) (Math.sin(phase * 2 * (float) Math.PI / 360.0f + 2 * Math.PI
                                * frequency * 0 / width)), mRadius / 2, mPaint);
        canvas.drawCircle(
                width - 1,
                cy
                        - amplifier
                        * (float) (Math.sin(phase * 2 * (float) Math.PI / 360.0f + 2 * Math.PI
                                * frequency * (width - 1) / width)), mRadius / 2, mPaint);
    }
}
