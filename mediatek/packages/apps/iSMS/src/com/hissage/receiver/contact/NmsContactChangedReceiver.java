package com.hissage.receiver.contact;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;

import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsConfig;
import com.hissage.contact.NmsContactManager;
import com.hissage.imagecache.NmsContactAvatarCache;
import com.hissage.message.ip.NmsIpMessageConsts.NmsRefreshContactList;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateGroupAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsUpdateSystemContactAction;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.util.log.NmsLog;

public class NmsContactChangedReceiver extends BroadcastReceiver {

    private static final String TAG = "NmsContactChangedReceiver";

    @Override
    public void onReceive(Context arg0, Intent arg1) {
        if (arg1 == null) {
            NmsLog.error(TAG, "got invalid param, intent is null");
            return;
        }
        
        String action = arg1.getAction();
        NmsLog.trace(TAG, "get broadcast action is: " + action);
        if (TextUtils.isEmpty(action)) {
            return;
        }
        
        if (!NmsConfig.mIsDBInitDone) {
            NmsLog.error(TAG, "db not init done, just ignore this event: " + action);
            return ;
        }

        if (action.equals(NmsUpdateGroupAction.NMS_UPDATE_GROUP)) {
            int groupId = arg1.getIntExtra(NmsUpdateGroupAction.NMS_GROUP_ID, -1);
            if (groupId <= 0) {
                NmsLog.error(TAG, "groupId <= 0");
                return;
            }
            NmsContactAvatarCache.getInstance().removeCache((short) groupId);
        } else if (action.equals(NmsRefreshContactList.NMS_REFRESH_CONTACTS_LIST)) {
            int engContactId = arg1.getIntExtra(NmsRefreshContactList.NMS_CONTACT_ID, -1);
            if (engContactId <= 0) {
                NmsLog.error(TAG, "engContactId <= 0");
                return;
            }
            NmsContactManager.getInstance(null).setRefresh(true);
            NmsContactAvatarCache.getInstance().removeCache((short) engContactId);
            if (NmsContactApi.getInstance(null).isMyselfEngineContactId((short) engContactId)) {
                short[] ids = NmsIpMessageApiNative.nmsGetGroupIdList();
                if (ids != null && ids.length > 0) {
                    for (short id : ids) {
                        NmsContactAvatarCache.getInstance().removeCache(id);
                    }
                }
                // NmsImageCache.getInstance().clearCaches();
            }
        } else if (action.equals(NmsUpdateSystemContactAction.NMS_UPDATE_CONTACT)) {
            NmsContactManager.getInstance(null).setRefresh(true);
            NmsContactAvatarCache.getInstance().clearCaches();
        } else {
            NmsLog.warn(TAG, "receive unknown intent.");
        }

    }

}
