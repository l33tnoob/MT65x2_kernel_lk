package com.hissage.contact;

import android.text.TextUtils;

import com.hissage.util.log.NmsLog;

public class NmsGroupChatContact extends NmsContact {

    /**
     * 
     */
    private static final long serialVersionUID = 3065136303709165560L;

    private static String TAG = "NmsGroupChatContact";

    public static final int NMS_CLIENT_GUID_LEN = 41;
    public static final String NMS_GUID_PREFIX = "7--";

    private int simId; // sim card id that group chat belong to
    private short createrId;
    private short[] memberIds;
    private boolean isAlive;

//    public static final Parcelable.Creator<NmsGroupChatContact> CREATOR = new Parcelable.Creator<NmsGroupChatContact>() {
//        public NmsGroupChatContact createFromParcel(Parcel in) {
//            return new NmsGroupChatContact(in);
//        }
//
//        public NmsGroupChatContact[] newArray(int size) {
//            return new NmsGroupChatContact[size];
//        }
//    };

    public NmsGroupChatContact() {

    }

    public NmsGroupChatContact(short jid, int jtype, String jname, String jnumber,
            String jsignature, int jstatus, int jtime, boolean jblocked, int jsimId,
            short jcreaterId, short[] jmemberIds, boolean jisAlive) {
        super(jid, jtype, jname, jnumber, jsignature, jstatus, jtime, jblocked);
        simId = jsimId;
        createrId = jcreaterId;
        memberIds = jmemberIds;
        isAlive = jisAlive;
    }

//    public NmsGroupChatContact(Parcel in) {
//        readFromParcel(in);
//    }

    public String getGuid() {
        return getNumber();
    }

    public void setCreaterId(short createrId) {
        this.createrId = createrId;
    }

    public short getCreaterId() {
        return createrId;
    }

    public void setMemberIds(short[] memberIds) {
        this.memberIds = memberIds;
    }

    public short[] getMemberIds() {
        return memberIds;
    }

    public void setAlive(boolean isAlive) {
        this.isAlive = isAlive;
    }

    public boolean isAlive() {
        return isAlive;
    }

    public int getSimId() {
        return simId;
    }

    public int getMemberCount() {
        int count = 0;

        if (isAlive) {
            count++; // myself
        }

        if (memberIds != null) {
            count += memberIds.length;
        }

        return count;
    }

    public static boolean isGroupChatContactNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            NmsLog.error(TAG, "number is empty, return false");
            return false;
        }

        if (number.length() == NMS_CLIENT_GUID_LEN
                && number.indexOf(NmsGroupChatContact.NMS_GUID_PREFIX) == 0) {
            return true;
        } else {
            return false;
        }
    }

//    @Override
//    public void readFromParcel(Parcel in) {
//        // super.readFromParcel(in);
//
//        simId = in.readInt();
//        createrId = (short) in.readInt();
//        int length = in.readInt();
//        if (length > 0) {
//            int[] ids = new int[length];
//            in.readIntArray(ids);
//            memberIds = new short[length];
//            for (int i = 0; i < length; ++i) {
//                memberIds[i] = (short) ids[i];
//            }
//        }
//        isAlive = (in.readInt() == 1 ? true : false);
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        super.writeToParcel(out, flags);
//
//        out.writeInt(simId);
//        out.writeInt(createrId);
//        if (memberIds != null && memberIds.length > 0) {
//            out.writeInt(memberIds.length);
//            int[] ids = new int[memberIds.length];
//            for (int i = 0; i < memberIds.length; ++i) {
//                ids[i] = memberIds[i];
//            }
//            out.writeIntArray(ids);
//        } else {
//            out.writeInt(0);
//        }
//        out.writeInt(isAlive ? 1 : 0);
//    }
}