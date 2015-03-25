/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.net.wifi;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * A class representing a hotspot client
 * @hide
 */
public class HotspotClient implements Parcelable {

    /**
     * The MAC address of the client
     * @hide
     * @internal
     */
    public String deviceAddress;

    /**
     * The flag indicates whether this client is blocked or not
     * @hide
     * @internal
     */
    public boolean isBlocked = false;

    /**
     * @hide
     */
    public HotspotClient(String address, boolean blocked) {
        deviceAddress = address;
        isBlocked = blocked;
    }

    /**
     * @hide
     */
    public HotspotClient(HotspotClient source) {
        if (source != null) {
            deviceAddress = source.deviceAddress;
            isBlocked = source.isBlocked;
        }
    }

    /**
     * @hide
     */
    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        sbuf.append(" deviceAddress: ").append(deviceAddress);
        sbuf.append('\n');
        sbuf.append(" isBlocked: ").append(isBlocked);
        sbuf.append("\n");
        return sbuf.toString();
    }

    /** Implement the Parcelable interface {@hide} */
    public int describeContents() {
        return 0;
    }

    /** Implement the Parcelable interface {@hide} */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(deviceAddress);
        dest.writeByte(isBlocked ? (byte)1 : (byte)0);
    }

    /** Implement the Parcelable interface {@hide} */
    public static final Creator<HotspotClient> CREATOR =
        new Creator<HotspotClient>() {
            public HotspotClient createFromParcel(Parcel in) {
                HotspotClient result = new HotspotClient(in.readString(), in.readByte() == 1 ? true : false);
                return result;
            }

            public HotspotClient[] newArray(int size) {
                return new HotspotClient[size];
            }
        };
}
