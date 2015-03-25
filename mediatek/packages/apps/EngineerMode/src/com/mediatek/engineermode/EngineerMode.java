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

package com.mediatek.engineermode;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.PagerTabStrip;
import android.support.v4.view.ViewPager;
import android.view.View;

import com.mediatek.xlog.Xlog;

/**
 * This is main UI of EngineerMode. It uses viewPager to show each classified
 * modules. There are six viewPager, each viewPager is in one category:
 * 1.telephony;
 * 2.connectivity;
 * 3.hardware testing;
 * 4.location;
 * 5.log&debugging;
 * 6.others.
 * RDs can add each module in HomeViewData.java file.
 *
 * @author mtk54034
 * 
 */
public class EngineerMode extends Activity {

    private static final String TAG = "EM/MainView";
    private static final int TAB_COUNT = 6; // Total count of PagerView
    // Define each tabs which will attach to PagerView
    private PrefsFragment mTabs[] = new PrefsFragment[TAB_COUNT];

    // Record each viewPager title string IDs in array:
    private static final int[] TAB_TITLE_IDS = { R.string.tab_telephony,
            R.string.tab_connectivity, R.string.tab_hardware_testing,
            R.string.tab_location, R.string.tab_log_and_debugging,
            R.string.tab_others, };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        final FragmentManager fragmentManager = getFragmentManager();
        final FragmentTransaction transaction = fragmentManager
                .beginTransaction();
        Xlog.v(TAG, "new fregments");
        for (int i = 0; i < TAB_COUNT; i++) {
            mTabs[i] = new PrefsFragment();
            mTabs[i].setResource(i);
            transaction.add(R.id.viewpager, mTabs[i], String.valueOf(i));
            transaction.hide(mTabs[i]);
        }

        ViewPager viewPager;
        PagerTabStrip pagerTabStrip;

        viewPager = (ViewPager) findViewById(R.id.viewpager);
        pagerTabStrip = (PagerTabStrip) findViewById(R.id.pagertitle);
        pagerTabStrip
                .setTabIndicatorColorResource(android.R.color.holo_blue_light);

        transaction.commitAllowingStateLoss();
        fragmentManager.executePendingTransactions();

        MyPagerAdapter pagerAdapter = new MyPagerAdapter();
        viewPager.setAdapter(pagerAdapter);
        viewPager.setCurrentItem(0);

    }

    class MyPagerAdapter extends PagerAdapter {
        private final FragmentManager mFragmentManager;
        private FragmentTransaction mCurTransaction = null;
        private Fragment mCurPrimaryItem;

        MyPagerAdapter() {
            mFragmentManager = getFragmentManager();
        }

        @Override
        public int getCount() {
            return TAB_COUNT;
        }

        @Override
        public void destroyItem(View container, int position, Object object) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            mCurTransaction.hide((Fragment) object);
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return getString(TAB_TITLE_IDS[position]).toString();
        }

        @Override
        public Object instantiateItem(View container, int position) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            Fragment fragment = getFragment(position);
            mCurTransaction.show(fragment);

            // Non primary pages are not visible.
            fragment.setUserVisibleHint(fragment.equals(mCurPrimaryItem));
            return fragment;
        }

        @Override
        public void finishUpdate(View container) {
            if (mCurTransaction != null) {
                mCurTransaction.commitAllowingStateLoss();
                mCurTransaction = null;
                mFragmentManager.executePendingTransactions();
            }
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return ((Fragment) object).getView() == view;
        }

        @Override
        public void setPrimaryItem(View container, int position, Object object) {
            Fragment fragment = (Fragment) object;
            if (!fragment.equals(mCurPrimaryItem)) {
                if (mCurPrimaryItem != null) {
                    mCurPrimaryItem.setUserVisibleHint(false);
                }
                mCurPrimaryItem = fragment;
            }
        }

        private Fragment getFragment(int position) {
            if (position < TAB_COUNT) {
                return mTabs[position];
            }
            throw new IllegalArgumentException("position: " + position);
        }
    }
}
