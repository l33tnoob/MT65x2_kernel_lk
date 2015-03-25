package android.net.wifi;

import android.os.Parcelable;
import android.os.Parcel;
/**
 * {@hide}
 */
public class PPPOEInfo implements Parcelable {
    public enum Status {
        OFFLINE,
        CONNECTING,
        ONLINE,
    }

    public Status status;

    /** Online time, seconds */
    public long online_time;

    public PPPOEInfo() {
        status = Status.OFFLINE;
    }

    public PPPOEInfo(Status s) {
        status = s;
    }

    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        sbuf.append(" status: ").append(status.toString());
        sbuf.append('\n');
        sbuf.append(" online_time: ").append(online_time);
        sbuf.append("\n");
        return sbuf.toString();
    }

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

    /** copy constructor {@hide} */
    public PPPOEInfo(PPPOEInfo source) {
        if (source != null) {
            status = source.status;
            online_time = source.online_time;
        }
    }

    /** Implement the Parcelable interface {@hide} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(status.name());
        dest.writeLong(online_time);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<PPPOEInfo> CREATOR =
        new Creator<PPPOEInfo>() {
            public PPPOEInfo createFromParcel(Parcel in) {
                PPPOEInfo result = new PPPOEInfo();
                result.status = Status.valueOf(in.readString());
                result.online_time = in.readLong();
                return result;
            }

            public PPPOEInfo[] newArray(int size) {
                return new PPPOEInfo[size];
            }
        };
}