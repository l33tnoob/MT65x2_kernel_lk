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

public class DemolistAnimation extends Activity {
    private ListView mListView;
    private MyAdapter mMyAdapter;
    public List<String> mListTag = new ArrayList<String>();
    private List<String> mData = new ArrayList<String>();
    private HashMap<String, Object> mImageMap = new HashMap<String, Object>();
    private static final String packageName = "com.mediatek.ngin3d.demo";

    private String[] mAnimationActivity2 = {
            "AspectRatioDemo", "AnimationDemo", "AnimationCloneDemo",
            "CameraAnimationDemo", "Canvas2dDemo", "ContainerRotationDemo1",
            "ContainerRotationDemo2", "DisplayAreaDemo", "DragAnimationDemo",
            "ImplicitAnimationDemo", "PlaneDemo", "RippleDemo", "ScriptDemo",
            "SnowFall", "GLTextureViewDemo",
            "BumpmapDemo"
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
        mImageMap.put("AspectRatioDemo", R.drawable.demo_aspectratiodemo);
        mImageMap.put("AnimationDemo", R.drawable.demo_animationdemo);
        mImageMap.put("AnimationCloneDemo", R.drawable.demo_animationclonedemo);

        mImageMap.put("CameraAnimationDemo", R.drawable.demo_cameraanimationdemo);
        mImageMap.put("Canvas2dDemo", R.drawable.demo_canvas2ddemo);

        mImageMap.put("ContainerRotationDemo1", R.drawable.demo_containerrotationdemo1);
        mImageMap.put("ContainerRotationDemo2", R.drawable.demo_containerrotationdemo2);
        mImageMap.put("DisplayAreaDemo", R.drawable.demo_displayareademo);

        mImageMap.put("DragAnimationDemo", R.drawable.demo_draganimationdemo);
        mImageMap.put("ImplicitAnimationDemo", R.drawable.demo_implicitanimationdemo);
        mImageMap.put("PlaneDemo", R.drawable.demo_planedemo);

        mImageMap.put("RippleDemo", R.drawable.demo_rippledemo);
        mImageMap.put("ScriptDemo", R.drawable.demo_scriptdemo);
        mImageMap.put("SnowFall", R.drawable.demo_snowfall);
        mImageMap.put("GLTextureViewDemo", R.drawable.demo_gltextureviewdemo);

        mImageMap.put("BumpmapDemo", R.drawable.demo_bumpmapdemo);
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

        mListTag.add("Animation");
        mData.add("Animation");
        for (int i = 0; i < mAnimationActivity2.length; i++) {
            mData.add(mAnimationActivity2[i]);
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
            View view;
            if (mListTag.contains(getItem(position))) {

                view = LayoutInflater.from(getContext()).inflate(R.layout.list_item_tag, null);
            } else {

                view = LayoutInflater.from(getContext()).inflate(R.layout.list_item, null);

                if (getImageFromMap(getItem(position)) != null) {

                    Bitmap bmp = readBitMap(this.getContext(),
                            (Integer) getImageFromMap(getItem(position)));

                    ImageView imageView = (ImageView) view.findViewById(R.id.group_list_item_image);
                    imageView.setImageBitmap(bmp);
                }
            }

            TextView textView = (TextView) view.findViewById(R.id.group_list_item_text);
            textView.setText(getItem(position));

            return view;
        }

    }

}
