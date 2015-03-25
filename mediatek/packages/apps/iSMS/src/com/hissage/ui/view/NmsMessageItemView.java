package com.hissage.ui.view;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.Browser;
import android.provider.MediaStore.Video.Thumbnails;
import android.telephony.PhoneNumberUtils;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.method.HideReturnsTransformationMethod;
import android.text.style.URLSpan;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.QuickContactBadge;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.download.NmsDownloadManager;
import com.hissage.imagecache.NmsContactAvatarCache;
import com.hissage.imageworker.NmsContactAvatarWorker;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpCalendarMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.struct.SelectRecordIdList;
import com.hissage.ui.activity.NmsQuickContactActivity;
import com.hissage.ui.adapter.NmsChatListAdapter;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageConsts;
import com.hissage.util.message.MessageUtils;
import com.hissage.util.message.SmileyParser;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageStatus;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageType;
import com.mediatek.mms.ipmessage.IpMessageConsts.RemoteActivities;
import com.mediatek.mms.ipmessage.message.IpAttachMessage;
import com.mediatek.mms.ipmessage.message.IpImageMessage;

public class NmsMessageItemView extends LinearLayout {

    private View mStatusLayout;
    private TextView mTime;
    private TextView mSendingText;
    private NmsServiceIndicator mType;
    private ImageView mStatus;
    private ImageView mImportantFlag;
    private TextView mTextContent;
    private LinearLayout mTimeDividerZone;
    private View mTimeDivider;
    private TextView mTimeDividerText;
    private View mTimeLeftDivider;
    private View mTimeRightDivider;
    private View mTimeBottomDivider;
    private CheckBox mMarkBox;
    private LinearLayout mOutGoingBg;
    private TextView mSenderName;
    private ImageView mImageContent;
    private NmsGifView mGifView;
    private ImageView mImageAction;
    private TextView mImageSize;
    private ProgressBar mDownLoadProgress;
    private TextView mCaption;
    private View mSendNameSeparator;
    private QuickContactBadge mSenderPhoto;
    private TextView mContactName;
    private TextView mCalendarTitle;
    private TextView mLocationAddr;
    private ImageView mLocationImg;
    private ImageView mVideoContent;
    private ImageView mVideoAction;
    private TextView mVideoSize;
    private TextView mGroupCfg;
    private LinearLayout mImageSizeBg;
    private View mLeftDivider;
    private View mRightDivider;
    private View mBottomDivider;
    private View mUnreadDivider;
    private View mUnreadLeftDivider;
    private View mUnreadRightDivider;
    private View mUnreadBottomDivider;
    private TextView mUnreadCount;
    private TextView mAudioinfo;
    private ImageView mDownloadAudio;
    private Button mLoadAllMsg;
    private TextView mReadedburnMsg;
    private TextView mReadedburnTime;
    private ImageView mReadedburnIcon;
    private ImageView mBurnAction;
    private TextView mBurnSize;
    private LinearLayout mBurnSizeBg;
    private boolean mMarkState = false;
    private boolean mMarkAllState = false;
    private SNmsImMsgCountInfo mCountInfo;
    private int mDispType;
    private Context mContext;
    private Handler mHandler;
    private reSendDialog mResendDialog;
    public tryAllAgainListener mListener;
    private int mWidth = 0;
    private int mScreenWidth = 0;
    private LruCache<String, NmsContact> mChildCache;
    private NmsContactAvatarWorker mImageWorker;

    private float textSize;
    private String TAG = "MessageItemView";

    private NmsIpMessage mMsgCont;
    private NmsContact mContact;
    
    private final static Map<Integer, CharSequence> mSimInfoMap = new HashMap<Integer, CharSequence>() ;

    private final static float MAX_SCALE = 0.4f;
    private final static float MIN_SCALE = 0.3f;

    private class ItemOption {
        public int actionId;
        public String itemName;
    }

    public NmsMessageItemView(Context context) {
        super(context);
        // TODO Auto-generated constructor stub
        mContext = context;
        textSize = MessageUtils.getMTKPreferenceFontFloat(mContext , 18);
        mImageWorker = new NmsContactAvatarWorker(mContext, R.drawable.ic_contact_picture,
                NmsContactAvatarCache.getInstance());
    }

    public NmsMessageItemView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        textSize = MessageUtils.getMTKPreferenceFontFloat(mContext , 18);
        mImageWorker = new NmsContactAvatarWorker(mContext, R.drawable.ic_contact_picture,
                NmsContactAvatarCache.getInstance());
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mMarkBox = (CheckBox) findViewById(R.id.cb_mark);
        mStatusLayout = findViewById(R.id.status);
        if (mStatusLayout != null) {
            mTime = (TextView) mStatusLayout.findViewById(R.id.tv_time);
            mType = (NmsServiceIndicator) mStatusLayout.findViewById(R.id.tv_type);
            mSendingText = (TextView) mStatusLayout.findViewById(R.id.tv_sending);
            mStatus = (ImageView) mStatusLayout.findViewById(R.id.iv_status);
            mImportantFlag = (ImageView) mStatusLayout.findViewById(R.id.iv_important);
        }
        mTextContent = (TextView) findViewById(R.id.tv_text_content);
        mTimeDividerZone = (LinearLayout) findViewById(R.id.time_divider_zone);
        mTimeDivider = findViewById(R.id.time_divider);
        if (null != mTimeDivider) {
            mTimeDividerText = (TextView) mTimeDivider.findViewById(R.id.tv_time_divider);
            mTimeLeftDivider = mTimeDivider.findViewById(R.id.time_left_divider);
            mTimeRightDivider = mTimeDivider.findViewById(R.id.time_right_divider);
            mTimeBottomDivider = mTimeDivider.findViewById(R.id.time_bottom_divider);
        }
        mOutGoingBg = (LinearLayout) findViewById(R.id.ll_out_going_bg);
        mSenderName = (TextView) findViewById(R.id.tv_sender_name);
        mSendNameSeparator = findViewById(R.id.sender_name_separator);
        mSenderPhoto = (QuickContactBadge) findViewById(R.id.iv_sender_photo);
        mContactName = (TextView) findViewById(R.id.tv_contact_name);
        mCalendarTitle = (TextView) findViewById(R.id.tv_calendar_title);
        mLocationAddr = (TextView) findViewById(R.id.tv_location_addr);
        mLocationImg = (ImageView) findViewById(R.id.iv_location_img);
        mImageContent = (ImageView) findViewById(R.id.iv_image_content);
        mGifView = (NmsGifView) findViewById(R.id.iv_gif_content);
        mImageSize = (TextView) findViewById(R.id.tv_image_size);
        mVideoContent = (ImageView) findViewById(R.id.iv_video_content);
        mVideoSize = (TextView) findViewById(R.id.tv_video_size);
        mDownLoadProgress = (ProgressBar) findViewById(R.id.pb_downLoad_progress);
        mCaption = (TextView) findViewById(R.id.tv_caption);
        mImageAction = (ImageView) findViewById(R.id.ib_image_action);
        mVideoAction = (ImageView) findViewById(R.id.ib_video_action);
        mGroupCfg = (TextView) findViewById(R.id.tv_group_cfg);
        mLeftDivider = findViewById(R.id.left_divider);
        mRightDivider = findViewById(R.id.right_divider);
        mBottomDivider = findViewById(R.id.bottom_divider);
        mAudioinfo = (TextView) findViewById(R.id.tv_audio_info);
        mDownloadAudio = (ImageView) findViewById(R.id.iv_download_audio);
        mReadedburnMsg=(TextView)findViewById(R.id.tv_readedburn_msg);
        mReadedburnTime=(TextView)findViewById(R.id.tv_readedburn_time);
        mReadedburnIcon=(ImageView)findViewById(R.id.iv_readedburn_icon);
        mBurnAction=(ImageView)findViewById(R.id.ib_burn_action);
        mBurnSize=(TextView)findViewById(R.id.tv_burn_size);
        mBurnSizeBg=(LinearLayout)findViewById(R.id.ll_burn_size_bg);

        mImageSizeBg = (LinearLayout) findViewById(R.id.ll_image_size_bg);

        mUnreadDivider = findViewById(R.id.unread_divider);
        if (null != mUnreadDivider) {
            mUnreadLeftDivider = mUnreadDivider.findViewById(R.id.unread_left_divider);
            mUnreadRightDivider = mUnreadDivider.findViewById(R.id.unread_right_divider);
            mUnreadBottomDivider = mUnreadDivider.findViewById(R.id.unread_bottom_divider);
            mUnreadCount = (TextView) mUnreadDivider.findViewById(R.id.tv_unread_divider);
        }

        mLoadAllMsg = (Button) findViewById(R.id.bt_load_all_message);

        mChildCache = new LruCache<String, NmsContact>(10);
    }

    public boolean onMessageListItemLongClick(int position) {
        if (mMsgCont == null) {
            return false;
        }
        if (!mMarkState) {
            if (mMsgCont instanceof NmsIpAttachMessage) {
                if ((mMsgCont.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0
                        && mMsgCont.status == NmsIpMessageStatus.INBOX
                        && mReadedburnTime.getVisibility() == View.VISIBLE) {
                    if (((NmsIpAttachMessage) mMsgCont).isInboxMsgDownloalable()) {
                        if (!NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId,
                                mMsgCont.id)) {
                            NmsDownloadManager.getInstance().nmsDownload(mMsgCont.ipDbId,
                                    mMsgCont.id);
                        }
                    } else {
                        Intent intent = new Intent();
                        intent.setAction(NmsIpMessageConsts.ACTION_READED_BURN_DETAILS);
                        intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                                NmsIpMessageConsts.CLASS_NAME_READED_BURN_DETAILS);
                        intent.putExtra("ipDbId", mMsgCont.ipDbId);
                        intent.putExtra("position", position);
                        intent.putExtra("time",
                                Integer.parseInt(((NmsIpImageMessage) mMsgCont).caption));
                        mContext.startActivity(intent);
                    }
                    return true;
                }
            }
        }
        return false;
    }

    public void onMessageListItemClick() {
        if (mMsgCont == null) {
            return;
        }

        // Check for links. If none, do nothing; if 1, open it; if >1, ask user
        // to pick one

        if (mMarkState) {
            boolean selected = mMarkBox.isChecked();
            mMarkBox.setChecked(!mMarkBox.isChecked());
            if (!selected) {
                setBackgroundResource(R.drawable.list_selected_holo_light);
            } else {
                setBackgroundDrawable(null);
            }
        } else {
            if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED
                    || mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
                if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
                    NmsGroupChatContact gc = (NmsGroupChatContact) mContact;
                    if (!gc.isAlive()
                            || !MessageUtils.isCurrentSim(mContext, gc.getSimId())
                            || !(NmsIpMessageApiNative.nmsGetActivationStatus(gc.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                            || gc.getMemberCount() == 1) {
                    } else {
                        showResendDialog();
                        return;
                    }
                } else {
                    showResendDialog();
                    return;
                }
            }
            if (mMsgCont.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                String body = ((NmsIpTextMessage) mMsgCont).body;
                if (!TextUtils.isEmpty(body)
                        && (SmileyParser.getInstance().getLargeRes(body) != 0
                                || SmileyParser.getInstance().getDynamicRes(body) != 0
                                || SmileyParser.getInstance().getAdRes(body) != 0 || SmileyParser
                                .getInstance().getXmRes(body) != 0)) {
                    return;
                }
                onTextMessageClick();
            }
            if (mMsgCont instanceof NmsIpAttachMessage) {
                if (((NmsIpAttachMessage) mMsgCont).isInboxMsgDownloalable()) {
                    if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId,
                            mMsgCont.id)) {
                        NmsDownloadManager.getInstance().nmsCancelDownload(mMsgCont.ipDbId,
                                mMsgCont.id);
                    } else {
                        NmsDownloadManager.getInstance().nmsDownload(mMsgCont.ipDbId, mMsgCont.id);
                    }
                } else {
                    openMedia();
                }
            }
        }
    }

    private void openMedia() {
        if (mMsgCont.type == NmsIpMessageConsts.NmsIpMessageType.VCARD) {
            NmsIpVCardMessage msg = (NmsIpVCardMessage) mMsgCont;
            if (TextUtils.isEmpty(msg.path)) {
                NmsLog.error(TAG, "open vcard failed, msgid: " + msg.id);
                return;
            }
            if (!NmsCommonUtils.getSDCardStatus()) {
                MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SAVE);
                return;
            }
            if (NmsCommonUtils.getSDcardAvailableSpace() < msg.size) {
                Toast.makeText(mContext, R.string.STR_NMS_SDSPACE_NOT_ENOUGH, Toast.LENGTH_LONG)
                        .show();
                return;
            }

            String dest = NmsCommonUtils.getCachePath(mContext) + "temp"
                    + msg.path.substring(msg.path.lastIndexOf(".vcf"));
            NmsCommonUtils.copy(msg.path, dest);

            File vcardFile = new File(dest);
            Uri vcardUri = Uri.fromFile(vcardFile);
            Intent i = new Intent();
            i.setAction(android.content.Intent.ACTION_VIEW);
            i.setDataAndType(vcardUri, "text/x-vcard");
            mContext.startActivity(i);
        } else if (mMsgCont.type == NmsIpMessageConsts.NmsIpMessageType.CALENDAR) {
            NmsIpCalendarMessage msg = (NmsIpCalendarMessage) mMsgCont;
            if (TextUtils.isEmpty(msg.path)) {
                NmsLog.error(TAG, "open vcard failed, msgid: " + msg.id);
                return;
            }
            if (!NmsCommonUtils.getSDCardStatus()) {
                MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SAVE);
                return;
            }
            if (NmsCommonUtils.getSDcardAvailableSpace() < msg.size) {
                Toast.makeText(mContext, R.string.STR_NMS_SDSPACE_NOT_ENOUGH, Toast.LENGTH_LONG)
                        .show();
                return;
            }

            String dest = NmsCommonUtils.getCachePath(mContext) + "temp"
                    + msg.path.substring(msg.path.lastIndexOf(".vcs"));
            NmsCommonUtils.copy(msg.path, dest);

            File calendarFile = new File(dest);
            Uri calendarUri = Uri.fromFile(calendarFile);
            Intent intent = new Intent();
            intent.setAction(android.content.Intent.ACTION_VIEW);
            intent.setDataAndType(calendarUri, "text/x-vcalendar");
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException e) {
                NmsLog.error(TAG, "can't open calendar");
            }
        } else {
            if ((mMsgCont.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) == 0) {
            Intent intent = new Intent();
            intent.setAction(NmsIpMessageConsts.ACTION_ALL_MEDIA_DETAILS);
            intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                    NmsIpMessageConsts.CLASS_NAME_ALL_MEDIA_DETAILS);
            intent.putExtra("ipDbId", mMsgCont.ipDbId);
            mContext.startActivity(intent);
            // NmsStartActivityApi.nmsStartMediaDetailActivity(mContext,
            // mMsgCont.id);
            }
        }
    }

    private void onTextMessageClick() {
        URLSpan[] spans = mTextContent.getUrls();

        if (spans.length == 0) {
            // Do nothing.
        } else if (spans.length == 1) {
            Uri uri = Uri.parse(spans[0].getURL());
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
            mContext.startActivity(intent);
        } else {
            final java.util.ArrayList<String> urls = MessageUtils.extractUris(spans);

            ArrayAdapter<String> adapter = new ArrayAdapter<String>(mContext,
                    android.R.layout.select_dialog_item, urls) {
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    View v = super.getView(position, convertView, parent);
                    try {
                        String url = getItem(position).toString();
                        TextView tv = (TextView) v;
                        Drawable d = mContext.getPackageManager().getActivityIcon(
                                new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
                        if (d != null) {
                            d.setBounds(0, 0, d.getIntrinsicHeight(), d.getIntrinsicHeight());
                            tv.setCompoundDrawablePadding(10);
                            tv.setCompoundDrawables(d, null, null, null);
                        }
                        final String telPrefix = "tel:";
                        if (url.startsWith(telPrefix)) {
                            url = PhoneNumberUtils.formatNumber(url.substring(telPrefix.length()));
                        }
                        tv.setText(url);
                    } catch (android.content.pm.PackageManager.NameNotFoundException ex) {
                        // it's ok if we're unable to set the drawable for
                        // this
                        // view - the user
                        // can still use it
                    }
                    return v;
                }
            };

            AlertDialog.Builder b = new AlertDialog.Builder(mContext);

            DialogInterface.OnClickListener click = new DialogInterface.OnClickListener() {
                @Override
                public final void onClick(DialogInterface dialog, int which) {
                    if (which >= 0) {
                        Uri uri = Uri.parse(urls.get(which));
                        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                        intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
                        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                        mContext.startActivity(intent);
                    }
                    dialog.dismiss();
                }
            };

            b.setTitle(R.string.STR_NMS_SELECT_ACTION_TITLE);
            b.setCancelable(true);
            b.setAdapter(adapter, click);

            b.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                @Override
                public final void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });

            b.show();
        }
    }

    public void bind(NmsIpMessage msg, NmsContact contact, boolean markState, boolean markAllState,
            int dispType, SNmsImMsgCountInfo countInfo) {
        if (null == msg || null == contact) {
            NmsLog.error(TAG, "bind msg failed, invalid msg or contact");
            return;
        }
        setLongClickable(false);
        DisplayMetrics dm = new DisplayMetrics();
        WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        wmg.getDefaultDisplay().getMetrics(dm);
        if (dm.heightPixels > dm.widthPixels) {
            mWidth = dm.widthPixels;
        } else {
            mWidth = dm.heightPixels;
        }
        mScreenWidth = dm.widthPixels;

        mMarkState = markState;
        mMarkAllState = markAllState;
        mMsgCont = msg;
        mContact = contact;
        mCountInfo = countInfo;
        mDispType = dispType;

        if (dispType != NmsChatListAdapter.GROUP_CREATE_ITEM
                && dispType != NmsChatListAdapter.LOAD_ALL_MESSAGE) {
            setMarkView();
            mTime.setText(MessageUtils.getShortTimeString(mContext, (long) msg.time * 1000));
            if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED 
                    || mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX
                    || mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX_PENDING) {
                mTime.setVisibility(View.GONE);
            } else {
                mTime.setVisibility(View.VISIBLE);
            }
            
            CharSequence indicator = getSimIndicatorInCache(mContext, mMsgCont.simId);
            if(indicator != null){
                mType.setIndicator(indicator,false);
            }else{
                mType.setIndicator(MessageUtils.getServiceIndiactorName(mContext, mMsgCont.simId,
                        mMsgCont.protocol),true);
                mType.setServiceColor(MessageUtils.getServiceIndicatorColor(mContext,
                        mMsgCont.simId));
            }
            if ((mMsgCont.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) != 0) {
                mImportantFlag.setVisibility(View.VISIBLE);
            } else {
                mImportantFlag.setVisibility(View.GONE);
            }
        }

        if (dispType != NmsChatListAdapter.LOAD_ALL_MESSAGE) {
            if (null != mTimeDividerZone) {
                mTimeDividerZone.setVisibility(View.GONE);
            }
            mTimeDivider.setVisibility(View.GONE);
            if (null != mUnreadDivider) {
                mUnreadDivider.setVisibility(View.GONE);
            }

            mTimeDividerText.setText(MessageUtils.getTimeDividerString(mContext,
                    (long) msg.time * 1000));
        }
        switch (dispType) {
        case NmsChatListAdapter.OUTGOING_TEXT_ITEM: {
            setOutGoingItem();
            setTextItem();
            mTextContent.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
            break;
        }

        case NmsChatListAdapter.INCOMING_TEXT_ITEM: {
            setInComingItem();
            setTextItem();
            mTextContent.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
            break;
        }

        case NmsChatListAdapter.OUTGOING_IMG_ITEM: {
            setOutGoingItem();
            setImageItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_IMG_ITEM: {
            setInComingItem();
            setImageItem();
            break;
        }
        case NmsChatListAdapter.OUTGOING_READBURN_ITEM: {
            setOutGoingItem();
            setBurnItem();
            break;
        }
        
        case NmsChatListAdapter.INCOMING_READBURN_ITEM: {
            setInComingItem();
            setBurnItem();
            break;
        }

        case NmsChatListAdapter.OUTGOING_LOC_ITEM: {
            setOutGoingItem();
            setLocationItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_LOC_ITEM: {
            setInComingItem();
            setLocationItem();
            break;
        }

        case NmsChatListAdapter.OUTGOING_VCARD_ITEM: {
            setOutGoingItem();
            setVcardItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_VCARD_ITEM: {
            setInComingItem();
            setVcardItem();
            break;
        }

        case NmsChatListAdapter.OUTGOING_AUDIO_ITEM: {
            setOutGoingItem();
            setAudioItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_AUDIO_ITEM: {
            setInComingItem();
            setAudioItem();
            break;
        }

        case NmsChatListAdapter.OUTGOING_VIDEO_ITEM: {
            setOutGoingItem();
            setVideoItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_VIDEO_ITEM: {
            setInComingItem();
            setVideoItem();
            break;
        }

        case NmsChatListAdapter.GROUP_CREATE_ITEM: {
            setGroupItem();
            break;
        }

        case NmsChatListAdapter.LOAD_ALL_MESSAGE:
            setLoadAllMessage();
            break;

        case NmsChatListAdapter.INCOMING_LARGEEMO_ITEM: {
            setInComingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getLargeRes(body);
            if (id != 0) {
                mImageContent.setImageResource(id);
            }
            break;
        }

        case NmsChatListAdapter.OUTGOING_LARGEEMO_ITEM: {
            setOutGoingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getLargeRes(body);
            if (id != 0) {
                mImageContent.setImageResource(id);
            }
            break;
        }

        case NmsChatListAdapter.INCOMING_DYNAMICEMO_ITEM: {
            setInComingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getDynamicRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.OUTGOING_DYNAMICEMO_ITEM: {
            setOutGoingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getDynamicRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.OUTGOING_ADEMO_ITEM: {
            setOutGoingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getAdRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.INCOMING_ADEMO_ITEM: {
            setInComingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getAdRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.OUTGOING_XMEMO_ITEM: {
            setOutGoingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getXmRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.INCOMING_XMEMO_ITEM: {
            setInComingItem();
            String body = ((NmsIpTextMessage) mMsgCont).body;
            int id = SmileyParser.getInstance().getXmRes(body);
            if (id != 0) {
                mGifView.setSource(id);
            }
            break;
        }

        case NmsChatListAdapter.OUTGOING_CALENDAR_ITEM: {
            setOutGoingItem();
            setCalendarItem();
            break;
        }

        case NmsChatListAdapter.INCOMING_CALENDAR_ITEM: {
            setInComingItem();
            setCalendarItem();
            break;
        }

        default:
            break;
        }
    }

    private void setLoadAllMessage() {
        mLoadAllMsg.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                Message msg = mHandler.obtainMessage();
                msg.what = MessageConsts.NMS_LOAD_ALL_MESSAGE;
                mHandler.sendMessage(msg);
            }
        });
    }

    private void setMarkView() {
        if (mMarkState) {
            mMarkBox.setVisibility(View.VISIBLE);
            if (mMarkAllState) {
                if (SelectRecordIdList.get().isContains((short) mMsgCont.ipDbId)) {
                    mMarkBox.setChecked(false);
                    setBackgroundDrawable(null);
                } else {
                    mMarkBox.setChecked(true);
                    setBackgroundResource(R.drawable.list_selected_holo_light);
                }
            } else {
                if (SelectRecordIdList.get().isContains((short) mMsgCont.ipDbId)) {
                    mMarkBox.setChecked(true);
                    setBackgroundResource(R.drawable.list_selected_holo_light);
                } else {
                    mMarkBox.setChecked(false);
                    setBackgroundDrawable(null);
                }
            }
        } else {
            mMarkBox.setVisibility(View.GONE);
            setBackgroundDrawable(null);
        }
    }

    private void setOutGoingItem() {
        mStatus.setImageResource(getStatusResourceId(mMsgCont.status));
        if (mMsgCont.protocol == NmsIpMessageConsts.NmsMessageProtocol.IP) {
            mOutGoingBg.setBackgroundDrawable(getResources().getDrawable(
                    R.drawable.outgoing_nms_selector));
        } else {
            mOutGoingBg.setBackgroundDrawable(getResources().getDrawable(
                    R.drawable.outgoing_sms_selector));
        }
        if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX
                || mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX_PENDING) {
            mSendingText.setVisibility(View.VISIBLE);
        } else {
            mSendingText.setVisibility(View.GONE);
        }
    }

    private void setInComingItem() {
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            mSenderName.setVisibility(View.VISIBLE);
            mSendNameSeparator.setVisibility(View.VISIBLE);
            mSenderPhoto.setVisibility(View.VISIBLE);
            NmsContact child = getChildContact(mMsgCont.from);
            if (child != null) {
                String number = child.getNumber();
                String name = child.getName();
                if (TextUtils.isEmpty(name)) {
                    name = TextUtils.isEmpty(number) ? "" : number;
                }
                mSenderName.setText(name);
                // Bitmap avatar =
                // NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId(
                // child.getId());
                // if (null == avatar) {
                // mSenderPhoto.setImageResource(R.drawable.ic_contact_picture);
                // } else {
                // mSenderPhoto.setImageBitmap(avatar);
                // }
                mImageWorker.loadImage(child.getId(), mSenderPhoto);
            }
            if (mMarkState) {
                mSenderPhoto.setClickable(false);
            } else {
                mSenderPhoto.setOnClickListener(new View.OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        // TODO Auto-generated method stub
                        Intent intent = new Intent(mContext, NmsQuickContactActivity.class);
                        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                        intent.putExtra("PHONE_NUM", mMsgCont.from);

                        mContext.startActivity(intent);
                    }
                });
                // mSenderPhoto .assignContactFromPhone(mMsgCont.from, true);
            }
        } else {
            mSenderName.setVisibility(View.GONE);
            mSendNameSeparator.setVisibility(View.GONE);
            mSenderPhoto.setVisibility(View.GONE);
        }
    }

    private void setImageItem() {
        if (((NmsIpImageMessage) mMsgCont).isInboxMsgDownloalable()) {
            mImageAction.setImageResource(R.drawable.isms_chat_download_selector);
            if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId, mMsgCont.id)) {
                mImageAction.setImageResource(R.drawable.isms_chat_stop_selector);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.VISIBLE);
                }
                int progress = NmsDownloadManager.getInstance().nmsGetProgress(mMsgCont.ipDbId,
                        mMsgCont.id);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setProgress(progress);
                }
                mImageSize.setVisibility(View.GONE);
            } else {
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.GONE);
                }
                mImageSize.setVisibility(View.VISIBLE);
                mImageSize
                        .setText(MessageUtils.formatFileSize(((NmsIpImageMessage) mMsgCont).size));
            }
            if (mMarkState) {
                mImageAction.setClickable(false);
            } else {
                mImageAction.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        if (!((NmsIpImageMessage) mMsgCont).isInboxMsgDownloalable() || mMarkState) {
                            return;
                        }
                        if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId,
                                mMsgCont.id)) {
                            NmsDownloadManager.getInstance().nmsCancelDownload(mMsgCont.ipDbId,
                                    mMsgCont.id);
                        } else {
                            NmsDownloadManager.getInstance().nmsDownload(mMsgCont.ipDbId,
                                    mMsgCont.id);
                        }
                    }
                });
            }

            setPicView(((NmsIpImageMessage) mMsgCont).thumbPath, null, mImageContent);
            mImageSizeBg.setVisibility(View.VISIBLE);
        } else {
            setPicView(((NmsIpImageMessage) mMsgCont).path,
                    ((NmsIpImageMessage) mMsgCont).thumbPath, mImageContent);
            // mImageAction.setImageResource(R.drawable.play);
            if (null != mDownLoadProgress) {
                mDownLoadProgress.setVisibility(View.GONE);
            }
            mImageAction.setClickable(false);
            // mImageSize.setVisibility(View.GONE);
            mImageSizeBg.setVisibility(View.GONE);
        }

        if (TextUtils.isEmpty(((NmsIpImageMessage) mMsgCont).caption)) {
            mCaption.setVisibility(View.GONE);
        } else {
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setTextSize(textSize);
            SpannableStringBuilder buf = new SpannableStringBuilder();
            SmileyParser parser = SmileyParser.getInstance();
            buf.append(parser.addSmileySpans(((NmsIpImageMessage) mMsgCont).caption));
            mCaption.setText(buf);
        }
    }

    private void setBurnItem() {
        if (((NmsIpImageMessage) mMsgCont).isInboxMsgDownloalable()) {
            mBurnAction.setImageResource(R.drawable.isms_chat_download_selector);
            if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId, mMsgCont.id)) {
                mBurnAction.setImageResource(R.drawable.isms_chat_stop_selector);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.VISIBLE);
                }
                int progress = NmsDownloadManager.getInstance().nmsGetProgress(mMsgCont.ipDbId,
                        mMsgCont.id);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setProgress(progress);
                }
                mBurnSize.setVisibility(View.GONE);
            } else {
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.GONE);
                }
                mBurnSize.setVisibility(View.VISIBLE);
                mBurnSize
                        .setText(MessageUtils.formatFileSize(((NmsIpImageMessage) mMsgCont).size));
            }
            if (mMarkState) {
                mBurnAction.setClickable(false);
            } else {
                mBurnAction.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        if (!((NmsIpImageMessage) mMsgCont).isInboxMsgDownloalable() || mMarkState) {
                            return;
                        }
                        if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId,
                                mMsgCont.id)) {
                            NmsDownloadManager.getInstance().nmsCancelDownload(mMsgCont.ipDbId,
                                    mMsgCont.id);
                        } else {
                            NmsDownloadManager.getInstance().nmsDownload(mMsgCont.ipDbId,
                                    mMsgCont.id);
                        }
                    }
                });
            }
            mBurnSizeBg.setVisibility(View.VISIBLE);
            SpannableStringBuilder buf = new SpannableStringBuilder();
            SmileyParser parser = SmileyParser.getInstance();
            buf.append(parser.addSmileySpans(((NmsIpImageMessage) mMsgCont).caption));
            if (!TextUtils.isEmpty(((NmsIpImageMessage) mMsgCont).caption)) {
                mReadedburnTime.setVisibility(View.VISIBLE);
                mReadedburnTime.setText(buf + getResources().getString(R.string.STR_NMS_SECOND));
                mReadedburnIcon.setImageResource(R.drawable.isms_share_burn_active);
                mReadedburnMsg.setText(getResources().getString(R.string.STR_NMS_READED_BURN_DOWNLOAD)); 
            }
        } else {
            if (null != mDownLoadProgress) {
                mDownLoadProgress.setVisibility(View.GONE);
            }
            mBurnAction.setClickable(false);
            mBurnSizeBg.setVisibility(View.GONE);

            if (!TextUtils.isEmpty(((NmsIpImageMessage) mMsgCont).caption)&&((NmsIpImageMessage) mMsgCont).status==NmsIpMessageStatus.INBOX) {
                mReadedburnTime.setVisibility(View.VISIBLE);
                SpannableStringBuilder buf = new SpannableStringBuilder();
                SmileyParser parser = SmileyParser.getInstance();
                buf.append(parser.addSmileySpans(((NmsIpImageMessage) mMsgCont).caption));
                if (!setPicView(((NmsIpImageMessage) mMsgCont).path,((NmsIpImageMessage) mMsgCont).thumbPath,null)||buf.toString().equals("0")){
                    mReadedburnMsg.setText(getResources().getString(R.string.STR_NMS_READED_BURN_PIC_DESTROY));
                    mReadedburnTime.setVisibility(View.GONE);
                    mReadedburnIcon.setImageResource(R.drawable.isms_share_burn);
                }else{
                    mReadedburnTime.setVisibility(View.VISIBLE);
                    mReadedburnTime.setText(buf + getResources().getString(R.string.STR_NMS_SECOND));
                    mReadedburnMsg.setText(getResources().getString(R.string.STR_NMS_READED_BURN_CHECK));
                    mReadedburnIcon.setImageResource(R.drawable.isms_share_burn_active);
                }
            } else {
                mReadedburnTime.setVisibility(View.GONE);
            }
        }
    }

    private void setVideoItem() {
        if (((NmsIpVideoMessage) mMsgCont).isInboxMsgDownloalable()) {
            mVideoAction.setImageResource(R.drawable.isms_chat_download_selector);
            if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId, mMsgCont.id)) {
                mVideoAction.setImageResource(R.drawable.isms_chat_stop_selector);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.VISIBLE);
                }
                int progress = NmsDownloadManager.getInstance().nmsGetProgress(mMsgCont.ipDbId,
                        mMsgCont.id);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setProgress(progress);
                }
                mVideoSize.setVisibility(View.GONE);
            } else {
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.GONE);
                }
                mVideoSize.setVisibility(View.VISIBLE);
                mVideoSize
                        .setText(MessageUtils.formatFileSize(((NmsIpVideoMessage) mMsgCont).size));
            }
            mVideoAction.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    if (!((NmsIpVideoMessage) mMsgCont).isInboxMsgDownloalable() || mMarkState) {
                        return;
                    }
                    if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId,
                            mMsgCont.id)) {
                        NmsDownloadManager.getInstance().nmsCancelDownload(mMsgCont.ipDbId,
                                mMsgCont.id);
                    } else {
                        NmsDownloadManager.getInstance().nmsDownload(mMsgCont.ipDbId, mMsgCont.id);
                    }
                }
            });
            setVideoView(null, ((NmsIpVideoMessage) mMsgCont).thumbPath);
            // setPicView(((NmsIpVideoMessage) mMsgCont).thumbPath, null,
            // mVideoContent);
            mImageSizeBg.setVisibility(View.VISIBLE);
        } else {
            setVideoView(((NmsIpVideoMessage) mMsgCont).path,
                    ((NmsIpVideoMessage) mMsgCont).thumbPath);
            if (null != mDownLoadProgress) {
                mDownLoadProgress.setVisibility(View.GONE);
            }
            mImageSizeBg.setVisibility(View.GONE);
        }

        if (TextUtils.isEmpty(((NmsIpVideoMessage) mMsgCont).caption)) {
            mCaption.setVisibility(View.GONE);
        } else {
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setTextSize(textSize);
            SpannableStringBuilder buf = new SpannableStringBuilder();
            SmileyParser parser = SmileyParser.getInstance();
            buf.append(parser.addSmileySpans(((NmsIpVideoMessage) mMsgCont).caption));
            mCaption.setText(buf);
        }
    }

    private void setAudioItem() {
        if (((NmsIpVoiceMessage) mMsgCont).isInboxMsgDownloalable()) {
            if (mDownloadAudio != null) {
                mDownloadAudio.setVisibility(View.GONE);
                mDownloadAudio.setImageResource(R.drawable.isms_chat_download_selector);
            }
            if (NmsDownloadManager.getInstance().nmsIsDownloading(mMsgCont.ipDbId, mMsgCont.id)) {
                if (mDownloadAudio != null) {
                    mDownloadAudio.setImageResource(R.drawable.isms_chat_stop_selector);
                }
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.VISIBLE);
                }
                int progress = NmsDownloadManager.getInstance().nmsGetProgress(mMsgCont.ipDbId,
                        mMsgCont.id);
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setProgress(progress);
                }
                mAudioinfo.setVisibility(View.GONE);
            } else {
                if (null != mDownLoadProgress) {
                    mDownLoadProgress.setVisibility(View.GONE);
                }
                mAudioinfo.setVisibility(View.VISIBLE);
                mAudioinfo
                        .setText(MessageUtils.formatFileSize(((NmsIpVoiceMessage) mMsgCont).size));
            }
        } else {
            if (mDownloadAudio != null) {
                mDownloadAudio.setVisibility(View.GONE);
            }
            if (null != mDownLoadProgress) {
                mDownLoadProgress.setVisibility(View.GONE);
            }
            mAudioinfo.setVisibility(View.VISIBLE);
            mAudioinfo.setText(MessageUtils
                    .formatAudioTime(((NmsIpVoiceMessage) mMsgCont).durationTime));
        }

        if (TextUtils.isEmpty(((NmsIpVoiceMessage) mMsgCont).caption)) {
            mCaption.setVisibility(View.GONE);
        } else {
            mCaption.setVisibility(View.VISIBLE);
            mCaption.setTextSize(textSize);
            SpannableStringBuilder buf = new SpannableStringBuilder();
            SmileyParser parser = SmileyParser.getInstance();
            buf.append(parser.addSmileySpans(((NmsIpVoiceMessage) mMsgCont).caption));
            mCaption.setText(buf);
        }
    }

    public void setVideoView(String path, String bakPath) {
        Bitmap bp = null;
        int degree = 0;

        if (!TextUtils.isEmpty(path)) {
            bp = ThumbnailUtils.createVideoThumbnail(path, Thumbnails.MICRO_KIND);
            degree = NmsBitmapUtils.getExifOrientation(path);
        }

        if (null == bp) {
            if (!TextUtils.isEmpty(bakPath)) {
                BitmapFactory.Options options = NmsBitmapUtils.getOptions(bakPath);
                options.inJustDecodeBounds = true;
                BitmapFactory.decodeFile(bakPath, options);
                bp = NmsBitmapUtils.getBitmapByPath(bakPath, NmsBitmapUtils.getOptions(bakPath),
                        options.outWidth, options.outHeight);
                degree = NmsBitmapUtils.getExifOrientation(bakPath);
            }
        }
        if (null != bp) {
            if (degree != 0) {
                bp = NmsBitmapUtils.rotate(bp, degree);
            }
            mVideoContent.setImageBitmap(bp);
        } else {
            mVideoContent.setImageResource(R.drawable.all_media_video);
        }
    }

    private boolean setPicView(String path, String bakPath, ImageView v) {
        BitmapFactory.Options options = NmsBitmapUtils.getOptions(path);
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(path, options);
        int width = options.outWidth;
        int height = options.outHeight;
        int w = options.outWidth;

        Bitmap bitmap = null;

        if (width > mWidth * MAX_SCALE) {
            w = (int) (mWidth * MAX_SCALE);
            bitmap = NmsBitmapUtils.getBitmapByPath(path, options, w, height * w / width);
        } else if (width > mWidth * MIN_SCALE) {
            w = (int) (mWidth * MIN_SCALE);
            bitmap = NmsBitmapUtils.getBitmapByPath(path, options, w, height * w / width);
        } else {
            bitmap = NmsBitmapUtils.getBitmapByPath(path, options, width, height);
        }
        if (bitmap != null&&v!=null) {
            ViewGroup.LayoutParams params = (ViewGroup.LayoutParams) v.getLayoutParams();
            params.height = height * w / width;
            params.width = w;
            v.setLayoutParams(params);
            v.setImageBitmap(bitmap);
        } else {
            if (!TextUtils.isEmpty(bakPath)) {
                options = NmsBitmapUtils.getOptions(bakPath);
                options.inJustDecodeBounds = true;
                BitmapFactory.decodeFile(bakPath, options);
                bitmap = NmsBitmapUtils.getBitmapByPath(bakPath,
                        NmsBitmapUtils.getOptions(bakPath), options.outWidth, options.outHeight);
            }
            if(v!=null){
                if (bitmap != null) {
                    v.setImageBitmap(bitmap);
                } else {
                    v.setImageResource(R.drawable.all_media_image);
                }
            }
        }

        if (null == bitmap) {
            return false;
        }
        if (null != mImageSizeBg) {
            android.widget.RelativeLayout.LayoutParams p = (android.widget.RelativeLayout.LayoutParams) mImageSizeBg
                    .getLayoutParams();
            if (w != 0 && w > mContext.getResources().getDimensionPixelOffset(R.dimen.img_minwidth)) {
                p.width = w;
            } else {
                p.width = mContext.getResources().getDimensionPixelOffset(R.dimen.img_minwidth);
            }
            mImageSizeBg.setLayoutParams(p);
        }
        return true;
    }

    private void setTextItem() {
        NmsIpTextMessage msg = (NmsIpTextMessage) mMsgCont;
        SpannableStringBuilder buf = new SpannableStringBuilder();
        SmileyParser parser = SmileyParser.getInstance();
        boolean hasBody = !TextUtils.isEmpty(msg.body);
        if (hasBody) {
            buf.append(parser.addSmileySpans(msg.body));
        }
        mTextContent.setTextSize(textSize);
        mTextContent.setText(buf);
    }

    private void setGroupItem() {
        int width = getResources().getDimensionPixelOffset(R.dimen.groupcfg_port_width);
        mGroupCfg.setMaxWidth(mScreenWidth - width);
        mGroupCfg.setText(((NmsIpTextMessage) mMsgCont).body);
    }

    private void setVcardItem() {
        mContactName.setText(((NmsIpVCardMessage) mMsgCont).name);
    }

    private void setCalendarItem() {
        mCalendarTitle.setText(((NmsIpCalendarMessage) mMsgCont).summary);
    }

    private void setLocationItem() {
        mLocationAddr.setText(((NmsIpLocationMessage) mMsgCont).address);
        String path = ((NmsIpLocationMessage) mMsgCont).path;
        if (NmsCommonUtils.isExistsFile(path)) {
            Bitmap bm = BitmapFactory.decodeFile(path);
            mLocationImg.setImageBitmap(bm);
        } else {
            mLocationImg.setImageResource(R.drawable.default_map_small);
        }

    }

    private NmsContact getChildContact(String number) {
        NmsContact child = mChildCache.get(number);
        if (null == child) {
            child = NmsIpMessageApiNative.nmsGetContactInfoViaNumber(number);
            if (child != null) {
                mChildCache.put(number, child);
            }
        }
        return child;
    }

    public void clearChildCache() {
        mChildCache.evictAll();
    }

    public int getStatusResourceId(int status) {
        int id = 0;
        if (status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX
                || status == NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX_PENDING) {
            id = R.drawable.im_meg_status_sending;
        } else if (status == NmsIpMessageConsts.NmsIpMessageStatus.SENT) {
            id = R.drawable.im_meg_status_out;
        } else if (status == NmsIpMessageConsts.NmsIpMessageStatus.DELIVERED) {
            id = R.drawable.im_meg_status_reach;
        } else if (status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
            id = R.drawable.im_meg_status_error;
        } else if (status == NmsIpMessageConsts.NmsIpMessageStatus.VIEWED) {
            id = R.drawable.im_meg_status_read;
        } else if (status == NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED) {
            id = R.drawable.im_meg_status_error;
        } else {
            id = R.drawable.im_meg_status_out;
        }
        return id;
    }

    public void showTimeDivider() {
        mTimeDivider.setVisibility(View.VISIBLE);
    }

    public void showMutiTimeDivider() {
        mTimeDivider.setVisibility(View.VISIBLE);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mTimeBottomDivider.setVisibility(View.GONE);
        mTimeLeftDivider.setVisibility(View.GONE);
        mTimeRightDivider.setVisibility(View.GONE);
    }

    public void showWithBottomTimeDivider() {
        mTimeDivider.setVisibility(View.VISIBLE);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mTimeBottomDivider.setVisibility(View.VISIBLE);
        mTimeLeftDivider.setVisibility(View.GONE);
        mTimeRightDivider.setVisibility(View.GONE);
    }

    public void showSingleTimeDivider() {
        mTimeDivider.setVisibility(View.VISIBLE);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mTimeBottomDivider.setVisibility(View.GONE);
        mTimeLeftDivider.setVisibility(View.VISIBLE);
        mTimeRightDivider.setVisibility(View.VISIBLE);
    }

    public void showMutiGroupDivider() {
        mGroupCfg.setVisibility(View.VISIBLE);
        mLeftDivider.setVisibility(View.GONE);
        mRightDivider.setVisibility(View.GONE);
        mBottomDivider.setVisibility(View.GONE);
    }

    public void showBottomGroupDivider() {
        mGroupCfg.setVisibility(View.VISIBLE);
        mBottomDivider.setVisibility(View.VISIBLE);
        mLeftDivider.setVisibility(View.GONE);
        mRightDivider.setVisibility(View.GONE);
    }

    public void showSingleGroupDivider() {
        mGroupCfg.setVisibility(View.VISIBLE);
        mLeftDivider.setVisibility(View.VISIBLE);
        mRightDivider.setVisibility(View.VISIBLE);
        mBottomDivider.setVisibility(View.GONE);
    }

    public void hideGroupDivider() {
        mGroupCfg.setVisibility(View.GONE);
        mLeftDivider.setVisibility(View.GONE);
        mRightDivider.setVisibility(View.GONE);
        mBottomDivider.setVisibility(View.GONE);
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public void showSingleUnreadDivider(int count) {
        String str = String.format(getResources().getString(R.string.STR_NMS_UNREAD_FORMAT), count);
        mUnreadCount.setText(str);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mUnreadDivider.setVisibility(View.VISIBLE);
        mUnreadBottomDivider.setVisibility(View.GONE);
        mUnreadLeftDivider.setVisibility(View.VISIBLE);
        mUnreadRightDivider.setVisibility(View.VISIBLE);
    }

    public void showUnreadWithBottomDivider(int count) {
        String str = String.format(getResources().getString(R.string.STR_NMS_UNREAD_FORMAT), count);
        mUnreadCount.setText(str);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mUnreadDivider.setVisibility(View.VISIBLE);
        mUnreadBottomDivider.setVisibility(View.VISIBLE);
        mUnreadLeftDivider.setVisibility(View.GONE);
        mUnreadRightDivider.setVisibility(View.GONE);
    }

    public void showMutiUnreadDivider(int count) {
        String str = String.format(getResources().getString(R.string.STR_NMS_UNREAD_FORMAT), count);
        mUnreadCount.setText(str);
        if (null != mTimeDividerZone) {
            mTimeDividerZone.setVisibility(View.VISIBLE);
        }
        mUnreadDivider.setVisibility(View.VISIBLE);
        mUnreadBottomDivider.setVisibility(View.VISIBLE);
        mUnreadLeftDivider.setVisibility(View.GONE);
        mUnreadRightDivider.setVisibility(View.GONE);
    }

    public void hideUnreadDivider() {
        if (mTimeDivider.getVisibility() == View.VISIBLE) {
            if (null != mTimeDividerZone) {
                mTimeDividerZone.setVisibility(View.VISIBLE);
            }
        } else {
            if (null != mTimeDividerZone) {
                mTimeDividerZone.setVisibility(View.GONE);
            }
        }
        mUnreadDivider.setVisibility(View.GONE);
        mUnreadBottomDivider.setVisibility(View.GONE);
        mUnreadLeftDivider.setVisibility(View.GONE);
        mUnreadRightDivider.setVisibility(View.GONE);
    }

    private void showResendDialog() {
        if (mMarkState) {
            return;
        }
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsGroupChatContact gc = (NmsGroupChatContact) mContact;
            if (!gc.isAlive()
                    || !MessageUtils.isCurrentSim(mContext, gc.getSimId())
                    || !(NmsIpMessageApiNative.nmsGetActivationStatus(gc.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                    || gc.getMemberCount() == 1) {
                return;
            }
        }
        if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED
                || mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED) {
            mResendDialog = new reSendDialog(mContext);
            mResendDialog.bind();
        }
    }

    public void setTryAllAgainListener(tryAllAgainListener l) {
        mListener = l;
    }

    public interface tryAllAgainListener {
        public void tryAllAgain();
    }

    private class reSendDialog {

        public final static int tryAgain = 0;
        public final static int tryAllAgain = 1;
        public final static int sendViaTextMsg = 2;
        public final static int sendViaMultimediaMsg = 3;
        public final static int continueTry = 4;

        private Context mContext;
        private AlertDialog mDialog = null;
        private ArrayList<ItemOption> mOptionList = null;
        private AlertDialog.Builder mBuilder;
        private ItemAdapter mAdapter;

        private class ItemOption {
            public int actionId;
            public String itemName;
        }

        public reSendDialog(Context context) {
            mContext = context;
            mOptionList = new ArrayList<ItemOption>();
        }

        public void bind() {
            initStringList();
            createDialog();
        }

        private void addItem(int cmdId, int resId) {
            ItemOption item = new ItemOption();
            item.actionId = cmdId;
            item.itemName = mContext.getString(resId);
            mOptionList.add(item);
        }

        private void tryToAddTryAgainItem() {
            if (mMsgCont.status != NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
                return;
            }
            addItem(tryAgain, R.string.STR_NMS_TRY_AGAIN);
        }

        private void tryToAddTryAllAgainItem() {
            if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED
                    && mCountInfo.failedCount <= 1) {
                return;
            }
            if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED
                    && mCountInfo.notDeliveredCount <= 1) {
                return;
            }
            addItem(tryAllAgain, R.string.STR_NMS_TRY_ALL_AGAIN);
        }

        private void tryToAddContinueTryItem() {
            if (mMsgCont.status != NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED) {
                return;
            }
            addItem(continueTry, R.string.STR_NMS_CONTINUE_TRY);
        }

        private void tryToAddSendViaTextItem() {
            if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
                return;
            }
            if (mMsgCont.type != NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                return;
            }
            addItem(sendViaTextMsg, R.string.STR_NMS_SEND_VIA_TEXT);
        }

        private void tryToAddSendMitimediaMsgItem() {
            if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
                return;
            }
            if (mMsgCont.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                return;
            }
            addItem(sendViaMultimediaMsg, R.string.STR_NMS_SEND_VIA_MULTIMEDIA);
        }

        private void initStringList() {
            mOptionList.clear();
            tryToAddTryAgainItem();
            tryToAddContinueTryItem();
            tryToAddTryAllAgainItem();
            tryToAddSendViaTextItem();
            tryToAddSendMitimediaMsgItem();

            mAdapter = new ItemAdapter(mContext, mOptionList);
        }

        private void createDialog() {
            if (null == mBuilder) {
                mBuilder = new AlertDialog.Builder(mContext);
            }
            if (mMsgCont.status == NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
                mBuilder.setTitle(R.string.STR_NMS_FAILED_TITLE);
            } else {
                mBuilder.setTitle(R.string.STR_NMS_NOT_DELIVERED_TITLE);
            }
            mBuilder.setAdapter(mAdapter, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    int id = mOptionList.get(which).actionId;
                    doAction(id);
                }
            });
            mDialog = mBuilder.create();
            mDialog.show();
        }

        private void doAction(int actionId) {
            switch (actionId) {
            case tryAgain:
                engineadapter.get().nmsUIResendMsg(
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        (short) mMsgCont.ipDbId, 0);
                break;
            case tryAllAgain:
                mListener.tryAllAgain();
                break;
            case continueTry:
                break;
            case sendViaMultimediaMsg:
                break;
            case sendViaTextMsg:
                engineadapter.get().nmsUIResendMsg(
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        (short) mMsgCont.ipDbId, 1);
                break;
            default:
                NmsLog.trace(TAG, "unknow action not handle, action: " + actionId);
                break;
            }
        }

        private class ItemAdapter extends BaseAdapter {

            private ArrayList<ItemOption> list;
            private Context content;

            public ItemAdapter(Context context, ArrayList<ItemOption> list) {
                this.content = context;
                this.list = list;
            }

            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                View view = convertView;

                if (convertView == null) {
                    LayoutInflater mInflater = (LayoutInflater) mContext
                            .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                    view = mInflater.inflate(R.layout.chat_long_press_item, null);
                }
                TextView v = (TextView) view.findViewById(R.id.LongItem);
                v.setText(list.get(position).itemName);
                return view;
            }

            @Override
            public int getCount() {
                // TODO Auto-generated method stub
                return list.size();
            }

            @Override
            public Object getItem(int position) {
                // TODO Auto-generated method stub
                return null;
            }

            @Override
            public long getItemId(int position) {
                // TODO Auto-generated method stub
                return 0;
            }
        }
    }
    
   synchronized private static CharSequence getSimIndicatorInCache(Context context, int simId) {
        try {
            if (mSimInfoMap.containsKey(simId))
                return mSimInfoMap.get(simId) ;
            
            CharSequence simIndicator = MessageUtils.getMTKServiceIndicator(context, simId) ;
            
            if (simIndicator != null && !TextUtils.isEmpty(simIndicator)) {
                mSimInfoMap.put(simId, simIndicator) ;
                return simIndicator ;
            }
            
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
        
        return "" ;
    }
    
    synchronized static public void resetSimIndicatorCache() {
        try {
            mSimInfoMap.clear() ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        } 
    }
}
