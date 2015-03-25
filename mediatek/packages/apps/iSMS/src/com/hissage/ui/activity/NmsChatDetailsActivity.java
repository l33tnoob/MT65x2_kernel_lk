package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.media.MediaMetadataRetriever;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.MediaStore.Audio;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;
import android.util.DisplayMetrics;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Gallery;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.api.NmsiSMSApi;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.download.NmsDownloadManager;
import com.hissage.imagecache.NmsContactAvatarCache;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsHesineApiConsts.NmsGetContactMsgRecordIdListFlag;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpCalendarMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDelIpMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDownloadAttachStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsNewMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsShareLocationDone;
import com.hissage.message.ip.NmsIpMessageConsts.NmsSimStatus;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.message.smsmms.NmsCreateSmsThread;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.notification.NmsNotificationManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.struct.SNmsMsgType;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.struct.SelectRecordIdList;
import com.hissage.ui.adapter.NmsChatListAdapter;
import com.hissage.ui.view.NmsBottomPanelView;
import com.hissage.ui.view.NmsBottomPanelView.ComposeMessageListener;
import com.hissage.ui.view.NmsChatLongPressDialog;
import com.hissage.ui.view.NmsChatLongPressDialog.onItemClickListener;
import com.hissage.ui.view.NmsMessageItemView;
import com.hissage.ui.view.NmsSwitchSIMView;
import com.hissage.ui.view.NmsSwitchSIMView.ActionListener;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.data.NmsImportantList;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;
import com.hissage.util.message.NmsUIApiUtils;

public class NmsChatDetailsActivity extends NmsShareBaseActivity {

    private View mCustomTitle;
    private TextView mTopTitle;
    private ImageView mMuteLogo;
    private TextView mTopSubtitle;
    private NmsContact mContact;
    private ListView mChatList;
    private NmsBottomPanelView mBottomPanel;
    private NmsSwitchSIMView mSwitchPanel;
    private NmsChatListAdapter mChatListAdapter;
    private boolean mMarkState = false;
    private boolean mMarkAllState = false;
    private boolean mForwardDownloadState = false;
    private short mContactId;
    private Button mChatSelect;
    private ImageView mChatBg;
    private MessageReceiver mReceiver = null;
    private NmsContactAvatarCache mAvatarCache;
    private Bitmap mAvatar;
    private Context mContext;
    private ActionMode mSelectMode;
    private NmsChatLongPressDialog mLpDialog;
    private ProgressDialog mDialog;
    private boolean mDeleteImportant = false;
    private boolean mNeedShowLoadMsg = false;
    private DisplayMetrics mDisplay;
    private ProgressDialog mPdVcardWait;
    private AlertDialog viewDetialsDialog;
    private ComponentName mComponentName;

    ///M: add for jira-526
    private int mUnDownloadedIpMessageCount = 0;
    private int mDownloadedIpMessageStepCounter;
    private int mDownloadedIpMessageStepCounterSuccess;
    private int mDownloadedIpMessageStepCounterFail;
    /// @}

    private NmsChatSettings mChatSettings;

    private final static String SIM_CHANGE = "android.intent.action.SMS_DEFAULT_SIM";

    private final static int NMS_ENG_MSG_ENABLE_SIM = 34;
    private final static int NMS_ENG_MSG_DISABLE_SIM = 35;

    private String mVcardName;

    private String mAddress = null;

    private String TAG = "ChatDetailsActivity";
    
    private int mReadedburnTime=7;

    private Runnable sendSketch = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                sendImage(DRAW_SKETCH, mHandler);
                refreshAndScrollList();
            }

            mHandler.removeCallbacks(sendSketch);
        }
    };

    private Runnable sendPic = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                sendImage(TAKE_PHOTO, mHandler);
                mHandler.removeCallbacks(sendPic);
                refreshAndScrollList();
            }

            mHandler.removeCallbacks(sendPic);
        }
    };
    private Runnable sendReadedburn = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                sendImage(TAKE_READED_BURN_PHOTO, mHandler);
                mHandler.removeCallbacks(sendReadedburn);
                refreshAndScrollList();
            }
            
            mHandler.removeCallbacks(sendReadedburn);
        }
    };

    private Runnable updataContactAvatar = new Runnable() {
        @Override
        public void run() {
            ActionBar actionBar = getActionBar();
            actionBar.setLogo(new BitmapDrawable(mAvatar));
            mHandler.removeCallbacks(updataContactAvatar);
        }
    };

    private void sendImage(int requestCode, Handler handler) {
        String caption="";
        NmsIpImageMessage msg = null;
        int flag=0;
        if (requestCode == DRAW_SKETCH) {
            msg = new NmsIpImageMessage(NmsIpMessageConsts.NmsIpMessageType.SKETCH);
        }else if(requestCode == TAKE_READED_BURN_PHOTO){
            msg = new NmsIpImageMessage(NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            flag=NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ;
            caption=Integer.toString(mReadedburnTime);
        } else {
            msg = new NmsIpImageMessage(NmsIpMessageConsts.NmsIpMessageType.PICTURE);
        }
        if (NmsConfig.getSendAsSMSFlag() && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
            msg.send(mContext, flag,mContact.getNumber(), mDstPath, caption,
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                    NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), false, handler);
        } else {
            msg.send(mContext, flag,mContact.getNumber(), mDstPath, caption,
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                    NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), false,
                    handler);
        }
    }

    private void sendVcardMsg(String vcardName, String dstPath) {
        if (TextUtils.isEmpty(dstPath)) {
            NmsLog.error(TAG, "sendVcardMsg: param is error");
            return;
        }
        if (mContext == null || mContact == null) {
            NmsLog.error(TAG, "sendVcardMsg: mContext/mContact is error");
            return;
        }

        NmsIpVCardMessage msg = new NmsIpVCardMessage();
        if (NmsConfig.getSendAsSMSFlag() && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
            msg.send(mContact.getNumber(), dstPath, vcardName, (int) NmsPlatformAdapter
                    .getInstance(mContext).getCurrentSimId(),
                    NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, false);
        } else {
            msg.send(mContact.getNumber(), dstPath, vcardName, (int) NmsPlatformAdapter
                    .getInstance(mContext).getCurrentSimId(),
                    NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, false);
        }
    }

    private Runnable sendVcard = new Runnable() {
        public void run() {
            if (mPdVcardWait != null && mPdVcardWait.isShowing()) {
                mPdVcardWait.dismiss();
            }

            if (MessageUtils.isValidAttach(mDstPath, true)) {
                sendVcardMsg(mVcardName, mDstPath);
                refreshAndScrollList();
            } else {
                Toast.makeText(mContext, R.string.STR_NMS_SHARE_VCARD_FAILED, Toast.LENGTH_SHORT)
                        .show();
            }

            mHandler.removeCallbacks(sendVcard);
        }
    };

    private Runnable sendVideo = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                NmsIpVideoMessage msg = new NmsIpVideoMessage();
                if (NmsConfig.getSendAsSMSFlag()
                        && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                    msg.send(mContact.getNumber(), mDstPath, "", duration, (int) NmsPlatformAdapter
                            .getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), false);
                } else {
                    msg.send(mContact.getNumber(), mDstPath, "", duration, (int) NmsPlatformAdapter
                            .getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), false);
                }
                refreshAndScrollList();
            }

            mHandler.removeCallbacks(sendVideo);
        }
    };

    private Runnable sendAudio = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                NmsIpVoiceMessage msg = new NmsIpVoiceMessage();
                if (NmsConfig.getSendAsSMSFlag()
                        && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                    msg.send(mContact.getNumber(), mDstPath, "", duration, (int) NmsPlatformAdapter
                            .getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), false);
                } else {
                    msg.send(mContact.getNumber(), mDstPath, "", duration, (int) NmsPlatformAdapter
                            .getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), false);
                }
                refreshAndScrollList();
            }

            mHandler.removeCallbacks(sendAudio);
        }
    };

    private Runnable sendCalendar = new Runnable() {
        public void run() {
            if (NmsCommonUtils.isExistsFile(mDstPath) && NmsCommonUtils.getFileSize(mDstPath) != 0) {
                NmsIpCalendarMessage msg = new NmsIpCalendarMessage();
                if (NmsConfig.getSendAsSMSFlag()
                        && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                    msg.send(mContact.getNumber(), mDstPath, calendarSummary,
                            (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, false);
                } else {
                    msg.send(mContact.getNumber(), mDstPath, calendarSummary,
                            (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                            NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, false);
                }
                refreshAndScrollList();
            }

            mHandler.removeCallbacks(sendCalendar);
        }
    };

    @Override
    public void onRestart() {
        super.onRestart();
        initialize(mContactId);
        // changeWallPaper();
        invalidateOptionsMenu();
        // TODO jhnie, add broadcast id at here
        NmsPlatformAdapter.getInstance(mContext).CancelNotification(123);
    }

    public void nmsUnregisterReceiver() {
        try {
            if (mReceiver != null) {
                unregisterReceiver(mReceiver);
                mReceiver = null;
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        NmsLog.trace(TAG, "====IM is create====");
        super.onCreate(savedInstanceState);
        getWindow().setSoftInputMode(
                WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE
                        | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
        setContentView(R.layout.chat_details_activity);
        if (null != savedInstanceState) {
            short contactId = savedInstanceState.getShort(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID,
                    (short) -1);
            boolean loadAllMessage = savedInstanceState
                    .getBoolean(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE);
            Intent intent = new Intent();
            intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, contactId);
            intent.putExtra(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE, loadAllMessage);
            setIntent(intent);
        }
        mContext = this;
        mComponentName = getComponentName();
        mReceiver = new MessageReceiver();

        mAvatarCache = NmsContactAvatarCache.getInstance();

        initResourceRefs();
        Intent intent = getIntent();
        mNeedShowLoadMsg = intent.getBooleanExtra(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE,
                false);
        initialize((short) -1);
        initMessageList();
        // changeWallPaper();
    }

    @Override
    public void cancelActivate() {
        initBottomPanel(false);
        invalidateOptionsMenu();
    }

    private void setTextSize() {
        float textSize = MessageUtils.getMTKPreferenceFontFloat(mContext, 18);
        mBottomPanel.setEditTextSize(textSize);
    }

    @Override
    public void onResume() {
        NmsLog.trace(TAG, "onResume");
        super.onResume();
        
        NmsIpMessageApiNative.nmsCheckDefaultSmsAppChanged() ;

        NmsMessageItemView.resetSimIndicatorCache();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsIpMessageConsts.NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
        filter.addAction(NmsIpMessageConsts.NmsRefreshMsgList.NMS_REFRESH_MSG_LIST);
        filter.addAction(NmsIpMessageConsts.NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION);
        filter.addAction(NmsIpMessageConsts.NmsIpMessageStatus.NMS_READEDBURN_TIME_ACTION);
        filter.addAction(NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION);
        filter.addAction(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE);
        filter.addAction(NmsIpMessageConsts.NmsUpdateGroupAction.NMS_UPDATE_GROUP);
        filter.addAction(NmsSimStatus.NMS_SIM_STATUS_ACTION);
        filter.addAction(NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE);
        filter.addAction(SIM_CHANGE);
        filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
        registerReceiver(mReceiver, filter);
        if (mContact.getType() == NmsContactType.HISSAGE_USER) {
            NmsIpMessageApiNative.nmsEnterChatMode(mContact.getNumber());
        }
        if (mDialog != null) {
            mDialog.dismiss();
        }
        onlyRefreshList();
        NmsNotificationManager.getInstance(mContext).nmsEnterChatMode(mContactId);
        initBottomPanel(false);
        invalidateOptionsMenu();
        // TODO jhnie, add broadcast id at here
        NmsPlatformAdapter.getInstance(mContext).CancelNotification(123);
        NmsPlatformAdapter.getInstance(mContext).hideSIMIndicator(mComponentName);
        NmsPlatformAdapter.getInstance(mContext).showSIMIndicator(mComponentName);
        NmsConfig.setCurrentConversation(mContactId);

        if(NmsConfig.isAndroidKitKatOnward){
            mBottomPanel.setForbidSendIpMessage(NmsSMSMMSManager.isDefaultSmsApp());
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        setMessageAsRead();
        if (viewDetialsDialog != null) {
            viewDetialsDialog.dismiss();
            viewDetialsDialog = null;
        }
        NmsPlatformAdapter.getInstance(mContext).hideSIMIndicator(mComponentName);
        NmsLog.trace(TAG, "onPause");
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        mNeedShowLoadMsg = intent.getBooleanExtra(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE,
                false);
        initialize((short) -1);
        initMessageList();
        // changeWallPaper();
        invalidateOptionsMenu();
    }

    @Override
    public void onStop() {
        NmsLog.trace(TAG, "onStop");
        super.onStop();
        if (mContact.getType() == NmsContactType.HISSAGE_USER) {
            NmsIpMessageApiNative.nmsExitFromChatMode(mContact.getNumber());
        }

        if (!mNeedShowLoadMsg) {
            try {
                if (mReceiver != null) {
                    unregisterReceiver(mReceiver);
                }
            } catch (Exception e) {
                NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
            }
        }
        NmsNotificationManager.getInstance(mContext).nmsExitOrPauseChatMode();
    }

    @Override
    protected void onStart() {
        super.onStart();
        setTextSize();
    }

    @Override
    public void onDestroy() {
        if (mBottomPanel.isShown()) {
            mBottomPanel.saveToDraft();
        }
        if (mNeedShowLoadMsg) {
            try {
                if (mReceiver != null) {
                    unregisterReceiver(mReceiver);
                }
            } catch (Exception e) {
                NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
            }
        }

        NmsImportantList.get().clearAll();
        mBottomPanel.clearFocus();
        mBottomPanel.recycleAllView();
        mSwitchPanel.removeAllViewsInLayout();
        mSwitchPanel.clearFocus();
        mChatList.removeAllViewsInLayout();
        mChatList.clearFocus();
        NmsConfig.setCurrentConversation(0);

        mChatListAdapter = null;
        mChatList = null;
        mBottomPanel = null;
        mChatBg = null;
        mContext = null;
        mCustomTitle = null;
        mTopTitle = null;
        mMuteLogo = null;
        mTopSubtitle = null;
        mContact = null;
        mChatSelect = null;
        mAvatar = null;

        super.onDestroy();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != RESULT_OK) {
            NmsLog.error(TAG, "data is null or Bad resultCode, " + resultCode);
            return;
        }

        switch (requestCode) {
        case TAKE_PHOTO:
            if (!MessageUtils.isValidAttach(mDstPath, false)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }
            if (!MessageUtils.isPic(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }

            if (NmsConfig.getCaptionFlag() && NmsConfig.getPhotoCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            } else {
                mHandler.postDelayed(sendPic, 100);
            }
            break;

        case RECORD_VIDEO:
            if (!getVideoOrPhoto(data, requestCode)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }
            if (!MessageUtils.isVideo(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            if (!MessageUtils.isFileStatusOk(mContext, mDstPath)) {
                NmsLog.error(TAG, "record video failed, invalid file");
                return;
            }
            if (NmsConfig.getCaptionFlag() && NmsConfig.getVideoCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, duration,
                        NmsIpMessageConsts.NmsIpMessageType.VIDEO);
            } else {
                mHandler.postDelayed(sendVideo, 100);
            }
            break;

        case DRAW_SKETCH:
            if (!getVideoOrPhoto(data, requestCode)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }

            mHandler.postDelayed(sendSketch, 100);
            break;

        case SHARE_CONTACT:

            mPdVcardWait = new ProgressDialog(mContext);
            mPdVcardWait.setMessage(getText(R.string.STR_NMS_WAIT));
            mPdVcardWait.setCancelable(false);
            mPdVcardWait.show();

            if (NmsSMSMMSManager.getInstance(mContext).isExtentionFieldExsit() == 1) {
                final long[] contactsId = data
                        .getLongArrayExtra("com.mediatek.contacts.list.pickcontactsresult");
                if (contactsId == null || contactsId.length <= 0) {
                    NmsLog.error(TAG, "SHARE_CONTACT: contactsId is null");
                    mVcardName = "";
                    mDstPath = null;
                    mHandler.postDelayed(sendVcard, 100);
                    break;
                }

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        if (contactsId.length == 1) {
                            mVcardName = NmsContactApi.getInstance(mContext)
                                    .getSystemNameViaSystemContactId(contactsId[0]);
                        } else {
                            mVcardName = String.format(getString(R.string.STR_NMS_VCARD_NAME),
                                    contactsId.length);
                        }

                        mDstPath = NmsPlatformAdapter.getInstance(mContext).getVcfViaSysContactId(
                                mContext, contactsId);

                        mHandler.postDelayed(sendVcard, 100);
                    }
                }).start();

            } else {
                final Uri contactUri = data.getData();
                if (null == contactUri) {
                    NmsLog.error(TAG, "SHARE_CONTACT: contactUri is null");
                    mVcardName = "";
                    mDstPath = null;
                    mHandler.postDelayed(sendVcard, 100);
                    break;
                }

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        long selectContactId = NmsContactApi.getInstance(mContext)
                                .getSystemContactIdViaContactUri(contactUri);
                        if (selectContactId <= 0) {
                            NmsLog.error(TAG, "SHARE_CONTACT selectContactId <= 0");
                            mVcardName = "";
                            mDstPath = null;
                        } else {
                            mVcardName = NmsContactApi.getInstance(mContext)
                                    .getSystemNameViaSystemContactId(selectContactId);
                            mDstPath = NmsPlatformAdapter
                                    .getInstance(mContext)
                                    .getVcfViaSysContactId(mContext, new long[] { selectContactId });
                        }

                        mHandler.postDelayed(sendVcard, 100);
                    }
                }).start();
            }

            break;

        case CHOOSE_PHOTO:
            if (!getVideoOrPhoto(data, requestCode)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }
            if (!MessageUtils.isPic(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }

            if (NmsConfig.getCaptionFlag() && NmsConfig.getPhotoCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            } else {
                mHandler.postDelayed(sendPic, 100);
            }
            break;

        case CHOOSE_VIDEO:
            if (!getVideoOrPhoto(data, requestCode)) {
                if (NmsCommonUtils.getFileSize(mDstPath) > NmsCustomUIConfig.MAX_ATTACH_SIZE) {
                    Toast.makeText(mContext, R.string.STR_NMS_FILE_LIMIT, Toast.LENGTH_SHORT)
                            .show();
                } else {
                    Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                }
                return;
            }
            if (!MessageUtils.isVideo(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            if (!MessageUtils.isFileStatusOk(mContext, mDstPath)) {
                NmsLog.error(TAG, "choose video failed, invalid file");
                return;
            }
            if (NmsConfig.getCaptionFlag() && NmsConfig.getVideoCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, duration,
                        NmsIpMessageConsts.NmsIpMessageType.VIDEO);
            } else {
                mHandler.postDelayed(sendVideo, 100);
            }
            break;

        case RECORD_AUDIO:
            if (!getVideoOrPhoto(data, requestCode)) {
                return;
            }
            if (!MessageUtils.isAudio(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            if (!MessageUtils.isFileStatusOk(mContext, mDstPath)) {
                NmsLog.error(TAG, "record audio failed, invalid file");
                return;
            }
            if (NmsConfig.getCaptionFlag() && NmsConfig.getAudioCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, duration,
                        NmsIpMessageConsts.NmsIpMessageType.VOICE);
            } else {
                mHandler.postDelayed(sendAudio, 100);
            }
            break;

        case SHARE_LOCATION:
            sendLocationMsg(data);
            break;

        case SHARE_CALENDAR:
            String calendar = data.getDataString();
            if (TextUtils.isEmpty(calendar)) {
                return;
            }
            getCalendarFromMtk(mContext, calendar);
            mHandler.postDelayed(sendCalendar, 100);
            break;

        case GET_FORWARD_CONTACT:
            String contactStr = data.getStringExtra(NmsIpMessageConsts.NMS_SELECTION_CONTACTID);
            if (!TextUtils.isEmpty(contactStr)) {
                if (NmsSMSMMSManager.getInstance(mContext).isExtentionFieldExsit() == 1) {

                    int selectCount = 0;
                    short[] selectId = new short[getSelectCount()];
                    for (short id : getSelectList()) {
                        if (!SelectRecordIdList.get().isDownloadContains(id)) {
                            selectId[selectCount] = id;
                            selectCount++;
                        }
                    }

                    int[] msgId = engineadapter.get().nmsUIGetIpMsgPlatformIds(selectId,
                            selectCount);
                    String idArray = "";
                    if (msgId != null && msgId.length != 0) {
                        for (int id : msgId) {
                            if (TextUtils.isEmpty(idArray)) {
                                idArray = String.valueOf(id);
                            } else {
                                idArray = idArray + "," + id;
                            }
                        }
                    }
                    contactStr = contactStr.replaceAll(";", ",");
                    Intent intent = new Intent("com.android.mms.ui.MultiForwardMessageActivity");
                    intent.setClassName("com.android.mms",
                            "com.android.mms.ui.MultiForwardMessageActivity");
                    intent.putExtra("MultiForwardMessageParamNumbers", contactStr);
                    intent.putExtra("MultiForwardMessageParamMessageIds", idArray);
                    startActivity(intent);

                    if (mForwardDownloadState && SelectRecordIdList.get().getDownloadSize() > 0) {
                        Toast.makeText(mContext, R.string.STR_NMS_FORWARD_SOME_FAILED,
                                Toast.LENGTH_SHORT).show();
                    }
                    mSelectMode.finish();
                } else {

                }
            }
            break;
        case CHOOSE_AUDIO:
            if (null == data) {
                NmsLog.error(TAG, "no audio ring is selected.");
                return;
            }
            Uri uri = (Uri) data.getParcelableExtra(RingtoneManager.EXTRA_RINGTONE_PICKED_URI);
            if (null == uri) {
                uri = data.getData();
            }
            if (null == uri) {
                NmsLog.error(TAG, "choose audio failed, uri is null");
                return;
            }
            final String scheme = uri.getScheme();
            if (scheme.equals("file")) {
                mDstPath = uri.getEncodedPath();
            } else {
                ContentResolver cr = getContentResolver();
                Cursor c = cr.query(uri, null, null, null, null);
                c.moveToFirst();
                try {
                    mDstPath = c.getString(c.getColumnIndexOrThrow(Audio.Media.DATA));
                    c.close();
                } catch (Exception ex) {
                    NmsLog.error(TAG, ex.getMessage());
                    c.close();
                    return;
                }
            }

            if (!MessageUtils.isAudio(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }

            if (!MessageUtils.isFileStatusOk(mContext, mDstPath)) {
                NmsLog.error(TAG, "choose audio failed, invalid file");
                return;
            }

            MediaMetadataRetriever retriever = new MediaMetadataRetriever();

            try {
                retriever.setDataSource(mContext, uri);
                String dur = retriever
                        .extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
                if (dur != null) {
                    duration = Integer.parseInt(dur);
                    duration = duration / 1000 == 0 ? 1 : duration / 1000;
                }
            } catch (Exception ex) {
                NmsLog.error(TAG,
                        "MediaMetadataRetriever failed to get duration for " + uri.getPath());
            } finally {
                retriever.release();
            }

            if (NmsConfig.getCaptionFlag() && NmsConfig.getAudioCaptionFlag()) {
                mBottomPanel.setAttachPath(mDstPath, duration,
                        NmsIpMessageConsts.NmsIpMessageType.VOICE);
            } else {
                mHandler.postDelayed(sendAudio, 100);
            }
            break;
        case TAKE_READED_BURN_PHOTO:
            if (!MessageUtils.isValidAttach(mDstPath, false)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }
            if (!MessageUtils.isPic(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            showSendReadedBurnMsgDialog(mDstPath);
            break;
            
        case CHOOSE_READED_BURN_PHOTO:
            if (!getVideoOrPhoto(data, requestCode)) {
                Toast.makeText(mContext, R.string.STR_NMS_ERR_FILE, Toast.LENGTH_SHORT).show();
                return;
            }
            if (!MessageUtils.isPic(mDstPath)) {
                Toast.makeText(mContext, R.string.STR_NMS_INVALID_FILE_TYPE, Toast.LENGTH_SHORT)
                        .show();
                return;
            }
            showSendReadedBurnMsgDialog(mDstPath);
            break;

        default:
            break;
        }
    }

    public void showSendReadedBurnMsgDialog(String path) {
        View view = LayoutInflater.from(mContext)
                .inflate(R.layout.readed_burn_message_dialog, null);
        ImageView imageView = (ImageView) view.findViewById(R.id.ip_message_burn_thumbnail);
        ImageButton imageButton = (ImageButton) view.findViewById(R.id.send_button_readedburn);
        Gallery gallery = (Gallery) view.findViewById(R.id.gallery_burn);
        final TextView textView = (TextView) view.findViewById(R.id.tv_text_msg);
        MyAdapter myAdapter = new MyAdapter();
        gallery.setAdapter(myAdapter);
        gallery.setSelection(6);
        gallery.setOnItemSelectedListener(new OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                mReadedburnTime = arg2 + 1;
                String hint = String.format(mContext.getString(R.string.STR_NMS_DESTROY_HINT),
                        mReadedburnTime);
                int start = hint.indexOf(Integer.toString(mReadedburnTime));
                int end = start + Integer.toString(mReadedburnTime).length();
                SpannableString spannableString = new SpannableString(hint);
                spannableString.setSpan(
                        new ForegroundColorSpan(getResources().getColor(
                                R.color.hissage_numbers_color)), start, end,
                        Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                spannableString.setSpan(new StyleSpan(android.graphics.Typeface.BOLD), start, end,
                        Spanned.SPAN_EXCLUSIVE_INCLUSIVE);
                textView.setText(spannableString);
            }

            @Override
            public void onNothingSelected(AdapterView<?> arg0) {
            }
        });
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        Bitmap bitmap = BitmapFactory.decodeFile(path, options);
        options.inJustDecodeBounds = false;
        int l = Math.max(options.outHeight, options.outWidth);
        int be = (int) (l / 100);
        if (be <= 0)
            be = 1;
        options.inSampleSize = be;
        bitmap = BitmapFactory.decodeFile(path, options);
        if (null != bitmap) {
            imageView.setImageBitmap(bitmap);
        } else {
            imageView.setImageResource(R.drawable.isms_choose_a_photo);
        }

        final AlertDialog alertDialog = new AlertDialog.Builder(mContext)
                .setView(view)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {

                    }
                }).show();
        imageButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mHandler.postDelayed(sendReadedburn, 100);
                alertDialog.cancel();
            }
        });
    }

    class MyAdapter extends BaseAdapter {
        private LayoutInflater inflater;

        public MyAdapter() {
            inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return 12;
        }

        @Override
        public Object getItem(int position) {
            return position + 1;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            TextView textView1 = (TextView) convertView;
            if (textView1 == null) {
                textView1 = (TextView) inflater.inflate(R.layout.readed_burn_gallery_item, parent,
                        false);
            }
            textView1.setText(Integer.toString(position + 1));
            return textView1;
        }
    }

    private void initialize(short contactId) {
        if (contactId == -1) {
            Intent intent = getIntent();
            String action = intent.getAction();
            if (NmsIpMessageConsts.NMS_ACTION_VIEW.equals(action)) {
                contactId = intent.getShortExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID,
                        (short) -1);
            } else if (NmsIpMessageConsts.ACTION_CHAT_DETAILS.equals(action)) {
                long threadId = intent.getLongExtra("thread_id", 0);
                if (threadId > 0) {
                    NmsContact contact = NmsIpMessageApiNative
                            .nmsGetContactInfoViaThreadId(threadId);
                    if (null != contact) {
                        contactId = contact.getId();
                    }
                } else {
                    contactId = intent.getShortExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID,
                            (short) -1);
                }
            } else {
                contactId = intent.getShortExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID,
                        (short) -1);
            }

            NmsLog.trace(TAG, "get action is: " + action + ", contactId is: " + contactId);
        }

        if (contactId != mContactId && mContact != null) {
            if (mSelectMode != null) {
                mSelectMode.finish();
            }
        }

        boolean needSetDraft = false;

        if (mContactId == contactId) {
            needSetDraft = false;
        } else {
            needSetDraft = true;
        }

        mContactId = contactId;
        mContact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(mContactId);
        if (null == mContact) {
            NmsLog.error(TAG, "get contact failed, contact is null, contactId is: " + mContactId);
            // Toast.makeText(mContext, "invalid contact",
            // Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        mAddress = mContact.getNumber();
        initBottomPanel(needSetDraft);
        updateTitle();
        updateActionBar();
        initChatSettings();
    }

    private void initBottomPanel(boolean needSetDraft) {
        
        if (NmsCommonUtils.getSDCardFullStatus()) {
            mBottomPanel.setVisibility(View.GONE);
            mBottomPanel.showKeyBoard(false);
            mSwitchPanel.setVisibility(View.VISIBLE);
            mSwitchPanel.setDbStoreFullNotice();
            return;
        }
        
        if (mContact.getType() == NmsContact.NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsGroupChatContact contact = (NmsGroupChatContact) mContact;
            if (!contact.isAlive()) {
                mBottomPanel.setVisibility(View.GONE);
                mBottomPanel.showKeyBoard(false);
                mSwitchPanel.setVisibility(View.VISIBLE);
                mSwitchPanel.setLeaveNotice();
            } else {
                if (MessageUtils.isCurrentSim(mContext, contact.getSimId())) {
                    if (NmsIpMessageApiNative.nmsGetActivationStatus(contact.getSimId()) != NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                        mSwitchPanel.setVisibility(View.VISIBLE);
                        mBottomPanel.setVisibility(View.GONE);
                        mBottomPanel.showKeyBoard(false);
                        mSwitchPanel.setEnableMode(contact.getSimId());
                    } else {
                        if (mMarkState) {
                            mBottomPanel.hiden();
                            mBottomPanel.setVisibility(View.GONE);
                            mSwitchPanel.setVisibility(View.GONE);
                        } else {
                            mSwitchPanel.setVisibility(View.GONE);
                            mBottomPanel.setVisibility(View.VISIBLE);
                            mBottomPanel.bind(mContact, needSetDraft);
                            mHandler.postDelayed(new Runnable() {
                                @Override
                                public void run() {
                                    // TODO Auto-generated method stub
                                    mBottomPanel.setDraftCountTip();
                                }
                            }, 200);
                        }
                    }
                } else {
                    if (NmsIpMessageApiNative.nmsGetActivationStatus(contact.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_NOT_EXIST) {
                        mBottomPanel.setVisibility(View.GONE);
                        mBottomPanel.showKeyBoard(false);
                        mSwitchPanel.setVisibility(View.VISIBLE);
                        mSwitchPanel.setLoseSimNotice();
                    } else {
                        mBottomPanel.setVisibility(View.GONE);
                        mBottomPanel.showKeyBoard(false);
                        mSwitchPanel.setVisibility(View.VISIBLE);
                        mSwitchPanel.setSwitchMode(contact.getSimId());
                    }
                }
            }
        } else {
            if (mMarkState) {
                mBottomPanel.hiden();
                mBottomPanel.setVisibility(View.GONE);
                mSwitchPanel.setVisibility(View.GONE);
            } else {
                mSwitchPanel.setVisibility(View.GONE);
                mBottomPanel.setVisibility(View.VISIBLE);
                mBottomPanel.bind(mContact, needSetDraft);
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        NmsLog.trace(TAG, "onSaveInatanceState, contactId is: " + mContactId);
        super.onSaveInstanceState(outState);
        outState.putShort(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mContactId);
        outState.putBoolean(NmsIpMessageConsts.NMS_SHOW_LOAD_ALL_MESSAGE, mNeedShowLoadMsg);
    }

    private void initResourceRefs() {
        mDisplay = new DisplayMetrics();
        WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        wmg.getDefaultDisplay().getMetrics(mDisplay);
        mChatBg = (ImageView) findViewById(R.id.iv_chat_bg);
        mChatList = (ListView) findViewById(R.id.lv_chat_list);
        mBottomPanel = (NmsBottomPanelView) findViewById(R.id.bottom_panel);
        mSwitchPanel = (NmsSwitchSIMView) findViewById(R.id.bottom_switch_panel);
        mSwitchPanel.setActionListener(new ActionListener() {

            @Override
            public void doAction(int type) {
                if (NmsSwitchSIMView.ENABLE == type) {
                    int simId = ((NmsGroupChatContact) mContact).getSimId();
                    if (NmsIpMessageApiNative.nmsGetActivationStatus(simId) == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED) {
                        engineadapter.get().nmsUIEnableSimService(simId);
                        showEnableSimDlg();
                    } else {
						//M: Activation Statistics
                        showActivitionDlg(simId, 1, NmsIpMessageConsts.NmsUIActivateType.DIALOG);
                    }
                } else if (NmsSwitchSIMView.SWITCH == type) {
                    if (NmsPlatformAdapter.getInstance(mContext).setCurrentSimId(
                            ((NmsGroupChatContact) mContact).getSimId())) {

                        NmsPlatformAdapter.getInstance(mContext).hideSIMIndicator(mComponentName);
                        NmsPlatformAdapter.getInstance(mContext).showSIMIndicator(mComponentName);
                        initialize(mContactId);
                        invalidateOptionsMenu();
                    }
                }
            }
        });
        mBottomPanel.setHandler(mHandler);
        mBottomPanel.setComposeMessageListener(new ComposeMessageListener() {

            @Override
            public void doAction(int type, short value) {
                if (type == ComposeMessageListener.ACTION_CONTENT_CHANGE) {
                    invalidateOptionsMenu();
                }
            }
        });
    }

    private void showEnableSimDlg() {
        mDialog = new ProgressDialog(mContext);
        mDialog.setMessage(getString(R.string.STR_NMS_ENABLE_PROCESS));
        mDialog.setCancelable(false);
        mDialog.show();
    }

    private void updateActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowCustomEnabled(true);
        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowTitleEnabled(false);
        actionBar.setDisplayUseLogoEnabled(true);
    }

    private void updateTitle() {
        if (mContact == null) {
            return;
        }
        ActionBar actionBar = getActionBar();

        Bitmap avatar = mAvatarCache.getBitmapFromMemCache(mContact.getId());
        if (avatar != null) {
            actionBar.setLogo(new BitmapDrawable(avatar));
        } else {
            if (mAvatar != null) {
                actionBar.setLogo(new BitmapDrawable(mAvatar));
            } else {
                actionBar.setLogo(R.drawable.ic_contact_picture);
            }
            new Thread(new Runnable() {
                @Override
                public void run() {
                    mAvatar = NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId(
                            mContact.getId());
                    if (mAvatar != null) {
                        mHandler.postDelayed(updataContactAvatar, 100);
                    } else {
                        NmsLog.trace(TAG,
                                "the contact avatar is null. engContactId=" + mContact.getId());
                    }
                }
            }).start();
        }

        LayoutInflater inflate = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCustomTitle = inflate.inflate(R.layout.message_title, null);
        mTopTitle = (TextView) mCustomTitle.findViewById(R.id.tv_top_title);
        mMuteLogo = (ImageView) mCustomTitle.findViewById(R.id.iv_silent);

        mTopSubtitle = (TextView) mCustomTitle.findViewById(R.id.tv_top_subtitle);
        mTopTitle.setText(getLineOne());
        mTopSubtitle.setText(getLineTwo());
        actionBar.setCustomView(mCustomTitle);
    }

    private String getLineOne() {
        if (mContact == null) {
            return "invalid contact";
        }
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            String name = mContact.getName();
            return TextUtils.isEmpty(name) ? "" : name;
        } else {
            String name = mContact.getName();
            String number = mContact.getNumber();
            if (!NmsContactApi.getInstance(mContext).isExistSystemContactViaNumber(number)) {
                name = TextUtils.isEmpty(number) ? "" : number;
            } else {
                if (TextUtils.isEmpty(name)) {
                    name = TextUtils.isEmpty(number) ? "" : number;
                }
            }
            return name;
        }
    }

    private String getLineTwo() {
        if (mContact == null) {
            return "invalid contact";
        }
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            short createrId = ((NmsGroupChatContact) mContact).getCreaterId();
            NmsContact creater = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(createrId);
            if (null != creater) {
                String number = creater.getNumber();
                String name = creater.getName();
                if (!NmsContactApi.getInstance(mContext).isExistSystemContactViaNumber(number)) {
                    name = TextUtils.isEmpty(number) ? "" : number;
                } else {
                    if (TextUtils.isEmpty(name)) {
                        name = TextUtils.isEmpty(number) ? "" : number;
                    }
                }
                if (((NmsGroupChatContact) mContact).isAlive()) {
                    return String.format(getString(R.string.STR_NMS_GROUP_SUBTITLE), name,
                            ((NmsGroupChatContact) mContact).getMemberCount() - 1);
                } else {
                    return String.format(getString(R.string.STR_NMS_GROUP_DEAD_SUBTITLE), name,
                            ((NmsGroupChatContact) mContact).getMemberCount());
                }
            } else {
                return "invalid contact";
            }
        } else {
            String number = mContact.getNumber();
            return TextUtils.isEmpty(number) ? "" : number;
        }
    }

    @Override
    public void loadAllMessage() {
        short id = (short) mChatListAdapter.getItemId(mChatList.getFirstVisiblePosition());
        mChatListAdapter.setLoadAllMessage();
        mNeedShowLoadMsg = false;
        NmsImportantList.get().clearAll();
        onlyRefreshList();
        int index = 0;
        short[] allId = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(mContactId,
                NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL, false);
        for (int i = 0; i < allId.length; i++) {
            if (allId[i] == id) {
                index = i;
                break;
            }
        }
        mChatList.setSelectionFromTop(index, 0);
    }

    private OnCreateContextMenuListener mCreateContextMenu = new OnCreateContextMenuListener() {

        @Override
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            if (mMarkState) {
                return;
            }
            AdapterContextMenuInfo info = (AdapterContextMenuInfo) menuInfo;
            int position = info.position;
            final NmsIpMessage msg = (NmsIpMessage) mChatListAdapter.getItem(position);
            mLpDialog = new NmsChatLongPressDialog(mContext);
            mLpDialog.bind(msg, mContact, menu);
            mLpDialog.setOnItemClick(new onItemClickListener() {

                @Override
                public void onItemClick(int action) {
                    if (action == NmsChatLongPressDialog.delete) {
                        if (mNeedShowLoadMsg) {
                            short id = mLpDialog.getId();
                            for (int i = 0; i < NmsImportantList.get().size(); i++) {
                                short d = NmsImportantList.get().getElement(i);
                                if (d == id) {
                                    NmsImportantList.get().removeElement(d);
                                    break;
                                }
                            }
                        }
                        onlyRefreshList();
                    } else if (action == NmsChatLongPressDialog.viewDetails) {
                        String details = null;

                        String dateTime = MessageUtils.formatTimeStampString(mContext,
                                (long) msg.time * 1000, true);
                        if (msg.status == NmsIpMessageConsts.NmsIpMessageStatus.INBOX) {
                            String sendTime = MessageUtils.formatTimeStampString(mContext,
                                    (long) msg.timesend * 1000, true);
                            details = String.format(
                                    mContext.getString(R.string.STR_NMS_RECEIVED_MSG_DETAILS),
                                    msg.from, sendTime, dateTime);
                        } else {
                            details = String.format(
                                    mContext.getString(R.string.STR_NMS_SEND_MSG_DETAILS), dateTime);
                        }
                        viewDetialsDialog = new AlertDialog.Builder(mContext)
                                .setTitle(R.string.STR_NMS_VIEW_DETAILS_TITLE).setMessage(details)
                                .create();
                        viewDetialsDialog.show();

                    }
                }
            });
        }
    };

    private void initMessageList() {
        int importantCount = engineadapter.get().nmsUIGetSaveMsgCountInContact(mContactId);
        if (mNeedShowLoadMsg) {
            if (importantCount != 0) {
                short importantId[] = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(
                        mContactId, NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_SAVED,
                        false);
                for (int i = 0; i < importantCount; i++) {
                    NmsImportantList.get().addElement(importantId[i]);
                }

                NmsImportantList.get().addElement(NmsChatListAdapter.LOAD_ALL_MESSAGE_ID);
                short[] allId = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(mContactId,
                        NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL, false);
                if (allId.length == importantCount) {
                    NmsImportantList.get().clearAll();
                    mChatListAdapter = new NmsChatListAdapter(mContext, mContactId, null);
                    mNeedShowLoadMsg = false;
                } else {
                    short[] unreadId = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(
                            mContactId,
                            NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_UNREAD, false);
                    if (unreadId.length != 0) {
                        for (int i = 0; i < unreadId.length; i++) {
                            NmsImportantList.get().addElement(unreadId[i]);
                        }
                    }
                    mChatListAdapter = new NmsChatListAdapter(mContext, mContactId,
                            NmsImportantList.get().getIdList());
                }
            } else {
                mChatListAdapter = new NmsChatListAdapter(mContext, mContactId, null);
                mNeedShowLoadMsg = false;
            }
        } else {
            mChatListAdapter = new NmsChatListAdapter(mContext, mContactId, null);
            mNeedShowLoadMsg = false;
        }
        mChatList.setAdapter(mChatListAdapter);
       
        mChatListAdapter.setHandler(mHandler);
        mChatList.setItemsCanFocus(false);
        mChatList.setOnCreateContextMenuListener(mCreateContextMenu);
        mChatList.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mBottomPanel.showKeyBoard(false);
                switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_UP:
                    mContext.sendBroadcast(new Intent(NmsIpMessageConsts.ACTION_READENBURN)); 
                    break;
                }
                return false;
            }
        });
        
        mChatList.setOnItemLongClickListener(new OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                if (view != null) {
                    return ((NmsMessageItemView) view).onMessageListItemLongClick(position);
                }else{
                    return false;
                }
             
            }
        });
        
        mChatList.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                NmsIpMessage msg = (NmsIpMessage) mChatListAdapter.getItem(position);
                if (null == msg) {
                    NmsLog.error(TAG, "onItemClick failed, msg is null");
                    return;
                }
                if (mMarkState) {
                    if (mChatListAdapter.getItemViewType(position) == NmsChatListAdapter.GROUP_CREATE_ITEM) {
                        return;
                    }
                    if (SelectRecordIdList.get().isContains(
                            (short) mChatListAdapter.getItemId(position))) {
                        SelectRecordIdList.get().removeElement(
                                (short) mChatListAdapter.getItemId(position));
                        if ((msg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) != 0) {
                            if (mMarkAllState) {
                                SelectRecordIdList.get().addImportantCount();
                            } else {
                                SelectRecordIdList.get().reduceImportantCount();
                            }
                        }
                    } else {
                        SelectRecordIdList.get().addElement(
                                (short) mChatListAdapter.getItemId(position));
                        if ((msg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) != 0) {
                            if (mMarkAllState) {
                                SelectRecordIdList.get().reduceImportantCount();
                            } else {
                                SelectRecordIdList.get().addImportantCount();
                            }
                        }
                    }
                    updateSelectCount();
                }
                ((NmsMessageItemView) view).onMessageListItemClick();

            }
        });
        long targetMsgId = getIntent().getLongExtra("select_id", -1);
        int targetIpMsgId = -1;
        if (targetMsgId == -1) {
            targetIpMsgId = getIntent().getIntExtra("select_ip_id", -1);
        } else {
            NmsIpMessage msg = NmsIpMessageApi.getInstance(mContext).nmsGetIpMsgInfo(targetMsgId);
            if (msg != null) {
                targetIpMsgId = msg.ipDbId;
            } else {
                targetIpMsgId = -1;
            }
        }

        if (targetIpMsgId != -1) {
            int index = 0;
            short[] allId = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(mContactId,
                    NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL, false);
            for (int i = 0; i < allId.length; i++) {
                if (allId[i] == targetIpMsgId) {
                    index = i;
                    break;
                }
            }
            mChatList.setSelectionFromTop(index, 0);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.message_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mContact == null) {
            return false;
        }
        initMenu(menu);
        return super.onPrepareOptionsMenu(menu);
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mBottomPanel.resize();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mBottomPanel.isShow()) {
                mBottomPanel.hiden();
                return true;
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    private void initMenu(Menu menu) {
        if (mChatListAdapter != null) {
            if ((mChatListAdapter.countInfo).showAllMedia()) {
                menu.setGroupVisible(R.id.ViewAllMediaP, true);
            } else {
                menu.setGroupVisible(R.id.ViewAllMediaP, false);
            }
            if ((mChatListAdapter.countInfo).showAllLocation()) {
                menu.setGroupVisible(R.id.ViewAllLocationP, true);
            } else {
                menu.setGroupVisible(R.id.ViewAllLocationP, false);
            }
            if (mChatListAdapter.countInfo.allMsgCount != 0
                    && (mChatListAdapter.countInfo.allMsgCount != mChatListAdapter.countInfo
                            .getGroupCfgCount())) {
                menu.setGroupVisible(R.id.selectMessageP, true);
            } else {
                menu.setGroupVisible(R.id.selectMessageP, false);
            }
        }

        if (mChatSettings != null && mChatSettings.mThreadID > 0) {
            if (mContact.isBlocked()) {
                menu.setGroupVisible(R.id.markAsSpamP, false);
                menu.setGroupVisible(R.id.removeSpamP, true);
            } else {
                menu.setGroupVisible(R.id.markAsSpamP, true);
                menu.setGroupVisible(R.id.removeSpamP, false);
            }
        } else {
            menu.setGroupVisible(R.id.markAsSpamP, false);
            menu.setGroupVisible(R.id.removeSpamP, false);
        }

        if (mBottomPanel.getTextLength() != 0 || mBottomPanel.isContainAttach()) {
            menu.setGroupVisible(R.id.discardP, true);
        } else {
            menu.setGroupVisible(R.id.discardP, false);
        }

        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsGroupChatContact gc = (NmsGroupChatContact) mContact;
            menu.setGroupVisible(R.id.callP, false);
            menu.setGroupVisible(R.id.inviteToChatP, false);
            menu.setGroupVisible(R.id.inviteToISMSP, false);
            menu.setGroupVisible(R.id.clearChatHistoryP, true);
            if (gc.isAlive()
                    && MessageUtils.isCurrentSim(mContext, gc.getSimId())
                    && (NmsIpMessageApiNative.nmsGetActivationStatus(gc.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)) {
                menu.setGroupVisible(R.id.infoP, true);
                menu.setGroupVisible(R.id.editGroupP, true);
                menu.setGroupVisible(R.id.leaveGroupP, true);
                // menu.setGroupVisible(R.id.InsertQuickTextP, true);
                
                    if (mChatSettings != null && mChatSettings.mThreadID > 0) {
                        menu.setGroupVisible(R.id.ChatSettingP, true);
                    } else {
                        menu.setGroupVisible(R.id.ChatSettingP, false);
                    }

            } else {
                menu.setGroupVisible(R.id.infoP, false);
                menu.setGroupVisible(R.id.editGroupP, false);
                menu.setGroupVisible(R.id.leaveGroupP, false);
                menu.setGroupVisible(R.id.markAsSpamP, false);
                menu.setGroupVisible(R.id.removeSpamP, false);
                menu.setGroupVisible(R.id.discardP, false);
                menu.setGroupVisible(R.id.ChatSettingP, false);
                // menu.setGroupVisible(R.id.InsertQuickTextP, false);
            }
            if (gc.isAlive()
                    && gc.getMemberIds() != null
                    && MessageUtils.isCurrentSim(mContext, gc.getSimId())
                    && (NmsIpMessageApiNative.nmsGetActivationStatus(gc.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)) {
                menu.setGroupVisible(R.id.InsertQuickTextP, true);
            } else {
                menu.setGroupVisible(R.id.InsertQuickTextP, false);
            }

        } else {
            menu.setGroupVisible(R.id.callP, true);
            menu.setGroupVisible(R.id.infoP, false);
            if (mContact.getType() == NmsContactType.HISSAGE_USER) {
                menu.setGroupVisible(R.id.inviteToChatP, true);
                menu.setGroupVisible(R.id.inviteToISMSP, false);
            } else if (mContact.getType() == NmsContactType.NOT_HISSAGE_USER) {
                menu.setGroupVisible(R.id.inviteToChatP, false);
                menu.setGroupVisible(R.id.inviteToISMSP, false);
                if (NmsCommonUtils.isCurrentSimcardActivated(mContext)) {
                    menu.setGroupVisible(R.id.inviteToISMSP, true);
                }
            }
            menu.setGroupVisible(R.id.editGroupP, false);
            menu.setGroupVisible(R.id.leaveGroupP, false);
            menu.setGroupVisible(R.id.clearChatHistoryP, false);
        }
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        
        if(NmsConfig.isAndroidKitKatOnward){
          if(!NmsSMSMMSManager.isDefaultSmsApp()){
             Toast.makeText(this, getResources().getString(R.string.STR_OPER_FAILED_FOR_NOT_DEFAULT_SMS_APP), Toast.LENGTH_LONG).show();
              return false;
          }
        }
        int id = item.getItemId();
        if (id == android.R.id.home) {
            openConversationList();
            finish();
        } else if (id == R.id.info) {
            viewGroupChat();
        } else if (id == R.id.editGroup) {
            editGroupChat();
        } else if (id == R.id.call) {
            MessageUtils.callContact(mContext, mContact.getNumber());
        } else if (id == R.id.inviteToChat) {
            inviteFriendsToChat();
        } else if (id == R.id.selectMessage) {
            setMarkState(true);
        } else if (id == R.id.markAsSpam) {
            markAsSpam(true);
        } else if (id == R.id.removeSpam) {
            markAsSpam(false);
        } else if (id == R.id.ViewAllMedia) {
            Intent intent = new Intent(mContext, NmsAllMediaActivity.class);
            intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mContactId);
            startActivity(intent);
        } else if (id == R.id.ViewAllLocation) {
            Intent intent = new Intent(mContext, NmsAllLocationsActivity.class);
            intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mContactId);
            startActivity(intent);
        } else if (id == R.id.ChatSetting) {
            long threadId = NmsCreateSmsThread.getOrCreateThreadId(mContext, mContact.getNumber());
            NmsStartActivityApi.nmsStartChatSettingsActivity(mContext, threadId);
        } else if (id == R.id.leaveGroup) {
            MessageUtils.showLeaveGroupDialog(mContext, mContactId, false);
        } else if (id == R.id.clearChatHistory) {
            MessageUtils.clearChatHistory(mContext, mContactId);
        } else if (id == R.id.discard) {
            mBottomPanel.clearText();
            mBottomPanel.clearAttach();
        } else if (id == R.id.InsertQuickText) {
            showQuickTextDialog();
        } else {
            NmsLog.error(TAG, "Menu click not handle");
        }
        return super.onMenuItemSelected(featureId, item);
    }

    private void inviteFriendsToChat() {
        final int currentSimId = (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId();
        if (currentSimId > 0) {
            SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(currentSimId);
            if (info != null) {
                if (NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED != info.status) {
                    NmsAlertDialogUtils.showDialog(this, R.string.STR_NMS_ISMS_ENABLE_TITLE,
                            R.drawable.ic_dialog_alert_holo_light,
                            R.string.STR_NMS_ISMS_ENABLE_CONTENT, R.string.STR_NMS_ENABLE,
                            R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {

                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    // TODO Auto-generated method stub
                                    NmsIpMessageApiNative.nmsEnableIpService(currentSimId);
                                }
                            }, null);
                } else {
                    String[] contacts = { String.valueOf(mContactId) };
                    NmsStartActivityApi.nmsStartNewGroupChatActivity(mContext, contacts,
                            currentSimId);
                }
            } else {

            }

        } else {

        }
    }

    private int getSelectCount() {
        int count = 0;
        if (mMarkAllState) {
            short[] id = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(mContactId,
                    NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL, false);
            count = id.length - SelectRecordIdList.get().getSelectCount();
        } else {
            count = SelectRecordIdList.get().getSelectCount();
        }
        return count;
    }

    private void updateSelectCount() {
        int count = getSelectCount();
        mChatSelect.setText(String.format(getString(R.string.STR_NMS_SELECTED), count));
        if (mSelectMode != null) {
            mSelectMode.invalidate();
        }
    }

    private void markAsSpam(boolean spamFlag) {
        NmsLog.trace(TAG, "click spam contact");
        short[] ids = { mContactId };
        if (spamFlag) {
            NmsIpMessageApiNative.nmsAsyncAddContactToSpamList(ids);
        } else {
            NmsIpMessageApiNative.nmsAsyncDeleteContactFromSpamList(ids);
        }

        // mDialog = new ProgressDialog(mContext);
        // mDialog.setMessage(getString(R.string.STR_NMS_ENABLE_PROCESS));
        // mDialog.setCancelable(false);
        // mDialog.show();
    }

    private void setMarkState(boolean flag) {
        NmsLog.trace(TAG, "click set mark state, state is: " + flag);
        mMarkState = flag;
        mChatListAdapter.setMarkState(flag);
        if (flag) {
            SelectActionMode mSelectActionMode = new SelectActionMode();
            mSelectMode = startActionMode(mSelectActionMode);
            mBottomPanel.hiden();
            mBottomPanel.setVisibility(View.GONE);
            mSwitchPanel.setVisibility(View.GONE);
            if (mNeedShowLoadMsg) {
                loadAllMessage();
            }
        } else {
            initBottomPanel(false);
            mChatListAdapter.setMarkAllState(false);
            mMarkAllState = flag;
            mForwardDownloadState = false;
            mBottomPanel.focusBottomEditer();
        }

        SelectRecordIdList.get().clearAll();
        SelectRecordIdList.get().clearDownloadList();
        SelectRecordIdList.get().setImportantCount(0);
        setMessageAsRead();
        refreshAndScrollList();
    }

    private void setSelectAll() {
        NmsLog.trace(TAG, "click set mark all state");
        mMarkAllState = true;
        mChatListAdapter.setMarkAllState(true);
        SelectRecordIdList.get().clearAll();
        int importantCount = engineadapter.get().nmsUIGetSaveMsgCountInContact(mContactId);
        SelectRecordIdList.get().setImportantCount(importantCount);
        updateSelectCount();
        setMessageAsRead();
        onlyRefreshList();
    }

    private void setDeselectAll() {
        mMarkAllState = false;
        mChatListAdapter.setMarkAllState(false);
        SelectRecordIdList.get().clearAll();
        SelectRecordIdList.get().setImportantCount(0);
        updateSelectCount();
        onlyRefreshList();
    }

    private short[] getSelectList() {
        short[] idList = SelectRecordIdList.get().getIdList();
        if (mMarkAllState) {
            int count = getSelectCount();
            short[] idAll = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(mContactId,
                    NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL, false);
            short[] id = new short[count];
            int i = 0, j = 0;
            for (i = 0; i < idAll.length; i++) {
                if (!SelectRecordIdList.get().isContains(idAll[i])) {
                    id[j] = idAll[i];
                    j++;
                }
            }
            return id;
        } else {
            return idList;
        }
    }

    private Runnable deleteMsg = new Runnable() {

        @Override
        public void run() {
            if (!mDeleteImportant
                    && getSelectCount() == SelectRecordIdList.get().getImportantCount()) {
                return;
            }

            mDialog = new ProgressDialog(mContext);
            mDialog.setMessage(getString(R.string.STR_NMS_DELETE_WAITTING));
            mDialog.setCancelable(false);
            mDialog.show();
            NmsIpMessageApiNative.nmsDeleteIpMsg(getSelectList(), mDeleteImportant, true);
            mDeleteImportant = false;
            mSelectMode.finish();
        }
    };

    private void deleteSelectMsg() {
        int selectCount = getSelectCount();
        NmsLog.trace(TAG, "click delete select msg, count is: " + selectCount);
        if (selectCount == 0) {
            return;
        }
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View llNoNotice = inflater.inflate(R.layout.dialog_with_checkbox, null);
        final CheckBox notice = (CheckBox) llNoNotice.findViewById(R.id.NoNotice);
        notice.setText(R.string.STR_NMS_DELETE_IMPORTANT);
        Builder builder = new AlertDialog.Builder(mContext)
                .setIcon(R.drawable.ic_dialog_alert_holo_light)
                .setTitle(R.string.STR_NMS_DELETE_TITLE).setMessage(R.string.STR_NMS_DELETE_MSG)
                .setPositiveButton(R.string.STR_NMS_DELETE, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (notice.isChecked()) {
                            mDeleteImportant = true;
                        }
                        mHandler.post(deleteMsg);
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null);
        if (SelectRecordIdList.get().show()) {
            builder.setView(llNoNotice);
        }
        builder.create().show();
    }

    private boolean forwardMsgIsInMsgDownloalable() {
        SelectRecordIdList.get().clearDownloadList();
        short[] idList = getSelectList();
        for (short id : idList) {
            NmsIpMessage msg = NmsiSMSApi.nmsGetIpMsgInfoViaDbId(id);
            if (msg != null && msg instanceof NmsIpAttachMessage
                    && !(msg instanceof NmsIpLocationMessage)
                    && ((NmsIpAttachMessage) msg).isInboxMsgDownloalable()) {
                SelectRecordIdList.get().addDownloadId(id);
            }
        }
        if (SelectRecordIdList.get().getDownloadCount() > 0) {
            return true;
        }
        return false;
    }

    private void downloadSelectMsg() {
        int index = SelectRecordIdList.get().getDownloadIndex();
        if (index >= 0) {
            short id = SelectRecordIdList.get().getDownloadId(index);
            NmsIpMessage msg = NmsiSMSApi.nmsGetIpMsgInfoViaDbId(id);

            if (msg != null && msg instanceof NmsIpAttachMessage
                    && ((NmsIpAttachMessage) msg).isInboxMsgDownloalable()) {

                if (!NmsDownloadManager.getInstance().nmsIsDownloading(msg.ipDbId, msg.id)) {
                    NmsDownloadManager.getInstance().nmsDownload(msg.ipDbId, 0);
                }

            } else {
                downloadSelectMsgResult(id, 0);
            }
        }
    }

    private boolean downloadSelectMsgResult(short dbId, int code) {
        boolean result = false;
		///M: add for jira-526
        String strTmp;
		/// @}
        if (!mForwardDownloadState) {
            return false;
        }

        result = SelectRecordIdList.get().isDownloadContains(dbId);

        if (code >= 0) {
            SelectRecordIdList.get().removeDownloadId(dbId);
        } else {
            SelectRecordIdList.get().reduceDownloadCount(dbId);
        }
  
  		///M: add for jira-526          
        strTmp = String.format(getString(R.string.STR_NMS_FORWARD_DOWNLOAD_INFO1), mDownloadedIpMessageStepCounter,mUnDownloadedIpMessageCount);
        mDialog.setMessage(strTmp);
		/// @}

        if (SelectRecordIdList.get().getDownloadCount() > 0) {
            downloadSelectMsg();
            return result;
        }

        if (mDialog != null) {
            mDialog.dismiss();
            mDialog = null;
        }
        
        // /M: add for jira-526
        if (mDownloadedIpMessageStepCounterFail == mUnDownloadedIpMessageCount) {
            strTmp = String.format(getString(R.string.STR_NMS_FORWARD_DOWNLOAD_FAILED));

            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE)
                    .setCancelable(true)
                    .setPositiveButton(R.string.STR_NMS_RETRY,
                            new DialogInterface.OnClickListener() {
                                public final void onClick(DialogInterface dialog, int which) {
                                    dialog.dismiss();
                                    mUnDownloadedIpMessageCount = 0;
                                    forwardSelectMsg();
                                }
                            }).setNegativeButton(R.string.STR_NMS_CANCEL, null).setMessage(strTmp)
                    .show();
            return result;
        }

        if (mDownloadedIpMessageStepCounterFail > 0) {
            strTmp = String.format(getString(R.string.STR_NMS_FORWARD_DOWNLOAD_INFO2),
                    mDownloadedIpMessageStepCounterSuccess, mDownloadedIpMessageStepCounterFail);

            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE)
                    .setCancelable(true)
                    .setPositiveButton(R.string.STR_NMS_FORWARD_CONTIUE,
                            new DialogInterface.OnClickListener() {
                                public final void onClick(DialogInterface dialog, int which) {
                                    dialog.dismiss();
                                    mUnDownloadedIpMessageCount = 0;
                                    NmsStartActivityApi
                                            .nmsStartAllContactSelectionActivityForResult(mContext,
                                                    GET_FORWARD_CONTACT);
                                }
                            }).setNegativeButton(R.string.STR_NMS_CANCEL, null).setMessage(strTmp)
                    .show();

            return result;
        }
        /// @}
        
        if (getSelectList().length == SelectRecordIdList.get().getDownloadSize()) {
            Toast.makeText(mContext, R.string.STR_NMS_FORWARD_ALL_FAILED, Toast.LENGTH_SHORT)
                    .show();
        } else {
            NmsStartActivityApi.nmsStartAllContactSelectionActivityForResult(mContext,
                    GET_FORWARD_CONTACT);
        }
        return result;
    }

    private void forwardSelectMsg() {
        // TODO froward selected msg
        if (getSelectList().length == 0) {
            return;
        }

        if (forwardMsgIsInMsgDownloalable()) {
			///M: add for jira-526
            mDownloadedIpMessageStepCounter = 0;
            mDownloadedIpMessageStepCounterSuccess = 0;
            mDownloadedIpMessageStepCounterFail = 0;

            mUnDownloadedIpMessageCount = SelectRecordIdList.get().getDownloadCount();
            if (mDialog == null) {
                String strTmp;
                mForwardDownloadState = true;
                mDialog = new ProgressDialog(mContext);
                mDialog.setCancelable(true);
                mDialog.setTitle(R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE1);
                mDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);

                strTmp = String.format(getString(R.string.STR_NMS_FORWARD_DOWNLOAD_INFO1),
                        mDownloadedIpMessageStepCounter, mUnDownloadedIpMessageCount);
                mDialog.setMessage(strTmp);

                mDialog.setButton(getText(R.string.STR_NMS_CANCEL),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int i) {
                                if (mDialog != null) {
                                    mDialog.dismiss();
                                    mDialog = null;
                                }
                            }
                        });

                downloadSelectMsg();
            }
            mDialog.show();

            /// @}
        } else {
            NmsStartActivityApi.nmsStartAllContactSelectionActivityForResult(mContext,
                    GET_FORWARD_CONTACT);
        }
    }

    private void importantSelectMsg(final boolean importantFlag) {
        short[] id = getSelectList();
        NmsLog.trace(TAG, "click important select msg, count is: " + id.length);
        if (id.length == 0) {
            return;
        }

        if (importantFlag)
            NmsIpMessageApiNative.nmsAddMsgToImportantList(id);
        else
            NmsIpMessageApiNative.nmsDeleteMsgFromImportantList(id);
        // mDialog = new ProgressDialog(mContext);
        // mDialog.setMessage(getString(R.string.STR_NMS_ENABLE_PROCESS));
        // mDialog.setCancelable(false);
        // mDialog.show();
        mSelectMode.finish();
    }

    private void sendLocationMsg(Intent data) {
        if (data == null) {
            NmsLog.error(TAG, "sendLocationMsg intent is null!");
            return;
        }

        // NmsIpLocationMessage msg = (NmsIpLocationMessage) data
        // .getSerializableExtra(NmsIpMessageConsts.NMS_SHARE_LOCATION_DONE);

        NmsIpLocationMessage msg = new NmsIpLocationMessage();

        msg.address = data.getStringExtra(NmsShareLocationDone.NMS_LOCATION_ADDRESS);
        msg.latitude = data.getDoubleExtra(NmsShareLocationDone.NMS_LOCATION_LATITUDE, -1);
        msg.longitude = data.getDoubleExtra(NmsShareLocationDone.NMS_LOCATION_LONGITUDE, -1);
        msg.path = data.getStringExtra(NmsShareLocationDone.NMS_LOCATION_PATH);

        if (NmsConfig.getSendAsSMSFlag() && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
            msg.send(mContact.getNumber(), (int) NmsPlatformAdapter.getInstance(mContext)
                    .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, false);
        } else {
            msg.send(mContact.getNumber(), (int) NmsPlatformAdapter.getInstance(mContext)
                    .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, false);
        }
    }

    private void viewGroupChat() {
        NmsUIApiUtils.viewGroupChatInfo(mContext, mContactId);
    }

    private void editGroupChat() {
        NmsUIApiUtils.editGroupChat(mContext, mContactId);
    }

    private void showQuickTextDialog() {
        final String mmsPackageName = "com.android.mms";
        Context mmsContext = null;
        List<String> quickTextList = new ArrayList<String>();
        try {
            mmsContext = mContext.createPackageContext(mmsPackageName,
                    Context.CONTEXT_IGNORE_SECURITY);
            Cursor cursor = mmsContext.getContentResolver().query(
                    Uri.parse("content://mms-sms/quicktext"), null, null, null, null);

            if (cursor != null) {
                while (cursor.moveToNext()) {
                    quickTextList.add(cursor.getString(1));
                }
                cursor.close();
            }
            Resources res = mmsContext.getResources();

            String[] defaultQuickTexts = res.getStringArray(res.getIdentifier(
                    "default_quick_texts", "array", mmsPackageName));

            String quickTextTitle = res.getString(res.getIdentifier("select_quick_text", "string",
                    mmsPackageName));

            for (int i = 0; i < defaultQuickTexts.length; i++) {
                quickTextList.add(defaultQuickTexts[i]);
            }

            List<Map<String, ?>> entries = new ArrayList<Map<String, ?>>();
            for (String text : quickTextList) {
                HashMap<String, Object> entry = new HashMap<String, Object>();
                entry.put("text", text);
                entries.add(entry);
            }

            final SimpleAdapter qtAdapter = new SimpleAdapter(mContext, entries,
                    R.layout.quick_text_list_item, new String[] { "text" },
                    new int[] { R.id.quick_text });

            AlertDialog.Builder qtBuilder = new AlertDialog.Builder(mContext);
            qtBuilder.setTitle(quickTextTitle);
            qtBuilder.setCancelable(true);
            qtBuilder.setAdapter(qtAdapter, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    // TODO Auto-generated method stub
                    @SuppressWarnings("unchecked")
                    HashMap<String, Object> item = (HashMap<String, Object>) qtAdapter
                            .getItem(which);
                    mBottomPanel.insertQuickText((String) item.get("text"));
                    dialog.dismiss();
                }
            });
            qtBuilder.create().show();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private class SelectActionMode implements ActionMode.Callback {

        @Override
        public boolean onCreateActionMode(final ActionMode mode, Menu menu) {
            ViewGroup v = (ViewGroup) LayoutInflater.from(mContext).inflate(
                    R.layout.chat_select_action_bar, null);
            mode.setCustomView(v);

            mChatSelect = ((Button) v.findViewById(R.id.bt_chat_select));

            mChatSelect.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    PopupMenu popup = new PopupMenu(mContext, v);

                    Menu menu = popup.getMenu();
                    popup.getMenuInflater().inflate(R.menu.select_menu, menu);

                    short[] id = engineadapter.get().nmsUIGetMsgRecrodIdListViaContactId(
                            mContactId, NmsGetContactMsgRecordIdListFlag.NMS_GET_CONTACT_MSG_ALL,
                            false);
                    if (id.length == getSelectCount()) {
                        menu.setGroupVisible(R.id.group_select_all, false);
                        menu.setGroupVisible(R.id.group_select_cancel, true);
                    } else {
                        menu.setGroupVisible(R.id.group_select_all, true);
                        menu.setGroupVisible(R.id.group_select_cancel, false);
                    }

                    popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        public boolean onMenuItemClick(MenuItem item) {
                            int id = item.getItemId();
                            if (id == R.id.menu_select_all) {
                                setSelectAll();
                            } else if (id == R.id.menu_select_cancel) {
                                setDeselectAll();
                            } else {
                                return true;
                            }
                            return false;
                        }
                    });

                    popup.show();
                }
            });
            updateSelectCount();
            getMenuInflater().inflate(R.menu.message_select_menu, menu);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            if (SelectRecordIdList.get().show()) {
                menu.setGroupVisible(R.id.remove_importantP, true);
            } else {
                menu.setGroupVisible(R.id.remove_importantP, false);
            }

            if (getSelectCount() == SelectRecordIdList.get().getImportantCount()) {
                menu.setGroupVisible(R.id.importantP, false);
            } else {
                menu.setGroupVisible(R.id.importantP, true);
            }

            if (getSelectCount() == 0) {
                menu.setGroupEnabled(R.id.deleteP, false);
                menu.setGroupEnabled(R.id.forwardP, false);

            } else {
                menu.setGroupEnabled(R.id.deleteP, true);
                menu.setGroupEnabled(R.id.forwardP, true);

            }
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            int id = item.getItemId();
            if (id == R.id.delete) {
                deleteSelectMsg();
            } else if (id == R.id.forward) {
                forwardSelectMsg();
            } else if (id == R.id.important) {
                importantSelectMsg(true);
            } else if (id == R.id.remove_important) {
                importantSelectMsg(false);
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            mSelectMode = null;
            setMarkState(false);
        }
    }

    public void scrollList(boolean isRefreshList) {
        if (isRefreshList) {
            refreshAndScrollList();
        } else {
            if (mDialog != null && mDialog.isShowing()) {
                return;
            }
            if (mChatList.getTranscriptMode() != ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL) {
                mChatList.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
                if (mChatListAdapter != null)
                    mChatListAdapter.redrawList();
            }
        }
    }

    private void refreshAndScrollList() {
        NmsLog.trace(TAG, "refresh and scroll list");
        if (mDialog != null && mDialog.isShowing()) {
            return;
        }
        mChatList.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
        if (null != mChatListAdapter) {
            if (mNeedShowLoadMsg) {
                mChatListAdapter.setImportantList(NmsImportantList.get().getIdList());
            }
            mChatListAdapter.notifyDataSetChanged();
        }
        if (mMarkAllState) {
            updateSelectCount();
        }
    }

    private void onlyRefreshList() {
        NmsLog.trace(TAG, "only refresh list");
        if (mDialog != null && mDialog.isShowing()) {
            return;
        }
        mChatList.setTranscriptMode(ListView.TRANSCRIPT_MODE_DISABLED);
        if (null != mChatListAdapter) {
            if (mNeedShowLoadMsg) {
                mChatListAdapter.setImportantList(NmsImportantList.get().getIdList());
            }
            mChatListAdapter.notifyDataSetChanged();
        }
        if (mMarkAllState) {
            updateSelectCount();
        }
    }

    public void setMessageAsRead() {
        if (mDialog != null && mDialog.isShowing()) {
            return;
        }
        if (null != mChatListAdapter) {
            mChatListAdapter.setRead();
        }
    }

    private class MessageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            NmsLog.trace(TAG, "get broadcast action is: " + action);
            if (TextUtils.isEmpty(action)) {
                return;
            }
            
            //add for jira-557
            if(NmsIntentStrId.NMS_REG_STATUS.equals(intent.getAction())){
                NmsLog.trace(TAG, "NmsChatDetailsActivity recv regstatus intent: " + intent);
                int regStatus = intent.getIntExtra("regStatus", -1);
                switch (regStatus) {
                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
                    Toast.makeText(getApplicationContext(),R.string.STR_NMS_ENABLE_SUCCESS, Toast.LENGTH_SHORT).show();
                    break;
                }
                return;
            }
            //add end

            if (NmsIpMessageConsts.NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION.equals(action)) {
                SNmsMsgKey msgKey = (SNmsMsgKey) intent.getSerializableExtra(SNmsMsgKey.MsgKeyName);
                if (null == msgKey || msgKey.contactRecId != mContactId) {
                    return;
                }
                NmsPlatformAdapter.getInstance(mContext).CancelNotification(123);
                setMessageAsRead();
                if (mNeedShowLoadMsg) {
                    NmsIpMessage msg = (NmsIpMessage) intent
                            .getSerializableExtra(NmsNewMessageAction.NMS_IP_MESSAGE);
                    if (NmsImportantList.get().getIdList().isEmpty()) {
                        mNeedShowLoadMsg = false;
                    } else {
                        NmsImportantList.get().addElement((short) msg.ipDbId);
                    }
                }
                if (mMarkState) {
                    onlyRefreshList();
                } else {
                    refreshAndScrollList();
                }
                
                if(mChatSettings == null || mChatSettings.mThreadID <= 0){
                    initChatSettings();
                }
            }

            if (NmsIpMessageConsts.NmsRefreshMsgList.NMS_REFRESH_MSG_LIST.equals(action)) {
                refreshAndScrollList();
            }

            if (NmsIpMessageConsts.NmsIpMessageStatus.NMS_MESSAGE_STATUS_ACTION.equals(action)) {
                onlyRefreshList();
            }
            if (NmsIpMessageConsts.NmsIpMessageStatus.NMS_READEDBURN_TIME_ACTION.equals(action)) {
                int time =intent.getIntExtra(NmsIpMessageStatus.NMS_IP_MSG_TIME, 0);
                short ipDbId=intent.getShortExtra(NmsIpMessageStatus.NMS_IP_MSG_IPDB_ID, (short)0);
                int position =intent.getIntExtra(NmsIpMessageStatus.NMS_IP_MSG_POSITION, 0);
                if ((((NmsIpMessage)mChatListAdapter.getItem(position)).flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0){
                    NmsIpImageMessage msg = (NmsIpImageMessage) mChatListAdapter.getItem(position);
                    msg.setCaption(Integer.toString(time));
                    mChatListAdapter.putitem(position,msg);
                    onlyRefreshList();
                }
            }

            if (NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION
                    .equals(action)) {
                int status = intent
                        .getIntExtra(
                                NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                                -2);

                short dbId = (short) intent.getLongExtra(
                        NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID, -1);
                if (status == NmsIpMessageConsts.NmsDownloadAttachStatus.DONE) {
					///M: add for jira-526
                    mDownloadedIpMessageStepCounterSuccess++;
                    mDownloadedIpMessageStepCounter++;                    
                    /// @}
                    downloadSelectMsgResult(dbId, 0);
                    onlyRefreshList();
                } else {
                    if (status == NmsIpMessageConsts.NmsDownloadAttachStatus.FAILED) {
						///M:add for jira-526
                        mDownloadedIpMessageStepCounterFail++;
                        mDownloadedIpMessageStepCounter++;
						/// @}

                        if (!downloadSelectMsgResult(dbId, -1)
                                && !NmsChatLongPressDialog.isForwardDonwload(dbId)) {
                            Toast.makeText(mContext, R.string.STR_NMS_DOWNLOAD_FAILED,
                                    Toast.LENGTH_SHORT).show();
                        }
                        onlyRefreshList();
                    } else {
                        mChatListAdapter.updateDownLoadPercentage();
                    }
                }
            }

            if (NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE.equals(action)) {
                if (mDialog != null && mDialog.isShowing()) {
                    mDialog.dismiss();
                }

                short[] ids = intent.getShortArrayExtra(NmsDelIpMessageAction.NMS_IP_MESSAGE_DB_ID);
                for (short id : ids) {
                    if (NmsDownloadManager.getInstance().nmsIsDownloading(id, 0)) {
                        NmsDownloadManager.getInstance().nmsCancelDownload(id, 0);
                    }
                }
                refreshAndScrollList();
            }

            if (NmsIpMessageConsts.NmsUpdateGroupAction.NMS_UPDATE_GROUP.equals(action)) {
                int groupId = intent.getIntExtra(
                        NmsIpMessageConsts.NmsUpdateGroupAction.NMS_GROUP_ID, -1);
                if (groupId == mContactId) {
                    initialize(mContactId);
                    invalidateOptionsMenu();
                    mChatListAdapter.updateContact();
                    onlyRefreshList();
                }
            }

            if (NmsSimStatus.NMS_SIM_STATUS_ACTION.equals(action)) {
                int status = intent.getIntExtra(NmsSimStatus.NMS_SIM_STATUS, -1);
                if (status == NMS_ENG_MSG_ENABLE_SIM) {
                    if (mDialog != null && mDialog.isShowing()) {
                        mDialog.dismiss();
                    }
                    initBottomPanel(false);
                    invalidateOptionsMenu();
                } else {

                }
            }

            if (NmsIntentStrId.NMS_INTENT_UPDATE_SYS_MSG_DONE.equals(action)) {
                if (mDialog != null && mDialog.isShowing()) {
                    mDialog.dismiss();
                }
                initialize(mContactId);
                invalidateOptionsMenu();
                onlyRefreshList();
            }

            if (SIM_CHANGE.equals(action)) {
                NmsPlatformAdapter.getInstance(mContext).hideSIMIndicator(mComponentName);
                NmsPlatformAdapter.getInstance(mContext).showSIMIndicator(mComponentName);
                initialize(mContactId);
                invalidateOptionsMenu();
            }
        }

    }

    private Runnable updateTitleAndWallPaper = new Runnable() {
        public void run() {
            synchronized (updateTitleAndWallPaper) {
                invalidateOptionsMenu();
                if (null != mChatSettings && mChatSettings.isMute()) {
                    mMuteLogo.setVisibility(View.VISIBLE);
                } else {
                    mMuteLogo.setVisibility(View.GONE);
                }

                if (mChatSettings != null && mChatSettings.mContactId > 0) {
                    Uri uri = mChatSettings.nmsGetChatWallpaper();
                    if (null != uri) {
                        String path = uri.getEncodedPath();
                        Bitmap bt = NmsBitmapUtils.getBitmapByPath(path,
                                NmsBitmapUtils.getOptions(path), mDisplay.widthPixels,
                                mDisplay.heightPixels);
                        if (null != bt) {
                            // Drawable d = new BitmapDrawable(getResources(),
                            // bt);
                            // mChatBg.setBackgroundDrawable(d);
                            mChatBg.setImageBitmap(bt);
                        } else {
                            // mChatBg.setBackgroundColor(getResources().getColor(R.color.chat_list_bg));
                            mChatBg.setImageResource(R.color.chat_list_bg);
                        }
                        return;
                    }
                }
                mChatBg.setImageResource(R.color.chat_list_bg);
            }
        }
    };
    
    private void initChatSettings(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                // TODO Auto-generated method stub
                NmsChatSettings chatSetting = new NmsChatSettings(mContext, mAddress, mContactId);
                synchronized (updateTitleAndWallPaper) {
                    mChatSettings = chatSetting;
                    mHandler.post(updateTitleAndWallPaper);
                }
            }
        }).start();
    }
}
