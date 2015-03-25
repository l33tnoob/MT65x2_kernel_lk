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

package com.mediatek.engineermode.sensor;

import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.xlog.Xlog;
import java.util.ArrayList;

public class EmSensor {
    private static final String TAG = "EmSensor";
    public static int RET_SUCCESS = 1;
    public static int RET_FAILED = 0;

    public static int doGsensorCalibration(int tolerance) {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_DO_GSENSOR_CALIBRATION, 1,
                tolerance);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int getGsensorCalibration(float[] result) {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_GET_GSENSOR_CALIBRATION, 0);
        if (ret.length >= 4 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            try {
                result[0] = Float.parseFloat(ret[1]);
                result[1] = Float.parseFloat(ret[2]);
                result[2] = Float.parseFloat(ret[3]);
                return RET_SUCCESS;
            } catch (NumberFormatException e) {
                // Let it return RET_FAILED
            }
        }
        return RET_FAILED;
    }

    public static int clearGsensorCalibration() {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_CLEAR_GSENSOR_CALIBRATION, 0);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int doGyroscopeCalibration(int tolerance) {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_DO_GYROSCOPE_CALIBRATION, 1,
                tolerance);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int getGyroscopeCalibration(float[] result) {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_GET_GYROSCOPE_CALIBRATION, 0);
        if (ret.length >= 4 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            try {
                result[0] = Float.parseFloat(ret[1]);
                result[1] = Float.parseFloat(ret[2]);
                result[2] = Float.parseFloat(ret[3]);
                return RET_SUCCESS;
            } catch (NumberFormatException e) {
                // Let it return RET_FAILED
            }
        }
        return RET_FAILED;
    }

    public static int clearGyroscopeCalibration() {
        String[] ret = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_CLEAR_GYROSCOPE_CALIBRATION, 0);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int doPsensorCalibration(int min, int max) {
        String ret[] = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_DO_CALIBRATION, 2,
                min, max);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int clearPsensorCalibration() {
        String ret[] = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_CLEAR_CALIBRATION, 0);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int setPsensorThreshold(int high, int low) {
        String ret[] = runCmdInEmSvr(
                AFMFunctionCallEx.FUNCTION_EM_SENSOR_SET_THRESHOLD, 2,
                high, low);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static native int getPsensorData();

    public static native int getPsensorThreshold(int[] result);

    public static native int calculatePsensorMinValue();

    public static native int getPsensorMinValue();

    public static native int calculatePsensorMaxValue();

    public static native int getPsensorMaxValue();

    static public String[] runCmdInEmSvr(int index, int paramNum, int... param) {
        ArrayList<String> arrayList = new ArrayList<String>();
        AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
        boolean result = functionCall.startCallFunctionStringReturn(index);
        functionCall.writeParamNo(paramNum);
        for (int i : param) {
            functionCall.writeParamInt(i);
        }
        if (result) {
            FunctionReturn r;
            do {
                r = functionCall.getNextResult();
                if (r.mReturnString.isEmpty()) {
                    break;
                }
                arrayList.add(r.mReturnString);
            } while (r.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
            if (r.mReturnCode == AFMFunctionCallEx.RESULT_IO_ERR) {
                Xlog.d(TAG, "AFMFunctionCallEx: RESULT_IO_ERR");
                arrayList.clear();
                arrayList.add("ERROR");
            }
        } else {
            Xlog.d(TAG, "AFMFunctionCallEx return false");
            arrayList.clear();
            arrayList.add("ERROR");
        }
        return arrayList.toArray(new String[arrayList.size()]);
    }

    static {
        System.loadLibrary("em_sensor_jni");
    }
}
