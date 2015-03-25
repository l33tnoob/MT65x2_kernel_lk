package com.hissage.ui.view;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Handler;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsConfig;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.ui.view.NmsCaptionEditor.ActionListener;
import com.hissage.ui.view.NmsEmoticonPanel.EditEmoticonListener;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageConsts;

public class NmsBottomPanelView extends LinearLayout {

    private ImageButton mEmoticonButton;
    private ImageButton mShareButton;
    private NmsCaptionEditor mCaptionEditor;
    private ImageButton mSendButton;
    private TextView mTextCounter;
    private NmsSharePanel mSharePanel;
    private NmsEmoticonPanel mEmoticonPanel;
    private Handler mHandler;
    private Context mContext;
    private NmsContact mContact;
    private String mAttachPath;
    private int mAttachType = -1;
    private int mDuration;
    private boolean mShowKeyBoardFromShare = false;
    private boolean mShowKeyBoardFromEmoticon = false;
    private boolean mExternalFlag = false;
    private String TAG = "BottomPnelView";
    private ComposeMessageListener mListener;
    private boolean isActivated = false;

    private long mTypingDetecterCycle = 6000;
    private boolean mIfNeedTypeInfo = false;
    private boolean mIsTypeStarted = false;
    private boolean mIsOnTyping = false;
    private Timer typingTimer = null;
    private boolean allRecipientActivated = true;
    private String recipientNumber = null;

    public static final int PANEL_ACTION_SHARE = 1;
    public static final int PANEL_ACTION_EMOTICON = 2;
    public static final int PANEL_ACTION_SEND_EMOTICON = 3;
    public static final int PANEL_ACTION_CAPTION = 4;
    public static final int PANEL_ACTION_SEND = 5;
    public static final int PANEL_ACTION_SEND_MSG = 6;

    private static final int SEND_BTN_STYLE_DISABLED = 1;
    private static final int SEND_BTN_STYLE_SMS = 2;
    private static final int SEND_BTN_STYLE_ISMS = 3;

    public NmsBottomPanelView(Context context) {
        this(context, null);
    }

    public NmsBottomPanelView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public void setEditTextSize(float textSize) {
        mCaptionEditor.setTextSize(textSize);
    }

    public void setExternalFlag(boolean flag) {
        mExternalFlag = flag;
    }

    class UpdateTypingTask extends TimerTask {
        public void run() {
            if (true == mIsOnTyping) {
                mIsOnTyping = false;
                NmsIpMessageApiNative.nmsSendChatMode(mContact.getNumber(),
                        NmsContact.NmsContactStauts.TYPING);
            } else {
                mIsTypeStarted = false;
                typingTimer.cancel();
                typingTimer = null;
                NmsIpMessageApiNative.nmsSendChatMode(mContact.getNumber(),
                        NmsContact.NmsContactStauts.STOP_TYPING);
            }
        }
    }

    public void handleTypingStatus() {
        if (false == mIsTypeStarted) {
            mIsTypeStarted = true;
            typingTimer = new Timer();
            typingTimer.schedule(new UpdateTypingTask(), 0, mTypingDetecterCycle);
        }
        mIsOnTyping = true;
    }
    public void setForbidSendIpMessage(boolean mIsSmsEnabled){

        if (mIsSmsEnabled) {
            mEmoticonButton.setImageResource(R.drawable.isms_emoticon);
            mEmoticonButton.setClickable(true);
            mShareButton.setImageResource(R.drawable.isms_share);
            mShareButton.setClickable(true);
            mCaptionEditor.setForbidSend(mIsSmsEnabled);
            mSendButton.setClickable(true);
            setSendButton();
        } else {
            mEmoticonButton.setImageResource(R.drawable.default_n);
            mEmoticonButton.setClickable(false);
            mShareButton.setImageResource(R.drawable.isms_share_disable);
            mShareButton.setClickable(false);
            mSendButton.setImageResource(R.drawable.isms_chat_button_send_unable);
            mSendButton.setClickable(false);
            mCaptionEditor.setForbidSend(mIsSmsEnabled);
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mEmoticonButton = (ImageButton) findViewById(R.id.ib_emoticon);
        mShareButton = (ImageButton) findViewById(R.id.ib_share);
        mCaptionEditor = (NmsCaptionEditor) findViewById(R.id.caption_editor);
        mSendButton = (ImageButton) findViewById(R.id.ib_send_button);
        mTextCounter = (TextView) findViewById(R.id.tv_text_counter);
        mSharePanel = (NmsSharePanel) findViewById(R.id.share_panel);

        showSharePanel(false);
        mEmoticonPanel = (NmsEmoticonPanel) findViewById(R.id.emoticon_panel);
        showEmoticonPanel(false);
        mEmoticonPanel.setEditEmoticonListener(new EditEmoticonListener() {

            @Override
            public void doAction(int type, String emoticonName) {
                if (type == EditEmoticonListener.addEmoticon) {
                    insertEmoticon(emoticonName);
                } else if (type == EditEmoticonListener.delEmoticon) {
                    delEmoticon();
                } else if (type == EditEmoticonListener.sendEmoticon) {
                    sendEmoticon(emoticonName);
                }
            }
        });

        mCaptionEditor.addActionListener(new ActionListener() {

            @Override
            public void doAction(int type) {
                if (type == NmsCaptionEditor.ACTION_TOUCH) {
                    mHandler.sendEmptyMessage(MessageConsts.NMS_SCROLL_LIST);
                    showSharePanel(false);
                    showEmoticonPanel(false);
                    showKeyBoard(true);
                    if (mExternalFlag) {
                        mListener.doAction(PANEL_ACTION_CAPTION, (short) 0);
                    }
                } else if (type == NmsCaptionEditor.ACTION_TEXT_CHANGE) {
                    int charCount = mCaptionEditor.getTextContent().length();
                    setCountTip(charCount);
                    setSendButton();
                    mListener.doAction(ComposeMessageListener.ACTION_CONTENT_CHANGE, (short) 0);
                    if (true == mIfNeedTypeInfo && charCount > 0) {
                        handleTypingStatus();
                    }
                } else if (type == NmsCaptionEditor.ACTION_DELETE_ATTACH) {
                    clearAttach();
                }
            }
        });

        mEmoticonButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mExternalFlag) {
                    mListener.doAction(PANEL_ACTION_EMOTICON, (short) 0);
                }
                mHandler.sendEmptyMessage(MessageConsts.NMS_REFRESH_AND_SCROLL_LIST);
                if (mShowKeyBoardFromEmoticon) {
                    showEmoticonPanel(false);
                    showSharePanel(false);
                    showKeyBoard(true);
                } else {
                    showEmoticonPanel(true);
                    showSharePanel(false);
                    showKeyBoard(false);
                }
            }
        });

        mShareButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mExternalFlag) {
                    mListener.doAction(PANEL_ACTION_SHARE, (short) 0);
                }
                mHandler.sendEmptyMessage(MessageConsts.NMS_REFRESH_AND_SCROLL_LIST);
                if (mShowKeyBoardFromShare) {
                    showSharePanel(false);
                    showEmoticonPanel(false);
                    showKeyBoard(true);
                } else {
                    showSharePanel(true);
                    showEmoticonPanel(false);
                    showKeyBoard(false);
                }
            }
        });

        mSendButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                if (mExternalFlag) {
                    mListener.doAction(PANEL_ACTION_SEND, (short) 0);
                }
                sendMessage();
            }
        });

        setCanSendMsg(true);
    }

    /**
     * Delete emoticon or text.
     */
    private void delEmoticon() {
        mCaptionEditor.deleteEmoticon();
    }

    /**
     * 
     * Send large emoticon or animation directly.
     * 
     * @param body
     *            The coding of large emoticon or animation.
     */
    private void sendEmoticon(String body) {
        if (null == mContact && null == recipientNumber) {
            mCaptionEditor.setTextContent(body);
            return;
        } else if (recipientNumber != null) {
            short contactId = NmsContactApi.getInstance(mContext).getEngineContactIdViaNumber(
                    recipientNumber);
            mContact = engineadapter.get().nmsUIGetContact(contactId);
            if (null == mContact) {
                NmsLog.error(TAG, "send message got error, contact is null, invalid contactId = "
                        + contactId);
                return;
            }
        } else {
            // do nothing
        }

        if (mExternalFlag) {
            mListener.doAction(PANEL_ACTION_SEND_EMOTICON, mContact.getId());
        }

        if (mContact.getType() == NmsContactType.NOT_HISSAGE_USER
                || (mContact.getType() == NmsContactType.HISSAGE_BROADCAST && !allNumberActivated(mContact
                        .getNumber()))) {
            final String[] numArray = mContact.getNumber().split(",");
            for (int i = 0; i < numArray.length; i++) {
                sendSMS(mContact.getNumber(), numArray[i], body, (int) NmsPlatformAdapter
                        .getInstance(mContext).getCurrentSimId());
            }
        } else {
            NmsIpTextMessage msg = new NmsIpTextMessage();
            if (NmsConfig.getSendAsSMSFlag()
                    && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                msg.send(mContact.getNumber(), body, (int) NmsPlatformAdapter.getInstance(mContext)
                        .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.AUTO,
                        (short) -1, false);
            } else {
                msg.send(mContact.getNumber(), body, (int) NmsPlatformAdapter.getInstance(mContext)
                        .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL,
                        (short) -1, false);
            }
        }
    }

    /**
     * Bind contact and load draft.
     * 
     * @param contact
     *            Contact information.
     * @param setDraft
     *            Whether need to load draft or not.
     */
    public void bind(NmsContact contact, boolean setDraft) {
        long currentSimId = NmsPlatformAdapter.getInstance(mContext).getCurrentSimId();
        if (NmsIpMessageApiNative.nmsGetActivationStatus((int) currentSimId) == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
            isActivated = true;
        }
        if (null != mContact && setDraft) {
            saveToDraft();
            mCaptionEditor.setTextContent("");
            clearAttach();
        }
        mContact = contact;

        setSendButton();
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            if (!((NmsGroupChatContact) mContact).isAlive()
                    || ((NmsGroupChatContact) mContact).getMemberCount() == 1) {
                setCanSendMsg(false);
            } else {
                setCanSendMsg(true);
            }
            // set hint for group chat
            mCaptionEditor.setIsGroupChat(true);
        }

        if (setDraft) {
            setDraft();
        }
        // after initial set text operation
        if (mContact.getType() == NmsContactType.HISSAGE_USER) {
            mIfNeedTypeInfo = true;
        }
    }

    public void bind() {
        long currentSimId = NmsPlatformAdapter.getInstance(mContext).getCurrentSimId();
        if (NmsIpMessageApiNative.nmsGetActivationStatus((int) currentSimId) == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
            isActivated = true;
        }
        setSendButton();
    }

    public void updateContact(ArrayList<NmsContact> contactArray) {
        if (contactArray != null && contactArray.size() > 0) {
            recipientNumber = "";
            allRecipientActivated = true;
            for (int i = 0; i < contactArray.size(); i++) {
                recipientNumber = recipientNumber.concat(contactArray.get(i).getNumber() + ",");
                if (contactArray.get(i).getType() != NmsContactType.HISSAGE_USER) {
                    allRecipientActivated = false;
                }
            }
        } else {
            recipientNumber = null;
            allRecipientActivated = false;
        }

        setSendButton();
    }

    public void setDraft() {
        int draftId = engineadapter.get().nmsUIGetContactDraftMsgId(mContact.getId());
        
        NmsIpMessage msg = null;
        if(draftId != -1){
            msg = engineadapter.get().nmsUIGetMsgKey((short) draftId);
            if(msg == null){
                engineadapter.get().nmsUIDeleteContactDraftMsg(mContact.getId(), 0);
            }
        }
        
        if (msg != null) {
            int type = msg.type;
            if (type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
                mCaptionEditor.setTextContent(((NmsIpTextMessage) msg).body);
                clearAttach();
            } else if (type == NmsIpMessageConsts.NmsIpMessageType.PICTURE) {
                if ("[draft]".equals(((NmsIpImageMessage) msg).caption)) {
                    mCaptionEditor.setTextContent("");
                } else {
                    mCaptionEditor.setTextContent(((NmsIpImageMessage) msg).caption);
                }
                setAttachPath(((NmsIpImageMessage) msg).path, type);
            } else if (type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
                if ("[draft]".equals(((NmsIpVideoMessage) msg).caption)) {
                    mCaptionEditor.setTextContent("");
                } else {
                    mCaptionEditor.setTextContent(((NmsIpVideoMessage) msg).caption);
                }
                setAttachPath(((NmsIpVideoMessage) msg).path,
                        ((NmsIpVideoMessage) msg).durationTime, type);
            } else if (type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
                if ("[draft]".equals(((NmsIpVoiceMessage) msg).caption)) {
                    mCaptionEditor.setTextContent("");
                } else {
                    mCaptionEditor.setTextContent(((NmsIpVoiceMessage) msg).caption);
                }
                setAttachPath(((NmsIpVoiceMessage) msg).path,
                        ((NmsIpVoiceMessage) msg).durationTime, type);
            }
        } else {
            mCaptionEditor.setTextContent("");
            clearAttach();
        }
    }

    /**
     * Clear attachment in input box, and disabled send button.
     */
    public void clearAttach() {
        mAttachPath = "";
        mAttachType = -1;
        mDuration = 0;
        mCaptionEditor.deleteAttach();
        setSendButton();
    }

    public boolean allNumberActivated(String number) {
        boolean allActivated = true;
        String[] tempNum = number.split(",");

        for (int i = 0; i < tempNum.length; i++) {
            // check validation of each num
            if (!NmsContactApi.getInstance(mContext).isHissageNumber(tempNum[i])) {
                allActivated = false;
                break;
            }
        }
        return allActivated;
    }

    private void setSendButton() {
        if ((null == mContact && null == recipientNumber)
                || (TextUtils.isEmpty(mCaptionEditor.getTextContent()) && TextUtils
                        .isEmpty(mAttachPath))) {
            mSendButton.setEnabled(false);
        } else {
            mSendButton.setEnabled(true);
        }

        if (null == mContact && null == recipientNumber) {
            setSendButtonStyle(SEND_BTN_STYLE_DISABLED);
        } else if (!isActivated) {
            setSendButtonStyle(SEND_BTN_STYLE_SMS);
        } else if (recipientNumber != null) {
            if (allRecipientActivated) {
                setSendButtonStyle(SEND_BTN_STYLE_ISMS);
            } else {
                setSendButtonStyle(SEND_BTN_STYLE_SMS);
            }
        } else if (mContact.getType() == NmsContactType.NOT_HISSAGE_USER
                || (mContact.getType() == NmsContactType.HISSAGE_BROADCAST && !allNumberActivated(mContact
                        .getNumber()))) {
            setSendButtonStyle(SEND_BTN_STYLE_SMS);
        } else {
            setSendButtonStyle(SEND_BTN_STYLE_ISMS);
        }
    }

    private void setSendButtonStyle(int style) {
        int resId = 0;

        switch (style) {
        case SEND_BTN_STYLE_DISABLED:
            resId = R.drawable.isms_chat_button_send_unable;
            break;
        case SEND_BTN_STYLE_SMS:
            resId = R.drawable.isms_chat_button_sent_sms;
            break;
        case SEND_BTN_STYLE_ISMS:
            resId = R.drawable.isms_chat_button_sent_isms;
            break;
        default:
            NmsLog.error(TAG, "set bottom panel send button error, invalid type: " + style);
            break;
        }
        mSendButton.setImageResource(resId);

    }

    private void showEmoticonPanel(boolean showEmoticonFlag) {
        if (showEmoticonFlag) {
            mEmoticonPanel.setVisibility(View.VISIBLE);
            mEmoticonButton.setImageResource(R.drawable.isms_keyboard);
        } else {
            mEmoticonPanel.setVisibility(View.GONE);
            mEmoticonButton.setImageResource(R.drawable.isms_emoticon);
        }
        mShowKeyBoardFromEmoticon = showEmoticonFlag;
    }

    /**
     * Show or hide share panel.
     * 
     * @param showShareFlag
     *            true for show share panel,false for hide share panel.
     */
    private void showSharePanel(boolean showShareFlag) {
        if (showShareFlag) {
            mSharePanel.setVisibility(View.VISIBLE);
            mShareButton.setImageResource(R.drawable.isms_keyboard);
        } else {
            mSharePanel.setVisibility(View.GONE);
            mShareButton.setImageResource(R.drawable.isms_share);
        }
        mShowKeyBoardFromShare = showShareFlag;
    }

    /**
     * Show or hide keyboard.
     * 
     * @param flag
     *            true for show keyboard,false for hide keyboard.
     */
    public void showKeyBoard(boolean flag) {
        mCaptionEditor.showKeyBoard(flag);
    }

    /**
     * Show or hide bottom panel.
     * 
     * @param flag
     *            true for show bottom panel,false for hide bottom panel.
     */
    public void setCanSendMsg(boolean flag) {
        if (flag) {
            setVisibility(View.VISIBLE);
        } else {
            setVisibility(View.GONE);
        }
        // mEmoticonButton.setEnabled(flag);
        // mSendButton.setEnabled(flag);
        // mShareButton.setEnabled(flag);
        // mCaptionEditor.setEditEnabled(flag);
        // if (flag) {
        // mStopEdit.setVisibility(View.GONE);
        // } else {
        // mStopEdit.setVisibility(View.VISIBLE);
        // clearAttach();
        // clearText();
        // }
    }

    /**
     * Send message , include Ip message and SMS/MMS
     */
    private void sendMessage() {
        if (null == mContact && null == recipientNumber) {
            NmsLog.error(TAG, "send message got error, contact is null");
            return;
        } else if (recipientNumber != null) {
            short contactId = NmsContactApi.getInstance(mContext).getEngineContactIdViaNumber(
                    recipientNumber);
            mContact = engineadapter.get().nmsUIGetContact(contactId);
            NmsLog.trace(TAG,
                    "conatctId is " + mContact.getId() + ", num is " + mContact.getNumber());
            if (null == mContact) {
                NmsLog.error(TAG, "send message got error, contact is null, invalid contactId = "
                        + contactId);
                return;
            }
        } else {
            // do nothing
        }

        final String body = mCaptionEditor.getTextContent();

        if (!isActivated
                || mContact.getType() == NmsContactType.NOT_HISSAGE_USER
                || (mContact.getType() == NmsContactType.HISSAGE_BROADCAST && !allNumberActivated(mContact
                        .getNumber()))) {
            final String[] numArray = mContact.getNumber().split(",");
            if (mAttachType >= NmsIpMessageConsts.NmsIpMessageType.PICTURE
                    && mAttachType < NmsIpMessageConsts.NmsIpMessageType.COUNT) {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.STR_NMS_SEND_MMS_QUERY_TITLE)
                        .setMessage(R.string.STR_NMS_SEND_MMS_QUERY)
                        .setNegativeButton(R.string.STR_NMS_CANCEL, null)
                        .setPositiveButton(R.string.STR_NMS_OK,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        if (mExternalFlag) {
                                            mListener.doAction(PANEL_ACTION_SEND_MSG,
                                                    mContact.getId());
                                        }

                                        sendMMS(mContact.getNumber(), body, mAttachPath,
                                                (int) NmsPlatformAdapter.getInstance(mContext)
                                                        .getCurrentSimId());

                                        mCaptionEditor.setTextContent("");
                                        clearAttach();
                                    }
                                }).show();

            } else {
                if (mExternalFlag) {
                    mListener.doAction(PANEL_ACTION_SEND_MSG, mContact.getId());
                }
                for (int i = 0; i < numArray.length; i++) {
                    sendSMS(mContact.getNumber(), numArray[i], body, (int) NmsPlatformAdapter
                            .getInstance(mContext).getCurrentSimId());
                }
                mCaptionEditor.setTextContent("");
                clearAttach();
            }
        } else {
            if (mExternalFlag) {
                mListener.doAction(PANEL_ACTION_SEND_MSG, mContact.getId());
            }

            sendIpMessage("",0);
            mCaptionEditor.setTextContent("");
            clearAttach();
        }
    }

    /**
     * send SMS
     * 
     * @param number
     *            Recipient
     * @param msg
     *            Message body
     * @param simId
     *            SIM id
     * 
     */
    protected void sendSMS(String threadNumber, String number, String msg, int simId) {
        // do nothing
    }

    /**
     * send MMS
     * 
     * @param number
     *            Recipient
     * @param msg
     *            Message body
     * @param simId
     *            SIM id
     * 
     */
    protected void sendMMS(String number, String msg, String attachPath, int simId) {
        // do nothing
    }

    /**
     * Send Ip message and clear input box. Contains text message, picture with
     * caption, audio with caption, video with caption.
     */
    private void sendIpMessage(String caption,int flag) {
        if (null == mContact) {
            NmsLog.error(TAG, "bottom panel send message got error, mContact is null");
            return;
        }
        // stop typing status before send
        if (mIfNeedTypeInfo && typingTimer != null) {
            mIsTypeStarted = false;
            mIsOnTyping = false;
            typingTimer.cancel();
            typingTimer = null;
            NmsIpMessageApiNative.nmsSendChatMode(mContact.getNumber(),
                    NmsContact.NmsContactStauts.STOP_TYPING);
        }

        String body;
        if(TextUtils.isEmpty(caption)){
            body = mCaptionEditor.getTextContent();
        }else{
            body=caption;
        }
        if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.PICTURE) {
            NmsIpImageMessage msg = new NmsIpImageMessage(
                    NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            if (NmsConfig.getSendAsSMSFlag()
                    && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                msg.send(mContext, flag,mContact.getNumber(), mAttachPath, body, (int) NmsPlatformAdapter
                        .getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), true, mHandler);
            } else {
                msg.send(mContext, flag,mContact.getNumber(), mAttachPath, body, (int) NmsPlatformAdapter
                        .getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), true, mHandler);
            }
        } else if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
            NmsIpVideoMessage msg = new NmsIpVideoMessage();
            if (NmsConfig.getSendAsSMSFlag()
                    && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                msg.send(mContact.getNumber(), mAttachPath, body, mDuration,
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), true);
            } else {
                msg.send(mContact.getNumber(), mAttachPath, body, mDuration,
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), true);
            }
        } else if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
            NmsIpVoiceMessage msg = new NmsIpVoiceMessage();
            if (NmsConfig.getSendAsSMSFlag()
                    && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                msg.send(mContact.getNumber(), mAttachPath, body, mDuration,
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact.getId(), true);
            } else {
                msg.send(mContact.getNumber(), mAttachPath, body, mDuration,
                        (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                        NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, mContact.getId(), true);
            }
        } else {
            NmsIpTextMessage msg = new NmsIpTextMessage();
            if (NmsConfig.getSendAsSMSFlag()
                    && mContact.getType() != NmsContactType.HISSAGE_GROUP_CHAT) {
                msg.send(mContact.getNumber(), body, (int) NmsPlatformAdapter.getInstance(mContext)
                        .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.AUTO, mContact
                        .getId(), true);
            } else {
                msg.send(mContact.getNumber(), body, (int) NmsPlatformAdapter.getInstance(mContext)
                        .getCurrentSimId(), NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL,
                        mContact.getId(), true);
            }
        }
        // deleteDraft();
        mCaptionEditor.setTextContent("");
        clearAttach();
    }

    /**
     * Insert normal emoticon
     * 
     * @param text
     *            The coding of normal emoticon
     */
    public void insertEmoticon(String text) {
        mCaptionEditor.insertEmoticon(text);
    }

    /**
     * Insert quick text
     * 
     * @param text
     *            The coding of normal emoticon
     */
    public void insertQuickText(String text) {
        mCaptionEditor.insertQuick(text);
    }

    /**
     * Set Handler. It used to bottom panel do asynchronous actions.
     * 
     * @param handler
     *            Handler
     */
    public void setHandler(Handler handler) {
        mHandler = handler;
        mEmoticonPanel.setHandler(mHandler);
        mSharePanel.setHandler(mHandler);
    }

    /**
     * Display text count.
     * 
     * @param count
     *            the count of text.
     */
    private void setCountTip(int count) {
        if (count == 0) {
            mTextCounter.setVisibility(View.GONE);
        } else {
            if (mCaptionEditor.getLineCount() >= 2) {
                mTextCounter.setVisibility(View.VISIBLE);
                if (mAttachType == -1) {
                    mTextCounter.setText(count + " / " + NmsCustomUIConfig.MESSAGE_MAX_LENGTH);
                } else {
                    mTextCounter.setText(count + " / " + NmsCustomUIConfig.CAPTION_MAX_LENGTH);
                }
            } else {
                mTextCounter.setVisibility(View.GONE);
            }
        }
    }

    /**
     * Display the count of text draft
     */
    public void setDraftCountTip() {
        setCountTip(mCaptionEditor.getTextContent().length());
    }

    /**
     * Activity need to call, when screen orientation changed. The share panel
     * and emoticon panel will be rebuild;
     */
    public void resize() {
        mSharePanel.resetShareItem();
        mEmoticonPanel.resetShareItem();
    }

    /**
     * Sets the attach.
     * 
     * @param path
     *            the path of attachment
     * @param duration
     *            the duration of audio or video.
     * @param attachType
     *            the type of attachment
     */
    private void setAttach(String path, int duration, int attachType) {
        mAttachPath = path;
        mAttachType = attachType;
        mDuration = duration;
        mCaptionEditor.seteAttach(path, attachType);
        setSendButton();
    }

    /**
     * Set picture attachment.
     * 
     * @param path
     *            the path of attachment.
     * @param attachType
     *            The type of attachment(Detail type defined at
     *            NmsIpMessageType)
     */
    public void setAttachPath(String path, int attachType) {
        if (mAttachType != -1 && !TextUtils.isEmpty(mAttachPath)) {
            showReplaceAttachDialog(path, 0, attachType);
        } else {
            setAttach(path, 0, attachType);
        }
    }

    /**
     * Set video or audio attachment.
     * 
     * @param path
     *            The path of attachment.
     * @param duration
     *            The duration of audio or video.
     * @param attachType
     *            The type of attachment(Detail type defined at
     *            NmsIpMessageType)
     */
    public void setAttachPath(String path, int duration, int attachType) {
        if (mAttachType != -1 && !TextUtils.isEmpty(mAttachPath)) {
            showReplaceAttachDialog(path, duration, attachType);
        } else {
            setAttach(path, duration, attachType);
        }
    }

    /**
     * If there is an object inserted in text input field for captions, user
     * tries to insert another object which is also set to be shared with
     * caption, in this case, system will inform the user "There is an
     * attachment in input field. Do you want to replace it with this
     * attachment.
     * 
     * @param path
     *            the path of attachment
     * @param duration
     *            the duration of video or audio, picture will set 0.
     * @param attachType
     *            The type of attachment(Detail type defined at
     *            NmsIpMessageType)
     */
    private void showReplaceAttachDialog(final String path, final int duration, final int attachType) {
        new AlertDialog.Builder(mContext).setTitle(R.string.STR_NMS_REPLACE_ATTACH_TITLE)
                .setMessage(R.string.STR_NMS_REPLACE_ATTACH_MSG)
                .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        setAttach(path, duration, attachType);
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).create().show();
    }

    /**
     * Returns the visibility of emoticon panel and share panel.
     * 
     * @return true if emoticon panel or share panel is shown.
     */
    public boolean isShow() {
        if (mSharePanel.isShown()) {
            return true;
        }
        if (mEmoticonPanel.isShown()) {
            return true;
        }
        return false;
    }

    /**
     * The keyboard, share panel and emoticon panel will be hide;
     */
    public void hiden() {
        showKeyBoard(false);
        if (mSharePanel.isShown()) {
            showSharePanel(false);
        }
        if (mEmoticonPanel.isShown()) {
            showEmoticonPanel(false);
        }
    }

    public boolean isEmptyMsg() {
        boolean ret = false;
        String body = mCaptionEditor.getTextContent();
        if (TextUtils.isEmpty(body) && mAttachType == -1 && TextUtils.isEmpty(mAttachPath)) {
            ret = true;
        } else {
            ret = false;
        }
        return ret;
    }

    /**
     * Save draft to DB. if input box is empty, will delete draft.
     */
    public void saveToDraft() {
        String body = mCaptionEditor.getTextContent();
        if (TextUtils.isEmpty(body) && mAttachType == -1 && TextUtils.isEmpty(mAttachPath)) {
            deleteDraft();
            return;
        }
        int ret = -1;
        if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.PICTURE) {
            NmsIpImageMessage msg = new NmsIpImageMessage(
                    NmsIpMessageConsts.NmsIpMessageType.PICTURE);
            ret = msg.saveDraft(mContact.getNumber(), mAttachPath, body, (int) NmsPlatformAdapter
                    .getInstance(mContext).getCurrentSimId());
        } else if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
            NmsIpVideoMessage msg = new NmsIpVideoMessage();
            ret = msg.saveDraft(mContact.getNumber(), mAttachPath, body, mDuration,
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId());
        } else if (mAttachType == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
            NmsIpVoiceMessage msg = new NmsIpVoiceMessage();
            ret = msg.saveDraft(mContact.getNumber(), mAttachPath, body, mDuration,
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId());
        } else {
            NmsIpTextMessage msg = new NmsIpTextMessage();
            ret = msg.saveDraft(mContact.getNumber(), body,
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId());
        }
        if (ret == -1) {
            NmsLog.error(TAG, "save draft failed");
        } else {
            Toast.makeText(mContext, R.string.STR_NMS_SAVE_AS_DRAFT, Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Delete draft.
     */
    private void deleteDraft() {
        int draftId = engineadapter.get().nmsUIGetContactDraftMsgId(mContact.getId());
        if (draftId != -1) {
            engineadapter.get().nmsUIDeleteContactDraftMsg(mContact.getId(), 1);
        }
    }

    /**
     * Return the length text content.
     * 
     * @return the length of text.
     */
    public int getTextLength() {
        return mCaptionEditor.getTextContent().length();
    }

    /**
     * Return the content of caption editor.
     * 
     * @return the string of text content.
     */
    public String getTextContent() {
        return mCaptionEditor.getTextContent();
    }

    public void focusBottomEditer() {
        mCaptionEditor.requestEditFocus();
    }

    /**
     * Return whether contains attachment.
     * 
     * @return true if contains attachment.
     */
    public boolean isContainAttach() {
        if (mAttachType != -1 && !TextUtils.isEmpty(mAttachPath)) {
            return true;
        } else {
            return false;
        }
    }

    public void setText(String str) {
        mCaptionEditor.setTextContent(str);
    }

    public void setSection(int index) {
        mCaptionEditor.setSection(index);
    }

    /**
     * Clear input box.
     */

    public void recycleAllView() {
        mEmoticonPanel.recycleView();
        mSharePanel.recycleView();
        removeAllViews() ;
    }

    public void clearText() {
        mCaptionEditor.setTextContent("");
    }

    public void setComposeMessageListener(ComposeMessageListener l) {
        mListener = l;
    }

    public interface ComposeMessageListener {
        public final static int ACTION_CONTENT_CHANGE = 0;

        public void doAction(int type, short value);
    }
}
