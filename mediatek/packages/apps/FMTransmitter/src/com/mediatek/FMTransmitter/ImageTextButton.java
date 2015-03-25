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

package com.mediatek.FMTransmitter;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.Button;

public class ImageTextButton extends Button {
    
    private static final String NAME_SPACE =  "http://FMTransmitter.mediatek.com/custom";   
    private int mResourceId = 0;   
    private Bitmap mBitmap; 
    public ImageTextButton(Context context,  AttributeSet attrs) {
        super(context, attrs);
        // TODO Auto-generated constructor stub
        setClickable(true);  
        mResourceId = attrs.getAttributeResourceValue(NAME_SPACE, "icon", R.drawable.ipower);
        mBitmap = BitmapFactory.decodeResource(getResources(), mResourceId);   
        
    }
    @Override  
    protected void onDraw(Canvas canvas) {   
        // TODO Auto-generated method stub 
        /*
        Paint mPaint = new Paint();
        CharSequence text = this.getText();
        float width = mPaint.measureText(text, 0, text.length());
        */  
        int x = (((this.getMeasuredWidth() >> 1) - mBitmap.getWidth()) >> 3) * 3;   
        int y = (this.getHeight() - mBitmap.getWidth()) >> 1;   
        canvas.drawBitmap(mBitmap, x, y, null);    
        //canvas.translate(0, (this.getMeasuredHeight()>>1)-(int)this.getTextSize());   
        canvas.translate(this.getMeasuredWidth() >> 3, 0);      
        super.onDraw(canvas);   
    }   
  
    public void setIcon(Bitmap bitmap) {   
        this.mBitmap = bitmap;   
                invalidate();   
    }   
    public void setIcon(int resourceId) {   
        this.mBitmap = BitmapFactory.decodeResource(getResources(), resourceId);   
                invalidate();
    }   
    
}
