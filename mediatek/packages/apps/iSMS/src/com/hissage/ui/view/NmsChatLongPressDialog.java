package com.hissage.ui.view;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.provider.Browser;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.text.ClipboardManager;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.download.NmsDownloadManager;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpCalendarMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsDownloadAttachStatus;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;
import com.mediatek.mms.ipmessage.IpMessageConsts.IpMessageType;
import com.mediatek.mms.ipmessage.message.IpTextMessage;

public class NmsChatLongPressDialog implements MenuItem.OnMenuItemClickListener {

    public final static int sendViaText = 0;
    public final static int tryAgain = 1;
    public final static int copy = 2;
    public final static int forward = 3;
    public final static int share = 4;
    public final static int delete = 5;
    public final static int flagAsImportant = 6;
    public final static int removeImportant = 7;
    public final static int saveInSDcard = 8;
    public final static int continueTry = 9;
    public final static int viewDetails = 10;
    public final static int addToBookmark = 11;
    public final static int addToContacts = 12;

    private static ArrayList<Short> mForwardDownloadList = new ArrayList<Short>();
    private final static String TAG = "ChatLongPressDialog";

    private final String prefixHttp = "http://";
    private final String prefixHttps = "https://";
    private final String prefixPhone = "tel:";
    private final String prefixEmail = "mailto:";

    private Context mContext;
    private onItemClickListener mListener;
    private NmsIpMessage mMessage;
    private NmsContact mContact;
    private ContextMenu mMenu;
    private HashMap<Integer, String> mIdUrlMap;
    private ArrayList<String> mHttpUrls;
    private DownloadReceiver mReceiver;
    
    /// M: add for jira-526 @{
    private ProgressDialog mDownloadDialog;
    
    private int mUnDownloadedIpMessageCount = 0;
    private int mDownloadedIpMessageStepCounter;
    private int mDownloadedIpMessageStepCounterSuccess;
    private int mDownloadedIpMessageStepCounterFail;
    /// @}

    public NmsChatLongPressDialog(Context context) {
        mContext = context;
    }

    public void bind(NmsIpMessage msg, NmsContact contact, ContextMenu menu) {
        if (null == msg) {
            NmsLog.error(TAG, "failed to create dialog, msg is null ");
            return;
        }

        mMessage = msg;
        mContact = contact;
        mMenu = menu;

        if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_ADD_CFG
                || mMessage.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_CREATE_CFG
                || mMessage.type == NmsIpMessageConsts.NmsIpMessageType.GROUP_QUIT_CFG) {
            return;
        }

        mMenu.setHeaderTitle(R.string.STR_NMS_LONG_PRESS_TITLE);
        initStringList();
    }

    public short getId() {
        return (short) mMessage.ipDbId;
    }

    private void addItem(int cmdId, int resId) {
        mMenu.add(0, cmdId, 0, resId).setOnMenuItemClickListener(this);
    }

    private void addItem(int cmdId, String res) {
        mMenu.add(0, cmdId, 0, res).setOnMenuItemClickListener(this);
    }

    private void tryToAddTryAgainItem() {
        if (mMessage.status != NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
            return;
        }
        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            NmsGroupChatContact contact = (NmsGroupChatContact) mContact;
            if (!contact.isAlive()
                    || !MessageUtils.isCurrentSim(mContext, contact.getSimId())
                    || !(NmsIpMessageApiNative.nmsGetActivationStatus(contact.getSimId()) == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED)
                    || contact.getMemberCount() == 1) {
                return;
            }
        }
        addItem(tryAgain, R.string.STR_NMS_TRY_AGAIN);
    }

    private void tryToAddContinueTryItem() {
        if (mMessage.status != NmsIpMessageConsts.NmsIpMessageStatus.NOT_DELIVERED) {
            return;
        }
        addItem(continueTry, R.string.STR_NMS_CONTINUE_TRY);
    }

    private void tryToAddSendViaTextItem() {
        if (mMessage.status != NmsIpMessageConsts.NmsIpMessageStatus.FAILED) {
            return;
        }

        if (mMessage.type != NmsIpMessageConsts.NmsIpMessageType.TEXT) {
            return;
        }

        if (TextUtils.isEmpty(((NmsIpTextMessage) mMessage).body)) {
            return;
        }

        if (mContact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
            return;
        }

        addItem(sendViaText, R.string.STR_NMS_SEND_VIA_TEXT);
    }

    private void tryToAddForwardMsgItem() {
//        if (mMessage instanceof NmsIpAttachMessage) {
//            if (!(mMessage instanceof NmsIpLocationMessage)) {
//                NmsIpAttachMessage msg = (NmsIpAttachMessage) mMessage;
//                if (msg.isInboxMsgDownloalable()) {
//                    return;
//                }
//                if (TextUtils.isEmpty(msg.path) || !NmsCommonUtils.isExistsFile(msg.path)) {
//                    return;
//                }
//            }
//        }

        addItem(forward, R.string.STR_NMS_FORWARD);
    }

    private void tryToAddImportantMsgItem() {
        if ((mMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) == 0) {
            addItem(flagAsImportant, R.string.STR_NMS_FLAG_IMPORTANT);
        }
    }

    private void tryToAddCancelImportantMsgItem() {
        if ((mMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) != 0) {
            addItem(removeImportant, R.string.STR_NMS_REMOVE_IMPORTANT);
        }
    }

    private void tryToAddDeleteMsgItem() {
        addItem(delete, R.string.STR_NMS_DELETE);
    }

    private void tryToAddShareMsgItem() {
        if (mMessage instanceof NmsIpAttachMessage) {
            if (!(mMessage instanceof NmsIpLocationMessage)) {
                NmsIpAttachMessage msg = (NmsIpAttachMessage) mMessage;
                if (msg.isInboxMsgDownloalable()) {
                    return;
                }
                if (TextUtils.isEmpty(msg.path) || !NmsCommonUtils.isExistsFile(msg.path)) {
                    return;
                }
            }
        }

        addItem(share, R.string.STR_NMS_SHARE);
    }

    private void tryToAddStoreMsgItem() {
        if (!(mMessage instanceof NmsIpAttachMessage)) {
            return;
        }

        if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
            return;
        }

        if ((mMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
            return;
        }
        addItem(saveInSDcard, R.string.STR_NMS_SAVE_IN_SDCARD);
    }

    private void tryToAddCopyTextItem() {
        if (mMessage.type != NmsIpMessageConsts.NmsIpMessageType.TEXT) {
            return;
        }
        if (TextUtils.isEmpty(((NmsIpTextMessage) mMessage).body)) {
            return;
        }

        addItem(copy, R.string.STR_NMS_COPY);
    }

    private void addPhoneNumOrEmailAddressToContacts(int id) {
        Intent intent = new Intent(Intent.ACTION_INSERT_OR_EDIT);
        intent.setType(Contacts.CONTENT_ITEM_TYPE);
        String url = mIdUrlMap.get(id);
        if (url.startsWith(prefixEmail)) {
            String email = url.replace(prefixEmail, "");
            intent.putExtra(ContactsContract.Intents.Insert.EMAIL, email);
        } else {
            String phonenum = url.replace(prefixPhone, "");
            intent.putExtra(ContactsContract.Intents.Insert.PHONE, phonenum);
            intent.putExtra(ContactsContract.Intents.Insert.PHONE_TYPE,
                    ContactsContract.CommonDataKinds.Phone.TYPE_MOBILE);
        }
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
        mContext.startActivity(intent);
    }

    private void tryToAddAddBookmarkAndAddContact() {
        if (mMessage.type != NmsIpMessageConsts.NmsIpMessageType.TEXT) {
            return;
        }
        if (TextUtils.isEmpty(((NmsIpTextMessage) mMessage).body)) {
            return;
        }

        SpannableString sp = new SpannableString(((NmsIpTextMessage) mMessage).body);
        Linkify.addLinks(sp, Linkify.ALL);

        URLSpan[] urls = sp.getSpans(0, ((NmsIpTextMessage) mMessage).body.length(), URLSpan.class);

        if (urls.length == 0) {
            return;
        } else {
            boolean ifHaveHttpUrl = false;
            int cmdId = addToContacts;
            int httpIndex = 0;
            mIdUrlMap = new HashMap<Integer, String>();
            mHttpUrls = new ArrayList<String>();
            for (URLSpan url : urls) {
                String tempUrl = url.getURL();
                String tempStr = null;
                if (tempUrl.startsWith(prefixHttp) || tempUrl.startsWith(prefixHttps)) {
                    if (!ifHaveHttpUrl) {
                        addItem(addToBookmark, R.string.STR_NMS_ADD_TO_BOOKMARK);
                    }
                    ifHaveHttpUrl = true;
                    mHttpUrls.add(tempUrl);
                } else if (tempUrl.startsWith(prefixPhone)) {
                    tempStr = tempUrl.replace(prefixPhone, "");
                    if (NmsContactApi.getInstance(mContext).isExistSystemContactViaNumber(tempStr)) {
                        continue;
                    }
                    mIdUrlMap.put(cmdId, tempUrl);
                    addItem(cmdId, String.format(
                            mContext.getString(R.string.STR_NMS_ADD_TO_CONTACTS), tempStr));
                    cmdId++;
                } else if (tempUrl.startsWith(prefixEmail)) {
                    tempStr = tempUrl.replace(prefixEmail, "");
                    if (NmsContactApi.getInstance(mContext).isExistSystemContactViaEmail(tempStr)) {
                        continue;
                    }
                    mIdUrlMap.put(cmdId, tempUrl);
                    addItem(cmdId, String.format(
                            mContext.getString(R.string.STR_NMS_ADD_TO_CONTACTS), tempStr));
                    cmdId++;
                }
            }
        }

    }

    private void tryToAddViewDetailsItem() {
        addItem(viewDetails, R.string.STR_NMS_VIEW_DETAILS);
    }

    private void initStringList() {

        tryToAddSendViaTextItem();
        tryToAddTryAgainItem();
        // tryToAddContinueTryItem();
        tryToAddCopyTextItem();
        tryToAddForwardMsgItem();
        tryToAddShareMsgItem();
        tryToAddDeleteMsgItem();

        // if (MessageUtils.isActivateSimCard(mContext)) {
        // tryToAddImportantMsgItem();
        // tryToAddCancelImportantMsgItem();
        // }
        tryToAddImportantMsgItem();
        tryToAddCancelImportantMsgItem();

        tryToAddAddBookmarkAndAddContact();
        tryToAddViewDetailsItem();
        tryToAddStoreMsgItem();
    }

    private void forwardMessage() {
        NmsLog.trace(TAG, "forward msg protocol is : " + mMessage.protocol);
        
        if (NmsSMSMMSManager.getInstance(mContext).isExtentionFieldExsit() == 1) {
            Intent intent = new Intent("com.android.mms.ui.ComposeMessageActivity");
            intent.setClassName("com.android.mms", "com.android.mms.ui.ComposeMessageActivity");
            intent.putExtra("forwarded_message", true);
            if (mMessage.protocol == NmsIpMessageConsts.NmsMessageProtocol.IP) {
                intent.putExtra("forwarded_ip_message", true);
                intent.putExtra("ip_msg_id", mMessage.id);
            } else if (mMessage.protocol == NmsIpMessageConsts.NmsMessageProtocol.SMS) {
                intent.putExtra("sms_body", ((NmsIpTextMessage) mMessage).body);
            } else {
                return;
            }
            mContext.startActivity(intent);
        } else {
            
        }
    }
    
    /// M: add for jira-526   
    private void showIpMessageDownloadDlg(NmsIpMessage msgItem) {
        mReceiver = new DownloadReceiver();
        IntentFilter filter = new IntentFilter(
                NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION);
        mContext.registerReceiver(mReceiver, filter);

        mForwardDownloadList.add((short) mMessage.ipDbId);
        if (!NmsDownloadManager.getInstance().nmsIsDownloading(mMessage.ipDbId, mMessage.id)) {
            NmsDownloadManager.getInstance().nmsDownload(mMessage.ipDbId, 0);
        }

        mUnDownloadedIpMessageCount = 1;
        mDownloadedIpMessageStepCounter = 0;

        if (mDownloadDialog == null) {
            String strTmp;

            mDownloadDialog = new ProgressDialog(mContext);
            mDownloadDialog.setCancelable(true);
            mDownloadDialog.setTitle(R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE1);
            mDownloadDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);

            strTmp = String.format(mContext.getString(R.string.STR_NMS_FORWARD_DOWNLOAD_INFO1),
                    mDownloadedIpMessageStepCounter, mUnDownloadedIpMessageCount);
            mDownloadDialog.setMessage(strTmp);

            mDownloadDialog.setButton(mContext.getText(R.string.STR_NMS_CANCEL),
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int i) {
                            if (mDownloadDialog != null) {
                                mDownloadDialog.dismiss();
                                mDownloadDialog = null;
                            }
                        }
                    });
        }

        mDownloadDialog.show();
    }
    
    /// @}

    private void preForwardMessage() {
        if (mMessage instanceof NmsIpAttachMessage && !(mMessage instanceof NmsIpLocationMessage)
                && ((NmsIpAttachMessage) mMessage).isInboxMsgDownloalable()) {
            
            showIpMessageDownloadDlg(mMessage);
            return;
        } else {
            forwardMessage();
        }
    }

    private void saveMsgInSDCard() {
        if (!NmsCommonUtils.getSDCardStatus()) {
            MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SAVE);
            return;
        }

        long availableSpace = NmsCommonUtils.getSDcardAvailableSpace();
        int size = ((NmsIpAttachMessage) mMessage).size;

        if (availableSpace <= 5 * 1024 || availableSpace <= size) {
            // new
            // AlertDialog.Builder(mContext).setTitle(R.string.STR_NMS_TIPTITLE)
            // .setMessage(R.string.STR_NMS_SDSPACE_NOT_ENOUGH)
            // .setPositiveButton(R.string.STR_NMS_OK, null).create().show();
            Toast.makeText(mContext, R.string.STR_NMS_SDSPACE_NOT_ENOUGH, Toast.LENGTH_LONG).show();
            return;
        }

        String source = ((NmsIpAttachMessage) mMessage).path;
        String attName = source.substring(source.lastIndexOf("/") + 1);
        String dstFile = "";
        dstFile = NmsCommonUtils.getCachePath(mContext) + attName;
        int i = 1;
        while (NmsCommonUtils.isExistsFile(dstFile)) {
            dstFile = NmsCommonUtils.getCachePath(mContext) + "(" + i + ")" + attName;
            i++;
        }
        NmsCommonUtils.copy(source, dstFile);
        
        /// M: add for ipmessage 89 platfrom @{
        Intent intent = new Intent(android.content.Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
        Uri uri = Uri.fromFile(new File(dstFile));
        intent.setData(uri);
        mContext.sendBroadcast(intent);
        /// }@
        
        String saveSuccess = String.format(mContext.getString(R.string.STR_NMS_SAVE_FILE), dstFile);
        Toast.makeText(mContext, saveSuccess, Toast.LENGTH_SHORT).show();
    }

    private void deleteMessage() {
        int resId = 0;
        int titleResId = 0;
        if ((mMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_IMPORTANT) != 0) {
            resId = R.string.STR_NMS_DELETE_IMPORTANT_WARNING;
            titleResId = R.string.STR_NMS_DELETE_IMPORTANT_TITLE;
        } else {
            resId = R.string.STR_NMS_DELETE_WARNING;
            titleResId = R.string.STR_NMS_DELETE_TITLE;
        }
        new AlertDialog.Builder(mContext).setIcon(R.drawable.ic_dialog_alert_holo_light)
                .setTitle(titleResId).setMessage(resId)
                .setPositiveButton(R.string.STR_NMS_DELETE, new OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        short[] id = { (short) mMessage.ipDbId };
                        NmsIpMessageApiNative.nmsDeleteIpMsg(id, true, true);
                        mListener.onItemClick(delete);
                    }
                }).setNegativeButton(R.string.STR_NMS_CANCEL, null).create().show();

    }

    private void shareMsg() {
        if (mMessage instanceof NmsIpAttachMessage) {
            if (!NmsCommonUtils.getSDCardStatus()) {
                MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SHARE);
                return;
            }

            long availableSpace = NmsCommonUtils.getSDcardAvailableSpace();
            int size = ((NmsIpAttachMessage) mMessage).size;

            if (availableSpace <= 5 * 1024 || availableSpace <= size) {
                // new
                // AlertDialog.Builder(mContext).setTitle(R.string.STR_NMS_TIPTITLE)
                // .setMessage(R.string.STR_NMS_SDSPACE_NOT_ENOUGH)
                // .setPositiveButton(R.string.STR_NMS_OK,
                // null).create().show();
                Toast.makeText(mContext, R.string.STR_NMS_SDSPACE_NOT_ENOUGH, Toast.LENGTH_LONG)
                        .show();
                return;
            }
        }
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent = setIntent(intent);
        intent.putExtra(Intent.EXTRA_SUBJECT, R.string.STR_NMS_MAIN);
        try {
            mContext.startActivity(Intent.createChooser(intent,
                    mContext.getString(R.string.STR_NMS_SHARE_TITLE)));
        } catch (Exception e) {
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    private void viewMsgDetails() {
        mListener.onItemClick(viewDetails);
    }

    private Intent setIntent(Intent intent) {
        if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
            intent.setType("text/plain");
            intent.putExtra(Intent.EXTRA_TEXT, ((NmsIpTextMessage) mMessage).body);
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE
                || mMessage.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
            if ((mMessage.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
                intent.setType("text/plain");
                intent.putExtra(Intent.EXTRA_TEXT, mContext.getResources().getString(R.string.STR_NMS_READED_BURN_PIC));
            }else{
            NmsIpImageMessage msg = (NmsIpImageMessage) mMessage;
            int index = msg.path.lastIndexOf(".");
            if (msg.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
                index = msg.path.indexOf(".ske.png");
            }
            String end = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + end;
            NmsCommonUtils.copy(msg.path, dest);
            intent.setType("image/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (msg.caption != null) {
                intent.putExtra("sms_body", msg.caption);
                intent.putExtra(Intent.EXTRA_TEXT, msg.caption);
                }
            }
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
            NmsIpVoiceMessage msg = (NmsIpVoiceMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + name;
            NmsCommonUtils.copy(msg.path, dest);
            intent.setType("audio/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (msg.caption != null) {
                intent.putExtra("sms_body", msg.caption);
                intent.putExtra(Intent.EXTRA_TEXT, msg.caption);
            }
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VCARD) {
            NmsIpVCardMessage msg = (NmsIpVCardMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + name;
            NmsCommonUtils.copy(msg.path, dest);
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.setType("text/x-vcard");
            intent.putExtra(Intent.EXTRA_STREAM, u);
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
            NmsIpVideoMessage msg = (NmsIpVideoMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + name;
            NmsCommonUtils.copy(msg.path, dest);
            intent.setType("video/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
            if (msg.caption != null) {
                intent.putExtra("sms_body", msg.caption);
                intent.putExtra(Intent.EXTRA_TEXT, msg.caption);
            }
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.LOCATION) {
            NmsIpLocationMessage msg = (NmsIpLocationMessage) mMessage;
            if (TextUtils.isEmpty(msg.path)) {
                intent.setType("text/plain");
                intent.putExtra(Intent.EXTRA_TEXT, msg.address);
            } else {
                int index = msg.path.lastIndexOf(".map.png");
                String end = msg.path.substring(index);
                String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + end;
                NmsCommonUtils.copy(msg.path, dest);
                intent.setType("image/*");
                File f = new File(dest);
                Uri u = Uri.fromFile(f);
                intent.putExtra(Intent.EXTRA_STREAM, u);
            }
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.CALENDAR) {
            NmsIpCalendarMessage msg = (NmsIpCalendarMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + name;
            NmsCommonUtils.copy(msg.path, dest);
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.setType("text/x-vcalendar");
            intent.putExtra(Intent.EXTRA_STREAM, u);
        } else {
            intent.setType("unknown");
        }
        return intent;
    }

    private void copyToClipboard(String s) {
        ClipboardManager cm = (ClipboardManager) mContext
                .getSystemService(Context.CLIPBOARD_SERVICE);
        cm.setText(s);
    }

    public void setOnItemClick(onItemClickListener l) {
        mListener = l;
    }

    public interface onItemClickListener {
        public void onItemClick(int action);
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        int itemId = item.getItemId();
        short msgIds[] = new short[1];
        switch (itemId) {

        case sendViaText:
            break;

        case tryAgain:
            engineadapter.get().nmsUIResendMsg(
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(),
                    (short) mMessage.ipDbId, 0);
            mListener.onItemClick(itemId);
            break;

        case copy:
            copyToClipboard(((NmsIpTextMessage) mMessage).body);
            break;

        case forward:
            preForwardMessage();
            break;

        case delete:
            deleteMessage();
            break;

        case share:
            shareMsg();
            break;

        case flagAsImportant:
            msgIds[0] = (short) mMessage.ipDbId;
            NmsIpMessageApiNative.nmsAddMsgToImportantList(msgIds);
            mListener.onItemClick(itemId);
            break;

        case removeImportant:
            msgIds[0] = (short) mMessage.ipDbId;
            NmsIpMessageApiNative.nmsDeleteMsgFromImportantList(msgIds);
            mListener.onItemClick(itemId);
            break;

        case saveInSDcard:
            NmsIpAttachMessage msg = (NmsIpAttachMessage) mMessage;
              if (msg.isInboxMsgDownloalable() || TextUtils.isEmpty(msg.path) || !NmsCommonUtils.isExistsFile(msg.path)) {
                Toast.makeText(mContext, R.string.imsp_save_chat_history_failed, Toast.LENGTH_SHORT).show() ;
                break ;
              }
            saveMsgInSDCard();
            break;
        case viewDetails:
            viewMsgDetails();
            break;
        case addToBookmark:
            if (1 == mHttpUrls.size()) {
                Browser.saveBookmark(mContext, null, mHttpUrls.get(0).toString());
            } else {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.STR_NMS_ADD_TO_BOOKMARK)
                        .setItems((String[]) mHttpUrls.toArray(new String[mHttpUrls.size()]),
                                new OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        Browser.saveBookmark(mContext, null, mHttpUrls.get(which)
                                                .toString());
                                    }
                                }).create().show();
            }
            break;
        default:
            if (mIdUrlMap != null && mIdUrlMap.containsKey(itemId)) {
                addPhoneNumOrEmailAddressToContacts(itemId);
            } else {
                NmsLog.trace(TAG, "unknow action not handle, action: " + itemId);
            }
            break;
        }
        return false;
    }

    public static boolean isForwardDonwload(short dbId){
        NmsLog.trace(TAG, "mForwardDownloadList size: " + mForwardDownloadList.size());
        return mForwardDownloadList.contains(dbId);
    }
    private class DownloadReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            String strTmp;
            NmsLog.trace(TAG, "DownloadReceiver action is: " + action);
            if (TextUtils.isEmpty(action)) {
                return;
            }

            int status = intent.getIntExtra(
                    NmsIpMessageConsts.NmsDownloadAttachStatus.NMS_DOWNLOAD_ATTACH_STATUS_ACTION,
                    -2);

            short dbId = (short) intent.getLongExtra(NmsDownloadAttachStatus.NMS_DOWNLOAD_MSG_ID,
                    -1);

            if (dbId == mMessage.ipDbId
                    && (status == NmsIpMessageConsts.NmsDownloadAttachStatus.DONE || status == NmsIpMessageConsts.NmsDownloadAttachStatus.FAILED)) {
                if (mDownloadDialog == null)
                    return;

                if (mDownloadDialog != null) {
                    mDownloadDialog.dismiss();
                    mDownloadDialog = null;
                }

                if (status == NmsIpMessageConsts.NmsDownloadAttachStatus.DONE) {
                    forwardMessage();
                } else {
                    strTmp = String.format(
                            mContext.getString(R.string.STR_NMS_FORWARD_DOWNLOAD_FAILED));

                    AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
                    builder.setTitle(R.string.STR_NMS_FORWARD_DOWNLOAD_TITLE)
                            .setCancelable(true)
                            .setPositiveButton(R.string.STR_NMS_RETRY,
                                    new DialogInterface.OnClickListener() {
                                        public final void onClick(DialogInterface dialog, int which) {
                                            dialog.dismiss();
                                            mUnDownloadedIpMessageCount = 0;
                                            showIpMessageDownloadDlg(mMessage);
                                            return;
                                        }
                                    }).setNegativeButton(R.string.STR_NMS_CANCEL, null)
                            .setMessage(strTmp).show();
                }
                mForwardDownloadList.remove((Short) dbId);
                mContext.unregisterReceiver(mReceiver);
            }
        }

    }
}
