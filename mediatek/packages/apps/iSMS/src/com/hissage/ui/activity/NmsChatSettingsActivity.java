package com.hissage.ui.activity;

import java.io.File;
import java.io.IOException;
import java.util.Date;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.view.MenuItem;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.db.NmsContentResolver;
import com.hissage.db.NmsDBUtils;
import com.hissage.jni.engineadapter;
import com.hissage.message.NmsDownloadMessageHistory;
import com.hissage.message.ip.NmsIpMessageConsts.NmsSaveHistory;
import com.hissage.ui.view.NmsWallpaperChooser;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsChatSettingsActivity extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {

    private static final String TAG = "chat_settings";

    private static final String KEY_WALLPAPER = "wallpaper";
    private static final String KEY_NOTIFICATION = "notification";
    private static final String KEY_MUTE = "mute";
    private static final String KEY_RINGTONE = "ringtone";
    private static final String KEY_VIBRATE = "vibrate";
    private static final String KEY_DOWNLOAD = "download";
    private static final String KEY_EMAIL = "email";
    private static final String KEY_CLEAR = "clear";

    private static final int PICK_RINGTONE = 1;
    private static final int PICK_WALLPAPER = 2;
    private static final int PICK_GALLERY = 3;
    private static final int PICK_PHOTO = 4;

    private NmsChatSettings mSettings = null;

    private Preference mWallpaper = null;
    private CheckBoxPreference mNotification = null;
    private ListPreference mMute = null;
    private Preference mRingtone = null;
    private CheckBoxPreference mVibrate = null;
    private Preference mDownload = null;
    private Preference mEmail = null;
    private Preference mClear = null;

    private static boolean mNeedEmail = false;
    private ProgressDialog mDialog = null;
    private ProgressDialog mWallpaperDlg = null;

    private String wallpaperCache = null;
    private String[] mMuteValues = null;

    private static final int SAVE_WALLPAPER_DONE = 1024;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message message) {
            switch (message.what) {
            case SAVE_WALLPAPER_DONE:
                if (mWallpaperDlg != null) {
                    mWallpaperDlg.dismiss();
                    mWallpaperDlg = null;
                }
                Toast.makeText(NmsChatSettingsActivity.this, R.string.STR_NMS_SAVE_WALLPAPER_OK,
                        Toast.LENGTH_SHORT).show();
                return;
            default:
                // do nothing
                break;
            }
        }
    };
    private BroadcastReceiver downloadHistoryRecver = null;

    private int getMuteIndex(int mute) {
        int ret = 0;
        switch (mute) {
        case 0:
            ret = 0;
            break;
        case 1:
            ret = 1;
            break;
        case 4:
            ret = 2;
            break;
        case 8:
            ret = 3;
            break;
        default:
            // do nothing
        }
        return ret;
    }

    private void updateStatus() {
        mNotification.setChecked(mSettings.mNotification == 1);

        if (TextUtils.isEmpty(mSettings.mRingtone)) {
            Uri uri = RingtoneManager.getActualDefaultRingtoneUri(getApplicationContext(),
                    RingtoneManager.TYPE_NOTIFICATION);
            if (null != uri) {
                mSettings.mRingtone = uri.toString();
            } else {
                mSettings.mRingtone = "";
            }

        }

        Ringtone tone = RingtoneManager.getRingtone(getApplicationContext(),
                Uri.parse(mSettings.mRingtone));
        if (null != tone) {
            String title = tone.getTitle(getApplicationContext());
            if (!TextUtils.isEmpty(title)) {
                mRingtone.setSummary(title);
            }
        }

        mVibrate.setChecked(mSettings.mVibrate == 1);

        if (mSettings.mMute_start > 0) {
            int currentTime = (int) (System.currentTimeMillis() / 1000);
            if ((mSettings.mMute * 3600 + mSettings.mMute_start) <= currentTime) {
                mSettings.mMute = 0;
                mSettings.mMute_start = 0;
            }
        }

        if (mSettings.mNotification == 0) {
            mMute.setEnabled(false);
            mRingtone.setEnabled(false);
            mVibrate.setEnabled(false);
        } else {
            mMute.setEnabled(true);
            mRingtone.setEnabled(true);
            mVibrate.setEnabled(true);
        }

        String muteValue = mMuteValues[getMuteIndex(mSettings.mMute)];
        mMute.setSummary(muteValue);
        mMute.setValue(muteValue);

        NmsDBUtils.getDataBaseInstance(this).nmsSetChatSettings(mSettings);

    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        switch (id) {
        case android.R.id.home: {
            finish();
            break;
        }
        }
        return super.onMenuItemSelected(featureId, item);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
        // getActionBar().setLogo(R.drawable.isms);
        Intent i = getIntent();
        if (null == i) {
            finish();
            return;
        }
        short contactId = (short) i.getShortExtra(NmsChatSettings.CHAT_SETTINGS_KEY, (short) 0);
        if (contactId <= 0) {
            NmsLog.error(TAG, "can not get contact id, so finish.");
            finish();
            return;
        }

        mSettings = new NmsChatSettings(this, contactId);
        if (mSettings.mContactId <= 0) {
            mSettings.mContext = this;
            mSettings.mContactId = contactId;
        }

        addPreferencesFromResource(R.xml.chat_settings);

        mWallpaper = findPreference(KEY_WALLPAPER);
        mNotification = (CheckBoxPreference) findPreference(KEY_NOTIFICATION);
        mMute = (ListPreference) findPreference(KEY_MUTE);
        mMute.setOnPreferenceChangeListener(this);
        mRingtone = findPreference(KEY_RINGTONE);
        mVibrate = (CheckBoxPreference) findPreference(KEY_VIBRATE);
        mDownload = findPreference(KEY_DOWNLOAD);
        mEmail = findPreference(KEY_EMAIL);
        mClear = findPreference(KEY_CLEAR);
        mMuteValues = getResources().getStringArray(R.array.chat_settings_mute_list);

    }

    @Override
    public void onResume() {
        super.onResume();
        updateStatus();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        if (downloadHistoryRecver != null) {
            unregisterReceiver(downloadHistoryRecver);
        }
        NmsDBUtils.getDataBaseInstance(this).nmsSetChatSettings(mSettings);
        super.onDestroy();
    }

    private void pickRingtoneResult(Intent data) {
        Uri uri = data.getParcelableExtra(android.media.RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
        if (uri != null) {
            mSettings.mRingtone = uri.toString();
        }
        updateStatus();

    }

    private void saveFile2Wallpaper(String FilePath) {

        String wallpaperPath = NmsCommonUtils.getSDCardPath(this) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "wallpaper" + File.separator;
        wallpaperPath += mSettings.mContactId;

        if (!NmsCommonUtils.isExistsFile(FilePath) || NmsCommonUtils.getFileSize(FilePath) <= 0) {
            NmsLog.error(TAG, "wallpaper file: " + FilePath
                    + "is not exsit or filepath is 0, save it error.");
            return;
        }
        File file = new File(wallpaperPath);
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdirs();
        }
        file.delete();
        try {
            if (!file.createNewFile()) {
                return;
            }
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            return;
        }
        NmsCommonUtils.copy(FilePath, wallpaperPath);
        mSettings.mWallPaper = Uri.fromFile(file).toString();
        updateStatus();
    }

    private void pickWallpaperFromGalleryResult(Intent data) {
        if (null == data) {
            NmsLog.error(TAG, "pick wallpaper from gallery result data is null");
        }
        Uri uri = data.getData();
        ContentResolver cr = this.getContentResolver();
        final String[] selectColumn = { "_data" };
        Cursor cursor = null;
        String FilePath = null;
        try {
            cursor = NmsContentResolver.query(cr, uri, selectColumn, null, null, null);
            if (null == cursor) {
                NmsLog.error(TAG, "pick wallpaper from gallery result cursor is null");
                return;
            }
            if (0 == cursor.getCount()) {
                cursor.close();
                NmsLog.error(TAG, "pick wallpaper from gallery result cursor getcount is 0");
                return;
            }
            cursor.moveToFirst();
            FilePath = cursor.getString(0);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != cursor && !cursor.isClosed()) {
                cursor.close();
            }
        }
        saveFile2Wallpaper(FilePath);
        mHandler.sendEmptyMessage(SAVE_WALLPAPER_DONE);
        return;
    }

    private void pickWallpaperFromCamResult(Intent data) {
        if (!NmsCommonUtils.isExistsFile(wallpaperCache)
                || NmsCommonUtils.getFileSize(wallpaperCache) == 0) {
            return;
        }

        showSaveWallPaperDlg();

        new Thread() {
            public void run() {

                byte[] content = NmsBitmapUtils.resizeImgByMaxLength(wallpaperCache, (float) 1000f);

                try {
                    NmsCommonUtils.nmsStream2File(content, wallpaperCache);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTrace(e);
                }

                saveFile2Wallpaper(wallpaperCache);
                mHandler.sendEmptyMessage(SAVE_WALLPAPER_DONE);
            }
        }.start();
    }

    private void showSaveWallPaperDlg() {

        if (mWallpaperDlg != null) {
            return;
        }
        mWallpaperDlg = new ProgressDialog(this);
        mWallpaperDlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mWallpaperDlg.setMessage(getString(R.string.STR_NMS_WAIT_SAVE_WALLPAPER));
        mWallpaperDlg.setIndeterminate(false);
        mWallpaperDlg.setCancelable(false);
        mWallpaperDlg.show();
    }

    private void pickWallpaperFromSys(Intent data) {
        final int sourceId = data.getIntExtra("wallpaper_index", -1);

        if (sourceId < 0) {
            return;
        }

        showSaveWallPaperDlg();

        new Thread() {
            public void run() {
                Bitmap b = BitmapFactory.decodeResource(getResources(), sourceId);

                try {
                    wallpaperCache = NmsCommonUtils.getCachePath(NmsChatSettingsActivity.this)
                            + mSettings.mContactId + ".png";
                    NmsCommonUtils.nmsStream2File(NmsCommonUtils.bitmap2ByteArray(b),
                            wallpaperCache);
                    saveFile2Wallpaper(wallpaperCache);
                } catch (Exception e) {
                    NmsLog.nmsPrintStackTrace(e);
                }
                if (b != null) {
                    b.recycle();
                }
                mHandler.sendEmptyMessage(SAVE_WALLPAPER_DONE);
            }
        }.start();

    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK) {
            NmsLog.error(TAG, "onActivityResult return not ok: " + resultCode);
            return;
        }

        switch (requestCode) {
        case PICK_RINGTONE:
            pickRingtoneResult(data);
            break;

        case PICK_GALLERY:
            pickWallpaperFromGalleryResult(data);
            break;

        case PICK_PHOTO:
            pickWallpaperFromCamResult(data);
            break;

        case PICK_WALLPAPER:
            pickWallpaperFromSys(data);
            break;

        default:
            NmsLog.error(TAG, "not handle onActivityResult code: " + requestCode);
        }
    }

    private void pickRingtone() {

        Intent intent = new Intent(android.media.RingtoneManager.ACTION_RINGTONE_PICKER);
        intent.putExtra(android.media.RingtoneManager.EXTRA_RINGTONE_TYPE,
                android.media.RingtoneManager.TYPE_NOTIFICATION);
        intent.putExtra(android.media.RingtoneManager.EXTRA_RINGTONE_TITLE,
                getString(R.string.STR_NMS_RINGTONE_TITLE));
        intent.putExtra(android.media.RingtoneManager.EXTRA_RINGTONE_EXISTING_URI,
                Uri.parse(mSettings.mRingtone));
        intent.putExtra(android.media.RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
        startActivityForResult(intent, PICK_RINGTONE);
    }

    private void pickSysWallpaper() {
        Intent intent = new Intent(this, NmsWallpaperChooser.class);
        startActivityForResult(intent, PICK_WALLPAPER);
    }

    private void pickWallpaperFromCam() {
        wallpaperCache = NmsCommonUtils.getCachePath(this) + mSettings.mContactId + ".png";
        File out = new File(wallpaperCache);
        if (!out.getParentFile().exists()) {
            out.getParentFile().mkdirs();
        }
        Uri uri = Uri.fromFile(out);
        Intent imageCaptureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        imageCaptureIntent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
        imageCaptureIntent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 1);
        try {
            startActivityForResult(imageCaptureIntent, PICK_PHOTO);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private void pickWallpaperFromGallery() {

        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType("image/*");
        try {
            startActivityForResult(
                    Intent.createChooser(intent,
                            this.getResources().getText(R.string.STR_NMS_SYS_GALLERY)),
                    PICK_GALLERY);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private void pickChatWallpaper() {
        new AlertDialog.Builder(this).setTitle(R.string.STR_NMS_CHAT_WALLPAPER)
                .setItems(R.array.chat_settings_wallpaper, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        if (which == 0) {
                            pickSysWallpaper();
                        } else if (which == 1) {
                            pickWallpaperFromCam();
                        } else if (which == 2) {
                            pickWallpaperFromGallery();
                        } else {
                            mSettings.mWallPaper = null;
                            updateStatus();
                            mHandler.sendEmptyMessage(SAVE_WALLPAPER_DONE);
                        }
                    }
                }).create().show();

    }

    private class NmsDownloadHistoryRecver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                int progress = intent.getIntExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_PROGRESS, -1);
                if (progress > 0) {
                    mDialog.setMessage(getString(R.string.STR_NMS_DOWNLOAD_HISTORY_DLG) + progress
                            + "%");
                }

                int done = intent.getIntExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_DONE, 1);

                if (done != 1) {
                    mDialog.dismiss();
                }

                if (done == NmsSaveHistory.NMS_OK) {
                    String filePath = intent
                            .getStringExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_FILE);
                    if (mNeedEmail) {
                        Intent share = new Intent(android.content.Intent.ACTION_SEND);
                        share.setType("message/rfc822");
                        share.putExtra(Intent.EXTRA_STREAM, Uri.parse("file://" + filePath));
                        startActivity(share);
                    } else {
                        Toast.makeText(NmsChatSettingsActivity.this,
                                String.format(getString(R.string.STR_NMS_DOWNLOAD_OK), filePath),
                                Toast.LENGTH_LONG).show();
                    }
                } else if (done == NmsSaveHistory.NMS_ERROR) {
                    String filePath = intent
                            .getStringExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_FILE);
                    NmsCommonUtils.deletefile(filePath);
                    Toast.makeText(NmsChatSettingsActivity.this, R.string.STR_NMS_DOWNLOAD_ERROR,
                            Toast.LENGTH_LONG).show();
                } else if (done == NmsSaveHistory.NMS_EMPTY) {
                    String filePath = intent
                            .getStringExtra(NmsSaveHistory.NMS_DOWNLOAD_HISTORY_FILE);
                    NmsCommonUtils.deletefile(filePath);
                    Toast.makeText(NmsChatSettingsActivity.this, R.string.STR_NMS_DOWNLOAD_EMPTY,
                            Toast.LENGTH_LONG).show();
                } else {
                    NmsLog.error(TAG, "not handle download history status:" + done);
                }
                if (done != 1) {
                    mNeedEmail = false;
                }
            }
        }
    }

    protected boolean isSDCardReady() {
        String sdStatus = Environment.getExternalStorageState();

        if (TextUtils.isEmpty(sdStatus)) {
            return false;
        }

        return sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);
    }

    private void downloadChatHistroy() {
        if (!isSDCardReady()) {
            Toast.makeText(NmsChatSettingsActivity.this, R.string.STR_NMS_NO_SDCARD,
                    Toast.LENGTH_LONG).show();
            mNeedEmail = false;
            return;
        }
        
        long availableSpace = NmsCommonUtils.getSDcardAvailableSpace();
        if (availableSpace <= 5 * 1024 * 1024) {
//            new AlertDialog.Builder(NmsChatSettingsActivity.this).setTitle(R.string.STR_NMS_TIPTITLE)
//                    .setMessage(R.string.STR_NMS_SDSPACE_NOT_ENOUGH)
//                    .setPositiveButton(R.string.STR_NMS_OK, null).create().show();
        	Toast.makeText(NmsChatSettingsActivity.this, R.string.STR_NMS_SDSPACE_NOT_ENOUGH, Toast.LENGTH_LONG).show();
            mNeedEmail = false;
            return;
        }
        
        String zipFile = NmsCommonUtils.getSDCardPath(this) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY + File.separator;
        zipFile += mSettings.mContactId + "_" + NmsConsts.SDF1.format(new Date()) + ".zip";

        mDialog = new ProgressDialog(this);
        mDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mDialog.setMessage(getString(R.string.STR_NMS_DOWNLOAD_HISTORY_DLG));
        mDialog.setIndeterminate(false);
        mDialog.setCancelable(false);
        mDialog.show();

        if (null == downloadHistoryRecver) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(NmsSaveHistory.NMS_ACTION_DOWNLOAD_HISTORY);
            downloadHistoryRecver = new NmsDownloadHistoryRecver();
            registerReceiver(downloadHistoryRecver, filter);
        }

        NmsDownloadMessageHistory
                .dumpContactMessages(new short[] { mSettings.mContactId }, zipFile);
    }

    private void clearChatHistory() {
        new AlertDialog.Builder(this).setIcon(R.drawable.ic_dialog_alert_holo_light)
                .setTitle(R.string.STR_NMS_SKETCH_DELETE)
                .setMessage(R.string.STR_NMS_CLEAR_HISTORY_CONTENT)
                .setPositiveButton(R.string.STR_NMS_DELETE, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        clearHistory();
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).create().show();
    }

    private void clearHistory() {

        final ProgressDialog dlg = new ProgressDialog(this);
        dlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dlg.setMessage(getString(R.string.STR_NMS_CLEARING));
        dlg.setIndeterminate(false);
        dlg.setCancelable(false);
        dlg.show();

        new Thread() {
            @Override
            public void run() {
                long current = System.currentTimeMillis();
                engineadapter.get().nmsUIDeleteMsgViaContactRecId(mSettings.mContactId, 1, 0, 1);
                long end = System.currentTimeMillis();
                if (end - current < 2000) {
                    try {
                        sleep(2000 + current - end);
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        NmsLog.nmsPrintStackTrace(e);
                    }
                }
                dlg.dismiss();
            }
        }.start();

    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {

        if (preference == mWallpaper) {
            pickChatWallpaper();
        } else if (preference == mRingtone) {
            pickRingtone();
        } else if (preference == mNotification) {
            mSettings.mNotification = (mSettings.mNotification == 1 ? 0 : 1);
            updateStatus();
        } else if (preference == mVibrate) {
            mSettings.mVibrate = (mSettings.mVibrate == 0 ? 1 : 0);
            updateStatus();
        } else if (preference == mDownload) {
            new AlertDialog.Builder(NmsChatSettingsActivity.this).setTitle(R.string.STR_NMS_SVAE_TITLE)
            .setIcon(R.drawable.ic_dialog_alert_holo_light)
            .setMessage(R.string.STR_NMS_SAVE_CONTENT)
            .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    downloadChatHistroy();
                    dialog.cancel();
                }
            })
            .setNegativeButton(R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.cancel();
                }
            }).create().show();

        } else if (preference == mEmail) {
            new AlertDialog.Builder(NmsChatSettingsActivity.this).setTitle(R.string.STR_NMS_EMAIL_TITLE)
            .setIcon(R.drawable.ic_dialog_alert_holo_light)
            .setMessage(R.string.STR_NMS_EMAIL_CONTENT)
            .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    mNeedEmail = true;
                    downloadChatHistroy();
                    dialog.cancel();
                }
            })
            .setNegativeButton(R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.cancel();
                }
            }).create().show();
        } else if (preference == mClear) {
            clearChatHistory();
        } else {
            // do noting
        }

        return true;
    }

    private void updateMuteValue(String val) {
        if (mMuteValues[0].equals(val)) {
            mSettings.mMute = 0;
        } else if (mMuteValues[1].equals(val)) {
            mSettings.mMute = 1;
        } else if (mMuteValues[2].equals(val)) {
            mSettings.mMute = 4;
        } else if (mMuteValues[3].equals(val)) {
            mSettings.mMute = 8;
        } else {
            // do nothing
        }

        if (mSettings.mMute > 0) {
            mSettings.mMute_start = (int) (System.currentTimeMillis() / 1000);
        } else {
            mSettings.mMute_start = 0;
        }
    }

    public boolean onPreferenceChange(Preference preference, Object objValue) {

        if (preference == mMute) {
            mMute.setSummary(objValue.toString());
            updateMuteValue(objValue.toString());
            updateStatus();
        }
        return true;
    }
}
