package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
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
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsiSMSApi;
import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImFlag;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImReadMode;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDelIpMessageAction;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsNewMessageAction;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.struct.SNmsMsgKey;
import com.hissage.struct.SNmsMsgType;
import com.hissage.ui.adapter.NmsAllLocationsListAdapter;
import com.hissage.util.log.NmsLog;

public class NmsAllLocationsActivity extends Activity {

    private final static String TAG = "NmsAllLocationsActivity";

    private final static int HANDLER_DATA_ERROR = 0;
    private final static int HANDLER_DATA_EMPTY = 1;
    private final static int HANDLER_DATA_READY = 2;

    private ListView mListView;
    private NmsAllLocationsListAdapter mListAdapter;
    private ProgressBar mWait;
    private TextView mPrompt;

    private AllLocationsMessageReceiver mReceiver;

    private Context mContext;
    private short mEngineContactId;
    private NmsContact mContact;
    private SNmsImMsgCountInfo mMsgCountInfo;
    private List<NmsIpLocationMessage> mIpLocMsgList;

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
        setContentView(R.layout.all_locations);

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

        mIpLocMsgList = new ArrayList<NmsIpLocationMessage>();
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
        actionBar.setTitle(R.string.STR_NMS_ALL_LOCATIONS_TITLE);
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
        mReceiver = new AllLocationsMessageReceiver();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION);
        filter.addAction(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE);
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
        mListAdapter = new NmsAllLocationsListAdapter(mContext, mEngineContactId, mIpLocMsgList);
        mListView.setAdapter(mListAdapter);
        mListView.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                Intent intent = new Intent(mContext, NmsAllMediaDetailsActivity.class);
                NmsIpLocationMessage msg = (NmsIpLocationMessage) mListAdapter.getItem(arg2);
                intent.putExtra("type", 1);
                intent.putExtra("ipDbId", msg.ipDbId);
                mContext.startActivity(intent);
            }
        });
        mWait.setVisibility(View.GONE);
        mPrompt.setText(R.string.STR_NMS_ALL_MEDIA_EMPTY);
    }

    private void sortList() {
        if (mIpLocMsgList == null) {
            NmsLog.error(TAG, "ipLocMsgList is null");
            return;
        }

        if (mIpLocMsgList.size() == 0) {
            NmsLog.warn(TAG, "ipLocMsgList.size() is zero, do not sort");
            return;
        }

        Comparator<NmsIpLocationMessage> timeComparator = new Comparator<NmsIpLocationMessage>() {
            @Override
            public int compare(NmsIpLocationMessage lhs, NmsIpLocationMessage rhs) {
                if (lhs == null || rhs == null) {
                    NmsLog.error(TAG, "lhs or rhs is/are null");
                    return 0;
                }
                return rhs.time - lhs.time;
            }
        };

        Collections.sort(mIpLocMsgList, timeComparator);
    }

    private void delListInfo(short[] ids) {
        if (ids == null) {
            NmsLog.error(TAG, "ids is null");
            return;
        }
        if (mIpLocMsgList == null) {
            NmsLog.error(TAG, "mIpLocMsgList is null");
            return;
        }

        for (int i = 0; i < ids.length; ++i) {
            for (int j = 0; j < mIpLocMsgList.size(); ++j) {
                NmsIpLocationMessage ipLocMsg = mIpLocMsgList.get(j);
                if (ipLocMsg == null) {
                    NmsLog.warn(TAG, "mIpLocMsgList is null, continue");
                    continue;
                }
                if (ipLocMsg.ipDbId == ids[i]) {
                    mIpLocMsgList.remove(j);
                    break;
                }
            }
        }

    }

    private void addListInfo(NmsIpLocationMessage ipLocMsg, boolean isSort) {
        if (ipLocMsg == null) {
            NmsLog.error(TAG, "ipMsg is null");
            return;
        }
        if (mIpLocMsgList == null) {
            NmsLog.error(TAG, "ipLocMsgList is null");
            return;
        }

        mIpLocMsgList.add(ipLocMsg);

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

                for (int i = mMsgCountInfo.allMsgCount - 1; i >= 0; --i) {
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

                    if (ipMsg.type == NmsIpMessageType.LOCATION) {
                        addListInfo((NmsIpLocationMessage) ipMsg, false);
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

    private class AllLocationsMessageReceiver extends BroadcastReceiver {
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
                    NmsLog.trace(TAG, "This message is not the session");
                    return;
                }

                int type = intent.getIntExtra(NmsNewMessageAction.NMS_NEW_MESSAGE_ACTION, -1);
                if (type != SNmsMsgType.NMS_UI_MSG_UPDATE_MSG) {
                    NmsIpMessage ipMsg = (NmsIpMessage) intent
                            .getSerializableExtra(NmsNewMessageAction.NMS_IP_MESSAGE);
                    if (ipMsg.type == NmsIpMessageType.LOCATION) {
                        NmsIpLocationMessage ipLocMsg = NmsIpLocationMessage
                                .formatLocationMsg((NmsIpLocationMessage) ipMsg);
                        addListInfo(ipLocMsg, true);
                        updateListView();
                    }
                }
            } else if (action.equals(NmsDelIpMessageAction.NMS_DEL_IP_MSG_DONE)) {
                short[] ids = intent.getShortArrayExtra(NmsDelIpMessageAction.NMS_IP_MESSAGE_DB_ID);
                delListInfo(ids);
                updateListView();
            }

        }
    }
}
