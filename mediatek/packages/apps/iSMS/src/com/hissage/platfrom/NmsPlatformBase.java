package com.hissage.platfrom;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import com.hissage.config.NmsConfig;
import com.hissage.db.NmsDBUtils;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

import android.content.Context;
import android.text.TextUtils;

abstract class NmsPlatformBase {
    private static final String TAG = "NmsPlatformBase";
    public static final int NMS_STANDEALONE_MODE = 0;
    public static final int NMS_INTEGRATION_MODE = 1;
    protected int mPlatfromMode = NMS_STANDEALONE_MODE;
    protected Context mContext = null;
    
    protected NmsPlatformBase(Context context){
        mContext = context;
    }
    
    protected String getModeString(){
        return NMS_STANDEALONE_MODE == mPlatfromMode?"standalone":"integration";
    }
    
    public boolean isIntegrationMode(){
        return NMS_INTEGRATION_MODE == mPlatfromMode;
    }
    
    protected long getGoogleDefaultSimId(Context context) {
        String imsi = NmsConfig.getSim1IMSI(context);
        if (TextUtils.isEmpty(imsi)) {
            NmsLog.error(TAG, "can not get current imsi, so sim id return SINGLE_CARD_SIM_ID");
            return NmsConsts.SINGLE_CARD_SIM_ID;
        }
        return NmsDBUtils.getDataBaseInstance(context).nmsGetSimId(imsi);
    }
    
    protected void printMethods(Class<?> className) {
        Method[] methods = className.getMethods();
        for (int i = 0; i < methods.length; i++) {
            int mod = methods[i].getModifiers();
            System.out.print(Modifier.toString(mod) + " ");
            System.out.print(methods[i].getReturnType().getName());
            System.out.print(" " + methods[i].getName() + "(");
            Class[] parameterTypes = methods[i].getParameterTypes();
            for (int j = 0; j < parameterTypes.length; j++) {
                System.out.print(parameterTypes[j].getName());
                if (parameterTypes.length > j + 1) {
                    System.out.print(", ");
                }
            }
            System.out.println(")");
        }
    }
}
