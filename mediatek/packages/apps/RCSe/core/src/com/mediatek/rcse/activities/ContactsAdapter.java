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
import android.graphics.Color;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AbsListView.OnScrollListener;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.AsyncAvatarView;
import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;

import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.List;

/**
 * A Adapter act as contacts adapter.
 */
public class ContactsAdapter extends ContactBaseAdapter implements OnScrollListener {

    public static final String TAG = "ContactsAdapter";

    public static final int NORMAL_ITEM_COLOR = Color.rgb(255, 255, 255);

    public static final int SELECT_ITEM_COLOR = Color.rgb(53, 145, 168);

    public ContactsAdapter(Context context,
            BaseListFragment<? extends ContactBaseAdapter> contactListFragment) {
        super(context, contactListFragment);
    }

    /**
     * This class defined to handle the operation of querying the real-time
     * capabilities while scrolling the contact list
     */
    static class CapabilityHandler {
        // The singleton CapabilityHandler instance
        private static CapabilityHandler sInstance = null;
        // Capability AsyncTask instance
        private CapabilityTask mCapabilityTask = null;

        // Capability AsyncTask
        private class CapabilityTask extends AsyncTask<List<String>, Void, Boolean> {
            @Override
            protected Boolean doInBackground(List<String>... params) {
                if (null != params[0]) {
                    int count = params[0].size();
                    for (int i = 0; i < count; i++) {
                        queryContactCapabilities(params[0].get(i));
                    }
                    return true;
                } else {
                    Logger.d(TAG, "The contact list is null");
                    return false;
                }
            }
        }

        /**
         * Querying the capabilities of specify contact
         * 
         * @param contact Used to indicate which contact to query
         */
        private void queryContactCapabilities(String contact) {
            Logger.w(TAG, "queryContactCapabilities()");
            if (contact == null) {
                Logger.w(TAG, "The contact is null!");
                return;
            }
            CapabilityApi capabilityApi = ApiManager.getInstance().getCapabilityApi();
            if (capabilityApi == null) {
                Logger.w(TAG, "capabilityApi is null!");
                return;
            }
            if (!capabilityApi.isBinderAlive()) {
                Logger.w(TAG, "capabilityApi is dead!");
                return;
            }
            Logger.v(TAG, "Query the capabilities of " + contact);
            capabilityApi.getContactCapabilities(contact);
        }

        /**
         * Constructor
         */
        private CapabilityHandler() {
        }

        /**
         * Get the singleton CapabilityHandler instance
         * 
         * @return CapabilityHandler instance
         */
        public static synchronized CapabilityHandler getInstance() {
            if (sInstance == null) {
                sInstance = new CapabilityHandler();
            }
            return sInstance;
        }

        /**
         * Cancel current running task
         */
        public synchronized void cancel() {
            Logger.d(TAG, "Cancel current running task");
            if (mCapabilityTask != null) {
                mCapabilityTask.cancel(true);
                mCapabilityTask = null;
            } else {
                Logger.d(TAG, "cancel operation falied because the mCapabilityTask is null");
            }
        }

        /**
         * Start querying the contact capabilities
         */
        public synchronized void start(List<String> contactList) {
            cancel();
            mCapabilityTask = new CapabilityTask();
            mCapabilityTask.execute(contactList);
        }
    }

    @Override
    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,
            int totalItemCount) {
    }

    @Override
    public void onScrollStateChanged(AbsListView view, int scrollState) {
        if (scrollState == OnScrollListener.SCROLL_STATE_IDLE) {
            int firstPos = view.getFirstVisiblePosition();
            int lastPos = view.getLastVisiblePosition();

            RcsContact firstRcsContact = (RcsContact) view.getItemAtPosition(firstPos);
            int sectionIndex = -1;
            if (null != firstRcsContact) {
                sectionIndex = firstRcsContact.mSectionIndex;
            } else {
                Logger.e(TAG,
                        "onScrollStateChanged, firstRcsContact is null, return! sectionIndex = "
                                + sectionIndex);
                return;
            }

            int position = getPositionForSection(sectionIndex);
            Logger.v(TAG, "sectionIndex = " + sectionIndex + ",position = " + position);
            firstPos = view.getFirstVisiblePosition();
            lastPos = view.getLastVisiblePosition();
            Logger.v(TAG, "firstPos = " + firstPos + ",lastPos = " + lastPos + ",the num = "
                    + (lastPos - firstPos) + ",position = " + position);
            List<String> contactList = new ArrayList<String>();
            for (int i = firstPos; i <= lastPos; i++) {
                RcsContact rcsContact = (RcsContact) view.getItemAtPosition(i);
                if (rcsContact != null) {
                    String contact = rcsContact.mNumber;
                    if (contact != null) {
                        Logger.d(TAG, "The contact is " + contact);
                        contactList.add(contact);
                    } else {
                        Logger.d(TAG, "The contact is null");
                    }
                }
            }
            CapabilityHandler.getInstance().start(contactList);
        }
    }

    @Override
    public View inflateView(LayoutInflater inflater, ViewGroup parent) {
        return inflater.inflate(R.layout.contacts_list_item, parent, false);
    }

    @Override
    public ViewHolder onConfiguerItemView(View view, int position, boolean isIndexer) {
        Logger.v(TAG, "onConfiguerItemView(),isIndexer = " + isIndexer + ",position = " + position);
        ViewHolder holder = (ViewHolder) view.getTag();
        if (holder == null) {
            holder = new ViewHolder();
            holder.isIndexer = isIndexer;
            if (!isIndexer) {
                // inflate a contact item
                infalteContactItemView(view, holder, position);
            } else {
                // inflate a indexer item
                infalteIndexerItemView(view, holder, position);
            }
        } else {
            // Need to check whether inflate a view
            if ((holder.isIndexer && isIndexer)) {
                // Set indexer's letter
                updateIndexerItemView(view, holder, position);
                Logger.v(TAG, "indexer's letter view is exist ");
            } else if (!holder.isIndexer && !isIndexer) {
                // Set contact item's display name
                updateContactItemView(view, holder, position);
                Logger.v(TAG, "contact item's view is exist ");

            } else if (!holder.isIndexer && isIndexer) {
                // holder is not a indexer but now the item is indexer
                // inflate a indexer item
                infalteIndexerItemView(view, holder, position);
            } else {
                // holder is a indexer but now the item is not indexer
                // inflate a contact item
                infalteContactItemView(view, holder, position);
            }
            holder.isIndexer = isIndexer;
        }
        if (mContactsList != null) {
            RcsContact contact = mContactsList.get(position);
            if (contact != null) {
                if (contact.mSelected) {
                    view.setBackgroundColor(SELECT_ITEM_COLOR);
                } else {
                    view.setBackgroundColor(NORMAL_ITEM_COLOR);
                }
            } else {
                Logger.e(TAG, "onConfiguerItemView, contact is null!");
            }
        } else {
            Logger.e(TAG, "onConfiguerItemView, mContactsList is null!");
        }
        return holder;
    }

    public void clear() {
        getSelectedContactList().clear();
        cleanCheckList();
    }

    private void infalteContactItemView(View view, ViewHolder holder, int position) {
        holder.name = (TextView) view.findViewById(R.id.contact_list_item_name);
        holder.number = (TextView) view.findViewById(R.id.contact_list_item_number);
        holder.header = (AsyncAvatarView) view.findViewById(R.id.contact_list_item_icon);
        holder.firstLetter = (TextView) view.findViewById(R.id.alpha_indexer);
        holder.firstLetter.setVisibility(View.GONE);
        holder.contactItemLayout = (RelativeLayout) view.findViewById(R.id.contact_item_layout);
        holder.contactItemLayout.setVisibility(View.VISIBLE);
        view.setTag(holder);
        holder.name.setText(mContactsList.get(position).mDisplayName);
        holder.number.setText(mContactsList.get(position).mNumber);
    }

    private void infalteIndexerItemView(View view, ViewHolder holder, int position) {
        holder.firstLetter = (TextView) view.findViewById(R.id.alpha_indexer);
        holder.firstLetter.setVisibility(View.VISIBLE);
        holder.number = null;
        holder.header = null;
        holder.name = null;
        holder.contactItemLayout = (RelativeLayout) view.findViewById(R.id.contact_item_layout);
        holder.contactItemLayout.setVisibility(View.GONE);
        view.setTag(holder);
        holder.firstLetter.setText(mContactsList.get(position).mSortKey);
    }

    private void updateContactItemView(View view, ViewHolder holder, int position) {
        holder.name.setText(mContactsList.get(position).mDisplayName);
        holder.number.setText(mContactsList.get(position).mNumber);
        holder.firstLetter.setVisibility(View.GONE);
        holder.contactItemLayout.setVisibility(View.VISIBLE);
    }

    private void updateIndexerItemView(View view, ViewHolder holder, int position) {
        holder.firstLetter.setText(mContactsList.get(position).mSortKey);
        holder.firstLetter.setVisibility(View.VISIBLE);
        holder.number = null;
        holder.header = null;
        holder.name = null;
        holder.contactItemLayout.setVisibility(View.GONE);
        // Make number null, if it is a indexer
        TextView numberView =
                (TextView) holder.contactItemLayout.findViewById(R.id.contact_list_item_number);
        numberView.setText(null);
    }
}
