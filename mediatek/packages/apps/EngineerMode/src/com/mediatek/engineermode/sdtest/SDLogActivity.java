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

package com.mediatek.engineermode.sdtest;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Environment;
import android.os.StatFs;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;
import android.widget.ToggleButton;
import android.widget.AdapterView.OnItemClickListener;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Vector;

/**
 * Description: Test SD card by constantly reading and writing,
 * 
 * @author mtk54043
 * 
 */
public class SDLogActivity extends Activity implements OnItemClickListener, DialogInterface.OnClickListener {

    private static final String TAG = "SD Log";

    private boolean mState;
    private boolean mThreadState = true;
    private int mFileCount = 0;

    private Vector<String> mFileList;
    private Random mRandom;
    private ToggleButton mToggleButton;
    private ActionThread mThread;
    private ListView mLVChooseStorage;
    private StorageManager mStorageManager;
    private int mSelectedIndex;

    private static final int FILECOUNT_MAX = 200;
    private static final int OPERATOR_TYPE = 3;
    private static final int AVAILABLESPACE = 4193304;
    private static final int COUNT = 10;
    private static final int PRE_FILE_SIZE = 256;

    private static final int SHORT_TIME = 50;
    private static final int LONG_TIME = 500;

    private static final String FODERNAME = "EM_SDLog";
    private static final String FILENAME = "EM_SDLOG_TESTFILE";
    private static final String MAP_KEY_ITEM_TITLE = "item_title";
    private static final String MAP_KEY_ITEM_DESCR = "item_descr";
    private static final String NO_AVAILABLE_STORAGE = "No Available Storage";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_sdlog_activity);
        mToggleButton = (ToggleButton) findViewById(R.id.desense_sdlog_toggle_btn);
        mToggleButton.setOnClickListener(new ButtonClickListener());
        mStorageManager = (StorageManager)getSystemService(Context.STORAGE_SERVICE);
        String firstStorageTag = getFirstAvailStorageTag();
        if (NO_AVAILABLE_STORAGE.equals(firstStorageTag)) {
            Toast.makeText(this, NO_AVAILABLE_STORAGE, Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        mSelectedIndex = 0;
        mLVChooseStorage = (ListView)findViewById(R.id.sdtest_choose_storage);
        mLVChooseStorage.setAdapter(constructAdapter(firstStorageTag));
        mLVChooseStorage.setOnItemClickListener(this);
        mFileList = new Vector<String>();
        mRandom = new Random();
        checkSDCard();
        createFileForder();
        mThread = new ActionThread();
    }
    
    private String getFirstAvailStorageTag() {
        List<String> list = getAvailStorageTags();
        if (list.size() > 0) {
            return list.get(0);
        } else {
            return NO_AVAILABLE_STORAGE;
        }
    }
    
    private List<String> getAvailStorageTags() {
        List<String> list = new ArrayList<String>();
        StorageVolume[] volumes = mStorageManager.getVolumeList();
        for (int i = 0; i < volumes.length; i++) {
            String path = volumes[i].getPath();
            String state = mStorageManager.getVolumeState(path);
            //Elog.d(TAG, "getFirstAvailStorage - Path: " + path + " State: " + state);
            if (state.equals(Environment.MEDIA_MOUNTED)) {
                list.add(volumes[i].getDescription(this));
            }
        }
        return list;
    }
    
    private StorageVolume getMountedVolumeById(int index) {
        StorageVolume[] volumes = mStorageManager.getVolumeList();
        int mountedIndx = 0;
        for (int i = 0; i < volumes.length; i++) {
            String path = volumes[i].getPath();
            String state = mStorageManager.getVolumeState(path);
            if (state.equals(Environment.MEDIA_MOUNTED)) {
                if (mountedIndx == index) {
                    return volumes[i];
                }
                mountedIndx++;
            }
        }
        return null;
    }
    
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        switch (parent.getId()) {
        case R.id.sdtest_choose_storage:
            if (position == 0) {
                if (mToggleButton.isChecked()) {
                    Toast.makeText(this, getString(R.string.sd_test_stop_test_first), 
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                showSingleChoiceDialog(getString(R.string.sd_test_choose_storage), 
                        getAvailStorageTags(),
                        mSelectedIndex,
                        this);
            }
            break;
        default:
            Elog.w(TAG, "Unknown view id:" + view.getId());
        }
    }
    
    @Override
    public void onClick(DialogInterface dialog, int which) {
        mSelectedIndex = which;
        mLVChooseStorage.setAdapter(constructAdapter(getAvailStorageTags().get(mSelectedIndex)));
        dialog.dismiss();
    }
    
    private void showSingleChoiceDialog(String title, List<String> itemList, 
            int checkedId, DialogInterface.OnClickListener listener) {
        CharSequence[] items = new CharSequence[itemList.size()];
        for (int i = 0; i < items.length; i++) {
            items[i] = itemList.get(i);
        }
        AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                false).setTitle(title).setSingleChoiceItems(items, checkedId, listener).
                setNegativeButton(android.R.string.cancel, null).create();
        dialog.show();
    }
    
    private ListAdapter constructAdapter(String descr) {
        List<Map<String, String>> list = new ArrayList<Map<String, String>>();
        Map<String, String> itemMap = new HashMap<String, String>();
        itemMap.put(MAP_KEY_ITEM_TITLE, getString(R.string.sd_test_choose_storage));
        itemMap.put(MAP_KEY_ITEM_DESCR, descr);
        list.add(itemMap);
        SimpleAdapter adapter = new SimpleAdapter(this, list, 
                R.layout.em_simple_list_item_layout,
                new String[]{MAP_KEY_ITEM_TITLE, MAP_KEY_ITEM_DESCR},
                new int[]{R.id.em_list_item_title, R.id.em_list_item_descr});
        return adapter;
    }

    @Override
    protected void onStop() {
        super.onStop();
        Elog.i(TAG, "DesenceSDLogActivity onStop()");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        init();
    }

    @Override
    protected void onDestroy() {
        mState = false;
        mThreadState = false;
        emptyForder(true);
        Elog.i(TAG, "DesenceSDLogActivity onDestroy()");
        super.onDestroy();
    }

    private void init() {
        mState = false;
        mThreadState = true;
        mFileCount = 0;
        mFileList.clear();
        mFileList = new Vector<String>();
        mRandom = new Random();
        mThread = new ActionThread();
    }

    private class ButtonClickListener implements View.OnClickListener {

        public void onClick(View v) {
            if (v.getId() == mToggleButton.getId()) {
                mToggleButton.setEnabled(false);
                if (mToggleButton.isChecked()) {
                    mState = true;
                    createFileForder();
                    if (!mThread.isAlive()) {
                        mThread.start();
                    }
                    Elog.i(TAG, "mSDLogToggleButton is checked");
                } else {
                    mState = false;
                    Elog.i(TAG, "mSDLogToggleButton is unchecked");
                }
                mToggleButton.setEnabled(true);
            }
        }
    }

    private void checkSDCard() {
        if (!isSdMounted()) {
            new AlertDialog.Builder(this).setTitle("Warning!").setMessage("Please insert SD card!").setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            finish();
                        }
                    }).create().show();
        } else if (!isSdWriteable()) {
            new AlertDialog.Builder(this).setTitle("Warning!").setMessage("SD card isn't writeable!").setPositiveButton(
                    "OK", new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            finish();
                        }
                    }).create().show();
        } else if (getSdAvailableSpace() < AVAILABLESPACE) {
            new AlertDialog.Builder(this).setTitle("Warning!").setMessage("SD card space < 4M!").setPositiveButton("OK",
                    new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                            finish();
                        }
                    }).create().show();
        }
    }

    private void emptyForder(boolean isDeleteForder) {
        File testForder = new File(getSdPath() + File.separator + FODERNAME);
        if (testForder.exists() && testForder.isDirectory()) {
            File[] fileList = testForder.listFiles();
            if (null != fileList) {
                for (File file : fileList) {
                    if (file.exists()) {
                        file.delete();
                        Elog.v(TAG, "Delete File :" + file.getPath());
                    }
                }
            }
            if (isDeleteForder) {
                testForder.delete();
            }
        }
    }

    private void createFileForder() {
        if (isSdMounted()) {
            File testForder = new File(getSdPath() + File.separator + FODERNAME);
            if (!testForder.exists()) {
                testForder.mkdir();
                Elog.i(TAG, "createFileForder: " + testForder.getPath());
            }
        }
    }

    private class ActionThread extends Thread {

        @Override
        public void run() {
            createAndWriteFile();
            while (true) {
                if (!mThreadState) {
                    break;
                }
                
                if (!isSdMounted()) {
                    SDLogActivity.this.runOnUiThread(new Runnable(){
                        @Override
                        public void run() {
                            mToggleButton.setChecked(false);
                            mSelectedIndex = 0;
                            mLVChooseStorage.setAdapter(constructAdapter(getAvailStorageTags().get(mSelectedIndex)));
                            Toast.makeText(SDLogActivity.this, 
                                    getString(R.string.sd_test_sd_removed), 
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
                    mState = false;
                }

                if (mFileCount >= FILECOUNT_MAX) {
                    emptyForder(false);
                    mFileList.clear();
                    mFileCount = 0;
                    createAndWriteFile();
                    Elog.w(TAG, "mFileCount > 200 , empty forder.");
                }

                if (mState) {
                    switch (getRandom(OPERATOR_TYPE)) {
                    case 0:
                        createAndWriteFile();
                        break;
                    case 1:
                        readFile();
                        break;
                    case 2:
                        deleteFile();
                        break;
                    default:
                        break;
                    }
                    try {
                        Thread.sleep(SHORT_TIME);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                } else {
                    Elog.i(TAG, "mThread : mState == false");
                    try {
                        Thread.sleep(LONG_TIME);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private void createAndWriteFile() {
        if (getSdAvailableSpace() < AVAILABLESPACE) {
            emptyForder(false);
        }
        if (isSdWriteable()) {
            File testFile = new File(getSdPath() + File.separator + FODERNAME + File.separator + FILENAME + mFileCount);
            if (!testFile.exists()) {
                try {
                    testFile.createNewFile();
                    Elog.i(TAG, "CreateAndWriteFile :" + testFile.getPath());
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            mFileList.add(FILENAME + mFileCount);
            mFileCount++;
            FileOutputStream outputStream = null;
            try {
                outputStream = new FileOutputStream(testFile);
                try {
                    for (int i = 0; i < COUNT; i++) {
                        outputStream.write(SDLOG_TEXT.getBytes());
                    }
                    outputStream.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    try {
                        if (null != outputStream) {
                            outputStream.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    private void deleteFile() {
        if (mFileList.size() > 0) {
            File deleteFile = new File(getSdPath() + File.separator + FODERNAME + File.separator
                    + mFileList.get(getRandom(mFileList.size())));
            Elog.i(TAG, "deleteFile: " + deleteFile.getPath());
            if (deleteFile.exists()) {
                deleteFile.delete();
                mFileList.remove(deleteFile.getName());
            } else {
                Elog.w(TAG, "deleteFile doesn't exist!");
            }
        } else {
            createAndWriteFile();
        }
    }

    private void readFile() {
        if (mFileList.size() > 0) {
            File readFile = new File(getSdPath() + File.separator + FODERNAME + File.separator
                    + mFileList.get(getRandom(mFileList.size())));
            Elog.i(TAG, "readFile: " + readFile.getPath());
            if (readFile.exists()) {
                FileInputStream inputStream = null;
                try {
                    inputStream = new FileInputStream(readFile);
                    byte[] buffer = new byte[PRE_FILE_SIZE];
                    try {
                        int len = inputStream.read(buffer);
                        while (len != -1) {
                            len = inputStream.read(buffer);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        try {
                            if (null != inputStream) {
                                inputStream.close();
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            } else {
                Elog.w(TAG, "readFile doesn't exist!");
            }
        } else {
            createAndWriteFile();
        }
    }

    private int getRandom(int count) {
        if (count <= 0) {
            return 0;
        }

        return mRandom.nextInt(count);
    }

    private boolean isSdMounted() {
        // if (Environment.MEDIA_MOUNTED.equals(Environment
        // .getExternalStorageState())
        // || Environment.MEDIA_MOUNTED_READ_ONLY.equals(Environment
        // .getExternalStorageState())) {
        // return true;
        // } else {
        // return false;
        // }

        //return (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) 
        //        || Environment.MEDIA_MOUNTED_READ_ONLY.equals(Environment.getExternalStorageState()));
        String state;
        state = mStorageManager.getVolumeState(getMountedVolumeById(mSelectedIndex).getPath());
        return (Environment.MEDIA_MOUNTED.equals(state) 
                        || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state));
    }

    // private static boolean isSdReadable() {
    // return isSdMounted();
    // }

    private boolean isSdWriteable() {
        //return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
        String state;
        state = mStorageManager.getVolumeState(getMountedVolumeById(mSelectedIndex).getPath());
        return Environment.MEDIA_MOUNTED.equals(state);
    }

    private String getSdPath() {
        //return Environment.getExternalStorageDirectory().getPath();
        StorageVolume sv = getMountedVolumeById(mSelectedIndex);
        if (sv != null) {
            return sv.getPath();
        } 
        return null;
    }

    private long getSdAvailableSpace() {
        if (isSdMounted()) {
            String sdcard = getSdPath();
            StatFs statFs = new StatFs(sdcard);
            long availableSpace = (long) statFs.getBlockSize() * statFs.getAvailableBlocks();

            return availableSpace;
        } else {
            return -1;
        }
    }

    private static final String SDLOG_TEXT = "Copyright Statement:This software/firmware"
            + " and related documentation MediaTek Softwareare* protected under relevant "
            + "copyright laws. The information contained herein* is confidential and proprietary"
            + " to MediaTek Inc. and/or its licensors.* Without the prior written permission of "
            + "MediaTek inc. and/or its licensors,* any reproduction, modification, use or "
            + "disclosure of MediaTek Software,* and information contained herein, in whole "
            + "or in part, shall be strictly prohibited. MediaTek Inc. (C) 2010. All rights "
            + "reserved** BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES "
            + "AND AGREES* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (MEDIATEK SOFTWARE)"
            + "* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON* "
            + "AN AS-IS BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,* EXPRESS"
            + " OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED OF* MERCHANTABILITY,"
            + " FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.* NEITHER DOES MEDIATEK PROVIDE"
            + " ANY WARRANTY WHATSOEVER WITH RESPECT TO THE* SOFTWARE OF ANY THIRD PARTY WHICH MAY "
            + "BE USED BY, INCORPORATED IN, OR* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER "
            + "AGREES TO LOOK ONLY TO SUCH* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. "
            + "RECEIVER EXPRESSLY ACKNOWLEDGES* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN"
            + " FROM ANY THIRD PARTY ALL PROPER LICENSES* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK "
            + "SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEKSOFTWARE RELEASES MADE TO RECEIVER'S "
            + "SPECIFICATION OR TO CONFORM TO A PARTICULARSTANDARD OR OPEN FORUM. RECEIVER'S SOLE "
            + "AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE ANCUMULATIVE LIABILITY WITH RESPECT TO "
            + "THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, TO REVISE OR "
            + "REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY LICENSE FEES OR SERVICE"
            + " CHARGE PAID BY RECEIVER TOMEDIATEK FOR SUCH MEDIATEK  AT ISSUE.The following"
            + " software/firmware and/or related documentation have been modified"
            + " by MediaTek Inc. All revisions are subject to any receiver'sapplicable license "
            + "agreements with MediaTek Inc.";
}
