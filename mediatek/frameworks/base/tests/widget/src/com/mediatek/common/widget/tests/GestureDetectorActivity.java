package com.mediatek.common.widget.tests;

import android.os.Bundle;  
import android.util.Log;
import android.view.GestureDetector;  
import android.view.MotionEvent;  
import android.view.View;
import android.view.ViewGroup;  
import android.app.Activity;
import android.view.Menu;
import android.view.GestureDetector.OnGestureListener;
import android.view.animation.AnimationUtils;

public class GestureDetectorActivity extends Activity {

	public ViewGroup container1, container2;

	private GestureDetector gestureDetector;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.gesture_detector_main);

		// 监听屏幕动作事件
		SampleGuest gestureListener = new SampleGuest(this);
		gestureDetector = new GestureDetector(gestureListener);

		container1 = (ViewGroup) findViewById(R.id.container1);
		container2 = (ViewGroup) findViewById(R.id.container2);
	}

	// called automatically, any screen action will Triggered it
	public boolean onTouchEvent(MotionEvent event) {
		if (gestureDetector.onTouchEvent(event))
			return true;
		else
			return false;
	}
	
	public class SampleGuest implements OnGestureListener {
		GestureDetectorActivity se;

		private static final int SWIPE_MAX_OFF_PATH = 100;

		private static final int SWIPE_MIN_DISTANCE = 100;

		private static final int SWIPE_THRESHOLD_VELOCITY = 100;

		public SampleGuest(GestureDetectorActivity se) {
			this.se = se;
		}

		public boolean onDown(MotionEvent e) {
			Log.d("TAG", "[onDown]");
			return true;
		}

		public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
				float velocityY) {
			if (Math.abs(e1.getY() - e2.getY()) > SWIPE_MAX_OFF_PATH)
				return false;

			if ((e1.getX() - e2.getX()) > SWIPE_MIN_DISTANCE
					&& Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
				se.container1.setAnimation(AnimationUtils.loadAnimation(se,
						R.anim.push_left_out));
				se.container2.setVisibility(View.VISIBLE);
				se.container2.setAnimation(AnimationUtils.loadAnimation(se,
						R.anim.push_right_in));
				se.container1.setVisibility(View.GONE);

			} else if ((e2.getX() - e1.getX()) > SWIPE_MIN_DISTANCE
					&& Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
				se.container1.setVisibility(View.VISIBLE);
				se.container1.setAnimation(AnimationUtils.loadAnimation(se,
						R.anim.push_left_in));
				se.container2.setAnimation(AnimationUtils.loadAnimation(se,
						R.anim.push_right_out));
				se.container2.setVisibility(View.GONE);
			}
			return true;
		}

		public void onLongPress(MotionEvent e) {
			Log.d("TAG", "[onLongPress]");
		}

		public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
				float distanceY) {
			Log.d("TAG", "[onScroll]");
			return true;
		}

		public void onShowPress(MotionEvent e) {
			Log.d("TAG", "[onShowPress]");

		}

		public boolean onSingleTapUp(MotionEvent e) {
			Log.d("TAG", "[onSingleTapUp]");
			return true;
		}
	}
}
