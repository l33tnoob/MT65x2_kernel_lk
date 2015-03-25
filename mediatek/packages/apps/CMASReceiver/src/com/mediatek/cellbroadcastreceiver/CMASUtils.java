package com.mediatek.cellbroadcastreceiver;

import android.util.Log;
import java.util.Arrays;
import java.util.List;

public class CMASUtils {

    private static final String TAG = "[CMAS]CMASUtils";

    public static final int PRIORITY_PRESIDENTIAL = 12;
    public static final int PRIORITY_NONE_PRESIDENTIAL = 10;
    public static final int PRIORITY_EXTREAM = 8;
    public static final int PRIORITY_SEVERE = 4;
    public static final int PRIORITY_AMBER = 0;

    private static final Integer[] presidential_msgId = {4370};
    /*private static final Integer[] extream_msgId = {4371, 4372};
    private static final Integer[] severe_msgId = {4373, 4374, 4375, 4376, 4377, 4378};
    private static final Integer[] amber_msgId = {4379};
    private static final Integer[] none_presidential_msgId = {4371, 4372, 4373, 4374, 4375, 4376, 4377, 4378, 4379}; */
    private static final List<Integer> PRESIDENTIAL_MSGID = Arrays.asList(presidential_msgId);


    public static int getMsgPriority(int msgId) {
        Log.d(TAG, "getMsgPriority:: msgId = " + msgId);

        if (PRESIDENTIAL_MSGID.contains(msgId)) {
            return PRIORITY_PRESIDENTIAL;
        } else {
            return PRIORITY_NONE_PRESIDENTIAL;
        }
    }

    public static String convertMsgId2Str(int msgId) {
        if (msgId < 4370 || msgId > 4399) {
            Log.d(TAG, "convertMsgId2Str msgId = " + msgId + ", error");
            return null;
        }

        return "MsgId" + String.valueOf(msgId - 4370 + 1);
    }

}
