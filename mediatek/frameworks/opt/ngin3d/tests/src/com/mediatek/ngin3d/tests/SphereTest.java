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

package com.mediatek.ngin3d.tests;


import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Sphere;
import com.mediatek.ngin3d.presentation.ImageSource;

public class SphereTest extends Ngin3dTest {
    public void testCreateFromBitmap() {
        Bitmap bitmap = BitmapFactory.decodeResource(getInstrumentation().getContext().getResources(), R.drawable.earth);
        Sphere sphere = Sphere.createFromBitmap(bitmap);
        assertEquals(ImageSource.BITMAP, sphere.getValue(Sphere.PROP_IMG_SRC).srcType);
    }

    public void testCreateFromFile() {
        Sphere sphere = Sphere.createFromFile("/sdcard/mage/photo_01_big.jpg");
        assertEquals(ImageSource.FILE, sphere.getValue(Sphere.PROP_IMG_SRC).srcType);
    }

    public void testCreateFromResource() {
        Sphere sphere = Sphere.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.earth);
        assertEquals(ImageSource.RES_ID, sphere.getValue(Sphere.PROP_IMG_SRC).srcType);
    }

    public void testSetScale() {
        Sphere sphere = Sphere.createFromResource(getInstrumentation().getContext().getResources(), R.drawable.earth);
        Scale scale = new Scale(1, 1);
        sphere.setScale(scale);
        assertEquals(1f, sphere.getScale().y);
    }

}
