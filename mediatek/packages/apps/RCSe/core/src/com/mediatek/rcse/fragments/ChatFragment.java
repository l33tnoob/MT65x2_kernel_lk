/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.fragments;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.drm.OmaDrmStore;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;

import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.AssetFileDescriptor;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.provider.Telephony.ThreadSettings;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Space;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.SelectContactsActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.AsyncGalleryView;
import com.mediatek.rcse.activities.widgets.AttachmentTypeSelectorAdapter;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.ContactsListManager.OnDisplayNameChangedListener;
import com.mediatek.rcse.activities.widgets.UnreadMessagesContainer;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.emoticons.PageAdapter;
import com.mediatek.rcse.emoticons.ScrollLayout;
//import com.mediatek.rcse.fragments.One2OneChatFragment.CompressDialog;
//import com.mediatek.rcse.fragments.One2OneChatFragment.FileProcessingAsyncTask;
//import com.mediatek.rcse.fragments.One2OneChatFragment.RepickDialog;
//import com.mediatek.rcse.fragments.One2OneChatFragment.IChatMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
//import com.mediatek.rcse.fragments.One2OneChatFragment.WarningDialog;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;
import com.mediatek.rcse.service.NetworkChangedReceiver;
import com.mediatek.rcse.service.NetworkChangedReceiver.OnNetworkStatusChangedListerner;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettings.OnWallPaperChangedListener;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;
import android.provider.Telephony.MmsSms;


/**
 * A abstract class defines the common interface for chat.
 */
public abstract class ChatFragment extends Fragment implements OnDisplayNameChangedListener,
        OnNetworkStatusChangedListerner, PageAdapter.OnEmotionItemSelectedListener {

    private static final String TAG = "ChatFragment";
    protected Uri mCameraTempFileUri = null;
    private static final String CONTENT_IMAGE_TYPE = "/images/";
    private static final String CONTENT_AUDIO_TYPE = "/audio/";
    private static final String CONTENT_VIDEO_TYPE = "/video/";
    private static final String VCARD_SCHEMA = "content://com.android.contacts/contacts/as_vcard";
    private static final String ALL_VCARD_SCHEMA =
            "content://com.android.contacts/contacts/as_multi_vcard/";
    private static final String VCARD_DATA_TYPE = "text/x-vcard";
    private static final String VCARD_SUFFIX = ".vcf";
    private static final String VCALENDAR_SCHEMA = "content://com.mediatek.calendarimporter/";
    private static final String VCALENDAR_DATA_TYPE = "text/x-vcalendar";
    private static final String VCALENDAR_SUFFIX = ".vcs";
    private static final String READABLE_RIGHT = "r";

    public static final int GROUP_MIN_MEMBER_NUM = 2;

    public static final String OPEN_PAREN = "(";

    public static final String CLOSE_PAREN = ")";

    public static final int ONE = 1;

    protected static final long THREAD_ID_MAIN = 1;
    
    public static final int RESULT_CODE_CAMERA = 10;

    public static final int RESULT_CODE_GALLERY = 11;

    public static final int RESULT_CODE_FILE_MANAGER = 12;
    
    private static final String IMAGE_TYPE = "image/";
    
    public static final String GALLERY_TYPE = "image/*";
    
    public static final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";

    public static Uri MMS_CONENT_URI = Uri.withAppendedPath(MmsSms.CONTENT_URI, "thread_settings");

    protected RepickDialog mRepickDialog = new RepickDialog();

    public static final String SHOW_REMOTE_OFFLINE_REMINDER = "a";

    public static final String SHOW_NEW_MESSAGE_REMINDER = "b";

    public static final String SHOW_IS_TYPING_REMINDER = "c";

    // d is in groupchatfragment
    public static final String SHOW_ALL_PARTICIPANTS_LEFT = "e";

    public static final String SHOW_NETWORK_ERROR_REMINDER = "a";

    public static final String SHOW_OTHER_MESSAGE_REMINDER = "b";

    public static final String SHOW_BLOCKED_PARTICIPANT_MESSAGE_REMINDER = "c";

    protected static final String SPACE = "";

    public static final File SDCARDDIEFILE = Environment.getExternalStorageDirectory();

    public static final String SLASH = "/";

    public static final String RCSE_FILE_DIR = SDCARDDIEFILE + SLASH + "Joyn";

    public static final String RCSE_TEMP_FILE_DIR = RCSE_FILE_DIR + SLASH + "temp";

    public static final String RCSE_TEMP_FILE_NAME_HEADER = "tmp_joyn_";

    public static final String JPEG_SUFFIX = ".jpg";

    public static final int RESULT_CODE_ADD_CONTACTS = 13;
    
    protected CompressDialog mCompressDialog = null;

    protected static final String DATE_FORMAT_STRING = "yyyy-MM-dd";

    public static final int MAX_INPUT_REMIND_VIBRATOR_TIME = 100;

    protected static final String SEPERATOR_COLON = ":";

    protected static final String SEPERATOR_DOMAIN = "@";

    protected static final String RCSE_COMPRESSED_FILE_DIR = RCSE_FILE_DIR + SLASH + "compressed";

    public static final String WALLPAPER_PATH = "/data/data/com.android.providers.telephony/app_wallpaper";

    public static final String CONTENT_SCHEMA = "content://";

    public static final String FILE_SCHEMA = "FILE://";

    public static final String FILE_SCHEMA_SMALL = "file://";

    protected Object mTag = null;
    
    protected String mFileName = null;
    protected int mFiletransferEnableStatus = One2OneChat.FILETRANSFER_ENABLE_OK;
    protected WarningDialog mWarningDialog = new WarningDialog();
    protected boolean mIsBottom = true;

    protected AttachmentTypeSelectorAdapter mAttachmentTypeSelectorAdapter;

    protected DialogOfAddAttachment mDialogOfAddAttachment = new DialogOfAddAttachment();

    protected Boolean mIsNewMessageNotify = Boolean.FALSE;

    protected TreeSet<String> mTopReminderSortedSet = new TreeSet<String>();

    protected final TreeSet<String> mTextReminderSortedSet = new TreeSet<String>();

    private static final String COLON = ": ";

    protected ImageButton mBtnEmotion = null;

    // Emotion
    public static final int ITEMS_PER_PAGE = 20;

    private static final int ORIENTATION_LANDSCAPE_ITEMS_PER_LINE = 10;

    private static final int ORIENTATION_PORTRAIT_ITEMS_PER_LINE = 5;

    private static final int PAGE_INDICATOR_PADDING = 5;
    
    public static final int EMOTION_ICON_WIDTH = 48;

    public static final int EMOTION_ICON_HEIGHT = 48;

    private static final int PARTICIPANTS_LIST_WIDTH = 144;

    private final HashMap<Integer, List<Integer>> mResIdsMap = new HashMap<Integer, List<Integer>>();

    private final ArrayList<PageAdapter> mListenerList = new ArrayList<PageAdapter>();
    
    private final ArrayList<ImageView> mPageIndicators = new ArrayList<ImageView>();
    
    private LinearLayout mEmotionLayout;

    private LinearLayout mPageIndicator;

    private ScrollLayout mCurPageView;

    private GridView mGridView;

    private GridView mShareGridView = null;

    private ArrayList<Integer> mResIds;

    private int mPageCount;

    private int mCurItemsPerLine = ORIENTATION_PORTRAIT_ITEMS_PER_LINE;
    
    private int mCurrentPage;
    
    private InputMethodManager mInputMethodService;
    // Emotion

    private final WallPaperChanged mWallPaperChangedListener = new WallPaperChanged();

    protected Map<Integer, ISentChatMessage> mPreMessageMap = new ConcurrentHashMap<Integer, ISentChatMessage>();
    protected static final Date DEFAULT_PRE_SEND_MESSAGE_DATE = new Date();
    static {
        DEFAULT_PRE_SEND_MESSAGE_DATE.setTime(Long.MAX_VALUE);
    }

    private boolean mIsNetworkConnected = true;
    
    public static final int CHOOSE_A_PHOTO            = 0;
    public static final int TAKE_PICTURE            = 1;
    public static final int CHOOSE_FILE            = 2;
    private static final int[] SHARE_ACTIONS = {
        CHOOSE_A_PHOTO, TAKE_PICTURE, CHOOSE_FILE};
    
    /*private final String[] shareActions = {getString(R.string.choose_a_photo),
    	getString(R.string.take_a_photo), getString(R.string.choose_a_file)};*/
    
    private final String[] shareActions = {"Choose a photo",
        	"Take a photo", "Choose a file"};
    
    public static final int[] shareIconArray = { R.drawable.rcse_choose_a_photo,
    	R.drawable.rcse_take_a_photo, R.drawable.rcse_choose_a_file};
    
    
    /**
     * Empty constructor.
     */
    public ChatFragment() {

    }

    /**
     * Crate dir needed by rcse.
     */
    static {
        FileFactory.createDirectory(RCSE_FILE_DIR);
        FileFactory.createDirectory(RCSE_TEMP_FILE_DIR);
    }

    protected Handler mUiHandler = new Handler(Looper.getMainLooper());

    private final TextWatcher mTextWatcher = new TextWatcher() {
        private int mEditStart;

        private int mCount;

        @Override
        public void afterTextChanged(Editable s) {
            int length = s.length();
            if (length > ChatScreenActivity.MAX_CHAT_MSG_LENGTH) {
                Logger.i(TAG, "afterTextChanged, length out of max message size, s = " + s);
                int savedLength = ChatScreenActivity.MAX_CHAT_MSG_LENGTH - mCount;
                int deleteLength = length - ChatScreenActivity.MAX_CHAT_MSG_LENGTH;
                int selection = mEditStart + savedLength;
                s.delete(mEditStart + savedLength, mEditStart + savedLength + deleteLength);

                mMessageEditor.setText(s);
                mMessageEditor.setSelection(selection);
                Vibrator vibrator = (Vibrator) getActivity().getSystemService(
                        Context.VIBRATOR_SERVICE);
                AudioManager audioManager = (AudioManager) getActivity().getSystemService(
                        Context.AUDIO_SERVICE);

                if (vibrator != null && audioManager != null
                        && audioManager.getRingerMode() != AudioManager.RINGER_MODE_SILENT) {
                    vibrator.vibrate(MAX_INPUT_REMIND_VIBRATOR_TIME);
                } else {
                    Logger.e(TAG, "afterTextChanged, vibrator is null or is silent mode!");
                }
            } else {
                Logger.i(TAG, "afterTextChanged, s = " + s);
            }
            Logger.i(TAG, "afterTextChanged() the length is " + length);
            if (length > 0) {
                handleTextchanged(false);
            } else {
                handleTextchanged(true);
            }
            updateSendButtonState(s.toString());
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            mEditStart = start;
            mCount = s.length();
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }
    };

    protected View mContentView = null;
    protected View mBtnAddView = null;

    protected EditText mMessageEditor = null;

    protected TextView mTypingText = null;

    protected TextView mGroupParticipantsLeftText = null;

    protected TextView mRemoteOfflineText = null;

    protected TextView mMessageReminderText = null;

    protected TextView mNetworkErrorText = null;

    protected TextView mBlockedParticipantText = null;

    protected TextView mMgToOtherWinReminderText = null;

    protected ImageView mForwardToSettingsView = null;

    protected ImageButton mBtnSend = null;

    protected ListView mMessageListView = null;

    protected ChatAdapter mMessageAdapter = null;

    protected List<Participant> mParticipantList = null;

    protected abstract void onSend(String message);

    protected void handleTextchanged(boolean isEmpty) {
        Logger.i(TAG, "handleTextChanged()");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(ChatController.EVENT_TEXT_CHANGED,
                mTag, isEmpty);
        controllerMessage.sendToTarget();
    }

    
    protected void updateEmoticonLayout(boolean isSubjectDialog, View subjectView)
    {
		DisplayMetrics displayMetrics = getActivity().getApplicationContext()
				.getResources().getDisplayMetrics();
		boolean isPortrait = displayMetrics.widthPixels < displayMetrics.heightPixels;
		Configuration config = new Configuration();
		if (isPortrait) {
			config.orientation = Configuration.ORIENTATION_PORTRAIT;
		} else {
			config.orientation = Configuration.ORIENTATION_LANDSCAPE;
		}
    	if(isSubjectDialog)
    	{
    		
    	    mEmotionLayout = (LinearLayout) subjectView.findViewById(R.id.emotions_area_subject);
    	    mCurPageView = (ScrollLayout) subjectView.findViewById(R.id.chat_emotions_subject);
            mPageIndicator = (LinearLayout) subjectView.findViewById(R.id.page_indicator_subject);
            mMessageEditor = (EditText) subjectView.findViewById(R.id.inputEditText);
            initEmotionArea(config);
    	    
    	}
    	else{
    		//reset view for fragment
    		mCurPageView = (ScrollLayout) mContentView.findViewById(R.id.chat_emotions);
            mPageIndicator = (LinearLayout) mContentView.findViewById(R.id.page_indicator);
            mEmotionLayout = (LinearLayout) mContentView.findViewById(R.id.emotions_area);
            mBtnEmotion = (ImageButton) mContentView.findViewById(R.id.btn_chat_emoticon);
            mBtnEmotion.setOnClickListener(mBtnEmotionClickListener);
            mMessageEditor = (EditText) mContentView.findViewById(R.id.et_chat_message);
            initEmotionArea(config);
    	}
    }
    
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        // TODO
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        Logger.v(TAG, "ChatFragment onAttach(),activity = " + activity);
        ContactsListManager.getInstance().addListener(this);
    }

    protected void updateSendButtonState(String text) {
        if (text.length() > 0) {
            mBtnSend.setEnabled(true);
            mBtnSend.setFocusable(true);
        } else {
            mBtnSend.setEnabled(false);
            mBtnSend.setFocusable(false);
        }
    }

    protected void updateEmotionViewState(boolean isEnable) {
        if (isEnable) {
            mEmotionLayout.setVisibility(View.VISIBLE);
            mShareGridView.setVisibility(View.GONE);
        } else {
            mEmotionLayout.setVisibility(View.GONE);
        }
    }

    protected void showTopReminder() {
        Logger.d(TAG, "showTopReminder() entry");
        if (mTopReminderSortedSet != null) {
            int sortedSize = mTopReminderSortedSet.size();
            Logger.d(TAG, "showTopReminder() the mTopReminderSortedSet size is " + sortedSize);
            if (sortedSize > 0) {
                String reminder = mTopReminderSortedSet.first();
                Logger.d(TAG, "showTopReminder() the reminder is " + reminder);
                handleShowTopReminder(reminder);
            } else {
                handleClearTopReminder();
            }
        } else {
            Logger.e(TAG, "showReminder() the mTopReminderSortedSet is null");
        }
        Logger.d(TAG, "showTopReminder() exit");
    }

    /**
     * show the different type Reminder text view in the mTextReminderSortedSet, the first one has the Top priority, prior to
     * show the first one.
     */
    protected void showReminderList() {
        int sortedSize = mTextReminderSortedSet.size();
        Logger.w(TAG, "showReminderList() the mTextReminderSortedSet size is " + sortedSize);
        if (sortedSize > 0) {
            String reminder = mTextReminderSortedSet.first();
            Logger.d(TAG, "showReminderList() the reminder is " + reminder);
            handleShowReminder(reminder);
        } else {
            handleClearReminder();
        }
    }

    protected void handleShowTopReminder(String reminder) {
        Logger.d(TAG, "handleShowTopReminder() entry reminder is " + reminder);
        if (SHOW_NETWORK_ERROR_REMINDER.equals(reminder)) {
            if (mNetworkErrorText != null) {
                if (mMgToOtherWinReminderText != null) {
                    mMgToOtherWinReminderText.setVisibility(View.GONE);
                }
                if (mForwardToSettingsView != null) {
                    mForwardToSettingsView.setVisibility(View.VISIBLE);
                }
                if (mBlockedParticipantText != null) {
                	mBlockedParticipantText.setVisibility(View.GONE);
                }
                mNetworkErrorText.setVisibility(View.VISIBLE);
            }
        } else if (SHOW_OTHER_MESSAGE_REMINDER.equals(reminder)) {
            if (mMgToOtherWinReminderText != null) {
                if (mNetworkErrorText != null) {
                    mNetworkErrorText.setVisibility(View.GONE);
                }
                if (mForwardToSettingsView != null) {
                    mForwardToSettingsView.setVisibility(View.GONE);
                }
                if (mBlockedParticipantText != null) {
                	mBlockedParticipantText.setVisibility(View.GONE);
                }
                mMgToOtherWinReminderText.setVisibility(View.VISIBLE);
            }
        } else if (SHOW_BLOCKED_PARTICIPANT_MESSAGE_REMINDER.equals(reminder)) {
            if (mBlockedParticipantText != null) {
                if (mNetworkErrorText != null) {
                    mNetworkErrorText.setVisibility(View.GONE);
                }
                if (mForwardToSettingsView != null) {
                    mForwardToSettingsView.setVisibility(View.GONE);
                }
                if (mMgToOtherWinReminderText != null) {
                    mMgToOtherWinReminderText.setVisibility(View.GONE);
                }
                mBlockedParticipantText.setVisibility(View.VISIBLE);
            }
        } else {
            handleClearTopReminder();
        }
    }

    protected boolean handleShowReminder(String reminder) {
        if (SHOW_REMOTE_OFFLINE_REMINDER.equals(reminder)) {
            if (mRemoteOfflineText != null) {
                if (mTypingText != null) {
                    mTypingText.setVisibility(View.GONE);
                }
                if (mMessageReminderText != null) {
                    mMessageReminderText.setVisibility(View.GONE);
                }
                mRemoteOfflineText.setVisibility(View.VISIBLE);
            }
        } else if (SHOW_NEW_MESSAGE_REMINDER.equals(reminder)) {
            if (mMessageReminderText != null) {
                if (mRemoteOfflineText != null) {
                    mRemoteOfflineText.setVisibility(View.GONE);
                }
                if (mTypingText != null) {
                    mTypingText.setVisibility(View.GONE);
                }
                mMessageReminderText.setVisibility(View.VISIBLE);
            }
        } else if (SHOW_IS_TYPING_REMINDER.equals(reminder)) {
            if (mTypingText != null) {
                if (mRemoteOfflineText != null) {
                    mRemoteOfflineText.setVisibility(View.GONE);
                }
                if (mMessageReminderText != null) {
                    mMessageReminderText.setVisibility(View.GONE);
                }
                mTypingText.setVisibility(View.VISIBLE);
            }
        }
        else if(SHOW_ALL_PARTICIPANTS_LEFT.equals(reminder))
        {
        	if(mGroupParticipantsLeftText !=null)
        	{
        		if (mRemoteOfflineText != null) {
                    mRemoteOfflineText.setVisibility(View.GONE);
                }
                if (mMessageReminderText != null) {
                    mMessageReminderText.setVisibility(View.GONE);
                }
                if (mTypingText != null) {
                    mTypingText.setVisibility(View.GONE);
                }
                mGroupParticipantsLeftText.setVisibility(View.VISIBLE);
        	}
        }
        else {
            handleClearReminder();
        }
        return true;
    }

    protected void handleClearReminder() {
        Logger.d(TAG, "handleClearReminder() entry");
        if (mTypingText != null) {
            mTypingText.setVisibility(View.GONE);
        }
        if (mMessageReminderText != null) {
            mMessageReminderText.setVisibility(View.GONE);
        }
        if (mRemoteOfflineText != null) {
            mRemoteOfflineText.setVisibility(View.GONE);
        }
        if(mGroupParticipantsLeftText != null)
        {
        	mGroupParticipantsLeftText.setVisibility(View.GONE);
        }
    }

    protected void handleClearTopReminder() {
        Logger.d(TAG, "handleClearTopReminder() entry");
        if (mNetworkErrorText != null) {
            mNetworkErrorText.setVisibility(View.GONE);
        }
        if (mForwardToSettingsView != null) {
            mForwardToSettingsView.setVisibility(View.GONE);
        }
        if (mMgToOtherWinReminderText != null) {
            mMgToOtherWinReminderText.setVisibility(View.GONE);
        }
        Logger.d(TAG, "handleClearTopReminder() exit");
    }
    
    private OnScrollListener mOnScrollListener = new AbsListView.OnScrollListener() {

        @Override
        public void onScrollStateChanged(AbsListView view, int scrollState) {
        }

        @Override
        public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
                int totalItemCount) {
            Logger.d(TAG, "onScroll() totalItemCount: " + totalItemCount + " mMessageListView: "
                    + mMessageListView + " mMessageReminderText: " + mMessageReminderText);
            if (totalItemCount > 0) {
                if (mMessageListView != null) {
                    if (firstVisibleItem + visibleItemCount < totalItemCount) {
                        mMessageListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_NORMAL);
                        mIsBottom = false;
                        Logger.d(TAG, "onScroll it is not the bottom first is" + firstVisibleItem
                                + " visibleitem is " + visibleItemCount + " total is "
                                + totalItemCount);
                    } else {
                        mIsBottom = true;
                        mMessageListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
                        if (mIsNewMessageNotify.booleanValue()) {
                            Logger.d(TAG, "onScroll it is the bottom .mNewMessageNotify is "
                                    + mIsNewMessageNotify.booleanValue());
                            if (mMessageReminderText != null) {
                                mMessageReminderText.setText(SPACE);
                                mTextReminderSortedSet.remove(SHOW_NEW_MESSAGE_REMINDER);
                                showReminderList();
                            }
                            mIsNewMessageNotify = Boolean.FALSE;
                        }
                        Logger.d(TAG, "onScroll it is the bottom .first is" + firstVisibleItem
                                + " visibleitem is " + visibleItemCount + " total is "
                                + totalItemCount);
                    }
                }
            }
        }
    };

    private View.OnClickListener mMessageReminderClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Logger.d(TAG, "onClick() mMessageReminderText: " + mMessageReminderText
                    + " mMessageListView: " + mMessageListView + " mIsNewMessageNotify: "
                    + mIsNewMessageNotify.booleanValue());
            if (mIsNewMessageNotify.booleanValue()) {
                if (mMessageListView != null) {
                    ListAdapter adapter = mMessageListView.getAdapter();
                    mMessageListView.setSelection(adapter.getCount());
                    mMessageListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
                    mIsNewMessageNotify = Boolean.FALSE;
                    mIsBottom = true;
                    if (mMessageReminderText != null) {
                        mMessageReminderText.setText(SPACE);
                        mTextReminderSortedSet.remove(SHOW_NEW_MESSAGE_REMINDER);
                        showReminderList();
                    }
                }
            }
        }
    };
    
    protected View.OnClickListener mMessageEditorClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View arg0) {
            showImm(true);
        }
    };
    
    private View.OnClickListener mNetworkErrorClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View arg0) {
            Intent intent = new Intent(android.provider.Settings.ACTION_WIFI_SETTINGS);
            startActivity(intent);
            refreshNetworkErrorText();
        }
    };
    
    private View.OnClickListener mBtnAddViewClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View arg0) {
            onAddAttachment();
        }
    };
    
    protected View.OnClickListener mBtnEmotionClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View arg0) {
            int visibility = getEmotionsVisibility();
            Logger.d(TAG, "getEmotionsVisibility() entry-emotion visibility is "
                    + visibility);
            if (visibility == View.GONE) {
                showImm(false);
            } else if (visibility == View.VISIBLE) {
                showImm(true);
            }
        }
    };
    
    public View.OnClickListener mBtnSendClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View arg0) {
            Editable text = mMessageEditor.getText();
            final String message = text.toString();
            Logger.d(TAG, "onClick() mBtnSend  message is " + message + " mMessageListView: "
                    + mMessageListView + " mMessageReminderText: " + mMessageReminderText);
            if (text.length() > 0) {
                onSend(message);
                mMessageEditor.setText("");
                if (mMessageListView != null) {
                    ListAdapter adapter = mMessageListView.getAdapter();
                    mMessageListView.setSelection(adapter.getCount());
                    mMessageListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
                    mIsNewMessageNotify = Boolean.FALSE;
                    mIsBottom = true;
                    if (mMessageReminderText != null) {
                        mMessageReminderText.setText(SPACE);
                        mTextReminderSortedSet.remove(SHOW_NEW_MESSAGE_REMINDER);
                        showReminderList();
                    }
                }
            }
        }
    };

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        initEmotionArea(newConfig);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        NetworkChangedReceiver.addListener(this);
        Logger.d(TAG, "onCreate() mContentView: " + mContentView
                + " Logger.getIsIntegrationMode(): " + Logger.getIsIntegrationMode());
        if (null == mContentView) {
            Logger.d(TAG, "onCreate() mContentView is null, then create the view");
            LayoutInflater inflater = LayoutInflater.from(getActivity());
            if (null == inflater) {
                Logger.e(TAG, "onCreate() inflater is null");
                return;
            }
            View view = inflater.inflate(getFragmentResource(), null);
            mContentView = view;
            mMgToOtherWinReminderText = (TextView) mContentView.findViewById(R.id.unread_message);
            mMessageEditor = (EditText) mContentView.findViewById(R.id.et_chat_message);
            if (Logger.getIsIntegrationMode()) {
                mMessageEditor.setHint(R.string.text_ipmsg_hint_compose);
            }
            mMessageEditor.setOnClickListener(mMessageEditorClickListener);
            mMessageEditor.requestFocus();
            mMessageEditor.addTextChangedListener(mTextWatcher);
            int maxLength = RcsSettings.getInstance().getMaxGroupChatMessageLength();
            InputFilter[] filterArray = new InputFilter[1];
            filterArray[0] = new InputFilter.LengthFilter(maxLength);
            mMessageEditor.setFilters(filterArray);
            mMessageListView = (ListView) mContentView.findViewById(R.id.chat_message_list);
            mMessageListView.setDivider(null);
            mTypingText = (TextView) mContentView.findViewById(R.id.text_typing_prompt);
            mRemoteOfflineText = (TextView) mContentView
                    .findViewById(R.id.text_remote_offline_reminder);
            mMessageReminderText = (TextView) mContentView.findViewById(R.id.text_message_reminder);
            mMessageReminderText.setOnClickListener(mMessageReminderClickListener);
            mForwardToSettingsView = (ImageView) mContentView
                    .findViewById(R.id.image_forward_settings);
            mNetworkErrorText = (TextView) mContentView.findViewById(R.id.text_network_error);
            mNetworkErrorText.setOnClickListener(mNetworkErrorClickListener);
            mBlockedParticipantText = (TextView)mContentView.findViewById(R.id.text_blocked_participant);
            mMessageAdapter = new ChatAdapter(mMessageListView);
            mMessageListView.setAdapter(mMessageAdapter);
            mMessageListView.setOnScrollListener(mOnScrollListener);
            onAdapterPrepared();
            
                mBtnAddView = mContentView.findViewById(R.id.btn_chat_add);
                mBtnAddView.setOnClickListener(mBtnAddViewClickListener);
            
            mShareGridView = (GridView)mContentView.findViewById(R.id.gv_share_gridview);
            ShareAdapter adapter = new ShareAdapter(shareActions, shareIconArray);
            mShareGridView.setAdapter(adapter);
            mShareGridView.setOnItemClickListener(new OnItemClickListener() {

				@Override
				public void onItemClick(AdapterView<?> parent, View view,
						int position, long id) {
					switch(position)
					{
						case 0:
							//chose a photo
							startGallery();
							break;
						case 1:
							startCamera();
							//take photo
							break;
						case 2:
							startFileManager();
							//choose file
							break;
						default:
							break;
					}
					
				}
			});

            if(this instanceof GroupChatFragment)
            {
            	mGroupParticipantsLeftText = (TextView) mContentView.findViewById(R.id.all_participants_left);
            }
            mInputMethodService = (InputMethodManager) getActivity().getSystemService(
                    Context.INPUT_METHOD_SERVICE);

            try {
                mCurPageView = (ScrollLayout) mContentView.findViewById(R.id.chat_emotions);
                mPageIndicator = (LinearLayout) mContentView.findViewById(R.id.page_indicator);
                DisplayMetrics displayMetrics = getActivity().getApplicationContext()
                        .getResources().getDisplayMetrics();
                boolean isPortrait = displayMetrics.widthPixels < displayMetrics.heightPixels;
                Configuration config = new Configuration();
                if (isPortrait) {
                    config.orientation = Configuration.ORIENTATION_PORTRAIT;
                } else {
                    config.orientation = Configuration.ORIENTATION_LANDSCAPE;
                }
                mCurrentPage = 0;
                initEmotionArea(config);
                mEmotionLayout = (LinearLayout) mContentView.findViewById(R.id.emotions_area);
                mBtnEmotion = (ImageButton) mContentView.findViewById(R.id.btn_chat_emoticon);
                mBtnEmotion.setOnClickListener(mBtnEmotionClickListener);
            } catch (NotFoundException e) {
                Logger.e(TAG, "Emotion relevant resource not found");
            }
            mBtnSend = (ImageButton) mContentView.findViewById(R.id.btn_chat_send);
            if (Logger.getIsIntegrationMode()) {
                Drawable drawable = getActivity().getApplicationContext().getResources()
                        .getDrawable(R.drawable.ic_send_isms);
                mBtnSend.setImageDrawable(drawable);
            }
            updateSendButtonState(mMessageEditor.getText().toString());
            mBtnSend.setOnClickListener(mBtnSendClickListener);
        }
        initWallPaperLoader();
    }

 
    private void showImm(Boolean isShow) {
        Logger.d(TAG, "showImm() entry-isShow is " + isShow);
        mMessageListView.setFastScrollEnabled(false);
        if (isShow) {
            updateEmotionViewState(false);
            mBtnEmotion.setImageResource(R.drawable.emoticon);
            mInputMethodService.showSoftInput(mMessageEditor, 0);
        } else {
            mInputMethodService.hideSoftInputFromWindow(mMessageEditor.getWindowToken(), 0);
            updateEmotionViewState(true);
            mBtnEmotion.setImageResource(R.drawable.keyboard);
        }
        mMessageListView.setFastScrollEnabled(true);
        mMessageAdapter.updateAdapter();
    }

    /**
     * Get the emotion table visibility
     * 
     * @return View.GONE, View.VISIBLE or View.INVISIBLE
     */
    public int getEmotionsVisibility() {
        Logger.d(TAG, "getEmotionsVisibility() entry");
        return mEmotionLayout.getVisibility();
    }
    
    /**
     * Hide the emotion table
     */
    public void hideEmotions() {
        Logger.d(TAG, "getEmotionsVisibility() entry-emotion visibility is "
                + getEmotionsVisibility());
        if (getEmotionsVisibility() == View.VISIBLE) {
            mEmotionLayout.setVisibility(View.GONE);
        }
    }

    private void insertEmoticon(String text) {
        Logger.d(TAG, "insertEmoticon entry, text: " + text);
        int index = mMessageEditor.getSelectionStart();
        Editable edit = mMessageEditor.getEditableText();
        if (text.length() > ChatScreenActivity.MAX_CHAT_MSG_LENGTH - edit.length()) {
            Logger.d(TAG, "insertEmoticon reach max length");
            return;
        }
        if (index < 0 || index >= edit.length()) {
            edit.append(text);
        } else {
            edit.insert(index, text);
        }
        Logger.d(TAG, "insertEmoticon exit, text: " + text);
    }

    @Override
    public void onEmotionItemSelectedListener(PageAdapter adapter, int position) {
        String text = EmoticonsModelImpl.getInstance().getEmotionCode(position);
        insertEmoticon(text);
    }

    @SuppressWarnings("deprecation")
    private void initEmotionArea(Configuration newConfig) {
        // Page counts;
        mResIds = EmoticonsModelImpl.getInstance().getResourceIdArray();
        int resCount = mResIds.size();
        boolean hasMod = false;
        mPageCount = resCount / ITEMS_PER_PAGE;
        if (resCount % ITEMS_PER_PAGE != 0) {
            hasMod = true;
            mPageCount += 1;
        }

        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            mCurItemsPerLine = ORIENTATION_LANDSCAPE_ITEMS_PER_LINE;
        } else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
            mCurItemsPerLine = ORIENTATION_PORTRAIT_ITEMS_PER_LINE;
        }
        int linesPerPage = ITEMS_PER_PAGE / mCurItemsPerLine;
        mCurPageView.getLayoutParams().height = EMOTION_ICON_HEIGHT * linesPerPage;
        
        mPageIndicators.clear();
        // Initial page resource array map
        for (int i = 0; i < mPageCount; i++) {
            int pageNo = i;
            int pageStart = pageNo * ITEMS_PER_PAGE;
            int pageStop = 0;
            if (pageNo == mPageCount - 1) {
                if (hasMod) {
                    pageStop = resCount;
                } else {
                    pageStop = (pageNo + 1) * ITEMS_PER_PAGE;
                }
            } else {
                pageStop = (pageNo + 1) * ITEMS_PER_PAGE;
            }
            mResIdsMap.put(Integer.valueOf(pageNo), mResIds.subList(pageStart, pageStop));
            ImageView imageView = new ImageView(this.getActivity());
            mPageIndicators.add(imageView);
        }

        // Initial the page view
        if (mGridView != null) {
            mCurPageView.removeAllViews();
        }

        for (int i = 0; i < mListenerList.size(); i++) {
            PageAdapter adapter = mListenerList.get(i);
            if (adapter != null) {
                Logger.d(TAG, "initEmotionArea()-adapter is not null");
                adapter.unregisterListener();
            } else {
                Logger.d(TAG, "initEmotionArea()-adapter is null");
            }
        }

        for (int i = 0; i < mPageCount; i++) {
            mGridView = new GridView(this.getActivity());
            mGridView.setVerticalScrollBarEnabled(false);
            mGridView.setGravity(Gravity.CENTER);
            PageAdapter adapter = new PageAdapter(this.getActivity(), mResIdsMap.get(Integer.valueOf(i)), i);
            adapter.registerListener(this);
            mGridView.setAdapter(adapter);
            mGridView.setNumColumns(mCurItemsPerLine);
            mGridView.setColumnWidth(EMOTION_ICON_WIDTH);
            mGridView.setStretchMode(GridView.STRETCH_COLUMN_WIDTH);
            mCurPageView.addView(mGridView);
            mListenerList.add(adapter);
        }

        setPageIndicator(mCurrentPage);

        // Set page listener
        mCurPageView.setPageListener(new ScrollLayout.PageListener() {
            public void page(int page) {
                setPageIndicator(page);
            }
        });
    }

    private void setPageIndicator(int page) {
        Logger.d(TAG, "setPageIndicator()-page = " + page);
        mCurrentPage = page;
        mPageIndicator.removeAllViews();
        int size = mPageIndicators.size();
        for (int i = 0; i < size; i++) {
            ImageView imageView = mPageIndicators.get(i);
            Logger.d(TAG, "setPageIndicator()-i = " + i);
            if (i == page) {
                imageView.setImageResource(R.drawable.bg_img_item_true);
            } else {
                imageView.setImageResource(R.drawable.bg_img_item);
            }
            mPageIndicator.addView(imageView);
            if (i < size - 1) {
                Space space = new Space(this.getActivity());
                space.setMinimumWidth(PAGE_INDICATOR_PADDING);
                mPageIndicator.addView(space);
            }
        }
    }

    /**
     * Get the XML layout resource.
     * 
     * @return ID for the XML layout resource to load.
     */
    protected abstract int getFragmentResource();

    /**
     * Kick off a dialog to choose file transfer from : gallery, camera or file manager.
     */
    /*protected void onAddAttachment() {
        Logger.w(TAG, "onAddAttachment(), this method should be override by One2OneChatFragment");
    }*/

    protected void onAddAttachment() {
        /*if (mAttachmentTypeSelectorAdapter == null) {
            mAttachmentTypeSelectorAdapter = new AttachmentTypeSelectorAdapter(getActivity());
        }
        // Fixed the issue when double tap on File Transfer Icon caused JE
        FragmentManager fm = getFragmentManager();
        Fragment fragment = fm.findFragmentByTag("DialogOfAddAttachment");        	
        if (fragment instanceof DialogOfAddAttachment && fragment!=null) {        	
        fm.beginTransaction().remove(fragment).commitAllowingStateLoss();
        }      		

        mDialogOfAddAttachment.show(getFragmentManager(), DialogOfAddAttachment.TAG);*/
    	
    	//need to show gridview
    	int visibility = mShareGridView.getVisibility();
    	if(visibility == View.GONE)
    	{
    	    mShareGridView.setVisibility(View.VISIBLE);
    	    mEmotionLayout.setVisibility(View.GONE);
    	    mInputMethodService.hideSoftInputFromWindow(mMessageEditor.getWindowToken(), 0);
    	}
    	else
    	{
    		mShareGridView.setVisibility(View.GONE);
    	}
    	
    	
    }

    /*protected void addAttachment(int type) {
        Logger.w(TAG, "addAttachment(int type), this method by One2OneChatFragment");
    }*/
    
    public class WarningDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "WarningDialog";
        private CheckBox mCheckRemind = null;
        Activity mActivity = null;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .create();
            alertDialog.setTitle(R.string.file_size_warning);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog, null);
                mCheckRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mCheckRemind != null) {
                    boolean isCheck = mCheckRemind.isChecked();
                    Logger.w(TAG, "WarningDialog onClick ischeck" + isCheck);
                    SharedPreferences sPrefer =
                            PreferenceManager.getDefaultSharedPreferences(mActivity);
                    Editor remind = sPrefer.edit();
                    remind.putBoolean(SettingsFragment.RCS_REMIND, !isCheck);
                    remind.commit();
                }
                handleFileTransferInvite(mFileName);
            }
            this.dismissAllowingStateLoss();
        }
    }
    public class RepickDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            mRequestCode = getArguments().getInt(Utils.MESSAGE);
            final AlertDialog alertDialog;
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            alertDialog.setTitle(R.string.large_file_repick_title);
            alertDialog.setMessage(getActivity().getText(R.string.large_file_repick_message));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mRequestCode == RESULT_CODE_CAMERA) {
                    startCamera();
                } else if (mRequestCode == RESULT_CODE_FILE_MANAGER) {
                    startFileManager();
                } else {
                    startGallery();
                }
            }
            this.dismissAllowingStateLoss();
        }
    }
    
    private void startCamera() {
        boolean success = FileFactory.createDirectory(RCSE_FILE_DIR);
        success = FileFactory.createDirectory(RCSE_TEMP_FILE_DIR);
        Logger.d(TAG, "startCamera() success: " + success + " RCSE_TEMP_FILE_DIR: "
                + RCSE_TEMP_FILE_DIR);
        mCameraTempFileUri = Uri.fromFile(new File(ChatFragment.RCSE_TEMP_FILE_DIR,
                ChatFragment.RCSE_TEMP_FILE_NAME_HEADER
                        + String.valueOf(System.currentTimeMillis()) + ChatFragment.JPEG_SUFFIX));
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraTempFileUri);
        startActivityForResult(intent, RESULT_CODE_CAMERA);
    }

    private void startGallery() {
        Logger.v(TAG, "startGallery()");
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType(GALLERY_TYPE);
        startActivityForResult(intent, RESULT_CODE_GALLERY);
    }

    private void startFileManager() {
        Logger.v(TAG, "startFileManager()");
        Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        if (FeatureOption.MTK_DRM_APP) {
            intent.putExtra(OmaDrmStore.DrmExtra.EXTRA_DRM_LEVEL, OmaDrmStore.DrmExtra.DRM_LEVEL_SD);
        }
        startActivityForResult(intent, RESULT_CODE_FILE_MANAGER);
    }
    
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Logger.v(TAG, "onActivityResult() requestCode = " + requestCode + ",resultCode = "
                + resultCode + ",data = " + data);
     
            if (requestCode == RESULT_CODE_CAMERA || requestCode == RESULT_CODE_GALLERY
                    || requestCode == RESULT_CODE_FILE_MANAGER) {
                Bundle arguments = new Bundle();
                arguments.putInt(Utils.MESSAGE, requestCode);
                mRepickDialog.setArguments(arguments);
            }
            new FileProcessingAsyncTask(requestCode, resultCode, data).execute();
        
    }
    public class CompressDialog extends DialogFragment implements DialogInterface.OnClickListener {
        private static final String TAG = "CompressDialog";
        private CheckBox mCheckNotRemind = null;
        private Activity mActivity = null;
        private String mOriginFileName;

        /**
         * Constructor
         */
        public CompressDialog() {

        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Logger.d(TAG, "CompressDialog onCreateDialog entry");
            mOriginFileName = getArguments().getString(Utils.MESSAGE);
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .create();
            alertDialog.setTitle(R.string.compress_image_title);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog, null);
                mCheckNotRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView.findViewById(R.id.warning_content);
                contentView.setText(R.string.compress_image_content);
            } else {
                Logger.e(TAG, "activity is null in WarningDialog");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            RcsSettings.getInstance().saveRemindCompressFlag(mCheckNotRemind.isChecked());
            if (which == DialogInterface.BUTTON_POSITIVE) {
                handleOk();
            } else {
                Logger.d(TAG, "the user cancle compressing image");
                handleCancel();
            }
            this.dismissAllowingStateLoss();
        }

        private void handleOk() {
            Logger.i(TAG, "handleOk()");
            new AsyncTask<Void, Void, String>() {
                @Override
                protected String doInBackground(Void... params) {
                    if (mCheckNotRemind.isChecked()) {
                        Logger.d(TAG, "the user enable compressing image and not remind again");
                        RcsSettings.getInstance().setCompressingImage(true);
                    }
                    return doCompress(mOriginFileName);
                }

                @Override
                protected void onPostExecute(String result) {
                    Logger.v(TAG, "onPostExecute(),result = " + result);
                    if (result != null) {
                        mFileName = result;
                        handleFileSizeWhenInvite(result);
                    }
                }
            }.execute();
        }

        private void handleCancel() {
            Logger.i(TAG, "handleCancel()");
            mFileName = mOriginFileName;
            handleFileSizeWhenInvite(mOriginFileName);
        }
    }
    protected String doCompress(final String originFileName) {
        Logger.d(TAG, "doCompress(): originFileName = " + originFileName);
        return Utils.compressImage(originFileName);
    }

    protected boolean isImageFile(String fileName) {
        Logger.d(TAG, "isImageFile() entry with fileName = " + fileName);
        if (fileName == null) {
            return false;
        }
        String type = AsyncGalleryView.getMimeType(fileName);
        if (type != null && type.startsWith(IMAGE_TYPE)) {
            return true;
        }
        return false;
    }
    
    protected void handleFileSizeWhenInvite(String fileName) {
        File file = new File(fileName);
        long fileSize = file.length();
        long maxFileSize = ApiManager.getInstance().getMaxSizeforFileThransfer();
        long warningFileSize = ApiManager.getInstance().getWarningSizeforFileThransfer();
        boolean shouldWarning = false;
        boolean shouldRepick = false;
        if (warningFileSize != 0 && fileSize >= warningFileSize) {
            shouldWarning = true;
        }
        if (maxFileSize != 0 && fileSize >= maxFileSize) {
            shouldRepick = true;
        }
        Logger.d(TAG, "handleFileSizeWhenInvite() maxFileSize: " + maxFileSize
                + " warningFileSize: " + warningFileSize + " shouldWarning: " + shouldWarning
                + " shouldRepick: " + shouldRepick);
        if (shouldRepick) {
            mFileName = null;
            mRepickDialog.show(getFragmentManager(), RepickDialog.TAG);
        } else if (shouldWarning) {
            Activity activity = getActivity();
            if (activity != null) {
                SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(activity);
                Boolean remind = prefer.getBoolean(SettingsFragment.RCS_REMIND, true);
                Logger.w(TAG, "handleFileSizeWhenInvite() remind: " + remind);
                if (remind) {
                    mWarningDialog.show(getFragmentManager(), WarningDialog.TAG);
                } else {
                    handleFileTransferInvite(fileName);
                }
            }
        } else {
            handleFileTransferInvite(fileName);
        }
    }
    
    protected interface IChatMessage extends IChatWindowMessage {
        Date getMessageDate();
    }
    public abstract class FileTransfer implements IFileTransfer, IChatMessage {
        protected FileStruct mFileStruct = null;

        protected Status mStatus = Status.WAITING;

        protected boolean mAutoAccept = false;

        protected Object mTag;

        protected long mProgress = 0;

        FileTransfer(FileStruct fileStruct) {
            mFileStruct = fileStruct;
        }

        

        @Override
        public void setProgress(long progress) {
            mProgress = progress;
            mUiHandler.post(new Runnable() {

                @Override
                public void run() {
                    if (mMessageAdapter == null) {
                        Logger.d(TAG, "setProgress mMessageAdapter is null");
                        return;
                    }
                    mMessageAdapter.notifyDataSetChanged();
                }
            });
        }

        public long getProgress() {
            return mProgress;
        }

        @Override
        public void setStatus(Status status) {
            if (status == Status.TRANSFERING || status != mStatus) {
                mStatus = status;
                Runnable runnable = new Runnable() {

                    @Override
                    public void run() {
                        if (mMessageAdapter == null) {
                            Logger.d(TAG, "setStatus mMessageAdapter is null");
                            return;
                        }
                        mMessageAdapter.notifyDataSetChanged();
                    }
                };

                Thread currentThread = Thread.currentThread();
                if (currentThread.getId() == THREAD_ID_MAIN) {
                    runnable.run();
                } else {
                    mUiHandler.post(runnable);
                }
            } else {
                Logger.d(TAG, "setStatus(), should not update data. status now = " + status
                        + " status before =" + mStatus);
            }
        }

        public Status getStatue() {
            return mStatus;
        }

        public FileStruct getFileStruct() {
            return mFileStruct;
        }

        /**
         * Get file transfer chat screen tag.
         * 
         * @return tag of file transfer screen.
         */
        public Object getTag() {
            return mTag;
        }

        /**
         * Set file transfer chat screen tag.
         * 
         * @return void
         */
        public void setTag(Object tag) {
            mTag = tag;
        }

        /**
         * Set the file path the file transfer, when received a file
         * 
         * @param file path
         */
        public void setFilePath(String filePath) {
            mFileStruct.mFilePath = filePath;
        }

        /**
         * Update file transfer tag and size
         * 
         * @param received file path
         * @param received file size
         */
        public void updateTag(String transferTag, long transferSize) {
            mFileStruct.mFileTransferTag = transferTag;
            mFileStruct.mSize = transferSize;
        }

        /**
         * Get the date when the file transfer was received.
         * 
         * @return The date when the file transfer was received.
         */
        public Date getMessageDate() {
            if (mFileStruct != null) {
                return mFileStruct.mDate;
            } else {
                Logger.e(TAG, "getMessageDate, mFileStruct is null!");
                return null;
            }
        }

        /**
         * Get the message id.
         */
        public String getId() {
            return mFileStruct.mFileTransferTag.toString();
        }
    }
    
    protected Map<Object, FileTransfer> mPreFileTransferMap =
        new ConcurrentHashMap<Object, FileTransfer>();
    
    public IFileTransfer addSentFileTransfer(FileStruct file) {
    	Logger.d(TAG, "addSentFileTransfer()  chatfragment file: " + file);
    	return null;
    }
    
    private Bundle onSentFile(FileStruct fileStruct) {
        Logger.d(TAG, "onSentFile() fileStruct: " + fileStruct);
        if (null != fileStruct) {
            Object fileTransferTag = fileStruct.mFileTransferTag;
            mPreFileTransferMap.put(fileTransferTag,
                    (FileTransfer) addSentFileTransfer(fileStruct));
            if (fileTransferTag instanceof Parcelable) {
                Bundle data = new Bundle();
                data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG,
                        (Parcelable) fileTransferTag);
                return data;
            } else {
                Logger.w(TAG, "onSentFile() invalid file transfer tag: " + fileTransferTag);
                return null;
            }
        } else {
            Logger.w(TAG, "onSentFile() fileStruct is null");
            return null;
        }
    }
    private void handleFileTransferInvite(String filePath) {
        Logger.i(TAG, "handleFileTransferInvite()");
        if (mFiletransferEnableStatus == One2OneChat.FILETRANSFER_ENABLE_OK) {
            ControllerImpl controller = ControllerImpl.getInstance();
            FileStruct fileStruct = FileStruct.from(filePath);
            Bundle data = onSentFile(fileStruct);
            
            Message controllerMessage = controller.obtainMessage(
                    ChatController.EVENT_FILE_TRANSFER_INVITATION, mTag, filePath);
            controllerMessage.setData(data);
            controllerMessage.sendToTarget();
        } else {
            Activity activity = getActivity();
            if (activity != null) {
                switch (mFiletransferEnableStatus) {
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                        Toast.makeText(activity,
                                getString(R.string.filetransfer_disable_reason_capability_failed),
                                Toast.LENGTH_LONG).show();
                        break;
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                        Toast.makeText(activity, getString(R.string.file_transfer_off_line),
                                Toast.LENGTH_LONG).show();
                        break;
                    case One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE:
                        Toast.makeText(activity,
                                getString(R.string.file_transfer_receiver_offline),
                                Toast.LENGTH_LONG).show();
                        break;
                    default:
                        Logger.d(TAG, "handleFileTransferInvite the mFiletransferEnableStatus is "
                                + mFiletransferEnableStatus);
                }
            } else {
                Logger.e(TAG, "handleFileTransferInvite the activity is null");
            }
        }
    }
    class FileProcessingAsyncTask extends AsyncTask<Void, Void, String> {

        private int mRequestCode = -1;
        private int mResultCode = -1;
        private Intent mData = null;

        public FileProcessingAsyncTask(int requestCode, int resultCode, Intent data) {
            mRequestCode = requestCode;
            mResultCode = resultCode;
            mData = data;
        }

         @Override
        protected String doInBackground(Void... params) {
            Uri uri = null;
            if (Activity.RESULT_CANCELED == mResultCode) {
                Logger.w(TAG, "doInBackground(), the user cancel to select a picture");
                return null;
            }
            if (mRequestCode == RESULT_CODE_CAMERA) {
                uri = mCameraTempFileUri;
            } else if (mRequestCode == RESULT_CODE_GALLERY || mRequestCode == RESULT_CODE_FILE_MANAGER) {
                if (mData != null) {
                    String uriString = mData.getDataString();
                    if(uriString == null){
                    return null;
                }
                    Logger.d(TAG, "doInBackground(),uriString is " + uriString);
                    uri = Uri.parse(uriString);
                } else {
                    Logger.w(TAG,"doInBackground()-RESULT_CODE_GALLERY:mData is null");
                    return null;
                }
            } else {
                Logger.w(TAG, "doInBackground() unkown result");
                return null;
            }
            String fileFullName = getFileFullPathFromUri(uri);
            boolean isImage = isImageFile(fileFullName);
            Logger.d(TAG, "isImage = " + isImage);
            String compressedFileName = null;
            if (isImage && RcsSettings.getInstance().isEnabledCompressingImageFromDB()) {
                Logger.d(TAG, "Compress the image, do not hit the user");
                mCompressDialog = null;
                compressedFileName = doCompress(fileFullName);
            } else if (isImage && !RcsSettings.getInstance().isEnabledCompressingImageFromDB()) {
                boolean remind = RcsSettings.getInstance().restoreRemindCompressFlag();
                Logger.d(TAG, "Do hit the user to select whether to compress. remind = " + remind);
                if (remind) {
                    mCompressDialog = new CompressDialog();
                    Bundle arguments = new Bundle();
                    arguments.putString(Utils.MESSAGE, fileFullName);
                    mCompressDialog.setArguments(arguments);
                    mCompressDialog.show(getFragmentManager(), CompressDialog.TAG);
                } else {
                    Logger.d(TAG, "Do not compress image");
                    compressedFileName = fileFullName;
                }
            } else {
                Logger.d(TAG, "Do not compress non image file");
                compressedFileName = fileFullName;
            }
            Logger.d(TAG, "The compressed image file name = " + compressedFileName);
            return compressedFileName;
        }

        protected void onPostExecute(String fileName) {
            Logger.v(TAG, "onPostExecute(),fileName = " + fileName + ",mResultCode = "
                    + mResultCode);
            if (fileName != null) {
                mFileName = fileName;
                handleFileSizeWhenInvite(fileName);
            }
        }

        private String getFileFullPathFromUri(Uri uri) {
            if (uri == null) {
                Logger.e(TAG, "getFileFullPathFromUri()-uri is null");
                return null;
            }
            String uriString = Uri.decode(uri.toString());
            if (uriString == null) {
                Logger.e(TAG, "getFileFullPathFromUri()-uriString is null");
                return null;
            }
            Logger.d(TAG, "getFileFullPathFromUri()-The uri is:[" + uriString + "]");
            if (uriString.startsWith(FILE_SCHEMA) || uriString.startsWith(FILE_SCHEMA_SMALL)) {
                uriString = uriString.substring(FILE_SCHEMA.length(), uriString.length());
                return uriString;
            } else if (uriString.startsWith(CONTENT_SCHEMA)) {
                int findPos = uriString.indexOf(CONTENT_SCHEMA) + CONTENT_SCHEMA.length();
                String fileFullName = null;
                Cursor cursor = null;
                if (uriString.indexOf(CONTENT_IMAGE_TYPE) > findPos) {
                    cursor = ChatFragment.this.getActivity().getContentResolver()
                            .query(uri, new String[] {
                                MediaStore.Images.ImageColumns.DATA
                            }, null, null, null);
                    if (cursor != null) {
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
                        cursor.close();
                    }
                } else if (uriString.indexOf(CONTENT_AUDIO_TYPE) > findPos) {
                    cursor = ChatFragment.this.getActivity().getContentResolver()
                            .query(uri, new String[] {
                                MediaStore.Audio.AudioColumns.DATA
                            }, null, null, null);
                    if (cursor != null) {
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Audio.Media.DATA));
                        cursor.close();
                    }
                } else if(uriString.indexOf(CONTENT_VIDEO_TYPE) > findPos){
                    cursor = ChatFragment.this.getActivity().getContentResolver().query(uri,
                            new String[] {
                                    MediaStore.Video.VideoColumns.DATA,
                            }, null, null, null);
                    if(cursor != null){
                        cursor.moveToFirst();
                        fileFullName = cursor.getString(cursor
                                .getColumnIndex(MediaStore.Video.VideoColumns.DATA));
                        cursor.close();
                    }
            } else {
                    Logger.d(TAG, "getFileFullPathFromUri()-invalid media type");
                }
                return fileFullName;
            } else if (uriString.startsWith(VCARD_SCHEMA) || uriString.startsWith(ALL_VCARD_SCHEMA)) {
                String fileFullName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis()
                        + VCARD_SUFFIX;
                try {
                    AssetFileDescriptor fd = ChatFragment.this.getActivity()
                            .getContentResolver().openAssetFileDescriptor(uri, READABLE_RIGHT);
                    FileInputStream fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);
                    fis.close();
                    File dir = new File(RCSE_TEMP_FILE_DIR);
                    if (!dir.exists()) {
                        if (!dir.mkdir()) {
                            Logger.e(TAG, "getFileFullPathFromUri()-create dir failed");
                            return null;
                        }
                    }
                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(data);
                    fos.close();
                } catch (FileNotFoundException fileNotFoundException) {
                    Logger.e(TAG, "getFileFullPathFromUri()-fileNotFoundException");
                    fileFullName = null;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    Logger.e(TAG,"getFileFullPathFromUri()-iOException while accessing the stream");
                    fileFullName = null;
                    iOException.printStackTrace();
                } finally {
                    return fileFullName;
                }
            } else if (uriString.startsWith(VCALENDAR_SCHEMA)) {
                String fileFullName = RCSE_TEMP_FILE_DIR + System.currentTimeMillis()
                        + VCALENDAR_SUFFIX;
                try {
                    AssetFileDescriptor fd = ChatFragment.this.getActivity()
                            .getContentResolver().openAssetFileDescriptor(uri, READABLE_RIGHT);
                    FileInputStream fis = fd.createInputStream();
                    byte[] data = new byte[fis.available()];
                    fis.read(data);
                    fis.close();
                    File dir = new File(RCSE_TEMP_FILE_DIR);
                    if (!dir.exists()) {
                        if (!dir.mkdir()) {
                            Logger.e(TAG, "getFileFullPathFromUri()-create dir failed");
                            return null;
                        }
                    }
                    File file = new File(fileFullName);
                    file.setWritable(true);
                    file.setReadable(true);
                    FileOutputStream fos = new FileOutputStream(file);
                    fos.write(data);
                    fos.close();
                } catch (FileNotFoundException fileNotFoundException) {
                    Logger.e(TAG, "getFileFullPathFromUri()-fileNotFoundException");
                    fileFullName = null;
                    fileNotFoundException.printStackTrace();
                } catch (IOException iOException) {
                    Logger.e(TAG,"getFileFullPathFromUri()-iOException while accessing the stream");
                    fileFullName = null;
                    iOException.printStackTrace();
                } finally {
                    return fileFullName;
                }
            } else {
                Logger.e(TAG, "getFileFullPathFromUri()-invalid uri = " + uriString);
                return null;
            }
        }
    }// end FileProcessingAsyncTask
    protected void addAttachment(int type) {
        Logger.v(TAG, "addAttachment(), type = " + type);
        switch (type) {
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_GALLERY:
                startGallery();
                break;
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_CAMERA:
                startCamera();
                break;
            case AttachmentTypeSelectorAdapter.ADD_FILE_FROM_FILE_MANAGER:
                startFileManager();
                break;
            default:
                Logger.w(TAG, "Can not handle attachment type of " + type);
                break;
        }
    }

    public class DialogOfAddAttachment extends DialogFragment implements
            DialogInterface.OnClickListener {
        public static final String TAG = "DialogOfAddAttachment";

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            return new AlertDialog.Builder(getActivity()).setIcon(R.drawable.ic_dialog_attach)
                    .setTitle(R.string.file_transfer_title).setAdapter(
                            mAttachmentTypeSelectorAdapter, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    addAttachment(mAttachmentTypeSelectorAdapter
                                            .buttonToCommand(which));
                                    dismissAllowingStateLoss();
                                }
                            }).create();
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {

        }

        @Override
        public void dismissAllowingStateLoss() {
            super.dismissAllowingStateLoss();
            FragmentManager fm = getFragmentManager();
            Fragment fragment = fm.findFragmentByTag(TAG);
            Log.d(TAG, "dismissAllowingStateLoss(): fragment = " + fragment);
            if (fragment == this) {
                Log.d(TAG, "dismissAllowingStateLoss(): remove fragment from fragment manger");
                fm.beginTransaction().remove(this).commitAllowingStateLoss();
            }
        }

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if (mContentView != null) {
            return mContentView;
        } else {
            return super.onCreateView(inflater, container, savedInstanceState);
        }
    }

    protected abstract void onAdapterPrepared();

    /**
     * Convert the participant list into a String for title use
     * 
     * @param participantList The participant list need to be converted
     * @return The participant's display name, or the participants' display name.
     */
    public static String getParticipantsName(Participant[] participants) {
        Logger.d(TAG, "getParticipantsName() entry, this is participant version");
        if (participants == null || 0 == participants.length) {
            Logger.w(TAG, "getParticipantsName() participants is empty");
            return getDefaultGroupChatName();
        }

        ArrayList<String> numbers = new ArrayList<String>();
        for (Participant participant : participants) {
            if (null != participant) {
                String number = participant.getContact();
                if (!TextUtils.isEmpty(number)) {
                    numbers.add(number);
                }
            }
        }
        if (numbers.size() > 0) {
            return getParticipantsName(numbers.toArray(new String[0]));
        } else {
            Logger.w(TAG, "getParticipantsName() numbers is empty");
            return getDefaultGroupChatName();
        }
    }

    /**
     * Convert the participant info list into a String for title use
     * 
     * @param participantsInfo The participant info list need to be converted
     * @return The participant's display name, or the participants' display name.
     */
    public static String getParticipantsName(ParticipantInfo[] participantsInfo) {
        Logger.d(TAG, "getParticipantsName() entry, this is participantsInfo version");
        if (participantsInfo == null || 0 == participantsInfo.length) {
            Logger.w(TAG, "getParticipantsName() participantsInfo is empty");
            return getDefaultGroupChatName();
        }

        ArrayList<String> numbers = new ArrayList<String>();
        for (ParticipantInfo pInfo : participantsInfo) {
            if (null != pInfo) {
                String number = pInfo.getContact();
                Logger.d(TAG, "getParticipantsName() number: " + number);
                if (!TextUtils.isEmpty(number)) {
                    numbers.add(number);
                }
            }
        }
        if (numbers.size() > 0) {
            return getParticipantsName(numbers.toArray(new String[0]));
        } else {
            Logger.w(TAG, "getParticipantsName() numbers is empty");
            return getDefaultGroupChatName();
        }
    }

    /**
     * Convert the number list into a String for title use
     * 
     * @param numbers The number list need to be converted
     * @return The participant's display name, or the participants' display name.
     */
    public static String getParticipantsName(String[] numbers) {
        Logger.d(TAG, "getParticipantsName() entry, this is number version");
        if (numbers == null || 0 == numbers.length) {
            Logger.w(TAG, "getParticipantsName() numbers is empty");
            return getDefaultGroupChatName();
        }
        final StringBuilder stringBuilder = new StringBuilder();
        for (String number : numbers) {
            if (null != number) {
                String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                        number);
                Logger.d(TAG, "getParticipantsName() got displayname " + displayName
                        + " from the number " + number);
                if (null != displayName) {
                    stringBuilder.append(displayName);
                    stringBuilder.append(",");
                }
            }
        }
        int length = stringBuilder.length();

        if (length > 1) {
            stringBuilder.deleteCharAt(length - 1);
        } else if (0 == length) {
            Logger.w(TAG, "getParticipantsName() stringBuilder is empty");
            return getDefaultGroupChatName();
        }
        String participantsName = stringBuilder.toString();
        Logger.v(TAG, "getParticipantsName() group members are " + participantsName);
        return participantsName;
    }

    private static String getDefaultGroupChatName() {
        Logger.d(TAG, "getDefaultGroupChatName() entry  ");
        Resources resources = null;
        try {
            resources = AndroidFactory.getApplicationContext().getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        String defaultGroupChatName = null;
        if (null != resources) {
            defaultGroupChatName = resources.getString(R.string.default_group_chat_subject);
            Logger.d(TAG, "getDefaultGroupChatName() defaultGroupChatName is "
                    + defaultGroupChatName);
        }
        return defaultGroupChatName;
    }

    /**
     * @return The participants number in your group chat
     */
    public int getParticipantsNum() {
        if (mParticipantList == null) {
            Logger.w(TAG, "mParticipantList is null");
            return 0;
        } else {
            return mParticipantList.size();
        }
    }

    /**
     * Clear the history.
     * 
     * @return True if success, else false.
     */
    public boolean clearHistory() {
        Logger.d(TAG, "clearHistory entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_CLEAR_CHAT_HISTORY, mTag, null);
        controllerMessage.sendToTarget();
        return true;
    }

    /**
     * @return The tag of the ChatFragment.
     */
    public Object getChatFragmentTag() {
        return mTag;
    }

    @Override
    public void onDestroy() {
        Logger.v(TAG, "onDestroy() entry");
        mAttachmentTypeSelectorAdapter = null;
        NetworkChangedReceiver.removeListener(this);
        for (int i = 0; i < mListenerList.size(); i++) {
            PageAdapter adapter = mListenerList.get(i);
            if (adapter != null) {
                Logger.d(TAG, "initEmotionArea()-adapter is not null");
                adapter.unregisterListener();
            }
        }
        destoryWallPaperLoader();
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        ContactsListManager.getInstance().removeListener(this);
        super.onDetach();
        Logger.v(TAG, "ChatFragment onDetach()");
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
        resume();
    }

    /**
     * Resume this fragment.
     */
    public void resume() {
        if (mMessageListView != null) {
            if (Logger.getIsIntegrationMode() && this instanceof GroupChatFragment) {
                Logger.d(TAG, "resume(), this is a group chat fragment and in itegate mode");
                loadWallPaperFromMms();
            }
            ListAdapter adapter = mMessageListView.getAdapter();
            mMessageListView.setSelection(adapter.getCount());
            mMessageListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
            mIsNewMessageNotify = Boolean.FALSE;
            mIsBottom = true;
            if (mMessageReminderText != null) {
                mMessageReminderText.setText(SPACE);
                mTextReminderSortedSet.remove(SHOW_NEW_MESSAGE_REMINDER);
                showReminderList();
            }
        }
        refreshNetworkErrorText();
    }

    @Override
    public void onStop() {
        // TODO
        super.onStop();
        Logger.v(TAG, "ChatFragment onStop()");
    }

    /**
     * Add contacts to current chat fragment.
     */
    public boolean addContacts() {
        Logger.d(TAG, "addContacts() entry");
        if (ContactsListManager.IS_SUPPORT) {
            Intent intent = new Intent(ChatMainActivity.ACTION_START_CONTACT);
            intent.setType(ChatMainActivity.INTENT_TYPE);
            long[] phoneIdList = ContactsListManager.getInstance().getPhoneIdTobeShow(
                    mParticipantList);
            intent.putExtra(ChatMainActivity.RESTRICT_LIST, phoneIdList);
            startActivityForResult(intent, RESULT_CODE_ADD_CONTACTS);
        } else {
            Intent intent = new Intent();
            Activity activity = getActivity();
            if (activity != null) {
                intent.putParcelableArrayListExtra(ChatScreenActivity.KEY_EXSITING_PARTICIPANTS,
                        new ArrayList<Participant>(mParticipantList));
                intent.setClass(activity, SelectContactsActivity.class);
                startActivityForResult(intent, RESULT_CODE_ADD_CONTACTS);
            } else {
                Logger.e(TAG, "addContacts() activity is null.");
                return false;
            }
        }
        Logger.d(TAG, "addContacts() exit");
        return true;
    }

    /**
     * Add unread message of the next window.
     * 
     * @param message The unread message.
     */
    public void addUnreadMessage(final InstantMessage message) {
        if (message == null) {
            Logger.d(TAG, "addUnreadMessage() message is null");
            return;
        }
        if (mContentView == null) {
            Logger.d(TAG, "addUnreadMessage() mContentView is null");
            return;
        }
        final String contact = PhoneUtils.extractNumberFromUri(message.getRemote());
        final String displayName = ContactsListManager.getInstance().getDisplayNameByPhoneNumber(
                contact);
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                StringBuilder builder = new StringBuilder();
                if (displayName != null) {
                    builder.append(displayName);
                    builder.append(COLON);
                } else {
                    builder = build(contact);
                }
                builder.append(message.getTextMessage());
                mMgToOtherWinReminderText.setText(builder.toString());
                Logger.d(TAG, "addUnreadMessage() add SHOW_MESSAGE_REMINDER");
                mTopReminderSortedSet.add(SHOW_OTHER_MESSAGE_REMINDER);
                showTopReminder();
            }
        });
        mMgToOtherWinReminderText.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ChatScreenActivity activity = ((ChatScreenActivity) getActivity());
                if (activity == null) {
                    Logger.d(TAG, "addUnreadMessage() onClick activity is null");
                    return;
                }
                UnreadMessagesContainer instance = UnreadMessagesContainer.getInstance();
                ParcelUuid tag = instance.getLatestUnreadMessageChat();
                instance.switchTo(tag, activity.getChatWindowManager());
                mMgToOtherWinReminderText.setText("");
                mTopReminderSortedSet.remove(SHOW_OTHER_MESSAGE_REMINDER);
                showTopReminder();
            }
        });
        Logger.d(TAG, "addUnreadMessage() exit");
    }

    private StringBuilder build(String contact) {
        StringBuilder builder = new StringBuilder();
        int start = contact.indexOf(SEPERATOR_COLON);
        if (start != -1) {
            int end = contact.indexOf(SEPERATOR_DOMAIN, start + 1);
            if (end > 0) {
                builder.append(contact.substring(start + 1, end));
                builder.append(COLON);
            }
        }
        Logger.d(
                TAG,
                "build() contact: " + contact + " start: " + start + " builder: "
                        + builder.toString());
        return builder;
    }

    /**
     * When switch ChatFragment this method should be called to remove ui.
     */
    public abstract void removeChatUi();

    /**
     * Set chat screen's title.
     */
    public abstract void setChatScreenTitle();

    public void onDisplayNameChanged() {
        if (null != mMessageAdapter) {
            Logger.d(TAG, "onDisplayNameChanged() updates");
            mMessageAdapter.notifyDataSetChanged();
        } else {
            Logger.e(TAG, "onDisplayNameChanged() mMessageAdapter is null");
        }
        setChatScreenTitle();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        // Override this method to workaround a google issue happen when API
        // level > 11
        outState.putString("work_around_tag", "work_around_content");
        super.onSaveInstanceState(outState);
    }

    private void refreshNetworkErrorText() {
        if (mNetworkErrorText != null) {
            if (!mIsNetworkConnected) {
                mTopReminderSortedSet.add(SHOW_NETWORK_ERROR_REMINDER);
                Logger.d(TAG, "refreshNetworkErrorText(),"
                        + "add SHOW_NETWORK_ERROR_REMINDER in mReminderSortedSet");
            } else {
                mTopReminderSortedSet.remove(SHOW_NETWORK_ERROR_REMINDER);
                Logger.d(TAG, "refreshNetworkErrorText(),"
                        + "remove SHOW_NETWORK_ERROR_REMINDER in mReminderSortedSet");
            }
            showTopReminder();
        } else {
            Logger.e(TAG, "refreshNetworkErrorText(), mNetworkErrorText is null!");
        }
    }

    @Override
    public void onNetworkStatusChanged(boolean isConnected) {
        Logger.d(TAG, "onNetworkStatusChanged() entry");
        mIsNetworkConnected = isConnected;
        this.getActivity().runOnUiThread(new Runnable() {
            public void run() {
                refreshNetworkErrorText();
            }
        });
        Logger.d(TAG, "onNetworkStatusChanged() exit");
    }

    private void initWallPaperLoader() {
        Logger.d(TAG, "initWallPaperLoader()");
        if (Logger.getIsIntegrationMode() && this instanceof GroupChatFragment) {
            Logger.d(TAG,
                    "initWallPaperLoader(), this is a group chat fragment and in itegate mode");
            loadWallPaperFromMms();
        } else {
            RcsSettings.getInstance().registerWallPaperChangedListener(mWallPaperChangedListener);
            int chatWallPaperId = RcsSettings.getInstance().getChatWallpaperId();
            if (chatWallPaperId == 0) {
                String chatWallPaperFileName = RcsSettings.getInstance().getChatWallpaper();
                Logger.d(TAG, "set background use file name, chatWallPaperFileName = "
                        + chatWallPaperFileName);
                if (chatWallPaperFileName == null) {
                    // Init not ready, then set default background
                    mMessageListView.setBackgroundResource(R.drawable.rcs_wallpaper_default);
                } else {
                    loadWallPaperByFileName(chatWallPaperFileName);
                }
            } else {
                Logger.d(TAG, "set background use resource id");
                mMessageListView.setBackgroundResource(chatWallPaperId);
            }
        }
    }

    private void destoryWallPaperLoader() {
        Logger.d(TAG, "destoryWallPaperLoader()");
        mMessageListView.setBackgroundDrawable(null);
        RcsSettings.getInstance().unregisterWallPaperChangedListener(mWallPaperChangedListener);
    }

    private void loadWallPaperByFileName(String wallPaperName) {
        Logger.d(TAG, "loadWallPaperByFileName(), wallPaperName = " + wallPaperName);
        ImageLoader.getInstance();
        Bitmap bitmap = ImageLoader.requestImage(wallPaperName, new LoadWallPaperFinished());
        if (bitmap != null) {
            setWallPaperByDrawable(new BitmapDrawable(bitmap));
        }
    }

    private void setWallPaperById(final int resId) {
        Logger.d(TAG, "setWallPaperById");
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "set background by id");
                mMessageListView.setBackgroundResource(resId);
            }
        });
    }

    private void setWallPaperByDrawable(final BitmapDrawable bitmapDrawable) {
        Logger.d(TAG, "setWallPaperByDrawable");
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "set background by drawable");
                mMessageListView.setBackgroundDrawable(bitmapDrawable);
            }
        });
    }

    /**
     * A listener to listen whether the bitmap has been loaded.
     */
    private class LoadWallPaperFinished implements OnLoadImageFinishListener {
        private static final String TAG = "LoadWallPaperFinished";

        @Override
        public void onLoadImageFished(Bitmap image) {
            Logger.d(TAG, "onLoadImageFished");
            // Set listview's background
            setWallPaperByDrawable(new BitmapDrawable(image));
        }
    }

    /**
     * A listener to listen whether the wall paper has been changed.
     */
    private class WallPaperChanged implements OnWallPaperChangedListener {
        private static final String TAG = "WallPaperChanged";

        @Override
        public void onWallPaperChanged(String wallPaper) {
            Logger.d(TAG, "onWallPaperChanged(), wallPaper = " + wallPaper);
            int chatWallPaperId = 0;
            try {
                chatWallPaperId = Integer.valueOf(wallPaper);
            } catch (NumberFormatException e) {
                Logger.w(TAG, "wallPaper is not a resource id");
            }
            if (chatWallPaperId == 0) {
                loadWallPaperByFileName(wallPaper);
            } else {
                setWallPaperById(chatWallPaperId);
            }
        }
    }

    protected void showToast(int messageId) {
        Logger.v(TAG, "showToast() entry, messageId = " + messageId);
        Activity activity = getActivity();
        Logger.v(TAG, "activity = " + activity);
        if (activity != null) {
            Toast toast = Toast.makeText(activity.getApplicationContext(), messageId,
                    Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
        }
        Logger.v(TAG, "showToast() exit");
    }

    /**
     * Load wall paper from mms and set it on listview. Notice this method
     * should only be called in integrate mode.
     * 
     * @return Always return true
     */
    @SuppressWarnings({
            "rawtypes", "unchecked"
    })
    private boolean loadWallPaperFromMms() {
        Logger.v(TAG, "loadWallPaperFromMms");
        new AsyncTask() {
            @Override
            protected void onPostExecute(Object result) {
                if (result instanceof BitmapDrawable) {
                    mMessageListView.setBackgroundDrawable((BitmapDrawable) result);
                } else if (result instanceof Integer) {
                    Logger.d(TAG, "It is a resource id");
                    mMessageListView.setBackgroundResource((Integer) result);
                }
            }

            @Override
            protected Object doInBackground(Object... params) {
                final Activity activity = getActivity();
                Logger.v(TAG, "activity = " + activity);
                if (activity != null) {
                    return loadWallPaperFromMmsDB(activity);
                }
                return null;
            }
        }.execute();
        return true;
    }

    /**
     * Load wall paper from mms database. Notice this method should only be
     * called in integrate mode.
     * 
     * @param activity A activity this fragment attached
     * @return A resource ID or a {@link BitmapDrawable} object
     */
    private Object loadWallPaperFromMmsDB(Activity activity) {
        String filePath = getWallPaperPath(activity);
        Logger.d(TAG, "WallPaperFromMmsDB(), filePath = " + filePath);
        if (TextUtils.isEmpty(filePath)) {
            return null;
        }
        if (filePath.startsWith(CONTENT_SCHEMA) || filePath.startsWith(FILE_SCHEMA)) {
            return loadWallPaperForFileSchema(activity, filePath);
        } else if (filePath.startsWith(WALLPAPER_PATH)) {
            return loadWallPaperForPath(activity);
        } else if (TextUtils.isDigitsOnly(filePath)) {
            int resId = 0;
            try {
                resId = Integer.parseInt(filePath);
                if (resId > 0) {
                    return resId;
                }
            } catch (NumberFormatException e) {
                Logger.w(TAG, "Unknown wall paper resource id! resourceId = " + resId);
            }
            Logger.w(TAG, "Unknown wall paper resource! filePath = " + filePath);
        }
        return null;
    }

    /**
     * Read wall paper file path. Notice this method should only be called in
     * integrate mode.
     * 
     * @param activity A activity this fragment attached
     * @return The wall paper file path.
     */
    private String getWallPaperPath(Activity activity) {
        // Load the global wall paper
        long threadId = 0L;
        String filePath = null;
        Cursor cursor = null;
        try {
            cursor = activity.getContentResolver().query(MMS_CONENT_URI, new String[] {
                    ThreadSettings._ID, ThreadSettings.WALLPAPER
            }, ThreadSettings.THREAD_ID + " = " + threadId, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                filePath = cursor.getString(1);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return filePath;
    }

    /**
     * Get a {@link BitmapDrawable} to set background for {@link ListView}.
     * Notice this method should only be called in integrate mode.
     * 
     * @param activity A activity this fragment attached
     * @return The wall paper file path.
     * @return A {@link BitmapDrawable} for the wall paper
     */
    private BitmapDrawable loadWallPaperForFileSchema(Activity activity, String filePath) {
        Uri imageUri = Uri.parse(filePath);
        InputStream is = null;
        try {
            is = activity.getContentResolver().openInputStream(imageUri);
            BitmapDrawable bitmatDrawable = new BitmapDrawable(is);
            return bitmatDrawable;
        } catch (FileNotFoundException e) {
            Logger.w(TAG, "FileNotFoundException: " + e.getMessage());
        } finally {
            if (null != is) {
                try {
                    is.close();
                } catch (IOException e) {
                    Logger.w(TAG, "IOException happened when close InputStream.");
                }
            }
        }
        return null;
    }

    /**
     * Get a {@link BitmapDrawable} to set background for {@link ListView}.
     * Notice this method should only be called in integrate mode.
     * 
     * @param activity A activity this fragment attached
     * @return A {@link BitmapDrawable} for the wall paper
     */
    private BitmapDrawable loadWallPaperForPath(Activity activity) {
        Logger.d(TAG, "loadWallPaperForPath() entry");
        long threadId = 0L;
        Uri imageUri = ContentUris.withAppendedId(PluginUtils.SMS_CONTENT_URI, threadId);
        InputStream inputStreams = null;
        try {
            inputStreams = activity.getContentResolver().openInputStream(imageUri);
            BitmapDrawable bitmatDrawable = new BitmapDrawable(inputStreams);
            return bitmatDrawable;
        } catch (FileNotFoundException e) {
            Logger.w(TAG, "loadWallPaperForPath() FileNotFoundException: " + e.getMessage());
        } catch (NullPointerException e) {
            Logger.w(TAG, "loadWallPaperForPath() NullPointerException: " + e.getMessage());
        } finally {
            if (null != inputStreams) {
                try {
                    inputStreams.close();
                } catch (IOException e) {
                    Logger.w(TAG, "loadWallPaperForPath() IOException: " + e.getMessage());
                }
            }
        }
        Logger.d(TAG, "loadWallPaperForPath() exit with null");
        return null;
    }
    
    private class ShareAdapter extends BaseAdapter {

        private String[] mStringArray;
        private int[] mIconArray;

        public ShareAdapter(String[] stringArray, int[] iconArray) {
            mStringArray = stringArray;
            mIconArray = iconArray;
        }

        @Override
        public int getCount() {
            int count = 0;
            return 3;
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(final int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                
                convertView = LayoutInflater.from(getActivity()).inflate(R.layout.share_grid_item, null);
                convertView.setTag(convertView);
            } else {
                convertView = (View) convertView.getTag();
            }

            TextView text = (TextView) convertView.findViewById(R.id.tv_share_name);
            ImageView img = (ImageView) convertView.findViewById(R.id.iv_share_icon);
            if (position < mStringArray.length) {
                text.setText(mStringArray[position]);
                img.setImageResource(mIconArray[position]);
            }
            return convertView;
        }
    }
}
