package com.hissage.message.ip;

import android.text.TextUtils;

public class NmsIpAttachMessage extends NmsIpMessage {

    /**
     * 
     */
    private static final long serialVersionUID = -1670913175797610248L;
    public String path;
    public String url; /* for inbox msg, it may contain the downloaded url */
    public int size;

    public NmsIpAttachMessage() {
        super();
    }

    public NmsIpAttachMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize) {
        this(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom);
        path = jpath;
        url = jurl;
        size = jsize;
    }

    public NmsIpAttachMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom);
    }

    public boolean isInboxMsgDownloalable() {
        return !TextUtils.isEmpty(url);
    }

//    @Override
//    public void readFromParcel(Parcel in) {
//        // super.readFromParcel(in);
//        path = in.readString();
//        url = in.readString();
//        size = in.readInt();
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        super.writeToParcel(out, flags);
//        out.writeString(path);
//        out.writeString(url);
//        out.writeInt(size);
//    }

    public void setAttachPath(String path) {
        this.path = path;
    }
}
