
package com.mediatek.DataUsageLockScreenClient;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Canvas2d;
import com.mediatek.ngin3d.presentation.Graphics2d;

public class CircleDrawer extends Canvas2d {
    private Paint mInCircle;
    private Paint mOutCircle;
    private float mCanvasW;
    private float mCanvasH;
    private float mInCircleRadius;
    private float mOutCircleRadius;

    public CircleDrawer(float radius, int color) {
        setDirtyRect(null);

        mCanvasW = (radius + Constants.OUT_CIRCLE_RADIUS) * 2;
        mCanvasH = (radius + Constants.OUT_CIRCLE_RADIUS) * 2;
        mInCircleRadius = radius;
        mOutCircleRadius = radius + Constants.IN_CIRCLE_RADIUS;

        mInCircle = new Paint();
        mInCircle.setColor(color);
        mInCircle.setAntiAlias(true);
        mInCircle.setStyle(Paint.Style.STROKE);
        mInCircle.setStrokeWidth((float) Constants.OUT_CIRCLE_RADIUS);

        mOutCircle = new Paint();
        mOutCircle.setColor(Constants.OUT_CIRCLE_COLOR_ALPHA_NORMAL);
        mOutCircle.setAntiAlias(true);
        mOutCircle.setStyle(Paint.Style.STROKE);
        mOutCircle.setStrokeWidth((float) Constants.IN_CIRCLE_RADIUS);
    }

    @Override
    protected void drawRect(Box rect, Graphics2d g2d) {
        super.drawRect(rect, g2d);
        Canvas canvas = g2d.beginDraw((int) mCanvasW, (int) mCanvasH, 0);
        canvas.drawCircle(mCanvasW / 2, mCanvasH / 2, mInCircleRadius, mInCircle);
        canvas.drawCircle(mCanvasW / 2, mCanvasH / 2, mOutCircleRadius, mOutCircle);
        g2d.endDraw();
    }

    public void setOutCircleColor(int color) {
        mOutCircle.setColor(color);
    }
}
