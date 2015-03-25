package com.mediatek.common.widget.tests;

import java.lang.reflect.Constructor;
import java.util.regex.Pattern;

import junit.framework.Assert;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.os.SystemClock;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.Window;
import android.widget.TextView;

public class ActionBarUtils {
	private final int TIMEOUT = 10000;
	private Activity mActivity;
	private Instrumentation mInstrumentation;
	
	public ActionBarUtils(Activity activity, Instrumentation instrumentation) {
		mActivity = activity;
		mInstrumentation = instrumentation;
	}
	
	public void sleepMini() {
		sleep(300);
	}
	
	public void sleep(int time) {
		try {
			Thread.sleep(time);
		} catch (InterruptedException ignored) {}
	}
	
	public void clickOnActionBarHomeButton() {
		MenuItem homeMenuItem = null;

		try {
			Class<?> cls = Class.forName("com.android.internal.view.menu.ActionMenuItem");
			Class<?> partypes[] = new Class[6];
			partypes[0] = Context.class;
			partypes[1] = Integer.TYPE;
			partypes[2] = Integer.TYPE;
			partypes[3] = Integer.TYPE;
			partypes[4] = Integer.TYPE;
			partypes[5] = CharSequence.class;
			Constructor<?> ct = cls.getConstructor(partypes);
			Object argList[] = new Object[6];
			argList[0] = mActivity;
			argList[1] = 0;
			argList[2] = android.R.id.home;
			argList[3] = 0;
			argList[4] = 0;
			argList[5] = mActivity.getTitle();
			homeMenuItem = (MenuItem) ct.newInstance(argList);
		} catch (Exception ex) {
			Log.d("ActionBarTest", "Can not find methods to invoke Home button.");
		}

		if (homeMenuItem != null) {
			mActivity.getWindow().getCallback().onMenuItemSelected(Window.FEATURE_OPTIONS_PANEL, homeMenuItem);
		}
	}
	
	public void clickOnActionBarItem(int resourceId){
		mInstrumentation.invokeMenuActionSync(mActivity, resourceId, 0);
	}
	
	public void doSearch(String query) {
		mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_SEARCH);
        
        KeyCharacterMap keymap = KeyCharacterMap.load(KeyCharacterMap.VIRTUAL_KEYBOARD);
        for (KeyEvent key : keymap.getEvents(query.toCharArray())) {
        	mInstrumentation.sendKeySync(key);
        }
        mInstrumentation.sendCharacterSync(KeyEvent.KEYCODE_ENTER);
        
        /*
        	Instrumentation instrumentation = getInstrumentation(); 
	   		instrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_SEARCH); 
	   		instrumentation.sendCharacterSync(KeyEvent.KEYCODE_F); 
	   		instrumentation.sendCharacterSync(KeyEvent.KEYCODE_O); 
	   		instrumentation.sendCharacterSync(KeyEvent.KEYCODE_O);
         * */
    }
	
	public void clickOnScreen(float x, float y) {
		long downTime = SystemClock.uptimeMillis();
		long eventTime = SystemClock.uptimeMillis();
		MotionEvent event = MotionEvent.obtain(downTime, eventTime,
				MotionEvent.ACTION_DOWN, x, y, 0);
		MotionEvent event2 = MotionEvent.obtain(downTime, eventTime,
				MotionEvent.ACTION_UP, x, y, 0);
		try{
			mInstrumentation.sendPointerSync(event);
			mInstrumentation.sendPointerSync(event2);
			sleepMini();
		}catch(SecurityException e){
			Assert.assertTrue("Click can not be completed!", false);
		}
	}

	public void clickLongOnScreen(float x, float y, int time) {
		long downTime = SystemClock.uptimeMillis();
		long eventTime = SystemClock.uptimeMillis();
		MotionEvent event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_DOWN, x, y, 0);
		try{
			mInstrumentation.sendPointerSync(event);
		}catch(SecurityException e){
			Assert.assertTrue("Click can not be completed! Something is in the way e.g. the keyboard.", false);
		}
		eventTime = SystemClock.uptimeMillis();
		event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_MOVE, 
				x + ViewConfiguration.getTouchSlop() / 2,
				y + ViewConfiguration.getTouchSlop() / 2, 0);
		mInstrumentation.sendPointerSync(event);
		if(time > 0)
			sleep(time);
		else
			sleep((int)(ViewConfiguration.getLongPressTimeout() * 2.5f));

		eventTime = SystemClock.uptimeMillis();
		event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP, x, y, 0);
		mInstrumentation.sendPointerSync(event);
		sleep(500);
	}
	
	public void drag(float fromX, float toX, float fromY, float toY,
			int stepCount) {
		long downTime = SystemClock.uptimeMillis();
		long eventTime = SystemClock.uptimeMillis();
		float y = fromY;
		float x = fromX;
		float yStep = (toY - fromY) / stepCount;
		float xStep = (toX - fromX) / stepCount;
		MotionEvent event = MotionEvent.obtain(downTime, eventTime,MotionEvent.ACTION_DOWN, fromX, fromY, 0);
		try {
			mInstrumentation.sendPointerSync(event);
		} catch (SecurityException ignored) {}
		for (int i = 0; i < stepCount; ++i) {
			y += yStep;
			x += xStep;
			eventTime = SystemClock.uptimeMillis();
			event = MotionEvent.obtain(downTime, eventTime,MotionEvent.ACTION_MOVE, x, y, 0);
			try {
				mInstrumentation.sendPointerSync(event);
			} catch (SecurityException ignored) {}
		}
		eventTime = SystemClock.uptimeMillis();
		event = MotionEvent.obtain(downTime, eventTime, MotionEvent.ACTION_UP,toX, toY, 0);
		try {
			mInstrumentation.sendPointerSync(event);
		} catch (SecurityException ignored) {}
	}
}
