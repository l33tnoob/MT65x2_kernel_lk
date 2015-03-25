
package com.mediatek.smsdbpermission;

import com.mediatek.common.smsdbpermission.ISmsDbVisitor;


public class SmsDbVisitor implements ISmsDbVisitor {
    private static final String LOG_TAG = "[SmsDbVisitor]";
    private static final boolean DBG = true;

    private static final String DATATRANSFER_PACKAGE_NAME = "com.mediatek.datatransfer";
    private static final String APST_PACKAGE_NAME = "com.mediatek.apst.target";
    private static final String BACKUPRESTOR_PACKAGE_NAME = "com.mediatek.backuprestore";
    private static final String PHONEPRIVACY_PACKAGE_NAME = "com.mediatek.ppl";

    public String [] getPackageNames() {
        return new String [] { DATATRANSFER_PACKAGE_NAME, 
                               APST_PACKAGE_NAME,
                               PHONEPRIVACY_PACKAGE_NAME,
                               BACKUPRESTOR_PACKAGE_NAME };
    }

}

