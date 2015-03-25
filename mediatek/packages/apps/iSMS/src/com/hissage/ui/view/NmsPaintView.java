package com.hissage.ui.view;

import java.util.ArrayList;

import com.hissage.ui.activity.NmsSketchActivity.Tools;
import com.hissage.util.log.NmsLog;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class NmsPaintView extends View {

    public String Tag = "PaintView";
    
    private Paint mPaint = null;
    private Bitmap mBitmap;
    private ArrayList<ActionTool> mActionList = null;
    private ActionTool mCurAction = null;
    private int mCurrentPaintIndex = -1;
    private Path mPath;
    private Paint mBitmapPaint;
    private float mX, mY;
    private Canvas mCanvas;
    private static final float TOUCH_TOLERANCE = 4;

    private int mColor;
    private int mSize;
    private Tools mTool;
    
    public boolean mIsDraw = false;

    public NmsPaintView(Context context, AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
        mPaint = new Paint();

        mPaint.setAntiAlias(true);
        mPaint.setDither(true);
        mPaint.setColor(Color.WHITE);
        mPaint.setStrokeWidth(12);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeJoin(Paint.Join.ROUND);
        mPaint.setStrokeCap(Paint.Cap.ROUND);

        mCurAction = new Pen();
        mColor = Color.RED;
        mSize = 12;
        mTool = Tools.PEN;

        mPath = new Path();
        mBitmapPaint = new Paint(Paint.DITHER_FLAG);
        mActionList = new ArrayList<ActionTool>();
    }

    // @Override
    protected void onDraw(Canvas canvas) {
        if (mTool == Tools.PEN) {
            mPaint.setColor(mColor);
        }
        mPaint.setStrokeWidth(mSize);
        canvas.drawColor(0xffffffff);
        canvas.drawBitmap(mBitmap, 0, 0, mBitmapPaint);
        
        if (mPath != null) {
            canvas.drawPath(mPath, mPaint);
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        mBitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        mCanvas = new Canvas(mBitmap);
    }

    private void drawMainPanel(Canvas canvas) {
        mBitmap = Bitmap.createBitmap(this.getWidth(), this.getHeight(), Config.ARGB_8888);
        mCanvas.setBitmap(mBitmap);
        for (int i = 0; i <= mCurrentPaintIndex; i++) {
            mActionList.get(i).draw(mCanvas);
        }

        if (mCurAction != null) {
            mCurAction.draw(mCanvas);
        }

        invalidate();
    }

    private void setCurrentActionType(float x, float y) {
        switch (mTool) {
        case PEN:
            mCurAction = new Pen(x, y, mSize, mColor);
            break;
        case ERASER:
            mCurAction = new Eraser(x, y, mSize, mColor);
            break;
        default:
            NmsLog.trace(Tag, "The tools is not handle");
        }
    }

    private void touchStart(float x, float y) {
        mPath.moveTo(x, y);
        mX = x;
        mY = y;
    }

    private void touchMove(float x, float y) {
        float dx = Math.abs(x - mX);
        float dy = Math.abs(y - mY);
        if (dx >= TOUCH_TOLERANCE || dy >= TOUCH_TOLERANCE) {
            mPath.quadTo(mX, mY, (x + mX) / 2, (y + mY) / 2);
            mX = x;
            mY = y;
        }
    }

    private void touchUp() {
        mPath.lineTo(mX, mY);
        mCanvas.drawPath(mPath, mPaint);
        mPath = null;
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
            mPath = new Path();
            setCurrentActionType(touchX, touchY);
            clearUndoSteps();
            touchStart(touchX, touchY);
            invalidate();
            break;
        case MotionEvent.ACTION_MOVE:
            if (mCurAction != null) {
                mCurAction.move(touchX, touchY);
                mIsDraw = true;
            }

            touchMove(touchX, touchY);
            invalidate();
            break;
        case MotionEvent.ACTION_UP:
            if (mCurAction != null) {
                mCurAction.move(touchX, touchY);
                mActionList.add(mCurAction);
                mCurrentPaintIndex++;
                mCurAction = null;
            }
            touchUp();
            invalidate();
            break;
        }
        return true;
    }

    private void clearUndoSteps() {
        for (int i = mActionList.size() - 1; i > mCurrentPaintIndex; i--) {
            mActionList.remove(i);
        }
    }

    public int undo() {
        if (mCurrentPaintIndex >= 0) {
            mCurrentPaintIndex--;
            drawMainPanel(mCanvas);
            return 0;
        }
        
        return -1;

       
    }

    public int redo() {
        if ((mCurrentPaintIndex + 1) < mActionList.size()) {
            mCurrentPaintIndex++;
            drawMainPanel(mCanvas);
            return 0;
        } 
        
        return -1;
    }

    public void setSize(int size) {
        this.mSize = size;
    }

    public void setColor(int color) {
        this.mColor = color;
    }

    public void setTool(Tools tool) {
        this.mTool = tool;
        switch (mTool) {
        case PEN:
            mPaint.setColor(mColor);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeJoin(Paint.Join.ROUND);
            mPaint.setStrokeCap(Paint.Cap.ROUND);
            mPaint.setXfermode(null);
            break;
        case ERASER:
            //mPaint.setAlpha(0);
            mPaint.setStyle(Paint.Style.STROKE);
            mPaint.setStrokeJoin(Paint.Join.ROUND);
            mPaint.setStrokeCap(Paint.Cap.SQUARE);
            mPaint.setColor(Color.WHITE);
            //mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
            break;
        default:
            NmsLog.trace(Tag, "The tools is not handle");
        }
    }

    public void clear() {
        mIsDraw = false;
        mCurrentPaintIndex = -1;
        if (mActionList != null) {
            mActionList.clear();
        }
        mBitmap = Bitmap.createBitmap(this.getWidth(), this.getHeight(), Bitmap.Config.ARGB_8888);
        mCanvas = new Canvas(mBitmap);
        invalidate();
    }
    
    public Bitmap buildSketchBitmap(){
        if (mIsDraw == false) {
            return null;
        }
        
        Bitmap bitmaps = Bitmap.createBitmap(mBitmap, 0, 0, getWidth(), getHeight());
        Paint paint = new Paint();   
        Canvas canvas = new Canvas(bitmaps);
        canvas.drawColor(Color.WHITE);
        canvas.drawBitmap(mBitmap, 0, 0, paint);
        return bitmaps;
    } 
}

abstract class ActionTool {
    protected int color;
    protected int size;

    ActionTool() {
        color = Color.BLACK;
    }

    ActionTool(int color) {
        this.color = color;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public abstract void draw(Canvas canvas);

    public abstract void move(float mx, float my);

}

class Pen extends ActionTool {
    Path path;

    Pen() {
        super();
        path = new Path();
    }

    Pen(float x, float y, int size, int color) {
        super(color);
        path = new Path();
        this.size = size;
        path.moveTo(x, y);
        path.lineTo(x, y);
    }

    public void draw(Canvas canvas) {
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setDither(true);
        paint.setColor((int)color);
        paint.setStrokeWidth(size);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);
        paint.setStrokeCap(Paint.Cap.ROUND);
        canvas.drawPath(path, paint);
    }

    public void move(float mx, float my) {
        path.lineTo(mx, my);
    }
}

class Eraser extends ActionTool {
    Path path;

    Eraser() {
        super();
        path = new Path();
    }

    Eraser(float x, float y, int size, int color) {
        super(color);
        path = new Path();
        this.size = size;
        path.moveTo(x, y);
        path.lineTo(x, y);
    }

    public void draw(Canvas canvas) {
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setDither(true);
        
        paint.setColor(Color.WHITE);
        paint.setStrokeWidth(size);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeJoin(Paint.Join.ROUND);
        paint.setStrokeCap(Paint.Cap.SQUARE);
        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
        canvas.drawPath(path, paint);
        
        canvas.drawPath(path, paint);
    }

    public void move(float mx, float my) {
        path.lineTo(mx, my);
    }

}
