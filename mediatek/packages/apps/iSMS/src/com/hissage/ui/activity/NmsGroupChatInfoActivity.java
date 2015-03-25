package com.hissage.ui.activity;

import java.io.Serializable;

import android.app.ActionBar;
import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.ActionMode;
import android.view.ActionMode.Callback;
import android.view.View.OnClickListener;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.api.NmsiSMSApi;
import com.hissage.config.NmsChatSettings;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContactManager;
import com.hissage.message.ip.NmsHesineApiConsts;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.view.NmsGroupManageView;
import com.hissage.ui.view.NmsGroupManageView.GroupChatMode;
import com.hissage.ui.view.NmsGroupManageView.OnGroupChatClickListener;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsGroupChatInfoActivity extends NmsBaseActivity implements OnGroupChatClickListener {

    public String Tag = "GroupChatInfoActivity";

    public final static int REQ_INVITE_MORE = 1000;

    private NmsGroupManageView mGMV;
    private GroupChatMode mGroupMode = GroupChatMode.VIEW;
    private short mGroupId;

    private int simId;
    private boolean showMedia;
    private boolean showLoaction;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.group_manage_info);

        Intent intent = getIntent();
        mGroupId = intent.getShortExtra("groupId", (short) -1);

        simId = (int) NmsPlatformAdapter.getInstance(this).getCurrentSimId();

        NmsLog.trace(Tag, "Start GroupChatInfoActivity, the groupId:" + mGroupId);
        mGroupMode = GroupChatMode.VIEW;

        mGMV = (NmsGroupManageView) findViewById(R.id.group_manage);
        mGMV.setGroupChatId(mGroupId);
        mGMV.setOnGroupChatClickListener(this);

        mGMV.setGroupChatMode(mGroupMode);

        setupActionBar();
    }

    private void setupActionBar() {

        ActionBar actionBar = getActionBar();
        if (mGroupMode == GroupChatMode.VIEW) {
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

            actionBar.setTitle(R.string.STR_NMS_GROUP_INFO_TITLE);
            actionBar.setBackgroundDrawable(this.getResources().getDrawable(android.R.drawable.dark_header));

        } else {
            actionBar.setBackgroundDrawable(this.getResources().getDrawable(
                    R.drawable.action_bar_selection));
            actionBar.setDisplayShowTitleEnabled(false);
            actionBar.setDisplayShowHomeEnabled(false);
            actionBar.setDisplayHomeAsUpEnabled(false);
            actionBar.setDisplayShowCustomEnabled(true);

            ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(
                    R.layout.action_bar_selection, null);
            actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                    ActionBar.DISPLAY_SHOW_CUSTOM);
            actionBar.setCustomView(v, new ActionBar.LayoutParams(
                    ActionBar.LayoutParams.WRAP_CONTENT, ActionBar.LayoutParams.WRAP_CONTENT,
                    Gravity.CENTER_VERTICAL | Gravity.LEFT));

            TextView tvTitle = (TextView) v.findViewById(R.id.tv_group_title);
            tvTitle.setText(R.string.STR_NMS_EDIT_GROUP_TITLE);

            ImageButton ivDone = (ImageButton) v.findViewById(R.id.iv_done);
            ivDone.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    // TODO Auto-generated method stub
                    updateGroupChat();
                }
            });
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        if (mGroupMode == GroupChatMode.VIEW) {
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.group_info_menu, menu);

            MenuItem media = menu.findItem(R.id.menu_view_all_media);
            media.setVisible(showMedia);
            MenuItem location = menu.findItem(R.id.menu_view_all_locations);
            location.setVisible(showLoaction);
        }
        return true;
    }
    
    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        SNmsImMsgCountInfo countInfo = NmsiSMSApi.nmsSetImMode((int) mGroupId,
                NmsHesineApiConsts.NmsImFlag.NMS_IM_FLAG_ALL,
                NmsHesineApiConsts.NmsImReadMode.NMS_IM_READ_MODE_ALL);

        showMedia = countInfo.showAllMedia();
        showLoaction = countInfo.showAllLocation();
        invalidateOptionsMenu();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        NmsLog.trace(Tag, "The options item selected:" + item.getTitle());

        if (item.getItemId() == android.R.id.home) {
            this.finish();
        } else if (item.getItemId() == R.id.menu_add_members) {
            if (invalidate()) {
                addGroupChatMembers();
            }
        } else if (item.getItemId() == R.id.menu_edit) {
            if (invalidate()) {
                editGroupChatInfo();
            }
        } else if (item.getItemId() == R.id.menu_view_all_media) {
            viewAllMedia();
        } else if (item.getItemId() == R.id.menu_view_all_locations) {
            Intent intent = new Intent(this, NmsAllLocationsActivity.class);
            intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mGroupId);
            startActivity(intent);
        } else if (item.getItemId() == R.id.menu_chat_settings) {
            mGMV.startChatSettings();
        } else if (item.getItemId() == R.id.menu_leave_group) {
            if (invalidate()) {
                MessageUtils.showLeaveGroupDialog(this, mGroupId, true);
            }
        } else {
            NmsLog.trace(Tag, "The unknown options item");
            return true;
        }

        return false;
    }

    private void viewAllMedia() {
        Intent i = new Intent(this, NmsAllMediaActivity.class);
        i.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, mGroupId);
        startActivity(i);
    }

    private void updateGroupChat() {
        if (!invalidate()) {
            return;
        }

        mGroupMode = GroupChatMode.VIEW;

        if (!TextUtils.isEmpty(mGMV.getGroupChatName())) {
            NmsLog.trace(Tag, "Update group chat name, the groupId:" + mGroupId + " and the name:"
                    + mGMV.getGroupChatName());
            boolean flag = NmsIpMessageApiNative.nmsModifyGroupName(mGroupId,
                    mGMV.getGroupChatName());
            if (flag) {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_NAME_SUCCESS, Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_NAME_FAILED, Toast.LENGTH_SHORT).show();
            }
        }

        mGMV.setGroupChatMode(GroupChatMode.VIEW);

        setupActionBar();
        invalidateOptionsMenu();
    }
    
    private void updateGroupChatName() {
        if (!invalidate()) {
            return;
        }

        mGroupMode = GroupChatMode.VIEW;

        if (!TextUtils.isEmpty(mGMV.getGroupChatName())) {
            NmsLog.trace(Tag, "Update group chat name, the groupId:" + mGroupId + " and the name:"
                    + mGMV.getGroupChatName());
            boolean flag = NmsIpMessageApiNative.nmsModifyGroupName(mGroupId,
                    mGMV.getGroupChatName());
            if (flag) {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_SUCCESS, Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_FAILED, Toast.LENGTH_SHORT).show();
            }
        }

        mGMV.setGroupChatMode(GroupChatMode.VIEW);

        setupActionBar();
        invalidateOptionsMenu();
    }

    private void addGroupChatMembers() {
        if (!invalidate()) {
            return;
        }

        Intent i = new Intent(this, NmsContactSelectionActivity.class);
        i.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_HISSAGE);
        i.putExtra(NmsContactSelectionActivity.SELECTMAX, NmsCustomUIConfig.GROUPMEM_MAX_COUNT);

        Bundle bundle = new Bundle();
        bundle.putSerializable(NmsContactSelectionActivity.SELECTEDID,
                (Serializable) mGMV.getSelectedContact());
        i.putExtras(bundle);

        NmsLog.trace(Tag, "add group chat members, the requestCode:" + REQ_INVITE_MORE);
        startActivityForResult(i, REQ_INVITE_MORE);
    }

    private void editGroupChatInfo() {

        mGMV.setGroupChatMode(GroupChatMode.EDIT);
        mGroupMode = GroupChatMode.EDIT;

        setupActionBar();
        invalidateOptionsMenu();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        NmsLog.trace(Tag, "Get activtiy result requestCode:" + requestCode);

        switch (requestCode) {
        case REQ_INVITE_MORE: {
            if (data != null) {
                String[] contactId = data
                        .getStringArrayExtra(NmsContactSelectionActivity.CONTACTTAG);
                NmsLog.trace(Tag, "Invite more contacts and get contactId count:"
                        + contactId.length);
                mGMV.selectGroupMembers(contactId);
                mGMV.notifyGroupChatMemberChange();
            }
        }
            break;

        default:
            NmsLog.trace(Tag, "The unknown activity result");
            break;
        }
    }

    private boolean invalidate() {
        final int sim_id = mGMV.getGroupChatSimId();
        simId = (int) NmsPlatformAdapter.getInstance(this).getCurrentSimId();
        SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(simId);
        if (info != null && info.status == NmsSimActivateStatus.NMS_SIM_STATUS_DISABLED) {
            NmsAlertDialogUtils.showDialog(this, R.string.STR_NMS_ISMS_ENABLE_TITLE,
                    R.drawable.ic_dialog_alert_holo_light, R.string.STR_NMS_ISMS_ENABLE_CONTENT,
                    R.string.STR_NMS_ENABLE, R.string.STR_NMS_CANCEL,
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            NmsIpMessageApiNative.nmsEnableIpService(simId);
                        }
                    }, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            discard();
                        }
                    });
            return false;
        }
        
        if (!NmsSMSMMSManager.isDefaultSmsApp()) {
            Toast.makeText(this, R.string.STR_UNABLE_EDIT_FOR_NOT_DEFAULT_SMS_APP, Toast.LENGTH_SHORT).show();
            return false;
        }
        
        // Network
        if (!isNetworkReady()) {
            Toast.makeText(this, R.string.STR_NMS_NO_CONNECTION, Toast.LENGTH_SHORT).show();
            return false;
        }

        if (simId != mGMV.getGroupChatSimId()) {
            NmsAlertDialogUtils.showDialog(this, R.string.STR_NMS_SWITCH_TITLE,
                    R.drawable.ic_dialog_alert_holo_light, R.string.STR_NMS_SWITCH_EDIT_MESSAGE,
                    R.string.STR_NMS_SWITCH, R.string.STR_NMS_CANCEL,
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            changeSIMCard(sim_id);
                        }
                    }, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            discard();
                        }
                    });

            return false;
        }
        return true;
    }

    private void changeSIMCard(int simId) {
        NmsPlatformAdapter.getInstance(this).setCurrentSimId(simId);
    }

    private void discard() {
        this.finish();
    }

    @Override
    public void onInviteContact() {
        // TODO Auto-generated method stub
        addGroupChatMembers();
    }

    @Override
    public void onLeaveGroupChat() {
        // TODO Auto-generated method stub
        NmsLog.trace(Tag, "Leave group chat, the groupId:" + mGroupId);
        if (invalidate()) {
            mGMV.hideKeyborad();
            MessageUtils.showLeaveGroupDialog(this, mGroupId, true);
        }
    }

    @Override
    public void onBackPressed() {
        if (mGroupMode == GroupChatMode.EDIT) {      
            updateGroupChatName();
        } else {
            super.onBackPressed();
        }
    }
}
