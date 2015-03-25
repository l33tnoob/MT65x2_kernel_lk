package com.hissage.pn;

import android.app.Application;
import android.content.Context;
import android.util.Log;

import com.hissage.hpe.SDK;
import com.hissage.jni.engineadapter;
import com.hissage.service.NmsService;
import com.hissage.util.log.NmsLog;
import com.hissage.util.preference.NmsPreferences;

public class HpnsApplication extends Application {

    public static Context mGlobalContext = null;

    public void onCreate() {
        super.onCreate();
        mGlobalContext = this;
        NmsPreferences.initPreferences(mGlobalContext);
        NmsLog.init(this);

        if (Config.PN && (Config.LOAD_PN_ATONCE || engineadapter.get().nmsUIIsActivated())) {
            hpnsReceiver.loadPn(this) ;
            NmsLog.trace("PnService", "loadPN is called by pnApplation");
        }
    }

}
