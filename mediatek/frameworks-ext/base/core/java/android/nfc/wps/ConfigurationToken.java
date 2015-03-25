package android.nfc.wps;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/** @hide */
public class ConfigurationToken implements Parcelable {
	static final String TAG = "ConfigurationToken";

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_VERSION = { 0x10, 0x4A };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_CRIDENTIAL = { 0x10, 0x0E };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_NETWORK_INDEX = { 0x10, 0x26 };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_SSID = { 0x10, 0x45 };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_AUTHENTICATION_TYPE = { 0x10, 0x03 };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_ENCRYPTION_TYPE = { 0x10, 0x0F };

	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_NETWORK_KEY = { 0x10, 0x27 };
	
	public static byte[] mCONFIGURATION_ATTRIBUTE_ID_MAC_ADDRESS = { 0x10, 0x20 };
		
	
	public static byte mCONFIGURATIONTOKEN_VERSION_10 = 0x10;

	private byte[] mNetworkIndex;
	
	private byte[] mSSID;
	
	private byte[] mAuthenticationType;

	private byte[] mEncryptionType;
	
	private byte[] mNetworkKey;
	
	private byte[] mMacAddress;
	
	private byte[] mVendorExtension = new byte[0];

    public ConfigurationToken() {
    }

	public ConfigurationToken(Parcel in) {
        mNetworkIndex = new byte[in.readInt()];
        in.readByteArray(mNetworkIndex);
        mSSID = new byte[in.readInt()];
        in.readByteArray(mSSID);
        mAuthenticationType = new byte[in.readInt()];
        in.readByteArray(mAuthenticationType);
        mEncryptionType = new byte[in.readInt()];
        in.readByteArray(mEncryptionType);
        mNetworkKey = new byte[in.readInt()];
        in.readByteArray(mNetworkKey);
        mMacAddress = new byte[in.readInt()];
        in.readByteArray(mMacAddress);
        mVendorExtension = new byte[in.readInt()];
        in.readByteArray(mVendorExtension);
	}

	public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mNetworkIndex.length);
        dest.writeByteArray(mNetworkIndex);
        dest.writeInt(mSSID.length);
        dest.writeByteArray(mSSID);
        dest.writeInt(mAuthenticationType.length);
        dest.writeByteArray(mAuthenticationType);
        dest.writeInt(mEncryptionType.length);
        dest.writeByteArray(mEncryptionType);
        dest.writeInt(mNetworkKey.length);
        dest.writeByteArray(mNetworkKey);
        dest.writeInt(mMacAddress.length);
        dest.writeByteArray(mMacAddress);
        dest.writeInt(mVendorExtension.length);
        dest.writeByteArray(mVendorExtension);
	}

	// Getter and setter methods
	public void setNetworkIndex(byte[] networkIndex) {
		mNetworkIndex = networkIndex;
	}

	public void setSSID(byte[] ssid) {
		mSSID = ssid;
	}

	public void setAuthType(byte[] authenticationType) {
		mAuthenticationType = authenticationType;
	}

	public void setEncrypType(byte[] encryptionType) {
		mEncryptionType = encryptionType;
	}
	
	public void setNetworkKey(byte[] networkKey) {
		mNetworkKey = networkKey;
	}

	public void setMacAddress(byte[] macAddress) {
		mMacAddress = macAddress;
	}

	public void setVendorExtension (byte[] vendorExtension){
		mVendorExtension = vendorExtension;
	}

	public byte[] getNetworkIndex() {
		return mNetworkIndex;
	}

	public byte[] getSSID() {
		return mSSID;
	}

	public byte[] getAuthType() {
		return mAuthenticationType;
	}

	public byte[] getEncrypType() {
		return mEncryptionType;
	}
	
	public byte[] getNetworkKey() {
		return mNetworkKey;
	}

	public byte[] getMacAddress() {
		return mMacAddress;
	}
	
	public byte[] getVendorExtension() {
		return mVendorExtension;
	}

	public void testPrint() {		

	}


	public static final Parcelable.Creator CREATOR = new Parcelable.Creator() {
		public ConfigurationToken createFromParcel(Parcel in) {
			return new ConfigurationToken(in);
		}

		public ConfigurationToken[] newArray(int size) {
			return new ConfigurationToken[size];
		}
	};


	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}


}
