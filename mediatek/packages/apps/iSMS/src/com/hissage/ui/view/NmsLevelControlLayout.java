package com.hissage.ui.view;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.widget.Scroller;

public class NmsLevelControlLayout extends ViewGroup {

    private static final String TAG = "ScrollControlLayout";

    public static boolean startTouch = true;

    private boolean canMove = true;

    private Scroller scroller;
    private VelocityTracker velocityTracker;

    private int curScreen;
    private int defaultScreen = 1;

    private static final int TOUCH_STATE_REST = 0;
    private static final int TOUCH_STATE_SCROLLING = 1;

    private int touchState = TOUCH_STATE_REST;
    private static final int SNAP_VELOCITY = 600;

    private int touchSlop;

    private float lastMotionX;
    private float lastMotionY;

    private OnScrollToScreenListener onScrollToScreen = null;

    public NmsLevelControlLayout(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public NmsLevelControlLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        scroller = new Scroller(context);
        curScreen = defaultScreen;
        touchSlop = ViewConfiguration.get(getContext()).getScaledTouchSlop();
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        int childLeft = 0;
        final int childCount = getChildCount();
        for (int i = 0; i < childCount; i++) {
            final View childView = getChildAt(i);
            if (childView.getVisibility() != View.GONE) {
                final int childWidth = childView.getMeasuredWidth();
                childView.layout(childLeft, 0, childLeft + childWidth,
                        childView.getMeasuredHeight());
                childLeft += childWidth;
            }
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        final int width = MeasureSpec.getSize(widthMeasureSpec);
        final int widthMode = MeasureSpec.getMode(widthMeasureSpec);
        if (widthMode != MeasureSpec.EXACTLY) {
            throw new IllegalStateException("ScrollLayout only canmCurScreen run at EXACTLY mode!");
        }

        final int heightMode = MeasureSpec.getMode(heightMeasureSpec);
        if (heightMode != MeasureSpec.EXACTLY) {
            throw new IllegalStateException("ScrollLayout only can run at EXACTLY mode!");
        }
        // The children are given the same width and height as the scrollLayout
        final int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i).measure(widthMeasureSpec, heightMeasureSpec);
        }
        // Log.e(TAG, "moving to screen "+mCurScreen);
        scrollTo(curScreen * width, 0);
        doScrollAction(curScreen);
    }

    public void setTouchMove(boolean canMove) {
        this.canMove = canMove;
    }

    public void snapToDestination() {
        final int screenWidth = getWidth();
        final int destScreen = (getScrollX() + screenWidth / 2) / screenWidth;
        snapToScreen(destScreen);
    }

    public void snapToScreen(int whichScreen) {
        whichScreen = Math.max(0, Math.min(whichScreen, getChildCount() - 1));
        if (getScrollX() != (whichScreen * getWidth())) {
            final int delta = whichScreen * getWidth() - getScrollX();
            scroller.startScroll(getScrollX(), 0, delta, 0, Math.abs(delta) * 2);
            curScreen = whichScreen;
            doScrollAction(curScreen);
            invalidate();
        }
    }

    public void setToScreen(int whichScreen) {
        whichScreen = Math.max(0, Math.min(whichScreen, getChildCount() - 1));
        curScreen = whichScreen;
        scrollTo(whichScreen * getWidth(), 0);
        doScrollAction(whichScreen);
    }

    public int getCurScreen() {
        return curScreen;
    }

    public void autoRecovery() {
        scroller.abortAnimation();
    }

    @Override
    public void computeScroll() {
        // TODO Auto-generated method stub
        if (scroller.computeScrollOffset()) {
            scrollTo(scroller.getCurrX(), scroller.getCurrY());
            postInvalidate();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (velocityTracker == null) {
            velocityTracker = VelocityTracker.obtain();
        }

        if (!canMove) {
            return true;
        }

        velocityTracker.addMovement(event);
        final int action = event.getAction();
        final float x = event.getX();
        // final float y = event.getY();
        switch (action) {
        case MotionEvent.ACTION_DOWN:
            if (!scroller.isFinished()) {
                scroller.abortAnimation();
            }
            lastMotionX = x;
            break;
        case MotionEvent.ACTION_MOVE:
            int deltaX = (int) (lastMotionX - x);
            int scrollX = (int) getScrollX();
            int childCount = getChildCount();
            lastMotionX = x;

            if ((deltaX < 0 && Math.abs(deltaX) > scrollX)
                    || (deltaX > 0 && deltaX > ((childCount - 1) * getWidth() - scrollX))) {
                // do nothing
            } else {
                scrollBy(deltaX, 0);
            }
            break;
        case MotionEvent.ACTION_UP:
            final VelocityTracker velocityTrackers = velocityTracker;
            velocityTracker.computeCurrentVelocity(1000);
            int velocityX = (int) velocityTrackers.getXVelocity();
            if (velocityX > SNAP_VELOCITY && curScreen > 0) {
                // Fling enough to move left
                Log.e(TAG, "snap left");
                snapToScreen(curScreen - 1);
            } else if (velocityX < -SNAP_VELOCITY && curScreen < getChildCount() - 1) {
                // Fling enough to move right
                snapToScreen(curScreen + 1);
            } else {
                snapToDestination();
            }

            if (velocityTracker != null) {
                velocityTracker.recycle();
                velocityTracker = null;
            }
            touchState = TOUCH_STATE_REST;
            break;
        case MotionEvent.ACTION_CANCEL:
            touchState = TOUCH_STATE_REST;
            break;
        default:
            break;
        }
        return true;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        // TODO Auto-generated method stub
        final int action = ev.getAction();
        if ((action == MotionEvent.ACTION_MOVE) && (touchState != TOUCH_STATE_REST)) {
            return true;
        }
        final float x = ev.getX();
        final float y = ev.getY();
        switch (action) {
        case MotionEvent.ACTION_DOWN:
            lastMotionX = x;
            lastMotionY = y;
            touchState = scroller.isFinished() ? TOUCH_STATE_REST : TOUCH_STATE_SCROLLING;
            break;
        case MotionEvent.ACTION_MOVE:
            final int xDiff = (int) Math.abs(lastMotionX - x);
            if (xDiff > touchSlop) {
                if (Math.abs(lastMotionY - y) / Math.abs(lastMotionX - x) < 1)
                    touchState = TOUCH_STATE_SCROLLING;
            }
            break;
        case MotionEvent.ACTION_CANCEL:
        case MotionEvent.ACTION_UP:
            touchState = TOUCH_STATE_REST;
            break;
        default:
            break;
        }
        return touchState != TOUCH_STATE_REST;
    }

    private void doScrollAction(int whichScreen) {
        if (onScrollToScreen != null) {
            onScrollToScreen.doAction(whichScreen);
        }
    }

    public void setOnScrollToScreen(OnScrollToScreenListener paramOnScrollToScreen) {
        onScrollToScreen = paramOnScrollToScreen;
    }

    public void setDefaultScreen(int position) {
        curScreen = position;
    }

    public abstract interface OnScrollToScreenListener {
        public void doAction(int whichScreen);
    }
}
