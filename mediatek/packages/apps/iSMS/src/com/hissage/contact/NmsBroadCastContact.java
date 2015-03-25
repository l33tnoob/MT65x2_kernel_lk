package com.hissage.contact;

import android.os.Parcel;

public class NmsBroadCastContact extends NmsContact {
    /**
     * 
     */
    private static final long serialVersionUID = 8615495710954311089L;
    private short[] memberIds;

    public NmsBroadCastContact(short jid, int jtype, String jname, String jnumber,
            String jsignature, int jstatus, int jtime, boolean jblocked, short[] jmemberIds) {
        super(jid, jtype, jname, jnumber, jsignature, jstatus, jtime, jblocked);
        memberIds = jmemberIds;
    }
    
    public NmsBroadCastContact(){
        
    }

    public void setMemberIds(short[] memberIds) {
        this.memberIds = memberIds;
    }

    public short[] getMemberIds() {
        return memberIds;
    }

    public int memberCount() {
        int count = 0;

        if (memberIds != null) {
            count += memberIds.length;
        }

        return count;
    }

//    @Override
//    public void readFromParcel(Parcel in) {
//        // super.readFromParcel(in);
//
//        int length = in.readInt();
//        if (length > 0) {
//            int[] ids = new int[length];
//            in.readIntArray(ids);
//            memberIds = new short[length];
//            for (int i = 0; i < length; ++i) {
//                memberIds[i] = (short) ids[i];
//            }
//        }
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        super.writeToParcel(out, flags);
//
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
//    }

}
