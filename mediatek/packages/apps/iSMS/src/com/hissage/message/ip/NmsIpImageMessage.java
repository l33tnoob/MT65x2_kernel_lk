package com.hissage.message.ip;

import java.io.File;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
import com.hissage.util.message.MessageConsts;

public class NmsIpImageMessage extends NmsIpAttachMessage {

    /**
     * 
     */
    private static final long serialVersionUID = 6286760373090122022L;
    public int widthInPixel;
    public int heightInPixel;
    public String caption; /* max length is 100 */
    public String thumbPath; /* for send msg, it's "" */
    private NmsIpImageMessage mInstance;

    public NmsIpImageMessage(int msgType) {
        super();
        setType(msgType);
        mInstance = this;
    }

    public NmsIpImageMessage(int jid, int jipDbId, int jsimId, int jtype, int jstatus,
            int jprotocol, boolean jread, int jflag, int jtime, int jtimesend, String jto, String jfrom,
            String jpath, String jurl, int jsize, String jcaption, String jthumbPath) {
        super(jid, jipDbId, jsimId, jtype, jstatus, jprotocol, jread, jflag, jtime, jtimesend, jto, jfrom,
                jpath, jurl, jsize);
        caption = jcaption;
        thumbPath = jthumbPath;
    }

//    @Override
//    public void readFromParcel(Parcel in) {
//        super.readFromParcel(in);
//        widthInPixel = in.readInt();
//        heightInPixel = in.readInt();
//        caption = in.readString();
//        thumbPath = in.readString();
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        super.writeToParcel(out, flags);
//        out.writeInt(widthInPixel);
//        out.writeInt(heightInPixel);
//        out.writeString(caption);
//        out.writeString(thumbPath);
//    }

    public void send(final Context context, int flag,String number, final String attachPath, String caption, int simId,
            final int sendMode, final short contactId, final boolean delDraft, final Handler handler) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return;
        }
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.OUTBOX);
        setSimId(simId);
        setCaption(caption);
        setReceiver(number);
        setFlag(flag);
        mExecutorService.submit(new Runnable() {

            @Override
            public void run() {
                byte[] img = NmsBitmapUtils.resizeImgByMaxLength(attachPath, (float) 500);
                if (null == img) {
                    if (handler != null)
                        handler.sendEmptyMessage(MessageConsts.NMS_SEND_MSG_FAILED) ;
                    return;
                }
                try {
                    NmsCommonUtils.nmsStream2File(img, attachPath);
                    /// M: add for ipmessage 89 platfrom @{
                    Intent intent = new Intent(android.content.Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
                    Uri uri = Uri.fromFile(new File(attachPath));
                    intent.setData(uri);
                    context.sendBroadcast(intent);
                    /// }@
                } catch (Exception e) {
                    e.printStackTrace();
                    if (handler != null)
                        handler.sendEmptyMessage(MessageConsts.NMS_SEND_MSG_FAILED) ;
                    return;
                }
                mInstance.setAttachPath(attachPath);
                if (NmsIpMessageApiNative.nmsSaveIpMsg(mInstance, sendMode,delDraft, true) < 0 && handler != null)
                    handler.sendEmptyMessage(MessageConsts.NMS_SEND_MSG_FAILED) ;
            }
        });

    }

    public void setCaption(String caption) {
        this.caption = caption;
    }

    public int saveDraft(String number, String attachPath, String caption, int simId) {
        if (TextUtils.isEmpty(number) || !NmsCommonUtils.isExistsFile(attachPath)) {
            return -1;
        }
        setSimId(simId);
        setCaption(caption);
        setAttachPath(attachPath);
        setReceiver(number);
        setStatus(NmsIpMessageConsts.NmsIpMessageStatus.DRAFT);
        return NmsIpMessageApiNative.nmsSaveIpMsg(mInstance,
                NmsIpMessageConsts.NmsIpMessageSendMode.NORMAL, true, true);
    }
}
