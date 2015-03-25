package com.hissage.ui.adapter;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Pattern;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SectionIndexer;
import android.widget.TextView;
import android.widget.AbsListView.OnScrollListener;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsUIContact;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.ui.activity.NmsContactSelectionActivity;
import com.hissage.ui.activity.NmsContactSelectionActivity.SelectionMode;
import com.hissage.ui.view.NmsPinnedHeaderListView;
import com.hissage.ui.view.NmsPinnedHeaderListView.NmsPinnedHeaderAdapter;
import com.hissage.util.log.NmsLog;
import com.hissage.jni.engineadapter;

public class NmsContactSelectionAdapter extends BaseAdapter implements SectionIndexer,
        NmsPinnedHeaderAdapter, OnScrollListener {

    public String Tag = "ContactListAdapter";

    private SelectionMode mType;
    private Context mContext;
    private ArrayList<NmsUIContact> mContacList;
    private HashMap<String, String> mSelectedContactId;
    public ArrayList<NmsUIContact> mSearchList;
    private String mSearchStr = "";
    private boolean mIsSearch = false;
    private int mCount;
    private boolean mIsActivated = false;
    private int mSimId;
    private final LruCache<Long, Bitmap> mContactAvatarItemCach;

    private String currentStr, previewStr;

    private String[] sections;
    private String sectionLetter, nextSectionLetter;

    private SectionIndexer sectionIndexer = NmsContactSelectionAdapter.this;

    private ListView mListView;

    private static class ViewHolder {

        TextView alpha;
        TextView divider;
        TextView tvName;
        TextView tvPhoneNum;
        CheckBox cbCheck;
        ImageView ivLog;
        ImageView qcbAvatar;
    }

    public NmsContactSelectionAdapter(Context context, ListView listView,
            ArrayList<NmsUIContact> contacList, SelectionMode mode, int simId) {

        mSelectedContactId = new HashMap<String, String>();
        mContext = context;
        this.mListView = listView;

        mContacList = contacList;
        mCount = contacList.size();
        mType = mode;
        mSimId = simId;
        mContactAvatarItemCach = new LruCache<Long, Bitmap>(50);
        if (isActivated()) {
            mIsActivated = true;
        }
        this.sections = new String[mCount];
        for (int i = 0; i < mCount; i++) {
            String name = getAlpha(mContacList.get(i).getSortKey());
            sections[i] = name;
        }

        mListView.setFastScrollEnabled(mCount >= 10);
        mListView.setFastScrollAlwaysVisible(mCount >= 10);
    }

    public int getSelectedCount() {
        if (mSelectedContactId != null) {
            return mSelectedContactId.size();
        } else {
            return 0;
        }
    }

    public void exitSearch() {
        mIsSearch = false;
        mCount = mContacList.size();
        this.notifyDataSetChanged();
    }

    public int search(String queryString) {
        mIsSearch = true;
        mSearchStr = queryString;
        buildSearchList();
        mCount = mSearchList.size();
        this.notifyDataSetChanged();

        return mCount;
    }

    public int search(Cursor cursor, String queryString) {
        mIsSearch = true;
        mSearchStr = queryString;
        //buildSearchList();
        mSearchList = new ArrayList<NmsUIContact>();
        if (cursor == null) {
            mSearchList.addAll(mContacList);
        } else {
            try {
                while (cursor.moveToNext()) {
                    long source_id = cursor.getLong(0);
                    for (NmsUIContact contact : mContacList) {
                        long dest_id = contact.getSystemContactId();
                        if (dest_id == source_id) {
                            mSearchList.add(contact);
                        }
                    }
                }
            } finally {
                cursor.close();
            }
        }
        mCount = mSearchList.size();
        this.notifyDataSetChanged();
        return mCount;
    }

    private void buildSearchList() {
        mSearchList = new ArrayList<NmsUIContact>();
        for (NmsUIContact contact : mContacList) {
            if (isContain(contact.getSortKey()) || contact.getNumberOrEmail().contains(mSearchStr)) {
                mSearchList.add(contact);
            }
        }
    }

    private boolean isContain(String sortKey) {
        if (mSearchStr == null || mSearchStr.trim().length() == 0) {
            return true;
        }

        int index = 0;
        sortKey = sortKey.toUpperCase();
        int length = mSearchStr.trim().length();

        for (int i = 0; i < length; i++) {
            char s = mSearchStr.trim().charAt(i);
            if (index == -1 || !sortKey.substring(index).contains((s + "").toUpperCase())) {
                return false;
            } else {
                index = sortKey.indexOf((s + "").toUpperCase(), index) + 1;
            }
        }

        return true;
    }

    public boolean checkSelection(int index) {
        NmsUIContact c = null;

        if (!mIsSearch) {
            c = mContacList.get(index);
        } else {
            c = mSearchList.get(index);
        }

        String phoneNum = engineadapter.get().nmsUIGetUnifyPhoneNumber(c.getNumberOrEmail());

        if (mSelectedContactId.containsKey(phoneNum)) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public int getCount() {
        NmsLog.trace(Tag, "Get all converation count:" + mCount);
        return mCount;
    }

    public View getView(int position, View convertView, ViewGroup parent) {

        ViewHolder holder;
        if (convertView == null) {
            final LayoutInflater inflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.contact_item, null);

            holder = new ViewHolder();
            holder.alpha = (TextView) convertView.findViewById(R.id.alpha);
            holder.divider = (TextView) convertView.findViewById(R.id.listview_divider);
            holder.tvName = (TextView) convertView.findViewById(R.id.tv_contact_name);
            holder.tvPhoneNum = (TextView) convertView.findViewById(R.id.tv_contact_phonenumber);
            holder.cbCheck = (CheckBox) convertView.findViewById(R.id.cb_check);
            holder.ivLog = (ImageView) convertView.findViewById(R.id.iv_logo);
            holder.qcbAvatar = (ImageView) convertView.findViewById(R.id.qcb_avatar);
            holder.qcbAvatar.setClickable(false);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        if (mType == SelectionMode.SINGLE) {
            holder.cbCheck.setVisibility(View.GONE);
        } else {
            holder.cbCheck.setVisibility(View.VISIBLE);
        }

        NmsUIContact c = null;
        if (!mIsSearch) {
            c = mContacList.get(position);
        } else {
            c = mSearchList.get(position);
        }

        if (c != null) {
            String number = engineadapter.get().nmsUIGetUnifyPhoneNumber(c.getNumberOrEmail());
            if(TextUtils.isEmpty(number)){
                number = c.getNumberOrEmail();
            }
            if (mSelectedContactId.containsKey(number)) {
                holder.cbCheck.setChecked(true);
            } else {
                holder.cbCheck.setChecked(false);
            }

            if (mIsSearch) {
                String lineOne = c.getName();
                String lineTwo = c.getNumberOrEmail();
                int i = 0;
                int j = 0;
                int index = 0;
                Spannable splineOne = new SpannableString(lineOne);
                Spannable splineTwo = new SpannableString(lineTwo);

                for (i = 0; i < mSearchStr.length(); i++) {
                    String s = mSearchStr.substring(i, i + 1);
                    for (j = index; j < lineOne.length(); j++) {
                        String subone = lineOne.substring(j, j + 1);
                        if (s.compareToIgnoreCase(subone) == 0) {
                            splineOne.setSpan(new ForegroundColorSpan(mContext.getResources()
                                    .getColor(R.color.search_highlight)), j, j + 1,
                                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                            index = ++j;
                            break;
                        }
                    }
                }

                if (lineTwo.contains(mSearchStr)) {
                    index = lineTwo.indexOf(mSearchStr);
                    splineTwo.setSpan(
                            new ForegroundColorSpan(mContext.getResources().getColor(
                                    R.color.search_highlight)), index, index + mSearchStr.length(),
                            Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                }

                holder.tvName.setText(splineOne);
                holder.tvPhoneNum.setText(c.getNumberOrEmailType(mContext) + " " + splineTwo);
            } else {
                holder.tvName.setText(c.getName());
                holder.tvPhoneNum.setText(c.getNumberOrEmailType(mContext) + " "
                        + c.getNumberOrEmail());
            }

            currentStr = getAlpha(c.getSortKey());
            previewStr = null;
            if (!mIsSearch) {
                previewStr = (position - 1) >= 0 ? getAlpha(mContacList.get(position - 1)
                        .getSortKey()) : " ";
            } else {
                previewStr = (position - 1) >= 0 ? getAlpha(mSearchList.get(position - 1)
                        .getSortKey()) : " ";
            }

            if (!previewStr.equals(currentStr)) {
                holder.alpha.setVisibility(View.VISIBLE);
                holder.alpha.setText(currentStr);
                holder.divider.setVisibility(View.GONE);

            } else {

                holder.divider.setVisibility(View.VISIBLE);
                holder.alpha.setVisibility(View.GONE);
            }

            Bitmap avatar = mContactAvatarItemCach.get(c.getSystemContactId());
            if (avatar == null) {
                avatar = c.getAvatar(mContext);
                if (avatar == null) {
                    avatar = BitmapFactory.decodeResource(mContext.getResources(),
                            R.drawable.ic_contact_picture);
                }
                mContactAvatarItemCach.put(c.getSystemContactId(), avatar);
            }
            holder.qcbAvatar.setImageBitmap(avatar);

            if ((c.getType() == NmsContactType.HISSAGE_USER) && mIsActivated) {
                holder.ivLog.setVisibility(View.VISIBLE);
            } else {
                holder.ivLog.setVisibility(View.GONE);
            }
        }

        return convertView;
    }

    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();

        if (mIsSearch) {
            mCount = mSearchList.size();
            this.sections = new String[mCount];
            for (int i = 0; i < mCount; i++) {
                String name = getAlpha(mSearchList.get(i).getSortKey());
                sections[i] = name;
            }
        } else {
            mCount = mContacList.size();
            this.sections = new String[mCount];
            for (int i = 0; i < mCount; i++) {
                String name = getAlpha(mContacList.get(i).getSortKey());
                sections[i] = name;
            }
        }
        mListView.setFastScrollEnabled(mCount >= 10);
        mListView.setFastScrollAlwaysVisible(mCount >= 10);
    }

    private boolean isActivated() {
        boolean isActivated = false;
        if (mSimId == NmsContactSelectionActivity.DEFAUT_SIMID) {
            mSimId = (int) NmsPlatformAdapter.getInstance(mContext).getCurrentSimId();
        }

        if (NmsIpMessageApiNative.nmsGetActivationStatus(mSimId) == SNmsSimInfo.NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
            isActivated = true;
        }
        return isActivated;
    }

    public void check(int index) {

        NmsUIContact c = null;

        if (!mIsSearch) {
            c = mContacList.get(index);
        } else {
            c = mSearchList.get(index);
        }

        short cId = c.getEngineContactId();
        String contactId = String.valueOf(cId);
        String phoneNum = engineadapter.get().nmsUIGetUnifyPhoneNumber(c.getNumberOrEmail());// c.getNumberOrEmail();
        if (TextUtils.isEmpty(phoneNum)) {
            phoneNum = c.getNumberOrEmail();
        }
        if (mSelectedContactId.containsKey(phoneNum)) {
            mSelectedContactId.remove(phoneNum);
        } else {
            mSelectedContactId.put(phoneNum, contactId);
        }

        this.notifyDataSetChanged();
    }

    public long selectSingleContact(int index) {

        NmsUIContact c = null;

        if (!mIsSearch) {
            c = mContacList.get(index);
        } else {
            c = mSearchList.get(index);
        }

        long cId = c.getSystemContactId();

        return cId;
    }

    public int checkAll(boolean check) {
        if (check) {
            for (NmsUIContact c : mContacList) {
                short cId = c.getEngineContactId();
                String contactId = String.valueOf(cId);
                //String phoneNum = c.getNumberOrEmail();
                String phoneNum = engineadapter.get().nmsUIGetUnifyPhoneNumber(c.getNumberOrEmail());// c.getNumberOrEmail();
                if (TextUtils.isEmpty(phoneNum)) {
                    phoneNum = c.getNumberOrEmail();
                }
                mSelectedContactId.put(phoneNum, contactId);
            }
        } else {
            mSelectedContactId.clear();
        }

        this.notifyDataSetChanged();

        return mSelectedContactId.size();
    }

    public String[] getSelectContactId() {
        String[] contactId = new String[mSelectedContactId.size()];
        mSelectedContactId.values().toArray(contactId);

        return contactId;
    }

    public String getSelectPhoneNumber() {
        String phone = "";
        List<String> phoneList = new ArrayList<String>(mSelectedContactId.keySet());
        for (int i = 0; i < phoneList.size(); i++) {
            phone += phoneList.get(i) + ";";
        }

        if (!TextUtils.isEmpty(phone)) {
            phone = phone.substring(0, phone.length() - 1);
        }

        return phone;
    }

    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        return position;
    }

    @Override
    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return position;
    }

    private String getAlpha(String str) {

        if (str == null) {
            return "#";
        }
        if (str.trim().length() == 0) {
            return "#";
        }

        char c = str.trim().substring(0, 1).charAt(0);

        Pattern pattern = Pattern.compile("^[A-Za-z]+$");
        if (pattern.matcher(c + "").matches()) {
            return (c + "").toUpperCase();
        } else {
            return "#";
        }

    }

    @Override
    public int getPinnedHeaderState(int position) {
        // TODO Auto-generated method stub
        int realPosition = position;
        if (sectionIndexer == null || mCount == 0) {
            return NmsPinnedHeaderAdapter.PINNED_HEADER_GONE;
        }
        if (realPosition < 0 && realPosition >= sections.length) {
            return PINNED_HEADER_GONE;
        }

        sectionLetter = (String) sectionIndexer.getSections()[realPosition];
        if (realPosition + 1 >= sections.length) {
            return PINNED_HEADER_GONE;
        }
        nextSectionLetter = (String) sectionIndexer.getSections()[realPosition + 1];
        if (!sectionLetter.equals(nextSectionLetter)) {
            return PINNED_HEADER_PUSHED_UP;
        }

        return PINNED_HEADER_VISIBLE;
    }

    @Override
    public void configurePinnedHeader(View header, int position, int alpha) {
        if (mCount == 0) {
            return;
        }

        String title = (String) sectionIndexer.getSections()[position];

        ((TextView) header.findViewById(R.id.header_text)).setText(title);
    }

    @Override
    public Object[] getSections() {
        if (sectionIndexer == null) {

            return new String[] { "" };
        } else {
            return sections;
        }
    }

    @Override
    public int getPositionForSection(int section) {
        return section;
    }

    @Override
    public int getSectionForPosition(int position) {
        return position;
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {

    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
        if (view instanceof NmsPinnedHeaderListView) {
            ((NmsPinnedHeaderListView) view).configureHeaderView(firstVisibleItem);
        }
    }

}
