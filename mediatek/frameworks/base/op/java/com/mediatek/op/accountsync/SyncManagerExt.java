package com.mediatek.op.accountsync;

import com.mediatek.xlog.Xlog;
import com.mediatek.common.accountsync.ISyncManagerExt;

public class SyncManagerExt implements ISyncManagerExt {
    private static final String TAG = "SyncManagerExt";
    private static final boolean LOG = true;
    private static final boolean isAutoSync = true;

    /**
     * Get the OP default SyncAutomatically setting.
     * @return isAutoSync default auto sync setting.
     */
    public boolean getSyncAutomatically() {
        return isAutoSync;
    }
}
