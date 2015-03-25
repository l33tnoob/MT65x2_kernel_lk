/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.activities;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.RelativeLayout;
import android.widget.SectionIndexer;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.AsyncAvatarView;
import com.mediatek.rcse.api.Logger;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * A Adapter act as a super class for other contacts and blacklist Adapter.
 */
public abstract class ContactBaseAdapter extends BaseAdapter implements SectionIndexer {
    protected List<RcsContact> mContactsList = new ArrayList<RcsContact>();
    public static final String TAG = "ContactBaseAdapter";
    protected Context mContext = null;
    protected boolean mSelectAll = false;
    private List<String> mSelectedContactList = new ArrayList<String>();
    private Map<Integer, Integer> mSectionToPosition = new HashMap<Integer, Integer>();
    private String[] mSections = null;
    private static final int INDEX_LETTER_START_INDEX = 0;
    private static final int INDEX_LETTER_LENGTH = 1;

    public void setSelectAll(boolean selectAll) {
        if (selectAll) {
            getSelectedContactList().clear();
            for (RcsContact contact : getContactsList()) {
                contact.mSelected = true;
                getSelectedContactList().add(contact.mDisplayName);
            }
        } else {
            for (RcsContact contact : getContactsList()) {
                contact.mSelected = false;
            }
            getSelectedContactList().clear();
        }
        mSelectAll = selectAll;
    }

    public boolean getSelectAll() {
        return mSelectAll;
    }

    public List<String> getSelectedContactList() {
        return mSelectedContactList;
    }

    public void cleanCheckList() {
        if (mContactsList != null) {
            for (int i = 0; i < mContactsList.size(); i++) {
                if (mContactsList.get(i).mSelected) {
                    mContactsList.get(i).mSelected = false;
                }
            }
        }
    }

    public ContactBaseAdapter(Context context,
            BaseListFragment<? extends ContactBaseAdapter> fragment) {
        super();
        mContext = context;
    }

    @Override
    public int getCount() {
        if (mContactsList != null && mContactsList.size() != 0) {
            return mContactsList.size();
        } else {
            return 0;
        }
    }

    public void setContactsList(List<RcsContact> list) {
        if (mContactsList != null) {
            mContactsList.clear();
            mContactsList.addAll(list);
            initSectionsAndPositions();
        } else {
            Logger.e(TAG, "setContactsList, mContactsList is null!");
        }
    }

    public List<RcsContact> getContactsList() {
        return mContactsList;
    }

    @Override
    public Object getItem(int position) {
        if (mContactsList != null && position >= 0 && position < mContactsList.size()) {
            return mContactsList.get(position);
        } else {
            Logger.w(TAG, "getItem, mContactsList is null or position < 0, " + " position = "
                    + position);
            return null;
        }
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    /**
     * Remove the items from this adaptor
     * 
     * @param obj The items or item that to be removed
     */
    public void removeItems(Object obj) {
        if (null != obj && null != mContactsList && mContactsList.size() != 0) {
            mContactsList.remove(obj);
        }
    }

    public abstract View inflateView(LayoutInflater inflater, ViewGroup parent);

    public abstract ViewHolder onConfiguerItemView(View view, int position, boolean isIndexer);

    public static class ViewHolder {
        public TextView name;
        public TextView number;
        public TextView firstLetter;
        public AsyncAvatarView header;
        public boolean isIndexer;
        public RelativeLayout contactItemLayout;
        public TextView blockedTime;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        boolean isIndexer = false;
        if (mSectionToPosition != null && mSectionToPosition.containsValue(position)) {
            Logger.v(TAG, "Should inflate alpha indexer,position = " + position);
            isIndexer = true;
        }
        if (convertView == null) {
            LayoutInflater inflater = LayoutInflater.from(mContext);
            View tempView = inflateView(inflater, parent);
            configuerItemView(tempView, position, isIndexer);
            return tempView;
        } else {
            ViewHolder holder = (ViewHolder) convertView.getTag();
            if (!isIndexer && holder.header == null) {
                // If it is a alpha indexer before, now it is necessary to
                // inflate a view
                LayoutInflater inflater = LayoutInflater.from(mContext);
                View tempView = inflateView(inflater, parent);
                configuerItemView(tempView, position, isIndexer);
                return tempView;
            } else {
                configuerItemView(convertView, position, isIndexer);
                return convertView;
            }
        }
    }

    protected void configuerItemView(View convertView, int position, boolean isIndexer) {
        ViewHolder holder = onConfiguerItemView(convertView, position, isIndexer);
        Logger.v(TAG, "position = " + position + ",isIndexer = " + isIndexer);
        if (!isIndexer) {
            loadImage(convertView, position, holder);
        }
    }

    private void loadImage(View view, int position, ViewHolder holder) {
        if (view != null) {
            String contactNumber = mContactsList.get(position).mNumber;
            holder.header.setAsyncContact(contactNumber);
        } else {
            Logger.e(TAG, "loadImage() ----view is null");
        }
    }

    @Override
    public int getPositionForSection(int section) {
        Logger.d(TAG, "getPositionForSection() entry ,the section is " + section);
        int position = 0;
        if (null != mSectionToPosition && mSectionToPosition.containsKey(section)) {
            position = mSectionToPosition.get(section);
        } else {
            Logger.w(TAG, "mSectionToPosition is null or do not find section");
        }
        Logger.d(TAG, "getPositionForSection() exit,the position is " + position);
        return position;
    }

    @Override
    public int getSectionForPosition(int position) {
        Logger.d(TAG, "getSectionForPosition() entry,the position is " + position);
        int sectionIndex = 0;
        int size = (mContactsList == null) ? 0 : mContactsList.size();
        if (position > 0 && position <= size) {
            RcsContact item = mContactsList.get(position);
            if (null != item) {
                Logger.d(TAG, "getSectionForPosition(), the item is " + item);
                sectionIndex = item.mSectionIndex;
            } else {
                Logger.w(TAG, "item is null");
            }
        } else {
            Logger.w(TAG, "position is invalid");
        }
        Logger.d(TAG, "getSectionForPosition() exit, the sectionIndex is " + sectionIndex);
        return sectionIndex;
    }

    @Override
    public String[] getSections() {
        String[] returnSection = mSections.clone();
        Logger.d(TAG, "getSections(),mSections = " + mSections.length);
        return returnSection;
    }

    private void initSectionsAndPositions() {
        Logger.v(TAG, "initSectionsAndPositions entry");
        mSectionToPosition.clear();
        int contactSize = mContactsList == null ? 0 : mContactsList.size();
        Logger.v(TAG, "contactSize = " + contactSize);
        ArrayList<String> alphaIndexerList = new ArrayList<String>();
        int alphaIndexerNum = 0;
        for (int pos = 0; pos < contactSize; ++pos) {
            RcsContact contactItem = mContactsList.get(pos);
            String firstLetter =
                    contactItem.mSortKey.trim().substring(INDEX_LETTER_START_INDEX,
                            INDEX_LETTER_LENGTH).toUpperCase();
            if (!alphaIndexerList.contains(firstLetter)) {
                Logger.w(TAG, "firstLetter = " + firstLetter);
                alphaIndexerList.add(firstLetter);

                // add an alpha indexer item to list
                RcsContact alphaIndexerItem = new RcsContact(firstLetter, null);
                alphaIndexerItem.mSectionIndex = alphaIndexerNum;
                alphaIndexerItem.mSortKey = firstLetter;
                mContactsList.add(pos, alphaIndexerItem);
                mSectionToPosition.put(alphaIndexerItem.mSectionIndex, pos);
                contactSize = mContactsList.size();
                Logger.v(TAG, "mContactsList's size changed, contactSize = " + contactSize);
                ++alphaIndexerNum;
            } else {
                contactItem.mSectionIndex = alphaIndexerNum - 1;
                Logger.v(TAG, "The item is exist");
            }
            Logger.v(TAG, " contactItem[" + pos + "].mSortKey = " + contactItem.mSortKey
                    + ",.mSectionIndex = " + contactItem.mSectionIndex);
        }
        mSections = new String[alphaIndexerNum];
        alphaIndexerList.toArray(mSections);
        for (int pos = 0; pos < contactSize; ++pos) {
            Logger.v(TAG, " contactList[" + pos + "].mSortKey = " + mContactsList.get(pos).mSortKey
                    + ",.mSectionIndex = " + mContactsList.get(pos).mSectionIndex);
        }
        Iterator<Map.Entry<Integer, Integer>> iterator = mSectionToPosition.entrySet().iterator();
        Logger.v(TAG, "mSectionToPosition[=");
        for (; iterator.hasNext();) {
            Map.Entry<Integer, Integer> mapEntry = (Map.Entry<Integer, Integer>) iterator.next();
            Logger.v(TAG, "key=" + mapEntry.getKey());
            Logger.v(TAG, "value=" + mapEntry.getValue());
        }
        Logger.v(TAG, "]");
    }
}
