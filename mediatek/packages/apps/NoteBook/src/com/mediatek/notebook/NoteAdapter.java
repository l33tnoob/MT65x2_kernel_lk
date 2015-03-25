package com.mediatek.notebook;

import android.content.Context;
import android.content.res.ColorStateList;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.notebook.NotePad.Notes;

import java.util.ArrayList;
import java.util.List;

public class NoteAdapter extends BaseAdapter {

    private static final String TAG = "NoteAdapter";
    private Context mContext;
    public Cursor cur;
    public List<NoteItem> list = new ArrayList<NoteItem>();
    private Resources mResource;
    private ColorStateList mColorWork;
    private ColorStateList mColorPersonal;
    private ColorStateList mColorFamily;
    private ColorStateList mColorStudy;
    private String mGroupWork;
    private String mGroupPersonal;
    private String mGroupFamily;
    private String mGroupStudy;
    private static final int ALPHA_NOTEBOOK = 0X7FFFFFFF;

    public class NoteItem {
        public int id;
        public String note;
        public String create_time;
        public boolean isselect;
        public String notegroup;
        public String modify_time;
    }

    public NoteAdapter(NotesList context, Cursor cursor, int token) {
        mContext = context;
        cur = cursor;
        setDataOfCurrentActivity();
    }

    private void setDataOfCurrentActivity() {
        mResource = mContext.getResources();
        mColorWork = mResource.getColorStateList(R.color.work);
        mColorPersonal = mResource
                .getColorStateList(R.color.personal);
        mColorFamily = mResource
                .getColorStateList(R.color.family);
        mColorStudy = mResource.getColorStateList(R.color.study);
        mGroupWork = mResource.getString(R.string.menu_work);
        mGroupPersonal = mResource.getString(R.string.menu_personal);
        mGroupFamily = mResource.getString(R.string.menu_family);
        mGroupStudy = mResource.getString(R.string.menu_study);
    }

    @Override
    public int getCount() {
        //return cur.getCount();
        return list.size();
    }

    @Override
    public Object getItem(int position) {
        return list.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public static class ViewClass {
        TextView mTitle;
        TextView mCreateTime;
        TextView mGroupColor;
        TextView mNotegroup;
        LinearLayout mNoteItemHole;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ViewClass view;
        if (convertView == null) {
            convertView = LayoutInflater.from(mContext).inflate(
                    R.layout.noteslist_item_context, null);
            view = new ViewClass();
            view.mTitle = (TextView) convertView.findViewById(R.id.title);
            view.mCreateTime = (TextView) convertView
                    .findViewById(R.id.create_time);
            view.mGroupColor = (TextView) convertView
                    .findViewById(R.id.groupcolor);
            view.mNotegroup = (TextView) convertView.findViewById(R.id.group);
            view.mNoteItemHole = (LinearLayout) convertView
                    .findViewById(R.id.linearLayout1);
            convertView.setTag(view);
        } else {
            view = (ViewClass) convertView.getTag();
        }

        NoteItem item = list.get(position);
        convertView.setBackgroundColor(Color.TRANSPARENT);
        if (item.isselect) {
            int defColor = mResource.getColor(R.color.select);
            convertView.setBackgroundColor(getThemeColor(defColor));

        }
        view.mTitle.setText(item.note);
        view.mCreateTime.setText(item.create_time);
        if (item.notegroup.equals(mGroupPersonal)) {
            view.mNotegroup.setTextColor(mColorPersonal);
            view.mGroupColor.setBackgroundResource(R.color.personal);
        } else if (item.notegroup.equals(mGroupWork)) {
            view.mNotegroup.setTextColor(mColorWork);
            view.mGroupColor.setBackgroundResource(R.color.work);
        } else if (item.notegroup.equals(mGroupFamily)) {
            view.mNotegroup.setTextColor(mColorFamily);
            view.mGroupColor.setBackgroundResource(R.color.family);
        } else if (item.notegroup.equals(mGroupStudy)) {
            view.mNotegroup.setTextColor(mColorStudy);
            view.mGroupColor.setBackgroundResource(R.color.study);
        } else {
            view.mGroupColor.setBackgroundResource(R.color.none);
        }
        view.mNotegroup.setText(item.notegroup);
        return convertView;
    }

    public void checkboxClickAction(int position) {
         NoteItem item = list.get(position);
         item.isselect = !item.isselect;
         Notes.sDeleteNum = selectedNumber();
         this.notifyDataSetChanged();      
     }

    public void addList(NoteItem item) {
        list.add(item);
    }
    
    public String getFilter() {
        selectedNumber();
        StringBuilder filter = new StringBuilder("_id in ");
        filter.append("(");
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).isselect) {
                filter.append("\'");
                filter.append(String.valueOf(list.get(i).id));
                filter.append("\'");
                filter.append(",");
            }
        }
        filter.deleteCharAt(filter.length() - 1);
        filter.append(")");
        return String.valueOf(filter);
    }

    public void selectAllOrNoCheckbox(boolean userSelect) {            
        for (int i = 0; i < list.size(); i++) {
            list.get(i).isselect = userSelect;            
        }
        this.notifyDataSetChanged();
        Notes.sDeleteNum = selectedNumber();
    }

    public int selectedNumber() {
        int count = 0;
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).isselect) {
                count++;
            }
        }
        return count;
    }
    
    private int getThemeColor(int defColor) {
        int color = defColor;
        //if (FeatureOption.MTK_THEMEMANAGER_APP) {
        //    int themeColor = mContext.getResources().getThemeMainColor();
        //    if (0 != themeColor) {
        //        color = themeColor;
        //        color = color & ALPHA_NOTEBOOK;
        //    }
        //}
        return color;
    }
}
