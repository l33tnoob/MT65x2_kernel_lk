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

import com.mediatek.filemanager.utils.FileUtils;
import com.mediatek.filemanager.utils.LogUtils;

import java.text.CollationKey;
import java.text.Collator;
import java.text.RuleBasedCollator;
import java.util.Comparator;


public final class FileInfoComparator implements Comparator<FileInfo> {
    private static final String TAG = "FileInfoComparator";
    /** Create a instance comparator. */
    private static FileInfoComparator sInstance = new FileInfoComparator();

    public static final int SORT_BY_TYPE = 0;
    public static final int SORT_BY_NAME = 1;
    public static final int SORT_BY_SIZE = 2;
    public static final int SORT_BY_TIME = 3;

    /** Used for Hanzi */
    private RuleBasedCollator mCollator = null;

    private int mSortType = 0;

    /**
     * Constructor for FileInfoComparator class.
     */
    private FileInfoComparator() {
    }

    /**
     * This method set the sort mode. 0 means by type, 1 means by name, 2 means by size, 3 means by
     * time.
     * 
     * @param sort sort mode.
     */
    private void setSortType(int sort) {
        mSortType = sort;
        if (mCollator == null) {
            mCollator = (RuleBasedCollator) Collator.getInstance(java.util.Locale.CHINA);
        }
    }

    /**
     * This method get instance of FileInfoComparator.
     * 
     * @param sort sort mode.
     * @return a instance of FileInfoComparator.
     */
    public static FileInfoComparator getInstance(int sort) {
        sInstance.setSortType(sort);
        return sInstance;
    }

    /**
     * This method compares the files based on the order: category folders->common folders->files
     * 
     * @param op the first file
     * @param oq the second file
     * @return a negative integer, zero, or a positive integer as the first file is smaller than,
     *         equal to, or greater than the second file, ignoring case considerations.
     */
    @Override
    public int compare(FileInfo op, FileInfo oq) {
        // if only one is directory
        boolean isOpDirectory = op.isDirectory();
        boolean isOqDirectory = oq.isDirectory();
        if (isOpDirectory ^ isOqDirectory) {
            // one is a folder and one is not a folder
            LogUtils.v(TAG, op.getFileName() + " vs " + oq.getFileName() + " result="
                    + (isOpDirectory ? -1 : 1));
            return isOpDirectory ? -1 : 1;
        }

        switch (mSortType) {
        case SORT_BY_TYPE:
            return sortByType(op, oq);
        case SORT_BY_NAME:
            return sortByName(op, oq);
        case SORT_BY_SIZE:
            return sortBySize(op, oq);
        case SORT_BY_TIME:
            return sortByTime(op, oq);
        default:
            return sortByName(op, oq);
        }
    }

    /**
     * This method compares the files based on their type
     * 
     * @param op the first file
     * @param oq the second file
     * @return a negative integer, zero, or a positive integer as the first file is smaller than,
     *         equal to, or greater than the second file, ignoring case considerations.
     */
    private int sortByType(FileInfo op, FileInfo oq) {
        boolean isOpDirectory = op.isDirectory();
        boolean isOqDirectory = oq.isDirectory();
        if (isOpDirectory && isOqDirectory) {
            boolean isOpCategoryFolder = IconManager.getInstance().isSystemFolder(op);
            boolean isOqCategoryFolder = IconManager.getInstance().isSystemFolder(oq);
            if (isOpCategoryFolder ^ isOqCategoryFolder) {
                // on is category folder and one is normal folder
                LogUtils.i(TAG, op.getFileName() + " - " + oq.getFileName() + " result="
                        + (isOpCategoryFolder ? -1 : 1));
                return isOpCategoryFolder ? -1 : 1;
            }
        }
        if (!isOpDirectory && !isOqDirectory) {
            // both are not directory
            String opExtension = FileUtils.getFileExtension(op.getFileName());
            String oqExtension = FileUtils.getFileExtension(oq.getFileName());
            if (opExtension == null && oqExtension != null) {
                return -1;
            } else if (opExtension != null && oqExtension == null) {
                return 1;
            } else if (opExtension != null && oqExtension != null) {
                if (!opExtension.equalsIgnoreCase(oqExtension)) {
                    return opExtension.compareToIgnoreCase(oqExtension);
                }
            }
        }
        return sortByName(op, oq);
    }

    /**
     * This method compares the files based on their names.
     * 
     * @param op the first file
     * @param oq the second file
     * @return a negative integer, zero, or a positive integer as the first file is smaller than,
     *         equal to, or greater than the second file, ignoring case considerations.
     */
    private int sortByName(FileInfo op, FileInfo oq) {
        CollationKey c1 = mCollator.getCollationKey(op.getFileName());
        CollationKey c2 = mCollator.getCollationKey(oq.getFileName());
        return mCollator.compare(c1.getSourceString(), c2.getSourceString());
    }

    /**
     * This method compares the files based on their sizes
     * 
     * @param op the first file
     * @param oq the second file
     * @return a negative integer, zero, or a positive integer as the first file is smaller than,
     *         equal to, or greater than the second file, ignoring case considerations.
     */
    private int sortBySize(FileInfo op, FileInfo oq) {
        if (!op.isDirectory() && !oq.isDirectory()) {
            long opSize = op.getFileSize();
            long oqSize = oq.getFileSize();
            if (opSize != oqSize) {
                return opSize > oqSize ? -1 : 1;
            }
        }
        return sortByName(op, oq);
    }

    /**
     * This method compares the files based on their modified time
     * 
     * @param op the first file
     * @param oq the second file
     * @return a negative integer, zero, or a positive integer as the first file is smaller than,
     *         equal to, or greater than the second file, ignoring case considerations.
     */
    private int sortByTime(FileInfo op, FileInfo oq) {
        long opTime = op.getFileLastModifiedTime();
        long oqTime = oq.getFileLastModifiedTime();
        if (opTime != oqTime) {
            return opTime > oqTime ? -1 : 1;
        }
        return sortByName(op, oq);
    }
}