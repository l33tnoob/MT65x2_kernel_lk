package com.mediatek.hotknotbeam;

import android.app.Application;
import android.os.Process;
import android.content.Intent;
import android.content.Context;
import android.util.Log;

public class HotKnotBeamApplication extends Application {

    private static final String TAG = "HotKnotBeamApplication";

    public static Context sContext;

    public HotKnotBeamApplication() {

    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "start Hot Knot Beam service");
        Intent startIntent = new Intent(this, HotKnotBeamService.class);
        startService(startIntent);
        sContext = this;
    }

}



