
package com.mediatek.videofavorites.tests;

import android.app.Activity;
import android.os.Bundle;
import android.widget.FrameLayout;
import android.view.WindowManager;

public class DismissKeyguardActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // any dummy view.
        FrameLayout fl = new FrameLayout(this);
        setContentView(fl);

        // dismiss keygard and turn on backlight.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD
                             | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
                             | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                             | WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);

    }
}
