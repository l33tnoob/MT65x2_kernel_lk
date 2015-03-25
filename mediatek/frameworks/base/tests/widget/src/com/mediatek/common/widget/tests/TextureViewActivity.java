package com.mediatek.common.widget.tests;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.view.Gravity;
import android.view.TextureView;
import android.widget.FrameLayout;

@SuppressLint("NewApi")
@TargetApi(14)
public class TextureViewActivity extends Activity implements
		TextureView.SurfaceTextureListener {
	private Camera mCamera;
	private TextureView mTextureView;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mTextureView = new TextureView(this);
		mTextureView.setSurfaceTextureListener(this);

		setContentView(mTextureView);
	}

	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
			int height) {
		// TODO Auto-generated method stub
		mCamera = Camera.open();
		
		Size previewSize = mCamera.getParameters().getPreviewSize();
        mTextureView.setLayoutParams(new FrameLayout.LayoutParams (previewSize.width, previewSize.height, Gravity.CENTER)); 

		try {
			mCamera.setPreviewTexture(surface);
			mCamera.startPreview();
		} catch (IOException ioe) {
			// Something bad happened
		}

		// this is the sort of thing TextureView enables
		mTextureView.setRotation(45.0f);
		mTextureView.setAlpha(0.5f);
	}

	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width,
			int height) {
		// TODO Auto-generated method stub

	}

	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		// TODO Auto-generated method stub
		mCamera.stopPreview();
		mCamera.release();
		return true;
	}

	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		// TODO Auto-generated method stub

	}
}