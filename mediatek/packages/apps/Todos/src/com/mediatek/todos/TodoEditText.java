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

package com.mediatek.todos;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Vibrator;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.EditText;

public class TodoEditText extends EditText {
    private static final String TAG = "TodoEditText";

    private static final int LINE_UNDER_SPACING = 5;

    private boolean mDrawLines = false;
    private Rect mRect = null;
    private Paint mPaint = null;
    private int mMinLines = 4;
    private int mLineSpaceHeight = 0;
    private int mLineBottomPadding = 0;

    public TodoEditText(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * This will register a LengthFilter for the EditText. If user input words too much, it will
     * show a Toast to alert user.
     * 
     * @param maxLength
     */
    public void setMaxLength(int maxLength) {
        registerLengthFilter(getContext(), this, maxLength);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        return super.dispatchTouchEvent(event);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        // M: call super.onTouchEvent() first to make opening input method while tap it once works
        boolean result = super.onTouchEvent(event);
        switch (action) {
        case MotionEvent.ACTION_UP:
            ((EditTodoActivity) getContext()).onClickEditText();
            break;
        default:
            LogUtils.d(TAG, "cann't switch action:" + action);
        }
        return result;
    }

    public void setDrawLines(boolean draw) {
        if (mDrawLines != draw) {
            mDrawLines = draw;
            if (mDrawLines) {
                mRect = new Rect();
                mPaint = new Paint();
                mPaint.setColor(getResources().getColor(R.color.LineEditTextColor));
                mMinLines = getResources().getInteger(R.integer.LineEditTextMinLines);
                mLineSpaceHeight = getLineHeight() + LINE_UNDER_SPACING;
                mLineBottomPadding = mLineSpaceHeight >> 2;
            } else {
                mRect = null;
                mPaint = null;
            }
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mDrawLines) {
            drawTextLine(canvas);
        }
        super.onDraw(canvas);
    }

    /**
     * Draw line under text.
     * @param canvas
     */
    private void drawTextLine(Canvas canvas) {
        final int height = getHeight();
        int count = getLineCount();
        int baseLine = 0;
        for (int i = 0; i < count; i++) {
            baseLine = getLineBounds(i, mRect) + LINE_UNDER_SPACING;
            canvas.drawLine(mRect.left, baseLine, mRect.right, baseLine, mPaint);
        }
        for (; count < mMinLines; count++) {
            baseLine += mLineSpaceHeight;
            if (baseLine + mLineBottomPadding <= height) {
                canvas.drawLine(mRect.left, baseLine, mRect.right, baseLine, mPaint);
            } else {
                break;
            }
        }
    }

    public static void registerLengthFilter(final Context context, EditText editText,
            final int maxLength) {
        InputFilter[] filters = new InputFilter[1];
        filters[0] = new InputFilter.LengthFilter(maxLength) {

            public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                    int dstart, int dend) {
                final String destSub = dest.subSequence(dstart, dend).toString();
                final int destLen = getCharacterNum(dest.toString()) - getCharacterNum(destSub);
                final int sourceLen = getCharacterNum(source.toString());
                if (destLen + sourceLen > maxLength) {
                    // show Toast information
                    Utils.prompt(context, R.string.text_full);
                    /// M: add vibration @{
                    Vibrator vibrator = (Vibrator) context
                            .getSystemService(context.VIBRATOR_SERVICE);
                    boolean hasVibrator = vibrator.hasVibrator();
                    if (hasVibrator) {
                        vibrator.vibrate(new long[] { 100, 100 }, -1);
                    }
                    Log.w(TAG, "input out of range,hasVibrator:" + hasVibrator);
                    /// @}
                    if (destLen >= maxLength) {
                        return "";
                    } else {
                        final int keep = getKeepLength(source.toString(), maxLength - destLen);
                        return source.subSequence(start, keep);
                    }
                }
                return source;
            }
        };
        editText.setFilters(filters);
    }

    private static int getCharacterNum(final String content) {
        if (TextUtils.isEmpty(content)) {
            return 0;
        }
        return content.length() + getChineseNum(content);
    };

    private static int getChineseNum(String content) {
        char[] chars = content.toCharArray();
        int num = 0;
        for (int i = 0; i < chars.length; i++) {
            if ((char) (byte) chars[i] != chars[i]) {
                num++;
            }
        }
        return num;
    }

    private static int getKeepLength(final String content, final int max) {
        char[] chars = content.toCharArray();
        int keep = 0;
        int num = 0;
        for (; keep < chars.length; keep++) {
            if ((char) (byte) chars[keep] != chars[keep]) {
                if (num + 2 >= max) {
                    if (num + 2 == max) {
                        keep++;
                    }
                    break;
                }
                num += 2;
            } else {
                num++;
                if (num >= max) {
                    keep++;
                    break;
                }
            }
        }
        return keep;
    }
}
