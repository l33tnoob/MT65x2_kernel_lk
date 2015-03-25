package com.hissage.ui.view;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Configuration;
import android.os.Handler;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.FrameLayout;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;

import com.hissage.R;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsFeatureSupport;
import com.hissage.ui.view.NmsLevelControlLayout.OnScrollToScreenListener;
import com.hissage.util.message.MessageConsts;

public class NmsEmoticonPanel extends LinearLayout implements OnCheckedChangeListener {

    private Handler mHandler;
    private Context mContext;
    private View convertView;
    private NmsLevelControlLayout mScrollLayout;
    private LinearLayout mSharePanelMain;
    private RadioButton mDotFirst;
    private RadioButton mDotSec;
    private RadioButton mDotThird;
    private RadioButton mDotForth;
    private Button mDelEmoticon;
    private RadioButton mNormalTab;
    private RadioButton mLargeTab;
    private RadioButton mDynamicTab;
    private RadioButton mXmTab;
    private RadioButton mAdTab;

    private int mOrientation = 0;
    private int[] mColumnArray;
    private String[] mEmoticonName;
    private String[] mLargeName;
    private String[] mDynamicName;
    private String[] mAdName;
    private String[] mXmName;
    private int mNormalIndex = 0;
    private int mLargeIndex = 0;
    private int mDynamicIndex = 0;
    private int mAdIndex = 0;
    private int mXmIndex = 0;
    private EditEmoticonListener mListener;
    private NmsEmoticonPreview mPreview;
    private boolean isStop = false;
    private ImageView mLargeTabView,mDynamicTabView,mXmTabView,mAdTabView;
    private SharedPreferences preferences;
    private final static String TAG = "EmoticonPanel";

    private Runnable delEmoticon = new Runnable() {

        @Override
        public void run() {
            mListener.doAction(EditEmoticonListener.delEmoticon, "");
        }
    };

    public NmsEmoticonPanel(Context context) {
        super(context);
        mContext = context;
    }

    public NmsEmoticonPanel(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater inflater = LayoutInflater.from(context);
        convertView = inflater.inflate(R.layout.emoticon_panel, this, true);
        mContext = context;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mScrollLayout = (NmsLevelControlLayout) convertView.findViewById(R.id.emoticon_panel_zone);
        mSharePanelMain = (LinearLayout) convertView.findViewById(R.id.ll_emoticon_panel);
        mDotFirst = (RadioButton) convertView.findViewById(R.id.rb_dot_first);
        mDotSec = (RadioButton) convertView.findViewById(R.id.rb_dot_sec);
        mDotThird = (RadioButton) convertView.findViewById(R.id.rb_dot_third);
        mDotForth = (RadioButton) convertView.findViewById(R.id.rb_dot_forth);
        mNormalTab = (RadioButton) convertView.findViewById(R.id.smiley_panel_normal_btn);
        mLargeTab = (RadioButton) convertView.findViewById(R.id.smiley_panel_large_btn);
        mDynamicTab = (RadioButton) convertView.findViewById(R.id.smiley_panel_dynamic_btn);
        mXmTab = (RadioButton) convertView.findViewById(R.id.smiley_panel_xm_btn);
        mAdTab = (RadioButton) convertView.findViewById(R.id.smiley_panel_ad_btn);
        mEmoticonName = getResources().getStringArray(R.array.emoticon_name);
        mLargeName = getResources().getStringArray(R.array.large_emoticon_name);
        mDynamicName = getResources().getStringArray(R.array.dynamic_emoticon_name);
        mAdName = getResources().getStringArray(R.array.ad_emoticon_name);
        mXmName = getResources().getStringArray(R.array.xm_emoticon_name);
        mDelEmoticon = (Button) convertView.findViewById(R.id.smiley_panel_del_btn);
        mDelEmoticon.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    isStop = true;
                    new Thread(new Runnable() {

                        @Override
                        public void run() {
                            try {
                                int i = 0;
                                do {
                                    mHandler.post(delEmoticon);
                                    Thread.sleep((i > 5 ? 100 : 500));
                                    i++;
                                } while (isStop);
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                    }).start();

                } else if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL) {
                    isStop = false;
                }
                return false;
            }
        });
        FrameLayout normalSmileyPanel = (FrameLayout)findViewById(R.id.smiley_panel_normal);
        FrameLayout xmSmileyPanel = (FrameLayout)findViewById(R.id.smiley_panel_xm);
        FrameLayout adSmileyPanel = (FrameLayout)findViewById(R.id.smiley_panel_ad);
        FrameLayout dynamicSmileyPanel = (FrameLayout)findViewById(R.id.smiley_panel_dynamic);
        FrameLayout largeSmileyPanel = (FrameLayout)findViewById(R.id.smiley_panel_large);

        OnClickListener panelClickListener = new LinearLayout.OnClickListener() {
            public void onClick(View v) {
                int clickedId = v.getId();
                if (R.id.smiley_panel_normal == clickedId) {
                    mNormalTab.setChecked(true);
                } else if (R.id.smiley_panel_xm == clickedId) {
                    mXmTab.setChecked(true);
                }else if (R.id.smiley_panel_ad == clickedId) {
                    mAdTab.setChecked(true);
                }else if (R.id.smiley_panel_dynamic == clickedId) {
                    mDynamicTab.setChecked(true);
                }else if (R.id.smiley_panel_large == clickedId) {
                    mLargeTab.setChecked(true);
                }
            }
        };

        normalSmileyPanel.setOnClickListener(panelClickListener);
        xmSmileyPanel.setOnClickListener(panelClickListener);
        adSmileyPanel.setOnClickListener(panelClickListener);
        dynamicSmileyPanel.setOnClickListener(panelClickListener);
        largeSmileyPanel.setOnClickListener(panelClickListener);

        mLargeTabView =(ImageView)findViewById(R.id.smiley_panel_large_prompt);
        mDynamicTabView = (ImageView)findViewById(R.id.smiley_panel_dynamic_prompt);
        mXmTabView = (ImageView)findViewById(R.id.smiley_panel_xm_prompt);
        mAdTabView = (ImageView)findViewById(R.id.smiley_panel_ad_prompt);
        mNormalTab.setOnCheckedChangeListener(this);
        mLargeTab.setOnCheckedChangeListener(this);
        mDynamicTab.setOnCheckedChangeListener(this);
        mXmTab.setOnCheckedChangeListener(this);
        mAdTab.setOnCheckedChangeListener(this);
        mPreview = new NmsEmoticonPreview(mContext, this);
        resetShareItem();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        int checkedId = buttonView.getId();
        if (!isChecked) {
            return;
        }
        ///M.add prompt      {@
        Editor editor = null ;
        if(preferences!=null ){
         editor = preferences.edit();
        }
        if (R.id.smiley_panel_normal_btn == checkedId) {
            mNormalTab.setChecked(true);
            mLargeTab.setChecked(false);
            mDynamicTab.setChecked(false);
            mXmTab.setChecked(false);
            mAdTab.setChecked(false);
            addNormalPanel();
        } else if (R.id.smiley_panel_large_btn == checkedId) {
            mNormalTab.setChecked(false);
            mLargeTab.setChecked(true);
    
            if(mLargeTabView!=null&&preferences!=null ){
                mLargeTabView.setVisibility(View.GONE);   
                editor.putBoolean("mLargeTabView", false);
            }
            mDynamicTab.setChecked(false);
            mXmTab.setChecked(false);
            mAdTab.setChecked(false);
            addLargePanel();
        } else if (R.id.smiley_panel_dynamic_btn == checkedId) {
            mNormalTab.setChecked(false);
            mLargeTab.setChecked(false);
            mDynamicTab.setChecked(true);
            if(mLargeTabView!=null&&preferences!=null ){
                mDynamicTabView.setVisibility(View.GONE);  
                editor.putBoolean("mDynamicTabView", false);
            }
            mXmTab.setChecked(false);
            mAdTab.setChecked(false);
            addDynamicPanel();
        } else if (R.id.smiley_panel_xm_btn == checkedId) {
            mNormalTab.setChecked(false);
            mLargeTab.setChecked(false);
            mDynamicTab.setChecked(false);
            mXmTab.setChecked(true);
        
            if(mLargeTabView!=null&&preferences!=null ){
                mXmTabView.setVisibility(View.GONE);
                editor.putBoolean("mXmTabView", false);
            }
  
            mAdTab.setChecked(false);
            addXmPanel();
        } else if (R.id.smiley_panel_ad_btn == checkedId) {
            mNormalTab.setChecked(false);
            mLargeTab.setChecked(false);
            mDynamicTab.setChecked(false);
            mXmTab.setChecked(false);
            mAdTab.setChecked(true);
            if(mLargeTabView!=null&&preferences!=null ){
                mAdTabView.setVisibility(View.GONE);
                editor.putBoolean("mAdTabView", false);
            }
            
            addAdPanel();
        }
        if(editor!=null ){
            editor.apply();
           }
        ///@}
    }

    /**
     * Build share item.
     */
    public void resetShareItem() {
        if (mNormalTab.isChecked()) {
            addNormalPanel();
        } else if (mLargeTab.isChecked()) {
            addLargePanel();
        } else if (mDynamicTab.isChecked()) {
            addDynamicPanel();
        } else if (mAdTab.isChecked()) {
            addAdPanel();
        } else if (mXmTab.isChecked()) {
            addXmPanel();
        }
        mScrollLayout.autoRecovery();

        ///M.add prompt      {@
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsFeatureSupport.NMS_MSG_FLAG_ACTIVATE_PROMPT ) != 0){
        if(preferences == null){
            try{
                Context otherAppsContext = mContext.createPackageContext("com.android.mms", Context.CONTEXT_IGNORE_SECURITY);
                preferences = otherAppsContext.getSharedPreferences("isms_activate", Context.MODE_WORLD_READABLE);
            }catch(Exception e){
  
            }finally{
                if(preferences ==null){
                    preferences = mContext.getSharedPreferences ("isms_activate", Context.MODE_MULTI_PROCESS);
                }
            }
        }
        if(mLargeTabView!=null&&preferences.getBoolean("mLargeTabView", true)){
            mLargeTabView.setVisibility(View.VISIBLE);
        }
        if(mDynamicTabView!=null&&preferences.getBoolean("mDynamicTabView", true)){
            mDynamicTabView.setVisibility(View.VISIBLE);
        }
        if(mXmTabView!=null&&preferences.getBoolean("mXmTabView", true)){
            mXmTabView.setVisibility(View.VISIBLE);
        }
        if(mAdTabView!=null&&preferences.getBoolean("mAdTabView", true)){
            mAdTabView.setVisibility(View.VISIBLE);
        }
        }
        ///@}
    }

    /**
     * Display normal emoticon page.
     */
    private void addNormalPanel() {
        mColumnArray = getResources().getIntArray(R.array.emoticon_column);
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        mScrollLayout.setDefaultScreen(mNormalIndex);
        mOrientation = getResources().getConfiguration().orientation;
        mDotFirst.setVisibility(View.VISIBLE);
        mDotSec.setVisibility(View.VISIBLE);
        mDotThird.setVisibility(View.VISIBLE);
        mDotForth.setVisibility(View.VISIBLE);
        int num = calculateNormalPageCount(mOrientation);
        for (int i = 0; i < num; i++) {
            addNormalPage(i);
        }
        mScrollLayout.setToScreen(mNormalIndex);
        mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
            @Override
            public void doAction(int whichScreen) {
                mNormalIndex = whichScreen;
                if (whichScreen == 0) {
                    mDotFirst.setChecked(true);
                } else if (whichScreen == 1) {
                    mDotSec.setChecked(true);
                } else if (whichScreen == 2) {
                    mDotThird.setChecked(true);
                } else {
                    mDotForth.setChecked(true);
                }
            }
        });
    }

    /**
     * Display large emoticon page.
     */
    private void addLargePanel() {
        mColumnArray = getResources().getIntArray(R.array.share_column);
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            int num = calculateLargePageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addLargePage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.GONE);
            if (mLargeIndex == 3) {
                mLargeIndex = 2;
            }
            mScrollLayout.setToScreen(mLargeIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mLargeIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        } else {
            int num = calculateLargePageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addLargePage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.VISIBLE);
            mDotFirst.setChecked(true);
            mScrollLayout.setToScreen(mLargeIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mLargeIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        }
        mScrollLayout.setDefaultScreen(mLargeIndex);
        if (mLargeIndex == 0) {
            mDotFirst.setChecked(true);
        } else if (mLargeIndex == 1) {
            mDotSec.setChecked(true);
        } else if (mLargeIndex == 2) {
            mDotThird.setChecked(true);
        } else {
            mDotForth.setChecked(true);
        }
    }

    /**
     * Display animation emoticon page.
     */
    private void addDynamicPanel() {
        mColumnArray = getResources().getIntArray(R.array.share_column);
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addDynamicPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.GONE);
            if (mDynamicIndex == 3) {
                mDynamicIndex = 2;
            }
            mScrollLayout.setToScreen(mDynamicIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mDynamicIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        } else {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addDynamicPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.VISIBLE);
            mScrollLayout.setToScreen(mDynamicIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mDynamicIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        }
        mScrollLayout.setDefaultScreen(mDynamicIndex);
        if (mDynamicIndex == 0) {
            mDotFirst.setChecked(true);
        } else if (mDynamicIndex == 1) {
            mDotSec.setChecked(true);
        } else if (mDynamicIndex == 2) {
            mDotThird.setChecked(true);
        } else {
            mDotForth.setChecked(true);
        }
    }

    private void addAdPanel() {
        mColumnArray = getResources().getIntArray(R.array.share_column);
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addAdPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.GONE);
            if (mAdIndex == 3) {
                mAdIndex = 2;
            }
            mScrollLayout.setToScreen(mAdIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mAdIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        } else {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addAdPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.VISIBLE);
            mScrollLayout.setToScreen(mAdIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mAdIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        }
        mScrollLayout.setDefaultScreen(mAdIndex);
        if (mAdIndex == 0) {
            mDotFirst.setChecked(true);
        } else if (mAdIndex == 1) {
            mDotSec.setChecked(true);
        } else if (mAdIndex == 2) {
            mDotThird.setChecked(true);
        } else {
            mDotForth.setChecked(true);
        }
    }

    private void addXmPanel() {
        mColumnArray = getResources().getIntArray(R.array.share_column);
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addXmPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.GONE);
            if (mXmIndex == 3) {
                mXmIndex = 2;
            }
            mScrollLayout.setToScreen(mXmIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mXmIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        } else {
            int num = calculateDynamicPageCount(mOrientation);
            for (int i = 0; i < num; i++) {
                addXmPage(i);
            }
            mDotSec.setVisibility(View.VISIBLE);
            mDotFirst.setVisibility(View.VISIBLE);
            mDotThird.setVisibility(View.VISIBLE);
            mDotForth.setVisibility(View.VISIBLE);
            mScrollLayout.setToScreen(mXmIndex);
            mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
                @Override
                public void doAction(int whichScreen) {
                    mXmIndex = whichScreen;
                    if (whichScreen == 0) {
                        mDotFirst.setChecked(true);
                    } else if (whichScreen == 1) {
                        mDotSec.setChecked(true);
                    } else if (whichScreen == 2) {
                        mDotThird.setChecked(true);
                    } else {
                        mDotForth.setChecked(true);
                    }
                }
            });
        }
        mScrollLayout.setDefaultScreen(mAdIndex);
        if (mXmIndex == 0) {
            mDotFirst.setChecked(true);
        } else if (mXmIndex == 1) {
            mDotSec.setChecked(true);
        } else if (mXmIndex == 2) {
            mDotThird.setChecked(true);
        } else {
            mDotForth.setChecked(true);
        }
    }

    /**
     * Return the count of normal emoticon page.
     * 
     * @param orientation
     *            The screen orientation.
     * @return The count of normal emoticon page.
     */
    private int calculateNormalPageCount(int orientation) {
        int onePage;
        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 4;
        } else {
            onePage = mColumnArray[1] * 2;
        }
        int total = MessageConsts.emoticonIdList.length;
        int count = total / onePage;
        if (total > count * onePage) {
            count++;
        }
        return count;
    }

    /**
     * Return the count of large emoticon page.
     * 
     * @param orientation
     *            The screen orientation.
     * 
     * @return The count of large emoticon page.
     */
    private int calculateLargePageCount(int orientation) {
        int onePage;
        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int total = MessageConsts.largeIconArr.length;
        int count = total / onePage;
        if (total > count * onePage) {
            count++;
        }
        return count;
    }

    /**
     * Return count of animation emoticon page.
     * 
     * @param orientation
     *            The screen orientation
     * @return The count of animation emoticon page.
     */
    private int calculateDynamicPageCount(int orientation) {
        int onePage;
        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int total = MessageConsts.dynamicIconArr.length;
        int count = total / onePage;
        if (total > count * onePage) {
            count++;
        }
        return count;
    }

    /**
     * Create the child of normal emoticon page, and add it to parent.
     * 
     * @param index
     *            the index of child.
     */
    private void addNormalPage(int index) {
        View v = LayoutInflater.from(mContext).inflate(R.layout.normal_emoticon_flipper,
                mScrollLayout, false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_normal_emoticon_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        EmoticonAdapter adapter = new EmoticonAdapter(getNormalIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String name = getEmoticonName(position);
                if (TextUtils.isEmpty(name)) {
                    return;
                }
                mListener.doAction(EditEmoticonListener.addEmoticon, name);
            }
        });
        mScrollLayout.addView(v);
    }

    /**
     * Create the child of large emoticon page, and add it to parent.
     * 
     * @param index
     *            the index of child.
     */
    private void addLargePage(int index) {
        View v = LayoutInflater.from(mContext).inflate(R.layout.large_emoticon_flipper,
                mScrollLayout, false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_large_emoticon_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        LargeAdapter adapter = new LargeAdapter(getLargeIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!mPreview.isShow()) {
                    String name = getLargeName(position);
                    mListener.doAction(EditEmoticonListener.sendEmoticon, name);
                }
            }
        });
        mScrollLayout.addView(v);
    }

    /**
     * Create the child of animation emoticon page, and add it to parent.
     * 
     * @param index
     *            the index of child.
     */
    private void addDynamicPage(int index) {
        View v = LayoutInflater.from(mContext).inflate(R.layout.dynamic_emoticon_flipper,
                mScrollLayout, false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_dynamic_emoticon_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        LargeAdapter adapter = new LargeAdapter(getDynamicIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!mPreview.isShow()) {
                    String name = getLargeName(position);
                    mListener.doAction(EditEmoticonListener.sendEmoticon, name);
                }
            }
        });
        gridView.setOnItemLongClickListener(new OnItemLongClickListener() {

            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                int resId = getPreviewIcon(position);
                if (resId == 0) {
                    return false;
                }
                mPreview.setEmoticon(resId);
                mPreview.showWindow();
                return true;
            }
        });

        gridView.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_CANCEL) {
                    mPreview.dissWindow();
                    return false;
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    mPreview.dissWindow();
                    return false;
                } else {
                    return false;
                }
            }
        });
        mScrollLayout.addView(v);
    }

    private void addAdPage(int index) {
        View v = LayoutInflater.from(mContext).inflate(R.layout.ad_emoticon_flipper, mScrollLayout,
                false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_ad_emoticon_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        LargeAdapter adapter = new LargeAdapter(getAdIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!mPreview.isShow()) {
                    String name = getLargeName(position);
                    mListener.doAction(EditEmoticonListener.sendEmoticon, name);
                }
            }
        });
        gridView.setOnItemLongClickListener(new OnItemLongClickListener() {

            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                int resId = getPreviewIcon(position);
                if (resId == 0) {
                    return false;
                }
                mPreview.setEmoticon(resId);
                mPreview.showWindow();
                return true;
            }
        });

        gridView.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_CANCEL) {
                    mPreview.dissWindow();
                    return false;
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    mPreview.dissWindow();
                    return false;
                } else {
                    return false;
                }
            }
        });
        mScrollLayout.addView(v);
    }

    private void addXmPage(int index) {
        View v = LayoutInflater.from(mContext).inflate(R.layout.xm_emoticon_flipper, mScrollLayout,
                false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_xm_emoticon_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        LargeAdapter adapter = new LargeAdapter(getXmIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (!mPreview.isShow()) {
                    String name = getLargeName(position);
                    mListener.doAction(EditEmoticonListener.sendEmoticon, name);
                }
            }
        });
        gridView.setOnItemLongClickListener(new OnItemLongClickListener() {

            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                int resId = getPreviewIcon(position);
                if (resId == 0) {
                    return false;
                }
                mPreview.setEmoticon(resId);
                mPreview.showWindow();
                return true;
            }
        });

        gridView.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_CANCEL) {
                    mPreview.dissWindow();
                    return false;
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    mPreview.dissWindow();
                    return false;
                } else {
                    return false;
                }
            }
        });
        mScrollLayout.addView(v);
    }

    /**
     * Return the resource id array of the normal emoticon page at index.
     * 
     * @param index
     *            the visiable page index of normal emoticon page.
     * @return the resource id array.
     */
    private int[] getNormalIconArray(int index) {
        int[] source = MessageConsts.emoticonIdList;
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 4;
        } else {
            onePage = mColumnArray[1] * 2;
        }
        int[] arr = new int[onePage];
        for (int i = 0; i < onePage; i++) {
            int index1 = index * onePage + i;
            if (index1 >= source.length) {
                break;
            }
            arr[i] = source[index * onePage + i];
        }
        return arr;
    }

    /**
     * Return the resource id array of the large emoticon page at index.
     * 
     * @param index
     *            the visiable page index of large emoticon page.
     * @return the resource id array.
     */
    private int[] getLargeIconArray(int index) {
        int[] source = MessageConsts.largeIconArr;
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int[] arr = new int[onePage];
        for (int i = 0; i < onePage; i++) {
            int index1 = index * onePage + i;
            if (index1 >= source.length) {
                break;
            }
            arr[i] = source[index * onePage + i];
        }
        return arr;
    }

    /**
     * Return the resource id array of the animation emoticon page at index.
     * 
     * @param index
     *            the visiable page index of animation emoticon page.
     * @return the resource id array.
     */
    private int[] getDynamicIconArray(int index) {
        int[] source = MessageConsts.dynamicPngIconArr;
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int[] arr = new int[onePage];
        for (int i = 0; i < onePage; i++) {
            int index1 = index * onePage + i;
            if (index1 >= source.length) {
                break;
            }
            arr[i] = source[index * onePage + i];
        }
        return arr;
    }

    private int[] getAdIconArray(int index) {
        int[] source = MessageConsts.adPngIconArr;
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int[] arr = new int[onePage];
        for (int i = 0; i < onePage; i++) {
            int index1 = index * onePage + i;
            if (index1 >= source.length) {
                break;
            }
            arr[i] = source[index * onePage + i];
        }
        return arr;
    }

    private int[] getXmIconArray(int index) {
        int[] source = MessageConsts.xmPngIconArr;
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        int[] arr = new int[onePage];
        for (int i = 0; i < onePage; i++) {
            int index1 = index * onePage + i;
            if (index1 >= source.length) {
                break;
            }
            arr[i] = source[index * onePage + i];
        }
        return arr;
    }

    /**
     * Return the coding of the normal emoticon.
     * 
     * @param position
     *            the position of gridview.
     * @return the coding of the normal emoticon.
     */
    private String getEmoticonName(int position) {
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 4;
        } else {
            onePage = mColumnArray[1] * 2;
        }
        if (position >= 20) {
            return null;
        }
        int index = position + onePage * mNormalIndex;
        if (index >= mEmoticonName.length) {
            return null;
        }
        return mEmoticonName[index];
    }

    /**
     * Return the coding of the large emoticon and animation emoticon.
     * 
     * @param position
     *            the position of gridview.
     * @return the coding of the large emoticon and animation emoticon.
     */
    private String getLargeName(int position) {
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        if (mLargeTab.isChecked()) {
            if (position >= onePage) {
                return "";
            }
            return mLargeName[onePage * mLargeIndex + position];
        } else if (mDynamicTab.isChecked()) {
            if (position >= onePage) {
                return "";
            }
            return mDynamicName[onePage * mDynamicIndex + position];
        } else if (mAdTab.isChecked()) {
            if (position >= onePage) {
                return "";
            }
            return mAdName[onePage * mAdIndex + position];
        } else if (mXmTab.isChecked()) {
            if (position >= onePage) {
                return "";
            }
            return mXmName[onePage * mXmIndex + position];
        } else {
            return "";
        }
    }

    /**
     * Return the resource id of the animation emoticon at position.
     * 
     * @param position
     *            the position of gridview.
     * @return The resource id.
     */
    private int getPreviewIcon(int position) {
        if (mLargeTab.isChecked()) {
            return 0;
        }
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }

        if (position >= onePage) {
            return 0;
        }
        if (mDynamicTab.isChecked()) {
            return MessageConsts.dynamicIconArr[onePage * mDynamicIndex + position];
        } else if (mAdTab.isChecked()) {
            return MessageConsts.adIconArr[onePage * mAdIndex + position];
        } else if (mXmTab.isChecked()) {
            return MessageConsts.xmIconArr[onePage * mXmIndex + position];
        } else {
            return 0;
        }
    }

    /**
     * Sets the handler.
     * 
     * @param handler
     *            the new handler
     */
    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public void recycleView() {
        if (mScrollLayout != null && mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
    }

    private class EmoticonAdapter extends BaseAdapter {

        private int[] iconArr;

        public EmoticonAdapter(int[] iconArray) {
            iconArr = iconArray;
        }

        @Override
        public int getCount() {
            return iconArr.length;
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
                convertView = LayoutInflater.from(mContext).inflate(R.layout.emoticon_grid_item,
                        null);
                convertView.setTag(convertView);
            } else {
                convertView = (View) convertView.getTag();
            }

            ImageView ivPre = (ImageView) convertView.findViewById(R.id.iv_emoticon_icon);

            ivPre.setImageResource(iconArr[position]);
            return convertView;
        }
    }

    private class LargeAdapter extends BaseAdapter {

        private int[] iconArr;

        public LargeAdapter(int[] iconArray) {
            iconArr = iconArray;
        }

        @Override
        public int getCount() {
            return iconArr.length;
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
                if (mLargeTab.isChecked()) {
                    convertView = LayoutInflater.from(mContext).inflate(
                            R.layout.large_emoticon_grid_item, null);
                } else if (mDynamicTab.isChecked()) {
                    convertView = LayoutInflater.from(mContext).inflate(
                            R.layout.dynamic_emoticon_grid_item, null);
                } else if (mAdTab.isChecked()) {
                    convertView = LayoutInflater.from(mContext).inflate(
                            R.layout.ad_emoticon_grid_item, null);
                } else {
                    convertView = LayoutInflater.from(mContext).inflate(
                            R.layout.xm_emoticon_grid_item, null);
                }
                convertView.setTag(convertView);
            } else {
                convertView = (View) convertView.getTag();
            }
            final ImageView img = (ImageView) convertView.findViewById(R.id.iv_large_emoticon_icon);
            img.setImageResource(iconArr[position]);

            return convertView;
        }
    }

    public void setEditEmoticonListener(EditEmoticonListener l) {
        mListener = l;
    }

    /**
     * The listener interface for receiving editEmoticon events. The class that
     * is interested in processing a editEmoticon event implements this
     * interface, and the object created with that class is registered with a
     * component using the component's
     * <code>setEditEmoticonListener<code> method. When
     * the addEmotion, delEmoticon and sendEmoticon event occurs, that object's appropriate
     * method is invoked.
     * 
     * @see EditEmoticonEvent
     */
    public interface EditEmoticonListener {

        public final static int addEmoticon = 0;
        public final static int delEmoticon = 1;
        public final static int sendEmoticon = 2;

        /**
         * Do edit emoticon action.
         * 
         * @param type
         *            action type
         * @param emotionName
         *            the coding of emoticon
         */
        public void doAction(int type, String emotionName);
    }
}
