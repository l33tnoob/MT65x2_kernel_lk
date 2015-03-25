package com.hissage.util.message;

import java.io.File;
import java.util.ArrayList;
import java.util.Date;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.CamcorderProfile;
import android.media.RingtoneManager;
import android.net.Uri;
import android.text.TextUtils;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.text.style.URLSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Toast;
import android.content.SharedPreferences;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.activity.NmsChatDetailsActivity;
import com.hissage.util.data.NmsDateTool;
import com.hissage.util.log.NmsLog;

public class MessageUtils {

    private static final String TAG = "MessageUtils";

    private static final String EXTRA_SHORTCUT_TOTAL_NUMBER = "com.android.launcher2.extra.shortcut.totalnumber";
    private static final String EXTRA_SHORTCUT_STEP_NUMBER = "com.android.launcher2.extra.shortcut.stepnumber";

    public static final String TEXT_SIZE = "message_font_size";

    public static float getMTKPreferenceFontFloat(Context mContext, float defaultValue) {
        float textSize = 18.0f;
        try {
            Context context = mContext.createPackageContext("com.android.mms",
                    Context.CONTEXT_IGNORE_SECURITY);
            SharedPreferences sp = context.getSharedPreferences("com.android.mms_preferences",
                    Context.MODE_PRIVATE | Context.MODE_MULTI_PROCESS);
            textSize = sp.getFloat(TEXT_SIZE, defaultValue);

        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }

        NmsLog.trace(TAG, "get current message text size:" + textSize);
        return textSize;
    }

    public static boolean isValidAttach(String path, boolean inspectSize) {
        if (!NmsCommonUtils.isExistsFile(path) || NmsCommonUtils.getFileSize(path) == 0) {
            NmsLog.error(TAG, "isValidAttach: file is not exist, or size is 0");
            return false;
        }
        if (inspectSize && NmsCommonUtils.getFileSize(path) > NmsCustomUIConfig.MAX_ATTACH_SIZE) {
            NmsLog.error(TAG, "file size is too large");
            return false;
        }
        return true;
    }

    public static boolean isCurrentSim(Context context, int simId) {
        long curSimId = NmsPlatformAdapter.getInstance(context).getCurrentSimId();
        return curSimId == simId;
    }

    public static String getServiceIndiactorName(Context context, int simId, int protocol) {
        String name = NmsPlatformAdapter.getInstance(context).getSimName(simId);
        if (null == name) {
            if (protocol == NmsIpMessageConsts.NmsMessageProtocol.SMS) {
                name = context.getResources().getString(R.string.STR_NMS_PROTOCOL_SMS);
            } else if (protocol == NmsIpMessageConsts.NmsMessageProtocol.MMS) {
                name = context.getResources().getString(R.string.STR_NMS_PROTOCOL_MMS);
            } else {
                name = context.getResources().getString(R.string.STR_NMS_PROTOCOL_IP);
            }
        }
        return name;
    }

    public static int getServiceIndicatorColor(Context context, int simId) {
        int resId = NmsPlatformAdapter.getInstance(context).getSimColor(simId);
        return resId > 0 ? resId : 0;
    }

    public static CharSequence getMTKServiceIndicator(Context context, int simId) {
        return NmsPlatformAdapter.getInstance(context).getSimIndicator(simId);
    }

    public static void callContact(Context context, String number) {
        Intent intent = new Intent(Intent.ACTION_DIAL, Uri.parse("tel://" + number));
        context.startActivity(intent);
    }

    public static boolean delShortcutFromHomeScr(Context context, short contactId) {

        Intent shortcutIntent = new Intent("com.android.launcher.action.UNINSTALL_SHORTCUT");
        String name = null;
        NmsContact contact = null;
        if (contactId > 0) {
            contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
            if (null == contact)
                return false;
            name = contact.getName();
        }

        if (TextUtils.isEmpty(name)) {
            shortcutIntent.putExtra(Intent.EXTRA_SHORTCUT_NAME,
                    context.getString(R.string.STR_NMS_MAIN));
        } else {
            shortcutIntent.putExtra(Intent.EXTRA_SHORTCUT_NAME, name);
        }

        shortcutIntent.putExtra("duplicate", false);

        Intent intent = new Intent(Intent.ACTION_MAIN);

        if (contactId <= 0 || null == contact) {
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setAction("android.intent.action.MAIN");

        } else {
            intent.addCategory("android.intent.category.DEFAULT");
            intent.setAction(NmsIpMessageConsts.NMS_ACTION_VIEW);

            intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, contactId);
        }

        shortcutIntent.putExtra(Intent.EXTRA_SHORTCUT_INTENT, intent);

        context.sendBroadcast(shortcutIntent);
        return true;
    }
    public static void selectRingtone(Context context, int requestCode) {
        if (context instanceof Activity) {
            Intent intent = new Intent(RingtoneManager.ACTION_RINGTONE_PICKER);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_SILENT, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_INCLUDE_DRM, false);
            intent.putExtra(RingtoneManager.EXTRA_RINGTONE_TITLE,
                    context.getString(R.string.STR_NMS_SELECT_AUDIO));
            ((Activity) context).startActivityForResult(intent, requestCode);
        }
    }

    public static void selectAudio(Context context, int requestCode) {
        if (context instanceof Activity) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            intent.addCategory(intent.CATEGORY_OPENABLE);
            intent.setType("audio/*");
            intent.setType("application/ogg");
            intent.setType("application/x-ogg");
            ((Activity) context).startActivityForResult(intent, requestCode);
        }
    }

    public static void createLoseSDCardNotice(Context context, int resId) {
        new AlertDialog.Builder(context).setTitle(R.string.STR_NMS_NO_SD_TITLE)
                .setIcon(R.drawable.ic_dialog_alert_holo_light).setMessage(resId)
                .setPositiveButton(R.string.STR_NMS_OK, null).create().show();
    }

    public static void clearChatHistory(final Context context, final short contactId) {
        new AlertDialog.Builder(context).setIcon(R.drawable.ic_dialog_alert_holo_light)
                .setTitle(R.string.STR_NMS_SKETCH_DELETE)
                .setMessage(R.string.STR_NMS_CLEAR_HISTORY_CONTENT)
                .setPositiveButton(R.string.STR_NMS_DELETE, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        showDeleteThreadWaitDialog(context, contactId, false);
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).create().show();
    }

    public static void showLeaveGroupDialog(final Context context, final short contactId,
            final boolean finish) {
        LayoutInflater inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View llNoNotice = inflater.inflate(R.layout.dialog_with_checkbox, null);
        final CheckBox notice = (CheckBox) llNoNotice.findViewById(R.id.NoNotice);
        notice.setText(R.string.STR_NMS_CLEAR_HISTORY_CONTENT);

        new AlertDialog.Builder(context).setIcon(R.drawable.ic_dialog_alert_holo_light)
                .setTitle(R.string.STR_NMS_LEAVE_GROUP)
                .setMessage(R.string.STR_NMS_LEAVE_GROUP_CONTENT).setView(llNoNotice)
                .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        if (NmsIpMessageApiNative.nmsExitFromGroup(contactId, notice.isChecked())) {
                            if (notice.isChecked()) {
                                if (context instanceof NmsChatDetailsActivity) {
                                    ((NmsChatDetailsActivity) context).nmsUnregisterReceiver();
                                }
                                showDeleteThreadWaitDialog(context, contactId, true);
                            } else {
                                if (finish) {
                                    if (context instanceof NmsChatDetailsActivity) {
                                        ((NmsChatDetailsActivity) context).nmsUnregisterReceiver();
                                    }
                                    ((Activity) context).finish();
                                }
                            }

                            Toast.makeText(context, R.string.STR_NMS_LEAVE_GROUP_SUCCESS,
                                    Toast.LENGTH_SHORT).show();
                            MessageUtils.delShortcutFromHomeScr(context, contactId);
                        } else {
                            Toast.makeText(context, R.string.STR_NMS_LEAVE_GROUP_FAILED,
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).create().show();
    }

    public static void showDeleteThreadWaitDialog(final Context context, final short contactId,
            final boolean needFinish) {
        final ProgressDialog dlg = new ProgressDialog(context);
        dlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dlg.setMessage(context.getString(R.string.STR_NMS_CLEARING));
        dlg.setIndeterminate(false);
        dlg.setCancelable(false);
        dlg.show();
        new Thread() {
            @Override
            public void run() {
                long current = System.currentTimeMillis();
                engineadapter.get().nmsUIDeleteMsgViaContactRecId(contactId, 1, 0, 1);
                long end = System.currentTimeMillis();
                if (end - current < 2000) {
                    try {
                        sleep(2000 + current - end);
                    } catch (InterruptedException e) {
                        NmsLog.nmsPrintStackTrace(e);
                    }
                }
                dlg.dismiss();
                if (needFinish) {
                    Intent intent = new Intent();
                    intent.setClassName("com.android.mms", "com.android.mms.ui.ConversationList");
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                    context.startActivity(intent);
                    ((Activity) context).finish();
                }
            }
        }.start();
    }

    public static ArrayList<String> extractUris(URLSpan[] spans) {
        int size = spans.length;
        ArrayList<String> accumulator = new ArrayList<String>();

        for (int i = 0; i < size; i++) {
            accumulator.add(spans[i].getURL());
        }
        return accumulator;
    }

    public static String formatFileSize(int size) {
        String result = "";
        int M = 1024 * 1024;
        int K = 1024;
        if (size > M) {
            int s = size % M / 100;
            if (s == 0) {
                result = size / M + "MB";
            } else {
                result = size / M + "." + s + "MB";
            }
        } else if (size > K) {
            int s = size % K / 100;
            if (s == 0) {
                result = size / K + "KB";
            } else {
                result = size / K + "." + s + "KB";
            }
        } else if (size > 0) {
            result = size + "B";
        } else {
            result = "invalid size";
        }
        return result;
    }

    public static String formatAudioTime(int duration) {
        String result = "";
        if (duration > 60) {
            if (duration % 60 == 0) {
                result = duration / 60 + "'";
            } else {
                result = duration / 60 + "'" + duration % 60 + "\"";
            }
        } else if (duration > 0) {
            result = duration + "\"";
        } else {
            result = "no duration";
        }
        return result;
    }

    public static String formatTimeStampString(Context context, long when) {
        return formatTimeStampString(context, when, false);
    }

    public static String formatTimeStampString(Context context, long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                | DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make
        // showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(context, when, format_flags);
    }

    public static boolean shouldShowTimeDivider(long curTime, long nextTime) {
        Date curDate = new Date(curTime);
        Date nextDate = new Date(nextTime);
        Date cur = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
        Date next = new Date(nextDate.getYear(), nextDate.getMonth(), nextDate.getDate(), 0, 0, 0);
        return (cur.getTime() != next.getTime());
    }

    public static String getShortTimeString(Context context, long time) {
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                | DateUtils.FORMAT_CAP_AMPM;
        format_flags |= DateUtils.FORMAT_SHOW_TIME;
        return DateUtils.formatDateTime(context, time, format_flags);
    }

    public static int getAllMediaTimeDividerType(long when) {
        if (when < 0) {
            NmsLog.error(TAG, "when < 0");
            return -1;
        }

        Time now = new Time();
        now.set(System.currentTimeMillis());
        int nowYear = now.year;
        int nowMonth = now.month + 1;
        int nowWeek = now.getWeekNumber();

        Time then = new Time();
        then.set(when);
        int thenYear = then.year;
        int thenMonth = then.month + 1;
        int thenWeek = then.getWeekNumber();

        int type = 0;
        if (nowYear == thenYear && nowMonth == thenMonth && nowWeek == thenWeek) {
            type = thenYear * 10000 + thenMonth * 100 + thenWeek;
        } else {
            type = thenYear * 10000 + thenMonth * 100;
        }

        return type;
    }

    public static String monthNumber2String(Context context, int number) {
        String monthString = "";

        switch (number) {
        case 1:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_JAN);
            break;
        case 2:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_FEB);
            break;
        case 3:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_MAR);
            break;
        case 4:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_APR);
            break;
        case 5:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_MAY);
            break;
        case 6:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_JUN);
            break;
        case 7:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_JUL);
            break;
        case 8:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_AUG);
            break;
        case 9:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_SEPT);
            break;
        case 10:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_OCT);
            break;
        case 11:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_NOV);
            break;
        case 12:
            monthString = context.getString(R.string.STR_NMS_ALL_MEDIA_DEC);
            break;
        default:
            NmsLog.error("monthNumber2String", "month number(" + number + ") is invalid");
            break;
        }

        return monthString;
    }

    public static String allMediaTimeDividerType2String(Context context, int type) {
        if (type < 0) {
            NmsLog.error("allMediaTimeDividerType2String", "type is UNKNOWN");
            return "";
        }

        Time now = new Time();
        now.set(System.currentTimeMillis());
        int nowYear = now.year;
        int nowMonth = now.month + 1;
        int nowWeek = now.getWeekNumber();

        int thenYear = type / 10000;
        int thenMonth = (type % 10000) / 100;
        int thenWeek = (type % 10000) % 100;

        String ret = "";
        if (nowYear == thenYear && nowMonth == thenMonth && nowWeek == thenWeek) {
            ret = context.getString(R.string.STR_NMS_ALL_MEDIA_TIME_THIS_WEEK);
        } else if (nowYear == thenYear && nowMonth == thenMonth) {
            ret = context.getString(R.string.STR_NMS_ALL_MEDIA_TIME_THIS_MONTH);
        } else {
            ret = thenYear + " " + monthNumber2String(context, thenMonth);
        }

        return ret;
    }

    public static String getTimeDividerString(Context context, long when) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT | DateUtils.FORMAT_ABBREV_ALL
                | DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
            Date curDate = new Date();
            Date cur = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate(), 0, 0, 0);
            long oneDay = 24 * 60 * 60 * 1000;
            long elapsedTime = cur.getTime() - when;
            if (elapsedTime < oneDay && elapsedTime > 0) {
                return context.getResources().getString(R.string.STR_NMS_YESTERDAY);
            }
        } else {
            return context.getString(R.string.STR_NMS_TODAY);
        }
        return NmsDateTool.getDispTimeStr(when);
    }

    public static int getVideoCaptureDurationLimit() {
        CamcorderProfile camcorder = CamcorderProfile.get(CamcorderProfile.QUALITY_LOW);
        return camcorder == null ? 0 : camcorder.duration;
    }

    public static String encodeDurationFilename(String oriFileName, int duration) {

        if (oriFileName == null) {
            NmsLog.error(TAG, "fatal error for encodeDurationFilename that oriFileName is null");
            return "";
        }

        String newName = null;
        String attachName = oriFileName.substring(oriFileName.lastIndexOf("/") + 1);

        if (attachName == null) {
            NmsLog.error(TAG, "fatal error for encodeDurationFilename that attachName is null");
            return "";
        }

        attachName = duration + "_" + attachName;
        newName = oriFileName.substring(0, oriFileName.lastIndexOf("/")) + File.separator
                + attachName;

        return newName;
    }

    public static int decodeDurationFilename(String oriFileName) {

        if (oriFileName == null) {
            NmsLog.error(TAG, "fatal error for decodeDurationFilename that oriFileName is null");
            return 0;
        }

        String attachName = oriFileName.substring(oriFileName.lastIndexOf("/") + 1);

        if (attachName == null) {
            NmsLog.error(TAG, "fatal error for decodeDurationFilename that attachName is null");
            return 0;
        }

        int index = attachName.indexOf("_");

        if (index < 0) {
            NmsLog.error(TAG, "fatal error for decodeDurationFilename can not find the sep");
            return 0;
        }

        String durationStr = attachName.substring(0, index);

        return Integer.valueOf(durationStr);
    }

    public static boolean isActivateSimCard(Context context) {
        for (int i = 0; i < 2; i++) {
            long simId = NmsPlatformAdapter.getInstance(context).getSimIdBySlotId(i);
            SNmsSimInfo simInfo = NmsIpMessageApiNative.nmsGetSimInfoViaSimId((int) simId);
            if (simInfo != null) {
                if (simInfo.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                    return true;
                }
            }
        }

        return false;
    }

    public static boolean isFileStatusOk(Context context, String path) {
        if (TextUtils.isEmpty(path)) {
            Toast.makeText(context, R.string.STR_NMS_NO_SUCH_FILE, Toast.LENGTH_SHORT).show();
            return false;
        }
        if (!NmsCommonUtils.isExistsFile(path)) {
            Toast.makeText(context, R.string.STR_NMS_NO_SUCH_FILE, Toast.LENGTH_SHORT).show();
            return false;
        }
        if (NmsCommonUtils.getFileSize(path) > NmsCustomUIConfig.MAX_ATTACH_SIZE) {
            Toast.makeText(context, R.string.STR_NMS_OVER_FILE_LIMIT, Toast.LENGTH_SHORT).show();
            return false;
        }
        return true;
    }

    public static boolean isPic(String name) {
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        String path = name.toLowerCase();
        if (path.endsWith(".png") || path.endsWith(".jpg") || path.endsWith(".jpeg")
                || path.endsWith(".bmp") || path.endsWith(".gif")) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isVideo(String name) {
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        String path = name.toLowerCase();
        if (path.endsWith(".mp4") || path.endsWith(".3gp")) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isAudio(String name) {
        if (TextUtils.isEmpty(name)) {
            return false;
        }
        String extArrayString[] = { ".amr", ".ogg", ".mp3", ".aac", ".ape", ".flac", ".wma",
                ".wav", ".mp2", ".mid", ".3gpp" };
        String path = name.toLowerCase();
        for (String ext : extArrayString) {
            if (path.endsWith(ext))
                return true;
        }

        return false;
    }
}
