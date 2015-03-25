package com.mediatek.common.widget.tests;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentManager;
import android.content.Context;
import android.media.AudioManager;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.SearchView;
import android.widget.TextView;

@SuppressLint("NewApi")
@TargetApi(15)
public class VolumePanelTest extends ActivityInstrumentationTestCase2<VolumePanelActivity> {
	private VolumePanelActivity mActivity;
	
	private ActionBarUtils mActionBarUtils;

	public VolumePanelTest() {
		super("com.mediatek.common.widget.tests", VolumePanelActivity.class);
	}
	
	@Override
	protected void setUp() throws Exception {
		// TODO Auto-generated method stub
		super.setUp();
		setActivityInitialTouchMode(false);
		mActivity = (VolumePanelActivity)getActivity();
		mActionBarUtils = new ActionBarUtils(mActivity, getInstrumentation());
	}
	
	@Override
	protected void tearDown() throws Exception {
		// TODO Auto-generated method stub
		super.tearDown();
		System.out.println("tearDown");
	}
	
	public void testPreConditions() {
	    assertTrue(mActivity != null);
	}
	
	public int getVolumeIndex(boolean bUp, int cur, int max) {
		if(bUp) {
			return (cur < max) ? cur + 1 : max;
		} else {
			return (cur > 0) ? cur - 1 : 0;
		}
	}
	
	public void testVolumeChange() {
		AudioManager audioManager;
		int curVolume, maxVolume, expVolume;

		audioManager = (AudioManager)mActivity.getSystemService(Context.AUDIO_SERVICE);
		assertNotNull(audioManager);
		
		curVolume = audioManager.getStreamVolume(AudioManager.STREAM_RING);
		maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_RING);
		
		getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
		getInstrumentation().waitForIdleSync();
		
		expVolume = (curVolume < maxVolume) ? curVolume + 1 : maxVolume;
		assertTrue("volume is wrong", expVolume == audioManager.getStreamVolume(AudioManager.STREAM_RING));
		
		curVolume = expVolume;

		getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_DOWN);
		getInstrumentation().waitForIdleSync();
		
		expVolume = (curVolume > 0) ? curVolume - 1 : 0;
		assertTrue("volume is wrong", expVolume == audioManager.getStreamVolume(AudioManager.STREAM_RING));
		
		curVolume = expVolume;
	}
	
	public void testVolumeChangeMax() {
		AudioManager audioManager;
		int curVolume, maxVolume, expVolume;

		audioManager = (AudioManager)mActivity.getSystemService(Context.AUDIO_SERVICE);
		assertNotNull(audioManager);
		
		curVolume = audioManager.getStreamVolume(AudioManager.STREAM_RING);
		maxVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_RING);
		
		for(int i=0; i<10; ++i) {
			getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
			getInstrumentation().waitForIdleSync();
			
			expVolume = getVolumeIndex(true, curVolume, maxVolume);
			assertTrue("volume is wrong", expVolume == audioManager.getStreamVolume(AudioManager.STREAM_RING));
			curVolume = expVolume;
		}
		
		for(int i=0; i<10; ++i) {
			getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_DOWN);
			getInstrumentation().waitForIdleSync();
			
			expVolume = getVolumeIndex(false, curVolume, maxVolume);
			assertTrue("volume is wrong", expVolume == audioManager.getStreamVolume(AudioManager.STREAM_RING));
			curVolume = expVolume;
		}
	}
	
	public void testVibrateSetting() {
		AudioManager audioManager;
		int loopCount;

		audioManager = (AudioManager)mActivity.getSystemService(Context.AUDIO_SERVICE);
		assertNotNull(audioManager);
		
		loopCount = 50;
		while(AudioManager.RINGER_MODE_SILENT != audioManager.getRingerMode() && loopCount > 0) {
			getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_DOWN);
			getInstrumentation().waitForIdleSync();
			
			loopCount--;
		}
		assertTrue("Ringer mode is rong", AudioManager.RINGER_MODE_SILENT == audioManager.getRingerMode());
		assertTrue("volume is wrong", 0 == audioManager.getStreamVolume(AudioManager.STREAM_RING));
		
		getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
		getInstrumentation().waitForIdleSync();
		
		assertTrue("Ringer mode is rong", AudioManager.RINGER_MODE_VIBRATE == audioManager.getRingerMode());
		assertTrue("volume is wrong", 0 == audioManager.getStreamVolume(AudioManager.STREAM_RING));
		
		getInstrumentation().sendKeyDownUpSync(KeyEvent.KEYCODE_VOLUME_UP);
		getInstrumentation().waitForIdleSync();
		
		assertTrue("Ringer mode is rong", AudioManager.RINGER_MODE_NORMAL == audioManager.getRingerMode());
		assertTrue("volume is wrong", 1 == audioManager.getStreamVolume(AudioManager.STREAM_RING));
	}
}
