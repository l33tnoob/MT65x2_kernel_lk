package android.net.wifi;

import android.os.Parcelable;
import android.os.Parcel;
/**
 * {@hide}
 */
public class PPPOEConfig implements Parcelable {
    /** Username from ISP */
    public String username;

    /** User Password */
    public String password;

    public String interf = "wlan0";

    public int lcp_echo_interval = 10;

    public int lcp_echo_failure = 2;

    public int mtu = 1492;

    public int mru = 1492;

    public int timeout = 10;

    public int MSS = 1412;

    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        sbuf.append(" username: ").append(username);
        sbuf.append('\n');
        sbuf.append(" password: ").append(password);
        sbuf.append('\n');
        sbuf.append(" interf: ").append(interf);
        sbuf.append("\n");
        sbuf.append(" lcp_echo_interval: ").append(lcp_echo_interval);
        sbuf.append("\n");
        sbuf.append(" lcp_echo_failure: ").append(lcp_echo_failure);
        sbuf.append("\n");
        sbuf.append(" mtu: ").append(mtu);
        sbuf.append("\n");
        sbuf.append(" mru: ").append(mru);
        sbuf.append("\n");
        sbuf.append(" timeout: ").append(timeout);
        sbuf.append("\n");
        sbuf.append(" MSS: ").append(MSS);
        sbuf.append("\n");
        return sbuf.toString();
    }

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

    public PPPOEConfig() {
    }

    /** copy constructor {@hide} */
    public PPPOEConfig(PPPOEConfig source) {
        if (source != null) {
            username = source.username;
            password = source.password;
            interf = source.interf;
            lcp_echo_interval = source.lcp_echo_interval;
            lcp_echo_failure = source.lcp_echo_failure;
            mtu = source.mtu;
            mru = source.mru;
            timeout = source.timeout;
            MSS = source.MSS;
        }
    }

    /** Implement the Parcelable interface {@hide} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(username);
        dest.writeString(password);
        dest.writeString(interf);
        dest.writeInt(lcp_echo_interval);
        dest.writeInt(lcp_echo_failure);
        dest.writeInt(mtu);
        dest.writeInt(mru);
        dest.writeInt(timeout);
        dest.writeInt(MSS);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<PPPOEConfig> CREATOR =
        new Creator<PPPOEConfig>() {
            public PPPOEConfig createFromParcel(Parcel in) {
                PPPOEConfig result = new PPPOEConfig();
                result.username = in.readString();
                result.password = in.readString();
                result.interf = in.readString();
                result.lcp_echo_interval = in.readInt();
                result.lcp_echo_failure = in.readInt();
                result.mtu = in.readInt();
                result.mru = in.readInt();
                result.timeout = in.readInt();
                result.MSS = in.readInt();
                return result;
            }

            public PPPOEConfig[] newArray(int size) {
                return new PPPOEConfig[size];
            }
        };
}