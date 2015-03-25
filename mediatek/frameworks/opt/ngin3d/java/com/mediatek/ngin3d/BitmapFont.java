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

import android.content.res.Resources;
import com.mediatek.ngin3d.utils.Ngin3dException;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.StringTokenizer;

/**
 * Bitmap font for use in bitmap text.
 * This must be initialized before being used.
 */
public class BitmapFont {

    private final int mResId;
    private final Resources mResources;
    private final HashMap<Character, CharacterInfo> mCharMap = new HashMap<Character, CharacterInfo>();
    private int mScaleH;
    private int mWordWidth;

    /**
     * Initialize a bitmap font style with FNT and image file.
     *
     * @param resources Android resource
     * @param settingId Setting file resource id
     * @param resId     Image resource id.
     */
    public BitmapFont(Resources resources, int settingId, int resId) {
        mResId = resId;
        mResources = resources;
        initialize(resources.openRawResource(settingId));
    }

    /**
     * Text character data.
     */
    public static class CharacterInfo {
        public int srcX;
        public int srcY;
        /** width in pixels */
        public int width;
        /** height in pixels */
        public int height;
        public int xOffset;
        public int yOffset;
        public int xAdvance;
    }

    private void initialize(InputStream settingFile) {
        parseFntFile(settingFile);
    }

    /**
     * Get the bounding rectangle for set of CharacterInfo.
     *
     * @param charInfo The dimension-information of the character
     * @param rect The overall bounding rectangle
     */
    public void getCharRect(CharacterInfo charInfo, Box rect) {
        rect.set(charInfo.srcX, mScaleH - charInfo.srcY - charInfo.height, charInfo.width + charInfo.srcX, mScaleH - charInfo.srcY);
    }

    private static final char CHAR_SPACE = ' ';

    /**
     * Get the Character Info for the specified character.
     *
     * @param ch The character
     * @return The set of Character Info data
     */
    public CharacterInfo getCharInfo(char ch) {
        CharacterInfo charInfo;
        charInfo = mCharMap.get(ch);
        if (charInfo == null) {
            charInfo = mCharMap.get(CHAR_SPACE);    // replace as space
            if (charInfo == null) {
                throw new Ngin3dException("Cannot find replacement character");
            }
        }
        return charInfo;
    }

    public Image createCharImage() {
        return Image.createFromResource(mResources, mResId);
    }

    public void setupCharImage(Image image) {
        image.setImageFromResource(mResources, mResId);
    }

    private void parseFntFile(InputStream settingFile) {
        BufferedReader reader = new BufferedReader(new InputStreamReader(settingFile, Charset.defaultCharset()));
        try {
            reader.readLine();  // info

            String line = reader.readLine();
            if (line == null) {
                throw new Ngin3dException("Invalid font file");
            }
            String[] common = line.split(" ");
            if (common.length < 4) {
                throw new Ngin3dException("Invalid font file");
            }
            if (!common[1].startsWith("lineHeight=")) {
                throw new Ngin3dException("Invalid font file");
            }

            if (!common[2].startsWith("base=")) {
                throw new Ngin3dException("Invalid font file");
            }

            mScaleH = Integer.parseInt(common[4].substring(7));
            line = reader.readLine();
            if (line == null) {
                throw new Ngin3dException("Invalid font file");
            }
            String[] pages = line.split(" ", 4);
            if (!pages[2].startsWith("file=")) {
                throw new Ngin3dException("Invalid font file");
            }

            line = reader.readLine();  // char number

            while (true) {
                line = reader.readLine();
                if (line == null) {
                    break;
                }
                if (line.startsWith("kernings ")) {
                    break;
                }
                if (!line.startsWith("char ")) {
                    continue;
                }

                CharacterInfo font = new CharacterInfo();
                StringTokenizer tokens = new StringTokenizer(line, " ");
                tokens.nextToken();
                String[] tmpChar = tokens.nextToken().toString().split("=", 2);
                char ch = (char) Integer.parseInt(tmpChar[1]);

                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.srcX = Integer.parseInt(tmpChar[1]);
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.srcY = Integer.parseInt(tmpChar[1]);
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.width = Integer.parseInt(tmpChar[1]);
                if (font.width > mWordWidth) {
                    mWordWidth = font.width;
                }
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.height = Integer.parseInt(tmpChar[1]);
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.xOffset = Integer.parseInt(tmpChar[1]);
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.yOffset = Integer.parseInt(tmpChar[1]);
                tmpChar = tokens.nextToken().toString().split("=", 2);
                font.xAdvance = Integer.parseInt(tmpChar[1]);

                mCharMap.put(ch, font);
            }
        } catch (IOException ex) {
            throw new Ngin3dException("Error loading font file", ex);
        } finally {
            Utils.closeQuietly(reader);
        }
    }

    /**
     * @hide
     */
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof BitmapFont)) return false;

        BitmapFont that = (BitmapFont) o;

        if (mResId != that.mResId) return false;
        if (mResources == null ? that.mResources != null : !mResources.equals(that.mResources)) return false;

        return true;
    }

    /**
     * @hide
     */
    @Override
    public int hashCode() {
        int result = mResId;
        result = 31 * result + (mResources == null ? 0 : mResources.hashCode());
        return result;
    }
}
