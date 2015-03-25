package com.hissage.struct;

import java.io.Serializable;


public class SNmsSimInfo implements Serializable {

    /**
     * 
     */
    private static final long serialVersionUID = 1797865557409986781L;

    public static final class NmsSimActivateStatus {
        final public static int NMS_SIM_STATUS_NOT_EXIST = 0; // not exist
        final public static int NMS_SIM_STATUS_INVALID = 1; // can not get the
                                                            // number for
                                                            // activated
        final public static int NMS_SIM_STATUS_ACTIVATING = 2; // activating
        final public static int NMS_SIM_STATUS_NOT_ACTIVATED = 3; // not
                                                                  // activated
        final public static int NMS_SIM_STATUS_ACTIVATED = 4; // activated
        final public static int NMS_SIM_STATUS_DISABLED = 5; // disable (must
                                                             // activated first)
    }

    public int simId; // sim id in system db
    public int status; // status of class NmsSimActivateStatus
    public String imsi; // imsi
    public String number; // number, may be empty

//    public static final Parcelable.Creator<SNmsSimInfo> CREATOR = new Parcelable.Creator<SNmsSimInfo>() {
//        public SNmsSimInfo createFromParcel(Parcel in) {
//            return new SNmsSimInfo(in);
//        }
//
//        public SNmsSimInfo[] newArray(int size) {
//            return new SNmsSimInfo[size];
//        }
//    };

    public SNmsSimInfo() {
    }

//    private SNmsSimInfo(Parcel in) {
//        readFromParcel(in);
//    }
//
//    @Override
//    public int describeContents() {
//        return 0;
//    }
//
//    public void readFromParcel(Parcel in) {
//        simId = (short) in.readInt();
//        status = in.readInt();
//        imsi = in.readString();
//        number = in.readString();
//
//    }
//
//    @Override
//    public void writeToParcel(Parcel out, int flags) {
//        out.writeInt(simId);
//        out.writeInt(status);
//        out.writeString(imsi);
//        out.writeString(number);
//    }
}
