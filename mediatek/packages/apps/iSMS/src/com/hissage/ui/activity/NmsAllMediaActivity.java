package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsiSMSApi;
import com.hissage.contact.NmsContact;
import com.hissage.imagecache.NmsMessageMediaCache;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImFlag;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImReadMode;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDelIpMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDownloadAttachStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsMessageProtocol;
import com.hissage.message.ip.NmsIpMessageConsts.NmsNewMessageAction;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.struct.SNmsMsgType;
import com.hissage.ui.adapter.NmsAllMediaListAdapter;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsAllMediaActivity extends Activity {

    public class AllMediaIpMessage {
        public int timeType;
        public List<NmsIpMessage> ipMsgList;
    }

    private final static String TAG = "NmsAllMediaActivity";

    private final static int HANDLER_DATA_ERROR = 1000;
    private final static int HANDLER_DATA_EMPTY = 1001;
    private final static int HANDLER_DATA_READY = 1002;

    private ListView mListView;
    private NmsAllMediaListAdapter mListAdapter;
    private ProgressBar mWait;
    private TextView mPrompt;

    private AllMediaMessageReceiver mReceiver;

    private Context mContext;
    private short mEngineContactId;
    private NmsContact mContact;
    private SNmsImMsgCountInfo mMsgCountInfo;
    private List<AllMediaIpMessage> mAllMediaIpMsgList;
    private HashMap<Integer, AllMediaIpMessage> mKeyValue;
    private NmsMessageMediaCache mMsgMediaCache;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            NmsLog.trace(TAG, "handler received msg type: " + msg.what);
            switch (msg.what) {
            case HANDLER_DATA_ERROR:
                doDataError();
                break;
            case HANDLER_DATA_EMPTY:
                doDataEmpty();
                break;
            case HANDLER_DATA_READY:
                doDataReady();
                break;
            default:
                break;
            }
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.all_media);

        init();

        threadLoadListInfo();
    }

    private void init() {
        initFiled();
        initComponents();
        initContactInfo();
        initBroadcastReceiver();
    }

    private void initFiled() {
        mContext = this;
        mKeyValue = new HashMap<Integer, NmsAllMediaActivity.AllMediaIpMessage>();
        mAllMediaIpMsgList = new ArrayList<NmsAllMediaActivity.AllMediaIpMessage>();
        mMsgMediaCache = new NmsMessageMediaCache();
    }

    private void initComponents() {
        initActionBar();
        initListView();
    }

    private void initListView() {
        mWait = (ProgressBar) findViewById(R.id.pb_wait);
        mPrompt = (TextView) findViewById(R.id.tv_prompt);

        mListView = (ListView) findViewById(R.id.lv_list);

        View emptyView = (View) findViewById(R.id.ll_prompt);
        mListView.setEmptyView(emptyView);
    }

    private void initActionBar() {
        ActionBar actionBar = getActionBar();

        actionBar.setDisplayShowHomeEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowTitleEnabled(true);
        actionBar.setDisplayUseLogoEnabled(true);

        actionBar.setLogo(R.drawable.ic_contact_picture);
        actionBar.setTitle(R.string.STR_NMS_ALL_MEDIA_TITLE);
    }

    private void setActionBarLogo(Bitmap logo) {
        if (logo == null) {
            NmsLog.error(TAG, "logo is null");
            return;
        }

        ActionBar actionBar = getActionBar();
        actionBar.setLogo(new BitmapDrawable(logo));
    }

    private void updateActionBarLogo() {
        if (mEngineContactId <= 0) {
            NmsLog.error(TAG, "updateActionBarLogo: mEngineContactId <= 0");
            return;
        }

        Bitmap avatar = NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId(
                mEngineContactId);
        if (avatar != null) {
            setActionBarLogo(avatar);
        }
    }

    private void initContactInfo() {
        Intent intent = getIntent();
        if (null == intent) {
            NmsLog.error(TAG, "initContactInfo. intent is null!");
            return;
        }

        long threadId = intent.getLongExtra("thread_id", 0);
        if (threadId > 0) {
            mContact = NmsIpMessageApiNative.nmsGetContactInfoViaThreadId(threadId);
            if (null != mContact) {
                mEngineContactId = mContact.getId();
            }
        } else {
            mEngineContactId = intent.getShortExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID,
                    (short) -1);
            mContact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(mEngineContactId);
        }

        updateActionBarLogo();
    }

    private void initBroadcastReceiver() {
        mReceiver = new AllMediaMessageReceiver();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
        filter.addAction(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE);
        filter.addAction(NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION);
        registerReceiver(mReceiver, filter);
    }

    private void doDataError() {
        mWait.setVisibility(View.GONE);
        mPrompt.setText(R.string.STR_NMS_ALL_MEDIA_ERROR);
    }

    private void doDataEmpty() {
        mWait.setVisibility(View.GONE);
        mPrompt.setText(R.string.STR_NMS_ALL_MEDIA_EMPTY);
    }

    private void doDataReady() {
        mListAdapter = new NmsAllMediaListAdapter(mContext, mEngineContactId, mAllMediaIpMsgList,
                mMsgMediaCache);
        mListView.setAdapter(mListAdapter);
        mWait.setVisibility(View.GONE);
        mPrompt.setText(R.string.STR_NMS_ALL_MEDIA_EMPTY);
    }

    private void sortList() {
        if (mAllMediaIpMsgList == null) {
            NmsLog.error(TAG, "mAllMediaIpMsgList is null");
            return;
        }

        if (mAllMediaIpMsgList.size() == 0) {
            NmsLog.warn(TAG, "mAllMediaIpMsgList.size() is zero, do not sort");
            return;
        }

        Comparator<AllMediaIpMessage> timeComparator = new Comparator<AllMediaIpMessage>() {
            @Override
            public int compare(AllMediaIpMessage lhs, AllMediaIpMessage rhs) {
                if (lhs == null || rhs == null) {
                    NmsLog.error(TAG, "lhs or rhs is/are null");
                    return 0;
                }
                return lhs.timeType - rhs.timeType;
            }
        };

        Collections.sort(mAllMediaIpMsgList, timeComparator);
    }

    private void delListInfo(short[] ids) {
        if (ids == null) {
            NmsLog.error(TAG, "ids is null");
            return;
        }
        if (mAllMediaIpMsgList == null) {
            NmsLog.error(TAG, "mAllMediaIpMsgList is null");
            return;
        }
        if (mKeyValue == null) {
            NmsLog.error(TAG, "mKeyValue is null");
            return;
        }

        for (int i = 0; i < ids.length; ++i) {
            for (int j = 0; j < mAllMediaIpMsgList.size(); ++j) {
                boolean isDeal = false;
                AllMediaIpMessage allMediaIpMsg = mAllMediaIpMsgList.get(j);
                if (allMediaIpMsg == null) {
                    NmsLog.warn(TAG, "allMediaIpMsg is null, continue");
                    continue;
                }
                List<NmsIpMessage> ipMsgList = allMediaIpMsg.ipMsgList;
                if (ipMsgList == null) {
                    NmsLog.warn(TAG, "ipMsgList is null, continue");
                    continue;
                }
                for (int k = 0; k < ipMsgList.size(); ++k) {
                    NmsIpMessage ipMsg = ipMsgList.get(k);
                    if (ipMsg == null) {
                        NmsLog.warn(TAG, "ipMsg is null, continue");
                        continue;
                    }
                    if (ipMsg.ipDbId == ids[i]) {
                        ipMsgList.remove(k);
                        if (ipMsgList.isEmpty()) {
                            mAllMediaIpMsgList.remove(j);
                            mKeyValue.remove(Integer.valueOf(allMediaIpMsg.timeType));
                        }
                        if (mMsgMediaCache != null) {
                            mMsgMediaCache.removeCache((short) ipMsg.ipDbId);
                        }
                        isDeal = true;
                        break;
                    }
                }
                if (isDeal) {
                    break;
                }
            }
        }

    }

    private void addListInfo(NmsIpMessage ipMsg, boolean isSort) {
        if (ipMsg == null) {
            NmsLog.error(TAG, "ipMsg is null");
            return;
        }
        if (mAllMediaIpMsgList == null) {
            NmsLog.error(TAG, "mAllMediaIpMsgList is null");
            return;
        }

        int thisType = MessageUtils.getAllMediaTimeDividerType((long) ipMsg.time * 1000);

        AllMediaIpMessage amIpMsg = mKeyValue.get(Integer.valueOf(thisType));
        if (amIpMsg == null) {
            amIpMsg = new AllMediaIpMessage();
            amIpMsg.ipMsgList = new ArrayList<NmsIpMessage>();
            amIpMsg.ipMsgList.add(ipMsg);
            amIpMsg.timeType = thisType;

            mAllMediaIpMsgList.add(amIpMsg);

            mKeyValue.put(thisType, amIpMsg);
        } else {
            amIpMsg.ipMsgList.add(ipMsg);
        }

        if (isSort) {
            sortList();
        }
    }

    private void threadLoadListInfo() {
        NmsLog.trace(TAG, "thread to load the list info");

        new Thread() {
            @Override
            public void run() {
                super.run();

                mMsgCountInfo = NmsiSMSApi.nmsSetImMode((int) mEngineContactId,
                        NmsImFlag.NMS_IM_FLAG_ALL, NmsImReadMode.NMS_IM_READ_MODE_ALL);
                if (mMsgCountInfo == null) {
                    NmsLog.error(TAG, "mMsgCountInfo is null. handler send msg, msg type: "
                            + HANDLER_DATA_ERROR);
                    mHandler.sendEmptyMessage(HANDLER_DATA_ERROR);
                    return;
                }

                if (mMsgCountInfo.allMsgCount <= 0) {
                    NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_EMPTY);
                    mHandler.sendEmptyMessage(HANDLER_DATA_EMPTY);
                    return;
                }

                for (int i = 0; i < mMsgCountInfo.allMsgCount; ++i) {
                    NmsIpSessionMessage session = NmsiSMSApi.nmsGetMessage(i);
                    if (session == null) {
                        NmsLog.error(TAG, "session is null! Throw away.");
                        continue;
                    }
                    NmsIpMessage ipMsg = session.ipMsg;
                    if (ipMsg == null) {
                        NmsLog.error(TAG, "ipMsg is null! Throw away.");
                        continue;
                    }

                    boolean isAdd = false;
                    if (ipMsg.protocol == NmsMessageProtocol.MMS) {
                        if (NmsSMSMMSManager.getInstance(NmsAllMediaActivity.this).isMmsDownloaded(ipMsg.id)) {
                            isAdd = true;
                        }
                    } else if (ipMsg.type == NmsIpMessageType.PICTURE
                            || ipMsg.type == NmsIpMessageType.VOICE
                            || ipMsg.type == NmsIpMessageType.SKETCH
                            || ipMsg.type == NmsIpMessageType.VIDEO) {
                        if (!((NmsIpAttachMessage) ipMsg).isInboxMsgDownloalable()) {
                            if ((ipMsg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) == 0) {
                                isAdd = true;
                            }
                        }
                    }

                    if (isAdd) {
                        addListInfo(ipMsg, false);
                    }
                }

                sortList();

                NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_READY);
                mHandler.sendEmptyMessage(HANDLER_DATA_READY);
            }
        }.start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        NmsLog.trace(TAG, "onDestroy");

        unregisterReceiver(mReceiver);

        if (mMsgMediaCache != null) {
            mMsgMediaCache.clearCaches();
            mMsgMediaCache = null;
        }
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {

        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            break;
        default:
            break;
        }

        return super.onMenuItemSelected(featureId, item);
    }

    private void updateListView() {
        mListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_DISABLED);
        if (mListAdapter == null) {
            NmsLog.error(TAG, "mListAdapter is null");
            return;
        }

        // mListAdapter.notifyDataSetChanged();
        NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_READY);
        mHandler.sendEmptyMessage(HANDLER_DATA_READY);
    }

    private void doReceiveIpMsg(NmsIpMessage ipMsg) {
        if (ipMsg == null) {
            NmsLog.error(TAG, "doNewIpMsg. ipMsg == null");
            return;
        }

        if (ipMsg.type == NmsIpMessageType.PICTURE || ipMsg.type == NmsIpMessageType.SKETCH
                || ipMsg.type == NmsIpMessageType.VOICE || ipMsg.type == NmsIpMessageType.VIDEO) {
            if (!((NmsIpAttachMessage) ipMsg).isInboxMsgDownloalable()) {
                if ((ipMsg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) == 0) {
                    addListInfo(ipMsg, true);
                }
                updateListView();
            } else {
                NmsLog.trace(TAG, "this msg is not download.");
            }
        } else if (ipMsg.protocol == NmsMessageProtocol.MMS) {
            if (NmsSMSMMSManager.getInstance(NmsAllMediaActivity.this).isMmsDownloaded(ipMsg.id)) {
                addListInfo(ipMsg, true);
                updateListView();
            } else {
                NmsLog.trace(TAG, "this msg(mms) is not download.");
            }
        } else {
            NmsLog.trace(TAG, "this msg ignore");
        }
    }

    private void doReceiveIpMsg(long msgId) {
        if (msgId <= 0) {
            NmsLog.error(TAG, "doReceiveIpMsg. msgId error. msgId:" + msgId);
        }

        long threadId = NmsSMSMMSManager.getInstance(NmsAllMediaActivity.this).getThreadViaSysMsgId(msgId);
        if (threadId <= 0) {
            NmsLog.error(TAG, "doReceiveIpMsg. threadId error. threadId:" + threadId);
        }

        short id = NmsSMSMMSManager.getInstance(NmsAllMediaActivity.this).getEngineContactIdViaThreadId(threadId);

        if (id == mEngineContactId) {
            NmsIpMessage ipMsg = NmsIpMessageApiNative.nmsGetIpMsgInfo(msgId);
            doReceiveIpMsg(ipMsg);
        } else {
            NmsLog.trace(TAG, "doReceiveIpMsg. this message is not the session, contactRecId=" + id
                    + ", engineContactId=" + mEngineContactId);
        }
    }

    private class AllMediaMessageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            NmsLog.trace(TAG, "get broadcast action is: " + action);
            if (TextUtils.isEmpty(action)) {
                return;
            }

            if (action.equals(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION)) {
                SNmsMsgKey msgKey = (SNmsMsgKey) intent.getSerializableExtra(SNmsMsgKey.MsgKeyName);
                if (null == msgKey || msgKey.contactRecId != mEngineContactId) {
                    NmsLog.trace(TAG, "This message is not the session, contactRecId="
                            + msgKey.contactRecId + ", engineContactId=" + mEngineContactId);
                    return;
                }

                int type = intent.getIntExtra(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION, -1);
                if (type != SNmsMsgType.NMS_UI_MSG_UPDATE_MSG) {
                    NmsIpMessage ipMsg = (NmsIpMessage) intent
                            .getSerializableExtra(NmsNewMessageAction.NMS_IP_MESSAGE);
                    doReceiveIpMsg(ipMsg);
                }
            } else if (action.equals(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE)) {
                short[] ids = intent.getShortArrayExtra(NmsDelIpMessageAction.NMS_IP_MESSAGE_DB_ID);
                delListInfo(ids);
                updateListView();
            } else if (action.equals(NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION)) {
                int status = intent.getIntExtra(
                        NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION, -2);
                if (status == NmsIpMessageConsts.NmsDownloadAttachStatus.DONE) {
                    long msgId = intent.getLongExtra(NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID,
                            -1);
                    doReceiveIpMsg(msgId);
                }
            }

        }
    }
}
