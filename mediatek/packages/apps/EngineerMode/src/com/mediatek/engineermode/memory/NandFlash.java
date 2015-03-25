/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.memory;

import android.app.TabActivity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.xlog.Xlog;

import java.io.IOException;

public class NandFlash extends TabActivity {

    private static final String TAG = "EM/Memory_flash";
    protected static final String FILE_NAND = "/proc/driver/nand";
    protected static final String FILE_MOUNTS = "/proc/mounts";
    protected static final String FILE_EMMC = "/proc/emmc";
    protected static final String FILE_MTD = "/proc/mtd";
    private static final String FILE_CID = "/sys/block/mmcblk0/device/cid";
    private static final String FILE_DUMCHAR_INFO = "/proc/dumchar_info";
    private static final String READ_COMMAND = "cat ";
    private static final String EMMC_ID_HEADER = "emmc ID: ";
    private boolean mHaveEmmc = false;

    private TextView mTvCommInfo = null;
    private TextView mTvFSInfo = null;
    private TextView mTvPartInfo = null;
    private String mCommonTabName = null;
    private String mFileSysTabName = null;
    private String mPartitionTabName = null;
    private String mTabId = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mHaveEmmc = this.getIntent().getBooleanExtra(Memory.FLASH_TYPE, false);
        mCommonTabName = getString(R.string.memory_comm_info);
        mFileSysTabName = getString(R.string.memory_file_sys_info);
        mPartitionTabName = getString(R.string.memory_partition_info);
        TabHost tabHost = getTabHost();
        LayoutInflater.from(this).inflate(R.layout.memory_nand_tabs,
                tabHost.getTabContentView(), true);
        // tab1
        tabHost.addTab(tabHost.newTabSpec(mCommonTabName).setIndicator(
                mCommonTabName).setContent(R.id.comm_info_view));

        // tab2
        tabHost.addTab(tabHost.newTabSpec(mFileSysTabName).setIndicator(
                mFileSysTabName).setContent(R.id.file_sys_view));

        // tab3
        tabHost.addTab(tabHost.newTabSpec(mPartitionTabName).setIndicator(
                mPartitionTabName).setContent(R.id.partition_view));
        mTvCommInfo = (TextView) findViewById(R.id.comm_info);
        mTvFSInfo = (TextView) findViewById(R.id.file_sys_info);
        mTvPartInfo = (TextView) findViewById(R.id.partition_info);
        tabHost.setOnTabChangedListener(new OnTabChangeListener() {
            public void onTabChanged(String tabId) {
                mTabId = tabId;
                showTabContent();
            }
        });

        // init
        mTabId = mCommonTabName;
        showTabContent();
    }

    /**
     * Show TAB content
     */
    private void showTabContent() {
        if (mTabId.equals(mCommonTabName)) {
            if (mHaveEmmc) {
                mTvCommInfo.setText(getEmmcCommon());
            } else {
                mTvCommInfo.setText(getInfo(FILE_NAND));
            }
        } else if (mTabId.equals(mFileSysTabName)) {
            mTvFSInfo.setText(getInfo(FILE_MOUNTS));
        } else if (mTabId.equals(mPartitionTabName)) {
            if (mHaveEmmc) {
                mTvPartInfo.setText(getInfo(FILE_EMMC));
            } else {
                mTvPartInfo.setText(getInfo(FILE_MTD));
            }
        }
    }

    /**
     * Get EMMC common, contains {@link #FILE_CID} and
     * {@link #FILE_DUMCHAR_INFO}
     * 
     * @return EMMC common information
     */
    private String getEmmcCommon() {
        final String emmcId = getInfo(FILE_CID);
        Xlog.v(TAG, "emmcId: " + emmcId);
        StringBuilder sb = new StringBuilder();
        sb.append(EMMC_ID_HEADER);
        sb.append(emmcId);
        sb.append("\n");
        final MmcCid cid = new MmcCid();
        if (cid.parse(emmcId)) {
            sb.append(cid.toString());
            sb.append("\n");
        }
        sb.append("\n");
        sb.append(getInfo(FILE_DUMCHAR_INFO));
        return sb.toString();
    }

    /**
     * Get file content
     * 
     * @param file
     *            The file's path
     * @return The file's content
     */
    private String getInfo(String file) {
        String result = null;
        try {
            int ret = ShellExe.execCommand(READ_COMMAND + file);
            if (0 == ret) {
                result = ShellExe.getOutput();
            } else {
                result = getString(R.string.memory_getinfo_error);
            }
        } catch (IOException e) {
            Xlog.i(TAG, e.toString());
            result = e.toString();
        }
        return result;
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        showTabContent();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    static class MmcCid {
        
        private static final int LENGTH_CID = 32;
        private static final int RADIX_16 = 16;
        private static final int YEAR_BASE = 1997;
        private static final int START_SERIAL = 4;
        private static final int START_OEMID = 26;
        private static final int LENGTH_OEMID = 4;
        private static final int START_MANFID = 30;
        private static final int LENGTH_MANFID = 2;
        private static final int PROD_NAME_LENGTH = 6;
        private static final int LENGTH_SERIAL = 8;
        private static final int START_YEAR = 2;
        private static final int LENGTH_YEAR = 1;
        private static final int START_MONTH = 3;
        private static final int LENGTH_MONTH = 1;
        private static final int START_PRV = 12;
        private static final int LENGTH_PRV = 2;
        private static final int START_NAME = 24;
        private static final int LENGTH_NAME = 2;
        private static final String MANNAME_SANDISK = "sandisk";
        private static final String MANNAME_MICRON = "micron";
        private static final String MANNAME_SAMSUNG = "samsung";
        private static final String MANNAME_HYNIX = "hynix";
        private static final String MANNAME_UNKNOWN = "unknown";
        private static final int MASK_MANFID = 0xFFFF;
        private static final int ID_SANDISK = 0x2;
        private static final int ID_MICRON = 0x13;
        private static final int ID_SAMSUNG = 0x15;
        private static final int ID_HYNIX = 0x90;
        private static final int MASK_PRV = 0xF;
        private static final int BIT_PRV = 4;
        private static final int BIT_MONTH = 8;
        int mManfid = 0;
        char[] mProdName = null;
        String mSerial = null;
        int mOemId = 0;
        int mYear = 0;
        int mPrv = 0;
        // int mHwRev = 0;
        // int mFwRev = 0;
        int mMonth = 0;
        // int mCbox = 0;

        /**
         * Parsing CID string
         * 
         * @param cidStr
         *            CID string
         * @return True if parsing succeed
         */
        public boolean parse(String cidStr) {
            boolean result = false;
            if (null == cidStr || LENGTH_CID != cidStr.length()) {
                result = false;
            } else {
                try {
                    char[] chs = cidStr.toCharArray();
                    mManfid = Integer.parseInt(getSub(chs, START_MANFID,
                            LENGTH_MANFID), RADIX_16);
                    char[] name = new char[PROD_NAME_LENGTH];
                    for (int i = 0; i < name.length; i++) {
                        name[i] = (char) Integer.parseInt(getSub(chs,
                                START_NAME - LENGTH_NAME * i, LENGTH_NAME),
                                RADIX_16);
                    }
                    mProdName = name;
                    mSerial = getSub(chs, START_SERIAL, LENGTH_SERIAL);
                    mOemId = Integer.parseInt(getSub(chs, START_OEMID,
                            LENGTH_OEMID), RADIX_16);
                    mYear = Integer.parseInt(getSub(chs, START_YEAR,
                            LENGTH_YEAR), RADIX_16)
                            + YEAR_BASE;
                    mMonth = Integer.parseInt(getSub(chs, START_MONTH,
                            LENGTH_MONTH), RADIX_16);
                    mPrv = Integer.parseInt(getSub(chs, START_PRV, LENGTH_PRV),
                            RADIX_16);
                    result = true;
                } catch (NumberFormatException e) {
                    Xlog.d(TAG, "parse emmc ID NumberFormatException: "
                            + e.getMessage());
                    result = false;
                } catch (ArrayIndexOutOfBoundsException e) {
                    Xlog.d(TAG,
                            "parse emmc ID ArrayIndexOutOfBoundsException: "
                                    + e.getMessage());
                    result = false;
                }
            }
            return result;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("manfid: ");
            String manname = null;
            switch (mManfid & MASK_MANFID) {
            case ID_SANDISK:
                manname = MANNAME_SANDISK;
                break;
            case ID_MICRON:
                manname = MANNAME_MICRON;
                break;
            case ID_SAMSUNG:
                manname = MANNAME_SAMSUNG;
                break;
            case ID_HYNIX:
                manname = MANNAME_HYNIX;
                break;
            default:
                manname = MANNAME_UNKNOWN;
                break;
            }
            sb.append(manname);
            sb.append("\n");
            sb.append(String.format("OEM/Application ID: 0x%1$04x", mOemId));
            sb.append("\n");
            sb.append(String.format("product name: %s", new String(mProdName)));
            sb.append("\n");
            sb.append(String.format("product revision: %d.%d PRV = 0x%x",
                    mPrv >> BIT_PRV, mPrv & MASK_PRV, mPrv));
            sb.append("\n");
            sb.append(String.format("product serial number: 0x%s", mSerial));
            sb.append("\n");
            sb.append(String.format("manufacturing date: %s/%d MDT = 0x%04x",
                    mMonth, mYear, mMonth << BIT_MONTH | (mYear - YEAR_BASE)));
            return sb.toString();
        }

        /**
         * Get sub string from char array
         * 
         * @param chs
         *            Source char array
         * @param start
         *            Index read from
         * @param length
         *            Read length
         * @return The sub string
         * @throws ArrayIndexOutOfBoundsException
         *             Array is indexed with a value less than zero, or greater
         *             than or equal to the size of the array.
         */
        private String getSub(char[] chs, int start, int length)
                throws ArrayIndexOutOfBoundsException {
            int endIndex = chs.length - start;
            int startIndex = chs.length - start - length;
            StringBuilder sb = new StringBuilder();
            for (int i = startIndex; i < endIndex; i++) {
                sb.append(chs[i]);
            }
            return sb.toString();
        }
    }

}
