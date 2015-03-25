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

package com.mediatek.apst.target.ftp;

import com.mediatek.apst.target.util.Debugger;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.BindException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.sql.Date;
import java.text.SimpleDateFormat;
import java.util.Locale;

public class FtpConnection extends Thread {

    private static final Object[] TAG = new Object[] { "APST/FTP" };
    private static final boolean LOGGABLE = false;
    private static final String USERNAME = "APST";
    private static final String PASSWORD = "APST";
    private static final int TIME_OUT = 6000;
    private static final int DATA_PORT = 19878;
    private static final int BUFFER_SIZE = 102400;
    private static final String FTP_CMD_USER = "USER";
    private static final String FTP_CMD_PASS = "PASS";
    private static final String FTP_CMD_QUIT = "QUIT";
    private static final String FTP_CMD_TYPE = "TYPE";
    private static final String FTP_CMD_OPTS = "OPTS";
    private static final String FTP_CMD_SYST = "SYST";
    private static final String FTP_CMD_NOOP = "NOOP";
    private static final String FTP_CMD_CWD = "CWD";
    private static final String FTP_CMD_CDUP = "CDUP";
    private static final String FTP_CMD_SIZE = "SIZE";
    private static final String FTP_CMD_PWD = "PWD";
    private static final String FTP_CMD_PORT = "PORT";
    private static final String FTP_CMD_PASV = "PASV";
    private static final String FTP_CMD_RETR = "RETR";
    private static final String FTP_CMD_STOR = "STOR";
    private static final String FTP_CMD_LIST = "LIST";
    private static final String FTP_CMD_DELE = "DELE";
    private static final String FTP_CMD_MKD = "MKD";
    private static final String FTP_CMD_RNFR = "RNFR";
    private static final String FTP_CMD_RNTO = "RNTO";
    private static final String FTP_CMD_RMD = "RMD";
    private static final String FTP_CMD_FEAT = "FEAT";
    private static final String FTP_CMD_SITE = "SITE";
    private String mCurrentDir = "/";
    private Socket mSocket = null;;
    private BufferedReader mReader = null;
    private BufferedOutputStream mWriter = null;
    private ServerSocket mPasvSocket = null;
    private final String mHost = "127.0.0.1";
    private int mPort = 19878;
    private boolean mBinaryMode = false;
    private String mUserName = null;
    private String mPassword = null;
    private File mRenameFrom = null;
    private String mFtpCharset = null;

    // The approximate number of milliseconds in 6 months
    private static final long MS_IN_SIX_MONTHS = 6 * 30 * 24 * 60 * 60 * 1000;

    /**
     * @param socket
     *            The socket used to communication.
     */
    public FtpConnection(Socket socket) {
        this.mSocket = socket;
        this.mFtpCharset = FtpService.sFtpEncoding;
        Debugger.logW(TAG, "FtpCharset: " + mFtpCharset);
    }

    /*
     * (non-Javadoc)
     * 
     * @see java.lang.Thread#run()
     */
    public void run() {
        String command;
        try {
            mReader = new BufferedReader(new InputStreamReader(mSocket
                    .getInputStream(), mFtpCharset), 8192);
            mWriter = new BufferedOutputStream(mSocket.getOutputStream(), 8192);
            response("220-Welcome message......\r\n");
            response("220 Notice:last msg\r\n");
            for (;;) {
                command = mReader.readLine();
                if (command == null) {
                    break;
                }
                parseCommand(command);
                if (command.equals("QUIT")) {
                    break;
                }
            }
        } catch (SocketException se) {
            Debugger.logW(TAG, "SocketException: " + se.getMessage());
        } catch (IOException e) {
            Debugger.logW(TAG, "IOException: Start Ftp connection error!"
                    + e.getMessage());
        } finally {
            try {
                if (mReader != null) {
                    mReader.close();
                    mReader = null;
                }
            } catch (IOException e) {
                Debugger.logW(TAG, "reader exception: " + e.getMessage());
            }
            try {
                if (mWriter != null) {
                    mWriter.close();
                    mWriter = null;
                }
            } catch (IOException e) {
                Debugger.logW(TAG, "writer exception: " + e.getMessage());
            }
            try {
                if (this.mPasvSocket != null) {
                    mPasvSocket.close();
                    mPasvSocket = null;
                }
            } catch (IOException e) {
                Debugger.logW(TAG, "pasvSocket exception: " + e.getMessage());
            }
            try {
                if (this.mSocket != null) {
                    mSocket.close();
                    mSocket = null;
                }
            } catch (IOException e) {
                Debugger.logW(TAG, "socket exception: " + e.getMessage());
            }
        }
    }

    /**
     * Send message to client
     * 
     * @param s
     *            The message send to client.
     * @throws IOException
     *             Signals a general, I/O-related error.
     */
    private void response(String s) throws IOException {
        byte[] strBytes;
        try {
            strBytes = s.getBytes(mFtpCharset);
        } catch (UnsupportedEncodingException e) {
            Debugger.logW(TAG, "Unsupported encoding: " + mFtpCharset);
            strBytes = s.getBytes();
        }
        mWriter.write(strBytes);
        mWriter.flush();
    }

    /**
     * Process command
     * 
     * @param s
     *            The command to parse.
     * @throws IOException
     *             Signals a general, I/O-related error.
     */
    private void parseCommand(String s) throws IOException {
        if (s == null || s.equals("")) {
            return;
        }
        String upperCmd = s.toUpperCase();
        if (upperCmd.startsWith(FTP_CMD_USER)) {
            mUserName = getParameter(s, false);
            response("331 Send password\r\n");
        } else if (upperCmd.startsWith(FTP_CMD_PASS)) {
            mPassword = getParameter(s, false);
            if (mUserName.equals(USERNAME) && mPassword.equals(PASSWORD)) {
                ftpLog(TAG, "Username Password Correct!");
                response("230 Access granted\r\n");
            } else {
                response("530 Login incorrect.\r\n");
            }
        } else if (s.equalsIgnoreCase(FTP_CMD_QUIT)) {
            ftpLog(TAG, "QUIT");
            response("221 Goodbye\r\n");
        } else if (upperCmd.startsWith(FTP_CMD_TYPE)) {
            String output;
            ftpLog(TAG, "TYPE executing");
            String param = getParameter(s, false);
            if (param.equals("I") || param.equals("L 8")) {
                mBinaryMode = true;
                output = "200 Binary type set\r\n";
            } else if (param.equals("A") || param.equals("A N")) {
                mBinaryMode = false;
                output = "200 ASCII type set\r\n";
            } else {
                output = "503 Malformed TYPE command\r\n";
            }
            response(output);
            ftpLog(TAG, "TYPE complete");
        } else if (upperCmd.startsWith(FTP_CMD_OPTS)) {
            String param = getParameter(s, false);
            String errString = null;
            mainBlock: {
                if (param == null) {
                    errString = "550 Need argument to OPTS\r\n";
                    Debugger
                            .logD(TAG, "Couldn't understand empty OPTS command");
                    break mainBlock;
                }
                String[] splits = param.split(" ");
                if (splits.length != 2) {
                    errString = "550 Malformed OPTS command\r\n";
                    Debugger.logD(TAG, "Couldn't parse OPTS command");
                    break mainBlock;
                }
                String optName = splits[0].toUpperCase();
                String optVal = splits[1].toUpperCase();
                if (optName.equals("UTF8")) {
                    if (optVal.equals("ON")) {
                        Debugger.logI(TAG, "Got OPTS UTF8 ON");
                    } else {
                        ftpLog(TAG,
                                "Ignoring OPTS UTF8 for something besides ON");
                    }
                    break mainBlock;
                } else {
                    Debugger.logI(TAG, "Unrecognized OPTS option: " + optName);
                    errString = "502 Unrecognized option\r\n";
                    break mainBlock;
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "Template log message");
            } else {
                response("504 OPTS accepted\r\n");
                Debugger.logI(TAG, "Handled OPTS ok");
            }
        } else if (s.equalsIgnoreCase(FTP_CMD_SYST)) {
            ftpLog(TAG, "SYST executing");
            response("215 UNIX Type: L8\r\n");
            ftpLog(TAG, "SYST finished");
        } else if (s.equalsIgnoreCase(FTP_CMD_NOOP)) {
            response("200 NOOP OK\r\n");
        } else if (upperCmd.startsWith(FTP_CMD_CWD)) {
            ftpLog(TAG, "CWD executing");
            String param = getParameter(s, false);
            File newDir;
            String errString = null;
            mainblock: {
                ftpLog(TAG, "WorkingDir ---------------->" + this.mCurrentDir);
                ftpLog(TAG, "Param ---------------->" + param);
                newDir = inputPathToChrootedFile(new File(this.mCurrentDir),
                        param);

                // Ensure the new path does not violate the chroot restriction
                if (violatesChroot(newDir)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    response(errString);
                    ftpLog(TAG, errString);
                    break mainblock;
                }
                try {
                    newDir = newDir.getCanonicalFile();
                    // if (!newDir.isDirectory()) {
                    // response("550 Can't CWD to invalid directory\r\n");
                    // } else if (newDir.canRead()) {
                    if (newDir.canExecute()) {
                        this.mCurrentDir = newDir.getCanonicalPath();
                        response("250 CWD successful\r\n");
                    } else {
                        response("550 That path is inaccessible\r\n");
                    }
                } catch (IOException e) {
                    response("550 Invalid path\r\n");
                    break mainblock;
                }
            }
            ftpLog(TAG, "CWD complete");
        } else if (s.equalsIgnoreCase(FTP_CMD_CDUP)) {
            ftpLog(TAG, "CDUP executing");
            File newDir;
            String errString = null;
            mainBlock: {
                File workingDir = new File(this.mCurrentDir);
                newDir = workingDir.getParentFile();
                if (newDir == null) {
                    errString = "550 Current dir cannot find parent\r\n";
                    break mainBlock;
                }
                // Ensure the new path does not violate the chroot restriction
                if (violatesChroot(newDir)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainBlock;
                }
                try {
                    newDir = newDir.getCanonicalFile();
                    // if (!newDir.isDirectory()) {
                    // errString = "550 Can't CWD to invalid directory\r\n";
                    // break mainBlock;
                    // } else if (newDir.canRead()) {
                    if (newDir.canRead()) {
                        this.mCurrentDir = newDir.getCanonicalPath();
                    } else {
                        errString = "550 That path is inaccessible\r\n";
                        break mainBlock;
                    }
                } catch (IOException e) {
                    errString = "550 Invalid path\r\n";
                    break mainBlock;
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "CDUP error: " + errString);
            } else {
                response("200 CDUP successful\r\n");
                ftpLog(TAG, "CDUP success");
            }
        } else if (upperCmd.startsWith(FTP_CMD_SIZE)) {
            ftpLog(TAG, "SIZE executing");
            String errString = null;
            String param = getParameter(s, false);
            long size = 0;
            mainblock: {
                File currentDir = new File(this.mCurrentDir);
                File target = null;
                if (param.contains(File.separator)) {
                    target = new File(param);
                    if (target.isDirectory()) {
                        errString = "550 No directory traversal allowed in SIZE param\r\n";
                        break mainblock;
                    }
                } else {
                    target = inputPathToChrootedFile(currentDir, param);
                }

                // We should have caught any invalid location access before now,
                // but
                // here we check again, just to be explicitly sure.
                if (violatesChroot(target)) {
                    errString = "550 SIZE target violates chroot\r\n";
                    break mainblock;
                }
                if (!target.exists()) {
                    errString = "550 Cannot get the SIZE of nonexistent object\r\n";
                    break mainblock;
                }
                if (!target.isFile()) {
                    errString = "550 Cannot get the size of a non-file\r\n";
                    break mainblock;
                }
                size = target.length();
            }
            if (errString != null) {
                response(errString);
            } else {
                response("213 " + size + "\r\n");
            }
            ftpLog(TAG, "SIZE complete");
        } else if (s.equalsIgnoreCase(FTP_CMD_PWD)) {
            ftpLog(TAG, "PWD executing");
            try {
                String currentDir = new File(this.mCurrentDir)
                        .getCanonicalPath();
                ftpLog(TAG, "currentDir: " + currentDir);
                // currentDir = currentDir.substring(new File("/")
                // .getCanonicalPath().length());
                // Debugger.logW(TAG, "currentDir: " + currentDir);
                // The root directory requires special handling to restore its
                // leading slash
                if (currentDir.length() == 0) {
                    currentDir = "/";
                }
                response("257 \"" + currentDir + "\"\r\n");
            } catch (IOException e) {
                ftpLog(TAG, "PWD canonicalize");
            }
            ftpLog(TAG, "PWD complete");
        } else if (upperCmd.startsWith(FTP_CMD_PORT)) {
            if (mPasvSocket != null) {
                mPasvSocket.close();
                mPasvSocket = null;
            }
            String errString = null;
            mainBlock: {
                String param = getParameter(s, false);
                if (param.contains("|") && param.contains("::")) {
                    errString = "550 No IPv6 support, reconfigure your client\r\n";
                    break mainBlock;
                }
                String[] substrs = param.split(",");
                if (substrs.length != 6) {
                    errString = "550 Malformed PORT argument\r\n";
                    break mainBlock;
                }
                for (int i = 0; i < substrs.length; i++) {
                    // Check that each IP/port octet is numeric and not too long
                    if (!substrs[i].matches("[0-9]+")
                            || substrs[i].length() > 3) {
                        errString = "550 Invalid PORT argument: " + substrs[i]
                                + "\r\n";
                        break mainBlock;
                    }
                }
                byte[] ipBytes = new byte[4];
                for (int i = 0; i < 4; i++) {
                    try {
                        // We have to manually convert unsigned to signed
                        // byte representation.
                        int ipByteAsInt = Integer.parseInt(substrs[i]);
                        if (ipByteAsInt >= 128) {
                            ipByteAsInt -= 256;
                        }
                        ipBytes[i] = (byte) ipByteAsInt;
                    } catch (NumberFormatException e) {
                        errString = "550 Invalid PORT format: " + substrs[i]
                                + "\r\n";
                        break mainBlock;
                    }
                }

                mPort = Integer.parseInt(substrs[4]) * 256
                        + Integer.parseInt(substrs[5]);
            }
            if (errString == null) {
                response("200 PORT OK\r\n");
                ftpLog(TAG, "PORT completed");
            } else {
                ftpLog(TAG, "PORT error: " + errString);
                response(errString);
            }
        } else if (s.equalsIgnoreCase(FTP_CMD_PASV)) {
            if (mPasvSocket != null) {
                ftpLog(TAG, "PasvSocket is not null: pasvSocket.close()");
                mPasvSocket.close();
                mPasvSocket = null;
            }
            ftpLog(TAG, "PASV mode begin");
            try {
                boolean bindFalse = true;
                int count = 0;
                do {
                    try {
                        Thread.sleep(100);
                        if (count++ > 50) {
                            break;
                        }
                        mPasvSocket = new ServerSocket(DATA_PORT, 50);
                        bindFalse = false;
                        ftpLog(TAG,
                                "PasvSocket: New Pasv ServerSocket! Data Port: "
                                        + DATA_PORT);
                    } catch (BindException e) {
                        Debugger.logW(TAG, "BindException: " + e.getMessage());
                        bindFalse = true;
                    } catch (SocketException e) {
                        Debugger.logW(TAG, "SocketException" + e.getMessage());
                        bindFalse = true;
                    }
                } while (bindFalse);
                InetAddress addr = mSocket.getLocalAddress();
                if (addr == null) {
                    Debugger.logW(TAG, "PASV IP string invalid");
                    response("502 Couldn't open a port\r\n");
                    return;
                }
                ftpLog(TAG, "PASV sending IP: " + addr.getHostAddress());
                StringBuilder response = new StringBuilder(
                        "227 Entering Passive Mode (");
                // Output our IP address in the format xxx,xxx,xxx,xxx
                response.append(addr.getHostAddress().replace('.', ','));
                response.append(",");
                // Output our port in the format p1,p2 where port=p1*256+p2
                response.append(DATA_PORT / 256);
                response.append(",");
                response.append(DATA_PORT % 256);
                response.append(").\r\n");
                String responseString = response.toString();
                response(responseString);
            } catch (InterruptedException e) {
                Debugger.logW(TAG, "PASV Exception: " + e.getMessage());
                if (mPasvSocket != null) {
                    mPasvSocket.close();
                    mPasvSocket = null;
                }
            }
        } else if (upperCmd.startsWith(FTP_CMD_RETR)) {
            ftpLog(TAG, "RETR executing");
            String param = getParameter(s, false);
            File fileToRetr;
            String errString = null;
            Socket dataSocket;
            if (mPasvSocket != null) {
                dataSocket = mPasvSocket.accept();
            } else {
                dataSocket = new Socket(this.mHost, this.mPort);
                Debugger.logW(TAG, "dataSocket: New Data Socket!");
            }
            dataSocket.setSoTimeout(TIME_OUT);
            mainblock: {
                fileToRetr = inputPathToChrootedFile(
                        new File(this.mCurrentDir), param);
                ftpLog(TAG, "Download file dir = "
                        + fileToRetr.getCanonicalPath());
                if (violatesChroot(fileToRetr)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainblock;
                } else if (fileToRetr.isDirectory()) {
                    ftpLog(TAG, "Ignoring RETR for directory");
                    errString = "550 Can't RETR a directory\r\n";
                    break mainblock;
                } else if (!fileToRetr.exists()) {
                    ftpLog(TAG, "Can't RETR nonexistent file: "
                            + fileToRetr.getAbsolutePath());
                    errString = "550 File does not exist\r\n";
                    break mainblock;
                } else if (!fileToRetr.canRead()) {
                    ftpLog(TAG, "Failed RETR permission (canRead() is false)");
                    errString = "550 No read permissions\r\n";
                    break mainblock;
                }
                FileInputStream in = null;
                try {
                    in = new FileInputStream(fileToRetr);
                    byte[] buffer = new byte[BUFFER_SIZE];
                    int bytesRead;
                    response("150 Sending file\r\n");
                    if (mBinaryMode) {
                        ftpLog(TAG, "Transferring in binary mode");
                        while ((bytesRead = in.read(buffer)) != -1) {
                            if (!sendViaDataSocket(buffer, 0, bytesRead,
                                    dataSocket.getOutputStream())) {
                                errString = "426 Data socket error\r\n";
                                Debugger.logW(TAG, "Data socket error");
                                break mainblock;
                            }
                        }
                    } else {
                        ftpLog(TAG, "Transferring in ASCII mode");
                        boolean lastBufEndedWithCR = false;
                        while ((bytesRead = in.read(buffer)) != -1) {
                            int startPos = 0;
                            int endPos = 0;
                            byte[] crnBuf = { '\r', '\n' };
                            // Added by MTK54046 @ 2011-10-11
                            buffer = new String(buffer).getBytes(mFtpCharset);
                            bytesRead = buffer.length;
                            for (endPos = 0; endPos < bytesRead; endPos++) {
                                if (buffer[endPos] == '\n') {
                                    sendViaDataSocket(buffer, startPos, endPos
                                            - startPos, dataSocket
                                            .getOutputStream());
                                    if (endPos == 0) {
                                        // handle special case where newline
                                        // occurs at the beginning of a buffer
                                        if (!lastBufEndedWithCR) {
                                            // Send an \r only if the the
                                            // previous
                                            // buffer didn't end with an \r
                                            sendViaDataSocket(crnBuf, 0, 1,
                                                    dataSocket
                                                            .getOutputStream());
                                        }
                                    } else if (buffer[endPos - 1] != '\r') {
                                        // The file did not have \r before \n,
                                        // add it
                                        sendViaDataSocket(crnBuf, 0, 1,
                                                dataSocket.getOutputStream());
                                    } else {
                                        // The file did have \r before \n, don't
                                        // change
                                        ftpLog(TAG, "parseCommand");
                                    }
                                    startPos = endPos;
                                }
                            }
                            sendViaDataSocket(buffer, startPos, endPos
                                    - startPos, dataSocket.getOutputStream());
                            if (buffer[bytesRead - 1] == '\r') {
                                lastBufEndedWithCR = true;
                            } else {
                                lastBufEndedWithCR = false;
                            }
                        }
                    }
                } catch (FileNotFoundException e) {
                    errString = "550 File not found\r\n";
                    break mainblock;
                } catch (IOException e) {
                    errString = "425 Network error\r\n";
                    break mainblock;
                } finally {
                    if (null != in) {
                        in.close();
                        in = null;
                    }
                }
            }
            try {
                if (errString != null) {
                    response(errString);
                } else {
                    response("226 Transmission finished\r\n");
                }
            } catch (IOException e) {
                Debugger.logW(TAG, "Exception: " + e.getMessage());
            } finally {
                dataSocket.close();
                dataSocket = null;
                if (mPasvSocket != null) {
                    mPasvSocket.close();
                    mPasvSocket = null;
                }
            }
            ftpLog(TAG, "RETR done");
        } else if (upperCmd.startsWith(FTP_CMD_STOR)) {
            Socket dataSocket;
            if (mPasvSocket != null) {
                dataSocket = mPasvSocket.accept();
            } else {
                dataSocket = new Socket(this.mHost, this.mPort);
                Debugger.logW(TAG, "dataSocket: New Data Socket!");
            }
            dataSocket.setSoTimeout(TIME_OUT);
            ftpLog(TAG, "STOR/APPE executing");
            boolean append = false;
            String param = getParameter(s, false);
            File storeFile = inputPathToChrootedFile(
                    new File(this.mCurrentDir), param);
            String errString = null;
            FileOutputStream out = null;
            try {
                mainblock: {
                    if (violatesChroot(storeFile)) {
                        errString = "550 Invalid name or chroot violation\r\n";
                        break mainblock;
                    }
                    if (storeFile.isDirectory()) {
                        errString = "451 Can't overwrite a directory\r\n";
                        break mainblock;
                    }

                    try {
                        if (storeFile.exists()) {
                            if (!append) {
                                if (!storeFile.delete()) {
                                    errString = "451 Couldn't truncate file\r\n";
                                    break mainblock;
                                }
                            }
                        }
                        out = new FileOutputStream(storeFile, append);
                    } catch (FileNotFoundException e) {
                        try {
                            errString = "451 Couldn't open file \"" + param
                                    + "\" aka \""
                                    + storeFile.getCanonicalPath()
                                    + "\" for writing\r\n";
                        } catch (IOException ioe) {
                            errString = "451 Couldn't open file, nested exception\r\n";
                        }
                        break mainblock;
                    }

                    ftpLog(TAG, "Data socket ready");
                    response("150 Data socket ready\r\n");
                    byte[] buffer = new byte[BUFFER_SIZE];
                    int numRead = 0;

                    if (mBinaryMode) {
                        ftpLog(TAG, "Mode is binary");
                    } else {
                        ftpLog(TAG, "Mode is ascii");
                    }
                    while (true) {
                        numRead = receiveFromDataSocket(buffer, dataSocket);
                        switch (numRead) {
                        case -1:
                            ftpLog(TAG, "Returned from final read");
                            // We're finished reading
                            break mainblock;
                        case 0:
                            errString = "426 Couldn't receive data\r\n";
                            break mainblock;
                        case -2:
                            errString = "425 Could not connect data socket\r\n";
                            break mainblock;
                        default:
                            try {
                                if (mBinaryMode) {
                                    out.write(buffer, 0, numRead);
                                } else {
                                    // ASCII mode, substitute \r\n to \n
                                    int startPos = 0;
                                    int endPos;
                                    // Added by MTK54046 @ 2011-10-11
                                    buffer = new String(buffer)
                                            .getBytes(mFtpCharset);
                                    numRead = buffer.length;
                                    for (endPos = 0; endPos < numRead; endPos++) {
                                        if (buffer[endPos] == '\r') {
                                            // Our hacky method is to drop all
                                            // \r
                                            out.write(buffer, startPos, endPos
                                                    - startPos);
                                            startPos = endPos + 1;
                                        }
                                    }
                                    if (startPos < numRead) {
                                        out.write(buffer, startPos, endPos
                                                - startPos);
                                    }
                                }
                                out.flush();
                            } catch (IOException e) {
                                errString = "451 File IO problem. Device might be full.\r\n";
                                Debugger.logW(TAG, "Exception while storing: "
                                        + e.getMessage());
                                break mainblock;
                            }
                            break;
                        }
                    }
                }
                if (errString != null) {
                    ftpLog(TAG, "STOR error: " + errString.trim());
                    response(errString);
                } else {
                    response("226 Transmission complete\r\n");
                }

            } catch (IOException e) {
                Debugger.logW(TAG, "IOException: " + e.getMessage());
            } finally {
                if (dataSocket != null) {
                    dataSocket.close();
                    dataSocket = null;
                }
                if (out != null) {
                    out.close();
                    out = null;
                }
                if (mPasvSocket != null) {
                    mPasvSocket.close();
                    mPasvSocket = null;
                }
            }
            ftpLog(TAG, "STOR finished");
        } else if (upperCmd.startsWith(FTP_CMD_LIST)) {
            Socket dataSocket = null;
            if (mPasvSocket != null) {
                dataSocket = mPasvSocket.accept();
            } else {
                dataSocket = new Socket(this.mHost, this.mPort);
                ftpLog(TAG, "dataSocket: New Data Socket!");
            }
            dataSocket.setSoTimeout(TIME_OUT);
            response("150 Opening ASCII mode data connection\r\n");
            try {
                responseList(s, dataSocket);
                response("226 transfer complete\r\n");
            } catch (SocketTimeoutException se) {
                Debugger.logW(TAG, "LIST SocketTimeoutException: "
                        + se.getMessage());
            } catch (IOException e) {
                Debugger.logW(TAG, "LIST IOException: " + e.getMessage());
            } finally {
                if (dataSocket != null) {
                    dataSocket.close();
                    dataSocket = null;
                }
                if (mPasvSocket != null) {
                    try {
                        mPasvSocket.close();
                    } catch (IOException e) {
                        Debugger.logW(TAG, e.getMessage());
                    }
                    mPasvSocket = null;
                }
            }

        } else if (upperCmd.startsWith(FTP_CMD_DELE)) {
            ftpLog(TAG, "DELE executing");
            String param = getParameter(s, false);
            File storeFile = inputPathToChrootedFile(
                    new File(this.mCurrentDir), param);
            String errString = null;
            if (violatesChroot(storeFile)) {
                errString = "550 Invalid name or chroot violation\r\n";
            } else if (storeFile.isDirectory()) {
                errString = "550 Can't DELE a directory\r\n";
            } else if (!storeFile.delete()) {
                errString = "450 Error deleting file\r\n";
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "DELE failed: " + errString.trim());
            } else {
                response("250 File successfully deleted\r\n");
            }
            ftpLog(TAG, "DELE finished");
        } else if (upperCmd.startsWith(FTP_CMD_MKD)) {
            ftpLog(TAG, "MKD executing");
            String param = getParameter(s, false);
            File toCreate;
            String errString = null;
            mainblock: {
                // If the param is an absolute path, use it as is. If it's a
                // relative path, prepend the current working directory.
                if (param.length() < 1) {
                    errString = "550 Invalid name\r\n";
                    break mainblock;
                }
                toCreate = inputPathToChrootedFile(new File(this.mCurrentDir),
                        param);
                if (violatesChroot(toCreate)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainblock;
                }
                if (toCreate.exists()) {
                    errString = "550 Already exists\r\n";
                    break mainblock;
                }
                if (!toCreate.mkdir()) {
                    errString = "550 Error making directory (permissions?)\r\n";
                    break mainblock;
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "MKD error: " + errString.trim());
            } else {
                response("250 Directory created\r\n");
            }
            ftpLog(TAG, "MKD complete");
        } else if (upperCmd.startsWith(FTP_CMD_RNFR)) {
            String param = getParameter(s, false);
            String errString = null;
            File file = null;
            mainblock: {
                file = inputPathToChrootedFile(new File(this.mCurrentDir),
                        param);
                if (violatesChroot(file)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainblock;
                }
                if (!file.exists()) {
                    errString = "450 Cannot rename nonexistent file\r\n";
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "RNFR failed: " + errString.trim());
                setRenameFrom(null);
            } else {
                response("350 Filename noted, now send RNTO\r\n");
                setRenameFrom(file);
            }
        } else if (upperCmd.startsWith(FTP_CMD_RNTO)) {
            String param = getParameter(s, false);
            String errString = null;
            File toFile = null;
            ftpLog(TAG, "RNTO executing\r\n");
            mainblock: {
                ftpLog(TAG, "param: " + param);
                toFile = inputPathToChrootedFile(new File(this.mCurrentDir),
                        param);
                ftpLog(TAG, "RNTO parsed: " + toFile.getPath());
                if (violatesChroot(toFile)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainblock;
                }
                File fromFile = getRenameFrom();
                if (fromFile == null) {
                    errString = "550 Rename error, maybe RNFR not sent\r\n";
                    break mainblock;
                }
                if (!fromFile.renameTo(toFile)) {
                    errString = "550 Error during rename operation\r\n";
                    break mainblock;
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "RNFR failed: " + errString.trim());
            } else {
                response("250 rename successful\r\n");
            }
            setRenameFrom(null);
            ftpLog(TAG, "RNTO finished");
        } else if (upperCmd.startsWith(FTP_CMD_RMD)) {
            ftpLog(TAG, "RMD executing");
            String param = getParameter(s, false);
            File toRemove;
            String errString = null;
            mainblock: {
                if (param.length() < 1) {
                    errString = "550 Invalid argument\r\n";
                    break mainblock;
                }
                toRemove = inputPathToChrootedFile(new File(this.mCurrentDir),
                        param);
                if (violatesChroot(toRemove)) {
                    errString = "550 Invalid name or chroot violation\r\n";
                    break mainblock;
                }
                if (!toRemove.isDirectory()) {
                    errString = "550 Can't RMD a non-directory\r\n";
                    break mainblock;
                }
                if (toRemove.equals(new File("/"))) {
                    errString = "550 Won't RMD the root directory\r\n";
                    break mainblock;
                }
                if (!recursiveDelete(toRemove)) {
                    errString = "550 Deletion error, possibly incomplete\r\n";
                    break mainblock;
                }
            }
            if (errString != null) {
                response(errString);
                ftpLog(TAG, "RMD failed: " + errString.trim());
            } else {
                response("250 Removed directory\r\n");
            }
            ftpLog(TAG, "RMD finished");
        } else if (s.equalsIgnoreCase(FTP_CMD_FEAT)) {
            response("211-Features supported\r\n");
            response(" UTF8\r\n"); // advertise UTF8 support (fixes bug 14)
            response("211 End\r\n");
        } else if (upperCmd.startsWith(FTP_CMD_SITE)) {
            ftpLog(TAG, "Command: site");
            response("200 invalid command:" + s + "\r\n");
        } else {
            ftpLog(TAG, "---------------->Invalid Command: " + s);
            response("200 invalid command:" + s + "\r\n");
        }
    }

    // Respose List command
    /**
     * @param input
     *            The input data.
     * @param dataSocket
     *            The socket to send data.
     * @throws IOException
     *             Signals a general, I/O-related error.
     */
    private void responseList(String input, Socket dataSocket)
            throws IOException {
        mainblock: {
            String param = getParameter(input, false);
            ftpLog(TAG, "List parameter: " + param);

            while (param.startsWith("-")) {
                ftpLog(TAG, "LIST is skipping dashed arg " + param);
                param = getParameter(param, false);
            }

            File fileToList = null;
            if (param.equals("")) {
                fileToList = new File(this.mCurrentDir);
            } else {
                if (param.contains("*")) {
                    response("550 LIST does not support wildcards\r\n");
                    break mainblock;
                }
                fileToList = new File(fileToList, param);
                if (violatesChroot(fileToList)) {
                    response("450 Listing target violates chroot\r\n");
                    break mainblock;
                }
            }
            String listing;
            if (fileToList.isDirectory()) {
                StringBuilder response = new StringBuilder();
                listDirectory(response, fileToList);
                listing = response.toString();
            } else {
                listing = makeLsString(fileToList);
                if (listing == null) {
                    response("450 Couldn't list that file\r\n");
                    break mainblock;
                }
            }
            byte[] bytes = listing.getBytes(mFtpCharset);
            sendViaDataSocket(bytes, 0, bytes.length, dataSocket
                    .getOutputStream());
        }
    }

    /**
     * @param toDelete
     *            The file to delete.
     * @return true if this file was deleted, false otherwise.
     */
    protected boolean recursiveDelete(File toDelete) {
        if (!toDelete.exists()) {
            return false;
        }
        if (toDelete.isDirectory()) {
            // If any of the recursive operations fail, then we return false
            boolean success = true;
            File[] entries = toDelete.listFiles();
            if (null == entries) {
                return false;
            }
            for (File entry : entries) {
                success &= recursiveDelete(entry);
            }
            ftpLog(TAG, "Recursively deleted: " + toDelete);
            return success && toDelete.delete();
        } else {
            ftpLog(TAG, "RMD deleting file: " + toDelete);
            return toDelete.delete();
        }
    }

    /**
     * @param file The file to check.
     * @return Whether the file violates the chroot.
     */
    public boolean violatesChroot(File file) {
        File chroot = new File("/");
        try {
            String canonicalPath = file.getCanonicalPath();
            if (!canonicalPath.startsWith(chroot.toString())) {
                ftpLog(TAG, "Path violated folder restriction, denying");
                ftpLog(TAG, "path: " + canonicalPath);
                ftpLog(TAG, "chroot: " + chroot.toString());
                return true; // the path must begin with the chroot path
            }
            return false;
        } catch (IOException e) {
            ftpLog(TAG, "Path canonicalization problem: " + e.toString());
            ftpLog(TAG, "When checking file: " + file.getAbsolutePath());
            return true; // for security, assume violation
        }
    }

    /**
     * @param input
     *            The input command.
     * @param silent
     *            Whether to print log.
     * @return The parameter in command.
     */
    public String getParameter(String input, boolean silent) {
        if (input == null) {
            return "";
        }
        int firstSpacePosition = input.indexOf(' ');
        if (firstSpacePosition == -1) {
            return "";
        }
        String retString = input.substring(firstSpacePosition + 1);

        // Remove trailing whitespace
        retString = retString.replaceAll("\\s+$", "");
        if (!silent) {
            ftpLog(TAG, "Parsed argument: " + retString);
        }
        return retString;
    }

    /**
     * @param response
     *            The string to save info.
     * @param dir
     *            The dir to traverse.
     * @return The string about the result.
     */
    public String listDirectory(StringBuilder response, File dir) {
        if (!dir.isDirectory()) {
            return "500 Internal error, listDirectory on non-directory\r\n";
        }
        ftpLog(TAG, "Listing directory: " + dir.toString());
        File[] entries = dir.listFiles();
        if (entries == null) {
            return "500 Couldn't list directory. Check config and mount status.\r\n";
        }
        ftpLog(TAG, "Dir len " + entries.length);
        for (File entry : entries) {
            String curLine = makeLsString(entry);
            if (curLine != null) {
                response.append(curLine);
            }
        }
        return null;
    }

    /**
     * @param file
     *            The file to list.
     * @return The information of the file.
     */
    protected String makeLsString(File file) {
        StringBuilder response = new StringBuilder();
        if (!file.exists()) {
            ftpLog(TAG, "makeLsString had nonexistent file");
            return null;
        }
        String lastNamePart = file.getName();
        // Many clients can't handle files containing these symbols
        if (lastNamePart.contains("*") || lastNamePart.contains("/")) {
            ftpLog(TAG, "Filename omitted due to disallowed character");
            return null;
        }
        if (file.isDirectory()) {
            response.append("drwxr-xr-x 1 owner group");
        } else {
            // todo: think about special files, symlinks, devices
            response.append("-rw-r--r-- 1 owner group");
        }
        // The next field is a 13-byte right-justified space-padded file size
        long fileSize = file.length();
        String sizeString = String.valueOf(fileSize);
        int padSpaces = 13 - sizeString.length();
        while (padSpaces-- > 0) {
            response.append(' ');
        }
        response.append(sizeString);
        // The format of the timestamp varies depending on whether the mtime
        // is 6 months old
        long mTime = file.lastModified();
        SimpleDateFormat format;
        // Temporarily commented out.. trying to fix Win7 display bug
        if (System.currentTimeMillis() - mTime > MS_IN_SIX_MONTHS) {
            // The mtime is less than 6 months ago
            format = new SimpleDateFormat(" MMM dd HH:mm ", Locale.US);
        } else {
            // The mtime is more than 6 months ago
            format = new SimpleDateFormat(" MMM dd  yyyy ", Locale.US);
        }
        response.append(format.format(new Date(file.lastModified())));
        response.append(lastNamePart);
        response.append("\r\n");
        return response.toString();
    }

    /**
     * @param bytes
     *            The data(bytes) to send.
     * @param start
     *            The start position of the bytes.
     * @param len
     *            The length of the bytes.
     * @param dataOutputStream
     *            The stream for output.
     * @return Whether succeed to send data.
     */
    private boolean sendViaDataSocket(byte[] bytes, int start, int len,
            OutputStream dataOutputStream) {
        if (dataOutputStream == null) {
            ftpLog(TAG, "Can't send via null dataOutputStream");
            return false;
        }
        if (len == 0) {
            return true; // this isn't an "error"
        }
        try {
            dataOutputStream.write(bytes, start, len);
            dataOutputStream.flush();
        } catch (SocketTimeoutException e) {
            ftpLog(TAG, "SocketTimeoutException when sendViaDataSocket: "
                    + e.getMessage());
            return true;
        } catch (IOException e) {
            ftpLog(TAG, "Couldn't write output stream for data socket");
            ftpLog(TAG, e.toString());
            return false;
        }

        return true;
    }

    /**
     * @param existingPrefix
     *            The file directory.
     * @param param
     *            The file name.
     * @return The ChrootedFile or null.
     */
    private File inputPathToChrootedFile(File existingPrefix, String param) {
        try {
            if (param.charAt(0) == '/') {
                // The STOR contained an absolute path
                File chroot = new File("/");
                return new File(chroot, param);
            }
        } catch (IndexOutOfBoundsException e) {
            return null;
        }
        // The STOR contained a relative path
        return new File(existingPrefix, param);
    }

    /**
     * @param buf
     *            The buffer to save the data.
     * @param dataSocket
     *            The socket to get the data.
     * @return The count of the received the bytes.
     */
    private int receiveFromDataSocket(byte[] buf, Socket dataSocket) {
        int bytesRead;

        if (dataSocket == null) {
            ftpLog(TAG, "Can't receive from null dataSocket");
            return -2;
        }
        if (!dataSocket.isConnected()) {
            ftpLog(TAG, "Can't receive from unconnected socket");
            return -2;
        }
        InputStream in;
        try {
            in = dataSocket.getInputStream();
            while ((bytesRead = in.read(buf, 0, buf.length)) == 0) {
                ftpLog(TAG, "receiveFromDataSocket");
            }
            if (bytesRead == -1) {
                return -1;
            }
        } catch (SocketTimeoutException e) {
            ftpLog(TAG, "SocketTimeoutException: " + e.getMessage());
            return 0;
        } catch (IOException e) {
            ftpLog(TAG, "Error reading data socket");
            return 0;
        }
        return bytesRead;
    }

    /**
     * Close the socket.
     */
    public void closeSocket() {
        try {
            if (null != mSocket) {
                mSocket.close();
            }
        } catch (IOException e) {
            Debugger.logW(TAG, "IOException: socket " + e.getMessage());
        }
    }

    /**
     * @return The file of the beginning to rename.
     */
    public File getRenameFrom() {
        return mRenameFrom;
    }

    /**
     * @param renameFrom
     *            The beginning file for renaming.
     */
    public void setRenameFrom(File renameFrom) {
        this.mRenameFrom = renameFrom;
    }

    /**
     * @param tag
     *            The tag of the log.
     * @param log
     *            The log to save.
     */
    private void ftpLog(Object[] tag, String log) {
        if (LOGGABLE) {
            Debugger.logI(tag, log);
        }
    }
}
