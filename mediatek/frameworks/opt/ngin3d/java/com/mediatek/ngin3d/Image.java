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
import android.graphics.Bitmap;
import com.mediatek.ngin3d.presentation.BitmapGenerator;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * A 2D (self lit) Plane Actor that has a bitmap texture.
 * <p>
 * Reminder: Built-in ngin3D graphical primitives are created y-down by default,
 * and are self-lit so they are <b>not</b> affected by scene Lights. This is OK
 * use as billboards, but not for use in a 3D scene.
 */
public class Image extends Plane {

    public static final int FILTER_QUALITY_LOW = 0;
    public static final int FILTER_QUALITY_MEDIUM = 1;
    public static final int FILTER_QUALITY_HIGH = 2;

    ///////////////////////////////////////////////////////////////////////////
    // Property handling

    /**
     * @hide
     */
    static final Property<ImageSource> PROP_IMG_SRC = new Property<ImageSource>("image_source", null);
    /**
     * @hide
     */
    static final Property<Integer> PROP_FILTER_QUALITY = new Property<Integer>("filter_quality", FILTER_QUALITY_HIGH);
    /**
     * @hide
     */
    static final Property<Boolean> PROP_KEEP_ASPECT_RATIO = new Property<Boolean>("keep_aspect_ratio", false);
    /**
     * @hide
     */
    static final Property<Integer> PROP_REPEAT_X = new Property<Integer>("repeat_x", 0);
    /**
     * @hide
     */
    static final Property<Integer> PROP_REPEAT_Y = new Property<Integer>("repeat_y", 0);
    /**
     * @hide
     */
    static final Property<Boolean> PROP_ENABLE_MIPMAP = new Property<Boolean>("enable_mipmap", false);

    static {
        PROP_SIZE.addDependsOn(PROP_IMG_SRC);
        PROP_SRC_RECT.addDependsOn(PROP_IMG_SRC);
    }

    protected Image() {
        super(false);
    }

    protected Image(boolean isYUp) {
        super(isYUp);
    }

    /**
     * Apply the image information data.
     *
     * @param property property type to be applied
     * @param value    property value to be applied
     * @return if the property is successfully applied
     * @hide
     */
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property.sameInstance(PROP_IMG_SRC)) {
            ImageSource src = (ImageSource) value;
            if (src == null) {
                return false;
            }
            getPresentation().setImageSource(src);
            // Store image real size if there is no specific value of size
            if (getValue(PROP_SIZE).width < 0) {
                setSize(getPresentation().getSize());
            }
            return true;
        } else if (property.sameInstance(PROP_FILTER_QUALITY)) {
            Integer quality = (Integer) value;
            getPresentation().setFilterQuality(quality);
            return true;
        } else if (property.sameInstance(PROP_KEEP_ASPECT_RATIO)) {
            Boolean kar = (Boolean) value;
            getPresentation().setKeepAspectRatio(kar);
            return true;
        } else if (property.sameInstance(PROP_REPEAT_X)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_REPEAT_Y)) {
            enableApplyFlags(FLAG_APPLY_LATER_IN_BATCH);
            return true;
        } else if (property.sameInstance(PROP_ENABLE_MIPMAP)) {
            Boolean enable = (Boolean) value;
            getPresentation().enableMipmap(enable);
            return true;
        }
        return false;
    }

    /**
     * @hide
     */
    @Override
    protected void applyBatchValues() {
        Integer repeatX = getValue(PROP_REPEAT_X);
        Integer repeatY = getValue(PROP_REPEAT_Y);
        getPresentation().setRepeat(repeatX, repeatY);
    }

    /**
     * Create an Image object with blank bitmap.
     * @return A blank Image object
     */
    public static Image createEmptyImage() {
        return createEmptyImage(false);
    }

    /**
     * Create an Image object with blank bitmap.
     *
     * @param width width of new image
     * @param height height of new image
     * @return A blank Image object
     */
    public static Image createEmptyImage(int width, int height) {
        Image image = new Image(false);
        image.setEmptyImage(width, height);
        return image;
    }

    /**
     * Create Image object with blank bitmap with option for a Y-Up orientaton.
     * @return A blank Image object
     */
    public static Image createEmptyImage(boolean isYUp) {
        Image image = new Image(isYUp);
        image.setEmptyImage();
        return image;
    }

    /**
     * Create an Image object from specific file name.
     *
     * @param filename Texture file name
     * @return The Image object
     */
    public static Image createFromFile(String filename) {
        return createFromFile(filename, false);
    }

    /**
     * Create an Image object from specific file name.
     *
     * @param filename Texture file name
     * @param isYUp   True for creating a Y-up quad, default is Y-down
     * @return The Image object
     */
    public static Image createFromFile(String filename, boolean isYUp) {
        Image image = new Image(isYUp);
        image.setImageFromFile(filename);
        return image;
    }

    /**
     * Create an Image object from bitmap. Note that the bitmap cannot be recycled. Otherwise the image cannot be
     * displayed correctly after rendering engine is shutdown and restarted again.
     *
     * @param bitmap The bitmap image in memory
     * @return The Image object
     */
    public static Image createFromBitmap(Bitmap bitmap) {
        return createFromBitmap(bitmap, false);
    }

    /**
     * Create an Image object from bitmap. Note that the bitmap cannot be recycled. Otherwise the image cannot be
     * displayed correctly after rendering engine is shutdown and restarted again.
     *
     * @param bitmap The bitmap image in memory
     * @param isYUp  True for creating a Y-up quad, default is Y-down
     * @return The Image object
     */
    public static Image createFromBitmap(Bitmap bitmap, boolean isYUp) {
        Image image = new Image(isYUp);
        image.setImageFromBitmap(bitmap);
        return image;
    }

    /**
     * Create an Image object from specified bitmap generator. Note that the generated bitmap may be recycled anytime to
     * reduce memory footprint. If the bitmap is needed again, the generate() method will be called again to generate a
     * new one.
     *
     * @hide BitmapGenerator is implemented as part of the internal Presentation API
     *
     * @param bitmapGenerator the bitmap generator
     * @return The Image object
     */
    public static Image createFromBitmapGenerator(BitmapGenerator bitmapGenerator) {
        return createFromBitmapGenerator(bitmapGenerator, false);
    }

    /**
     * Create an Image object from specified bitmap generator. Note that the generated bitmap may be recycled anytime to
     * reduce memory footprint. If the bitmap is needed again, the generate() method will be called again to generate a
     * new one.
     *
     * @hide BitmapGenerator is implemented as part of the internal Presentation API
     *
     * @param bitmapGenerator the bitmap generator
     * @param isYUp   true for creating a Y-up quad, default is Y-down
     * @return The Image object
     */
    public static Image createFromBitmapGenerator(BitmapGenerator bitmapGenerator, boolean isYUp) {
        Image image = new Image(isYUp);
        image.setImageFromBitmapGenerator(bitmapGenerator);
        return image;
    }

    /**
     * Create an Image object from android resource and resource ID.
     *
     * @param resources Android resource
     * @param resId     Android resource ID
     * @return The Image object
     */
    public static Image createFromResource(Resources resources, int resId) {
        return createFromResource(resources, resId, false);
    }

    /**
     * Create an Image object from android resource and resource id
     *
     * @param resources Android resource
     * @param resId     Android resource id
     * @param isYUp   True for creating a Y-up quad, default is Y-down
     * @return The Image object
     */
    public static Image createFromResource(Resources resources, int resId, boolean isYUp) {
        Image image = new Image(isYUp);
        image.setImageFromResource(resources, resId);
        return image;
    }

    /**
     * Create an Image object from android asset file.
     * The asset will be loaded from AssetManager in A3M.
     *
     * @param assetName Asset file name
     * @return The Image object
     */
    public static Image createFromAsset(String assetName) {
        return createFromAsset(assetName, false);
    }

    /**
     * Create an Image object from android asset.
     * The asset will be loaded from AssetManager in native.
     *
     * @param assetName Asset file name
     * @param isYUp   True for creating a Y-up quad, default is Y-down
     * @return The Image object
     */
    public static Image createFromAsset(String assetName, boolean isYUp) {
        Image image = new Image(isYUp);
        image.setImageFromAsset(assetName);
        return image;
    }

    /**
     * Set an empty Image object with size 1 x 1 blank bitmap.
     */
    public void setEmptyImage() {
        Bitmap bitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ALPHA_8);
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP, bitmap));
    }

    /**
     * Set an empty Image object given dimensions.
     */
    public void setEmptyImage(int width, int height) {
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP, bitmap));
    }




    /**
     * Specify the Image object by the image file name.
     *
     * @param filename Image file name
     */
    public void setImageFromFile(String filename) {
        if (filename == null) {
            throw new NullPointerException("filename cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.FILE, filename));
    }

    /**
     * Specify the image by a Bitmap object. Note that the bitmap cannot be recycled. Otherwise the image cannot be
     * displayed correctly after rendering engine is shutdown and restarted again.
     *
     * @param bitmap The bitmap image
     */
    public void setImageFromBitmap(Bitmap bitmap) {
        if (bitmap == null) {
            throw new NullPointerException("bitmap cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP, bitmap));
    }

    /**
     * Specify the image by a Bitmap generator. Note that the generated bitmap cannot be recycled. Otherwise the image cannot be
     * displayed correctly after rendering engine is shutdown and restarted again.
     *
     * @hide BitmapGenerator is implemented as part of the internal Presentation API
     *
     * @param bitmapGenerator the bitmap generator
     */
    public void setImageFromBitmapGenerator(BitmapGenerator bitmapGenerator) {
        if (bitmapGenerator == null) {
            throw new NullPointerException("bitmapGenerator cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP_GENERATOR, bitmapGenerator));
    }

    /**
     * Specify the Image object by android resource and resource id.
     *
     * @param resources Android resource
     * @param resId     Android resource id
     */
    public void setImageFromResource(Resources resources, int resId) {
        if (resources == null) {
            throw new NullPointerException("resources cannot be null");
        }
        ImageDisplay.Resource res = new ImageDisplay.Resource(resources, resId);
        Box box = new Box();
        Dimension dim = new Dimension(-1, -1);

        // IMG_SRC, SRC_RECT and SIZE must be applied together or there will be trouble when using texture atlas
        Transaction.beginPropertiesModification();
        TextureAtlas.Atlas atlas = TextureAtlas.getDefault().getFrame(res, box, dim);
        if (atlas == null) {
            setSourceRect(null);
            setPropImgSrc(new ImageSource(ImageSource.RES_ID, res));
        } else {
            // The resource is in TextureAtlas, transfer to atlas resource/asset.
            setSourceRect(box);
            if (atlas.getAssetName() == null) {
                res.resId = atlas.getResourceId();
                setPropImgSrc(new ImageSource(ImageSource.RES_ID, res));
            } else {
                setPropImgSrc(new ImageSource(ImageSource.ASSET, atlas.getAssetName()));
            }
        }
        // The image might ever use TextureAtlas, we need reset image size property if it's not in TextureAtlas now
        setValueInTransaction(PROP_SIZE, dim);
        Transaction.commit();
    }


    /**
     * Specify the Image object by the asset file name.
     *
     * @param assetName Asset file name
     */
    public void setImageFromAsset(String assetName) {
        if (assetName == null) {
            throw new NullPointerException("asset name cannot be null");
        }
        setValueInTransaction(PROP_IMG_SRC, new ImageSource(ImageSource.ASSET, assetName));
    }

    private void setPropImgSrc(ImageSource imgSrc) {
        setValueInTransaction(PROP_IMG_SRC, imgSrc);
    }

    /**
     * Set the filter quality of this Image object.
     *
     * @param quality Quality value to be set
     */
    public void setFilterQuality(int quality) {
        if (quality < FILTER_QUALITY_LOW || quality > FILTER_QUALITY_HIGH) {
            throw new IllegalArgumentException("Invalid quality value: " + quality);
        }
        setValue(PROP_FILTER_QUALITY, quality);
    }

    /**
     * Get the quality value of this Image object.
     *
     * @return The quality value
     */
    public int getFilterQuality() {
        return getValue(PROP_FILTER_QUALITY);
    }

    /**
     * Set if the image of this Image object need to keep the aspect ratio.
     *
     * @param kar True if the aspect ratio must be preserved
     */
    public void setKeepAspectRatio(boolean kar) {
        setValue(PROP_KEEP_ASPECT_RATIO, kar);
    }

    /**
     * Check the image of this Image object is keeping aspect ratio.
     *
     * @return True if the image keeps its aspect ratio
     */
    public boolean isKeepAspectRatio() {
        return getValue(PROP_KEEP_ASPECT_RATIO);
    }

    /**
     * Set the number of times the image is repeated in each direction.
     *
     * @param repeatX Repeat count for x axis
     * @param repeatY Repeat count for y axis
     */
    public void setRepeat(int repeatX, int repeatY) {
        setValue(PROP_REPEAT_X, repeatX);
        setValue(PROP_REPEAT_Y, repeatY);
    }

    /**
     * Set the number of times the image is repeated in X direction.
     *
     * @return Repeat count for x axis
     */
    public int getRepeatX() {
        return getValue(PROP_REPEAT_X);
    }

    /**
     * Set the number of times the image is repeated in Y direction.
     *
     * @return Repeating count for y axis
     */
    public int getRepeatY() {
        return getValue(PROP_REPEAT_Y);
    }

    /**
     * Enable mipmap of the object or not.
     *
     * @param enable True to enable mip-mapping
     */
    public void enableMipmap(boolean enable) {
        setValue(PROP_ENABLE_MIPMAP, enable);
    }

    /**
     * Check mipmap of the object is enabled or not.
     *
     * @return True if mip-mapping is enabled
     */
    public boolean isMipmapEnable() {
        return getValue(PROP_ENABLE_MIPMAP);
    }

    private static ExecutorService sExecutorService = Executors.newSingleThreadExecutor();

    private class BitmapLoader implements Runnable {
        private final ImageSource mSource;

        BitmapLoader(ImageSource src) {
            mSource = src;
        }

        public void run() {
            BitmapGenerator generator = (BitmapGenerator) mSource.srcInfo;
            generator.cacheBitmap();
            setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP_GENERATOR, generator));
            Thread.yield();
        }
    }

    /**
     * @hide
     */
    public void loadAsync() {
        ImageSource src = getValue(PROP_IMG_SRC);
        if (src == null) {
            return;
        }
        if (src.srcType == ImageSource.BITMAP_GENERATOR) {
            BitmapGenerator generator = (BitmapGenerator) src.srcInfo;
            if (generator.getCachedBitmap() == null) {
                sExecutorService.submit(new BitmapLoader(src));
            }
        }
    }
}
