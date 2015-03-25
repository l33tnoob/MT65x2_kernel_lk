/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.filterpacks.ve;

import java.util.Arrays;
import java.util.HashMap;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.VideoEventFilter;

import android.content.res.Resources;
import android.filterfw.core.FilterContext;
import android.filterfw.core.GLFrame;
import android.filterfw.core.ShaderProgram;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

/**
 * @hide
 */
public class VideoEvent extends HashMap<String, Object> {
    private static int[] mCount = {0};
    protected MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    private String mName;
    private String mType;

    protected Long mStart;
    protected Long mEnd;
    protected Long mEffectStart;
    protected Long mEffectEnd;

    protected boolean mGotMainFrame = false;
    protected GLFrame mMainFrame;

    protected boolean mGotBgFrame = false;
    protected GLFrame mBgFrame;

    protected int mOrientation = 0;


    private static String mCopyShaderWithColor =
        "precision mediump float;\n" +
        "uniform sampler2D tex_sampler_0;\n" +
        "uniform vec4 ccc;\n" +
        "varying vec2 v_texcoord;\n" +
        "void main() {\n" +
        "  gl_FragColor = texture2D(tex_sampler_0, v_texcoord);\n" +
        "  if (ccc.a <= 0.0) {\n" +
        "    gl_FragColor = ccc;\n" +
        "  }\n" +
        "}\n";

    protected ShaderProgram mCopyProgramWithColor;
    protected float mColor[] = new float[4];

    public VideoEvent(String type, String name, long start, long end) {
        mTool.log('d', getClass().getSimpleName() + "() " + type + " @ " + name);
        mTool.setIDandIncrease(mCount);

        mType = type;
        mName = name;
        setDuration(start, end);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    public void setOrientation(int ori) {
        mOrientation = ori;
    }

    public void setDuration(long start, long end){
        mStart = start;
        if (end < start) {
            mEnd = start;
        } else {
            mEnd = end;
        }
        setDurationEffectRelatedTime(0, end-start);
    }

    public void setDurationEffectRelatedTime(long start, long length){
        mEffectStart = mStart + start;
        mEffectEnd = mEffectStart + length;

        if(mEffectEnd > mEnd) {
            mEnd = mEffectEnd;
        }
    }

    public String getType() {
        return mType;
    }

    public String getName() {
        return mName;
    }

    public Long getStartTime() {
        return mStart;
    }

    public Long getEndTime() {
        return mEnd;
    }

    public String getKey() {
        return mTool.getID() + "_" + getType() + getName();
    }

    @Override
    public String toString() {
        String result = "";
        Object[] allKeys = this.keySet().toArray();
        Arrays.sort(allKeys);
        for (Object key : allKeys) {
            Object obj = this.get(key);
            result += "   " + key + ": " + obj + "\n";
        }
        return "[" + mStart + "~" + mEnd + "]\n" + result;
    }

    public void putThumbnail(Resources res, Object resource, String isMove, String isFadeOut) {
        mTool.log('d', mName + ".putThumbnail() " + resource + " m:" + isMove + " fo:" + isFadeOut);

        if (null != resource) {
            Bitmap bitmap = null;
            if (resource instanceof Bitmap) {
                bitmap = (Bitmap)resource;
            } else if (resource instanceof Integer && res != null) {
                bitmap = MyUtility.getBitmapFromResource(res, (Integer)resource);
            } else if (resource instanceof String) {
                bitmap = BitmapFactory.decodeFile((String)resource);
            }

            if (null != bitmap) {
                this.put("bitmap", bitmap);
                if ("1".equalsIgnoreCase(isMove)) {
                    this.put("bitmap_move", isMove);
                }
                if ("1".equalsIgnoreCase(isFadeOut)) {
                    this.put("bitmap_fadeout", isFadeOut);
                }
            }
        }
    }

    //Override
    public Object put(String key, Object obj) {
        if ((obj + "").length() < 1 || (obj + "").equalsIgnoreCase("null")) {
            return null;
        }
        return super.put(key, obj);
    }

    public void putEdge(Resources res, Object resource) {
        mTool.log('d', mName + ".putEdge() " + resource);

        if (null != resource) {
            Bitmap bitmap = null;
            if (resource instanceof Bitmap) {
                bitmap = (Bitmap)resource;
            } else if (resource instanceof Integer && res != null) {
                bitmap = MyUtility.getBitmapFromResource(res, (Integer)resource);
            } else if (resource instanceof String) {
                bitmap = BitmapFactory.decodeFile((String)resource);
            }

            if (null != bitmap) {
                this.put("edge", bitmap);
            }
        }
    }

    public void putBackground(Resources res, Object resource, String isStill, String isFadeIn, String initTimeOffset) {
        mTool.log('d', mName + ".putBackground() " + resource + " isStill:" + isStill);

        if (null != resource) {
            Bitmap bitmap = null;
            if (resource instanceof Bitmap) {
                bitmap = (Bitmap)resource;
            } else if (resource instanceof Integer && res != null) {
                bitmap = MyUtility.getBitmapFromResource(res, (Integer)resource);
            } else if (resource instanceof String) {
                bitmap = BitmapFactory.decodeFile((String)resource);
            }

            if (null != bitmap) {
                this.put("background", bitmap);
            } else if (resource instanceof String) {
                if (((String)resource).contains("video")) {
                    this.put("background", (String)resource);
                }
                this.put("background_initoffsettime", initTimeOffset);
            }

            if (this.containsKey("background")) {
                if ("1".equalsIgnoreCase(isStill)) {
                    this.put("background_still", isStill);
                }
                if ("1".equalsIgnoreCase(isFadeIn)) {
                    this.put("background_fadein", isFadeIn);
                }
            }
        }
    }

    public void open(FilterContext context, VideoEventFilter myfilter) {
        mTool.log('d', mName + ".open() ");

        mCopyProgramWithColor = new ShaderProgram(context, mCopyShaderWithColor);
        mColor[0] = mColor[1] = mColor[2] = mColor[3] = 1.0f;
        mCopyProgramWithColor.setHostValue("ccc", mColor);

        if (this.containsKey("bitmap")) {
            Bitmap bitmap = (Bitmap)this.get("bitmap");
            mMainFrame = MyUtility.createBitmapFrame(context, bitmap);
            mGotMainFrame = true;
        }

        if (this.containsKey("background")) {
            if (this.get("background") instanceof Bitmap) {
                Bitmap bitmap = (Bitmap) this.get("background");
                mBgFrame = MyUtility.createBitmapFrame(context, bitmap);
                mGotBgFrame = true;
            }
        }
    }

    public boolean process(FilterContext context, VideoEventFilter myfilter, boolean isRenderOutput, GLFrame output) {
        return false;
    }

    public void close(FilterContext context, VideoEventFilter myfilter) {
        mTool.log('d', mName + ".close() ");
        cleanResource();
    }

    protected void cleanResource() {
        mTool.log('d', mName + ".cleanResource() ");

        if (null != mMainFrame) {
            mMainFrame.release();
            mMainFrame = null;
        }

        if (null != mBgFrame) {
            mBgFrame.release();
            mBgFrame = null;
        }

        if (this.containsKey("bitmap")) {
            Bitmap bitmap = (Bitmap)this.get("bitmap");
            bitmap.recycle();
            this.remove("bitmap");
        }

        if (this.containsKey("background")) {
            if (this.get("background") instanceof Bitmap) {
                Bitmap bitmap = (Bitmap) this.get("background");
                bitmap.recycle();
                this.remove("background");
            }
        }
    }
}
