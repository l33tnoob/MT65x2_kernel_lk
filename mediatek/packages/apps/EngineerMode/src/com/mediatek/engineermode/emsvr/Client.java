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

import android.net.LocalSocket;
import android.net.LocalSocketAddress;

import com.mediatek.xlog.Xlog;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.charset.Charset;

/**
 * 
 * @author MTK80905
 */
public class Client {

    private static final String TAG = "EM/client";
    public static final int PARAM_TYPE_STRING = 1;
    public static final int PARAM_TYPE_INT = 2;
    private static final int PARAM_INT_LENGTH = 4;
    private static final String ERROR_NO_INIT = "NOT INIT";
    private static final String ERROR_PARAM_NUM = "param < 0";
    private static final int STATUS_SUCCESS = 0;
    private static final int STATUS_ERROR = -1;
    private static final int MAX_BUFFER_SIZE = 1024;
    private static final String EMPTY = "";
    private static final String EM_SERVER_NAME = "EngineerModeServer";

    private LocalSocket mSocket = null;
    private DataInputStream mInputStream = null;
    private DataOutputStream mOutputStream = null;
    private int mStatus = STATUS_SUCCESS;

    /**
     * Connect to EM native server
     */
    public void startClient() {
        try {
            // mSocket = new Socket("127.0.0.1", 37121);
            // mSocket(10000);
            mSocket = new LocalSocket();
            // LocalSocketAddress.Namespace.FILESYSTEM
            mSocket.connect(new LocalSocketAddress(EM_SERVER_NAME));
            mInputStream = new DataInputStream(mSocket.getInputStream());
            mOutputStream = new DataOutputStream(mSocket.getOutputStream());
            mStatus = STATUS_SUCCESS;
        } catch (IOException e) {
            Xlog.w(TAG, "startclient IOException " + e.getMessage());
            mStatus = STATUS_ERROR;
        }
    }

    /**
     * Read response string from EM server
     * 
     * @return Response string
     * @throws IOException
     *             Input stream exception
     */
    synchronized String read() throws IOException {
        if (STATUS_ERROR == mStatus || null == mInputStream) {
            throw new IOException(ERROR_NO_INIT);
        }
        String result = null;
        int len = mInputStream.readInt();
        if (len > MAX_BUFFER_SIZE) {
            len = MAX_BUFFER_SIZE;
        }
        byte bb[] = new byte[len];
        int x = mInputStream.read(bb, 0, len);
        if (-1 == x) {
            result = EMPTY;
        } else {
            result = new String(bb, Charset.defaultCharset());
        }
        return result;
    }

    /**
     * Send function number to EM server
     * 
     * @param functionNo
     *            Function ID
     * @throws IOException
     *             Output stream exception
     */
    synchronized void writeFunctionNo(String functionNo) throws IOException {
        if (STATUS_ERROR == mStatus || null == mOutputStream) {
            throw new IOException(ERROR_NO_INIT);
        }
        if (functionNo == null || functionNo.length() == 0) {
            return;
        }
        mOutputStream.writeInt(functionNo.length());
        mOutputStream.write(functionNo.getBytes(Charset.defaultCharset()), 0, functionNo.length());
        return;
    }

    /**
     * Send parameter number to EM server
     * 
     * @param paramNum
     *            Parameter total count
     * @throws IOException
     *             Output stream exception
     */
    synchronized void writeParamNo(int paramNum) throws IOException {
        if (STATUS_ERROR == mStatus || null == mOutputStream) {
            throw new IOException(ERROR_NO_INIT);
        }
        if (paramNum < 0) {
            throw new IOException(ERROR_PARAM_NUM);
        }
        mOutputStream.writeInt(paramNum);
    }

    /**
     * Send parameter to EM server
     * 
     * @param param
     *            Parameter
     * @throws IOException
     *             Output stream exception
     */
    synchronized void writeParamInt(int param) throws IOException {
        if (STATUS_ERROR == mStatus || null == mOutputStream) {
            throw new IOException(ERROR_NO_INIT);
        }
        mOutputStream.writeInt(PARAM_TYPE_INT);
        mOutputStream.writeInt(PARAM_INT_LENGTH);
        mOutputStream.writeInt(param);
    }

    /**
     * Send string to EM server as parameter
     * 
     * @param param
     *            Parameter
     * @throws IOException
     *             Output stream exception
     */
    synchronized void writeParamString(String param) throws IOException {
        if (STATUS_ERROR == mStatus || null == mOutputStream) {
            throw new IOException(ERROR_NO_INIT);
        }
        mOutputStream.writeInt(PARAM_TYPE_STRING);
        mOutputStream.writeInt(param.length());
        mOutputStream.write(param.getBytes(Charset.defaultCharset()), 0, param.length());
    }

    /**
     * Stop connection with EM server
     */
    public void stopClient() {
        if (null == mInputStream || null == mOutputStream || null == mSocket) {
            return;
        }
        try {
            mOutputStream.close();
            mInputStream.close();
            mSocket.close();
        } catch (IOException e) {
            Xlog.w(TAG, "stop client IOException: " + e.getMessage());
        }
    }

}
