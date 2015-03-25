package com.mediatek.engineermode.tests;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.Instrumentation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.graphics.Point;
import android.net.Uri;
import android.os.SystemClock;
import android.test.InstrumentationTestCase;
import android.test.TouchUtils;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.MotionEvent.PointerCoords;
import android.view.MotionEvent.PointerProperties;
import android.view.View;

import com.mediatek.xlog.Xlog;

import java.util.Random;
import java.util.ArrayList;

public class TestCaseUtils {
    public static String TAG = "TouchScreenTest";
    public static long DEFAULT_SLEEP_TIME = 300;
    public static long SECOND = 1000;
    public static final String MAIL_TYPE_GOOGLE = "com.google";

    private static final int EVENT_MIN_INTERVAL = 1000;

    private static ArrayList<Uri> mTestAccountUris = new ArrayList<Uri>();

    /**
     * sleep for time.
     * 
     * @param time The time to sleep
     */
    public static void sleepForTime(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private static int MAX_STEP = 20;
    private static int FINGER_NUMBER = 4;

    static class Finger {
        public PointerProperties properties;
        public PointerCoords coords;

        private Point start = new Point();
        private Point end = new Point();
        private float step;
        private float stepX;
        private float stepY;

        private int mLeft;
        private int mRight;
        private int mTop;
        private int mBottom;

        public boolean isOnScreen = false;
        public boolean isLogging = false;

        public Finger(int id, int left, int top, int right, int bottom) {
            mLeft = left;
            mRight = right;
            mTop = top;
            mBottom = bottom;

            properties = new PointerProperties();
            properties.id = id;
            properties.toolType = MotionEvent.TOOL_TYPE_FINGER;
            isOnScreen = false;
            isLogging = false;
            resetPos();
        }

        private void resetPos() {
            Random random = new Random();
            start.x = random.nextInt(mRight - mLeft) + mLeft;
            start.y = random.nextInt(mBottom - mTop) + mTop;
            end.x = random.nextInt(mRight - mLeft) + mLeft;
            end.y = random.nextInt(mBottom - mTop) + mTop;

            step = random.nextInt(MAX_STEP) + 1;
            stepX = (end.x - start.x) / step;
            stepY = (end.y - start.y) / step;

            coords = new PointerCoords();
            coords.x = start.x;
            coords.y = start.y;
            coords.pressure = 1;
            coords.size = 1;
        }

        public void down() {
            isOnScreen = true;
            isLogging = true;
            resetPos();
        }

        public void up() {
            isOnScreen = false;
        }

        public boolean move() {
            coords.x += stepX;
            coords.y += stepY;
            step = step - 1;
            if (step <= 0) {
                Random random = new Random();
                start.x = end.x;
                start.y = end.y;
                end.x = random.nextInt(mRight - mLeft) + mLeft;
                end.y = random.nextInt(mBottom - mTop) + mTop;

                step = random.nextInt(MAX_STEP) + 1;
                stepX = (end.x - start.x) / step;
                stepY = (end.y - start.y) / step;

                coords.x = start.x;
                coords.y = start.y;
            }
            return true;
        }
    };

    static private void printAtLog(Finger[] fingers) {
        Xlog.d(TAG, "generateDownUpEvents log start");
        int count = 0;
        for (int j = 0; j < fingers.length; j++) {
            if (!fingers[j].isLogging) {
                continue;
            }
            Xlog.d(TAG, "Touch pos: " + (int) fingers[j].coords.x + "," + (int) fingers[j].coords.y);
            if (fingers[j].isOnScreen) {
                count++;
            }
        }
        Xlog.d(TAG, "generateDownUpEvents log end, count = " + count);
    }

    private static void down(Instrumentation inst, long downTime, long eventTime, Finger[] fingers, int fingerId) {
        MotionEvent event;
        int count = 0;
        int action = 0;
        PointerProperties[] properties = new PointerProperties[FINGER_NUMBER];
        PointerCoords[] pointerCoords = new PointerCoords[FINGER_NUMBER];

        fingers[fingerId].down();

        for (int j = 0; j < fingers.length; j++) {
            if (fingers[j].isOnScreen) {
                properties[count] = fingers[j].properties;
                pointerCoords[count] = fingers[j].coords;
                count = count + 1;
            }
        }

        if (count == 1) {
            action = MotionEvent.ACTION_DOWN;
        } else {
            for (int j = 0; j < count; j++) {
                if (properties[j].id == fingerId) {
                    action = MotionEvent.ACTION_POINTER_DOWN | (j << MotionEvent.ACTION_POINTER_INDEX_SHIFT);
                }
            }
        }

        event = MotionEvent.obtain(downTime, eventTime, action, count, properties,
                pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
        
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        printAtLog(fingers);
    }

    private static void up(Instrumentation inst, long downTime, long eventTime, Finger[] fingers, int fingerId) {
        MotionEvent event;
        int count = 0;
        int action = 0;
        PointerProperties[] properties = new PointerProperties[FINGER_NUMBER];
        PointerCoords[] pointerCoords = new PointerCoords[FINGER_NUMBER];

        for (int j = 0; j < fingers.length; j++) {
            if (fingers[j].isOnScreen) {
                properties[count] = fingers[j].properties;
                pointerCoords[count] = fingers[j].coords;
                count = count + 1;
            }
        }

        if (count == 1) {
            action = MotionEvent.ACTION_UP;
        } else {
            for (int j = 0; j < count; j++) {
                if (properties[j].id == fingerId) {
                    action = MotionEvent.ACTION_POINTER_UP | (j << MotionEvent.ACTION_POINTER_INDEX_SHIFT);
                }
            }
        }

        event = MotionEvent.obtain(downTime, eventTime, action, count, properties,
                pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
        
        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        fingers[fingerId].up();

        printAtLog(fingers);
    }

    public static void generateDownUpEvents(Instrumentation inst, long startTime,
            int left, int top, int right, int bottom, int duration) {
        Random random = new Random();
        long eventTime = startTime;
        Finger[] fingers = new Finger[FINGER_NUMBER];
        for (int i = 0; i < fingers.length; i++) {
            fingers[i] = new Finger(i, left, top, right, bottom);
        }

        for (int i = 0; i < fingers.length; i++) {
            down(inst, startTime, eventTime, fingers, i);
        }

//        downAll(inst, startTime, eventTime, fingers);
        int moveEventNumber = duration / EVENT_MIN_INTERVAL;
        while (moveEventNumber > 0) {
            int fingerId = random.nextInt(FINGER_NUMBER * 2);
            if (fingerId >= FINGER_NUMBER) {
                // Put down a finger
                for (int j = 0; j < fingers.length; j++) {
                    if (!fingers[j].isOnScreen) {
                        fingerId = j;
                        break;
                    }
                }
                if (fingerId >= FINGER_NUMBER) {
                    continue;
                }

                down(inst, startTime, eventTime, fingers, fingerId);
                eventTime += EVENT_MIN_INTERVAL;
            } else {
                // Lift up a finger
                if (!fingers[fingerId].isOnScreen) {
                    continue;
                }

                up(inst, startTime, eventTime, fingers, fingerId);
                eventTime += EVENT_MIN_INTERVAL;
            }

            moveEventNumber--;
        }
    }

    /**
     * generate scale gestures
     */
    public static void generateScaleGesture(Instrumentation inst, long startTime,
            int left, int top, int right, int bottom, int duration) {

        Random random = new Random();
        MotionEvent event;
        Finger[] fingers = new Finger[FINGER_NUMBER];
        for (int i = 0; i < fingers.length; i++) {
            fingers[i] = new Finger(i, left, top, right, bottom);
        }

        PointerProperties[] properties = new PointerProperties[FINGER_NUMBER];
        PointerCoords[] pointerCoords = new PointerCoords[FINGER_NUMBER];
        for (int j = 0; j < fingers.length; j++) {
            properties[j] = fingers[j].properties;
            pointerCoords[j] = fingers[j].coords;
        }

        long downTime = startTime;
        long eventTime = startTime;

        // ////////////////////////////////////////////////////////////
        // events sequence
        // 1. send ACTION_DOWN event of one start point
        // 2. send ACTION_POINTER_DOWN of start points
        // 3. send ACTION_MOVE of middle points
        // 4. repeat step 3 with updated middle points (x,y),
        // until reach the end points
        // 5. send ACTION_POINTER_UP of end points
        // 6. send ACTION_UP of one end point
        // ////////////////////////////////////////////////////////////

        // step 1
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, 1, properties,
                pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);

        inst.sendPointerSync(event);
        inst.waitForIdleSync();

        // step 2
        for (int i = 2; i <= FINGER_NUMBER; i++) {
            event = MotionEvent.obtain(downTime, eventTime,
                    MotionEvent.ACTION_POINTER_DOWN | ((i - 1) << MotionEvent.ACTION_POINTER_INDEX_SHIFT), i,
                    properties, pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);

            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }

        int moveEventNumber = duration / EVENT_MIN_INTERVAL;
        for (int i = 0; i < moveEventNumber; i++) {
            // update the move events
            eventTime += EVENT_MIN_INTERVAL;

            int action = MotionEvent.ACTION_MOVE;
            int count = FINGER_NUMBER;

            for (int j = 0; j < fingers.length; j++) {
                fingers[j].move();
            }

            event = MotionEvent.obtain(downTime, eventTime, action, count, properties,
                    pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);

            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }

        // step 5
        eventTime += EVENT_MIN_INTERVAL;
        for (int i = FINGER_NUMBER; i >= 2; i--) {
            event = MotionEvent.obtain(downTime, eventTime,
                    MotionEvent.ACTION_POINTER_UP | ((i - 1) << MotionEvent.ACTION_POINTER_INDEX_SHIFT), i,
                    properties, pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);

            inst.sendPointerSync(event);
            inst.waitForIdleSync();
        }

        // step 6
        eventTime += EVENT_MIN_INTERVAL;
        event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, 1, properties,
                pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
        inst.sendPointerSync(event);
        inst.waitForIdleSync();
    }

    public static class Point {
        public float x = 0.0f;
        public float y = 0.0f;
    }

    /**
     * Generate click gesture at specific position (x, y)
     */
    public static void generateClickGesture(Instrumentation inst, float x, float y) {
        long downTime = SystemClock.uptimeMillis();
        MotionEvent mv = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_DOWN, x, y, 0);
        inst.sendPointerSync(mv);
        inst.waitForIdleSync();

        mv = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(), MotionEvent.ACTION_UP, x, y,
                0);
        inst.sendPointerSync(mv);
        inst.waitForIdleSync();

        TestCaseUtils.sleepForTime(1 * TestCaseUtils.SECOND);
    }

    /**
     * Generate hover events on the view
     * @param view the view which receive the hover events
     */
    public static void generateHoverEvents(Instrumentation inst, View view) {
        int[] xy = new int[2];
        view.getLocationOnScreen(xy);
        int width = view.getWidth();
        int height = view.getHeight();
        final int steps = 100;
        final int xStep = width / steps;
        final int yStep = height / steps;

        long downTime = SystemClock.uptimeMillis();
        // top left
        MotionEvent mv = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_HOVER_ENTER, xy[0], xy[1], 0);
        inst.sendPointerSync(mv);
        inst.waitForIdleSync();

        // hover move
        for (int i = 0; i < steps; ++i) {
            mv = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                    MotionEvent.ACTION_HOVER_MOVE, xy[0] + xStep * i, xy[1] + yStep * i, 0);
            inst.sendPointerSync(mv);
            inst.waitForIdleSync();
        }

        // bottom right
        mv = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_HOVER_EXIT, xy[0] + width, xy[1] + height, 0);
        inst.sendPointerSync(mv);
        inst.waitForIdleSync();

        TestCaseUtils.sleepForTime(1 * TestCaseUtils.SECOND);
    }

    /**
     * Back to home, where the different context is
     */
    public static void backToHome(Instrumentation instr) {
        while (true) {
            try {
                instr.waitForIdleSync();
                instr.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
            } catch (SecurityException e) {
                break; // already back to home, break
            }
        }
        sleepForTime(1 * SECOND);
    }


}
