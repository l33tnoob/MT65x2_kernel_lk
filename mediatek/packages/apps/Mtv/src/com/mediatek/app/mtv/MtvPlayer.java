/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.app.mtv;



import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;

import android.preference.PreferenceManager;
import android.provider.MediaStore.Video;
import android.media.MediaMetadataRetriever;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.text.Editable;
import android.text.Selection;
import android.text.format.DateFormat;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TouchDelegate;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewConfiguration;
import android.view.ViewStub;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


import com.android.internal.telephony.ITelephony;
import com.mediatek.app.mtv.MtvEngine.ListItem;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.mtvbase.ChannelManager;


import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;
/**
 * Activity of the Camera which used to see preview and take pictures.
 */
public class MtvPlayer extends Activity implements View.OnClickListener,
        SurfaceHolder.Callback {

    private static final String TAG = "ATV/MtvPlayer";
    
    //private static final int UPDATE_SIGNAL_PERIOD = 20000;//in ms.

    private static final String STATE_CURRENT_TABLE = "curr_table";
    private static final String STATE_CURRENT_CHANNEL = "curr_chn";
    private static final String STATE_PREVIOUS_CHANNEL = "prev_chn";
    private static final String STATE_MUTED = "muted";
    private static final String STATE_CHANNEL_LIST = "chList";
    private static final String STATE_CHANNEL_NAME_LIST = "chNameList";
    
    //some menu ids
    private static final int MENU_RECORD = 100;
 
    private static final int MSG_SAVING_DONE = 0xf0009002; 
    private static final int MSG_UPDATE_RECORD_TIME = 0xf0009003;
    private static final int MSG_STOP_RECORDING = 0xf0009005;    
    private static final int MSG_INITIALIZATION = 0xf0009006;
    private static final int MSG_CONTROL_PANEL_ONOFF = 0xf0009007;
    private static final int MSG_SWITCH_CHANNEL_DONE = 0xf0009008;

    //dialog id
    private static final byte VIDEO_BRIGHTNESS = 1;//map to mConfigItem,don't change it.
    private static final byte VIDEO_CONTRAST = 2;//map to mConfigItem,don't change it.
    private static final byte VIDEO_SATURATION = 3;//map to mConfigItem,don't change it.    
    //dialog id
    private static final int DIALOG_INITIALIZE_HW_FAIL = 1;    
    
    private static final int POSITION_RECORD = 1;
    private static final int POSITION_GOTO_GALLERY = 2;
    private static final int POSITION_PREVIOUS_CHANNEL = 3;
    private static final int POSITION_VIDEO_SETTING = 4;
    private static final int POSITION_SOUND_SETTING = 5;
    private static final int POSITION_RENAME = 6;    
    

       //ATV audio formats
   private static final int MTS_MONO = 0x00000001;
   private static final int MTS_STEREO = 0x00000002;
   private static final int MTS_SAP = 0x00000004;   
   /* Japan & Korea MPX */
   private static final int MPX_MONO = 0x00000008;  
   private static final int MPX_STEREO = 0x00000010;
   private static final int MPX_SUB = 0x00000020;
   private static final int MPX_MAIN_SUB = 0x00000040;
   private static final int MPX_MAIN = 0x00000080;
   
   private static final int FM_MONO = 0x00000100;
   private static final int A2_STEREO = 0x00000200;
   private static final int A2_DUAL1 = 0x00000400;
   private static final int A2_DUAL2 = 0x00000800;     
   
   private static final int NICAM_MONO = 0x00001000;   
   private static final int NICAM_STEREO = 0x00002000;
   private static final int NICAM_DUAL1 = 0x00004000;
   private static final int NICAM_DUAL2 = 0x00008000;     
   
   private static final int AUDIO_MASK_MTS = (MTS_MONO | MTS_STEREO | MTS_SAP);
   private static final int AUDIO_MASK_MPX = (MPX_MONO | MPX_STEREO | MPX_SUB | MPX_MAIN_SUB | MPX_MAIN);
   private static final int AUDIO_MASK_FM = FM_MONO; 
   private static final int AUDIO_MASK_A2    = (A2_STEREO | A2_DUAL1 | A2_DUAL2);
   private static final int AUDIO_MASK_NICAM = (NICAM_MONO | NICAM_STEREO | NICAM_DUAL1 | NICAM_DUAL2); 
    

    private static final int INITIALIZE_SINGLE_LIST_PANEL = 0x1;    
    private static final int INITIALIZE_RECORD_UI = 0x2;
    private static final int INITIALIZE_VIDEO_PANEL = 0x4;        
    private static final int INITIALIZE_RENAME_EDITOR = 0x8;

    private static final int INITIALIZE_SECONDARY = INITIALIZE_RECORD_UI | INITIALIZE_VIDEO_PANEL
                                                    | INITIALIZE_RENAME_EDITOR;
    private static final int INITIALIZE_ALL = INITIALIZE_SINGLE_LIST_PANEL | INITIALIZE_SECONDARY;
    private int mInitializationMark;

    private MtvEngine mEngine;
    private boolean mMuted;//indicate whether the tv is muted now.   
    //private boolean mDidRegister = false;
    private boolean mScreenOn = true;//indicate whether the window is being focused.
    private boolean mBackPressed;    
    private boolean mServerDied;//underlayer server is died.    
    private boolean mChannelSwitching;
    private boolean mChannelChanged;

    private RelativeLayout mSurface;
    private SurfaceView mSurfaceView;
    private SurfaceView mSurfaceViewBlack;
    private TextView mRecordingTimeView;
    private View mProgressView;    
    private TextView mChannelNumView;     
    //private ImageView mSignalIndicator;
    //private Drawable mSignal[] = new Drawable[5];
    private SurfaceHolder mSurfaceHolder;
    private ChannelProvider mChannelProvider;
    private long mRemaingStorage;//how many storage we have now.
    private boolean mOpenFailed;    
    private static final long LOW_STORAGE_THRESHOLD = 512L * 1024L;

    private static final int ANIMATION_DURATION = 500;
    private static final int CONTROL_PANEL_ON_DURATION = 4000;
    
    private View mVideoSettingPanel = null;
    private View mSingleListPanel = null;
    private View mControlSettingPanel = null;
    private View mRenameEditor = null;
    private View mTranslateAnimationPanel = null;
    private View mAlphaAnimationPanel = null;
    
    private ListView mSingleListView = null;    
    private TextView mSingleListTitle = null;
    
    private int mSelectedValueBrightness;
    private int mSelectedValueContrast;
    private int mSelectedValueSaturation;
    private int mOldValueBrightness;
    private int mOldValueContrast;
    private int mOldValueSaturation;
    

    private long mRecordingStartTime;
    private String mCreatedVideoFilename;
    private ContentValues mCurrentVideoValues;    

    private OpenChannelThread mOpenThread = null;

    // Copied from MediaPlaybackService in the Music Player app. Should be
    // public, but isn't.
    private static final String SERVICECMD =
            "com.android.music.musicservicecommand";
    private static final String CMDNAME = "command";
    private static final String CMDPAUSE = "pause";

    private static final int MINIMUM = 0;
    private static final int MAXIMUM = 255;
    private static final String KEY_BRIGHTNESS = "prefer_brightness";
    private static final String KEY_CONTRAST = "prefer_contrast";
    private static final String KEY_SATURATION = "prefer_saturation";
    private static final String KEY_SOUND_FORMAT = "prefer_audio_format";
    private SharedPreferences mPreferences; 
    
    private int mOldValue;
    private int mSelectedValue;
    private int mAudioFormat;    
    private ArrayList<CharSequence> mAudioFormats; 
    private ArrayList<Integer> mAudioFormatsMap;       
    private CharSequence mAllAudioFormats[];
    
    private CallStateListener mIncomingCallListener;    
    private ContentResolver mContentResolver;  
    private final Handler mMainHandler = new MainHandler();
    private static final int STATE_NOT_CREATED = 0;
    private static final int STATE_CREATED = 1;
    private static final int STATE_OPENED = 2;
    private static final int STATE_CAPTURING = 3;
    private static final int STATE_RECORDING = 4;
    private static final int STATE_SAVING = 5;
    private static final int STATE_PAUSED = 6;
    private static final int STATE_UNINITIALIED_OPENED = 7;
    
    private static final long INVALID_DURATION = -1l;
    private static final long FILE_ERROR = -12;
    
    private int mState;     
    private boolean mRedrawFlag = false;

    private Drawable mMuteView;
    private Drawable mUnmuteView;  

    //private Bitmap mLastPictureThumb;  //TODO:implement snapshot playback in future.
    private Thread mJpgSaving = null;

    private boolean mIsPanelHidable;


    private ArrayList<ListItem> mList;
    private int mCurrentChannel = -1;
    private int mPreviousChannel = -1;

    private boolean mWatchOff = false;

    private BroadcastReceiver mReceiver = null;
    //private BroadcastReceiver mScreenStatusReceiver = null;
    /**
     * This Handler is used to post message back onto the main thread of the
     * application
     */
    private class MainHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            XLogUtils.d(TAG, "handleMessage what = " + msg.what);
            switch (msg.what) {

                case MSG_INITIALIZATION:
                    initVideoPanel();
                    initRenameEditor();
                    initRecordingUI();
                    break;
                case MtvEngine.MTV_MSG_NOTIFY_POWER_ON:
                    Intent intent = new Intent(MtvEngine.NOTIFY_POWER_ON);
                    sendBroadcast(intent);                
                    break;
                case MSG_CONTROL_PANEL_ONOFF: 
                   if (msg.arg1 == 0) {
                        fadeOut(mControlSettingPanel);
                    } else {
                        fadeIn(mControlSettingPanel);
                    }
                    break;
                case MSG_STOP_RECORDING:
                    stopVideoRecording(true);
                    break;
                /*
                case MSG_UPDATE_SIGNAL_INDICATOR: {
                    updateSignalIndicator();
                    break;
                }*/
                case MSG_SAVING_DONE:
                    if (mState == STATE_SAVING) {
                        mState = STATE_OPENED;
                    }
                    Toast.makeText(MtvPlayer.this,getResources().
                               getString(R.string.saved_to_gallery), 500).show();
                    mJpgSaving = null;
                    break;
                case MSG_UPDATE_RECORD_TIME:
                    updateRecordingTime();
                    break;
                case MtvEngine.MTV_MSG_CAPTURE_DONE:
                    onCapture((byte[])msg.obj);
                    break;
                case MtvEngine.MTV_AUDIO_FORMAT_CHANGED:
                    onAudioFormatChanged(msg.arg1);
                    break;
                case MtvEngine.MTV_MSG_INITIALIZE_DONE:
                    if (msg.arg1 == 0) {
                        initializeFalse();
                    } else {
                        //config video with the value user chose last time.
                        mEngine.configVideo(VIDEO_BRIGHTNESS,mPreferences
                                             .getInt(KEY_BRIGHTNESS, (MAXIMUM - MINIMUM) / 2));
                        mEngine.configVideo(VIDEO_CONTRAST,mPreferences
                                             .getInt(KEY_CONTRAST, (MAXIMUM - MINIMUM) / 2));
                        mEngine.configVideo(VIDEO_SATURATION,mPreferences
                                              .getInt(KEY_SATURATION, (MAXIMUM - MINIMUM) / 2));
                    }
                    break;
                case MtvEngine.MTV_MSG_OPEN_DONE:
                    if (msg.arg1 == 1) {
                        openTrue();
                    } else {
                        openFalse();
                    }        
                    break;    
                case MtvEngine.MTV_PLAYER_ERROR_SERVER_DIED:
                case MtvEngine.MTV_PLAYER_ERROR_UNKNOWN:
                    mServerDied = true;
                    finish();
                    break;
                //case MtvEngine.MTV_RECORDER_INFO_MAX_DURATION_REACHED:
                case MtvEngine.MTV_RECORDER_INFO_MAX_FILESIZE_REACHED:
                    // Show the toast.
                    Toast.makeText(MtvPlayer.this, getResources().getString(R.string.spaceIsLow_content),
                                       Toast.LENGTH_LONG).show();
                    stopVideoRecording(true);
                    break;
                case MtvEngine.MTV_RECORDER_ERROR_UNKNOWN:
                    stopVideoRecording(false);
                    break;

                case MSG_SWITCH_CHANNEL_DONE:
                    mSurfaceViewBlack.setVisibility(View.INVISIBLE);
                    mChannelSwitching = false;
                    if (mChannelChanged) {
                        switchChannel();
                    break;
                }
                default:
                    break;
        }
      }
   }
    /*
    private void updateSignalIndicator(){
        mSignalIndicator.setImageDrawable(mSignal[mEngine.getSignalStrengh()]);
        mMainHandler.sendEmptyMessageDelayed(MSG_UPDATE_SIGNAL_INDICATOR,
                UPDATE_SIGNAL_PERIOD);
    }*/

    /**
     * It should be called once only. We could have done these things in onCreate() but we want to
     * make screen appear as soon as possible.
     */
    private void initializeApp() {

        XLogUtils.d(TAG, "initializeApp()");
        findViewById(R.id.btn_mute).setOnClickListener(this);
        findViewById(R.id.btn_previous).setOnClickListener(this);
        
        findViewById(R.id.btn_next).setOnClickListener(this);
        findViewById(R.id.btn_capture).setOnClickListener(this);
        
        mMuteView = getResources().getDrawable(R.drawable.btn_ic_mute);
        mUnmuteView = getResources().getDrawable(R.drawable.btn_ic_unmute);  

    }

    //this method is used to disable the control bar from being focusable and clickable during the 
    //presence of other panels(channel panel,video setting panel...) and re-enable the control bar after 
    //the other panels are dismissed.
    private void setControlBarEnable(boolean enable) {

        XLogUtils.d(TAG, "setControlBarEnable() enable = " + enable);
        /*
        if (enable) {
            ((ViewGroup)mControlSettingPanel).setDescendantFocusability(ViewGroup.FOCUS_BEFORE_DESCENDANTS);
            if (mControlSettingPanel.isInTouchMode() == false) {
                mControlSettingPanel.requestFocus();
            }            
        } else {
            ((ViewGroup)mControlSettingPanel).setDescendantFocusability(ViewGroup.FOCUS_BLOCK_DESCENDANTS);            
        }    */

        
        View view = findViewById(R.id.btn_mute);
        view.setFocusable(enable);
        view.setClickable(enable);    
        
        view = findViewById(R.id.btn_previous);
        view.setFocusable(enable);
        view.setClickable(enable);    

        view = findViewById(R.id.btn_next);
        view.setFocusable(enable);
        view.setClickable(enable);            

        view = findViewById(R.id.btn_capture);
        view.setFocusable(enable);
        view.setClickable(enable);    

        view = findViewById(R.id.channel_num);
        view.setFocusable(enable);
        view.setClickable(enable);    

    }

    private void onCapture(final byte [] jpegData) {
            XLogUtils.d(TAG, "onCapture()");
            if (mState != STATE_CAPTURING) {
                return;
            }     
        
            mJpgSaving = new Thread(new Runnable() {
                public void run() {
                    storeImage(jpegData);
                    mMainHandler.sendEmptyMessage(MSG_SAVING_DONE);
                }
            });
            mState = STATE_SAVING;
            mJpgSaving.start();
    }

    private void storeImage(final byte [] jpegData) {

    XLogUtils.d(TAG, "storeImage()");
        try {
        long dateTaken = System.currentTimeMillis();
        String name = createName(dateTaken) + ".jpg";

        Uri mLastContentUri = ImageManager.addImage(
            mContentResolver,
            name,
            dateTaken,
            null, 
            ImageManager.getDefaultStoragePath(this), name,
            null, jpegData);

        if (mLastContentUri != null/*this means we got an error*/) {
            //long tt1 = System.currentTimeMillis();
            ImageManager.setImageSize(mContentResolver, mLastContentUri,
                new File(ImageManager.getDefaultStoragePath(this),
                name).length());
            //long tt2 = System.currentTimeMillis();
            //XLogUtils.d(TAG, "ImageManager.setImageSize needs " + (tt2 - tt1));
        }
        } catch (Exception ex) {
        XLogUtils.e(TAG, "Exception while compressing image.", ex);
        }
    }    
    
    private void doSnap() {

        XLogUtils.d(TAG, "doSnap(), state:" + mState);
        if (mState != STATE_OPENED) {
            return;
        }    

        mRemaingStorage = ImageManager.getStorageStatus(this);

        String noStorageText = getStorageStatusHint();        

        if (noStorageText != null) {
            Toast.makeText(this,noStorageText, 500).show();    
            return;
        }

        mState = STATE_CAPTURING;

        mEngine.capture();
    }    
    
    private static String createName(long dateTaken) {
        return DateFormat.format("yyyy-MM-dd kk.mm.ss", dateTaken).toString();
    }

    
    //Will display the control bar.Only used in QVGA now    
    private class ScreenTouchListener implements View.OnTouchListener {
        public boolean onTouch(View view, MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    return true;
                case MotionEvent.ACTION_UP:
                    changePanelVisibility();
                    return true;
                default:
                    break;                    
            }
            return false;
        }
    }
    
    //this method is used in QVGA LCD case to show/hide the control bar.    
    private void changePanelVisibility() {
        clearSettingPanelMessage();
        if (!slideOut(mTranslateAnimationPanel)) {
            if (mControlSettingPanel.getVisibility() == View.VISIBLE) {
                fadeOut(mControlSettingPanel);
            } else {
                fadeIn(mControlSettingPanel);
            }
        }
    }
    
    private void clearSettingPanelMessage() {
        if (mIsPanelHidable) {
            mMainHandler.removeMessages(MSG_CONTROL_PANEL_ONOFF);        
        }
    }

    /**
     * Save to retrieve Mtvplayer current state before being killed,
     * the state can be restored in onCreate().
     * @param state Bundle in which to place your saved state.
     */
    @Override
    protected void onSaveInstanceState(Bundle state) {
        state.putString(STATE_CURRENT_TABLE,mChannelProvider.getTableLocation());        
        state.putInt(STATE_CURRENT_CHANNEL, mCurrentChannel);
        state.putInt(STATE_PREVIOUS_CHANNEL, mPreviousChannel);
        state.putBoolean(STATE_MUTED, mMuted);
        int size = mList.size();
        String[] chList = new String[size];        
        String[] chNameList = new String[size];
        for (int i = 0; i < size; i++) {
            chList[i] = mList.get(i).mCh;
            chNameList[i] = mList.get(i).mName;            
        }
        state.putStringArray(STATE_CHANNEL_LIST, chList);
        state.putStringArray(STATE_CHANNEL_NAME_LIST, chNameList);
        super.onSaveInstanceState(state);
    }

    /**
     * Called when the activity is starting.
     * @param icicle Saved instance state.
     */
    @Override
    public void onCreate(Bundle icicle) {
        XLogUtils.d(TAG, "onCreate()");

        super.onCreate(icicle); 

        //we don't need to call this in normal case when MtvPlayer is activated by ChannelListActivity,
        //but we need to do it when it is recovered from an OOM death.
        //start engine initialization before the time-consuming function setContentView 
        //could reduce the launching time for the first time entrance.
        mEngine = MtvEngine.getEngine(MtvEngine.MTV_ATV);
        mEngine.setEventCallback(new MtvEngine.EventCallback() {
                public void onEvent(Message msg) {
                    mMainHandler.handleMessage(msg);
                }
        });

        //must initialize mPreferences before turnOn() because it may be called 
        //in the handler for MTV_MSG_INITIALIZE_DONE;
        mPreferences = PreferenceManager.getDefaultSharedPreferences(this);

        mEngine.turnOn();             
        
        setContentView(R.layout.mtv_player);
        mSurface = ((RelativeLayout)findViewById(R.id.surface));
        mSurfaceView = (SurfaceView) findViewById(R.id.display_surface);
        mSurfaceViewBlack = (SurfaceView) findViewById(R.id.display_surface_black);
        mSurfaceView.setOnTouchListener(new OnTouchListener() {
            public boolean onTouch(View v, MotionEvent event) {
                return slideOut(mTranslateAnimationPanel);
            }            
        });
        
        mContentResolver = getContentResolver();    
        
        mChannelProvider = ChannelProvider.instance(this,getResources().getStringArray(R.array.supported_location_codes));

        mCurrentChannel = mEngine.getCurrentChannel();
        mPreviousChannel = mCurrentChannel;

        if (mCurrentChannel == -1) {
            if (icicle == null) {
                finish();
                return;
            } else {
                String table = icicle.getString(STATE_CURRENT_TABLE);
                mCurrentChannel = icicle.getInt(STATE_CURRENT_CHANNEL,-1);
                mPreviousChannel = icicle.getInt(STATE_PREVIOUS_CHANNEL,-1);
                mMuted = icicle.getBoolean(STATE_MUTED,false);
                if (mCurrentChannel == -1) {
                    finish();
                    return;
                }
                mChannelProvider.setTableLocation(table);
            }
        }
        
        mList = (ArrayList<ListItem>)mEngine.getChannelList();
        if (mList  == null) {
            if (icicle == null) {
                finish();
                return;
            }            
            String[] chList = icicle.getStringArray(STATE_CHANNEL_LIST); 
            String[] chNameList = icicle.getStringArray(STATE_CHANNEL_NAME_LIST);            
            if (chList == null || chNameList == null) {
                finish();
                return;
            }            
            
            mList = new ArrayList<ListItem>();
            int size = chList.length;
            for (int i = 0; i < size; i++) {
                mList.add(new ListItem(Integer.parseInt(chList[i]),chNameList[i]));                
            }
        }
        
        // fix open a channel delay issue when stop scan
        try {
            mEngine.setFirstChannel(Integer.parseInt(mList.get(mCurrentChannel).mCh), mChannelProvider);
        } catch (ChannelManager.ChannelTableEmpty em) {
            em.printStackTrace();
            mEngine.turnOff(true);
            showDialog(DIALOG_INITIALIZE_HW_FAIL);
        }

        //let volume key handle media volume control. 
        setVolumeControlStream(AudioManager.STREAM_MATV);

        SurfaceHolder holder = mSurfaceView.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        mControlSettingPanel = (RelativeLayout)findViewById(R.id.control_bar);
        
        setDisplaySizeAndMenuBotton();                    
        
        //this is the first view being focused because it has element <requestFocus/>.
        mChannelNumView = (TextView)findViewById(R.id.channel_num);
        mChannelNumView.setText(mList.get(mCurrentChannel).mCh);
        mChannelNumView.setOnClickListener(this);
        
        mProgressView = findViewById(R.id.progress_indicator);
        
        installIntentFilter();
        
        initializeApp();

        mState = STATE_CREATED;
        mEngine.watchOn();    
    }

    //determine the display size based on LCD size.    
    private void setDisplaySizeAndMenuBotton() {
        WindowManager windowManager =
            (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        
        int lcdWidthNow = windowManager.getDefaultDisplay().getWidth();
        int lcdHeightNow = windowManager.getDefaultDisplay().getHeight();
                int lcdWidth = Math.max(lcdWidthNow, lcdHeightNow);
                int lcdHeight = Math.min(lcdWidthNow, lcdHeightNow);

        XLogUtils.d(TAG, "screen size: " + lcdWidth + "x" + lcdHeight);

        //determine the display size based on LCD size.
        android.view.ViewGroup.LayoutParams lp = mSurfaceView.getLayoutParams();
        lp.width = lcdWidth;
        lp.height = lcdHeight;

        View menuButton = null;
        if (!ViewConfiguration.get(this).hasPermanentMenuKey()) {
            menuButton = findViewById(R.id.options_menu);
            menuButton.setClickable(true);
            menuButton.setFocusable(true);
            menuButton.setOnClickListener(this);
            menuButton.setVisibility(View.VISIBLE);
            
            //enlarge the touchable area.
            final View button = menuButton;
            final View parent = (View) button.getParent(); 
            parent.post(new Runnable() {
                   public void run() { 
                    final Rect r = new Rect(); 
                    button.getHitRect(r); 
                    r.left -= 50;
                    r.right += 10;
                    parent.setTouchDelegate(new TouchDelegate(r,button));
                } 
            });            
        }        
        
        if (lcdWidth == 320 && lcdHeight == 240) {   //QVGA
            RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams)findViewById(R.id.surface).getLayoutParams();
            params.addRule(RelativeLayout.LEFT_OF, -1); 
            mSurfaceView.setOnTouchListener(new ScreenTouchListener());
            mSurfaceView.requestFocus();
            initControlPanel();
            mControlSettingPanel.setVisibility(View.INVISIBLE);
            mIsPanelHidable = true;
            
            if (menuButton != null) {
                RelativeLayout.LayoutParams obl = (RelativeLayout.LayoutParams)menuButton.getLayoutParams();
                obl.addRule(RelativeLayout.LEFT_OF, -1);
                obl.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);                
            }
        } else {
            android.view.ViewGroup.LayoutParams clp = mControlSettingPanel.getLayoutParams();
            XLogUtils.d(TAG, "control panel width: " + clp.width);
            int menuWidth = 0;
            if (menuButton != null) {
                RelativeLayout.LayoutParams obl = (RelativeLayout.LayoutParams)menuButton.getLayoutParams();
                menuWidth = obl.width;
            }
            lp.width = lp.width - clp.width - menuWidth;
            //must keep the ratio 320*240 which is the output signal ratio of ATV chip.
            if (lp.width > 0 && lp.height > 0) {
                if (lp.width * 240  > 320 * lp.height) {
                    lp.width =  lp.height * 320 / 240;
                } else if (lp.width * 240  < 320 * lp.height) {
                    lp.height = lp.width * 240 / 320;
                } 
            }
        }

        XLogUtils.d(TAG, "display size: " + lp.width + "x" + lp.height);
        android.view.ViewGroup.LayoutParams lpBlack = mSurfaceViewBlack.getLayoutParams();
        lpBlack.width = lp.width;
        lpBlack.height = lp.height;
    }

    private void initControlPanel() {    
        mControlSettingPanel.setOnTouchListener(new OnTouchListener() {
            public boolean onTouch(View v, MotionEvent event) {
                //if (mTranslateAnimationPanel == mRenameEditor) {
                    //slideOut(mRenameEditor);
                //}
                return true;
            }
            
        });
}

    
    private void initSingleListPanel() {
        
        if ((mInitializationMark & INITIALIZE_SINGLE_LIST_PANEL) != 0) {
             return;
        }

        mSingleListPanel = ((ViewStub)findViewById(R.id.stub_single_list)).inflate();
        mSingleListPanel.setOnTouchListener(new OnTouchListener() {

            public boolean onTouch(View v, MotionEvent event) {
                // TODO Auto-generated method stub
                return true;
            }
            
        });
        
        mSingleListView = (ListView)findViewById(R.id.single_list_view);
        mSingleListTitle = (TextView)findViewById(R.id.list_title);
        mSingleListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        //sound formats initializtion.
        mAudioFormats = new ArrayList<CharSequence>(); 
        mAudioFormatsMap = new ArrayList<Integer>();
        mAllAudioFormats = getResources().getTextArray(R.array.AudioFormats);

        mInitializationMark |= INITIALIZE_SINGLE_LIST_PANEL;
        
    }
    
    private void updateChannelList() {
        mOldValue = mCurrentChannel;
        mSingleListTitle.setText(getResources().getString(R.string.channel_list));
        String[] list = new String[mList.size()];
        for (int i = 0; i < mList.size(); i++) {
            list[i] = (mList.get(i).mCh) + ". " + (mList.get(i).mName);                
        }
        mSingleListView.setAdapter(new ArrayAdapter<String>(this,
                R.layout.single_list_item, list));
        
        mSingleListView.setOnItemClickListener(new OnItemClickListener() {

            public void onItemClick(AdapterView<?> parent, View view, int position,
                    long id) {
                // TODO Auto-generated method stub
                if (mCurrentChannel != position) {    
                    mPreviousChannel = mCurrentChannel;                    
                    mCurrentChannel = position;                            
                    mChannelNumView.setText(mList.get(mCurrentChannel).mCh);    
                    switchChannel(); 
                    
                }
            }
            
        });
        
        mSingleListView.setItemChecked(mCurrentChannel,true);
        
    }

    private void updateSoundFormatsList() {
        mOldValue = mPreferences.getInt(KEY_SOUND_FORMAT,0);
        mSelectedValue = mOldValue;
        onAudioFormatChanged(mEngine.getAudioFormat());
        
        myAssert(mAudioFormat != 0);

        mSingleListTitle.setText(getResources().getString(R.string.choose_sound_mode));
        
        CharSequence[] formats = mAudioFormats.toArray(new CharSequence[mAudioFormats.size()]);
        mSingleListView.setAdapter(new ArrayAdapter<CharSequence>(this,
               R.layout.single_list_item, formats));
        
        mSingleListView.setOnItemClickListener(new OnItemClickListener() {

            public void onItemClick(AdapterView<?> parent, View view, int position,
                    long id) {
                // TODO Auto-generated method stub
                mSelectedValue = mAudioFormatsMap.get(position);
                if (mSelectedValue != mOldValue) {
                    
                    mEngine.setAudioFormat(mSelectedValue);
                    
                    SharedPreferences.Editor editor = mPreferences.edit();
                    editor.putInt(KEY_SOUND_FORMAT, mSelectedValue);
                    editor.commit();                    
                }
            }
            
        });
        
        int position = mAudioFormatsMap.indexOf(mSelectedValue);
        mSingleListView.setItemChecked(position > -1 ? position : 0, true);
    }

    private void initRenameEditor() {
        if ((mInitializationMark & INITIALIZE_RENAME_EDITOR) != 0) {
             return;
        }
        
        ((ViewStub)findViewById(R.id.stub_rename_editor)).inflate();
        mRenameEditor = findViewById(R.id.rename_editor);    
        
        mInitializationMark |= INITIALIZE_RENAME_EDITOR;
    }
    
    private void initRecordingUI() {
        
        if ((mInitializationMark & INITIALIZE_RECORD_UI) != 0) {
            return;
        }
        
        ((ViewStub)findViewById(R.id.stub_recording_time_text)).inflate();
        mRecordingTimeView = (TextView) findViewById(R.id.recording_time);
        if (mRedrawFlag) {            
            mSurface.removeView(mRecordingTimeView);
            mSurface.addView(mRecordingTimeView);
            mRedrawFlag = false;
        }

        ((ViewStub)findViewById(R.id.stub_stop_recording_icon))
            .inflate()
            .setOnClickListener(this);
        mInitializationMark |= INITIALIZE_RECORD_UI;
    }

    private void initVideoPanel() {
        
        if ((mInitializationMark & INITIALIZE_VIDEO_PANEL) != 0) {
            return;
        }
        //video panel
        mVideoSettingPanel = ((ViewStub)findViewById(R.id.stub_video_setting)).inflate();
        //mVideoSettingPanel = findViewById(R.id.rtl_video_setting);
        mVideoSettingPanel.setOnTouchListener(new OnTouchListener() {

            public boolean onTouch(View v, MotionEvent event) {
                // TODO Auto-generated method stub
                return true;
            }
            
        });
        
        SeekBar skbBrightness = (SeekBar)findViewById(R.id.sek_brightness);
        SeekBar skbContrast = (SeekBar)findViewById(R.id.sek_contrast);
        SeekBar skbSaturation = (SeekBar)findViewById(R.id.sek_saturation);
        
        skbBrightness.setMax(MAXIMUM - MINIMUM);
        skbBrightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO apply brightness and save preference
                if (mSelectedValueBrightness != mOldValueBrightness) {
                    mEngine.configVideo((byte)VIDEO_BRIGHTNESS, mSelectedValueBrightness);
                    
                    SharedPreferences.Editor editor = mPreferences.edit();
                    editor.putInt(KEY_BRIGHTNESS, mSelectedValueBrightness);
                    editor.commit();
                    mOldValueBrightness = mSelectedValueBrightness;
                }
            }

            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
                
            }

            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                // TODO apply brightness
                mSelectedValueBrightness = progress + MINIMUM;
                mEngine.configVideo((byte)VIDEO_BRIGHTNESS, mSelectedValueBrightness);                 
            }
        });

        //some devices like e1kv2 may use trackball instead of touch screen to control the seekbar,
        //we need a place to save the changes for such cases and onFocusChange should be a suitable place.
        skbBrightness.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    // TODO apply brightness and save preference
                    if (mSelectedValueBrightness != mOldValueBrightness) {
                        mEngine.configVideo((byte)VIDEO_BRIGHTNESS, mSelectedValueBrightness);
                        
                        SharedPreferences.Editor editor = mPreferences.edit();
                        editor.putInt(KEY_BRIGHTNESS, mSelectedValueBrightness);
                        editor.commit();
                        mOldValueBrightness = mSelectedValueBrightness;
                    }
                }
            }
        });
            
        skbContrast.setMax(MAXIMUM - MINIMUM);
        skbContrast.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO apply contrast and save preference
                if (mSelectedValueContrast != mOldValueContrast) {
                    mEngine.configVideo((byte)VIDEO_CONTRAST, mSelectedValueContrast);
                    
                    SharedPreferences.Editor editor = mPreferences.edit();
                    editor.putInt(KEY_CONTRAST, mSelectedValueContrast);
                    editor.commit();
                    mOldValueContrast = mSelectedValueContrast;
                }
            }

            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
                
            }

            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                // TODO apply contrast
                mSelectedValueContrast = progress + MINIMUM;
                mEngine.configVideo((byte)VIDEO_CONTRAST, mSelectedValueContrast); 
            }
        });
        skbContrast.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    // TODO apply contrast and save preference
                    if (mSelectedValueContrast != mOldValueContrast) {
                        mEngine.configVideo((byte)VIDEO_CONTRAST, mSelectedValueContrast);
                        
                        SharedPreferences.Editor editor = mPreferences.edit();
                        editor.putInt(KEY_CONTRAST, mSelectedValueContrast);
                        editor.commit();
                        mOldValueContrast = mSelectedValueContrast;
                    }
                }
            }
        });
        
        skbSaturation.setMax(MAXIMUM - MINIMUM);
        skbSaturation.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO apply saturation and save preference
                if (mSelectedValueSaturation != mOldValueSaturation) {
                    mEngine.configVideo((byte)VIDEO_SATURATION, mSelectedValueSaturation);
                    
                    SharedPreferences.Editor editor = mPreferences.edit();
                    editor.putInt(KEY_SATURATION, mSelectedValueSaturation);
                    editor.commit();
                    mOldValueSaturation = mSelectedValueSaturation;
                }
            }

            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub
                
            }

            public void onProgressChanged(SeekBar seekBar, int progress,
                    boolean fromUser) {
                // TODO apply saturation
                mSelectedValueSaturation = progress + MINIMUM;
                mEngine.configVideo((byte)VIDEO_SATURATION, mSelectedValueSaturation); 
                
            }
        });
       
        skbSaturation.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    // TODO apply saturation and save preference
                    if (mSelectedValueSaturation != mOldValueSaturation) {
                        mEngine.configVideo((byte)VIDEO_SATURATION, mSelectedValueSaturation);
                        
                        SharedPreferences.Editor editor = mPreferences.edit();
                        editor.putInt(KEY_SATURATION, mSelectedValueSaturation);
                        editor.commit();
                        mOldValueSaturation = mSelectedValueSaturation;
                    }
                }
            }
        });        
        mInitializationMark |= INITIALIZE_VIDEO_PANEL;       
    }
    
    private void updateVideoPanel() {
        mOldValueBrightness = mPreferences.getInt(KEY_BRIGHTNESS, (MAXIMUM - MINIMUM) / 2);
        mSelectedValueBrightness = mOldValueBrightness;
        SeekBar skbBrightness = (SeekBar)findViewById(R.id.sek_brightness);            
        skbBrightness.setProgress(mOldValueBrightness - MINIMUM);    
        
        mOldValueContrast = mPreferences.getInt(KEY_CONTRAST, (MAXIMUM - MINIMUM) / 2);
        mSelectedValueContrast = mOldValueContrast;
        SeekBar skbContrast = (SeekBar)findViewById(R.id.sek_contrast);            
        skbContrast.setProgress(mOldValueContrast - MINIMUM);    
        
        mOldValueSaturation = mPreferences.getInt(KEY_SATURATION, (MAXIMUM - MINIMUM) / 2);
        mSelectedValueSaturation = mOldValueSaturation;
        SeekBar skbSaturation = (SeekBar)findViewById(R.id.sek_saturation);            
        skbSaturation.setProgress(mOldValueSaturation - MINIMUM);
    }

    /**
     * Called when the current window of the activity gains or loses focus.
     * @param hasFocus Whether the window of this activity has focus.
     */
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        XLogUtils.d(TAG, "onWindowFocusChanged() hasFocus = " + hasFocus + ",mScreenOn = " + mScreenOn);
        //see ALPS00126991.ALPS00136434.ALPS00137154
        //we need to handle below conditions:
        //1.entering suspend state by pressing suspend key.
        //In this scenario,onResume will be called while user press suspend key to exit suspend state.
        //we don't want TV to be opened at this point because user can't watch it until the screen is unlocked.
        //2.(ALPS00137154) when backlight sensor is used during call holding state.
        //In this scenario,onWindowFocusChanged pair(false/true) is not called but onPaused/onResume pair is called.
        //because we will turn off tv in onPause we need to turn on tv in onResume.
        if (!mScreenOn && hasFocus) {
            mScreenOn = true;
            XLogUtils.d(TAG, "onWindowFocusChanged() mState = " + mState + ",mOpenFailed = " + mOpenFailed);
            if (mState == STATE_CREATED && !mOpenFailed) {
            mEngine.watchOn();
            if (mProgressView.getVisibility() != View.VISIBLE) {                        
                mProgressView.setVisibility(View.VISIBLE);                                
            }
            }
        } else if (!hasFocus) {
            mScreenOn = false;
        }  
        super.onWindowFocusChanged(hasFocus);
    }
/*
    @Override
    public void onStart() {
        XLogUtils.d(TAG, "onStart()");
    
        super.onStart();
        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_USER_PRESENT);
        mScreenStatusReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                    XLogUtils.d(TAG, "ACTION_SCREEN_OFF");                
                    mScreenOn = false;
                    if (mState == STATE_PAUSED) {
                        mEngine.turnOff(true);
                    }
                } else if (action.equals(Intent.ACTION_USER_PRESENT)) {
               //we only turn on tv when screen is unlocked other than ACTION_SCREEN_ON is fired
                    XLogUtils.d(TAG, "ACTION_USER_PRESENT");
                    if (!mScreenOn) {
                    mScreenOn = true;
                    if (mState == STATE_CREATED) {
                        mEngine.watchOn();
                        mProgressView.setVisibility(View.VISIBLE);
                    }                
                }
            }
            }
        };
        
        registerReceiver(mScreenStatusReceiver, filter);
    }
*/
/*
    @Override
    protected void onStop() {
        super.onStop();
        unregisterReceiver(mScreenStatusReceiver);
    }
    
*/
    private void myAssert(boolean noassert) {
        if (!noassert) {
            throw new RuntimeException(TAG + " assertion failed!");
        }
    }


    private void initCallStateListener() {
        TelephonyManager tmgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        if (tmgr == null) {
            XLogUtils.e(TAG, "tmgr is null!! ");
        } else {
            mIncomingCallListener = new CallStateListener();
            tmgr.listen(mIncomingCallListener, PhoneStateListener.LISTEN_CALL_STATE);
        }
    }

    private void releaseCallStateListener() {
        if (mIncomingCallListener != null) {
            TelephonyManager tmgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
            if (tmgr == null) {
                XLogUtils.e(TAG, "tmgr is null!! ");
            } else {
                tmgr.listen(mIncomingCallListener, PhoneStateListener.LISTEN_NONE);
            }
            mIncomingCallListener = null;
        }
    }

    private boolean isMuteAllowed() {
        return mState == STATE_OPENED || mState == STATE_CAPTURING || mState == STATE_SAVING || mState == STATE_RECORDING;
    }

    /*private void mute() {
        if (isMuteAllowed()) {
        mEngine.setMute(true);
        }
        ((ImageView)findViewById(R.id.btn_mute)).setImageDrawable(mMuteView);                    
        mMuted = true;
    }

    private void unmute() {
        if (isMuteAllowed()) {
        mEngine.setMute(false);
        }
        ((ImageView)findViewById(R.id.btn_mute)).setImageDrawable(mUnmuteView);                 
        mMuted = false;
    }*/
    
    private void setMuteValue(boolean isMute) {
        if (isMuteAllowed()) {
            mEngine.setMute(isMute);
        }
        if (isMute) {
            ((ImageView)findViewById(R.id.btn_mute)).setImageDrawable(mMuteView);
        } else {
            ((ImageView)findViewById(R.id.btn_mute)).setImageDrawable(mUnmuteView);
        }
        mMuted = isMute;
    }

private class OpenChannelThread extends Thread {
        private boolean mReopen = false;        
        private final Object mLock = new Object();
        
        @Override
        public void run() {                
            while (true) {
                mEngine.changeChannel(Integer.parseInt(mList.get(mCurrentChannel).mCh)); 
                mMainHandler.sendEmptyMessage(MSG_SWITCH_CHANNEL_DONE);

                try {
                    waitOnThread();
                } catch (InterruptedException e) {
                    break;
                }

            }
        }        
        
        public void stopThread() {
                XLogUtils.d(TAG, "stopThread");
                interrupt();
        }                    

        public void reopen() {
            synchronized (mLock) {
                XLogUtils.d(TAG, "reopen");
                mReopen = true;
                mLock.notify();
            }
        }        
            
            
        private void waitOnThread() throws InterruptedException {
            synchronized (mLock) {
                XLogUtils.d(TAG, "waitOnThread");
                //by checking mReopen here we can catch those "notify" called before "wait".
                while (!mReopen) {
                    mLock.wait();
                }
                mReopen = false;
            }
        }
    }    

    private void switchChannel() {
        XLogUtils.d(TAG, "switchChannel() mState = " + mState + " mChannelSwitching = " + mChannelSwitching);

        if (mState != STATE_OPENED) {
            return;
        } else if (mChannelSwitching) {
            //just set a flag and do channel switch later.
            mChannelChanged = true;
            return;
        }
        
        mChannelSwitching = true;
        mChannelChanged = false;

        mMainHandler.removeMessages(MSG_SWITCH_CHANNEL_DONE);
        mSurfaceViewBlack.setVisibility(View.VISIBLE);
        if (mOpenThread == null) {
            mOpenThread = new OpenChannelThread();                    
            mOpenThread.start();                
        } else {
            mOpenThread.reopen();
        }
    }    

    /**
     * Called when a view has been clicked.
     * @param v The view that was clicked.
     */
    public void onClick(View v) {
        switch(mState) {
            case STATE_CREATED:
            case STATE_UNINITIALIED_OPENED:        
                Toast.makeText(this,getResources().getString(R.string.please_wait), 500).show();
                return;        
            case STATE_RECORDING:
                if (v.getId() != R.id.btn_mute && v.getId() != R.id.btn_stop_record) {
                //only allow mute and stop record when in recording state.
                    Toast.makeText(this,getResources().getString(R.string.stop_record_first), 1000).show();                
                    return;
                }
                break;
            case STATE_CAPTURING:
            case STATE_SAVING:
                //ignore clicking in such states.
                return;
            default:                
                break;
        }    

        doActionById(v);
    }

     /**
      * There will lots of click event, we will do it in this function.
      * @param v The view that was clicked.
      */
     private void doActionById(View v) {
        switch (v.getId()) {
            case R.id.options_menu:
                openOptionsMenu();
                break;
            case R.id.btn_mute:
                //clearSettingPanelMessage();
                if (mMuted) {
                    setMuteValue(false);
                } else {
                    setMuteValue(true);
                }                
                break;
            case R.id.btn_previous:
                //clearSettingPanelMessage();
                if (mList.size() > 1) {
                    mPreviousChannel = mCurrentChannel;
                    if (++mCurrentChannel >= mList.size()) {
                      //go to the first channel
                        mCurrentChannel = 0;
                    }
                    mChannelNumView.setText(mList.get(mCurrentChannel).mCh);
                    switchChannel();                
                }
                break;
            case R.id.btn_next:
                //clearSettingPanelMessage();
                if (mList.size() > 1) {
                    mPreviousChannel = mCurrentChannel;
                    if (--mCurrentChannel < 0) {
                        //go to the last channel
                        mCurrentChannel = mList.size() - 1;
                    }
                    mChannelNumView.setText(mList.get(mCurrentChannel).mCh);
                    switchChannel(); 
                }
                break;
            case R.id.channel_num:
                initSingleListPanel();
                updateChannelList();
                slideIn(mSingleListPanel);
                break;                
            case R.id.btn_capture:
                //clearSettingPanelMessage();
                doSnap();                
                break;
            case R.id.btn_stop_record:
                //clearSettingPanelMessage();
                // the view system changes the drawable state after posting click notification    
                //in order to let the button to be changed to pressed state,postpone the job of stopVideoRecording.
                mMainHandler.sendEmptyMessage(MSG_STOP_RECORDING);                
                break;
            default:
                break;                
        }
     }


    private void installIntentFilter() {
        // install an intent filter to receive SD card related events.
        IntentFilter intentFilter =
                new IntentFilter(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        //intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        mReceiver = new MyBroadcastReceiver();
        registerReceiver(mReceiver, intentFilter);
        //mDidRegister = true;
    }

    private void watch() {        
        try {            
            mEngine.setFirstChannel(
                    Integer.parseInt(mList.get(mCurrentChannel).mCh),mChannelProvider);
            mEngine.setSurface(mSurfaceHolder.getSurface());
        } catch (IOException ex) { //Exception ex
            ex.printStackTrace();
            mEngine.turnOff(true);
            showDialog(DIALOG_INITIALIZE_HW_FAIL);
            return;
        } catch (ChannelManager.ChannelTableEmpty em) {
            em.printStackTrace();
            mEngine.turnOff(true);
            showDialog(DIALOG_INITIALIZE_HW_FAIL);
        }

        mEngine.setMute(mMuted);
        //set flag here instead of in onCreate otherwise it will enter sleeping mode when back from other activities.
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);              
        
        mProgressView.setVisibility(View.GONE);     
        initSingleListPanel();
        if (mInitializationMark != INITIALIZE_ALL) {
            mMainHandler.sendEmptyMessage(MSG_INITIALIZATION);    
        }
        
        mState = STATE_OPENED;            
        //Debug.stopMethodTracing();
    }
    
    private void openTrue() {            
        XLogUtils.d(TAG, "openTrue() mState = " + mState);
        switch(mState) {
            case STATE_NOT_CREATED://this is because opening TV is done when watchOn is called in onCreate.
            case STATE_CREATED:
                if (mSurfaceHolder == null) { //app initialization is not done or surface is not ready.
                    mState = STATE_UNINITIALIED_OPENED;
                } else {
                    watch();
                }
                break;                    
            default:
                //ingore asnchronous events because it may already in queue when state is changed.
                return;
        }        
    }

    private void openFalse() {                    
        XLogUtils.d(TAG, "openFalse() mState = " + mState);
        if (mState == STATE_CREATED) {
            mOpenFailed = true;
            mProgressView.setVisibility(View.GONE); 
            showDialog(DIALOG_INITIALIZE_HW_FAIL);        
        }            
    }

    private void initializeFalse() {            
        XLogUtils.d(TAG, "initializeFalse() mState = " + mState);
        //we treat it the same as open false at present.
        //Maybe we can provide different error message to user in future.
        if (mState == STATE_CREATED) {
            mOpenFailed = true;
            mProgressView.setVisibility(View.GONE); 
            showDialog(DIALOG_INITIALIZE_HW_FAIL);                
        }                            
    }    

    /**
     * Call after onCreate() or after onRestart() when the activity had been stopped,
     * but is now again being displayed to the user. It will be followed by onResume().
     */
    @Override
    protected void onStart() {
        XLogUtils.d(TAG, "onStart()");
        super.onStart();
        if (FeatureOption.MTK_VT3G324M_SUPPORT) {  
            boolean isVTIdle = true;
            try {
                ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
                if (iTel == null) {
                    XLogUtils.e(TAG, "iTel is null!! ");
                } else {  //see ALPS00047426.
                    isVTIdle = iTel.isVTIdle();
                }
            } catch (RemoteException ex) {
                ex.printStackTrace();
            } finally {
                if (!isVTIdle) {                        
                    //setResult(-100);//use -100 to indicate this is an unsolicited exit due to VT call.
                    finish();
                }
            }
        }
    }

    /**
     * Called after onResoreInstanceState(), onRestart(), onPause(), for your activity to
     * start interacting with the user.
     */
    @Override
    public void onResume() {
        XLogUtils.d(TAG, "onResume()");
        mWatchOff = false;
        super.onResume();

        // resolve quit the application when the call coming and enter into matv
        // by application switch
        if (FeatureOption.MTK_VT3G324M_SUPPORT) {
            boolean isVTIdle = true;
            try {
                ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
                if (iTel == null) {
                    XLogUtils.e(TAG, "iTel is null!! ");
                } else { // we need to check it always.see ALPS00047426.
                    isVTIdle = iTel.isIdle();
                }
            } catch (RemoteException ex) {
                ex.printStackTrace();
            } finally {
                if (!isVTIdle) {
                    finish();
                }
            }
        }

        // resolve for alps00110576,in order to make mProgressView and mRecordingTimeView can be drawn above surfaceview 
        // so we must eremove/add mProgressView and mRecordingTimeView
        if (mRedrawFlag) {
            mSurface.addView(mSurfaceView);
            mSurface.removeView(mProgressView);
            mSurface.addView(mProgressView);
            // mRecordingTimeView might be null when surfaceview was be removed last time,
            // so we will remove/add mRecordingTimeView after mRecordingTimeView is initialized.
            // ALPS00243029
            if (mRecordingTimeView != null) {
                mSurface.removeView(mRecordingTimeView);
                mSurface.addView(mRecordingTimeView);
                mRedrawFlag = false;
            }

        }

        switch(mState) {
            case STATE_CREATED:
            case STATE_UNINITIALIED_OPENED:    
                break;        
            case STATE_PAUSED:
                
                if (mScreenOn) {
                    KeyguardManager km = (KeyguardManager)getSystemService(Context.KEYGUARD_SERVICE); 
                    if (km == null /*reguard it as no keyguard*/ || !km.inKeyguardRestrictedInputMode()) {
                        mEngine.watchOn();
                        if (mProgressView.getVisibility() != View.VISIBLE) {
                            mProgressView.setVisibility(View.VISIBLE);             
                        }
                    }
                }

                /*else {
                    mEngine.turnOn();
                }*/
                mState = STATE_CREATED;
                break;                
            default:
                myAssert(false);
        }    
        //set flag here instead of in onCreate otherwise it will enter sleeping mode when back from other activities.
        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);  
               
        initCallStateListener();

        //mute background music and FM.
        Intent i = new Intent(SERVICECMD);
        i.putExtra(CMDNAME, CMDPAUSE);
        sendBroadcast(i);

        i = new Intent("com.mediatek.FMRadio.FMRadioService.ACTION_TOFMSERVICE_POWERDOWN");
        sendBroadcast(i);
    }

    /**
     *Called the an activity is going into the background, but has not been killed.
     */
    @Override
    protected void onPause() {
        XLogUtils.d(TAG, "onPause()");
        mSurface.removeView(mSurfaceView);
        mRedrawFlag = true;
                mWatchOff = true;

        if (!mServerDied) {
            switch(mState) {
                case STATE_RECORDING:    
                        //mute it here to prevent sound comes out after screen is off. 
                    mEngine.setMute(true);
                    stopVideoRecording(true);
                    //fall through
                case STATE_CREATED:
                case STATE_UNINITIALIED_OPENED:    
                case STATE_OPENED:                
                case STATE_CAPTURING:                
                    break;        
                case STATE_SAVING:                
                    if (!mMainHandler.hasMessages(MSG_SAVING_DONE)) {
                        try {
                            mJpgSaving.join();
                        } catch (InterruptedException e) {
                            // do nothing
                            XLogUtils.d(TAG, "onPause() InterruptedException");
                        }
                    }    
                    mJpgSaving = null;                
                    break;
                default:
                    myAssert(false);
            }    
        }

        releaseCallStateListener();    
        
        // Remove the messages in the event queue.
        mMainHandler.removeMessages(MSG_INITIALIZATION);        
        mMainHandler.removeMessages(MSG_SAVING_DONE);   

        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);  

        if (mTranslateAnimationPanel != null) {
            mTranslateAnimationPanel.setVisibility(View.INVISIBLE);
            setControlBarEnable(true);
        }
        clearSettingPanelMessage();
        
        //we can recover the current channel in case of recreation due to configuration change.
        mEngine.setCurrentChannel(mCurrentChannel);
        
        if (!mServerDied) {
            //do not force shutdown if it is going back to ChannelListActivity.
            mEngine.turnOff(!mBackPressed);
        }
        
        
        mState = STATE_PAUSED;
        super.onPause();        
    }

    /**
     * Perform any final cleanup before an activity is destoryed.
     */
    @Override
    public void onDestroy() {
        if (mOpenThread != null) {
            mOpenThread.stopThread();
        }
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
        }
        if (!mWatchOff) {
            // Now mtvPlayer has been finished (maybe has been canceled by activity manager)
            mEngine.turnOff(true);
        }
        super.onDestroy();        
    }

    /**
     * Called when the activity has detected the user's press of the back key.
     */
    @Override
    public void onBackPressed() {
        XLogUtils.d(TAG, "onBackPressed() mBackPressed = " + mBackPressed);
        if (mState == STATE_CAPTURING || mState == STATE_SAVING) {
            // ignore backs while we're taking a picture
            return;
        } else if (mState == STATE_RECORDING) {
            stopVideoRecording(true);
            return;
        }
        if (slideOut(mTranslateAnimationPanel) 
            || fadeOut(mAlphaAnimationPanel)) {
            return;
        } 
        mBackPressed = true;
        super.onBackPressed();
    }

    /**
     * Called when the activity has detected the user's long press of key.
     * @param keyCode The value in event.getKeyCode().
     * @param event Description of the key event.
     * @return If you handled the event, return true. if you want to allow the event
     * to be handled by the next receiver, return false.
     */
    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        XLogUtils.d(TAG, "onKeyLongPress() mState = " + mState);
    
        if (keyCode == KeyEvent.KEYCODE_CAMERA) {
                switch(mState) {
                    case STATE_CAPTURING:                
                        //do not allow entering camera when in capturing process.
                        return true;
                    case STATE_RECORDING:
                        stopVideoRecording(true);
                        //fall through
                    case STATE_UNINITIALIED_OPENED:    
                    case STATE_CREATED:                         
                    case STATE_OPENED:
                        mEngine.watchOff();
                        mState = STATE_CREATED;                        
                        break;        
                    case STATE_SAVING:                
                        if (!mMainHandler.hasMessages(MSG_SAVING_DONE)) {
                            try {
                                mJpgSaving.join();
                            } catch (InterruptedException e) {
                                // do nothing
                                XLogUtils.d(TAG, "onKeyLongPress() InterruptedException");
                            }
                        }    
                        mJpgSaving = null;    
                        mEngine.watchOff();
                        mState = STATE_CREATED;                        
                        break;
                    default:
                        break;
                }            
        }

        return super.onKeyLongPress(keyCode, event);
    }

    /**
     * Called when a key down event has occurred.
     * @param keyCode The value in event.getKeyCode().
     * @param event Description of the key event.
     * @return If you handled the event, return true. if you want to allow the event
     * to be handled by the next receiver, return false.
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_SEARCH:
                // do nothing since we don't want search box which may cause UI crash
                // TODO: mapping to other useful feature
                return true;
            case KeyEvent.KEYCODE_MENU:
                if (event.isLongPress()) {
                    return true;    // consume LongPress to prevent VK popup.
                }
                return false;
            case KeyEvent.KEYCODE_MEDIA_PREVIOUS:
            case KeyEvent.KEYCODE_MEDIA_NEXT:
            case KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE:
                // TODO: Handle next / previous accordingly, for now we're
                // just consuming the events.
                return true;

            default:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }    

    /**
     * This is called immediately after any structural changes(format or size) have been
     * made to the surface.
     * @param holder The SurfaceHolder whose surface has changed.
     * @param format The new PixelFormat of the surface.
     * @param w The new width of the surface.
     * @param h The new height of the surface.
     */
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        // Make sure we have a surface in the holder before proceeding.
        if (holder.getSurface() == null) {
            XLogUtils.d(TAG, "holder.getSurface() == null");
            return;
        }

        XLogUtils.d(TAG, "surfaceChanged() mState = " + mState);

        // We need to save the holder for later use.
        mSurfaceHolder = holder;

        if (mState == STATE_UNINITIALIED_OPENED) {
            watch();          
        }          
    }

    /**
     * This is called immediately after the surface is first created.
     * @param holder The SurfaceHodler whose surface is being created.
     */
    public void surfaceCreated(SurfaceHolder holder) {

        XLogUtils.d(TAG, "surfaceCreated()");
    }

    /**
     * This is called immediately after the surface is first destroyed.
     * @param holder The SurfaceHodler whose surface is being created.
     */
    public void surfaceDestroyed(SurfaceHolder holder) {
        XLogUtils.d(TAG, "surfaceDestroyed()");
        mSurfaceHolder = null;
    }

    private void gotoGallery() {
        MenuHelper.gotoMtvGallery(this);
    }

    /**
     * Prepare the Screen's standard option menu to be displayed.
     * @param menu The option menu as last shown or first initialized by onCreateOptionMenu().
     */
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);

        //just hide the setting panel if it is being showed and do not show up options menu for this time.
        if ((mRenameEditor != null && mRenameEditor.getVisibility() == View.VISIBLE) 
            || slideOut(mTranslateAnimationPanel) || fadeOut(mAlphaAnimationPanel) 
            || mChannelSwitching) {
            return false;
        } else if (mState == STATE_OPENED) {
            return true;
        } else if (mState == STATE_RECORDING) {
            Toast.makeText(this,getResources().getString(R.string.stop_record_first), 1000).show(); 
        }            
            return false;            
    }

    /**
     * Initialize the contents of the Activity's standard options menu.
     * @param menu The option menu in which you place your items.
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.player_menu, menu);
        return true;
    }

    /**
     * This hook is called whenever an item in your options menu is selected.
     * @param item The menu item that was selected.
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        
        switch(id) {
            case R.id.record:
                initRecordingUI();
                startVideoRecording();
                break;
            
            case R.id.gallery:
                gotoGallery();
                break;
            
            case R.id.previous_channel:
                gotoPreviousChannel();
                break;                
            
            case R.id.video_setting:
                initVideoPanel();
                updateVideoPanel();    
                slideIn(mVideoSettingPanel);
                break;
            
            case R.id.sound_setting:
                initSingleListPanel();
                updateSoundFormatsList();
                slideIn(mSingleListPanel);
                break;
            
            case R.id.rename:
                initRenameEditor();                
                mOldValue = mCurrentChannel;//Solve ALPS00127375.mCurrentChannel may be changed during renaming.
                EditText nameTextEdit = (EditText)findViewById(R.id.rename_box);
                //nameTextEdit.setHighFocusPriority(true);
                nameTextEdit.setText(mList.get(mOldValue).mName,TextView.BufferType.EDITABLE);
                Editable text = nameTextEdit.getText();
                int index = text.length();
                Selection.setSelection(text, index);    
                findViewById(R.id.rename_button_ok).setOnClickListener(new View.OnClickListener() {
                    public void onClick(View v) {
                                EditText nameTextEdit = (EditText)findViewById(R.id.rename_box);
                                String name = nameTextEdit.getText().toString();
                                if (!"".equals(name)) {
                                    ListItem item = mList.get(mOldValue);
                                    item.mName = name;
                                    mChannelProvider
                                       .updateChannelName(Integer.parseInt(item.mCh),name);
                                }
                                slideOut(mRenameEditor);
                            }
                });
                slideIn(mRenameEditor);
                break;
            default:
                break;
        }
        return true;
    }
    
    private void stopVideoRecording(boolean toSave) {
        XLogUtils.d(TAG, "stopVideoRecording");
        if (mState == STATE_RECORDING) {
            try {
                mEngine.stopRecord();
            } catch (RuntimeException e) {
                XLogUtils.e(TAG, "stop fail: " + e.getMessage());
            }

            XLogUtils.v(TAG, "Setting current video filename: "
                    + mCreatedVideoFilename);
            
            findViewById(R.id.btn_stop_record).setVisibility(View.GONE);
            findViewById(R.id.btn_capture).setVisibility(View.VISIBLE);
                mRecordingTimeView.setVisibility(View.GONE);
                
            releaseRecorder();    
            mState = STATE_OPENED;    
            
            mRemaingStorage = ImageManager.getStorageStatus(this);

            if (toSave && mCreatedVideoFilename != null && mRemaingStorage != ImageManager.ERROR_NO_STORAGE) {
                registerVideo();
                Toast.makeText(MtvPlayer.this,getResources().getString(R.string.saved_to_gallery), 500).show();
            }
            mCreatedVideoFilename = null;
    }
    }

    private void releaseRecorder() {
        XLogUtils.v(TAG, "Releasing recorder.");
            cleanupEmptyFile();
            mEngine.releaseRecorder();
    }

    private void updateRecordingTime() {
        if (mState != STATE_RECORDING) {
            return;
        }
        long now = SystemClock.uptimeMillis();
        long delta = now - mRecordingStartTime;

        
        long nextUpdateDelay = 1000 - (delta % 1000);
        long seconds = delta / 1000; // round to nearest

        long minutes = seconds / 60;
        long hours = minutes / 60;
        long remainderMinutes = minutes - (hours * 60);
        long remainderSeconds = seconds - (minutes * 60);

        String secondsString = Long.toString(remainderSeconds);
        if (secondsString.length() < 2) {
            secondsString = "0" + secondsString;
        }
        String minutesString = Long.toString(remainderMinutes);
        if (minutesString.length() < 2) {
            minutesString = "0" + minutesString;
        }
        String text = minutesString + ":" + secondsString;
        if (hours > 0) {
            String hoursString = Long.toString(hours);
            if (hoursString.length() < 2) {
                hoursString = "0" + hoursString;
            }
            text = hoursString + ":" + text;
        }
        mRecordingTimeView.setText(text);
        mMainHandler.sendEmptyMessageDelayed(
                MSG_UPDATE_RECORD_TIME, nextUpdateDelay);
    }
    private class CallStateListener extends PhoneStateListener {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            if (state == TelephonyManager.CALL_STATE_RINGING || state == TelephonyManager.CALL_STATE_OFFHOOK) {
                if (mState == STATE_RECORDING) {
                    XLogUtils.v(TAG, "Recording stopped due to incoming call");
                    stopVideoRecording(true);
                }

            }
        }
    }    
    
    private class MyBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            XLogUtils.d(TAG, "onReceive action = " + action);
            
            if (action.equals(Intent.ACTION_MEDIA_EJECT) || action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {                
                if (mState == STATE_RECORDING) {
                    //mCreatedVideoFilename is created based on the default sdcard directory when it is created.
                    //however the default sdcard directory may be changed now, 
                    //so we can't get it by Environment.getDefaultStorageDirectory() here.                     
                    String pathInUse = getVideoDirectory();
                    String removingPath = intent.getData().getPath();
                    XLogUtils.d(TAG, "path_in_use = " + pathInUse + " removing_path = " + removingPath);

                    //if removing sdcard is not the one we are using,do nothing.
                    if (!pathInUse.equals(removingPath)) {
                        return;
                    }
                    
                     //mute it here to prevent sound comes out after screen is off. 
                    mEngine.setMute(true);
                    stopVideoRecording(false);
                    finish();
            }
        }
    }
    }

       
    private String getVideoDirectory() {
        int pos = mCreatedVideoFilename.indexOf(ImageManager.MTV_GALLARY_SUBPATH);
        return mCreatedVideoFilename.substring(0,pos);  
    }       
    
    private void createVideoPath() {
        long dateTaken = System.currentTimeMillis();
        String title = createName(dateTaken);
        String displayName = title + ".3gp"; // Used when emailing.
        String mtvDirPath = ImageManager.getDefaultStoragePath(this);
        //File dir = new File(mtvDirPath);
        //dir.mkdirs();
        SimpleDateFormat dateFormat = new SimpleDateFormat(
                getString(R.string.video_file_name_format),Locale.US);
        Date date = new Date(dateTaken);
        String filepart = dateFormat.format(date);
        String filename = mtvDirPath + "/" + filepart + ".3gp";
        ContentValues values = new ContentValues(7);
        values.put(Video.Media.TITLE, title);
        values.put(Video.Media.DISPLAY_NAME, displayName);
        values.put(Video.Media.DATE_TAKEN, dateTaken);
        values.put(Video.Media.MIME_TYPE, "video/3gpp");
        values.put(Video.Media.DATA, filename);
        values.put(Video.Media.RESOLUTION, "320x240");
        mCreatedVideoFilename = filename;
        XLogUtils.v(TAG, "Current video filename: " + mCreatedVideoFilename);
        mCurrentVideoValues = values;
    }

    private void registerVideo() {

            Uri videoTable = Uri.parse("content://media/external/video/media");
            mCurrentVideoValues.put(Video.Media.SIZE,
                    new File(mCreatedVideoFilename).length());
        mCurrentVideoValues.put(Video.Media.DURATION, getDuration());
            videoTable = mContentResolver.insert(videoTable,
                    mCurrentVideoValues);
            XLogUtils.v(TAG, "Current video URI: " + videoTable);

        mCurrentVideoValues = null;
    }
    
    private long getDuration() {
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(mCreatedVideoFilename);
            return Long.valueOf(retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION));
        } catch (IllegalArgumentException e) {
            return INVALID_DURATION;
        } catch (RuntimeException e) {
            return FILE_ERROR;
        } finally {
            retriever.release();
        }
    }

    private String getStorageStatusHint() {
        if (mRemaingStorage == ImageManager.ERROR_NO_STORAGE) {        
            return getString(R.string.no_storage);
        } else if (mRemaingStorage == ImageManager.ERROR_MEDIA_CHECKING) {
            return getString(R.string.preparing_sd);
        } else if (mRemaingStorage < LOW_STORAGE_THRESHOLD) {
            return getString(R.string.spaceIsLow_content);
        } else {
            return null;
        }
    }
    
    private void startVideoRecording() {
        XLogUtils.d(TAG, "startVideoRecording mState = " + mState);
        if (mState != STATE_OPENED) { //solve ALPS00225019:ignore quick clicking.
            return;
        }
        mRemaingStorage = ImageManager.getStorageStatus(this);
        
        String noStorageText = getStorageStatusHint();

        if (noStorageText != null) {
            Toast.makeText(this,noStorageText, 500).show();    
            return;
        }       
        
        createVideoPath();

        long maxFileSize = mRemaingStorage - LOW_STORAGE_THRESHOLD / 4;
        if (maxFileSize > (long)2 << 30) {
            maxFileSize = (long)2 << 30;//maxium file size supported on FAT.
        }        
        
        try {
            mEngine.startRecord(mCreatedVideoFilename,maxFileSize,
                mSurfaceHolder.getSurface()); 
        } catch (Exception e) {
            XLogUtils.e(TAG, "Could not start media recorder. ", e);
            mEngine.releaseRecorder();
            return;
        }
            
        findViewById(R.id.btn_stop_record).setVisibility(View.VISIBLE);
        /*we can't set it to GONE otherwise btn_stop_record is positioned incorrectly*/
        findViewById(R.id.btn_capture).setVisibility(View.INVISIBLE);
        
        mRecordingStartTime = SystemClock.uptimeMillis();
        mRecordingTimeView.setVisibility(View.VISIBLE); 
        if (mIsPanelHidable) {
            mMainHandler.sendMessageDelayed(mMainHandler
                  .obtainMessage(MSG_CONTROL_PANEL_ONOFF, 1,0, null),ANIMATION_DURATION);
            mMainHandler.sendMessageDelayed(mMainHandler
                  .obtainMessage(MSG_CONTROL_PANEL_ONOFF, 0,0, null),CONTROL_PANEL_ON_DURATION);
        }
        
        mState = STATE_RECORDING;
        
        updateRecordingTime();
    }
    
    private void cleanupEmptyFile() {
        if (mCreatedVideoFilename != null) {
            File f = new File(mCreatedVideoFilename);
            if (f.length() == 0 && f.delete()) {
                XLogUtils.v(TAG, "Empty video file deleted: " + mCreatedVideoFilename);
                mCreatedVideoFilename = null;
            }
        }
    } 

    /**
     * Create managed dialog.
     * only create dialog for the initialization failure case.
     * Not suggest to create dialog for prompting information to users and interact with users in MtvPlayer.
     * @param id Int The id of the dialog.
     * @return Dialog The dialog we created.
     */
    protected Dialog onCreateDialog(int id) {
        Dialog dlg = null;
        if (id == DIALOG_INITIALIZE_HW_FAIL) {                
                 dlg = new AlertDialog.Builder(this)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setTitle(R.string.analog_tv)
                .setMessage(R.string.hw_init_failed)    
                .setNeutralButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        //
                    }
                })          
                .create();
                
                dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        finish();
                    }
                });  
        }
        
        return dlg;
    }
    
    
    private void gotoPreviousChannel() {
        if (mState != STATE_OPENED) { //solve ALPS00125835:ignore quick clicking
            return;
        }
        
        if (mCurrentChannel != mPreviousChannel) {
            int temp = mPreviousChannel;
            mPreviousChannel = mCurrentChannel;
            mCurrentChannel = temp;        
            mChannelNumView.setText(mList.get(mCurrentChannel).mCh);
            switchChannel(); 
        }
    }     


    private void onAudioFormatChanged(int newFormat) {
           
        XLogUtils.d(TAG, "onAudioFormatChanged() newFormat = " + newFormat + "; mAudioFormat = " + mAudioFormat);
           if (mAudioFormat != newFormat) {
               mAudioFormatsMap.clear(); 
               mAudioFormats.clear();
               //Since a specific program limits to a specific audio system,we separate the matching process by audio system.
               if ((newFormat & AUDIO_MASK_MTS) != 0) {
                   /* MTS SYSTEM */
                   if ((newFormat & MTS_MONO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[0]);
                       mAudioFormatsMap.add(MTS_MONO);
                   }
                   
                   if ((newFormat & MTS_STEREO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[1]);  
                       mAudioFormatsMap.add(MTS_STEREO);
                   }              
                   
                   if ((newFormat & MTS_SAP) != 0) {
                       mAudioFormats.add(mAllAudioFormats[2]); 
                       mAudioFormatsMap.add(MTS_SAP);
                   }                     
               } else if ((newFormat & AUDIO_MASK_MPX) != 0) {
                   if ((newFormat & MPX_MONO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[0]);
                       mAudioFormatsMap.add(MPX_MONO);
                   }
                   
                   if ((newFormat & MPX_STEREO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[1]); 
                       mAudioFormatsMap.add(MPX_STEREO);
                   }              
                   
                   if ((newFormat & MPX_SUB) != 0) {
                       mAudioFormats.add(mAllAudioFormats[3]); 
                       mAudioFormatsMap.add(MPX_SUB);
                   }  
                   
                   if ((newFormat & MPX_MAIN_SUB) != 0) {
                       mAudioFormats.add(mAllAudioFormats[4]);
                       mAudioFormatsMap.add(MPX_MAIN_SUB);
                   }              
                   
                   if ((newFormat & MPX_MAIN) != 0) {
                       mAudioFormats.add(mAllAudioFormats[5]); 
                       mAudioFormatsMap.add(MPX_MAIN);
                   }                 
               } else if ((newFormat & AUDIO_MASK_A2) != 0) {
                   if ((newFormat & A2_STEREO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[1]); 
                       mAudioFormatsMap.add(A2_STEREO);
                   }
                   
                   if ((newFormat & A2_DUAL1) != 0) {
                       mAudioFormats.add(mAllAudioFormats[6]);
                       mAudioFormatsMap.add(A2_DUAL1);
                   }              
                   
                   if ((newFormat & A2_DUAL2) != 0) {
                       mAudioFormats.add(mAllAudioFormats[7]);  
                       mAudioFormatsMap.add(A2_DUAL2);
                   }  
                   
               } else if ((newFormat & AUDIO_MASK_NICAM) != 0) {
                   if ((newFormat & NICAM_MONO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[8]); 
                       mAudioFormatsMap.add(NICAM_MONO);
                   }
                   
                   if ((newFormat & NICAM_STEREO) != 0) {
                       mAudioFormats.add(mAllAudioFormats[9]);  
                       mAudioFormatsMap.add(NICAM_STEREO);
                   }              
                   
                   if ((newFormat & NICAM_DUAL1) != 0) {
                       mAudioFormats.add(mAllAudioFormats[10]); 
                       mAudioFormatsMap.add(NICAM_DUAL1);
                   }  
                   
                   if ((newFormat & NICAM_DUAL2) != 0) {
                       mAudioFormats.add(mAllAudioFormats[11]); 
                       mAudioFormatsMap.add(NICAM_DUAL2);
                   }  
               }  else if ((newFormat & AUDIO_MASK_FM) != 0) {
                   /* FM, MONO only (NICAM & A2 first) */
                   mAudioFormats.add(mAllAudioFormats[0]); 
                   mAudioFormatsMap.add(FM_MONO);
               }
               mAudioFormat = newFormat;
           }
       }           
    private boolean slideOut(View view) {
        if (null == view || view.getVisibility() == View.INVISIBLE) {
            return false;
        } 
                
        
        mTranslateAnimationPanel = null;
        view.setVisibility(View.GONE);
        setControlBarEnable(true);
        
        Animation animation = null;
        if (view == mRenameEditor) {
            animation = new TranslateAnimation(0, 0, 0, view.getHeight());
        } else {
            animation = new TranslateAnimation(0, view.getWidth(), 0, 0);
        }        
        animation.setDuration(ANIMATION_DURATION);
        view.startAnimation(animation);
        
        return true;
    }
    private boolean slideIn(View view) {
        return slideIn(view, 0);
    }
    private boolean slideIn(View view, int offset) {
        if (null == view || view.getVisibility() == View.VISIBLE) {
            return false;
        }
        mTranslateAnimationPanel = view;
        view.setVisibility(View.VISIBLE);
        setControlBarEnable(false);//solve ALPS00132049
        mSurfaceView.requestLayout();    
        
        Animation animation = null;
        if (view == mRenameEditor) {
            animation = new TranslateAnimation(0, 0, view.getHeight(), 0);
        } else {
            animation = new TranslateAnimation(view.getWidth(), 0, 0, 0);
        }
        animation.setDuration(ANIMATION_DURATION);
        animation.setStartOffset(offset);
        view.startAnimation(animation);
        
        return true;
    }
    private boolean fadeOut(View view) {
        if (null == view || view.getVisibility() == View.INVISIBLE) {
            return false;
        } 
        
        mAlphaAnimationPanel = null;
        view.setVisibility(View.INVISIBLE);
        
        Animation animation = null;
        animation = new AlphaAnimation(1F, 0F);
        animation.setDuration(ANIMATION_DURATION);
        view.startAnimation(animation);
        
        return true;
    }
    private boolean fadeIn(View view) {
        return fadeIn(view, 0);
    }
    private boolean fadeIn(View view, int offset) {
        if (null == view || view.getVisibility() == View.VISIBLE) {
            return false;
        }
        mAlphaAnimationPanel = view;
        view.setVisibility(View.VISIBLE);
        
        Animation animation = null;
        animation = new AlphaAnimation(0F, 1F);
        animation.setDuration(ANIMATION_DURATION);
        animation.setStartOffset(offset);
        view.startAnimation(animation);
        
        return true;
    }
}
