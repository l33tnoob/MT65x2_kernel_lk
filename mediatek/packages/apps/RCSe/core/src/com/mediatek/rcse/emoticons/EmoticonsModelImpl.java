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

package com.mediatek.rcse.emoticons;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.ImageSpan;

import com.mediatek.rcse.api.Logger;

import com.orangelabs.rcs.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This class defined to manager the emotion icon.
 */
public class EmoticonsModelImpl {
    /**
     * Logger tag
     */
    private static final String TAG = "EmoticonsModelImpl";
    /**
     * EmoticonsModelImpl instance
     */
    private static EmoticonsModelImpl sInstance = null;
    /**
     * Emotion texts resource id.
     */
    public static final int EMOTION_TEXTS = R.array.emotion_codes;
    /**
     * Emotion codes resource id.
     */
    public static final int EMOTION_CODES = R.array.emotion_resid;
    /**
     * Exception for mismatch between resource ID and text.
     */
    private static final String NOT_MATCH_EXCEPTION = "Smiley resource ID/text mismatch";
    /**
     * The average length of each icon.
     */
    private static final int EMTION_LENGTH_AVG = 3;
    /**
     * This map between the emotion code and resource id.
     */
    private final HashMap<String, Integer> mEmoticonsMap;
    /**
     * Get the Resources instance.
     */
    private final Resources mResource;
    /**
     * Emotion icons' texts.
     */
    private final String[] mSmileyTexts;
    /**
     * Patter for all icons.
     */
    private final Pattern mPattern;
    /**
     * The Context.
     */
    private final Context mContext;
    /**
     * Resource ids.
     */
    private final ArrayList<Integer> mResIds = new ArrayList<Integer>();

    /**
     * Private constructor
     * 
     * @param context The Context
     */
    private EmoticonsModelImpl(Context context) {
        mContext = context;
        mResource = mContext.getResources();
        mSmileyTexts = mResource.getStringArray(EMOTION_TEXTS);
        mEmoticonsMap = buildSmileyToRes();
        mPattern = buildPattern();
    }

    /**
     * Initialize an instance.
     * 
     * @param context The context for the instance.
     */
    public static void init(Context context) {
        Logger.d(TAG, "init() entry");
        sInstance = new EmoticonsModelImpl(context);
        Logger.d(TAG, "init() exit");
    }

    /**
     * Get the EmoticonsModelImpl instance
     * 
     * @return EmoticonsModelImpl instance
     */
    public static EmoticonsModelImpl getInstance() {
        return sInstance;
    }

    /**
     * Get the emotion text by specify emotion icon id.
     * 
     * @param emotionIconId The emotion icon id used to query the emotion text.
     * @return The emotion text of specific emotion icon id.
     */
    public String getEmotionCode(int position) {
        Logger.d(TAG, "getEmotionCode() entry, position: " + position);
        String emotionText = null;
        if (position >= 0 && position < mSmileyTexts.length) {
            emotionText = mSmileyTexts[position];
        }
        Logger.d(TAG, "getEmotionCode() exit: " + emotionText);
        return emotionText;
    }

    /**
     * Build emotion icons map.
     * 
     * @return The map of emotion icons.
     */
    private HashMap<String, Integer> buildSmileyToRes() {
        TypedArray resIdArray = mResource.obtainTypedArray(EMOTION_CODES);
        int length = mSmileyTexts.length;
        if (length != resIdArray.length()) {
            throw new IllegalStateException(NOT_MATCH_EXCEPTION);
        }
        HashMap<String, Integer> smileyToRes = new HashMap<String, Integer>(length);
        for (int i = 0; i < length; i++) {
            Integer resourceId = Integer.valueOf(resIdArray.getResourceId(i, 0));
            smileyToRes.put(mSmileyTexts[i], resourceId);
            mResIds.add(resourceId);
        }
        resIdArray.recycle();
        return smileyToRes;
    }

    /**
     * Builds the regular expression we use to find smileys in
     * {@link #addSmileySpans}.
     */
    private Pattern buildPattern() {
        // Set the StringBuilder capacity with the assumption that the average
        // smiley is 3 characters long.
        StringBuilder patternString = new StringBuilder(mSmileyTexts.length * EMTION_LENGTH_AVG);
        // Build a regex that looks like (:-)|:-(|...), but escaping the smilies
        // properly so they will be interpreted literally by the regex matcher.
        patternString.append('(');
        for (String s : mSmileyTexts) {
            patternString.append(Pattern.quote(s));
            patternString.append('|');
        }
        // Replace the extra '|' with a ')'
        patternString.replace(patternString.length() - 1, patternString.length(), ")");
        return Pattern.compile(patternString.toString());
    }

    /**
     * Adds ImageSpans to a CharSequence that replace textual emoticons such as
     * :-) with a graphical version.
     * 
     * @param text A CharSequence possibly containing emoticons
     * @return A CharSequence annotated with ImageSpans covering any recognized
     *         emoticons.
     */
    private CharSequence addSmileySpans(CharSequence text) {
        Logger.d(TAG, "addSmileySpans() entry, text: " + text);
        SpannableStringBuilder builder = new SpannableStringBuilder(text);
        Matcher matcher = mPattern.matcher(text);
        while (matcher.find()) {
            int resId = mEmoticonsMap.get(matcher.group());
            Drawable drawable = mResource.getDrawable(resId);
            int bound = mContext.getResources()
                    .getDimensionPixelOffset(R.dimen.emoticon_bound_size);
            drawable.setBounds(0, 0, bound, bound);
            builder.setSpan(new ImageSpan(drawable), matcher.start(), matcher.end(),
                    Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        Logger.d(TAG, "addSmileySpans() exit, buf: " + builder.toString());
        return builder;
    }

    /**
     * Format message to parse icon.
     * 
     * @param text The text.
     * @return The formatted char sequence.
     */
    public CharSequence formatMessage(String text) {
        Logger.d(TAG, "formatMessage() entry, text: " + text);
        SpannableStringBuilder buf = new SpannableStringBuilder();
        if (!TextUtils.isEmpty(text)) {
            buf.append(addSmileySpans(text));
        }
        Logger.d(TAG, "formatMessage() exit, buf: " + buf.toString());
        return buf;
    }

    /**
     * Get the resource id array
     * 
     * @return The resource id array
     */
    public ArrayList<Integer> getResourceIdArray() {
        return mResIds;
    }
}
