/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ngin3d.demo;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class DemolistBasic extends Activity {
    private ListView mListView;
    private MyAdapter mMyAdapter;
    public List<String> mListTag = new ArrayList<String>();
    private List<String> mData = new ArrayList<String>();
    private HashMap<String, Object> mImageMap = new HashMap<String, Object>();
    private static final String packageName = "com.mediatek.ngin3d.demo";

    private String[] mBasicActivity1 = {
            "EmptyStage", "BitmapFontDemo", "HelloCaster", "LongStringDemo", "MipmapDemo",
            "RotationDemo", "TextDemo", "CuboidDemo",
            "SphereDemo", "VideoTextureDemo", "ScaleRotationDemo", "StereoSpace3DDemo",
            "Space3DDemo", "LayerDemo", "RenderTargetDemo"
    };

    /***
     * this method to avoid loading the picture from Resource caused by OOM
     *
     * @param context
     * @param resId
     * @return Bitmap
     */
    public static Bitmap readBitMap(Context context, int resId) {
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inPreferredConfig = Bitmap.Config.RGB_565;
        opt.inPurgeable = true;
        opt.inInputShareable = true;

        InputStream is = context.getResources().openRawResource(resId);
        return BitmapFactory.decodeStream(is, null, opt);
    }

    private void initImageMap() {
        mImageMap.clear();
        mImageMap.put("EmptyStage", R.drawable.demo_emptystage);
        mImageMap.put("BitmapFontDemo", R.drawable.demo_bitmapfontdemo);
        mImageMap.put("HelloCaster", R.drawable.demo_hellocaster);

        mImageMap.put("LongStringDemo", R.drawable.demo_hellongin3d);
        mImageMap.put("MipmapDemo", R.drawable.demo_mipmapdemo);
        mImageMap.put("RotationDemo", R.drawable.demo_rotationdemo);

        mImageMap.put("TextDemo", R.drawable.demo_textdemo);

        mImageMap.put("CuboidDemo", R.drawable.demo_cuboiddemo);
        mImageMap.put("SphereDemo", R.drawable.demo_space3ddemoapp);
        mImageMap.put("VideoTextureDemo", R.drawable.demo_videotexturedemo);

        mImageMap.put("ScaleRotationDemo", R.drawable.demo_scalerotationdemoapp);
        mImageMap.put("StereoSpace3DDemo", R.drawable.demo_stereo3ddemoapp);
        mImageMap.put("Space3DDemo", R.drawable.demo_space3ddemoapp);
        mImageMap.put("LayerDemo", R.drawable.demo_layerdemo);
        mImageMap.put("RenderTargetDemo", R.drawable.demo_rendertarget);

    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mListView = (ListView) findViewById(R.id.list);
        mMyAdapter = new MyAdapter(this,
                android.R.layout.simple_expandable_list_item_1, getData());

        mListView.setAdapter(mMyAdapter);

        initImageMap();

        mListView.setOnItemClickListener(new ListView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
                try {
                    startActivity(new Intent(getBaseContext(), Class.forName(packageName + "."
                            + mData.get(position))));
                } catch (ClassNotFoundException e) {

                    e.printStackTrace();
                }

            }
        });

    }

    private Object getImageFromMap(Object key) {

        return mImageMap.get(key);
    }

    private List<String> getData() {

        mData.add("Basic");
        mListTag.add("Basic");
        for (int i = 0; i < mBasicActivity1.length; i++) {
            mData.add(mBasicActivity1[i]);
        }

        return mData;
    }

    class MyAdapter extends ArrayAdapter<String> {

        public MyAdapter(Context context, int textViewResourceId,
                List<String> objects) {
            super(context, textViewResourceId, objects);

        }

        @Override
        public boolean areAllItemsEnabled() {
            return false;
        }

        @Override
        public boolean isEnabled(int position) {

            return !mListTag.contains(getItem(position));
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;

            if (mListTag.contains(getItem(position))) {

                view = LayoutInflater.from(getContext()).inflate(R.layout.list_item_tag, null);
            } else {

                view = LayoutInflater.from(getContext()).inflate(R.layout.list_item, null);

                if (getImageFromMap(getItem(position)) != null) {

                    Bitmap bmp = readBitMap(this.getContext(),
                            (Integer) getImageFromMap(getItem(position)));
                    // int resId = (Integer) getImageFromMap(getItem(position));
                    ImageView imageView = (ImageView) view.findViewById(R.id.group_list_item_image);
                    imageView.setImageBitmap(bmp);
                    /*
                     * if(!bmp.isRecycled() ){ bmp.recycle(); System.gc(); }
                     */
                    // imageView.setImageResource(resId);
                    // imageView.setBackgroundResource(resId);
                }

            }

            TextView textView = (TextView) view.findViewById(R.id.group_list_item_text);
            textView.setText(getItem(position));

            return view;
        }

    }

}
