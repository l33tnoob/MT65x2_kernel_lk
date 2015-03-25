package com.mediatek.engineermode.nfc;

public class NfcUtils {

    private static final int NUM_4 = 4;
    /**
     * Used to debug
     * 
     * @param array
     *            print object
     * @return print string
     */
    public static String printArray(Object array) {
        String res = "";
        if (array instanceof byte[]) {
            for (int i = 0; i < ((byte[]) array).length; i++) {
                if (i != 0 && i % NUM_4 == 0) {
                    res += "\n";
                }
                res += String.format("0x%02X ", ((byte[]) array)[i]);
            }
        } else if (array instanceof short[]) {
            for (int i = 0; i < ((short[]) array).length; i++) {
                if (i != 0 && i % NUM_4 == 0) {
                    res += "\n";
                }
                res += String.format("0x%04X ", ((short[]) array)[i]);
            }
        } else {
            res = "UNSUPPORTED TYPE.";
        }
        return res;
    }
}
