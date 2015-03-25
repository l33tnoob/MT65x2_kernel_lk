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
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.widgets.AsyncAvatarView;
import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.R;

/**
 * A Adapter act as blacklist Adapter.
 */
public class BlackListAdapter extends ContactBaseAdapter {
    private static final String TAG = "BlackListAdapter";

    public BlackListAdapter(Context context, BaseListFragment<? extends ContactBaseAdapter> fragment) {
        super(context, fragment);
    }

    @Override
    public View inflateView(LayoutInflater inflater, ViewGroup parent) {
        return inflater.inflate(R.layout.black_list_item, parent, false);
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
        return holder;
    }

    private void infalteContactItemView(View view, final ViewHolder holder, int position) {
        holder.name = (TextView) view.findViewById(R.id.black_list_item_name);
        holder.blockedTime = (TextView) view.findViewById(R.id.black_list_item_time);
        holder.header = (AsyncAvatarView) view.findViewById(R.id.contact_list_item_icon);
        holder.firstLetter = (TextView) view.findViewById(R.id.alpha_indexer);
        holder.firstLetter.setVisibility(View.GONE);
        holder.contactItemLayout = (RelativeLayout) view.findViewById(R.id.black_list_item_layout);
        holder.contactItemLayout.setVisibility(View.VISIBLE);
        view.setTag(holder);
        holder.name.setText(mContactsList.get(position).mDisplayName);
        holder.blockedTime.setText(mContactsList.get(position).getBlockTime());
    }

    private void infalteIndexerItemView(View view, ViewHolder holder, int position) {
        holder.firstLetter = (TextView) view.findViewById(R.id.alpha_indexer);
        holder.firstLetter.setVisibility(View.VISIBLE);
        holder.number = null;
        holder.header = null;
        holder.name = null;
        holder.contactItemLayout = (RelativeLayout) view.findViewById(R.id.black_list_item_layout);
        holder.contactItemLayout.setVisibility(View.GONE);
        view.setTag(holder);
        holder.firstLetter.setText(mContactsList.get(position).mSortKey);
    }

    private void updateContactItemView(View view, final ViewHolder holder, int position) {
        holder.name.setText(mContactsList.get(position).mDisplayName);
        holder.firstLetter.setVisibility(View.GONE);
        holder.contactItemLayout.setVisibility(View.VISIBLE);
    }

    private void updateIndexerItemView(View view, ViewHolder holder, int position) {
        holder.firstLetter.setText(mContactsList.get(position).mSortKey);
        holder.firstLetter.setVisibility(View.VISIBLE);
        holder.contactItemLayout.setVisibility(View.GONE);
    }
}
