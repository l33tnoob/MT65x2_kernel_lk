package com.hissage.contact;

import java.io.Serializable;

public class NmsContact implements Serializable {

    /**
     * 
     */
    private static final long serialVersionUID = -128484149963059237L;
    public static final String TAG = "NmsContact";

    public static final class NmsContactType {
        public static final int NOT_HISSAGE_USER = 0;
        public static final int HISSAGE_USER = 1;
        public static final int HISSAGE_GROUP_CHAT = 2;
        public static final int HISSAGE_BROADCAST = 3;
    }

    public static final class NmsContactStauts {
        public static final int OFFLINE = 0;
        public static final int ONLINE = 1;
        public static final int TYPING = 2;
        public static final int STOP_TYPING = 3;
        public static final int RECORDING = 4;
        public static final int STOP_RECORDING = 5;
        public static final int SKETCHING = 6;
        public static final int STOP_SKETCHING = 7;
        public static final int STATUSCOUNT = 8;
    }

    private short id; /* Hissage db id */
    private int type; /* NmsContactType */
    // private byte[] avatar;
    private String name;
    private String number; /*
                            * for NotHissageUser and HissageUser, it save real
                            * number, for HissageGroup it is virtual number ""
                            */
    private String signature; /* status means signature in QQ */
    private int status; /* NmsContactStauts */
    private int onlineTime; /* last online time */
    private boolean isBlocked;

//    public static final Parcelable.Creator<NmsContact> CREATOR = new Parcelable.Creator<NmsContact>() {
//        public NmsContact createFromParcel(Parcel in) {
//            NmsContact contact = new NmsContact(in);
//            if (contact.getType() == NmsContactType.HISSAGE_GROUP_CHAT) {
//                NmsGroupChatContact gcc = new NmsGroupChatContact();
//                gcc.ConstructContact(contact);
//                gcc.readFromParcel(in);
//                return (NmsGroupChatContact) gcc;
//            } else if (contact.getType() == NmsContactType.HISSAGE_BROADCAST) {
//                NmsBroadCastContact bcc = new NmsBroadCastContact();
//                bcc.ConstructContact(contact);
//                bcc.readFromParcel(in);
//                return (NmsBroadCastContact) bcc;
//            } else {
//                return contact;
//            }
//        }
//
//        public NmsContact[] newArray(int size) {
//            return new NmsContact[size];
//        }
//    };

    public NmsContact(short jid, int jtype, String jname, String jnumber, String jsignature,
            int jstatus, int jtime, boolean jblocked) {
        id = jid;
        type = jtype;
        name = jname;
        number = jnumber;
        signature = jsignature;
        status = jstatus;
        onlineTime = jtime;
        isBlocked = jblocked;
    }

    public NmsContact() {
    }

    public void ConstructContact(NmsContact contact) {
        id = contact.id;
        type = contact.type;
        // avatar = contact.avatar;
        name = contact.name;
        number = contact.number;
        signature = contact.signature;
        status = contact.status;
        onlineTime = contact.onlineTime;
        isBlocked = contact.isBlocked;
    }
//
//    private NmsContact(Parcel in) {
//        readFromParcel(in);
//    }
//
//    @Override
//    public int describeContents() {
//        return 0;
//    }
//
//    public void readFromParcel(Parcel in) {
//        id = (short) in.readInt();
//        type = in.readInt();
//        // int length = in.readInt();
//        // if (length > 0) {
//        // avatar = new byte[length];
//        // in.readByteArray(avatar);
//        // }
//        name = in.readString();
//        number = in.readString();
//        signature = in.readString();
//        status = in.readInt();
//        onlineTime = in.readInt();
//        isBlocked = in.readInt() == 1 ? true : false;
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        out.writeInt(id);
//        out.writeInt(type);
//        // if (avatar != null && avatar.length > 0) {
//        // out.writeInt(avatar.length);
//        // out.writeByteArray(avatar);
//        // } else {
//        // out.writeInt(0);
//        // }
//        out.writeString(name);
//        out.writeString(number);
//        out.writeString(signature);
//        out.writeInt(status);
//        out.writeInt(onlineTime);
//        out.writeInt(isBlocked == true ? 1 : 0);
//    }

    public void setId(short id) {
        this.id = id;
    }

    public short getId() {
        return id;
    }

    public void setType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    // public void setAvatar(byte[] avatar) {
    // this.avatar = avatar;
    // }
    //
    // public byte[] getAvatar() {
    // return avatar;
    // }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setNumber(String number) {
        this.number = number;
    }

    public String getNumber() {
        return number;
    }

    public void setSignature(String signature) {
        this.signature = signature;
    }

    public String getSignature() {
        return signature;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public int getStatus() {
        return status;
    }

    public void setOnlineTime(int onlineTime) {
        this.onlineTime = onlineTime;
    }

    public int getOnlineTime() {
        return onlineTime;
    }

    public void setBlocked(boolean isBlocked) {
        this.isBlocked = isBlocked;
    }

    public boolean isBlocked() {
        return isBlocked;
    }

}
