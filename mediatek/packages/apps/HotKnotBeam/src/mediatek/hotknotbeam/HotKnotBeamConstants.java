package com.mediatek.hotknotbeam;

import java.io.File;

public class HotKnotBeamConstants {

    public enum State {
        CONNECTING, RUNNING, COMPLETE
    }

    //Transfer File attribute
    static protected final String   QUERY_NUM = "num";          //Specify
    static protected final String   QUERY_FORMAT = "format";    //Specify how to save the file. No change file name or rename auto
    static protected final String   QUERY_FOLDER = "folder";    //Specify where to seave
    static protected final String   QUERY_SHOW   = "show";      //Specify whether to display notificaiton
    static protected final String   QUERY_ZIP   = "zip";      //Specify whether to display notificaiton

    static protected final String   QUERY_VALUE_YES   = "yes";      //
    static protected final String   QUERY_VALUE_NO   = "no";      //

    static protected final int      MAX_IDLE_COUNTER = 10;

    //File transfer
    static protected final long     MAX_FILE_UPLOAD_SIZE = 2 * 1024 * 1024 * 1024;  //Only support 2G
    static protected final String   MAX_HOTKNOT_BEAM_FOLDER = "HotKnot";
    static protected final String   MAX_HOTKNOT_BEAM_TEMP_ZIP = "tmp.zip";

    //Socket layer
    static protected final int      MAX_TIMEOUT_VALUE = 30 * 1000; //The default timer value is 30 seconds
    static protected final int      MAX_RETRY_COUNT = 3; //The default timer value is 30 seconds
    static protected final int      RETRY_SLEEP_TIMER = 2 * 1000; //The default timer value is 30 seconds
    static protected final int      SERVICE_PORT = 19273;

    //Notficiation progress
    static protected final int      FILE_PROGRESS_POLL = 1 * 1000;
    static protected final int      MAX_KB_SIZE = 1024;
    static protected final int      MAX_MB_SIZE = 1024 * 1024;
    static protected final String   MAX_MB_FORMAT = "%1dMB / %2dMB";
    static protected final String   MAX_KB_FORMAT = "%1dKB / %2dKB";
    static protected final String   MAX_FORMAT = "%1dB / %2dB";

    static protected final String   BEAM_FINISH_COMMAND = "__Finish__";

    //Test configuration
    static protected final String   TEST_FILE_URI = "file://mnt/sdcard/testimg.jpg";

    public static final String STORAGE_AUTHORITY = "com.mediatek.hotknotbeam.documents";
    public static final String STORAGE_ROOT_ID = "HotKnot";

}
