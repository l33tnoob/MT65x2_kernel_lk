package com.mediatek.rcse.activities;

import java.util.ArrayList;
import java.util.List;


import android.app.ActionBar;
import android.app.Activity;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;

public class ParticipantsListActivity extends ListActivity implements OnClickListener {
    private ArrayList<String> participantsDisplayName = new ArrayList<String>();
    protected LayoutInflater mInflator = null;
    private View mArea;
    private ListView mBanner;
    private List <ParticipantInfo> participantsInfoList = new ArrayList<ParticipantInfo>();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            setContentView(R.layout.participantslistactivity_layout);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_CUSTOM);
        LayoutInflater inflater = LayoutInflater.from(getApplicationContext());
        ViewGroup view = (ViewGroup) inflater.inflate(R.layout.participantlist_actionbar_layout, null);       
        TextView textView = (TextView) view.findViewById(R.id.text_title);
        textView.setText("Group participants");
        actionBar.setCustomView(view);

        ListView listView = (ListView) findViewById(R.id.list1);
        Button returnButton = (Button) findViewById(R.id.button1);
        returnButton.setOnClickListener(this);
        
        ListArrayAdapter customAdapter = new ListArrayAdapter(); 
        listView.setAdapter(customAdapter);       
        
        Intent intent = getIntent();  
        participantsInfoList = intent.getParcelableArrayListExtra("participantsinfo");
         
    }
    
    
    public class ListArrayAdapter extends BaseAdapter {        
            
            
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            
            View itemView = convertView;
            if(itemView ==null)
            {   LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                itemView = inflater.inflate(R.layout.participantlist, parent, false);                
            }
            bindView(itemView, participantsInfoList.get(position));           
            return itemView;            
        }

        private void bindView(View itemView, ParticipantInfo info) {

            String contact = info.getContact();
            AsyncImageView avatar = (AsyncImageView) itemView
                    .findViewById(R.id.peer_avatar);
            boolean active = User.STATE_CONNECTED.equals(info.getState());
            avatar.setAsyncContact(contact, !active);
            TextView statusView = (TextView) itemView
                    .findViewById(R.id.participant_status);
            statusView.setText(active ? getString(R.string.group_active)
                    : getString(R.string.group_inactive));
            TextView remoteName = (TextView) itemView
                    .findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact));
        }



        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return participantsInfoList.size();
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return participantsInfoList.get(position);
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return 0;
        }
    }


    @Override
    public void onClick(View v) {
        this.finish();
        
    }

}
