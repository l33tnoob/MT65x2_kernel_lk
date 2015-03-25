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

package com.mediatek.engineermode.dfo;

import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;
import java.util.ArrayList;

public class DfoNative {
    private static final String TAG = "DfoNative";
    public static class DfoReadCount {
        public int count;
    }

    public static class DfoReadReq {
        public int index;
    }

    public static class DfoReadCnf {
        public String name;
        public int value;
        public int partition;
    }

    public static class DfoWriteReq {
        public String name;
        public int value;
        public int partition;
        public int save;
    }

    public static class DfoDefaultSize {
        public int width;
        public int height;
    }

    public static int RET_SUCCESS = 0;
    public static int RET_FAILED = -1;

    public static int init() {
        String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_INIT, 0);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int readCount(DfoReadCount count) {
        try {
            String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_READ_COUNT, 0);
            Xlog.d(TAG, "len " + ret.length);
            Xlog.d(TAG, "ret " + ret[0]);
            if (ret.length > 1 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
                count.count = Integer.parseInt(ret[1]);
                return RET_SUCCESS;
            }
        } catch (NumberFormatException e) {
            // fall through
        }
        return RET_FAILED;
    }

    public static int read(DfoReadReq req, DfoReadCnf cnf) {
        try {
            String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_READ, 1, req.index);
            if (ret.length > 3 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
                cnf.name = ret[1];
                cnf.value = Integer.parseInt(ret[2]);
                cnf.partition = Integer.parseInt(ret[2]);
                return RET_SUCCESS;
            }
        } catch (NumberFormatException e) {
            // fall through
        }
        return RET_FAILED;
    }

    public static int write(DfoWriteReq req) {
        String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_WRITE, 4,
                req.name, req.value, req.partition, req.save);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int deinit() {
        String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_DEINIT, 0);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int propertySet(int height, int width) {
        String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_PROPERTY_SET, 2, height, width);
        if (ret.length > 0 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
            return RET_SUCCESS;
        }
        return RET_FAILED;
    }

    public static int getDefaultSize(DfoDefaultSize size) {
        try {
            String[] ret = runCmdInEmSvr(AFMFunctionCallEx.FUNCTION_EM_DFO_GET_DEFAULT_SIZE, 0);
            if (ret.length > 2 && String.valueOf(RET_SUCCESS).equals(ret[0])) {
                size.width = Integer.parseInt(ret[1]);
                size.height = Integer.parseInt(ret[2]);
                return RET_SUCCESS;
            }
        } catch (NumberFormatException e) {
            // fall through
        }
        return RET_FAILED;
    }

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

    static public String[] runCmdInEmSvr(int index, int paramNum, String param0, int... param) {
        ArrayList<String> arrayList = new ArrayList<String>();
        AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
        boolean result = functionCall.startCallFunctionStringReturn(index);
        functionCall.writeParamNo(paramNum);
        functionCall.writeParamString(param0);
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

}
