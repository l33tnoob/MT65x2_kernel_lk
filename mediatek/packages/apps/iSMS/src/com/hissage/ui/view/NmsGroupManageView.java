package com.hissage.ui.view;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.Editable;
import android.text.Selection;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.imagecache.NmsContactAvatarCache;
import com.hissage.imageworker.NmsContactAvatarWorker;
import com.hissage.jni.engineadapter;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsCreateSmsThread;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.ui.activity.NmsChatDetailsActivity;
import com.hissage.ui.activity.NmsProfileSettingsActivity;
import com.hissage.ui.activity.NmsQuickContactActivity;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;

public class NmsGroupManageView extends RelativeLayout {

    public String Tag = "NmsGroupManageView";

    private static int CREATE_INVITE_MEMBER = 0;
    private static int LEAVE_GROUP_CHAT = 0;

    private static short GROUP_INVITE_MORE_ID = -9;
    private static int INVITE_GROUP_CHAT = 0;
    public enum GroupChatMode {
        VIEW, CREATE, EDIT
    }

    private Context mContext;
    private TextView mTvGroupTitle;
    private TextView mTvMemberTitle;
    private TextView mTvGroupNum;
    private Button btnChat;
    private EditText etGroupName;
    private TextView tvGroupName;

    private RelativeLayout rlGroupInfo;

    private GridView gridview;
    private GroupChatAdapter adapter;

    private OnGroupChatClickListener mListener;
    private GroupChatMode groupChatMode;

    private NmsGroupChatContact groupContact;
    private short groupId;

    private ArrayList<NmsContact> memberList = new ArrayList<NmsContact>();

    public NmsGroupManageView(Context context) {
        super(context);
        mContext = context;
    }

    public NmsGroupManageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;

        View convertView = LayoutInflater.from(context).inflate(R.layout.group_chat_manage, this,
                true);
        mTvGroupTitle = (TextView) convertView.findViewById(R.id.tv_group);
        mTvMemberTitle = (TextView) convertView.findViewById(R.id.tv_members);
        mTvGroupNum = (TextView) convertView.findViewById(R.id.tv_number);
        etGroupName = (EditText) convertView.findViewById(R.id.et_groupName);
        tvGroupName = (TextView) convertView.findViewById(R.id.tv_group_name);
        btnChat = (Button) convertView.findViewById(R.id.ib_chat);
        rlGroupInfo = (RelativeLayout) convertView.findViewById(R.id.rl_group_info);
        gridview = (GridView) convertView.findViewById(R.id.gv_members);
        gridview.setSelector(new ColorDrawable(Color.TRANSPARENT));
        adapter = new GroupChatAdapter(context);
    }

    public void setGroupChatMode(GroupChatMode mode) {
        groupChatMode = mode;
        notifyGroupChatChange();
    }
    
    public void startChatSettings(){
        long threadId = NmsCreateSmsThread.getOrCreateThreadId(mContext, groupContact.getNumber());
        NmsStartActivityApi.nmsStartChatSettingsActivity(mContext, threadId);
    }

    public void editGroupChat() {
        groupChatMode = GroupChatMode.EDIT;

        mTvGroupTitle.setText(R.string.STR_NMS_GROUP_NAME);
        rlGroupInfo.setVisibility(View.GONE);
        etGroupName.setVisibility(View.VISIBLE);
        mTvMemberTitle.setText(R.string.STR_NMS_GROUP_MEMBERS);
        mTvGroupNum.setVisibility(View.GONE);

        buildEditGroupMembers();
        buildExtraMembers();

        showKeyborad();

        Editable etext = etGroupName.getText();
        int position = etext.length();
        Selection.setSelection(etext, position);

        notifyGroupChatMemberChange();
    }

    public void createGroupChat() {
        groupChatMode = GroupChatMode.CREATE;

        mTvGroupTitle.setText(R.string.STR_NMS_GROUP_NAME);
        rlGroupInfo.setVisibility(View.GONE);
        etGroupName.setVisibility(View.VISIBLE);
        mTvMemberTitle.setText(R.string.STR_NMS_GROUP_MEMBERS);
        mTvGroupNum.setVisibility(View.GONE);
        buildExtraMembers();

        notifyGroupChatMemberChange();
    }

    public void setGroupChatId(short groupId) {
        this.groupId = groupId;
    }

    public int getGroupChatSimId() {
        return groupContact.getSimId();
    }

    public void setContactIdList(String[] contactId, int simId) {
        memberList.clear();

        NmsContact selfPerson = new NmsContact();
        selfPerson.setName(this.getResources().getText(R.string.STR_NMS_GROUP_MEMEBER_YOU)
                .toString());
        selfPerson.setId(NmsContactApi.getInstance(mContext).getMyselfEngineContactIdViaSimId(simId));

        memberList.add(selfPerson);

        if (contactId != null) {
            for (String id : contactId) {
                NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(Short
                        .valueOf(id));
                if (contact != null) {
                    memberList.add(contact);
                } else {
                    NmsLog.error(Tag, "can't get contact info. engineContactId:" + id);
                }
            }
        }
    }

    public void selectGroupMembers(String[] contactId) {
        List<String> members = new ArrayList<String>();
        if (contactId != null) {
            NmsLog.trace(Tag, "Add group members to group chat, the groupId:" + groupId);
            if (groupId > 0) {
                for (String id : contactId) {
                    short engineContactId = Short.valueOf(id);
                    NmsContact contact = NmsIpMessageApiNative
                            .nmsGetContactInfoViaEngineId(engineContactId);
                    if (contact != null) {
                        members.add(contact.getNumber());
                    } else {
                        NmsLog.error(Tag, "can't get contact info. engineContactId:" + id);
                    }
                }

                boolean ret = NmsIpMessageApiNative.nmsAddMembersToGroup(groupId, members);
                if (ret) {
                    for (String id : contactId) {
                        short engineContactId = Short.valueOf(id);
                        NmsContact contact = NmsIpMessageApiNative
                                .nmsGetContactInfoViaEngineId(engineContactId);
                        if (contact != null) {
                            memberList.add(contact);
                        } else {
                            NmsLog.error(Tag, "can't get contact info. engineContactId:" + id);
                        }
                    }
                    Toast.makeText(mContext, R.string.STR_NMS_ADD_GROUP_MEMBER_SUCCESS,
                            Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(mContext, R.string.STR_NMS_ADD_GROUP_MEMBER_FAILED,
                            Toast.LENGTH_SHORT).show();
                }
            } else {
                for (String id : contactId) {
                    short engineContactId = Short.valueOf(id);
                    NmsContact contact = NmsIpMessageApiNative
                            .nmsGetContactInfoViaEngineId(engineContactId);
                    if (contact != null) {
                        memberList.add(contact);
                    } else {
                        NmsLog.error(Tag, "can't get contact info. engineContactId:" + id);
                    }
                }
            }
        }

        if (GroupChatMode.VIEW == groupChatMode) {
            buildViewGroupMembers();
        }
    }

    public void viewGroupMembers(short[] contactId) {
        memberList.clear();

        NmsContact selfPerson = new NmsContact();
        selfPerson.setName(this.getResources().getText(R.string.STR_NMS_GROUP_MEMEBER_YOU)
                .toString());
        selfPerson.setId(NmsContactApi.getInstance(mContext).getMyselfEngineContactIdViaSimId(
                (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId()));

        memberList.add(selfPerson);

        if (contactId != null) {
            for (short id : contactId) {
                NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(id);
                if (contact != null) {
                    memberList.add(contact);
                } else {
                    NmsLog.error(Tag, "can't get contact info. engineContactId:" + id);
                }
            }
        }
    }

    public void notifyGroupChatMemberChange() {
		if (GroupChatMode.EDIT == groupChatMode
				|| GroupChatMode.CREATE == groupChatMode) {
			if (memberList.size() > INVITE_GROUP_CHAT + 1) {
				memberList.remove(INVITE_GROUP_CHAT);
				buildExtraMembers();
			} else if (memberList.size() < INVITE_GROUP_CHAT+1) {
				INVITE_GROUP_CHAT--;
			}
		}

		gridview.setAdapter(adapter);
		gridview.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> arg0, View arg1,
					int position, long arg3) {
				if (GroupChatMode.VIEW == groupChatMode) {
					if (position == 0) {
					    long gropSimId= getGroupChatSimId() ;
					    int slotid = NmsPlatformAdapter.getInstance(mContext).getSlotIdBySimId(gropSimId) ;
						Intent intent = new Intent(mContext,NmsProfileSettingsActivity.class);
						intent.putExtra(NmsConsts.SIM_ID,slotid ) ;
						mContext.startActivity(intent);
						return;
					}
					Intent intent = new Intent(mContext,NmsQuickContactActivity.class);
					intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
					NmsContact c = memberList.get(position);
					intent.putExtra("contactId", c.getId());
					intent.putExtra("PHONE_NUM", c.getNumber());
					mContext.startActivity(intent);
				}
			}

		});
    }


    protected void notifyGroupChatChange() {
        if (GroupChatMode.VIEW == groupChatMode) {
            mTvGroupTitle.setText(R.string.STR_NMS_GROUP_INFO);
            rlGroupInfo.setVisibility(View.VISIBLE);
            etGroupName.setVisibility(View.GONE);
            mTvMemberTitle.setText(R.string.STR_NMS_GROUP_MEMBERS);
            mTvGroupNum.setVisibility(View.VISIBLE);

            buildViewGroupMembers();

            hideKeyborad();

        } else if (GroupChatMode.EDIT == groupChatMode) {
            mTvGroupTitle.setText(R.string.STR_NMS_GROUP_NAME);
            rlGroupInfo.setVisibility(View.GONE);
            etGroupName.setVisibility(View.VISIBLE);
            mTvMemberTitle.setText(R.string.STR_NMS_GROUP_MEMBERS);
            mTvGroupNum.setVisibility(View.GONE);

            buildEditGroupMembers();
            buildExtraMembers();

            showKeyborad();

            Editable etext = etGroupName.getText();
            int position = etext.length();
            Selection.setSelection(etext, position);
        } else {
            NmsLog.trace(Tag, "GroupChatMode is:" + GroupChatMode.CREATE);
        }

        notifyGroupChatMemberChange();
    }

    private void buildEditGroupMembers() {
        if (groupContact == null) {
            groupContact = NmsIpMessageApiNative.nmsGetGroupInfoViaGroupId(groupId);
        }

        short[] members = groupContact.getMemberIds();
        viewGroupMembers(members);
        etGroupName.setText(groupContact.getName());
    }

    private void buildViewGroupMembers() {
        groupContact = NmsIpMessageApiNative.nmsGetGroupInfoViaGroupId(groupId);
        
        short[] members = groupContact.getMemberIds();
        
        viewGroupMembers(members);
        tvGroupName.setText(groupContact.getName());
        mTvGroupNum.setText(String.valueOf(groupContact.getMemberCount()));
        btnChat.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                chat();
            }
        });
    }

    private void chat() {
        Intent intent = new Intent(mContext, NmsChatDetailsActivity.class);
        intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, groupId);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        ((Activity) mContext).finish();
        mContext.startActivity(intent);
    }

    private void buildExtraMembers() {
        NmsContact invitePerson = new NmsContact();
        invitePerson.setName(this.getResources().getText(R.string.STR_NMS_GROUP_INVITE_MORE)
                .toString());
        invitePerson.setId(GROUP_INVITE_MORE_ID);
        INVITE_GROUP_CHAT = memberList.size();
        memberList.add(invitePerson);
    }

    public class GroupChatAdapter extends BaseAdapter {
        private Context mContext;
        private NmsContactAvatarWorker mImageWorker;

        public GroupChatAdapter(Context c) {
            mContext = c;
            mImageWorker = new NmsContactAvatarWorker(mContext, R.drawable.ic_contact_picture,
                    NmsContactAvatarCache.getInstance());
        }

        private class ViewHolder {
            ImageView imageUser;
            ImageButton imageDelete;
            TextView tvName;
            ImageButton imageAddUser;
            TextView tvInviteMore;
        };

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return memberList.size();
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

        @Override
        public View getView(final int position, View convertView, ViewGroup parent) {
            // TODO Auto-generated method stub
            ViewHolder viewholder = null;
            if (convertView == null) {
                viewholder = new ViewHolder();

                final LayoutInflater inflater = (LayoutInflater) mContext
                        .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                convertView = inflater.inflate(R.layout.group_contact_item, null);

                viewholder.imageUser = (ImageView) convertView.findViewById(R.id.iv_contact);
                viewholder.imageDelete = (ImageButton) convertView.findViewById(R.id.iv_delete);
                viewholder.tvName = (TextView) convertView.findViewById(R.id.tv_name);
                viewholder.imageAddUser = (ImageButton) convertView.findViewById(R.id.ib_add_group);
                viewholder.tvInviteMore = (TextView) convertView.findViewById(R.id.tv_invite_more);
                
                convertView.setTag(viewholder);
            } else {
                viewholder = (ViewHolder) convertView.getTag();
            }

            viewholder.imageDelete.setVisibility(View.VISIBLE);

            if (GroupChatMode.CREATE == groupChatMode) {
                if(position == INVITE_GROUP_CHAT){
                    viewholder.imageUser.setVisibility(View.GONE);
                    viewholder.imageAddUser.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.VISIBLE);
                    viewholder.imageDelete.setVisibility(View.GONE);
                    viewholder.tvName.setVisibility(View.GONE);
                }
                else if (position == 0) {
                    viewholder.imageUser.setVisibility(View.VISIBLE);
                    viewholder.imageAddUser.setVisibility(View.GONE);
                    viewholder.imageDelete.setVisibility(View.GONE);
                    viewholder.tvName.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.GONE);
                } else {
                    viewholder.imageUser.setVisibility(View.VISIBLE);
                    viewholder.imageAddUser.setVisibility(View.GONE);
                    viewholder.imageDelete.setVisibility(View.VISIBLE);
                    viewholder.tvName.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.GONE);
                }
            } else if (GroupChatMode.EDIT == groupChatMode) {
                if(position == INVITE_GROUP_CHAT){
                    viewholder.imageUser.setVisibility(View.GONE);
                    viewholder.imageAddUser.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.VISIBLE);
                    viewholder.imageDelete.setVisibility(View.GONE);
                    viewholder.tvName.setVisibility(View.GONE);
                }
                else if (position == LEAVE_GROUP_CHAT) {
                    viewholder.imageUser.setVisibility(View.VISIBLE);
                    viewholder.imageAddUser.setVisibility(View.GONE);
                    viewholder.imageDelete.setImageResource(R.drawable.ic_leave_group);
                    viewholder.imageDelete.setVisibility(View.VISIBLE);
                    viewholder.tvName.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.GONE);
                } else {
                    viewholder.imageUser.setVisibility(View.VISIBLE);
                    viewholder.imageAddUser.setVisibility(View.GONE);
                    if(engineadapter.get().nmsUIIsGroupChatCreater(groupId) == true){
                        viewholder.imageDelete.setImageResource(R.drawable.ic_leave_group);
                        viewholder.imageDelete.setVisibility(View.VISIBLE);
                    }else{
                    	 viewholder.imageDelete.setVisibility(View.GONE);
                    }
                    	
                    viewholder.tvName.setVisibility(View.VISIBLE);
                    viewholder.tvInviteMore.setVisibility(View.GONE);
                }
            } else {
                viewholder.imageUser.setVisibility(View.VISIBLE);
                viewholder.imageAddUser.setVisibility(View.GONE);
                viewholder.tvName.setVisibility(View.VISIBLE);
                viewholder.tvInviteMore.setVisibility(View.GONE);
                viewholder.imageDelete.setVisibility(View.GONE);
            }

            viewholder.imageDelete.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View arg0) {
                    // TODO Auto-generated method stub
                    if (GroupChatMode.EDIT == groupChatMode) {
                    	if(position == 0){
                    		 confirmExitFromGroup();
                    	}else{
                    		confirmDeleteGroupMemberDialog(position);
                    	}
                       
                    } else {
                        confirmRemoveGroupMemberDialog(position);
                    }

                }
            });
            
            viewholder.imageAddUser.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View arg0) {
                    // TODO Auto-generated method stub
                    mListener.onInviteContact();
                }
            });

            NmsContact c = memberList.get(position);

            if (c != null) {
                if (c.getId() == GROUP_INVITE_MORE_ID) {
                    viewholder.imageUser.setImageResource(R.drawable.ic_contact_picture);
                } else {
                    viewholder.tvName.setText(c.getName());
                    mImageWorker.loadImage(c.getId(), viewholder.imageUser);
                }
            }

            return convertView;
        }
    }

    public void confirmExitFromGroup() {
        mListener.onLeaveGroupChat();
    }

    public void confirmRemoveGroupMemberDialog(final int position) {
        NmsAlertDialogUtils.showDialog(mContext, R.string.STR_NMS_CONFIRM_REMOVE_TITLE, 0,
                R.string.STR_NMS_REMOVE_CONTENT, R.string.STR_NMS_REMOVE_OK,
                R.string.STR_NMS_REMOVE_CANCEL, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        memberList.remove(position);
                        notifyGroupChatMemberChange();
                    }
                }, null);
    }
	public void confirmDeleteGroupMemberDialog(final int position) {
	    
        if (!NmsSMSMMSManager.isDefaultSmsApp()) {
            Toast.makeText(mContext, R.string.STR_UNABLE_EDIT_FOR_NOT_DEFAULT_SMS_APP, Toast.LENGTH_SHORT).show();
            return;
        }
        
        // Network
        if (!NmsCommonUtils.isNetworkReady(mContext)) {
            Toast.makeText(mContext, R.string.STR_NMS_NO_CONNECTION, Toast.LENGTH_SHORT).show();
            return;
        }
	    
        NmsAlertDialogUtils.showDialog(mContext, R.string.STR_NMS_CONFIRM_REMOVE_TITLE, 0,
                R.string.STR_NMS_REMOVE_CONTENT, R.string.STR_NMS_REMOVE_OK,
                R.string.STR_NMS_REMOVE_CANCEL, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (engineadapter.get().nmsUIDelMembersFromGroup(groupId,memberList.get(position).getNumber()) >= 0) {
                        memberList.remove(position);
                        notifyGroupChatMemberChange();
                        } else {
                            Toast.makeText(mContext, R.string.STR_NMS_DELETE_GROUP_MEMBER_FAILED,
                                    Toast.LENGTH_SHORT).show();
                        }
                    }
                }, null);
    }

    public void setOnGroupChatClickListener(OnGroupChatClickListener listener) {
        this.mListener = listener;
    }

    public interface OnGroupChatClickListener {
        public void onInviteContact();

        public void onLeaveGroupChat();
    }

    public String getGroupChatName() {
        return etGroupName.getText().toString();
    }

    public List<String> getChatMembers() {
        List<String> list = new ArrayList<String>(10);

        for (NmsContact c : memberList) {
            if (c.getNumber() != null && !c.getName().equals("")) {
                list.add(c.getNumber());
            }
        }

        return list;
    }

    public HashSet<String> getSelectedContact() {
        HashSet<String> s = new HashSet<String>();

        for (NmsContact c : memberList) {
            if (c.getId() > 0) {
                s.add(String.valueOf(c.getId()));
            }
        }

        return s;
    }

    public void hideKeyborad() {
        InputMethodManager imm = (InputMethodManager) mContext
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm != null) {
            imm.hideSoftInputFromWindow(etGroupName.getWindowToken(), 0);
        }
    }

    private void showKeyborad() {
        InputMethodManager imm = (InputMethodManager) getContext().getSystemService(
                Context.INPUT_METHOD_SERVICE);
        etGroupName.requestFocus();
        imm.showSoftInput(etGroupName, 0);
    }
}
