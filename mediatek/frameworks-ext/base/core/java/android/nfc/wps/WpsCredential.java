package android.nfc.wps;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
* @hide
*/
public class WpsCredential implements Parcelable {
	static final String TAG = "WpsCredential";

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_VERSION = { 0x10, 0x4A };

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_CRIDENTIAL = { 0x10, 0x0E };

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_NETWORK_INDEX = { 0x10, 0x26 };

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_SSID = { 0x10, 0x45 };

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_AUTHENTICATION_TYPE = { 0x10, 0x03 };

	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_ENCRYPTION_TYPE = { 0x10, 0x0F };

	public static byte[] WPS_CREDENTIALATTRIBUTE_ID_NETWORK_KEY = { 0x10, 0x27 };
	
	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_MAC_ADDRESS = { 0x10, 0x20 };
		
	public static byte[] WPS_CREDENTIAL_ATTRIBUTE_ID_AP_CHANNEL = { 0x10, 0x01 };

    public static byte WPS_CREDENTIAL_VERSION_10 = 0x10;

    public static byte[] WPS_CREDENTIAL_AP_CHANNEL = {0x00,0x01};

    public static byte WPS_CREDENTIAL_NETWORK_INDEX = 0x01;

    public static byte[] WPS_CREDENTIAL_VENDOR_EXT = {0x00,0x37,0x2A,0x00,0x01,0x20};

	private byte mNetworkIndex;

	private String mSSID;//byte[] mSSID;
	
	private short mAuthenticationType;//byte[] mAuthenticationType;

	private short mEncryptionType;//byte[] mEncryptionType;
	
	private String mNetworkKey;//byte[] mNetworkKey;
	
	private byte[] mMacAddress;

	private byte[] mApChannel;
    
	private byte[] mVendorExtension;

    public WpsCredential() {
        
        Log.d(TAG, "    WpsCredential()  Construct set default");
        mNetworkIndex = WPS_CREDENTIAL_NETWORK_INDEX;
        mApChannel = WPS_CREDENTIAL_AP_CHANNEL;
        mVendorExtension = WPS_CREDENTIAL_VENDOR_EXT;
    }

	public WpsCredential(Parcel in) {
        //mNetworkIndex = new byte[in.readInt()];
        mNetworkIndex = in.readByte();
        mSSID = in.readString();
        mAuthenticationType = (short)in.readInt();
        mEncryptionType = (short)in.readInt();
        mNetworkKey = in.readString();
        mMacAddress = new byte[in.readInt()];
        in.readByteArray(mMacAddress);
        mApChannel = new byte[in.readInt()];
        in.readByteArray(mApChannel);
        mVendorExtension = new byte[in.readInt()];
        in.readByteArray(mVendorExtension);
	}

	public void writeToParcel(Parcel dest, int flags) {
        //dest.writeInt(mNetworkIndex.length);
        dest.writeByte(mNetworkIndex);
        dest.writeString(mSSID);
        dest.writeInt((int)mAuthenticationType);
        dest.writeInt((int)mEncryptionType);
        dest.writeString(mNetworkKey);
        dest.writeInt(mMacAddress.length);
        dest.writeByteArray(mMacAddress);
        dest.writeInt(mApChannel.length);
        dest.writeByteArray(mApChannel);        
        dest.writeInt(mVendorExtension.length);
        dest.writeByteArray(mVendorExtension);
	}

	// Getter and setter methods
	public void setNetworkIndex(byte networkIndex) {
		mNetworkIndex = networkIndex;
	}

	public void setSSID(String ssid) {
		mSSID = ssid;
	}

	public void setAuthType(short authenticationType) {
		mAuthenticationType = authenticationType;
	}

	public void setEncrypType(short encryptionType) {
		mEncryptionType = encryptionType;
	}
	
	public void setNetworkKey(String networkKey) {
		mNetworkKey = networkKey;
	}

	public void setMacAddress(byte[] macAddress) {
		mMacAddress = macAddress;
	}
    
	public void setApChannel(byte[] apChannel) {
		mApChannel = apChannel;
	}

	public void setVendorExtension (byte[] vendorExtension){
		mVendorExtension = vendorExtension;
	}

	public byte getNetworkIndex() {
		return mNetworkIndex;
	}

	public String getSSID() {
		return mSSID;
	}

	public short getAuthType() {
		return mAuthenticationType;
	}

	public short getEncrypType() {
		return mEncryptionType;
	}
	
	public String getNetworkKey() {
		return mNetworkKey;
	}

	public byte[] getMacAddress() {
		return mMacAddress;
	}

	public byte[] getApChannel() {
		return mApChannel;
	}
    
	public byte[] getVendorExtension() {
		return mVendorExtension;
	}

	public void testPrint() {		

	}


	public static final Parcelable.Creator CREATOR = new Parcelable.Creator() {
		public WpsCredential createFromParcel(Parcel in) {
			return new WpsCredential(in);
		}

		public WpsCredential[] newArray(int size) {
			return new WpsCredential[size];
		}
	};


	public int describeContents() {
		// TODO Auto-generated method stub
		return 0;
	}


}
