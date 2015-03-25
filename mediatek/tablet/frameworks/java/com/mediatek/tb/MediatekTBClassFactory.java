
package com.mediatek.tb;

import java.util.HashMap; //import java.util.List;
import java.util.Map;

import android.os.SystemClock;
import android.util.Log;

/// M: tablet feature start @{

/// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT 
import com.mediatek.common.telephony.IOnlyOwnerSimSupport;

/// M: tablet feature end @}


public final class MediatekTBClassFactory {

    private static final boolean DEBUG_PERFORMANCE = true;
    private static final boolean DEBUG_GETINSTANCE = true;
    private static final String TAG = "MediatekTBClassFactory";

    // mediatek-tablet.jar public interface map used for interface class matching.
    private static Map<Class, String> tbInterfaceMap = new HashMap<Class, String>();
    static {
        tbInterfaceMap.put(IOnlyOwnerSimSupport.class, 
                "com.mediatek.tb.telephony.OnlyOwnerSimSupportTB");
    }

    /**
     * Get the tablet specific class name.
     * 
     * 
     */
    public static String getTbIfClassName(Class<?> clazz) {
        String ifClassName = null;

        if (tbInterfaceMap.containsKey(clazz)) {
            ifClassName = tbInterfaceMap.get(clazz);
        }

        return ifClassName;
    }
}
