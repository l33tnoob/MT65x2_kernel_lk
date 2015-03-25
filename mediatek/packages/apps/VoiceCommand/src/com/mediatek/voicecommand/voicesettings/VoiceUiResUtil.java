package com.mediatek.voicecommand.voicesettings;

import com.mediatek.voicecommand.R;
import com.mediatek.xlog.Xlog;

public class VoiceUiResUtil {
    private final static String TAG = "VoiceUiResUtil";
    private final static int APP_PHONE = 1;
    private final static int APP_GALLERY3D = 2;
    private final static int APP_DESKCLOCK = 3;
    private final static int APP_MUSIC = 4;

    public static int getSummaryResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.alarm_command_summary_format;
        } else if (processID == APP_PHONE) {
            return R.string.incomming_command_summary_format;
        } else if (processID == APP_MUSIC) {
            return R.string.music_command_summary_format;
        } else if (processID == APP_GALLERY3D) {
            return R.string.camera_command_summary_format;
        } else {
            Xlog.d(TAG, "voice ui not support processID" + processID);
            return 0;
        }
    }

    public static int getIconId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.drawable.ic_menu_alarm;
        } else if (processID == APP_PHONE) {
            return R.drawable.ic_menu_call;
        } else if (processID == APP_GALLERY3D) {
            return R.drawable.ic_menu_camera;
        } else {
            Xlog.d(TAG, "voice ui not support processID" + processID);
            return 0;
        }
    }

    /**
     * According to the process ID, get the process preference id
     *
     * @param processID
     * @return
     */
    public static int getProcessTitleResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.alarm_app_name;
        } else if (processID == APP_PHONE) {
            return R.string.incoming_call_app_name;
        } else if (processID == APP_MUSIC) {
            return R.string.music_app_name;
        } else if (processID == APP_GALLERY3D) {
            return R.string.camera_app_name;
        } else {
            Xlog.d(TAG, "voice ui not support processID" + processID);
            return 0;
        }
    }

    /**
     * According to the process ID, get the process preference id
     *
     * @param processID
     * @return
     */
    public static int getCommandTitleResourceId(int processID) {
        if (processID == APP_DESKCLOCK) {
            return R.string.voice_ui_alarm_command_title;
        } else if (processID == APP_PHONE) {
            return R.string.voice_ui_phone_command_title;
        } else if (processID == APP_GALLERY3D) {
            return R.string.voice_ui_camera_command_title;
        } else {
            Xlog.d(TAG, "voice ui not support processID" + processID);
            return 0;
        }
    }
}
