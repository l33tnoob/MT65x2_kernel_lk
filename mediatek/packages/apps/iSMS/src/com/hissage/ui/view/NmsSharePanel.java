package com.hissage.ui.view;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;
import com.hissage.R;
import com.hissage.config.NmsCommonUtils;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsFeatureSupport;
import com.hissage.ui.view.NmsLevelControlLayout.OnScrollToScreenListener;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageConsts;
import com.hissage.util.message.MessageUtils;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;

public class NmsSharePanel extends LinearLayout {

    private Handler mHandler;
    private Context mContext;
    private View mConvertView;
    private NmsLevelControlLayout mScrollLayout;
    private LinearLayout mSharePanelMain;
    private RadioButton mDotFirst;
    private RadioButton mDotSec;

    private int mOrientation;
    private int[] mColumnArray;
    private int mScreenIndex;

    public final static int takePhoto = 0;
    public final static int recordVideo = 1;
    public final static int recordAudio = 2;
    public final static int drawSketch = 3;
    public final static int choosePhoto = 4;
    public final static int chooseVideo = 5;
    public final static int chooseAudio = 6;
    public final static int shareLocation = 7;
    public final static int shareContact = 8;
    public final static int shareCalendar = 9;
    public final static int shareReadedBurn = 10;

    public final static String SHARE_ACTION = "shareAction";
    private final static String TAG = "SharePanel";
    
    private boolean isShowSkechPrompt = true ,isShowLocationPrompt = true , isShowReadBurnPrompt = true;
    private String skechString = "", locationString = "" , burnAfterRead = "" ;
    private SharedPreferences   preferences ;
    

    public NmsSharePanel(Context context) {
        super(context);
        mContext = context;
    }

    public NmsSharePanel(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater inflater = LayoutInflater.from(context);
        mConvertView = inflater.inflate(R.layout.share_panel, this, true);
        mContext = context;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mScrollLayout = (NmsLevelControlLayout) mConvertView.findViewById(R.id.share_panel_zone);
        mSharePanelMain = (LinearLayout) mConvertView.findViewById(R.id.share_panel_main);
        mDotFirst = (RadioButton) mConvertView.findViewById(R.id.rb_dot_first);
        mDotSec = (RadioButton) mConvertView.findViewById(R.id.rb_dot_sec);
        resetShareItem();
    }

    public void resetShareItem() {
        mOrientation = getResources().getConfiguration().orientation;
        if (mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
        addSharePage(0);
        addSharePage(1);
        mDotSec.setVisibility(View.VISIBLE);
        mDotFirst.setVisibility(View.VISIBLE);
        mDotFirst.setChecked(true);
        mScrollLayout.setOnScrollToScreen(new OnScrollToScreenListener() {
            @Override
            public void doAction(int whichScreen) {
                mScreenIndex = whichScreen;
                if (whichScreen == 0) {
                    mDotFirst.setChecked(true);
                } else {
                    mDotSec.setChecked(true);
                }
            }
        });
        mScrollLayout.setDefaultScreen(mScreenIndex);
        mScrollLayout.autoRecovery();
    }

    private void addSharePage(int index) {
        mColumnArray = getResources().getIntArray(R.array.share_column);
        
        skechString = getResources().getString(R.string.STR_NMS_DRAW_SKETCH);
        locationString = getResources().getString(R.string.STR_NMS_SHARE_LOCATION);
        burnAfterRead = getResources().getString(R.string.STR_NMS_READED_BURN);

        View v = LayoutInflater.from(mContext)
                .inflate(R.layout.share_flipper, mScrollLayout, false);
        GridView gridView = (GridView) v.findViewById(R.id.gv_share_gridview);
        android.view.ViewGroup.LayoutParams params = mSharePanelMain.getLayoutParams();
        if (mOrientation == Configuration.ORIENTATION_LANDSCAPE) {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_lan_height);
        } else {
            params.height = getResources().getDimensionPixelOffset(R.dimen.share_panel_port_height);
        }
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsFeatureSupport.NMS_MSG_FLAG_ACTIVATE_PROMPT ) != 0){
        try{
            Context otherAppsContext = mContext.createPackageContext("com.android.mms", Context.CONTEXT_IGNORE_SECURITY);
              preferences  = otherAppsContext.getSharedPreferences("isms_activate", Context.MODE_MULTI_PROCESS);
              
            isShowSkechPrompt = preferences.getBoolean("isShowSkechPrompt", true);
            isShowLocationPrompt = preferences.getBoolean("isShowLocationPrompt", true);
            isShowReadBurnPrompt = preferences.getBoolean("isShowReadBurnPrompt", true);
        }   catch(Exception e){
            isShowSkechPrompt = false;
            isShowLocationPrompt = false;
            isShowReadBurnPrompt = false;
        }finally{
            
        }
        
        }else{
            isShowSkechPrompt = false;
            isShowLocationPrompt = false;
            isShowReadBurnPrompt = false; 
        }

        mSharePanelMain.setLayoutParams(params);
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            gridView.setNumColumns(mColumnArray[0]);
        } else {
            gridView.setNumColumns(mColumnArray[1]);
        }
        final ShareAdapter adapter = new ShareAdapter(getLableArray(index), getIconArray(index));
        gridView.setAdapter(adapter);
        gridView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
//                if (!NmsCommonUtils.getSDCardStatus()) {
                
                
                if (false){
                    MessageUtils.createLoseSDCardNotice(mContext, R.string.STR_NMS_CANT_SHARE);
                } else {
                    Message msg = mHandler.obtainMessage();
                    Bundle bundle = new Bundle();
                    bundle.putInt(SHARE_ACTION, getActionId(position));
                    msg.setData(bundle);
                    msg.what = MessageConsts.NMS_SHARE;
                    mHandler.sendMessage(msg);
                }
                
                TextView text = (TextView) view.findViewById(R.id.tv_share_name);
                if(isShowSkechPrompt && text.getText().equals(skechString) ){
                    SharedPreferences.Editor editor = preferences.edit();
                    isShowSkechPrompt = false;
                    editor.putBoolean("isShowSkechPrompt", isShowSkechPrompt);
                    editor.apply();
                    adapter.notifyDataSetChanged();
                }else if(isShowLocationPrompt && text.getText().equals(locationString)){
                    isShowLocationPrompt = false;
                    SharedPreferences.Editor editor = preferences.edit();
                    editor.putBoolean("isShowLocationPrompt", isShowLocationPrompt);
                    editor.apply();
                    adapter.notifyDataSetChanged();
                }else if(isShowReadBurnPrompt && text.getText().equals(burnAfterRead)){
                    isShowReadBurnPrompt = false;
                    SharedPreferences.Editor editor = preferences.edit();
                    editor.putBoolean("isShowReadBurnPrompt", isShowReadBurnPrompt);
                    editor.apply();
                    adapter.notifyDataSetChanged();
                }
            }
              
        });
        mScrollLayout.addView(v);
    }

    private String[] getLableArray(int index) {
        String[] source = null;
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
            source = getResources().getStringArray(R.array.share_string_array);
        }else{
            source = getResources().getStringArray(R.array.share_string_array_no_new_features);
        }
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        if (index == 0) {
            String[] index0 = new String[onePage];
            for (int i = 0; i < onePage; i++) {
                index0[i] = source[i];
            }
            return index0;
        } else {
            int count = source.length - onePage;
            String[] index1 = new String[count];
            for (int i = 0; i < count; i++) {
                index1[i] = source[onePage + i];
            }
            return index1;
        }
    }

    private int[] getIconArray(int index) {
        int[] source = null;
        if ((NmsIpMessageConsts.SWITCHVARIABLE & NmsIpMessageConsts.NmsIpMessageFlag.NMS_MSG_FLAG_BURN_AFTER_READ) != 0) {
            source = MessageConsts.shareIconArr;
        }else{
            source = MessageConsts.shareIconArrNoNewFeatures;
        }
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        if (index == 0) {
            int[] index0 = new int[onePage];
            for (int i = 0; i < onePage; i++) {
                index0[i] = source[i];
            }
            return index0;
        } else {
            int count = source.length - onePage;
            int[] index1 = new int[count];
            for (int i = 0; i < count; i++) {
                index1[i] = source[onePage + i];
            }
            return index1;
        }
    }

    private int getActionId(int position) {
        int onePage;
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
            onePage = mColumnArray[0] * 2;
        } else {
            onePage = mColumnArray[1];
        }
        if (mScreenIndex == 0) {
            return position;
        } else {
            return onePage + position;
        }
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public void recycleView() {
        if (mScrollLayout != null && mScrollLayout.getChildCount() != 0) {
            mScrollLayout.removeAllViews();
        }
    }

    private class ShareAdapter extends BaseAdapter {
        private String[] strArr;
        private int[] iconArr;
        public ShareAdapter(String[] stringArray, int[] iconArray) {
            strArr = stringArray;
            iconArr = iconArray;
        }

        @Override
        public int getCount() {
            int count = 0;
            if (mOrientation == Configuration.ORIENTATION_PORTRAIT) {
                count = mColumnArray[0] * 2;
            } else {
                count = mColumnArray[1];
            }
            return count;
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
                convertView = LayoutInflater.from(mContext).inflate(R.layout.share_grid_item, null);
                convertView.setTag(convertView);
            } else {
                convertView = (View) convertView.getTag();
            }

            TextView text = (TextView) convertView.findViewById(R.id.tv_share_name);
            ImageView img = (ImageView) convertView.findViewById(R.id.iv_share_icon);
            ImageView imgPrompt = (ImageView) convertView.findViewById(R.id.share_item_prompt);
            if (position < strArr.length) {
                text.setText(strArr[position]);
                img.setImageResource(iconArr[position]);

            if(isShowSkechPrompt && strArr[position].equals(skechString)){
                imgPrompt.setVisibility(View.VISIBLE);
            }else if (isShowLocationPrompt && strArr[position].equals(locationString)){
                imgPrompt.setVisibility(View.VISIBLE);
            }else if(isShowReadBurnPrompt && strArr[position].equals(burnAfterRead)){
                imgPrompt.setVisibility(View.VISIBLE);
            }else {
                imgPrompt.setVisibility(View.GONE);
            }
            }
            return convertView;
        }
    }
}
