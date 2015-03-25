package com.mediatek.common.agps;

import android.os.Parcel;
import android.os.Parcelable;

public final class MtkAgpsCdmaProfile implements Parcelable {
    public String   mName;          // 1. AGPS profile name
    public int      mMcpEnable;     // 2.
    public String   mMcpAddr;       // 3.
    public int      mMcpPort;       // 4.
    public int      mPdeAddrValid;  // 5.
    public int      mPdeIpType;     // 6.
    public String   mPdeIp4Addr;    // 7.
    public String   mPdeIp6Addr;    // 8.
    public int      mPdePort;       // 9.
    public int      mPdeUrlValid;   // 10.
    public String   mPdeUrlAddr;    // 11.

    public String toString() {
        String str = new String();
        str = " MtkAgpsCdmaProfile mName=[" + mName + "] mMcpEnable=[" + mMcpEnable + "] mMcpAddr=[" + mMcpAddr + 
            "] mMcpPort=[" + mMcpPort + "] mPdeAddrValid=[" + mPdeAddrValid + "] mPdeIpType=[" + mPdeIpType + 
            "] mPdeIp4Addr=[" + mPdeIp4Addr + "] mPdeIp6Addr=[" + mPdeIp6Addr + "] mPdePort=[" + mPdePort +
            "] mPdeUrlValid=[" + mPdeUrlValid + "] mPdeUrlAddr=[" + mPdeUrlAddr + "]";
        return str;
    }

    public static final Parcelable.Creator<MtkAgpsCdmaProfile> CREATOR = new Parcelable.Creator<MtkAgpsCdmaProfile>() {
        public MtkAgpsCdmaProfile createFromParcel(Parcel in) {
            MtkAgpsCdmaProfile profile = new MtkAgpsCdmaProfile();
            profile.readFromParcel(in);
            return profile;
        }
        public MtkAgpsCdmaProfile[] newArray(int size) {
            return new MtkAgpsCdmaProfile[size];
        }
    };

    public MtkAgpsCdmaProfile() {}
    public MtkAgpsCdmaProfile(String name, int mcpEnable, String mcpAddr, int mcpPort, 
        int pdeAddrValid, int pdeIpType, String pdeIp4Addr, String pdeIp6Addr, int pdePort, 
        int pdeUrlValid, String pdeUrlAddr) {
        this.mName              = name;
        this.mMcpEnable         = mcpEnable;
        this.mMcpAddr           = mcpAddr;
        this.mMcpPort           = mcpPort;
        this.mPdeAddrValid      = pdeAddrValid;
        this.mPdeIpType         = pdeIpType;
        this.mPdeIp4Addr        = pdeIp4Addr;
        this.mPdeIp6Addr        = pdeIp6Addr;
        this.mPdePort           = pdePort;
        this.mPdeUrlValid       = pdeUrlValid;
        this.mPdeUrlAddr        = pdeUrlAddr;
    }

    //@Override
    public int describeContents() {
        return 0;
    }

    //@Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mName);
        out.writeInt(mMcpEnable);
        out.writeString(mMcpAddr);
        out.writeInt(mMcpPort);
        out.writeInt(mPdeAddrValid);
        out.writeInt(mPdeIpType);
        out.writeString(mPdeIp4Addr);
        out.writeString(mPdeIp6Addr);
        out.writeInt(mPdePort);
        out.writeInt(mPdeUrlValid);
        out.writeString(mPdeUrlAddr);
    }

    //@Override
    public void readFromParcel(Parcel in) {
        mName           = in.readString();
        mMcpEnable      = in.readInt();
        mMcpAddr        = in.readString();
        mMcpPort        = in.readInt();
        mPdeAddrValid   = in.readInt();
        mPdeIpType      = in.readInt();
        mPdeIp4Addr     = in.readString();
        mPdeIp6Addr     = in.readString();
        mPdePort        = in.readInt();
        mPdeUrlValid    = in.readInt();
        mPdeUrlAddr     = in.readString();
    }
}
