package com.mediatek.hotknot;

import android.os.Parcel;
import android.os.Parcelable;


/**
 * Represents an immutable HotKnot message.
 * Uses {@link #HotKnotMessage(String mimeType, byte[] data)} to construct
 * a HotKnot message.
 */
public final class HotKnotMessage implements Parcelable {
    private final String mMimeType;
    private final byte[] mData;

    HotKnotMessage(HotKnotMessage msg) {
        mMimeType = msg.getMimeType();
        mData = msg.getData();
    }

    /**
     * Constructs a HotKnot Message.
     * The mimeType parameter will be normalized to follow Android best
     * practices for intent filtering.
     * @param mimeType a valid MIME type
     * @param data payload as bytes
     */
    public HotKnotMessage(String mimeType, byte[] data) {
        if (mimeType == null) {
            throw new NullPointerException("mimeType cannot be null");
        }

        mMimeType = mimeType;
        mData = data;
    }

    /**
     * Returns the MIME type.
     */
    public String getMimeType() {
        return mMimeType;
    }

    /**
     * Gets the data payload.
     */
    public byte[] getData() {
        return mData;
    }

    /** @hide */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("HotKnotMessage: ");
        synchronized (this) {
            builder.append(mMimeType);
            if(mData != null) {
                builder.append(" length:" + mData.length);
            }
        }
        return builder.toString();
    }

    /** @hide */
    @Override
    public int describeContents() {

        return 0;
    }

    /** @hide */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(mMimeType);
        if(mData != null) {
            dest.writeInt(mData.length);
            dest.writeByteArray(mData);
        } else {
            dest.writeInt(0);
        }
    }

    /** @hide */
    public static final Parcelable.Creator<HotKnotMessage> CREATOR = new Parcelable.Creator<HotKnotMessage>() {
        @Override
        public HotKnotMessage createFromParcel(Parcel in) {
            String mimeType = null;
            byte[] data = null;
            mimeType = in.readString();
            int datalen = in.readInt();
            if(datalen > 0) {
                data = new byte[datalen];
                in.readByteArray(data);
            }
            return new HotKnotMessage(mimeType, data);
        }

        @Override
        public HotKnotMessage[] newArray(int size) {
            return new HotKnotMessage[size];
        }
    };

}
