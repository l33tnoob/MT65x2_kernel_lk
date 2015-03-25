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

package com.mediatek.apst.target.util;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

public abstract class GraphicUtils {

    /**
     * @param drawable
     *            The drawable to draw.
     * @param destW
     *            The width to draw.
     * @param destH
     *            The height to draw.
     * @return The bitmap drawed.
     */
    public static Bitmap drawable2Bitmap(Drawable drawable, int destW, int destH) {
        Bitmap bm = Bitmap.createBitmap(destW, destH, Config.ARGB_8888);
        Canvas canvas = new Canvas(bm);
        drawable.setBounds(0, 0, destW, destH);
        drawable.draw(canvas);
        return bm;
    }

    /**
     * @param bm
     *            The bitmap to bytes.
     * @param format
     *            specifies the known formats a bitmap can be compressed into.
     * @param quality
     *            The quality of the bitmap.
     * @return The byte array of the bitmap.
     */
    public static byte[] bitmap2Bytes(Bitmap bm, CompressFormat format,
            int quality) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bm.compress(CompressFormat.PNG, 100, baos);
        byte[] bytes = baos.toByteArray();
        // Release resources
        try {
            baos.close();
        } catch (IOException e) {
            Debugger.logE(new Object[] { bm, format, quality }, null, e);
        }
        return bytes;
    }

    /**
     * @param drawable
     *            The drawable to draw.
     * @param destW
     *            The width to draw.
     * @param destH
     *            The height to draw.
     * @param format
     *            specifies the known formats a bitmap can be compressed into.
     * @param quality
     *            The quality of the bitmap.
     * @return The byte array of the bitmap.
     */
    public static byte[] drawable2Bytes(Drawable drawable, int destW,
            int destH, CompressFormat format, int quality) {
        Bitmap bm = drawable2Bitmap(drawable, destW, destH);
        byte[] bytes = bitmap2Bytes(bm, format, quality);
        // Release resources
        bm.recycle();
        return bytes;
    }
}
