package com.hissage.api;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Handler;

import com.hissage.R;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContactManager;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.ui.activity.NmsAudioActivity;
import com.hissage.ui.activity.NmsContactSelectionActivity;
import com.hissage.ui.activity.NmsReadedBurnDetailsActivity;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsStartActivityApi {

    private static final String TAG = "NmsStartActivityApi";

    public static void nmsStartChatSettingsActivity(Context context, long threadId) {
        if (threadId <= 0 || null == context) {
            NmsLog.error(TAG, "nmsShowChatSettingsActivity param error: " + context + "threadId: "
                    + threadId);
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }

        if (1 == NmsSMSMMSManager.getInstance(context).isExtentionFieldExsit()) {
            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_MTK_CHAT_SETTINGS);
            intent.setClassName("com.android.mms", "com.android.mms.ui.ChatPreferenceActivity");
            intent.putExtra("chatThreadId", threadId);
            context.startActivity(intent);
        } else {
            NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaThreadId(threadId);
            if (null == contact) {
                NmsLog.error(TAG, "nmsStartChatSettingsActivity get NmsContact error: " + threadId);
                return;
            }

            if (null == context || contact.getId() <= 0) {
                NmsLog.error(TAG, "nmsShowChatSettingsActivity param error: " + context
                        + "contactId: " + contact.getId());
                return;
            }

            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_CHAT_SETTINGS);
            intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                    NmsIpMessageConsts.CLASS_NAME_CHAT_SETTINGS);
            intent.putExtra(NmsChatSettings.CHAT_SETTINGS_KEY, contact.getId());
            context.startActivity(intent);
        }
    }

    public static void nmsStartIpMessageSystemSettingsActivity(Context context) {
        if (null == context) {
            NmsLog.error(TAG, "show ipmessage system settings param error: " + context);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_SYSTEM_SETTINGS);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_SYSTEM_SETTINGS);
        context.startActivity(intent);
    }
	//M: Activation Statistics
    public static void nmsStartActivitionActivity(Context context, long sim_id, int actviateType) {
        if (null == context
                || (sim_id <= NmsConsts.INVALID_SIM_ID && sim_id != NmsConsts.ALL_SIM_CARD)) {
            NmsLog.error(TAG, "show activition activity param error: " + sim_id);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_ACTIVATION);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_ACTIVATION);
        intent.putExtra(NmsConsts.SIM_ID, sim_id);
		//M: Activation Statistics
        intent.putExtra(NmsConsts.ACTIVATE_TYPE, actviateType) ;
        context.startActivity(intent);
    }

    public static void nmsStartTermActivity(Context context, long sim_id) {
        if (null == context
                || (sim_id <= NmsConsts.INVALID_SIM_ID && sim_id != NmsConsts.ALL_SIM_CARD)) {
            NmsLog.error(TAG, "show Term activity param error: " + sim_id);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_TERM);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS, NmsIpMessageConsts.CLASS_NAME_TERM);
        intent.putExtra(NmsConsts.SIM_ID, sim_id);
        context.startActivity(intent);
    }

    public static void nmsStartProfileActivity(Context context) {
        if (null == context) {
            NmsLog.error(TAG, "show profile activity param error: " + context);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_PROFILE);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_PROFILE);
        context.startActivity(intent);
    }

    public static void nmsStartLocationActivityForResult(Context context, int requestCode) {
        if (null == context) {
            NmsLog.error(TAG, "Start Location Activity param error, context is null");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        try {
            Class.forName("com.google.android.maps.MapActivity");

            Class.forName("com.hissage.ui.activity.NmsLocationActivity");

            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_LOCATION);
            intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                    NmsIpMessageConsts.CLASS_NAME_LOCATION);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            ((Activity) context).startActivityForResult(intent, requestCode);
        } catch (Exception e) {
            NmsLog.trace(TAG, "this phone is not google maps.jar");

            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_LOCATION_NO_MAP);
            intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                    NmsIpMessageConsts.CLASS_NAME_LOCATION_NO_MAP);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            ((Activity) context).startActivityForResult(intent, requestCode);
        }
    }

    public static void nmsStartAllMediaActivity(Context context, long threadId) {
        if (null == context) {
            NmsLog.error(TAG, "Start AllMedia Activity param error, context is null");
            return;
        }
        if (threadId < 0) {
            NmsLog.error(TAG, "Start AllMedia Activity param error, threadId < 0");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_ALL_MEDIA);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_ALL_MEDIA);
        intent.putExtra("thread_id", threadId);
        context.startActivity(intent);
    }

    public static void nmsStartAllLocationActivity(Context context, long threadId) {
        if (null == context) {
            NmsLog.error(TAG, "Start AllLocation Activity param error, context is null");
            return;
        }
        if (threadId < 0) {
            NmsLog.error(TAG, "Start AllLocation Activity param error, threadId < 0");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_ALL_LOCATION);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_ALL_LOCATION);
        intent.putExtra("thread_id", threadId);
        context.startActivity(intent);
    }

    public static void nmsStartChatDetailsActivity(Context context, long threadId, int contactId,long selectId,
            boolean needShowLoadAllMsg, boolean needStartNewTask) {
        if (null == context) {
            NmsLog.error(TAG, "Start ChatDetails Activity param error, context is null");
            return;
        }
        if (threadId < 0) {
            NmsLog.error(TAG, "Start ChatDetails Activity param error, threadId < 0");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        int flag = Intent.FLAG_ACTIVITY_CLEAR_TOP;
        if (needStartNewTask) {
            flag |= Intent.FLAG_ACTIVITY_NEW_TASK;
        }
        intent.setFlags(flag);
        intent.setAction(NmsIpMessageConsts.ACTION_CHAT_DETAILS);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_CHAT_DETAILS);
        intent.putExtra("thread_id", threadId);
        intent.putExtra("engineContactId", (short)contactId);
        intent.putExtra("select_id", selectId);
        intent.putExtra(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE, needShowLoadAllMsg);
        context.startActivity(intent);
    }

    public static void nmsStartCreateGroupChatActivity(Context context, int requestCode, int simId) {
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_CONTACT_SELECTION);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_CONTACT_SELECTION);
        intent.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_HISSAGE);
        intent.putExtra(NmsContactSelectionActivity.SELECTMAX, NmsCustomUIConfig.GROUPMEM_MAX_COUNT);
        intent.putExtra(NmsContactSelectionActivity.SIMID, simId);
        ((Activity) context).startActivityForResult(intent, requestCode);
    }

    public static void nmsStartContactSelectionActivityForResult(final Context context,
            final Context nmsContext, final int requestCode) {

        if (null == context || requestCode < 0) {
            NmsLog.error(TAG, "The Context is null or requestCode is invildate");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        final int simId = (int) NmsPlatformAdapter.getInstance(context).getNmsCurrentSimId();

        if (simId == -1) {
            // always ask
            NmsAlertDialogUtils.showSelectSimCardDialog(context, nmsContext,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            int sim_Id = (int) NmsPlatformAdapter.getInstance(context)
                                    .getSimIdBySlotId(which);
				NmsPlatformAdapter.getInstance(context).setCurrentSimId(sim_Id) ;							
                            if (!NmsAlertDialogUtils.checkSimCardActivty(context, nmsContext,
                                    sim_Id, requestCode, -1)) {
                                nmsStartCreateGroupChatActivity(context, requestCode, sim_Id);
                            }
                        }
                    });
            return;
        } else {
            if (!NmsAlertDialogUtils.checkSimCardActivty(context, nmsContext, simId, requestCode, 0)) {
                nmsStartCreateGroupChatActivity(context, requestCode, simId);
            }
        }
    }

    public static void nmsStartNoniSMSContactSelectionActivityForResult(Context context,
            int requestCode) {
        if (null == context || requestCode < 0) {
            NmsLog.error(TAG, "The Context is null or requestCode is invildate");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_CONTACT_SELECTION);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_CONTACT_SELECTION);
        intent.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_NOT_HISSAGE);
        intent.putExtra(NmsContactSelectionActivity.SELECTMAX, Integer.MAX_VALUE);
        ((Activity) context).startActivityForResult(intent, requestCode);
    }

    public static void nmsStartAllContactSelectionActivityForResult(Context context, int requestCode) {
        if (null == context || requestCode < 0) {
            NmsLog.error(TAG, "The Context is null or requestCode is invildate");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_CONTACT_SELECTION);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_CONTACT_SELECTION);
        intent.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_ALL);
        intent.putExtra(NmsContactSelectionActivity.SELECTMAX, Integer.MAX_VALUE);
        ((Activity) context).startActivityForResult(intent, requestCode);
    }

    public static void nmsStartNewGroupChatActivity(Context context, String[] contactIdStrs,
            int simId) {
        if (null == contactIdStrs || contactIdStrs.length == 0) {
            NmsLog.error(TAG, "Selection contactId is null");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_NEW_EDIT_GROUP_CHAT);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_NEW_EDIT_GROUP_CHAT);
        intent.putExtra(NmsContactSelectionActivity.CONTACTTAG, contactIdStrs);
        intent.putExtra(NmsContactSelectionActivity.SIMID, simId);

        context.startActivity(intent);
    }

    public static void nmsStartMediaDetailActivity(Context context, long msgId) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }

        if (msgId < 0) {
            NmsLog.error(TAG, "The msgId is error, msgId:" + msgId);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_ALL_MEDIA_DETAILS);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_ALL_MEDIA_DETAILS);
        intent.putExtra("msgId", msgId);
        context.startActivity(intent);
    }
    public static void nmsStartReadedBurnDetailActivity(Context context, long msgId,int time) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }
        if (msgId < 0) {
            NmsLog.error(TAG, "The msgId is error, msgId:" + msgId);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
            NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_READED_BURN_DETAILS);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_READED_BURN_DETAILS);
        intent.putExtra("msgId", msgId);
        intent.putExtra("time", time);
        context.startActivity(intent);
    }
    public static void nmsFinishReadedBurnDetailActivity(final Context context){
        context.sendBroadcast(new Intent(NmsIpMessageConsts.ACTION_READENBURN)); 
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                context.sendBroadcast(new Intent(NmsIpMessageConsts.ACTION_READENBURN)); 
            }
        }, 300);
    }

    public static void nmsStartQuickContactActivity(Context context, long threadId) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }
        if (threadId < 0) {
            NmsLog.error(TAG, "The threadId is error, threadId:" + threadId);
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_QUICK_CONTACT);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_QUICK_CONTACT);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra("threadId", threadId);
        context.startActivity(intent);
    }

    public static void nmsStartSketchActivityForResult(Context context, int requestCode) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_SKETCH);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_SKETCH);
        ((Activity) context).startActivityForResult(intent, requestCode);
    }

    public static void nmsStartAudioActivityForResult(Context context, int requestCode,
            long maxFileSize) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setAction(NmsIpMessageConsts.ACTION_MAKE_AUDIO);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_AUDIO);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra(NmsAudioActivity.MAX_AUDIO_DURATION, NmsCustomUIConfig.AUDIO_MAX_DURATION);
        intent.putExtra(NmsAudioActivity.MAX_FILE_SIZE_KEY, maxFileSize);
        intent.setType("audio/amr");
        ((Activity) context).startActivityForResult(intent, requestCode);
    }

    public static void nmsStartServiceCenterActivity(Context context) {
        if (null == context) {
            NmsLog.error(TAG, "The Context is null");
            return;
        }
        if(!NmsIpMessageApi.getInstance(context).nmsServiceIsReady()){
        	NmsIpMessageApi.getInstance(context).nmsStartIpService(context);
        }
        Intent intent = new Intent();
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.setAction(NmsIpMessageConsts.ACTION_SERVICE_CENTER);
        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                NmsIpMessageConsts.CLASS_NAME_SERVICE_CENTER);
        context.startActivity(intent);
    }
}
