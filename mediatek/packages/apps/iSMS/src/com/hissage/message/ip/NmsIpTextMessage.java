package com.hissage.message.ip;

import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.jni.engineadapter;

public class NmsIpTextMessage extends NmsIpMessage {

    /**
     * 
     */
    private static final long serialVersionUID = 2964776117262276797L;
    public String body;
    private NmsIpTextMessage mInstance;

    public NmsIpTextMessage() {
        super();
        setType(NmsIpMessageConsts.NmsIpMessageType.TEXT);
        mInstance = this;
    }

    public NmsIpTextMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jbody) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom);
        body = jbody;
    }

    // @Override
    // public void readFromParcel(Parcel in) {
    // // super.readFromParcel(in);
    // body = in.readString();
    // }
    //
    // @Override
    // public void writeToParcel(Parcel out, int flags) {
    // super.writeToParcel(out, flags);
    // out.writeString(body);
    // }

    public void send(String number, String body, int simId, final int sendMode,
            final short contactId, final boolean delDraft) {
        if (TextUtils.isEmpty(number) || TextUtils.isEmpty(body)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        setSimId(simId);
        setReceiver(number);
        setBody(body);
        mExecutorService.submit(new Runnable() {

            @Override
            public void run() {
                NmsIpMessageApiNative.nmsSaveIpMsg(mInstance, sendMode, delDraft, true);
            }
        });
    }

    private void setBody(String body) {
        this.body = body;
    }

    public int saveDraft(String number, String body, int simId) {
        if (TextUtils.isEmpty(number) || TextUtils.isEmpty(body)) {
            return -1;
        }
        setSimId(simId);
        setReceiver(number);
        setBody(body);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(this,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }
}
