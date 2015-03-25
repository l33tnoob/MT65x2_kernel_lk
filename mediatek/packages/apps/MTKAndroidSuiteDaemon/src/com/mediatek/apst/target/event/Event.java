/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.apst.target.event;

import java.util.HashMap;
import java.util.Map;

public class Event {
    private Map<String, Object> mArgs;

    /**
     * constructor.
     */
    public Event() {
        mArgs = new HashMap<String, Object>();
    }

    /**
     * @param argumentsCount
     *            The count of the argument.
     */
    public Event(int argumentsCount) {
        mArgs = new HashMap<String, Object>(argumentsCount);
    }

    /**
     * @param key
     *            The key of the argument map.
     * @return The value with key.
     */
    public Object get(String key) {
        return mArgs.get(key);
    }

    /**
     * @param key
     *            The key of the argument map.
     * @return The Integer of key.
     */
    public int getInt(String key) {
        return (Integer) (mArgs.get(key));
    }

    /**
     * @param key
     *            The key of the argument map.
     * @return The String type of the key.
     */
    public String getString(String key) {
        return (String) (mArgs.get(key));
    }

    /**
     * @param key
     *            The key of the argument map.
     * @return The Long type of the key.
     */
    public long getLong(String key) {
        return (Long) (mArgs.get(key));
    }

    /**
     * @param key
     *            The key of the argument map.
     * @return Boolean type of the key.
     */
    public boolean getBoolean(String key) {
        return (Boolean) (mArgs.get(key));
    }

    /**
     * @param key
     *            The key of the argument map to put.
     * @param value
     *            The value with key to put.
     * @return The current event.
     */
    public Event put(String key, Object value) {
        mArgs.put(key, value);
        return this;
    }
}
