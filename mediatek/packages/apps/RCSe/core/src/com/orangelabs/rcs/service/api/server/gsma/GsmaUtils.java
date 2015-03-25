package com.orangelabs.rcs.service.api.server.gsma;

import com.orangelabs.rcs.service.api.client.gsma.GsmaClientConnector;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Build;

/**
 * GSMA utility functions
 * 
 * @author jexa7410
 */
public class GsmaUtils {
	
    /**
     * Set RCS client activation state
     * 
     * @param ctx Context
     * @param state Activation state
     */
    public static void setClientActivationState(Context ctx, boolean state) {
        SharedPreferences preferencesLegacy = null;
        SharedPreferences preferences = null;
        String packageName = ctx.getPackageName();
        String prefName = new String(packageName + "." + GsmaClientConnector.GSMA_PREFS_NAME);

        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.GINGERBREAD_MR1) {
            preferencesLegacy = ctx.getSharedPreferences(GsmaClientConnector.GSMA_PREFS_NAME, Context.MODE_WORLD_READABLE);
            preferences = ctx.getSharedPreferences(prefName, Context.MODE_WORLD_READABLE);
        } else {
            preferencesLegacy = ctx.getSharedPreferences(GsmaClientConnector.GSMA_PREFS_NAME, Context.MODE_WORLD_READABLE + Context.MODE_MULTI_PROCESS);
            preferences = ctx.getSharedPreferences(prefName, Context.MODE_WORLD_READABLE + Context.MODE_MULTI_PROCESS);
        }
        Editor editor = preferencesLegacy.edit();
        editor.putBoolean(GsmaClientConnector.GSMA_CLIENT_ENABLED, state);
        editor.commit();

        editor = preferences.edit();
		editor.putBoolean(GsmaClientConnector.GSMA_CLIENT_ENABLED, state);
		editor.commit();
    }
}