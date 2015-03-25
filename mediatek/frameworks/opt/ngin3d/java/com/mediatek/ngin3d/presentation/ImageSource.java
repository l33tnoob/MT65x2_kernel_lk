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

package com.mediatek.ngin3d.presentation;

import com.mediatek.ngin3d.utils.JSON;

/**
 * A class that responsible for storing image source reference and information.
 */
public class ImageSource implements JSON.ToJson {
    public static final int FILE = 1;
    public static final int BITMAP = 2;
    public static final int RES_ID = 3;
    public static final int BITMAP_GENERATOR = 4;
    public static final int VIDEO_TEXTURE = 5;
    public static final int ASSET = 6;

    /**
     * Initialize this class with image source type and its data
     * @param srcType  image source type
     * @param srcInfo   image data
     */
    public ImageSource(int srcType, Object srcInfo) {
        this(srcType, srcInfo, 0);
    }

    /**
     *  Initialize this class with image source type, its data, and the option variable
     * @param srcType   image source type
     * @param srcInfo   image data
     * @param options  option variable
     */
    public ImageSource(int srcType, Object srcInfo, int options) {
        this.srcType = srcType;
        this.srcInfo = srcInfo;
        this.options = options;
    }

    public int srcType;
    public Object srcInfo;

    public static final int RECYCLE_AFTER_USE = 1 << 0;     // only valid for bitmap source
    public int options;

    /**
     * Convert the image source property to string for output
     * @return   output string
     */
    @Override
    public String toString() {
        return String.format("ImageSource: {type:%d, info:%s}", srcType, srcInfo);
    }

    /**
     * Convert the image source property to JSON formatted String
     * @return   output JSON formatted String
     */
    public String toJson() {
        return String.format("{ImageSource: {type:%d, info:%s}}", srcType, JSON.toJson(srcInfo));
    }
}
