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

import android.content.Context;
import android.util.Log;
import com.mediatek.ngin3d.Utils;
import com.mediatek.ngin3d.utils.Ngin3dException;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;

/**
 * shall init 1 inflator for each composition object.
 *
 * @hide This level of detail should not be exposed in an abstract API
 */
public class KeyframeDataLoader {
    static final String ANCHOR_POINT = "AnchorPoint";
    static final String POINT_OF_INTEREST = "PointOfInterest";
    static final String POSITION = "Position";
    static final String SCALE = "Scale";
    static final String ROTATION = "Rotation";
    static final String XROTATION = "XRotation";
    static final String YROTATION = "YRotation";
    static final String ZROTATION = "ZRotation";
    static final String OPACITY = "Opacity";
    static final String ORIENTATION = "Orientation";
    static final String KEYFRAMES = "Keyframes";
    static final String FRAMES = "Frames";
    static final String PARAMETERS = "Parameters";
    static final String TRANSFORM = "Transform";
    static final String COMPOSITIONS = "Compositions";
    static final String LAYERS = "Layers";
    static final String MARKER = "Marker";
    static final String ACTION = "Action";
    static final String MARKER_TIME = "Time";
    static final String COMP_WIDTH = "Width";
    static final String COMP_HEIGHT = "Height";
    static final String DURATION = "Dur";

    static final String LAYER_WIDTH = "LayWidth";
    static final String LAYER_HEIGHT = "LayHeight";
    static final String COMPOSITION_IN = "CompIn";
    static final String COMPOSITION_DURATION = "CompDur";
    static final String NORMALIZATION = "Normalization";

    static final String VALUE = "Val";
    static final String X_AXIS = "X";
    static final String Y_AXIS = "Y";
    static final String Z_AXIS = "Z";
    static final String CURVE_TYPE = "Type";
    static final String KEYFRAME_TIME = "Time";
    static final String IN_TANX = "InTanX";
    static final String IN_TANY = "InTanY";
    static final String IN_TANZ = "InTanZ";
    static final String OUT_TANX = "OutTanX";
    static final String OUT_TANY = "OutTanY";
    static final String OUT_TANZ = "OutTanZ";
    static final String IN_TANVAL = IN_TANX;
    static final String OUT_TANVAL = OUT_TANX;
    static final String VERSION = "Version";

    static final String TAG = "KeyframeDataLoader";

    private JSONObject mProject;

    private static final int[] ANIMATION_TYPES = new int[] {
        Samples.TRANSLATE, Samples.ROTATE, Samples.SCALE, Samples.ALPHA,
        Samples.X_ROTATE, Samples.Y_ROTATE, Samples.Z_ROTATE, Samples.ANCHOR_POINT
    };

    public KeyframeDataLoader(Context context, int resId) {
        InputStream i = context.getResources().openRawResource(resId);

        try {
            int length = i.available();
            byte[] b = new byte[length];
            i.read(b, 0, length);
            final String s = new String(b, Charset.defaultCharset());
            JSONObject obj = new JSONObject(s);
            mProject = obj;
        } catch (IOException e) {
            throw new Ngin3dException(e);
        } catch (JSONException e) {
            throw new Ngin3dException(e);
        } finally {
            Utils.closeQuietly(i);
        }
    }

    /**
     * return a layer object with given name, if objectName is null, return the 1st layer
     * directly
     *
     * @param objectName layer name
     * @return JSONObject of layer
     */
    public JSONObject getLayer(String objectName) {
        JSONObject actorObj;

        final JSONArray actorsArray = mProject.optJSONArray(LAYERS);
        if (actorsArray == null) {
            // only 1 object, return.
            return mProject;
        } else {
            final int size = actorsArray.length();
            for (int i = 0; i < size; i++) {
                actorObj = actorsArray.optJSONObject(i);
                if (actorObj != null) {
                    if (objectName == null) {
                        Log.v(TAG, "objectName is null, return first layer directly");
                        return actorObj;
                    } else {
                        String s = actorObj.optString("LayName");
                        if (s.equals(objectName)) {
                            return actorObj;
                        }
                    }
                }
            }
        }

        return null;
    }

    private String getAnimationString(int animationType) {
        switch (animationType) {
        case Samples.TRANSLATE:
            return POSITION;

        case Samples.ROTATE:
            return ORIENTATION;

        case Samples.SCALE:
            return SCALE;

        case Samples.ALPHA:
            return OPACITY;

        case Samples.X_ROTATE:
            return XROTATION;

        case Samples.Y_ROTATE:
            return YROTATION;

        case Samples.Z_ROTATE:
            return ZROTATION;

        case Samples.ANCHOR_POINT:
            return ANCHOR_POINT;

        default:
            return null;
        }
    }

    private void fillJsonToStringArray(String[] dest, JSONArray src, final int size) {
        if (src == null) {
            Log.e(TAG, "Null src is specified");
            return;
        }
        for (int i = 0; i < size; i++) {
            dest[i] = src.optString(i);
        }
    }

    private void fillJsonToFloatArray(float[] dest, JSONArray src, final int size) {
        if (src == null) {
            Log.e(TAG, "Null src is specified");
            return;
        }
        for (int i = 0; i < size; i++) {
            dest[i] = (float) src.optDouble(i);
        }
    }

    private void fillJsonToArray(int[] dest, JSONArray src, final int size) {
        if (src == null) {
            Log.e(TAG, "Null src is specified");
            return;
        }
        for (int i = 0; i < size; i++) {
            dest[i] = src.optInt(i);
        }
    }

    private int getKeyframeVersion(JSONObject layer) {
        return layer.optInt(VERSION);
    }

    private int getTargetWidth(JSONObject layer) {
        return layer.optInt(LAYER_WIDTH);
    }

    private int getTargetHeight(JSONObject layer) {
        return layer.optInt(LAYER_HEIGHT);
    }

    private KeyframeData getMarkerData(JSONObject layer) {
        JSONObject marker = layer.optJSONObject(MARKER);
        if (marker == null)
            return null;
        JSONArray time = marker.optJSONArray(MARKER_TIME);
        if (time == null) {
            return null;
        }
        int markerSize = time.length();
        float[] pValues = new float[markerSize];
        String[] sValues = new String[markerSize];
        fillJsonToFloatArray(pValues, marker.optJSONArray(MARKER_TIME), markerSize);
        fillJsonToStringArray(sValues, marker.optJSONArray(ACTION), markerSize);
        Samples samples = new Samples(Samples.MARKER);
        samples.add(Samples.MARKER_TIME, pValues);
        samples.add(Samples.ACTION, sValues);
        return new KeyframeData(1, 1, samples);
    }

    private KeyframeData getKeyframeData(JSONObject layer, int sampleType) {
        String type = getAnimationString(sampleType);
        if (type == null || layer == null) {
            return null;
        }

        final float[] x;
        final float[] y;
        final float[] z;
        int duration = (int) (layer.optDouble(COMPOSITION_DURATION) * 1000);
        int delay = ((int) (layer.optDouble(COMPOSITION_IN) * 1000));
        boolean normalized = layer.optBoolean(NORMALIZATION);

        JSONObject anim = layer.optJSONObject(type);
        // parse general part
        // type, time
        if (anim == null) {
            Log.d(TAG, String.format("no Anime type %s , return null animation", type));
            return null;
        }
        Samples samples = new Samples(sampleType);

        JSONArray keyFrames;
        keyFrames = anim.optJSONArray(CURVE_TYPE);
        if (keyFrames == null) {
            if (sampleType == Samples.ALPHA) {
                float[] alpha = new float[1];
                alpha[0] = (float) anim.optDouble(VALUE);
                samples.add(Samples.VALUE, alpha);
                return new KeyframeData(0, 0, samples);
            } else if (sampleType == Samples.X_ROTATE || sampleType == Samples.Y_ROTATE
                || sampleType == Samples.Z_ROTATE) {
                return null;
            } else {
                x = new float[1];
                y = new float[1];
                z = new float[1];
                x[0] = (float) anim.optDouble(X_AXIS);
                y[0] = (float) anim.optDouble(Y_AXIS);
                z[0] = (float) anim.optDouble(Z_AXIS);
                samples.add(Samples.X_AXIS, x);
                samples.add(Samples.Y_AXIS, y);
                samples.add(Samples.Z_AXIS, z);
                return new KeyframeData(0, 0, samples, normalized);
            }
        }
        final int frameSize = keyFrames.length();

        int[] curveType = new int[frameSize];
        float[] time = new float[frameSize];
        fillJsonToArray(curveType, keyFrames, frameSize);
        fillJsonToFloatArray(time, anim.optJSONArray(KEYFRAME_TIME), frameSize);

        samples.add(Samples.CURVE_TYPE, curveType).add(Samples.KEYFRAME_TIME, time);

        // parse specific part
        if (sampleType == Samples.ALPHA) {
            // val, intanx, outtanx.
            float[][] pValues = new float[3][frameSize];
            fillJsonToFloatArray(pValues[0], anim.optJSONArray(VALUE), frameSize);
            fillJsonToFloatArray(pValues[1], anim.optJSONArray(IN_TANVAL), frameSize);
            fillJsonToFloatArray(pValues[2], anim.optJSONArray(OUT_TANVAL), frameSize);

            samples.add(Samples.VALUE, pValues[0])
                .add(Samples.IN_TANVAL, pValues[1])
                .add(Samples.OUT_TANVAL, pValues[2]);
        } else if (sampleType == Samples.X_ROTATE) {
            float[] zeroArray = new float[frameSize];
            float[][] xValues = new float[3][frameSize];
            fillJsonToFloatArray(xValues[0], anim.optJSONArray(VALUE), frameSize);
            fillJsonToFloatArray(xValues[1], anim.optJSONArray(IN_TANVAL), frameSize);
            fillJsonToFloatArray(xValues[2], anim.optJSONArray(OUT_TANVAL), frameSize);
            samples.add(Samples.X_AXIS, xValues[0])
                .add(Samples.IN_TANX, xValues[1])
                .add(Samples.OUT_TANX, xValues[2])
                .add(Samples.Y_AXIS, zeroArray)
                .add(Samples.IN_TANY, zeroArray)
                .add(Samples.OUT_TANY, zeroArray)
                .add(Samples.Z_AXIS, zeroArray)
                .add(Samples.IN_TANZ, zeroArray)
                .add(Samples.OUT_TANZ, zeroArray);
        } else if (sampleType == Samples.Y_ROTATE) {
            float[] zeroArray = new float[frameSize];
            float[][] yValues = new float[3][frameSize];
            fillJsonToFloatArray(yValues[0], anim.optJSONArray(VALUE), frameSize);
            fillJsonToFloatArray(yValues[1], anim.optJSONArray(IN_TANVAL), frameSize);
            fillJsonToFloatArray(yValues[2], anim.optJSONArray(OUT_TANVAL), frameSize);
            samples.add(Samples.X_AXIS, zeroArray)
                .add(Samples.IN_TANX, zeroArray)
                .add(Samples.OUT_TANX, zeroArray)
                .add(Samples.Y_AXIS, yValues[0])
                .add(Samples.IN_TANY, yValues[1])
                .add(Samples.OUT_TANY, yValues[2])
                .add(Samples.Z_AXIS, zeroArray)
                .add(Samples.IN_TANZ, zeroArray)
                .add(Samples.OUT_TANZ, zeroArray);
        } else if (sampleType == Samples.Z_ROTATE) {
            float[] zeroArray = new float[frameSize];
            float[][] zValues = new float[3][frameSize];
            fillJsonToFloatArray(zValues[0], anim.optJSONArray(VALUE), frameSize);
            fillJsonToFloatArray(zValues[1], anim.optJSONArray(IN_TANVAL), frameSize);
            fillJsonToFloatArray(zValues[2], anim.optJSONArray(OUT_TANVAL), frameSize);
            samples.add(Samples.X_AXIS, zeroArray)
                .add(Samples.IN_TANX, zeroArray)
                .add(Samples.OUT_TANX, zeroArray)
                .add(Samples.Y_AXIS, zeroArray)
                .add(Samples.IN_TANY, zeroArray)
                .add(Samples.OUT_TANY, zeroArray)
                .add(Samples.Z_AXIS, zValues[0])
                .add(Samples.IN_TANZ, zValues[1])
                .add(Samples.OUT_TANZ, zValues[2]);
        } else {
            float[][] values = new float[9][frameSize];
            fillJsonToFloatArray(values[0], anim.optJSONArray(X_AXIS), frameSize);
            fillJsonToFloatArray(values[1], anim.optJSONArray(IN_TANX), frameSize);
            fillJsonToFloatArray(values[2], anim.optJSONArray(OUT_TANX), frameSize);
            fillJsonToFloatArray(values[3], anim.optJSONArray(Y_AXIS), frameSize);
            fillJsonToFloatArray(values[4], anim.optJSONArray(IN_TANY), frameSize);
            fillJsonToFloatArray(values[5], anim.optJSONArray(OUT_TANY), frameSize);
            fillJsonToFloatArray(values[6], anim.optJSONArray(Z_AXIS), frameSize);
            fillJsonToFloatArray(values[7], anim.optJSONArray(IN_TANZ), frameSize);
            fillJsonToFloatArray(values[8], anim.optJSONArray(OUT_TANZ), frameSize);
            samples.add(Samples.X_AXIS, values[0])
                .add(Samples.IN_TANX, values[1])
                .add(Samples.OUT_TANX, values[2])
                .add(Samples.Y_AXIS, values[3])
                .add(Samples.IN_TANY, values[4])
                .add(Samples.OUT_TANY, values[5])
                .add(Samples.Z_AXIS, values[6])
                .add(Samples.IN_TANZ, values[7])
                .add(Samples.OUT_TANZ, values[8]);
        }
        return new KeyframeData(duration, delay, samples, normalized);
    }

    /**
     * Get layer animation
     *
     * @param name          layer name
     * @param animationType animation type
     * @return animation object
     */
    @Deprecated
    public KeyframeData getKeyframeData(String name, int animationType) {
        Log.v(TAG, "GetAnimation(): " + name);
        JSONObject layer = getLayer(name);

        // support old format json files.
        JSONObject para = layer.optJSONObject(PARAMETERS);
        if (para != null) {
            JSONObject trans = para.optJSONObject(TRANSFORM);
            if (trans != null) {
                Log.v(TAG, "Transform object exists");
                layer = trans;
            }
        }

        return getKeyframeData(layer, animationType);
    }

    /**
     * Get associated keyframe data set.
     *
     * @return maybe an empty keyframeDataSize. Check the size to know whether it's empty.
     */
    public KeyframeDataSet getKeyframeDataSet() {
        KeyframeDataSet dataSet = null;
        JSONObject actorObj = getLayer(null);

        for (int type : ANIMATION_TYPES) {
            KeyframeData data = getKeyframeData(actorObj, type);
            if (data != null) {
                if (dataSet == null) {
                    dataSet = new KeyframeDataSet();
                }
                dataSet.add(data);
            }
        }

        if (dataSet != null) {
            dataSet.setVersion(getKeyframeVersion(actorObj));
            dataSet.setTargetWidth(getTargetWidth(actorObj));
            dataSet.setTargetHeight(getTargetHeight(actorObj));
            dataSet.setMarker(getMarkerData(actorObj));
        }
        return dataSet;
    }

    private float getValue(JSONObject j, String item) {
        JSONArray v = j.optJSONArray(item);

        if (v == null) {
            return (float) j.optDouble(item);
        } else {
            return (float) v.optDouble(0);
        }
    }

}
