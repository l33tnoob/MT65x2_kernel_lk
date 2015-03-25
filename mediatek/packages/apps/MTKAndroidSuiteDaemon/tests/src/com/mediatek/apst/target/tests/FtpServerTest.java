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

package com.mediatek.apst.target.tests;

import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.apst.target.ftp.FtpServer;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

public class FtpServerTest extends AndroidTestCase {
    private static final String TAG = "FtpServerTest";
    private static final int FTP_PORT = 2222;
    private static final int DATA_PORT = 19878;
    private FtpServer mFtpServerThread;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mFtpServerThread = new FtpServer();
        mFtpServerThread.start();
    }

    @Override
    protected void tearDown() throws Exception {
        mFtpServerThread.destroy();
        mFtpServerThread.interrupt();
        super.tearDown();
    }

    public void test01_handleCommand() {
        assertTrue(mFtpServerThread.isAlive());
        Socket commandClient = null;
        DataOutputStream cmdOutput = null;
        DataInputStream cmdInput = null;
        ReadThread cmdReadThread = null;
        Socket commandClient2 = null;
        DataOutputStream cmdOutput2 = null;
        DataInputStream cmdInput2 = null;
        ReadThread cmdReadThread2 = null;
        try {
            int i = 0;
            //
            i = 0;
            while (++i < 10) {
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e1) {
                    e1.printStackTrace();
                }
                try {
                    commandClient2 = new Socket("127.0.0.1", FTP_PORT);
                    break;
                } catch (UnknownHostException e) {
                    Log.w(TAG, "UnknownHostException: " + e.getMessage());
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (i == 10) {
                Log.v(TAG, "connect2 timeout");
                return;
            }
            commandClient2.setSoTimeout(10000);
            cmdOutput2 = new DataOutputStream(commandClient2.getOutputStream());
            cmdInput2 = new DataInputStream(commandClient2.getInputStream());
            cmdReadThread2 = new ReadThread(cmdInput2);
            cmdReadThread2.start();
            sendCommand(cmdOutput2, "USER APST");
            sendCommand(cmdOutput2, "PASS APST");
            sendCommand(cmdOutput2, "opts utf8 on");
            sendCommand(cmdOutput2, "syst");
            sendCommand(cmdOutput2, "site help");
            sendCommand(cmdOutput2, "PWD");
            sendCommand(cmdOutput2, "CWD /sdcard/");
            sendCommand(cmdOutput2, "QUIT");
            cmdReadThread2.interrupt();
            cmdOutput2.close();
            cmdInput2.close();
            commandClient2.close();
            i = 0;
            while (++i < 10) {
                try {
                    Thread.sleep(500);
                } catch (InterruptedException e1) {
                    e1.printStackTrace();
                }
                try {
                    commandClient = new Socket("127.0.0.1", FTP_PORT);
                    break;
                } catch (UnknownHostException e) {
                    Log.w(TAG, "UnknownHostException: " + e.getMessage());
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (i == 10) {
                Log.v(TAG, "connect timeout");
                return;
            }

            commandClient.setSoTimeout(10000);
            cmdOutput = new DataOutputStream(commandClient.getOutputStream());
            cmdInput = new DataInputStream(commandClient.getInputStream());
            cmdReadThread = new ReadThread(cmdInput);
            cmdReadThread.start();
            sendCommand(cmdOutput, "USER APST");
            sendCommand(cmdOutput, "PASS APST");
            sendCommand(cmdOutput, "opts utf8 on");
            sendCommand(cmdOutput, "syst");
            sendCommand(cmdOutput, "site help");
            sendCommand(cmdOutput, "PWD");
            sendCommand(cmdOutput, "CWD /sdcard/");
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "LIST");
            receiveData();
            sendCommand(cmdOutput, "noop");
            sendCommand(cmdOutput, "MKD testdir");
            sendCommand(cmdOutput, "MKD testdir1");
            // assertTrue(new File("/sdcard/testdir").exists());
            sendCommand(cmdOutput, "CWD testdir");
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "LIST");
            receiveData();
            sendCommand(cmdOutput, "noop");

            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "STOR test.txt");
            writeData();
            sendCommand(cmdOutput, "TYPE I");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "STOR test_bin.txt");
            writeData();
            // assertTrue(new File("/sdcard/testdir/test.txt").exists());
            sendCommand(cmdOutput, "RNFR test.txt");
            sendCommand(cmdOutput, "RNTO test1.txt");
            // assertTrue(new File("/sdcard/testdir/test1.txt").exists());
            sendCommand(cmdOutput, "SIZE test1.txt");
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "RETR test1.txt");
            receiveData();
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "RETR /sdcard/testdir/test1.txt");
            receiveData();
            sendCommand(cmdOutput, "TYPE I");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "RETR test_bin.txt");
            receiveData();
            sendCommand(cmdOutput, "DELE test1.txt");
            // assertFalse(new File("/sdcard/testdir/test1.txt").exists());
            sendCommand(cmdOutput, "CDUP");
            sendCommand(cmdOutput, "SIZE /sdcard/testdir");
            sendCommand(cmdOutput, "RMD testdir");
            sendCommand(cmdOutput, "RMD testdir1");
            sendCommand(cmdOutput, "CDUP");
            sendCommand(cmdOutput, "CDUP");
            sendCommand(cmdOutput, "CDUP");
            // assertFalse(new File("/sdcard/testdir").exists());
            sendCommand(cmdOutput, "PORT 127,0,0,1,77,166");
            sendCommand(cmdOutput, "FEAT");
            sendCommand(cmdOutput, "QUIT");
        } catch (UnknownHostException e) {
            Log.w(TAG, "UnknownHostException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "IOException: " + e.getMessage());
        } catch (InterruptedException e) {
            Log.w(TAG, "InterruptedException: " + e.getMessage());
        } finally {
            if (null != cmdReadThread) {
                cmdReadThread.interrupt();
            }
            if (null != cmdOutput) {
                try {
                    cmdOutput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != cmdInput) {
                try {
                    cmdInput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != commandClient) {
                try {
                    commandClient.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }

            if (null != cmdReadThread2) {
                cmdReadThread2.interrupt();
            }
            if (null != cmdOutput2) {
                try {
                    cmdOutput2.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != cmdInput2) {
                try {
                    cmdInput2.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != commandClient2) {
                try {
                    commandClient2.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }

        }
    }

    public void test02_handleError() {
        assertTrue(mFtpServerThread.isAlive());
        Socket commandClient = null;
        DataOutputStream cmdOutput = null;
        DataInputStream cmdInput = null;
        ReadThread cmdReadThread = null;
        int i = 0;
        while (++i < 10) {
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Log.w(TAG, "InterruptedException: " + e.getMessage());
            }
            try {
                commandClient = new Socket("127.0.0.1", FTP_PORT);
                break;
            } catch (UnknownHostException e) {
                Log.w(TAG, "UnknownHostException: " + e.getMessage());
            } catch (IOException e) {
                Log.w(TAG, "IOException: " + e.getMessage());
            }
        }
        if (i == 10) {
            Log.v(TAG, "connect timeout");
            return;
        }
        try {
            commandClient.setSoTimeout(10000);
            cmdOutput = new DataOutputStream(commandClient.getOutputStream());
            cmdInput = new DataInputStream(commandClient.getInputStream());
            cmdReadThread = new ReadThread(cmdInput);
            cmdReadThread.start();
            sendCommand(cmdOutput, "USER APST");
            sendCommand(cmdOutput, "PASS ERROR");
            sendCommand(cmdOutput, "TYPE E");
            sendCommand(cmdOutput, "TYPE L 8");
            sendCommand(cmdOutput, "TYPE A N");
            sendCommand(cmdOutput, "opts");
            sendCommand(cmdOutput, "opts utf8 off");
            sendCommand(cmdOutput, "opts GB on");
            sendCommand(cmdOutput, "CWD /cache");
            sendCommand(cmdOutput, "SIZE /sdcard/testdirerror");
            sendCommand(cmdOutput, "PORT 127,0,0,1,77");
            sendCommand(cmdOutput, "PORT 127,0,0,1,77,1166");
            sendCommand(cmdOutput, "PORT 127,200,0,1,77,166");
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "RETR /sdcard");
            receiveData();
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "RETR /sdcard/notexists");
            receiveData();
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "STOR /sdcard");
            writeData();
            sendCommand(cmdOutput, "CWD /sdcard/");
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "LIST - mtklog");
            receiveData();
            sendCommand(cmdOutput, "TYPE A");
            sendCommand(cmdOutput, "PASV");
            sendCommand(cmdOutput, "LIST *");
            receiveData();
            sendCommand(cmdOutput, "DELE /sdcard");
            sendCommand(cmdOutput, "MKD");
            sendCommand(cmdOutput, "MKD /sdcard");
            sendCommand(cmdOutput, "MKD /testpermission");
            sendCommand(cmdOutput, "RNFR /sdcard/notexists");
            sendCommand(cmdOutput, "RNTO /sdcard/notexists");
            sendCommand(cmdOutput, "RNFR /system");
            sendCommand(cmdOutput, "RNTO /systemtest");
            sendCommand(cmdOutput, "RMD");
            sendCommand(cmdOutput, "RMD /");
            sendCommand(cmdOutput, "RMD /system/app/MTKAndroidSuiteDaemon.apk");
            sendCommand(cmdOutput, "QUIT");
        } catch (UnknownHostException e) {
            Log.w(TAG, "UnknownHostException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "IOException: " + e.getMessage());
        } catch (InterruptedException e) {
            Log.w(TAG, "InterruptedException: " + e.getMessage());
        } finally {
            if (null != cmdReadThread) {
                cmdReadThread.interrupt();
            }
            if (null != cmdOutput) {
                try {
                    cmdOutput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != cmdInput) {
                try {
                    cmdInput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != commandClient) {
                try {
                    commandClient.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
        }
    }

    private void sendCommand(DataOutputStream output, String command)
            throws InterruptedException, IOException {
        Log.v(TAG, "SEND COMMAND: " + command);
        output.writeBytes(command + "\r\n");
        output.flush();
        Thread.sleep(1000);
    }

    private void writeData() {
        Socket dataClient = null;
        DataOutputStream dataOutput = null;
        DataInputStream dataInput = null;
        int i = 0;
        while (++i < 10) {
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Log.w(TAG, "InterruptedException: " + e.getMessage());
            }
            try {
                dataClient = new Socket("127.0.0.1", DATA_PORT);
                break;
            } catch (UnknownHostException e) {
                Log.w(TAG, "UnknownHostException: " + e.getMessage());
            } catch (IOException e) {
                Log.w(TAG, "IOException: " + e.getMessage());
            }
        }
        if (i == 10) {
            Log.v(TAG, "connect timeout");
            return;
        }
        try {
            dataOutput = new DataOutputStream(dataClient.getOutputStream());
            dataInput = new DataInputStream(dataClient.getInputStream());
            dataOutput.writeBytes("FtpServerTest test\r\n");
            dataOutput.flush();
        } catch (UnknownHostException e) {
            Log.w(TAG, "UnknownHostException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "IOException: " + e.getMessage());
        } finally {
            if (null != dataOutput) {
                try {
                    dataOutput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != dataInput) {
                try {
                    dataInput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != dataClient) {
                try {
                    dataClient.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
        }
    }

    private void receiveData() {
        Socket dataClient = null;
        DataOutputStream dataOutput = null;
        DataInputStream dataInput = null;
        ReadThread dataReadThread = null;
        int i = 0;
        while (++i < 10) {
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Log.w(TAG, "InterruptedException: " + e.getMessage());
            }
            try {
                dataClient = new Socket("127.0.0.1", DATA_PORT);
                break;
            } catch (UnknownHostException e) {
                Log.w(TAG, "UnknownHostException: " + e.getMessage());
            } catch (IOException e) {
                Log.w(TAG, "IOException: " + e.getMessage());
            }
        }
        if (i == 10) {
            Log.v(TAG, "connect timeout");
            return;
        }
        try {
            dataOutput = new DataOutputStream(dataClient.getOutputStream());
            dataInput = new DataInputStream(dataClient.getInputStream());
            dataReadThread = new ReadThread(dataInput);
            // dataReadThread.start();
            // dataReadThread.join();
            dataReadThread.run();
        } catch (UnknownHostException e) {
            Log.w(TAG, "UnknownHostException: " + e.getMessage());
        } catch (IOException e) {
            Log.w(TAG, "IOException: " + e.getMessage());
            // } catch (InterruptedException e) {
            // Log.w(TAG, "InterruptedException: " + e.getMessage());
        } finally {
            if (null != dataReadThread) {
                dataReadThread.interrupt();
            }
            if (null != dataOutput) {
                try {
                    dataOutput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != dataInput) {
                try {
                    dataInput.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
            if (null != dataClient) {
                try {
                    dataClient.close();
                } catch (IOException e) {
                    Log.w(TAG, "IOException: " + e.getMessage());
                }
            }
        }
    }

    private class ReadThread extends Thread {

        private static final int BUFFER_SIZE = 1024;
        private DataInputStream mReader = null;
        private byte[] mBuffer = null;

        public ReadThread(DataInputStream reader) {
            this.mReader = reader;
            this.mBuffer = new byte[BUFFER_SIZE];
        }

        @Override
        public void run() {
            super.run();
            int count = 0;
            try {
                while ((count = mReader.read(mBuffer)) != -1 && !interrupted()) {
                    Log.v(TAG, getId() + " RECEIVE: "
                            + new String(mBuffer, 0, count));
                }
            } catch (IOException e) {
                Log.w(TAG, "IOException: " + e.getMessage());
            } finally {
                if (null != mReader) {
                    try {
                        mReader.close();
                    } catch (IOException e) {
                        Log.w(TAG, "IOException: " + e.getMessage());
                    }
                }
            }
        }

    }

}
