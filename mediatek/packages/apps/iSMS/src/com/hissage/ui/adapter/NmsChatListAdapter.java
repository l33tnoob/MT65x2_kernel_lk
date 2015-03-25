package com.hissage.ui.adapter;

import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.R.integer;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.Handler;
import android.text.TextUtils;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsiSMSApi;
import com.hissage.contact.NmsContact;
import com.hissage.jni.engineadapter;
import com.hissage.jni.engineadapter.msgtype;
import com.hissage.message.ip.NmsHesineApiConsts;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.ui.view.NmsMessageItemView;
import com.hissage.ui.view.NmsMessageItemView.tryAllAgainListener;
import com.hissage.util.data.NmsImportantList;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;
import com.hissage.util.message.SmileyParser;
import com.mediatek.mms.ipmessage.message.IpImageMessage;

public class NmsChatListAdapter extends BaseAdapter {

    private Context mContext;
    private LayoutInflater inflater;
    private boolean mMarkState = false;
    private boolean mMarkAllState = false;
    private int mCount;
    private short mContactId;
    public SNmsImMsgCountInfo countInfo;
    private NmsContact mContact;
    private Handler mHandler;

    public final static short LOAD_ALL_MESSAGE_ID = -2;

    private String Tag = "NmsChatListAdapter";

    private ArrayList<Short> mImportantIdList = null;
    private final LruCache<Long, NmsIpMessage> mMessageItemCach;

    public final static int INCOMING_TEXT_ITEM = 0;
    public final static int OUTGOING_TEXT_ITEM = 1;
    public final static int INCOMING_LOC_ITEM = 2;
    public final static int OUTGOING_LOC_ITEM = 3;
    public final static int INCOMING_IMG_ITEM = 4;
    public final static int OUTGOING_IMG_ITEM = 5;
    public final static int INCOMING_VCARD_ITEM = 6;
    public final static int OUTGOING_VCARD_ITEM = 7;
    public final static int INCOMING_AUDIO_ITEM = 8;
    public final static int OUTGOING_AUDIO_ITEM = 9;
    public final static int INCOMING_VIDEO_ITEM = 10;
    public final static int OUTGOING_VIDEO_ITEM = 11;
    public final static int GROUP_CREATE_ITEM = 12;
    public final static int LOAD_ALL_MESSAGE = 13;
    public final static int INCOMING_LARGEEMO_ITEM = 14;
    public final static int OUTGOING_LARGEEMO_ITEM = 15;
    public final static int INCOMING_DYNAMICEMO_ITEM = 16;
    public final static int OUTGOING_DYNAMICEMO_ITEM = 17;
    public final static int INCOMING_ADEMO_ITEM = 18;
    public final static int OUTGOING_ADEMO_ITEM = 19;
    public final static int INCOMING_XMEMO_ITEM = 20;
    public final static int OUTGOING_XMEMO_ITEM = 21;
    public final static int INCOMING_CALENDAR_ITEM = 22;
    public final static int OUTGOING_CALENDAR_ITEM = 23;
    public final static int INCOMING_READBURN_ITEM = 24;
    public final static int OUTGOING_READBURN_ITEM = 25;
    public final static int VIEW_TYPE_COUNT = 26;

    private final static int TIME_DIVIDER_NONE = 0;
    private final static int TIME_DIVIDER_HEAD = 1;
    private final static int TIME_DIVIDER_MIDDLE = 2;
    private final static int TIME_DIVIDER_BOTTOM = 3;

    private final static int UNREAD_DIVIDER_NONE = 0;
    private final static int UNREAD_DIVIDER_HEAD = 1;
    private final static int UNREAD_DIVIDER_MIDDLE = 2;
    private final static int UNREAD_DIVIDER_BOTTOM = 3;

    private final static int GROUP_CFG_DIVIDER_NONE = 0;
    private final static int GROUP_CFG_DIVIDER_HEAD = 1;
    private final static int GROUP_CFG_DIVIDER_MIDDLE = 2;
    private final static int GROUP_CFG_DIVIDER_BOTTOM = 3;

    private final static int UNREAD_DIVIDER_INVALID_INDEX = -1;

    private int mUreadDeviderPos = UNREAD_DIVIDER_INVALID_INDEX;
    private boolean isEvictAll=true;
    /// M: add for readedburn updata time @{
    private Map<Integer,NmsIpMessage> mReadedburnTimeLock;
    /// @}

    private class ItemTypeInfo {
        public int mTimeDividerType;
        public int mUnreadDividerType;
        public int mGroupCfgDividerType;
    }

    private final static int[] layoutList = { R.layout.text_in_coming_item,
            R.layout.text_out_going_item, R.layout.location_in_coming_item,
            R.layout.location_out_going_item, R.layout.img_in_coming_item,
            R.layout.img_out_going_item, R.layout.vcard_in_coming_item,
            R.layout.vcard_out_going_item, R.layout.audio_in_coming_item,
            R.layout.audio_out_going_item, R.layout.video_in_coming_item,
            R.layout.video_out_going_item, R.layout.create_group_item, R.layout.load_all_message,
            R.layout.largeemo_in_coming_item, R.layout.largeemo_out_going_item,
            R.layout.dynamicemo_in_coming_item, R.layout.dynamicemo_out_going_item,
            R.layout.ademo_in_coming_item, R.layout.ademo_out_going_item,
            R.layout.xmemo_in_coming_item, R.layout.xmemo_out_going_item,
            R.layout.calendar_in_coming_item, R.layout.calendar_out_going_item ,
            R.layout.readedburn_in_coming_item,R.layout.readenburn_out_going_item};

    public NmsChatListAdapter(Context context, short contactId, ArrayList<Short> importantList) {
        mContext = context;
        inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mMessageItemCach = new LruCache<Long, NmsIpMessage>(50);
        mContactId = contactId;
        mContact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(mContactId);
        countInfo = NmsiSMSApi.nmsSetImMode((int) mContactId,
                NmsHesineApiConsts.NmsImFlag.NMS_IM_FLAG_ALL,
                NmsHesineApiConsts.NmsImReadMode.NMS_IM_READ_MODE_ALL);
        if (importantList == null) {
            mCount = countInfo.allMsgCount;
        } else {
            mImportantIdList = importantList;
            mCount = importantList.size();
        }
        /// M: add for readedburn updata time @{
        mReadedburnTimeLock = new HashMap<Integer,NmsIpMessage>();
        /// @}

        initUnReadDeviderPos();
    }
    
    public void updateContact(){
        mContact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(mContactId);
    }

    @Override
    public int getCount() {
        return mCount;
    }

    @Override
    public Object getItem(int arg0) {
        if (arg0 >= getCount() || arg0 < 0) {
            return null;
        }
        long position;
        if (mImportantIdList != null) {
            if (mImportantIdList.isEmpty()) {
                position = arg0;
            } else {
                position = mImportantIdList.get(arg0);
            }
        } else {
            position = arg0;
        }
        NmsIpMessage msg = mMessageItemCach.get(position);
        if (null == msg) {
            if (null != mImportantIdList && !mImportantIdList.isEmpty()) {
                if (position == LOAD_ALL_MESSAGE_ID) {
                    msg = new NmsIpMessage();
                    msg.id = LOAD_ALL_MESSAGE_ID;
                } else {
                    msg = NmsiSMSApi.nmsGetIpMsgInfoViaDbId((short) position);
                }
            } else {
                NmsIpSessionMessage session = NmsiSMSApi.nmsGetMessage((int) position);
                if (null == session) {
                    NmsLog.error(Tag, "get session failed, position is: " + position);
                    return null;
                }
                msg = session.ipMsg;
            }
            if (null == msg) {
                NmsLog.error(Tag, "get msg failed, position is: " + position);
                return null;
            }
            mMessageItemCach.put(position, msg);
        }
        return msg;
    }
    public void putitem(long position,NmsIpMessage msg){
        mReadedburnTimeLock.put((int)position,msg);
        isEvictAll=false;
        mMessageItemCach.put(position, msg);
    }

    @Override
    public long getItemId(int position) {
        NmsIpMessage msg = (NmsIpMessage) getItem(position);
        if (null == msg) {
            return -1;
        }
        return msg.ipDbId;
    }

    public void setImportantList(ArrayList<Short> list) {
        if (null != list && !list.isEmpty()) {
            mImportantIdList = list;
        } else {
            mImportantIdList = null;
        }
    }

    public void setLoadAllMessage() {
        if (mImportantIdList != null) {
            mImportantIdList.clear();
        }
    }

    @Override
    public int getViewTypeCount() {
        return VIEW_TYPE_COUNT;
    }

    public void setRead() {
        mUreadDeviderPos = UNREAD_DIVIDER_INVALID_INDEX;
        engineadapter.get().nmsUISetContactMsgReaded(mContactId);
        mUreadDeviderPos = UNREAD_DIVIDER_INVALID_INDEX;
    }

    @Override
    public void notifyDataSetChanged() {
        if(isEvictAll){
        mMessageItemCach.evictAll();
        }
        if(mMarkAllState || mMarkState){
            countInfo = NmsiSMSApi.nmsSetImMode((int) mContactId,
                    NmsHesineApiConsts.NmsImFlag.NMS_IM_FLAG_ALL,
                    NmsHesineApiConsts.NmsImReadMode.NMS_IM_READ_MODE_SELECT);
        }else{
            countInfo = NmsiSMSApi.nmsSetImMode((int) mContactId,
                    NmsHesineApiConsts.NmsImFlag.NMS_IM_FLAG_ALL,
                    NmsHesineApiConsts.NmsImReadMode.NMS_IM_READ_MODE_ALL);
        }
        if (null != mImportantIdList) {
            if (countInfo.allMsgCount == 0) {
                mImportantIdList.clear();
                NmsImportantList.get().clearAll();
                mCount = 0;
            } else {
                if (mImportantIdList.isEmpty()) {
                    mCount = countInfo.allMsgCount;
                } else {
                    mCount = mImportantIdList.size();
                }
            }
        } else {
            mCount = countInfo.allMsgCount;
        }
        super.notifyDataSetChanged();
    }
    
    public void redrawList() {
        super.notifyDataSetChanged();
    }

    @Override
    public int getItemViewType(int position) {
        NmsIpMessage msg = (NmsIpMessage) getItem(position);
        return getViewType(msg);
    }

    private int getViewType(NmsIpMessage msg) {
        int type = 0;
        if (null == msg) {
            return INCOMING_TEXT_ITEM;
        }
        if (msg.id == LOAD_ALL_MESSAGE_ID) {
            return LOAD_ALL_MESSAGE;
        }
        if (msg.status == NmsIpMessageConsts.NmsIpMessageStatus.INBOX) {
            if (msg.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                String body = ((NmsIpTextMessage) msg).body;
                if (!TextUtils.isEmpty(body) && SmileyParser.getInstance().getLargeRes(body) != 0) {
                    type = INCOMING_LARGEEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getDynamicRes(body) != 0) {
                    type = INCOMING_DYNAMICEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getAdRes(body) != 0) {
                    type = INCOMING_ADEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getXmRes(body) != 0) {
                    type = INCOMING_XMEMO_ITEM;
                } else {
                    type = INCOMING_TEXT_ITEM;
                }
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                type = INCOMING_LOC_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE
                    || msg.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                if ((msg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                    type = INCOMING_READBURN_ITEM;
                }else{
                    type = INCOMING_IMG_ITEM;
                }
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VCARD) {
                type = INCOMING_VCARD_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
                type = INCOMING_AUDIO_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
                type = INCOMING_VIDEO_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.CALENDAR) {
                type = INCOMING_CALENDAR_ITEM;
            }
        } else {
            if (msg.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                String body = ((NmsIpTextMessage) msg).body;
                if (!TextUtils.isEmpty(body) && SmileyParser.getInstance().getLargeRes(body) != 0) {
                    type = OUTGOING_LARGEEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getDynamicRes(body) != 0) {
                    type = OUTGOING_DYNAMICEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getAdRes(body) != 0) {
                    type = OUTGOING_ADEMO_ITEM;
                } else if (!TextUtils.isEmpty(body)
                        && SmileyParser.getInstance().getXmRes(body) != 0) {
                    type = OUTGOING_XMEMO_ITEM;
                } else {
                    type = OUTGOING_TEXT_ITEM;
                }
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
                type = OUTGOING_LOC_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE
                    || msg.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                if ((msg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                    type = OUTGOING_READBURN_ITEM;
                }else{
                    type = OUTGOING_IMG_ITEM;
                }
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VCARD) {
                type = OUTGOING_VCARD_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
                type = OUTGOING_AUDIO_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
                type = OUTGOING_VIDEO_ITEM;
            } else if (msg.type == NmsIpMessageConsts.NmsIpMessageType.CALENDAR) {
                type = OUTGOING_CALENDAR_ITEM;
            }
        }

        if (msg.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG
                || msg.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG
                || msg.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG) {
            type = GROUP_CREATE_ITEM;
        }
        return type;
    }

    private int getLayoutResIdViaType(int type) {
        if (type < 0 || type > layoutList.length) {
            NmsLog.error(Tag, "invalid display type:" + type);
            return 0;
        }
        return layoutList[type];
    }

    private boolean msgIsShowTimeDevider(NmsIpMessage curMsg, NmsIpMessage preMsg) {
        if (curMsg == null && preMsg == null) {
            NmsLog.error(Tag, "msgIsShowTime: error that curMsg and preMsg is null");
            return false;
        }

        if (curMsg == null)
            return false;

        if (preMsg == null
                || MessageUtils.shouldShowTimeDivider((long) curMsg.time * 1000,
                        (long) preMsg.time * 1000))
            return true;

        return false;
    }

    private boolean msgIsShowReadDevider(NmsIpMessage msg, int pos) {
        return msg != null && !msg.read && pos == mUreadDeviderPos;
    }

    private boolean msgIsShowGroupCfgDevider(NmsIpMessage msg) {
        return msg != null && getViewType(msg) == GROUP_CREATE_ITEM;
    }

    private void initUnReadDeviderPos() {
        mUreadDeviderPos = UNREAD_DIVIDER_INVALID_INDEX;
        int foundCount = 0;
        if (countInfo.unreadMsgCount > 0) {
            for (int i = countInfo.allMsgCount - 1; i >= 0; i--) {
                NmsIpMessage msg = (NmsIpMessage) getItem(i);
                if (msg != null && !msg.read) {
                    foundCount++;
                    if (foundCount == countInfo.unreadMsgCount) {
                        mUreadDeviderPos = i;
                        NmsLog.trace(Tag, "found the mUreadDeviderPos: " + mUreadDeviderPos
                                + " unreadCount: " + foundCount);
                        break;
                    }
                }
            }
        }
    }

    private ItemTypeInfo getItemTypeInfo(int pos) {
        ItemTypeInfo info = new ItemTypeInfo();

        NmsLog.trace(Tag, "getItemTypeInfo: try to get item type info for pos: " + pos + " count: "
                + mCount);

        if (pos < 0 || pos >= mCount) {
            NmsLog.error(Tag, "getItemTypeInfo: error that pos is invalid");
            return info;
        }

        NmsIpMessage curMsg = (NmsIpMessage) getItem(pos);

        if (curMsg == null) {
            NmsLog.error(Tag, "getItemTypeInfo: error that ipMsg is null");
            return info;
        }

        int itemType = getViewType(curMsg);

        if (itemType == LOAD_ALL_MESSAGE)
            return info;

        NmsIpMessage preMsg = (NmsIpMessage) getItem(pos - 1);
        NmsIpMessage nextMsg = (NmsIpMessage) getItem(pos + 1);

        // parse time type
        if (msgIsShowTimeDevider(curMsg, preMsg)) {

            if (preMsg != null && preMsg.id == LOAD_ALL_MESSAGE_ID) {
                NmsIpMessage realPreMsg = (NmsIpMessage) getItem(pos - 2);
                if (msgIsShowTimeDevider(curMsg, realPreMsg))
                    info.mTimeDividerType = TIME_DIVIDER_HEAD;
            } else {
                info.mTimeDividerType = TIME_DIVIDER_HEAD;

                if (msgIsShowGroupCfgDevider(preMsg))
                    info.mTimeDividerType = TIME_DIVIDER_BOTTOM;
            }
        }

        // parse unread type
        if (msgIsShowReadDevider(curMsg, pos)) {
            info.mUnreadDividerType = UNREAD_DIVIDER_HEAD;

            if (info.mTimeDividerType == TIME_DIVIDER_NONE) {
                if (msgIsShowGroupCfgDevider(preMsg))
                    info.mUnreadDividerType = UNREAD_DIVIDER_BOTTOM;
            } else if (info.mTimeDividerType == TIME_DIVIDER_HEAD) {
                info.mUnreadDividerType = UNREAD_DIVIDER_BOTTOM;
            } else if (info.mTimeDividerType == TIME_DIVIDER_BOTTOM) {
                info.mTimeDividerType = TIME_DIVIDER_MIDDLE;
                info.mUnreadDividerType = UNREAD_DIVIDER_BOTTOM;
            }
        }

        // parse group cfg type
        if (itemType == GROUP_CREATE_ITEM) {

            boolean nextMergeAble = msgIsShowTimeDevider(nextMsg, curMsg)
                    || msgIsShowGroupCfgDevider(nextMsg) || msgIsShowReadDevider(nextMsg, pos + 1);

            info.mGroupCfgDividerType = GROUP_CFG_DIVIDER_HEAD;

            if (info.mUnreadDividerType != UNREAD_DIVIDER_NONE) {
                NmsLog.error(Tag, "getItemTypeInfo: invalid unread type for group cfg item, type: "
                        + info.mUnreadDividerType);
                info.mUnreadDividerType = UNREAD_DIVIDER_NONE;
            }

            if (info.mTimeDividerType == TIME_DIVIDER_MIDDLE) {
                NmsLog.error(Tag,
                        "getItemTypeInfo: invalid time type: TIME_DIVIDER_MIDDLE for group cfg item");
                info.mTimeDividerType = TIME_DIVIDER_HEAD;
            }

            if (info.mTimeDividerType == TIME_DIVIDER_NONE) {
                if (msgIsShowGroupCfgDevider(preMsg))
                    info.mGroupCfgDividerType = nextMergeAble ? GROUP_CFG_DIVIDER_MIDDLE
                            : GROUP_CFG_DIVIDER_BOTTOM;
            } else if (info.mTimeDividerType == TIME_DIVIDER_HEAD) {
                info.mGroupCfgDividerType = nextMergeAble ? GROUP_CFG_DIVIDER_MIDDLE
                        : GROUP_CFG_DIVIDER_BOTTOM;
            } else if (info.mTimeDividerType == TIME_DIVIDER_BOTTOM) {
                info.mTimeDividerType = TIME_DIVIDER_MIDDLE;
                info.mGroupCfgDividerType = nextMergeAble ? GROUP_CFG_DIVIDER_MIDDLE
                        : GROUP_CFG_DIVIDER_BOTTOM;
            }
        }

        return info;
    }

    private void setItemDeviderType(NmsMessageItemView convertView, ItemTypeInfo info) {

        if (convertView == null) {
            NmsLog.error(Tag, "setItemDeviderType: error that convertView is null");
            return;
        }

        if (info == null) {
            NmsLog.error(Tag, "setItemDeviderType: error that info is null");
            return;
        }

        switch (info.mTimeDividerType) {

        case TIME_DIVIDER_HEAD:
            convertView.showSingleTimeDivider();
            break;

        case TIME_DIVIDER_MIDDLE:
            if (mMarkState || mMarkAllState)
                convertView.showSingleTimeDivider();
            else
                convertView.showMutiTimeDivider();
            break;

        case TIME_DIVIDER_BOTTOM:
            if (mMarkState || mMarkAllState)
                convertView.showSingleTimeDivider();
            else
                convertView.showWithBottomTimeDivider();
            break;

        case TIME_DIVIDER_NONE:
            break;

        default:
            NmsLog.error(Tag, "setItemDeviderType: unknown time type: " + info.mTimeDividerType);
            info.mTimeDividerType = TIME_DIVIDER_NONE;
            break;
        }

        switch (info.mUnreadDividerType) {

        case UNREAD_DIVIDER_HEAD:
            convertView.showSingleUnreadDivider(countInfo.unreadMsgCount);
            break;

        case UNREAD_DIVIDER_BOTTOM:
            convertView.showUnreadWithBottomDivider(countInfo.unreadMsgCount);
            break;

        case UNREAD_DIVIDER_NONE:
            break;

        default:
            NmsLog.error(Tag, "setItemDeviderType: unknown unread type: " + info.mUnreadDividerType);
            info.mUnreadDividerType = UNREAD_DIVIDER_NONE;
            break;
        }

        switch (info.mGroupCfgDividerType) {

        case GROUP_CFG_DIVIDER_HEAD:
            convertView.showSingleGroupDivider();
            break;

        case GROUP_CFG_DIVIDER_MIDDLE:
            convertView.showMutiGroupDivider();
            break;

        case GROUP_CFG_DIVIDER_BOTTOM:
            convertView.showBottomGroupDivider();
            break;

        case GROUP_CFG_DIVIDER_NONE:
            break;

        default:
            NmsLog.error(Tag, "setItemDeviderType: unknown group cfg type: "
                    + info.mGroupCfgDividerType);
            info.mGroupCfgDividerType = GROUP_CFG_DIVIDER_NONE;
            break;
        }

        if (mMarkState || mMarkAllState) {

            if (info.mUnreadDividerType != UNREAD_DIVIDER_NONE)
                convertView.hideUnreadDivider();

            if (info.mGroupCfgDividerType != GROUP_CFG_DIVIDER_NONE)
                convertView.hideGroupDivider();
        }
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        NmsIpMessage msg = (NmsIpMessage) getItem(position);
        /// M: add for readedburn updata time @{
        if(mReadedburnTimeLock.get(position)!=null){
            msg=mReadedburnTimeLock.get(position);
            if(Integer.parseInt(((NmsIpImageMessage)msg).caption)==0){
                mReadedburnTimeLock.remove(position);
            }
        }
        /// @}
        int type = getViewType(msg);
        if (convertView == null) {
            int resId = getLayoutResIdViaType(type);
            if (resId == 0) {
                convertView = inflater.inflate(R.layout.text_in_coming_item, null);
                return convertView;
            }
            convertView = inflater.inflate(resId, null);
            convertView.setTag(convertView);
        } else {
            convertView = (View) convertView.getTag();
        }
        if (null == msg) {
            NmsLog.error(Tag, "get item view failed, invalid msg");
            int count = ((NmsMessageItemView) convertView).getChildCount();
            for (int i = 0; i < count; i++) {
                ((NmsMessageItemView) convertView).getChildAt(i).setVisibility(View.GONE);
            }
            convertView.setVisibility(View.GONE);
            return convertView;
        }
        if (convertView instanceof NmsMessageItemView) {
            ((NmsMessageItemView) convertView).bind(msg, mContact, mMarkState, mMarkAllState, type,
                    countInfo);
            ((NmsMessageItemView) convertView).setTryAllAgainListener(new tryAllAgainListener() {

                @Override
                public void tryAllAgain() {
                    resendAllFailed();
                }
            });
            if (msg.id == LOAD_ALL_MESSAGE_ID) {
                ((NmsMessageItemView) convertView).setHandler(mHandler);
                return convertView;
            }

            setItemDeviderType((NmsMessageItemView) convertView, getItemTypeInfo(position));
            isEvictAll=true;
        }
        return convertView;
    }

    private void resendAllFailed() {
        final ProgressDialog dlg = new ProgressDialog(mContext);
        dlg.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dlg.setMessage(mContext.getString(R.string.STR_NMS_REAEND_ALL));
        dlg.setIndeterminate(false);
        dlg.setCancelable(false);
        dlg.show();
        new Thread() {
            @Override
            public void run() {
                long current = System.currentTimeMillis();
                engineadapter.get().nmsUIResendFailedMsgInContact(
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        mContactId);
                long end = System.currentTimeMillis();
                if (end - current < 2000) {
                    try {
                        sleep(2000 + current - end);
                    } catch (InterruptedException e) {
                        NmsLog.nmsPrintStackTrace(e);
                    }
                }
                dlg.dismiss();
            }
        }.start();

    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public void updateDownLoadPercentage() {
        if (null != mImportantIdList) {
            if (countInfo.allMsgCount == 0) {
                if (null != mImportantIdList) {
                    mImportantIdList.clear();
                    NmsImportantList.get().clearAll();
                }
                mCount = 0;
            } else {
                if (mImportantIdList.isEmpty()) {
                    mCount = countInfo.allMsgCount;
                } else {
                    mCount = mImportantIdList.size();
                }
            }
        } else {
            mCount = countInfo.allMsgCount;
        }
        super.notifyDataSetChanged();
    }

    public void setMarkState(boolean markState) {
        mMarkState = markState;
    }

    public void setMarkAllState(boolean markAllState) {
        mMarkAllState = markAllState;
    }
}
