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

package com.mediatek.rcse.test.emoticons;

import android.content.res.TypedArray;
import android.test.ActivityInstrumentationTestCase2;
import android.test.AndroidTestCase;
import android.text.SpannableStringBuilder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageView;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.emoticons.PageAdapter;
import com.mediatek.rcse.emoticons.PageAdapter.OnEmotionItemSelectedListener;
import com.mediatek.rcse.service.IRegistrationStatus;

import com.orangelabs.rcs.R;

import java.lang.InterruptedException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Defined to test the function of PageAdaptor
 */
public class PageAdaptorTest extends AndroidTestCase {
    private static final String TAG = "PageAdaptorTest";
    private PageAdapter mAdapter = null;
    private ArrayList<Integer> mResIds;
    private HashMap<Integer,OnClickListener> mListenerMap = null;
    private static final int POSITION = 1;
    private int mCurPostion = -1;
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;
    
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        EmoticonsModelImpl.init(getContext());
        mResIds = EmoticonsModelImpl.getInstance().getResourceIdArray();
        mAdapter = new PageAdapter(this.getContext(), mResIds, 0);
        assertNotNull(mAdapter);
    }

    /**
     * Test the registerListener() method
     */
    public void testCase1_registerListener() throws Exception, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase1_registerListener entry");
        OnEmotionItemSelectedListener listener = new OnEmotionItemSelectedListener() {
            public void onEmotionItemSelectedListener(PageAdapter adapter, int position) {

            }
        };
        mAdapter.registerListener(listener);
        Field fieldListener = PageAdapter.class.getDeclaredField("mListener");
        fieldListener.setAccessible(true);
        OnEmotionItemSelectedListener emotionListener = (OnEmotionItemSelectedListener)fieldListener.get(mAdapter);
        assertNotNull(emotionListener);
        Logger.d(TAG, "testCase1_registerListener exit");
    }
    
    /**
     * Test the getItem() method
     */
    public void testCase2_getItem() {
        Logger.d(TAG, "testCase2_getItem entry");
        Integer integer = (Integer) mAdapter.getItem(POSITION);
        assertEquals(mResIds.get(POSITION), integer);
        Logger.d(TAG, "testCase2_getItem exit");
    }
    
    /**
     * Test the onClick() method
     */
    public void testCase3_onClick() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, InterruptedException {
        Logger.d(TAG, "testCase3_onClick entry");
        OnEmotionItemSelectedListener listener = new OnEmotionItemSelectedListener() {
            public void onEmotionItemSelectedListener(PageAdapter adapter, int position) {
                mCurPostion = position;
            }
        };
        assertNotNull(mAdapter);
        mAdapter.registerListener(listener);
        
        Field fieldListenerMap = PageAdapter.class.getDeclaredField("mListenerMap");
        fieldListenerMap.setAccessible(true);
        mListenerMap = (HashMap<Integer,OnClickListener>)fieldListenerMap.get(mAdapter);
        assertNotNull(mListenerMap);
        
        ImageView view = (ImageView) mAdapter.getView(POSITION, null, null);
        assertNotNull(view);
        
        View.OnClickListener clickListener = (View.OnClickListener) mListenerMap.get(Integer
                .valueOf(POSITION));
        assertNotNull(clickListener);
        
        clickListener.onClick(view);
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                if (mCurPostion == POSITION) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
        Logger.d(TAG, "testCase3_onClick exit");
    }
}
