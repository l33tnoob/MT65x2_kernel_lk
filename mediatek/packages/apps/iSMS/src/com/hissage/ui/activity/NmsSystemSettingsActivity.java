package com.hissage.ui.activity;

import java.util.Date;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnKeyListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.URLSpan;
import android.util.Log;

import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsExternalStorage;
import com.hissage.config.NmsProfileSettings;
import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapterforjni;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.pn.hpnsReceiver;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsMsgType;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.struct.SNmsSystemStatus;
import com.hissage.ui.view.NmsPreferenceForProfile_1;
import com.hissage.ui.view.NmsPreferenceForProfile_2;
import com.hissage.ui.view.NmsSwitchPreference;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsDateTool;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class NmsSystemSettingsActivity extends PreferenceActivity {
    private static final String TAG = "NmsSystemSettingsActivity";

    private class SNmsSimInfoMTK {
        long sim_id = NmsConsts.INVALID_SIM_ID;
        String simName = null;
        String phone = null;
        int color = -1;
        boolean simEnable = false;
    }

    private SNmsSimInfoMTK[] mSimInfo = new SNmsSimInfoMTK[NmsConsts.SIM_CARD_COUNT];

    private static final String KEY_SENDASSMS = "nms_send_as_sms";
    private static final String KEY_CAPTION = "nms_caption";
    private static final String KEY_AUTODOWNLOAD = "nms_auto_download";
    private static final String KEY_NETWORKUSAGE = "nms_network_usage";
    // private static final String KEY_UPDATE = "nms_software_update";
    private static final String KEY_SIM_INFO_CATE = "nms_sim_info";
    private static final String KEY_SIM_INFO1 = "nms_sim1";
    private static final String KEY_SIM_INFO2 = "nms_sim2";
    private static final String KEY_PREFRENCE_CATE = "nms_preferences";
    private static final String KEY_ABOUT = "nms_about";
    private static final String KEY_READ_STATUS = "nms_show_read_status";
    private static final String KEY_REMINDERS = "nms_show_reminders";

    private PreferenceCategory mSimInfoCategory = null;
    private NmsPreferenceForProfile_1 mSimInfo1 = null;
    private NmsPreferenceForProfile_2 mSimInfo2 = null;
    private PreferenceCategory mPrefrenceCategory = null;
    private CheckBoxPreference mSendAsSms = null;
    private NmsSwitchPreference mCaptions = null;
    private CheckBoxPreference mAutoDownload = null;
    private Preference mNetworkUsage = null;
    private Preference mAbout = null;
    // private boolean mIgnore = false;
    private CheckBoxPreference mReadStatus = null;
    private CheckBoxPreference mReminders = null;
    private NmsSetProfileResultRecver mResultRecver = null;
    private AlertDialog mActivationDlg = null;
    private ProgressDialog mWaitDlg = null;
    private long mSimId = NmsConsts.INVALID_SIM_ID;
    private boolean isWaitDlg = false;
    private int soltId = -1;
    private boolean isChecked1 = false;
    private boolean isChecked2 = false;
    SNmsSimInfo info1 = null;
    SNmsSimInfo info2 = null;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message message) {
            switch (message.what) {
            case 0xffff:
                if (mWaitDlg != null) {
                    mWaitDlg.cancel();
                    mWaitDlg = null;
                }
                checkSimCardStatus();

                // updateStatus();
                return;
            default:
                // do nothing
                break;
            }
        }
    };

    private void getAllSimInfo() {

        for (int i = 0; i < NmsConsts.SIM_CARD_COUNT; ++i) {
            if (null == mSimInfo[i]) {
                mSimInfo[i] = new SNmsSimInfoMTK();
            }
            mSimInfo[i].sim_id = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(i);
            if (mSimInfo[i].sim_id > 0) {
                mSimInfo[i].simName = NmsPlatformAdapter.getInstance(this).getSimName(
                        mSimInfo[i].sim_id);
                mSimInfo[i].color = NmsPlatformAdapter.getInstance(this).getSimColor(
                        mSimInfo[i].sim_id);
                SNmsSimInfo sim = NmsIpMessageApiNative
                        .nmsGetSimInfoViaSimId((int) mSimInfo[i].sim_id);
                if (null != sim) {
                    mSimInfo[i].simEnable = NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED == sim.status;
                    mSimInfo[i].phone = sim.number;
                } else {
                    // do nothing
                }
            } else {
                // do nothing
            }
        }
    }

    private void showWaitDlg() {

        if (mWaitDlg != null) {
            return;
        }
        mWaitDlg = new ProgressDialog(this);
        mWaitDlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mWaitDlg.setMessage(getString(R.string.STR_NMS_WAIT));
        mWaitDlg.setIndeterminate(false);
        mWaitDlg.setCancelable(false);
        mWaitDlg.show();
        new Thread() {
            public void run() {
                try {
                    isWaitDlg = true;
                    sleep(6000);
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                mHandler.sendEmptyMessage(0xffff);
            }
        }.start();
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        int id = item.getItemId();
        if (id == android.R.id.home) {
            finish();
        } else if (id == R.id.resetSetting) {
            resetDefaultSetting();
        }
        return super.onMenuItemSelected(featureId, item);
    }

    private Drawable getAvatar(String filepath) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        Bitmap bitmap = BitmapFactory.decodeFile(filepath, options);
        options.inJustDecodeBounds = false;
        int l = Math.max(options.outHeight, options.outWidth);
        int be = (int) (l / 500);
        if (be <= 0)
            be = 1;
        options.inSampleSize = be;
        bitmap = BitmapFactory.decodeFile(filepath, options);
        if (null != bitmap) {
            return new BitmapDrawable(bitmap);
        } else {
            bitmap = BitmapFactory.decodeResource(this.getResources(),
                    R.drawable.contact_default_avatar);
            NmsLog.error(TAG, "can not parse profile setting avatar file: " + filepath);
            if (null != bitmap) {
                return new BitmapDrawable(bitmap);
            } else {
                NmsLog.error(TAG, "can not parse default avatar");
                return null;
            }
        }
    }

    private void resetDefaultSetting() {
        NmsConfig.setSendAsSMSFlag(0);

        NmsConfig.setCaptionFlag(0);
        NmsConfig.setAudioCaptionFlag(0);
        NmsConfig.setVideoCaptionFlag(0);
        NmsConfig.setPhotoCaptionFlag(0);

        NmsConfig.setAutoDownloadFlag(0);

        engineadapter.get().nmsUISetShowReadStatus(0);

        NmsConfig.setShowRemindersFlag(0);
        updateStatus();
    }

    private void checkSimCardStatus() {
        boolean enable = mSimInfo[soltId].simEnable;
        long simId = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(soltId);

        if ((simId > 0 && ((NmsIpMessageApiNative.nmsGetActivationStatus((int) simId) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) == enable))) {
            if (enable) {
                // disable failed
                Toast.makeText(this, R.string.STR_NMS_DISABLE_FAILED, Toast.LENGTH_SHORT).show();
            } else {
                // enable failed
                Toast.makeText(this, R.string.STR_NMS_ENABLE_FAILED, Toast.LENGTH_SHORT).show();
            }
        } else {
            if (enable) {
                // disable success
                Toast.makeText(this, R.string.STR_NMS_DISABLE_SUCCESS, Toast.LENGTH_SHORT).show();

            } else {
                // enable success
                Toast.makeText(this, R.string.STR_NMS_ENABLE_SUCCESS, Toast.LENGTH_SHORT).show();

            }
        }
        isWaitDlg = false;
        updateStatus();
    }

    private void updateStatus() {
        if (!isWaitDlg) {
            getAllSimInfo();
        }

        long simId1 = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_1);
        long simId2 = NmsPlatformAdapter.getInstance(this).getSimIdBySlotId(
                NmsConsts.SIM_CARD_SLOT_2);

        if (NmsSMSMMSManager.isDefaultSmsApp()
                && ((simId1 > 0 && NmsIpMessageApiNative.nmsGetActivationStatus((int) simId1) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                || (simId2 > 0 && NmsIpMessageApiNative.nmsGetActivationStatus((int) simId2) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED))) {
            mPrefrenceCategory.setEnabled(true);
            if (!NmsCommonUtils.getSDCardStatus() || NmsCommonUtils.getSDCardFullStatus()) {
                mAutoDownload.setEnabled(false);
            } else {
                mAutoDownload.setEnabled(true);
            }

            mNetworkUsage.setEnabled(true);
        } else {
            mPrefrenceCategory.setEnabled(false);
            mNetworkUsage.setEnabled(false);
        }
        info1 = NmsIpMessageApiNative
                .nmsGetSimInfoViaSimId((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id);
        info2 = NmsIpMessageApiNative
                .nmsGetSimInfoViaSimId((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id);
        mSimInfo1.setSelectable(false);
        mSimInfo2.setSelectable(false);

        if (simId1 > 0) {
            NmsProfileSettings profile_1 = engineadapter.get().nmsUIGetUserInfoViaImsi(info1.imsi);
            if (null != profile_1) {
                if (!TextUtils.isEmpty(profile_1.name)) {
                    mSimInfo1.setTitle(profile_1.name);
                }
                if (!TextUtils.isEmpty(profile_1.fileName)
                        && NmsCommonUtils.isExistsFile(profile_1.fileName)) {
                    mSimInfo1.setIcon(getAvatar(profile_1.fileName));
                } else {
                    mSimInfo1.setIcon(R.drawable.contact_default_avatar);
                }
            }
        }

        if (simId2 > 0) {
            // NmsProfileSettings profile_2 =
            // engineadapter.get().nmsUIGetUserInfoViaImsi(String.valueOf(info2.imsi));
            NmsProfileSettings profile_2 = engineadapter.get().nmsUIGetUserInfoViaImsi(info2.imsi);
            if (null != profile_2) {
                if (!TextUtils.isEmpty(profile_2.name)) {
                    mSimInfo2.setTitle(profile_2.name);
                }
                if (!TextUtils.isEmpty(profile_2.fileName)
                        && NmsCommonUtils.isExistsFile(profile_2.fileName)) {
                    mSimInfo2.setIcon(getAvatar(profile_2.fileName));
                } else {
                    mSimInfo2.setIcon(R.drawable.contact_default_avatar);
                }
            }
        }

        mSendAsSms.setChecked(NmsConfig.getSendAsSMSFlag());

        mCaptions.setChecked(NmsConfig.getCaptionFlag());

        mAutoDownload.setChecked(NmsConfig.getAutoDownloadFlag());

        mReadStatus.setChecked(engineadapter.get().nmsUIGetShowReadStatus() != 0);

        mReminders.setChecked(NmsConfig.getShowRemindersFlag());

        if (mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id > 0) {
            mSimInfoCategory.addPreference(mSimInfo1);
            mSimInfo1.setEnabled(true);
            if (TextUtils.isEmpty(mSimInfo[NmsConsts.SIM_CARD_SLOT_1].simName)) {
                mSimInfo1.setName("");
            } else {
                mSimInfo1.setName(mSimInfo[NmsConsts.SIM_CARD_SLOT_1].simName);
            }
            mSimInfo1.setNumber(mSimInfo[NmsConsts.SIM_CARD_SLOT_1].phone);
            mSimInfo1.setChecked(mSimInfo[NmsConsts.SIM_CARD_SLOT_1].simEnable);
            if (NmsSMSMMSManager.isDefaultSmsApp()) {
                mSimInfo1.setEnabled(true) ;
                activateChangeListener1();
            } else {
                mSimInfo1.setEnabled(false) ;
            }
        } else {
            mSimInfoCategory.removePreference(mSimInfo1);
        }

        if (mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id > 0) {
            mSimInfoCategory.addPreference(mSimInfo2);
            mSimInfo2.setEnabled(true);
            if (TextUtils.isEmpty(mSimInfo[NmsConsts.SIM_CARD_SLOT_2].simName)) {
                mSimInfo2.setName("");
            } else {
                mSimInfo2.setName(mSimInfo[NmsConsts.SIM_CARD_SLOT_2].simName);
            }
            mSimInfo2.setNumber(mSimInfo[NmsConsts.SIM_CARD_SLOT_2].phone);
            mSimInfo2.setChecked(mSimInfo[NmsConsts.SIM_CARD_SLOT_2].simEnable);
            if (NmsSMSMMSManager.isDefaultSmsApp()) {
                mSimInfo2.setEnabled(true) ;
                activateChangeListener2();
            } else {
                mSimInfo2.setEnabled(false) ;
            }
        } else {
            mSimInfoCategory.removePreference(mSimInfo2);
        }

    }

    private void init() {

        mSimInfoCategory = (PreferenceCategory) findPreference(KEY_SIM_INFO_CATE);
        mSimInfo1 = (NmsPreferenceForProfile_1) findPreference(KEY_SIM_INFO1);
        mSimInfo2 = (NmsPreferenceForProfile_2) findPreference(KEY_SIM_INFO2);

        mSendAsSms = (CheckBoxPreference) findPreference(KEY_SENDASSMS);
        mCaptions = (NmsSwitchPreference) findPreference(KEY_CAPTION);
        mAutoDownload = (CheckBoxPreference) findPreference(KEY_AUTODOWNLOAD);
        mNetworkUsage = findPreference(KEY_NETWORKUSAGE);
        mPrefrenceCategory = (PreferenceCategory) findPreference(KEY_PREFRENCE_CATE);
        mAbout = findPreference(KEY_ABOUT);
        mReadStatus = (CheckBoxPreference) findPreference(KEY_READ_STATUS);
        mReminders = (CheckBoxPreference) findPreference(KEY_REMINDERS);

    }

    private void showNetUsage() {
        long lastTime = NmsConfig.getClearFlowTime();
        String time = getString(R.string.STR_NMS_NEVER_CLEAR);
        SNmsSystemStatus sysStatus = engineadapter.get().nmsUIGetSystemStatus();
        if (null == sysStatus) {
            NmsLog.error(TAG, "get system status error");
            return;
        }
        if (lastTime > 0) {
            time = NmsDateTool.getDateFormat(this).format(new Date(lastTime));
            time += (" " + DateFormat.getTimeFormat(this).format(new Date(lastTime)));
        }
        String dlgContent =null ;
        if (mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id > 0) {
            if (mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id > 0) {
                 dlgContent = String.format(
                        getString(R.string.STR_NMS_DOUBLE_NETWORK_DLG_CONTENT),
                        (float) ((float) sysStatus.sendBytesWifi) / 1024,
                        (float) ((float) sysStatus.sendBytesSim1) / 1024,
                        (float) ((float) sysStatus.sendBytesSim2) / 1024,
                        (float) ((float) sysStatus.recvBytesWifi) / 1024, 
                        (float) ((float) sysStatus.recvBytesSim1) / 1024, 
                        (float) ((float) sysStatus.recvBytesSim2) / 1024, 
                         time);                                               // SIM1 wifi 
                                                                             // and
                                                                             // SIM2
            } else {
                 dlgContent = String.format(
                        getString(R.string.STR_NMS_SINGLE_NETWORK_DLG_CONTENT),
                        (float) ((float) sysStatus.sendBytesWifi) / 1024,
                        (float) ((float) sysStatus.sendBytesSim1) / 1024,
                        (float) ((float) sysStatus.recvBytesWifi) / 1024,
                        (float) ((float) sysStatus.recvBytesSim1 / 1024), time); // SIM1 wifi 
            }
        } else {
            if (mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id > 0) {
                 dlgContent = String.format(
                        getString(R.string.STR_NMS_SINGLE_NETWORK_DLG_CONTENT),
                        (float) ((float) sysStatus.sendBytesWifi) / 1024,
                        (float) ((float) sysStatus.sendBytesSim2) / 1024,
                        (float) ((float) sysStatus.recvBytesWifi) / 1024,
                        (float) ((float) sysStatus.recvBytesSim2 / 1024), time); // SIM2 wifi 

            }
        }

        new AlertDialog.Builder(this).setTitle(R.string.STR_NMS_NETWORK_USAGE_TITLE)
                .setMessage(dlgContent)
                .setPositiveButton(R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    }
                }).setNegativeButton(R.string.STR_NMS_RESET, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        NmsConfig.setClearFlowTime(System.currentTimeMillis());
                        engineadapter.get().nmsUIClearFlow();
                    }
                }).create().show();

    }

    private class MyURLSpan extends ClickableSpan {
        @Override
        public void onClick(View widget) {
            if (mActivationDlg != null) {
                mActivationDlg.dismiss();
                mActivationDlg = null;
            }
            Intent i = new Intent(NmsSystemSettingsActivity.this, NmsTermActivity.class);
            i.putExtra(NmsConsts.SIM_ID, mSimId);
            startActivity(i);
        }
    }

    protected void showActivitionDlg(final long sim_id, int mode) {
        mSimId = sim_id;

        LayoutInflater factory = LayoutInflater.from(this);
        final View view = factory.inflate(R.layout.alert_dialog_text_view, null);

        TextView textView = (TextView) view.findViewById(R.id.term_textview);

        String termContent = getString(NmsBaseActivity.WELCOME == mode ? R.string.STR_NMS_TERM_WARN_WELCOME
                : R.string.STR_NMS_TERM_WARN_ACTIVATE);
        SpannableString ss = new SpannableString(termContent);

        ss.setSpan(new URLSpan("noting"),
                termContent.indexOf(getString(R.string.STR_NMS_MENU_LICENSE_AGREEMENT)),
                termContent.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        ((TextView) textView).setText(ss);

        textView.setMovementMethod(LinkMovementMethod.getInstance());
        CharSequence text = textView.getText();
        if (text instanceof Spannable) {
            int end = text.length();
            Spannable sp = (Spannable) textView.getText();
            URLSpan[] urls = sp.getSpans(0, end, URLSpan.class);
            SpannableStringBuilder style = new SpannableStringBuilder(text);
            style.clearSpans();// should clear old spans
            for (URLSpan url : urls) {
                MyURLSpan myURLSpan = new MyURLSpan();
                style.setSpan(myURLSpan, sp.getSpanStart(url), sp.getSpanEnd(url),
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            textView.setText(style);
        }
        // M: Activation Statistics
        NmsStatistics.incKeyVal(NmsStatistics.KEY_SETTING_ACTIVATE_PROMPT);
        mActivationDlg = new AlertDialog.Builder(this)
                .setTitle(
                        NmsBaseActivity.WELCOME == mode ? R.string.STR_NMS_WELCOME_ACTIVE
                                : R.string.STR_NMS_ACTIVE)
                .setView(view)
                .setPositiveButton(R.string.STR_NMS_AGREE_AND_CONTINUE,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                NmsStartActivityApi.nmsStartActivitionActivity(
                                        // M: Activation Statistics
                                        NmsSystemSettingsActivity.this, (int) sim_id,
                                        NmsIpMessageConsts.NmsUIActivateType.SETTING);
                            }
                        }).create();
        mActivationDlg.setOnKeyListener(new OnKeyListener() {

            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                // TODO Auto-generated method stub
                if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
                    mActivationDlg.cancel();
                    updateStatus();
                    return false;
                }
                return false;
            }

        });
        mActivationDlg.show();

    }

    private void regRecver() {
        mResultRecver = new NmsSetProfileResultRecver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsProfileSettings.CMD_RESULT_KEY);
        filter.addAction(NmsIpMessageConsts.NmsSimStatus.NMS_SIM_STATUS_ACTION);
        filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
        registerReceiver(mResultRecver, filter);
    }
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setDisplayShowHomeEnabled(true);
        getActionBar().setLogo(R.drawable.isms);

        addPreferencesFromResource(R.xml.system_settings);
        init();
        updateStatus();

        regRecver();
    }

    public void activateChangeListener1() {

        View.OnTouchListener listener1 = new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // TODO Auto-generated method stub

                if (event.getAction() == MotionEvent.ACTION_UP) {

                    getAllSimInfo();

                    isChecked1 = mSimInfo1.isChecked();

                    if (null == info1) {

                        mSimInfo1.setChecked(false);

                        Toast.makeText(NmsSystemSettingsActivity.this,
                                R.string.STR_NMS_ENABLE_FAILED, Toast.LENGTH_SHORT).show();
                        return false;
                    }

                    if (!isChecked1 && info1.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                        mSimInfo1.setChecked(false);

                        if (NmsCommonUtils.isNetworkReady(NmsSystemSettingsActivity.this)) {
                            showActivitionDlg(mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id,
                                    NmsBaseActivity.WELCOME);
                            return false;
                        } else {
                            Toast.makeText(NmsSystemSettingsActivity.this,
                                    R.string.STR_NMS_NO_CONNECTION, Toast.LENGTH_SHORT).show();
                            return false;
                        }
                    }

                    if (!isChecked1) {
                        if ((NmsCommonUtils.getSDCardStatus()
                                && NmsCommonUtils.getSDcardAvailableSpace() < 5 * 1024 * 1024
                                && NmsCommonUtils.getExternalSdCardAvailableSpace() < 5 * 1024 * 1024 && NmsCommonUtils
                                .getSysStorageAvailableSpace() < 5 * 1024 * 1024)
                                || (!(NmsCommonUtils.getSDCardStatus()) && NmsCommonUtils
                                        .getSysStorageAvailableSpace() < 5 * 1024 * 1024)) {
                            // (SD && SD < 5M && Phone < 5M) || (No SD && Phone
                            // < 5M )
                            mSimInfo1.setChecked(false);
                            Toast.makeText(NmsSystemSettingsActivity.this,
                                    R.string.STR_NMS_SPACE_NOT_ENOUGH_ADD, Toast.LENGTH_SHORT)
                                    .show();
                            return false;
                        }

                    }

                    soltId = 0;

                    if (isChecked1) {
                        if (NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED != info1.status) {
                            NmsIpMessageApiNative
                                    .nmsDisableIpService((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id);
                            hpnsReceiver.stopHPNS(NmsSystemSettingsActivity.this);
                            showWaitDlg();
                            return false;
                        } else {
                            updateStatus();
                            return false;
                        }

                    } else {

                        if (NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED != info1.status) {
                            NmsIpMessageApiNative
                                    .nmsEnableIpService((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_1].sim_id);
                            hpnsReceiver.startHPNS(NmsSystemSettingsActivity.this);
                            showWaitDlg();
                            return false;

                        } else {
                            updateStatus();
                            return false;
                        }

                    }

                }
                return false;
            }

        };

        mSimInfo1.setMyListener(listener1);

    }

    public void activateChangeListener2() {

        View.OnTouchListener listener2 = new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // TODO Auto-generated method stub

                if (event.getAction() == MotionEvent.ACTION_UP) {

                    getAllSimInfo();

                    isChecked2 = mSimInfo2.isChecked();

                    if (null == info2) {

                        mSimInfo2.setChecked(false);

                        Toast.makeText(NmsSystemSettingsActivity.this,
                                R.string.STR_NMS_ENABLE_FAILED, Toast.LENGTH_SHORT).show();
                        return false;
                    }

                    if (!isChecked2 && info2.status < NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                        mSimInfo2.setChecked(false);

                        if (NmsCommonUtils.isNetworkReady(NmsSystemSettingsActivity.this)) {
                            showActivitionDlg(mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id,
                                    NmsBaseActivity.WELCOME);
                            return false;
                        } else {
                            Toast.makeText(NmsSystemSettingsActivity.this,
                                    R.string.STR_NMS_NO_CONNECTION, Toast.LENGTH_SHORT).show();
                            return false;
                        }
                    }

                    if (!isChecked2) {
                        if ((NmsCommonUtils.getSDCardStatus()
                                && NmsCommonUtils.getSDcardAvailableSpace() < 5 * 1024 * 1024
                                && NmsCommonUtils.getExternalSdCardAvailableSpace() < 5 * 1024 * 1024 && NmsCommonUtils
                                .getSysStorageAvailableSpace() < 5 * 1024 * 1024)
                                || (!(NmsCommonUtils.getSDCardStatus()) && NmsCommonUtils
                                        .getSysStorageAvailableSpace() < 5 * 1024 * 1024)) {
                            // (SD && SD < 5M && Phone < 5M) || (No SD && Phone
                            // < 5M )
                            mSimInfo2.setChecked(false);
                            Toast.makeText(NmsSystemSettingsActivity.this,
                                    R.string.STR_NMS_SPACE_NOT_ENOUGH_ADD, Toast.LENGTH_SHORT)
                                    .show();
                            return false;
                        }
                    }

                    soltId = 1;

                    if (isChecked2) {
                        if (NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED != info2.status) {
                            NmsIpMessageApiNative
                                    .nmsDisableIpService((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id);
                            hpnsReceiver.stopHPNS(NmsSystemSettingsActivity.this);
                            showWaitDlg();
                            return false;
                        } else {
                            updateStatus();
                            return false;
                        }

                    } else {

                        if (NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED != info2.status) {
                            NmsIpMessageApiNative
                                    .nmsEnableIpService((int) mSimInfo[NmsConsts.SIM_CARD_SLOT_2].sim_id);
                            hpnsReceiver.startHPNS(NmsSystemSettingsActivity.this);
                            showWaitDlg();
                            return false;

                        } else {
                            updateStatus();
                            return false;
                        }

                    }

                }
                return false;
            }

        };

        mSimInfo2.setMyListener(listener2);

    }

    @Override
    public void onPause() {
        super.onPause();

    }

    @Override
    public void onResume() {
        super.onResume();
        updateStatus();
    }

    @Override
    public void onDestroy() {
        if (mResultRecver != null) {
            unregisterReceiver(mResultRecver);
        }
        super.onDestroy();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {

        if (preference == mAutoDownload) {
            NmsConfig.setAutoDownloadFlag(mAutoDownload.isChecked() ? 1 : 0);
        } else if (preference == mReadStatus) {
            engineadapter.get().nmsUISetShowReadStatus(mReadStatus.isChecked() ? 1 : 0);
        } else if (preference == mReminders) {
            NmsConfig.setShowRemindersFlag(mReminders.isChecked() ? 0 : 1);
        } else if (preference == mCaptions) {
            Intent i = new Intent(this, NmsCaptionSettingsActivity.class);
            this.startActivity(i);
            return false;
        } else if (preference == mSendAsSms) {
            NmsConfig.setSendAsSMSFlag(mSendAsSms.isChecked() ? 0 : 1);
            // M: Add new feature: ISMS-214
            NmsStatistics
                    .incKeyVal(NmsConfig.getSendAsSMSFlag() ? NmsStatistics.KEY_OPEN_SEND_BY_SMS
                            : NmsStatistics.KEY_CLOSE_SEND_BY_SMS);
        } else if (preference == mNetworkUsage) {
            showNetUsage();
        } else if (preference == mAbout) {
            Intent i = new Intent(this, NmsAboutActivity.class);
            this.startActivity(i);
        }
        return true;
    }

    public class NmsSetProfileResultRecver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {

            //add for jira-557
            if(NmsIntentStrId.NMS_REG_STATUS.equals(intent.getAction())){
                int regStatus = intent.getIntExtra("regStatus", -1);
                switch (regStatus) {
                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
                    Toast.makeText(getApplicationContext(),R.string.STR_NMS_ENABLE_SUCCESS, Toast.LENGTH_SHORT).show();
                    updateStatus();
                    break;
                }
                return;
            }
            //add end

            if (null == intent || !NmsProfileSettings.CMD_RESULT_KEY.equals(intent.getAction())) {
                NmsLog.error(TAG, "recv error intent: " + intent);
                return;
            }

            NmsLog.error(TAG, "recv intent action: " + intent.getAction());
            if (intent.getAction().equals(NmsIpMessageConsts.NmsSimStatus.NMS_SIM_STATUS_ACTION)) {
                mHandler.sendEmptyMessage(0xffff);
            } else {
                int cmdCode = intent.getIntExtra(NmsProfileSettings.CMD_CODE, -1);
                if (NmsProfileSettings.CMD_SET_HESINE_INFO_ACK == cmdCode) {
                    int errorCode = intent.getIntExtra(NmsProfileSettings.CMD_RESULT_KEY, -1000);
                    if (0 == errorCode) {
                        Toast.makeText(NmsService.getInstance(), R.string.STR_NMS_SET_PROFILE_OK,
                                Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(NmsService.getInstance(), R.string.STR_NMS_SET_PROFILE_FAIL,
                                Toast.LENGTH_SHORT).show();
                    }
                }

                updateStatus();
            }
        }

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.system_setting_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        return super.onPrepareOptionsMenu(menu);
    }

}
