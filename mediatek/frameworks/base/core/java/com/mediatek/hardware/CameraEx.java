/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.hardware;

import android.hardware.Camera;
import android.hardware.Camera.MAVCallback;
import android.hardware.Camera.Parameters;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.xlog.Xlog;

/**
 * The CameraEx class is used to start/stop MAV capture. This class
 * depends on com.android.Camera.java and the client for the Camera service,
 * which manages the actual camera hardware.
 * 
 * The work flow for you to take a MAV photo is similar to taking a normal picture,
 * i.e. calling startMav instead of takePicture, and calling stopMAV after the capture number reaches
 * the number which is set when starting MAV.In addition, you can stop MAV in the process of capture,
 * and then set the parameter[isMerge] to false.
 */
public class CameraEx {
    private static final String TAG = "CameraEx";
    private CameraEx() {}

    /**
     * An interface which contains a callback for MAV.
     * You must implements it and define the behavior,
     * e.g. with a progress bar to show the progress of MAV.
     */
    public interface MavCallback
    {
        /**
         * Callback for MAV when a frame is done.
         * For example, if the first parameter of startMav is 25, it will be called 25 times firstly,
         * after that you should call stopMav and set the isMerge = 1 to save the MAV picture,
         * then it will be called at 26th time to inform you the picture is ready.
         */
        void onFrame();
    }

    /**
     * The method is starting capture the number of images of MAV.
     * The max. number is 25 and min. number is 1.
     * It will take a group photo used to display the view in different angles, and compress
     * these photos to a picture inside.
     *
     * @param num Entire number of MAV pictures
     * @param camera Instance of com.android.hardware.Camera.
     */
    public static void startMav(int num, Camera camera) {
	camera.startMAV(num);
    }

    /**
     * The method is to set MavCallback listener to camera device.
     *
     * @param cb Listener of MavCallback
     * @param camera Instance of com.android.hardware.Camera.
     */
    public static void setMavCallback(final MavCallback cb, Camera camera) {
        MAVCallback mMavCallback = new MAVCallback() {
            public void onFrame(byte[] jpegData) {
                if (cb != null) {
                    cb.onFrame();
                }
            }
        };
	camera.setMAVCallback(mMavCallback);
    }

    /**
     * The method is to stop MAV capture and user can set the flag--isMerge to 0 or 1 to indicate
     * the file should be saved or not. If isMerge is 1, there will be a callback when the merge is done.
     *
     * @param isMerge Flag for merging all the pictures together
     * @param camera Instance of com.android.hardware.Camera.
     */
    public static void stopMav(int isMerge, Camera camera) {
	camera.stopMAV(isMerge);
    }
}
