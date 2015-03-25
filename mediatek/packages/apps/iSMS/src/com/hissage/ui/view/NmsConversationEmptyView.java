package com.hissage.ui.view;

import com.hissage.R;
import com.hissage.api.NmsStartActivityApi;
//M:Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class NmsConversationEmptyView extends LinearLayout {
    
    private Context mContext;
    private View convertView;
    private TextView tvContent;
    private LinearLayout llActivate;
    private LinearLayout llImportant;
    private RelativeLayout llSpam;
    private RelativeLayout llGroupChat;
    private Button btnActivate;
    
    public NmsConversationEmptyView(Context context) {
        super(context);
    }

    public NmsConversationEmptyView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
        LayoutInflater inflater = LayoutInflater.from(context);
        convertView = inflater.inflate(R.layout.conversation_empty, this, true);
        
        tvContent = (TextView)convertView.findViewById(R.id.tv_empty_content);
        llImportant = (LinearLayout)convertView.findViewById(R.id.ll_empty_important);
        llSpam = (RelativeLayout)convertView.findViewById(R.id.ll_empty_spam);
        llGroupChat = (RelativeLayout)convertView.findViewById(R.id.ll_empty_groupchat);
        llActivate = (LinearLayout)convertView.findViewById(R.id.ll_empty_activate);
        btnActivate = (Button)convertView.findViewById(R.id.btn_activate);
        btnActivate.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
				//M:Activation Statistics
                NmsStartActivityApi.nmsStartActivitionActivity(mContext, NmsPlatformAdapter.getInstance(mContext).getCurrentSimId(), NmsIpMessageConsts.NmsUIActivateType.OTHER);
            }});
    }
    
    
    public void setSpamEmpty(boolean isActivate){
        llSpam.setVisibility(View.VISIBLE);
        tvContent.setText(R.string.STR_NMS_SPAM_EMPTY);
        setActivate(isActivate);

    }
    
    public void setImportantEmpty(boolean isActivate){
        llImportant.setVisibility(View.VISIBLE);
        tvContent.setText(R.string.STR_NMS_IMPORTANT_EMPTY);
        setActivate(isActivate);
    }
    public void setGroupChatEmpty(boolean isActivate){
        llGroupChat.setVisibility(View.VISIBLE);
        tvContent.setText(R.string.STR_NMS_GROUPCHAT_EMPTY);
        setActivate(isActivate);
    }
    
    public void setAllChatEmpty(){
        tvContent.setGravity(Gravity.CENTER_HORIZONTAL);
        tvContent.setText(R.string.STR_NMS_ALLCHAT_EMPTY);
    }
    
    private void setActivate(boolean isActivate){
        if(isActivate){
            llActivate.setVisibility(View.GONE);
        }else{
            llActivate.setVisibility(View.VISIBLE);
        }
    }
}
