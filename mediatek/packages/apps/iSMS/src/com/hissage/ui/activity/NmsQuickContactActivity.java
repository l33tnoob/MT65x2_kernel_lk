package com.hissage.ui.activity;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import android.app.Activity;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.PhoneLookup;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsContactApi;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContactManager;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.imagecache.NmsContactAvatarCache;
import com.hissage.imageworker.NmsContactAvatarWorker;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.ui.view.NmsGroupManageView;
import com.hissage.util.log.NmsLog;
import com.hissage.util.message.MessageUtils;

public class NmsQuickContactActivity extends Activity {

    public String Tag = "QuickContactActivity";

    private class QuickContactDetail {
        // long contactId;
        String name;
        Bitmap avatar;
        List<QuickContactPhone> numbers;
    }

    private class QuickContactPhone {
        String number;
        String numberType;
        boolean isHissage;
    }

    private Context mContext;
    private long threadId;
    private short contactId;
    private NmsContact contact;

    private String phoneNum;

    static final String[] PHONE_LOOKUP_PROJECTION = new String[] { PhoneLookup._ID,
            PhoneLookup.LOOKUP_KEY, };
    static final int PHONE_ID_COLUMN_INDEX = 0;
    static final int PHONE_LOOKUP_STRING_COLUMN_INDEX = 1;

    private QuickContactDetail nativeContact;
    private ArrayList<NmsContact> memberList = new ArrayList<NmsContact>();

    private boolean isActive = true;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        mContext = this;// this.getApplicationContext();
        threadId = getIntent().getLongExtra("threadId", (long) -1);
        contactId = getIntent().getShortExtra("contactId", (short) -1);
        phoneNum = getIntent().getStringExtra("PHONE_NUM");

        int simId = (int) NmsPlatformAdapter.getInstance(this).getCurrentSimId();
        SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(simId);
        if (info != null && info.status != NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
            isActive = false;
        }

        if (threadId > 0) {
            contact = NmsIpMessageApiNative.nmsGetContactInfoViaThreadId(threadId);
        } else {
            if (contactId > 0) {
                contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
            } else {
                contact = NmsIpMessageApiNative.nmsGetContactInfoViaNumber(phoneNum);
            }
        }

        try {
            if (contact instanceof NmsGroupChatContact) {
                if (simId != ((NmsGroupChatContact) contact).getSimId()) {
                    isActive = false;
                }

                initQuickGroupCard();
            } else {
                initQuickContactCard();
            }
        } catch (Exception e) {
            NmsLog.error(Tag, "onCreate got the execption: " + NmsLog.nmsGetStactTrace(e));
        }
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

    public final static int REQ_OUTSIDE_INVITE_MORE = 1001;

    private void addGroupChatMembers() {
        Intent i = new Intent(this, NmsContactSelectionActivity.class);
        i.putExtra(NmsContactSelectionActivity.LOADTYPE, NmsContactManager.TYPE_HISSAGE);
        i.putExtra(NmsContactSelectionActivity.SELECTMAX, NmsCustomUIConfig.GROUPMEM_MAX_COUNT);

        Bundle bundle = new Bundle();

        bundle.putSerializable(NmsContactSelectionActivity.SELECTEDID,
                (Serializable) this.getSelectedContact());

        i.putExtras(bundle);

        NmsLog.trace(Tag, "add group chat members, the requestCode:" + REQ_OUTSIDE_INVITE_MORE);
        startActivityForResult(i, REQ_OUTSIDE_INVITE_MORE);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        NmsLog.trace(Tag, "Get activtiy result requestCode:" + requestCode);

        switch (requestCode) {
        case REQ_OUTSIDE_INVITE_MORE: {
            if (data != null) {
                String[] contactId = data
                        .getStringArrayExtra(NmsContactSelectionActivity.CONTACTTAG);
                NmsLog.trace(Tag, "Invite more contacts and get contactId count:"
                        + contactId.length);
                selectGroupMembers(contactId);
                chat();

            }
        }
            break;

        default:
            NmsLog.trace(Tag, "The unknown activity result");
            break;
        }
    }

    public void selectGroupMembers(String[] contactId) {
        List<String> members = new ArrayList<String>();
        NmsGroupChatContact groupContact = (NmsGroupChatContact) contact;
        short groupId = groupContact.getId();
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

        // if (GroupChatMode.VIEW == groupChatMode) {
        // buildViewGroupMembers();
        // }
    }

    private void initQuickGroupCard() {
        setContentView(R.layout.quick_group_contact);

        NmsGroupChatContact groupContact = (NmsGroupChatContact) contact;
        short[] members = groupContact.getMemberIds();
        buildGroupMembers(members, groupContact.isAlive());

        TextView tvName = (TextView) this.findViewById(R.id.tv_name);
        tvName.setText(groupContact.getName());

        TextView tvCount = (TextView) this.findViewById(R.id.tv_count);
        tvCount.setText(String.valueOf(groupContact.getMemberCount()));

        Button btnInvite = (Button) this.findViewById(R.id.btn_invite);
        btnInvite.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                addGroupChatMembers();
            }
        });
        Button btnChat = (Button) this.findViewById(R.id.btn_chat);
        btnChat.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                chat();
            }
        });

        Button btnWelcome = (Button) this.findViewById(R.id.btn_welcome);
        btnWelcome.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                chat();
            }
        });
        LinearLayout llWelcome = (LinearLayout) this.findViewById(R.id.ll_group);

        if (groupContact.isAlive() && isActive) {
            btnWelcome.setVisibility(View.GONE);
        } else {
            llWelcome.setVisibility(View.GONE);
            btnWelcome.setVisibility(View.VISIBLE);
        }

        GridView gv = (GridView) this.findViewById(R.id.gv_group_card);

        GroupListAdapter adapter = new GroupListAdapter(this);
        gv.setAdapter(adapter);

        gv.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                // TODO Auto-generated method stub
                viewGroupChatInfo();
            }
        });
    }

    private void leaveGroup() {
        MessageUtils.showLeaveGroupDialog(this, contact.getId(), true);
    }

    private void chat() {
        finish();
        Intent intent = new Intent(this, NmsChatDetailsActivity.class);
        intent.putExtra(NmsIpMessageConsts.NMS_ENGINE_CONTACT_ID, contact.getId());
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(intent);
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();

    }

    public void buildGroupMembers(short[] contactId, boolean isAlive) {
        memberList.clear();

        if (isAlive) {
            NmsContact selfPerson = new NmsContact();
            selfPerson.setName(this.getResources().getText(R.string.STR_NMS_GROUP_MEMEBER_YOU)
                    .toString());
            selfPerson.setId(NmsContactApi.getInstance(mContext).getMyselfEngineContactIdViaSimId(
                    (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId()));

            memberList.add(selfPerson);
        }

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

    private void viewGroupChatInfo() {

        NmsGroupChatContact groupContact = (NmsGroupChatContact) contact;

        if (groupContact.isAlive() && isActive) {
            finish();
            Intent i = new Intent(this, NmsGroupChatInfoActivity.class);
            i.putExtra("groupId", groupContact.getId());
            startActivity(i);
        }
    }

    private void startAsyncQuery() {
        if (threadId > 0) {
            phoneNum = NmsSMSMMSManager.getInstance(NmsQuickContactActivity.this)
                    .getMsgAddressViaThreadId((int) threadId);
        }

        Cursor cursor = this.managedQuery(
                Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, phoneNum),
                PHONE_LOOKUP_PROJECTION, null, null, null);
        if (cursor != null && cursor.moveToFirst()) {
            long contactId = cursor.getLong(PHONE_ID_COLUMN_INDEX);
            Cursor cursors = this.managedQuery(Data.CONTENT_URI, new String[] { Data.MIMETYPE,
                    Data.DATA1, Data.DATA2, Data.DATA3, Data.DATA15 }, Data.CONTACT_ID + "=?",
                    new String[] { String.valueOf(contactId) }, null);
            if (cursors != null) {
                setQuickContactCursor(cursors);
            }
        } else {
            initAddContactDailog();
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void initQuickContactCard() {
        startAsyncQuery();
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private static class ViewHolder {
        TextView tvLineOne;
        TextView tvLineTwo;
        ImageView ivChat;
        ImageView ivVideoCall;
        LinearLayout llCall;
    }

    private static class ViewGroupHolder {
        ImageView imageUser;
        TextView tvName;
    }

    public class PhoneListAdapter extends BaseAdapter {
        private Context context;

        public PhoneListAdapter(Context context) {
            this.context = context;
        }

        @Override
        public int getCount() {
            // TODO Auto-generated method stub
            return nativeContact.numbers.size();
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            // TODO Auto-generated method stub
            ViewHolder holder;

            if (convertView == null) {

                LayoutInflater inflater = (LayoutInflater) context
                        .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                convertView = inflater.inflate(R.layout.contact_detail_item, null);

                holder = new ViewHolder();

                holder.tvLineOne = (TextView) convertView.findViewById(R.id.tv_lineOne);
                holder.tvLineTwo = (TextView) convertView.findViewById(R.id.tv_lineTwo);
                holder.ivChat = (ImageView) convertView.findViewById(R.id.iv_chat);
                holder.ivVideoCall = (ImageView) convertView.findViewById(R.id.iv_video_call);
                holder.llCall = (LinearLayout) convertView.findViewById(R.id.ll_call);

                convertView.setTag(holder);

            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            final QuickContactPhone c = nativeContact.numbers.get(position);
            if (null != c) {
                holder.tvLineOne.setText(c.number);
                holder.tvLineTwo.setText(c.numberType);
                if (c.isHissage) {
                    holder.ivChat.setImageResource(R.drawable.chat_isms);
                } else {
                    holder.ivChat.setImageResource(R.drawable.ic_chat);
                }

                holder.ivChat.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        // TODO Auto-generated method stub
                        sendMessage(c.number);
                    }
                });
                
                holder.ivVideoCall.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        // TODO Auto-generated method stub
                        videoCall(c.number);
                    }
                });
 
                holder.llCall.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        // TODO Auto-generated method stub
                        call(c.number);
                    }
                });

            }

            return convertView;
        }
    }

    private void call(String phoneNum) {

        this.finish();
        Intent myIntentDial = new Intent("android.intent.action.CALL", Uri.parse("tel:" + phoneNum));
        startActivity(myIntentDial);
    }

    private void videoCall(String phoneNum) {
        this.finish();
        Intent _intent = new Intent(Intent.ACTION_CALL, Uri.parse("tel:" + phoneNum));
        _intent.putExtra("com.android.phone.extra.video", true);
        startActivity(_intent);

    }

    private void sendMessage(String phoneNum) {
        this.finish();
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.putExtra("address", phoneNum);
        intent.setType("vnd.android-dir/mms-sms");
        startActivity(intent);
    }

    public class GroupListAdapter extends BaseAdapter {
        private Context mContext;
        private NmsContactAvatarWorker mImageWorker;

        public GroupListAdapter(Context c) {
            mContext = c;
            mImageWorker = new NmsContactAvatarWorker(mContext, R.drawable.ic_contact_picture,
                    NmsContactAvatarCache.getInstance());
        }

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
            ViewGroupHolder viewholder = null;
            if (convertView == null) {
                viewholder = new ViewGroupHolder();

                final LayoutInflater inflater = (LayoutInflater) mContext
                        .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                convertView = inflater.inflate(R.layout.quick_group_contact_item, null);

                viewholder.imageUser = (ImageView) convertView.findViewById(R.id.iv_contact);
                viewholder.tvName = (TextView) convertView.findViewById(R.id.tv_name);

                convertView.setTag(viewholder);
            } else {
                viewholder = (ViewGroupHolder) convertView.getTag();
            }

            NmsContact c = memberList.get(position);

            if (c != null) {
                viewholder.tvName.setText(c.getName());
                mImageWorker.loadImage(c.getId(), viewholder.imageUser);
            }

            return convertView;
        }
    }

    private void setQuickContactCursor(Cursor cursor) {

        if (cursor != null && cursor.moveToFirst()) {
            nativeContact = new QuickContactDetail();
            // contact.contactId = contactId;
            nativeContact.numbers = new ArrayList<NmsQuickContactActivity.QuickContactPhone>();
            do {
                String mimeType = cursor.getString(cursor.getColumnIndex(Data.MIMETYPE));
                if (mimeType.equals(StructuredName.CONTENT_ITEM_TYPE)) {
                    String name = cursor.getString(cursor
                            .getColumnIndex(StructuredName.DISPLAY_NAME));
                    nativeContact.name = name;
                } else if (mimeType.equals(Phone.CONTENT_ITEM_TYPE)) {
                    boolean isAdd = true;
                    String number = cursor.getString(cursor.getColumnIndex(Phone.NUMBER));
                    if (number == null) {
                        continue;
                    }
                    for (int i = 0; i < nativeContact.numbers.size(); ++i) {
                        if (number.equals(nativeContact.numbers.get(i).number)) {
                            isAdd = false;
                            break;
                        }
                    }
                    if (!isAdd) {
                        continue;
                    }
                    QuickContactPhone phone = new QuickContactPhone();
                    phone.number = number;
                    String lable = Phone.getTypeLabel(getResources(),
                            cursor.getInt(cursor.getColumnIndex(Phone.TYPE)),
                            cursor.getString(cursor.getColumnIndex(Phone.LABEL))).toString();
                    phone.isHissage = NmsIpMessageApiNative.nmsIsiSMSNumber(number);
                    if (phone.isHissage) {
                        phone.numberType = lable
                                + this.getText(R.string.STR_NMS_ISMS_ENABLE).toString();
                    } else {
                        phone.numberType = lable;
                    }

                    nativeContact.numbers.add(phone);
                } else if (mimeType.equals(Photo.CONTENT_ITEM_TYPE)) {
                    byte[] photoBytes = cursor.getBlob(cursor.getColumnIndex(Photo.PHOTO));
                    if (photoBytes != null) {
                        nativeContact.avatar = BitmapFactory.decodeByteArray(photoBytes, 0,
                                photoBytes.length);
                    }
                }
            } while (cursor.moveToNext());
        }
        if (cursor != null) {
            cursor.close();
        }

        updateUI();
    }

    private void updateUI() {

        setContentView(R.layout.quick_contact);
        ImageView ivAvatar = (ImageView) this.findViewById(R.id.iv_avatar);

        if (contact != null) {
            Bitmap bitmap = NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId(
                    contact.getId());
            if (bitmap != null) {
                ivAvatar.setImageBitmap(bitmap);
            }
        }

        TextView tvName = (TextView) this.findViewById(R.id.tv_name);
        tvName.setText(nativeContact.name);

        ListView lvPhone = (ListView) this.findViewById(R.id.lv_phonelist);

        LayoutParams lp = (LayoutParams) lvPhone.getLayoutParams();
        if (nativeContact.numbers.size() > 2) {
            lp.height = this.getResources().getDimensionPixelOffset(R.dimen.phone_list_quick);
        }

        lvPhone.setLayoutParams(lp);

        PhoneListAdapter adapter = new PhoneListAdapter(this);
        lvPhone.setAdapter(adapter);

        lvPhone.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                // TODO Auto-generated method stub
                call(nativeContact.numbers.get(arg2).number);
            }
        });
    }

    public void initAddContactDailog() {
        setContentView(R.layout.quick_strange);

        TextView tvMessage = (TextView) this.findViewById(R.id.tv_message);
        String s = String.format(getText(R.string.STR_NMS_ADD_PHONE_CONTACT).toString(), phoneNum);
        tvMessage.setText(s);

        Button btnCancel = (Button) this.findViewById(R.id.btn_cancel);
        btnCancel.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                NmsQuickContactActivity.this.finish();
            }
        });
        Button btnOK = (Button) this.findViewById(R.id.btn_ok);
        btnOK.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                addNewContact();
            }
        });
    }

    public void addNewContact() {

        Intent intent = new Intent(Intent.ACTION_INSERT);
        intent.setType("vnd.android.cursor.dir/person");
        intent.setType("vnd.android.cursor.dir/contact");
        intent.setType("vnd.android.cursor.dir/raw_contact");
        intent.putExtra(android.provider.ContactsContract.Intents.Insert.PHONE, phoneNum);

        this.finish();

        startActivity(intent);
    }

}
