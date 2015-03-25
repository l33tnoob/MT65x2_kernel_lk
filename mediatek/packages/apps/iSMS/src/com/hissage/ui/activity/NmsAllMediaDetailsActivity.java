package com.hissage.ui.activity;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsiSMSApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.location.NmsLocationManager;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImFlag;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImReadMode;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsMessageProtocol;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.ip.NmsIpVoiceMessage;
import com.hissage.message.smsmms.NmsSMSMMS;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.ui.view.NmsLevelControlLayout;
import com.hissage.ui.view.NmsLevelControlLayout.OnScrollToScreenListener;
import com.hissage.ui.view.NmsMultimediaAudioView;
import com.hissage.ui.view.NmsMultimediaBaseView;
import com.hissage.ui.view.NmsMultimediaImageView;
import com.hissage.ui.view.NmsMultimediaLocationView;
import com.hissage.ui.view.NmsMultimediaMMSView;
import com.hissage.ui.view.NmsMultimediaVideoView;
import com.hissage.util.data.NmsImportantList;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsAllMediaDetailsActivity extends Activity {

    public enum MediaType {
        IMAGE, LOCATION, VIDEO, AUDIO, MMS
    }

    public enum ViewMode {
        ALL, SINGLE
    }

    private final static String TAG = "AllMediaDetailsActivity";

    private final static int HANDLER_DATA_ERROR = 1000;
    private final static int HANDLER_DATA_EMPTY = 1001;
    private final static int HANDLER_DATA_READY = 1002;

    private Context mContext;
    public List<NmsIpMessage> ipMediaList = new ArrayList<NmsIpMessage>();
    private NmsContact mContact;
    private NmsIpMessage mMessage;
    private MediaType mediaType;
    private NmsLevelControlLayout viewContainer;
    private OnScrollToScreenListener scrollListener;
    private TextView tvNumTitle;
    private int currentIndex;
    private ViewMode viewMode;
    private LinearLayout tipContainer;
    private ArrayList<NmsMultimediaBaseView> mListMediaView = new ArrayList<NmsMultimediaBaseView>();
    private int type = 0;
    private long msgId = -1;
    private short ipDbId = -1;
    private MenuItem shareItem;

    private boolean ready = false;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            NmsLog.trace(TAG, "handler received msg type: " + msg.what);
            switch (msg.what) {
            case HANDLER_DATA_ERROR:
                break;
            case HANDLER_DATA_READY:
                ready = true;
                initMediaPage();
                break;
            default:
                break;
            }
        }

    };

    private void initSinglePage() {
        NmsMultimediaBaseView convertView = null;
        if (mMessage instanceof NmsIpImageMessage) {
            convertView = new NmsMultimediaImageView(this, (NmsIpImageMessage) mMessage);
            mediaType = MediaType.IMAGE;
        } else if (mMessage instanceof NmsIpVideoMessage) {
            convertView = new NmsMultimediaVideoView(this, (NmsIpVideoMessage) mMessage);
            mediaType = MediaType.VIDEO;
        } else if (mMessage instanceof NmsIpVoiceMessage) {
            convertView = new NmsMultimediaAudioView(this, (NmsIpVoiceMessage) mMessage);
            mediaType = MediaType.AUDIO;
        } else if (mMessage instanceof NmsIpLocationMessage) {
            convertView = new NmsMultimediaLocationView(this, (NmsIpLocationMessage) mMessage);
            mediaType = MediaType.LOCATION;
        } else {
            convertView = new NmsMultimediaMMSView(this, mMessage);
            mediaType = MediaType.MMS;
        }

        convertView.setMediaImage();
        mListMediaView.add(convertView);
        viewContainer.addView(convertView);
        viewContainer.setDefaultScreen(0);
    }
    
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putLong("msgId", msgId);
        outState.putInt("ipDbId", ipDbId);
        outState.putInt("type", type);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.all_media_detail);
        viewContainer = (NmsLevelControlLayout) findViewById(R.id.sl_media);

        mContext = this;
        if (null == savedInstanceState) {
            msgId = this.getIntent().getLongExtra("msgId", (long) -1);
            ipDbId = (short) this.getIntent().getIntExtra("ipDbId", -1);
            type = this.getIntent().getIntExtra("type", 0);
        } else {
            msgId = savedInstanceState.getLong("msgId", (long) -1);
            ipDbId = (short) savedInstanceState.getInt("ipDbId", -1);
            type = savedInstanceState.getInt("type", 0);
        }

        if (msgId > 0) {
            mMessage = NmsIpMessageApiNative.nmsGetIpMsgInfo(msgId);
            mContact = NmsIpMessageApiNative.nmsGetContactInfoViaMsgId(msgId);
        } else {
            mMessage = NmsiSMSApi.nmsGetIpMsgInfoViaDbId(ipDbId);
            mContact = NmsiSMSApi.nmsGetContactInfoViaDbId(ipDbId);
        }

        if (mContact == null || mMessage == null) {
            NmsLog.error(TAG, "The contact is null and msgId:" + msgId + " ipDBid:" + ipDbId);
            this.finish();
            return;
        }

        if ((ipDbId > 0 || msgId > 0) && type == 0) {
            viewMode = ViewMode.SINGLE;
            initSinglePage();
        } else {
            viewContainer.setVisibility(View.GONE);
            viewMode = ViewMode.ALL;
            if (mMessage instanceof NmsIpImageMessage) {
                mediaType = MediaType.IMAGE;
            } else if (mMessage instanceof NmsIpLocationMessage) {
                mediaType = MediaType.LOCATION;
            } else {
                mediaType = MediaType.VIDEO;
            }

            tipContainer = (LinearLayout) findViewById(R.id.ll_prompt);
            loadAllMedia();
        }

        initActionBar();
    }

    private void initMediaPage() {

        tipContainer.setVisibility(View.GONE);
        viewContainer.setVisibility(View.VISIBLE);

        NmsMultimediaBaseView convertView = null;

        for (int i = 0; i < ipMediaList.size(); i++) {

            final NmsIpMessage session = ipMediaList.get(i);

            if (session instanceof NmsIpImageMessage) {
                convertView = new NmsMultimediaImageView(this, (NmsIpImageMessage) session);
            } else if (session instanceof NmsIpVideoMessage) {
                convertView = new NmsMultimediaVideoView(this, (NmsIpVideoMessage) session);
            } else if (session instanceof NmsIpVoiceMessage) {
                convertView = new NmsMultimediaAudioView(this, (NmsIpVoiceMessage) session);
            } else if (session instanceof NmsIpLocationMessage) {
                convertView = new NmsMultimediaLocationView(this, (NmsIpLocationMessage) session);
            } else {
                convertView = new NmsMultimediaMMSView(this, (NmsIpMessage) session);
            }

            if (currentIndex == i) {
                convertView.setMediaImage();
            }
            mListMediaView.add(convertView);
            viewContainer.addView(convertView);
        }

        scrollListener = new OnScrollToScreenListener() {
            @Override
            public void doAction(int whichScreen) {
                if (whichScreen >= ipMediaList.size()) {
                    return;
                }

                mMessage = ipMediaList.get(whichScreen);
                if (mMessage.protocol == NmsMessageProtocol.MMS) {
                    mediaType = MediaType.MMS;
                } else if (mMessage instanceof NmsIpImageMessage) {
                    mediaType = MediaType.IMAGE;
                } else if (mMessage instanceof NmsIpLocationMessage) {
                    mediaType = MediaType.LOCATION;
                } else {
                    mediaType = MediaType.VIDEO;
                }

                currentIndex = whichScreen;
                tvNumTitle.setText(currentIndex + 1 + " of " + mListMediaView.size());

                destoryOrSetPartMedia(whichScreen);
                resetOptionItem();
            }
        };

        viewContainer.setOnScrollToScreen(scrollListener);
        viewContainer.setDefaultScreen(currentIndex);
        if (viewMode == ViewMode.SINGLE) {
            viewContainer.setTouchMove(false);
        } else {
            viewContainer.setTouchMove(true);
        }
    }

    private void resetOptionItem() {
        if (shareItem != null) {
            if (mediaType == MediaType.MMS) {
                shareItem.setVisible(false);
            } else {
                shareItem.setVisible(true);
            }
        }
    }

    private void destoryOrSetPartMedia(int whichScreen) {
        if (whichScreen >= 0 && whichScreen <= ipMediaList.size() - 1) {
            if (whichScreen + 1 <= ipMediaList.size() - 1) {
                NmsMultimediaBaseView nextConvertView = mListMediaView.get(whichScreen + 1);
                nextConvertView.setMediaImage();
            }

            if (whichScreen - 1 >= 0) {
                NmsMultimediaBaseView preConvertView = mListMediaView.get(whichScreen - 1);
                preConvertView.setMediaImage();
            }

            if (whichScreen - 2 >= 0 && whichScreen + 2 <= mListMediaView.size() - 1) {
                NmsMultimediaBaseView ppreConvertView = mListMediaView.get(whichScreen - 2);
                NmsMultimediaBaseView nnextConvertView = mListMediaView.get(whichScreen + 2);
                nnextConvertView.destoryMediaImage();
                ppreConvertView.destoryMediaImage();
            }
        }
    }

    private void loadAllMedia() {
        NmsLog.trace(TAG, "thread to load the list info");

        new Thread() {
            @Override
            public void run() {

                SNmsImMsgCountInfo mMsgCountInfo = NmsiSMSApi.nmsSetImMode((int) mContact.getId(),
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

                    if (mediaType == MediaType.LOCATION) {
                        if (ipMsg.type == NmsIpMessageType.LOCATION) {
                            if (!((NmsIpAttachMessage) ipMsg).isInboxMsgDownloalable()) {
                                ipMediaList.add(ipMsg);
                            }
                        }
                    } else {

                        if (ipMsg.protocol == NmsMessageProtocol.MMS) {
                            if (NmsSMSMMSManager.getInstance(NmsAllMediaDetailsActivity.this)
                                    .isMmsDownloaded(ipMsg.id)) {
                                ipMediaList.add(ipMsg);
                            }
                        } else if (ipMsg.type == NmsIpMessageType.PICTURE
                                || ipMsg.type == NmsIpMessageType.VOICE
                                || ipMsg.type == NmsIpMessageType.SKETCH
                                || ipMsg.type == NmsIpMessageType.VIDEO) {
                            if (!((NmsIpAttachMessage) ipMsg).isInboxMsgDownloalable()) {
                                if ((ipMsg.flag & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) == 0) {
                                    ipMediaList.add(ipMsg);
                                }
                            }
                        }
                    }
                }

                sortList();

                initCurrIndex();

                NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_READY);
                mHandler.sendEmptyMessage(HANDLER_DATA_READY);
            }
        }.start();
    }

    private void initActionBar() {
        ActionBar actionBar = getActionBar();

        final int MASK = ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_SHOW_HOME
                | ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_CUSTOM;

        final int current = actionBar.getDisplayOptions() & MASK;

        int newFlags = 0;
        newFlags |= ActionBar.DISPLAY_SHOW_TITLE;

        newFlags |= ActionBar.DISPLAY_HOME_AS_UP;
        newFlags |= ActionBar.DISPLAY_SHOW_TITLE;

        if (current != newFlags) {
            actionBar.setDisplayOptions(newFlags, MASK);
        }

        ViewGroup v = (ViewGroup) LayoutInflater.from(this)
                .inflate(R.layout.action_bar_media, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                        | Gravity.CENTER_HORIZONTAL));

        tvNumTitle = (TextView) v.findViewById(R.id.tv_title_number);
        ImageButton ibPre = (ImageButton) v.findViewById(R.id.ib_navigate);

        Button tvTitle = (Button) v.findViewById(R.id.tv_title);

        tvTitle.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                NmsAllMediaDetailsActivity.this.finish();
            }
        });

        ImageButton ivBack = (ImageButton) v.findViewById(R.id.ib_back);
        if (mediaType == MediaType.LOCATION) {
            ivBack.setImageResource(R.drawable.ic_location);
        } else {
            ivBack.setImageResource(R.drawable.ic_menu_back);
        }
        if (mediaType != MediaType.LOCATION && viewMode == ViewMode.ALL) {
            tvTitle.setText(R.string.STR_NMS_ALL_MEDIA_TITLE);
        } else if (mediaType == MediaType.LOCATION && viewMode == ViewMode.ALL) {
            tvTitle.setText(R.string.STR_NMS_ALL_LOCATIONS_TITLE);
        } else if (viewMode == ViewMode.SINGLE) {
            ibPre.setVisibility(View.GONE);
            tvTitle.setVisibility(View.GONE);
            actionBar.setDisplayShowCustomEnabled(true);
            actionBar.setDisplayShowHomeEnabled(true);
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setDisplayShowTitleEnabled(false);
            actionBar.setDisplayUseLogoEnabled(true);

            tvNumTitle.setVisibility(View.GONE);

            if (mContact != null) {
                Bitmap avatar = NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId(
                        mContact.getId());
                if (avatar != null) {
                    actionBar.setLogo(new BitmapDrawable(avatar));
                } else {
                    actionBar.setLogo(R.drawable.ic_contact_picture);
                }
            }
        }

        ivBack.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                openAllMedia();
            }
        });

    }

    public void openAllMedia() {
        if (mediaType == MediaType.LOCATION) {
            Intent i = new Intent(this, NmsAllLocationsActivity.class);
            i.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            i.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mContact.getId());
            this.finish();
            startActivity(i);
        } else {
            Intent i = new Intent(this, NmsAllMediaActivity.class);
            i.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            i.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mContact.getId());
            this.finish();
            startActivity(i);
        }
    }

    private void initCurrIndex() {
        if (ipMediaList == null) {
            NmsLog.error(TAG, "ipLocMsgList is null");
            return;
        }

        for (int i = 0; i < ipMediaList.size(); ++i) {
            NmsIpMessage ipMsg = ipMediaList.get(i);
            if (ipMsg == null) {
                NmsLog.error(TAG, "initCurrIndex. ipMsg is null");
                continue;
            }
            if (ipMsg.ipDbId == mMessage.ipDbId) {
                NmsLog.trace(TAG, "currIndex is " + ipMsg.ipDbId);
                currentIndex = i;
                break;
            }
        }
    }

    private void sortList() {
        if (ipMediaList == null) {
            NmsLog.error(TAG, "ipMediaList is null");
            return;
        }

        if (ipMediaList.size() == 0) {
            NmsLog.warn(TAG, "ipMediaList.size() is zero, do not sort");
            return;
        }

        Comparator<NmsIpMessage> timeComparator = new Comparator<NmsIpMessage>() {
            @Override
            public int compare(NmsIpMessage lhs, NmsIpMessage rhs) {
                if (lhs == null || rhs == null) {
                    NmsLog.error(TAG, "lhs or rhs is/are null");
                    return 0;
                }

                if (mediaType == MediaType.LOCATION) {
                    return rhs.time - lhs.time;
                } else {
                    return lhs.time - rhs.time;
                }
            }
        };

        Collections.sort(ipMediaList, timeComparator);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        MenuInflater inflater = getMenuInflater();
        if (mediaType == MediaType.LOCATION) {
            inflater.inflate(R.menu.location_detail_menu, menu);
        } else {
            inflater.inflate(R.menu.media_detail_menu, menu);
            shareItem = menu.findItem(R.id.menu_media_share);
            resetOptionItem();
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!ready && viewMode == ViewMode.ALL) {
            return false;
        }

        if (item.getItemId() == android.R.id.home) {
            finish();
        } else if (item.getItemId() == R.id.menu_media_trash) {
            deleteMedia();
        } else if (item.getItemId() == R.id.menu_media_share) {
            shareMedia();
        } else if (item.getItemId() == R.id.menu_location_map) {
            openMap();
        } else {
            return true;
        }

        return false;
    }

    public void playMedia() {
        NmsMultimediaBaseView view = mListMediaView.get(currentIndex);
        view.play();
    }

    public void deleteMedia() {
        new AlertDialog.Builder(this).setTitle(R.string.STR_NMS_DELETE_TITLE)
                .setMessage(getString(R.string.STR_NMS_DELETE_MEDIA))
                .setNegativeButton(R.string.STR_NMS_CANCEL, null)
                .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        delete();
                    }
                }).show();
    }

    private void delete() {
        short[] id = { (short) mMessage.ipDbId };
        if (NmsImportantList.get().isContains((short) mMessage.ipDbId)) {
            NmsImportantList.get().removeElement((short) mMessage.ipDbId);
        }

        if (mMessage.protocol == NmsMessageProtocol.MMS) {
            NmsSMSMMSManager.getInstance(NmsAllMediaDetailsActivity.this).deleteSMS(
                    NmsSMSMMS.PROTOCOL_MMS, new int[] { (int) mMessage.id }, true);
        } else {
            NmsIpMessageApiNative.nmsDeleteIpMsg(id, true, true);
        }

        if (viewMode == ViewMode.SINGLE) {
            finish();
        } else {
            if (mListMediaView.size() == 1) {
                finish();
                return;
            }

            viewContainer.removeViewAt(currentIndex);
            mListMediaView.remove(currentIndex);
            ipMediaList.remove(currentIndex);

            if (currentIndex == mListMediaView.size()) {
                currentIndex--;
                viewContainer.setDefaultScreen(currentIndex);

            }

        }
    }

    private void openMap() {
        if (mMessage instanceof NmsIpLocationMessage) {
            String uriString = NmsLocationManager.getUrl((NmsIpLocationMessage) mMessage);
            if (!TextUtils.isEmpty(uriString)) {
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(uriString));
                try {
                    startActivity(intent);
                } catch (Exception e) {
                    Toast.makeText(this, R.string.STR_NMS_NO_APP, Toast.LENGTH_SHORT).show();
                    NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
                }
            }
        }
    }

    private void shareMedia() {
        if (mMessage instanceof NmsIpAttachMessage) {
            if (!NmsCommonUtils.getSDCardStatus()) {
                MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SHARE);
                return;
            }
        }
        Intent intent = new Intent(Intent.ACTION_SEND);
        intent = setNewIntent(intent);
        intent.putExtra(Intent.EXTRA_SUBJECT, R.string.STR_NMS_MAIN);
        try {
            mContext.startActivity(Intent.createChooser(intent,
                    mContext.getString(R.string.STR_NMS_SHARE_TITLE)));
        } catch (Exception e) {
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(e));
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        viewContainer.autoRecovery();
        // NmsUtils.trace(Tag, "onConfigurationChange");
    }

    private Intent setNewIntent(Intent intent) {
        if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.TEXT) {
            intent.setType("text/plain");
            intent.putExtra(Intent.EXTRA_TEXT, ((NmsIpTextMessage) mMessage).body);
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.PICTURE
                || mMessage.type == NmsIpMessageConsts.NmsIpMessageType.SKETCH) {
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
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VOICE) {
            NmsIpVoiceMessage msg = (NmsIpVoiceMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + name;
            NmsCommonUtils.copy(msg.path, dest);
            intent.setType("audio/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VCARD) {
            NmsIpVCardMessage msg = (NmsIpVCardMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + name;
            NmsCommonUtils.copy(msg.path, dest);
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.setDataAndType(u, "text/x-vcard");
        } else if (mMessage.type == NmsIpMessageConsts.NmsIpMessageType.VIDEO) {
            NmsIpVideoMessage msg = (NmsIpVideoMessage) mMessage;
            int index = msg.path.lastIndexOf("/");
            String name = msg.path.substring(index);
            String dest = NmsCommonUtils.getCachePath(mContext) + "temp" + name;
            NmsCommonUtils.copy(msg.path, dest);
            intent.setType("video/*");
            File f = new File(dest);
            Uri u = Uri.fromFile(f);
            intent.putExtra(Intent.EXTRA_STREAM, u);
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
        } else {
            intent.setType("unknown");
        }
        return intent;
    }

}
