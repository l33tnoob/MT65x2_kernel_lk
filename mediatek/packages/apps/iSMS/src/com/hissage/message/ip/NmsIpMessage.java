package com.hissage.message.ip;

import java.io.Serializable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class NmsIpMessage implements Serializable {

    /**
     * 
     */
    private static final long serialVersionUID = -3189283736811262910L;
    public long id; // message id in MMSSMS.db, value of "_id".
    public int ipDbId; // message id in ip message db
    public int simId; // sim id that message belong to
    public int type; // nmsIpMessageType
    public int status; // nmsIpMessageStatus
    public int protocol; // MMS,SMS,IP
    public boolean read;
    public int flag; // flag for saved or blocked
    public int time;
    public int timesend; //for sendtime
    public String to;
    public String from;
    private final static int DEFAUT_THREAD_NUM = 5;
    public static ExecutorService mExecutorService;

    public NmsIpMessage() {
        protocol = NmsIpMessageConsts.NmsMessageProtocol.IP;
        mExecutorService = Executors.newFixedThreadPool(DEFAUT_THREAD_NUM);
    }

    public NmsIpMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus, int jprotocol,
            boolean jread, int jflag, int jtime, int jtimesend , String jto, String jfrom) {
        id = jid;
        ipDbId = jipDbId;
        simId = jsimId;
        type = jtype;
        status = jstatus;
        protocol = jprotocol;
        read = jread;
        flag = jflag;
        time = jtime;
        timesend = jtimesend;
        to = jto;
        from = jfrom;
    }

    public void ConstructIpMessage(NmsIpMessage ipmsg) {
        id = ipmsg.id;
        ipDbId = ipmsg.ipDbId;
        simId = ipmsg.simId;
        type = ipmsg.type;
        status = ipmsg.status;
        protocol = ipmsg.protocol;
        read = ipmsg.read;
        flag = ipmsg.flag;
        time = ipmsg.time;
        timesend= ipmsg.timesend;
        to = ipmsg.to;
        from = ipmsg.from;
    }

//    private NmsIpMessage(Parcel in) {
//        readFromParcel(in);
//    }
//
//    @Override
//    public int describeContents() {
//        return 0;
//    }
//
//    public void readFromParcel(Parcel in) {
//        id = in.readLong();
//        ipDbId = in.readInt();
//        simId = in.readInt();
//        type = in.readInt();
//        status = in.readInt();
//        protocol = in.readInt();
//        read = (in.readInt() == 1 ? true : false);
//        flag = in.readInt();
//        time = in.readInt();
//        to = in.readString();
//        from = in.readString();
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        out.writeLong(id);
//        out.writeInt(ipDbId);
//        out.writeInt(simId);
//        out.writeInt(type);
//        out.writeInt(status);
//        out.writeInt(protocol);
//        out.writeInt(read == true ? 1 : 0);
//        out.writeInt(flag);
//        out.writeInt(time);
//        out.writeString(to);
//        out.writeString(from);
//    }

    public void setReceiver(String number) {
        to = number;
    }

    public void setSimId(int id) {
        simId = id;
    }

    public void setType(int type) {
        this.type = type;
    }

    public void setStatus(int status) {
        this.status = status;
    }
    public int getFlag() {
        return flag;
    }

    public void setFlag(int flag) {
        this.flag = flag;
    }
  
}
