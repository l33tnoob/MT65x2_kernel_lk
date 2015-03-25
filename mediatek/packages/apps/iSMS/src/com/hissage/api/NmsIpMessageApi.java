package com.hissage.api;

import android.R.integer;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;

import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class NmsIpMessageApi {

    public static final String FUNC_ID_nmsStartIpService = "1";
    public static final String FUNC_ID_nmsServiceIsReady = "2";
    public static final String FUNC_ID_nmsGetActivationStatus = "3";
    public static final String FUNC_ID_nmsEnableIpService = "4";
    public static final String FUNC_ID_nmsDisableIpService = "5";
    public static final String FUNC_ID_nmsGetIpMsgInfo = "6";
    public static final String FUNC_ID_nmsSaveIpMsg = "7";
    public static final String FUNC_ID_nmsDeleteIpMsg = "8";
    public static final String FUNC_ID_nmsSetIpMsgAsViewed = "9";
    public static final String FUNC_ID_nmsSetThreadAsViewed = "10";
    public static final String FUNC_ID_nmsDownloadAttach = "11";
    public static final String FUNC_ID_nmsCancelDownloading = "12";
    public static final String FUNC_ID_nmsIsDownloading = "13";
    public static final String FUNC_ID_nmsGetDownloadProgress = "14";
    public static final String FUNC_ID_nmsGetContactInfoViaThreadId = "15";
    public static final String FUNC_ID_nmsGetContactInfoViaMsgId = "16";
    public static final String FUNC_ID_nmsGetContactInfoViaNumber = "17";
    public static final String FUNC_ID_nmsGetContactInfoViaEngineId = "18";
    public static final String FUNC_ID_nmsIsiSMSNumber = "19";
    public static final String FUNC_ID_nmsNeedShowInviteDlg = "20";
    public static final String FUNC_ID_nmsNeedShowReminderDlg = "21";
    public static final String FUNC_ID_nmsHandleInviteDlgLaterCmd = "22";
    public static final String FUNC_ID_nmsHandleInviteDlgInviteCmd = "23";
    public static final String FUNC_ID_nmsNeedShowSwitchAcctDlg = "24";
    public static final String FUNC_ID_nmsGetGroupIdList = "25";
    public static final String FUNC_ID_nmsEnterChatMode = "26";
    public static final String FUNC_ID_nmsSendChatMode = "27";
    public static final String FUNC_ID_nmsExitFromChatMode = "28";
    public static final String FUNC_ID_nmsSaveChatHistory = "29";
    public static final String FUNC_ID_nmsGetSimInfoViaSimId = "30";
    public static final String FUNC_ID_nmsAddContactToSpamList = "31";
    public static final String FUNC_ID_nmsDeleteContactFromSpamList = "32";
    public static final String FUNC_ID_nmsAddMsgToImportantList = "33";
    public static final String FUNC_ID_nmsDeleteMsgFromImportantList = "34";
    public static final String FUNC_ID_nmsGetContactAvatarViaThreadId = "35";
    public static final String FUNC_ID_nmsGetContactAvatarViaMsgId = "36";
    public static final String FUNC_ID_nmsGetContactAvatarViaNumber = "37";
    public static final String FUNC_ID_nmsGetContactAvatarViaEngineId = "38";
    public static final String FUNC_ID_nmsReSendFailedMsg = "39";
    public static final String FUNC_ID_nmsGetIpMsgCountOfThread = "40";
    public static final String FUNC_ID_nmsGetCaptionFlag = "41";
    public static final String FUNC_ID_nmsGetPhotoCaptionFlag = "42";
    public static final String FUNC_ID_nmsGetVideoCaptionFlag = "43";
    public static final String FUNC_ID_nmsGetAudioCaptionFlag = "44";
    public static final String FUNC_ID_nmsGetAutoDownloadFlag = "45";
    public static final String FUNC_ID_nmsDeleteIpMsgViaThreadId = "46";
    public static final String FUNC_ID_nmsGetIpMsgCountOfTypeInThread = "47";
    public static final String FUNC_ID_nmsDeleteDraftMsgInThread = "48";
    public static final String FUNC_ID_nmsAddActivatePromptStatistics = "53";
    public static final String FUNC_ID_nmsIsIpMessageSrvNumber = "54";
	public static final String FUNC_ID_nmsGetIpMessageSrvNumberName = "55";
	public static final String FUNC_ID_nmsIsIpMsgSendable = "56";
    private static final String FUNC_ID_addPrivateClickTimeStatistics = "57";
    private static final String FUNC_ID_addPrivateOpenFlagStatistics = "58";
    private static final String FUNC_ID_addPrivateContactsStatistics = "59";
    private static final String FUNC_ID_getUnifyPhoneNumber = "60";
    private static final String FUNC_ID_nmsCheckDefaultSmsApp = "61";
    private String AUTH = "com.hissage.remote.api.providers";
    public final Uri API_CONTENT_URI = Uri.parse("content://" + AUTH);

    private Context mContext = null;
    private static NmsIpMessageApi mInstance = null;
    private ContentResolver mApiProviders = null;

    private NmsIpMessageApi(Context c) {
        if (c != null)
            mContext = c.getApplicationContext();
        mApiProviders = mContext.getContentResolver();
    }

    public synchronized static NmsIpMessageApi getInstance(Context c) {
        if (null == mInstance) {

            mInstance = new NmsIpMessageApi(c);
        }
        return mInstance;
    }

    public synchronized void nmsStartIpService(final Context context) {
        Intent i = new Intent(context, NmsService.class);
        context.startService(i);
    }

    public boolean nmsServiceIsReady() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsServiceIsReady, null, null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsServiceIsReady, false);
        } else {
            return false;
        }
    }

    /* Get indicated sim card current activation status */
    public int nmsGetActivationStatus(int sim_id) {
        Bundle param = new Bundle();
        param.putInt(FUNC_ID_nmsGetActivationStatus + 1, sim_id);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetActivationStatus, null,
                param);
        if (back != null) {
            return back.getInt(FUNC_ID_nmsGetActivationStatus, -1);
        } else {
            return -1;
        }
    };

    public void nmsEnableIpService(int sim_id) {
        Bundle param = new Bundle();
        param.putInt(FUNC_ID_nmsEnableIpService + 1, sim_id);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsEnableIpService, null, param);
    };

    public void nmsDisableIpService(int sim_id) {
        Bundle param = new Bundle();
        param.putInt(FUNC_ID_nmsDisableIpService + 1, sim_id);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDisableIpService, null, param);
    };

    public NmsIpMessage nmsGetIpMsgInfo(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsGetIpMsgInfo + 1, msgId);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetIpMsgInfo, null, param);
        if (back != null) {
            return (NmsIpMessage) back.getSerializable(FUNC_ID_nmsGetIpMsgInfo);
        } else {
            return null;
        }
    }

    public int nmsSaveIpMsg(NmsIpMessage msg, int sendMsgMode) {
        Bundle param = new Bundle();
        param.putSerializable(FUNC_ID_nmsSaveIpMsg + 1, msg);
        param.putInt(FUNC_ID_nmsSaveIpMsg + 2, sendMsgMode);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsSaveIpMsg, null, param);
        if (back != null) {
            return back.getInt(FUNC_ID_nmsSaveIpMsg);
        } else {
            return -1;
        }
    }

    public void nmsDeleteIpMsg(final long[] ids, boolean delImportant, boolean isDelMsgInSmsDb) {
        if (ids == null || ids.length <= 0) {
            NmsLog.error(HissageTag.api, "nmsDeleteIpMsg param error.");
            return;
        }
        Bundle param = new Bundle();
        param.putLongArray(FUNC_ID_nmsDeleteIpMsg + 1, ids);
        param.putBoolean(FUNC_ID_nmsDeleteIpMsg + 2, delImportant);
        param.putBoolean(FUNC_ID_nmsDeleteIpMsg + 3, isDelMsgInSmsDb);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDeleteIpMsg, null, param);
    };

    public void nmsDeleteIpMsgViaThreadId(long threadId, boolean isDelMsgInSmsDb) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsDeleteIpMsgViaThreadId + 1, threadId);
        param.putBoolean(FUNC_ID_nmsDeleteIpMsgViaThreadId + 2, isDelMsgInSmsDb);
        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDeleteIpMsgViaThreadId, null, param);
    }

    public void nmsSetIpMsgAsViewed(long msgId) {
        Bundle param = new Bundle();
        param.putShort(FUNC_ID_nmsSetIpMsgAsViewed + 1, (short) msgId);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsSetIpMsgAsViewed, null, param);
    };

    public void nmsSetThreadAsViewed(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsSetThreadAsViewed + 1, threadId);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsSetThreadAsViewed, null, param);

    }

    public void nmsDownloadAttach(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsDownloadAttach + 1, msgId);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDownloadAttach, null, param);
    };

    public void nmsCancelDownloading(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsCancelDownloading + 1, msgId);

        mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsCancelDownloading, null, param);
    };

    public boolean nmsIsDownloading(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsIsDownloading + 1, msgId);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsIsDownloading, null, param);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsIsDownloading, false);
        } else {
            return false;
        }
    }

    public int nmsGetDownloadProgress(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsGetDownloadProgress + 1, msgId);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetDownloadProgress, null,
                param);
        if (back != null) {
            return back.getInt(FUNC_ID_nmsGetDownloadProgress, 0);
        } else {
            return 0;
        }
    }

    public NmsContact nmsGetContactInfoViaThreadId(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsGetContactInfoViaThreadId + 1, threadId);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactInfoViaThreadId,
                null, param);
        if (back != null) {
            return (NmsContact) back.getSerializable(FUNC_ID_nmsGetContactInfoViaThreadId);
        } else {
            return null;
        }
    }

    public NmsContact nmsGetContactInfoViaMsgId(long msgId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsGetContactInfoViaMsgId + 1, msgId);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactInfoViaMsgId,
                    null, param);
            if (back != null) {
                return (NmsContact) back.getSerializable(FUNC_ID_nmsGetContactInfoViaMsgId);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    public NmsContact nmsGetContactInfoViaNumber(String phoneNumber) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsGetContactInfoViaNumber + 1, phoneNumber);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactInfoViaNumber,
                    null, param);
            if (back != null) {
                return (NmsContact) back.getSerializable(FUNC_ID_nmsGetContactInfoViaNumber);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    public NmsContact nmsGetContactInfoViaEngineId(int engineContactId) {
        Bundle param = new Bundle();
        param.putShort(FUNC_ID_nmsGetContactInfoViaEngineId + 1, (short) engineContactId);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactInfoViaEngineId,
                    null, param);
            if (back != null) {
                return (NmsContact) back.getSerializable(FUNC_ID_nmsGetContactInfoViaEngineId);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    public boolean nmsIsiSMSNumber(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsIsiSMSNumber + 1, number);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsIsiSMSNumber, null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsIsiSMSNumber);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public boolean nmsNeedShowInviteDlg(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsNeedShowInviteDlg + 1, threadId);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsNeedShowInviteDlg, null,
                    param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsNeedShowInviteDlg);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public int nmsNeedShowReminderDlg(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsNeedShowReminderDlg + 1, threadId);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsNeedShowReminderDlg, null,
                    param);
            if (back != null) {
                return back.getInt(FUNC_ID_nmsNeedShowReminderDlg);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return 0;
    }

    public boolean nmsHandleInviteDlgLaterCmd(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsHandleInviteDlgLaterCmd + 1, threadId);

        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsHandleInviteDlgLaterCmd,
                    null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsHandleInviteDlgLaterCmd);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public boolean nmsHandleInviteDlgInviteCmd(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsHandleInviteDlgInviteCmd + 1, threadId);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsHandleInviteDlgInviteCmd,
                    null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsHandleInviteDlgInviteCmd);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public boolean nmsNeedShowSwitchAcctDlg(long threadId) {
        Bundle param = new Bundle();
        param.putLong(FUNC_ID_nmsNeedShowSwitchAcctDlg + 1, threadId);

        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsNeedShowSwitchAcctDlg,
                    null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsNeedShowSwitchAcctDlg);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public int[] nmsGetGroupIdList() {

        try {
            Bundle back = mApiProviders
                    .call(API_CONTENT_URI, FUNC_ID_nmsGetGroupIdList, null, null);
            if (back != null) {
                short[] ret = back.getShortArray(FUNC_ID_nmsGetGroupIdList);
                int[] list = null;
                if (ret != null) {
                    list = new int[ret.length];
                    for (int i = 0; i < ret.length; ++i) {
                        list[i] = ret[i];
                    }
                }
                return list;
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    public void nmsEnterChatMode(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsEnterChatMode + 1, number);

        try {
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsEnterChatMode, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void nmsSendChatMode(String number, int status) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsSendChatMode + 1, number);
        param.putInt(FUNC_ID_nmsSendChatMode + 2, status);

        try {
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsSendChatMode, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void nmsExitFromChatMode(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsExitFromChatMode + 1, number);

        try {
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsExitFromChatMode, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void nmsSaveChatHistory(long threadId[]) {
        Bundle param = new Bundle();
        param.putLongArray(FUNC_ID_nmsSaveChatHistory + 1, threadId);

        try {
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsSaveChatHistory, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public SNmsSimInfo nmsGetSimInfoViaSimId(int simId) {
        Bundle param = new Bundle();
        param.putInt(FUNC_ID_nmsGetSimInfoViaSimId + 1, simId);

        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetSimInfoViaSimId, null,
                    param);
            if (back != null) {
                return (SNmsSimInfo) back.getSerializable(FUNC_ID_nmsGetSimInfoViaSimId);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return null;
    }

    public boolean nmsAddContactToSpamList(int[] engineContactIds) {
        Bundle param = new Bundle();
        if (engineContactIds == null || engineContactIds.length <= 0) {
            NmsLog.error(HissageTag.api, "The engineContactIds is null or the length is 0");
            return false;
        }
        short[] s = new short[engineContactIds.length];

        for (int i = 0; i < engineContactIds.length; i++) {
            s[i] = (short) engineContactIds[i];
        }

        param.putShortArray(FUNC_ID_nmsAddContactToSpamList + 1, s);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsAddContactToSpamList, null,
                param);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsAddContactToSpamList);
        } else {
            NmsLog.error(HissageTag.api, "failed to add contact to spam list, return false");
            return false;
        }
    }

    public boolean nmsDeleteContactFromSpamList(int[] engineContactIds) {
        Bundle param = new Bundle();
        if (engineContactIds == null || engineContactIds.length <= 0) {
            NmsLog.error(HissageTag.api, "The engineContactIds is null or the length is 0");
            return false;
        }
        short[] s = new short[engineContactIds.length];

        for (int i = 0; i < engineContactIds.length; i++) {
            s[i] = (short) engineContactIds[i];
        }
        param.putShortArray(FUNC_ID_nmsDeleteContactFromSpamList + 1, s);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDeleteContactFromSpamList,
                null, param);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsDeleteContactFromSpamList);
        } else {
            NmsLog.error(HissageTag.api, "failed to delete contact from spam list, return false");
            return false;
        }
    }

    public boolean nmsAddMsgToImportantList(long[] msgIds) {
        if (msgIds == null || msgIds.length <= 0) {
            NmsLog.error(HissageTag.api, "nmsAddMsgToImportantList param error.");
            return false;
        }
        Bundle param = new Bundle();
        param.putLongArray(FUNC_ID_nmsAddMsgToImportantList + 1, msgIds);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsAddMsgToImportantList, null,
                param);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsAddMsgToImportantList);
        } else {
            NmsLog.error(HissageTag.api, "failed to add msg to important list, return false");
            return false;
        }
    }

    public boolean nmsDeleteMsgFromImportantList(long[] msgIds) {
        if (msgIds == null || msgIds.length <= 0) {
            NmsLog.error(HissageTag.api, "nmsDeleteMsgFromImportantList param error.");
            return false;
        }
        Bundle param = new Bundle();
        param.putLongArray(FUNC_ID_nmsDeleteMsgFromImportantList + 1, msgIds);

        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsDeleteMsgFromImportantList,
                null, param);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsDeleteMsgFromImportantList);
        } else {
            NmsLog.error(HissageTag.api, "failed to delete msg from important list, return false");
            return false;
        }
    }

    public Bitmap nmsGetContactAvatarViaThreadId(long threadId) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsGetContactAvatarViaThreadId + 1, threadId);

            Bundle back = mApiProviders.call(API_CONTENT_URI,
                    FUNC_ID_nmsGetContactAvatarViaThreadId, null, param);
            if (back != null)
                return (Bitmap) back.getParcelable(FUNC_ID_nmsGetContactAvatarViaThreadId);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return null;
    }

    public Bitmap nmsGetContactAvatarViaMsgId(long msgId) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsGetContactAvatarViaMsgId + 1, msgId);

            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactAvatarViaMsgId,
                    null, param);
            if (back != null)
                return (Bitmap) back.getParcelable(FUNC_ID_nmsGetContactAvatarViaMsgId);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return null;
    }

    public Bitmap nmsGetContactAvatarViaNumber(String phoneNumber) {
        try {
            Bundle param = new Bundle();
            param.putString(FUNC_ID_nmsGetContactAvatarViaNumber + 1, phoneNumber);

            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetContactAvatarViaNumber,
                    null, param);
            if (back != null)
                return (Bitmap) back.getParcelable(FUNC_ID_nmsGetContactAvatarViaNumber);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return null;
    }

    public Bitmap nmsGetContactAvatarViaEngineId(int engineContactId) {
        try {
            Bundle param = new Bundle();
            param.putInt(FUNC_ID_nmsGetContactAvatarViaEngineId + 1, engineContactId);

            Bundle back = mApiProviders.call(API_CONTENT_URI,
                    FUNC_ID_nmsGetContactAvatarViaEngineId, null, param);
            if (back != null)
                return (Bitmap) back.getParcelable(FUNC_ID_nmsGetContactAvatarViaEngineId);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return null;
    }

    public void nmsReSendFailedMsg(long simId, long msgId) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsReSendFailedMsg + 1, simId);
            param.putLong(FUNC_ID_nmsReSendFailedMsg + 2, msgId);

            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsReSendFailedMsg, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public int nmsGetIpMsgCountOfThread(long threadId) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsGetIpMsgCountOfThread + 1, threadId);
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetIpMsgCountOfThread,
                    null, param);
            if (back != null)
                return back.getInt(FUNC_ID_nmsGetIpMsgCountOfThread);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return 0;
    }

    public boolean nmsGetCaptionFlag() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetCaptionFlag, null, null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsGetCaptionFlag, false);
        } else {
            return false;
        }
    }

    public boolean nmsGetPhotoCaptionFlag() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetPhotoCaptionFlag, null,
                null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsGetPhotoCaptionFlag, false);
        } else {
            return false;
        }
    }

    public boolean nmsGetVideoCaptionFlag() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetVideoCaptionFlag, null,
                null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsGetVideoCaptionFlag, false);
        } else {
            return false;
        }
    }

    public boolean nmsGetAudioCaptionFlag() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetAudioCaptionFlag, null,
                null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsGetAudioCaptionFlag, false);
        } else {
            return false;
        }
    }

    public boolean nmsGetAutoDownloadFlag() {
        Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetAutoDownloadFlag, null,
                null);
        if (back != null) {
            return back.getBoolean(FUNC_ID_nmsGetAutoDownloadFlag, false);
        } else {
            return false;
        }
    }

    /*
     * the value of typeFlag please refer to the class :
     * NmsIpMessageConsts::NmsIpMessageMediaTypeFlag
     */
    public int nmsGetIpMsgCountOfTypeInThread(long threadId, int typeFlag) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsGetIpMsgCountOfTypeInThread + 1, threadId);
            param.putInt(FUNC_ID_nmsGetIpMsgCountOfTypeInThread + 2, typeFlag);
            Bundle back = mApiProviders.call(API_CONTENT_URI,
                    FUNC_ID_nmsGetIpMsgCountOfTypeInThread, null, param);
            if (back != null)
                return back.getInt(FUNC_ID_nmsGetIpMsgCountOfTypeInThread, 0);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return 0;
    }
    
    public int nmsDeleteDraftMsgInThread(long threadId) {
        try {
            Bundle param = new Bundle();
            param.putLong(FUNC_ID_nmsDeleteDraftMsgInThread + 1, threadId);
            Bundle back = mApiProviders.call(API_CONTENT_URI,
                    FUNC_ID_nmsDeleteDraftMsgInThread, null, param);
            if (back != null)
                return back.getInt(FUNC_ID_nmsDeleteDraftMsgInThread, -1);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        return 0;
    }
	//M: Activation Statistics
    public void nmsAddActivatePromptStatistics(int type) {
        try {
            Bundle param = new Bundle();
            param.putInt(FUNC_ID_nmsAddActivatePromptStatistics + 1, type);
            mApiProviders.call(API_CONTENT_URI,
                    FUNC_ID_nmsAddActivatePromptStatistics, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public boolean nmsIsIpMessageSrvNumber(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsIsIpMessageSrvNumber + 1, number);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsIsIpMessageSrvNumber, null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsIsIpMessageSrvNumber);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }

    public String nmsGetIpMessageSrvNumberName(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsGetIpMessageSrvNumberName + 1, number);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsGetIpMessageSrvNumberName,
                    null, param);
            if (back != null) {
                return back.getString(FUNC_ID_nmsGetIpMessageSrvNumberName);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }
	public boolean nmsIsIpMsgSendable(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_nmsIsIpMsgSendable + 1, number);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsIsIpMsgSendable, null, param);
            if (back != null) {
                return back.getBoolean(FUNC_ID_nmsIsIpMsgSendable);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return false;
    }
   /// M: Private Statistics
    public void addPrivateClickTimeStatistics(int clickTime) {
        try {
            Bundle param = new Bundle();
            param.putInt(FUNC_ID_addPrivateClickTimeStatistics + 1, clickTime);
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_addPrivateClickTimeStatistics, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void addPrivateOpenFlagStatistics(int openFlag) {
        try {
            Bundle param = new Bundle();
            param.putInt(FUNC_ID_addPrivateOpenFlagStatistics + 1, openFlag);
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_addPrivateOpenFlagStatistics, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void addPrivateContactsStatistics(int contacts) {
        try {
            Bundle param = new Bundle();
            param.putInt(FUNC_ID_addPrivateContactsStatistics + 1, contacts);
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_addPrivateContactsStatistics, null, param);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }
    
    public String nmsGetUnifyPhoneNumber(String number) {
        Bundle param = new Bundle();
        param.putString(FUNC_ID_getUnifyPhoneNumber + 1, number);
        try {
            Bundle back = mApiProviders.call(API_CONTENT_URI, FUNC_ID_getUnifyPhoneNumber,
                    null, param);
            if (back != null) {
                return back.getString(FUNC_ID_getUnifyPhoneNumber);
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }
    
    public void nmsCheckDefaultSmsAppChanged() {
        try {
            mApiProviders.call(API_CONTENT_URI, FUNC_ID_nmsCheckDefaultSmsApp, null, null);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

}
