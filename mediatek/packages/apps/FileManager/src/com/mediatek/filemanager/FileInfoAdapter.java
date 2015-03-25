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

package com.mediatek.filemanager;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.utils.FileUtils;
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.ThemeUtils;

import java.util.ArrayList;
import java.util.List;


public class FileInfoAdapter extends BaseAdapter {
    private static final String TAG = "FileInfoAdapter";

    private static final int DEFAULT_SECONDARY_SIZE_TEXT_COLOR = 0xff414141;
    private static final int DEFAULT_PRIMARY_TEXT_COLOR = Color.BLACK;
    private static final float CUT_ICON_ALPHA = 0.6f;
    private static final float HIDE_ICON_ALPHA = 0.3f;
    private static final float DEFAULT_ICON_ALPHA = 1f;
    public static final int MODE_NORMAL = 0;
    public static final int MODE_EDIT = 1;
    public static final int MODE_SEARCH = 2;

    private Context mContext;
    private final Resources mResources;
    private final LayoutInflater mInflater;
    private final List<FileInfo> mFileInfoList;
    private final FileInfoManager mFileInfoManager;

    private int mMode = MODE_NORMAL;
    FileManagerService mService = null;

    /**
     * The constructor to construct a FileInfoAdapter.
     * 
     * @param context the context of FileManagerActivity
     * @param fileManagerService the service binded with FileManagerActivity
     * @param fileInfoManager a instance of FileInfoManager, which manages all files.
     */
    public FileInfoAdapter(Context context, FileManagerService fileManagerService,
            FileInfoManager fileInfoManager) {
        mContext = context;
        mResources = context.getResources();
        mInflater = LayoutInflater.from(context);
        mService = fileManagerService;
        mFileInfoManager = fileInfoManager;
        mFileInfoList = fileInfoManager.getShowFileList();
    }

    /**
     * This method gets index of certain fileInfo(item) in fileInfoList
     * 
     * @param fileInfo the fileInfo which wants to be located.
     * @return the index of the item in the listView.
     */
    public int getPosition(FileInfo fileInfo) {
        return mFileInfoList.indexOf(fileInfo);
    }

    /**
     * This method sets the item's check boxes
     * 
     * @param id the id of the item
     * @param checked the checked state
     */
    public void setChecked(int id, boolean checked) {
        FileInfo checkInfo = mFileInfoList.get(id);
        if (checkInfo != null) {
            checkInfo.setChecked(checked);
        }
    }

    /**
     * This method sets all items' check boxes
     * 
     * @param checked the checked state
     */
    public void setAllItemChecked(boolean checked) {
        for (FileInfo info : mFileInfoList) {
            info.setChecked(checked);
        }
        notifyDataSetChanged();
    }

    /**
     * This method gets the number of the checked items
     * 
     * @return the number of the checked items
     */
    public int getCheckedItemsCount() {
        int count = 0;
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                count++;
            }
        }
        return count;
    }

    /**
     * This method gets the list of the checked items
     * 
     * @return the list of the checked items
     */
    public List<FileInfo> getCheckedFileInfoItemsList() {
        List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                fileInfoCheckedList.add(fileInfo);
            }
        }
        return fileInfoCheckedList;
    }

    /**
     * This method gets the first item in the list of the checked items
     * 
     * @return the first item in the list of the checked items
     */
    public FileInfo getFirstCheckedFileInfoItem() {
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                return fileInfo;
            }
        }
        return null;
    }

    /**
     * This method gets the count of the items in the name list
     * 
     * @return the number of the items
     */
    @Override
    public int getCount() {
        return mFileInfoList.size();
    }

    /**
     * This method gets the name of the item at the specified position
     * 
     * @param pos the position of item
     * @return the name of the item
     */
    @Override
    public FileInfo getItem(int pos) {
        return mFileInfoList.get(pos);
    }

    /**
     * This method gets the item id at the specified position
     * 
     * @param pos the position of item
     * @return the id of the item
     */
    @Override
    public long getItemId(int pos) {
        return pos;
    }

    /**
     * This method change all checked items to be unchecked state
     */
    private void clearChecked() {
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                fileInfo.setChecked(false);
            }
        }
    }

    /**
     * This method changes the display mode of adapter between MODE_NORMAL, MODE_EDIT, and
     * MODE_SEARCH
     * 
     * @param mode the mode which will be changed to be.
     */
    public void changeMode(int mode) {
        LogUtils.d(TAG, "changeMode, mode = " + mode);
        switch (mode) {
        case MODE_NORMAL:
            clearChecked();
            break;
        case MODE_SEARCH:
            mFileInfoList.clear();
            break;
        default:
            break;
        }
        mMode = mode;
        notifyDataSetChanged();
    }

    /**
     * This method gets current display mode of the adapter.
     * 
     * @return current display mode of adapter
     */
    public int getMode() {
        return mMode;
    }

    /**
     * This method checks that current mode equals to certain mode, or not.
     * 
     * @param mode the display mode of adapter
     * @return true for equal, and false for not equal
     */
    public boolean isMode(int mode) {
        return mMode == mode;
    }

    /**
     * This method gets the view for each item to be displayed in the list view
     * 
     * @param pos the position of the item
     * @param convertView the view to be shown
     * @param parent the parent view
     * @return the view to be shown
     */
    @Override
    public View getView(int pos, View convertView, ViewGroup parent) {
        LogUtils.d(TAG, "getView, pos = " + pos + ",mMode = " + mMode);
        FileViewHolder viewHolder;
        View view = convertView;

        if (view == null) {
            view = mInflater.inflate(R.layout.adapter_fileinfos, null);
            viewHolder = new FileViewHolder((TextView) view.findViewById(R.id.edit_adapter_name),
                    (TextView) view.findViewById(R.id.edit_adapter_size), (ImageView) view
                            .findViewById(R.id.edit_adapter_img));
            view.setTag(viewHolder);
            // navListItemView.setMinimumHeight(mListItemMinHeight);
        } else {
            viewHolder = (FileViewHolder) view.getTag();
        }

        FileInfo currentItem = mFileInfoList.get(pos);
        viewHolder.mName.setText(currentItem.getShowName());
        viewHolder.mName.setTextColor(DEFAULT_PRIMARY_TEXT_COLOR); // default
        viewHolder.mSize.setTextColor(DEFAULT_SECONDARY_SIZE_TEXT_COLOR);
        view.setBackgroundColor(Color.TRANSPARENT);

        switch (mMode) {
        case MODE_EDIT:
            if (currentItem.isChecked()) {
                view.setBackgroundColor(ThemeUtils.getThemeColor(mContext));
            }
            setSizeText(viewHolder.mSize, currentItem);
            break;
        case MODE_NORMAL:
            setSizeText(viewHolder.mSize, currentItem);
            break;
        case MODE_SEARCH:
            setSearchSizeText(viewHolder.mSize, currentItem);
            break;
        default:
            break;
        }
        setIcon(viewHolder, currentItem,parent.getLayoutDirection());
        
        return view;
    }

    private void setSearchSizeText(TextView textView, FileInfo fileInfo) {
        textView.setText(fileInfo.getShowParentPath());
        textView.setVisibility(View.VISIBLE);
    }

    private void setSizeText(TextView textView, FileInfo fileInfo) {
        if (fileInfo.isDirectory()) {
            if (MountPointManager.getInstance().isMountPoint(fileInfo.getFileAbsolutePath())) {
                StringBuilder sb = new StringBuilder();
                long freeSpace = fileInfo.getFile().getFreeSpace();
                String freeSpaceString = FileUtils.sizeToString(freeSpace);
                long totalSpace = fileInfo.getFile().getTotalSpace();
                String totalSpaces = FileUtils.sizeToString(totalSpace);
                LogUtils.d(TAG, "setSizeText, file name = " + fileInfo.getFileName()
                        + ",file path = " + fileInfo.getFileAbsolutePath());
                LogUtils.d(TAG, "setSizeText, freeSpace = " + freeSpace + ",totalSpace = "
                        + totalSpace);
                
                sb.append(mResources.getString(R.string.free_space)).append(" ");
                sb.append(freeSpaceString).append(" \n");
                sb.append(mResources.getString(R.string.total_space)).append(" ");
                sb.append(totalSpaces).append(" ");
                textView.setText(sb.toString());
                textView.setVisibility(View.VISIBLE);
            } else {
                // it is a directory
                textView.setVisibility(View.GONE);
            }
        } else {
            StringBuilder sb = new StringBuilder();
            sb.append(mResources.getString(R.string.size)).append(" ");
            sb.append(fileInfo.getFileSizeStr());
            textView.setText(sb.toString());
            textView.setVisibility(View.VISIBLE);
        }
    }

    private void setIcon(FileViewHolder viewHolder, FileInfo fileInfo,int viewDirection) {
        Bitmap icon = IconManager.getInstance().getIcon(mResources, fileInfo, mService,viewDirection);

        viewHolder.mIcon.setImageBitmap(icon);
        viewHolder.mIcon.setAlpha(DEFAULT_ICON_ALPHA);
        if (FileInfoManager.PASTE_MODE_CUT == mFileInfoManager.getPasteType()) {
            if (mFileInfoManager.isPasteItem(fileInfo)) {
                viewHolder.mIcon.setAlpha(CUT_ICON_ALPHA);
            }
        }
        if (fileInfo.isHideFile()) {
            viewHolder.mIcon.setAlpha(HIDE_ICON_ALPHA);
        }
    }

    static class FileViewHolder {
        protected TextView mName;
        protected TextView mSize;
        protected ImageView mIcon;

        /**
         * The constructor to construct an edit view tag
         * 
         * @param name the name view of the item
         * @param size the size view of the item
         * @param icon the icon view of the item
         * @param box the check box view of the item
         */
        public FileViewHolder(TextView name, TextView size, ImageView icon) {
            this.mName = name;
            this.mSize = size;
            this.mIcon = icon;
        }
    }
}
