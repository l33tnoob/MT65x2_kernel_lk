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
/** \file
 * Texture Cache for J3M
 */
package com.mediatek.ngin3d.j3m;

import java.nio.ByteBuffer;
import java.util.LinkedList;
import android.graphics.Bitmap;
import android.util.Log;

import com.mediatek.j3m.Texture;
import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.presentation.BitmapGenerator;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * A texture cache that maps texture source (file, resource, or anything else)
 * to their texture object in memory. This class can prevent loading multiple
 * texture for the same 'source'.
 * @hide
 */
public class TextureCache {
    private static final String TAG = "TextureCache";
    private static final int MAX_GENERATOR_LIST_SIZE = 24;
    private static final String PVR = "pvr";

    private final LinkedList<BitmapGenerator> mGeneratorCache =
            new LinkedList<BitmapGenerator>();

    private final J3mPresentationEngine mEngine;

    public TextureCache(J3mPresentationEngine engine) {
        mEngine = engine;
    }

    /**
     * Release the texture from this cache object
     * @param key  texture key
     */
    public void release(Object key) {
        // \todo implement
        Log.e(TAG, "release() not implemented.");
    }

    /**
     *  Gets the texture from android bitmap.
     * @param bitmap   android bitmap
     * @return   specific texture object
     */
    protected Texture2D getTexture(Bitmap bitmap) {
        int format;
        int type;
        int bytes;

        if (Bitmap.Config.ARGB_8888.equals(bitmap.getConfig())) {
            format = Texture.Format.RGBA;
            type = Texture.Type.UNSIGNED_BYTE;
            bytes = 4;
        } else if (Bitmap.Config.RGB_565.equals(bitmap.getConfig())) {
            format = Texture.Format.RGB;
            type = Texture.Type.UNSIGNED_SHORT_5_6_5;
            bytes = 2;
        } else if (Bitmap.Config.ALPHA_8.equals(bitmap.getConfig())) {
            format = Texture.Format.ALPHA;
            type = Texture.Type.UNSIGNED_BYTE;
            bytes = 1;
        } else if (Bitmap.Config.ARGB_4444.equals(bitmap.getConfig())) {
            format = Texture.Format.RGBA;
            type = Texture.Type.UNSIGNED_SHORT_4_4_4_4;
            bytes = 2;
            Log.w(TAG, "Bitmap Config ARGB_4444 is deprecated.");
        } else {
            Log.e(TAG, "Bitmap Config unrecognised.");
            return null;
        }

        int height = bitmap.getHeight();
        int width = bitmap.getWidth();

        ByteBuffer byteBuffer = ByteBuffer.allocate(width * height * bytes);
        bitmap.copyPixelsToBuffer(byteBuffer);
        byte[] bitmapArray = byteBuffer.array();

        Texture2D texture = mEngine.getAssetPool().createTexture2D(
                width, height, format, type, bitmapArray);
        return texture;
    }

    /**
     * Gets the texture from file name.
     * @param filename   file name of the texture data
     * @return   specific texture object
     */
    protected Texture2D getTexture(String filename) {
        return mEngine.getAssetPool().getTexture2D(filename);
    }

    /**
     * Gets the texture by resource ID
     * @param id Resource ID of the texture
     * @return   specific texture object
     */
    protected Texture2D getTexture(int id) {
        String name = mEngine.getResources().getResourceName(id);
        return mEngine.getAssetPool().getTexture2D(name);
    }

    /**
     * Gets texture by giving types of source.
     * @param src  image types. could be file, bitmap, bitmap generator,
     * and android resource
     * @return  texture object
     */
    public Texture2D getTexture(ImageSource src) {
        Texture2D result;
        switch (src.srcType) {
        case ImageSource.FILE: {
            result = getTexture((String) src.srcInfo);
            break;
        }

        case ImageSource.BITMAP: {
            Bitmap bitmap = (Bitmap) src.srcInfo;
            result = getTexture(bitmap);

            if ((src.options & ImageSource.RECYCLE_AFTER_USE) != 0) {
                bitmap.recycle();
            }
            break;
        }

        case ImageSource.RES_ID: {
            result = getTexture(((ImageDisplay.Resource) src.srcInfo).resId);
            break;
        }

        case ImageSource.ASSET: {
            result = getTexture((String) src.srcInfo);
            break;
        }

        case ImageSource.BITMAP_GENERATOR: {
            BitmapGenerator generator = (BitmapGenerator) src.srcInfo;
            // If generator has cached bitmap, add it to generator cache.
            if (generator.getCachedBitmap() != null) {
                addToGeneratorCache(generator);
            }
            Bitmap bitmap = generator.getBitmap();
            result = getTexture(bitmap);

            if ((src.options & ImageSource.RECYCLE_AFTER_USE) != 0) {
                bitmap.recycle();
            }
            break;
        }

        case ImageSource.VIDEO_TEXTURE: {
            // We require a 'virtual' texture to link Android video and renderer
            Texture2D texture = mEngine.getAssetPool().createTexture2D(
                0, 0, 0, 0, null);
            result = texture;
            break;
        }

        default:
            throw new Ngin3dException("Unsupported image source");
        }
        return  result;
    }

    protected void addToGeneratorCache(BitmapGenerator generator) {
        if (mGeneratorCache.contains(generator)) {
            // Move the generator to the end of list
            mGeneratorCache.remove(generator);
            mGeneratorCache.addLast(generator);
        } else {
            mGeneratorCache.add(generator);
            if (mGeneratorCache.size() > MAX_GENERATOR_LIST_SIZE) {
                mGeneratorCache.removeFirst().free();
            }
        }
    }
}
