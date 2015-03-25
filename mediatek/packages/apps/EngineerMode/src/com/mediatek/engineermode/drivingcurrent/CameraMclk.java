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

package com.mediatek.engineermode.drivingcurrent;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.Spinner;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.cameranew.AutoCalibration;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class CameraMclk extends Activity {

    private static final String TAG = "EM/CameraMCLK";

    private Spinner mCurrentSpinner;
    private Button mSwitchBtn;
    private String mCurrentMCLK = "0";
    private static final String FILE_PATH = "/data/data/com.mediatek.engineermode/sharefile";
    private static final String FILE_NAME = "mclkdriving";
    private File mFile;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_camera);
        mCurrentSpinner = (Spinner) this.findViewById(R.id.desense_camera_spinner);
        mSwitchBtn = (Button) this.findViewById(R.id.desense_camera_button);
        mCurrentMCLK = "0";
        initListeners();
    }

    private void initListeners() {
        mCurrentSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                Elog.i(TAG, "Hopping bit Selected : " + position);
                switch (position) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                        mCurrentMCLK = String.valueOf(position);
                        writeFile();
                        break;
                    case 4:
                        deleteFile();
                        break;
                    default:
                        break;
                }
            }

            public void onNothingSelected(AdapterView<?> parent) {
                Elog.i(TAG, "Hopping bit Selected nothing.");
            }

        });
        mSwitchBtn.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setClass(CameraMclk.this, AutoCalibration.class);
                CameraMclk.this.startActivity(intent);
            }
        });
    }

    private void writeFile() {
        Elog.i(TAG, "writeFile() currentMCLK :" + mCurrentMCLK);
        File dir = new File(FILE_PATH);
        if (!dir.exists() || !dir.isDirectory()) {
            if (!dir.mkdirs()) {
                Elog.i(TAG, "Make dir error!");
            }
        }
        mFile = new File(FILE_PATH, FILE_NAME);
        if (!mFile.exists()) {
            try {
                mFile.createNewFile();
            } catch (IOException e) {
                Elog.w(TAG, "create file error");
            }
        }

        FileOutputStream fileOutputStream = null;
        try {
            fileOutputStream = new FileOutputStream(mFile, false);
            fileOutputStream.write(mCurrentMCLK.getBytes());
            fileOutputStream.close();
        } catch (FileNotFoundException e) {
            Elog.w(TAG, e.toString());
        } catch (IOException e) {
            Elog.w(TAG, e.toString());
        }
    }

    private void deleteFile() {
        mFile = new File(FILE_PATH, FILE_NAME);
        if (mFile.exists()) {
            if (!mFile.delete()) {
                Elog.w(TAG, "delete file error");
            }
        }
        File dir = new File(FILE_PATH);
        if (dir.exists()) {
            if (!dir.delete()) {
                Elog.w(TAG, "delete direct error");
            }
        }
    }

}
