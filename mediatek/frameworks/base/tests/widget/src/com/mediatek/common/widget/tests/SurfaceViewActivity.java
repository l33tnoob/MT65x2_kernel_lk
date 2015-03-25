package com.mediatek.common.widget.tests;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

public class SurfaceViewActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(new MySurfaceView(this));
	}

	private class MySurfaceView extends SurfaceView implements
			SurfaceHolder.Callback, Runnable {
		private SurfaceHolder mSurfaceHolder;
		private Paint mPaint;
		private Thread mThread;
		private int mWidth;
		private int mHeight;
		private Canvas mCanvas;

		public MySurfaceView(Context context) {
			super(context);
			mSurfaceHolder = this.getHolder();
			mSurfaceHolder.addCallback(this);
			mThread = new Thread(this);
			mPaint = new Paint();
			mPaint.setAntiAlias(true);
			mPaint.setColor(Color.RED);

			this.setKeepScreenOn(true);
		}

		public void surfaceChanged(SurfaceHolder holder, int format, int width,
				int height) {
			// TODO Auto-generated method stub

		}

		public void surfaceCreated(SurfaceHolder holder) {
			mWidth = this.getWidth();
			mHeight = this.getHeight();
			mThread.start();
		}

		public void surfaceDestroyed(SurfaceHolder holder) {
			// TODO Auto-generated method stub

		}

		public void run() {
			while (true) {
				draw();
				try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}

		private void draw() {
			try {
				mCanvas = mSurfaceHolder.lockCanvas();
				int rand = (int)(Math.random()*3);
				switch(rand)
				{
				case 0:
					mCanvas.drawColor(Color.WHITE);
					break;
				case 1:
					mCanvas.drawColor(Color.BLACK);
					break;
				case 2:
					mCanvas.drawColor(Color.RED);
					break;
				}
				mCanvas.drawText("SurfaceViewTest", mWidth / 2, mHeight / 2, mPaint);
			} catch (Exception e) {
				// TODO: handle exception
			} finally {
				if (mCanvas != null) {
					mSurfaceHolder.unlockCanvasAndPost(mCanvas);
				}
			}
		}
	}
}