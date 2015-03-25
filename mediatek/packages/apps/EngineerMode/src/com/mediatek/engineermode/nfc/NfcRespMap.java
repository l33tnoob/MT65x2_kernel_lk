package com.mediatek.engineermode.nfc;

import java.util.HashMap;

public class NfcRespMap {

    public static final String KEY_SETTINGS = "nfc.settings";
    public static final String KEY_SS_REGISTER_NOTIF =
        "nfc.software_stack.reg_notif";
    public static final String KEY_SS_SCAN_COMPLETE =
        "nfc.software_stack.scan_complete";
    public static final String KEY_SS_TAG_DECT =
        "nfc.software_stack.normaltag_dect";
    public static final String KEY_SS_P2P_TARGET_DECT =
        "nfc.software_stack.p2p_dect";

    private final HashMap<String, Object> mContainer =
        new HashMap<String, Object>();
    private static volatile NfcRespMap sRespMap = new NfcRespMap();

    /**
     * Get the class's instance, single mode
     * 
     * @return NfcRespMap's single instance
     */
    public static NfcRespMap getInst() {
        if (sRespMap == null) {
            synchronized (NfcRespMap.class) {
                sRespMap = new NfcRespMap();
            }
        }
        return sRespMap;
    }

    /**
     * clear the hash map
     */
    public void clear() {
        mContainer.clear();
    }

    /**
     * Put a key value to the hash map
     * 
     * @param key
     *            : key
     * @param resp
     *            : value
     */
    public void put(String key, Object resp) {
        mContainer.put(key, resp);
    }

    /**
     * take value from the hash map
     * 
     * @param key
     *            : value's key
     * @return value
     */
    public Object take(String key) {
        return mContainer.get(key);
    }

}
