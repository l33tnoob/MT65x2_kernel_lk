package com.mediatek.cts.window;

import com.mediatek.cts.window.stub.R;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.MenuItem;
import android.view.Surface;
import android.view.View;
import android.widget.PopupMenu;

public class LandscapeStubActivity extends Activity {
    boolean mIsLandscape;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.windowstub_layout);
        DisplayManager displayManager = (DisplayManager)getSystemService(Context.DISPLAY_SERVICE);
        Display display  = displayManager.getDisplay(Display.DEFAULT_DISPLAY);
        mIsLandscape = (display.getRotation() == Surface.ROTATION_90)?true:false;
    }

    @Override
    public void onResume() {
        super.onResume();
    }
    
    public boolean isLandscape() {
        return mIsLandscape;
    }
}
