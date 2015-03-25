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

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;

import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.io.File;

/**
 * A activity to choose wall paper from Camera.
 */
public class ChatCameraWallPaperSettingsActivity extends Activity {
    private static final String TAG = "ChatCameraWallPaperSettingsActivity";
    private Uri mCameraWallPaperUri = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWallPaperFromCamera();
    }

    private void getWallPaperFromCamera() {
        Logger.v(TAG, "getWallPaperFromCamera(), RCSE_TEMP_FILE_DIR = "
                + ChatFragment.RCSE_TEMP_FILE_DIR);
        if (FileFactory.createDirectory(ChatFragment.RCSE_FILE_DIR)) {
            Logger.d(TAG, "Create rcse dir success");
        } else {
            Logger.w(TAG, "Create rcse dir failed");
        }
        if (FileFactory.createDirectory(ChatFragment.RCSE_TEMP_FILE_DIR)) {
            Logger.d(TAG, "Create rcse tmp dir success");
        } else {
            Logger.w(TAG, "Create rcse tmp dir failed");
        }
        mCameraWallPaperUri = Uri.fromFile(new File(ChatFragment.RCSE_TEMP_FILE_DIR,
                ChatFragment.RCSE_TEMP_FILE_NAME_HEADER
                        + String.valueOf(System.currentTimeMillis()) + ChatFragment.JPEG_SUFFIX));
        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, mCameraWallPaperUri);
        startActivityForResult(intent, 0);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Logger.d(TAG, "onActivityResult(), requestCode = " + requestCode + ", resultCode = "
                + resultCode + ", data = " + data);
        if (resultCode != Activity.RESULT_OK) {
            finish();
            return;
        }
        onCameraResult();
    }

    private void onCameraResult() {
        String fileNameString = mCameraWallPaperUri.toString();
        Logger.d(TAG, "onCameraResult(), Camera return, fileNameString = " + fileNameString);
        if (fileNameString != null && fileNameString.startsWith(One2OneChatFragment.FILE_SCHEMA)) {
            String fileFullPath = fileNameString.substring(
                    One2OneChatFragment.FILE_SCHEMA.length(), fileNameString.length());
            RcsSettings.getInstance().setChatWallpaper(fileFullPath);
        } else {
            Logger.e(TAG, "fileNameString = " + fileNameString + ",is not start with file://");
        }
        finish();
    }

}
