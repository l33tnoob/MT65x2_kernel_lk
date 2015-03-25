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

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.Display;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.EulerOrder;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.StageView;

public class EulerOrderDemo extends Activity {

    private Container mScenario;
    private int mAngleX = 190;
    private int mAngleY = 10;
    private int mAngleZ = 10;
    private Glo3D landscape;
    private Glo3D landscape2;
    private Glo3D landscape3;
    private Glo3D landscape4;
    private Glo3D landscape5;
    private Glo3D landscape6;
    private Glo3D direct_light;
    private MyDemoView mMyDemoView;
    protected Stage mStage;
    private SeekBar mSeekBar1;
    private SeekBar mSeekBar2;
    private SeekBar mSeekBar3;
    private OnSeekBarChangeListener mSeekBarListen1;
    private OnSeekBarChangeListener mSeekBarListen2;
    private OnSeekBarChangeListener mSeekBarListen3;
    private EditText mEdit1;
    private EditText mEdit2;
    private EditText mEdit3;

    private class MyDemoView extends StageView {
        public MyDemoView(Context context) {
            super(context);

        }
    }

    private void initControler() {
        mSeekBar1 = (SeekBar) findViewById(R.id.seekBar1);
        mSeekBar2 = (SeekBar) findViewById(R.id.seekBar2);
        mSeekBar3 = (SeekBar) findViewById(R.id.seekBar3);

        mEdit1 = (EditText) findViewById(R.id.editText1);
        mEdit2 = (EditText) findViewById(R.id.EditText01);
        mEdit3 = (EditText) findViewById(R.id.EditText02);

        mSeekBarListen1 = new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
                fastChangeAngle(arg0.getProgress(), mAngleY, mAngleZ);
                mEdit1.setText("" + arg0.getProgress());

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

        };

        mSeekBarListen2 = new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
                fastChangeAngle(mAngleX, arg0.getProgress(), mAngleZ);
                mEdit2.setText("" + arg0.getProgress());

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

        };

        mSeekBarListen3 = new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
                fastChangeAngle(mAngleX, mAngleY, arg0.getProgress());
                mEdit3.setText("" + arg0.getProgress());

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

        };

        mSeekBar1.setMax(360);
        mSeekBar1.setProgress(mAngleX);
        mSeekBar1.setOnSeekBarChangeListener(mSeekBarListen1);
        mEdit1.setText("" + mAngleX);

        mSeekBar2.setMax(360);
        mSeekBar2.setProgress(mAngleY);
        mSeekBar2.setOnSeekBarChangeListener(mSeekBarListen2);
        mEdit2.setText("" + mAngleY);

        mSeekBar3.setMax(360);
        mSeekBar3.setProgress(mAngleZ);
        mSeekBar3.setOnSeekBarChangeListener(mSeekBarListen3);
        mEdit3.setText("" + mAngleZ);

    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mMyDemoView = new MyDemoView(this);
        mStage = mMyDemoView.getStage();
        setContentView(R.layout.seekbar);

        RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        // RelativeLayout mLayout = (RelativeLayout)
        // getLayoutInflater().inflate(R.layout.seekbar,
        // null);
        LinearLayout lyscol = (LinearLayout) findViewById(R.id.view_container);
        lyscol.addView(mMyDemoView, lp);

        initControler();

        mScenario = new Container();

        // add a directional light to the scene to illuminate the landscape
        direct_light = Glo3D.createFromAsset("direct_light.glo");
        direct_light.setRotation(new Rotation (-120, 20, 0));
        mScenario.add(direct_light);

        landscape = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape);
        landscape.setPosition(new Point(0.25f, 0.14f, 0f, true));
        landscape.setScale(new Scale(10, 10, 10));
        landscape.setRotation(new Rotation(EulerOrder.XYZ, mAngleX, mAngleY, mAngleZ));
        Text orderMode = new Text("XYZ");
        orderMode.setScale(new Scale(0.5f, 0.5f));
        orderMode.setScale(new Scale(0.5f, 0.5f));
        orderMode.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode.setPosition(new Point(0.25f, 0.24f, true));
        mScenario.add(orderMode);

        landscape2 = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape2);
        landscape2.setPosition(new Point(0.54f, 0.14f, 0f, true));
        landscape2.setScale(new Scale(10, 10, 10));
        landscape2.setRotation(new Rotation(EulerOrder.XZY, mAngleX, mAngleY, mAngleZ));
        Text orderMode2 = new Text("XZY");
        orderMode2.setScale(new Scale(0.5f, 0.5f));
        orderMode2.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode2.setPosition(new Point(0.54f, 0.24f, true));
        mScenario.add(orderMode2);

        landscape3 = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape3);
        landscape3.setPosition(new Point(0.83f, 0.14f, 0f, true));
        landscape3.setScale(new Scale(10, 10, 10));
        landscape3.setRotation(new Rotation(EulerOrder.YZX, mAngleX, mAngleY, mAngleZ));
        Text orderMode3 = new Text("YZX");
        orderMode3.setScale(new Scale(0.5f, 0.5f));
        orderMode3.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode3.setPosition(new Point(0.83f, 0.24f, true));
        mScenario.add(orderMode3);

        landscape4 = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape4);
        landscape4.setPosition(new Point(0.25f, 0.39f, 0f, true));
        landscape4.setScale(new Scale(10, 10, 10));
        landscape4.setRotation(new Rotation(EulerOrder.YXZ, mAngleX, mAngleY, mAngleZ));
        Text orderMode4 = new Text("YXZ");
        orderMode4.setScale(new Scale(0.5f, 0.5f));
        orderMode4.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode4.setPosition(new Point(0.25f, 0.49f, true));
        mScenario.add(orderMode4);

        landscape5 = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape5);
        landscape5.setPosition(new Point(0.54f, 0.39f, 0f, true));
        landscape5.setScale(new Scale(10, 10, 10));
        landscape5.setRotation(new Rotation(EulerOrder.ZXY, mAngleX, mAngleY, mAngleZ));
        Text orderMode5 = new Text("ZXY");
        orderMode5.setScale(new Scale(0.5f, 0.5f));
        orderMode5.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode5.setPosition(new Point(0.54f, 0.49f, true));
        mScenario.add(orderMode5);

        landscape6 = Glo3D.createFromAsset("landscape.glo");
        mScenario.add(landscape6);
        landscape6.setPosition(new Point(0.83f, 0.39f, 0f, true));
        landscape6.setScale(new Scale(10, 10, 10));
        landscape6.setRotation(new Rotation(EulerOrder.ZYX, mAngleX, mAngleY, mAngleZ));
        Text orderMode6 = new Text("ZYX");
        orderMode6.setScale(new Scale(0.5f, 0.5f));
        orderMode6.setBackgroundColor(new Color(255, 0, 0, 128));
        orderMode6.setPosition(new Point(0.83f, 0.49f, true));
        mScenario.add(orderMode6);

        mStage.add(mScenario);

    }

    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.euler_order_menu, menu);
        return true;
    }

    private void fastChangeAngle(int x, int y, int z) {
        mAngleX = x;
        mAngleY = y;
        mAngleZ = z;
        landscape.setRotation(new Rotation(EulerOrder.XYZ, mAngleX, mAngleY, mAngleZ));
        landscape2.setRotation(new Rotation(EulerOrder.XZY, mAngleX, mAngleY, mAngleZ));
        landscape3.setRotation(new Rotation(EulerOrder.YZX, mAngleX, mAngleY, mAngleZ));
        landscape4.setRotation(new Rotation(EulerOrder.YXZ, mAngleX, mAngleY, mAngleZ));
        landscape5.setRotation(new Rotation(EulerOrder.ZXY, mAngleX, mAngleY, mAngleZ));
        landscape6.setRotation(new Rotation(EulerOrder.ZYX, mAngleX, mAngleY, mAngleZ));
    }

    private void fastSetSeekBar(int progress) {
        mSeekBar1.setProgress(progress);
        mSeekBar2.setProgress(progress);
        mSeekBar3.setProgress(progress);
        mEdit1.setText("" + progress);
        mEdit2.setText("" + progress);
        mEdit3.setText("" + progress);
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        int item_id = item.getItemId();

        switch (item_id) {
            case R.id.ten:
                fastChangeAngle(0, 0, 0);
                fastSetSeekBar(0);

                break;

            case R.id.twenty:
                fastChangeAngle(30, 30, 30);
                fastSetSeekBar(30);

                break;
            case R.id.thirty:
                fastChangeAngle(60, 60, 60);
                fastSetSeekBar(60);

                break;
            case R.id.forty:
                fastChangeAngle(90, 90, 90);
                fastSetSeekBar(90);

                break;
            default:
                return false;
        }
        return true;
    }
}
