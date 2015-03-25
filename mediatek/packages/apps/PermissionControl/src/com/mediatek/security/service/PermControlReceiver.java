package com.mediatek.security.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.UserHandle;

import com.mediatek.common.mom.IMobileManager;
import com.mediatek.xlog.Xlog;

public class PermControlReceiver extends BroadcastReceiver {
    private static final String TAG = "PermControlBootReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent != null) {
            String action = intent.getAction();
            Xlog.d(TAG, "action = " + action);
            // Send from Moms when user switched and then launch service
            if (IMobileManager.ACTION_USER_CHANGE.equals(action)) {
                int status = intent.getIntExtra(IMobileManager.ACTION_EXTRA_STATUS, IMobileManager.USER_SWITCHED);
                Xlog.d(TAG,"ACTION_USER_CHANGE status = " + status);
                if (status == IMobileManager.USER_SWITCHED) {
                    int userId = intent.getIntExtra(IMobileManager.ACTION_EXTRA_USER, -1);
                    int currentUserId = UserHandle.myUserId();
                    Xlog.d(TAG,"ACTION_USER_CHANGE userId = " + userId + " and currentUserId = " + currentUserId);
                    if (userId == currentUserId) {
                        startControlService(context);
                    }
                }
            } else if (IMobileManager.ACTION_PERM_MGR_CHANGE.equals(action)) {
                handleAttachChgIntent(context,intent);
            }
        }
    }
    
    private void startControlService(Context context) {
        boolean isEnable = PermControlUtils.isInHouseEnabled(context);
        if (isEnable) {
            boolean isOn = PermControlUtils.isPermControlOn(context); 
            Xlog.d(TAG,"startControlService isEnable = " + isEnable + " isOn = " + isOn);
            if (isOn) {
                Intent intent = new Intent();
                intent.setAction(PermControlUtils.START_SERVICE_ACTION);
                intent.setClass(context, PermControlService.class);
                context.startService(intent);
            } else {
                PermControlUtils.showHintNotify(context);
            }
        }
    }
    
    private void handleAttachChgIntent(Context context, Intent intent) {
        int attachState = intent.getIntExtra(IMobileManager.ACTION_EXTRA_STATUS, IMobileManager.MGR_ATTACHED);
        int uid = intent.getIntExtra(IMobileManager.ACTION_EXTRA_UID, -1);
        int myUid = Binder.getCallingUid();
        Xlog.d(TAG,"attachState = " + attachState + " uid = " + uid + " myUid = " + myUid);
        if (attachState == IMobileManager.MGR_DETACHED) {
            if (uid == myUid) {
                PermControlUtils.setInHouseEnabled(context, false);
                Intent newIntent = new Intent(context,PermControlService.class);
                context.stopService(newIntent);
            } else if (uid != myUid) {
                PermControlUtils.setInHouseEnabled(context, true);
                startControlService(context);
            }
        } else if (attachState == IMobileManager.MGR_ATTACHED) {
            //if other app attached then disable in house app
            // this case means in house not attached before
            if (uid != myUid) {
                Xlog.d(TAG,"3rd app attached disable in house");
                PermControlUtils.setInHouseEnabled(context, false);
                PermControlUtils.cancelNotification(context);
            }
        }
    }
}
