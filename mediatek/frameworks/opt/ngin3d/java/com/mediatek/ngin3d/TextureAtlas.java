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
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.utils.Ngin3dException;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.HashMap;

/**
 * Texture Atlas - large image containing many sub-images and a
 * JSON description of where the sub-images are located.
 */
public class TextureAtlas {

    private static final String FRAMES = "frames";
    private static final String FRAME = "frame";
    private static final String ROTATED = "rotated";
    private static final String TRIMMED = "trimmed";
    private static final String SPRITE_SOURCE_SIZE = "spriteSourceSize";
    private static final String SOURCE_SIZE = "sourceSize";
    private static final String X = "x";
    private static final String Y = "y";
    private static final String WIDTH = "w";
    private static final String HEIGHT = "h";

    private static TextureAtlas sDefault;

    /**
     * Gets the default atlas object.
     *
     * @return  Texture atlas object.
     */
    public static TextureAtlas getDefault() {
        synchronized (TextureAtlas.class) {
            if (sDefault == null) {
                sDefault = new TextureAtlas();
            }
        }
        return sDefault;
    }

    private final HashMap<String, Atlas> mAtlasSet = new HashMap<String, Atlas>();

    /**
     * The Atlas that contains one image and one JSONObject.
     */
    static class Atlas {
        protected final int mImageId;
        protected final String mAsset;
        protected final JSONObject mFrames;

        public Atlas(int resId, JSONObject frames) {
            mImageId = resId;
            mFrames = frames;
            mAsset = null;
        }

        public Atlas(String asset, JSONObject frames) {
            mAsset = asset;
            mFrames = frames;
            mImageId = 0;
        }

        public int getResourceId() {
            return mImageId;
        }

        public String getAssetName() {
            return mAsset;
        }
    }

    private JSONObject getJSONObject(Resources resources, int scriptId) {
        JSONObject frames;
        InputStream is = resources.openRawResource(scriptId);
        try {
            int length = is.available();
            byte[] b = new byte[length];
            if (is.read(b, 0, length) == length) {
                final String s = new String(b, Charset.defaultCharset());
                JSONObject atlas = new JSONObject(s);
                frames = atlas.optJSONObject(FRAMES);
            } else {
                throw new Ngin3dException("JSON of Packer List doesn't read completely");
            }
        } catch (IOException e) {
            throw new Ngin3dException(e);
        } catch (JSONException e) {
            throw new Ngin3dException(e);
        } finally {
            Utils.closeQuietly(is);
        }
        return frames;
    }

    /**
     * Add a new image into this atlas object.
     *
     * @param resources  Android resource manager
     * @param asset  Atlas asset name
     * @param scriptId  JSON file id.
     */
    public void add(Resources resources, String asset, int scriptId) {
        if (mAtlasSet.containsKey(asset)) {
            return;
        }
        JSONObject frames = getJSONObject(resources, scriptId);
        if (frames != null) {
            mAtlasSet.put(asset, new Atlas(asset, frames));
        }
    }

    /**
     * Add a new image into this atlas object.
     *
     * @param resources  Android resource manager
     * @param atlasId  Resource id
     * @param scriptId  JSON file id.
     */
    public void add(Resources resources, int atlasId, int scriptId) {
        String name = resources.getResourceName(atlasId);
        if (mAtlasSet.containsKey(name)) {
            return;
        }
        JSONObject frames = getJSONObject(resources, scriptId);

        if (frames != null) {
            mAtlasSet.put(name, new Atlas(atlasId, frames));
        }
    }

    private String getResourceFilename(ImageDisplay.Resource res) {
        String resName = res.resources.getString(res.resId);
        return resName.substring(resName.lastIndexOf("/") + 1);
    }

    /**
     * Gets the specific image information from the atlas by image resource , box , and dimension information.
     *
     * @hide Dependent on class in the internal presntation layer
     *
     * @param res  image resource information.
     * @param rect  box information.
     * @param size  dimension information.
     * @return  the atlas object which stores the specific image. null if the image isn't in texture atlas.
     */
    public Atlas getFrame(ImageDisplay.Resource res, Box rect, Dimension size) {
        String fileName = getResourceFilename(res);

        for (Atlas atlas : mAtlasSet.values()) {
            JSONObject resObject = atlas.mFrames.optJSONObject(fileName);
            if (resObject != null) {
                JSONObject frame = resObject.optJSONObject(FRAME);
                int x = frame.optInt(X);
                int y = frame.optInt(Y);
                int w = frame.optInt(WIDTH);
                int h = frame.optInt(HEIGHT);
                rect.set(x, y, w + x, h + y);

                JSONObject sourceSize = resObject.optJSONObject(SOURCE_SIZE);
                size.width = sourceSize.optInt(WIDTH);
                size.height = sourceSize.optInt(HEIGHT);
                return atlas;
            }
        }
        return null;
    }

    /**
     * Clears all of the setting of this atlas.
     */
    public void cleanup() {
        mAtlasSet.clear();
    }

    /**
     * For test ONLY. Check if the atlas set is empty.
     * @return  boolean indicate atlas set is empty.
     * @hide For test only.
     */
    public boolean isEmpty() {
        return mAtlasSet.isEmpty();
    }

}
