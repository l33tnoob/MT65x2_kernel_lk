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

import com.mediatek.rcse.api.Logger;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * This is a template class for any kind of cache object
 */
public class Cacher<K extends Object, V extends Object> {
    private static final String TAG = Cacher.class.getSimpleName();

    private final Map<K, TagReference> mTagCache = new ConcurrentHashMap<K, TagReference>();
    private final ReferenceQueue<V> mTagRecycler = new ReferenceQueue<V>();

    /**
     * Cache a value with a key
     * @param key The key
     * @param value The value
     */
    public void putValue(K key, V value) {
        Logger.d(TAG, "putValue() tag: " + value);
        mTagCache.put(key, new TagReference(key, value, mTagRecycler));
        clearUnreachableTag();
    }

    /**
     * Get a value of a key
     * @param key The key
     * @return The value
     */
    public V getValue(K key) {
        clearUnreachableTag();
        WeakReference<V> reference = mTagCache.get(key);
        if (null != reference) {
            V value = reference.get();
            Logger.d(TAG, "getValue() key: " + key + ", value: " + value);
            return value;
        } else {
            Logger.w(TAG, "getValue() tag for key: " + key + " doesn't exist");
            return null;
        }
    }

    private void clearUnreachableTag() {
        TagReference reference = null;
        while (null != (reference = (TagReference) mTagRecycler.poll())) {
            K key = reference.mKey;
            Logger.d(TAG, "clearUnreachableTag () remove unreached tag " + key);
            mTagCache.remove(key);
        }
    }

    private class TagReference extends WeakReference<V> {
        private K mKey = null;
        public TagReference(K k, V r, ReferenceQueue<? super V> q) {
            super(r, q);
            mKey = k;
        }
    }
}
