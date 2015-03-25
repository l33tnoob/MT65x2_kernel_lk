package com.hissage.api;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;

import com.hissage.config.NmsConfig;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpMessage;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.service.NmsService;
import com.hissage.util.log.NmsLog;

public class NmsApiProvider extends ContentProvider {

    private static final int FUNC_ID_nmsStartIpService = 1;
    private static final int FUNC_ID_nmsServiceIsReady = 2;
    private static final int FUNC_ID_nmsGetActivationStatus = 3;
    private static final int FUNC_ID_nmsEnableIpService = 4;
    private static final int FUNC_ID_nmsDisableIpService = 5;
    private static final int FUNC_ID_nmsGetIpMsgInfo = 6;
    private static final int FUNC_ID_nmsSaveIpMsg = 7;
    private static final int FUNC_ID_nmsDeleteIpMsg = 8;
    private static final int FUNC_ID_nmsSetIpMsgAsViewed = 9;
    private static final int FUNC_ID_nmsSetThreadAsViewed = 10;
    private static final int FUNC_ID_nmsDownloadAttach = 11;
    private static final int FUNC_ID_nmsCancelDownloading = 12;
    private static final int FUNC_ID_nmsIsDownloading = 13;
    private static final int FUNC_ID_nmsGetDownloadProgress = 14;
    private static final int FUNC_ID_nmsGetContactInfoViaThreadId = 15;
    private static final int FUNC_ID_nmsGetContactInfoViaMsgId = 16;
    private static final int FUNC_ID_nmsGetContactInfoViaNumber = 17;
    private static final int FUNC_ID_nmsGetContactInfoViaEngineId = 18;
    private static final int FUNC_ID_nmsIsiSMSNumber = 19;
    private static final int FUNC_ID_nmsNeedShowInviteDlg = 20;
    private static final int FUNC_ID_nmsNeedShowReminderDlg = 21;
    private static final int FUNC_ID_nmsHandleInviteDlgLaterCmd = 22;
    private static final int FUNC_ID_nmsHandleInviteDlgInviteCmd = 23;
    private static final int FUNC_ID_nmsNeedShowSwitchAcctDlg = 24;
    private static final int FUNC_ID_nmsGetGroupIdList = 25;
    private static final int FUNC_ID_nmsEnterChatMode = 26;
    private static final int FUNC_ID_nmsSendChatMode = 27;
    private static final int FUNC_ID_nmsExitFromChatMode = 28;
    private static final int FUNC_ID_nmsSaveChatHistory = 29;
    private static final int FUNC_ID_nmsGetSimInfoViaSimId = 30;
    private static final int FUNC_ID_nmsAddContactToSpamList = 31;
    private static final int FUNC_ID_nmsDeleteContactFromSpamList = 32;
    private static final int FUNC_ID_nmsAddMsgToImportantList = 33;
    private static final int FUNC_ID_nmsDeleteMsgFromImportantList = 34;
    private static final int FUNC_ID_nmsGetContactAvatarViaThreadId = 35;
    private static final int FUNC_ID_nmsGetContactAvatarViaMsgId = 36;
    private static final int FUNC_ID_nmsGetContactAvatarViaNumber = 37;
    private static final int FUNC_ID_nmsGetContactAvatarViaEngineId = 38;
    private static final int FUNC_ID_nmsReSendFailedMsg = 39;
    private static final int FUNC_ID_nmsGetIpMsgCountOfThread = 40;
    private static final int FUNC_ID_nmsGetCaptionFlag = 41;
    private static final int FUNC_ID_nmsGetPhotoCaptionFlag = 42;
    private static final int FUNC_ID_nmsGetVideoCaptionFlag = 43;
    private static final int FUNC_ID_nmsGetAudioCaptionFlag = 44;
    private static final int FUNC_ID_nmsGetAutoDownloadFlag = 45;
    private static final int FUNC_ID_nmsDeleteIpMsgViaThreadId = 46;
    private static final int FUNC_ID_nmsGetIpMsgCountOfTypeInThread = 47;
    private static final int FUNC_ID_nmsDeleteDraftMsgInThread = 48;
	//M: Activation Statistics
    private static final int FUNC_ID_nmsAddActivatePromptStatistics = 53;
    
    private static final int FUNC_ID_nmsIsIpMessageSrvNumber = 54;
	private static final int FUNC_ID_nmsGetIpMessageSrvNumberName = 55;

	private static final int FUNC_ID_nmsIsIpMsgSendable = 56 ;
    /// M: Private Statistics
    private static final int FUNC_ID_addPrivateClickTimeStatistics = 57;
    private static final int FUNC_ID_addPrivateOpenFlagStatistics = 58;
    private static final int FUNC_ID_addPrivateContactsStatistics = 59;
    private static final int FUNC_ID_getUnifyPhoneNumber = 60;
    
    /* for feature use when MMS can detect the default sms app changed event */
    private static final int FUNC_ID_nmsCheckDefaultSmsApp = 61;
    
    private String AUTH = "com.hissage.remote.api.providers";
    public final Uri API_CONTENT_URI = Uri.parse("content://" + AUTH);
    private String TAG = "NmsApiProvider";

    @Override
    public Bundle call(String method, String arg, Bundle extras) {
        if (!NmsService.bCEngineRunflag) {
            Intent i = new Intent(this.getContext(), NmsService.class);
            this.getContext().startService(i);
            NmsLog.error(TAG, "NmsService is not running, so start it.");
        }

        try {
            synchronized (NmsSMSMMSManager.getInstance(this.getContext())) {
                if (!NmsConfig.mIsDBInitDone) {
                    NmsLog.error(TAG,
                            "NmsService is running, but Engine not init done, so have to wait it.....");
                    NmsSMSMMSManager.getInstance(this.getContext()).wait();
                }
            }
        } catch (InterruptedException e) {
            NmsLog.nmsPrintStackTrace(e);
            NmsLog.error(TAG,
                    "NmsService is running, but Engine not init done, so have to wait it.....Done or time out:");
        }

        if (TextUtils.isEmpty(method)) {
            NmsLog.error(TAG, "method is null, so we do not know call which one.");
            return null;
        }

        Bundle retBundle = new Bundle();

        switch (Integer.parseInt(method)) {
        case FUNC_ID_nmsStartIpService:
            break;
        case FUNC_ID_nmsServiceIsReady:
            retBundle.putBoolean(method, NmsConfig.mIsDBInitDone);
            break;
        case FUNC_ID_nmsGetActivationStatus:
            retBundle.putInt(method,
                    NmsIpMessageApiNative.nmsGetActivationStatus(extras.getInt(method + 1, 0)));
            break;
        case FUNC_ID_nmsEnableIpService:
            NmsIpMessageApiNative.nmsEnableIpService(extras.getInt(method + 1, 0));
            break;
        case FUNC_ID_nmsDisableIpService:
            NmsIpMessageApiNative.nmsDisableIpService(extras.getInt(method + 1, 0));
            break;
        case FUNC_ID_nmsGetIpMsgInfo:
            retBundle.putSerializable(method,
                    NmsIpMessageApiNative.nmsGetIpMsgInfo(extras.getLong(method + 1, 0)));
            break;
        case FUNC_ID_nmsSaveIpMsg:
            retBundle.putInt(
                    method,
                    NmsIpMessageApiNative.nmsSaveIpMsg(
                            (NmsIpMessage) extras.getSerializable(method + 1),extras.getInt(method + 2, 0), false, false));
            break;
        case FUNC_ID_nmsDeleteIpMsg:
            long[] ids = extras.getLongArray(method + 1);
            short[] realIds = new short[ids.length];
            for (int i = 0; i < ids.length; ++i) {
                short realId = NmsSMSMMSManager.getInstance(this.getContext())
                        .getNmsRecordIDViaSysId(ids[i]);
                if (realId > 0) {
                    realIds[i] = realId;
                }
            }
            NmsIpMessageApiNative.nmsDeleteIpMsg(realIds, extras.getBoolean(method + 2, false),
                    extras.getBoolean(method + 3, false));
            break;
        case FUNC_ID_nmsSetIpMsgAsViewed:
            NmsIpMessageApiNative.nmsSetIpMsgAsViewed(extras.getShort(method + 1, (short) 0));
            break;
        case FUNC_ID_nmsSetThreadAsViewed:
            engineadapter.get().nmsUISetContactMsgReaded(
                    NmsSMSMMSManager.getInstance(getContext()).getEngineContactIdViaThreadId(
                            extras.getLong(method + 1)));
            break;
        case FUNC_ID_nmsDownloadAttach:
            NmsIpMessageApiNative
                    .nmsDownloadAttach(NmsSMSMMSManager.getInstance(getContext())
                            .getNmsRecordIDViaSysId(extras.getLong(method + 1)), extras
                            .getLong(method + 1));
            break;
        case FUNC_ID_nmsCancelDownloading:
            NmsIpMessageApiNative
                    .nmsCancelDownloading(NmsSMSMMSManager.getInstance(getContext())
                            .getNmsRecordIDViaSysId(extras.getLong(method + 1)), extras
                            .getLong(method + 1));
            break;

        case FUNC_ID_nmsIsDownloading:
            retBundle.putBoolean(
                    method,
                    NmsIpMessageApiNative.nmsIsDownloading(
                            NmsSMSMMSManager.getInstance(getContext()).getNmsRecordIDViaSysId(
                                    extras.getLong(method + 1)), extras.getLong(method + 1)));
            break;
        case FUNC_ID_nmsGetDownloadProgress:
            retBundle.putInt(
                    method,
                    NmsIpMessageApiNative.nmsGetDownloadProgress(
                            NmsSMSMMSManager.getInstance(getContext()).getNmsRecordIDViaSysId(
                                    extras.getLong(method + 1)), extras.getLong(method + 1)));
            break;
        case FUNC_ID_nmsGetContactInfoViaThreadId:
            retBundle.putSerializable(method, NmsIpMessageApiNative
                    .nmsGetContactInfoViaThreadId(extras.getLong(method + 1, 0)));
            break;
        case FUNC_ID_nmsGetContactInfoViaMsgId:
            retBundle.putSerializable(method,
                    NmsIpMessageApiNative.nmsGetContactInfoViaMsgId(extras.getLong(method + 1, 0)));
            break;
        case FUNC_ID_nmsGetContactInfoViaNumber:
            retBundle.putSerializable(method,
                    NmsIpMessageApiNative.nmsGetContactInfoViaNumber(extras.getString(method + 1)));
            break;
        case FUNC_ID_nmsGetContactInfoViaEngineId:
            retBundle.putSerializable(method, NmsIpMessageApiNative
                    .nmsGetContactInfoViaEngineId(extras.getShort(method + 1, (short) 0)));
            break;
        case FUNC_ID_nmsIsiSMSNumber:
            retBundle.putBoolean(method,
                    NmsIpMessageApiNative.nmsIsiSMSNumber(extras.getString(method + 1)));
            break;
        case FUNC_ID_nmsNeedShowInviteDlg:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsNeedShowInviteDlg(
                    getContext(),
                    NmsSMSMMSManager.getInstance(getContext()).getEngineContactIdViaThreadId(
                            extras.getLong(method + 1, -1))));
            break;
        case FUNC_ID_nmsNeedShowReminderDlg:
            retBundle.putInt(method, NmsIpMessageApiNative.nmsNeedShowReminderDlg(
                    getContext(),
                    NmsSMSMMSManager.getInstance(getContext()).getEngineContactIdViaThreadId(
                            extras.getLong(method + 1, -1))));
            break;
        case FUNC_ID_nmsHandleInviteDlgLaterCmd:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsHandleInviteDlgLaterCmd(
                    getContext(), NmsSMSMMSManager.getInstance(getContext())
                            .getEngineContactIdViaThreadId(extras.getLong(method + 1, -1))));
            break;
        case FUNC_ID_nmsHandleInviteDlgInviteCmd:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsHandleInviteDlgInviteCmd(
                    getContext(), NmsSMSMMSManager.getInstance(getContext())
                            .getEngineContactIdViaThreadId(extras.getLong(method + 1, -1))));
            break;
        case FUNC_ID_nmsNeedShowSwitchAcctDlg:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsNeedShowSwitchAcctDlg(
                    getContext(), NmsSMSMMSManager.getInstance(getContext())
                            .getEngineContactIdViaThreadId(extras.getLong(method + 1, -1))));
            break;
        case FUNC_ID_nmsGetGroupIdList:
            retBundle.putShortArray(method, NmsIpMessageApiNative.nmsGetGroupIdList());
            break;
        case FUNC_ID_nmsEnterChatMode:
            NmsIpMessageApiNative.nmsEnterChatMode(extras.getString(method + 1));
            break;
        case FUNC_ID_nmsSendChatMode:
            NmsIpMessageApiNative.nmsSendChatMode(extras.getString(method + 1),
                    extras.getInt(method + 2));
            break;
        case FUNC_ID_nmsExitFromChatMode:
            NmsIpMessageApiNative.nmsExitFromChatMode(extras.getString(method + 1));
            break;
        case FUNC_ID_nmsSaveChatHistory:
            NmsIpMessageApiNative.nmsSaveChatHistory(extras.getLongArray(method + 1));
            break;
        case FUNC_ID_nmsGetSimInfoViaSimId:
            retBundle.putSerializable(method,
                    NmsIpMessageApiNative.nmsGetSimInfoViaSimId(extras.getInt(method + 1, 0)));
            break;

        case FUNC_ID_nmsAddContactToSpamList:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsAddContactToSpamList(extras.getShortArray(method + 1)));
            break;
        case FUNC_ID_nmsDeleteContactFromSpamList:
            retBundle.putBoolean(method, NmsIpMessageApiNative.nmsDeleteContactFromSpamList(extras.getShortArray(method + 1)));
            break;
        case FUNC_ID_nmsAddMsgToImportantList:
            long[] idList = extras.getLongArray(method + 1);
            short[] realIdList = new short[idList.length];
            for (int i = 0; i < idList.length; ++i) {
                short realId = NmsSMSMMSManager.getInstance(this.getContext())
                        .getNmsRecordIDViaSysId(idList[i]);
                if (realId > 0) {
                    realIdList[i] = realId;
                }
            }
            retBundle
                    .putBoolean(method, NmsIpMessageApiNative.nmsAddMsgToImportantList(realIdList));
            break;
        case FUNC_ID_nmsDeleteMsgFromImportantList:
            long[] idList1 = extras.getLongArray(method + 1);
            short[] realIdList1 = new short[idList1.length];
            for (int i = 0; i < idList1.length; ++i) {
                short realId = NmsSMSMMSManager.getInstance(this.getContext())
                        .getNmsRecordIDViaSysId(idList1[i]);
                if (realId > 0) {
                    realIdList1[i] = realId;
                }
            }
            retBundle.putBoolean(method,
                    NmsIpMessageApiNative.nmsDeleteMsgFromImportantList(realIdList1));
            break;

        case FUNC_ID_nmsGetContactAvatarViaThreadId:
            retBundle.putParcelable(method, NmsIpMessageApiNative
                    .nmsGetContactAvatarViaThreadId(extras.getLong(method + 1, 0)));
            break;

        case FUNC_ID_nmsGetContactAvatarViaMsgId:
            retBundle.putParcelable(method, NmsIpMessageApiNative
                    .nmsGetContactAvatarViaMsgId(extras.getLong(method + 1, 0)));
            break;

        case FUNC_ID_nmsGetContactAvatarViaNumber:
            retBundle.putParcelable(method, NmsIpMessageApiNative
                    .nmsGetContactAvatarViaNumber(extras.getString(method + 1, "")));
            break;

        case FUNC_ID_nmsGetContactAvatarViaEngineId:
            retBundle.putParcelable(method, NmsIpMessageApiNative
                    .nmsGetContactAvatarViaEngineId((short) extras.getInt(method + 1, 0)));
            break;
        case FUNC_ID_nmsReSendFailedMsg:
            NmsIpMessageApiNative.nmsResendMsg(extras.getLong(method + 1, 0),
                    extras.getLong(method + 2, 0));
            break;
        case FUNC_ID_nmsGetIpMsgCountOfThread:
            retBundle.putInt(method,
                    NmsIpMessageApiNative.nmsGetIpMsgCountOfThread(extras.getLong(method + 1, 0)));
            break;
        case FUNC_ID_nmsGetCaptionFlag:
            retBundle.putBoolean(method, NmsConfig.getCaptionFlag());
            break;
        case FUNC_ID_nmsGetPhotoCaptionFlag:
            retBundle.putBoolean(method, NmsConfig.getPhotoCaptionFlag());
            break;
        case FUNC_ID_nmsGetVideoCaptionFlag:
            retBundle.putBoolean(method, NmsConfig.getVideoCaptionFlag());
            break;
        case FUNC_ID_nmsGetAudioCaptionFlag:
            retBundle.putBoolean(method, NmsConfig.getAudioCaptionFlag());
            break;
        case FUNC_ID_nmsGetAutoDownloadFlag:
            retBundle.putBoolean(method, NmsConfig.getAutoDownloadFlag());
            break;
        case FUNC_ID_nmsDeleteIpMsgViaThreadId:
            NmsIpMessageApiNative.nmsDeleteIpMsgViaThreadId(extras.getLong(method + 1, 0),
                    extras.getBoolean(method + 2, false));
            break;
        case FUNC_ID_nmsGetIpMsgCountOfTypeInThread:
            retBundle.putInt(
                    method,
                    NmsIpMessageApiNative.nmsGetIpMsgCountOfTypeInThread(
                            extras.getLong(method + 1, -1), extras.getInt(method + 2, 0)));
            break;
        case FUNC_ID_nmsDeleteDraftMsgInThread:    
            retBundle.putInt(
                    method,
                    NmsIpMessageApiNative.nmsDeleteDraftMsgInThread(
                            extras.getLong(method + 1, -1)));
            break;        
		//M: Activation Statistics
        case FUNC_ID_nmsAddActivatePromptStatistics:
            NmsIpMessageApiNative.nmsAddActivatePromptStatistics(extras.getInt(method + 1, NmsIpMessageConsts.NmsUIActivateType.OTHER)) ;
            break ;

        case FUNC_ID_nmsIsIpMessageSrvNumber:
            retBundle.putBoolean(method,
                    NmsIpMessageApiNative.nmsIsIpMessageSrvNumber(extras.getString(method + 1,"")));
            break;  
		case FUNC_ID_nmsGetIpMessageSrvNumberName:
			retBundle.putString(method,
					NmsIpMessageApiNative.nmsGetIpMessageSrvNumberName(extras.getString(method + 1,"")));
			break;	
		case FUNC_ID_nmsIsIpMsgSendable:
			retBundle.putBoolean(method,
                    NmsIpMessageApiNative.nmsIsIpMsgSendable(extras.getString(method + 1)));
			break ;

		case FUNC_ID_addPrivateClickTimeStatistics:
            NmsIpMessageApiNative.addPrivateClickTimeStatistics(extras.getInt(method + 1, 0));
            break;
        case FUNC_ID_addPrivateOpenFlagStatistics:
            NmsIpMessageApiNative.addPrivateOpenFlagStatistics(extras.getInt(method + 1, 0));
            break;
        case FUNC_ID_addPrivateContactsStatistics:
            NmsIpMessageApiNative.addPrivateContactsStatistics(extras.getInt(method + 1, 0));
            break;
        case FUNC_ID_getUnifyPhoneNumber:
            retBundle.putString(method,
                    NmsIpMessageApiNative.getUnifyPhoneNumber(extras.getString(method + 1,"")));
            break;
            
        case FUNC_ID_nmsCheckDefaultSmsApp:
            NmsIpMessageApiNative.nmsCheckDefaultSmsAppChanged() ;
            break ;
            
        default:
            NmsLog.error(TAG, "arg changed ok, but can not find this api, u sure add it correctly?"
                    + method);
            return null;
        }

        return retBundle;
    }

    @Override
    public int delete(Uri arg0, String arg1, String[] arg2) {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public String getType(Uri uri) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public boolean onCreate() {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        // TODO Auto-generated method stub
        return 0;
    }

}
