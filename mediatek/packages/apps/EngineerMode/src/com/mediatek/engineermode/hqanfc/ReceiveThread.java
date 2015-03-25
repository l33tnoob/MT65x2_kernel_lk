package com.mediatek.engineermode.hqanfc;

import android.content.Context;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.DataConvert;

import java.io.DataInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Class Name: ReceiveThread
 * <p>
 * Package: com.mediatek.nfcdemo.communication
 * <p>
 * Created on: 2012-6-27
 * <p>
 * <p>
 * Description:
 * <p>
 * Runnable for polling data from socket IO stream.
 * <p>
 * 
 * @author mtk54045
 * @version V1.0
 */
public class ReceiveThread implements Runnable {

    private DataInputStream mInputStream;
    private boolean mRunning;
    private Context mContext;

    /**
     * Constructor with parameters.
     * 
     */
    public ReceiveThread(DataInputStream is, Context context) {
        mContext = context;
        mInputStream = is;
        mRunning = false;
    }

    /**
     * Set whether the runnable should run.
     */
    public void setRunning(boolean running) {
        this.mRunning = running;
    }

    // @Override
    public void run() {
        if (mInputStream == null) {
            System.out.println("[ReceiveThread]The dispatcher or stream object is null!");
            return;
        }
        mRunning = true;
        NfcCommandHandler commandHandler = NfcCommandHandler.getInstance(mContext);
        while (mRunning) {
            try {
                byte[] b = new byte[NfcCommand.RECEIVE_DATA_SIZE];
                mInputStream.read(b);
                for (int i = 0; i < b.length; i++) {
                    System.out.print(b[i] + " ");
                }
                System.out.println("done receive");
                ByteBuffer buffer = ByteBuffer.wrap(b);
                byte[] type = new byte[NfcCommand.INT_SIZE];
                buffer.get(type);
                int msgType = DataConvert.byteToInt(type);
                Elog.d(NfcMainPage.TAG, "[ReceiveThread:info]Recieved data message type is "
                        + msgType);
                if (msgType > CommandType.MTK_NFC_EM_MSG_END) {
                    Elog.v(NfcMainPage.TAG,
                            "[ReceiveThread]receive message is not the correct msg and the content: "
                                    + new String(b));
                } else {
                    byte[] lenght = new byte[NfcCommand.INT_SIZE];
                    buffer.get(lenght);
                    int msgLen = DataConvert.byteToInt(lenght);
                    Elog.d(NfcMainPage.TAG, "[ReceiveThread:info]Recieved data message lenght is "
                            + msgLen);
                    NfcCommand mainMessage;
                    if (msgLen == 0) {
                        mainMessage = new NfcCommand(msgType, null);
                    } else {
                        byte[] bufferCont = new byte[msgLen];
                        buffer.get(bufferCont);
                        mainMessage = new NfcCommand(msgType, ByteBuffer.wrap(bufferCont));
                    }
                    commandHandler.execute(mainMessage);
                }
            } catch (IOException e) {
                String errorMsg = e.getMessage();
                if(errorMsg.equals("Try again")) {
                    
                }else {
                    mRunning = false;
                    e.printStackTrace();
                }
                Elog.v(NfcMainPage.TAG, "[ReceiveThread]receive thread IOException: " + errorMsg);
                
            }
        }
        if (!mRunning) {
            commandHandler.destroy();
        }
        // mRunning = false;
    }

    /**
     * Whether the runnable is running.
     * 
     * @return True if running, otherwise false.
     */
    public boolean isRunning() {
        return mRunning;
    }
}
