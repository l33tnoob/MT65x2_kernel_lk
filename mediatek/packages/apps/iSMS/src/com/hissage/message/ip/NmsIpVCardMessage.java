package com.hissage.message.ip;

import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;

public class NmsIpVCardMessage extends NmsIpAttachMessage {

    /**
     * 
     */
    private static final long serialVersionUID = -7577893809927875588L;
    public String name;
    private NmsIpVCardMessage mInstance;

    public NmsIpVCardMessage() {
        super();
        setType(NmsIpMessageConsts.NmsIpMessageType.VCARD);
        mInstance = this;
    }

    public NmsIpVCardMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize, String jname) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom,
                jpath, jurl, jsize);
        name = jname;
    }

    // @Override
    // public void readFromParcel(Parcel in) {
    // super.readFromParcel(in);// attach message
    // name = in.readString();
    // }
    //
    // @Override
    // public void writeToParcel(Parcel out, int flags) {
    // super.writeToParcel(out, flags);
    // out.writeString(name);
    // }

    public void send(String number, String attachPath, String name, int simId, final int sendMode,
            final boolean delDraft) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        setContactName(name);
        setSimId(simId);
        setAttachPath(attachPath);
        setReceiver(number);
        mExecutorService.submit(new Runnable() {
            @Override
            public void run() {
                NmsIpMessageApiNative.nmsSaveIpMsg(mInstance, sendMode, delDraft, true);
            }
        });

    }

    private void setContactName(String name) {
        this.name = TextUtils.isEmpty(name) ? "" : name;
    }

    public int saveDraft(String number, String attachPath, String name, int simId) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return -1;
        }
        setContactName(name);
        setSimId(simId);
        setAttachPath(attachPath);
        setReceiver(number);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(mInstance,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }
}
