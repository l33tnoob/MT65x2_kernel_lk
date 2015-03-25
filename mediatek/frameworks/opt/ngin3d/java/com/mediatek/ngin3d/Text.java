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

package com.mediatek.ngin3d;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.Layout;
import android.text.StaticLayout;
import com.mediatek.ngin3d.utils.Ngin3dException;
import com.mediatek.ngin3d.utils.JSON;

/**
 * To display text using system fonts.
 */
public class Text extends Canvas2d {

    private Paint.FontMetrics mFontMetrics = mPaint.getFontMetrics();
    private int mTextBackgroundColor;
    private float mFadeOutPoint = 1;
    private static final String THREEDOT = "\u2026";

    /**
     * Initialize a plain text with empty content.
     */
    public Text() {
        this("");
    }

    /**
     * Initialize a plain text with an input string.
     * @param text  a string to be set in this text class
     */
    public Text(String text) {
        this(text, false);
    }

    /**
     * Initialize a plain text with an input string.
     * @param text  a string to be set in this text class
     * @param isYUp   true for creating a Y-up quad, default is Y-down
     */
    public Text(String text, boolean isYUp) {
        super(isYUp);
        setText(text);
        mPaint.setAntiAlias(true);
        setMaterial("ngin3d:textfadequad.mat");
    }

    @Override
    protected void applyBatchValues() {
        // Reset fade out point, the text might be re-configured.
        mFadeOutPoint = 1;
        // Reset font size since it might be auto adjusted,
        mPaint.setTextSize(getValue(PROP_TEXT_SIZE));
        String text = getText();
        int maxWidth = getValue(PROP_TEXT_MAX_WIDTH);
        float width = Math.max(mPaint.measureText(text), 1);
        Canvas canvas;
        // Without text width limitation
        if (maxWidth == 0) {
            canvas = getPresentation().beginDraw((int) width, (int) (-mFontMetrics.top + mFontMetrics.bottom) + 1, mTextBackgroundColor);
            canvas.drawText(text, 0, (int) -mFontMetrics.top, mPaint);
        } else {
            int maxLines = getValue(PROP_TEXT_MAX_LINES);
            autoAdjustFontSize();
            StaticLayout layout = new StaticLayout(text, mPaint, maxWidth, getValue(PROP_TEXT_ALIGNMENT), 1.0f, 1.0f, false);
            // Single-line with text width limitation
            if (width < maxWidth) {
                canvas = getPresentation().beginDraw((int) width, (int) (-mFontMetrics.top + mFontMetrics.bottom) + 1, mTextBackgroundColor);
                canvas.drawText(text, 0, (int) -mFontMetrics.top, mPaint);
            // Multiple-line with text width limitation
            } else {
                // Single line with "..." ellipsize
                if (getValue(PROP_TEXT_SINGLE_LINE) || maxLines == 1) {
                    text = getEllipsizedText(text, maxWidth);
                    canvas = getPresentation().beginDraw((int) mPaint.measureText(text), (int) (-mFontMetrics.top + mFontMetrics.bottom) + 1, mTextBackgroundColor);
                    canvas.drawText(text, 0, (int) -mFontMetrics.top, mPaint);
                // Limited multiple line with "..." ellipsize
                } else if (maxLines > 1 && layout.getLineCount() > maxLines) {
                    String result = getFixedLinesEllipsizedText(maxLines, maxWidth, layout, text);

                    // create a new layout for result String
                    StaticLayout updatedLayout = new StaticLayout(result, mPaint, maxWidth, getValue(PROP_TEXT_ALIGNMENT), 1.0f, 1.0f, false);
                    canvas = getPresentation().beginDraw(updatedLayout.getWidth(), updatedLayout.getHeight(), mTextBackgroundColor);
                    updatedLayout.draw(canvas);
                // Unlimited multiple line and without "..." ellipsize
                } else {
                    canvas = getPresentation().beginDraw(layout.getWidth(), layout.getHeight(), mTextBackgroundColor);
                    layout.draw(canvas);
                }
            }
        }
        getPresentation().setFadeOutCoord(mFadeOutPoint, 1);
        getPresentation().endDraw();
    }

    private String getFixedLinesEllipsizedText(int maxLines, int maxWidth, StaticLayout layout, String text) {
        int offset = layout.getLineStart(maxLines);
        String lineString = getLineString(maxLines, layout, text);

        // If the length of final line string with 3 dot is bigger than maxWidth, discard the latest 2 characters.
        String result = lineString + THREEDOT;
        if ((int) mPaint.measureText(result) > maxWidth) {
            result = text.substring(0, offset - 2) + THREEDOT;
        } else {
            result = text.substring(0, offset) + THREEDOT;
        }
        return result;
    }

    private String getLineString(int line, StaticLayout layout, String text) {
        int offset1 = layout.getLineStart(line - 1);
        int offset2 = layout.getLineStart(line);
        return text.substring(offset1, offset2);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Property

    /**
     * The property of maximum width of this text.
     */
    static final Property<Integer> PROP_TEXT_MAX_LINES = new Property<Integer>("text_max_lines", 0);

    /**
     * The property of maximum width of this text.
     */
    static final Property<Integer> PROP_TEXT_MAX_WIDTH = new Property<Integer>("text_max_width", 0);

    /**
     * The property of maximum lines of this text.
     */
    static final Property<Boolean> PROP_TEXT_SINGLE_LINE = new Property<Boolean>("text_single_line", false);

    /**
     * The property of auto adjust font size of this text. The range is 0 ~ 1.
     */
    static final Property<Layout.Alignment> PROP_TEXT_ALIGNMENT = new Property<Layout.Alignment>("text_size_alignment", Layout.Alignment.ALIGN_NORMAL);

    /**
     * The property of auto adjust font size of this text. The range is 0 ~ 1.
     */
    static final Property<Float> PROP_TEXT_SIZE_ADJUST_THRESHOLD = new Property<Float>("text_size_adjust_threshold", 0.1f);

    /**
     * The property of size of this text.
     */
    static final Property<Float> PROP_TEXT_SIZE = new Property<Float>("text_size", 32.f);
    /**
     * The property of color of this text.
     */
    static final Property<Color> PROP_TEXT_COLOR = new Property<Color>("text_color", Color.WHITE);
    /**
     * The property of background color of this text.
     */
    static final Property<Color> PROP_TEXT_BACKGROUND_COLOR = new Property<Color>("text_background_color", Color.TRANSPARENT);
    /**
     * The property of typeface of this text.
     */
    static final Property<Typeface> PROP_TEXT_TYPEFACE = new Property<Typeface>("text_typeface", Typeface.DEFAULT);
    /**
     * The property of shadow layer of this text.
     */
    static final Property<ShadowLayer> PROP_TEXT_SHADOW_LAYER = new Property<ShadowLayer>("text_shadow_layer", null);
    /**
     * The property of text body of this text.
     */
    static final Property<String> PROP_TEXT = new Property<String>("text", "", PROP_TEXT_SIZE, PROP_TEXT_COLOR,
        PROP_TEXT_TYPEFACE, PROP_TEXT_SHADOW_LAYER, PROP_TEXT_MAX_WIDTH, PROP_TEXT_SINGLE_LINE);

    /** Trim over-long text by fading out the last characters */
    public static final int ELLIPSIZE_BY_FADEOUT = 1;
    /** Trim over-long text by replacing the last characters with ... */
    public static final int ELLIPSIZE_BY_3DOT = 2;

    /**
     * The property of shadow layer of this text.
     */
    static final Property<Integer> PROP_TEXT_ELLIPSIZE_STYLE = new Property<Integer>("text_ellipsize_style", ELLIPSIZE_BY_FADEOUT);

    /**
     * Apply the properties to this text object.
     * @param property  input property type
     * @param value  input property value
     * @return  true if the properties is successfully set.
     */
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property.sameInstance(PROP_TEXT)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_SIZE)) {
            final Float textSize = (Float) value;
            mPaint.setTextSize(textSize);
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_COLOR)) {
            final Color textColor = (Color) value;
            mPaint.setColor(textColor.getRgb());
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_BACKGROUND_COLOR)) {
            final Color textBackgroundColor = (Color) value;
            mTextBackgroundColor = textBackgroundColor.getRgb();
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_TYPEFACE)) {
            final Typeface tf = (Typeface) value;
            mPaint.setTypeface(tf);
            mFontMetrics = mPaint.getFontMetrics();
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_SHADOW_LAYER)) {
            ShadowLayer shadowLayer = (ShadowLayer) value;
            if (shadowLayer == null) {
                mPaint.setShadowLayer(0, 0, 0, 0);
            } else {
                mPaint.setShadowLayer(shadowLayer.radius, shadowLayer.dx, shadowLayer.dy, shadowLayer.color);
            }
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_MAX_LINES)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_MAX_WIDTH)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_SINGLE_LINE)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_SIZE_ADJUST_THRESHOLD)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_ALIGNMENT)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_TEXT_ELLIPSIZE_STYLE)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        }

        return false;
    }

    /**
     * Set maximum width of text.
     * @param width  the maximum width of text.
     */
    public final void setMaxWidth(int width) {
        setValue(PROP_TEXT_MAX_WIDTH, width);
    }

    /**
     * Get maximum width of text.
     * @return  The maximum width of text.
     */
    public final int getMaxWidth() {
        return getValue(PROP_TEXT_MAX_WIDTH);
    }

    /**
     * Set the style of ellipsis (...) - ELLIPSIZE_BY_FADEOUT & ELLIPSIZE_BY_3DOT.
     * @param style  the style of ellipsis.
     */
    public final void setEllipsizeStyle(int style) {
        setValue(PROP_TEXT_ELLIPSIZE_STYLE, style);
    }

    /**
     * Get the style of ellipsis.
     * @return  The style of ellipsis, example ELLIPSIZE_BY_3DOT
     */
    public final int getEllipsizeStyle() {
        return getValue(PROP_TEXT_ELLIPSIZE_STYLE);
    }

    /**
     * Set maximum lines of text.
     * @param lines  the maximum lines of text.
     */
    public final void setMaxLines(int lines) {
        setValue(PROP_TEXT_MAX_LINES, lines);
    }

    /**
     * Get maximum lines of text.
     * @return  The maximum lines of text.
     */
    public final int getMaxLines() {
        return getValue(PROP_TEXT_MAX_LINES);
    }

    /**
     * Set the text as single line. The excess string will be replaced by three dots.
     * @param single  True for single line and false for multiple line.
     */
    public final void setSingleLine(boolean single) {
        setValue(PROP_TEXT_SINGLE_LINE, single);
    }

    /**
     * Get single line status of the text.
     * @return  True for single line and false for multiple line
     */
    public final boolean getSingleLine() {
        return getValue(PROP_TEXT_SINGLE_LINE);
    }

    /**
     * Set the text font size auto adjust threshold.
     * @param threshold  Threshold to trigger font size auto adjust.
     */
    public final void setSizeAdjustThreshold(float threshold) {
        if (threshold > 1 || threshold < 0) {
            throw new Ngin3dException("The threshold of size adjust should be 0 ~ 1");
        }
        setValue(PROP_TEXT_SIZE_ADJUST_THRESHOLD, threshold);
    }

    /**
     * Get the text font size auto adjust threshold.
     * @return  The threshold to adjust font size automatically.
     */
    public final float getSizeAdjustThreshold() {
        return getValue(PROP_TEXT_SIZE_ADJUST_THRESHOLD);
    }

    /**
     * Set the text alignment.
     * @param alignment  ALIGN_CENTER, ALIGN_NORMAL,or ALIGN_OPPOSITE.
     */
    public final void setAlignment(Layout.Alignment alignment) {
        setValue(PROP_TEXT_ALIGNMENT, alignment);
    }

    /**
     * Get the text alignment mode
     * @return  Alignment mode of text, example ALIGN_CENTER
     */
    public final Layout.Alignment getAlignment() {
        return getValue(PROP_TEXT_ALIGNMENT);
    }

    /**
     * Get the text body of this text object.
     * @return  text body.
     */
    public final String getText() {
        return getValue(PROP_TEXT);
    }

    /**
     * Set the text body of this text object.
     * @param text New text string.
     */
    public final void setText(String text) {
        if (text == null) {
            throw new Ngin3dException("The text cannot be null");
        }
        setValue(PROP_TEXT, text);
    }

    /**
     * Get the text size property of this text object.
     * @return  text size.
     */
    public float getTextSize() {
        return getValue(PROP_TEXT_SIZE);
    }

    /**
     * Set the text size property of this text object.
     * @param textSize  the size of text value to be set.
     */
    public void setTextSize(float textSize) {
        setValue(PROP_TEXT_SIZE, textSize);
    }

    /**
     * Set the color of the text.
     * @param color Text color.
     */
    public void setTextColor(Color color) {
        setValue(PROP_TEXT_COLOR, color);
    }

    /**
     * Get the text color.
     * @return  Text color.
     */
    public Color getTextColor() {
        return getValue(PROP_TEXT_COLOR);
    }

    /**
     * Set the background color.
     * @param color  Text background color.
     */
    public void setBackgroundColor(Color color) {
        setValue(PROP_TEXT_BACKGROUND_COLOR, color);
    }

    /**
     * Get the text background color.
     * @return  Text background color.
     */
    public Color getBackgroundColor() {
        return getValue(PROP_TEXT_BACKGROUND_COLOR);
    }

    /**
     * Set the typeface of this text object.
     * @param tf  the typeface to be set.
     */
    public void setTypeface(Typeface tf) {
        setValue(PROP_TEXT_TYPEFACE, tf);
    }

    /**
     * Get the typeface of this text object.
     * @return  the typeface of this object.
     */
    public Typeface getTypeface() {
        return getValue(PROP_TEXT_TYPEFACE);
    }

    /**
     * Text shadow settings.
     */
    public static class ShadowLayer implements JSON.ToJson {
        /**
         * The radius variable of shadow.
         */
        public float radius;
        /**
         * The x displacement variable of shadow.
         */
        public float dx;
         /**
         * The y displacement variable of shadow.
         */
        public float dy;
         /**
         * The color variable of shadow.
         */
        public int color;

        ShadowLayer(float radius, float dx, float dy, int color) {
            this.radius = radius;
            this.dx = dx;
            this.dy = dy;
            this.color = color;
        }

        /**
         * Generate a text description of this object.
         *
         * @return Descriptive text string
         */
        @Override
        public String toString() {
            return "ShadowLayer: radius : " + radius + ", x : " + dx + ", y : " + dy + ", color : " + color;
        }

        /**
         * Generate a text description of this object in JSON format.
         *
         * @return Descriptive text string  in JSON format
         */
        public String toJson() {
            return "{ShadowLayer: {radius : " + radius + ", x : " + dx + ", y : " + dy + ", color : " + color + "}}";
        }
    }

    /**
     * Set the shadow layer of this text object.
     *
     * @param radius  the radius setting for shadow.
     * @param dx  the x displacement of the shadow.
     * @param dy  the y displacement of the shadow.
     * @param color  the color displacement of the shadow.
     */
    public void setShadowLayer(float radius, float dx, float dy, int color) {
        setValue(PROP_TEXT_SHADOW_LAYER, new ShadowLayer(radius, dx, dy, color));
    }

    /**
     * Get the shadow layer of this text object.
     *
     * @return  shadow layer.
     */
    public ShadowLayer getShadowLayer() {
        return getValue(PROP_TEXT_SHADOW_LAYER);
    }

    private void autoAdjustFontSize() {
        String text = getText();
        int maxWidth = getValue(PROP_TEXT_MAX_WIDTH);
        float adjustThreshold = getValue(PROP_TEXT_SIZE_ADJUST_THRESHOLD);
        float fontSize = getValue(PROP_TEXT_SIZE);
        float textWidth;
        int offset;
        do {
            mPaint.setTextSize(fontSize--);
            textWidth = mPaint.measureText(text);
            offset = (int)textWidth % maxWidth;
        } while (textWidth > maxWidth && ((float)offset / (float)maxWidth) < adjustThreshold);
    }

    private String getEllipsizedText(String text, float width) {
        int count = mPaint.breakText(text, true, width, null);
        if (getValue(PROP_TEXT_ELLIPSIZE_STYLE) == ELLIPSIZE_BY_FADEOUT) {
            mFadeOutPoint = 1 - (1.0f / (float)count);
            return text.substring(0, count);
        } else {
            return text.substring(0, count - 2) + THREEDOT;
        }
    }
}
