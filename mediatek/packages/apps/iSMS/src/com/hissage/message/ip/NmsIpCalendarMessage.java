package com.hissage.message.ip;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;

import android.text.TextUtils;

public class NmsIpCalendarMessage extends NmsIpAttachMessage {

    /**
     * 
     */
    private static final long serialVersionUID = -6497146321570006047L;
    public String summary;
    private NmsIpCalendarMessage mInstance;

    public NmsIpCalendarMessage() {
        super();
        setType(NmsIpMessageConsts.NmsIpMessageType.CALENDAR);
        mInstance = this;
    }

    public NmsIpCalendarMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize, String jsummary) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom,
                jpath, jurl, jsize);
        summary = jsummary;
    }

    // @Override
    // public void readFromParcel(Parcel in) {
    // super.readFromParcel(in);// attach message
    // summary = in.readString();
    // }
    //
    // @Override
    // public void writeToParcel(Parcel out, int flags) {
    // super.writeToParcel(out, flags);
    // out.writeString(summary);
    // }

    public void send(String number, String attachPath, String jsummary, int simId,
            final int sendMode, final boolean delDraft) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        summary = jsummary;
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

    public int saveDraft(String number, String attachPath, String jsummary, int simId) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return -1;
        }
        summary = jsummary;
        setSimId(simId);
        setAttachPath(attachPath);
        setReceiver(number);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(mInstance,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }
}
