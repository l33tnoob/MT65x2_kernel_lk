package com.hissage.ui.activity;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.R.integer;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.db.NmsContentResolver;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.ui.view.NmsSharePanel;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageConsts;
import com.hissage.util.message.MessageUtils;
import com.hissage.util.message.SmileyParser;

public class NmsShareBaseActivity extends NmsBaseActivity {

    public final static int TAKE_PHOTO = 1000;
    public final static int RECORD_VIDEO = 1001;
    public final static int RECORD_AUDIO = 1002;
    public final static int DRAW_SKETCH = 1003;
    public final static int CHOOSE_PHOTO = 1004;
    public final static int CHOOSE_VIDEO = 1005;
    public final static int CHOOSE_AUDIO = 1006;
    public final static int SHARE_LOCATION = 1007;
    public final static int SHARE_CONTACT = 1008;
    public final static int SHARE_CALENDAR = 1009;
    public final static int GET_FORWARD_CONTACT = 1010;
    public final static int TAKE_READED_BURN_PHOTO = 1011;
    public final static int CHOOSE_READED_BURN_PHOTO = 1012;

    public String mPhotoFilePath = "";
    public String mPicTempPath = "";
    public String mAudioTempPath = "";
    public String mVideoTempPath = "";
    public String mVcardTempPath = "";
    public String mCalendarTempPath = "";
    public String mDstPath = "";
    public int duration = 0;
    public String calendarSummary = "";

    private final static String TAG = "BaseActivity";
    public final static String NMS_SKETCH_PATH = "sketch_path";
    
    protected int scrollListInteverl = 1000 ;
    private boolean isDoingScrollList = false ;
    
    public Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            NmsLog.trace(TAG, "handler msg type is: " + msg.what);
            super.handleMessage(msg);
            switch (msg.what) {
            case MessageConsts.NMS_SHARE:
                doMoreAction(msg);
                break;
            case MessageConsts.NMS_LOAD_ALL_MESSAGE:
                loadAllMessage();
                break;
            case MessageConsts.NMS_REFRESH_AND_SCROLL_LIST:
                doScrollList(true) ;
                break;
            case MessageConsts.NMS_SCROLL_LIST:
                doScrollList(false) ;
                break ;
            case MessageConsts.NMS_SEND_MSG_FAILED:
                showSendMsgFailedError() ;
                break ;
            default:
                NmsLog.error(TAG, "msg type: " + msg.what + "not handler");
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initialize();
        SmileyParser.init(this);
    }

    public void loadAllMessage() {

    }

    public void scrollList(boolean isRefreshList) {
    }
    
    private final void doScrollList(boolean isRefreshList) {
        if (scrollListInteverl <= 0 || !isRefreshList) {
            scrollList(isRefreshList);
            return ;
        }
        
        if (isDoingScrollList) 
            return ;
        
        final boolean argRefreshList = isRefreshList ; 
        isDoingScrollList = true ;
        mHandler.postDelayed(new Runnable() {
            public void run() {
                if (!isDoingScrollList) 
                    return ;
                
                try {
                    scrollList(argRefreshList) ;
                    isDoingScrollList = false ;
                } catch (Exception e) {
                    NmsLog.error(TAG, "got execption in doScrollList: " + e.toString()) ;
                    isDoingScrollList = false ;
                }
            }
        }, scrollListInteverl) ;
    }
    
    private void initialize() {
        mPicTempPath = NmsCommonUtils.getPicCachePath(this);

        mAudioTempPath = NmsCommonUtils.getAudioCachePath(this);

        mVideoTempPath = NmsCommonUtils.getVideoCachePath(this);

        mVcardTempPath = NmsCommonUtils.getVcardCachePath(this);

        mCalendarTempPath = NmsCommonUtils.getVcalendarCachePath(this);

        NmsCommonUtils.getCachePath(this);
    }
    
    private void showSendMsgFailedError() {
        Toast.makeText(this, R.string.STR_NMS_SEND_MSG_FAILED, Toast.LENGTH_SHORT).show();
    }

    private void doMoreAction(Message msg) {
        Bundle bundle = msg.getData();
        int action = bundle.getInt(NmsSharePanel.SHARE_ACTION);

        if (!NmsCommonUtils.getSDCardStatus()) {
            MessageUtils.createLoseSDCardNotice(NmsShareBaseActivity.this,
                    R.string.STR_NMS_CANT_SHARE);
            return;
        }

        switch (action) {
        case NmsSharePanel.takePhoto:
            takePhoto(TAKE_PHOTO);
            break;

        case NmsSharePanel.recordVideo:
            recordVideo();
            break;

        case NmsSharePanel.drawSketch:
            drawSketch();
            break;

        case NmsSharePanel.shareContact:
            shareContact();
            break;

        case NmsSharePanel.choosePhoto:
            choosePhoto(CHOOSE_PHOTO);
            break;

        case NmsSharePanel.chooseVideo:
            chooseVideo();
            break;

        case NmsSharePanel.recordAudio:
            recordAudio();
            break;

        case NmsSharePanel.shareLocation:
            shareLocation();
            break;

        case NmsSharePanel.chooseAudio:
            chooseAudio();
            break;
        case NmsSharePanel.shareCalendar:
            shareCalendar();
            break;
        case NmsSharePanel.shareReadedBurn:
            if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                chooseReadedBurnPhoto();
            }
            break;
        default:
            NmsLog.error(TAG, "invalid share action type: " + action);
            break;
        }
    }

    public void chooseReadedBurnPhoto() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(this);
        alertBuilder.setTitle(R.string.STR_NMS_SELECT_ACTION_TITLE);
        String[] items = new String[2];
        items[0] = getString(R.string.STR_NMS_TAKE_PHOTO);
        items[1] = getString(R.string.STR_NMS_CHOOSE_PHOTO);
        alertBuilder.setItems(items, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                case 0:
                    takePhoto(TAKE_READED_BURN_PHOTO);
                    break;
                case 1:
                    choosePhoto(CHOOSE_READED_BURN_PHOTO);
                    break;
                default:
                    break;
                }
            }
        });
        alertBuilder.create().show();
    }
    public void chooseAudio() {
        AlertDialog.Builder alertBuilder = new AlertDialog.Builder(this);
        alertBuilder.setTitle(R.string.STR_NMS_CHOOSE_AUDIO_TITLE);
        String[] items = new String[2];
        items[0] = getString(R.string.STR_NMS_CHOOSE_RINGTONE_ITEM);
        items[1] = getString(R.string.STR_NMS_CHOOSE_AUDIO_ITEM);
        alertBuilder.setItems(items, new OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                case 0:
                    MessageUtils.selectRingtone(NmsShareBaseActivity.this, CHOOSE_AUDIO);
                    break;
                case 1:
                    if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                        Toast.makeText(NmsShareBaseActivity.this, R.string.STR_NMS_INSERT_SDCARD,
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    MessageUtils.selectAudio(NmsShareBaseActivity.this, CHOOSE_AUDIO);
                    break;
                }
            }
        });
        alertBuilder.create().show();
    }

    public void getCalendarFromMtk(Context context, String calendar) {

        Uri calendarUri = Uri.parse(calendar);
        InputStream is = null;
        OutputStream os = null;
        ContentResolver cr = this.getContentResolver();
        Cursor cursor = null;

        cursor = cr.query(calendarUri, null, null, null, null);
        if (0 == cursor.getCount()) {
            NmsLog.error(TAG, "take calendar cursor getcount is 0");
        }
        cursor.moveToFirst();
        calendarSummary = cursor.getString(0);
        if (calendarSummary != null) {
            int sub = calendarSummary.lastIndexOf(".");
            calendarSummary = calendarSummary.substring(0, sub);
        }

        if (cursor != null) {
            cursor.close();
        }

        String fileName = System.currentTimeMillis() + ".vcs";
        mDstPath = mCalendarTempPath + File.separator + fileName;

        File file = new File(mDstPath);
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
        try {
            is = context.getContentResolver().openInputStream(calendarUri);
            os = new BufferedOutputStream(new FileOutputStream(file));
        } catch (FileNotFoundException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        byte[] buffer = new byte[256];
        try {
            for (int len = 0; (len = is.read(buffer)) != -1;) {
                os.write(buffer, 0, len);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public void shareCalendar() {
        Intent intent = new Intent("android.intent.action.CALENDARCHOICE");
        intent.setType("text/x-vcalendar");
        intent.putExtra("request_type", 0);
        try {
            startActivityForResult(intent, SHARE_CALENDAR);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public void recordAudio() {
        Intent intent = new Intent(this, NmsAudioActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.putExtra(NmsAudioActivity.MAX_AUDIO_DURATION, NmsCustomUIConfig.AUDIO_MAX_DURATION);
        intent.putExtra(NmsAudioActivity.MAX_FILE_SIZE_KEY, NmsCustomUIConfig.MAX_FILE_SIZE_KEY);
        intent.setType("audio/amr");
        startActivityForResult(intent, RECORD_AUDIO);
    }

    public void drawSketch() {
        Intent i = new Intent(this, NmsSketchActivity.class);
        String fileName = System.currentTimeMillis() + ".ske.png";
        mPhotoFilePath = mPicTempPath + File.separator + fileName;
        mDstPath = mPhotoFilePath;
        i.putExtra(NMS_SKETCH_PATH, mPhotoFilePath);
        startActivityForResult(i, DRAW_SKETCH);
    }

    public void recordVideo() {
        Intent intent = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 0);
        intent.putExtra(MediaStore.EXTRA_SIZE_LIMIT, NmsCustomUIConfig.VIDEO_MAX_SIZE);
        // intent.putExtra(MediaStore.EXTRA_DURATION_LIMIT,
        // NmsCustomUIConfig.VIDEO_MAX_DURATION);
        String fileName = System.currentTimeMillis() + ".3gp";
        mDstPath = mVideoTempPath + File.separator + fileName;
        File f = new File(mDstPath);
        Uri uri = Uri.fromFile(f);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, uri);

        try {
            startActivityForResult(intent, RECORD_VIDEO);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public void choosePhoto(int requestCode) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_PICK);
        intent.setType("image/*");
        String fileName = String.valueOf(System.currentTimeMillis());
        mPhotoFilePath = mPicTempPath + File.separator + fileName;
        mDstPath = mPhotoFilePath;
        try {
            startActivityForResult(intent, requestCode);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public void takePhoto(int requestCode) {
        Intent imageCaptureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        String fileName = System.currentTimeMillis() + ".jpg";
        mPhotoFilePath = mPicTempPath + File.separator + fileName;
        mDstPath = mPhotoFilePath;
        File out = new File(mPhotoFilePath);
        Uri uri = Uri.fromFile(out);
        imageCaptureIntent.putExtra(MediaStore.EXTRA_OUTPUT, uri);
        imageCaptureIntent.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, 1);
        try {
            startActivityForResult(imageCaptureIntent, requestCode);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public void shareLocation() {
        NmsStartActivityApi.nmsStartLocationActivityForResult(this, SHARE_LOCATION);
    }

    public void shareContact() {
        if (NmsSMSMMSManager.getInstance(this).isExtentionFieldExsit() == 1) {
            // integration
            try {
                Intent intent = new Intent("android.intent.action.contacts.list.PICKMULTICONTACTS");
                intent.setType(Phone.CONTENT_TYPE);
                startActivityForResult(intent, SHARE_CONTACT);
            } catch (Exception e) {
                Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
                NmsLog.error(TAG, "Intent pick system contact failed");
            }
        } else {
            // standalone
            try {
                Intent intent = new Intent(Intent.ACTION_PICK,
                        ContactsContract.Contacts.CONTENT_URI);
                startActivityForResult(intent, SHARE_CONTACT);
            } catch (Exception e) {
                Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
                NmsLog.error(TAG, "Intent pick system contact failed");
            }
        }
    }

    public void chooseVideo() {
        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setType("video/*");
        try {
            startActivityForResult(intent, CHOOSE_VIDEO);
        } catch (Exception e) {
            Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    public boolean getVideoOrPhoto(Intent data, int requestCode) {
        boolean ret = false;
        if (null == data) {
            NmsLog.error(TAG, "take video error, result intent is null.");
            return ret;
        }

        Uri uri = data.getData();
        ContentResolver cr = this.getContentResolver();
        Cursor cursor = null;
        try {
            if (requestCode == TAKE_PHOTO || requestCode == CHOOSE_PHOTO
                    ||requestCode==CHOOSE_READED_BURN_PHOTO||requestCode==TAKE_READED_BURN_PHOTO) {
                final String[] selectColumn = { "_data" };
                cursor = NmsContentResolver.query(cr, uri, selectColumn, null, null, null);
            } else {
                final String[] selectColumn = { "_data", "duration" };
                cursor = NmsContentResolver.query(cr, uri, selectColumn, null, null, null);
            }
            if (null == cursor) {
                if (requestCode == RECORD_AUDIO) {
                    mDstPath = uri.getEncodedPath();
                    duration = data.getIntExtra("audio_duration", 0);
                    duration = duration / 1000 == 0 ? 1 : duration / 1000;
                } else {
                    mPhotoFilePath = uri.getEncodedPath();
                    mDstPath = mPhotoFilePath;
                    duration = 1;
                }
            } else {
                if (0 == cursor.getCount()) {
                    NmsLog.error(TAG, "take video cursor getcount is 0");
                    ret = false;
                }
                cursor.moveToFirst();
                if (requestCode == TAKE_PHOTO || requestCode == CHOOSE_PHOTO
                        ||requestCode==CHOOSE_READED_BURN_PHOTO||requestCode==TAKE_READED_BURN_PHOTO) {
                    mPhotoFilePath = cursor.getString(cursor.getColumnIndex("_data"));
                    if (requestCode == CHOOSE_PHOTO||requestCode==CHOOSE_READED_BURN_PHOTO) {
                        if (!TextUtils.isEmpty(mPhotoFilePath) && mPhotoFilePath.contains(".")) {
                            int index = mPhotoFilePath.lastIndexOf(".");
                            String suffix = mPhotoFilePath.substring(index);
                            mDstPath += suffix;
                        }
                        NmsCommonUtils.copy(mPhotoFilePath, mDstPath);
                    } else {
                        mDstPath = mPhotoFilePath;
                    }
                } else {
                    mDstPath = cursor.getString(cursor.getColumnIndex("_data"));
                    duration = cursor.getInt(cursor.getColumnIndex("duration"));
                    duration = duration / 1000 == 0 ? 1 : duration / 1000;
                }
            }

            if (requestCode == TAKE_PHOTO || requestCode == CHOOSE_PHOTO
                    || requestCode == DRAW_SKETCH) {
                if (MessageUtils.isValidAttach(mDstPath, false)) {
                    ret = true;
                } else {
                    ret = false;
                }
            } else {
                if (MessageUtils.isValidAttach(mDstPath, true)) {
                    ret = true;
                } else {
                    ret = false;
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (null != cursor && !cursor.isClosed()) {
                cursor.close();
            }
        }
        return ret;
    }
}
