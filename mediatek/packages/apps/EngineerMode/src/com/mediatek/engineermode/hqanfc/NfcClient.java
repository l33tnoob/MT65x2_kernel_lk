package com.mediatek.engineermode.hqanfc;

import android.content.Context;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.hqanfc.NfcCommand.DataConvert;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmReq;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

/**
 * Class Name: ConnectClient
 */
public class NfcClient {

    public static final int DEFAULT_PORT = 7500;
    public static final int DEFAULT_TIMEOUT = 4000;
   // protected Socket mSocket;
    protected LocalSocket mSocket;
    protected ReceiveThread mPollingThr;
    protected Thread mThread;
    protected DataOutputStream mOutputStream;
    protected DataInputStream mInputStream;
    private static final String LOCALHOST_IP_ADDRESS = "127.0.0.1";
    private static final int MAX_DISCON_TIMES = 5;
    private static final int SLEEP_TIME = 100;
    private boolean mConnected = false;
    private static NfcClient sInstance;
    
    private static final String LOCALSOCKET_NAME = "/data/nfc_socket/mtknfc_server";

    public static NfcClient getInstance() {
        if (sInstance == null) {
            sInstance = new NfcClient();
        }
        return sInstance;
    }

    public synchronized int sendCommand(int msgType, NfcEmReq cmdReq) {
        if (mSocket == null || mOutputStream == null || !isConnected()) {
            Elog.d(NfcMainPage.TAG, "[NfcClient]send command fail");
            return -1;
        }
        Elog.d(NfcMainPage.TAG, "[NfcClient]Send command type: " + msgType);
        try {
            ByteBuffer buffer = null;
            if (cmdReq == null) {
                buffer = ByteBuffer.allocate(NfcCommand.MAIN_MESSAGE_SIZE);
                buffer.put(DataConvert.intToLH(msgType));
                buffer.put(DataConvert.intToLH(0));
            } else {
                buffer = ByteBuffer
                        .allocate(cmdReq.getContentSize() + NfcCommand.MAIN_MESSAGE_SIZE);
                buffer.put(DataConvert.intToLH(msgType));
                int msglen = cmdReq.getContentSize();
                Elog.d(NfcMainPage.TAG, "[NfcClient]Send command lenght: " + msglen);
                buffer.put(DataConvert.intToLH(msglen));
                cmdReq.writeRaw(buffer);
            }
            for (int i = 0; i < buffer.array().length; i++) {
                System.out.print(buffer.array()[i] + " ");
            }
            System.out.println("done send");
            mOutputStream.write(buffer.array());
            mOutputStream.flush();
        } catch (IOException e) {
            Elog.w(NfcMainPage.TAG, "[NfcClient]sendCommand IOException: " + e.getMessage());
            e.printStackTrace();
            return -1;
        }
        return msgType;
    }

    // @Override
    public boolean closeConnection() {
        Elog.d(NfcMainPage.TAG, "[NfcClient]closeConnection().");
        if (mSocket == null || !isConnected()) {
            Elog.d(NfcMainPage.TAG, "[NfcClient]close connection fail");
            return false;
        }
        mPollingThr.setRunning(false);
        int cnt = 0;
        while (mPollingThr.isRunning() && mThread.isAlive() && cnt < MAX_DISCON_TIMES) {
            try {
                Thread.sleep(SLEEP_TIME);
                cnt++;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        if (mPollingThr.isRunning() || mThread.isAlive()) {
            mThread.interrupt();
        }
        try {
            mInputStream.close();
            mInputStream = null;
            mOutputStream.close();
            mOutputStream = null;
        } catch (IOException e) {
            Elog.w(NfcMainPage.TAG, "[NfcClient]closeConnection IOException: " + e.getMessage());
        }
        if (null != mSocket) {
            try {
                mSocket.close();
            } catch (IOException e) {
                Elog.w(NfcMainPage.TAG, "[NfcClient]closeConnection finally IOException: "
                        + e.getMessage());
            }
            mSocket = null;
        }
        sInstance = null;
        setConnected(false);
        Elog.v(NfcMainPage.TAG, "[NfcClient]close connection success");
        return true;
    }

    // @Override
    public boolean createConnection(Context context) {
        Elog.d(NfcMainPage.TAG, "[NfcClient]createConnection().");
        int timeout = 30000;
        if (mSocket != null && isConnected()) {
            Elog.v(NfcMainPage.TAG, "[NfcClient]createConnection: has connected");
            return true;
        }
        try {
            //mSocket = new Socket(LOCALHOST_IP_ADDRESS, DEFAULT_PORT);
            mSocket = new LocalSocket();
            LocalSocketAddress name = new LocalSocketAddress(LOCALSOCKET_NAME, LocalSocketAddress.Namespace.FILESYSTEM);
            mSocket.connect(name);
            mSocket.setSoTimeout(timeout);
        } catch (IOException e) {
            Elog.w(NfcMainPage.TAG, "[NfcClient]createConnection IOException: " + e.getMessage());
            mSocket = null;
        }
        if (mSocket == null) {
            return false;
        }
        try {
            mOutputStream = new DataOutputStream(mSocket.getOutputStream());
            mInputStream = new DataInputStream(mSocket.getInputStream());
        } catch (IOException e) {
            Elog.w(NfcMainPage.TAG, "[NfcClient]getStream IOException: " + e.getMessage());
            try {
                mSocket.close();
                mSocket = null;
            } catch (IOException e1) {
                mSocket = null;
            }
            return false;
        }
        setConnected(true);
        mPollingThr = new ReceiveThread(mInputStream, context);
        mThread = new Thread(mPollingThr);
        mThread.start();
        return true;
    }

    public void setConnected(boolean connected) {
        this.mConnected = connected;
    }

    public boolean isConnected() {
        return mConnected;
    }
}
