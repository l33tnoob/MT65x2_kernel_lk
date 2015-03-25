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

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mediatek.engineermode.emsvr;

import com.mediatek.xlog.Xlog;

import java.io.EOFException;
import java.io.IOException;

/**
 * 
 * @author MTK80905
 */
public class AFMFunctionCallEx {

    private static final String TAG = "EM/functioncall";

    public static final int FUNCTION_EM_BASEBAND = 10001;
    public static final int FUNCTION_EM_CPU_FREQ_TEST_START = 20001;
    public static final int FUNCTION_EM_CPU_FREQ_TEST_STOP = 20002;
    public static final int FUNCTION_EM_CPU_FREQ_TEST_CURRENCT = 20003;
    public static final int FUNCTION_EM_FB0_IOCTL = 30001;
    public static final int FUNCTION_EM_CPU_STRESS_TEST_APMCU = 40001;
    public static final int FUNCTION_EM_CPU_STRESS_TEST_SWCODEC = 40002;
    public static final int FUNCTION_EM_CPU_STRESS_TEST_BACKUP = 40003;
    public static final int FUNCTION_EM_CPU_STRESS_TEST_THERMAL = 40004;
    public static final int FUNCTION_EM_SENSOR_DO_CALIBRATION = 50001;
    public static final int FUNCTION_EM_SENSOR_CLEAR_CALIBRATION = 50002;
    public static final int FUNCTION_EM_SENSOR_SET_THRESHOLD = 50003;
    public static final int FUNCTION_EM_SENSOR_DO_GSENSOR_CALIBRATION = 50004;
    public static final int FUNCTION_EM_SENSOR_GET_GSENSOR_CALIBRATION = 50005;
    public static final int FUNCTION_EM_SENSOR_CLEAR_GSENSOR_CALIBRATION = 50006;
    public static final int FUNCTION_EM_SENSOR_DO_GYROSCOPE_CALIBRATION = 50007;
    public static final int FUNCTION_EM_SENSOR_GET_GYROSCOPE_CALIBRATION = 50008;
    public static final int FUNCTION_EM_SENSOR_CLEAR_GYROSCOPE_CALIBRATION = 50009;
    public static final int FUNCTION_EM_DFO_INIT = 60001;
    public static final int FUNCTION_EM_DFO_READ_COUNT = 60002;
    public static final int FUNCTION_EM_DFO_READ = 60003;
    public static final int FUNCTION_EM_DFO_WRITE = 60004;
    public static final int FUNCTION_EM_DFO_DEINIT = 60005;
    public static final int FUNCTION_EM_DFO_PROPERTY_SET = 60006;
    public static final int FUNCTION_EM_DFO_GET_DEFAULT_SIZE = 60007;
    public static final int FUNCTION_EM_MSDC_SET_CURRENT = 70001;
    public static final int FUNCTION_EM_MSDC_GET_CURRENT = 70002;
    public static final int FUNCTION_EM_MSDC_SET30_MODE = 70003;
     
    public static final int RESULT_FIN = 0;
    public static final int RESULT_CONTINUE = 1;
    public static final int RESULT_IO_ERR = -1;

    private static final String ERROR = "ERROR ";

    private Client mSocket = null;

    /**
     * Prepare to call function by EM native server
     * 
     * @param functionId
     *            Functional ID
     * @return True if call function success
     */
    public boolean startCallFunctionStringReturn(int functionId) {
        boolean result = false;
        mSocket = new Client();
        mSocket.startClient();
        try {
            mSocket.writeFunctionNo(String.valueOf(functionId));
            result = true;
        } catch (IOException e) {
            Xlog.w(TAG, "StartCallFunctionStringReturnEXP " + e.getMessage());
            result = false;
        }
        return result;
    }

    /**
     * Send parameter number
     * 
     * @param number
     *            Parameters number
     * @return True if send success to server
     */
    public boolean writeParamNo(int number) {
        boolean ret = false;
        try {
            mSocket.writeParamNo(number);
            ret = true;
        } catch (IOException e) {
            ret = false;
        }
        return ret;
    }

    /**
     * Send "int" parameter to server
     * 
     * @param param
     *            Parameter
     * @return True if send success to server
     */
    public boolean writeParamInt(int param) {
        boolean ret = false;
        try {
            mSocket.writeParamInt(param);
            ret = true;

        } catch (IOException e) {
            ret = false;
        }
        return ret;
    }

    /**
     * Send string to server as parameter
     * 
     * @param string
     *            Parameter
     * @return True if send success to server
     */
    public boolean writeParamString(String string) {
        boolean ret = false;
        try {
            mSocket.writeParamString(string);
            ret = true;
        } catch (IOException e) {
            ret = false;
        }
        return ret;
    }

    /**
     * Get response from EM server
     * 
     * @return #FunctionReturn returnCode=0 if socket close normally
     *         returnCode=1 has more data to be read. returnCode = -1 close
     *         abnormally.
     */
    public FunctionReturn getNextResult() {
        FunctionReturn ret = new FunctionReturn();
        try {
            ret.mReturnString = mSocket.read();
            if (ret.mReturnString.isEmpty()) {
                ret.mReturnCode = RESULT_FIN;
                endCallFunction();
            } else {
                ret.mReturnCode = RESULT_CONTINUE;
            }
        } catch (EOFException e) {
            endCallFunction();
            ret.mReturnCode = RESULT_FIN;
            ret.mReturnString = "";
        } catch (IOException e) {
            endCallFunction();
            ret.mReturnCode = RESULT_IO_ERR;
            ret.mReturnString = ERROR + e.getMessage();
        }
        return ret;
    }

    /**
     * Stop connection with EM server
     */
    private void endCallFunction() {
        mSocket.stopClient();
    }

}
