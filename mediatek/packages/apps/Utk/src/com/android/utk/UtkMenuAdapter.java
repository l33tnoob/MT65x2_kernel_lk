/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.utk;

import com.android.internal.telephony.cdma.utk.Item;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;

/**
 * Icon list view adapter to show the list of UTK items.
 */
public class UtkMenuAdapter extends ArrayAdapter<Item> {
    private final LayoutInflater mInflater;
    private boolean mIcosSelfExplanatory = false;

    public UtkMenuAdapter(Context context, List<Item> items,
            boolean icosSelfExplanatory) {
        super(context, 0, items);
        mInflater = LayoutInflater.from(context);
        mIcosSelfExplanatory = icosSelfExplanatory;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Item item = getItem(position);

        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.utk_menu_item, parent,
                    false);
        }

        if (!mIcosSelfExplanatory || (mIcosSelfExplanatory && item.icon == null)) {
            ((TextView) convertView.findViewById(R.id.text)).setText(item.text);
        }
        ImageView imageView = ((ImageView) convertView.findViewById(R.id.icon));
        if (item.icon == null) {
            imageView.setVisibility(View.GONE);
        } else {
            imageView.setImageBitmap(item.icon);
            imageView.setVisibility(View.VISIBLE);
        }

        return convertView;
    }
}
