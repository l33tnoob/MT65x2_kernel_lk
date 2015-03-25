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

package com.mediatek.ngin3d.animation;

import com.mediatek.ngin3d.Point;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * The set of keyframe data
 */
public class KeyframeDataSet implements Serializable {

    // change this UID when keyframeDataset changes.
    private static final long serialVersionUID = 10L;
    // TODO: how to fix when animaiton JSON file updated, but cache exists?

    protected transient int mDuration;
    protected transient int mDelay = Integer.MAX_VALUE;

    private transient int mOpacity = 255;   // 0 - 255. upsample during parsing json
    private transient Point mAnchor = new Point(0, 0);
    private transient Point mScale = new Point(1, 1);
    private transient Point mRotation = new Point();
    private transient Point mPosition = new Point();

    private transient int mVersion;
    private transient int mTargetWidth;
    private transient int mTargetHeight;
    private transient ArrayList<KeyframeData> mAnimations = new ArrayList<KeyframeData>();
    private transient KeyframeData mMarker;

    public KeyframeDataSet() {
        super();
    }

    public List<KeyframeData> getList() {
        return Collections.unmodifiableList(mAnimations);
    }

    public void applyMarker(KeyframeAnimation anim) {
        if (mMarker != null) {
            Samples samples = mMarker.getSamples();
            float[] time = samples.get(Samples.MARKER_TIME);
            String[] actions = samples.getString(Samples.ACTION);
            for (int i = 0; i < time.length; i++) {
                anim.addMarkerAtTime(actions[i], (int) (time[i] * 1000));
            }
        }
    }

    public void add(KeyframeData animation) {
        if (animation == null) {
            throw new IllegalArgumentException("Animation cannot be null.");
        }

        mDuration = Math.max(animation.mDuration, mDuration);
        mDelay = Math.min(mDelay, animation.mDelay);
        mAnimations.add(animation);
    }

    private void writePoint(Point p, ObjectOutputStream oos) throws IOException {
        oos.writeFloat(p.x);
        oos.writeFloat(p.y);
        oos.writeFloat(p.z);
    }

    private Point readPoint(ObjectInputStream ois) throws IOException {
        Point p = new Point();
        p.x = ois.readFloat();
        p.y = ois.readFloat();
        p.z = ois.readFloat();

        return p;
    }

    private void writeObject(ObjectOutputStream s) throws IOException {
        s.defaultWriteObject();
        s.writeInt(mVersion);
        s.writeInt(mTargetWidth);
        s.writeInt(mTargetHeight);
        s.writeInt(mDuration);
        s.writeInt(mDelay);
        s.writeInt(mOpacity);
        writePoint(mAnchor, s);
        writePoint(mScale, s);
        writePoint(mPosition, s);
        writePoint(mRotation, s);
        s.writeObject(mMarker);

        s.writeInt(mAnimations.size());
        for (KeyframeData f : mAnimations) {
            s.writeObject(f);
        }
    }

    private void readObject(ObjectInputStream s) throws IOException, ClassNotFoundException {
        s.defaultReadObject();
        mVersion = s.readInt();
        mTargetWidth = s.readInt();
        mTargetHeight = s.readInt();
        mDuration = s.readInt();
        mDelay = s.readInt();
        mOpacity = s.readInt();
        mAnchor = readPoint(s);
        mScale = readPoint(s);
        mPosition = readPoint(s);
        mRotation = readPoint(s);
        mMarker = (KeyframeData)s.readObject();

        int numFrameData = s.readInt();
        mAnimations = new ArrayList<KeyframeData>();
        for (int i = 0; i < numFrameData; i++) {
            mAnimations.add((KeyframeData) s.readObject());
        }
    }

    /*
     * utilities for set/get initial positions.
     */
    public void setOpacity(int o) {
        mOpacity = o;
    }

    public int getOpacity() {
        return mOpacity;
    }

    public void setAnchor(float x, float y) {
        mAnchor.x = x;
        mAnchor.y = y;
    }

    public Point getAnchor() {
        return new Point(mAnchor.x, mAnchor.y, 0);
    }

    public void setScale(float x, float y) {
        mScale.x = x;
        mScale.y = y;
    }

    public Point getScale() {
        return new Point(mScale.x, mScale.y, 0);
    }

    public void setPosition(float x, float y, float z) {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
    }

    public Point getPosition() {
        return new Point(mPosition.x, mPosition.y, mPosition.z);
    }

    public void setRotation(float x, float y, float z) {
        mRotation.x = x;
        mRotation.y = y;
        mRotation.z = z;
    }

    public Point getRotation() {
        return new Point(mRotation.x, mRotation.y, mRotation.z);
    }

    public void setMarker(KeyframeData marker) {
        mMarker = marker;
    }

    public void setVersion(int version) {
        mVersion = version;
    }

    public void setTargetWidth(int width) {
        mTargetWidth = width;
    }

    public void setTargetHeight(int height) {
        mTargetHeight = height;
    }

    public int getTargetWidth() {
        return mTargetWidth;
    }

    public int getTargetHeight() {
        return mTargetHeight;
    }

}
