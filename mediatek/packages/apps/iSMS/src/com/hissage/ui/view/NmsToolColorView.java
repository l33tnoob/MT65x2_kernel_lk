package com.hissage.ui.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.hissage.R;

public class NmsToolColorView extends View {

    private Context mContext;
    private int currentColor = Color.BLACK;

    private int currentIndex = -2;
    private int colorX = 15;
    private int colorY = 15;
    private int colorSide = 30;
    private int colorWidthNum = 6;
    private int colorHeightNum = 3;

    private Bitmap colorChoice;

    private int[] color = { 0xffe01f26, 0xfff26522, 0xfff7941d, 0xffffd800, 0xff80b834, 0xff19a82d,
            0xff00a669, 0xff00a99d, 0xff00aeef, 0xff0072bc, 0xff2e3192, 0xff003663, 0xff662d91,
            0xff92278f, 0xffec008c, 0xffed145b, 0xff000000, 0xff333333 };

    private int[] customColor;

    public void setCustomColorCell(int[] cColor) {
        customColor = cColor;
    }

    public void setCurrentColor(int c) {
        currentColor = c;
        for (int i = 0; i < color.length; i++) {
            if (c == color[i]) {
                currentIndex = i;
                break;
            }
        }
    }

    public int getCurrentColor() {
        return currentColor;
    }

    public NmsToolColorView(Context context) {
        super(context);
        mContext = context;
    }

    public NmsToolColorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        colorChoice = Bitmap.createBitmap(BitmapFactory.decodeResource(getResources(),
                R.drawable.ic_color_selected));

    }

    // @Override
    protected void onDraw(Canvas canvas) {
        colorSide = (this.getWidth() - 30) / 6;
        drawColrPanel(canvas);
    }

    private void drawColrPanel(Canvas canvas) {
        drawPaintColorPanel(canvas);
    }

    private void drawPaintColorPanel(Canvas canvas) {
        Paint paint = new Paint();

        paint.setAntiAlias(true);
        paint.setColor(Color.BLACK);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(2);

        paint.setColor(Color.WHITE);
        paint.setStyle(Paint.Style.STROKE);
        for (int i = 0; i < colorWidthNum; i++)
            for (int j = 0; j < colorHeightNum; j++) {
                int cellX = colorX + i * colorSide;
                int cellY = colorY + j * colorSide;
                int cellBX = colorX + (i + 1) * colorSide;
                int cellBY = colorY + (j + 1) * colorSide;
                int paintColor = i + j * colorWidthNum;

                drawColorText(canvas, cellX, cellY, cellBX, cellBY, paintColor);

                if (currentIndex == paintColor) {
                    // canvas.drawRect(cellX, cellY, cellBX, cellBY, paint);
                    drawChoiceColor(canvas, cellBX, cellBY);
                }
            }

        drawLineCell(canvas);

        drawCustomColorCell(canvas);
    }

    private void drawChoiceColor(Canvas canvas, int left, int top) {
        canvas.drawBitmap(colorChoice, left - 40, top - 40, null);
    }

    private void drawColorText(Canvas canvas, int cellX, int cellY, int cellBX, int cellBY,
            int paintColor) {

        drawCellColor(canvas, cellX, cellY, cellBX, cellBY, color[paintColor]);

    }

    private void drawLineCell(Canvas canvas) {

        int cellX = colorX;
        int cellY = colorY + 3 * colorSide + 10;
        int cellBX = colorX + (4 + 1 + 1) * colorSide;
        int cellBY = cellY;

        Paint bpaint = new Paint();

        bpaint.setAntiAlias(true);
        bpaint.setStyle(Paint.Style.STROKE);
        bpaint.setStrokeWidth(1);
        bpaint.setColor(Color.GRAY);

        canvas.drawLine(cellX, cellY, cellBX, cellBY, bpaint);
    }

    private void drawCustomColorCell(Canvas canvas) {
        Paint bpaint = new Paint();

        bpaint.setAntiAlias(true);
        bpaint.setColor(Color.BLACK);
        bpaint.setStyle(Paint.Style.STROKE);
        bpaint.setStrokeWidth(2);
        bpaint.setColor(Color.WHITE);
        bpaint.setStyle(Paint.Style.STROKE);

        for (int j = 0; j < colorWidthNum; j++) {
            int cellX = colorX + j * colorSide;
            int cellY = colorY + 3 * colorSide + 20;
            int cellBX = colorX + (j + 1) * colorSide;
            int cellBY = colorY + (3 + 1) * colorSide + 20;

            drawCellColor(canvas, cellX, cellY, cellBX, cellBY, customColor[j]);

            if (currentIndex - 18 == j) {
                // canvas.drawRect(cellX, cellY, cellBX, cellBY, bpaint);
                drawChoiceColor(canvas, cellBX, cellBY);
            }
        }
    }

    private void drawCellColor(Canvas canvas, int cellX, int cellY, int cellBX, int cellBY,
            long color) {
        Paint paint = new Paint();

        paint.setColor((int) color);
        paint.setStyle(Paint.Style.FILL);
        canvas.drawRect(cellX + 8, cellY + 8, cellBX - 8, cellBY - 8, paint);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int antion = event.getAction();
        if (antion == MotionEvent.ACTION_CANCEL) {
            return false;
        }

        float touchX = event.getX();
        float touchY = event.getY();

        switch (event.getAction()) {
        case MotionEvent.ACTION_DOWN:

            boolean ret = testTouchColorPanel(touchX, touchY);
            invalidate();
            return ret;
        case MotionEvent.ACTION_MOVE:
            invalidate();
            break;
        case MotionEvent.ACTION_UP:
            invalidate();
            break;
        default:

        }

        return true;
    }

    public boolean testTouchColorPanel(float x, float y) {
        if (x > colorX && y > colorY && x < colorX + colorSide * colorWidthNum
                && y < colorY + colorSide * colorHeightNum) {

            int tx = (int) ((x - colorX) / colorSide);
            int ty = (int) ((y - colorY) / colorSide);
            int index = ty * colorWidthNum + tx;

            currentIndex = index;
            currentColor = color[index];

            return true;
        }

        if (x > colorX && y > +10 + colorY + colorHeightNum * colorSide
                && x < colorX + colorSide * colorWidthNum
                && y < 20 + colorY + (colorHeightNum + 1) * colorSide) {

            int tx = (int) ((x - colorX) / colorSide);
            currentIndex = tx + 18;
            currentColor = customColor[currentIndex - 18];

            return true;
        }

        return false;
    }
}
