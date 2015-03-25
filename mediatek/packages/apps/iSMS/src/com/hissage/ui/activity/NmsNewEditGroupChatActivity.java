package com.hissage.ui.activity;

import java.io.Serializable;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsStartActivityApi;
import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContactManager;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.view.NmsGroupManageView;
import com.hissage.ui.view.NmsGroupManageView.OnGroupChatClickListener;
import com.hissage.util.data.NmsAlertDialogUtils;
import com.hissage.util.data.NmsApiErrorCode;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsNewEditGroupChatActivity extends NmsBaseActivity implements
        OnGroupChatClickListener {

    public String Tag = "NewGroupChatActivity";

    public static String SIMID = "SIMID";

    public final static int REQ_INVITE_MORE = 1000;
    private short mGroupId = -1;
    private int simId = 0;
    private NmsGroupManageView mGMV;
    private ImageButton ivDone;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.group_manage_info);

        NmsLog.trace(Tag, "Start NewGroupChatActivity...");
        Intent intent = getIntent();
        mGroupId = intent.getShortExtra("groupId", (short) -1);
        String[] contactId = intent.getStringArrayExtra(NmsContactSelectionActivity.CONTACTTAG);

        simId = intent.getIntExtra(NmsContactSelectionActivity.SIMID, 0);
        
        NmsLog.trace(Tag, "Get group chat simId:" +  simId);
        
        NmsLog.trace(Tag, "The GroupChat groupId:" + mGroupId);

        mGMV = (NmsGroupManageView) findViewById(R.id.group_manage);
        mGMV.setOnGroupChatClickListener(this);
        if (mGroupId > 0) {
            mGMV.setGroupChatId(mGroupId);
            mGMV.editGroupChat();
        } else {
            mGMV.setContactIdList(contactId, simId);
            mGMV.createGroupChat();
        }

        initActionBar();

    }

    private void initActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setBackgroundDrawable(this.getResources().getDrawable(
                R.drawable.action_bar_selection));
        actionBar.setDisplayShowTitleEnabled(false);
        actionBar.setDisplayShowHomeEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(false);

        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(R.layout.action_bar_selection,
                null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL | Gravity.LEFT));

        TextView tvTitle = (TextView) v.findViewById(R.id.tv_group_title);

        if (mGroupId > 0) {
            tvTitle.setText(R.string.STR_NMS_EDIT_GROUP_TITLE);
        } else {
            tvTitle.setText(R.string.STR_NMS_CREATE_GROUP_TITLE);
        }

        ivDone = (ImageButton) v.findViewById(R.id.iv_done);
        ivDone.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                finishGroupChat();
            }
        });
    }

    private boolean invalidateManage() {
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

    private void finishGroupChat() {

        String name = mGMV.getGroupChatName();

        NmsLog.trace(Tag, "Get group chat name:" + name);
        if (TextUtils.isEmpty(name)) {
            Toast.makeText(this, R.string.STR_NMS_GROUP_NAME_EMPTY, Toast.LENGTH_SHORT).show();
            return;
        }

        if (mGroupId > 0) {
//            if (!invalidateManage()) {
//                return;
//            }

            this.finish();
            NmsLog.trace(Tag, "Modify group chat, the groupId:" + mGroupId + " and the name:"
                    + name);

            boolean flag = (NmsIpMessageApiNative.nmsModifyGroupName(mGroupId, name));

            if (flag) {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_NAME_SUCCESS, Toast.LENGTH_SHORT)
                        .show();

                mGMV.hideKeyborad();
            } else {
                // NmsApiErrorCode.get(this).showApiResultToast(ret);
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_NAME_FAILED, Toast.LENGTH_SHORT)
                        .show();
            }
        } else {
            NmsLog.trace(Tag, "Create group chat name:" + name);
            ivDone.setEnabled(false);
            short gId = NmsIpMessageApiNative.nmsCreateGroup(simId, name, mGMV.getChatMembers());

            if (gId > 0) {
                this.finish();
                Intent intent = new Intent();
                intent.setAction(NmsIpMessageConsts.ACTION_CHAT_DETAILS);
                intent.setClassName(NmsIpMessageConsts.PACKAGE_NAME_NMS,
                        NmsIpMessageConsts.CLASS_NAME_CHAT_DETAILS);
                NmsLog.trace(Tag, "View group chat detail and the groupId:" + gId);
                intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, gId);
                startActivity(intent);
            } else {
                ivDone.setEnabled(true);
                NmsApiErrorCode.get(this).showApiResultToast(gId);
                // Toast.makeText(this,
                // R.string.STR_NMS_CREATE_GROUP_NAME_FAILED,
                // Toast.LENGTH_SHORT).show();
            }
        }
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
        }
    }

    @Override
    public void onBackPressed() {
        if (mGroupId > 0) {
//            if (!invalidateManage()) {
//                this.finish();
//                return;
//            }

            String name = mGMV.getGroupChatName();

            NmsLog.trace(Tag, "Get group chat name:" + name);
            if (TextUtils.isEmpty(name)) {
                Toast.makeText(this, R.string.STR_NMS_GROUP_NAME_EMPTY, Toast.LENGTH_SHORT).show();
                return;
            }

            this.finish();
            NmsLog.trace(Tag, "Modify group chat, the groupId:" + mGroupId + " and the name:"
                    + name);

            boolean flag = (NmsIpMessageApiNative.nmsModifyGroupName(mGroupId, name));

            if (flag) {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_SUCCESS, Toast.LENGTH_SHORT)
                        .show();
                mGMV.hideKeyborad();
            } else {
                Toast.makeText(this, R.string.STR_NMS_UPDATE_GROUP_FAILED, Toast.LENGTH_SHORT)
                        .show();
            }
        } else {
            NmsAlertDialogUtils.showDialog(this, R.string.STR_NMS_BACK_TITLE, 0,
                    R.string.STR_NMS_GROUP_CHAT_BACK, R.string.STR_NMS_OK, R.string.STR_NMS_CANCEL,
                    new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            // TODO Auto-generated method stub
                            NmsNewEditGroupChatActivity.this.finish();
                        }
                    }, null);
        }
    }

    @Override
    public void onInviteContact() {
        // TODO Auto-generated method stub
        if (mGroupId > 0) {
            if (invalidateManage()) {
                inviateMore();
            }
        } else {
            inviateMore();
        }
    }

    private void inviateMore() {
        Intent i = new Intent(this, NmsContactSelectionActivity.class);
        i.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_HISSAGE);
        i.putExtra(NmsContactSelectionActivity.SELECTMAX, NmsCustomUIConfig.GROUPMEM_MAX_COUNT);
        Bundle bundle = new Bundle();
        bundle.putSerializable(NmsContactSelectionActivity.SELECTEDID,
                (Serializable) mGMV.getSelectedContact());
        i.putExtras(bundle);
        NmsLog.trace(Tag, "Invite more contacts, the requestCode:" + REQ_INVITE_MORE);
        startActivityForResult(i, REQ_INVITE_MORE);
    }

    @Override
    public void onLeaveGroupChat() {
        // TODO Auto-generated method stub
        if (invalidateManage()) {
            MessageUtils.showLeaveGroupDialog(this, mGroupId, true);
        }
    }
}
