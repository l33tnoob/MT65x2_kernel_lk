package com.hissage.util.queue;

//import packages.
import java.util.Vector;

import com.hissage.util.log.NmsLog;

public final class NmsMessagePump {

    // data member of message pump.
    private Vector<NmsMessage> cmdQueue = new Vector<NmsMessage>();

    public NmsMessage getMessage() {
        NmsMessage msg = null;
        do {
            synchronized (cmdQueue) {
                if (cmdQueue.size() > 0) {
                    msg = (NmsMessage) cmdQueue.elementAt(0);
                    cmdQueue.removeElementAt(0);
                } else {
                    try {
                        cmdQueue.wait();
                    } catch (Exception e) {
                        NmsLog.nmsPrintStackTrace(e);
                    } finally {
                        cmdQueue.notify();
                    }
                }
            }
        } while (msg == null);

        return msg;
    }

    // public void sendMessage(NmsMessage msg) {
    // // add message to queue.
    // synchronized (cmdQueue) {
    // cmdQueue.addElement(msg);
    // cmdQueue.notifyAll();
    // }
    //
    // // wait message to processed.
    // synchronized (msg) {
    // try {
    // msg.wait();
    // } catch (Exception e) {
    // msg.setException(e);
    // }
    // }
    // }

    public void postMessage(NmsMessage msg) {
        // add message to queue.
        synchronized (cmdQueue) {
            cmdQueue.addElement(msg);
            cmdQueue.notifyAll();
        }
    }
}
