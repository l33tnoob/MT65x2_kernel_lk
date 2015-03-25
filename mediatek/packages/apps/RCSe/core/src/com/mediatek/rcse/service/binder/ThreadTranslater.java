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

package com.mediatek.rcse.service.binder;

import android.text.TextUtils;
import android.util.LruCache;

import com.mediatek.rcse.api.Logger;

import java.util.Map;

/**
 * This class will cache a tag and translate a remote string to be a cached tag
 */
public final class ThreadTranslater {
    private static final String TAG = ThreadTranslater.class.getSimpleName();
    private static final int MAX_CACHE_SIZE = 200;
    private static final LruCache<Long, String> THREAD_CACHE = new LruCache<Long, String>(
            MAX_CACHE_SIZE);
    
    /**
     * Save a threadId and relevant tag  into cache
     * @param threadId The threadId to be cached
     * @param tag The tag to be cached
     */
    public static void saveThreadandTag(Long threadId, String tag) {
        Logger.d(TAG, "saveThreadandTag() entry threadId is " + threadId + " tag is " + tag);
        THREAD_CACHE.put(threadId, tag);
    }

    /**
     * Translate a threadId into a real tag
     * 
     * @param threadId The threadId to be translated
     * @return The real tag
     */
    public static String translateThreadId(Long threadId) {
        Logger.d(TAG, "translateThreadId() entry threadId is " + threadId);
        String result = THREAD_CACHE.get(threadId);
        Logger.d(TAG, "translateThreadId() result is " + result);
        return result;
    }
    
	public static Long translateTag(String tag) {
		Logger.d(TAG, "translateThreadId() entry threadId is " + tag);
		Long threadID = 0L;
		Map<Long, String> map = THREAD_CACHE.snapshot();
		for (Map.Entry<Long, String> e : map.entrySet()) {
			Long key = e.getKey();
			String value = e.getValue();
			if (value.equals(tag))
			{
				threadID =  key;
			}
		}
		return threadID;
	}

    
    /**
     * check if the tag is in the cache
     * 
     * @param tag The tag relevant to group
     * @return boolean if exit
     */
    public static boolean tagExistInCache(String tag) {
        Logger.d(TAG, "tagExistInCache() entry tag is " + tag);
        if (TextUtils.isEmpty(tag)) {
            Logger.d(TAG, "tagExistInCache() tag is null ");
            return false;
        }
        Map<Long, String> map = THREAD_CACHE.snapshot();
        boolean exist = map.containsValue(tag);
        Logger.d(TAG, "tagExistInCache() exist is " + exist);
        return exist;
    }
}
