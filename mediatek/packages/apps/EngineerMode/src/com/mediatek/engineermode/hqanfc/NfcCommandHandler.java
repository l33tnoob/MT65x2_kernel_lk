package com.mediatek.engineermode.hqanfc;

import android.content.Context;
import android.content.Intent;

import com.mediatek.engineermode.Elog;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * 
 * @author mtk54045
 */
public class NfcCommandHandler {

    private static final int QUEUE_SIZE = 50;
    private BlockingQueue<NfcCommand> mCommandQueue = new ArrayBlockingQueue<NfcCommand>(QUEUE_SIZE);
    private Thread mConsumerThr;
    private Context mContext;
    private static NfcCommandHandler sInstance;

    public static synchronized NfcCommandHandler getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new NfcCommandHandler(context);
        }
        return sInstance;
    }

    public NfcCommandHandler(Context context) {
        Elog.w(NfcMainPage.TAG, "[NfcCommandHandler]NfcCommandHandler NfcCommandHandler()");
        mContext = context;
        mConsumerThr = new Thread(new Consumer());
        mConsumerThr.start();
    }

    public void destroy() {
        mCommandQueue.clear();
        mConsumerThr.interrupt();
    }

    // put the recieve data to the queue
    public boolean execute(NfcCommand entity) {
        Elog.w(NfcMainPage.TAG, "[NfcCommandHandler] execute()");
        boolean result = false;
        try {
            mCommandQueue.put((NfcCommand) entity);
            result = true;
        } catch (InterruptedException ex) {
            Elog.w(NfcMainPage.TAG, "[NfcCommandHandler]execute InterruptedException: " + ex.getMessage());
        }
        return result;
    }

    private class Consumer implements Runnable {

        // private BlockingQueue<NfcCommand> commandQueue;
        //
        // public Consumer(BlockingQueue<NfcCommand> queue) {
        // this.commandQueue = queue;
        // }

        @Override
        public void run() {
            while (true) {
                try {
                    Elog.w(NfcMainPage.TAG, "[NfcCommandHandler] Consumer take.");
                    if (mCommandQueue == null) {
                        Elog.w(NfcMainPage.TAG, "[NfcCommandHandler] mCommandQueue is null.");
                    } else {
                        NfcCommand command = mCommandQueue.take();
                        processCommand(command); // 取出一个命令，调用processCommand解析命令并采取相应动作
                    }
                } catch (InterruptedException ex) {
                    Elog.w(NfcMainPage.TAG, "[NfcCommandHandler]Consumer InterruptedException: " + ex.getMessage());
                    ex.printStackTrace();
                }
            }
        }
    }

    // // inner class: Take the recieve data from the queue and handler it
    // private class Consumer implements Runnable {
    //
    // @Override
    // public void run() {
    // Elog.w(NfcMainPage.TAG, "NfcCommandHandler Consumer");
    // while (true) {
    // try {
    // NfcCommand command = mCommandQueue.take();
    // processCommand(command);
    // } catch (InterruptedException ex) {
    // Elog.w(NfcMainPage.TAG, "Consumer InterruptedException: " +
    // ex.getMessage());
    // ex.printStackTrace();
    // }
    // }
    // }
    // }

    private void processCommand(NfcCommand receiveData) {
        Intent intent = new Intent();
        intent.setAction(NfcCommand.ACTION_PRE + receiveData.getMessageType());
        intent.putExtra(NfcCommand.MESSAGE_CONTENT_KEY, receiveData.getMessageContent().array());
        mContext.sendBroadcast(intent);
    }
}
