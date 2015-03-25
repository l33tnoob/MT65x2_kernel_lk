package com.mediatek.common.widget.tests;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.ProgressBar;

import com.mediatek.common.widget.tests.ProgressBarActivity;

public class ProgressBarActivityTest extends ActivityInstrumentationTestCase2<ProgressBarActivity> {

    Activity mActivity;
    ProgressBar mProgressBar;

    public ProgressBarActivityTest() {
        super("android.widget.ProgressBarActivity", ProgressBarActivity.class);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected void setUp() throws Exception {
        // TODO Auto-generated method stub
        super.setUp();
        setActivityInitialTouchMode(false);
        mActivity = getActivity();
        mProgressBar = (ProgressBar)mActivity.findViewById(R.id.testProgressBar);
    }

    @Override
    protected void tearDown() throws Exception {
        // TODO Auto-generated method stub
        super.tearDown();
        System.out.println("tearDown");
    }

    public void testRefreshProgress() {
        mActivity.runOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                mProgressBar.requestFocus();
            }
        });

        mProgressBar.setSecondaryProgress(20);
        mProgressBar.setProgress(10);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(30);
        mProgressBar.setProgress(20);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(40);
        mProgressBar.setProgress(30);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(50);
        mProgressBar.setProgress(40);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(60);
        mProgressBar.setProgress(50);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(70);
        mProgressBar.setProgress(60);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(80);
        mProgressBar.setProgress(70);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(90);
        mProgressBar.setProgress(80);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setSecondaryProgress(100);
        mProgressBar.setProgress(90);

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        mProgressBar.setProgress(100);
    }

}
