package com.hissage.message.ip;

import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsCommonUtils;

public class NmsIpVoiceMessage extends NmsIpAttachMessage {

    /**
     * 
     */
    private static final long serialVersionUID = 2230096888851812225L;
    public int durationTime;
    public String caption;
    private NmsIpVoiceMessage mInstance;

    public NmsIpVoiceMessage() {
        super();
        setType(NmsIpMessageConsts.NmsIpMessageType.VOICE);
        mInstance = this;
    }

    public NmsIpVoiceMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize, int jdurationTime, String jcaption) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom,
                jpath, jurl, jsize);
        durationTime = jdurationTime;
        caption = jcaption;
    }

    // @Override
    // public void readFromParcel(Parcel in) {
    // super.readFromParcel(in);
    // durationTime = in.readInt();
    // caption = in.readString();
    // }
    //
    // @Override
    // public void writeToParcel(Parcel out, int flags) {
    // super.writeToParcel(out, flags);
    // out.writeInt(durationTime);
    // out.writeString(caption);
    // }

    public void send(String number, String attachPath, String caption, int duration, int simId,
            final int sendMode, final short contactId, final boolean delDraft) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        setSimId(simId);
        setCaption(caption);
        setDuration(duration);
        setAttachPath(attachPath);
        setReceiver(number);
        mExecutorService.submit(new Runnable() {
            @Override
            public void run() {
                NmsIpMessageApiNative.nmsSaveIpMsg(mInstance, sendMode, delDraft, true);
            }
        });
    }

    private void setCaption(String caption) {
        this.caption = caption;
    }

    private void setDuration(int duration) {
        durationTime = duration;
    }

    public int saveDraft(String number, String attachPath, String caption, int duration, int simId) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return -1;
        }
        setSimId(simId);
        setCaption(caption);
        setDuration(duration);
        setAttachPath(attachPath);
        setReceiver(number);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(mInstance,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }
}
